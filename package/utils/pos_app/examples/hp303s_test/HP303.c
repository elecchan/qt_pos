#include <linux/i2c.h>
#include <linux/i2c/uav_sensor.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#define SENSOR_NAME "HP303"
#define SENSOR_I2C_SLAVE_ADDRESS 0x77
#define SENSOR_I2C_SPEED		(1200 * 1000)

#define HP303_PSR_TMP_READ_REG_ADDR	0x00
#define HP303_PRS_CFG_REG_ADDR			0x06
#define HP303_TMP_CFG_REG_ADDR			0x07
#define HP303_MEAS_CFG_REG_ADDR		0x08
#define HP303_CFG_REG_ADDR				0x09
#define HP303_PROD_REV_ID_REG_ADDR		0x0D
#define HP303_COEF_REG_ADDR			0x10
#define HP303_TMP_COEF_SRCE_REG_ADDR	0x28

#define HP303_PSR_TMP_READ_LEN     		6
#define HP303_COEF_LEN						18
#define HP303_TMP_COEF_SRCE_REG_POS_MASK	7

#define HP303_CFG_PRS_SHIFT_EN_SET_VAL	0x04
#define HP303_CFG_TMP_SHIFT_EN_SET_VAL	0x08

#define HP303_MODE_BACKGROUND_ALL		0x07//0b00000111


#define OSR_1		0x00//0b00000000
#define OSR_2		0x01//0b00000001
#define OSR_4		0x02//0b00000010
#define OSR_8		0x03//0b00000011
#define OSR_16		0x04//0b00000100
#define OSR_32		0x05//0b00000101
#define OSR_64		0x06//0b00000110
#define OSR_128		0x07//0b00000111

#define OSR_SF_1	524288
#define OSR_SF_2	1572864
#define OSR_SF_4	3670016
#define OSR_SF_8	7864320
#define OSR_SF_16	253952
#define OSR_SF_32	516096
#define OSR_SF_64	1040384
#define OSR_SF_128	2088960

#define TMP_MR_4	0x20//0b00100000
#define TMP_MR_8	0x30//0b00100000
#define TMP_MR_16	0x40//0b00100000
#define TMP_MR_32	0x50//0b00100000

#define PM_MR_8		0x30//0b00110000
#define PM_MR_16	0x40//0b00110000
#define PM_MR_32	0x50//0b00110000
#define PM_MR_64	0x60//0b01100000

#define HP303_TEMPERATURE_OSR	OSR_8
#define HP303_PRESSURE_OSR		OSR_32
#define HP303_TEMPERATURE_MR	TMP_MR_8
#define HP303_PRESSURE_MR		PM_MR_32

#define POW_2_15_MINUS_1	0x7FFF
#define POW_2_16			0x10000
#define POW_2_11_MINUS_1	0x7FF
#define POW_2_12			0x1000
#define POW_2_20			0x100000
#define POW_2_19_MINUS_1	524287

#define TMP_EXT_ASIC	0x00
#define TMP_EXT_MEMS	0x80

typedef struct
{
	int16_t	C0;
	int16_t	C1;
	int32_t	C00;
	int32_t	C10;
	int16_t	C01;
	int16_t	C11;
	int16_t	C20;
	int16_t	C21;
	int16_t	C30;
	uint32_t tmp_osr_scale_coeff;
	uint32_t prs_osr_scale_coeff;
}HP303_calib_data;

struct HP303_report_s{
	uint8_t  pressure[3];
	uint8_t  temperature[3];
	HP303_calib_data calib;
};

static HP303_calib_data HP303_calib; 
static struct i2c_client *HP303_i2c_client = NULL;

static const unsigned short normal_i2c[2] = {SENSOR_I2C_SLAVE_ADDRESS, I2C_CLIENT_END};

static dev_t devno;
static struct cdev * cdev; 
static struct device *device;
static struct class *cls;

int HP303_read8(struct i2c_client *client,uint8_t reg,uint8_t * data)
{
	int ret;
  
	ret = i2c_smbus_read_i2c_block_data(client,reg,1,data);

	return ret;
}

int HP303_write8(struct i2c_client *client,uint8_t reg,uint8_t val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client,reg,val);

	printk("==HP303_write8 %02x %02x \r\n",reg,val);


	return ret;
}

static bool HP303_i2c_test(struct i2c_client * client)
{
	uint8_t id;
	int ret;
	
	ret = HP303_read8(client,HP303_PROD_REV_ID_REG_ADDR,&id);
	if(ret < 0){
		printk("==%s== get id failed\r\n",__func__);
		return false;
	}
	
	printk("==%s== %x\r\n",__func__,id);

	return true;
}

static int HP303_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	pr_info("%s: addr=0x%x\n",__func__,client->addr);
	strlcpy(info->type, SENSOR_NAME, I2C_NAME_SIZE);
	return 0;
}

static int HP303_read_calib_coeffs(struct i2c_client *client)
{
    int ret;
    uint8_t read_buffer[HP303_COEF_LEN] = {0};
	uint8_t data;

	ret = i2c_smbus_read_i2c_block_data(client,HP303_COEF_REG_ADDR,HP303_COEF_LEN,read_buffer);
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

int HP303_config(struct i2c_client *client,uint8_t osr_temp,uint8_t mr_temp,uint8_t osr_press,uint8_t mr_press)
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

    ret = HP303_write8(client,HP303_TMP_CFG_REG_ADDR,config);
    if (ret < 0)
        return -EIO;

    /*Prepare a configuration word for PRS_CFG register*/
    /*First Set the PM_RATE[2:0] -> 6:4 */
    config = (u8) ( 0x00 ) | ((u8)mr_press);

    /*Set the PM_PRC[3:0] -> 3:0 */
    config |= ((u8)osr_press);

    ret = HP303_write8(client,HP303_PRS_CFG_REG_ADDR,config);
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
    ret = HP303_write8(client,HP303_CFG_REG_ADDR,config);
    if (ret < 0)
		return -EIO;

	/*Update state accordingly with proper scaling factors based on oversampling rates*/
    HP303_calib.tmp_osr_scale_coeff = HP303_get_scaling_coef(osr_temp);
    HP303_calib.prs_osr_scale_coeff = HP303_get_scaling_coef(osr_press);

    return 0;
}

