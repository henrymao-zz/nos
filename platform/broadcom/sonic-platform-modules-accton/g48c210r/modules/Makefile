ifneq ($(KERNELRELEASE),)
obj-m:= x86-64-accton-g48c210r-cpld.o x86-64-accton-g48c210r-psu.o  \
	    x86-64-accton-g48c210r-leds.o ym2651y.o
	    
else
ifeq (,$(KERNEL_SRC))
#$(error KERNEL_SRC is not defined)
KVERSION=3.16.0-8-amd64
KERNEL_DIR   = /usr/src/linux-headers-$(KVERSION)/
KERNELDIR:=$(KERNEL_DIR)
else
KERNELDIR:=$(KERNEL_SRC)
endif
PWD:=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD) modules
clean:
	rm -rf *.o *.mod.o *.mod.o *.mod.c *.ko .*cmd .tmp_versions Module.markers Module.symvers modules.order
endif
