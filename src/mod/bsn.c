/*========================================*\
    文件 : bsn.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

#include <sqlite3.h>

#include "unt.h"
#include "bsn.h"

//共享内存.
struct sbsnMemory *vbsnMemory;

//通道点位图.
int vbsnMap[]=
{
	13,14,15,10,11,12,8 ,-1,9 ,
	5 ,6 ,7 ,2 ,3 ,4 ,0 ,-1,1 ,
	29,30,31,26,27,28,24,-1,25,
	21,22,23,18,19,20,16,-1,17,
	45,46,47,42,43,44,40,-1,41,
	37,38,39,34,35,36,31,-1,33,
	61,62,63,58,59,60,56,-1,57,
	53,54,55,50,51,52,48,-1,49
};

#define mbsnNode(bitmap,type,channel) (bitmap|1ULL<<vbsnMap[3*(channel-1)+type])
#define mbsnChannel(bitmap,type,corner,lane) (bitmap|type<<(8*(corner-1)+2*(lane-1)))

/*========================================*\
    功能 : 控制检查
    参数 : (输入)模块
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnCtrl(char *module)
{
	return 0;
}

/*========================================*\
    功能 : 手动切换
    参数 : (输入)模块
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnHand(char *module)
{
	return 0;
}

/*========================================*\
    功能 : 时间检查
    参数 : 空
    返回 : (成功)0
           (失败)-1
           (拒绝)100
\*========================================*/
int fbsnTime(void)
{
	struct sbsnStage *stage;
	stage=&vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex];
	if(stage->times[stage->timeindex].isgreen==0)
		return 0;
	time_t current;
	time(&current);
	int record;
	record=difftime(current,stage->times[0].start);
	if(record<stage->mingreen)
		return 100;
	return 0;
}

/*========================================*\
    功能 : 计划检查
    参数 : 空
    返回 : (成功/临界时间)0
           (失败)-1
           (不是临界时间)100
\*========================================*/
int fbsnPlan(void)
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

	time_t stamp;
	stamp=time(NULL);
	if(stamp==(time_t)-1)
	{
		sqlite3_close(db);
		muntLogError("time",errno,strerror(errno),"[]");
		return -1;
	}
	struct tm *tm;
	tm=localtime(&stamp);
	if(tm==NULL)
	{
		sqlite3_close(db);
		muntLogError("localtime",errno,strerror(errno),"[]");
		return -1;
	}

	int periodtable=0;

	char recm[12+1];
	char recd[31+1];
	char recw[7 +1];

	int findplan=0;

	result=sprintf(sql,"select month,date,day,period from PLAN");
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

		char m[12+1];
		char d[31+1];
		char w[7 +1];

		strcpy(m,(char*)sqlite3_column_text(stmt,0));
		if(m[tm->tm_mon]=='0')
			continue;
		strcpy(d,(char*)sqlite3_column_text(stmt,1));
		if(d[tm->tm_mday-1]=='0')
			continue;
		strcpy(w,(char*)sqlite3_column_text(stmt,2));
		if(w[tm->tm_wday]=='0')
			continue;

		if
		(
			findplan==0||
			funtBitCount(m)<funtBitCount(recm)||
			funtBitCount(d)<funtBitCount(recd)||
			funtBitCount(w)<funtBitCount(recw)
		)
		{
			strcpy(recm,m);
			strcpy(recd,d);
			strcpy(recw,w);

			periodtable=sqlite3_column_int(stmt,3);

			findplan=1;
		}
	}
	sqlite3_finalize(stmt);

	if(findplan==0)
	{
		sqlite3_close(db);
		return 100;
	}

	int isedge=0;

	result=sprintf(sql,"select hour,minute from PERIOD where periodtable=%d order by hour,minute",periodtable);
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

		int h;
		h=sqlite3_column_int(stmt,0);
		if(h!=tm->tm_hour)
			continue;
		int m;
		m=sqlite3_column_int(stmt,1);
		if(m!=tm->tm_min)
			continue;

		isedge=1;
		break;
	}
	sqlite3_finalize(stmt);

	if(isedge==0)
	{
		sqlite3_close(db);
		return 100;
	}

	sqlite3_close(db);
	return 0;
}

/*========================================*\
    功能 : 获取方案
    参数 : (出入)方案数据
    返回 : (成功)0
           (失败)-1
           (配置缺失)100
\*========================================*/
int fbsnSchemeGet(struct sbsnScheme *scheme)
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

	time_t stamp;
	stamp=time(NULL);
	if(stamp==(time_t)-1)
	{
		sqlite3_close(db);
		muntLogError("time",errno,strerror(errno),"[]");
		return -1;
	}
	struct tm *tm;
	tm=localtime(&stamp);
	if(tm==NULL)
	{
		sqlite3_close(db);
		muntLogError("localtime",errno,strerror(errno),"[]");
		return -1;
	}

	char recm[12+1];
	char recd[31+1];
	char recw[7 +1];

	int findplan=0;

	result=sprintf(sql,"select plan,month,date,day,period from PLAN");
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

		char m[12+1];
		char d[31+1];
		char w[7 +1];

		strcpy(m,(char*)sqlite3_column_text(stmt,1));
		if(m[tm->tm_mon]=='0')
			continue;
		strcpy(d,(char*)sqlite3_column_text(stmt,2));
		if(d[tm->tm_mday-1]=='0')
			continue;
		strcpy(w,(char*)sqlite3_column_text(stmt,3));
		if(w[tm->tm_wday]=='0')
			continue;

		if
		(
			findplan==0||
			funtBitCount(m)<funtBitCount(recm)||
			funtBitCount(d)<funtBitCount(recd)||
			funtBitCount(w)<funtBitCount(recw)
		)
		{
			strcpy(recm,m);
			strcpy(recd,d);
			strcpy(recw,w);

			scheme->planindex=sqlite3_column_int(stmt,0);
			scheme->periodtable=sqlite3_column_int(stmt,4);

			findplan=1;
		}
	}
	sqlite3_finalize(stmt);

	if(findplan==0)
	{
		sqlite3_close(db);
		return 100;
	}

	int findperiod=0;
	char tempmode[1+1];

	result=sprintf(sql,"select periodindex,hour,minute,mode,scheme from PERIOD where periodtable=%d order by hour,minute",scheme->periodtable);
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

		int h;
		h=sqlite3_column_int(stmt,1);
		if(tm->tm_hour<h)
			break;
		if(tm->tm_hour>h)
			goto goon;
		int m;
		m=sqlite3_column_int(stmt,2);
		if(tm->tm_min<m)
			break;

		goon:;
		strcpy(tempmode,(char*)sqlite3_column_text(stmt,3));

		scheme->periodindex=sqlite3_column_int(stmt,0);
		scheme->schemeindex=sqlite3_column_int(stmt,4);
		findperiod=1;
	}
	sqlite3_finalize(stmt);

	if(findperiod==0)
	{
		sqlite3_close(db);
		return 100;
	}

	if(tempmode[0]=='0'||tempmode[0]=='1'||tempmode[0]=='2')
	{
		scheme->isfixed=0;
		if(tempmode[0]=='0')
			scheme->isadapt=0;
		else
			scheme->isadapt=1;
	}
	else
	{
		fbsnSpecialNode(scheme,tempmode[0]);
	}

	sqlite3_close(db);
	return 0;
}

