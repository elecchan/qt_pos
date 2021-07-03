/*
复位按键驱动
 */
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/timer.h> 
//The Kenel header file, include soc virtual address
#include <asm/mach-ralink/rt_mmap.h>


#define RALINK_SYSCTL_ADDR       0xb0000000    // system control
#define RALINK_REG_GPIOMODE      (RALINK_SYSCTL_ADDR + 0x60) //GPIO MODE

#define RALINK_PRGIO_ADDR        0xb0000600 // Programmable I/O

#define RALINK_REG_PIO2300DATA   (RALINK_PRGIO_ADDR + 0x20)//数据地址
#define RALINK_REG_PIO2300DIR    (RALINK_PRGIO_ADDR + 0x24)//方向地址

#define RALINK_REG_PIO3924DATA   (RALINK_PRGIO_ADDR + 0x48)//数据地址
#define RALINK_REG_PIO3924DIR    (RALINK_PRGIO_ADDR + 0x4C)//方向地址

#define RALINK_REG_PIO7140DATA   (RALINK_PRGIO_ADDR + 0x70)//数据地址
#define RALINK_REG_PIO7140DIR    (RALINK_PRGIO_ADDR + 0x74)//方向地址

static u32 ralink_gpio2300_data = 0;
static u32 ralink_gpio3924_data = 0;
static u32 ralink_gpio7140_data = 0;

#define DRIVER_NAME "gpio_key"
//按键GPIO号,低电平按下
#define RST_KEY    20

//设置gpio模式为
static void set_gpio_mode(void)
{
    u32 gpiomode;
	  //PA_G_GPIO_MODE
    gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
    gpiomode |= (0x1<<20);
    *(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);
}

//设置gpio的数据方向
static void set_gpio_dir(int gpio,int dir)
{
    u32 gpiomode;
	if((gpio>=0) && (gpio<=23)) {
   	 	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300DIR));
   	 	gpiomode &= ~(0x01<<gpio);
   	 	gpiomode |= (dir?0x01:0x0)<<gpio;
		*(volatile u32 *)(RALINK_REG_PIO2300DIR) = cpu_to_le32(gpiomode);
	}else if((gpio>=24) && (gpio<=39)) {
   	 	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
   	 	gpiomode &= ~(0x01<<(gpio-24));
   	 	gpiomode |= (dir?0x01:0x0)<<(gpio-24);
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(gpiomode);
	}else if((gpio>=40) && (gpio<=71)) {
   	 	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
   	 	gpiomode &= ~(0x01<<(gpio-40));
   	 	gpiomode |= (dir?0x01:0x0)<<(gpio-40);
		*(volatile u32 *)(RALINK_REG_PIO7140DIR) = cpu_to_le32(gpiomode);
	}   
}

//扫描定义的按键是否被按下
static int scan_gpio_num(int gpio)
{
	//printk("%s\n",__func__);
  if((gpio >= 0) && (gpio <= 23)) {
		return ((ralink_gpio2300_data&(1<<gpio))?1:0);
	}else if((gpio >= 24) && (gpio <= 39)) {
		return ((ralink_gpio3924_data&(1<<(gpio-24)))?1:0);
	}else if((gpio >= 40) && (gpio <= 71)) {
		return ((ralink_gpio7140_data&(1<<(gpio-40)))?1:0);
	}
    return 1;
}


//打开设备
static int key_driver_open(struct inode *inode, struct file *file)
{
    set_gpio_mode();            
    set_gpio_dir(RST_KEY,0);
    return 0;
}
//关闭设备
static int key_driver_close(struct inode *inode, struct file *file)
{
    return 0;
}
static int key_driver_read(struct  file  *filp,  char __user *buf, size_t count,  loff_t *offp)
{
    int key_value;
    //获取数据寄存器数据
    ralink_gpio2300_data = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300DATA));
    key_value = scan_gpio_num(RST_KEY);
    copy_to_user(buf,&key_value,sizeof(key_value));
    //printk("%s %d\n",__func__,key_value);
    return 0;
}

static struct file_operations key_fops =
{
    .owner    = THIS_MODULE,
    .open     = key_driver_open,
    .release  = key_driver_close,
    .read     = key_driver_read,
};

static struct miscdevice key_misc =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DRIVER_NAME,
    .fops  = &key_fops,
};


//初始化
static int __init key_driver_init(void)
{
    misc_register(&key_misc);//初始化设备
    printk("driver_init OK!\n");
	set_gpio_mode();            
    set_gpio_dir(RST_KEY,0);    

    return 0;
}

//退出
static void __exit key_driver_exit(void)
{
    int ret;
    ret = misc_deregister(&key_misc);//注销设置
    if(ret < 0)
        printk("key_driver_exit error.\n");
    printk("key_driver_exit\n");
}

module_init(key_driver_init);
module_exit(key_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rong");
