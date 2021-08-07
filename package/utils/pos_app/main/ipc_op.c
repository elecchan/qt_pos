#include <stdio.h>

#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <curl/curl.h>

#include "ipc_op.h"
#include "dev_op.h"
#include "shmdata.h"
#include "base64.h"
#include "parse_printer.h"

#define MAXLEN 8*1024
#define Debug 0

extern int iconv_err;
static int hk_ipc_str(char *disp,int x,int y,int id,int disp_floor);
static int hk_ipc_str_old(char *disp,int x,int y,int id,int disp_floor);
static int hk_pos_str(char *inbuf,int line,int delay);
static int uni_ipc_str(char *disp,int x,int y,int id,int disp_floor);
static int uni_pos_str(char *inbuf,int line,int delay);
static int uni_ipc_test(char *disp,int x,int y,int id,int disp_floor);

char hkAddStr[1024] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<TextOverlay version=\"2.0\" xmlns=\"http://www.hikvision.com/ver20/XMLSchema\">\r\n";

IpcFmt ipcFmt[] = {
	// product        version        ch    func
	{"hikvision", "Default",    	  4, hk_ipc_str},
	{"hikvision", "DS-2CD3320D-I",    4, hk_ipc_str},
	{"hikvision", "DS-2CC52D5S-IT3",  4, hk_ipc_str},
	{"uniview",   "Default",    	  4, uni_ipc_str},
	{"uniview",   "IPC-S322-IR",      4, uni_ipc_str},
};

PosFmt posFmt[] = {
	// product        version        ch    func
	{"hikvision", "Default",    	  4, hk_pos_str},
	{"hikvision", "DS-2CD3320D-I",    4, hk_pos_str},
	{"hikvision", "DS-2CC52D5S-IT3",  4, hk_pos_str},
	{"uniview",   "Default",    	  4, uni_pos_str},
	{"uniview",   "IPC-S322-IR",      4, uni_pos_str},
};

int findIpcVersionIndex(void)
{
	unsigned int i;
	for(i=0;i<sizeof(ipcFmt)/sizeof(IpcFmt);i++){
		if((!strcmp(ipcFmt[i].ipcVersion,ipc_conf->version))&&(!strcmp(ipcFmt[i].productName,ipc_conf->productName)))
			return i;
 	}
	return -1;
}

int findPosVersionIndex(void)
{
	unsigned int i;
	for(i=0;i<sizeof(posFmt)/sizeof(PosFmt);i++){
		if((!strcmp(posFmt[i].ipcVersion,ipc_conf->version))&&(!strcmp(posFmt[i].productName,ipc_conf->productName)))
			return i;
 	}
	return -1;
}

