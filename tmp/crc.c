/*========================================*\
    文件 : crc.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>

unsigned short crc16(unsigned char *data,int size)
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
	//return crc<<8&0XFF00|crc>>8&0X00FF;
}

int main(void)
{
	char a[]="\x01\x02";
	short b;
	b=crc16(a,2);
	printf("%x\n",*((unsigned char*)&b+0));
	printf("%x\n",*((unsigned char*)&b+1));
	return 0;
}
