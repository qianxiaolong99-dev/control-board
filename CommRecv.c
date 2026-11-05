
#include "main.h"

//帧数据head有10ms,必须在head内取走;帧75ms内处理完
bit GetDecodeCode(void)
{
	if (1 == g_bDecodeSuccess)
	{		
		g_bDecodeSuccess = 0;
		
		g_cReceiveData[3] = g_cDecodeData[3];
		g_cReceiveData[2] = g_cDecodeData[2];
		g_cReceiveData[1] = g_cDecodeData[1];
		g_cReceiveData[0] = g_cDecodeData[0];
		//g_ulDecodeData.all = 0;
		memset(g_cDecodeData, 0, 4);
		return 1;
	}
	else
	{
		return 0;
	}
}

void vClearDecodingParam(void)
{
	g_cDecodePinHcnt = 0;	//高电平计数
	g_cDecodePinLcnt = 0;
	g_bPrevPinValue = 0;			//上一次引脚电平
	g_bCurrPinValue = 0;			//当前引脚电平
	g_bBitVal0Decoding = 0;	//判断bit=0的启动flag
	g_bBitVal1Decoding = 0;	//判断bit=1的启动flag

	g_bDecodBody = 0;			//解码中的flag
	g_cDecodedPosition = 0;			//解码当前byte中的bit位置
	g_bDecodeSuccess = 0;		//清除解码成功flag
	//g_ulDecodeData.all = 0;
	memset(g_cDecodeData, 0, 4);
}

void vInitDecoding(void)
{
	//先初始化varients	
	vClearDecodingParam();
	
	g_bIsAllowedCopyData=1;//允许给中断发送的数据赋值
	g_bIsAllowedGetData=0;//不允许中断获取数据
	
	//125us中断初始化	
	//---------------------------------------
	//TMR1L = 0x30;			//赋初值
	//TMR1H = 0xF8;
	//TMR1IF = 0;				//清中断标志位
	//TMR1IE = 1;				//允许Timer1中断	
	//T1CON = 0x01;			//开启Timer1，使用内部时钟源Fosc，预分频比为1:1
	//---------------------------------------	

	//初始化用于decode的引脚	
	SET_DECODE_PIN_AS_INPUT;	//设置port B的bit0为输入引脚
	SET_DECODE_PIN_PULLUP;
}


/***********************************************
函数功能：decode in 125us timer interrupt
************************************************/
volatile bit PinHigh2Low;
volatile bit PinLow2High;
void vDecodeInTimer()
{
	//获取引脚信息
	if(DECODE_PIN)//high
	{
		g_cDecodePinHcnt++;
		g_bCurrPinValue = 1;			
	}
	else//low
	{
		g_cDecodePinLcnt++;
		g_bCurrPinValue = 0;
	}
	
	PinHigh2Low = 0;
	PinLow2High = 0;
	if(g_bCurrPinValue == 0 && g_bPrevPinValue == 1)
	{
		PinHigh2Low = 1;
		g_cDecodePinLcnt = 1;
	}
	else if(g_bCurrPinValue == 1 && g_bPrevPinValue == 0)
	{
		PinLow2High = 1;
		g_cDecodePinHcnt = 1;
	}

	//-------------------------------------------------------------------------	
	if(g_bDecodBody == 1)//decoding one bit
	{
		/*if(PinLow2High == 1)//decode low voltage
		{
			if((BIT0_LOWCOUNT_MIN<g_cDecodePinLcnt) && (g_cDecodePinLcnt<BIT0_LOWCOUNT_MAX))
			{
				g_bBitVal0Decoding = 1;
			}
			else if((BIT1_LOWCOUNT_MIN<g_cDecodePinLcnt) && (g_cDecodePinLcnt<BIT1_LOWCOUNT_MAX))
			{
				g_bBitVal1Decoding = 1;
			}
		}
		else */
		if(PinHigh2Low == 1)//decode high voltage
		{
			if((BIT0_HIGHCOUNT_MIN < g_cDecodePinHcnt) && (g_cDecodePinHcnt < BIT0_HIGHCOUNT_MAX))
			{
				//当前bit值是0
				//g_ulDecodeData.all |= 0<< (31-g_cDecodedPosition);
				if(g_cDecodedPosition > 23)
				{
					g_cDecodeData[0] |= 0<<(31-g_cDecodedPosition);
				}
				else if(g_cDecodedPosition>15)
				{
					g_cDecodeData[1] |= 0<<(23-g_cDecodedPosition);
				}
				else if(g_cDecodedPosition>7)
				{
					g_cDecodeData[2] |= 0<<(15-g_cDecodedPosition);
				}
				else
				{
					g_cDecodeData[3] |= 0<<(7-g_cDecodedPosition);
				}
				g_cDecodedPosition++;
			}
			else if((BIT1_HIGHCOUNT_MIN < g_cDecodePinHcnt) && (g_cDecodePinHcnt < BIT1_HIGHCOUNT_MAX))
			{
				//当前bit值是1
				//g_ulDecodeData.all |= 1<< (31-g_cDecodedPosition);
				if(g_cDecodedPosition > 23)
				{
					g_cDecodeData[0] |= 1<<(31-g_cDecodedPosition);
				}
				else if(g_cDecodedPosition>15)
				{
					g_cDecodeData[1] |= 1<<(23-g_cDecodedPosition);
				}
				else if(g_cDecodedPosition>7)
				{
					g_cDecodeData[2] |= 1<<(15-g_cDecodedPosition);
				}
				else
				{
					g_cDecodeData[3] |= 1<<(7-g_cDecodedPosition);
				}
				g_cDecodedPosition++;
			}
			else//码值错误,全部清除,wait decode head
			{
				vClearDecodingParam();
			}

			//decode 4 byte done
			if (31 < g_cDecodedPosition)
			{
				g_cDecodedPosition = 0;

				g_bDecodeSuccess = 1;
				g_cDecodePinHcnt = 0;	//高电平计数
				g_cDecodePinLcnt = 0;
				g_bBitVal0Decoding = 0;	//判断bit=0的启动flag
				g_bBitVal1Decoding = 0;	//判断bit=1的启动flag
				g_bDecodBody = 0;			//解码中的flag
			}	
		}
	}	
	else
	{
		if(PinHigh2Low == 1)
		{
			//头码10ms,启动decode
			if(HEAD_HIGHCOUNT_MIN <= g_cDecodePinHcnt)
			{
				g_bDecodeSuccess = 0;
				g_bDecodBody = 1;
				g_cDecodePinHcnt = 0;
				g_cDecodePinLcnt = 1;//当前已经计数1次
				g_cDecodedPosition = 0;
				memset(g_cDecodeData, 0, 4);
			}
			else
			{//it isn't head
				g_cDecodePinHcnt = 0;
			}
		}
	}
	
	g_bPrevPinValue = g_bCurrPinValue;
}




