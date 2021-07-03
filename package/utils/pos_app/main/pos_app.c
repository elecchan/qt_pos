#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <pthread.h>

#include "shmdata.h"
#include "conf_op.h"
#include "ipc_op.h"
#include "dev_op.h"
#include "parse_printer.h"

#define Debug 0
#define UART_INIT 1
//接收串口最大数据缓存 128KB
#define MAX 128*1024
int first_run(void);
void do_main(void);
//串口数据缓存跟临时缓存
char uart_buf[MAX];
char uart_temp[MAX];
//解析打印机串口数据后的输出缓存
char out_buf[MAX];
//接收到串口总的数据长度
volatile int uart_len = 0;
//ipc的刷新周期
volatile int delay = 1;
volatile int key_count = 0;
//重启标志
int reboot = 0;
int recovery = 0;
int usleep_count = 0;
int uart_send_flag = 0;

//线程,不断获取中断状态
void *read_int_thread(void) {
	int int_count;
	//printf("go to thread\n");
	while(1) {
		//获取当前楼层
		floor_conf->currentFloor = get_int_count();
		if((floor_conf->currentFloor != floor_conf->lastFloor) || (floor_conf->floorStatus != floor_conf->lastFloorStatus)) {
			//判断电梯层数是否在设置范围内,如果是才更新楼层显示
			if(floor_conf->currentFloor < 0) {
				if(abs(floor_conf->currentFloor) <= floor_conf->floorBelow) {				
					floor_conf->dataUpdate = 1;//楼层数据更新,ipc显示也要更新
				}else {
					floor_conf->currentFloor = floor_conf->lastFloor;
					//电梯楼层超出设置范围,重新设置当前楼层
					set_int_count(floor_conf->currentFloor);
				}
			}else {
				if(floor_conf->currentFloor <= floor_conf->floorAbove) {
					floor_conf->dataUpdate = 1;
				}else {
					floor_conf->currentFloor = floor_conf->lastFloor;
					set_int_count(floor_conf->currentFloor);
				}
			}
			floor_conf->lastFloor = floor_conf->currentFloor;
			floor_conf->lastFloorStatus = floor_conf->floorStatus;
			//printf("get count=%d status=%d\n",floor_conf->currentFloor,floor_conf->floorStatus);
		}
		usleep(50*1000);//50ms
	}
}
//再开启一个线程,判断复位按键以及恢复出厂设置按键的检测
void *read_key_thread(void) {
	while(1) {
		//获取按键值,等于0即按键按下，计数按键次数
		if(key_press() == 0) {
			key_count++;
			//printf("key_count=%d\n",key_count);
		}else {
			//按键长按8S,恢复出厂设置
			if(key_count > 160) {
				//长按5S恢复出厂设置
				recovery = 1;
				key_count = 0;
				break;
			}else if(key_count > 60) {
				//按键长按3S,重启设备
				reboot = 1;
				key_count = 0;
				break;
			}
			key_count = 0;
		}
		usleep(50*1000);//50ms
	}
	//恢复出厂设置
	if(recovery) {
		printf("go to recovery sysytem\n");
		system("jffs2reset -y");
		system("reboot");
	}
	//重启
	if(reboot) {
		printf("go to reboot sysytem\n");
		system("reboot");
	}
}

