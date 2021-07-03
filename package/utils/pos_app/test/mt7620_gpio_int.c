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
#include <asm/mach-ralink/surfboardint.h>
//The Kenel header file, include soc virtual address
#include <asm/mach-ralink/rt_mmap.h>


#define RALINK_SYSCTL_ADDR       0xb0000000    // system control
#define RALINK_REG_GPIOMODE      (RALINK_SYSCTL_ADDR + 0x60) //GPIO MODE

#define RALINK_PRGIO_ADDR        0xb0000600 // Programmable I/O

#define RALINK_REG_PIO2300INT    (RALINK_PRGIO_ADDR + 0x00)//中断地址
#define RALINK_REG_PIO2300EDGE   (RALINK_PRGIO_ADDR + 0x04)//边沿触发方式地址
#define RALINK_REG_PIO2300RENA   (RALINK_PRGIO_ADDR + 0x08)//上升沿触发掩码
#define RALINK_REG_PIO2300FENA   (RALINK_PRGIO_ADDR + 0x0C)
#define RALINK_REG_PIO2300DATA   (RALINK_PRGIO_ADDR + 0x20)//数据地址
#define RALINK_REG_PIO2300DIR    (RALINK_PRGIO_ADDR + 0x24)//方向地址

#define RALINK_REG_PIO3924INT    (RALINK_PRGIO_ADDR + 0x38)//中断地址
#define RALINK_REG_PIO3924EDGE   (RALINK_PRGIO_ADDR + 0x3C)//边沿触发方式地址
#define RALINK_REG_PIO3924RENA   (RALINK_PRGIO_ADDR + 0x40)//上升沿触发掩码
#define RALINK_REG_PIO3924FENA   (RALINK_PRGIO_ADDR + 0x44)
#define RALINK_REG_PIO3924DATA   (RALINK_PRGIO_ADDR + 0x48)//数据地址
#define RALINK_REG_PIO3924DIR    (RALINK_PRGIO_ADDR + 0x4C)//方向地址

#define RALINK_REG_PIO7140INT    (RALINK_PRGIO_ADDR + 0x60)//中断地址
#define RALINK_REG_PIO7140EDGE   (RALINK_PRGIO_ADDR + 0x64)//边沿触发方式地址
#define RALINK_REG_PIO7140RENA   (RALINK_PRGIO_ADDR + 0x68)//上升沿触发掩码
#define RALINK_REG_PIO7140FENA   (RALINK_PRGIO_ADDR + 0x6C)
#define RALINK_REG_PIO7140DATA   (RALINK_PRGIO_ADDR + 0x70)//数据地址
#define RALINK_REG_PIO7140DIR    (RALINK_PRGIO_ADDR + 0x74)//方向地址

#define  RALINK_IRQ_ADDR         0xb0000200  
#define  RALINK_REG_INTENA       (RALINK_IRQ_ADDR   + 0x34)//enable 中断地址
#define  RALINK_REG_INTDIS       (RALINK_IRQ_ADDR   + 0x38)//disable 中断地址

#define DRIVER_NAME "gpio_int"
//#define UP_KEY    1
//#define DOWN_KEY  2
#define UP_KEY    24
#define DOWN_KEY  25

#define CMD_FLAG  'a'
#define USERSPACE_RD		1
#define USERSPACE_WR		2

//保存当前中断方式
static u32 ralink_gpio2300_intp = 0;
static u32 ralink_gpio3924_intp = 0;
static u32 ralink_gpio7140_intp = 0;
//保存当前边沿触发方式
static u32 ralink_gpio2300_edge = 0;
static u32 ralink_gpio3924_edge = 0;
static u32 ralink_gpio7140_edge = 0;
//用来判断两个中断先后
static char key_press = 0;
//gpio对应信息
struct gpio_status
{
    int gpio_num;
    u32 key_value;//键值传给用户空间
};

static struct gpio_status g_gpio[2] = {
	{UP_KEY,     0},
	{DOWN_KEY,  0},
};

typedef struct _userspace_data {
    int set_count;
	int get_count;
	unsigned char status;
} userspace_data;

