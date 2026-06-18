#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
/* KernelPolicyGuard - main kernel module for dynamic policy enforcement */

#include <linux/atomic.h>
#include <linux/errno.h>
#include <linux/ftrace.h>
#include <linux/genetlink.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/kprobes.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/seqlock.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KernelPolicyGuard Research");
MODULE_DESCRIPTION("KernelPolicyGuard - Dynamic kernel policy enforcement via fprobe/kprobe hooks");
MODULE_VERSION("0.1");

#define CHRONOS_MAX_RULES 16
#define MAX_PENDING_WORK 1000
#define CHRONOS_ACTION_ALLOW 0
#define CHRONOS_ACTION_BLOCK 1

enum chronos_hook_id {
	CHRONOS_HOOK_NONE = 0,
	CHRONOS_HOOK_IP,
	CHRONOS_HOOK_FILE_OPEN,
	CHRONOS_HOOK_TASK,
	CHRONOS_HOOK_NET,
	CHRONOS_HOOK_IPC,
	CHRONOS_HOOK_INODE,
};

enum chronos_rule_op {
	CHRONOS_OP_NONE = 0,
	CHRONOS_OP_EQ,
	CHRONOS_OP_GT,
	CHRONOS_OP_LT,
	CHRONOS_OP_MASK,
};

struct chronos_rule {
	enum chronos_hook_id hook;
	enum chronos_rule_op op;
	u64 val;
	u32 action;
};

struct chronos_policy_config {
	bool enabled;
	u32 rule_count;
	struct chronos_rule rules[CHRONOS_MAX_RULES];
};

struct chronos_work_item {
	struct work_struct work;
	unsigned long ip;
};

static struct chronos_policy_config chronos_cfg = {
	.enabled = true,
};
static seqcount_t chronos_cfg_seq = SEQCNT_ZERO(chronos_cfg_seq);
static atomic_t chronos_pending_work_count = ATOMIC_INIT(0);
static atomic_t chronos_dropped_events = ATOMIC_INIT(0);

static struct kobject *chronos_kobj;
static struct workqueue_struct *chronos_wq;
static struct fprobe chronos_fp = { 0 };
static struct kprobe chronos_kp = { 0 };
static DEFINE_MUTEX(chronos_lock);
static bool chronos_fprobe_registered;
static bool chronos_kprobe_registered;

enum chronos_genl_cmd {
	KPG_CMD_UNSPEC = 0,
	KPG_CMD_GET_STATS,
	KPG_CMD_SET_POLICY,
	__KPG_CMD_MAX,
};

enum chronos_genl_attr {
	KPG_ATTR_UNSPEC = 0,
	KPG_ATTR_ENABLED,
	KPG_ATTR_PENDING,
	KPG_ATTR_DROPPED,
	KPG_ATTR_RULE_COUNT,
	KPG_ATTR_RULES,
	__KPG_ATTR_MAX,
};

static int chronos_genl_get_stats(struct sk_buff *skb,
				 struct genl_info *info);
static int chronos_genl_set_policy(struct sk_buff *skb,
				 struct genl_info *info);

static struct genl_ops chronos_genl_ops[] = {
	{
		.cmd = KPG_CMD_GET_STATS,
		.doit = chronos_genl_get_stats,
	},
	{
		.cmd = KPG_CMD_SET_POLICY,
		.doit = chronos_genl_set_policy,
	},
};

static struct genl_family chronos_genl_family = {
	.name = "KERNEL_POLICY_GUARD",
	.version = 1,
	.module = THIS_MODULE,
	.ops = chronos_genl_ops,
	.n_ops = ARRAY_SIZE(chronos_genl_ops),
	.maxattr = __KPG_ATTR_MAX,
};

static void chronos_policy_worker(struct work_struct *work);

static void chronos_get_config(struct chronos_policy_config *cfg)
{
	unsigned int seq;

	do {
		seq = read_seqcount_begin(&chronos_cfg_seq);
		cfg->enabled = READ_ONCE(chronos_cfg.enabled);
		cfg->rule_count = READ_ONCE(chronos_cfg.rule_count);
		memcpy(cfg->rules, chronos_cfg.rules,
		       sizeof(cfg->rules));
	} while (read_seqcount_retry(&chronos_cfg_seq, seq));
}

static bool chronos_rule_matches(const struct chronos_rule *rule,
				 unsigned long ip)
{
	switch (rule->op) {
	case CHRONOS_OP_EQ:
		return rule->hook == CHRONOS_HOOK_IP && ip == (unsigned long)rule->val;
	case CHRONOS_OP_GT:
		return rule->hook == CHRONOS_HOOK_IP && ip > (unsigned long)rule->val;
	case CHRONOS_OP_LT:
		return rule->hook == CHRONOS_HOOK_IP && ip < (unsigned long)rule->val;
	case CHRONOS_OP_MASK:
		return rule->hook == CHRONOS_HOOK_IP &&
		       ((unsigned long)rule->val & ip) == (unsigned long)rule->val;
	default:
		return false;
	}
}

