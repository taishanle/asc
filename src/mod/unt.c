/*========================================*\
    文件 : unt.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "unt.h"
#include "bsn.h"

/*========================================*\
    功能 : 按位统计
    参数 : (输入)数据
    返回 : 位数
\*========================================*/
int funtBitCount(char *data)
{
	int count=0;
	int i;
	for(i=0;i<strlen(data);i++)
		if(data[i]=='1')
			count++;
	return count;
}

/*========================================*\
    功能 : 校验计算
    参数 : (输入)数据内容
           (输入)数据长度
    返回 : 结果
\*========================================*/
short funtCRC16(char *data,int size)
{
	unsigned short crc=0XFFFF;
	int i;
	for(i=0;i<size;i++)
	{
		crc^=data[i];
		int j;
		for(j=0;j<8;j++)
		{
			char bit=crc&0X01;
			crc>>=1;
			if(bit)
				crc^=0XA001;
		}
	}
	return crc;
}

/*========================================*\
    功能 : 网络参数BIN->ASC
    参数 : (输入)来源数据
           (输出)目的数据
           (输入)分隔字符
    返回 : 空
\*========================================*/
void funtNetBin2Asc(char *srcdata,char *dstdata,char split)
{
	int size;
	if(split=='.')
		size=4;
	else
	if(split==':')
		size=6;
	int current=0;
	int i;
	for(i=0;i<size;i++)
	{
		if(split=='.')
		{
			current+=sprintf(dstdata+current,"%d%c",srcdata[i],split);
		}
		else
		if(split==':')
		{
			dstdata[current]=srcdata[i]/16;
			if(dstdata[current]<0X0A)
				dstdata[current]+='0';
			else
				dstdata[current]+='A'-10;
			current++;
			dstdata[current]=srcdata[i]%16;
			if(dstdata[current]<0X0A)
				dstdata[current]+='0';
			else
				dstdata[current]+='A'-10;
			current++;
			dstdata[current++]=split;
		}
	}
	dstdata[current-1]='\0';
	return;
}

/*========================================*\
    功能 : 网络参数ASC->BIN
    参数 : (输入)来源数据
           (输出)目的数据
           (输入)分隔字符
    返回 : 空
\*========================================*/
void funtNetAsc2Bin(char *srcdata,char *dstdata,char split)
{
	int current=0;
	char *position1=srcdata;
	char *position2;
	while(1)
	{
		position2=strchr(position1,split);
		if(position2==NULL)
		{
			if(split=='.')
			{
				dstdata[current++]=atoi(position1);
			}
			else
			if(split==':')
			{
				if(isdigit(*position1))
					dstdata[current]=16*(*position1-'0');
				else
					dstdata[current]=16*(*position1-'A'+10);
				if(isdigit(*position1+1))
					dstdata[current++]+=*(position1+1)-'0';
				else
					dstdata[current++]+=*(position1+1)-'A'+10;
			}
			break;
		}
		else
		{
			if(split=='.')
			{
				*position2='\0';
				dstdata[current++]=atoi(position1);
			}
			else
			if(split==':')
			{
				if(isdigit(*position1))
					dstdata[current]=16*(*position1-'0');
				else
					dstdata[current]=16*(*position1-'A'+10);
				if(isdigit(*(position1+1)))
					dstdata[current++]+=*(position1+1)-'0';
				else
					dstdata[current++]+=*(position1+1)-'A'+10;
			}
		}
		position1=position2+1;
	}
	return;
}