int main()
{
	int ret,i;
	pthread_t read_int_id = NULL;
	pthread_t read_key_id = NULL;
	if(Debug)
		printf( "page size=%d\n",getpagesize(  ) );
	//开机-根据配置文件是否初始化设备
	first_run();
	//开启两个线程,一个检测电梯一个检测按键
	ret = pthread_create(&read_int_id, NULL, (void*)read_int_thread, NULL);
    if(ret){
        printf("Create int pthread error!/n");
        //return 1;
    }
    ret = pthread_create(&read_key_id, NULL, (void*)read_key_thread, NULL);
    if(ret){
        printf("Create key pthread error!/n");
    }

	//进入循环
	while(1) {
		if(ipc_conf->useful == 1) {
			ipc_conf->useful = 0;
			ipc_conf->dataUpdate = 1;//ipc数据更新标志
			//解析并更新配置
			parse_ipc_conf();
			//如果楼显打开
			if(ipc_conf->floorEnable == 1) {
				//解析并更新配置
				parse_floor_conf();
				//设置底层楼层计数
				//set_int_count(floor_conf->currentFloor);
				//清除显示
				if((i = findIpcVersionIndex()) != -1) {
					ipcFmt->func(" ",50,50, 1,0);
					ipcFmt->func(" ",50,100,2,0);
					ipcFmt->func(" ",50,150,3,0);
					ipcFmt->func(" ",50,200,4,0);
				}
			}
			//如果pos显示打开
			if(ipc_conf->posEnable == 1) {
				//解析并更新配置
				parse_pos_conf();
				//清除显示
				if((i = findIpcVersionIndex()) != -1) 
					ipcFmt->func(" ",ipc_conf->xPosition,ipc_conf->yPosition,1,0);
				if((0 < ipc_conf->refresh) &&(ipc_conf->refresh < 10))
					delay = ipc_conf->refresh;//获取ipc字符刷新周期
				else 
					delay = 1;//默认为1S
				//没打开设备则打开设备,已经打开的则重新配置串口信息
				if(pos_conf->ttyFd < 0)
					pos_conf->ttyFd = open_tty(pos_conf->baudRate,pos_conf->dataBit,pos_conf->parity,pos_conf->stopBit);
				else
					tty_config(pos_conf->ttyFd,pos_conf->baudRate,pos_conf->dataBit,pos_conf->parity,pos_conf->stopBit);
			}else {
				//如果pos勾选框没勾选,判断是否打开了pos机相关设备,打开了则关闭
				if(pos_conf->ttyFd > 0) {
					close_tty(pos_conf->ttyFd);
					pos_conf->ttyFd = -1;				
				}
				if(pos_conf->usbFd > 0) {

				}
			}
		}
		//更新pos相关配置
		if(pos_conf->useful == 1) {
			pos_conf->useful = 0;//清零
			pos_conf->dataUpdate = 1;
			//解析并更新配置
			parse_pos_conf();
			//根据pos设置更新设备配置
			if(ipc_conf->posEnable == 1) {
				//uart interface
				if(pos_conf->interfaceType == 1) {
					if(pos_conf->ttyFd < 0)
						pos_conf->ttyFd = open_tty(pos_conf->baudRate,pos_conf->dataBit,pos_conf->parity,pos_conf->stopBit);
					else
						tty_config(pos_conf->ttyFd,pos_conf->baudRate,pos_conf->dataBit,pos_conf->parity,pos_conf->stopBit);
				}
				//usb interface
				if(pos_conf->interfaceType == 2) {
					//close uart interface
					if(pos_conf->ttyFd > 0) {
						close_tty(pos_conf->ttyFd);
						pos_conf->ttyFd = -1;				
					}
				}
			}else {
				if(pos_conf->ttyFd > 0) {
					close_tty(pos_conf->ttyFd);
					pos_conf->ttyFd = -1;				
				}
				if(pos_conf->usbFd > 0) {
					pos_conf->usbFd = -1;
				}
			}
		}
		do_main();
		//usleep(500*1000);
		//sleep(1);
	}
	//delete the share mem
	delete_shm();
	free(floor_conf);
	return 0;
}

int first_run(void) {
	//创建共享内存
	creat_shm();
	//初始化配置
	init_conf();
	parse_ipc_conf();
	parse_floor_conf();
	//设置底层楼层计数
	set_int_count(floor_conf->currentFloor);
	parse_pos_conf();
	//初始化设备
	init_dev();
	//串口初始化,看情况是否添加
	#ifdef UART_INIT
	system("reg s 0xb0000000");
	system("reg w 0x60 0x00583701");
	#endif
	return 0;
}

void do_main() {
	int i,ret;
	if(ipc_conf->floorEnable == 1) {
		//满足以下两个要求,更新ipc显示
		if((ipc_conf->dataUpdate == 1) || (floor_conf->dataUpdate == 1)) {
			if((i = findIpcVersionIndex()) != -1) {
				ipcFmt->func(ipc_conf->mess,ipc_conf->xPosition,ipc_conf->yPosition,1,1);
			}
			ipc_conf->dataUpdate = 0;
			floor_conf->dataUpdate = 0;
		}
	}else if(ipc_conf->posEnable == 1) {
		//不断获取串口数据
		ret = tty_recv(pos_conf->ttyFd,uart_temp,MAX);
		if(ret > 0) {
			printf("get uart data len:%d\n",ret);
			usleep_count = 0;
			//
			if(Debug) {
				for(i=0;i<ret;i++)
					printf("%x ",uart_buf[i]&0xff);
				printf("\n");
			}
			//接收到串口数据拷貝到緩存區
			for(i=0;i<ret;i++) {
				uart_buf[uart_len] = uart_temp[i];
				uart_len++;
			}
		}else {
			//一次性接收打印机串口数据完毕
			//串口数据长度大于0,判断串口接收到数据,将全部数据进行解析发送
			if(uart_len > 0) {			
				//解析串口数据并发送
				printf("uart data recv over,len=%d\n",uart_len);
				get_printer_data(uart_buf,out_buf,uart_len-1);
				if((i = findPosVersionIndex()) != -1) {
					//将解析出来的串口数据out_buf显示到ipc上
					posFmt->func(out_buf,4,delay);
				}
				//清除缓存
				memset(out_buf,0,sizeof(out_buf));
				memset(uart_buf,0,sizeof(uart_buf));
				uart_len = 0;
				usleep_count = 0;
				uart_send_flag = 1;
			}
		}
		//延时,让串口缓存数据
		usleep(200*1000);
		usleep_count++;
		//POS机显示2S后清屏
		if((usleep_count > 30) && (uart_send_flag == 1)) {
			uart_send_flag = 0;
			if((i = findIpcVersionIndex()) != -1) {
				ipcFmt->func(" ",50,50, 1,0);
				ipcFmt->func(" ",50,100,2,0);
				ipcFmt->func(" ",50,150,3,0);
				ipcFmt->func(" ",50,200,4,0);
			}
		}
	}else {
		
	}
}
