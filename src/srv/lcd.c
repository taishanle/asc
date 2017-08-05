/*========================================*\
    文件 : lcd.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sqlite3.h>

#include "unt.h"
#include "bsn.h"

/*========================================*\
    功能 : 0X10处理函数(获取时间)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X10(struct suntComMsg *msg)
{
	time_t stamp;
	stamp=time(NULL);
	if(stamp==(time_t)-1)
	{
		muntLogError("time",errno,strerror(errno),"[]");
		msg->size[0]=0X00;
		msg->size[1]=0X00;
		return;
	}
	struct tm *tm;
	tm=localtime(&stamp);
	if(tm==NULL)
	{
		muntLogError("localtime",errno,strerror(errno),"[]");
		msg->size[0]=0X00;
		msg->size[1]=0X00;
		return;
	}
	msg->data[0]=tm->tm_year+1900-2000;
	msg->data[1]=tm->tm_mon+1;
	msg->data[2]=tm->tm_mday;
	msg->data[3]=tm->tm_hour;
	msg->data[4]=tm->tm_min;
	msg->data[5]=tm->tm_sec;
	msg->size[0]=0X06;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X12处理函数(获取模式)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X12(struct suntComMsg *msg)
{
	msg->data[0]=0X00;

	char curmode[1+1];
	funtIniStrGet("control","curmode",curmode);

	if(curmode[0]=='r')
		msg->data[1]=0XFF;
	else
	if(curmode[0]=='y')
		msg->data[1]=0XFE;
	else
	if(curmode[0]=='c')
		msg->data[1]=0XFD;
	else
	if(curmode[0]=='m')
		msg->data[1]=0XFC;
	else
	if(curmode[0]=='s')
		msg->data[1]=0XFB;
	else
	if(0X41<=curmode[0]&&curmode[0]<=0X60)
		msg->data[1]=curmode[0]-0X40;
	else
	if(curmode[0]=='p')
		msg->data[1]=0X20;
	else
	if(curmode[0]=='a')
		msg->data[1]=0X21;

	msg->size[0]=0X02;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X13处理函数(手自转换)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X13(struct suntComMsg *msg)
{
	int result;

	char aohmode[1+1];
	funtIniStrGet("control","aohmode",aohmode);

	if(msg->data[0]==0X01&&aohmode[0]!='h')
	{
		funtIniStrSet("control","aohmode","h");
		char curmode[1+1];
		funtIniStrGet("control","curmode",curmode);
		funtIniStrSet("control","bakmode",curmode);
	}
	else
	if(msg->data[0]==0X02&&aohmode[0]!='a')
	{
		char bakmode[1+1];
		funtIniStrGet("control","bakmode",bakmode);
		char curmode[1+1];
		funtIniStrGet("control","curmode",curmode);

		if(curmode[0]!=bakmode[0])
		{
			result=fbsnMode(bakmode[0]);
			if(result==-1)
			{
				msg->data[0]=0XEE;
				msg->size[0]=0X01;
				msg->size[1]=0X00;
				return;
			}
			if(result==100)
			{
				msg->data[0]=0X2E;
				msg->size[0]=0X01;
				msg->size[1]=0X00;
				return;
			}

			funtIniStrSet("control","aohmode","a");
		}
	}

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X14处理函数(设置模式)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X14(struct suntComMsg *msg)
{
	int result;

	char aohmode[1+1];
	funtIniStrGet("control","aohmode",aohmode);

	if(aohmode[0]=='h'&&msg->data[0]==0XFF)
		result=fbsnMode('r');
	else
	if(aohmode[0]=='h'&&msg->data[0]==0XFE)
		result=fbsnMode('y');
	else
	if(aohmode[0]=='h'&&msg->data[0]==0XFD)
		result=fbsnMode('c');
	else
	if(0X01<=msg->data[0]&&msg->data[0]<=0X20)
		result=fbsnMode(msg->data[0]+0X40);
	else
	if(msg->data[0]==0X21)
		result=fbsnMode('p');
	else
	if(msg->data[0]==0X22)
		result=fbsnMode('a');
	else
	if(msg->data[0]==0X24)
		result=fbsnMode('m');

	if(result==-1)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	if(result==100)
	{
		msg->data[0]=0X2E;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	else
	{
		msg->data[0]=0X0E;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
}

/*========================================*\
    功能 : 0X15处理函数(下一步骤)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X15(struct suntComMsg *msg)
{
	int result;

	char aohmode[1+1];
	funtIniStrGet("control","aohmode",aohmode);

	if(aohmode[0]!='h')
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}

	result=fbsnMode('s');
	if(result==-1)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	if(result==100)
	{
		msg->data[0]=0X2E;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	else
	{
		msg->data[0]=0X0E;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
}

/*========================================*\
    功能 : 0X27处理函数(获取网络)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X27(struct suntComMsg *msg)
{
	int result;

	char ip[16];
	char mask[16];
	char gate[16];
	char mac[32];
	result=funtNetGet(ip,mask,gate,mac);
	if(result==-1)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}

	funtIniStrSet("net","ip",ip);
	funtIniStrSet("net","mask",mask);
	funtIniStrSet("net","gate",gate);
	funtIniStrSet("net","mac",mac);

	funtNetAsc2Bin(ip,msg->data+0,'.');
	funtNetAsc2Bin(mask,msg->data+4,'.');
	funtNetAsc2Bin(gate,msg->data+8,'.');
	funtNetAsc2Bin(mac,msg->data+12,':');
	msg->data[18]=0X00;

	msg->size[0]=0X17;
	msg->size[1]=0X00;

	return;
}

/*========================================*\
    功能 : 0X28处理函数(设置网络)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X28(struct suntComMsg *msg)
{
	int result;

	char ip[16];
	funtNetBin2Asc(msg->data+0,ip,'.');
	char mask[16];
	funtNetBin2Asc(msg->data+4,mask,'.');
	char gate[16];
	funtNetBin2Asc(msg->data+8,gate,'.');
	char mac[32];
	funtNetBin2Asc(msg->data+12,mac,':');

	result=funtNetSet(ip,mask,gate,mac);
	if(result==-1)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}

	funtIniStrSet("net","ip",ip);
	funtIniStrSet("net","mask",mask);
	funtIniStrSet("net","gate",gate);
	funtIniStrSet("net","mac",mac);

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X40处理函数(获取计划)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X40(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from PLAN");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[0]!=0)
		result=sprintf(sql,"select plan,month,date,day,period from PLAN where plan='%d'",msg->data[0]);
	else
	if(msg->data[0]==0&&(msg->data[1]==0&&msg->data[2]==0))
		result=sprintf(sql,"select plan,month,date,day,period from PLAN");
	else
	if(msg->data[0]==0&&(msg->data[1]!=0&&msg->data[2]!=0))
		result=sprintf(sql,"select plan,month,date,day,period from PLAN limit %d offset %d",msg->data[2],(msg->data[1]-1)*msg->data[2]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);

		char temp1[32];
		unsigned long long temp2;

		memcpy(temp1,sqlite3_column_text(stmt,1),12);
		temp2=0;
		int i;
		for(i=0;i<12;i++)
			if(temp1[i]=='1')
				temp2|=1ULL<<i;
		*(msg->data+current++)=*((char*)&temp2+1);
		*(msg->data+current++)=*((char*)&temp2+0);

		memcpy(temp1,sqlite3_column_text(stmt,2),31);
		temp2=0;
		for(i=0;i<31;i++)
			if(temp1[i]=='1')
				temp2|=1ULL<<i;
		*(msg->data+current++)=*((char*)&temp2+3);
		*(msg->data+current++)=*((char*)&temp2+2);
		*(msg->data+current++)=*((char*)&temp2+1);
		*(msg->data+current++)=*((char*)&temp2+0);

		memcpy(temp1,sqlite3_column_text(stmt,3),7);
		temp2=0;
		for(i=0;i<7;i++)
			if(temp1[i]=='1')
				temp2|=1ULL<<i;
		*(msg->data+current++)=*((char*)&temp2+0);

		msg->data[current++]=sqlite3_column_int(stmt,4);

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X41处理函数(获取时段表)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X41(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from (select distinct periodtable from PERIOD)");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[0]==0&&msg->data[1]==0)
		result=sprintf(sql,"select distinct periodtable from PERIOD");
	else
	if(msg->data[0]!=0&&msg->data[1]!=0)
		result=sprintf(sql,"select distinct periodtable from PERIOD limit %d offset %d",msg->data[1],(msg->data[0]-1)*msg->data[1]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X42处理函数(获取时段)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X42(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from PERIOD where periodtable=%d",msg->data[0]);
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[1]==0&&msg->data[2]==0)
		result=sprintf(sql,"select periodindex,hour,minute,scheme,mode from PERIOD where periodtable=%d",msg->data[0]);
	else
	if(msg->data[1]!=0&&msg->data[2]!=0)
		result=sprintf(sql,"select periodindex,hour,minute,scheme,mode from PERIOD where periodtable=%d limit %d offset %d",msg->data[0],msg->data[2],(msg->data[1]-1)*msg->data[2]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);
		msg->data[current++]=sqlite3_column_int(stmt,1);
		msg->data[current++]=sqlite3_column_int(stmt,2);
		msg->data[current++]=sqlite3_column_int(stmt,3);
		char mode[1+1];
		strcpy(mode,(char*)sqlite3_column_text(stmt,4));
		switch(mode[0])
		{
			case '0':
			msg->data[current++]=0X00;
			break;
			case '1':
			msg->data[current++]=0X01;
			break;
			case '2':
			msg->data[current++]=0X02;
			break;
			case 'm':
			msg->data[current++]=0X06;
			break;
			case 'c':
			msg->data[current++]=0X07;
			break;
			case 'y':
			msg->data[current++]=0X08;
			break;
			case 'r':
			msg->data[current++]=0X09;
			break;
		}

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X43处理函数(获取方案)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X43(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from SCHEME");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[0]!=0)
		result=sprintf(sql,"select scheme,cycle,stage from SCHEME where scheme=%d",msg->data[0]);
	else
	if(msg->data[0]==0&&(msg->data[1]==0&&msg->data[2]==0))
		result=sprintf(sql,"select scheme,cycle,stage from SCHEME");
	else
	if(msg->data[0]==0&&(msg->data[1]!=0&&msg->data[2]!=0))
		result=sprintf(sql,"select scheme,cycle,stage from SCHEME limit %d offset %d",msg->data[2],(msg->data[1]-1)*msg->data[2]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);
		msg->data[current++]=sqlite3_column_int(stmt,1);
		msg->data[current++]=sqlite3_column_int(stmt,2);

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X44处理函数(获取阶段表)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X44(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from (select distinct stagetable from STAGE)");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[0]==0&&msg->data[1]==0)
		result=sprintf(sql,"select distinct stagetable from STAGE");
	else
	if(msg->data[0]!=0&&msg->data[1]!=0)
		result=sprintf(sql,"select distinct stagetable from STAGE limit %d offset %d",msg->data[1],(msg->data[0]-1)*msg->data[1]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X45处理函数(获取阶段)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X45(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from STAGE where stagetable=%d",msg->data[0]);
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[1]==0&&msg->data[2]==0)
		result=sprintf(sql,"select stageindex,phase,greenlight,greenflash,yellow,red,manlight,manflash,delta,mingreen,maxgreen from STAGE where stagetable=%d",msg->data[0]);
	else
	if(msg->data[1]!=0&&msg->data[2]!=0)
		result=sprintf(sql,"select stageindex,phase,greenlight,greenflash,yellow,red,manlight,manflash,delta,mingreen,maxgreen from STAGE where stagetable=%d limit %d offset %d",msg->data[0],msg->data[2],(msg->data[1]-1)*msg->data[2]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);

		char temp1[32];
		unsigned long long temp2;

		memcpy(temp1,sqlite3_column_text(stmt,1),32);
		temp2=0;
		int i;
		for(i=0;i<32;i++)
			if(temp1[i]=='1')
				temp2|=1ULL<<i;
		*(msg->data+current++)=*((char*)&temp2+3);
		*(msg->data+current++)=*((char*)&temp2+2);
		*(msg->data+current++)=*((char*)&temp2+1);
		*(msg->data+current++)=*((char*)&temp2+0);

		msg->data[current++]=sqlite3_column_int(stmt,2);
		msg->data[current++]=sqlite3_column_int(stmt,3);
		msg->data[current++]=sqlite3_column_int(stmt,4);
		msg->data[current++]=sqlite3_column_int(stmt,5);
		msg->data[current++]=sqlite3_column_int(stmt,6);
		msg->data[current++]=sqlite3_column_int(stmt,7);
		msg->data[current++]=sqlite3_column_int(stmt,8);
		msg->data[current++]=sqlite3_column_int(stmt,9);
		msg->data[current++]=sqlite3_column_int(stmt,10);

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X46处理函数(获取相位)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X46(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,"select count(*) from PHASE");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_step(stmt);
	if(result!=SQLITE_ROW)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}
	int count1=sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	if(msg->data[0]!=0)
		result=sprintf(sql,"select phase,channel,corner,lane,conflict from PHASE where phase='%d'",msg->data[0]);
	else
	if(msg->data[0]==0&&(msg->data[1]==0&&msg->data[2]==0))
		result=sprintf(sql,"select phase,channel,corner,lane,conflict from PHASE");
	else
	if(msg->data[0]==0&&(msg->data[1]!=0&&msg->data[2]!=0))
		result=sprintf(sql,"select phase,channel,corner,lane,conflict from PHASE limit %d offset %d",msg->data[2],(msg->data[1]-1)*msg->data[2]);
	else
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->data[1]=0X00;
		msg->size[0]=0X02;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	int current=2;
	int count2=0;

	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		msg->data[current++]=sqlite3_column_int(stmt,0);
		msg->data[current++]=sqlite3_column_int(stmt,1);
		msg->data[current++]=sqlite3_column_int(stmt,2);
		msg->data[current++]=sqlite3_column_int(stmt,3);

		char temp1[32];
		unsigned long long temp2;

		memcpy(temp1,sqlite3_column_text(stmt,4),32);
		temp2=0;
		int i;
		for(i=0;i<32;i++)
			if(temp1[i]=='1')
				temp2|=1ULL<<i;
		*(msg->data+current++)=*((char*)&temp2+3);
		*(msg->data+current++)=*((char*)&temp2+2);
		*(msg->data+current++)=*((char*)&temp2+1);
		*(msg->data+current++)=*((char*)&temp2+0);

		count2++;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	msg->data[0]=count1;
	msg->data[1]=count2;
	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X47处理函数(获取路口)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X47(struct suntComMsg *msg)
{
	int result;

	int current=1;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	sqlite3_stmt *stmt;
	char sql[1024];

	result=sprintf(sql,
		"select corner,name from CORNER");
	result=sqlite3_prepare_v2(db,sql,result,&stmt,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0X00;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	while(1)
	{
		result=sqlite3_step(stmt);
		if(result!=SQLITE_ROW)
			break;
		int corner;
		corner=sqlite3_column_int(stmt,0);
		msg->data[0]|=0X01<<(corner-1);

		strcpy(msg->data+current,(char*)sqlite3_column_text(stmt,1));
		memset(msg->data+current+strlen(msg->data+current),0,16-strlen(msg->data+current));
		current+=16;
	}
	sqlite3_finalize(stmt);

	sqlite3_close(db);

	*(short*)msg->size=current;

	return;
}

/*========================================*\
    功能 : 0X50处理函数(设置计划)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X50(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	char sql[1024];

	char m[12+1];
	char d[31+1];
	char w[7 +1];

	if(msg->data[0]!=0X01)
	{
		int i;
		unsigned int temp;

		*((char*)&temp+0)=msg->data[3];
		*((char*)&temp+1)=msg->data[2];
		*((char*)&temp+2)=0;
		*((char*)&temp+3)=0;
		for(i=0;i<12;i++)
			m[i]=(temp&1ULL<<i)==0?'0':'1';
		m[12]='\0';

		*((char*)&temp+0)=msg->data[7];
		*((char*)&temp+1)=msg->data[6];
		*((char*)&temp+2)=msg->data[5];
		*((char*)&temp+3)=msg->data[4];
		for(i=0;i<31;i++)
			d[i]=(temp&1ULL<<i)==0?'0':'1';
		d[31]='\0';

		*((char*)&temp+0)=msg->data[8];
		*((char*)&temp+1)=0;
		*((char*)&temp+2)=0;
		*((char*)&temp+3)=0;
		for(i=0;i<7;i++)
			w[i]=(temp&1ULL<<i)==0?'0':'1';
		w[7]='\0';
	}

	if(msg->data[0]==0X00)
		result=sprintf(sql,"insert into PLAN values (%d,'%s','%s','%s',%d)",msg->data[1],m,d,w,msg->data[9]);
	else
	if(msg->data[0]==0X01)
		result=sprintf(sql,"delete from PLAN where plan=%d",msg->data[1]);
	else
	if(msg->data[0]==0X02)
		result=sprintf(sql,"update PLAN set month='%s',date='%s',day='%s',period=%d where plan=%d",m,d,w,msg->data[9],msg->data[1]);
	else
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;
}

/*========================================*\
    功能 : 0X52处理函数(设置时段)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X52(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	char sql[1024];

	switch(msg->data[6])
	{
		case 0X00:
		msg->data[6]='0';
		break;
		case 0X01:
		msg->data[6]='1';
		break;
		case 0X02:
		msg->data[6]='2';
		break;
		case 0X06:
		msg->data[6]='m';
		break;
		case 0X07:
		msg->data[6]='c';
		break;
		case 0X08:
		msg->data[6]='y';
		break;
		case 0X09:
		msg->data[6]='r';
		break;
	}

	if(msg->data[0]==0X00)
		result=sprintf(sql,"insert into PERIOD values (%d,%d,%d,%d,%d,'%c')",msg->data[1],msg->data[2],msg->data[3],msg->data[4],msg->data[5],msg->data[6]);
	else
	if(msg->data[0]==0X01)
		result=sprintf(sql,"delete from PERIOD where periodtable=%d and periodindex=%d",msg->data[1],msg->data[2]);
	else
	if(msg->data[0]==0X02)
	{
		result=sprintf(sql,"update PERIOD set hour=%d,minute=%d,scheme=%d,mode='%c' where periodtable=%d and periodindex=%d",msg->data[3],msg->data[4],msg->data[5],msg->data[6],msg->data[1],msg->data[2]);
	}
	else
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;
}

/*========================================*\
    功能 : 0X53处理函数(设置方案)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X53(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	char sql[1024];

	if(msg->data[0]==0X00)
		result=sprintf(sql,"insert into SCHEME values (%d,%d,%d)",msg->data[1],msg->data[2],msg->data[3]);
	else
	if(msg->data[0]==0X01)
		result=sprintf(sql,"delete from SCHEME where scheme=%d",msg->data[1]);
	else
	if(msg->data[0]==0X02)
		result=sprintf(sql,"update SCHEME set cycle=%d,stage=%d where scheme=%d",msg->data[2],msg->data[3],msg->data[1]);
	else
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;
}

/*========================================*\
    功能 : 0X55处理函数(设置阶段)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X55(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	char sql[1024];

	char phase[32+1];

	if(msg->data[0]!=0X01)
	{
		int i;
		unsigned int temp;

		*((char*)&temp+0)=msg->data[6];
		*((char*)&temp+1)=msg->data[5];
		*((char*)&temp+2)=msg->data[4];
		*((char*)&temp+3)=msg->data[3];
		for(i=0;i<32;i++)
			phase[i]=(temp&1ULL<<i)==0?'0':'1';
		phase[32]='\0';
	}

	if(msg->data[0]==0X00)
		result=sprintf(sql,"insert into STAGE values (%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d)",msg->data[1],msg->data[2],phase,msg->data[7],msg->data[8],msg->data[9],msg->data[10],msg->data[11],msg->data[12],msg->data[13],msg->data[14],msg->data[15]);
	else
	if(msg->data[0]==0X01)
		result=sprintf(sql,"delete from STAGE where stagetable=%d and stageindex=%d",msg->data[1],msg->data[2]);
	else
	if(msg->data[0]==0X02)
		result=sprintf(sql,"update STAGE set phase='%s',greenlight=%d,greenflash=%d,yellow=%d,red=%d,manlight=%d,manflash=%d,delta=%d,mingreen=%d,maxgreen=%d where stagetable=%d and stageindex=%d",phase,msg->data[7],msg->data[8],msg->data[9],msg->data[10],msg->data[11],msg->data[12],msg->data[13],msg->data[14],msg->data[15],msg->data[1],msg->data[2]);
	else
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;
}

/*========================================*\
    功能 : 0X56处理函数(设置相位)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X56(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	char sql[1024];

	char conflict[32+1];

	if(msg->data[0]!=0X01)
	{
		int i;
		unsigned int temp;

		*((char*)&temp+0)=msg->data[8];
		*((char*)&temp+1)=msg->data[7];
		*((char*)&temp+2)=msg->data[6];
		*((char*)&temp+3)=msg->data[5];
		for(i=0;i<32;i++)
			conflict[i]=(temp&1ULL<<i)==0?'0':'1';
		conflict[32]='\0';
	}

	if(msg->data[0]==0X00)
		result=sprintf(sql,"insert into PHASE values (%d,%d,%d,%d,'%s')",msg->data[1],msg->data[2],msg->data[3],msg->data[4],conflict);
	else
	if(msg->data[0]==0X01)
		result=sprintf(sql,"delete from PHASE where phase=%d",msg->data[1]);
	else
	if(msg->data[0]==0X02)
		result=sprintf(sql,"update PHASE set channel=%d,corner=%d,lane=%d,conflict='%s' where phase=%d",msg->data[2],msg->data[3],msg->data[4],conflict,msg->data[1]);
	else
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}
	result=sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		sqlite3_close(db);
		return;
	}

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;
}

/*========================================*\
    功能 : 0X57处理函数(设置路口)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X57(struct suntComMsg *msg)
{
	int result;

	char dbpath[64];
	sprintf(dbpath,"%s/etc/asc.db",getenv("ASC"));
	sqlite3 *db;
	result=sqlite3_open(dbpath,&db);
	if(result!=SQLITE_OK)
	{
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	char sql[1024];

	sqlite3_exec(db,"begin transaction",NULL,NULL,NULL); 

	result=sprintf(sql,"delete from CORNER");
	result=sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(result!=SQLITE_OK)
		goto failure;

	int i;
	int j;
	for(i=0,j=0;i<8;i++)
	{
		if(msg->data[0]&0X01>>i)
		{
			char name[32];
			strcpy(name,msg->data+1+j*32);
			result=sprintf(sql,"insert into CORNER values (%d,'%s')",i+1,name);
			result=sqlite3_exec(db,sql,NULL,NULL,NULL);
			if(result!=SQLITE_OK)
				goto failure;
		}
	}

	sqlite3_exec(db,"commit transaction",NULL,NULL,NULL); 
	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;

	failure:
	sqlite3_exec(db,"rollback transaction",NULL,NULL,NULL); 
	msg->data[0]=0XEE;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	sqlite3_close(db);
	return;
}

/*========================================*\
    功能 : 0X60处理函数(设置参数结束)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X60(struct suntComMsg *msg)
{
	char curmode[1+1];
	funtIniStrGet("control","curmode",curmode);
	if((0X41<=curmode[0]&&curmode[0]<=0X60)||curmode[0]=='p'||curmode[0]=='a')
		fbsnMode(curmode[0]);

	msg->data[0]=0X00;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X61处理函数(获取当前配置)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X61(struct suntComMsg *msg)
{
	msg->data[0]=vbsnMemory->fmlscheme.planindex;
	msg->data[1]=vbsnMemory->fmlscheme.periodtable;
	msg->data[2]=vbsnMemory->fmlscheme.periodindex;
	msg->data[3]=vbsnMemory->fmlscheme.schemeindex;
	msg->data[4]=vbsnMemory->fmlscheme.stagetable;
	msg->size[0]=0X05;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X64处理函数(获取硬件)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X64(struct suntComMsg *msg)
{
	char status[1+1];

	msg->data[0]=0X0E;

	msg->data[1]=0X01;

	funtIniStrGet("module","lcd",status);
	if(status[0]=='n')
		msg->data[2]=0X01;
	else
	if(status[0]=='a')
		msg->data[2]=0X02;

	funtIniStrGet("module","int",status);
	if(status[0]=='n')
		msg->data[3]=0X01;
	else
	if(status[0]=='a')
		msg->data[3]=0X02;

	msg->data[4]=0X02;

	funtIniStrGet("module","drv0",status);
	if(status[0]=='n')
		msg->data[5]=0X01;
	else
	if(status[0]=='a')
		msg->data[5]=0X02;

	funtIniStrGet("module","drv1",status);
	if(status[0]=='n')
		msg->data[6]=0X01;
	else
	if(status[0]=='a')
		msg->data[6]=0X02;

	funtIniStrGet("module","drv2",status);
	if(status[0]=='n')
		msg->data[7]=0X01;
	else
	if(status[0]=='a')
		msg->data[7]=0X02;

	funtIniStrGet("module","drv3",status);
	if(status[0]=='n')
		msg->data[8]=0X01;
	else
	if(status[0]=='a')
		msg->data[8]=0X02;

	msg->size[0]=0X09;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X65处理函数(设置时间)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X65(struct suntComMsg *msg)
{
	int result;

	struct tm tm;
	bzero(&tm,sizeof(tm));
	tm.tm_year=msg->data[0]+2000-1900;
	tm.tm_mon =msg->data[1]-1;
	tm.tm_mday=msg->data[2];
	tm.tm_hour=msg->data[3];
	tm.tm_min =msg->data[4];
	tm.tm_sec =msg->data[5];

	struct timeval tv;
	tv.tv_sec=mktime(&tm);
	if(tv.tv_sec==-1)
	{
		muntLogError("mktime",errno,strerror(errno),"[]");
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	tv.tv_usec=0;
	result=settimeofday(&tv,NULL);
	if(result==-1)
	{
		muntLogError("settimeofday",errno,strerror(errno),"[]");
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}

	int rtc;
	rtc=open("/dev/rtc",O_WRONLY);
	if(rtc==-1)
	{
		muntLogError("open",errno,strerror(errno),"[]");
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	result=ioctl(rtc,RTC_SET_TIME,&tm);
	if(result==-1)
	{
		muntLogError("ioctl",errno,strerror(errno),"[]");
		msg->data[0]=0XEE;
		msg->size[0]=0X01;
		msg->size[1]=0X00;
		return;
	}
	close(rtc);

	char curmode[1+1];
	funtIniStrGet("control","curmode",curmode);
	if(curmode[0]=='p'||curmode[0]=='a')
		fbsnMode(curmode[0]);

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X66处理函数(获取行人按键参数)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X66(struct suntComMsg *msg)
{
	int vehmintime;
	funtIniIntGet("control","vehmintime",&vehmintime);
	int manmintime;
	funtIniIntGet("control","manmintime",&manmintime);
	int manmaxtime;
	funtIniIntGet("control","manmaxtime",&manmaxtime);

	msg->data[0]=vehmintime;
	msg->data[1]=manmintime;
	msg->data[2]=manmaxtime;
	msg->size[0]=0X03;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 0X67处理函数(设置行人按键参数)
    参数 : (出入)消息队列消息
    返回 : 空
\*========================================*/
void flcd0X67(struct suntComMsg *msg)
{
	int vehmintime=msg->data[0];
	funtIniIntSet("control","vehmintime",vehmintime);
	int manmintime=msg->data[1];
	funtIniIntSet("control","manmintime",manmintime);
	int manmaxtime=msg->data[2];
	funtIniIntSet("control","manmaxtime",manmaxtime);

	msg->data[0]=0X0E;
	msg->size[0]=0X01;
	msg->size[1]=0X00;
	return;
}