//中文上下utf8编码
char up_utf8[3] = {0xE4,0xB8,0x8A};//上
char down_utf8[3] = {0xE4,0xB8,0x8B};//下
char pause_utf8[3] = {0x00E5,0x0081,0x009C};//停
char exception_utf8[6] = {0xE5,0xBC,0x82,0xE5,0xB8,0xB8};//異常
//中文转码出错,请检查软件设置以及设备连接
char iconv_err_utf8[58] = {0xE4,0xB8,0xAD,0xE6,0x96,0x87,0xE8,0xBD,0xAC,0xE7,0xA0,0x81,0xE5,0x87,0xBA,0xE9,0x94,0x99,0x2C,0xE8,0xAF,0xB7,0xE6,0xA3,0x80,0xE6,0x9F,0xA5,0xE8,0xBD,0xAF,0xE4,0xBB,0xB6,0xE8,0xAE,0xBE,0xE7,0xBD,0xAE,0xE4,0xBB,0xA5,0xE5,0x8F,0x8A,0xE8,0xAE,0xBE,0xE5,0xA4,0x87,0xE8,0xBF,0x9E,0xE6,0x8E,0xA5};
static int hk_ipc_str_old(char *disp,int x,int y,int id,int disp_floor) {
    int sockfd, ret, i, h;
    struct sockaddr_in servaddr;
    char sendData[4096],strtemp2[1024],strtemp[100],base64Str[50];
    int len;
 
    //获取系统时间并打印,测试用
   	struct timeval tv;
    struct timezone tz;
    gettimeofday (&tv , &tz);
    printf("system time start:%d %d\n",tv.tv_sec,tv.tv_usec);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            printf("socket error!\n");
            return -1;
    };
 
    memset(&servaddr, '\0',sizeof(servaddr));
    servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ipc_conf->ipAddr);
    servaddr.sin_port = htons(ipc_conf->port);
 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
            printf("connect error!\n");
            return -2;
    }
    printf("connect OK\n");
 	
	memset(sendData,0,sizeof(sendData));
	memset(strtemp,0,sizeof(strtemp));
	memset(strtemp2,0,sizeof(strtemp2));
	//设置显示通道
	strcat(strtemp2,hkAddStr);
	sprintf(strtemp,"<id>%d</id>\r\n",id);
	strcat(strtemp2,strtemp);
	strcat(strtemp2,"<enabled>true</enabled>\r\n");
	//设置字符坐标
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"<positionX>%d</positionX>\r\n",x);
	strcat(strtemp2,strtemp);
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"<positionY>%d</positionY>\r\n",y);
	strcat(strtemp2,strtemp);
	//设置显示字符
	strcat(strtemp2,"<displayText>");
	strcat(strtemp2,disp);//显示内容
	//获取电梯楼层信息
	if(disp_floor == 1) {
        if(floor_conf->floorCount > 0) {
            memset(strtemp,0,sizeof(strtemp));
            //if((floor_conf->currentFloor <= floor_conf->floorBelow))
            sprintf(strtemp," %dF",floor_conf->currentFloor);
            if(floor_conf->floorStatus == UP)
                strcat(strtemp,up_utf8);
            if(floor_conf->floorStatus == DOWN)
                strcat(strtemp,down_utf8);
            if(floor_conf->floorStatus == PAUSE)
                strcat(strtemp,pause_utf8);
            if(floor_conf->floorStatus == EXCEPTION)
                strcat(strtemp,exception_utf8);
            strcat(strtemp2,strtemp);
        }
    }
	strcat(strtemp2,"</displayText>\r\n");
	strcat(strtemp2,"</TextOverlay>\r\n");
	
	memset(strtemp,0,sizeof(strtemp));
    memset(sendData, 0, sizeof(sendData));
    strcat(sendData, "PUT /ISAPI/System/Video/inputs/channels/1/overlays/text/1 HTTP/1.1\r\n");
	//设置IPC地址
	sprintf(strtemp,"Host: %s\r\n",ipc_conf->ipAddr);
	strcat(sendData,strtemp);
	//设置IPC的账号密码base64加密值
	memset(base64Str,0,sizeof(base64Str));
	strcat(base64Str,ipc_conf->userName);
	strcat(base64Str,":");
	strcat(base64Str,ipc_conf->passwd);
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"Authorization: Basic %s\r\n",base64_encode(base64Str));//账号密码base64加密
	strcat(sendData, strtemp);
    strcat(sendData, "Content-Type:text/xml\r\n");
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"Content-Length:%d\r\n",strlen(strtemp2));
    strcat(sendData, strtemp);
    strcat(sendData, "\r\n");
    strcat(sendData, strtemp2);
	//printf("%s \n",sendData);

	ret = send(sockfd,sendData,strlen(sendData),0);

    if (ret < 0) {
            printf("err code%d，mess:%s\n",errno, strerror(errno));
            return -3;
    }else{
            printf("send %d byte data!\n", ret);
    }
 
    close(sockfd);
    //打印系统时间
    if(Debug) {
    	gettimeofday (&tv , &tz);
    	printf("system time end:%d %d\n",tv.tv_sec,tv.tv_usec);
    }

    return 0;
}
//////////////////////////////////////////////////////////////////////////////////
typedef struct {  
    char *data;  
    char *pos;  
    char *last;  
} drp_upload_ctx;  

