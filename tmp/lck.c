/*========================================*\
    文件 : lck.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int vascShmLid;
int vascIniLid;

int fascLckInit(char *object)
{
	int result;

	if(strcmp(object,"shm")==0)
	{
		vascShmLid=open("../run/ascshm.lck",O_CREAT|O_RDWR,0600);
		if(vascShmLid==-1)
		{
			printf("open failed!\n");
			return -1;
		}
	}
	else
	if(strcmp(object,"ini")==0)
	{
		vascIniLid=open("../run/ascini.lck",O_CREAT|O_RDWR,0600);
		if(vascIniLid==-1)
		{
			printf("open failed!\n");
			return -1;
		}
	}

	return 0;
}

int fascLckLck(char *object,int flag)
{
	int result;

	int lid;
	if(strcmp(object,"shm")==0)
		lid=vascShmLid;
	else
	if(strcmp(object,"ini")==0)
		lid=vascIniLid;
	struct flock lock;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_WRLCK;
	if(flag==0)
	{
		result=fcntl(lid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{
			printf("fcntl failed!\n");
			return -1;
		}
		if(result==-1&&errno==EAGAIN)
			return 100;
	}
	else
	if(flag==1)
	{
		result=fcntl(lid,F_SETLKW,&lock);
		if(result==-1)
		{
			printf("fcntl failed!\n");
			return -1;
		}
	}

	return 0;
}

int fascLckUck(char *object,int flag)
{
	int result;

	int lid;
	if(strcmp(object,"shm")==0)
		lid=vascShmLid;
	else
	if(strcmp(object,"ini")==0)
		lid=vascIniLid;
	struct flock lock;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_UNLCK;
	if(flag==0)
	{
		result=fcntl(lid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{
			printf("fcntl failed!\n");
			return -1;
		}
		if(result==-1&&errno==EAGAIN)
			return 100;
	}
	else
	if(flag==1)
	{
		result=fcntl(lid,F_SETLKW,&lock);
		if(result==-1)
		{
			printf("fcntl failed!\n");
			return -1;
		}
	}

	return 0;
}

int main(void)
{
	int result;

	fascLckInit("shm");

	int pid;
	pid=fork();
	if(pid==-1)
	{
		printf("fork failed!\n");
		return -1;
	}
	else
	if(pid>0)
	{
		result=fascLckLck("shm",0);
		if(result==100)
			printf("father no lock\n");
		//fascLckLck("shm",1);
		//printf("father lck\n");
		sleep(3);
		fascLckUck("shm",0);
		printf("father uck\n");
	}
	else
	if(pid==0)
	{
		result=fascLckLck("shm",0);
		if(result==100)
			printf("son no lock\n");
		//fascLckLck("shm",1);
		//printf("son lck\n");
		sleep(3);
		fascLckUck("shm",0);
		printf("son uck\n");
	}

	return 0;
}
