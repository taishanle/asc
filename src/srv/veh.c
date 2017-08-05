/*========================================*\
    文件 : veh.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sqlite3.h>

#include "unt.h"
#include "bsn.h"

//车道数据.
struct svehLane
{
	//设备序号.
	int device;
	//车道序号.
	int lane;
	//相位序号.
	int phase;
};

//车检数据.
struct svehBase
{
	//车道总数.
	int lanecount;
	//车道数据.
	struct svehLane lanes[32];
	//车道车辆数量.
	int lanecounts[32];
	//阶段车辆数量.
	int stagecounts[8];
}vvehBase;

struct svehHead
{
	//报文长度.
	int length;
	char reserve1[2];
	//数据类型(B6-实时/B7-统计).
	char type;
	char reserve2[133];
};

struct svehRealtimeInfo
{
	char start;
	//指令.
	char command;
	char reserve1[2];
	//设备序号.
	short id;
	short length;
	//车道序号.
	char lane;
	//车速.
	char speed;
	//车道状态.
	char status;
	//排队长度.
	char queue;
	char reserve2[24];
};
struct svehRealtime
{
	struct svehHead head;
	char time[12];
	struct svehRealtimeInfo info;
	char reserve[24];
};

//倒数时间.
int vvehCntTime;

/*========================================*\
    功能 : 基础数据初始化
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fvehBaseInit(void)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		muntLogError("sqlite3_open",0,"0","[]");
		return -1;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	int i;
	for(i=0;i<sizeof(vvehBase.lanecounts)/sizeof(int);i++)
		vvehBase.lanecounts[i]=0;
	for(i=0;i<sizeof(vvehBase.stagecounts)/sizeof(int);i++)
		vvehBase.stagecounts[i]=0;
	vvehBase.lanecount=0;

	result=sprintf(sql,"select device,lane,phase from VEHICLE");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		sqlite3_close(db);
		muntLogError("sqlite3_prepare_v2",0,"0","[]");
		return -1;
	}
	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;

		vvehBase.lanes[vvehBase.lanecount].device=sqlite3_column_int(stmt,0);
		vvehBase.lanes[vvehBase.lanecount].lane  =sqlite3_column_int(stmt,1);
		vvehBase.lanes[vvehBase.lanecount].phase =sqlite3_column_int(stmt,2);
		vvehBase.lanecount++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);
	return 0;
}

/*========================================*\
    功能 : 步骤延长
    参数 : (输入)设备序号
           (输入)车道序号
    返回 : (成功)0
           (失败)-1
           (不需要延长)100
\*========================================*/
int fvehExtend(int device,int lane)
{
	int phase=0;
	int i;
	for(i=0;i<vvehBase.lanecount;i++)
	{
		if(vvehBase.lanes[i].device!=device)
			continue;
		if(vvehBase.lanes[i].lane!=lane)
			continue;
		phase=vvehBase.lanes[i].phase;
	}
	if(phase==0)
		return 100;

	struct sbsnStage *stage;
	stage=&vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex];

	if(stage->phase[phase-1]=='0')
		return 100;
	if(stage->times[stage->timeindex].isadapt==0)
		return 100;

	time_t current;
	time(&current);
	int record;
	record=difftime(current,stage->times[0].start);
	if(record<stage->mingreen+stage->greenflash-vvehCntTime-stage->delta)
		return 100;
	if(record>=stage->maxgreen+stage->greenflash-vvehCntTime)
		return 100;

	if(stage->maxgreen+stage->greenflash-vvehCntTime-record<stage->delta)
		funtClkTimeSnd(stage->maxgreen+stage->greenflash-vvehCntTime-record);
	else
		funtClkTimeSnd(stage->delta);

	return 0;
}

/*========================================*\
    功能 : 车检器服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fvehDeamon(void)
{
	int result;

	fvehBaseInit();

	funtIniIntGet("control","cnttime",&vvehCntTime);

	int lisid;
	lisid=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(lisid==-1)
	{
		muntLogError("socket",errno,strerror(errno),"[]");
		return -1;
	}

	struct sockaddr_in lisaddress;
	bzero(&lisaddress,sizeof(lisaddress));
	lisaddress.sin_family=AF_INET;
	inet_pton(AF_INET,"0.0.0.0",&lisaddress.sin_addr);
	int port;
	funtIniIntGet("develop","vehport",&port);
	lisaddress.sin_port=htons(port);

	int option;
	option=1;
	result=setsockopt(lisid,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	if(result==-1)
	{
		close(lisid);
		muntLogError("setsockopt",errno,strerror(errno),"[]");
		return -1;
	}
	result=bind(lisid,(struct sockaddr*)&lisaddress,sizeof(struct sockaddr_in));
	if(result==-1)
	{
		close(lisid);
		muntLogError("bind",errno,strerror(errno),"");
		return -1;
	}
	result=listen(lisid,1024);
	if(result==-1)
	{
		close(lisid);
		muntLogError("listen",errno,strerror(errno),"[]");
		return -1;
	}

	while(1)
	{
		struct sockaddr_in conaddress;
		bzero(&conaddress,sizeof(conaddress));
		int size;
		size=sizeof(conaddress);
		int conid;
		conid=accept(lisid,(struct sockaddr*)&conaddress,(socklen_t*)&size);
		if(conid==-1&&errno==EINTR)
			continue;
		if(conid==-1&&errno!=EINTR)
		{
			muntLogError("accept",errno,strerror(errno),"[]");
			continue;
		}

		if(vbsnMemory->fmlscheme.isadapt==0)
		{
			close(conid);
			continue;
		}

		char data[1024];

		int remain;
		int record;

		remain=sizeof(struct svehHead);
		record=0;
		while(remain>0)
		{
			result=read(conid,data+record,remain);
			if(result==-1&&(errno==EAGAIN||errno==EINTR))
				continue;
			if(result==-1)
			{
				muntLogError("read",errno,strerror(errno),"[]");
				close(conid);
				goto end;
			}
			remain-=result;
			record+=result;
		}

		if((unsigned char)data[6]==0XB6)
		{
			if(*(int*)data!=sizeof(struct svehRealtime))
			{
				close(conid);
				continue;
			}
		}
		else
		if((unsigned char)data[6]==0XB7)
		{
			close(conid);
			continue;
		}

		remain=*(int*)data-sizeof(struct svehHead);
		record=sizeof(struct svehHead);
		while(remain>0)
		{
			result=read(conid,data+record,remain);
			if(result==-1&&(errno==EAGAIN||errno==EINTR))
				continue;
			if(result==-1)
			{
				muntLogError("read",errno,strerror(errno),"[]");
				close(conid);
				goto end;
			}
			remain-=result;
			record+=result;
		}

		close(conid);

		struct svehRealtime *real;
		real=(struct svehRealtime*)data;

		if(real->info.command==0X02)
			continue;
		if(real->info.command==0X03)
			continue;

		short id;
		*((char*)&id+0)=*((char*)&real->info.id+1);
		*((char*)&id+1)=*((char*)&real->info.id+0);

		fvehExtend(id,real->info.lane);

		end:;
	}

	return 0;
}
