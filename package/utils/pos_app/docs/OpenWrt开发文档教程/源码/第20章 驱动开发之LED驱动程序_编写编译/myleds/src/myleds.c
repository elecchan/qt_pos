#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/uaccess.h>

#define MYLEDS_LED0_ON 	0
#define MYLEDS_LED0_OFF 	1
#define MYLEDS_LED1_ON 	2
#define MYLEDS_LED1_OFF 	3
#define MYLEDS_LED2_ON 	4
#define MYLEDS_LED2_OFF 	5
#define MYLEDS_LED3_ON 	6
#define MYLEDS_LED3_OFF 	7

volatile unsigned long *GPIOMODE;
volatile unsigned long *GPIO71_40_DIR;
volatile unsigned long *GPIO71_40_DATA;

static struct class *myleds_class;

static int myleds_open(struct inode *inode, struct file *file)
{
	/* 让GPIO#40、GPIO#41、GPIO#42、GPIO#43输出高电平，同时熄灭LED0、LED1、LED2、LED3、LED4 */
	*GPIO71_40_DATA &= ~((1<<0)|(1<<1)|(1<<2)|(1<<3));
	
	return 0;
}

static long myleds_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case MYLEDS_LED0_ON:	// 点亮LED0
			*GPIO71_40_DATA &= ~(1<<0);
			break;
		case MYLEDS_LED0_OFF:	// 熄灭LED0
			*GPIO71_40_DATA |= (1<<0);
			break;
		case MYLEDS_LED1_ON:	// 点亮LED1
			*GPIO71_40_DATA &= ~(1<<1);
			break;
		case MYLEDS_LED1_OFF:	// 熄灭LED1
			*GPIO71_40_DATA |= (1<<1);
			break;
		case MYLEDS_LED2_ON:	// 点亮LED2
			*GPIO71_40_DATA &= ~(1<<2);
			break;
		case MYLEDS_LED2_OFF:	// 熄灭LED2
			*GPIO71_40_DATA |= (1<<2);
			break;
		case MYLEDS_LED3_ON:	// 点亮LED3
			*GPIO71_40_DATA &= ~(1<<3);
			break;
		case MYLEDS_LED3_OFF:	// 熄灭LED3
			*GPIO71_40_DATA |= (1<<3);
			break;
		default:
			break;
	}
	
	return 0;
}

/* 1.分配、设置一个file_operations结构体 */
static struct file_operations myleds_fops = {
	.owner   			= THIS_MODULE,    				/* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    			= myleds_open,
	.unlocked_ioctl	= myleds_unlocked_ioctl,
};

int major;
static int __init myleds_init(void)
{
	printk("%s:Hello Wooya\n", __FUNCTION__);	// printk用于驱动中添加打印，用法和应用程序中的printf一样

	/* 2.注册 */
	major = register_chrdev(0, "myleds", &myleds_fops);

	/* 3.自动创建设备节点 */
	/* 创建类 */
	myleds_class = class_create(THIS_MODULE, "myleds");
	/* 类下面创建设备节点 */
	device_create(myleds_class, NULL, MKDEV(major, 0), NULL, "myleds");		// /dev/myleds

	/* 4.硬件相关的操作 */
	/* 映射寄存器的地址 */
	GPIOMODE = (volatile unsigned long *)ioremap(0x10000060, 4);
	GPIO71_40_DIR = (volatile unsigned long *)ioremap(0x10000674, 4);
	GPIO71_40_DATA = (volatile unsigned long *)ioremap(0x10000670, 4);

	/* 设置相应管脚用于GPIO */
	/*
	** LED0 ---- GPIO#40
	** LED1 ---- GPIO#41
	** LED2 ---- GPIO#42
	** LED3 ---- GPIO#43
	*/
	*GPIOMODE |= (0x1<<15);

	/* 将GPIO#40、GPIO#41、GPIO#42、GPIO#43、GPIO#44设置为输出 */
	*GPIO71_40_DIR = (1<<0)|(1<<1)|(1<<2)|(1<<3);

	return 0;
}

static void __exit myleds_exit(void)
{
	unregister_chrdev(major, "myleds");
	device_destroy(myleds_class, MKDEV(major, 0));
	class_destroy(myleds_class);
	iounmap(GPIOMODE);
	iounmap(GPIO71_40_DIR);
	iounmap(GPIO71_40_DATA);
}

module_init(myleds_init);
module_exit(myleds_exit);

MODULE_LICENSE("GPL");

