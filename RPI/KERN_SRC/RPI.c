/*
 ===============================================================================
 Driver Name		:		RPI
 Author			:		WANG XIANGWEI
 License			:		GPL
 Description		:		LINUX DEVICE DRIVER PROJECT
 ===============================================================================
 */

#include"RPI.h"

#define RPI_N_MINORS 1
#define RPI_FIRST_MINOR 0
#define RPI_BUFF_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WANG XIANGWEI");
MODULE_DESCRIPTION("Raspberry PI2 GPIO Device Driver module");

int RPI_major = 0;

dev_t RPI_device_num;

typedef struct privatedata {
	int nMinor;
	char buff[RPI_BUFF_SIZE];
	volatile pi2_gpio *gpio;
	struct cdev cdev;
//spinlock_t my_lock = SPIN_LOCK_UNLOCKED;
} RPI_private;

static RPI_private devices[RPI_N_MINORS];
static pi2_gpio *gpio_base;

static int RPI_open(struct inode *inode, struct file *filp) {
	RPI_private *priv = container_of(inode->i_cdev ,
			RPI_private ,cdev);
	filp->private_data = priv;

	PINFO("In char driver open() function\n");

	return 0;
}

static int RPI_release(struct inode *inode, struct file *filp) {
	RPI_private *priv;

	priv = filp->private_data;

	PINFO("In char driver release() function\n");

	return 0;
}

static ssize_t RPI_read(struct file *filp, char __user *ubuff, size_t count,
		loff_t *offp) {
	int n = 0;
	RPI_private *priv;

	priv = filp->private_data;

	PINFO("In char driver read() function\n");

	//PINFO("io address is:%x\n", *(gpio.base));

	return n;
}

static ssize_t RPI_write(struct file *filp, const char __user *ubuff,
		size_t count, loff_t *offp) {
	int n = 0;
	char *wbuff;
	char *rbuff;
	unsigned int pin;
	RPI_private *priv;
	priv = filp->private_data;
	PINFO("In char driver write() function\n");

	wbuff = priv->buff;
	rbuff = (char *)ubuff;
	while(n < RPI_BUFF_SIZE && n < count) {
		if(get_user(*wbuff++, ubuff++)) {
			n = 0;
			PERR("Error get pin number from user space\n");
			goto error;
		}
		n++;
	}

	if(kstrtouint(priv->buff, 10, &pin)) {
		PERR("Wrong pin number\n");
		//err = -EINVAL;
		goto error;
	}
	SET_OUTPUT(priv->gpio, pin);
	SET_HIGH(priv->gpio, pin);
	mdelay(500);
	SET_LOW(priv->gpio, pin);
	mdelay(500);
	SET_HIGH(priv->gpio, pin);
	mdelay(500);
	SET_LOW(priv->gpio, pin);
	mdelay(500);
	SET_HIGH(priv->gpio, pin);
	mdelay(500);
	SET_LOW(priv->gpio, pin);
	return n;
error:
	return 0;
}

static const struct file_operations RPI_fops =
		{ .owner = THIS_MODULE, .open = RPI_open, .release = RPI_release,
				.read = RPI_read, .write = RPI_write, };

static void __exit RPI_exit(void);

static int __init RPI_init(void) {
	int i = 0;
	int res = 0;

	PINFO("RPI_init Start\n");
	res = alloc_chrdev_region(&RPI_device_num, RPI_FIRST_MINOR, RPI_N_MINORS,
			DRIVER_NAME);
	if (res) {
		PERR("register device no failed\n");
		return -1;
	}
	RPI_major = MAJOR(RPI_device_num);

	gpio_base = ioremap(PI2_GPIO_BASE, sizeof(pi2_gpio));//__io_address(GPIO_BASE2);
	if (gpio_base == NULL) {
		PERR("Failed to call ioremap\n");
		goto error;
	}
	for (i = 0; i < RPI_N_MINORS; i++) {
		RPI_device_num = MKDEV(RPI_major ,RPI_FIRST_MINOR+i);
		cdev_init(&devices[i].cdev, &RPI_fops);
		cdev_add(&devices[i].cdev, RPI_device_num, 1);
		devices[i].gpio = gpio_base;

		devices[i].nMinor = RPI_FIRST_MINOR + i;
	}
	PINFO("RPI_init End\n");

	return 0;
	error: RPI_exit();
	return -1;
}

static void __exit RPI_exit(void) {
	int i;

	PINFO("RPI_exit Start\n");
	if (gpio_base) {
		iounmap(gpio_base);
	}

	for (i = 0; i < RPI_N_MINORS; i++) {
		RPI_device_num = MKDEV(RPI_major ,RPI_FIRST_MINOR+i);
		cdev_del(&devices[i].cdev);
	}

	unregister_chrdev_region(RPI_device_num, RPI_N_MINORS);
	PINFO("RPI_exit End\n");
}

module_init(RPI_init)
;
module_exit(RPI_exit)
;

