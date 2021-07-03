#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iconv.h"
#define MAXLEN 8*1024
extern int iconv_err;

typedef struct _PrinterCmd{
	char cmd_1;
	char cmd_2;
	char need_calc;
	unsigned int cmd_numb;
}PrinterCmd;

struct Ret{
	char need_calc;
	unsigned int cmd_numb;
};

PrinterCmd printerCmd1b[] = {
	{0x1b , 0x4a , 0, 1},
	{0x1b , 0x64 , 0, 1},
	{0x1b , 0x24 , 0, 2},
	{0x1b , 0x5b , 0, 2},
	{0x1b , 0x5c , 0 ,2},
	{0x1b , 0x44 , 0, 2},
	{0x1b , 0x61 , 0, 1},
	{0x1b , 0x63 , 0, 2},
	{0x1b , 0x70 , 0, 3},
	{0x1b , 0x3d , 0, 1},
	{0x1b , 0x32 , 0, 0},
	{0x1b , 0x33 , 0, 1},
	{0x1b , 0x40 , 0, 0},
	{0x1b , 0x20 , 0, 1},
	{0x1b , 0x21 , 0, 1},
	{0x1b , 0x25 , 0, 1},
	{0x1b , 0x26 , 1, 0},//
	{0x1b , 0x2d , 0, 1},
	{0x1b , 0x3f , 0, 1},
	{0x1b , 0x45 , 0, 1},
	{0x1b , 0x47 , 0, 1},
	{0x1b , 0x4d , 0, 1},
	{0x1b , 0x52 , 0, 1},
	{0x1b , 0x56 , 0, 1},
	{0x1b , 0x74 , 0, 1},
	{0x1b , 0x7b , 0, 1},
	{0x1b , 0x2a , 1, 0},//...
	{0x1b , 0x42 , 0, 2},
	{0x1b , 0x43 , 0, 3},
	{0x1b , 0x43 , 0, 3},
	{0x1b , 0x53 , 0, 0},
};
PrinterCmd printerCmd1c[] = {
	{0x1c , 0x70 , 0, 2},
	{0x1c , 0x71 , 1, 0},//done
	{0x1c , 0x21 , 0, 1},
	{0x1c , 0x26 , 0, 0},
	{0x1c , 0x2d , 0, 1},
	{0x1c , 0x2e , 0, 0},
	{0x1c , 0x32 , 0, 74},
	{0x1c , 0x53 , 0, 2},
	{0x1c , 0x57 , 0, 1},
	{0x1c , 0x57 , 0, 1},
};
PrinterCmd printerCmd1d[] = {
	{0x1d , 0x40 , 0, 0},
	{0x1d , 0x4c , 0, 2},
	{0x1d , 0x57 , 0, 2},
	{0x1d , 0x50 , 0, 2},
	{0x1d , 0x61 , 0, 1},
	{0x1d , 0x72 , 0, 1},
	{0x1d , 0x49 , 0, 1},
	{0x1d , 0x21 , 0, 1},
	{0x1d , 0x42 , 0, 1},
	{0x1d , 0x76 , 1, 0},//...
	{0x1d , 0x2a , 1, 0},//...
	{0x1d , 0x2f , 0, 1},
	{0x1d , 0x48 , 0, 1},
	{0x1d , 0x66 , 0, 1},
	{0x1d , 0x68 , 0, 1},
	{0x1d , 0x6b , 1, 0},//
	{0x1d , 0x77 , 0, 1},
	{0x1d , 0x28 , 0, 7},
	{0x1d , 0x0c , 0, 0},
	{0x1d , 0x56 , 0, 1},//end
	{0x1d , 0x28 , 0, 0},
	{0x1d , 0x28 , 0, 0},
	{0x1d , 0x62 , 0, 1},
};

struct Ret findPrinterCmd1bIndex(char cmd)
{
	unsigned int i;
	struct Ret ret;
	for(i=0;i<sizeof(printerCmd1b)/sizeof(PrinterCmd);i++){
		if(printerCmd1b[i].cmd_2 == cmd) {
			ret.need_calc = printerCmd1b[i].need_calc;
			ret.cmd_numb = printerCmd1b[i].cmd_numb;
			return ret;
		}		
 	}
	ret.need_calc = -1;
	return ret;
}
struct Ret findPrinterCmd1cIndex(char cmd)
{
	unsigned int i;
	struct Ret ret;
	for(i=0;i<sizeof(printerCmd1c)/sizeof(PrinterCmd);i++){
		if(printerCmd1c[i].cmd_2 == cmd) {
			ret.need_calc = printerCmd1c[i].need_calc;
			ret.cmd_numb = printerCmd1c[i].cmd_numb;
			return ret;
		}		
 	}
	ret.need_calc = -1;
	return ret;
}
struct Ret findPrinterCmd1dIndex(char cmd)
{
	unsigned int i;
	struct Ret ret;
	for(i=0;i<sizeof(printerCmd1d)/sizeof(PrinterCmd);i++){
		if(printerCmd1d[i].cmd_2 == cmd) {
			ret.need_calc = printerCmd1d[i].need_calc;
			ret.cmd_numb = printerCmd1d[i].cmd_numb;
			return ret;
		}		
 	}
	ret.need_calc = -1;
	return ret;
}

