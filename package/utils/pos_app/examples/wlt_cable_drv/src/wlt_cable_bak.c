#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/i2c-gpio.h>
#include <linux/hwmon.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/input.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <asm/io.h>
#include<linux/gpio.h>
#include "wlt_cable.h"

#define Debug
#define WLT_Printk(fmt,arg...)  printk("[WLT CABLE]"fmt"\n",##arg)
#define DEV_NAME "wlt_cable" 

//GPIO15-SCL GPIO17-SDA
#define CS_SCL_1 			*GPIODSET0 = (1<<15)
#define CS_SCL_0		    *GPIODCLR0 = (1<<15)
#define CS_SCL_OUT          *GPIOCTRL0 |= (1<<15)
#define CS_SDA_OUT			*GPIOCTRL0 |= (1<<17)
#define CS_SDA_IN			*GPIOCTRL0 &= ~(1<<17)
#define CS_SDA_DAT     		*GPIODATA0&(1<<17)
#define CS_SDA_OUT_1		*GPIODSET0 = (1<<17)
#define CS_SDA_OUT_0		*GPIODCLR0 = (1<<17)

#define DELAY udelay(5)

volatile unsigned long *GPIOMODE1;
volatile unsigned long *AGPIOCFG;
volatile unsigned long *GPIOCTRL0;
volatile unsigned long *GPIODATA0;
volatile unsigned long *GPIODSET0;
volatile unsigned long *GPIODCLR0;

static unsigned char i2c_addr = 0x14;
static unsigned char FunctionFlag = 0;
static unsigned char RenewFlag = 0;

void wlt_cable_gpio_init(void) {
	AGPIOCFG  = (volatile unsigned long *)ioremap(0x1000003c,4);
	GPIOMODE1 = (volatile unsigned long *)ioremap(0x10000060,4);
    GPIOCTRL0 = (volatile unsigned long *)ioremap(0x10000600,4);
	GPIODATA0 = (volatile unsigned long *)ioremap(0x10000620,4);
    GPIODSET0 = (volatile unsigned long *)ioremap(0x10000630,4);
    GPIODCLR0 = (volatile unsigned long *)ioremap(0x10000640,4);

	//1.ephy->gpio
	*AGPIOCFG |= (0xf<<17);
	//2.GPIO15\17->output 
	*GPIOCTRL0 |= ((1<<15)|(1<<17));
}

void wlt_cable_gpio_free(void) {
	iounmap(GPIOMODE1);
	iounmap(AGPIOCFG);
	iounmap(GPIOCTRL0);
	iounmap(GPIODATA0);
	iounmap(GPIODSET0);
	iounmap(GPIODCLR0);
}
static void i2c_start(void) {	
	CS_SDA_OUT_1;	
	CS_SCL_1;
	DELAY;
	CS_SDA_OUT_0;
	DELAY;
	CS_SCL_0;
 }

 static void i2c_stop(void)	{
	CS_SCL_0;
	CS_SDA_OUT_0;
	DELAY;
	CS_SCL_1;
	DELAY;
	CS_SDA_OUT_1;
 }

 static unsigned char i2c_read_ack(void) {
	//unsigned char errtime = 10;
	CS_SDA_IN;
	//DELAY;
	DELAY;
	CS_SCL_1;
	DELAY;
	DELAY;
	if(CS_SDA_DAT){
		WLT_Printk("ack err");
		CS_SDA_OUT;
		i2c_stop();
		return 1;
	}
	CS_SCL_0;
	CS_SDA_OUT;
	return 0;
 }

 static void i2c_no_ack(void) {
	CS_SDA_OUT_1;
	DELAY;
	CS_SCL_1;
	DELAY;
	CS_SCL_0;
 }
 
 static void i2c_ack(void) {
	CS_SDA_OUT_0;
	DELAY;
	CS_SCL_1;
	DELAY;
	CS_SCL_0;
 }
 
 static void i2c_send_byte(unsigned char data) {
	unsigned char i;
	for(i=0;i<8;i++)	{
		CS_SCL_0;
		if((data&0x80)>0)	
			CS_SDA_OUT_1;
		else
			CS_SDA_OUT_0;
		data<<=1;
		DELAY;
		CS_SCL_1;
		DELAY;
	}
	CS_SCL_0;
 }
 
 static unsigned char i2c_read_byte(void) {
	unsigned char i,data = 0;
	CS_SDA_IN;
	DELAY;
	for(i=0;i<8;i++) {
		data<<=1;
		CS_SCL_0;
		DELAY;
		CS_SCL_1;
		DELAY;
		if(CS_SDA_DAT) {
			data|=1;
		}
	}
	CS_SCL_0;
	CS_SDA_OUT;
	CS_SDA_OUT_1;
	return data;
 }	 