/* 中断事件标志, 中断服务程序将它置1，key_driver_read将它清0*/
static volatile int ev_press = 0;
static volatile int int_count = 0;
/**
* 设置gpio模式为
*/
static void set_gpio_mode(void)
{
    u32 gpiomode;
	//I2C as gpio mode gpio1-gpio2
    gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
    gpiomode |= (0x1<<0);
	//RGMII1 as gpio mode
	gpiomode |= (0x1<<9);
	//RGMII2 as gpio mode
	gpiomode |= (0x1<<10);

    *(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);
}
/**
* 设置gpio的数据方向
*/
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
//设置pio enable interrupt
static void enable_intp(void)
{   
    //在rt_mmap.h头文件中定义RALINK_INTCTL_PIO ，即第六位控制pio中断
    //#define RALINK_INTCTL_PIO       (1<<6)
    *(volatile u32 *)(RALINK_REG_INTENA) = cpu_to_le32(RALINK_INTCTL_PIO);
}
//设置pio disable interrupt
static void disable_intp(void)
{
    //在rt_mmap.h头文件中定义RALINK_INTCTL_PIO ，即第六位控制pio中断
    //#define RALINK_INTCTL_PIO       (1<<6)
    *(volatile u32 *)(RALINK_REG_INTDIS) = cpu_to_le32(RALINK_INTCTL_PIO);
}
//set Edge Interrupt
static void gpio_reg_irq(int gpio,int edge)
{
    unsigned long tmp;
   	if(edge == 1) {
		if((gpio>=0) && (gpio<=23)) {
   	 		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300RENA));
    		tmp |= (0x1 << gpio);
    		*(volatile u32 *)(RALINK_REG_PIO2300RENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300FENA));
    		tmp &= ~(0x1 << gpio);
    		*(volatile u32 *)(RALINK_REG_PIO2300FENA) = cpu_to_le32(tmp);
		}else if((gpio>=24) && (gpio<=39)) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924RENA));
    		tmp |= (0x1 << (gpio-24));
    		*(volatile u32 *)(RALINK_REG_PIO3924RENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924FENA));
    		tmp &= ~(0x1 << (gpio-24));
    		*(volatile u32 *)(RALINK_REG_PIO3924FENA) = cpu_to_le32(tmp);
		}else if((gpio>=40) && (gpio<=71)) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140RENA));
    		tmp |= (0x1 << (gpio-40));
    		*(volatile u32 *)(RALINK_REG_PIO7140RENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140FENA));
    		tmp &= ~(0x1 << (gpio-40));
    		*(volatile u32 *)(RALINK_REG_PIO7140FENA) = cpu_to_le32(tmp);
		}
    }else if(edge == 0){
		if((gpio>=0) && (gpio<=23)) {
   	 		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300FENA));
    		tmp |= (0x1 << gpio);
    		*(volatile u32 *)(RALINK_REG_PIO2300FENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300RENA));
    		tmp &= ~(0x1 << gpio);
    		*(volatile u32 *)(RALINK_REG_PIO2300RENA) = cpu_to_le32(tmp);
		}else if((gpio>=24) && (gpio<=39)) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924FENA));
    		tmp |= (0x1 << (gpio-24));
    		*(volatile u32 *)(RALINK_REG_PIO3924FENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924RENA));
    		tmp &= ~(0x1 << (gpio-24));
    		*(volatile u32 *)(RALINK_REG_PIO3924RENA) = cpu_to_le32(tmp);
		}else if((gpio>=40) && (gpio<=71)) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140FENA));
    		tmp |= (0x1 << (gpio-40));
    		*(volatile u32 *)(RALINK_REG_PIO7140FENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140RENA));
    		tmp &= ~(0x1 << (gpio-40));
    		*(volatile u32 *)(RALINK_REG_PIO7140RENA) = cpu_to_le32(tmp);
		}
	}
}
//先保存当前中断及触发寄存器的值,再清空
static void ralink_gpio_save_clear_intp(void)
{
    //保存当前中断寄存器数据
	ralink_gpio2300_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300INT));
	ralink_gpio3924_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924INT));
    ralink_gpio7140_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140INT));
    //保存当前边沿触发方式
    ralink_gpio2300_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2300EDGE));
	ralink_gpio3924_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924EDGE));
	ralink_gpio7140_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140EDGE));
    *(volatile u32 *)(RALINK_REG_PIO2300INT) = cpu_to_le32(0xFFFFFFFF);
    *(volatile u32 *)(RALINK_REG_PIO2300EDGE) = cpu_to_le32(0xFFFFFFFF);
	*(volatile u32 *)(RALINK_REG_PIO3924INT) = cpu_to_le32(0xFFFFFFFF);
    *(volatile u32 *)(RALINK_REG_PIO3924EDGE) = cpu_to_le32(0xFFFFFFFF);
	*(volatile u32 *)(RALINK_REG_PIO7140INT) = cpu_to_le32(0xFFFFFFFF);
    *(volatile u32 *)(RALINK_REG_PIO7140EDGE) = cpu_to_le32(0xFFFFFFFF);
}

//扫描定义的按键是否被按下
static int scan_gpio_num(void)
{
	printk("%s\n",__func__);
    if((UP_KEY >= 0) && (UP_KEY <= 23)) {
		g_gpio[0].key_value = ((ralink_gpio2300_intp&(1<<UP_KEY))?1:0);
	}else if((UP_KEY >= 24) && (UP_KEY <= 39)) {
		g_gpio[0].key_value = ((ralink_gpio3924_intp&(1<<(UP_KEY-24)))?1:0);
	}else if((UP_KEY >= 40) && (UP_KEY <= 71)) {
		g_gpio[0].key_value = ((ralink_gpio7140_intp&(1<<(UP_KEY-40)))?1:0);
	}
	if((DOWN_KEY >= 0) && (DOWN_KEY <= 23)) {
		g_gpio[1].key_value = ((ralink_gpio2300_intp&(1<<DOWN_KEY))?1:0);
	}else if((DOWN_KEY >= 24) && (DOWN_KEY <= 39)) {
		g_gpio[1].key_value = ((ralink_gpio3924_intp&(1<<(DOWN_KEY-24)))?1:0);
	}else if((DOWN_KEY >= 40) && (DOWN_KEY <= 71)) {
		g_gpio[1].key_value = ((ralink_gpio7140_intp&(1<<(DOWN_KEY-40)))?1:0);
	}
    return 0;
}