int HP303_resume(struct i2c_client *client)
{
    s32 ret;

    ret = HP303_write8(client,HP303_MEAS_CFG_REG_ADDR,(u8)HP303_MODE_BACKGROUND_ALL);
    if(ret < 0)
		return -EIO;

    return 0;
}


static int HP303_init_client(struct i2c_client *client)
{
	int ret;
   
    mdelay(40);

    /* read now the calibration coeffs, temperature coef source and store in driver state*/
    ret = HP303_read_calib_coeffs(client);
    if (ret < 0){
		goto err_handler_iio;
    }

    /* configure sensor for default ODR settings*/
    ret = HP303_config(client,
    					HP303_TEMPERATURE_OSR,
                        HP303_TEMPERATURE_MR,
                        HP303_PRESSURE_OSR,
                        HP303_PRESSURE_MR);
    if (ret < 0){
        goto err_handler_iio;
    }

    /* activate sensor*/
	ret = HP303_resume(client);
    if (ret < 0){
        goto err_handler_iio;
    }

	printk("==%s== done\r\n",__func__);
	
	return 0;

err_handler_iio:
	return ret;

}

int HP303_open(struct inode *inode, struct file *filp)
{
	int ret;
	
	printk("==%s==\r\n",__func__);

	if(HP303_i2c_test(HP303_i2c_client) == false){
		return -1;
	}
	
	ret = HP303_init_client(HP303_i2c_client);
	if(ret < 0){
		return -1;
	}
	
	return 0; 
}

int HP303_release(struct inode *inode, struct file *filp)
{
	printk("==%s==\r\n",__func__);
	return 0;
}

static int HP303_measure(struct i2c_client *client,struct HP303_report_s *pf)
{
	int ret;
	uint8_t read_buffer[HP303_PSR_TMP_READ_LEN] = {0};

	ret = i2c_smbus_read_i2c_block_data(client,HP303_PSR_TMP_READ_REG_ADDR,HP303_PSR_TMP_READ_LEN,read_buffer);
	if(ret < 0){
		printk("HP303_read32_data failed\r\n");
		return -1;
	}

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


static ssize_t HP303_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	int ret;
	struct HP303_report_s data;

	copy_from_user((void *)&data,buf,sizeof(data));
	
	ret = HP303_measure(HP303_i2c_client,&data);
	if(ret < 0){
		return -1;
	}
	
	copy_to_user(buf,(void *)&data,sizeof(data));
	
	return 1;
}

static const struct file_operations HP303_fops =
{
	.owner = THIS_MODULE,
	.read = HP303_read,
	.open = HP303_open,
	.release = HP303_release,
};

static int HP303_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	printk("==%s==\r\n",__func__);
	if(HP303_i2c_client == NULL && client != NULL){
		client->max_speed = SENSOR_I2C_SPEED;
		HP303_i2c_client = client;
	}

	if(HP303_i2c_client == NULL){
		printk("==%s== failed\r\n",__func__);
	}

	return 0;
}

static int HP303_remove(struct i2c_client *client)
{
	return 0;
}


static const struct i2c_device_id HP303_id[] = {
	{ SENSOR_NAME, 1 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, HP303_id);

static struct i2c_driver HP303_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name	= SENSOR_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= HP303_probe,
	.remove	= HP303_remove,
	.id_table = HP303_id,
	.detect = HP303_detect,
	.address_list	= normal_i2c,
};

static int __init HP303_init(void)
{
	int ret = -1;
	int result;

	printk("==%s==\r\n",__func__);

	ret = i2c_add_driver(&HP303_driver);
	if (ret < 0) {
		printk(KERN_INFO "add HP303 i2c driver failed\n");
		return -ENODEV;
	}

	result = alloc_chrdev_region(&devno, 0, 1, "HP303");
	if (result < 0)
		return result;

	cdev = cdev_alloc();  
	cdev_init(cdev, &HP303_fops);
	cdev->owner = THIS_MODULE;
	result = cdev_add(cdev,devno,1);

    cls = class_create(THIS_MODULE, "HP303");
    if(IS_ERR(cls)){
		ret = PTR_ERR(cls);
		printk("==%s== class_create failed:%d\r\n",__func__,ret);
		
    }
	
	device = device_create(cls,NULL,devno,NULL,"HP303");
	if(IS_ERR(device)){
		ret = PTR_ERR(device);
		printk("==%s== device_create failed:%d\r\n",__func__,ret);
	}

	printk("==%s== done:%d\r\n",__func__,result);

	return ret;
}

static void __exit HP303_exit(void)
{
	printk(KERN_INFO "remove HP303 i2c driver.\n");
	device_destroy(cls,devno);
	class_destroy(cls);
	cdev_del(cdev);
	unregister_chrdev_region(devno,1);
	i2c_del_driver(&HP303_driver);
}

module_init(HP303_init);
module_exit(HP303_exit);

MODULE_DESCRIPTION("HP303 Sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");

