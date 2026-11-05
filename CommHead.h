
#ifndef COMMUNICATION_HEAD_H_H
#define COMMUNICATION_HEAD_H_H

#include "mytype.h"
#include <cms.h>
#include <string.h>

typedef union
{
    unsigned char  all;
    struct
    {
       unsigned b0:1;
       unsigned b1:1;
       unsigned b2:1;
       unsigned b3:1;
       unsigned b4:1;
       unsigned b5:1;
	   unsigned b6:1;
       unsigned b7:1;
    }one;
}bits;

#define ENCODE_PIN  RB6
#define DECODE_PIN	RB5		//定义RB5口为解码用的端口
#define SET_DECODE_PIN_AS_INPUT		TRISB |= 0x20;
#define SET_DECODE_PIN_PULLUP		WPUB |= 0x20;


void vInitDecoding(void);
void vClearDecodingParam(void);
void vDecodeInTimer();

volatile uchar g_cReceiveData[4];//把解码出来的数据取出来
bit GetDecodeCode(void);//取出的数据存放在g_cReceiveData

//用于bit decode
#define HEAD_HIGHCOUNT_MIN		77	//79*UNIT(125us)

#define BIT0_LOWCOUNT_MIN		2	//4
#define BIT0_LOWCOUNT_MAX		6   //4
#define BIT0_HIGHCOUNT_MIN		10	//12
#define BIT0_HIGHCOUNT_MAX		14	//12
#define BIT1_HIGHCOUNT_MIN		BIT0_LOWCOUNT_MIN
#define BIT1_HIGHCOUNT_MAX		BIT0_LOWCOUNT_MAX
#define BIT1_LOWCOUNT_MIN		BIT0_HIGHCOUNT_MIN
#define BIT1_LOWCOUNT_MAX		BIT0_HIGHCOUNT_MAX

volatile uchar g_cDecodePinHcnt = 0;	//高电平计数
volatile uchar g_cDecodePinLcnt = 0;
volatile bit g_bPrevPinValue = 0;			//上一次引脚电平
volatile bit g_bCurrPinValue = 0;			//当前引脚电平
volatile bit g_bBitVal0Decoding = 0;	//判断bit=0的启动flag
volatile bit g_bBitVal1Decoding = 0;	//判断bit=1的启动flag

//用于data 组合
volatile bit g_bDecodBody = 0;			//头码成功,解码数据flag
volatile uchar g_cDecodedPosition = 0;	//解码bits
volatile uchar g_cDecodeData[4];//解码输出的值
volatile bit g_bDecodeSuccess = 0;//解码成功flag



volatile uchar SendFifo[4];//缓冲设置变更
volatile ulong g_lSendData = 0;//中断发送的数据
volatile bit g_bIsAllowedCopyData;//=1;//是否允许给中断发送的数据赋值
volatile bit g_bIsAllowedGetData;//=0;//是否允许中断获取数据
volatile bit g_bSendCmdHead;
volatile bit g_bSendCmdBody;
volatile uchar g_cSendCommNo = 0;//发送命令的位置

void vTxDataReady(void);
void vIntimeCopyData(void);
void vIntSendData(void);

//-------------------------------------------------------------------
//发送内容
volatile bits Work_Flag1;//工作Flag1
#define B_Heat_En			Work_Flag1.one.b0//加热使能 bit0
#define B_Fan_En			Work_Flag1.one.b1//风扇使能 bit1
#define B_Buzz_En			Work_Flag1.one.b2//蜂鸣器使能 bit2
#define B_Buzz_S_Out		Work_Flag1.one.b3//蜂鸣器短鸣1声（200ms）
#define B_Buzz_L_Out		Work_Flag1.one.b4//长鸣1声（400ms
#define B_HeatStop_CheckPan	Work_Flag1.one.b5//不加热但检锅 bit5
#define B_HeatStop_NoCheck	Work_Flag1.one.b6//不加热不检锅 bit6
#define B_PPGDP_Dis			Work_Flag1.one.b7//PPG抖频禁止 bit7

volatile bits Work_Flag2;//工作Flag2
#define B_PanOver_Dis		Work_Flag2.one.b0//锅底NTC超温判断禁止
#define B_PanFail_Dis		Work_Flag2.one.b1//锅底NTC失效判断禁止
#define B_PPGDP_Can			Work_Flag2.one.b7//PPG抖频强制使能

volatile uchar PowerWork;//加热功率值

volatile uchar Num_LxLowOn;//开功率次数
volatile uchar Num_LxLowOff;//关功率次数

//volatile uchar SlopeCurr_Adjust;//电流斜率调整值


//接收内容
#define BitIsTrue(Ram,Bit)	(Ram & (1<<Bit))

