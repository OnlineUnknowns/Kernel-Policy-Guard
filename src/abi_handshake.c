/* KernelPolicyGuard - ABI handshake implementation */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/timekeeping.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "../include/uapi/chronos_abi.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KernelPolicyGuard Research");
MODULE_DESCRIPTION("KernelPolicyGuard - ABI handshake scaffold");

static __u64 chronos_secret_seed;
static DECLARE_WAIT_QUEUE_HEAD(chronos_waitq);

static inline void pack_for_userspace(struct chronos_abi_layout *src,
				      struct chronos_abi_user_view *dst)
{
	dst->flags = src->flags;
	dst->token = src->token;
	dst->slot_id = src->slot_id;
	dst->nonce = src->nonce;
	dst->payload_len = src->payload_len;
	dst->reserved = src->reserved;
}

static inline __u64 chronos_layout_hash(void)
{
	__u64 ts = ktime_get_ns();
	return siphash_3u64(chronos_secret_seed ^ ts, ts, 0ULL);
}

static int chronos_do_layout_negotiation(struct chronos_abi_layout *layout,
					     struct chronos_abi_user_view *view)
{
	int retry;
	long timeout;

	for (retry = 0; retry < 3; retry++) {
		layout->flags = CHRONOS_ABI_VERSION;
		layout->slot_id = (u32)(chronos_layout_hash() & 0xffffffff);
		layout->payload_len = sizeof(*layout);
		layout->reserved = 0;
		layout->token = chronos_layout_hash();
		layout->nonce = chronos_layout_hash();

		pack_for_userspace(layout, view);

		timeout = wait_event_interruptible_timeout(
			chronos_waitq,
			signal_pending(current),
			msecs_to_jiffies(500));

		if (signal_pending(current))
			continue;

		if (timeout > 0)
			return 0;
	}

	return -ETIMEDOUT;
}

static int __init chronos_abi_init(void)
{
	get_random_bytes(&chronos_secret_seed, sizeof(chronos_secret_seed));
	return 0;
}

static void __exit chronos_abi_exit(void)
{
}

module_init(chronos_abi_init);
module_exit(chronos_abi_exit);
