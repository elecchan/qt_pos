#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>

#include "shmdata.h"

//分配的共享内存的原始首地址
void *shm_floor = NULL;
//共享内存标识符
int shmid_floor;

int main(int *argc,void *argv)
{
	//1.创建共享内存
	shmid_floor = shmget(SHM_FLOOR_ID, TEXT_SZ, 0666|IPC_CREAT);
	if(shmid_floor == -1)
	{
		fprintf(stderr, "floor shmget failed\n");
		exit(EXIT_FAILURE);
	}
	//2.将共享内存连接到当前进程的地址空间
	shm_floor = shmat(shmid_floor, 0, 0);
	if(shm_floor == (void*)-1)
	{
		fprintf(stderr, "floor shmat failed\n");
		exit(EXIT_FAILURE);
	}
	//3.设置共享内存
	floor_conf = (struct shared_floor_conf*)shm_floor;

	floor_conf->useful = 1;
	
	//4.把共享内存从当前进程中分离
	if(shmdt(shm_floor) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}