/*========================================*\
    功能 : 设置方案
    参数 : (出入)方案数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnSchemeSet(struct sbsnScheme *scheme)
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

	result=sprintf(sql,"select cycle,stage from SCHEME where scheme=%d",scheme->schemeindex);
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		sqlite3_close(db);
		muntLogError("sqlite3_prepare_v2",0,"0","[]");
		return -1;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		muntLogError("sqlite3_step",0,"0","[]");
		return -1;
	}
	scheme->schemecycle=sqlite3_column_int(stmt,0);
	scheme->stagetable=sqlite3_column_int(stmt,1);

	sqlite3_finalize(stmt);

	scheme->stagecount=0;

	result=sprintf(sql,"select phase,greenlight,greenflash,yellow,red,manlight,manflash,delta,mingreen,maxgreen from STAGE where stagetable=%d",scheme->stagetable);
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

		struct sbsnStage *stage;
		stage=&scheme->stages[scheme->stagecount];

		strcpy(stage->phase,(char*)sqlite3_column_text(stmt,0));

		stage->greenlight=sqlite3_column_int(stmt,1);
		stage->greenflash=sqlite3_column_int(stmt,2);
		stage->yellow    =sqlite3_column_int(stmt,3);
		stage->red       =sqlite3_column_int(stmt,4);
		stage->manlight  =sqlite3_column_int(stmt,5);
		stage->manflash  =sqlite3_column_int(stmt,6);
		stage->delta     =sqlite3_column_int(stmt,7);
		stage->mingreen  =sqlite3_column_int(stmt,8);
		stage->maxgreen  =sqlite3_column_int(stmt,9);

		scheme->stagecount++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);
	return 0;
}

/*========================================*\
    功能 : 获取点位
    参数 : (出入)方案数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnNode(struct sbsnScheme *scheme)
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
	int j;
	for(i=0;i<scheme->stagecount;i++)
	{
		struct sbsnStage *stage;
		stage=&scheme->stages[i];

		int corners[8]={0};

		for(j=0;j<sizeof(scheme->stages[i].phase);j++)
		{
			result=sprintf(sql,"select corner,lane from PHASE where phase=%d",j+1);
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

				int corner;
				corner=sqlite3_column_int(stmt,0);
				int lane;
				lane=sqlite3_column_int(stmt,1);

				if(scheme->stages[i].phase[j]=='1'&&lane!=4)
					corners[corner]=1;
			}
			sqlite3_finalize(stmt);
		}

		unsigned long long veh1drv[3]={0};
		unsigned long long man1drv[2]={0};
		unsigned long long veh0drv[3]={0};
		unsigned long long man0drv[2]={0};
		unsigned long long veh1lcd[3]={0};
		unsigned long long man1lcd[2]={0};
		unsigned long long veh0lcd[3]={0};
		unsigned long long man0lcd[2]={0};
		unsigned long long count[2]={0};

		for(j=0;j<sizeof(scheme->stages[i].phase);j++)
		{
			result=sprintf(sql,"select channel,corner,lane from PHASE where phase=%d",j+1);
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

				int channel;
				channel=sqlite3_column_int(stmt,0);
				int corner;
				corner=sqlite3_column_int(stmt,1);
				int lane;
				lane=sqlite3_column_int(stmt,2);

				if(scheme->stages[i].phase[j]=='1')
				{
					if(lane!=4)
					{
						veh1drv[0]=mbsnNode(veh1drv[0],0,channel);
						veh1drv[1]=mbsnNode(veh1drv[1],1,channel);
						veh1drv[2]=mbsnNode(veh1drv[2],2,channel);

						veh1lcd[0]=mbsnChannel(veh1lcd[0],3ULL,corner,lane);
						veh1lcd[1]=mbsnChannel(veh1lcd[1],2ULL,corner,lane);
						veh1lcd[2]=mbsnChannel(veh1lcd[2],1ULL,corner,lane);

						count[0]=mbsnNode(count[0],0,channel);
					}
					else
					{
						man1drv[0]=mbsnNode(man1drv[0],0,channel);
						man1drv[1]=mbsnNode(man1drv[1],2,channel);

						man1lcd[0]=mbsnChannel(man1lcd[0],3ULL,corner,lane);
						man1lcd[1]=mbsnChannel(man1lcd[1],1ULL,corner,lane);
					}
				}
				else
				{
					if(lane!=4)
					{
						veh0drv[0]=mbsnNode(veh0drv[0],2,channel);
						veh0drv[1]=mbsnNode(veh0drv[1],2,channel);
						veh0drv[2]=mbsnNode(veh0drv[2],2,channel);

						veh0lcd[0]=mbsnChannel(veh0lcd[0],1ULL,corner,lane);
						veh0lcd[1]=mbsnChannel(veh0lcd[1],1ULL,corner,lane);
						veh0lcd[2]=mbsnChannel(veh0lcd[2],1ULL,corner,lane);

						if(scheme->stages[(i+1)%scheme->stagecount].phase[j]=='1'&&corners[corner]==0)
							count[1]=mbsnNode(count[1],2,channel);
					}
					else
					{
						man0drv[0]=mbsnNode(man0drv[0],2,channel);
						man0drv[1]=mbsnNode(man0drv[1],2,channel);

						man0lcd[0]=mbsnChannel(man0lcd[0],1ULL,corner,lane);
						man0lcd[1]=mbsnChannel(man0lcd[1],1ULL,corner,lane);
					}
				}
			}
			sqlite3_finalize(stmt);
		}

		unsigned long long vehlightdrv[4];
		vehlightdrv[0]=veh1drv[0]|veh0drv[0];
		vehlightdrv[1]=veh1drv[0]|veh0drv[0];
		vehlightdrv[2]=veh1drv[1]|veh0drv[1];
		vehlightdrv[3]=veh1drv[2]|veh0drv[2];
		unsigned long long vehflashdrv[4];
		vehflashdrv[0]=0;
		vehflashdrv[1]=veh1drv[0];
		vehflashdrv[2]=0;
		vehflashdrv[3]=0;

		unsigned long long manlightdrv[3];
		manlightdrv[0]=man1drv[0]|man0drv[0];
		manlightdrv[1]=man1drv[0]|man0drv[0];
		manlightdrv[2]=man1drv[1]|man0drv[1];
		unsigned long long manflashdrv[3];
		manflashdrv[0]=0;
		manflashdrv[1]=man1drv[0];
		manflashdrv[2]=0;

		unsigned long long vehlightlcd[4];
		vehlightlcd[0]=veh1lcd[0]|veh0lcd[0];
		vehlightlcd[1]=veh1lcd[0]|veh0lcd[0];
		vehlightlcd[2]=veh1lcd[1]|veh0lcd[1];
		vehlightlcd[3]=veh1lcd[2]|veh0lcd[2];
		unsigned long long manlightlcd[3];
		manlightlcd[0]=man1lcd[0]|man0lcd[0];
		manlightlcd[1]=man1lcd[0]|man0lcd[0];
		manlightlcd[2]=man1lcd[1]|man0lcd[1];

		int vehcount=4;
		int mancount=3;
		int cntcount=2;

		int vehtime[4];
		vehtime[0]=scheme->isadapt==0?stage->greenlight:stage->mingreen;
		vehtime[1]=stage->greenflash;
		vehtime[2]=stage->yellow;
		vehtime[3]=stage->red;

		int mantime[3];
		mantime[1]=stage->manflash;
		mantime[2]=stage->red;
		mantime[0]=vehtime[0]+vehtime[1]+vehtime[2]+vehtime[3]-mantime[1]-mantime[2];

		int _cnttime;
		funtIniIntGet("control","cnttime",&_cnttime);

		int cnttime[2];
		cnttime[0]=vehtime[0]+vehtime[1]-_cnttime;
		cnttime[1]=vehtime[2]+vehtime[3];

		if(_cnttime==0)
		{
			stage->counts[0]=0;
			stage->counts[1]=0;
		}
		else
		{
			stage->counts[0]=count[0];
			stage->counts[1]=count[1];
		}

		stage->timecount=0;
		stage->times[0].isdrvstep=1;
		stage->times[0].islcdstep=1;
		stage->times[0].iscntstep=0;
		stage->drvtimes[0]=0;
		stage->lcdtimes[0]=0;
		stage->drvstepcount=0;
		stage->lcdstepcount=0;
		stage->cntstepcount=0;

		int vehindex=0;
		int manindex=0;
		int cntindex=0;

		while(1)
		{
			if(vehindex==vehcount&&manindex==mancount&&cntindex==cntcount)
				break;

			if(vehindex<vehcount&&vehtime[vehindex]==0)
			{
				vehindex++;
				continue;
			}
			if(manindex<mancount&&mantime[manindex]==0)
			{
				manindex++;
				continue;
			}
			if(cntindex<cntcount&&cnttime[cntindex]==0)
			{
				cntindex++;
				continue;
			}

			if(vehindex==0)
				stage->times[stage->timecount].isgreen=1;
			else
				stage->times[stage->timecount].isgreen=0;

			if(cntindex==0)
				stage->times[stage->timecount].isadapt=1;
			else
				stage->times[stage->timecount].isadapt=0;

			int temptime[3];
			temptime[0]=vehindex==vehcount?INT_MAX:vehtime[vehindex];
			temptime[1]=manindex==mancount?INT_MAX:mantime[manindex];
			temptime[2]=cntindex==cntcount?INT_MAX:cnttime[cntindex];

			if(temptime[0]<temptime[1])
				if(temptime[0]<temptime[2])
					stage->times[stage->timecount].time=temptime[0];
				else
					stage->times[stage->timecount].time=temptime[2];
			else
				if(temptime[1]<temptime[2])
					stage->times[stage->timecount].time=temptime[1];
				else
					stage->times[stage->timecount].time=temptime[2];

			int result[3];
			result[0]=temptime[0]==stage->times[stage->timecount].time?1:0;
			result[1]=temptime[1]==stage->times[stage->timecount].time?1:0;
			result[2]=temptime[2]==stage->times[stage->timecount].time?1:0;

			if(result[0]==0&&vehindex!=vehcount)
				vehtime[vehindex]-=stage->times[stage->timecount].time;
			if(result[1]==0&&manindex!=mancount)
				mantime[manindex]-=stage->times[stage->timecount].time;
			if(result[2]==0&&cntindex!=cntcount)
				cnttime[cntindex]-=stage->times[stage->timecount].time;

			stage->drvtimes[stage->drvstepcount]+=stage->times[stage->timecount].time;
			stage->lcdtimes[stage->lcdstepcount]+=stage->times[stage->timecount].time;
			stage->timecount++;

			if(result[0]==1||result[1]==1)
			{
				stage->lights[stage->drvstepcount]=0;
				stage->lights[stage->drvstepcount]|=vehlightdrv[vehindex];
				stage->lights[stage->drvstepcount]|=manlightdrv[manindex];
				stage->lights[stage->drvstepcount]=~stage->lights[stage->drvstepcount];
				stage->flashs[stage->drvstepcount]=0;
				stage->flashs[stage->drvstepcount]|=vehflashdrv[vehindex];
				stage->flashs[stage->drvstepcount]|=manflashdrv[manindex];

				stage->times[stage->timecount].isdrvstep=1;
				stage->drvstepcount++;
				if(stage->drvstepcount<5)
					stage->drvtimes[stage->drvstepcount]=0;
			}
			else
			{
				stage->times[stage->timecount].isdrvstep=0;
			}

			if(result[0]==1&&vehindex!=0)
			{
				stage->colors[stage->lcdstepcount]=0;
				stage->colors[stage->lcdstepcount]|=vehlightlcd[vehindex];
				stage->colors[stage->lcdstepcount]|=manlightlcd[manindex];

				stage->times[stage->timecount].islcdstep=1;
				stage->lcdstepcount++;
				if(stage->lcdstepcount<3)
					stage->lcdtimes[stage->lcdstepcount]=0;
			}
			else
			{
				stage->times[stage->timecount].islcdstep=0;
			}

			if(result[2]==1)
			{
				stage->times[stage->timecount].iscntstep=1;
				stage->cntstepcount++;
			}
			else
			{
				stage->times[stage->timecount].iscntstep=0;
			}


			if(result[0]==1)
				vehindex++;
			if(result[1]==1)
				manindex++;
			if(result[2]==1)
				cntindex++;
		}
	}

	for(i=0,scheme->schemetotal=0;i<scheme->stagecount;i++)
		for(j=0;j<scheme->stages[i].timecount;j++)
			scheme->schemetotal+=scheme->stages[i].times[j].time;
	for(i=0,scheme->drvstepcount=0;i<scheme->stagecount;i++)
		scheme->drvstepcount+=scheme->stages[i].drvstepcount;
	for(i=0,scheme->lcdstepcount=0;i<scheme->stagecount;i++)
		scheme->lcdstepcount+=scheme->stages[i].lcdstepcount;

	muntLogDebug("isadapt[%d]",scheme->isadapt);
	muntLogDebug("isfixed[%d]",scheme->isfixed);
	muntLogDebug("planindex[%d]",scheme->planindex);
	muntLogDebug("periodtable[%d]",scheme->periodtable);
	muntLogDebug("periodindex[%d]",scheme->periodindex);
	muntLogDebug("schemeindex[%d]",scheme->schemeindex);
	muntLogDebug("schemecycle[%d]",scheme->schemecycle);
	muntLogDebug("schemetotal[%d]",scheme->schemetotal);
	muntLogDebug("stagetable[%d]",scheme->stagetable);
	muntLogDebug("stagecount[%d]",scheme->stagecount);
	muntLogDebug("stageindex[%d]",scheme->stageindex);
	muntLogDebug("drvstepcount[%d]",scheme->drvstepcount);
	muntLogDebug("drvstepindex[%d]",scheme->drvstepindex);
	muntLogDebug("lcdstepcount[%d]",scheme->lcdstepcount);
	muntLogDebug("lcdstepindex[%d]",scheme->lcdstepindex);
	int m;
	for(m=0;m<scheme->stagecount;m++)
	{
		muntLogDebug("stage:%d,phase[%.32s]",m,scheme->stages[m].phase);
		muntLogDebug("stage:%d,greenligth[%d]",m,scheme->stages[m].greenlight);
		muntLogDebug("stage:%d,greenflash[%d]",m,scheme->stages[m].greenflash);
		muntLogDebug("stage:%d,yellow[%d]",m,scheme->stages[m].yellow);
		muntLogDebug("stage:%d,red[%d]",m,scheme->stages[m].red);
		muntLogDebug("stage:%d,manlight[%d]",m,scheme->stages[m].manlight);
		muntLogDebug("stage:%d,manflash[%d]",m,scheme->stages[m].manflash);
		muntLogDebug("stage:%d,delta[%d]",m,scheme->stages[m].delta);
		muntLogDebug("stage:%d,mingreen[%d]",m,scheme->stages[m].mingreen);
		muntLogDebug("stage:%d,maxgreen[%d]",m,scheme->stages[m].maxgreen);
		muntLogDebug("stage:%d,timecount[%d]",m,scheme->stages[m].timecount);
		muntLogDebug("stage:%d,timeindex[%d]",m,scheme->stages[m].timeindex);
		muntLogDebug("stage:%d,drvstepcount[%d]",m,scheme->stages[m].drvstepcount);
		muntLogDebug("stage:%d,lcdstepcount[%d]",m,scheme->stages[m].lcdstepcount);
		muntLogDebug("stage:%d,cntstepcount[%d]",m,scheme->stages[m].cntstepcount);
		muntLogDebug("stage:%d,drvstepindex[%d]",m,scheme->stages[m].drvstepindex);
		muntLogDebug("stage:%d,lcdstepindex[%d]",m,scheme->stages[m].lcdstepindex);
		muntLogDebug("stage:%d,cntstepindex[%d]",m,scheme->stages[m].cntstepindex);
		int n;
		for(n=0;n<scheme->stages[m].timecount;n++)
		{
			muntLogDebug("stage:%d,time:%d,time[%d]",m,n,scheme->stages[m].times[n].time);
			muntLogDebug("stage:%d,time:%d,isdrvstep[%d]",m,n,scheme->stages[m].times[n].isdrvstep);
			muntLogDebug("stage:%d,time:%d,islcdstep[%d]",m,n,scheme->stages[m].times[n].islcdstep);
			muntLogDebug("stage:%d,time:%d,iscntstep[%d]",m,n,scheme->stages[m].times[n].iscntstep);
			muntLogDebug("stage:%d,time:%d,isgreen[%d]",m,n,scheme->stages[m].times[n].isgreen);
			muntLogDebug("stage:%d,time:%d,isadapt[%d]",m,n,scheme->stages[m].times[n].isadapt);
		}
		for(n=0;n<5;n++)
		{
			muntLogDebug("stage:%d,drvstep:%d,lights[%llX]",m,n,scheme->stages[m].lights[n]);
			muntLogDebug("stage:%d,drvstep:%d,flashs[%llX]",m,n,scheme->stages[m].flashs[n]);
			muntLogDebug("stage:%d,drvstep:%d,drvtimes[%d]",m,n,scheme->stages[m].drvtimes[n]);
		}
		for(n=0;n<3;n++)
		{
			muntLogDebug("stage:%d,lcdstep:%d,colors[%llX]",m,n,scheme->stages[m].colors[n]);
			muntLogDebug("stage:%d,lcdstep:%d,lcdtimes[%d]",m,n,scheme->stages[m].lcdtimes[n]);
		}
		for(n=0;n<2;n++)
		{
			muntLogDebug("stage:%d,cntstep:%d,counts[%llX]",m,n,scheme->stages[m].counts[n]);
		}
	}

	sqlite3_close(db);
	return 0;
}

/*========================================*\
    功能 : 特殊点位设置
    参数 : (输入)方案数据
           (输入)模式 (全红)r
                      (黄闪)y
                      (关灯)c
                      (行人)m
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnSpecialNode(struct sbsnScheme *scheme,char mode)
{
	int result;

	if(mode=='r'||mode=='y'||mode=='c')
	{
		if(mode=='r')
		{
			memset(&scheme->stages[0].lights[0],0X92,8); //0X92=10010010
			memset(&scheme->stages[0].flashs[0],0X00,8); //0X00=00000000
			memset(&scheme->stages[0].colors[0],0X55,8); //0X55=01010101
			scheme->stages[0].lights[0]=~scheme->stages[0].lights[0];
		}
		else
		if(mode=='y')
		{
			memset(&scheme->stages[0].lights[0],0X49,8); //0X49=01001001
			memset(&scheme->stages[0].flashs[0],0X48,8); //0X48=01001000
			memset(&scheme->stages[0].colors[0],0XEA,8); //0XEA=11101010
			scheme->stages[0].lights[0]=~scheme->stages[0].lights[0];
		}
		else
		if(mode=='c')
		{
			memset(&scheme->stages[0].lights[0],0X00,8); //0X00=00000000
			memset(&scheme->stages[0].flashs[0],0X00,8); //0X00=00000000
			memset(&scheme->stages[0].colors[0],0X00,8); //0X00=00000000
			scheme->stages[0].lights[0]=~scheme->stages[0].lights[0];
		}

		scheme->isadapt=0;
		scheme->isfixed=1;
		/*
		scheme->planindex=0;
		scheme->periodtable=0;
		scheme->periodindex=0;
		*/
		scheme->schemeindex=0;
		scheme->schemecycle=0;
		scheme->schemetotal=0;
		scheme->stagetable=0;
		scheme->stagecount=1;
		scheme->stageindex=0;
		scheme->drvstepcount=1;
		scheme->drvstepindex=0;
		scheme->lcdstepcount=1;
		scheme->lcdstepindex=0;
		scheme->stages[0].greenlight=0;
		scheme->stages[0].greenflash=0;
		scheme->stages[0].yellow=0;
		scheme->stages[0].red=0;
		scheme->stages[0].manlight=0;
		scheme->stages[0].manflash=0;
		scheme->stages[0].delta=0;
		scheme->stages[0].mingreen=0;
		scheme->stages[0].maxgreen=0;
		scheme->stages[0].timecount=1;
		scheme->stages[0].timeindex=0;
		scheme->stages[0].drvstepcount=1;
		scheme->stages[0].drvstepindex=0;
		scheme->stages[0].lcdstepcount=1;
		scheme->stages[0].lcdstepindex=0;
		scheme->stages[0].cntstepcount=0;
		scheme->stages[0].cntstepindex=0;
		scheme->stages[0].drvtimes[0]=0;
		scheme->stages[0].lcdtimes[0]=0;
		scheme->stages[0].times[0].time=10;
		scheme->stages[0].times[0].isdrvstep=1;
		scheme->stages[0].times[0].islcdstep=1;
		scheme->stages[0].times[0].iscntstep=0;
		scheme->stages[0].times[0].isgreen=0;
		scheme->stages[0].times[0].isadapt=0;
	}
	else
	if(mode=='m')
	{
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

		unsigned long long vehdrv[3]={0};
		unsigned long long mandrv[2]={0};
		unsigned long long lcd[5]={0};

		int channel;

		result=sprintf(sql,"select channel from PHASE where corner=3 and lane=2");
		result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
		if(result!=SQLITE_OK)
		{
			sqlite3_close(db);
			muntLogError("sqlite3_prepare_v2",0,"0","[]");
			return -1;
		}
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			return 100;
		channel=sqlite3_column_int(stmt,0);
		vehdrv[0]=mbsnNode(vehdrv[0],0,channel);
		vehdrv[1]=mbsnNode(vehdrv[1],1,channel);
		vehdrv[2]=mbsnNode(vehdrv[2],2,channel);
		result=sqlite3_step(stmt);
		if(result==SQLITE_ROW)
			return 100;
		sqlite3_finalize(stmt);

		result=sprintf(sql,"select channel from PHASE where corner=7 and lane=2");
		result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
		if(result!=SQLITE_OK)
		{
			sqlite3_close(db);
			muntLogError("sqlite3_prepare_v2",0,"0","[]");
			return -1;
		}
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			return 100;
		channel=sqlite3_column_int(stmt,0);
		vehdrv[0]=mbsnNode(vehdrv[0],0,channel);
		vehdrv[1]=mbsnNode(vehdrv[1],1,channel);
		vehdrv[2]=mbsnNode(vehdrv[2],2,channel);
		sqlite3_finalize(stmt);
		result=sqlite3_step(stmt);
		if(result==SQLITE_ROW)
			return 100;
		sqlite3_finalize(stmt);

		result=sprintf(sql,"select channel from PHASE where corner=3 and lane=4");
		result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
		if(result!=SQLITE_OK)
		{
			sqlite3_close(db);
			muntLogError("sqlite3_prepare_v2",0,"0","[]");
			return -1;
		}
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			return 100;
		channel=sqlite3_column_int(stmt,0);
		mandrv[0]=mbsnNode(mandrv[0],2,channel);
		mandrv[1]=mbsnNode(mandrv[1],0,channel);
		result=sqlite3_step(stmt);
		if(result==SQLITE_ROW)
			return 100;
		sqlite3_finalize(stmt);

		result=sprintf(sql,"select channel from PHASE where corner=7 and lane=4");
		result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
		if(result!=SQLITE_OK)
		{
			sqlite3_close(db);
			muntLogError("sqlite3_prepare_v2",0,"0","[]");
			return -1;
		}
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			return 100;
		channel=sqlite3_column_int(stmt,0);
		mandrv[0]=mbsnNode(mandrv[0],2,channel);
		mandrv[1]=mbsnNode(mandrv[1],0,channel);
		result=sqlite3_step(stmt);
		if(result==SQLITE_ROW)
			return 100;
		sqlite3_finalize(stmt);

		sqlite3_close(db);

		scheme->stages[0].lights[0]=vehdrv[0]|mandrv[0];
		scheme->stages[0].flashs[0]=0ULL;

		scheme->stages[0].lights[1]=vehdrv[0]|mandrv[0];
		scheme->stages[0].flashs[1]=vehdrv[0];

		scheme->stages[0].lights[2]=vehdrv[1]|mandrv[0];
		scheme->stages[0].flashs[2]=0ULL;

		scheme->stages[0].lights[3]=vehdrv[2]|mandrv[0];
		scheme->stages[0].flashs[3]=0ULL;

		scheme->stages[1].lights[0]=vehdrv[2]|mandrv[1];
		scheme->stages[1].flashs[0]=0ULL;

		scheme->stages[1].lights[1]=vehdrv[2]|mandrv[1];
		scheme->stages[1].flashs[1]=mandrv[1];

		scheme->stages[1].lights[2]=vehdrv[2]|mandrv[0];
		scheme->stages[1].flashs[2]=0ULL;

		scheme->stages[0].lights[0]=~scheme->stages[0].lights[0];
		scheme->stages[0].lights[1]=~scheme->stages[0].lights[1];
		scheme->stages[0].lights[2]=~scheme->stages[0].lights[2];
		scheme->stages[0].lights[3]=~scheme->stages[0].lights[3];
		scheme->stages[1].lights[0]=~scheme->stages[1].lights[0];
		scheme->stages[1].lights[1]=~scheme->stages[1].lights[1];
		scheme->stages[1].lights[2]=~scheme->stages[1].lights[2];

		lcd[0]=mbsnChannel(lcd[0],3ULL,3,2);
		lcd[0]=mbsnChannel(lcd[0],3ULL,7,2);
		lcd[0]=mbsnChannel(lcd[0],1ULL,3,4);
		lcd[0]=mbsnChannel(lcd[0],1ULL,7,4);

		lcd[1]=mbsnChannel(lcd[1],2ULL,3,2);
		lcd[1]=mbsnChannel(lcd[1],2ULL,7,2);
		lcd[1]=mbsnChannel(lcd[1],1ULL,3,4);
		lcd[1]=mbsnChannel(lcd[1],1ULL,7,4);

		lcd[2]=mbsnChannel(lcd[2],1ULL,3,2);
		lcd[2]=mbsnChannel(lcd[2],1ULL,7,2);
		lcd[2]=mbsnChannel(lcd[2],1ULL,3,4);
		lcd[2]=mbsnChannel(lcd[2],1ULL,7,4);

		lcd[3]=mbsnChannel(lcd[3],1ULL,3,2);
		lcd[3]=mbsnChannel(lcd[3],1ULL,7,2);
		lcd[3]=mbsnChannel(lcd[3],3ULL,3,4);
		lcd[3]=mbsnChannel(lcd[3],3ULL,7,4);

		lcd[4]=mbsnChannel(lcd[4],1ULL,3,2);
		lcd[4]=mbsnChannel(lcd[4],1ULL,7,2);
		lcd[4]=mbsnChannel(lcd[4],1ULL,3,4);
		lcd[4]=mbsnChannel(lcd[4],1ULL,7,4);

		scheme->stages[0].colors[0]=lcd[0];
		scheme->stages[0].colors[1]=lcd[1];
		scheme->stages[0].colors[2]=lcd[2];
		scheme->stages[1].colors[0]=lcd[3];
		scheme->stages[1].colors[1]=lcd[4];

		scheme->isadapt=0;
		scheme->isfixed=1;
		/*
		scheme->planindex=0;
		scheme->periodtable=0;
		scheme->periodindex=0;
		*/
		scheme->schemeindex=0;
		scheme->schemecycle=0;
		scheme->schemetotal=0;
		scheme->stagetable=0;
		scheme->stagecount=2;
		scheme->stageindex=0;
		scheme->drvstepcount=7;
		scheme->drvstepindex=0;
		scheme->lcdstepcount=5;
		scheme->lcdstepindex=0;
		scheme->stages[0].greenlight=0;
		scheme->stages[0].greenflash=0;
		scheme->stages[0].yellow=0;
		scheme->stages[0].red=0;
		scheme->stages[0].manlight=0;
		scheme->stages[0].manflash=0;
		scheme->stages[0].delta=0;
		scheme->stages[0].mingreen=0;
		scheme->stages[0].maxgreen=0;
		scheme->stages[1].greenlight=0;
		scheme->stages[1].greenflash=0;
		scheme->stages[1].yellow=0;
		scheme->stages[1].red=0;
		scheme->stages[1].manlight=0;
		scheme->stages[1].manflash=0;
		scheme->stages[1].delta=0;
		scheme->stages[1].mingreen=0;
		scheme->stages[1].maxgreen=0;

		scheme->stages[0].timecount=4;
		scheme->stages[0].timeindex=0;
		scheme->stages[0].drvstepcount=4;
		scheme->stages[0].drvstepindex=0;
		scheme->stages[0].lcdstepcount=3;
		scheme->stages[0].lcdstepindex=0;
		scheme->stages[0].cntstepcount=0;
		scheme->stages[0].cntstepindex=0;
		scheme->stages[0].drvtimes[0]=0;
		scheme->stages[0].drvtimes[1]=0;
		scheme->stages[0].drvtimes[2]=0;
		scheme->stages[0].drvtimes[3]=0;
		scheme->stages[0].lcdtimes[0]=0;
		scheme->stages[0].lcdtimes[1]=0;
		scheme->stages[0].lcdtimes[2]=0;
		scheme->stages[0].times[0].time=0;
		scheme->stages[0].times[0].isdrvstep=1;
		scheme->stages[0].times[0].islcdstep=1;
		scheme->stages[0].times[0].iscntstep=0;
		scheme->stages[0].times[0].isgreen=0;
		scheme->stages[0].times[0].isadapt=0;
		scheme->stages[0].times[1].time=3;
		scheme->stages[0].times[1].isdrvstep=1;
		scheme->stages[0].times[1].islcdstep=0;
		scheme->stages[0].times[1].iscntstep=0;
		scheme->stages[0].times[1].isgreen=0;
		scheme->stages[0].times[1].isadapt=0;
		scheme->stages[0].times[2].time=2;
		scheme->stages[0].times[2].isdrvstep=1;
		scheme->stages[0].times[2].islcdstep=1;
		scheme->stages[0].times[2].iscntstep=0;
		scheme->stages[0].times[2].isgreen=0;
		scheme->stages[0].times[2].isadapt=0;
		scheme->stages[0].times[3].time=1;
		scheme->stages[0].times[3].isdrvstep=1;
		scheme->stages[0].times[3].islcdstep=1;
		scheme->stages[0].times[3].iscntstep=0;
		scheme->stages[0].times[3].isgreen=0;
		scheme->stages[0].times[3].isadapt=0;

		scheme->stages[1].timecount=3;
		scheme->stages[1].timeindex=0;
		scheme->stages[1].drvstepcount=3;
		scheme->stages[1].drvstepindex=0;
		scheme->stages[1].lcdstepcount=2;
		scheme->stages[1].lcdstepindex=0;
		scheme->stages[1].cntstepcount=0;
		scheme->stages[1].cntstepindex=0;
		scheme->stages[1].drvtimes[0]=0;
		scheme->stages[1].drvtimes[1]=0;
		scheme->stages[1].drvtimes[2]=0;
		scheme->stages[1].lcdtimes[0]=0;
		scheme->stages[1].lcdtimes[1]=0;
		scheme->stages[1].times[0].time=-1;
		scheme->stages[1].times[0].isdrvstep=1;
		scheme->stages[1].times[0].islcdstep=1;
		scheme->stages[1].times[0].iscntstep=0;
		scheme->stages[1].times[0].isgreen=0;
		scheme->stages[1].times[0].isadapt=0;
		scheme->stages[1].times[1].time=3;
		scheme->stages[1].times[1].isdrvstep=1;
		scheme->stages[1].times[1].islcdstep=0;
		scheme->stages[1].times[1].iscntstep=0;
		scheme->stages[1].times[1].isgreen=0;
		scheme->stages[1].times[1].isadapt=0;
		scheme->stages[1].times[2].time=1;
		scheme->stages[1].times[2].isdrvstep=1;
		scheme->stages[1].times[2].islcdstep=1;
		scheme->stages[1].times[2].iscntstep=0;
		scheme->stages[1].times[2].isgreen=0;
		scheme->stages[1].times[2].isadapt=0;
	}

	return 0;
}

