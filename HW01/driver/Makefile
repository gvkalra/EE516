MODNAME := stack_device
obj-m := ${MODNAME}.o
${MODNAME}-objs := main.o stack.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean