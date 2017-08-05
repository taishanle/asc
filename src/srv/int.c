/*========================================*\
    文件 : int.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include <sqlite3.h>

#include "unt.h"
#include "bsn.h"

//车道数据.
struct sintLane
{
	//端口序号.
	int port;
	//相位序号.
	int phase;
};

//车检数据.
struct sintBase
{
	//车道总数.
	int lanecount;
	//车道数据.
	struct sintLane lanes[32];
	//车道车辆数量.
	int lanecounts[32];
	//阶段车辆数量.
	int stagecounts[8];
}vintBase;

//倒数时间.
int vintCntTime;

/*========================================*\
    功能 : 基础数据初始化
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fintBaseInit(void)
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
	for(i=0;i<sizeof(vintBase.lanecounts)/sizeof(int);i++)
		vintBase.lanecounts[i]=0;
	for(i=0;i<sizeof(vintBase.stagecounts)/sizeof(int);i++)
		vintBase.stagecounts[i]=0;
	vintBase.lanecount=0;

	result=sprintf(sql,"select port,phase from INTERFACE");
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

		vintBase.lanes[vintBase.lanecount].port =sqlite3_column_int(stmt,0);
		vintBase.lanes[vintBase.lanecount].phase=sqlite3_column_int(stmt,1);
		vintBase.lanecount++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);
	return 0;
}

/*========================================*\
    功能 : 步骤延长
    参数 : (输入)端口序号
    返回 : (成功)0
           (失败)-1
           (不需要延长)100
\*========================================*/
int fintExtend(int port)
{
	int phase=0;
	int j;
	for(j=0;j<vintBase.lanecount;j++)
	{
		if(vintBase.lanes[j].port!=port)
			continue;
		phase=vintBase.lanes[j].phase;
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
	if(record<stage->mingreen+stage->greenflash-vintCntTime-stage->delta)
		return 100;
	if(record>=stage->maxgreen+stage->greenflash-vintCntTime)
		return 100;

	if(stage->maxgreen+stage->greenflash-vintCntTime-record<stage->delta)
		funtClkTimeSnd(stage->maxgreen+stage->greenflash-vintCntTime-record);
	else
		funtClkTimeSnd(stage->delta);

	return 0;
}

/*========================================*\
    功能 : 0XF3处理函数
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void fint0XF3(struct suntComMsg *msg)
{
	if(msg->data[0]==0X01)
	{
		int i;
		for(i=0;i<msg->data[1];i++)
		{
			if(msg->data[2+4*i+1]==0X00)
			{
				int port=msg->data[2+4*i+0];
				fintExtend(port);
				vbsnMemory->tmpvehinfo[port-1]++;
			}
		}
	}
	else
	if(msg->data[0]==0X02)
	{
	}
	else
	if(msg->data[0]==0X03)
	{
	}
	else
	if(msg->data[0]==0X04)
	{
	}

	msg->data[0]=0X00;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0XF4处理函数
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void fint0XF4(struct suntComMsg *msg)
{
	char curmode[1+1];
	funtIniStrGet("control","curmode",curmode);
	if(curmode[0]!='m')
		return;
	fbsnMode('m');
	return;
}

/*========================================*\
    功能 : 接口板服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fintDeamon(void)
{
	int result;

	funtMsgInit();

	funtIniIntGet("control","cnttime",&vintCntTime);

	int times;
	funtIniIntGet("develop","times",&times);
	int inter;
	funtIniIntGet("develop","inter",&inter);

	funtIniStrSet("module","int","n");
	int miss=0;

	fintBaseInit();

	while(1)
	{
		struct suntComMsg msg;
		result=funtMsgSrvRcv("int",&msg,1,1000*inter,1);
		if(result==100)
		{
			msg.type=0X01;
			msg.func=0X01;
			msg.data[0]=0X00;
			msg.size[0]=0X01;
			msg.size[1]=0X00;
			msg.src=0X01;

			msg.com=0X00;
			msg.dst=0X03;
			funtMsgCliSnd("int",&msg,1,100,4);

			msg.com=0X00;
			msg.dst=0X05;
			result=funtMsgCliRcv("int",&msg,1,100,4);
			if(result==100)
			{
				if(miss<times+1)
					miss++;
				if(miss==times)
					funtIniStrSet("module","int","a");
			}
			else
			{
				if(miss!=0)
				{
					funtIniStrSet("module","int","n");
					miss=0;
				}
			}
		}
		else
		{
			switch(msg.func)
			{
				case 0XF3: fint0XF3(&msg); break;
				case 0XF4: fint0XF4(&msg); break;
				default: continue;
			}
		}
	}

	return 0;
}
