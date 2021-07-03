#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

#define MAXLEN 1024

int main(int argc, char **argv)
{
	int ret;
	/* 需要转换的字符串 */
  	char inbuf[1024] = {0xC4,0xE3,0x30}; 
  	//char inbuf[1024] = {0x4f,0x60};
	/* 存放转换后的字符串 */
 	char tembuf[MAXLEN];
	char outbuf[MAXLEN];
 	memset (outbuf, 0, sizeof(tembuf));
	memset (outbuf, 0, sizeof(outbuf));

	ret = iconv_conv("GB2312","UNICODE-1-1",inbuf,tembuf);
	if(ret != 0)
		return -1;
	int i = 0;
  	for (i=0; i<strlen(tembuf); i++)
 	{
  	   	printf("%x ", tembuf[i]&0xff);
	}
	printf("\n");
	
	iconv_conv("UNICODE-1-1","UTF-8",tembuf,outbuf);
	if(ret != 0)
		return -1;
  	for (i=0; i<strlen(outbuf); i++)
 	{
  	   	printf("%x ", outbuf[i]&0xff);
	}
	printf("\n");

 	return 0;
}

int iconv_conv(char *encFrom,char *encTo,char *inbuf,char *outbuf) {
 	 //获得转换句柄
  	iconv_t cd = iconv_open (encTo, encFrom);
  	if (cd == (iconv_t)-1)
  	{
  	    printf ("iconv_open err\n");
		return -1;
 	}
  	size_t srclen = strlen (inbuf);
 	 /* 打印需要转换的字符串的长度 */
  	printf("conver srclen=%d\n", srclen);

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
 	     perror ("iconv");
		 return -2;
 	}
	/* 关闭句柄 */
  	iconv_close (cd);
	return 0;
}
