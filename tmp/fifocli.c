/*========================================*\
    文件 : fifocli.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	int result;

	char sndpath[64];
	strcpy(sndpath,"./sndfifo");
	char rcvpath[64];
	strcpy(rcvpath,"./rcvfifo");

	int i;
	for(i=0;i<100;i++)
	{
		result=fork();
		if(result>0)
			continue;

		int sndid;
		sndid=open(sndpath,O_WRONLY);
		if(sndid==-1)
		{
			//muntLogError("open",errno,strerror(errno),"[%s]",sndpath);
			//fprintf(stderr,"open failed!\n");
			printf("open failed!\n");
			return -1;
		}
		char action=i;
		result=write(sndid,&action,sizeof(action));
		if(result==-1)
		{
			//muntLogError("read",errno,strerror(errno),"[]");
			//fprintf(stderr,"read failed!\n");
			printf("read failed!\n");
			close(sndid);
			return -1;
		}
		close(sndid);

		int rcvid;
		rcvid=open(rcvpath,O_RDONLY);
		if(rcvid==-1)
		{
			//muntLogError("open",errno,strerror(errno),"[%s]",rcvpath);
			//fprintf(stderr,"open failed!\n");
			printf("open failed!\n");
			return -1;
		}
		//int action;
		result=read(rcvid,&action,sizeof(action));
		if(result==-1)
		{
			//muntLogError("write",errno,strerror(errno),"[]");
			//fprintf(stderr,"write failed!\n");
			printf("write failed!\n");
			close(rcvid);
			return -1;
		}
		close(rcvid);
		printf("%d:%d\n",getpid(),action);

		exit(0);
	}

	return 0;
}
