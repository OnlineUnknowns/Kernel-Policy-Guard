/* KernelPolicyGuard - cluster synchronization logic */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/rcupdate.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <net/sock.h>

#include "../policy/store.h"

static struct socket *kernel_sock;
static struct task_struct *cluster_thread;
static struct sockaddr_in cluster_addr;

static int cluster_listener_thread(void *data)
{
	struct msghdr msg = {0};
	struct kvec iov;
	char buf[256];
	int ret;

	while (!kthread_should_stop()) {
		memset(buf, 0, sizeof(buf));
		iov.iov_base = buf;
		iov.iov_len = sizeof(buf);
		msg.msg_name = &cluster_addr;
		msg.msg_namelen = sizeof(cluster_addr);
		msg.msg_flags = MSG_DONTWAIT;

		ret = kernel_recvmsg(kernel_sock, &msg, &iov, 1,
				     sizeof(buf), msg.msg_flags);
		if (ret == -EAGAIN || ret == -EWOULDBLOCK)
			continue;
		if (ret < 0) {
			if (ret == -EINTR || ret == -ERESTARTSYS)
				continue;
			break;
		}
	}

	return 0;
}

static int __init cluster_sync_init(void)
{
	int ret;

	ret = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM,
				      IPPROTO_UDP, &kernel_sock);
	if (ret)
		return ret;

	cluster_thread = kthread_run(cluster_listener_thread, NULL,
				     "chronos_cluster");
	if (IS_ERR(cluster_thread)) {
		ret = PTR_ERR(cluster_thread);
		goto err_sock_release;
	}

	return 0;

err_sock_release:
	sock_release(kernel_sock);
	kernel_sock = NULL;
	return ret;
}

static void __exit cluster_sync_exit(void)
{
	if (cluster_thread) {
		kthread_stop(cluster_thread);
		cluster_thread = NULL;
	}

	if (kernel_sock) {
		sock_release(kernel_sock);
		kernel_sock = NULL;
	}

	chronos_policy_release();
}

module_init(cluster_sync_init);
module_exit(cluster_sync_exit);
