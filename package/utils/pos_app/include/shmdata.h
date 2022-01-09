#ifndef _SHMDATA_H_HEADER  
#define _SHMDATA_H_HEADER  
#include "config.h"
//共享内存ID
#define SHM_IPC_ID    111
#define SHM_FLOOR_ID  222
#define SHM_POS_ID    333
//申请共享内存大小，单位B
#define TEXT_SZ 4096  

//IPC配置信息
struct shared_ipc_conf  
{  
	unsigned int useful;//数据是否有效
	unsigned int dataUpdate;
	unsigned int recvBit;
	unsigned int write;//作为一个标志，非0：表示可读，0表示可写  
	unsigned int floorEnable;
	unsigned int posEnable;
	unsigned int type;//IPC类型
	char version[32];//具体型号
	char productName[16];
	char userName[32];//用户名
	char passwd[32];//密码
	unsigned int port;//端口号
	char mess[512];
	unsigned int messCh;
	char ipAddr[32];//IP地址
	unsigned int xPosition;//X坐标开始位置
	unsigned int yPosition;//Y坐标开始位置
	int startFloor;//开始楼层
	int refresh;
	
	unsigned int reserve[32];
};  

//楼层配置信息
typedef struct _ChangeFloor{
	int number;
	char name[100];
}ChangeFloor;
struct shared_floor_conf
{
	unsigned int useful;//数据是否有效
	unsigned int dataUpdate;
	unsigned int recvBit;
	int startFloor;
	int floorStatus;
	int lastFloorStatus;
	unsigned int floorCount;//总的楼层数
	unsigned int floorBelow;//负楼层
	unsigned int floorAbove;//正楼层
	int currentFloor;
	//int actualFloor;
	int lastFloor;
	int dispLastFloor;
	//添加不显示楼层,跃层,改名楼层
	int noDisp[110];//最大长度为地面楼层数+地下楼层数
	//int yueFloor[110];
	ChangeFloor changeName[50]; 
	//统一改名
	char reName[100];
#ifdef SUPPORT_HP303S
	int floorAltitu;
	int floorAltituTotal;
	int useAltitu;
	int diffFloorAltituCnt;
	int currentAltitu;
	struct DiffFloorAltitu {
		int floor;
		int altitu;
	}diffFloorAltitu[16];
#endif
	unsigned int reserve[32];
};

//POS配置信息
struct shared_pos_conf
{
	unsigned int devConnectAbled;//设备是否已经连接,只有当tty或usb正确打开设备才置位1
	unsigned int dataUpdate;
	unsigned int useful;//数据是否有效
	unsigned int recvBit;
	unsigned int interfaceType;
	char *interfaceName;//接口类型
	
	unsigned int ttyConfNumber;//选择串口配置信息
	unsigned int ttyVersion;//型号
	int ttyFd;
	unsigned int openTtyErr;
	unsigned int baudRate;//波特率
	unsigned int dataBit;//数据位
	unsigned int parity;//奇偶校验
	unsigned int stopBit;//停止位
	
	unsigned int usbConfType;//选择USB接口类型
	int usbFd;
	unsigned int openUsbErr;
	unsigned int usbVersion;//型号
	char *usbName;
	
	unsigned int reserve[32];
};

//全局配置信息结构体
struct shared_ipc_conf *ipc_conf;
struct shared_floor_conf *floor_conf;
struct shared_pos_conf *pos_conf;
#endif
