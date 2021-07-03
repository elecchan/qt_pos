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
	/* ��GPIO#40��GPIO#41��GPIO#42��GPIO#43����ߵ�ƽ��ͬʱϨ��LED0��LED1��LED2��LED3��LED4 */
	*GPIO71_40_DATA &= ~((1<<0)|(1<<1)|(1<<2)|(1<<3));
	
	return 0;
}

static long myleds_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case MYLEDS_LED0_ON:	// ����LED0
			*GPIO71_40_DATA &= ~(1<<0);
			break;
		case MYLEDS_LED0_OFF:	// Ϩ��LED0
			*GPIO71_40_DATA |= (1<<0);
			break;
		case MYLEDS_LED1_ON:	// ����LED1
			*GPIO71_40_DATA &= ~(1<<1);
			break;
		case MYLEDS_LED1_OFF:	// Ϩ��LED1
			*GPIO71_40_DATA |= (1<<1);
			break;
		case MYLEDS_LED2_ON:	// ����LED2
			*GPIO71_40_DATA &= ~(1<<2);
			break;
		case MYLEDS_LED2_OFF:	// Ϩ��LED2
			*GPIO71_40_DATA |= (1<<2);
			break;
		case MYLEDS_LED3_ON:	// ����LED3
			*GPIO71_40_DATA &= ~(1<<3);
			break;
		case MYLEDS_LED3_OFF:	// Ϩ��LED3
			*GPIO71_40_DATA |= (1<<3);
			break;
		default:
			break;
	}
	
	return 0;
}

/* 1.���䡢����һ��file_operations�ṹ�� */
static struct file_operations myleds_fops = {
	.owner   			= THIS_MODULE,    				/* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
	.open    			= myleds_open,
	.unlocked_ioctl	= myleds_unlocked_ioctl,
};

int major;
static int __init myleds_init(void)
{
	printk("%s:Hello Wooya\n", __FUNCTION__);	// printk������������Ӵ�ӡ���÷���Ӧ�ó����е�printfһ��

	/* 2.ע�� */
	major = register_chrdev(0, "myleds", &myleds_fops);

	/* 3.�Զ������豸�ڵ� */
	/* ������ */
	myleds_class = class_create(THIS_MODULE, "myleds");
	/* �����洴���豸�ڵ� */
	device_create(myleds_class, NULL, MKDEV(major, 0), NULL, "myleds");		// /dev/myleds

	/* 4.Ӳ����صĲ��� */
	/* ӳ��Ĵ����ĵ�ַ */
	GPIOMODE = (volatile unsigned long *)ioremap(0x10000060, 4);
	GPIO71_40_DIR = (volatile unsigned long *)ioremap(0x10000674, 4);
	GPIO71_40_DATA = (volatile unsigned long *)ioremap(0x10000670, 4);

	/* ������Ӧ�ܽ�����GPIO */
	/*
	** LED0 ---- GPIO#40
	** LED1 ---- GPIO#41
	** LED2 ---- GPIO#42
	** LED3 ---- GPIO#43
	*/
	*GPIOMODE |= (0x1<<15);

	/* ��GPIO#40��GPIO#41��GPIO#42��GPIO#43��GPIO#44����Ϊ��� */
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