/*========================================*\
    功能 : 获取网络参数
    参数 : (输出)IP地址
           (输出)掩码
           (输出)网关
           (输出)MAC地址
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtNetGet(char *ip,char *mask,char *gate,char *mac)
{
	char command[128];
	FILE *fp;
	char text[32];

	sprintf(command,"ifconfig eth0|sed -n '2p'|awk '{print $2}'|awk -F ':' '{print $2}'");
	fp=popen(command,"r");
	if(fp==NULL)
	{
		muntLogError("popen",errno,strerror,"[]");
		return -1;
	}
	fgets(text,sizeof(text),fp);
	if(ferror(fp))
	{
		muntLogError("fgets",errno,strerror,"[]");
		return -1;
	}
	pclose(fp);
	text[strlen(text)-1]='\0';
	strcpy(ip,text);

	sprintf(command,"ifconfig eth0|sed -n '2p'|awk '{print $4}'|awk -F ':' '{print $2}'");
	fp=popen(command,"r");
	if(fp==NULL)
	{
		muntLogError("popen",errno,strerror,"[]");
		return -1;
	}
	fgets(text,sizeof(text),fp);
	if(ferror(fp))
	{
		muntLogError("fgets",errno,strerror,"[]");
		return -1;
	}
	pclose(fp);
	text[strlen(text)-1]='\0';
	strcpy(mask,text);

	sprintf(command,"route -n|grep '0.0.0.0'|awk '{print $2}'");
	fp=popen(command,"r");
	if(fp==NULL)
	{
		muntLogError("popen",errno,strerror,"[]");
		return -1;
	}
	fgets(text,sizeof(text),fp);
	if(ferror(fp))
	{
		muntLogError("fgets",errno,strerror,"[]");
		return -1;
	}
	pclose(fp);
	text[strlen(text)-1]='\0';
	strcpy(gate,text);

	sprintf(command,"ifconfig eth0|sed -n '1p'|awk '{print $5}'");
	fp=popen(command,"r");
	if(fp==NULL)
	{
		muntLogError("popen",errno,strerror,"[]");
		return -1;
	}
	fgets(text,sizeof(text),fp);
	if(ferror(fp))
	{
		muntLogError("fgets",errno,strerror,"[]");
		return -1;
	}
	pclose(fp);
	text[strlen(text)-1]='\0';
	strcpy(mac,text);

	return 0;
}

/*========================================*\
    功能 : 设置网络参数
    参数 : (输入)IP地址
           (输入)掩码
           (输入)网关
           (输入)MAC地址
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtNetSet(char *ip,char *mask,char *gate,char *mac)
{
	char command[128];
	sprintf(command,"ifconfig eth0 %s netmask %s >/dev/null 2>&1",ip,mask);
	system(command);
	sprintf(command,"route add default gw %s >/dev/null 2>&1",gate);
	system(command);
	sprintf(command,"ifconfig eth0 hw ether %s >/dev/null 2>&1",mac);
	system(command);

	return 0;
}

/*========================================*\
    功能 : 打开日志
    参数 : (输入)事件日志文件路径
           (输入)错误日志文件路径
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtLogInit(char *eventpath,char *errorpath)
{
	int result;

	int eventlogid;
	eventlogid=open(eventpath,O_RDWR|O_APPEND|O_CREAT,S_IRUSR);
	if(eventlogid==-1)
	{
		muntLogError("open",errno,strerror(errno),"[%s]",eventpath);
		return -1;
	}
	result=dup2(eventlogid,STDOUT_FILENO);
	if(result==-1)
	{
		muntLogError("dup2",errno,strerror(errno),"[]");
		return -1;
	}

	int errorlogid;
	errorlogid=open(errorpath,O_RDWR|O_APPEND|O_CREAT,S_IRUSR);
	if(errorlogid==-1)
	{
		muntLogError("open",errno,strerror(errno),"[%s]",errorpath);
		return -1;
	}
	result=dup2(errorlogid,STDERR_FILENO);
	if(result==-1)
	{
		muntLogError("dup2",errno,strerror(errno),"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 打印日志(任何版本都打印)
    参数 : (输入)日志文件句柄
           (输入)格式化字符串
    返回 : 空
\*========================================*/
void funtLogAnyhow(FILE *fp,char *format,...)
{
	va_list argument;
	va_start(argument,format);
	vfprintf(fp,format,argument);
	va_end(argument);
	fflush(fp);
	#ifndef NDEBUG
	#endif
	return;
}

/*========================================*\
    功能 : 打印日志(调试版本才打印)
    参数 : (输入)日志文件句柄
           (输入)格式化字符串
    返回 : 空
\*========================================*/
void funtLogDepend(FILE *fp,char *format,...)
{
	#ifndef NDEBUG
	va_list argument;
	va_start(argument,format);
	vfprintf(fp,format,argument);
	va_end(argument);
	fflush(fp);
	#endif
	return;
}

/*========================================*\
    功能 : 获取日期
    参数 : (输入)日期类型 (无分隔符/4位年份)0
                          (有分隔符/4位年份)1
                          (无分隔符/2位年份)2
                          (有分隔符/2位年份)3
    返回 : 日期
\*========================================*/
char *funtLogDate(int type)
{
	time_t stamp;
	stamp=time(NULL);
	if(type==0)
	{
		static char data[16];
		strftime(data,sizeof(data),"%Y%m%d",localtime(&stamp));
		return data;
	}
	else
	if(type==1)
	{
		static char data[16];
		strftime(data,sizeof(data),"%Y-%m-%d",localtime(&stamp));
		return data;
	}
	else
	if(type==2)
	{
		static char data[16];
		strftime(data,sizeof(data),"%y%m%d",localtime(&stamp));
		return data;
	}
	else
	if(type==3)
	{
		static char data[16];
		strftime(data,sizeof(data),"%y-%m-%d",localtime(&stamp));
		return data;
	}
	return NULL;
}

/*========================================*\
    功能 : 获取时间
    参数 : (输入)时间类型 (无分隔符/秒)0
                          (有分隔符/秒)1
                          (无分隔符/微秒)2
                          (有分隔符/微秒)3
    返回 : 时间
\*========================================*/
char *funtLogTime(int type)
{
	time_t stamp;
	stamp=time(NULL);
	if(type==0)
	{
		static char data[16];
		strftime(data,sizeof(data),"%H%M%S",localtime(&stamp));
		return data;
	}
	else
	if(type==1)
	{
		static char data[16];
		strftime(data,sizeof(data),"%H:%M:%S",localtime(&stamp));
		return data;
	}
	else
	if(type==2)
	{
		static char data[16];
		int result=strftime(data,sizeof(data),"%H%M%S",localtime(&stamp));
		struct timeval tv;
		gettimeofday(&tv,NULL);
		sprintf(data+result,"%06d",(int)tv.tv_usec);
		return data;
	}
	else
	if(type==3)
	{
		static char data[16];
		int result=strftime(data,sizeof(data),"%H:%M:%S",localtime(&stamp));
		struct timeval tv;
		gettimeofday(&tv,NULL);
		sprintf(data+result,":%06d",(int)tv.tv_usec);
		return data;
	}
	return NULL;
}

//配置文件锁文件描述符.
int vascIniLid;
//共享内存锁文件描述符.
int vascShmLid;

