#ifndef  _MAIN_H_
#define  _MAIN_H_

#include "mytype.h"
#include <cms.h>
#include <string.h>
#include "delay.h"
#include "Touch_Kscan_Library.h"
#include "REL_Sender.h"
#include "LED_Device_Demo.h"
#include "CommHead.h"
#include "StateMachine.h"
#include "UIProc.h"
#include "IHProc.h"
#include "PotProc.h"

// ==================== 系统设置 ======================
typedef struct
{
	uchar BoilingPoint;//沸点温度(跟高原设置相关)
	uchar SetTempr;//系统级的设置温度(Unit:0.5℃)
	//uchar CurTempr;//系统级的当前温度(Unit:0.5℃)
	
	uchar KWTime;//保温时间:0,30,60分
	uchar TemprUnit:1;//温度单位	
	uchar EnBeep:1;//提示音开关
	uchar EnGaoyuan:1;//高原加热开关
	uchar PreBoiling:1;//预沸腾开关
	//uchar BtStat:1;
}SYS_PARAM_Typedef;

#define MAX_TEMPR	200//最大温度100℃
#define MIN_BOILING_TEMPR	160//最小沸腾温度80℃

//系统设置
volatile SYS_PARAM_Typedef g_xSysSetting = {MAX_TEMPR, 184, 0, 0, 1, 0, 0};


#define MAX_SET_TEMPR_05C	188//94℃
#define MIN_SET_TEMPR_05C	80//40℃

#define MAX_SET_TEMPR_F		201//201℉
#define MIN_SET_TEMPR_F		104//104℉

//eeprom读写
bit Memory_Write(unsigned char Addr, unsigned char Value);
unsigned char Memory_Read(uchar Addr);

/*EEPROM位置说明*/
//地址 11,16 : 保温模式,值1,31,61
//地址 12,17 : 预沸腾,值1,2
//地址 13,18 : 温度单位,值1,2
//地址 14,19 : 提示音开关,值1,2
//地址 15,20 : 高原加热,值1,2
//地址 21,22 : 高原沸腾温度:默认200*0.5℃
typedef enum
{
	ADDR_KEEP_WARM_TIME =	11,
	ADDR_KEEP_WARM_TIME1,
	
	ADDR_PRE_BOILING =		13,
	ADDR_PRE_BOILING1,
	
	ADDR_TEMPR_UNIT =		15,
	ADDR_TEMPR_UNIT1,
	
	ADDR_SOUND_ONOFF =		17,
	ADDR_SOUND_ONOFF1,
	
	ADDR_GAOYUAN_ONOFF =	19,
	ADDR_GAOYUAN_ONOFF1,
	
	ADDR_TEMPR_BOILING =	21,
	ADDR_TEMPR_BOILING1,	
}SYSPARAM_ADDR_EnumDef;

void vReadSysCfg(void);


//==================== 加热 ======================
typedef struct
{
	uchar CurTempr;//系统级的当前温度(Unit:0.5℃)
	uchar TargTempr;//目标温度
	
	//这个变量作为初值用于计算吞掉的小球数量
	uchar GapTempr;//当前温度与设定温度温度差(仅start,放回壶时)
}IH_PROC_Typedef;
//需要加热到的目标温度 = 高原温度,最大温度,设置温度中的一个
volatile IH_PROC_Typedef g_xIhProcParam;


// ==================== 按键相关 ======================
#define CMS_STARTKEY    0x0001//1
#define CMS_SETKEY	    0x0002//2
#define CMS_INCREASEKEY	0x0020
#define CMS_DECREASEKEY	0x0004//0x04
#define CMS_OKKEY		0x0010

//---- 按键需要按下20ms才有效 ----
#define LONG_KEY_COUNT		70//长按1.5S后,长按有效
#define LONG_KEY_SHOT		10//0.2S处理一次长按的效果

volatile uchar  g_cDisableKeyTime;//按键按下后,需要多久才能进行下一个按键判断
volatile uchar  g_cLongPressCounter;//按键按下后的计数器
volatile bit	g_bKeyShortPressed;//短按处理flag

volatile bit    g_bStartKeyPressed;//power按键
volatile bit    g_bAddKeyPressed;//+
volatile bit    g_bDecKeyPressed;//-
volatile bit    g_bSetKeyPressed;//猛火键
volatile bit    g_bOkKeyPressed;//Time键




// ==================== 系统计时相关 ======================
typedef struct
{	
	uchar t20ms : 4;
	uchar t100ms : 4;
	uchar t500ms : 4;
	uchar t1S : 4;
	uchar t5S : 4;
	uchar t1Min : 4;
}LOOP_TIME_Typedef;
volatile LOOP_TIME_Typedef g_xLoopTime;
volatile uchar g_cT4msCounter=0;





// ==================== 其他 ======================


#endif














