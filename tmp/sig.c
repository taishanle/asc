/*========================================*\
    文件 : sig.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <setjmp.h>

timer_t vascMsgTimer;
int vascMsgIsTimeout;

#define cascMsgTimeout (SIGRTMAX-0)

/*
void fascMsgHand(int id)
{
	vascMsgIsTimeout=1;
	return;
}

int fascMsgInit(void)
{
	int result;

	struct sigaction action;
	bzero(&action,sizeof(action));
	result=sigfillset(&action.sa_mask);
	if(result==-1)
	{
		//mascLogError("sigemptyset",errno,strerror(errno),"[]");
		return -1;
	}
	action.sa_handler=fascMsgHand;
	result=sigaction(cascMsgTimeout,&action,NULL);
	if(result==-1)
	{
		//mascLogError("sigaction",errno,strerror(errno),"[]");
		return -1;
	}

	struct sigevent event;
	event.sigev_notify=SIGEV_SIGNAL;
	event.sigev_signo=cascMsgTimeout;
	result=timer_create(CLOCK_MONOTONIC,&event,&vascMsgTimer);
	if(result==-1)
	{
		//mascLogError("timer_create",errno,strerror(errno),"[]");
		return -1;
	}

	return 0;
}
*/

/*
jmp_buf jump;
int times;
void hand0(int id)
{
	times++;
	while(1)
	{
		printf("%d:abcd\n",times);
		usleep(100000);
	}
}
void hand1(int id)
{
	printf("efgh\n");
	siglongjmp(jump,1);
}

int main(void)
{
	int result;

	result=sigsetjmp(jump,1);
	switch(result)
	{
		case 1:
		goto position1;
	}

	struct sigaction action;
	bzero(&action,sizeof(action));
	action.sa_handler=hand0;
	result=sigaction(SIGRTMIN+0,&action,NULL);
	if(result==-1)
	{
		//mascLogError("sigaction",errno,strerror(errno),"[]");
		return -1;
	}
	bzero(&action,sizeof(action));
	action.sa_handler=hand1;
	result=sigaction(SIGRTMIN+1,&action,NULL);
	if(result==-1)
	{
		//mascLogError("sigaction",errno,strerror(errno),"[]");
		return -1;
	}

//	struct sigevent event;
//	event.sigev_notify=SIGEV_SIGNAL;
//	event.sigev_signo=cascMsgTimeout;
//	result=timer_create(CLOCK_MONOTONIC,&event,&vascMsgTimer);
//	if(result==-1)
//	{
//		//mascLogError("timer_create",errno,strerror(errno),"[]");
//		return -1;
//	}
//
//	struct itimerspec it;
//	bzero(&it,sizeof(it));
//	//it.it_value.tv_sec=1;
//	it.it_value.tv_nsec=100000000;
//	result=timer_settime(vascMsgTimer,0,&it,NULL);
//	if(result==-1)
//	{
//		//mascLogError("timer_settime",errno,strerror(errno),"[]");
//		return -1;
//	}

	sigqueue(getpid(),SIGRTMIN+1,(union sigval)0);
	printf("0000\n");

	while(1)
	{
		printf("1234\n");
		pause();
		position1:
		printf("5678\n");
	}
}
*/

int main(void)
{
	/*
	struct sigaction action;
	bzero(&action,sizeof(action));

	void hand(int id)
	{
		printf("11111111111111111\n");
	}

	action.sa_handler=hand;
	sigaction(SIGRTMIN,&action,NULL);

	sigset_t newset;
	sigset_t oldset;
	sigemptyset(&newset);
	sigaddset(&newset,SIGRTMIN);

	while(1)
	{
		sigprocmask(SIG_SETMASK,&newset,&oldset);
		printf("in block\n");
		sleep(4);
		sigprocmask(SIG_SETMASK,&oldset,NULL);
		printf("out block\n");
		sleep(4);
	}
	*/
	printf("%d",SIGRTMIN);
	return 0;
}
