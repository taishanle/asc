/*========================================*\
    文件 : fifosrv.c
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
	result=mkfifo(sndpath,0600);
	if(result==-1&&errno!=EEXIST)
	{
		//muntLogError("mkfifo",errno,strerror(errno),"[%s]",sndpath);
		fprintf(stderr,"mkfifo failed!\n");
		return -1;
	}
	if(result==-1&&errno==EEXIST)
	{
		unlink(sndpath);
		result=mkfifo(sndpath,0600);
		if(result==-1)
		{
			//muntLogError("mkfifo",errno,strerror(errno),"[%s]",sndpath);
			fprintf(stderr,"mkfifo failed!\n");
			return -1;
		}
	}

	char rcvpath[64];
	strcpy(rcvpath,"./rcvfifo");
	result=mkfifo(rcvpath,0600);
	if(result==-1&&errno!=EEXIST)
	{
		//muntLogError("mkfifo",errno,strerror(errno),"[%s]",rcvpath);
		fprintf(stderr,"mkfifo failed!\n");
		return -1;
	}
	if(result==-1&&errno==EEXIST)
	{
		unlink(rcvpath);
		result=mkfifo(rcvpath,0600);
		if(result==-1)
		{
			//muntLogError("mkfifo",errno,strerror(errno),"[%s]",rcvpath);
			fprintf(stderr,"mkfifo failed!\n");
			return -1;
		}
	}

	while(1)
	{
		int sndid;
		sndid=open(sndpath,O_RDONLY);
		if(sndid==-1)
		{
			//muntLogError("open",errno,strerror(errno),"[%s]",sndpath);
			fprintf(stderr,"open failed!\n");
			return -1;
		}
		char action;
		result=read(sndid,&action,sizeof(action));
		if(result==-1)
		{
			//muntLogError("read",errno,strerror(errno),"[]");
			fprintf(stderr,"read failed!\n");
			close(sndid);
			return -1;
		}
		close(sndid);
		printf("%d\n",action);

		int rcvid;
		rcvid=open(rcvpath,O_WRONLY);
		if(rcvid==-1)
		{
			//muntLogError("open",errno,strerror(errno),"[%s]",rcvpath);
			fprintf(stderr,"open failed!\n");
			return -1;
		}
		result=write(rcvid,&action,sizeof(action));
		if(result==-1)
		{
			//muntLogError("write",errno,strerror(errno),"[]");
			fprintf(stderr,"write failed!\n");
			close(rcvid);
			return -1;
		}
		close(rcvid);
	}

	return 0;
}
