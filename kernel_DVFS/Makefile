obj-m += test.o
PWD := $(shell pwd)
KVERSION := $(shell uname -r)
KERNEL_DIR ?= /lib/modules/$(KVERSION)/build
ccflags-y := -I/usr/src/linux-headers-5.10.104-tegra-ubuntu20.04_aarch64/kernel-5.10/include/linux/

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
