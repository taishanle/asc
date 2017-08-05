/*========================================*\
    文件 : clk.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>

#include <sys/ipc.h>
#include <sys/shm.h>

timer_t *timer;
void hand(int id)
{
	printf("1234\n");
}

int main(void)
{
	int result;
	struct tm now;
	bzero(&now,sizeof(now));
	now.tm_year=2017-1900;
	now.tm_mon=1-1;
	now.tm_mday=12;
	now.tm_hour=15;
	now.tm_min=30;
	now.tm_sec=0;
	int id;
	id=open("/dev/rtc",O_RDWR);
	if(id==-1)
	{
		fprintf(stderr,"open failed!\n");
		return -1;
	}
	result=ioctl(id,RTC_SET_TIME,&now);
	if(result==-1)
	{
		fprintf(stderr,"ioctl failed!\n");
		return -1;
	}
	close(id);
	system("hwclock -s");
	return 0;
	/*
	int result;
	int sid;
	sid=shmget(10000,8192,IPC_CREAT|0600);
	if(sid==-1)
	{
		//muntLogError("shmget",errno,strerror(errno),"[]");
		fprintf(stderr,"shmget failed!\n");
		return -1;
	}
	timer=(timer_t*)shmat(sid,0,0);
	if(timer==(timer_t*)-1)
	{
		//muntLogError("shmat",errno,strerror(errno),"[]");
		fprintf(stderr,"shmat failed!\n");
		return -1;
	}

	result=fork();
	if(result>0)
	{
		struct sigaction action;
		bzero(&action,sizeof(action));
		action.sa_handler=hand;
		result=sigaction(SIGRTMIN,&action,NULL);
		if(result!=0)
		{
			//muntLogError("sigaction",errno,strerror(errno),"[]");
			fprintf(stderr,"sigaction failed!\n");
			return -1;
		}
		struct sigevent event;
		event.sigev_notify=SIGEV_SIGNAL;
		event.sigev_signo=SIGRTMIN;
		result=timer_create(CLOCK_REALTIME,&event,timer);
		if(result==-1)
		{
			//muntLogError("timer_create",errno,strerror(errno),"[]");
			fprintf(stderr,"timer_create failed!\n");
			return -1;
		}
		while(1)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_gettime(timer,&it);
			if(result==-1)
			{
				//muntLogError("timer_settime",errno,strerror(errno),"[]");
				fprintf(stderr,"timer_gettime failed!\n");
				return -1;
			}
			printf("father:%d\n",it.it_value.tv_sec);
			usleep(100000);
		}
	}
	else
	if(result==0)
	{
		struct sigevent event;
		event.sigev_notify=SIGEV_SIGNAL;
		event.sigev_signo=SIGRTMIN;
		result=timer_create(CLOCK_REALTIME,&event,timer);
		if(result==-1)
		{
			//muntLogError("timer_create",errno,strerror(errno),"[]");
			fprintf(stderr,"timer_create failed!\n");
			return -1;
		}
		struct itimerspec it;
		bzero(&it,sizeof(it));
		it.it_value.tv_sec=2;
		it.it_value.tv_nsec=0;
		result=timer_settime(*timer,0,&it,NULL);
		if(result==-1)
		{
			//muntLogError("timer_settime",errno,strerror(errno),"[]");
			fprintf(stderr,"timer_settime failed!\n");
			fprintf(stderr,"%d-%s",errno,strerror(errno));
			return -1;
		}
		while(1)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_gettime(timer,&it);
			if(result==-1)
			{
				//muntLogError("timer_settime",errno,strerror(errno),"[]");
				fprintf(stderr,"timer_gettime failed!\n");
				return -1;
			}
			printf("son:%d\n",it.it_value.tv_sec);
			usleep(100000);
		}
	}
	return 0;
	*/
}