static int chronos_verify_policy(unsigned long ip)
{
	struct chronos_policy_config cfg;
	unsigned int i;

	chronos_get_config(&cfg);

	if (!cfg.enabled)
		return 0;

	for (i = 0; i < cfg.rule_count; i++) {
		if (chronos_rule_matches(&cfg.rules[i], ip)) {
			if (cfg.rules[i].action == CHRONOS_ACTION_BLOCK)
				return -EPERM;
			return 0;
		}
	}

	return 0;
}

static void chronos_policy_worker(struct work_struct *work)
{
	struct chronos_work_item *item =
		container_of(work, struct chronos_work_item, work);

	chronos_verify_policy(item->ip);
	atomic_dec(&chronos_pending_work_count);
	kfree(item);
}

static int chronos_fprobe_handler(struct fprobe *fp, struct pt_regs *regs,
				 unsigned long ip)
{
	struct chronos_work_item *item;

	if (!READ_ONCE(chronos_cfg.enabled) || !regs)
		return 0;

	if (atomic_read(&chronos_pending_work_count) >= MAX_PENDING_WORK) {
		atomic_inc(&chronos_dropped_events);
		return 0;
	}

	item = kmalloc(sizeof(*item), GFP_ATOMIC);
	if (!item)
		return 0;

	item->ip = ip;
	INIT_WORK(&item->work, chronos_policy_worker);
	atomic_inc(&chronos_pending_work_count);

	if (!queue_work(chronos_wq, &item->work)) {
		atomic_dec(&chronos_pending_work_count);
		kfree(item);
		return 0;
	}

	return 0;
}

static int chronos_kprobe_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct chronos_policy_config cfg;

	chronos_get_config(&cfg);

	if (!cfg.enabled)
		return 0;

	chronos_verify_policy((unsigned long)p->addr);
	return 0;
}

static ssize_t chronos_config_show(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	struct chronos_policy_config cfg;
	ssize_t ret;

	chronos_get_config(&cfg);
	ret = scnprintf(buf, PAGE_SIZE,
			 "enabled=%u\nrule_count=%u\ndropped_events=%u\n",
			 cfg.enabled ? 1 : 0,
			 cfg.rule_count,
			 atomic_read(&chronos_dropped_events));

	return ret;
}

static ssize_t chronos_config_store(struct kobject *kobj,
				  struct kobj_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned int enabled = 0;

	if (sscanf(buf, "enabled=%u", &enabled) == 1) {
		mutex_lock(&chronos_lock);
		write_seqcount_begin(&chronos_cfg_seq);
		chronos_cfg.enabled = enabled ? true : false;
		write_seqcount_end(&chronos_cfg_seq);
		mutex_unlock(&chronos_lock);
		return count;
	}

	return -EINVAL;
}

static ssize_t chronos_dropped_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n",
			 atomic_read(&chronos_dropped_events));
}

static ssize_t chronos_rules_show(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	struct chronos_policy_config cfg;
	ssize_t ret = 0;
	unsigned int i;

	chronos_get_config(&cfg);
	for (i = 0; i < cfg.rule_count; i++) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret,
				 "%u:%u:%llu:%u\n",
				 cfg.rules[i].hook,
				 cfg.rules[i].op,
				 (unsigned long long)cfg.rules[i].val,
				 cfg.rules[i].action);
	}

	return ret;
}

static ssize_t chronos_rules_store(struct kobject *kobj,
				  struct kobj_attribute *attr,
				  const char *buf, size_t count)
{
	struct chronos_policy_config cfg;
	unsigned int hook = 0;
	unsigned int op = 0;
	u64 val = 0;
	unsigned int action = 0;
	unsigned int i;

	if (!strncmp(buf, "clear", 5)) {
		mutex_lock(&chronos_lock);
		write_seqcount_begin(&chronos_cfg_seq);
		memset(chronos_cfg.rules, 0, sizeof(chronos_cfg.rules));
		chronos_cfg.rule_count = 0;
		write_seqcount_end(&chronos_cfg_seq);
		mutex_unlock(&chronos_lock);
		return count;
	}

	if (sscanf(buf, "add hook=%u op=%u val=%llu action=%u",
			  &hook, &op, (unsigned long long *)&val, &action) == 4) {
		chronos_get_config(&cfg);
		if (cfg.rule_count >= CHRONOS_MAX_RULES)
			return -ENOSPC;
		for (i = 0; i < cfg.rule_count; i++)
			if (cfg.rules[i].hook == hook &&
			    cfg.rules[i].op == op &&
			    cfg.rules[i].val == val &&
			    cfg.rules[i].action == action)
				return -EEXIST;
		mutex_lock(&chronos_lock);
		write_seqcount_begin(&chronos_cfg_seq);
		chronos_cfg.rules[chronos_cfg.rule_count].hook = hook;
		chronos_cfg.rules[chronos_cfg.rule_count].op = op;
		chronos_cfg.rules[chronos_cfg.rule_count].val = val;
		chronos_cfg.rules[chronos_cfg.rule_count].action = action;
		chronos_cfg.rule_count++;
		write_seqcount_end(&chronos_cfg_seq);
		mutex_unlock(&chronos_lock);
		return count;
	}

	return -EINVAL;
}

