#ifndef _IPC_OP_H_HEADER  
#define _IPC_OP_H_HEADER  

int findIpcVersionIndex(void);

typedef struct _IpcFmt{
	char productName[32];
	char ipcVersion[32];
	unsigned int chNumber;
	int (*func)(char *disp,int x,int y,int id,int disp_floor);
}IpcFmt;

typedef struct _PosFmt{
	char productName[32];
	char ipcVersion[32];
	unsigned int chNumber;
	int (*func)(char *inbuf,int line,int delay);
}PosFmt;

extern IpcFmt ipcFmt[];
extern PosFmt posFmt[];

#endif