static int read_callback(void *ptr, int size, int nmemb, void *stream)  
{  
    drp_upload_ctx *ctx = (drp_upload_ctx *) stream;  
    int len = 0;  
    if (ctx->pos >= ctx->last) {  
        return 0;  
    }    
    if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {  
        return 0;  
    }     
    len = ctx->last - ctx->pos;  
    if (len > size*nmemb) {  
        len = size * nmemb;  
    }        
    memcpy(ptr, ctx->pos, len);  
    ctx->pos += len;     
    printf("send len=%zu\n", len);     
    return len;  
}  

static int hk_ipc_str(char *disp,int x,int y,int id,int disp_floor) {
	printf("=======hk_ipc_str=======\n");
	char sendData[MAXLEN],diejiastr[1024],strtemp[100];
    int i;
	CURL *curl;  
    CURLcode res;  
    struct curl_slist   *header = NULL;  
	//url 是curl访问的地址，这个192.168.1.69 是需要根据设置摄像机的地址，进行修改      
    char url[100]; //"http://192.168.1.69//ISAPI/System/Video/inputs/channels/1/overlays/text/1";  
	memset(url,0,sizeof(url));
	sprintf(url,"http://%s//ISAPI/System/Video/inputs/channels/1/overlays/text/1",ipc_conf->ipAddr);
	//获取系统时间并打印,测试用
	struct timeval tv;
	struct timezone tz;

    drp_upload_ctx *upload_ctx = (drp_upload_ctx *) malloc(sizeof(drp_upload_ctx));  
    if (upload_ctx == NULL) {  
    return 1;  
    }  
      
	memset(strtemp,0,sizeof(strtemp));
	memset(diejiastr,0,sizeof(diejiastr));
	//设置显示通道
	strcat(diejiastr,hkAddStr);
	sprintf(strtemp,"<id>%d</id>\r\n",id);
	strcat(diejiastr,strtemp);
	strcat(diejiastr,"<enabled>true</enabled>\r\n");
	//设置字符坐标
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"<positionX>%d</positionX>\r\n",x);
	strcat(diejiastr,strtemp);
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"<positionY>%d</positionY>\r\n",y);
	strcat(diejiastr,strtemp);
	//设置显示字符
	strcat(diejiastr,"<displayText>");
	strcat(diejiastr,disp);//显示内容
	//获取电梯楼层信息
	if(disp_floor == 1) {
		//总的楼层数大于0才显示
		if(floor_conf->floorCount > 0) { 
			memset(strtemp,0,sizeof(strtemp));
            //判断是否是不显示楼层
            if(!it_is_disp_floor(floor_conf->currentFloor)) {
                //判断是否重命名
                if((i = it_is_change_name(floor_conf->currentFloor)) != -1) {
                    sprintf(strtemp," ");
                    strcat(strtemp,floor_conf->changeName[i].name);
                }else {
                	//判断是否统一改名
                	if(floor_conf->reName[0] != 0) {
                		sprintf(strtemp," %d",floor_conf->currentFloor);
                		strcat(strtemp,floor_conf->reName);
                	}else
			       		sprintf(strtemp," %dF",floor_conf->currentFloor);
                }
                floor_conf->dispLastFloor = floor_conf->currentFloor;
            }else {
            	//如果是不显示楼层就显示上次有显示的楼层
            	//如果是不显示楼层是当前楼层则先刷新显示开始楼层
            	if(floor_conf->dispLastFloor == 0) {
            		//判断是否统一改名
                	if(floor_conf->reName[0] != 0) {
                		sprintf(strtemp," %d",floor_conf->startFloor);
                		strcat(strtemp,floor_conf->reName);
                	}else
			       		sprintf(strtemp," %dF",floor_conf->startFloor);
            	}else if((i = it_is_change_name(floor_conf->dispLastFloor)) != -1) {
                    sprintf(strtemp," ");
                    strcat(strtemp,floor_conf->changeName[i].name);
                }else {
                    //判断是否统一改名
                	if(floor_conf->reName[0] != 0) {
                		sprintf(strtemp," %d",floor_conf->dispLastFloor);
                		strcat(strtemp,floor_conf->reName);
                	}else
			       		sprintf(strtemp," %dF",floor_conf->dispLastFloor);
                }
            }
			if(floor_conf->floorStatus == UP)
				strcat(strtemp,up_utf8);
			if(floor_conf->floorStatus == DOWN)
				strcat(strtemp,down_utf8);
			if(floor_conf->floorStatus == PAUSE)
				strcat(strtemp,pause_utf8);
			if(floor_conf->floorStatus == EXCEPTION)
				strcat(strtemp,exception_utf8);
			strcat(diejiastr,strtemp);
		}
	}
	strcat(diejiastr,"</displayText>\r\n");
	strcat(diejiastr,"</TextOverlay>\r\n");
	
    upload_ctx->data = (char *)diejiastr;  
    upload_ctx->pos = (char *)diejiastr;  
    upload_ctx->last = upload_ctx->pos + strlen(diejiastr);  
    curl_global_init(CURL_GLOBAL_ALL);  
    curl = curl_easy_init();  
    //添加"Except:"加快POST请求速度
    header = curl_slist_append(header, "Expect:");
	header = curl_slist_append(header, "PUT /ISAPI/System/Video/inputs/channels/1/overlays/text/1 HTTP/1.1");
	
	//Host是代表接收信息的IP，这个192.168.1.69 是需要根据设置摄像机的地址，进行修改	
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"Host: %s",ipc_conf->ipAddr);
	//header = curl_slist_append(header, "Host: 192.168.1.69");
	header = curl_slist_append(header, strtemp);
	header = curl_slist_append(header, "Content-Type:text/xml");
		if(header == NULL) {
			fprintf(stderr, "Header append() failed\n");
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			//getchar();
			return -1;
	}

    if (curl) {  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);  
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);  
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);  
    curl_easy_setopt(curl, CURLOPT_URL, url);  
    curl_easy_setopt(curl, CURLOPT_READDATA, upload_ctx);  
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,  
                        (curl_off_t)(upload_ctx->last - upload_ctx->pos));  
      
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); 
    
    if(header)
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);  

	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_DIGEST);
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"%s:%s",ipc_conf->userName,ipc_conf->passwd);
	//curl_easy_setopt(curl, CURLOPT_USERPWD, "admin:a12345678");
	curl_easy_setopt(curl, CURLOPT_USERPWD, strtemp);

	//打印时间
	if(Debug) {
    	gettimeofday (&tv , &tz);
    	printf("system time curl_easy_perform start:%d %d\n",tv.tv_sec,tv.tv_usec);
    }

    res = curl_easy_perform(curl);  
    //打印时间
    if(Debug) {
    	gettimeofday (&tv , &tz);
    	printf("system time curl_easy_perform end:%d %d\n",tv.tv_sec,tv.tv_usec);
    }

    if (res != CURLE_OK)  
        fprintf(stderr, "set failed: %s\n",curl_easy_strerror(res));        
        curl_easy_cleanup(curl);  
    } 
	else
		printf("set ok\n");      
    curl_global_cleanup();    
    if(header)
		curl_slist_free_all(header); 

    free(upload_ctx);    

    return 0;  
}
//////////////////////////////////////////////////////////////////////////////
static int hk_pos_str(char *inbuf,int line,int delay) {
	int i,j = 0,k = 0,line_c = 0;
	char buf[1024];
	char utf8_buf[2*1024];
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
            //出现转码错误停止发送数据
            if(iconv_err == 1) {
                iconv_err = 0;
                //发送数据异常提示
                hk_ipc_str(iconv_err_utf8,ipc_conf->xPosition,ipc_conf->yPosition,1,0);
                goto hk_pos_err;
            }
			printf("utf8:");
			for(k=0;k<strlen(utf8_buf);k++)
				printf("%x ",utf8_buf[k]&0xff);
			printf("\n");
			//把解析好的串口数据送到IPC显示
            if(strlen(utf8_buf) > 0)
			    hk_ipc_str(utf8_buf,ipc_conf->xPosition,ipc_conf->yPosition-line_c*50,line_c,0);
			//
			j = 0;	
			memset(buf,0,sizeof(buf));	
			memset(utf8_buf,0,sizeof(utf8_buf));
		}else {
			buf[j] = inbuf[i];
			j++;
			//解析到串口最后一行数据
			if(i == (strlen(inbuf) - 1)) {
				line_c++;
				for(k=0;k<strlen(buf);k++)
					printf("%x ",buf[k]&0xff);
				printf("\n");
				//change to utf8
				gb2312_to_utf8(buf,utf8_buf);
                if(iconv_err == 1) {
                    iconv_err = 0;
                    //发送数据异常提示
                    hk_ipc_str(iconv_err_utf8,ipc_conf->xPosition,ipc_conf->yPosition,1,0);
                    goto hk_pos_err;
                }
				printf("utf8:");
				for(k=0;k<strlen(utf8_buf);k++)
					printf("%x ",utf8_buf[k]&0xff);
				printf("\n");
				//把解析好的串口数据送到IPC显示
                if(strlen(utf8_buf) > 0)
				    hk_ipc_str(utf8_buf,ipc_conf->xPosition,ipc_conf->yPosition-line_c*50,line_c,0);
				//
				j = 0;	
				memset(buf,0,sizeof(buf));	
				memset(utf8_buf,0,sizeof(utf8_buf));
			}
		}
		if(line_c == line) {
			line_c = 0;
			sleep(1);
			//sleep(delay);
			//clear disp
			//hk_ipc_str(" ",50,50, 1,0);
			//hk_ipc_str(" ",50,100,2,0);
			//hk_ipc_str(" ",50,150,3,0);
			//hk_ipc_str(" ",50,200,4,0);
		}
	}
    hk_pos_err:
    return 0;
}
/*****************************************************************************************/
static int uni_ipc_test(char *disp,int x,int y,int id,int disp_floor) {
printf("=======uni_ipc_test=======\n");
	int sockfd, ret, i, h;
    struct sockaddr_in servaddr;
    char str1[4096], str2[4096], buf[100], *str;
	char strtemp[32];
	
    int len;
    fd_set   t_set1;
    struct timeval  tv;

	char ysudp_str[1024];


	//创建套接字
	int sockClient = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockClient < 0)
	{
		printf("socket() called failed! The error code is: %d\n", 1);
		return -1;
	}
	else
	{
		printf("socket() called succesful!\n");
	}
 
	memset(ysudp_str,'\0',1024);

