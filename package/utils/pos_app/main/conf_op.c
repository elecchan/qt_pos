/*
	解析配置文件
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>

#include "shmdata.h"
#include "conf_op.h"
#include "uci_op.h"
#include "dev_op.h"
#include "config.h"

//#define Debug 1

//获取uci配置项返回值
char retMsg[50];
char cmp_str[100];
char *str;

int init_conf(void) 
{
	//内存分配
	floor_conf = malloc(2048);
	ipc_conf->useful = 0;
	ipc_conf->recvBit = 0;
	ipc_conf->messCh = 1;//
	ipc_conf->dataUpdate = 0;
	floor_conf->useful = 0;
	floor_conf->recvBit = 0;
	floor_conf->dataUpdate = 0;
	floor_conf->currentFloor = 1;
	floor_conf->lastFloor = 0;
	floor_conf->floorStatus = 0;
	floor_conf->lastFloorStatus = 0;
	floor_conf->dispLastFloor = 0;
	pos_conf->useful = 0;
	pos_conf->dataUpdate = 0;
	pos_conf->devConnectAbled = 0;
	pos_conf->recvBit = 0;
	pos_conf->ttyFd = -1;
	pos_conf->openTtyErr = 0;
	pos_conf->usbFd = -1;
	pos_conf->openUsbErr = 0;
}

int parse_ipc_conf(void)
{
	//
	uci_get(get_ipc_fenable,retMsg,sizeof(retMsg));
	ipc_conf->floorEnable = atoi(retMsg);
	uci_get(get_ipc_penable,retMsg,sizeof(retMsg));
	ipc_conf->posEnable = atoi(retMsg);
	//获取IPC类型
	uci_get(get_ipc_type,retMsg,sizeof(retMsg));
	ipc_conf->type = atoi(retMsg);
	switch(ipc_conf->type)
	{
		case 1:
		strcpy(ipc_conf->productName ,"hikvision");	
		break;
		case 2:
		strcpy(ipc_conf->productName ,"dahua");	
		break;
		case 3:
		strcpy(ipc_conf->productName ,"xiongmai");	
		break;
		default:
		strcpy(ipc_conf->productName ,"unknow");
		break;
	}
	//获取型号
	uci_get(get_ipc_version,retMsg,sizeof(retMsg));
	strcpy(ipc_conf->version,retMsg);
	//获取IPC端口号
	uci_get(get_ipc_port,retMsg,sizeof(retMsg));
	ipc_conf->port = atoi(retMsg);
	//
	uci_get(get_ipc_mess,retMsg,sizeof(retMsg));
	strcpy(ipc_conf->mess,retMsg);
	
	//获取IPC用户名密码
	uci_get(get_ipc_username,retMsg,sizeof(retMsg));
	strcpy(ipc_conf->userName,retMsg);
	uci_get(get_ipc_passwd,retMsg,sizeof(retMsg));
	strcpy(ipc_conf->passwd,retMsg);
	
	//获取IPC地址
	uci_get(get_ipc_addr,retMsg,sizeof(retMsg));
	strcpy(ipc_conf->ipAddr,retMsg);
	
	//获取IPC水印的坐标
	uci_get(get_ipc_x,retMsg,sizeof(retMsg));
	ipc_conf->xPosition = atoi(retMsg);
	uci_get(get_ipc_y,retMsg,sizeof(retMsg));
	ipc_conf->yPosition = atoi(retMsg);
	
	//获取开始显示楼层数
	uci_get(get_ipc_startfloor,retMsg,sizeof(retMsg));
	ipc_conf->startFloor = atoi(retMsg);
	floor_conf->currentFloor = ipc_conf->startFloor;
	floor_conf->lastFloor = floor_conf->currentFloor;
	//floor_conf->actualFloor = ipc_conf->startFloor;
	//设置底层计数
	set_int_count(floor_conf->currentFloor);

	//refresh
	uci_get(get_ipc_refresh,retMsg,sizeof(retMsg));
	ipc_conf->refresh = atoi(retMsg);

	if(Debug)
		printf("ipc fenable:%d penable:%d type:%d version:%s product:%s port:%d mess:%s username:%s passwd:%s addr:%s x:%d y:%d startfloor:%d refresh:%d\n",ipc_conf->floorEnable,ipc_conf->posEnable,ipc_conf->type,ipc_conf->version,ipc_conf->productName,ipc_conf->port,ipc_conf->mess,ipc_conf->userName,ipc_conf->passwd,ipc_conf->ipAddr,ipc_conf->xPosition,ipc_conf->yPosition,ipc_conf->startFloor,ipc_conf->refresh);
	
	return 0;
}
//
//从字符串中截取两个子字符串之间的字符串
void strcat_from_2str(char *dest,char *src1,char *src2) {
    char *tmp1;
    char *tmp2;
    int len1;
    int len2;
    int len3;
    int i;
    memset(&cmp_str,0,sizeof(cmp_str));
    //先获取第一个字符串及后面的字符串
    tmp1 = strstr(dest,src1);
    if(tmp1 == NULL)
    	return;
    len1 = strlen(tmp1);
    //从前面基础获取后面字符串
    tmp2 = strstr(tmp1,src2);
    if(tmp2 == NULL)
    	return;
    len2 = strlen(tmp2);
    len3 = strlen(src1);
    for(i=0;i<(len1-len2-len3);i++) {
        cmp_str[i] = tmp1[len3+i];
    }
}
int parse_floor_conf(void)
{
	//获取正负楼层数
	uci_get(get_floor_below,retMsg,sizeof(retMsg));
	floor_conf->floorBelow = atoi(retMsg);
	uci_get(get_floor_above,retMsg,sizeof(retMsg));
	floor_conf->floorAbove = atoi(retMsg);
	floor_conf->startFloor = ipc_conf->startFloor;
	floor_conf->currentFloor = floor_conf->startFloor;
	//floor_conf->actualFloor = ipc_conf->startFloor;
	floor_conf->lastFloor = 0;
	floor_conf->dispLastFloor = 0;
	//得到总的楼层数
	floor_conf->floorCount = floor_conf->floorBelow + floor_conf->floorAbove;
	//获取统一改名
	uci_get(get_floor_rename,retMsg,sizeof(retMsg));
	strcpy(floor_conf->reName,retMsg);
	//解析不显示楼层,跃层,改名楼层
	int i = 0;
	char temp[1110];
	char *ptr;
	memset(temp,0,sizeof(temp)); 
	memset(floor_conf->noDisp,0,sizeof(floor_conf->noDisp));
	uci_get(get_floor_nodisp,temp,sizeof(temp));
	ptr = temp;
	//printf("nodisp 1:%s %d\n",ptr,atoi(ptr));
#ifdef SUPPORT_HP303S
	uci_get(get_floor_altitu,retMsg,sizeof(retMsg));
	floor_conf->floorAltitu = atoi(retMsg);//get altitu for average
	if(floor_conf->floorAltitu <= 0)//adjust if altitu is useful
		floor_conf->useAltitu = 0;
	else
		floor_conf->useAltitu = 1;
	printf("--------get average altitu=%d,flag=%d\n",floor_conf->floorAltitu,floor_conf->useAltitu);
#endif
	while(atoi(ptr) != 0) {
		if(atoi(ptr) < -9) {
			break;
		}else if(atoi(ptr) < 0) {
			floor_conf->noDisp[i] = atoi(ptr);
			ptr = ptr + 3;
		}else if(atoi(ptr) > 10) {
			floor_conf->noDisp[i] = atoi(ptr);
			ptr = ptr + 3;
		}else{
			floor_conf->noDisp[i] = atoi(ptr);
			ptr = ptr + 2;
		}		
		printf("nodisp:%d\n",floor_conf->noDisp[i]);
		i++;
	}
	/*
	uci_get(get_floor_yue,temp,sizeof(temp));
	ptr = temp;
	i = 0;
	//printf("yue 1:%s %d\n",ptr,atoi(ptr));
	while(atoi(ptr) != 0) {
		if(atoi(ptr) < -9) {
			break;
		}else if(atoi(ptr) < 0) {
			floor_conf->yueFloor[i] = atoi(ptr);
			ptr = ptr + 3;
		}else if(atoi(ptr) > 10) {
			floor_conf->yueFloor[i] = atoi(ptr);
			ptr = ptr + 3;
		}else{
			floor_conf->yueFloor[i] = atoi(ptr);
			ptr = ptr + 2;
		}		
		printf("yue:%d\n",floor_conf->yueFloor[i]);
		i++;
	}
	*/
	memset(floor_conf->changeName,0,sizeof(floor_conf->changeName));
	uci_get(get_floor_change,temp,sizeof(temp));
	ptr = temp;
	i = 0;
	printf("change 1:%s %d\n",ptr,atoi(ptr));
	while(atoi(ptr) != 0) {
		if(atoi(ptr) < -9) {
			break;
		}else if(atoi(ptr) < 0) {
			floor_conf->changeName[i].number = atoi(ptr);
			ptr = ptr + 2;
			strcat_from_2str(ptr,":",",");
			memset(floor_conf->changeName[i].name,0,100);
			strcpy(floor_conf->changeName[i].name,cmp_str);
			ptr = ptr + strlen(floor_conf->changeName[i].name) + 2;
		}else if(atoi(ptr) > 10) {
			floor_conf->changeName[i].number = atoi(ptr);
			ptr = ptr + 2;
			strcat_from_2str(ptr,":",",");
			memset(floor_conf->changeName[i].name,0,100);
			strcpy(floor_conf->changeName[i].name,cmp_str);
			ptr = ptr + strlen(floor_conf->changeName[i].name) + 2;
		}else{
			floor_conf->changeName[i].number = atoi(ptr);
			ptr = ptr + 1;
			strcat_from_2str(ptr,":",",");
			memset(floor_conf->changeName[i].name,0,100);
			strcpy(floor_conf->changeName[i].name,cmp_str);
			ptr = ptr + strlen(floor_conf->changeName[i].name) + 2;
		}	
		i++;
		if(i >= 10)
			break;
	}
	
	if(Debug)
		printf("floor below:%d above:%d count:%d\n",floor_conf->floorBelow,floor_conf->floorAbove,floor_conf->floorCount);
	return 0;
}
//判断是否跃层楼层
//是-返回1
//否-返回0
/*int it_is_yue_floor(int floor) {
	int i;
	for(i=0;i<(sizeof(floor_conf->yueFloor)/4);i++) {
		if(floor == floor_conf->yueFloor[i])
			return 1;
	}
	return 0;
}
*/
//判断是否不显示楼层
//是-返回1
//否-返回0
int it_is_disp_floor(int floor) {
	int i;
	for(i=0;i<(sizeof(floor_conf->noDisp)/4);i++) {
		if(floor == floor_conf->noDisp[i])
			return 1;
	}
	return 0;
}
//判断是否改名
//是:返回数组下标
//否:返回-1
int it_is_change_name(int floor) {
	int i;
	for(i=0;i<10;i++) {
		if(floor == floor_conf->changeName[i].number)
			return i;
	}
	return -1;
}

