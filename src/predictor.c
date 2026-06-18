/* KernelPolicyGuard - syscall predictor logic */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/tracepoint.h>
#include <linux/ptrace.h>
#include <linux/types.h>

#define SEC(NAME) __attribute__((section(NAME), used))

struct bpf_map_def {
	unsigned int type;
	unsigned int key_size;
	unsigned int value_size;
	unsigned int max_entries;
	unsigned int map_flags;
};

struct chronos_syscall_ctx {
	__u64 seq_hash;
	__u64 ts;
	__u32 syscall;
};

SEC("tracepoint/syscalls/sys_enter")
int chronos_predictor(struct tracepoint_raw_sys_enter *args)
{
	__u64 hash = 0;
	__u64 seq = 0;
	__u32 syscall = (unsigned int)args->id;

	hash = ((hash << 5) + hash) + syscall;
	seq = hash ^ (unsigned long)current;

	if (seq & 0x1ULL) {
		bpf_send_signal_helper(SIGSTOP, current, 0);
	}

	return 0;
}

char _license[] SEC("license") = "GPL";
