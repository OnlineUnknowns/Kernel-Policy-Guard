/* KernelPolicyGuard - policy storage definitions */
#ifndef _CHRONOS_POLICY_STORE_H
#define _CHRONOS_POLICY_STORE_H

#include <linux/atomic.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include <linux/types.h>

struct chronos_policy {
	u32 hash_seed;
	u64 net_blocklist_mask;
	u32 uid_whitelist[];
};

struct chronos_policy_store {
	struct chronos_policy __rcu *policy;
	atomic64_t version;
	spinlock_t lock;
};

extern struct chronos_policy_store chronos_policy_store;

int chronos_policy_update(struct chronos_policy *new_policy);
const struct chronos_policy *chronos_policy_lookup(void);
void chronos_policy_release(void);

#endif