/*========================================*\
    功能 : 模式设置
    参数 : (输入)模式 (定周期)0X41-0X60
                      (分时段)p
                      (自适应)a
                      (全红)r
                      (黄闪)y
                      (关灯)c
                      (行人)m
                      (步进)s
    返回 : (成功)0
           (失败)-1
           (拒绝)100
\*========================================*/
int fbsnMode(char mode)
{
	int result;

	struct sbsnScheme scheme;
	bzero(&scheme,sizeof(scheme));

	char curmode[1+1];
	funtIniStrGet("control","curmode",curmode);

	if(!(((0X41<=mode&&mode<=0X60)||mode=='p'||mode=='a')&&((0X41<=curmode[0]&&curmode[0]<=0X60)||curmode[0]=='p'||curmode[0]=='a')))
	{
		result=fbsnTime();
		if(result==-1)
			return -1;
		if(result==100)
			return 100;
	}

	int schpid;
	funtIniIntGet("deamon","sch",&schpid);

	if(mode=='s')
	{
		if(vbsnMemory->fmlscheme.isfixed==1)
			return 100;
		result=sigqueue(schpid,cbsnStepEnter,(union sigval)0);
		if(result==-1)
		{
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",schpid);
			return -1;
		}
		if(curmode[0]!='s')
			funtIniStrSet("control","curmode","s");
		return 0;
	}

	if(curmode[0]=='s'&&((0X41<=mode&&mode<=0X60)||mode=='p'||mode=='a'))
	{
		result=sigqueue(schpid,cbsnStepLeave,(union sigval)0);
		if(result==-1)
		{
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",schpid);
			return -1;
		}
		curmode[0]=mode;
		curmode[1]=0X00;
		funtIniStrSet("control","curmode",curmode);
		return 0;
	}

	if(curmode[0]=='m'&&mode=='m')
	{
		struct sbsnStage *stage;
		stage=&vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex];
		if(vbsnMemory->fmlscheme.stageindex==0&&stage->timeindex==0)
		{
			int vehmintime;
			funtIniIntGet("control","vehmintime",&vehmintime);
			time_t current;
			time(&current);
			int record;
			record=difftime(current,stage->times[0].start);
			if(record<vehmintime)
				return 100;
			result=sigqueue(schpid,cbsnManPress,(union sigval)0);
			if(result==-1)
			{
				muntLogError("sigqueue",errno,strerror(errno),"[%d]",schpid);
				return -1;
			}
		}
		else
		if(vbsnMemory->fmlscheme.stageindex==1&&stage->timeindex==0)
		{
			int manmintime;
			funtIniIntGet("control","manmintime",&manmintime);
			int manmaxtime;
			funtIniIntGet("control","manmaxtime",&manmaxtime);
			time_t current;
			time(&current);
			int record;
			record=difftime(current,stage->times[0].start);
			if(record>=manmaxtime)
				return 100;
			if(manmaxtime-record<manmintime)
				funtClkTimeSnd(manmaxtime-record);
			else
				funtClkTimeSnd(manmintime);
		}
		return 0;
	}

	if(0X41<=mode&&mode<=0X60)
	{
		scheme.isadapt=0;
		scheme.schemeindex=mode-0X40;
		fbsnSchemeSet(&scheme);
		fbsnNode(&scheme);
	}
	else
	if(mode=='p'||mode=='a')
	{
		result=fbsnSchemeGet(&scheme);
		if(result==100)
		{
			char backup[1+1];
			funtIniStrGet("control","backup",backup);
			scheme.schemeindex=backup[0]-0X40;
		}
		if(scheme.isfixed==0)
		{
			if(mode=='p')
				scheme.isadapt=0;

			fbsnSchemeSet(&scheme);
			fbsnNode(&scheme);
		}
	}
	else
	if(mode=='r'||mode=='y'||mode=='c'||mode=='m')
	{
		fbsnSpecialNode(&scheme,mode);
	}

	char isfixed;
	isfixed=vbsnMemory->fmlscheme.isfixed;

	funtLckLck("shm",1);
	vbsnMemory->flush=1;
	memcpy(&vbsnMemory->tmpscheme,&scheme,sizeof(struct sbsnScheme));
	funtLckUck("shm",1);

	if(isfixed==1||mode=='m'||mode=='r'||mode=='y'||mode=='c')
	{
		result=sigqueue(schpid,cbsnNextMode,(union sigval)0);
		if(result==-1)
		{
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",schpid);
			return -1;
		}
	}

	curmode[0]=mode;
	curmode[1]=0X00;
	funtIniStrSet("control","curmode",curmode);

	return 0;
}

