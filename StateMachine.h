#ifndef  _STATE_MACHINE_H_
#define  _STATE_MACHINE_H_

typedef enum
{
	POWER_OFF,		//断电
    SLEEPING,       //睡眠
	WAITING,		//等待加热
	HEATING,		//加热状态
	SETTING1,		//设置
	SETTING2,
	//ERROR,			//错误发生,不再加热
}E_SYSEM_STAT;
volatile E_SYSEM_STAT g_eSysStatus = POWER_OFF;//系统的状态


//=========== power off status
volatile uchar g_cLedAllOnCount = 150;//1S = 50 * 20ms


typedef enum
{
	ActIdle,

//==public action
	StartKey,//开始键
	SetKey,
	IncSKey,// + 键短按
	DecSKey,
	IncLKey,// + 键长按
	DecLKey,
	OkKey,//旋钮中间的短按
	OkLKey,//旋钮中间的长按

	Tim05STrig,//0.5S触发
	Tim1STrig,//1S触发
		
//==sleep status

//==waitting status
	Tim3MinTrig,//3min无动作触发,进入sleep	
	Tim5STrig,//5S触发
	
//==heating status
	LiftKettle,//提壶
	PBackKettle,//放回壶
	Tim10MinTrig,//提壶10min
	StartHeat,//从wait状态按键启动加热
	KeepWarm,//进入保温状态
	
//==setting status

}ACTION_Typedef;

volatile ACTION_Typedef g_xSysAction;

#define QUEUE_MAX_LEN 4
typedef struct
{
    ACTION_Typedef buf[QUEUE_MAX_LEN];
    uchar front;  // 队头下标
    uchar rear;   // 队尾下标
} Queue_t;

volatile Queue_t g_xActQueue={{0,0,0,0},0,0};
/**
 * @brief 入队（添加元素）
 */
bit bQueueEn(ACTION_Typedef val);

/**
 * @brief 出队（取出队首元素
 */
bit bQueueDe(ACTION_Typedef *pVal);

//==waitting status
volatile uchar g_cSwch2SleepTime;//无操作切换到sleep状态计时

//==heating status
volatile bit g_bKettleLift;//提壶的flag. 1,已提;0,未提
volatile uchar g_cLiftKettleTime;//提壶计时
volatile bit g_bKeepWarmFlag;//0.第一次加热;1.保温加热
volatile uchar g_cKeepWarmTime;//保温计时

//设置温度+,-
void vTemprIncrease(void);
void vTemprDecrease(void);


//状态处理函数
void vPowerOffStatus(void);
void vSleepStatus(void);
void vWaittingStatus(void);
void vHeatingStatus(void);
void vSetting1Status(void);
void vSetting2Status(void);

void vSystemStatus(void);


void vSysParam2Tft(uchar item);

#endif//_STATE_MACHINE_H_