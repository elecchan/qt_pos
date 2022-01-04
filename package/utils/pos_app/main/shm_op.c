#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>

#include "shmdata.h"
#include "shm_op.h"
#include "config.h"

//#define Debug 1

//分配的共享内存的原始首地址
void *shm_ipc = NULL;
void *shm_pos = NULL;
//共享内存标识符
int shmid_ipc;
int shmid_pos;

int creat_shm(void) 
{
	//1.创建2个共享内存
	shmid_ipc = shmget(SHM_IPC_ID, TEXT_SZ, 0666|IPC_CREAT);
	if(shmid_ipc == -1)
	{
		printf("ipc shmget failed\n");
		exit(EXIT_FAILURE);
	}
	shmid_pos = shmget(SHM_POS_ID, TEXT_SZ, 0666|IPC_CREAT);
	if(shmid_pos == -1)
	{
		printf("pos shmget failed\n");
		exit(EXIT_FAILURE);
	}
	//2.将共享内存连接到当前进程的地址空间
	shm_ipc = shmat(shmid_ipc, 0, 0);
	if(shm_ipc == (void*)-1)
	{
		printf("ipc shmat failed\n");
		exit(EXIT_FAILURE);
	}
	printf("ipc Memory attached at %X\n", (int)shm_ipc);
	shm_pos = shmat(shmid_pos, 0, 0);
	if(shm_pos == (void*)-1)
	{
		printf("pos shmat failed\n");
		exit(EXIT_FAILURE);
	}
	printf("pos Memory attached at %X\n", (int)shm_pos);
	//3.设置共享内存
	ipc_conf = (struct shared_ipc_conf*)shm_ipc;
	pos_conf = (struct shared_pos_conf*)shm_pos;
	
	return 0;
}

int delete_shm(void)
{
	//1.把共享内存从当前进程中分离
	if(shmdt(shm_ipc) == -1)
	{
		printf("shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	if(shmdt(shm_pos) == -1)
	{
		printf("shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	//2.删除共享内存
	if(shmctl(shmid_ipc, IPC_RMID, 0) == -1)
	{
		printf("shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	if(shmctl(shmid_pos, IPC_RMID, 0) == -1)
	{
		printf("shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
