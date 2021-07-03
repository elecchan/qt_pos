//#ifndef TEST_H
//#define TEST_H
///*
// * Client data (each client gets its own)
// */

//#define PACKET_SIZE		18//22
//typedef unsigned char uchar;


//struct I2C_Regs {	//
//    unsigned char FunctionFlag;		    //  功能标志
//    unsigned char RenewFlag;			//数据更新标志
//    unsigned char LineNum[8];			//
//    unsigned char Remote;				//测试头编号
//    unsigned char FreqSeq;              // reserve
//    unsigned char PRValue;				//reserve
//    unsigned int  ADC_Data;				//reserve
//} ;	//

//#define FUN_NET_TEST  'C'  // 网线测试器功能
//#define FUN_FIND_LINE 'S'  // 寻线器功能
//#define FUN_LIGHT_PW  'P'  // 光功率计功能
//#define ADDR_FUNCTION 0x00
//#define ADDR_UPDATE   0x01
//#define ADDR_DATA     0x02
//#define ADDR_LINE_NUM 0x0A
//#define I2C_UPDATE    0xaa // 数据更新标志位
//#define I2C_CLEAR     0x00  // 读完数据之后，要赋值本位为0，则开始下一次测试

///*error number */
//#define UPDATE_TIME_OUT -10
//#define READ_ERROR      -20


//struct i2c_wlt_data {
//    struct i2c_client *client;
//    struct file *fp;
//    struct I2C_Regs I2C_DataRW;

//};

//#endif // TEST_H


#ifndef TEST_H
#define TEST_H
/*
 * Client data (each client gets its own)
 */

#define PACKET_SIZE		18//22
typedef unsigned char uchar;


struct I2C_Regs {	//
    unsigned char FunctionFlag;		    //  功能标志
    unsigned char RenewFlag;			//数据更新标志
    unsigned char LineNum[8];			//
    unsigned char Remote;				//测试头编号
    unsigned char FreqSeq;              // reserve
    unsigned char PRValue;				//reserve
    unsigned int  ADC_Data;				//reserve
} ;	//

#define FUN_NET_TEST  'C'  // 网线测试器功能
#define FUN_FIND_LINE 'T'  // 寻线器功能
#define FUN_LIGHT_PW  'P'  // 光功率计功能
#define FUN_POE 	  'E'  // POE功能
#define REG_READ      'R'
#define ADDR_FUNCTION 0x00
#define ADDR_UPDATE   0x01
#define ADDR_DATA     0x02
#define ADDR_LINE_NUM 0x0A
#define ADDR_FREQ     0x0B // 寻线器
#define ADDR_PRVALUE  0x0C // 光功率计
#define ADDR_ADC      0x0D // 光功率计


#define I2C_UPDATE    0xaa // 数据更新标志位
#define I2C_CLEAR     0x00 // 读完数据之后，要赋值本位为0，则开始下一次测试



/*error number */
//#define UPDATE_TIME_OUT 0
#define UPDATE_TIME_OUT -10
#define READ_ERROR      -20



#endif // TEST_H

