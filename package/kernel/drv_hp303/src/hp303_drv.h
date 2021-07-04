#ifndef _HP303_DRV_H
#define _HP303_DRV_H

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

#define DELAY udelay(5)
#define DELAY_LONG msleep(15)

volatile unsigned long *GPIOMODE1;
volatile unsigned long *GPIOCTRL0;
volatile unsigned long *GPIODATA0;
volatile unsigned long *GPIODSET0;
volatile unsigned long *GPIODCLR0;

#define SCL_P	10
#define SDA_P	8

#define CS_SCL_1 			*GPIODSET0 = (1 << SCL_P)
#define CS_SCL_0		    *GPIODCLR0 = (1 << SCL_P)
#define CS_SCL_OUT          *GPIOCTRL0 |= (1 << SCL_P)
#define CS_SDA_OUT			*GPIOCTRL0 |= (1 << SDA_P)
#define CS_SDA_IN			*GPIOCTRL0 &= ~(1 << SDA_P)
#define CS_SDA_DAT     		*GPIODATA0 & (1 << SDA_P)
#define CS_SDA_OUT_1		*GPIODSET0 = (1 << SDA_P)
#define CS_SDA_OUT_0		*GPIODCLR0 = (1 << SDA_P)

#endif