/*========================================*\
    功能 : 打开文件锁
    参数 : (输入)对象
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtLckInit(char *object)
{
	if(strcmp(object,"ini")==0)
	{
		char inilckpath[64];
		sprintf(inilckpath,"%s/etc/ascini.lck",getenv("ASC"));
		vascIniLid=open(inilckpath,O_CREAT|O_RDWR,0600);
		if(vascIniLid==-1)
		{
			muntLogError("open",errno,strerror(errno),"[%s]",inilckpath);
			return -1;
		}
	}
	else
	if(strcmp(object,"shm")==0)
	{
		char shmlckpath[64];
		sprintf(shmlckpath,"%s/etc/ascshm.lck",getenv("ASC"));
		vascShmLid=open(shmlckpath,O_CREAT|O_RDWR,0600);
		if(vascShmLid==-1)
		{
			muntLogError("open",errno,strerror(errno),"[%s]",shmlckpath);
			return -1;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 文件锁加锁
    参数 : (输入)对象
           (输出)是否阻塞 (是)1
                          (否)0
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtLckLck(char *object,int action)
{
	int result;

	int lid;
	if(strcmp(object,"ini")==0)
		lid=vascIniLid;
	else
	if(strcmp(object,"shm")==0)
		lid=vascShmLid;

	struct flock lock;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_WRLCK;
	if(action==0)
	{
		result=fcntl(lid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{
			muntLogError("fcntl",errno,strerror(errno),"[]");
			return -1;
		}
		if(result==-1&&errno==EAGAIN)
			return 100;
	}
	else
	if(action==1)
	{
		result=fcntl(lid,F_SETLKW,&lock);
		if(result==-1)
		{
			muntLogError("fcntl",errno,strerror(errno),"[]");
			return -1;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 文件锁解锁
    参数 : (输入)锁文件
           (输出)是否阻塞 (是)1
                          (否)0
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtLckUck(char *object,int action)
{
	int result;

	int lid;
	if(strcmp(object,"ini")==0)
		lid=vascIniLid;
	else
	if(strcmp(object,"shm")==0)
		lid=vascShmLid;

	struct flock lock;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_type=F_UNLCK;
	if(action==0)
	{
		result=fcntl(lid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{
			muntLogError("fcntl",errno,strerror(errno),"[]");
			return -1;
		}
		if(result==-1&&errno==EAGAIN)
			return 100;
	}
	else
	if(action==1)
	{
		result=fcntl(lid,F_SETLKW,&lock);
		if(result==-1)
		{
			muntLogError("fcntl",errno,strerror(errno),"[]");
			return -1;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 获取字符配置
    参数 : (输入)段
           (输入)键
           (输出)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniStrGet(char *sec,char *key,char *data)
{
	funtLckLck("ini",1);

	char inipath[64];
	sprintf(inipath,"%s/etc/asc.ini",getenv("ASC"));
	FILE *fp;
	fp=fopen(inipath,"r");
	if(fp==NULL)
	{
		muntLogError("fopen",errno,strerror(errno),"[%s]",inipath);
		funtLckUck("ini",0);
		return -1;
	}

	int in;
	in=0;

	while(1)
	{
		char line[1024];
		fgets(line,sizeof(line),fp);
		if(ferror(fp))
		{
			muntLogError("fgets",errno,strerror(errno),"[]");
			fclose(fp);
			funtLckUck("ini",0);
			return -1;
		}
		if(feof(fp))
			break;

		if(line[0]=='#')
			continue;
		else
		if(line[0]=='\n')
			continue;
		else
		if(line[0]=='[')
		{
			line[strlen(line)-2]='\0';
			if(strcmp(line+1,sec)==0)
				in=1;
			else
			if(in==1)
			{
				funtLckUck("ini",0);
				return -1;
			}
		}
		else
		{
			if(in==0)
				continue;

			char *position1;
			char *position2;

			position1=line;
			position2=strchr(position1,'=');
			if(position2==NULL)
			{
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}
			*position2='\0';
			if(strcmp(position1,key)!=0)
				continue;

			position1=position2+1;
			position2=strchr(position1,'\n');
			if(position2==NULL)
			{
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}
			*position2='\0';
			strcpy(data,position1);

			break;
		}
	}

	fclose(fp);
	funtLckUck("ini",0);

	return 0;
}

/*========================================*\
    功能 : 获取数值配置
    参数 : (输入)段
           (输入)键
           (输出)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniIntGet(char *sec,char *key,int *data)
{
	funtLckLck("ini",1);

	char inipath[64];
	sprintf(inipath,"%s/etc/asc.ini",getenv("ASC"));
	FILE *fp;
	fp=fopen(inipath,"r");
	if(fp==NULL)
	{
		muntLogError("fopen",errno,strerror(errno),"[%s]",inipath);
		funtLckUck("ini",0);
		return -1;
	}

	int in;
	in=0;

	while(1)
	{
		char line[1024];
		fgets(line,sizeof(line),fp);
		if(ferror(fp))
		{
			muntLogError("fgets",errno,strerror(errno),"[]");
			fclose(fp);
			funtLckUck("ini",0);
			return -1;
		}
		if(feof(fp))
			break;

		if(line[0]=='#')
			continue;
		else
		if(line[0]=='\n')
			continue;
		else
		if(line[0]=='[')
		{
			line[strlen(line)-2]='\0';
			if(strcmp(line+1,sec)==0)
				in=1;
			else
			if(in==1)
			{
				funtLckUck("ini",0);
				return -1;
			}
		}
		else
		{
			if(in==0)
				continue;

			char *position1;
			char *position2;

			position1=line;
			position2=strchr(position1,'=');
			if(position2==NULL)
			{
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}
			*position2='\0';
			if(strcmp(position1,key)!=0)
				continue;

			position1=position2+1;
			position2=strchr(position1,'\n');
			if(position2==NULL)
			{
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}
			*position2='\0';
			*data=atoi(position1);

			break;
		}
	}

	fclose(fp);
	funtLckUck("ini",0);

	return 0;
}

/*========================================*\
    功能 : 设置字符配置
    参数 : (输入)段
           (输入)键
           (输入)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniStrSet(char *sec,char *key,char *data)
{
	int result;

	funtLckLck("ini",1);

	char inipath[64];
	sprintf(inipath,"%s/etc/asc.ini",getenv("ASC"));
	FILE *fp;
	fp=fopen(inipath,"r+");
	if(fp==NULL)
	{
		muntLogError("fopen",errno,strerror(errno),"[%s]",inipath);
		funtLckUck("ini",0);
		return -1;
	}

	int in;
	in=0;

	while(1)
	{
		long offset1;
		offset1=ftell(fp);
		if(offset1==-1)
		{
			muntLogError("ftell",errno,strerror(errno),"[]");
			fclose(fp);
			funtLckUck("ini",0);
			return -1;
		}

		char line[1024];
		fgets(line,sizeof(line),fp);
		if(ferror(fp))
		{
			muntLogError("fgets",errno,strerror(errno),"[]");
			fclose(fp);
			funtLckUck("ini",0);
			return -1;
		}
		if(feof(fp))
			break;

		if(line[0]=='#')
			continue;
		else
		if(line[0]=='\n')
			continue;
		else
		if(line[0]=='[')
		{
			line[strlen(line)-2]='\0';
			if(strcmp(line+1,sec)==0)
				in=1;
			else
			if(in==1)
			{
				funtLckUck("ini",0);
				return -1;
			}
		}
		else
		{
			if(in==0)
				continue;

			char *position1;
			char *position2;

			position1=line;
			position2=strchr(position1,'=');
			if(position2==NULL)
			{
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}
			*position2='\0';
			if(strcmp(position1,key)!=0)
				continue;

			struct stat status;
			result=fstat(fileno(fp),&status);
			if(result==-1)
			{
				muntLogError("fstat",errno,strerror(errno),"[]");
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			long offset2;
			offset2=ftell(fp);
			if(offset2==-1)
			{
				muntLogError("ftell",errno,strerror(errno),"[]");
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			char *text;
			text=(char*)malloc(status.st_size-offset2);
			if(text==NULL)
			{
				muntLogError("malloc",0,"0","[%d]",status.st_size-offset2);
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			int remain;
			int record;
			remain=status.st_size-offset2;
			record=0;
			while(remain>0)
			{
				result=fread(text+record,1,remain,fp);
				if(ferror(fp))
				{
					muntLogError("fread",errno,strerror(errno),"[]");
					free(text);
					fclose(fp);
					funtLckUck("ini",0);
					return -1;
				}
				remain-=result;
				record+=result;
			}

			result=fseek(fp,offset1,SEEK_SET);
			if(result==-1)
			{
				muntLogError("fseek",errno,strerror(errno),"[]");
				free(text);
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			int length;
			length=fprintf(fp,"%s=%s\n",key,data);

			remain=status.st_size-offset2;
			record=0;
			while(remain>0)
			{
				result=fwrite(text+record,1,remain,fp);
				if(ferror(fp))
				{
					muntLogError("fwrite",errno,strerror(errno),"[]");
					free(text);
					fclose(fp);
					funtLckUck("ini",0);
					return -1;
				}
				remain-=result;
				record+=result;
			}

			result=ftruncate(fileno(fp),status.st_size-(offset2-offset1)+length);
			if(result==-1)
			{
				muntLogError("ftruncate",errno,strerror(errno),"[%d]",status.st_size-(offset2-offset1)+length);
				free(text);
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			free(text);

			break;
		}
	}

	fclose(fp);
	funtLckUck("ini",0);

	return 0;
}

/*========================================*\
    功能 : 设置数值配置
    参数 : (输入)段
           (输入)键
           (输入)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniIntSet(char *sec,char *key,int data)
{
	int result;

	funtLckLck("ini",1);

	char inipath[64];
	sprintf(inipath,"%s/etc/asc.ini",getenv("ASC"));
	FILE *fp;
	fp=fopen(inipath,"r+");
	if(fp==NULL)
	{
		muntLogError("fopen",errno,strerror(errno),"[%s]",inipath);
		funtLckUck("ini",0);
		return -1;
	}

	int in;
	in=0;

	while(1)
	{
		long offset1;
		offset1=ftell(fp);
		if(offset1==-1)
		{
			muntLogError("ftell",errno,strerror(errno),"[]");
			fclose(fp);
			funtLckUck("ini",0);
			return -1;
		}

		char line[1024];
		fgets(line,sizeof(line),fp);
		if(ferror(fp))
		{
			muntLogError("fgets",errno,strerror(errno),"[]");
			fclose(fp);
			funtLckUck("ini",0);
			return -1;
		}
		if(feof(fp))
			break;

		if(line[0]=='#')
			continue;
		else
		if(line[0]=='\n')
			continue;
		else
		if(line[0]=='[')
		{
			line[strlen(line)-2]='\0';
			if(strcmp(line+1,sec)==0)
				in=1;
			else
			if(in==1)
			{
				funtLckUck("ini",0);
				return -1;
			}
		}
		else
		{
			if(in==0)
				continue;

			char *position1;
			char *position2;

			position1=line;
			position2=strchr(position1,'=');
			if(position2==NULL)
			{
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}
			*position2='\0';
			if(strcmp(position1,key)!=0)
				continue;

			struct stat status;
			result=fstat(fileno(fp),&status);
			if(result==-1)
			{
				muntLogError("fstat",errno,strerror(errno),"[]");
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			long offset2;
			offset2=ftell(fp);
			if(offset2==-1)
			{
				muntLogError("ftell",errno,strerror(errno),"[]");
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			char *text;
			text=(char*)malloc(status.st_size-offset2);
			if(text==NULL)
			{
				muntLogError("malloc",0,"0","[%d]",status.st_size-offset2);
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			int remain;
			int record;
			remain=status.st_size-offset2;
			record=0;
			while(remain>0)
			{
				result=fread(text+record,1,remain,fp);
				if(ferror(fp))
				{
					muntLogError("fread",errno,strerror(errno),"[]");
					free(text);
					fclose(fp);
					funtLckUck("ini",0);
					return -1;
				}
				remain-=result;
				record+=result;
			}

			result=fseek(fp,offset1,SEEK_SET);
			if(result==-1)
			{
				muntLogError("fseek",errno,strerror(errno),"[]");
				free(text);
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			int length;
			length=fprintf(fp,"%s=%d\n",key,data);

			remain=status.st_size-offset2;
			record=0;
			while(remain>0)
			{
				result=fwrite(text+record,1,remain,fp);
				if(ferror(fp))
				{
					muntLogError("fwrite",errno,strerror(errno),"[]");
					free(text);
					fclose(fp);
					funtLckUck("ini",0);
					return -1;
				}
				remain-=result;
				record+=result;
			}

			result=ftruncate(fileno(fp),status.st_size-(offset2-offset1)+length);
			if(result==-1)
			{
				muntLogError("ftruncate",errno,strerror(errno),"[%d]",status.st_size-(offset2-offset1)+length);
				free(text);
				fclose(fp);
				funtLckUck("ini",0);
				return -1;
			}

			free(text);

			break;
		}
	}

	fclose(fp);
	funtLckUck("ini",0);

	return 0;
}

//定时消息队列标识.
int vuntClkMid;

/*========================================*\
    功能 : 发送定时指令
    参数 : (输入)指令 (设置时间)>=0
                      (获取时间)<0
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtClkTimeSnd(int order)
{
	int result;
	struct suntClkMsg msg;
	msg.index=1;
	msg.order=order;
	do
		result=msgsnd(vuntClkMid,&msg,sizeof(struct suntClkMsg)-sizeof(long),0);
	while(result==-1&&errno==EINTR);
	if(result==-1)
	{
		muntLogError("msgsnd",errno,strerror(errno),"[%d]",vuntClkMid);
		return -1;
	}
	return 0;
}

/*========================================*\
    功能 : 接收定时指令
    参数 : (输出)剩余时间
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtClkTimeRcv(int *order)
{
	int result;
	struct suntClkMsg msg;
	do
		result=msgrcv(vuntClkMid,&msg,sizeof(struct suntClkMsg)-sizeof(long),2,0);
	while(result==-1&&errno==EINTR);
	if(result==-1)
	{
		muntLogError("msgrcv",errno,strerror(errno),"[%d]",vuntClkMid);
		return -1;
	}
	*order=msg.order;
	return 0;
}

//驱动板消息队列标识.
int vuntDrvMid;
//液晶屏消息队列标识.
int vuntLcdMid;
//接口板消息队列标识.
int vuntIntMid;

//消息队列收发定时.
timer_t vuntMsgTimer1;
timer_t vuntMsgTimer2;
//消息队列超时标志.
int vuntMsgIsTimeout1;
int vuntMsgIsTimeout2;

//消息发送或接收超时.
#define cuntMsgTimeout1 (SIGRTMAX-1)
#define cuntMsgTimeout2 (SIGRTMAX-2)

void funtMsgHand1(int id)
{
	vuntMsgIsTimeout1=1;
	return;
}
void funtMsgHand2(int id)
{
	vuntMsgIsTimeout2=1;
	return;
}

/*========================================*\
    功能 : 消息队列定时器初始化
    参数 : (输入)空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgInit(void)
{
	int result;

	struct sigaction action;
	bzero(&action,sizeof(action));
	action.sa_handler=funtMsgHand1;
	result=sigaction(cuntMsgTimeout1,&action,NULL);
	if(result==-1)
	{
		muntLogError("sigaction",errno,strerror(errno),"[]");
		return -1;
	}
	action.sa_handler=funtMsgHand2;
	result=sigaction(cuntMsgTimeout2,&action,NULL);
	if(result==-1)
	{
		muntLogError("sigaction",errno,strerror(errno),"[]");
		return -1;
	}

	struct sigevent event;
	event.sigev_notify=SIGEV_SIGNAL;
	event.sigev_signo=cuntMsgTimeout1;
	result=timer_create(CLOCK_MONOTONIC,&event,&vuntMsgTimer1);
	if(result==-1)
	{
		muntLogError("timer_create",errno,strerror(errno),"[]");
		return -1;
	}
	event.sigev_signo=cuntMsgTimeout2;
	result=timer_create(CLOCK_MONOTONIC,&event,&vuntMsgTimer2);
	if(result==-1)
	{
		muntLogError("timer_create",errno,strerror(errno),"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 作为服务方接收消息
    参数 : (输入)模块名称
           (输入)消息队列结构
           (输入)定时序号
           (输出)行为 (阻塞/指定超时)>0
                      (阻塞/永不超时)0
                      (不阻塞)-1
           (输入)尝试次数
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtMsgSrvRcv(char *module,struct suntComMsg *msg,int timer,int action,int times)
{
	int result;

	int mid;
	if(strcmp(module,"drv")==0)
		mid=vuntDrvMid;
	else
	if(strcmp(module,"lcd")==0)
		mid=vuntLcdMid;
	else
	if(strcmp(module,"int")==0)
		mid=vuntIntMid;

	if(timer==1)
		vuntMsgIsTimeout1=0;
	else
		vuntMsgIsTimeout2=0;

	int isfinish=0;
	int i;
	for(i=0;i<times;i++)
	{
		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			it.it_value.tv_sec=action/1000;
			it.it_value.tv_nsec=action%1000*1000000;
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		int another;
		do
			another=msgrcv(mid,msg,sizeof(struct suntComMsg)-sizeof(long),1<<4,action>=0?0:IPC_NOWAIT);
		while(another==-1&&errno==EINTR&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0);

		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		if(another==-1&&errno!=ENOMSG&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0)
		{
			muntLogError("msgrcv",errno,strerror(errno),"[%d]",mid);
			return -1;
		}
		if((another==-1&&errno==ENOMSG)||(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==1)
			return 100;

		isfinish=1;
		break;
	}
	if(isfinish==0)
		return 100;

	muntLogEvent("[%s][%s]",funtLogDate(1),funtLogTime(3));
	muntLogEvent("com[%02X]src[%02X]dst[%02X]type[%02X]func[%02X]size[%4d]:",
		msg->com,msg->src,msg->dst,msg->type,msg->func,*(short*)msg->size);
	int m;
	for(m=0;m<*(short*)msg->size;m++)
		muntLogEvent("[%02X]",msg->data[m]);
	muntLogEvent("\n");

	return 0;
}

/*========================================*\
    功能 : 作为服务方发送消息
    参数 : (输入)模块名称
           (输入)消息队列结构
           (输入)定时序号
           (输出)行为 (阻塞/指定超时)>0
                      (阻塞/永不超时)0
                      (不阻塞)-1
           (输入)尝试次数
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtMsgSrvSnd(char *module,struct suntComMsg *msg,int timer,int action,int times)
{
	int result;

	int mid;
	if(strcmp(module,"drv")==0)
		mid=vuntDrvMid;
	else
	if(strcmp(module,"lcd")==0)
		mid=vuntLcdMid;
	else
	if(strcmp(module,"int")==0)
		mid=vuntIntMid;

	if(timer==1)
		vuntMsgIsTimeout1=0;
	else
		vuntMsgIsTimeout2=0;

	msg->index=1<<8;

	int isfinish=0;
	int i;
	for(i=0;i<times;i++)
	{
		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			it.it_value.tv_sec=action/1000;
			it.it_value.tv_nsec=action%1000*1000000;
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		int another;
		do
			another=msgsnd(mid,msg,sizeof(struct suntComMsg)-sizeof(long),action>=0?0:IPC_NOWAIT);
		while(another==-1&&errno==EINTR&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0);

		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		if(another==-1&&errno!=ENOMEM&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0)
		{
			muntLogError("msgsnd",errno,strerror(errno),"[%d]",mid);
			return -1;
		}
		if((another==-1&&errno==ENOMEM)||(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==1)
			return 100;

		isfinish=1;
		break;
	}
	if(isfinish==0)
		return 100;

	muntLogEvent("[%s][%s]",funtLogDate(1),funtLogTime(3));
	muntLogEvent("com[%02X]src[%02X]dst[%02X]type[%02X]func[%02X]size[%4d]:",
		msg->com,msg->src,msg->dst,msg->type,msg->func,*(short*)msg->size);
	int m;
	for(m=0;m<*(short*)msg->size;m++)
		muntLogEvent("[%02X]",msg->data[m]);
	muntLogEvent("\n");

	return 0;
}

/*========================================*\
    功能 : 作为客户方发送消息
    参数 : (输入)模块名称
           (输入)消息队列结构
           (输入)定时序号
           (输出)行为 (阻塞/指定超时)>0
                      (阻塞/永不超时)0
                      (不阻塞)-1
           (输入)尝试次数
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtMsgCliSnd(char *module,struct suntComMsg *msg,int timer,int action,int times)
{
	int result;

	int mid;
	if(strcmp(module,"drv")==0)
		mid=vuntDrvMid;
	else
	if(strcmp(module,"lcd")==0)
		mid=vuntLcdMid;
	else
	if(strcmp(module,"int")==0)
		mid=vuntIntMid;

	if(timer==1)
		vuntMsgIsTimeout1=0;
	else
		vuntMsgIsTimeout2=0;

	msg->index=1<<8;

	int isfinish=0;
	int i;
	for(i=0;i<times;i++)
	{
		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			it.it_value.tv_sec=action/1000;
			it.it_value.tv_nsec=action%1000*1000000;
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		int another;
		do
			another=msgsnd(mid,msg,sizeof(struct suntComMsg)-sizeof(long),action>=0?0:IPC_NOWAIT);
		while(another==-1&&errno==EINTR&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0);

		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		if(another==-1&&errno!=ENOMEM&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0)
		{
			muntLogError("msgsnd",errno,strerror(errno),"[%d]",mid);
			return -1;
		}
		if((another==-1&&errno==ENOMEM)||(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==1)
			return 100;

		isfinish=1;
		break;
	}
	if(isfinish==0)
		return 100;

	muntLogEvent("[%s][%s]",funtLogDate(1),funtLogTime(3));
	muntLogEvent("com[%02X]src[%02X]dst[%02X]type[%02X]func[%02X]size[%4d]:",
		msg->com,msg->src,msg->dst,msg->type,msg->func,*(short*)msg->size);
	int m;
	for(m=0;m<*(short*)msg->size;m++)
		muntLogEvent("[%02X]",msg->data[m]);
	muntLogEvent("\n");

	return 0;
}

/*========================================*\
    功能 : 作为客户方接收消息
    参数 : (输入)模块名称
           (输入)消息队列结构
           (输入)定时序号
           (输出)行为 (阻塞/指定超时)>0
                      (阻塞/永不超时)0
                      (不阻塞)-1
           (输入)尝试次数
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtMsgCliRcv(char *module,struct suntComMsg *msg,int timer,int action,int times)
{
	int result;

	int mid;
	if(strcmp(module,"drv")==0)
		mid=vuntDrvMid;
	else
	if(strcmp(module,"lcd")==0)
		mid=vuntLcdMid;
	else
	if(strcmp(module,"int")==0)
		mid=vuntIntMid;

	if(timer==1)
		vuntMsgIsTimeout1=0;
	else
		vuntMsgIsTimeout2=0;

	int isfinish=0;
	int i;
	for(i=0;i<times;i++)
	{
		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			it.it_value.tv_sec=action/1000;
			it.it_value.tv_nsec=action%1000*1000000;
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		int another;
		do
			another=msgrcv(mid,msg,sizeof(struct suntComMsg)-sizeof(long),msg->func|msg->com<<8|1<<16,action>=0?0:IPC_NOWAIT);
		while(another==-1&&errno==EINTR&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0);

		if(action>0)
		{
			struct itimerspec it;
			bzero(&it,sizeof(it));
			result=timer_settime((timer==1?vuntMsgTimer1:vuntMsgTimer2),0,&it,NULL);
			if(result==-1)
			{
				muntLogError("timer_settime",errno,strerror(errno),"[]");
				return -1;
			}
		}

		if(another==-1&&errno!=ENOMSG&&(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==0)
		{
			muntLogError("msgrcv",errno,strerror(errno),"[%d]",mid);
			return -1;
		}
		if((another==-1&&errno==ENOMSG)||(timer==1?vuntMsgIsTimeout1:vuntMsgIsTimeout2)==1)
			continue;

		isfinish=1;
		break;
	}
	if(isfinish==0)
		return 100;

	muntLogEvent("[%s][%s]",funtLogDate(1),funtLogTime(3));
	muntLogEvent("com[%02X]src[%02X]dst[%02X]type[%02X]func[%02X]size[%4d]:",
		msg->com,msg->src,msg->dst,msg->type,msg->func,*(short*)msg->size);
	int m;
	for(m=0;m<*(short*)msg->size;m++)
		muntLogEvent("[%02X]",msg->data[m]);
	muntLogEvent("\n");

	return 0;
}

/*========================================*\
    功能 : 创建消息队列
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgCreate(char *module)
{
	int result;

	char msgkey[16];
	sprintf(msgkey,"%smsg",module);
	int msg;
	funtIniIntGet("develop",msgkey,&msg);

	int mid;
	mid=msgget(msg,0);
	if(mid==-1&&errno!=ENOENT)
	{
		muntLogError("msgget",errno,strerror(errno),"[%d]",msg);
		return -1;
	}
	else
	if(mid!=-1)
	{
		result=msgctl(mid,IPC_RMID,NULL);
		if(result==-1)
		{
			muntLogError("msgctl",errno,strerror(errno),"[%d]",mid);
			return -1;
		}
	}

	mid=msgget(msg,IPC_CREAT|0600);
	if(mid==-1)
	{
		muntLogError("msgget",errno,strerror(errno),"[%d]",msg);
		return -1;
	}

	struct msqid_ds status;
	result=msgctl(mid,IPC_STAT,&status);
	if(result==-1)
	{
		muntLogError("msgctl",errno,strerror(errno),"[%d]",mid);
		return -1;
	}
	char sizekey[64];
	sprintf(sizekey,"%smsgsize",module);
	int msgsize;
	funtIniIntGet("develop",sizekey,&msgsize);
	status.msg_qbytes=msgsize;
	result=msgctl(mid,IPC_SET,&status);
	if(result==-1)
	{
		muntLogError("msgctl",errno,strerror(errno),"[%d]",mid);
		return -1;
	}

	if(strcmp(module,"drv")==0)
		vuntDrvMid=mid;
	else
	if(strcmp(module,"lcd")==0)
		vuntLcdMid=mid;
	else
	if(strcmp(module,"int")==0)
		vuntIntMid=mid;
	else
	if(strcmp(module,"clk")==0)
		vuntClkMid=mid;

	return 0;
}

/*========================================*\
    功能 : 删除消息队列
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgRemove(char *module)
{
	int result;

	char msgkey[16];
	sprintf(msgkey,"%smsg",module);
	int msg;
	funtIniIntGet("develop",msgkey,&msg);

	int mid;
	mid=msgget(msg,0);
	if(mid==-1&&errno!=ENOENT)
	{
		muntLogError("msgget",errno,strerror(errno),"[%d]",msg);
		return -1;
	}
	else
	if(mid!=-1)
	{
		result=msgctl(mid,IPC_RMID,NULL);
		if(result==-1)
		{
			muntLogError("msgctl",errno,strerror(errno),"[%d]",mid);
			return -1;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 显示消息队列
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgList(char *module)
{
	char msgkey[16];
	sprintf(msgkey,"%smsg",module);
	int msg;
	funtIniIntGet("develop",msgkey,&msg);

	int mid;
	mid=msgget(msg,0);
	if(mid==-1&&errno!=ENOENT)
	{
		muntLogError("msgget",errno,strerror(errno),"[%d]",msg);
		return -1;
	}
	else
	if(mid==-1&&errno==ENOENT)
	{
		printf("%s[-]\n",msgkey);
		return 0;
	}
	else
	if(mid!=-1)
	{
		printf("%s[%d]\n",msgkey,mid);
		return 0;
	}

	return 0;
}

/*========================================*\
    功能 : 创建共享内存
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtShmCreate(void)
{
	int result;
	int sid;
	sid=shmget(10000,0,0);
	if(sid==-1&&errno!=ENOENT)
	{
		muntLogError("shmget",errno,strerror(errno),"[]");
		return -1;
	}
	else
	if(sid!=-1)
	{
		result=shmctl(sid,IPC_RMID,NULL);
		if(result==-1)
		{
			muntLogError("shmctl",errno,strerror(errno),"[%d]",sid);
			return -1;
		}
	}
	sid=shmget(10000,sizeof(struct sbsnMemory),IPC_CREAT|0600);
	if(sid==-1)
	{
		muntLogError("shmget",errno,strerror(errno),"[]");
		return -1;
	}
	vbsnMemory=(struct sbsnMemory*)shmat(sid,0,0);
	if(vbsnMemory==(struct sbsnMemory*)-1)
	{
		muntLogError("shmat",errno,strerror(errno),"[]");
		return -1;
	}
	memset(vbsnMemory,0,sizeof(struct sbsnMemory));
	return 0;
}

/*========================================*\
    功能 : 删除共享内存
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtShmRemove(void)
{
	int result;
	int sid;
	sid=shmget(10000,0,0);
	if(sid==-1&&errno!=ENOENT)
	{
		muntLogError("shmget",errno,strerror(errno),"[]");
		return -1;
	}
	else
	if(sid!=-1)
	{
		result=shmctl(sid,IPC_RMID,NULL);
		if(result==-1)
		{
			muntLogError("shmctl",errno,strerror(errno),"[%d]",sid);
			return -1;
		}
	}
	return 0;
}

/*========================================*\
    功能 : 显示共享内存
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtShmList(void)
{
	int sid;
	sid=shmget(10000,0,0);
	if(sid==-1&&errno!=ENOENT)
	{
		muntLogError("shmget",errno,strerror(errno),"[]");
		return -1;
	}
	else
	if(sid==-1&&errno==ENOENT)
	{
		printf("[-]\n");
		return 0;
	}
	else
	if(sid!=-1)
	{
		printf("[%d]\n",sid);
		return 0;
	}
	return 0;
}

/*========================================*\
    功能 : 启动服务进程
    参数 : (输入)模块名称
           (输入)处理函数
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtSrvBoot(char *module,void *hand)
{
	int result;
	int pid;
	funtIniIntGet("deamon",module,&pid);
	if(pid!=0)
	{
		result=sigqueue(pid,0,(union sigval)0);
		if(result==-1&&errno!=ESRCH)
		{
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",pid);
			return -1;
		}
		else
		if(result==0)
			return 0;
	}

	pid=fork();
	if(pid==-1)
	{
		muntLogError("fork",errno,strerror(errno),"[]");
		return -1;
	}
	if(pid==0)
	{
		char eventpath[64];
		sprintf(eventpath,"%s/log/%s.event",getenv("ASC"),module);
		char errorpath[64];
		sprintf(errorpath,"%s/log/%s.error",getenv("ASC"),module);
		funtLogInit(eventpath,errorpath);

		funtIniIntSet("deamon",module,getpid());

		if(strstr(module,"com")==NULL)
			((int(*)(void))hand)();
		else
			((int(*)(char*))hand)(module);

		exit(-1);
	}
	
	return 0;
}

/*========================================*\
    功能 : 停止服务进程
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtSrvShut(char *module)
{
	int result;
	int pid;
	funtIniIntGet("deamon",module,&pid);
	if(pid!=0)
	{
		result=sigqueue(pid,SIGKILL,(union sigval)0);
		if(result==-1&&errno!=ESRCH)
		{
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",pid);
			return -1;
		}
	}
	return 0;
}

/*========================================*\
    功能 : 显示服务进程
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtSrvList(char *module)
{
	int result;
	int pid;
	funtIniIntGet("deamon",module,&pid);
	if(pid!=0)
	{
		result=sigqueue(pid,0,(union sigval)0);
		if(result==-1&&errno!=ESRCH)
		{
			muntLogError("sigqueue",errno,strerror(errno),"[%d]",pid);
			return -1;
		}
		else
		if(result==-1&&errno==ESRCH)
		{
			printf("%s[-]\n",module);
			return 0;
		}
		if(result==0)
		{
			printf("%s[%d]\n",module,pid);
			return 0;
		}
	}
	else
	if(pid==0)
	{
		printf("%s[-]\n",module);
		return 0;
	}
	return 0;
}
