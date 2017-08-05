/*========================================*\
    文件 : bsn.h
    作者 : 陈乐群
\*========================================*/

#ifndef __BSN__
#define __BSN__

//模式切换信号.
#define cbsnNextMode (SIGRTMIN+0)
//步骤切换信号.
#define cbsnNextTime (SIGRTMIN+1)
//步进按键信号.
#define cbsnStepEnter (SIGRTMIN+2)
#define cbsnStepLeave (SIGRTMIN+3)
//行人按键信号.
#define cbsnManPress (SIGRTMIN+4)

//定时数据.
struct sbsnTime
{
	//步骤时间.
	int time;
	//是否驱动板步骤.
	int isdrvstep;
	//是否液晶屏步骤.
	int islcdstep;
	//是否倒计时步骤.
	int iscntstep;
	//是否常绿步骤.
	int isgreen;
	//是否感应步骤.
	int isadapt;
	//开始时间.
	time_t start;
};

//阶段数据.
struct sbsnStage
{
	//相位位图.
	char phase[32];

	//常绿时间.
	int greenlight;

	//绿闪时间.
	int greenflash;
	//常黄时间.
	int yellow;
	//常红时间.
	int red;

	//行人常亮时间.
	int manlight;
	//行人闪烁时间.
	int manflash;

	//单位延长时间.
	int delta;
	//最小绿灯时间.
	int mingreen;
	//最大绿灯时间.
	int maxgreen;

	//时间总数.
	int timecount;
	//时间序号.
	int timeindex;

	//驱动板步骤序号.
	int drvstepindex;
	//液晶屏步骤序号.
	int lcdstepindex;
	//倒计时步骤序号.
	int cntstepindex;

	//驱动板步骤序号.
	int drvstepcount;
	//液晶屏步骤序号.
	int lcdstepcount;
	//倒计时步骤序号.
	int cntstepcount;

	//定时数据.
	struct sbsnTime times[10];
	//驱动板步骤数据:常亮点位状态.
	unsigned long long lights[5];
	//驱动板步骤数据:闪烁点位状态.
	unsigned long long flashs[5];
	//液晶屏步骤数据:通道点位状态.
	unsigned long long colors[3];
	//倒计时步骤数据:倒数点位状态.
	unsigned long long counts[2];
	//驱动板步骤数据:步骤时间.
	int drvtimes[5];
	//液晶屏步骤数据:步骤时间.
	int lcdtimes[3];
};

//方案数据.
struct sbsnScheme
{
	//是否感应.
	int isadapt;
	//是否固定.
	int isfixed;

	//计划序号.
	int planindex;
	//时段表号.
	int periodtable;
	//时段序号.
	int periodindex;

	//方案序号.
	int schemeindex;
	//方案周期(用于统计感应模式).
	int schemecycle;
	//方案周期(实际所有阶段总和).
	int schemetotal;

	//阶段表号.
	int stagetable;
	//阶段序号.
	int stageindex;
	//阶段总数.
	int stagecount;

	//驱动板步骤总数.
	int drvstepcount;
	//驱动板步骤序号.
	int drvstepindex;
	//液晶屏步骤总数.
	int lcdstepcount;
	//液晶屏步骤序号.
	int lcdstepindex;

	//阶段数据.
	struct sbsnStage stages[16];
};

struct sbsnMemory
{
	//刷新标志.
	int flush;
	//临时方案数据.
	struct sbsnScheme tmpscheme;
	//正式方案数据.
	struct sbsnScheme fmlscheme;
	//临时车检器数据.
	int tmpvehinfo[32];
	//正式车检器数据.
	int fmlvehinfo[32];
};
extern struct sbsnMemory *vbsnMemory;

/*========================================*\
    功能 : 控制检查
    参数 : (输入)模块
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnCtrl(char *module);

/*========================================*\
    功能 : 手动切换
    参数 : (输入)模块
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnHand(char *module);

/*========================================*\
    功能 : 时间检查
    参数 : 空
    返回 : (成功)0
           (失败)-1
           (拒绝)100
\*========================================*/
int fbsnTime(void);

/*========================================*\
    功能 : 计划检查
    参数 : 空
    返回 : (成功/临界时间)0
           (失败)-1
           (不是临界时间)100
\*========================================*/
int fbsnPlan(void);

/*========================================*\
    功能 : 获取方案
    参数 : (输出)方案数据
    返回 : (成功)0
           (失败)-1
           (缺失)100
\*========================================*/
int fbsnSchemeGet(struct sbsnScheme *scheme);

/*========================================*\
    功能 : 设置方案
    参数 : (输出)方案数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnSchemeSet(struct sbsnScheme *scheme);

/*========================================*\
    功能 : 获取点位
    参数 : (输入)方案数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnNode(struct sbsnScheme *scheme);

/*========================================*\
    功能 : 特殊点位设置
    参数 : (输入)方案数据
           (输入)模式 (全红)r
                      (黄闪)y
                      (关灯)c
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnSpecialNode(struct sbsnScheme *scheme,char mode);

/*========================================*\
    功能 : 模式设置
    参数 : (输入)模式 (定周期)0X41-0X60
                      (分时段)p
                      (自适应)a
                      (全红)r
                      (黄闪)y
                      (关灯)c
                      (人行)m
                      (步进)s
    返回 : (成功)0
           (失败)-1
           (拒绝)100
\*========================================*/
int fbsnMode(char mode);

/*========================================*\
    功能 : 生成协议
    参数 : (输入)串口序号
           (输入)来源代码
           (输入)目的代码
           (输入)消息类型
           (输入)消息代码
           (输入)方案数据
           (输出)消息结构
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fbsnProt(char com,char src,char dst,char type,char func,struct sbsnScheme *scheme,struct suntComMsg *msg);

#endif
