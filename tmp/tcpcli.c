/*========================================*\
    文件 : tcpcli.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int vmodVehLisid;

typedef struct _NET_TPS_HEAD_
{
	int length;
	char reserve1[2];
	char type;
	char reserve2[133];
}NET_TPS_HEAD;

typedef struct _NET_TPS_REALTIME_INFO_
{
	char start;
	char command;
	char reserve1[2];
	short id;
	short length;
	char lane;
	char speed;
	char status;
	char queue;
	char reserve2[24];
}NET_TPS_REALTIME_INFO;
typedef struct _NET_TPS_REALTIME_
{
	NET_TPS_HEAD head;
	char time[12];
	NET_TPS_REALTIME_INFO info;
	char reserve[24];
}NET_TPS_REALTIME;

typedef struct _NET_TPS_LANE_PARAM_
{
	char lane;
	char speed;
	char reserve1[2];
	int light;
	int middle;
	int heavy;
	int timedis;
	int spacedis;
	short spacerat;
	short timerat;
	char reserve2[16];
}NET_TPS_LANE_PARAM;
typedef struct _NET_TPS_STATISTS_INFO_
{
	char start;
	char command;
	char reserve1[2];
	short id;
	short length;
	char total;
	char reserve2[15];
	char time[12];
	int period;
	NET_TPS_LANE_PARAM param[8];
}NET_TPS_STATISTS_INFO;
typedef struct _NET_TPS_STATISTS_
{
	NET_TPS_HEAD head;
	NET_TPS_STATISTS_INFO info;
	char reserve[128];
}NET_TPS_STATISTS;

int main(int argc,char *argv[])
{
	int result;

	result=fork();
	if(result>0)
		return 0;

	while(1)
	{
		usleep(10000);
		/*
		srand(time(NULL));
		if(rand()%2==0)
			continue;
		*/

		int conid;
		conid=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
		if(conid==-1)
		{
	//		mlogError("socket",errno,strerror(errno),"[]");
			return -1;
		}

		struct sockaddr_in conaddress;
		bzero(&conaddress,sizeof(conaddress));
		conaddress.sin_family=AF_INET;
		inet_pton(AF_INET,"127.0.0.1",&conaddress.sin_addr);
		conaddress.sin_port=htons(7200);

		result=connect(conid,(struct sockaddr*)&conaddress,sizeof(struct sockaddr_in));
		if(result==-1)
		{
	//		mlogError("connect",errno,strerror(errno),"[%s][%s]",vemuHost,vemuPort);
			return -1;
		}

		NET_TPS_REALTIME real;
		real.head.length=sizeof(NET_TPS_REALTIME);
		real.head.type=0XB6;

		/*
		short id=atoi(argv[1]);
		*((char*)&real.info.id+0)=*((char*)&id+1);
		*((char*)&real.info.id+1)=*((char*)&id+0);
		real.info.lane=atoi(argv[2]);
		*/

		/*
		srand(time(NULL));
		short id=rand()%4+36;
		*((char*)&real.info.id+0)=*((char*)&id+1);
		*((char*)&real.info.id+1)=*((char*)&id+0);
		real.info.lane=rand()%4+1;
		*/

		short id=36;
		*((char*)&real.info.id+0)=*((char*)&id+1);
		*((char*)&real.info.id+1)=*((char*)&id+0);
		real.info.lane=1;
		printf("%d,%d\n",id,real.info.lane);

		/*
		short id=i;
		*((char*)&real.info.id+0)=*((char*)&id+1);
		*((char*)&real.info.id+1)=*((char*)&id+0);
		real.info.lane=j;
		printf("%d,%d\n",i,j);
		*/

		result=write(conid,&real,sizeof(real));
		if(result==-1&&errno!=EAGAIN)
		{
	//		mlogError("write",errno,strerror(errno),"[]");
			close(conid);
			return -1;
		}

		close(conid);
	}

	return 0;
}
