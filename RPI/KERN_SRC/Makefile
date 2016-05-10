
obj-m		:= RPI.o 
#KERN_SRC	:= /lib/modules/$(shell uname -r)/build/
KERN_SRC	:= /home/spanfish/raspberrypi/linux-rpi-4.1.y_rebase
PWD			:= $(shell pwd)
ARC			:= arm
CROSS_COMPILE := /home/spanfish/raspberrypi/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
modules:
	make ARCH=$(ARC) CROSS_COMPILE=${CROSS_COMPILE} -C $(KERN_SRC) M=$(PWD) modules
	cp -f RPI.ko ~/public_html
	
install:
	make ARCH=$ARM CROSS_COMPILE=${CROSS_COMPILE} -C $(KERN_SRC) M=$(PWD) modules_install
	depmod -a
	
clean:
	make ARCH=$ARM CROSS_COMPILE=${CROSS_COMPILE} -C $(KERN_SRC) M=$(PWD) clean