//---------- 组包开始 -------------------
//ff 03 02 31 41 00 75
	ysudp_str[0] = 0xff;  //开头
	ysudp_str[1] = 0x03;  //长度
	ysudp_str[2] = 0x01;  //行数
	ysudp_str[3] = 0x31;  //内容
	ysudp_str[4] = 0x41;  //内容
	ysudp_str[5] = 0x00;  //包尾

	char checksum = 0x00; 
	for(i=0;i<6;i++)
		checksum = checksum + ysudp_str[i];

	ysudp_str[6] = checksum;  //校验位 ，累加和

//添加本次结束符  ff 00 ff

	ysudp_str[7] = 0xff;
	ysudp_str[8] = 0x00;
	ysudp_str[9] = 0xff;

//---------- 组包结束 -------------------

	struct sockaddr_in addrServer;
	addrServer.sin_addr.s_addr = inet_addr(ipc_conf->ipAddr);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(ipc_conf->port);
	
	//发送数据 ,注意这个发送字符串的长度，不能用strlen ，因为里面有 0x00,会截断 
	int err = sendto(sockClient, ysudp_str, 10, 0, (struct sockaddr*)&addrServer, sizeof(struct sockaddr));
	if (err < 0)
	{
		printf("sendto() called failed! The error code is: %s\n", 1);
		return -1;
	}
	else
	{
		printf("sendto() called successful!\n");
	}
 
	//关闭套接字
	close(sockClient);

    return 0;
}
static int uni_ipc_str(char *disp,int x,int y,int id,int disp_floor) {
	printf("=======uni_ipc_str=======\n");
	int sockfd, ret, i, h;
	int len = 0;
    struct sockaddr_in servaddr;
    char str1[1024]; 
	char strtemp[1024];
	char ysudp_str[1024];

	//创建套接字
	int sockClient = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockClient < 0)
	{
		printf("socket() called failed! The error code is: %d\n", 1);
		return -1;
	}
	else
	{
		printf("socket() called succesful!\n");
	}
 
	memset(str1,'\0',1024);
	memset(strtemp,'\0',1024);
	memset(ysudp_str,'\0',1024);
