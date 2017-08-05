/*========================================*\
    文件 : tcpsrv.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

	int lisid;
	lisid=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(lisid==-1)
	{
		//mascLogError("socket",errno,strerror(errno),"[]");
		fprintf(stderr,"socket failed!\n");
		return;
	}

	struct sockaddr_in lisaddress;
	bzero(&lisaddress,sizeof(lisaddress));
	lisaddress.sin_family=AF_INET;
	inet_pton(AF_INET,"0.0.0.0",&lisaddress.sin_addr);
	lisaddress.sin_port=htons(atoi(argv[1]));

	int option;
	option=1;
	result=setsockopt(lisid,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	if(result==-1)
	{
		close(lisid);
		lisid=0;
		//mascLogError("setsockopt",errno,strerror(errno),"[]");
		fprintf(stderr,"setsockopt failed!\n");
		return;
	}
	result=bind(lisid,(struct sockaddr*)&lisaddress,sizeof(struct sockaddr_in));
	if(result==-1)
	{
		close(lisid);
		lisid=0;
		//mascLogError("bind",errno,strerror(errno),"");
		fprintf(stderr,"bind failed!\n");
		return;
	}
	result=listen(lisid,1024);
	if(result==-1)
	{
		close(lisid);
		lisid=0;
		//mascLogError("listen",errno,strerror(errno),"[]");
		fprintf(stderr,"listen failed!\n");
		return;
	}

	while(1)
	{
		struct sockaddr_in conaddress;
		bzero(&conaddress,sizeof(conaddress));
		int size;
		size=sizeof(conaddress);
		int conid;
		conid=accept(lisid,(struct sockaddr*)&conaddress,&size);
		if(conid==-1)
		{
			//mascLogError("accept",errno,strerror(errno),"[]");
			fprintf(stderr,"accept failed!\n");
			return -1;
		}

		char data[1024];
		int length;

		int remain;
		int record;

		remain=sizeof(NET_TPS_HEAD);
		record=0;
		while(remain>0)
		{
			result=read(conid,data+record,remain);
			if(result==-1&&errno==EAGAIN)
				continue;
			if(result==-1&&errno!=EAGAIN)
			{
				//mascLogError("read",errno,strerror(errno),"[]");
				fprintf(stderr,"read failed!\n");
				return -1;
			}
			remain-=result;
			record+=result;
		}

		if((unsigned char)data[6]==0XB6)
		{
			if(*(int*)data!=sizeof(NET_TPS_REALTIME))
			{
				close(conid);
				continue;
			}
		}
		else
		if((unsigned char)data[6]==0XB7)
		{
			if(*(int*)data!=sizeof(NET_TPS_STATISTS))
			{
				close(conid);
				continue;
			}
		}

		remain=*(int*)data-sizeof(NET_TPS_HEAD);
		record=sizeof(NET_TPS_HEAD);
		while(remain>0)
		{
			result=read(conid,data+record,remain);
			if(result==-1&&errno==EAGAIN)
				continue;
			if(result==-1&&errno!=EAGAIN)
			{
				//mascLogError("read",errno,strerror(errno),"[]");
				fprintf(stderr,"read failed!\n");
				return -1;
			}
			remain-=result;
			record+=result;
		}

		if((unsigned char)data[6]==0XB6)
		{
			NET_TPS_REALTIME *real;
			real=(NET_TPS_REALTIME*)data;

			printf("开始:%02X\n",real->info.start);
			printf("指令:%02X\n",real->info.command);
			short id;
			*((char*)&id+0)=*((char*)&real->info.id+1);
			*((char*)&id+1)=*((char*)&real->info.id+0);
			printf("设备:%d\n",id);
			printf("车道:%d\n",real->info.lane);
			printf("速度:%d\n",real->info.speed);
			printf("状态:%d\n",real->info.status);
			printf("长度:%d\n",real->info.queue);
			printf("------------------------\n");
		}
		else
		if((unsigned char)data[6]==0XB7)
		{
			NET_TPS_STATISTS *stat;
			stat=(NET_TPS_STATISTS*)data;

			short id;
			*((char*)&id+0)=*((char*)&stat->info.id+1);
			*((char*)&id+1)=*((char*)&stat->info.id+0);
			printf("设备:%d\n",id);
			printf("总数:%d\n",stat->info.total);
			printf("开始:%s\n",stat->info.time);
			short period;
			*((char*)&period+0)=*((char*)&stat->info.period+1);
			*((char*)&period+1)=*((char*)&stat->info.period+0);
			printf("间隔:%d\n",period);
			int i;
			for(i=0;i<stat->info.total;i++)
			{
				printf("车道:%d",stat->info.param[i].lane);
				printf("速度:%d",stat->info.param[i].speed);
				int light;
				*((char*)&light+0)=*((char*)&stat->info.param[i].light+3);
				*((char*)&light+1)=*((char*)&stat->info.param[i].light+2);
				*((char*)&light+2)=*((char*)&stat->info.param[i].light+1);
				*((char*)&light+3)=*((char*)&stat->info.param[i].light+0);
				printf("小型:%d",light);
				int middle;
				*((char*)&middle+0)=*((char*)&stat->info.param[i].middle+3);
				*((char*)&middle+1)=*((char*)&stat->info.param[i].middle+2);
				*((char*)&middle+2)=*((char*)&stat->info.param[i].middle+1);
				*((char*)&middle+3)=*((char*)&stat->info.param[i].middle+0);
				printf("中型:%d",middle);
				int heavy;
				*((char*)&heavy+0)=*((char*)&stat->info.param[i].heavy+3);
				*((char*)&heavy+1)=*((char*)&stat->info.param[i].heavy+2);
				*((char*)&heavy+2)=*((char*)&stat->info.param[i].heavy+1);
				*((char*)&heavy+3)=*((char*)&stat->info.param[i].heavy+0);
				printf("大型:%d",heavy);
				int timedis;
				*((char*)&timedis+0)=*((char*)&stat->info.param[i].timedis+3);
				*((char*)&timedis+1)=*((char*)&stat->info.param[i].timedis+2);
				*((char*)&timedis+2)=*((char*)&stat->info.param[i].timedis+1);
				*((char*)&timedis+3)=*((char*)&stat->info.param[i].timedis+0);
				printf("时距:%d",timedis);
				int spacedis;
				*((char*)&spacedis+0)=*((char*)&stat->info.param[i].spacedis+3);
				*((char*)&spacedis+1)=*((char*)&stat->info.param[i].spacedis+2);
				*((char*)&spacedis+2)=*((char*)&stat->info.param[i].spacedis+1);
				*((char*)&spacedis+3)=*((char*)&stat->info.param[i].spacedis+0);
				printf("空距:%d",spacedis);
				int spacerat;
				*((char*)&spacerat+0)=*((char*)&stat->info.param[i].spacerat+1);
				*((char*)&spacerat+1)=*((char*)&stat->info.param[i].spacerat+0);
				printf("空占:%d",spacerat);
				int timerat;
				*((char*)&timerat+0)=*((char*)&stat->info.param[i].timerat+1);
				*((char*)&timerat+1)=*((char*)&stat->info.param[i].timerat+0);
				printf("时占:%d",timerat);
			}
			printf("------------------------\n");
		}

		close(conid);
	}

	close(lisid);
	return 0;
}