//打开设备
static int key_driver_open(struct inode *inode, struct file *file)
{
    return 0;
}
//关闭设备
static int key_driver_close(struct inode *inode, struct file *file)
{
    return 0;
}
//
userspace_data user_data = {
	.get_count = 0,
	.set_count = 0,
};
static long key_driver_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
	//printk("key_driver_ioctl cmd:%d\n",cmd);
    switch(cmd)
    {
		case USERSPACE_RD:
			//printk("key_driver_ioctl rd int_count:%d\n",int_count);
            user_data.get_count = int_count;
			if (copy_to_user((void __user *)arg,(void*)&user_data, sizeof(userspace_data)))
                return -EFAULT;
			break;
		case USERSPACE_WR:
            if (copy_from_user((void*)&user_data,(void __user *)arg, sizeof(userspace_data)))
                return -EFAULT;
			int_count = user_data.set_count;
			//printk("key_driver_ioctl wr int_count:%d\n",int_count);
			break;
		default:
			break;
    }
    return 0;
}

static struct file_operations key_fops =
{
    .owner        = THIS_MODULE,
    .open        = key_driver_open,
    .release    = key_driver_close,
    .unlocked_ioctl = key_driver_ioctl,
};

static struct miscdevice key_misc =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = DRIVER_NAME,
    .fops = &key_fops,
};

//中断处理函数
/*关于中断处理函数的返回值:中断程序的返回值是一个特殊类型—irqreturn_t。
中断程序的返回值却只有两个: IRQ_NONE和IRQ_HANDLED。*/
static irqreturn_t ralink_key_interrupt(int irq, void *irqaction)
{
    int ret;

    printk("interrupt handler...\n");
    //先保存当前中断及触发寄存器的值,再清空
    ralink_gpio_save_clear_intp();
    //查看是否按键被按下
    scan_gpio_num();
	if(g_gpio[0].key_value == 1) {
		//如果当前是第一次中断,key_press为1
		if(key_press == 0)
			key_press = 1;
		else {//如果不是第一次中断,判断为下降状态
			key_press = 0;
			int_count--;
			if(int_count == 0)
				int_count = -1;
			printk("key down press,int_count=%d\n",int_count);
			user_data.status = 2;//status为2表下降
		}		
		g_gpio[0].key_value == 0;
	}
	if(g_gpio[1].key_value == 1) {
		//如果当前是第一次中断,key_press为1
		if(key_press == 0)
			key_press = 1;
		else {//如果不是第一次中断,判断为上升状态
			key_press = 0;
			int_count++;
			if(int_count == 0)
				int_count = 1;
			printk("key up press,int_count=%d\n",int_count);
			user_data.status = 1;//status为1表上升
		}
		g_gpio[1].key_value == 0;		
	}
    return IRQ_RETVAL(IRQ_HANDLED);
}
//初始化
static int __init key_driver_init(void)
{
    int ret;
    /*注册中断请求
     中断号:SURFBOARDINT_GPIO
     中断处理函数:ralink_key_interrupt
     中断属性(方式):上升沿触发
     使用此中断的设备:gpio_key
    */
    ret = request_irq(SURFBOARDINT_GPIO, ralink_key_interrupt, IRQ_TYPE_EDGE_RISING,DRIVER_NAME, NULL);
    if (ret!=0)
        return ret;

    ret = misc_register(&key_misc);//初始化设备
    printk("driver_init OK!\n");
	//初始化IO以及中断
	set_gpio_mode();            //set RGMII2_GPIO_MODE to gpio mode.pro.p38
    set_gpio_dir(UP_KEY,0);    //set gpio60-gpio63 to gpin.
	set_gpio_dir(DOWN_KEY,0); 
    enable_intp();        //set pio enable interrupt
    gpio_reg_irq(UP_KEY,1);     //set Edge Interrupt
	gpio_reg_irq(DOWN_KEY,1);
	
    return ret;
}

//退出
static void __exit key_driver_exit(void)
{
    int ret;

    free_irq(SURFBOARDINT_GPIO,NULL);//注销中断
    ret = misc_deregister(&key_misc);//注销设置
    if(ret < 0)
        printk("key_driver_exit error.\n");
    printk("driver_exit\n");
	disable_intp();
}

module_init(key_driver_init);
module_exit(key_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rong");
