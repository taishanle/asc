/*========================================*\
    文件 : msg.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct sascMessage
{
	long index;
	char com;
	char src;
	char dst;
	char type;
	char func;
	char size[2];
	char data[1024];
};

int main(int argc,char *argv[])
{
	int result;

	int mid;
	mid=msgget(atoi(argv[1]),0);
	if(mid==-1)
	{
		printf("msgget failed!\n");
		return -1;
	}

	/*
	struct msqid_ds status;
	result=msgctl(mid,IPC_STAT,&status);
	if(result==-1)
	{
		printf("msgctl failed!\n");
		return -1;
	}
	printf("%d\n",status.msg_qbytes);
	printf("%d\n",status.msg_qnum);
	printf("%ld\n",status.__msg_cbytes);
	status.msg_qbytes=65536;
	result=msgctl(mid,IPC_SET,&status);
	if(result==-1)
	{
		printf("msgctl failed!\n");
		return -1;
	}
	*/

	struct sascMessage msg;
	bzero(&msg,sizeof(msg));
	result=msgrcv(mid,&msg,sizeof(struct sascMessage)-sizeof(long),0,0);
	if(result==-1)
	{
		printf("msgsnd failed!\n");
		return -1;
	}
	printf("%02x\n",msg.com);
	printf("%02x\n",msg.src);
	printf("%02x\n",msg.dst);
	printf("%02x\n",msg.type);
	printf("%02x\n",msg.func);
	printf("%d\n",*(short*)msg.size);
	int m;
	for(m=0;m<*(short*)msg.size;m++)
		printf("%02X",msg.data[m]);
	printf("\n");

	return 0;
}
