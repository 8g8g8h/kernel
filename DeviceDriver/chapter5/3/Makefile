CFLAGS+= -Wall


obj-m+=devone2.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

cleam:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
