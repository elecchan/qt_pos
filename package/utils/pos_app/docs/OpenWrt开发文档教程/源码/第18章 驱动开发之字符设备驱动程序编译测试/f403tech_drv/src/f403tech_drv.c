/*
** ����һЩͷ�ļ�����������д��������������ġ�
** ��: ������д������ʱ��Ӧ�ð�����Щͷ�ļ���?
** ��: ��д���������ʱ�����ǲ����ÿ����ȥ����Ҫ������Щͷ�ļ���
**     ����ֻ��Ҫ�ο����������������ͷ�ļ����ɣ�˵ͨ��һ�㣬����
**     �Ȱ��ں����������������ͷ�ļ��ȸ��ƹ������ţ�Ȼ���ڱ����
**     ʱ���ٸ�����ʾ��Ϣ����ӻ����޸�ͷ�ļ���
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

#define DEVICE_NAME     "f403tech"  /* ����ģʽ��ִ�С�cat /proc/devices����������豸���� */
#define F403TECH_MAJOR       0       /* ���豸�� */

static struct class *f403tech_drv_class;

static int f403tech_drv_open(struct inode *inode, struct file *file)
{
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk������������Ӵ�ӡ���÷���Ӧ�ó����е�printfһ��

	return 0;
}

static ssize_t f403tech_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk������������Ӵ�ӡ���÷���Ӧ�ó����е�printfһ��

	return 0;
}

static ssize_t f403tech_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk������������Ӵ�ӡ���÷���Ӧ�ó����е�printfһ��

	return 0;
}

/* ����ṹ���ַ��豸��������ĺ���
** ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
** ���ջ��������ṹ��ָ���Ķ�Ӧ����
**/
static struct file_operations f403tech_drv_fops = {
	.owner  	= THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
	.open   	= f403tech_drv_open,     
	.read	= f403tech_drv_read,	   
	.write	= f403tech_drv_write,	   
};

int major;
/*
** ִ��insmod����ʱ�ͻ����������� 
*/
static int __init f403tech_drv_init(void)
{
	/* ע���ַ��豸
	** �ⲽ��д�ַ��豸���������������
	** ����Ϊ���豸�š��豸���֡�file_operations�ṹ��
	** ���������豸�žͺ;����file_operations�ṹ��ϵ�����ˣ�
	** �������豸ΪF403TECH_MAJOR���豸�ļ�ʱ���ͻ����f403tech_drv_fops�е���س�Ա����
	** F403TECH_MAJOR������Ϊ0����ʾ���ں��Զ��������豸��
	*/
	major = register_chrdev(F403TECH_MAJOR, DEVICE_NAME, &f403tech_drv_fops);
	if (major < 0)
	{
		printk(DEVICE_NAME " can't register major number\n");
		return major;
	}

	/*
	** �������д������ڴ����豸�ڵ㣬�Ǳ���ġ�
	** ��Ȼ�������д�����У���ô�ͱ�����linuxϵͳ��������ͨ��mknod��������������豸�ڵ�
	*/
	/* ������ */
	f403tech_drv_class = class_create(THIS_MODULE, "f403tech");
	/* �����洴���豸�ڵ� */
	device_create(f403tech_drv_class, NULL, MKDEV(major, 0), NULL, "f403tech");		// /dev/f403tech

	/*
	** ��ӡһ��������Ϣ
	*/
	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk������������Ӵ�ӡ���÷���Ӧ�ó����е�printfһ��

	return 0;
}

/*
 * ִ��rmmod����ʱ�ͻ����������� 
 */
static void __exit f403tech_drv_exit(void)
{
	unregister_chrdev(major, "f403tech");		// ����ں�����register_chrdev�������ʹ��
	device_destroy(f403tech_drv_class, MKDEV(major, 0));	// ����ں�����device_create�������ʹ��
	class_destroy(f403tech_drv_class);	// ����ں�����class_create�������ʹ��

	printk("%s:Hello F403Tech\n", __FUNCTION__);	// printk������������Ӵ�ӡ���÷���Ӧ�ó����е�printfһ��
}

/* ������ָ����������ĳ�ʼ��������ж�غ��� */
module_init(f403tech_drv_init);
module_exit(f403tech_drv_exit);

/* �������������һЩ��Ϣ�����Ǳ���� */
MODULE_AUTHOR("http://www.f403tech.com");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("RT5350 FIRST Driver");
MODULE_LICENSE("GPL");

