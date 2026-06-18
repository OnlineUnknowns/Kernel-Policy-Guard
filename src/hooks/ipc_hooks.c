/* KernelPolicyGuard - IPC hook implementation */
#include <linux/ftrace.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/ipc.h>
#include <linux/shm.h>
#include <linux/capability.h>
#include <linux/cred.h>
#include <linux/sched.h>

#include "../../policy/store.h"

static int chronos_ipc_fprobe_entry(struct fprobe *fp, struct pt_regs *regs,
				    unsigned long ip)
{
	const struct chronos_policy *policy;
	kuid_t uid;

	policy = chronos_policy_lookup();
	if (!policy)
		return 0;

	uid = get_current_uid();
	if (uid.val == 0)
		return 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	return 0;
}

static int chronos_ipc_fprobe_exit(struct fprobe *fp, struct pt_regs *regs,
				   unsigned long ip)
{
	if (fp->retval && regs)
		ftrace_override_function_return(regs, fp->retval);

	return 0;
}

int chronos_ipc_hook_register(struct fprobe *fp)
{
	fp->entry_handler = chronos_ipc_fprobe_entry;
	fp->exit_handler = chronos_ipc_fprobe_exit;
	fp->func = "shmget";
	return register_fprobe(fp);
}
