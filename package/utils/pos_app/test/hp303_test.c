#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

static int HP303_fd = -1;

#define POW_2_23_MINUS_1	0x7FFFFF   //implies 2^23-1
#define POW_2_24			0x1000000

void HP303_convert(HP303_report_s * rp,float * pressure,float *temp)
{
	double	 press_raw;
	double	 temp_raw;
	
	double	 temp_scaled;
	double	 temp_final;
	double	 press_scaled;
	double	 press_final;
		
	press_raw = (rp->pressure[2]) + (rp->pressure[1]<<8) + (rp->pressure[0] <<16);
	temp_raw  = (rp->temperature[2]) + (rp->temperature[1]<<8) + (rp->temperature[0] <<16);

	//printf("%lx %lx\r\n",press_raw,temp_raw);
	
	 if(temp_raw > POW_2_23_MINUS_1)
	 	{
		 temp_raw = temp_raw - POW_2_24;
	  }
	
	 if(press_raw > POW_2_23_MINUS_1)
	 	{
		 press_raw = press_raw - POW_2_24;
	 }
	
	 temp_scaled = (double)temp_raw / (double) (rp->calib_coeffs.tmp_osr_scale_coeff);
	
	 temp_final =  (rp->calib_coeffs.C0 /2.0f) + rp->calib_coeffs.C1 * temp_scaled ;
	 
	 press_scaled = (double) press_raw / rp->calib_coeffs.prs_osr_scale_coeff;
	
	 press_final = rp->calib_coeffs.C00 +
					   press_scaled *  (  rp->calib_coeffs.C10 + press_scaled *
					   ( rp->calib_coeffs.C20 + press_scaled * rp->calib_coeffs.C30 )  ) +
					   temp_scaled * rp->calib_coeffs.C01 +
					   temp_scaled * press_scaled * ( rp->calib_coeffs.C11 +
													   press_scaled * rp->calib_coeffs.C21 );
	
	
	 press_final = press_final * 0.01f;  //to convert it into mBar
	
	 *temp = temp_final;
	 *pressure	  = press_final;  //press_final;


	//printf("(%lf %lf)(%f %f)\r\n",temp_final,press_final,*temp,*pressure);
	
}

float HP303_altitude_convert(float Press, float Ref_P)
{
	return 44330 * (1 - powf(((float)Press / (float)Ref_P),(1/5.255)));
}


bool HP303_open()
{
	HP303_fd = open("/dev/HP303", O_RDONLY);
	usleep(10000);
	if(HP303_fd < 0){
		printf("Can not open dev %s!\n", "/dev/HP303");
		return false;
	}

	return true;
}

bool HP303_read(float dt,float * pres,float * alt,float * temp)
{
	static float HP303_timer = 0.0f;
	static float HP303_init_timer = 0.0f;
	static float pressure_ref;
	HP303_report_s report_HP303; 
	float pressure_tmp;
	float temp_tmp;
	float alt_tmp;

	HP303_timer += dt;
	HP303_init_timer += dt;
	if(HP303_timer > (0.04f))
		{
		HP303_timer = 0.0f;
		}
	else
		{
		return false;
		}
	
	if(read(HP303_fd,&report_HP303,sizeof(report_HP303)) > 0)
		{
		HP303_convert(&report_HP303,&pressure_tmp,&temp_tmp);

		if(HP303_init_timer >= 1.0f && pressure_ref == 0)
			{
			pressure_ref = pressure_tmp;
			}

		if(pressure_ref != 0)
			{
			alt_tmp = HP303_altitude_convert(pressure_tmp,pressure_ref);
			//printf("%3.3f %3.3f %3.3f (%3.3f)\r\n",alt_tmp,pressure_tmp,temp_tmp,pressure_ref);
			
			*alt = alt_tmp;
			*pres = pressure_tmp;
			*temp = temp_tmp;

			return true;
			}
		else
			{
			return false;
			}
		return true;
		}
	else
		{
		return false;
	}
}

static float prev_altitu = 0;
void altitu_algm(float altitu)
{
	float curent_altitu = 0;
	static float diff_altitu = 0;
	if(fabs(altitu - prev_altitu) < 0.05) 
		diff_altitu += (altitu - prev_altitu);
	prev_altitu = altitu;
	curent_altitu = altitu - diff_altitu;
	printf("altitu = %fm,diff_altitu = %fm,curent_altitu = %fm\n",altitu,diff_altitu,curent_altitu);
}

void main(void) {
	float diff = 0;
	float altitu_prev = 0;
	int leaf = 0;
	printf("hp303 test...\n");
	float press,altitu,temp;
	if(HP303_open() == false) {
		return;
	}
	while(1) {
		if(HP303_read(0.05,&press,&altitu,&temp) == true) {
			leaf++;
			if(leaf > 5) {
				altitu_algm(altitu);
			}else
				prev_altitu = altitu;		
		}
		usleep(500 * 1000);
	}
}