volatile bits Status1_Work;		//工作状态1
volatile bits Status2_Work;		//工作状态2	
#define B_Pan_Error			Status1_Work.one.b0//未接线盘
#define B_Pan_No			Status1_Work.one.b1//无锅
#define B_Pan_Have			Status1_Work.one.b2//有锅
#define B_Pan_Steel			Status1_Work.one.b3//钢锅
#define B_Pan_Iron			Status1_Work.one.b4//铁锅
#define B_Zero_No			Status1_Work.one.b5//无过零
#define B_Zero_60Hz			Status1_Work.one.b6//过零频率 = 60Hz
#define B_Protect_BackPress	Status1_Work.one.b7//过压保护	

#define B_Protect_Stop		Status2_Work.one.b0//保护停止加热	
#define B_PPGStop_CheckPan	Status2_Work.one.b1//加热暂停但检锅
#define B_PPGStop_NoCheck	Status2_Work.one.b2//加热暂停不检锅
#define Status_PPGDP		Status2_Work.one.b3//抖频开启
#define Status_LxLow		Status2_Work.one.b4//连续低功率开启


volatile bits Flag1_Error;		//硬件故障标志1
volatile bits Flag2_Error;		//硬件故障标志2
#define B_EVolLow			Flag1_Error.one.b0//市电电压过低
#define B_EVolHigh			Flag1_Error.one.b1//市电电压过高
#define B_EIgbtOpen			Flag1_Error.one.b2//IGBT-NTC开路
#define B_EIgbtClose		Flag1_Error.one.b3//IGBT-NTC短路
#define B_EIgbtOver			Flag1_Error.one.b4//IGBT-NTC超温
#define B_EPanOpen			Flag1_Error.one.b5//锅底NTC开路
#define B_EPanClose			Flag1_Error.one.b6//锅底NTC短路
#define B_EPanOver			Flag1_Error.one.b7//锅底NTC超温

#define B_EPanFail			Flag2_Error.one.b0//锅底NTC失效
#define B_ETopOpen			Flag2_Error.one.b1//顶部NTC开路
#define B_ETopClose			Flag2_Error.one.b2//顶部NTC短路
#define B_ETopFail			Flag2_Error.one.b3//顶部NTC失效
#define B_Comm_Error		Flag2_Error.one.b4//通信故障


volatile uchar Status_PPGStop;		//保护状态
volatile bit B_L1_OVER_VOL;//1级过压次数过多
volatile bit B_VOL_SURGE;//电压浪涌
volatile bit B_CUR_SURGE;//电流浪涌
volatile bit B_L2_OVER_VOL;//2级过压
volatile bit B_CONDUCTION_STEP_H;//导通台阶过高
volatile bit B_PPG_WTDOG_OVER;//PPG看门狗溢出

//volatile uchar AD_Volatage;		//市电电压AD（高4位）

//volatile uchar AD_Volatage ;		//市电电压AD（低8位）
//volatile uchar AD_Current ;		//工作电流AD（低8位）

volatile uchar AD_TmprPan ;		//锅底温度AD
volatile uchar AD_TmprIgbt ;		//IGBT温度AD

//volatile uchar AD_TmprTop ;		//顶部温度AD 
//volatile uchar AD_VRef ;		//内部基准电压（1.2V）AD

//volatile uchar Vol_Flo;		//市电电压 - 80V
//volatile uchar Count_Syn;		//检锅振荡个数	

//volatile uchar Time_Syn >> 8 = Buf_DataRece[2];		//检锅振荡脉宽（高8位） 
//volatile uint Time_Syn;// = Buf_DataRece[1];		//检锅振荡脉宽（低8位）		

//volatile uchar PowerReal >> 8 = Buf_DataRece[2];	//实际输出功率（高8位） 
//volatile uint PowerReal;// = Buf_DataRece[1];		//实际输出功率（低8位）	

//volatile uchar Buf_PPGTMR >> 8 = Buf_DataRece[2];;	//当前PPG导通值（高8位）
//volatile uint Buf_PPGTMR;// = Buf_DataRece[1];		//当前PPG导通值（低8位）

//volatile uchar Count_BackPress >> 8 = Buf_DataRece[2];;	//1级过压保护计数（高8位）
//volatile uint Count_BackPress;// = Buf_DataRece[1];		//1级过压保护计数（低8位）

//volatile uchar Count_IgbtProtect >> 8 = Buf_DataRece[2];;	//IGBT硬件保护计数（高8位）
//volatile uint Count_IgbtProtect;// = Buf_DataRece[1];		//IGBT硬件保护计数（低8位）

//volatile uchar Elec.flo.integer = Buf_DataRece[2];;	//电量高位
//volatile uint Elec_Flo;// = Buf_DataRece[1];	//电量低位	



//#define Flash_EEP






#endif