/*========================================*\
    功能 : 生成协议
    参数 : (输入)串口序号
           (输入)来源代码
           (输入)目的代码
           (输入)消息类型
           (输入)消息代码
           (输入)方案数据
           (输出)消息结构
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnProt(char com,char src,char dst,char type,char func,struct sbsnScheme *scheme,struct suntComMsg *msg)
{
	msg->com=com;
	msg->src=src;
	msg->dst=dst;
	msg->type=type;
	msg->func=func;

	if(func==0XF1)
	{
		struct sbsnStage *stage;
		stage=&scheme->stages[scheme->stageindex];

		msg->data[0]=0X01;
		msg->data[1]=*((unsigned char*)&stage->lights[stage->drvstepindex]+2*(com-0X00)+1);
		msg->data[2]=*((unsigned char*)&stage->lights[stage->drvstepindex]+2*(com-0X00)+0);
		msg->data[3]=0X01;
		msg->data[4]=0X00;
		msg->data[5]=*((unsigned char*)&stage->flashs[stage->drvstepindex]+2*(com-0X00)+1);
		msg->data[6]=*((unsigned char*)&stage->flashs[stage->drvstepindex]+2*(com-0X00)+0);
		msg->data[7]=0X01;
		msg->data[8]=0X00;
		msg->data[9]=0X00;
		*(short*)&msg->size=10;
	}
	else
	if(func==0XFB)
	{
		struct sbsnStage *stage;
		stage=&scheme->stages[scheme->stageindex];

		msg->data[0]=*((unsigned char*)&stage->counts[stage->cntstepindex]+2*(com-0X00)+1);
		msg->data[1]=*((unsigned char*)&stage->counts[stage->cntstepindex]+2*(com-0X00)+0);
		*(short*)&msg->size=2;
	}
	else
	if(func==0X02)
	{
		struct sbsnStage *stage;
		stage=&scheme->stages[scheme->stageindex];

		memcpy(msg->data,&stage->colors[stage->lcdstepindex],8);

		char curmode[1+1];
		funtIniStrGet("control","curmode",curmode);
		if(curmode[0]=='r'||curmode[0]=='y'||curmode[0]=='c'||curmode[0]=='m')
		{
			if(curmode[0]=='r')
				msg->data[8]=0XFF;
			else
			if(curmode[0]=='y')
				msg->data[8]=0XFE;
			else
			if(curmode[0]=='c')
				msg->data[8]=0XFD;
			else
			if(curmode[0]=='m')
				msg->data[8]=0XFC;
		}
		else
		{
			if(0X41<=curmode[0]&&curmode[0]<=0X60)
				msg->data[8]=curmode[0]-0X40;
			else
			if(curmode[0]=='p')
				msg->data[8]=0X21;
			else
			if(curmode[0]=='a')
				msg->data[8]=0X22;
		}

		msg->data[ 9]=scheme->schemeindex;
		msg->data[10]=scheme->schemetotal;
		msg->data[11]=scheme->lcdstepcount;
		msg->data[12]=scheme->lcdstepindex+1;

		msg->data[13]=stage->lcdtimes[stage->lcdstepindex];
		msg->data[14]=stage->lcdtimes[stage->lcdstepindex];
		msg->data[15]=stage->maxgreen;
		msg->data[16]=stage->mingreen;

		*(short*)&msg->size=17;
	}
	else
	if(func==0XF8)
	{
		msg->data[0]=scheme->drvstepcount;

		int current=1;
		int i;
		for(i=0;i<scheme->stagecount;i++)
		{
			struct sbsnStage *stage;
			stage=&scheme->stages[i];
			int j;
			for(j=0;j<stage->drvstepcount;j++)
			{
				msg->data[current++]=*((unsigned char*)&stage->lights[j]+2*(com-0X00)+1);
				msg->data[current++]=*((unsigned char*)&stage->lights[j]+2*(com-0X00)+0);
				msg->data[current++]=stage->drvtimes[j];
				msg->data[current++]=0X00;
				msg->data[current++]=*((unsigned char*)&stage->flashs[j]+2*(com-0X00)+1);
				msg->data[current++]=*((unsigned char*)&stage->flashs[j]+2*(com-0X00)+0);
				msg->data[current++]=stage->drvtimes[j];
				msg->data[current++]=0X00;
				msg->data[current++]=0X00;
			}
		}
		*(short*)&msg->size=current;
	}

	return 0;
}

/*========================================*\
    功能 : 数据检查
    参数 : 
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnData()
{
	//行人清空时间<=车辆时间(常绿+绿闪+常黄)
	//倒数固定时间<=车辆时间(常绿+绿闪)
	//所有时间>=0
	//max>=min
	return 0;
}