static void i2c_send(unsigned char addr,int reg,int reg_len,unsigned char * data,int data_len) {
	int status;
	i2c_start();
	i2c_send_byte(addr);
	status = i2c_read_ack();
	i2c_send_byte(reg);
	status = i2c_read_ack();
	i2c_send_byte(data);
	status = i2c_read_ack();
	i2c_stop();
 }
 static unsigned char i2c_read(unsigned char addr,int reg,int reg_len,unsigned char *data,int data_len) {
	unsigned char i,status;
	//unsigned char data[16];
	//memset(data,0,16);
	
	i2c_start();
	i2c_send_byte(addr);
	status = i2c_read_ack();
	if(status < 0) {
		i2c_stop();
		return 0;
	}
	i2c_send_byte(reg);
	status = i2c_read_ack();
	//i2c_stop();
	i2c_start();
	i2c_send_byte(addr|0x01);
	status = i2c_read_ack();	
	
	for(i=0;i<data_len;i++)
		data[i] = i2c_read_byte();
	
	i2c_no_ack();
	i2c_stop();
	return data_len;
 }

/*读一个寄存器的接口*/
inline static uchar read_reg(uchar reg,char *buf,int length)
{ 
    return i2c_read(i2c_addr,reg,1,buf,length);  // 接收寄存器的值
}

/********************************************
 *   设置i2c的功能
*******************************************/
inline static uchar write_i2c(uchar addr,uchar onedata){
    i2c_send(i2c_addr,addr,1,&onedata,1);
    return 0;  // 发送寄存器地址 ,值
}

/********************************************
 *   等待i2c数据更新
*******************************************/
inline static char wait_net_update(void){
    char* updateFlag = &RenewFlag;
    int i = 0;
    *updateFlag = I2C_CLEAR;
    // no more than 300ms , is will return during 100-200ms
    for(;  i<100 ;i++){
        read_reg(ADDR_UPDATE, updateFlag,1);
//        printk(KERN_NOTICE "......wait_net_*updateFlag.buf is 0x%2x\n",*updateFlag);
        if((*updateFlag==I2C_CLEAR)){
            mdelay(30);
        }else{
            break;
        }
    }
    return *updateFlag;
}

/********************************************
 *   获取网线测量数据
 *   测试结果放在buf里面:
 *   测试结果是:9位的assic码
*******************************************/
inline int get_netline_data(uchar* buf){
    int rc;
 //   char tmp;

    //  1.  网线测试器功能   // 设置功能
//    read_reg(ts->client, ADDR_FUNCTION, &tmp,1);
//    printk(KERN_NOTICE "1..ADDR_FUNCTION is %02x\n",tmp);
//    if(tmp!=FUN_NET_TEST)
//        write_i2c(ts->client,ADDR_FUNCTION,FUN_NET_TEST);

    //  2、	数据更新标志
//    printk(KERN_NOTICE "2..get_netline_data...\n");
    if(wait_net_update() == I2C_CLEAR){
        return UPDATE_TIME_OUT;
    }

    //  3.  读 DATA and Line number
    rc = read_reg(ADDR_DATA, buf,9);
    //  4.  clear to data flag
    write_i2c(ADDR_UPDATE,I2C_CLEAR);
    return 9;
}

static int i2c_wlt_read (struct file *fp, char __user *usr_buf, size_t length, loff_t *loff){
    int rc;
    uchar  buf[30] = {0};
    switch(FunctionFlag)// 当前功能位
    {
        case FUN_NET_TEST:
            rc = get_netline_data(buf);//获取网线测量数据
        break;
        case FUN_FIND_LINE:// reserve
        break;
        case FUN_LIGHT_PW: // 光功率计功能
            rc = read_reg( ADDR_PRVALUE, buf,3);
        break;
    }
    if(rc == UPDATE_TIME_OUT){
        WLT_Printk( "i2c network test :UPDATE_TIME_OUT error....");
        return rc;
    }else if(rc < 0){
        WLT_Printk( "i2c network test :something error....");
        return rc;
    }
    if(rc < 64){
        if(copy_to_user(usr_buf,buf,rc)!=0){
            WLT_Printk( "i2c network test :copy_to_user error...");
            return -1;
        }
    }else{
        return -1;
    }

    return rc;

}

