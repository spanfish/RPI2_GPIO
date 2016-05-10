#define DRIVER_NAME "RPI"
#define PDEBUG(fmt,args...) printk(KERN_DEBUG"%s:"fmt,DRIVER_NAME, ##args)
#define PERR(fmt,args...) printk(KERN_ERR"%s:"fmt,DRIVER_NAME,##args)
#define PINFO(fmt,args...) printk(KERN_INFO"%s:"fmt,DRIVER_NAME, ##args)
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <linux/list.h>
#include <linux/irq.h>
#include <linux/slab.h>
//#include <linux/gpio.h>
#include <linux/platform_device.h>
//#include <mach/platform.h>
//#include <mach/platform.h>
//#include <mach/gpio.h>
#include <linux/time.h>
#include <linux/mman.h>

//Peripheral physical address
//see page No6 BCM2835Datasheet.pdf
#define PI2_PERI_BASE 0x3F000000
//GPIO base
//see page No90 BCM2835Datasheet.pdf
#define PI2_GPIO_BASE (PI2_PERI_BASE + 0x200000)
#define FUNC_CODE_INPUT  0x000
#define FUNC_CODE_OUTPUT 0x001
#define FUNC_CODE_ALT0   0x100
#define FUNC_CODE_ALT1   0x101
#define FUNC_CODE_ALT2   0x110
#define FUNC_CODE_ALT3   0x111
#define FUNC_CODE_ALT4   0x011
#define FUNC_CODE_ALT5   0x010
#define SET_INPUT(B,P) (*((B)->GPFSEL+((P)/10))=(*((B)->GPFSEL+((P)/10)) & ~(0x7<<((P)%10*3))) | (FUNC_CODE_INPUT<<((P)%10*3)))
#define SET_OUTPUT(B,P) (*((B)->GPFSEL+((P)/10))=(*((B)->GPFSEL+((P)/10)) & ~(0x7<<((P)%10*3))) | (FUNC_CODE_OUTPUT<<((P)%10*3)))
#define SET_HIGH(B,P) \
	do {\
		if((P) < 32) {\
			*((B)->GPSET)=(*((B)->GPSET) | (0x1<<((P))));\
		} else {\
			*((B)->GPSET+1)=(*((B)->GPSET+1) | (0x1<<((P)-32)));\
		}\
	} while(0)
#define SET_LOW(B,P) \
	do {\
		if((P) < 32) {\
			*((B)->GPCLR)=(*((B)->GPCLR) | (0x1<<((P))));\
		} else {\
			*((B)->GPCLR+1)=(*((B)->GPCLR+1) | (0x1<<((P)-32)));\
		}\
	} while(0)

typedef struct pi2_gpio {
	u_int32_t GPFSEL[6];//GPIO Function Select
	u_int32_t reserved0;
	u_int32_t GPSET[2];//GPIO Pin Output Set
	u_int32_t reserved1;
	u_int32_t GPCLR[2];//GPIO Pin Output Clear
	u_int32_t reserved2;
	u_int32_t GPLEV[2];//GPIO Pin Level
	u_int32_t reserved3;
	u_int32_t GPEDS[2];//GPIO Pin Event Detect Status
	u_int32_t reserved4;
	u_int32_t GPREN[2];//GPIO Pin Rising Edge Detect Enable
	u_int32_t reserved5;
	u_int32_t GPFEN[2];//GPIO Pin Falling Edge Detect Enable
	u_int32_t reserved6;
	u_int32_t GPHEN[2];//GPIO Pin High Detect Enable
	u_int32_t reserved7;
	u_int32_t GPLEN[2];//GPIO Pin Low Detect Enable
	u_int32_t reserved8;
	u_int32_t GPAREN[2];//GPIO Pin Async. Rising Edge Detect
	u_int32_t reserved9;
	u_int32_t GPAFEN[2];//GPIO Pin Async. Falling Edge Detect
	u_int32_t reserved10;
	u_int32_t GPPUD;	  //GPIO Pin Pull-up/down Enable
	u_int32_t GPPUDCLK[2];//GPIO Pin Pull-up/down Enable Clock
} pi2_gpio;

#define GPIOFSEL(x)  (0x00+(x)*4)

#define INP_GPIO(g) *(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
