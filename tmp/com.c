/*========================================*\
    文件 : com.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>

short crc16(char *data,int size)
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

int main(int argc,char *argv[])
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
	cfsetispeed(&tio,B115200);
	cfsetospeed(&tio,B115200);
	tio.c_cc[VTIME]=0;
	tio.c_cc[VMIN]=1;

	/*
	int i;
	for(i=0;i<8;i++)
	{
		printf("%d\n",i);
		char path[64];
		sprintf(path,"/dev/ttyUSB%d",i);
		int fd;
		fd=open(path,O_RDWR|O_NOCTTY);
		if(fd==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"open");
			continue;
		}
		result=tcsetattr(fd,TCSANOW,&tio);
		if(result==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"tcgetattr");
			return -1;
		}
		tcflush(fd,TCIOFLUSH);
	}
	return;
	*/

	/*
	while(1)
	{
		char data[1024];
		int size;

		//memcpy(data,"\x5A\xA5\x01\x05\x00\x0C\x01\xF8\x01\xCD\xCE\x01\x00\x00\x00\x01\x00\x00\x00\x00\xA5\x5A",22);
		//memcpy(data,"\x5A\xA5\x01\x05\x00\x0C\x01\xF1\x01\x6E\x6E\x05\x00\x00\x00\x00\x00\x00\x00\x00\xA5\x5A",22);
		//size=22;
		// *(short*)(data+18)=crc16(data+2,18);
		memcpy(data,"\x5A\xA5\x01\x02\x00\x03\x01\x01\x00\x00\x00\xA5\x5A",13);
		size=13;
		*(short*)(data+9)=crc16(data+2,7);

		int i;
		for(i=0;i<size;i++)
			printf("%02x",data[i]);
		printf("\n");

		size=write(fd,data,size);
		if(size==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"write");
			return -1;
		}

		printf("sendSize[%d]\n",size);

		size=read(fd,data,sizeof(data));
		if(size==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"read");
			return -1;
		}

		printf("recvSize[%d]\n",size);

		for(i=0;i<size;i++)
			printf("%02x",data[i]);
		printf("\n");

		printf("--------------------------------\n");
	}

	close(fd);
	*/

	int fd;
	fd=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY);
	if(fd==-1)
	{
		printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
		printf("[%d][%s][%s]\n",errno,strerror(errno),"open");
		return -1;
	}
	result=tcsetattr(fd,TCSANOW,&tio);
	if(result==-1)
	{
		printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
		printf("[%d][%s][%s]\n",errno,strerror(errno),"tcgetattr");
		return -1;
	}
	tcflush(fd,TCIOFLUSH);

	while(1)
	{
		char data[1024];
		int size;

		/*
		size=read(fd,data,sizeof(data));
		if(size==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"read");
			return -1;
		}

		printf("recvSize[%d]\n",size);

		int i;
		for(i=0;i<size;i++)
			printf("%02x",data[i]);
		printf("\n");

		printf("--------------------------------\n");
		*/

		memcpy(data,"\x5A\xA5\x01\x03\x00\x03\x01\x01\x01\x00\x00\xA5\x5A",13);
		size=13;
		*(short*)(data+9)=crc16(data+2,7);

		int i;
		for(i=0;i<size;i++)
			printf("%02x",data[i]);
		printf("\n");

		size=write(fd,data,size);
		if(size==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"write");
			return -1;
		}

		printf("sendSize[%d]\n",size);

		size=read(fd,data,sizeof(data));
		if(size==-1)
		{
			printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
			printf("[%d][%s][%s]\n",errno,strerror(errno),"read");
			return -1;
		}

		printf("recvSize[%d]\n",size);

		for(i=0;i<size;i++)
			printf("%02x",data[i]);
		printf("\n");

		printf("--------------------------------\n");
	}

	close(fd);
	return 0;
}
