/* KernelPolicyGuard - audit telemetry logging */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ring_buffer.h>
#include <linux/ktime.h>
#include <linux/types.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/splice.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define CHRONOS_AUDIT_BUF_SIZE (1 << 16)

struct chronos_event {
	__u64 timestamp;
	__u32 hook_id;
	__u32 uid;
};

static struct ring_buffer *chronos_trace_buffer;
static struct kobject *chronos_audit_kobj;

static ssize_t chronos_audit_read(struct file *file, char __user *buf,
				 size_t len, loff_t *ppos)
{
	struct ring_buffer_event *event;
	struct chronos_event *payload;
	size_t copied = 0;

	event = ring_buffer_read_prepare(chronos_trace_buffer, *ppos);
	if (!event)
		return 0;

	payload = ring_buffer_event_data(event);
	copied = sizeof(*payload);
	if (copied > len)
		copied = len;

	if (copy_to_user(buf, payload, copied)) {
		ring_buffer_read_finish(chronos_trace_buffer, event);
		return -EFAULT;
	}

	ring_buffer_read_finish(chronos_trace_buffer, event);
	*ppos += copied;
	return copied;
}

static int chronos_audit_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations chronos_audit_fops = {
	.owner = THIS_MODULE,
	.open = chronos_audit_open,
	.read = chronos_audit_read,
};

static ssize_t chronos_audit_sysfs_show(struct kobject *kobj,
					 struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "trace_buffer=%ps\n", chronos_trace_buffer);
}

static struct kobj_attribute chronos_audit_attr =
	__ATTR(trace, 0444, chronos_audit_sysfs_show, NULL);

int chronos_submit_audit(u32 hook_id, u32 uid)
{
	struct ring_buffer_event *event;
	struct chronos_event *payload;

	if (!chronos_trace_buffer)
		return -ENODEV;

	event = ring_buffer_lock_reserve(chronos_trace_buffer,
						 0, sizeof(struct chronos_event), 0);
	if (!event)
		return -EAGAIN;

	payload = ring_buffer_event_data(event);
	payload->timestamp = ktime_get_real_ns();
	payload->hook_id = hook_id;
	payload->uid = uid;

	ring_buffer_unlock_commit(chronos_trace_buffer, event, 0);
	return 0;
}

static int __init chronos_audit_init(void)
{
	int ret;

	chronos_trace_buffer = ring_buffer_alloc(CHRONOS_AUDIT_BUF_SIZE,
					      RB_FL_OVERWRITE);
	if (!chronos_trace_buffer)
		return -ENOMEM;

	chronos_audit_kobj = kobject_create_and_add("chronos_audit", kernel_kobj);
	if (!chronos_audit_kobj) {
		ring_buffer_free(chronos_trace_buffer);
		return -ENOMEM;
	}

	ret = sysfs_create_file(chronos_audit_kobj, &chronos_audit_attr.attr);
	if (ret) {
		kobject_put(chronos_audit_kobj);
		ring_buffer_free(chronos_trace_buffer);
		return ret;
	}

	return 0;
}

static void __exit chronos_audit_exit(void)
{
	if (chronos_audit_kobj) {
		sysfs_remove_file(chronos_audit_kobj, &chronos_audit_attr.attr);
		kobject_put(chronos_audit_kobj);
	}

	if (chronos_trace_buffer) {
		ring_buffer_free(chronos_trace_buffer);
	}
}

module_init(chronos_audit_init);
module_exit(chronos_audit_exit);
