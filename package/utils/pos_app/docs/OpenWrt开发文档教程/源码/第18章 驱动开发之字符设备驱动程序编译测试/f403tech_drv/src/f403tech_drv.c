/*
** 包含一些头文件，这是我们写驱动程序所必须的。
** 问: 我们在写驱动的时候，应该包含哪些头文件呢?
** 答: 在写驱动程序的时候，我们并不用刻意的去记需要包含哪些头文件，
**     我们只需要参考其他的驱动程序的头文件即可，说通俗一点，就是
**     先把内核中其他驱动程序的头文件先复制过来用着，然后在编译的
**     时候，再更加提示信息来添加或者修改头文件。
*/
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

#define DEVICE_NAME     "f403tech"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */
#define F403TECH_MAJOR       0       /* 主设备号 */

static struct class *f403tech_drv_class;

static int f403tech_drv_open(struct inode *inode, struct file *file)
{
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk用于驱动中添加打印，用法和应用程序中的printf一样

	return 0;
}

static ssize_t f403tech_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk用于驱动中添加打印，用法和应用程序中的printf一样

	return 0;
}

static ssize_t f403tech_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk用于驱动中添加打印，用法和应用程序中的printf一样

	return 0;
}

/* 这个结构是字符设备驱动程序的核心
** 当应用程序操作设备文件时所调用的open、read、write等函数，
** 最终会调用这个结构中指定的对应函数
**/
static struct file_operations f403tech_drv_fops = {
	.owner  	= THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open   	= f403tech_drv_open,     
	.read	= f403tech_drv_read,	   
	.write	= f403tech_drv_write,	   
};

int major;
/*
** 执行insmod命令时就会调用这个函数 
*/
static int __init f403tech_drv_init(void)
{
	/* 注册字符设备
	** 这步是写字符设备驱动程序所必须的
	** 参数为主设备号、设备名字、file_operations结构；
	** 这样，主设备号就和具体的file_operations结构联系起来了，
	** 操作主设备为F403TECH_MAJOR的设备文件时，就会调用f403tech_drv_fops中的相关成员函数
	** F403TECH_MAJOR可以设为0，表示由内核自动分配主设备号
	*/
	major = register_chrdev(F403TECH_MAJOR, DEVICE_NAME, &f403tech_drv_fops);
	if (major < 0)
	{
		printk(DEVICE_NAME " can't register major number\n");
		return major;
	}

	/*
	** 以下两行代码用于创建设备节点，是必须的。
	** 当然，如果不写这两行，那么就必须在linux系统命令行中通过mknod这个命令来创建设备节点
	*/
	/* 创建类 */
	f403tech_drv_class = class_create(THIS_MODULE, "f403tech");
	/* 类下面创建设备节点 */
	device_create(f403tech_drv_class, NULL, MKDEV(major, 0), NULL, "f403tech");		// /dev/f403tech

	/*
	** 打印一个调试信息
	*/
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk用于驱动中添加打印，用法和应用程序中的printf一样

	return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void __exit f403tech_drv_exit(void)
{
	unregister_chrdev(major, "f403tech");		// 与入口函数的register_chrdev函数配对使用
	device_destroy(f403tech_drv_class, MKDEV(major, 0));	// 与入口函数的device_create函数配对使用
	class_destroy(f403tech_drv_class);	// 与入口函数的class_create函数配对使用

	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk用于驱动中添加打印，用法和应用程序中的printf一样
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(f403tech_drv_init);
module_exit(f403tech_drv_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("http://www.f403tech.com");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("RT5350 FIRST Driver");
MODULE_LICENSE("GPL");