int parse_pos_conf(void)
{
	//获取接口类型
	uci_get(get_pos_interfacetype,retMsg,sizeof(retMsg));
	pos_conf->interfaceType = atoi(retMsg);
	pos_conf->interfaceName = (atoi(retMsg) == 1)?"serial":"USB";
	uci_get(get_pos_ttyversion,retMsg,sizeof(retMsg));
	pos_conf->ttyVersion = atoi(retMsg);
	uci_get(get_pos_usbversion,retMsg,sizeof(retMsg));
	pos_conf->usbVersion = atoi(retMsg);
	//默认优先使用串口接口
	switch(pos_conf->interfaceType) {
		case 1:
		uci_get(get_pos_ttynumber,retMsg,sizeof(retMsg));
		pos_conf->ttyConfNumber = atoi(retMsg);
		switch(pos_conf->ttyConfNumber) {
			case 1://配置1
			uci_get(get_pos_baudrate1,retMsg,sizeof(retMsg));
			pos_conf->baudRate = atoi(retMsg);
			uci_get(get_pos_databit1,retMsg,sizeof(retMsg));
			pos_conf->dataBit = atoi(retMsg);
			uci_get(get_pos_parity1,retMsg,sizeof(retMsg));
			pos_conf->parity = atoi(retMsg);
			uci_get(get_pos_stopbit1,retMsg,sizeof(retMsg));
			pos_conf->stopBit = atoi(retMsg);
			break;
			case 2://配置2
			uci_get(get_pos_baudrate2,retMsg,sizeof(retMsg));
			pos_conf->baudRate = atoi(retMsg);
			uci_get(get_pos_databit2,retMsg,sizeof(retMsg));
			pos_conf->dataBit = atoi(retMsg);
			uci_get(get_pos_parity2,retMsg,sizeof(retMsg));
			pos_conf->parity = atoi(retMsg);
			uci_get(get_pos_stopbit2,retMsg,sizeof(retMsg));
			pos_conf->stopBit = atoi(retMsg);
			break;
			case 3://配置3
			uci_get(get_pos_baudrate3,retMsg,sizeof(retMsg));
			pos_conf->baudRate = atoi(retMsg);
			uci_get(get_pos_databit3,retMsg,sizeof(retMsg));
			pos_conf->dataBit = atoi(retMsg);
			uci_get(get_pos_parity3,retMsg,sizeof(retMsg));
			pos_conf->parity = atoi(retMsg);
			uci_get(get_pos_stopbit3,retMsg,sizeof(retMsg));
			pos_conf->stopBit = atoi(retMsg);
			break;
			case 4://配置4
			uci_get(get_pos_baudrate4,retMsg,sizeof(retMsg));
			pos_conf->baudRate = atoi(retMsg);
			uci_get(get_pos_databit4,retMsg,sizeof(retMsg));
			pos_conf->dataBit = atoi(retMsg);
			uci_get(get_pos_parity4,retMsg,sizeof(retMsg));
			pos_conf->parity = atoi(retMsg);
			uci_get(get_pos_stopbit4,retMsg,sizeof(retMsg));
			pos_conf->stopBit = atoi(retMsg);
			break;
			default:
			pos_conf->baudRate = 0;
			break;
		}
		if(Debug)
			printf("tty configure interfaceType:%d ttyConfNumber:%d baudrate:%d databit:%d parity:%d stopbit:%d\n",pos_conf->interfaceType,pos_conf->ttyConfNumber,pos_conf->baudRate,pos_conf->dataBit,pos_conf->parity,pos_conf->stopBit);
		break;
		case 2:
		uci_get(get_pos_usbnumber,retMsg,sizeof(retMsg));
		pos_conf->usbConfType = atoi(retMsg);
		switch(pos_conf->usbConfType) {
			case 1:
			pos_conf->usbName = "hisense";
			break;
			case 2:
			pos_conf->usbName = "IBM";
			break;
			case 3:
			pos_conf->usbName = "ideal";
			break;
			default:
			pos_conf->usbName = "unknow";
			break;
		}
		if(Debug)
			printf("usb configure interfaceType:%d usbNumber:%d productName:%s\n",pos_conf->interfaceType,pos_conf->usbConfType,pos_conf->usbName);
		break;
		default:
		break;
	}
	return 0;
}
