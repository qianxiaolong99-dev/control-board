
//#include <cms.h>
#include "main.h"

//

void vTxDataReady(void)
{
	switch (g_cSendCommNo)
	{
	case 0://0xB0
	{
		//Work_Flag1 = B_Heat_En | ((uchar)B_Fan_En << 1) | ((uchar)B_HeatStop_CheckPan << 5)
		//	| ((uchar)B_HeatStop_NoCheck << 6) | ((uchar)B_PPGDP_Dis << 7);
		//Work_Flag2 = B_PanOver_Dis | ((uchar)B_PanFail_Dis << 1) | ((uchar)B_PPGDP_Can << 7);
		SendFifo[3] = 0xB0;				//命令字		
		SendFifo[2] = Work_Flag1.all;		//工作状态1				
		SendFifo[1] = Work_Flag2.all;		//工作状态2	
		SendFifo[0] = SendFifo[1] + SendFifo[2] + SendFifo[3];
		SendFifo[0] = ~SendFifo[0];	
	}
	break;
	case 1:
	{
		SendFifo[3] = 0xB1;				//命令字	
		/*if(PowerWork < 50)
		{
			SendFifo[2] = 80;		//加热功率值
		}
		else*/
		{
			SendFifo[2] = PowerWork;		//加热功率值
		}
					
		SendFifo[1] = 0;				//置0
		SendFifo[0] = SendFifo[1] + SendFifo[2] + SendFifo[3];
		SendFifo[0] = ~SendFifo[0];
	}
	break;
	case 2:
	{
		SendFifo[3] = 0xB2;				//命令字
		if(40 <= PowerWork && PowerWork < 50)
		{
			SendFifo[2] = 8;//Num_LxLowOn;		//开功率次数
			SendFifo[1] = 8;//Num_LxLowOff;		//关功率次数
		}
		else if(30 <= PowerWork && PowerWork < 40)
		{
			SendFifo[2] = 6;//Num_LxLowOn;		//开功率次数
			SendFifo[1] = 10;//Num_LxLowOff;		//关功率次数
		}
		else if(20 <= PowerWork && PowerWork < 30)
		{
			SendFifo[2] = 4;//Num_LxLowOn;		//开功率次数
			SendFifo[1] = 12;//Num_LxLowOff;		//关功率次数
		}
		else if(10 <= PowerWork && PowerWork < 20)
		{
			SendFifo[2] = 2;//Num_LxLowOn;		//开功率次数
			SendFifo[1] = 14;//Num_LxLowOff;		//关功率次数
		}
		else
		{
			SendFifo[2] = 16;//Num_LxLowOn;		//开功率次数
			SendFifo[1] = 0;//Num_LxLowOff;		//关功率次数
		}
		SendFifo[0] = SendFifo[1] + SendFifo[2] + SendFifo[3];
		SendFifo[0] = ~SendFifo[0];
	}
	break;
	//case 3:
	//{
	//	Sendbuffer_1[3] = 0xD0;				//命令字
	//	Sendbuffer_1[2] = SlopeCurr_Adjust;		//电流斜率调整值
	//	Sendbuffer_1[1] = ~SlopeCurr_Adjust;		//电流斜率调整值反码
	//}
	//break;
	default:
		break;
	}
}

void vClearBuzzerFlag()
{
	if(g_cSendCommNo == 0)//当前是否为0xB0命令
	{
		if(B_Buzz_S_Out)B_Buzz_S_Out = 0;
		if(B_Buzz_L_Out)B_Buzz_L_Out = 0;
	}
}

//*************************************************
void vIntimeCopyData(void)
{
	if (g_bIsAllowedCopyData)//是否允许赋值
	{
		g_bIsAllowedGetData = 0;//不允许得到数据
		g_bSendCmdHead = 1;
		g_bSendCmdBody = 0;

		g_lSendData = ((ulong)SendFifo[3]) << 24;
		g_lSendData += ((ulong)SendFifo[2]) << 16;
		g_lSendData += ((ulong)SendFifo[1]) << 8;
		g_lSendData += ((ulong)SendFifo[0]);

		g_bIsAllowedCopyData = 0;//不允许赋值
		g_bIsAllowedGetData = 1;//允许得到数据

		/*if (g_bCommError)
		{
			g_bIsAllowedGetData = 0;
		}
		else
		{
			g_bIsAllowedGetData = 1;//允许得到数据
		}*/
		
		//在此处判断是否发送了蜂鸣器短鸣,如果发送了就清零
		vClearBuzzerFlag();
		
		g_cSendCommNo++;
		if (g_cSendCommNo >= 3)
		{
			g_cSendCommNo = 0;
		}
	}
}

void vIntSendData(void)
{
	static uchar s_cSendCmdTimCount = 0;
	static uchar s_cSendCmdBitCount = 0;

	if (g_bIsAllowedGetData)//允许得到数据
	{
		if (g_bSendCmdHead)//发送命令头
		{
			ENCODE_PIN = 1;
			s_cSendCmdTimCount++;
			if (s_cSendCmdTimCount >= 80)//10ms
			{
				s_cSendCmdTimCount = 0;
				g_bSendCmdHead = 0;
				g_bSendCmdBody = 1;
			}
		}
		else if(g_bSendCmdBody)//发送命令body
		{
			s_cSendCmdTimCount++;
			if (g_lSendData & 0x80000000)//1
			{				
				if (s_cSendCmdTimCount == 1) { ENCODE_PIN = 0; }
				if (s_cSendCmdTimCount == 13) { ENCODE_PIN = 1; }
				if (s_cSendCmdTimCount >= 16)
				{
					s_cSendCmdTimCount = 0;
					g_lSendData = g_lSendData << 1;

					s_cSendCmdBitCount++;
					if (s_cSendCmdBitCount >= 32)//32bit发送完成
					{
						s_cSendCmdBitCount = 0;
						g_bSendCmdBody = 0;
						//ENCODE_PIN = 0;//拉低,等待结束计时
					}
				}
			}
			else//0
			{				
				if (s_cSendCmdTimCount == 1) { ENCODE_PIN = 0; }
				if (s_cSendCmdTimCount == 5) { ENCODE_PIN = 1; }
				if (s_cSendCmdTimCount >= 16)
				{
					s_cSendCmdTimCount = 0;
					g_lSendData = g_lSendData << 1;

					s_cSendCmdBitCount++;
					if (s_cSendCmdBitCount >= 32)
					{
						s_cSendCmdBitCount = 0;
						g_bSendCmdBody = 0;
						//ENCODE_PIN = 0;//拉低,等待结束计时
					}
				}
			}
		}
		else
		{
			ENCODE_PIN = 0;
			s_cSendCmdTimCount++;
			if (s_cSendCmdTimCount > 3)//0.5ms
			{
				s_cSendCmdTimCount = 0;
				g_bIsAllowedGetData = 0;//不允许得到数据
				g_bIsAllowedCopyData = 1;//允许赋值
				g_bSendCmdHead = 1;
				g_bSendCmdBody = 0;
			}
		}
	}
}











