/* KernelPolicyGuard - network hook implementation */
#include <linux/ftrace.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/netfilter.h>
#include <net/inet_sock.h>

#include "../../policy/store.h"

static int chronos_net_fprobe_entry(struct fprobe *fp, struct pt_regs *regs,
				    unsigned long ip)
{
	struct sk_buff *skb;
	struct inet_sock *inet;
	__be16 dport;

	skb = (struct sk_buff *)regs_get_argument(regs, 0);
	if (!skb || !skb->sk)
		return 0;

	inet = inet_sk(skb->sk);
	dport = ntohs(inet->inet_dport);

	if (dport == 0)
		return 0;

	if (dport == 4444) {
		fp->retval = -EPERM;
		kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
		return 0;
	}

	return 0;
}

static int chronos_net_fprobe_exit(struct fprobe *fp, struct pt_regs *regs,
				   unsigned long ip)
{
	if (fp->retval == -EPERM && regs)
		ftrace_override_function_return(regs, fp->retval);

	return 0;
}

int chronos_net_hook_register(struct fprobe *fp)
{
	fp->entry_handler = chronos_net_fprobe_entry;
	fp->exit_handler = chronos_net_fprobe_exit;
	fp->func = "ip_rcv_core";
	return register_fprobe(fp);
}
