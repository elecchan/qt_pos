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
#include <linux/gpio.h>
#include <linux/kobject.h>
#include "hp303_drv.h"

#define Debug
#define WLT_Printk(fmt,arg...)  printk("[QT HP303]"fmt"\n",##arg)
#define DEV_NAME "HP303" 

/*
	PINCTRL:
		ttyS0 -----> IIC
		TX ----> SDA  ---> GPIO8
		RX ----> SCL  ---> GPIO10
	REGISTER:
		0x10000060 : MODE REGISTER , UART set to gpio mode
			[4:2]:111
		0x10000620 : PIN DATA

		0x10000624 : PIN DIRECTION
		0x1000062C : PIN VALUE SET
		0x10000630 : PIN VALUE CLR
*/

static unsigned char i2c_addr = SENSOR_I2C_SLAVE_ADDRESS<<1;

void hp303_gpio_init(void) {
	uint32_t reg = 0;

	GPIOMODE1 = (volatile unsigned long *)ioremap(0x10000060,4);
    GPIOCTRL0 = (volatile unsigned long *)ioremap(0x10000624,4);
	GPIODATA0 = (volatile unsigned long *)ioremap(0x10000620,4);
    GPIODSET0 = (volatile unsigned long *)ioremap(0x1000062C,4);
    GPIODCLR0 = (volatile unsigned long *)ioremap(0x10000630,4);

	//1.uartf->gpio
	reg = *GPIOMODE1;
	reg |= (7<<2);
	*GPIOMODE1 = reg;
	//2.SDA SCL output
	*GPIOCTRL0 |= ((1 << SCL_P) | (1 << SDA_P));
	//3.clr gpio data
	*GPIODCLR0 |= ((1 << SCL_P) | (1 << SDA_P));
}

void hp303_gpio_free(void) {
	iounmap(GPIOMODE1);
	iounmap(GPIOCTRL0);
	iounmap(GPIODATA0);
	iounmap(GPIODSET0);
	iounmap(GPIODCLR0);
}
static void i2c_start(void) {	
	CS_SDA_OUT;
	CS_SDA_OUT_1;
	DELAY;
	DELAY;
	CS_SCL_1;
	DELAY;
	DELAY;
	CS_SDA_OUT_0;
	DELAY;
	DELAY;
 }

 static void i2c_stop(void)	{
	CS_SDA_OUT;
	CS_SCL_0;
	DELAY;
	CS_SDA_OUT_0;
	DELAY;
	CS_SCL_1;
	DELAY;
	CS_SDA_OUT_1;
	DELAY;
 }

 static unsigned char i2c_read_ack(void) {
	//unsigned char errtime = 10;
	CS_SCL_0;
	DELAY;
	DELAY;
	CS_SCL_1;
	DELAY;
	DELAY;
	CS_SDA_IN;
	DELAY;
	if(CS_SDA_DAT) {
		WLT_Printk("ack err");
		//ack_err = 1;
		CS_SCL_0;
		return 0;
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
	CS_SDA_OUT;
	CS_SDA_OUT_0;
	DELAY;
	CS_SCL_1;
	DELAY;
	CS_SCL_0;
	DELAY;
 }
 
 static void i2c_send_byte(unsigned char data) {
	unsigned char i;
	for(i=0;i<8;i++)	{
		CS_SCL_0;
		DELAY;
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
	DELAY;
 }
 
 static unsigned char i2c_read_byte(void) {
	unsigned char i,data = 0;
	CS_SDA_IN;
	for(i=0;i<8;i++) {
		CS_SCL_0;
		DELAY;
		CS_SCL_1;
		DELAY;
		data<<=1;
		if(CS_SDA_DAT) {
			data|=1;
		}
	}
	CS_SCL_0;
	DELAY;
	return data;
 }	 
static void i2c_send(unsigned char addr,int reg,int reg_len,unsigned char data,int data_len) {
	i2c_start();
	i2c_send_byte(addr);
	i2c_read_ack();
	i2c_send_byte(reg);
	i2c_read_ack();
	i2c_send_byte(data);
	i2c_read_ack();
	i2c_stop();
	//DELAY_LONG;
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
	i2c_stop();
	i2c_start();
	i2c_send_byte(addr|0x01);
	status = i2c_read_ack();	
	
	for(i=0;i<data_len;i++)
		data[i] = i2c_read_byte();
	
	i2c_no_ack();
	i2c_stop();
	return data_len;
 }

#if 1
static ssize_t hp303_sda_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t rc = 0;
    snprintf(buf, 32, "%d\n", (*GPIODATA0 >> SDA_P) & 0x0001);
    rc = strlen(buf);
    return rc;
}
static ssize_t hp303_sda_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	int val = 0;
	sscanf(buf,"%d",&val);
    WLT_Printk("%s set val:%d",__func__,val);
 	if(val == 1) {
 		WLT_Printk("set sda val to 1");
 		CS_SDA_OUT_1;
 	}
 	else {
 		WLT_Printk("set sda val to 0");
 		CS_SDA_OUT_0;
 	}
    return count;
}

