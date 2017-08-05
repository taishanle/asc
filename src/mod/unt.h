/*========================================*\
    文件 : unt.h
    作者 : 陈乐群
\*========================================*/

#ifndef __UNT__
#define __UNT__

/*========================================*\
    功能 : 按位统计
    参数 : (输入)数据
    返回 : 位数
\*========================================*/
int funtBitCount(char *data);

/*========================================*\
    功能 : 校验计算
    参数 : (输入)数据内容
           (输入)数据长度
    返回 : 结果
\*========================================*/
short funtCRC16(char *data,int size);

/*========================================*\
    功能 : 网络参数BIN->ASC
    参数 : (输入)来源数据
           (输出)目的数据
           (输入)分隔字符
    返回 : 空
\*========================================*/
void funtNetBin2Asc(char *srcdata,char *dstdata,char split);
/*========================================*\
    功能 : 网络参数ASC->BIN
    参数 : (输入)来源数据
           (输出)目的数据
           (输入)分隔字符
    返回 : 空
\*========================================*/
void funtNetAsc2Bin(char *srcdata,char *dstdata,char split);
/*========================================*\
    功能 : 获取网络参数
    参数 : (输出)IP地址
           (输出)掩码
           (输出)网关
           (输出)MAC地址
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtNetGet(char *ip,char *mask,char *gate,char *mac);
/*========================================*\
    功能 : 设置网络参数
    参数 : (输入)IP地址
           (输入)掩码
           (输入)网关
           (输入)MAC地址
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtNetSet(char *ip,char *mask,char *gate,char *mac);

/*========================================*\
    功能 : 打印日志(任何版本都打印)
    参数 : (输入)日志文件句柄
           (输入)格式化字符串
    返回 : 空
\*========================================*/
void funtLogAnyhow(FILE *fp,char *format,...);
/*========================================*\
    功能 : 打印日志(调试版本才打印)
    参数 : (输入)日志文件句柄
           (输入)格式化字符串
    返回 : 空
\*========================================*/
void funtLogDepend(FILE *fp,char *format,...);
/*========================================*\
    功能 : 获取日期
    参数 : (输入)日期类型 (无分隔符/4位年份)0
                          (有分隔符/4位年份)1
                          (无分隔符/2位年份)2
                          (有分隔符/2位年份)3
    返回 : 日期
\*========================================*/
char *funtLogDate(int type);
/*========================================*\
    功能 : 获取时间
    参数 : (输入)时间类型 (无分隔符/秒)0
                          (有分隔符/秒)1
                          (无分隔符/微秒)2
                          (有分隔符/微秒)3
    返回 : 时间
\*========================================*/
char *funtLogTime(int type);

//打印事件日志.
#define muntLogEvent(format,argument...)\
	funtLogDepend(stdout,format,##argument)
//打印调试日志.
#define muntLogDebug(format,argument...)\
	funtLogDepend(stdout,"**[%s][%s][%s][%d]"format"\n",funtLogDate(1),funtLogTime(3),__FILE__,__LINE__,##argument)
//打印错误日志.
#define muntLogError(function,code,info,format,argument...)\
	funtLogAnyhow(stderr,"##[%s][%s]\n##[%s][%d]\n##[%s][%s][%d][%s]\n##"format"\n\n",\
	funtLogDate(1),funtLogTime(3),__FILE__,__LINE__,__FUNCTION__,function,code,info,##argument)

/*========================================*\
    功能 : 打开文件锁
    参数 : (输入)对象
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtLckInit(char *object);
/*========================================*\
    功能 : 文件锁加锁
    参数 : (输入)对象
           (输出)是否阻塞 (是)1
                          (否)0
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtLckLck(char *object,int action);
/*========================================*\
    功能 : 文件锁解锁
    参数 : (输入)锁文件
           (输出)是否阻塞 (是)1
                          (否)0
    返回 : (成功)0
           (失败)-1
           (资源不足)100
\*========================================*/
int funtLckUck(char *object,int action);

/*========================================*\
    功能 : 获取字符配置
    参数 : (输入)段
           (输入)键
           (输出)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniStrGet(char *sec,char *key,char *data);
/*========================================*\
    功能 : 获取数值配置
    参数 : (输入)段
           (输入)键
           (输出)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniIntGet(char *sec,char *key,int *data);
/*========================================*\
    功能 : 设置字符配置
    参数 : (输入)段
           (输入)键
           (输入)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniStrSet(char *sec,char *key,char *data);
/*========================================*\
    功能 : 设置数值配置
    参数 : (输入)段
           (输入)键
           (输入)数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtIniIntSet(char *sec,char *key,int data);

struct suntClkMsg
{
	//消息标识.
	long index;
	//指令代码.
	int order;
};

//定时消息队列标识.
extern int vuntClkMid;

/*========================================*\
    功能 : 发送定时指令
    参数 : (输入)指令 (设置时间)>=0
                      (获取时间)<0
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtClkTimeSnd(int order);
/*========================================*\
    功能 : 接收定时指令
    参数 : (输出)剩余时间
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtClkTimeRcv(int *order);

struct suntComMsg
{
	//消息标识.
	long index;
	//串口序号.
	char com;
	//来源代码.
	char src;
	//目的代码.
	char dst;
	//消息类型.
	char type;
	//指令代码.
	char func;
	//数据长度.
	char size[2];
	//数据内容.
	char data[1024];
};

//驱动板消息队列标识.
extern int vuntDrvMid;
//液晶屏消息队列标识.
extern int vuntLcdMid;
//接口板消息队列标识.
extern int vuntIntMid;

/*========================================*\
    功能 : 消息队列定时器初始化
    参数 : (输入)空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgInit(void);

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
int funtMsgSrvRcv(char *module,struct suntComMsg *msg,int timer,int action,int times);
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
int funtMsgSrvSnd(char *module,struct suntComMsg *msg,int timer,int action,int times);
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
int funtMsgCliSnd(char *module,struct suntComMsg *msg,int timer,int action,int times);
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
int funtMsgCliRcv(char *module,struct suntComMsg *msg,int timer,int action,int times);

/*========================================*\
    功能 : 创建消息队列
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgCreate(char *module);
/*========================================*\
    功能 : 删除消息队列
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgRemove(char *module);
/*========================================*\
    功能 : 显示消息队列
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtMsgList(char *module);

/*========================================*\
    功能 : 创建共享内存
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtShmCreate(void);
/*========================================*\
    功能 : 删除共享内存
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtShmRemove(void);
/*========================================*\
    功能 : 显示共享内存
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtShmList(void);

/*========================================*\
    功能 : 启动服务进程
    参数 : (输入)模块名称
           (输入)处理函数
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtSrvBoot(char *module,void *hand);
/*========================================*\
    功能 : 停止服务进程
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtSrvShut(char *module);
/*========================================*\
    功能 : 显示服务进程
    参数 : (输入)模块名称
    返回 : (成功)0
           (失败)-1
\*========================================*/
int funtSrvList(char *module);

#endif
