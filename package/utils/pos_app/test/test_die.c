#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#define MAXLEN 8*1024

char hkAddStr[1024] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<TextOverlay version=\"2.0\" xmlns=\"http://www.hikvision.com/ver20/XMLSchema\">\r\n";
char ip[16] = "192.168.0.169";

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
static int hk_ipc_str(void) {
	char sendData[MAXLEN],diejiastr[1024],strtemp[100];
	CURL *curl;  
    CURLcode res;  
    struct curl_slist   *header = NULL;  
	//url 是curl访问的地址，这个192.168.1.69 是需要根据设置摄像机的地址，进行修改      
    char url[100]; //"http://192.168.1.69//ISAPI/System/Video/inputs/channels/1/overlays/text/1";  
	memset(url,0,sizeof(url));
	sprintf(url,"http://%s//ISAPI/System/Video/inputs/channels/1/overlays/text/1",ip);
	
    drp_upload_ctx *upload_ctx = (drp_upload_ctx *) malloc(sizeof(drp_upload_ctx));  
    if (upload_ctx == NULL) {  
    return 1;  
    }  
      
	memset(strtemp,0,sizeof(strtemp));
	memset(diejiastr,0,sizeof(diejiastr));
	//设置显示通道
	strcat(diejiastr,hkAddStr);
	sprintf(strtemp,"<id>%d</id>\r\n",1);
	strcat(diejiastr,strtemp);
	strcat(diejiastr,"<enabled>true</enabled>\r\n");
	//设置字符坐标
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"<positionX>%d</positionX>\r\n",100);
	strcat(diejiastr,strtemp);
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"<positionY>%d</positionY>\r\n",100);
	strcat(diejiastr,strtemp);
	//设置显示字符
	char disp[10] = "TEXT007";
	strcat(diejiastr,"<displayText>");
	strcat(diejiastr,disp);//显示内容
	strcat(diejiastr,"</displayText>\r\n");
	strcat(diejiastr,"</TextOverlay>\r\n");
	
    upload_ctx->data = (char *)diejiastr;  
    upload_ctx->pos = (char *)diejiastr;  
    upload_ctx->last = upload_ctx->pos + strlen(diejiastr);  
    curl_global_init(CURL_GLOBAL_ALL);  
    curl = curl_easy_init();  
	header = curl_slist_append(header, "PUT /ISAPI/System/Video/inputs/channels/1/overlays/text/1 HTTP/1.1");
	
	//Host是代表接收信息的IP，这个192.168.1.69 是需要根据设置摄像机的地址，进行修改	
	memset(strtemp,0,sizeof(strtemp));
	sprintf(strtemp,"Host: %s",ip);
	//header = curl_slist_append(header, "Host: 192.168.1.69");
	header = curl_slist_append(header, strtemp);
	header = curl_slist_append(header, "Content-Type:text/xml");
		if(header == NULL) {
			fprintf(stderr, "Header append() failed\n");
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			getchar();
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
	//sprintf(strtemp,"%s:%s",ipc_conf->userName,ipc_conf->passwd);
	curl_easy_setopt(curl, CURLOPT_USERPWD, "admin:admin12345");
	curl_easy_setopt(curl, CURLOPT_USERPWD, strtemp);

    res = curl_easy_perform(curl);  
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

void main(void) {
	hk_ipc_str();
}
