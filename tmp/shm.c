/*========================================*\
    文件 : shm.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//定时数据.
struct sbsnTime
{
	//步骤时间.
	int time;
	//是否驱动板步骤.
	int isdrvstep;
	//是否液晶屏步骤.
	int islcdstep;
	//是否倒计时步骤.
	int iscntstep;
	//是否常绿步骤.
	int isgreen;
	//是否感应步骤.
	int isadapt;
	//开始时间.
	time_t start;
};

//阶段数据.
struct sbsnStage
{
	//相位位图.
	char phase[32];

	//常绿时间.
	int greenlight;

	//绿闪时间.
	int greenflash;
	//常黄时间.
	int yellow;
	//常红时间.
	int red;

	//行人常亮时间.
	int manlight;
	//行人闪烁时间.
	int manflash;

	//单位延长时间.
	int delta;
	//最小绿灯时间.
	int mingreen;
	//最大绿灯时间.
	int maxgreen;

	//时间总数.
	int timecount;
	//时间序号.
	int timeindex;

	//驱动板步骤序号.
	int drvstepindex;
	//液晶屏步骤序号.
	int lcdstepindex;
	//倒计时步骤序号.
	int cntstepindex;

	//驱动板步骤序号.
	int drvstepcount;
	//液晶屏步骤序号.
	int lcdstepcount;
	//倒计时步骤序号.
	int cntstepcount;

	//定时数据.
	struct sbsnTime times[10];
	//驱动板步骤数据:常亮点位状态.
	unsigned long long lights[5];
	//驱动板步骤数据:闪烁点位状态.
	unsigned long long flashs[5];
	//液晶屏步骤数据:通道点位状态.
	unsigned long long colors[3];
	//倒计时步骤数据:倒数点位状态.
	unsigned long long counts[2];
	//驱动板步骤数据:步骤时间.
	int drvtimes[5];
	//液晶屏步骤数据:步骤时间.
	int lcdtimes[3];
};

//方案数据.
struct sbsnScheme
{
	//是否感应.
	int isadapt;
	//是否固定.
	int isfixed;

	//计划序号.
	int planindex;
	//时段表号.
	int periodtable;
	//时段序号.
	int periodindex;

	//方案序号.
	int schemeindex;
	//方案周期(用于统计感应模式).
	int schemecycle;
	//方案周期(实际所有阶段总和).
	int schemetotal;

	//阶段表号.
	int stagetable;
	//阶段序号.
	int stageindex;
	//阶段总数.
	int stagecount;

	//驱动板步骤总数.
	int drvstepcount;
	//驱动板步骤序号.
	int drvstepindex;
	//液晶屏步骤总数.
	int lcdstepcount;
	//液晶屏步骤序号.
	int lcdstepindex;

	//阶段数据.
	struct sbsnStage stages[16];
};

struct sbsnMemory
{
	//刷新标志.
	int flush;
	//临时方案数据.
	struct sbsnScheme tmpscheme;
	//正式方案数据.
	struct sbsnScheme fmlscheme;
	//临时车检器数据.
	int tmpvehinfo[32];
	//正式车检器数据.
	int fmlvehinfo[32];
};
struct sbsnMemory *vbsnMemory;

int main(void)
{
	int result;

	int sid;
	sid=shmget(10000,0,0);
	if(sid==-1)
	{
		fprintf(stderr,"shmget failed!\n");
		return -1;
	}
	//printf("%d\n",sid);
	//printf("%d\n",errno);
	//printf("%s\n",strerror(errno));

	//printf("%d\n",EACCES);
	//printf("%d\n",EEXIST);
	//printf("%d\n",EINVAL);
	//printf("%d\n",EINVAL);
	//printf("%d\n",ENFILE);
	//printf("%d\n",ENOENT);
	//printf("%d\n",ENOMEM);
	//printf("%d\n",ENOSPC);
	//printf("%d\n",EPERM );

	struct shmid_ds info;
	result=shmctl(sid,IPC_STAT,&info);
	if(result==-1)
	{
		fprintf(stderr,"shmctl failed!\n");
		return -1;
	}
	printf("%d\n",info.shm_segsz);
	printf("%d\n",sizeof(struct sbsnMemory));

	vbsnMemory=(struct sbsnMemory*)shmat(sid,0,0);
	if(vbsnMemory==(struct sbsnMemory*)-1)
	{
		fprintf(stderr,"shmat failed!\n");
		return -1;
	}
	printf("flush:%d\n",vbsnMemory->flush);

	{
		struct sbsnScheme *scheme;
		scheme=&vbsnMemory->tmpscheme;

		printf("isadapt[%d]\n",scheme->isadapt);
		printf("isfixed[%d]\n",scheme->isfixed);
		printf("planindex[%d]\n",scheme->planindex);
		printf("periodtable[%d]\n",scheme->periodtable);
		printf("periodindex[%d]\n",scheme->periodindex);
		printf("schemeindex[%d]\n",scheme->schemeindex);
		printf("schemecycle[%d]\n",scheme->schemecycle);
		printf("schemetotal[%d]\n",scheme->schemetotal);
		printf("stagetable[%d]\n",scheme->stagetable);
		printf("stagecount[%d]\n",scheme->stagecount);
		printf("stageindex[%d]\n",scheme->stageindex);
		printf("drvstepcount[%d]\n",scheme->drvstepcount);
		printf("drvstepindex[%d]\n",scheme->drvstepindex);
		printf("lcdstepcount[%d]\n",scheme->lcdstepcount);
		printf("lcdstepindex[%d]\n",scheme->lcdstepindex);
		int m;
		for(m=0;m<scheme->stagecount;m++)
		{
			printf("stage:%d,phase[%.32s]\n",m,scheme->stages[m].phase);
			printf("stage:%d,greenligth[%d]\n",m,scheme->stages[m].greenlight);
			printf("stage:%d,greenflash[%d]\n",m,scheme->stages[m].greenflash);
			printf("stage:%d,yellow[%d]\n",m,scheme->stages[m].yellow);
			printf("stage:%d,red[%d]\n",m,scheme->stages[m].red);
			printf("stage:%d,manlight[%d]\n",m,scheme->stages[m].manlight);
			printf("stage:%d,manflash[%d]\n",m,scheme->stages[m].manflash);
			printf("stage:%d,delta[%d]\n",m,scheme->stages[m].delta);
			printf("stage:%d,mingreen[%d]\n",m,scheme->stages[m].mingreen);
			printf("stage:%d,maxgreen[%d]\n",m,scheme->stages[m].maxgreen);
			printf("stage:%d,timecount[%d]\n",m,scheme->stages[m].timecount);
			printf("stage:%d,timeindex[%d]\n",m,scheme->stages[m].timeindex);
			printf("stage:%d,drvstepcount[%d]\n",m,scheme->stages[m].drvstepcount);
			printf("stage:%d,lcdstepcount[%d]\n",m,scheme->stages[m].lcdstepcount);
			printf("stage:%d,cntstepcount[%d]\n",m,scheme->stages[m].cntstepcount);
			printf("stage:%d,drvstepindex[%d]\n",m,scheme->stages[m].drvstepindex);
			printf("stage:%d,lcdstepindex[%d]\n",m,scheme->stages[m].lcdstepindex);
			printf("stage:%d,cntstepindex[%d]\n",m,scheme->stages[m].cntstepindex);
			int n;
			for(n=0;n<scheme->stages[m].timecount;n++)
			{
				printf("stage:%d,time:%d,time[%d]\n",m,n,scheme->stages[m].times[n].time);
				printf("stage:%d,time:%d,isdrvstep[%d]\n",m,n,scheme->stages[m].times[n].isdrvstep);
				printf("stage:%d,time:%d,islcdstep[%d]\n",m,n,scheme->stages[m].times[n].islcdstep);
				printf("stage:%d,time:%d,iscntstep[%d]\n",m,n,scheme->stages[m].times[n].iscntstep);
				printf("stage:%d,time:%d,isgreen[%d]\n",m,n,scheme->stages[m].times[n].isgreen);
				printf("stage:%d,time:%d,isadapt[%d]\n",m,n,scheme->stages[m].times[n].isadapt);
			}
			for(n=0;n<5;n++)
			{
				printf("stage:%d,drvstep:%d,lights[%llX]\n",m,n,scheme->stages[m].lights[n]);
				printf("stage:%d,drvstep:%d,flashs[%llX]\n",m,n,scheme->stages[m].flashs[n]);
				printf("stage:%d,drvstep:%d,drvtimes[%d]\n",m,n,scheme->stages[m].drvtimes[n]);
			}
			for(n=0;n<3;n++)
			{
				printf("stage:%d,lcdstep:%d,colors[%llX]\n",m,n,scheme->stages[m].colors[n]);
				printf("stage:%d,lcdstep:%d,lcdtimes[%d]\n",m,n,scheme->stages[m].lcdtimes[n]);
			}
			for(n=0;n<2;n++)
			{
				printf("stage:%d,cntstep:%d,counts[%llX]\n",m,n,scheme->stages[m].counts[n]);
			}
		}
	}

	printf("-------------------------------------------------------------\n");

	{
		struct sbsnScheme *scheme;
		scheme=&vbsnMemory->fmlscheme;

		printf("isadapt[%d]\n",scheme->isadapt);
		printf("isfixed[%d]\n",scheme->isfixed);
		printf("planindex[%d]\n",scheme->planindex);
		printf("periodtable[%d]\n",scheme->periodtable);
		printf("periodindex[%d]\n",scheme->periodindex);
		printf("schemeindex[%d]\n",scheme->schemeindex);
		printf("schemecycle[%d]\n",scheme->schemecycle);
		printf("schemetotal[%d]\n",scheme->schemetotal);
		printf("stagetable[%d]\n",scheme->stagetable);
		printf("stagecount[%d]\n",scheme->stagecount);
		printf("stageindex[%d]\n",scheme->stageindex);
		printf("drvstepcount[%d]\n",scheme->drvstepcount);
		printf("drvstepindex[%d]\n",scheme->drvstepindex);
		printf("lcdstepcount[%d]\n",scheme->lcdstepcount);
		printf("lcdstepindex[%d]\n",scheme->lcdstepindex);
		int m;
		for(m=0;m<scheme->stagecount;m++)
		{
			printf("stage:%d,phase[%.32s]\n",m,scheme->stages[m].phase);
			printf("stage:%d,greenligth[%d]\n",m,scheme->stages[m].greenlight);
			printf("stage:%d,greenflash[%d]\n",m,scheme->stages[m].greenflash);
			printf("stage:%d,yellow[%d]\n",m,scheme->stages[m].yellow);
			printf("stage:%d,red[%d]\n",m,scheme->stages[m].red);
			printf("stage:%d,manlight[%d]\n",m,scheme->stages[m].manlight);
			printf("stage:%d,manflash[%d]\n",m,scheme->stages[m].manflash);
			printf("stage:%d,delta[%d]\n",m,scheme->stages[m].delta);
			printf("stage:%d,mingreen[%d]\n",m,scheme->stages[m].mingreen);
			printf("stage:%d,maxgreen[%d]\n",m,scheme->stages[m].maxgreen);
			printf("stage:%d,timecount[%d]\n",m,scheme->stages[m].timecount);
			printf("stage:%d,timeindex[%d]\n",m,scheme->stages[m].timeindex);
			printf("stage:%d,drvstepcount[%d]\n",m,scheme->stages[m].drvstepcount);
			printf("stage:%d,lcdstepcount[%d]\n",m,scheme->stages[m].lcdstepcount);
			printf("stage:%d,cntstepcount[%d]\n",m,scheme->stages[m].cntstepcount);
			printf("stage:%d,drvstepindex[%d]\n",m,scheme->stages[m].drvstepindex);
			printf("stage:%d,lcdstepindex[%d]\n",m,scheme->stages[m].lcdstepindex);
			printf("stage:%d,cntstepindex[%d]\n",m,scheme->stages[m].cntstepindex);
			int n;
			for(n=0;n<scheme->stages[m].timecount;n++)
			{
				printf("stage:%d,time:%d,time[%d]\n",m,n,scheme->stages[m].times[n].time);
				printf("stage:%d,time:%d,isdrvstep[%d]\n",m,n,scheme->stages[m].times[n].isdrvstep);
				printf("stage:%d,time:%d,islcdstep[%d]\n",m,n,scheme->stages[m].times[n].islcdstep);
				printf("stage:%d,time:%d,iscntstep[%d]\n",m,n,scheme->stages[m].times[n].iscntstep);
				printf("stage:%d,time:%d,isgreen[%d]\n",m,n,scheme->stages[m].times[n].isgreen);
				printf("stage:%d,time:%d,isadapt[%d]\n",m,n,scheme->stages[m].times[n].isadapt);
			}
			for(n=0;n<5;n++)
			{
				printf("stage:%d,drvstep:%d,lights[%llX]\n",m,n,scheme->stages[m].lights[n]);
				printf("stage:%d,drvstep:%d,flashs[%llX]\n",m,n,scheme->stages[m].flashs[n]);
				printf("stage:%d,drvstep:%d,drvtimes[%d]\n",m,n,scheme->stages[m].drvtimes[n]);
			}
			for(n=0;n<3;n++)
			{
				printf("stage:%d,lcdstep:%d,colors[%llX]\n",m,n,scheme->stages[m].colors[n]);
				printf("stage:%d,lcdstep:%d,lcdtimes[%d]\n",m,n,scheme->stages[m].lcdtimes[n]);
			}
			for(n=0;n<2;n++)
			{
				printf("stage:%d,cntstep:%d,counts[%llX]\n",m,n,scheme->stages[m].counts[n]);
			}
		}
	}

	printf("-------------------------------------------------------------\n");

	printf("vehinfo:\n");
	int i;
	for(i=0;i<32;i++)
		printf("%d:%d\t",i+1,vbsnMemory->tmpvehinfo[i]);
	printf("\n");
	for(i=0;i<32;i++)
		printf("%d:%d\t",i+1,vbsnMemory->fmlvehinfo[i]);
	printf("\n");

	shmdt(vbsnMemory);

	return 0;
}