static struct kobj_attribute chronos_cfg_attr =
	__ATTR(config, 0644, chronos_config_show, chronos_config_store);
static struct kobj_attribute chronos_dropped_attr =
	__ATTR(dropped_events, 0444, chronos_dropped_show, NULL);
static struct kobj_attribute chronos_rules_attr =
	__ATTR(rules, 0644, chronos_rules_show, chronos_rules_store);

static int chronos_genl_get_stats(struct sk_buff *skb,
				 struct genl_info *info)
{
	struct sk_buff *reply;
	void *hdr;
	struct chronos_policy_config cfg;

	reply = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!reply)
		return -ENOMEM;

	hdr = genlmsg_put(reply, info->snd_portid, info->snd_seq,
			 &chronos_genl_family, 0, KPG_CMD_GET_STATS);
	if (IS_ERR(hdr))
		goto err_free;

	chronos_get_config(&cfg);
	nla_put_u32(reply, KPG_ATTR_ENABLED, cfg.enabled ? 1 : 0);
	nla_put_u32(reply, KPG_ATTR_PENDING,
			 atomic_read(&chronos_pending_work_count));
	nla_put_u32(reply, KPG_ATTR_DROPPED,
			 atomic_read(&chronos_dropped_events));
	nla_put_u32(reply, KPG_ATTR_RULE_COUNT, cfg.rule_count);
	genlmsg_end(reply, hdr);
	return genlmsg_unicast(&init_net, reply, info->snd_portid);

err_free:
	kfree_skb(reply);
	return -ENOMEM;
}

static int chronos_genl_set_policy(struct sk_buff *skb,
				 struct genl_info *info)
{
	struct chronos_policy_config cfg;
	u32 enabled = 0;
	u32 rule_count = 0;

	if (info->attrs[KPG_ATTR_ENABLED])
		enabled = nla_get_u32(info->attrs[KPG_ATTR_ENABLED]);
	if (info->attrs[KPG_ATTR_RULE_COUNT])
		rule_count = nla_get_u32(info->attrs[KPG_ATTR_RULE_COUNT]);

	mutex_lock(&chronos_lock);
	write_seqcount_begin(&chronos_cfg_seq);
	chronos_cfg.enabled = enabled ? true : false;
	if (rule_count <= CHRONOS_MAX_RULES)
		chronos_cfg.rule_count = rule_count;
	write_seqcount_end(&chronos_cfg_seq);
	mutex_unlock(&chronos_lock);

	chronos_get_config(&cfg);
	if (cfg.rule_count > CHRONOS_MAX_RULES)
		cfg.rule_count = CHRONOS_MAX_RULES;

	return 0;
}

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
	ret = sysfs_create_file(chronos_kobj, &chronos_dropped_attr.attr);
	if (ret)
		goto err_remove_cfg;
	ret = sysfs_create_file(chronos_kobj, &chronos_rules_attr.attr);
	if (ret)
		goto err_remove_dropped;

	ret = genl_register_family(&chronos_genl_family);
	if (ret)
		goto err_remove_rules;

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
			goto err_unregister_genl;
	}

	return 0;

err_unregister_genl:
	genl_unregister_family(&chronos_genl_family);
err_remove_rules:
	sysfs_remove_file(chronos_kobj, &chronos_rules_attr.attr);
err_remove_dropped:
	sysfs_remove_file(chronos_kobj, &chronos_dropped_attr.attr);
err_remove_cfg:
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

	genl_unregister_family(&chronos_genl_family);

	if (chronos_wq) {
		flush_workqueue(chronos_wq);
		destroy_workqueue(chronos_wq);
	}

	if (chronos_kobj) {
		sysfs_remove_file(chronos_kobj, &chronos_rules_attr.attr);
		sysfs_remove_file(chronos_kobj, &chronos_dropped_attr.attr);
		sysfs_remove_file(chronos_kobj, &chronos_cfg_attr.attr);
		kobject_put(chronos_kobj);
	}
}

module_init(chronos_init);
module_exit(chronos_exit);