/******************************************************
 *  用于寻线器的i2c写
 *******************************************************/
static ssize_t i2c_wlt_write(struct file *fp, const char __user *usr_buf, size_t len, loff_t *ptr) {
//    printk(KERN_NOTICE "read...\n");
    struct i2c_wlt_data *ts = fp->private_data;
    int rc;
    char buf[256] = {'0'};
    rc = read_reg(16, buf,2);
    if(rc>0)
        WLT_Printk( "read reg:16:0x%x,17:0x%x...",buf[0],buf[1]);
    else{
        WLT_Printk( "read error.....");
    }
    if(len>255){
        len = 255;
    }
    rc = copy_from_user(buf,usr_buf,len);
    WLT_Printk( "write is addr:0x%02x..data:0x%02x...",buf[0],buf[1]);
    return write_i2c(buf[0],buf[1]);
}
/******************************************************
// 设置功能
*******************************************************/
static int i2c_wlt_ioctl (struct file *fp, unsigned int cmd, unsigned long arg)
{
    char readReg;
    WLT_Printk("i2c_wlt_ioctl....cmd:%c...",cmd);
    switch(cmd)
    {
    case FUN_NET_TEST:
        WLT_Printk("cmd:%d..FUN_NET_TEST...",cmd);
        write_i2c(ADDR_FUNCTION,FUN_NET_TEST);
        FunctionFlag = FUN_NET_TEST;
        return cmd;
    case FUN_FIND_LINE:// reserve
        write_i2c(ADDR_FUNCTION,FUN_FIND_LINE);
        FunctionFlag = FUN_FIND_LINE;
        return cmd;
    case FUN_LIGHT_PW:
        write_i2c(ADDR_FUNCTION,FUN_LIGHT_PW);
        FunctionFlag = FUN_LIGHT_PW;
        return cmd;
    case FUN_POE:
        write_i2c(ADDR_FUNCTION,FUN_POE);
        FunctionFlag = FUN_POE;
        return cmd;
    case REG_READ:
		//mutex_lock(&mymutex);
        WLT_Printk("arg:%d..REG_READ...",arg);
        read_reg(arg, &readReg,1);// arg => addr
		//mutex_unlock(&mymutex);
		return readReg;
    }
    return 0;
}

int i2c_wlt_open(struct inode *inode, struct file *fp){
    WLT_Printk("i2c network test open...");
    RenewFlag = I2C_CLEAR;
    FunctionFlag = FUN_NET_TEST;
    return 0;
}

int i2c_wlt_release(struct inode *inode, struct file *fp){
    WLT_Printk("i2c network test close...");
    write_i2c(0,0x0);   //  write 0x1 use 0xaa
    return 0;
}

static const struct file_operations wlt_cable_fops ={
    .owner   = THIS_MODULE,
    .read    = i2c_wlt_read,
    .write   = i2c_wlt_write,
    .compat_ioctl =  i2c_wlt_ioctl,
    .unlocked_ioctl = i2c_wlt_ioctl,
    .open    = i2c_wlt_open,
    .release = i2c_wlt_release,
};

static struct miscdevice wlt_cable_dev={
    .minor = MISC_DYNAMIC_MINOR,//动态分配次设备号
    .name = DEV_NAME,//设备名
    .fops = &wlt_cable_fops,//文件操作的方法
};

static int __init wlt_cable_init(void)
{
    misc_register(&wlt_cable_dev);
	wlt_cable_gpio_init();
	//for test
	//CS_SCL_0;
	//CS_SDA_OUT_0;
	unsigned char buf = 0;
	write_i2c(0x0,0x00);
	read_reg(0x0,&buf,1);
	WLT_Printk("buf=0x%x",buf);
	return 0;
}

static void __exit wlt_cable_exit(void)
{
    misc_deregister(&wlt_cable_dev);
	wlt_cable_gpio_free();
}

module_init(wlt_cable_init);
module_exit(wlt_cable_exit);

MODULE_AUTHOR("rong");
MODULE_DESCRIPTION("IIC NETWORK TEST driver");
MODULE_LICENSE("GPL");
