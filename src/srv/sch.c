/*========================================*\
    文件 : sch.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "unt.h"
#include "bsn.h"

/*========================================*\
    功能 : 调度服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fschDeamon(void)
{
	int result;

	funtMsgInit();

	int controller1;
	int controller2;
	int controller3;
	int controller4;

	int isnextstep=0;

	void hand(int id)
	{
		if(id==cbsnNextMode)
		{
			funtClkTimeSnd(0);
			controller1=0;
			isnextstep=0;
			return;
		}
		else
		if(id==cbsnNextTime)
		{
			controller2=0;
			return;
		}
		else
		if(id==cbsnStepEnter)
		{
			if(vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex].timeindex==0)
				controller3=0;
			else
				controller3=1;
			isnextstep=1;
		}
		else
		if(id==cbsnStepLeave)
		{
			if(vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex].timeindex==0)
				controller3=0;
			else
				controller3=1;
			isnextstep=0;
			return;
		}
		else
		if(id==cbsnManPress)
		{
			if(vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex].timeindex==0)
				controller4=0;
			else
				controller4=1;
			return;
		}
	}

	struct sigaction action;
	bzero(&action,sizeof(action));
	action.sa_handler=SIG_IGN;
	sigaction(cbsnNextMode,&action,NULL);
	sigaction(cbsnNextTime,&action,NULL);
	sigaction(cbsnStepEnter,&action,NULL);
	sigaction(cbsnStepLeave,&action,NULL);
	sigaction(cbsnManPress,&action,NULL);

	struct suntComMsg drvmessage0;
	struct suntComMsg drvmessage1;
	struct suntComMsg drvmessage2;
	struct suntComMsg drvmessage3;
	struct suntComMsg lcdmessage0;

	char drvstatus0='n';
	char drvstatus1='n';
	char drvstatus2='n';
	char drvstatus3='n';

	struct sbsnScheme scheme;

	char backup[1+1];
	funtIniStrGet("control","backup",backup);
	scheme.schemeindex=backup[0]-0X40;
	fbsnSchemeSet(&scheme);
	fbsnNode(&scheme);

	fbsnProt(0X00,0X01,0X05,0X01,0XF8,&scheme,&drvmessage0);
	fbsnProt(0X01,0X01,0X05,0X01,0XF8,&scheme,&drvmessage1);
	fbsnProt(0X02,0X01,0X05,0X01,0XF8,&scheme,&drvmessage2);
	fbsnProt(0X03,0X01,0X05,0X01,0XF8,&scheme,&drvmessage3);

	funtMsgCliSnd("drv",&drvmessage0,1,100,4);
	funtMsgCliSnd("drv",&drvmessage1,1,100,4);
	funtMsgCliSnd("drv",&drvmessage2,1,100,4);
	funtMsgCliSnd("drv",&drvmessage3,1,100,4);

	funtMsgCliRcv("drv",&drvmessage0,1,100,4);
	funtMsgCliRcv("drv",&drvmessage1,1,100,4);
	funtMsgCliRcv("drv",&drvmessage2,1,100,4);
	funtMsgCliRcv("drv",&drvmessage3,1,100,4);

	int ytime;
	funtIniIntGet("control","ytime",&ytime);
	if(ytime>0)
	{
		fbsnSpecialNode(&scheme,'y');

		fbsnProt(0X00,0X01,0X05,0X01,0XF1,&scheme,&drvmessage0);
		fbsnProt(0X01,0X01,0X05,0X01,0XF1,&scheme,&drvmessage1);
		fbsnProt(0X02,0X01,0X05,0X01,0XF1,&scheme,&drvmessage2);
		fbsnProt(0X03,0X01,0X05,0X01,0XF1,&scheme,&drvmessage3);

		funtMsgCliSnd("drv",&drvmessage0,1,100,4);
		funtMsgCliSnd("drv",&drvmessage1,1,100,4);
		funtMsgCliSnd("drv",&drvmessage2,1,100,4);
		funtMsgCliSnd("drv",&drvmessage3,1,100,4);

		funtMsgCliRcv("drv",&drvmessage0,1,100,4);
		funtMsgCliRcv("drv",&drvmessage1,1,100,4);
		funtMsgCliRcv("drv",&drvmessage2,1,100,4);
		funtMsgCliRcv("drv",&drvmessage3,1,100,4);

		sleep(ytime);
	}

	int rtime;
	funtIniIntGet("control","rtime",&rtime);
	if(rtime>0)
	{
		fbsnSpecialNode(&scheme,'r');

		fbsnProt(0X00,0X01,0X05,0X01,0XF1,&scheme,&drvmessage0);
		fbsnProt(0X01,0X01,0X05,0X01,0XF1,&scheme,&drvmessage1);
		fbsnProt(0X02,0X01,0X05,0X01,0XF1,&scheme,&drvmessage2);
		fbsnProt(0X03,0X01,0X05,0X01,0XF1,&scheme,&drvmessage3);

		funtMsgCliSnd("drv",&drvmessage0,1,100,4);
		funtMsgCliSnd("drv",&drvmessage1,1,100,4);
		funtMsgCliSnd("drv",&drvmessage2,1,100,4);
		funtMsgCliSnd("drv",&drvmessage3,1,100,4);

		funtMsgCliRcv("drv",&drvmessage0,1,100,4);
		funtMsgCliRcv("drv",&drvmessage1,1,100,4);
		funtMsgCliRcv("drv",&drvmessage2,1,100,4);
		funtMsgCliRcv("drv",&drvmessage3,1,100,4);

		sleep(rtime);
	}

	funtIniStrSet("control","aohmode","a");
	funtIniStrSet("control","curmode","u");
	char defmode[1+1];
	funtIniStrGet("control","defmode",defmode);
	fbsnMode(defmode[0]);

	action.sa_handler=hand;
	sigaction(cbsnNextMode,&action,NULL);
	sigaction(cbsnNextTime,&action,NULL);
	sigaction(cbsnStepEnter,&action,NULL);
	sigaction(cbsnStepLeave,&action,NULL);
	sigaction(cbsnManPress,&action,NULL);

	sigset_t newset;
	sigset_t oldset;
	sigemptyset(&newset);
	sigaddset(&newset,cbsnNextMode);
	sigaddset(&newset,cbsnNextTime);
	sigaddset(&newset,cbsnStepEnter);
	sigaddset(&newset,cbsnStepLeave);
	sigaddset(&newset,cbsnManPress);

	sigprocmask(SIG_SETMASK,NULL,&oldset);
	sigprocmask(SIG_SETMASK,&newset,NULL);

	while(1)
	{
		controller1=1;

		funtLckLck("shm",1);
		if(vbsnMemory->flush==1)
		{
			vbsnMemory->flush=0;
			memcpy(&vbsnMemory->fmlscheme,&vbsnMemory->tmpscheme,sizeof(struct sbsnScheme));
		}
		funtLckUck("shm",1);

		for
		(
			vbsnMemory->fmlscheme.stageindex=0,vbsnMemory->fmlscheme.drvstepindex=0,vbsnMemory->fmlscheme.lcdstepindex=0;
			controller1&&vbsnMemory->fmlscheme.stageindex<vbsnMemory->fmlscheme.stagecount;
			vbsnMemory->fmlscheme.stageindex++
		)
		{
			struct sbsnStage *stage;
			stage=&vbsnMemory->fmlscheme.stages[vbsnMemory->fmlscheme.stageindex];

			for
			(
				stage->timeindex=0,stage->drvstepindex=0,stage->lcdstepindex=0,stage->cntstepindex=0;
				controller1&&stage->timeindex<stage->timecount;
				stage->timeindex++
			)
			{
				controller2=1;
				controller3=1;
				controller4=1;

				time(&stage->times[stage->timeindex].start);

				int time=stage->times[stage->timeindex].time;

				if(time==-1)
					funtIniIntGet("control","manmintime",&time);

				if(isnextstep==1&&stage->timeindex==0)
					time=0;
				funtClkTimeSnd(time);

				if(stage->times[stage->timeindex].isdrvstep==1)
				{
					fbsnProt(0X00,0X01,0X05,0X01,0XF1,&vbsnMemory->fmlscheme,&drvmessage0);
					fbsnProt(0X01,0X01,0X05,0X01,0XF1,&vbsnMemory->fmlscheme,&drvmessage1);
					fbsnProt(0X02,0X01,0X05,0X01,0XF1,&vbsnMemory->fmlscheme,&drvmessage2);
					fbsnProt(0X03,0X01,0X05,0X01,0XF1,&vbsnMemory->fmlscheme,&drvmessage3);

					funtMsgCliSnd("drv",&drvmessage0,1,100,4);
					funtMsgCliSnd("drv",&drvmessage1,1,100,4);
					funtMsgCliSnd("drv",&drvmessage2,1,100,4);
					funtMsgCliSnd("drv",&drvmessage3,1,100,4);

					result=funtMsgCliRcv("drv",&drvmessage0,1,100,4);
					if(result==0&&drvstatus0=='a')
					{
						drvstatus0='n';
						funtIniStrSet("module","drv0","n");
					}
					if(result==100&&drvstatus0=='n')
					{
						drvstatus0='a';
						funtIniStrSet("module","drv0","a");
					}

					result=funtMsgCliRcv("drv",&drvmessage1,1,100,4);
					if(result==0&&drvstatus1=='a')
					{
						drvstatus1='n';
						funtIniStrSet("module","drv1","n");
					}
					if(result==100&&drvstatus1=='n')
					{
						drvstatus1='a';
						funtIniStrSet("module","drv1","a");
					}

					result=funtMsgCliRcv("drv",&drvmessage2,1,100,4);
					if(result==0&&drvstatus2=='a')
					{
						drvstatus2='n';
						funtIniStrSet("module","drv2","n");
					}
					if(result==100&&drvstatus2=='n')
					{
						drvstatus2='a';
						funtIniStrSet("module","drv2","a");
					}

					result=funtMsgCliRcv("drv",&drvmessage3,1,100,4);
					if(result==0&&drvstatus3=='a')
					{
						drvstatus3='n';
						funtIniStrSet("module","drv3","n");
					}
					if(result==100&&drvstatus3=='n')
					{
						drvstatus3='a';
						funtIniStrSet("module","drv3","a");
					}

					stage->drvstepindex++;
					vbsnMemory->fmlscheme.drvstepindex++;
				}

				if(stage->times[stage->timeindex].islcdstep==1)
				{
					fbsnProt(0X00,0X01,0X02,0X01,0X02,&vbsnMemory->fmlscheme,&lcdmessage0);
					funtMsgCliSnd("lcd",&lcdmessage0,1,100,4);

					stage->lcdstepindex++;
					vbsnMemory->fmlscheme.lcdstepindex++;
				}

				if(stage->times[stage->timeindex].iscntstep==1)
				{
					fbsnProt(0X00,0X01,0X05,0X01,0XFB,&vbsnMemory->fmlscheme,&drvmessage0);
					fbsnProt(0X01,0X01,0X05,0X01,0XFB,&vbsnMemory->fmlscheme,&drvmessage1);
					fbsnProt(0X02,0X01,0X05,0X01,0XFB,&vbsnMemory->fmlscheme,&drvmessage2);
					fbsnProt(0X03,0X01,0X05,0X01,0XFB,&vbsnMemory->fmlscheme,&drvmessage3);

					funtMsgCliSnd("drv",&drvmessage0,1,100,4);
					funtMsgCliSnd("drv",&drvmessage1,1,100,4);
					funtMsgCliSnd("drv",&drvmessage2,1,100,4);
					funtMsgCliSnd("drv",&drvmessage3,1,100,4);

					result=funtMsgCliRcv("drv",&drvmessage0,1,100,4);
					if(result==0&&drvstatus0=='a')
					{
						drvstatus0='n';
						funtIniStrSet("module","drv0","n");
					}
					if(result==100&&drvstatus0=='n')
					{
						drvstatus0='a';
						funtIniStrSet("module","drv0","a");
					}

					result=funtMsgCliRcv("drv",&drvmessage1,1,100,4);
					if(result==0&&drvstatus1=='a')
					{
						drvstatus1='n';
						funtIniStrSet("module","drv1","n");
					}
					if(result==100&&drvstatus1=='n')
					{
						drvstatus1='a';
						funtIniStrSet("module","drv1","a");
					}

					result=funtMsgCliRcv("drv",&drvmessage2,1,100,4);
					if(result==0&&drvstatus2=='a')
					{
						drvstatus2='n';
						funtIniStrSet("module","drv2","n");
					}
					if(result==100&&drvstatus2=='n')
					{
						drvstatus2='a';
						funtIniStrSet("module","drv2","a");
					}

					result=funtMsgCliRcv("drv",&drvmessage3,1,100,4);
					if(result==0&&drvstatus3=='a')
					{
						drvstatus3='n';
						funtIniStrSet("module","drv3","n");
					}
					if(result==100&&drvstatus3=='n')
					{
						drvstatus3='a';
						funtIniStrSet("module","drv3","a");
					}

					stage->cntstepindex++;
				}

				while(controller1&&controller2&&controller3&&controller4)
					sigsuspend(&oldset);
			}
		}
	}

	return 0;
}
