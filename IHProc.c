#include "main.h"

void vSetBuzzer(uchar bShortLong)
{
	if(!g_xSysSetting.EnBeep) return;
	
	g_cSendCommNo = 0;
	if(bShortLong)
	{
		B_Buzz_L_Out = 1;		
	}
	else
	{
		B_Buzz_S_Out = 1;
	}
}

void vSetIHPower(POWVAL_EnumDef pow)
{
	if(pow == POW_0W)
	{
		B_Heat_En = 0;
	}
	else
	{
		B_Heat_En = 1;
		B_Fan_En = 1;
		PowerWork = pow;
	}
}

void vTemprProc(void)
{
	static uchar s_cOverTempr1Count=0;
	static uchar s_cOverTempr2Count=0;
	//炉面>180℃
	if(LUMIAN_TEMPE180_ADC < AD_TmprPan)
	{
		s_cOverTempr1Count++;
		if(10 < s_cOverTempr1Count)
		{
			s_cOverTempr1Count = 10;
			vSetIHPower(POW_0W);
			//增加无错误判断,防止覆盖其他错误
			if(g_cErrStatus == E_NONE)
				g_cErrStatus = E_LUMIAN_OVERTEMPR;
		}
	}
	else
	{
		s_cOverTempr1Count = 0;
		//温度回到120℃以下,且炉面曾经过温
		if((g_cErrStatus == E_LUMIAN_OVERTEMPR) && (AD_TmprPan < LUMIAN_TEMPE120_ADC))
			g_cErrStatus = E_NONE;
	}
	
	//MOS>110℃
	if(MOS_TEMPE110_ADC < AD_TmprIgbt)
	{
		s_cOverTempr2Count++;
		if(10 < s_cOverTempr2Count)
		{
			s_cOverTempr2Count = 10;
			vSetIHPower(POW_0W);
			//增加无错误判断,防止覆盖其他错误
			if(g_cErrStatus == E_NONE)
				g_cErrStatus = E_MOS_OVERTEMPR;
		}
	}
	else
	{
		s_cOverTempr2Count = 0;
		//温度回到100℃以下,且MOS曾经过温
		if((g_cErrStatus == E_MOS_OVERTEMPR)&&(AD_TmprIgbt < MOS_TEMPE100_ADC))
			g_cErrStatus = E_NONE;
	}
}

void vErrorProc(void)
{
	static uchar s_cIgbtErrCount=0;
	static uchar s_cPanErrCount=0;
	
	if(B_EVolLow){
		g_cErrStatus = E_VOL_LOW;//"E4"
		vSetIHPower(POW_0W);
	}
	if(B_EVolHigh){
		g_cErrStatus = E_VOL_HIGH;//"E3"
		vSetIHPower(POW_0W);
	}
	if(B_EIgbtOpen || B_EIgbtClose){
		s_cIgbtErrCount++;
		if(10<s_cIgbtErrCount)
		{
		s_cIgbtErrCount = 10;
		g_cErrStatus = E_NTC_MOS;//"E2"
		vSetIHPower(POW_0W);
		}
	}
	else
		s_cIgbtErrCount = 0;
		
	if(B_EPanOpen)
	{
		s_cPanErrCount++;
		if(10<s_cPanErrCount)
		{
			s_cPanErrCount = 10;
			g_cErrStatus = E_NTC_TOP_OPEN;//"E5"
			vSetIHPower(POW_0W);
		}
	}
	else if(B_EPanClose)
	{
		s_cPanErrCount++;
		if(10<s_cPanErrCount)
		{
			s_cPanErrCount = 10;
			g_cErrStatus = E_NTC_TOP_SHORT;//"E6"
			vSetIHPower(POW_0W);
		}
	}
	else
		s_cPanErrCount = 0;
}