static ssize_t hp303_scl_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t rc = 0;
    snprintf(buf, 32, "%d\n", (*GPIODATA0 >> SCL_P) & 0x0001);
    rc = strlen(buf);
    return rc;
}
static ssize_t hp303_scl_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	int val = 0;
	sscanf(buf,"%d",&val);
    WLT_Printk("%s set val:%d",__func__,val);
 	if(val == 1) {
 		WLT_Printk("set scl val to 1");
 		CS_SCL_1;
 	}
 	else {
 		WLT_Printk("set scl val to 0");
 		CS_SCL_0;
 	}
    return count;
}
 
static DEVICE_ATTR(hp303_sda, 0666, hp303_sda_show, hp303_sda_store);
static DEVICE_ATTR(hp303_scl, 0666, hp303_scl_show, hp303_scl_store);
static struct attribute *all_node_attrs[] = {
        &dev_attr_hp303_sda.attr,
        &dev_attr_hp303_scl.attr,
        NULL
};

static struct attribute_group all_node_attr_group = {
        .attrs = all_node_attrs,
};

#endif

static HP303_calib_data HP303_calib; 

int HP303_read8(uint8_t addr,uint8_t reg,uint8_t * data)
{
	int ret;
	ret = i2c_read(addr,reg,1,data,1);
	return ret;
}

int HP303_write8(uint8_t addr,uint8_t reg,uint8_t val)
{
	int ret = 1;
	i2c_send(addr,reg,1,val,1) ;
	return ret;
}
static bool HP303_i2c_test(uint8_t addr)
{
	uint8_t id;	
	HP303_read8(addr,HP303_COEF_REG_ADDR,&id);
	WLT_Printk("hp303 get conf=0x%x",id);
	if(id == 0x0c)
		return true;
	return false;
}

static int HP303_read_calib_coeffs(void)
{
    int ret;
    uint8_t read_buffer[HP303_COEF_LEN] = {0};
	uint8_t data;

	ret = i2c_read(i2c_addr,HP303_COEF_REG_ADDR,1,read_buffer,HP303_COEF_LEN);
	printk("%s:",__func__);
	for(int i = 0;i < ret;i++)
		printk("%d ",read_buffer[i]);
	printk("\n");
    if(ret != HP303_COEF_LEN)
		return ret;

    HP303_calib.C0 = (read_buffer[0] << 4) + ((read_buffer[1] >>4) & 0x0F);
    if(HP303_calib.C0 > POW_2_11_MINUS_1)
        HP303_calib.C0 = HP303_calib.C0 - POW_2_12;

    HP303_calib.C1 = (read_buffer[2] + ((read_buffer[1] & 0x0F)<<8));
    if(HP303_calib.C1 > POW_2_11_MINUS_1)
        HP303_calib.C1 = HP303_calib.C1 - POW_2_12;

    HP303_calib.C00 = ((read_buffer[4]<<4) + (read_buffer[3]<<12)) + ((read_buffer[5]>>4) & 0x0F);
    if(HP303_calib.C00 > POW_2_19_MINUS_1)
        HP303_calib.C00 = HP303_calib.C00 -POW_2_20;

    HP303_calib.C10 = ((read_buffer[5] & 0x0F)<<16) + read_buffer[7] + (read_buffer[6]<<8);
    if(HP303_calib.C10 > POW_2_19_MINUS_1)
        HP303_calib.C10 = HP303_calib.C10 - POW_2_20;

    HP303_calib.C01 = (read_buffer[9] + (read_buffer[8]<<8));
    if(HP303_calib.C01 > POW_2_15_MINUS_1)
        HP303_calib.C01 = HP303_calib.C01 - POW_2_16;

    HP303_calib.C11 = (read_buffer[11] + (read_buffer[10]<<8));
    if(HP303_calib.C11 > POW_2_15_MINUS_1)
        HP303_calib.C11 = HP303_calib.C11 - POW_2_16;

    HP303_calib.C20 = (read_buffer[13] + (read_buffer[12]<<8));
    if(HP303_calib.C20 > POW_2_15_MINUS_1)
        HP303_calib.C20 = HP303_calib.C20 - POW_2_16;

    HP303_calib.C21 = (read_buffer[15] + (read_buffer[14]<<8));
    if(HP303_calib.C21 > POW_2_15_MINUS_1)
        HP303_calib.C21 = HP303_calib.C21 - POW_2_16;

    HP303_calib.C30 = (read_buffer[17] + (read_buffer[16]<<8));
    if(HP303_calib.C30 > POW_2_15_MINUS_1)
        HP303_calib.C30 = HP303_calib.C30 - POW_2_16;
	
    return 0;
}