char inbuf[1024] = {0x1d,0x40,0x1c,0x26,0x1b,0x61,0x01,0x1b,0x21,0x30,0x1c,0x57,0x01,0xc1,0xaa,0xbb,0xfa,0xb2,0xe2,0xca,0xd4,0x0a,0x1b,0x61,0x00,0xbc,0xbc,0xca,0xf5,0xd6,0xb8,0xb1,0xea,0x3a,0x0a,0x33,0x36,0xa,0x45,0x54,0xa,0x11};
char outbuf[1024];
int get_printer_data(char *inbuf,char *outbuf,int len) {
	int i,j = 0;
	int offset;
	struct Ret ret;

	for(i=0;i<len;i++) {
		if(inbuf[i] == 0x1b) {
			if(inbuf[i+1] == 0x1c)//if data like(0x1b 0x1c)then exit
				break;
			ret = findPrinterCmd1bIndex(inbuf[i+1]);
			//需要计算偏移
			if(ret.need_calc == 1) {
				//(1b 2a m nL nH)datalen=
				if(inbuf[i+1] == 0x2a) {
					offset = (inbuf[i+2]>1)?((inbuf[i+3]+inbuf[i+4]*256)*3):(inbuf[i+3]+inbuf[i+4]*256);
					i = i + offset + 4;
				}
			}		
			////////////////////////////////////////////////////////////////////
			if(ret.need_calc == 0)
				i = i + ret.cmd_numb + 1;
		}else if(inbuf[i] == 0x1c) {
			ret = findPrinterCmd1cIndex(inbuf[i+1]);			
			//需要计算偏移
			if(ret.need_calc == 1) {
				//(1C 71 n xL xH yL yH)dataLen=(xL+xH*256)*(yL+yH*256)*n*8 Byte
				if(inbuf[i+1] == 0x71) {
					offset = inbuf[i+2]*(inbuf[i+3]+inbuf[i+4]*256)*(inbuf[i+5]+inbuf[i+6]*256)*8;
					printf("1c 71 pass:%d Byte\n",offset);
					if(offset < len) {
						i = i + offset + 8;
					}
					else 
						return -1;
				}
			}
			////////////////////////////////////////////////////////////////////
			if(ret.need_calc == 0)
				i = i + ret.cmd_numb + 1;
		}else if(inbuf[i] == 0x1d) {
			//if(inbuf[i+1] == 0x56)//if data like(0x1d 0x56)then exit
			//	break;
			ret = findPrinterCmd1dIndex(inbuf[i+1]);
			//需要计算偏移
			if(ret.need_calc == 1) {
				//(1d 2a x y)datalen=x*y*8
				if(inbuf[i+1] == 0x2a) {
					offset = inbuf[i+2]*inbuf[i+3]*8;
					i = i + offset + 3;
				}
				//(1d 76 30 m xL xH yL yH)datalen=(xL+xH*256)*(yL+yH*256)
				if(inbuf[i+1] == 0x76) {
					offset = (inbuf[i+4]+inbuf[i+5]*256)*(inbuf[i+6]+inbuf[i+7]*256);
					i = i + offset + 7;
				}
				//1.(1d 6b m d1...dk 00)
				//2.(1d 6b m n d1..dn)datalen=n
				if(inbuf[i+1] == 0x6b) {
					if(0 <= inbuf[i+2] <= 6) {
						//查找结束符0x00
						for(offset=1;offset<255;offset++) {
							if(inbuf[i+2+offset] == 0x0)
								break;
						}
						i = i + offset + 2;
					}else {
						offset = inbuf[i+3];
						i = i + offset + 3;
					}
				}
			}
			////////////////////////////////////////////////////////////////////
			if(ret.need_calc == 0)
				i = i + ret.cmd_numb + 1;
		}else if((inbuf[i] == 0)||(inbuf[i] == 0xc)) {
			;//如果是0或是换行符忽略掉
		}else {
			outbuf[j] = inbuf[i];
			j++;
		}
	}
	
	printf("%s outbuf len=%d\n",__func__,strlen(outbuf));
	for(i=0;i<20;i++)
		printf("%x ",outbuf[i]&0xff);
	printf("\n");
}
///////////////////////////////////////////////////////////////////////////////////
int iconv_conv(char *encFrom,char *encTo,char *inbuf,char *outbuf) {
 	 //获得转换句柄
  	iconv_t cd = iconv_open (encTo, encFrom);
  	if (cd == (iconv_t)-1)
  	{
  	    printf ("iconv_open err\n");
		return -1;
 	}
  	int srclen = 2;
 	/* 打印需要转换的字符串的长度 */
  	//printf("conver srclen=%d\n", srclen);

 	/* 由于iconv()函数会修改指针，所以要保存源指针 */
 	char **srcstart = &inbuf;
   	char **tempoutbuf = &outbuf;
	int outlen = MAXLEN;

 	/* 进行转换
 	*@param cd iconv_open()产生的句柄
  	*@param srcstart 需要转换的字符串
  	*@param srclen 存放还有多少字符没有转换
  	*@param tempoutbuf 存放转换后的字符串
  	*@param outlen 存放转换后,tempoutbuf剩余的空间
 	*
  	* */
 	size_t ret = iconv (cd, srcstart, &srclen, tempoutbuf, &outlen);
 	if (ret == -1)
 	{
 	     printf("iconv err\n");
 	}
	/* 关闭句柄 */
  	iconv_close (cd);
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////
int gb2312_to_utf8(char *inbuf,char *outbuf) {
	int i,j = 0,k = 0;
	int ret;
	//char temp_buf[150];
	char gb2312[2];
	char unicode[2];
	char utf8[3];
	for(i=0;i<strlen(inbuf);i++) {
		if(inbuf[i]&0x80) {
			gb2312[0] = inbuf[i];
			gb2312[1] = inbuf[i+1];
			i++;
			if(iconv_conv("GB2312","UNICODE-1-1",gb2312,unicode) == -1) {
				//转码错误,设置全局转换错误标志为1
				iconv_err = 1;
				break;
			}
			if( iconv_conv("UNICODE-1-1","UTF-8",unicode,utf8) == -1) {
				iconv_err = 1;
				break;
			}
			strcat(outbuf,utf8);
			j = j+3;
		}else {
			outbuf[j] = inbuf[i];
			j++;
		}
	} 
}

int hk_pos_str(char *inbuf,int line,int delay) {
	int i,j = 0,k = 0,line_c = 0;
	char buf[1024];
	char utf8_buf[1024];
	memset(buf,0,sizeof(buf));
	memset(utf8_buf,0,sizeof(utf8_buf));
	for(i=0;i<strlen(inbuf);i++) {
		if(inbuf[i] == 0xa) {//new line
			line_c++;
			for(k=0;k<strlen(buf);k++)
				printf("%x ",buf[k]&0xff);
			printf("\n");
			//change to utf8
			gb2312_to_utf8(buf,utf8_buf);
			printf("utf8:");
			for(k=0;k<strlen(utf8_buf);k++)
				printf("%x ",utf8_buf[k]&0xff);
			printf("\n");
			//
			j = 0;	
			memset(buf,0,sizeof(buf));	
			memset(utf8_buf,0,sizeof(utf8_buf));
		}else {
			buf[j] = inbuf[i];
			j++;
			if(i == (strlen(inbuf) - 1)) {
				for(k=0;k<strlen(buf);k++)
					printf("%x ",buf[k]&0xff);
				printf("\n");
				//change to utf8
				gb2312_to_utf8(buf,utf8_buf);
				printf("utf8:");
				for(k=0;k<strlen(utf8_buf);k++)
					printf("%x ",utf8_buf[k]&0xff);
				printf("\n");
				//
				j = 0;	
				memset(buf,0,sizeof(buf));	
				memset(utf8_buf,0,sizeof(utf8_buf));
			}
		}
		if(line_c == line) {
			printf("refresh time = %dS\n",delay);
			line_c = 0;
			sleep(delay);
		}
	}
}

void test(void) {
	int i;
	memset(outbuf,0,sizeof(outbuf));
	get_printer_data(inbuf,outbuf,42);
	for(i=0;i<strlen(outbuf);i++)
		printf("%x ",outbuf[i]&0xff);
	printf("\n");
	hk_pos_str(outbuf,2,1);
}
