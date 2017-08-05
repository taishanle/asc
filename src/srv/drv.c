/*========================================*\
    文件 : drv.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "unt.h"
#include "bsn.h"

/*========================================*\
    功能 : 驱动板服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdrvDeamon(void)
{
	int result;

	funtMsgInit();

	int times;
	funtIniIntGet("develop","times",&times);
	int inter;
	funtIniIntGet("develop","inter",&inter);

	funtIniStrSet("module","drv0","n");
	funtIniStrSet("module","drv1","n");
	funtIniStrSet("module","drv2","n");
	funtIniStrSet("module","drv3","n");

	int miss0=0;
	int miss1=0;
	int miss2=0;
	int miss3=0;

	while(1)
	{
		struct suntComMsg msg;
		result=funtMsgSrvRcv("drv",&msg,1,1000*inter,1);
		if(result==100)
		{
			msg.type=0X01;
			msg.func=0X01;
			msg.data[0]=0X00;
			msg.size[0]=0X01;
			msg.size[1]=0X00;
			msg.src=0X01;

			msg.com=0X00;
			msg.dst=0X05;
			funtMsgCliSnd("drv",&msg,1,100,4);
			msg.com=0X01;
			msg.dst=0X05;
			funtMsgCliSnd("drv",&msg,1,100,4);
			msg.com=0X02;
			msg.dst=0X05;
			funtMsgCliSnd("drv",&msg,1,100,4);
			msg.com=0X03;
			msg.dst=0X05;
			funtMsgCliSnd("drv",&msg,1,100,4);

			msg.com=0X00;
			msg.dst=0X05;
			result=funtMsgCliRcv("drv",&msg,1,100,4);
			if(result==100)
			{
				if(miss0<times+1)
					miss0++;
				if(miss0==times)
					funtIniStrSet("module","drv0","a");
			}
			else
			{
				if(miss0!=0)
				{
					funtIniStrSet("module","drv0","n");
					miss0=0;
				}
			}

			msg.com=0X01;
			msg.dst=0X05;
			result=funtMsgCliRcv("drv",&msg,1,100,4);
			if(result==100)
			{
				if(miss1<times+1)
					miss1++;
				if(miss1==times)
					funtIniStrSet("module","drv1","a");
			}
			else
			{
				if(miss1!=0)
				{
					funtIniStrSet("module","drv1","n");
					miss1=0;
				}
			}

			msg.com=0X02;
			msg.dst=0X05;
			result=funtMsgCliRcv("drv",&msg,1,100,4);
			if(result==100)
			{
				if(miss2<times+1)
					miss2++;
				if(miss2==times)
					funtIniStrSet("module","drv2","a");
			}
			else
			{
				if(miss2!=0)
				{
					funtIniStrSet("module","drv2","n");
					miss2=0;
				}
			}

			msg.com=0X03;
			msg.dst=0X05;
			result=funtMsgCliRcv("drv",&msg,1,100,4);
			if(result==100)
			{
				if(miss3<times+1)
					miss3++;
				if(miss3==times)
					funtIniStrSet("module","drv3","a");
			}
			else
			{
				if(miss3!=0)
				{
					funtIniStrSet("module","drv3","n");
					miss3=0;
				}
			}
		}
		else
		{
			msg.type=0X02;
			msg.src=0X01;
			msg.dst=0X05;

			switch(msg.func)
			{
				default: continue;
			}

			funtMsgSrvSnd("drv",&msg,1,100,4);
		}
	}

	return 0;
}
