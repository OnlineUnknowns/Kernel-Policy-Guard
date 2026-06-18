# KernelPolicyGuard kernel module build rules
obj-m += chronos.o
chronos-objs := chronos_main.o

EXTRA_CFLAGS += -std=gnu11 -Wall -Werror

all:
	$(MAKE) -C $(KERNEL_BUILD) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_BUILD) M=$(PWD) clean
