/*========================================*\
    文件 : com.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "unt.h"
#include "bsn.h"

//消息队列描述符.
int vcomMid;
//串口描述符列表.
int vcomCid[4];

/*========================================*\
    功能 : 串口发送服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fcomSndDeamon(void)
{
	int result;

	while(1)
	{
		struct suntComMsg msg;
		result=msgrcv(vcomMid,&msg,sizeof(msg)-sizeof(long),1<<8,0);
		if(result==-1&&errno!=EINTR)
		{
			muntLogError("msgrcv",errno,strerror(errno),"[%d]",vcomMid);
			return -1;
		}

		short size;
		char data[1024];
		data[0]=0X5A;
		data[1]=0XA5;
		data[2]=msg.src;
		data[3]=msg.dst;
		size=*(short*)msg.size+2;
		data[4]=*((char*)&size+1);
		data[5]=*((char*)&size+0);
		data[6]=msg.type;
		data[7]=msg.func;
		memcpy(data+8,msg.data,*(short*)msg.size);
		*(short*)(data+8+*(short*)msg.size)=funtCRC16(data+2,6+*(short*)msg.size);
		data[10+*(short*)msg.size]=0XA5;
		data[11+*(short*)msg.size]=0X5A;
		size=*(short*)msg.size+12;

		int remain;
		int record;

		remain=size;
		record=0;
		while(remain>0)
		{
			result=write(vcomCid[(int)msg.com],data+record,remain);
			if(result==-1)
			{
				muntLogError("write",errno,strerror(errno),"[]");
				return -1;
			}
			remain-=result;
			record+=result;
		}

		muntLogEvent("[%s][%s]",funtLogDate(1),funtLogTime(3));
		muntLogEvent("com[%02X]src[%02X]dst[%02X]type[%02X]func[%02X]size[%4d]:",
			msg.com,msg.src,msg.dst,msg.type,msg.func,*(short*)msg.size);
		int m;
		for(m=0;m<*(short*)msg.size;m++)
			muntLogEvent("[%02X]",msg.data[m]);
		muntLogEvent("\n");
	}

	return 0;
}

/*========================================*\
    功能 : 串口接收服务
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fcomRcvDeamon(void)
{
	int result;

	int epollid;
	epollid=epoll_create(1);
	if(epollid==-1)
	{
		muntLogError("epoll_create",errno,strerror(errno),"[]");
		return -1;
	}

	int i;
	for(i=0;i<4;i++)
	{
		if(vcomCid[i]==0)
			continue;

		struct epoll_event event;
		event.events=EPOLLIN|EPOLLERR|EPOLLHUP;
		event.data.fd=vcomCid[i]*100+i;
		result=epoll_ctl(epollid,EPOLL_CTL_ADD,vcomCid[i],&event);
		if(result==-1)
		{
			muntLogError("epoll_ctl",errno,strerror(errno),"[]");
			return -1;
		}
	}

	while(1)
	{
		struct epoll_event events[256];
		int count;
		count=epoll_wait(epollid,events,sizeof(events)/sizeof(struct epoll_event),-1);
		if(count==-1)
		{
			muntLogError("epoll_wait",errno,strerror(errno),"[]");
			return -1;
		}

		int i;
		for(i=0;i<count;i++)
		{
			if(events[i].events&EPOLLERR||events[i].events&EPOLLHUP)
			{
				close(events[i].data.fd/100);
				epoll_ctl(epollid,EPOLL_CTL_DEL,events[i].data.fd/100,NULL);
				continue;
			}

			short size;
			char data[1024];

			int remain;
			int record;

			result=read(events[i].data.fd/100,data+0,1);
			if(result==-1&&errno==EAGAIN)
				goto end;
			if(result==-1&&errno!=EAGAIN)
			{
				muntLogError("read",errno,strerror(errno),"[]");
				return -1;
			}
			if(data[0]!=0X5A)
				continue;
			result=read(events[i].data.fd/100,data+1,1);
			if(result==-1&&errno==EAGAIN)
				goto end;
			if(result==-1&&errno!=EAGAIN)
			{
				muntLogError("read",errno,strerror(errno),"[]");
				return -1;
			}
			if(data[1]!=0XA5)
				continue;

			remain=4;
			record=2;
			while(remain>0)
			{
				result=read(events[i].data.fd/100,data+record,remain);
				if(result==-1&&errno==EAGAIN)
					goto end;
				if(result==-1&&errno!=EAGAIN)
				{
					muntLogError("read",errno,strerror(errno),"[]");
					return -1;
				}
				remain-=result;
				record+=result;
			}

			*((char*)&size+0)=data[5];
			*((char*)&size+1)=data[4];

			remain=size+4;
			record=6;
			while(remain>0)
			{
				result=read(events[i].data.fd/100,data+record,remain);
				if(result==-1&&errno==EAGAIN)
					goto end;
				if(result==-1&&errno!=EAGAIN)
				{
					muntLogError("read",errno,strerror(errno),"[]");
					return -1;
				}
				remain-=result;
				record+=result;
			}

			if(data[0]!=0X5A||data[1]!=0XA5)
			{
				muntLogError("",0,"[head error]","");
				continue;
			}
			if(data[record-2]!=0XA5||data[record-1]!=0X5A)
			{
				muntLogError("",0,"[tail error]","");
				continue;
			}
			if(size!=record-10)
			{
				muntLogError("",0,"[size error]","");
				continue;
			}
			if(0)
			{
				muntLogError("",0,"[crc16 error]","");
				continue;
			}

			struct suntComMsg msg;
			msg.com=events[i].data.fd%100;
			msg.src=data[2];
			msg.dst=data[3];
			msg.type=data[6];
			msg.func=data[7];
			*(short*)msg.size=size-2;
			memcpy(msg.data,data+8,*(short*)msg.size);

			if(msg.type==0X02)
				msg.index=msg.func|msg.com<<8|1<<16;
			else
			if(msg.type==0X03)
				msg.index=1<<4;

			result=msgsnd(vcomMid,&msg,sizeof(msg)-sizeof(long),0);
			if(result==-1)
			{
				muntLogError("msgsnd",errno,strerror(errno),"[%d]",vcomMid);
				return -1;
			}

			muntLogEvent("[%s][%s]",funtLogDate(1),funtLogTime(3));
			muntLogEvent("com[%02X]src[%02X]dst[%02X]type[%02X]func[%02X]size[%4d]:",
				msg.com,msg.src,msg.dst,msg.type,msg.func,*(short*)msg.size);
			int m;
			for(m=0;m<*(short*)msg.size;m++)
				muntLogEvent("[%02X]",msg.data[m]);
			muntLogEvent("\n");

			end:;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 串口服务
    参数 : (输入)模块
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fcomDeamon(char *module)
{
	int result;


	struct termios tio;
	bzero(&tio,sizeof(tio));
	tio.c_cflag|=CLOCAL;
	tio.c_cflag|=CREAD;
	tio.c_cflag&=~CSIZE;
	tio.c_cflag&=~PARENB;
	tio.c_cflag|=CS8;
	tio.c_cflag&=~CSTOPB;
	tio.c_cc[VTIME]=0;
	tio.c_cc[VMIN]=1;
	cfsetispeed(&tio,B115200);
	cfsetospeed(&tio,B115200);

	char comkey[16];
	sprintf(comkey,"%.3scom",module+3);
	char ccom[64];
	funtIniStrGet("develop",comkey,ccom);

	bzero(vcomCid,sizeof(vcomCid));
	int i;
	for(i=0;i<strlen(ccom+3);i++)
	{
		char path[64];
		sprintf(path,"/dev/tty%.3s%c",ccom,*(ccom+3+i));
		vcomCid[i]=open(path,O_RDWR|O_NOCTTY);
		if(vcomCid[i]==-1)
		{
			muntLogError("open",errno,strerror(errno),"[%s]",path);
			return -1;
		}
		result=tcsetattr(vcomCid[i],TCSANOW,&tio);
		if(result==-1)
		{
			muntLogError("tcsetattr",errno,strerror(errno),"[]");
			return -1;
		}
		tcflush(vcomCid[i],TCIOFLUSH);
	}

	if(strstr(module,"drv")!=NULL)
		vcomMid=vuntDrvMid;
	else
	if(strstr(module,"lcd")!=NULL)
		vcomMid=vuntLcdMid;
	else
	if(strstr(module,"int")!=NULL)
		vcomMid=vuntIntMid;

	if(strstr(module,"snd")!=NULL)
		result=fcomSndDeamon();
	else
	if(strstr(module,"rcv")!=NULL)
		result=fcomRcvDeamon();

	return result;
}