static uint32_t HP303_get_scaling_coef (uint8_t osr)
{
    uint32_t scaling_coeff;

    switch (osr){
		case OSR_1:
			scaling_coeff = OSR_SF_1;
			break;
		case OSR_2:
			scaling_coeff = OSR_SF_2;
			break;
		case OSR_4:
			scaling_coeff = OSR_SF_4;
			break;
		case OSR_8:
			scaling_coeff = OSR_SF_8;
			break;
		case OSR_16:
			scaling_coeff = OSR_SF_16;
			break;
		case OSR_32:
			scaling_coeff = OSR_SF_32;
			break;
		case OSR_64:
			scaling_coeff = OSR_SF_64;
			break;
		case OSR_128:
			scaling_coeff = OSR_SF_128;
			break;
		default:
			scaling_coeff = OSR_SF_1;
			break;
    }
    return scaling_coeff;
}

int HP303_config(uint8_t osr_temp,uint8_t mr_temp,uint8_t osr_press,uint8_t mr_press)
{
    s32 ret;
    u8  config;

	printk("==%s== temp(%x,%x) press(%x,%x)\r\n",__func__,osr_temp,mr_temp,osr_press,mr_press);

   /* configure temperature measurements first*/
   /*Prepare a configuration word for TMP_CFG register*/
    config = (u8)TMP_EXT_MEMS;

    /*First Set the TMP_RATE[2:0] -> 6:4 */
    config |= ((u8)mr_temp);

   /*Set the TMP_PRC[3:0] -> 2:0 */
    config |= ((u8)osr_temp);

    ret = HP303_write8(i2c_addr,HP303_TMP_CFG_REG_ADDR,config);
    if (ret < 0)
        return -EIO;

    /*Prepare a configuration word for PRS_CFG register*/
    /*First Set the PM_RATE[2:0] -> 6:4 */
    config = (u8) ( 0x00 ) | ((u8)mr_press);

    /*Set the PM_PRC[3:0] -> 3:0 */
    config |= ((u8)osr_press);

    ret = HP303_write8(i2c_addr,HP303_PRS_CFG_REG_ADDR,config);
    if (ret < 0)
        return -EIO;

    /* always take configuration word from state*/
    config = 0;

    /*If oversampling rate for temperature is greater than 8 times, then set TMP_SHIFT bit in CFG_REG */
    if ((u8)osr_temp > (u8)OSR_8){
		config |= (u8)HP303_CFG_TMP_SHIFT_EN_SET_VAL;
    }

    /*If oversampling rate for pressure is greater than 8 times, then set P_SHIFT bit in CFG_REG */
    if ((u8)osr_press > (u8) OSR_8){
		config |= (u8)HP303_CFG_PRS_SHIFT_EN_SET_VAL;
    }

    /* write CFG_REG */
    ret = HP303_write8(i2c_addr,HP303_CFG_REG_ADDR,config);
    if (ret < 0)
		return -EIO;

	/*Update state accordingly with proper scaling factors based on oversampling rates*/
    HP303_calib.tmp_osr_scale_coeff = HP303_get_scaling_coef(osr_temp);
    HP303_calib.prs_osr_scale_coeff = HP303_get_scaling_coef(osr_press);

    return 0;
}
static int HP303_measure(struct HP303_report_s *pf)
{
	int ret;
	uint8_t read_buffer[HP303_PSR_TMP_READ_LEN] = {0};

	ret = i2c_read(i2c_addr,HP303_PSR_TMP_READ_REG_ADDR,1,read_buffer,HP303_PSR_TMP_READ_LEN);
	if(ret < 0){
		printk("HP303_read32_data failed\r\n");
		return -1;
	}
	WLT_Printk("measure val:%d %d %d %d %d %d",read_buffer[0],read_buffer[2],read_buffer[3], \
		read_buffer[4],read_buffer[5],read_buffer[6]);
	pf->pressure[0] = read_buffer[0];
	pf->pressure[1] = read_buffer[1];
	pf->pressure[2] = read_buffer[2];

	pf->temperature[0] = read_buffer[3];
	pf->temperature[1] = read_buffer[4];
	pf->temperature[2] = read_buffer[5];
	
	pf->calib.C0 = HP303_calib.C0;
	pf->calib.C1 = HP303_calib.C1;
	pf->calib.C00 = HP303_calib.C00;
	pf->calib.C10 = HP303_calib.C10;
	pf->calib.C01 = HP303_calib.C01;
	pf->calib.C11 = HP303_calib.C11;
	pf->calib.C20 = HP303_calib.C20;
	pf->calib.C21 = HP303_calib.C21;
	pf->calib.C30 = HP303_calib.C30;
	pf->calib.tmp_osr_scale_coeff = HP303_calib.tmp_osr_scale_coeff;
	pf->calib.prs_osr_scale_coeff = HP303_calib.prs_osr_scale_coeff;

	return 0;
}
int HP303_resume(void)
{
    s32 ret;

    ret = HP303_write8(i2c_addr,HP303_MEAS_CFG_REG_ADDR,(u8)HP303_MODE_BACKGROUND_ALL);
    if(ret < 0)
		return -EIO;

    return 0;
}


