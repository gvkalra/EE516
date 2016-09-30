MODNAME := procmon
obj-m := ${MODNAME}.o
${MODNAME}-objs := main.o sequence.o sorting.o manager.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean