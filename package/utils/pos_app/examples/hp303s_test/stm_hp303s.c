#include "hp303s.h"
#include "math.h"

/******************************************************************/
//IO方向设置
#define SDA_IN()  {GPIOB->MODER&=~(3<<(11*2));GPIOB->MODER|=0<<11*2;}	//PB7输入模式
#define SDA_OUT() {GPIOB->MODER&=~(3<<(11*2));GPIOB->MODER|=1<<11*2;} //PB7输出模式
//IO操作函数	 
#define IIC_SCL    PBout(10) //SCL
#define IIC_SDA    PBout(11) //SDA	 
#define READ_SDA   PBin(11)  //输入SDA 


int    c0;
int    c1;
long    c00;
long    c10;
int    c01;
int    c11;
int    c20;
int    c21;
int    c30;

uint32_t  HP303_T;
uint32_t  HP303_P;
uint32_t  HP303_high_value;

#define PRS_CFG	0x36//
#define TMP_CFG	0xb0//
#define CFG_REG	0x04 //

///KT,KP
double Scale_Factor[8]={524288,1572864,3670016,7864320,253952,516096,1040384,2088960};
#define KT	(Scale_Factor[TMP_CFG&0x0f])
#define KP	(Scale_Factor[PRS_CFG&0x0f])

//初始化IIC
//PB6-SCL PB7-SDA
static void IIC_Init(void)
{			
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟

  //GPIOB8,B9初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	IIC_SCL=1;
	IIC_SDA=1;
}
//产生IIC起始信号
static void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
static void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
static void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
//不产生ACK应答		    
static void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
static void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
static u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(2);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}
/******************************************************************/
//
char I2CBurstWrite(unsigned char chipAddr, unsigned char WriteAddr, unsigned char ptr[], unsigned char Length)
{
	u8 ack,i;
//	u8 data;
	
  IIC_Start();
	IIC_Send_Byte(chipAddr);
	ack=IIC_Wait_Ack();
	if(ack==1){
		IIC_Stop();
		return (char)-1;
	}
	IIC_Send_Byte(WriteAddr);
	ack=IIC_Wait_Ack();
	if(ack==1){
		IIC_Stop();
		return (char)-1;
	}
	for(i=0;i<Length;i++) {
		IIC_Send_Byte(ptr[i]);	
		ack=IIC_Wait_Ack();
		if(ack==1)
			break;
	}
	delay_us(20);
	IIC_Stop();
	delay_us(20);
	return 1;
} 
//
char I2CBurstRead(u8 chipAddr, u8 ReadAddr, u8 ptr[], u8 Length)
{
	u8 ack,i;
//	u8 data;
	
	IIC_Start();
	IIC_Send_Byte(chipAddr);
	ack=IIC_Wait_Ack();
	if(ack==1){
		IIC_Stop();
		return (char)-1;
	}
	IIC_Send_Byte(ReadAddr);
	ack=IIC_Wait_Ack();
	if(ack==1){
		IIC_Stop();
		return (char)-1;
	}
	IIC_Stop();
	
	IIC_Start();
	delay_us(20);
	IIC_Send_Byte(chipAddr|1);
	ack=IIC_Wait_Ack();
	if(ack==1){
		IIC_Stop();
		return (char)-1;
	}
	delay_us(20);
	for(i=0;i<(Length-1);i++)
		ptr[i]=IIC_Read_Byte(1);
	ptr[i]=IIC_Read_Byte(0);	
	delay_us(20);
	IIC_Stop();
	delay_us(20);
	return 1;
}	
void ReadCalCoef(void)
{
	uint8_t DataBuf[18];
//	u8 i;
	I2CBurstRead(0xee,0x10,DataBuf,18);
//	printf("hp303s 0x10-0x21 read reg:\r\n");
//	for(i=0;i<18;i++) {
//		I2CBurstRead(0xee, 0x10+i, DataBuf, 1);
//		printf("0x%x=0x%x ",0x10+i,DataBuf[0]);
//	}
	
	c0=DataBuf[0]<<4|((DataBuf[1] >>4) & 0x0F);
	if(c0>2047)
	{
		c0=c0-4096;
	}

	c1=(DataBuf[1]&0x0f)<<8| DataBuf[2]; 
	if(c1>2047)
	{
		c1=c1-4096;
	}


	c00=(DataBuf[3]<<12)|(DataBuf[4]<<4)|((DataBuf[5] >>4) & 0x0F);	


	if(c00>524287)
	{
		c00=c00-1048576;
	}

	c10=((DataBuf[5]&0x0f)<<16)|(DataBuf[6]<<8)|DataBuf[7]; 


	if(c10>524287)
	{
		c10=c10-1048576;
	}


	c01=(DataBuf[8]<<8)|(DataBuf[9]);
	if(c01>32767)
	{
		c01=c01-65536;
	}

	

	c11=(DataBuf[10]<<8)|(DataBuf[11]);
	if(c11>32767)
	{
		c11=c11-65536;
	}

	c20=(DataBuf[12]<<8)|(DataBuf[13]);
	if(c20>32767)
	{
		c20=c20-65536;            
	}
	
	c21=(DataBuf[14]<<8)|(DataBuf[15]);   
	if(c21>32767)
	{
		c21=c21-65536;
	} 
	
	c30=(DataBuf[16]<<8)|(DataBuf[17]); 
	if(c30>32767)
	{
		c30=c30-65536;
	}    
}
void setHP303(void)
{
		uint8_t DataBuf[2];
	  DataBuf[0]=PRS_CFG;    
		I2CBurstWrite(0xee,0x06,DataBuf,1);//pressure oversampling rate 64 times
		DataBuf[0] = 0;
		I2CBurstRead(0xee, 0x06, DataBuf, 1);
		//printf("1.0x%x=0x%x \r\n",0x06,DataBuf[0]);
	
    DataBuf[0]=TMP_CFG; 
		I2CBurstWrite(0xee,0x07,DataBuf,1);//temperature oversampling rate 1 times
	
    DataBuf[0]=CFG_REG;    
		I2CBurstWrite(0xee,0x09,DataBuf,1);
	
}
void ReadHP303(void)
{
    uint8_t DataBuf[3];
    long    Traw;
    long    Praw;
    double  Traw_sc;
    double  Praw_sc;
	
	double  d_HP303_P;
	
//    long i;
 
    DataBuf[0]=0x02;    
		I2CBurstWrite(0xee,0x08,DataBuf,1); //temperature measurement    

		delay_ms(10);

    I2CBurstRead(0xee,0x03,DataBuf,3);//Read Temperature adc
		delay_ms(5);
  //  printf("Read Temperature adc,%x,%x,%x\r\n",DataBuf[0],DataBuf[1],DataBuf[2]);
    Traw=(DataBuf[2]) + (DataBuf[1]<<8) + (DataBuf[0] <<16);    
    
    if(Traw>8388607)
    {
        Traw=Traw-16777216;
    } 
    
    DataBuf[0]=0x01;    
    I2CBurstWrite(0xee,0x08,DataBuf,1);//pressure measurement 
    
    delay_ms(120);
    I2CBurstRead(0xee,0x00,DataBuf,3);
		delay_ms(5);
	//	printf("pressure measurement ,%x,%x,%x\r\n",DataBuf[0],DataBuf[1],DataBuf[2]);
    Praw=(DataBuf[2]) + (DataBuf[1]<<8) + (DataBuf[0] <<16); 
    if(Praw>8388607)
    {
        Praw=Praw-16777216;
    }
    
    Traw_sc=((double)Traw)/524288;
    Praw_sc=((double)Praw)/1040384;   
		delay_ms(5);
		//printf("Praw=%d,Traw=%d,Praw_sc=%f,Traw_sc=%f\r\n",Praw,Traw,Praw_sc,Traw_sc);
    
    HP303_T=(float)c0*0.5f + (float)c1*Traw_sc;
    
    HP303_P=c00 + Praw_sc *(c10 + Praw_sc *(c20+ Praw_sc *c30)) + Traw_sc*c01 + Traw_sc *Praw_sc *(c11+Praw_sc *c21); 	
	
		d_HP303_P=c00 + Praw_sc *(c10 + Praw_sc *(c20+ Praw_sc *c30)) + Traw_sc*c01 + Traw_sc *Praw_sc *(c11+Praw_sc *c21); 	
		
		
		//HP303_high_value = 44330.76923 * (1-pow((d_HP303_P/101325),0.190391633));//
		HP303_high_value = 4433076.923 * (1-pow((d_HP303_P/101325),0.190391633));//
   	//HP303_P/=100;//Hpa
}


void hp303s_test() {
	//读取0x10-0x21寄存器
	u8 i;
	uint8_t data[3];
	printf("hp303s_test read reg:\r\n");
	for(i=0;i<18;i++) {
		I2CBurstRead(0xee, 0x10+i, data, 1);
		printf("0x%x=0x%x ",0x10+i,data[0]);
	}
	printf("\r\n");
	data[0]=0x01;    
  I2CBurstWrite(0xee,0x08,data,1);//pressure measurement 
	I2CBurstRead(0xee,0x00,data,1);//Read Temperature adc
  printf("Read p adc,%x,%x,%x\r\n",data[0],data[1],data[2]);
	data[0]=0x02;    
	I2CBurstWrite(0xee,0x08,data,1); //temperature measurement    
	I2CBurstRead(0xee,0x03,data,1);//Read Temperature adc
  printf("Read t adc,%x,%x,%x\r\n",data[0],data[1],data[2]);
}

void hp303s_init(void)
{
	IIC_Init();
	ReadCalCoef();	
	setHP303();
	//hp203b_command(SOFT_RST);
}

