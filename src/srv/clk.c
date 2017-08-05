/*========================================*\
    文件 : clk.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "unt.h"
#include "bsn.h"

/*========================================*\
    功能 : 定时服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fclkDeamon(void)
{
	int result;

	timer_t timer;
	void hand(int id)
	{
		int schpid;
		funtIniIntGet("deamon","sch",&schpid);
		result=sigqueue(schpid,cbsnNextTime,(union sigval)0);
		if(result==-1)
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",schpid);
		return;
	}

	struct sigaction action;
	bzero(&action,sizeof(action));
	action.sa_handler=hand;
	result=sigaction(cbsnNextTime,&action,NULL);
	if(result!=0)
	{
		muntLogError("sigaction",errno,strerror(errno),"[]");
		return -1;
	}

	struct sigevent event;
	event.sigev_notify=SIGEV_SIGNAL;
	event.sigev_signo=cbsnNextTime;
	result=timer_create(CLOCK_REALTIME,&event,&timer);
	if(result==-1)
	{
		muntLogError("timer_create",errno,strerror(errno),"[]");
		return -1;
	}

	while(1)
	{
		struct suntClkMsg msg;
		do
			result=msgrcv(vuntClkMid,&msg,sizeof(msg)-sizeof(long),1,0);
		while(result==-1&&errno==EINTR);
		if(result==-1&&errno!=EINTR)
		{
			muntLogError("msgrcv",errno,strerror(errno),"[%d]",vuntClkMid);
			return -1;
		}

		if(msg.order>=0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			it.it_value.tv_sec=msg.order;
			result=timer_settime(timer,0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}
		else
		if(msg.order<0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_gettime(timer,&it);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
			msg.index=2;
			msg.order=it.it_value.tv_sec;
			result=msgsnd(vuntClkMid,&msg,sizeof(msg)-sizeof(long),0);
			if(result==-1)
			{
				muntLogError("msgsnd",errno,strerror(errno),"[%d]",vuntClkMid);
				return -1;
			}
		}
	}

	return 0;
}