//---------------------------- 组包开始 -------------------
//ff 03 02 31 41 00 75
	ysudp_str[0] = 0xff;  //开头
	//ysudp_str[1] = strlen(disp);  //长度
	ysudp_str[2] = id;  //行数
	//ysudp_str[3] = 0x31;  //内容
	//ysudp_str[4] = 0x41;  //内容
	//ysudp_str[5] = 0x00;  
	strcat(str1,disp);//拷贝显示内容
	//获取电梯楼层信息
	if(disp_floor == 1) {
		//总的楼层数大于0才显示
		if(floor_conf->floorCount > 0) { 
			memset(strtemp,0,sizeof(strtemp));
            //判断是否是不显示楼层
            if(!it_is_disp_floor(floor_conf->currentFloor)) {
                //判断是否重命名
                if((i = it_is_change_name(floor_conf->currentFloor)) != -1) {
                    sprintf(strtemp," ");
                    strcat(strtemp,floor_conf->changeName[i].name);
                }else {
                	//判断是否统一改名
                	if(floor_conf->reName[0] != 0) {
                		sprintf(strtemp," %d",floor_conf->currentFloor);
                		strcat(strtemp,floor_conf->reName);
                	}else
			       		sprintf(strtemp," %dF",floor_conf->currentFloor);
                }
                floor_conf->dispLastFloor = floor_conf->currentFloor;
            }else {
            	//如果是不显示楼层就显示上次有显示的楼层
            	//如果是不显示楼层是当前楼层则先刷新显示开始楼层
            	if(floor_conf->dispLastFloor == 0) {
            		//判断是否统一改名
                	if(floor_conf->reName[0] != 0) {
                		sprintf(strtemp," %d",floor_conf->startFloor);
                		strcat(strtemp,floor_conf->reName);
                	}else
			       		sprintf(strtemp," %dF",floor_conf->startFloor);
            	}else if((i = it_is_change_name(floor_conf->dispLastFloor)) != -1) {
                    sprintf(strtemp," ");
                    strcat(strtemp,floor_conf->changeName[i].name);
                }else {
                    //判断是否统一改名
                	if(floor_conf->reName[0] != 0) {
                		sprintf(strtemp," %d",floor_conf->dispLastFloor);
                		strcat(strtemp,floor_conf->reName);
                	}else
			       		sprintf(strtemp," %dF",floor_conf->dispLastFloor);
                }
            }
			if(floor_conf->floorStatus == UP)
				strcat(strtemp,up_utf8);
			if(floor_conf->floorStatus == DOWN)
				strcat(strtemp,down_utf8);
			if(floor_conf->floorStatus == PAUSE)
				strcat(strtemp,pause_utf8);
			if(floor_conf->floorStatus == EXCEPTION)
				strcat(strtemp,exception_utf8);
			strcat(str1,strtemp);
		}
	}
	ysudp_str[1] = strlen(str1)+1;  //长度
	strcat(ysudp_str,str1);//拷贝全部显示内容
	len = 3 + strlen(str1);
	ysudp_str[len] = 0x00;//包尾
	char checksum = 0x00; 
	for(i=0;i<=len;i++)
		checksum = checksum + ysudp_str[i];
	ysudp_str[len+1] = checksum;  //校验位 ，累加和
	//添加本次结束符  ff 00 ff
	ysudp_str[len+2] = 0xff;
	ysudp_str[len+3] = 0x00;
	ysudp_str[len+4] = 0xff;
	len = len + 5;
	for(i=0;i<len;i++)
		printf("%x ",ysudp_str[i]);
	printf("\n");
