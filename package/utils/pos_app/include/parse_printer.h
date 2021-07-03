/*base64.h*/  
#ifndef _PARSE_PRINTER_H  
#define _PARSE_PRINTER_H  
int iconv_err;
int gb2312_to_utf8(char *inbuf,char *outbuf);
int get_printer_data(char *inbuf,char *outbuf,int len);  
  
#endif  