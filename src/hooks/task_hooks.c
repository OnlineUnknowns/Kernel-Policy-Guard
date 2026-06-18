/* KernelPolicyGuard - task hook implementation */
#include <linux/ftrace.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched/task.h>
#include <linux/uidgid.h>

#include "../../policy/store.h"

static int chronos_task_fprobe_entry(struct fprobe *fp, struct pt_regs *regs,
				     unsigned long ip)
{
	const struct chronos_policy *policy;
	struct task_struct *parent;
	struct task_struct *child;
	kuid_t uid;

	policy = chronos_policy_lookup();
	if (!policy)
		return 0;

	parent = current;
	uid = get_task_uid(parent);

	if (uid.val == 0)
		return 0;

	child = (struct task_struct *)regs_get_argument(regs, 0);
	if (!child)
		return 0;

	task_clear_flag(child, PF_FORKNOEXEC);
	task_clear_flag(child, PF_SUPERPRIV);

	return 0;
}

static int chronos_task_fprobe_exit(struct fprobe *fp, struct pt_regs *regs,
				    unsigned long ip)
{
	return 0;
}

int chronos_task_hook_register(struct fprobe *fp)
{
	fp->entry_handler = chronos_task_fprobe_entry;
	fp->exit_handler = chronos_task_fprobe_exit;
	fp->func = "do_fork";
	return register_fprobe(fp);
}
