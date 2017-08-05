/*========================================*\
    文件 : asc.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "unt.h"
#include "bsn.h"

int fschDeamon(void);
int fcrnDeamon(void);
int fclkDeamon(void);
int fdrvDeamon(void);
int flcdDeamon(void);
int fintDeamon(void);
int fvehDeamon(void);
int fcomDeamon(char *module);

int main(int argc,char *argv[])
{
	int result;

	funtLckInit("ini");
	funtLckInit("shm");

	struct option options[]=
	{
		{"boot",no_argument,NULL,'b'},
		{"shut",no_argument,NULL,'s'},
		{"list",no_argument,NULL,'l'},
		{"help",no_argument,NULL,'h'},
		{0,0,0,0}
	};

	int option;
	option=getopt_long(argc,argv,":bslh",options,NULL);

	switch(option)
	{
		case 'b':
		result=fork();
		if(result==-1)
		{
			muntLogError("fork",errno,strerror(errno),"[]");
			return -1;
		}
		if(result>0)
			return 0;

		char ip[16];
		funtIniStrGet("net","ip",ip);
		char mask[16];
		funtIniStrGet("net","mask",mask);
		char gate[16];
		funtIniStrGet("net","gate",gate);
		char mac[32];
		funtIniStrGet("net","mac",mac);
		funtNetSet(ip,mask,gate,mac);

		funtIniIntSet("deamon","sch",0);
		funtIniIntSet("deamon","crn",0);
		funtIniIntSet("deamon","clk",0);
		funtIniIntSet("deamon","drv",0);
		funtIniIntSet("deamon","lcd",0);
		funtIniIntSet("deamon","int",0);
		funtIniIntSet("deamon","veh",0);
		funtIniIntSet("deamon","comdrvsnd",0);
		funtIniIntSet("deamon","comdrvrcv",0);
		funtIniIntSet("deamon","comlcdsnd",0);
		funtIniIntSet("deamon","comlcdrcv",0);
		funtIniIntSet("deamon","comintsnd",0);
		funtIniIntSet("deamon","comintrcv",0);

		result=funtShmCreate();
		if(result==-1)
			return -1;

		result=funtMsgCreate("drv");
		if(result==-1)
			return -1;
		result=funtMsgCreate("lcd");
		if(result==-1)
			return -1;
		result=funtMsgCreate("int");
		if(result==-1)
			return -1;
		result=funtMsgCreate("clk");
		if(result==-1)
			return -1;

		result=funtSrvBoot("comdrvsnd",fcomDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("comdrvrcv",fcomDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("comlcdsnd",fcomDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("comlcdrcv",fcomDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("comintsnd",fcomDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("comintrcv",fcomDeamon);
		if(result==-1)
			return -1;

		result=funtSrvBoot("clk",fclkDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("sch",fschDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("crn",fcrnDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("drv",fdrvDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("lcd",flcdDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("int",fintDeamon);
		if(result==-1)
			return -1;
		result=funtSrvBoot("veh",fvehDeamon);
		if(result==-1)
			return -1;

		break;

		case 's':
		funtSrvShut("veh");
		funtSrvShut("int");
		funtSrvShut("lcd");
		funtSrvShut("drv");
		funtSrvShut("crn");
		funtSrvShut("sch");
		funtSrvShut("clk");
		funtSrvShut("comintrcv");
		funtSrvShut("comintsnd");
		funtSrvShut("comlcdrcv");
		funtSrvShut("comlcdsnd");
		funtSrvShut("comdrvrcv");
		funtSrvShut("comdrvsnd");

		funtIniIntSet("deamon","veh",0);
		funtIniIntSet("deamon","int",0);
		funtIniIntSet("deamon","lcd",0);
		funtIniIntSet("deamon","drv",0);
		funtIniIntSet("deamon","crn",0);
		funtIniIntSet("deamon","sch",0);
		funtIniIntSet("deamon","clk",0);
		funtIniIntSet("deamon","comintrcv",0);
		funtIniIntSet("deamon","comintsnd",0);
		funtIniIntSet("deamon","comlcdrcv",0);
		funtIniIntSet("deamon","comlcdsnd",0);
		funtIniIntSet("deamon","comdrvrcv",0);
		funtIniIntSet("deamon","comdrvsnd",0);

		funtMsgRemove("clk");
		funtMsgRemove("int");
		funtMsgRemove("lcd");
		funtMsgRemove("drv");

		funtShmRemove();

		break;

		case 'l':
		printf("----- shm -----\n");
		funtShmList();

		printf("----- msg -----\n");
		funtMsgList("drv");
		funtMsgList("lcd");
		funtMsgList("int");
		funtMsgList("clk");

		printf("----- srv -----\n");
		funtSrvList("sch");
		funtSrvList("crn");
		funtSrvList("clk");
		funtSrvList("drv");
		funtSrvList("lcd");
		funtSrvList("int");
		funtSrvList("veh");
		funtSrvList("comdrvsnd");
		funtSrvList("comdrvrcv");
		funtSrvList("comlcdsnd");
		funtSrvList("comlcdrcv");
		funtSrvList("comintsnd");
		funtSrvList("comintrcv");
		break;

		case 'h':
		case ':':
		case '?':
		case -1:
		printf("\n");
		printf("-b|--boot : 启动服务.\n");
		printf("-s|--shut : 停止服务.\n");
		printf("-l|--list : 显示服务.\n");
		printf("\n");
		break;
	}

	return 0;
}
