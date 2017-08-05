/*========================================*\
    文件 : crn.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "unt.h"
#include "bsn.h"

/*========================================*\
    功能 : 定时服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fcrnDeamon(void)
{
	int result;

	while(1)
	{
		time_t stamp;
		stamp=time(NULL);
		if(stamp==(time_t)-1)
		{
			muntLogError("time",errno,strerror(errno),"[]");
			return -1;
		}
		struct tm *tm;
		tm=localtime(&stamp);
		if(tm==NULL)
		{
			muntLogError("localtime",errno,strerror(errno),"[]");
			return -1;
		}
		sleep(60-tm->tm_sec);

		memcpy(vbsnMemory->fmlvehinfo,vbsnMemory->tmpvehinfo,sizeof(vbsnMemory->tmpvehinfo));
		memset(vbsnMemory->tmpvehinfo,0,sizeof(vbsnMemory->tmpvehinfo));

		system("hwclock -s");

		result=fbsnPlan();
		if(result==100)
			continue;

		char curmode[1+1];
		funtIniStrGet("control","curmode",curmode);
		if(curmode[0]=='p'||curmode[0]=='a')
			fbsnMode(curmode[0]);
	}

	return 0;
}