//call this function in loop
void RecvDataAnalysis(void)
{	
	//---------------------------------------
	if (GetDecodeCode())
	{
		uchar sum = 0;
		for (uint i = 0; i < 4; i++)
		{
			sum += g_cReceiveData[i];
		}

		if (sum != 0xFF) return;//check sum error

		switch (g_cReceiveData[3])
		{
		case 0xA0:
			Status1_Work.all = g_cReceiveData[2];		//工作状态1
			Status2_Work.all = g_cReceiveData[1];		//工作状态2	

			/*if(B_Pan_Error){
				g_cErrStatus = E_NO_CIRCLE;//"E7"
				vSetIHPower(POW_0W);
			}*/
			//-----------------------------------------------------------------
			//B_Pan_Error = BitIsTrue(g_cReceiveData[2], 0) ? 1 : 0;//未接线盘
			//B_Pan_No = Status1_Work.one.b1;// BitIsTrue(Status1_Work, 1) ? 1 : 0;//无锅
			//B_Pan_Have = Status1_Work.one.b2; //BitIsTrue(Status1_Work, 2) ? 1 : 0;//有锅
			//B_Pan_Steel = BitIsTrue(Status1_Work, 3) ? 1 : 0;//钢锅
			//B_Pan_Iron = BitIsTrue(Status1_Work, 4) ? 1 : 0;//铁锅
			//B_Zero_No = BitIsTrue(Status1_Work, 5) ? 1 : 0;//无过零
			//B_Zero_60Hz = BitIsTrue(Status1_Work, 6) ? 1 : 0;//过零频率 = 60Hz
			//B_Protect_BackPress = BitIsTrue(Status1_Work, 7) ? 1 : 0;//过压保护
			if(B_Pan_No && !B_Pan_Have)//提壶
			{
				if(!g_bKettleLift)//有壶状态
					bQueueEn(LiftKettle);
				g_bKettleLift = 1;
			}
			else if(!B_Pan_No && B_Pan_Have)
			{
				if(g_bKettleLift)//有壶状态
					bQueueEn(PBackKettle);
				g_bKettleLift = 0;
			}

			//-----------------------------------------------------------------
			//B_Protect_Stop = BitIsTrue(Status2_Work, 0) ? 1 : 0;//保护停止加热	
			//B_PPGStop_CheckPan = BitIsTrue(Status2_Work, 1) ? 1 : 0;//加热暂停但检锅
			//B_PPGStop_NoCheck = BitIsTrue(Status2_Work, 2) ? 1 : 0;//加热暂停不检锅
			//Status_PPGDP = BitIsTrue(Status2_Work, 3) ? 1 : 0;//抖频开启
			//Status_LxLow = BitIsTrue(Status2_Work, 4) ? 1 : 0;//连续低功率开启
			break;
		case 0xA1:
			Flag1_Error.all = g_cReceiveData[2];		//硬件故障标志1
			Flag2_Error.all = g_cReceiveData[1];		//硬件故障标志2
			vErrorProc();

			//-----------------------------------------------------------------	
			//B_EVolLow = BitIsTrue(Flag1_Error, 0) ? 1 : 0;//市电电压过低	
			//B_EVolHigh = BitIsTrue(Flag1_Error, 1) ? 1 : 0;//市电电压过高
			//B_EIgbtOpen = BitIsTrue(Flag1_Error, 2) ? 1 : 0;//IGBT-NTC开路
			//B_EIgbtClose = BitIsTrue(Flag1_Error, 3) ? 1 : 0;////IGBT-NTC短路
			//B_EIgbtOver = BitIsTrue(Flag1_Error, 4) ? 1 : 0;////IGBT-NTC超温
			//B_EPanOpen = BitIsTrue(Flag1_Error, 5) ? 1 : 0;////锅底NTC开路
			//B_EPanClose = BitIsTrue(Flag1_Error, 6) ? 1 : 0;////锅底NTC短路
			//B_EPanOver = BitIsTrue(Flag1_Error, 7) ? 1 : 0;////锅底NTC超温

			//-----------------------------------------------------------------	
			//B_EPanFail = BitIsTrue(Flag2_Error, 0) ? 1 : 0;//锅底NTC失效
			//B_ETopOpen = BitIsTrue(Flag2_Error, 1) ? 1 : 0;//顶部NTC开路
			//B_ETopClose = BitIsTrue(Flag2_Error, 2) ? 1 : 0;//顶部NTC短路
			//B_ETopFail = BitIsTrue(Flag2_Error, 3) ? 1 : 0;//顶部NTC失效
			//B_Comm_Error = BitIsTrue(Flag2_Error, 7) ? 1 : 0;//通信故障
			break;
		case 0xA2:
			Status_PPGStop = g_cReceiveData[2];		//保护状态
			//AD_Volatage = g_cReceiveData[1];		//市电电压AD（高4位）	
			//AD_Current//工作电流AD（高4位）

			//-----------------------------------------------------------------	
			//B_L1_OVER_VOL = BitIsTrue(Status_PPGStop, 0) ? 1 : 0;//1级过压次数过多
			//B_VOL_SURGE = BitIsTrue(Status_PPGStop, 1) ? 1 : 0;//电压浪涌
			//B_CUR_SURGE = BitIsTrue(Status_PPGStop, 2) ? 1 : 0;//电流浪涌
			//B_L2_OVER_VOL = BitIsTrue(Status_PPGStop, 3) ? 1 : 0;//2级过压
			//B_CONDUCTION_STEP_H = BitIsTrue(Status_PPGStop, 4) ? 1 : 0;//导通台阶过高
			//B_PPG_WTDOG_OVER = BitIsTrue(Status_PPGStop, 5) ? 1 : 0;//PPG看门狗溢出
			break;

		//case 0xA3:
		//	AD_Volatage = g_cReceiveData[2];		//市电电压AD（低8位）
		//	AD_Current = g_cReceiveData[1];		//工作电流AD（低8位）	
		//	break;
		case 0xA4:
			AD_TmprPan = g_cReceiveData[2];		//锅底温度AD
			AD_TmprIgbt = g_cReceiveData[1];	//IGBT温度AD
			vTemprProc();
			break;
		//case 0xA5:
		//	AD_TmprTop = g_cReceiveData[2];		//顶部温度AD 
		//	AD_VRef = g_cReceiveData[1];		//内部基准电压（1.2V）AD
		//	break;
		//case 0xA6:
		//	Vol_Flo = g_cReceiveData[2];		//市电电压 - 80V
		//	Count_Syn = g_cReceiveData[1];		//检锅振荡个数	
		//	break;
		//case 0xA7:
		//	Time_Syn = g_cReceiveData[2] << 8 + g_cReceiveData[1];
		//	//Time_Syn >> 8 = g_cReceiveData[2];		//检锅振荡脉宽（高8位） 
		//	//Time_Syn = g_cReceiveData[1];		//检锅振荡脉宽（低8位）		
		//	break;
		//case 0xA8:
		//	PowerReal = g_cReceiveData[2] << 8 + g_cReceiveData[1];
		//	//PowerReal >> 8 = g_cReceiveData[2];	//实际输出功率（高8位） 
		//	//PowerReal = g_cReceiveData[1];		//实际输出功率（低8位）		
		//	break;
		//case 0xA9:
		//	Buf_PPGTMR = g_cReceiveData[2] << 8 + g_cReceiveData[1];
		//	//Buf_PPGTMR >> 8 = g_cReceiveData[2];;	//当前PPG导通值（高8位）
		//	//Buf_PPGTMR = g_cReceiveData[1];		//当前PPG导通值（低8位）	
		//	break;
		//case 0xAA:
		//	Count_BackPress = g_cReceiveData[2] << 8 + g_cReceiveData[1];
		//	//Count_BackPress >> 8 = g_cReceiveData[2];;	//1级过压保护计数（高8位）
		//	//Count_BackPress = g_cReceiveData[1];		//1级过压保护计数（低8位）
		//	break;
		//case 0xAB:
		//	Count_IgbtProtect = g_cReceiveData[2] << 8 + g_cReceiveData[1];
		//	//Count_IgbtProtect >> 8 = g_cReceiveData[2];	//IGBT硬件保护计数（高8位）
		//	//Count_IgbtProtect = g_cReceiveData[1];		//IGBT硬件保护计数（低8位）
		//	break;
		//case 0xAC:
		//	Elec_Flo = g_cReceiveData[2] << 8 + g_cReceiveData[1];
		//	//Elec.flo.integer = g_cReceiveData[2];;	//电量高位
		//	//Elec.flo.radix = g_cReceiveData[1];	//电量低位	
		//	break;
		default:
			break;
		}
	}
}
