#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/shm.h>

#include "shmdata.h"
#include "conf_op.h"
#include "dev_op.h"

void init_dev(void) {
	if(ipc_conf->posEnable == 1) {
		switch(pos_conf->interfaceType) {
			case 1://使用串口接口
			pos_conf->ttyFd = open_tty(pos_conf->baudRate,pos_conf->dataBit,pos_conf->parity,pos_conf->stopBit);
			break;//使用usb接口
			
			case 2:
			break;
			default:
			pos_conf->usbFd = -1;
			pos_conf->ttyFd = -1;
			break;
		}
	}
}
//初始化设备
/********************************串口相关操作函数************************************************/
int open_tty(int baud,int databit,int parity,int stopbit) {
	int fd;
	
	fd = open(TTY_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd < 0)
		return -1;
	
	tty_config(fd,baud,databit,parity,stopbit);
	
	return fd;
}

int close_tty(int fd) {
	if(close(fd) < 0) 
		return -1;
	else
		return 0;
}

speed_t tty_getBaudrate(int baudrate) {
	switch(baudrate) {
		case 1:return B115200;
		case 2:return B57600;
		case 3:return B38400;
		case 4:return B19200;
		case 5:return B9600;
		case 6:return B4800;
		case 7:return B2400;
		default:return -1;
	}
}

int tty_config(int fd,int baud,int databit,int parity,int stopbit) {
	speed_t speed;  
	struct termios cfg;
	struct termios options;
	printf("fd:%d baud:%d %d %d %d\n",fd,baud,databit,parity,stopbit);
	speed = tty_getBaudrate(baud);
	if(speed == -1)
		return -2;
	
	tcgetattr(fd, &options);
	
	//cfsetispeed(&cfg, speed);
    //cfsetospeed(&cfg, speed);
	
	//修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;
	//不使用流控制
	options.c_cflag &= ~CRTSCTS;
	//设置数据位
    options.c_cflag &= ~CSIZE; //屏蔽其他标志位
    switch (databit) {
		case 5    :
        options.c_cflag |= CS5;
        break;
		case 6    :
        options.c_cflag |= CS6;
        break;
		case 7    :
        options.c_cflag |= CS7;
        break;
		case 8:
        options.c_cflag |= CS8;
        break;
		default:
        return -4;
    }
	//设置校验位
    switch (parity) {
		case 1://无校验
		options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
		break;
		case 2://Even-偶数校验
		options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
		break;
		case 3://Odd-奇数校验
		options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
		default:break;
	}
	// 设置停止位
    switch (stopbit) {
		case 1://1个停止位
		options.c_cflag &= ~CSTOPB;
        break;
		case 2://2个停止位
		options.c_cflag |= CSTOPB;
        break;
	}
	//修改输出模式，原始数据输出
    //options.c_oflag &= ~OPOST;
    //设置等待时间和最小接收字符
    //options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    //options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */
	//如果发生数据溢出，接收数据，但是不再读取
	options.c_oflag = 0; //输出模式  
    options.c_lflag = 0; //不激活终端模式
	cfsetospeed(&options, speed);//设置波特率 
    tcflush(fd,TCIFLUSH);
	//激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
        return -5;
    return 0;
}

int tty_send(int fd, char *data, int datalen)  
{  
    int len = 0;  
    len = write(fd, data, datalen);//实际写入的长度  
    if(len == datalen) {  
        return len;  
    } else {  
        tcflush(fd, TCOFLUSH);//TCOFLUSH刷新写入的数据但不传送  
        return -1;  
    }       
    return 0;  
}  

int tty_recv(int fd, char *data, int datalen)  
{  
    int len=0;         
    len = read(fd, data, datalen);   
    return len;  
}  
/*************************************电梯中断操作*************************************/
userspace_data user_data;
int get_int_count(void) {
	int fd;
	fd = open(INT_DEV, O_RDWR);
	if(fd < 0)
		return 0;
	int err = ioctl(fd, USERSPACE_RD, &user_data);
	if (err<0) {
        printf("USERSPACE_RD failed\n");
		return 0;
    }
	floor_conf->floorStatus = user_data.status;
	close(fd);
	return user_data.get_count;
}

int set_int_count(int data) {
	int fd;
	fd = open(INT_DEV, O_RDWR);
	if(fd < 0)
		return -1;
	user_data.set_count = data;
	int err = ioctl(fd, USERSPACE_WR, &user_data);
	if (err<0) {
        printf("USERSPACE_WR failed\n");
		return -2;
    }
	close(fd);
	return 0;
}
/*****************************************USB接口*************************************/

/*****************************************按键检测*************************************/
static int key_fd = -1;
int key_press(void) {
    int data;
    key_fd = open(KEY_DEV,O_RDWR | O_NONBLOCK);
    if(key_fd < 0)
        return 1;
    read(key_fd,&data,sizeof(data));
    close(key_fd);
    return data;
}
