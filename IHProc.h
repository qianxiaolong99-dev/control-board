#ifndef  _IH_PROCESS_H_
#define  _IH_PROCESS_H_


// ==================== 功率相关 ======================
//volatile uchar PowerWork;//加热的功率 = PowerWork * 10
typedef enum
{
	E_NONE,
	E_VOL_LOW,
	E_VOL_HIGH,
	E_NTC_MOS,
	E_NTC_TOP_OPEN,
	E_NTC_TOP_SHORT,
	E_NO_CIRCLE,	//未接线盘
	E_NO_POT,
	E_LUMIAN_OVERTEMPR,	//炉面过温
	E_MOS_OVERTEMPR,	//MOS过温
}IHERR_Typedef;
volatile IHERR_Typedef g_cErrStatus = E_NONE;

//采用设置的功率加热pow最大800
//pow输入值可以为20,30,40,50,60,70,80
typedef enum
{
	POW_0W=0,
	POW_100W=10,
	POW_200W=20,
	POW_300W=30,
	POW_400W=40,
	POW_500W=50,
	POW_600W=60,
	POW_700W=70,
	POW_800W=80
}POWVAL_EnumDef;
void vSetIHPower(POWVAL_EnumDef pow);

/*控制蜂鸣器:bShortLong:0,短鸣;1,长鸣*/
void vSetBuzzer(uchar bShortLong);

//==================== 通讯 ======================
void RecvDataAnalysis(void);


// ==================== NTC相关 ======================
//定义炉面温度NTC的ADC值
#define LUMIAN_TEMPE7_ADC	    6
#define LUMIAN_TEMPE50_ADC	    32
#define LUMIAN_TEMPE60_ADC	    44
#define LUMIAN_TEMPE80_ADC		74
#define LUMIAN_TEMPE100_ADC		111
#define LUMIAN_TEMPE110_ADC		130
#define LUMIAN_TEMPE120_ADC		147
#define LUMIAN_TEMPE140_ADC		178
#define LUMIAN_TEMPE160_ADC		201
#define LUMIAN_TEMPE180_ADC		218
#define LUMIAN_TEMPE200_ADC		229
#define LUMIAN_TEMPE220_ADC		237
#define LUMIAN_TEMPE240_ADC		242

#define MOS_TEMPE50_ADC		91
#define MOS_TEMPE60_ADC		114
#define MOS_TEMPE80_ADC		157
#define MOS_TEMPE100_ADC	192
#define MOS_TEMPE110_ADC	205
#define MOS_TEMPE115_ADC	210




#endif//_IH_PROCESS_H_