/*========================================*\
    文件 : time.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <errno.h>

int main(void)
{
	int result;
	struct tm tm;
	bzero(&tm,sizeof(tm));
	tm.tm_year=17+2000-1900;
	tm.tm_mon =3-1;
	tm.tm_mday=23;
	tm.tm_hour=10;
	tm.tm_min =0;
	tm.tm_sec =0;

	struct timeval tv;
	tv.tv_sec=mktime(&tm);
	if(tv.tv_sec==-1)
	{
		printf("%d-%s\n",errno,strerror(errno));
		return -1;
	}
	tv.tv_usec=0;
	result=settimeofday(&tv,NULL);
	if(result==-1)
	{
		printf("%d-%s\n",errno,strerror(errno));
		return -1;
	}

	int rtc;
	rtc=open("/dev/rtc",O_WRONLY);
	if(rtc==-1)
	{
		printf("%d-%s\n",errno,strerror(errno));
		return -1;
	}
	result=ioctl(rtc,RTC_SET_TIME,&tm);
	if(result==-1)
	{
		printf("%d-%s\n",errno,strerror(errno));
		return -1;
	}
	close(rtc);

	return 0;
}
