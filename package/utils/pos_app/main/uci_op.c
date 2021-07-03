#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int uci_get(const char * cmd, char *retmsg, int msg_len)
{
    FILE * fp;
    int res = -1;
    if (cmd == NULL || retmsg == NULL || msg_len < 0)
    {
        printf("Err: Fuc:system paramer invalid!\n");
        return 1;
    }
    if ((fp = popen(cmd, "rw") ) == NULL)
    {
        perror("popen");
        printf("Err: Fuc:popen error\n");
        return 2;
    }
    else
    {
        memset(retmsg, 0, msg_len);
        fread(retmsg, msg_len, 1,fp);
        if((res = pclose(fp)) == -1)
        {
            printf("Fuc:close popen file pointer fp error!\n");
            return 3;
        }
        
        retmsg[strlen(retmsg)-1] = '\0';
        return 0;
    }
}
