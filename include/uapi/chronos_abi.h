/* KernelPolicyGuard - ABI layout definitions */
#ifndef _UAPI_CHRONOS_ABI_H
#define _UAPI_CHRONOS_ABI_H

#include <linux/types.h>
#include <linux/uuid.h>

#define CHRONOS_ABI_MAGIC 0x43524F4E
#define CHRONOS_ABI_VERSION 1

struct chronos_abi_layout {
	__u32 flags;
	__u32 slot_id;
	__u32 payload_len;
	__u32 reserved;
	__u64 token;
	__u64 nonce;
};

struct chronos_abi_user_view {
	__u32 flags;
	__u64 token;
	__u32 slot_id;
	__u64 nonce;
	__u32 payload_len;
	__u32 reserved;
};

#endif
