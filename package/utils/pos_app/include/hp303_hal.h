#ifndef _HP303_HAL_H
#define _HP303_HAL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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

typedef struct{
	uint8_t  pressure[3];
	uint8_t  temperature[3];
	HP303_calib_data calib_coeffs;
}HP303_report_s;

#define HP303_UPDATE_RATE 50

#define POW_2_23_MINUS_1	0x7FFFFF   //implies 2^23-1
#define POW_2_24			0x1000000

bool HP303_open(void);
bool HP303_read(float dt,float * pres,float * alt,float * temp);

#endif