/*========================================*\
    功能 : 液晶屏服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int flcdDeamon(void)
{
	int result;

	funtMsgInit();

	int times;
	funtIniIntGet("develop","times",&times);
	int inter;
	funtIniIntGet("develop","inter",&inter);

	funtIniStrSet("module","lcd","n");
	int miss=0;

	while(1)
	{
		struct suntComMsg msg;
		result=funtMsgSrvRcv("lcd",&msg,1,1000*inter,1);
		if(result==100)
		{
			msg.type=0X01;
			msg.func=0X01;
			msg.data[0]=0X00;
			msg.size[0]=0X01;
			msg.size[1]=0X00;
			msg.src=0X01;

			msg.com=0X00;
			msg.dst=0X02;
			funtMsgCliSnd("lcd",&msg,1,100,4);

			msg.com=0X00;
			msg.dst=0X02;
			result=funtMsgCliRcv("lcd",&msg,1,100,4);
			if(result==100)
			{
				if(miss<times+1)
					miss++;
				if(miss==times)
					funtIniStrSet("module","lcd","a");
			}
			else
			{
				if(miss!=0)
				{
					funtIniStrSet("module","lcd","n");
					miss=0;
				}
			}
		}
		else
		{
			msg.type=0X02;
			msg.src=0X01;
			msg.dst=0X02;

			switch(msg.func)
			{
				case 0X10: flcd0X10(&msg); break;
				case 0X12: flcd0X12(&msg); break;
				case 0X13: flcd0X13(&msg); break;
				case 0X14: flcd0X14(&msg); break;
				case 0X15: flcd0X15(&msg); break;
				case 0X27: flcd0X27(&msg); break;
				case 0X28: flcd0X28(&msg); break;
				case 0X40: flcd0X40(&msg); break;
				case 0X41: flcd0X41(&msg); break;
				case 0X42: flcd0X42(&msg); break;
				case 0X43: flcd0X43(&msg); break;
				case 0X44: flcd0X44(&msg); break;
				case 0X45: flcd0X45(&msg); break;
				case 0X46: flcd0X46(&msg); break;
				case 0X47: flcd0X47(&msg); break;
				case 0X50: flcd0X50(&msg); break;
				case 0X52: flcd0X52(&msg); break;
				case 0X53: flcd0X53(&msg); break;
				case 0X55: flcd0X55(&msg); break;
				case 0X56: flcd0X56(&msg); break;
				case 0X57: flcd0X57(&msg); break;
				case 0X60: flcd0X60(&msg); break;
				case 0X61: flcd0X61(&msg); break;
				case 0X64: flcd0X64(&msg); break;
				case 0X65: flcd0X65(&msg); break;
				case 0X66: flcd0X66(&msg); break;
				case 0X67: flcd0X67(&msg); break;
				default: continue;
			}

			funtMsgSrvSnd("lcd",&msg,1,100,4);
		}
	}

	return 0;
}
