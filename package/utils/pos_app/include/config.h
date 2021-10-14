#ifndef _CONFIG_H
#define _CONFIG_H

#define SUPPORT_HP303S
#define SUPPORT_POS
//#define SUPPORT_INT_DETECTION

#define Debug 0
//#define UART_INIT 1
//接收串口最大数据缓存 128KB
#define MAX 128*1024
#define pos_debug(format,...) printf("[POS_DEBUG]%s:%d:%s,"format"\n",__FILE__,__LINE__,__func__,##__VA_ARGS__)
#endif