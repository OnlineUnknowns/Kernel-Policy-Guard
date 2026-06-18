#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
/* KernelPolicyGuard - original kernel module reference for dynamic policy enforcement */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ftrace.h>
#include <linux/kobject.h>
#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KernelPolicyGuard Research");
MODULE_DESCRIPTION("KernelPolicyGuard - Dynamic kernel policy enforcement via fprobe/kprobe hooks");
MODULE_VERSION("0.1");

struct chronos_policy_config {
	bool enabled;
	u32 threshold;
};

struct chronos_work_item {
	struct work_struct work;
	unsigned long ip;
};

static struct chronos_policy_config chronos_cfg = {
	.enabled = true,
	.threshold = 64,
};

static struct kobject *chronos_kobj;
static struct workqueue_struct *chronos_wq;
static struct fprobe chronos_fp = { 0 };
static struct kprobe chronos_kp = { 0 };
static DEFINE_MUTEX(chronos_lock);
static bool chronos_fprobe_registered;
static bool chronos_kprobe_registered;

static void chronos_policy_worker(struct work_struct *work);

static int chronos_verify_policy(unsigned long ip)
{
	if (!chronos_cfg.enabled)
		return 0;

	if (chronos_cfg.threshold == 0)
		return 0;

	pr_debug("chronos: policy check for ip=%#lx threshold=%u\n",
		 ip, chronos_cfg.threshold);

	return 0;
}

static void chronos_policy_worker(struct work_struct *work)
{
	struct chronos_work_item *item =
		container_of(work, struct chronos_work_item, work);

	chronos_verify_policy(item->ip);
	kfree(item);
}

static int chronos_fprobe_handler(struct fprobe *fp, struct pt_regs *regs,
				 unsigned long ip)
{
	struct chronos_work_item *item;

	if (!chronos_cfg.enabled || !regs)
		return 0;

	item = kmalloc(sizeof(*item), GFP_ATOMIC);
	if (!item)
		return 0;

	item->ip = ip;
	INIT_WORK(&item->work, chronos_policy_worker);

	if (!queue_work(chronos_wq, &item->work)) {
		kfree(item);
		return 0;
	}

	return 0;
}

static int chronos_kprobe_pre(struct kprobe *p, struct pt_regs *regs)
{
	if (!chronos_cfg.enabled)
		return 0;

	chronos_verify_policy((unsigned long)p->addr);
	return 0;
}

static ssize_t chronos_config_show(struct kobject *kobj,
			  struct kobj_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&chronos_lock);
	ret = scnprintf(buf, PAGE_SIZE,
			 "enabled=%u\nthreshold=%u\n",
			 chronos_cfg.enabled ? 1 : 0,
			 chronos_cfg.threshold);
	mutex_unlock(&chronos_lock);

	return ret;
}

static ssize_t chronos_config_store(struct kobject *kobj,
			  struct kobj_attribute *attr,
			  const char *buf, size_t count)
{
	unsigned int enabled = 0;
	unsigned int threshold = 0;

	if (sscanf(buf, "enabled=%u threshold=%u", &enabled, &threshold) == 2) {
		mutex_lock(&chronos_lock);
		chronos_cfg.enabled = enabled ? true : false;
		chronos_cfg.threshold = threshold;
		mutex_unlock(&chronos_lock);
		return count;
	}

	if (sscanf(buf, "enabled=%u", &enabled) == 1) {
		mutex_lock(&chronos_lock);
		chronos_cfg.enabled = enabled ? true : false;
		mutex_unlock(&chronos_lock);
		return count;
	}

	return -EINVAL;
}

static struct kobj_attribute chronos_cfg_attr =
	__ATTR(config, 0644, chronos_config_show, chronos_config_store);

static int __init chronos_init(void)
{
	int ret;

	chronos_wq = create_singlethread_workqueue("chronos_wq");
	if (!chronos_wq)
		return -ENOMEM;

	chronos_kobj = kobject_create_and_add("chronos", kernel_kobj);
	if (!chronos_kobj) {
		ret = -ENOMEM;
		goto err_destroy_wq;
	}

	ret = sysfs_create_file(chronos_kobj, &chronos_cfg_attr.attr);
	if (ret)
		goto err_put_kobj;

	chronos_fp.func = "security_file_open";
	chronos_fp.entry_handler = chronos_fprobe_handler;
	ret = register_fprobe(&chronos_fp);
	if (!ret) {
		chronos_fprobe_registered = true;
	} else {
		chronos_kp.pre_handler = chronos_kprobe_pre;
		chronos_kp.symbol_name = "security_file_open";
		ret = register_kprobe(&chronos_kp);
		if (!ret)
			chronos_kprobe_registered = true;
		else
			goto err_remove_sysfs;
	}

	return 0;

err_remove_sysfs:
	sysfs_remove_file(chronos_kobj, &chronos_cfg_attr.attr);
err_put_kobj:
	kobject_put(chronos_kobj);
err_destroy_wq:
	destroy_workqueue(chronos_wq);
	return ret;
}

static void __exit chronos_exit(void)
{
	if (chronos_fprobe_registered)
		unregister_fprobe(&chronos_fp);

	if (chronos_kprobe_registered)
		unregister_kprobe(&chronos_kp);

	if (chronos_wq) {
		flush_workqueue(chronos_wq);
		destroy_workqueue(chronos_wq);
	}

	if (chronos_kobj) {
		sysfs_remove_file(chronos_kobj, &chronos_cfg_attr.attr);
		kobject_put(chronos_kobj);
	}
}

module_init(chronos_init);
module_exit(chronos_exit);
