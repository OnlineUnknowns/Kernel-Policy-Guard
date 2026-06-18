/* KernelPolicyGuard - inode hook implementation */
#include <linux/ftrace.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/uidgid.h>

#include "../../policy/store.h"

static int chronos_inode_fprobe_entry(struct fprobe *fp, struct pt_regs *regs,
				      unsigned long ip)
{
	const struct chronos_policy *policy;
	kuid_t uid;

	policy = chronos_policy_lookup();
	if (!policy)
		return 0;

	uid = get_current_uid();
	if (uid.val != 0)
		return 0;

	return 0;
}

static int chronos_inode_fprobe_exit(struct fprobe *fp, struct pt_regs *regs,
				     unsigned long ip)
{
	if (regs && (unsigned long)regs_return_value(regs) == 0)
		return 0;

	if (regs)
		ftrace_override_function_return(regs, -EPERM);

	return 0;
}

int chronos_inode_hook_register(struct fprobe *fp)
{
	fp->entry_handler = chronos_inode_fprobe_entry;
	fp->exit_handler = chronos_inode_fprobe_exit;
	fp->func = "security_inode_create";
	return register_fprobe(fp);
}
