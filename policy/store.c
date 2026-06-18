/* KernelPolicyGuard - policy store implementation */
#include "store.h"

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

struct chronos_policy_store chronos_policy_store = {
	.policy = NULL,
	.version = ATOMIC64_INIT(0),
	.lock = __SPIN_LOCK_UNLOCKED(chronos_policy_store.lock),
};

int chronos_policy_update(struct chronos_policy *new_policy)
{
	struct chronos_policy *old_policy;
	unsigned long flags;

	if (!new_policy)
		return -EINVAL;

	spin_lock_irqsave(&chronos_policy_store.lock, flags);
	old_policy = rcu_dereference_protected(
		chronos_policy_store.policy,
		lockdep_is_held(&chronos_policy_store.lock));
	rcu_assign_pointer(chronos_policy_store.policy, new_policy);
	spin_unlock_irqrestore(&chronos_policy_store.lock, flags);

	atomic64_inc(&chronos_policy_store.version);

	if (old_policy)
		kfree_rcu(old_policy, rcu_head);

	return 0;
}

const struct chronos_policy *chronos_policy_lookup(void)
{
	const struct chronos_policy *policy;

	__rcu_read_lock();
	policy = rcu_dereference(chronos_policy_store.policy);
	__rcu_read_unlock();

	return policy;
}

void chronos_policy_release(void)
{
	struct chronos_policy *old_policy;
	unsigned long flags;

	spin_lock_irqsave(&chronos_policy_store.lock, flags);
	old_policy = rcu_dereference_protected(
		chronos_policy_store.policy,
		lockdep_is_held(&chronos_policy_store.lock));
	rcu_assign_pointer(chronos_policy_store.policy, NULL);
	spin_unlock_irqrestore(&chronos_policy_store.lock, flags);

	if (old_policy)
		kfree_rcu(old_policy, rcu_head);
}