//----------------------------- 组包结束 -------------------

	struct sockaddr_in addrServer;
	addrServer.sin_addr.s_addr = inet_addr(ipc_conf->ipAddr);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(ipc_conf->port);
	
	//发送数据 ,注意这个发送字符串的长度，不能用strlen ，因为里面有 0x00,会截断 
	int err = sendto(sockClient, ysudp_str, len, 0, (struct sockaddr*)&addrServer, sizeof(struct sockaddr));
	if (err < 0)
	{
		printf("sendto() called failed! The error code is: %s\n", 1);
		return -1;
	}
	else
	{
		printf("sendto() called successful!\n");
	}
 
	//关闭套接字
	close(sockClient);

    return 0;
}
static int uni_pos_str(char *inbuf,int line,int delay) {
	int i,j = 0,k = 0,line_c = 0;
	char buf[1024];
	char utf8_buf[2*1024];
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
            //出现转码错误停止发送数据
            if(iconv_err == 1) {
                iconv_err = 0;
                //发送数据异常提示
                uni_ipc_str(iconv_err_utf8,ipc_conf->xPosition,ipc_conf->yPosition,1,0);
                goto hk_pos_err;
            }
			printf("utf8:");
			for(k=0;k<strlen(utf8_buf);k++)
				printf("%x ",utf8_buf[k]&0xff);
			printf("\n");
			//把解析好的串口数据送到IPC显示
            if(strlen(utf8_buf) > 0)
			    uni_ipc_str(utf8_buf,ipc_conf->xPosition,ipc_conf->yPosition-line_c*50,line_c,0);
			//
			j = 0;	
			memset(buf,0,sizeof(buf));	
			memset(utf8_buf,0,sizeof(utf8_buf));
		}else {
			buf[j] = inbuf[i];
			j++;
			//解析到串口最后一行数据
			if(i == (strlen(inbuf) - 1)) {
				line_c++;
				for(k=0;k<strlen(buf);k++)
					printf("%x ",buf[k]&0xff);
				printf("\n");
				//change to utf8
				gb2312_to_utf8(buf,utf8_buf);
                if(iconv_err == 1) {
                    iconv_err = 0;
                    //发送数据异常提示
                    uni_ipc_str(iconv_err_utf8,ipc_conf->xPosition,ipc_conf->yPosition,1,0);
                    goto hk_pos_err;
                }
				printf("utf8:");
				for(k=0;k<strlen(utf8_buf);k++)
					printf("%x ",utf8_buf[k]&0xff);
				printf("\n");
				//把解析好的串口数据送到IPC显示
                if(strlen(utf8_buf) > 0)
				    uni_ipc_str(utf8_buf,ipc_conf->xPosition,ipc_conf->yPosition-line_c*50,line_c,0);
				//
				j = 0;	
				memset(buf,0,sizeof(buf));	
				memset(utf8_buf,0,sizeof(utf8_buf));
			}
		}
		if(line_c == line) {
			line_c = 0;
			sleep(1);
		}
	}
    hk_pos_err:
    return 0;
}