static int HP303_init_client(void)
{
	int ret;
   
    mdelay(40);

    /* read now the calibration coeffs, temperature coef source and store in driver state*/
    ret = HP303_read_calib_coeffs();
    if (ret < 0){
		goto err_handler_iio;
    }

    /* configure sensor for default ODR settings*/
    ret = HP303_config( HP303_TEMPERATURE_OSR,
                        HP303_TEMPERATURE_MR,
                        HP303_PRESSURE_OSR,
                        HP303_PRESSURE_MR);
    if (ret < 0){
        goto err_handler_iio;
    }

    /* activate sensor*/
	ret = HP303_resume();
    if (ret < 0){
        goto err_handler_iio;
    }

	WLT_Printk("==%s== done",__func__);
	
	return 0;

err_handler_iio:
	return ret;

}

static int hp303_read (struct file *fp, char __user *usr_buf, size_t length, loff_t *loff){ 
	int ret;
	struct HP303_report_s data;
	WLT_Printk("%s",__func__);
	copy_from_user((void *)&data,usr_buf,sizeof(data));
	
	ret = HP303_measure(&data);
	if(ret < 0){
		WLT_Printk("%s measure err",__func__);
		return -1;
	}
	
	copy_to_user(usr_buf,(void *)&data,sizeof(data));
    return 1;
}


int hp303_open(struct inode *inode, struct file *fp){
    WLT_Printk("%s",__func__);
    int ret;

	if(HP303_i2c_test(i2c_addr) == false){
		WLT_Printk("hp303 detect err");
		return -1;
	}
	
	ret = HP303_init_client();
	if(ret < 0){
		return -1;
	}
	
    return 0;
}

int hp303_release(struct inode *inode, struct file *fp){
    WLT_Printk("%s",__func__);
    return 0;
}

static const struct file_operations hp303_fops ={
    .owner   = THIS_MODULE,
    .read    = hp303_read,
    .open    = hp303_open,
    .release = hp303_release,
};

static struct miscdevice hp303_dev={
    .minor = MISC_DYNAMIC_MINOR,//动态分配次设备号
    .name = SENSOR_NAME,//设备名
    .fops = &hp303_fops,//文件操作的方法
};
struct kobject *all_node_device = NULL;
static int __init hp303_init(void)
{
	WLT_Printk("%s",__func__);
    misc_register(&hp303_dev);
	hp303_gpio_init();
	all_node_device = kobject_create_and_add("hp303", NULL);
    if (all_node_device == NULL) {
        pr_err("%s: subsystem_register failed\n", __func__);
        return -1;
    }
    int rc = sysfs_create_group(all_node_device, &all_node_attr_group);
    if (rc) {
        pr_err("%s: sysfs_create_file failed\n", __func__);
        kobject_del(all_node_device);
    }
	return 0;
}

static void __exit hp303_exit(void)
{
    misc_deregister(&hp303_dev);
	hp303_gpio_free();
	kobject_put(all_node_device);
}

module_init(hp303_init);
module_exit(hp303_exit);

MODULE_AUTHOR("test@qt.com");
MODULE_DESCRIPTION("HP303 driver");
MODULE_LICENSE("GPL");
