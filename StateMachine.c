
#include "main.h"

void vPowerOffStatus(void)
{
	//DispData[0] = 0x30;//0x20:startLED;0x10:settingLED
		
	g_cLedAllOnCount--;//上电倒计时器
	if (0 == g_cLedAllOnCount) //上电倒计时清零
	{
		g_cLedAllOnCount = 0;//倒计时器清零
		g_eSysStatus = WAITING;
		//DispData[0] = 0;
		
		if(0<g_xSysSetting.KWTime)
			g_xTftContent.KWOnOff=ICON_DISP;//显
		else
			g_xTftContent.KWOnOff=ICON_HIDE;//隐
		
		//切换页面
		g_xTftAction = SwchPage;
	}
}

void vSleepStatus(void)
{
	switch (g_xSysAction)
	{
	case Tim05STrig://更新sleep显示参数，且更新刷新屏动作	
		if(g_xTftContent.Sleep < 3)
			g_xTftContent.Sleep++;
		else
			g_xTftContent.Sleep=0;
		
		g_xTftAction = UpdSleepIcon;
		break;
	case StartKey:
	case SetKey:
	case IncSKey:
	case DecSKey://任何按键动作，退出sleep状态，回到waitting
		g_xTftAction = SwchPage;
		g_cSwch2SleepTime = 0;
		g_eSysStatus = WAITING;
		g_xTftContent.sEnFlash = 0;
		
		/*if(0<g_xSysSetting.KWTime)//保温图标
			g_xTftContent.KWOnOff=0;//显
		else
			g_xTftContent.KWOnOff=1;//隐
			*/
		break;
	default:
		break;
	}
}

void vTemprIncrease(void)
{
	//直接用显示的值进行加减(104-203;40-95)
	g_xTftContent.setT++;
	if(g_xSysSetting.TemprUnit)//华氏度
	{
		if(MAX_SET_TEMPR_F < g_xTftContent.setT)
			g_xTftContent.setT = MAX_SET_TEMPR_F;
		//仅华氏度需要转换
		g_xSysSetting.SetTempr = FConvert2C(g_xTftContent.setT);
	}
	else
	{
		if(MAX_SET_TEMPR_05C < g_xTftContent.setT)//95*2
			g_xTftContent.setT = MAX_SET_TEMPR_05C;
		//赋值
		g_xSysSetting.SetTempr = g_xTftContent.setT;
	}
	g_xIhProcParam.GapTempr = (g_xIhProcParam.TargTempr > g_xIhProcParam.CurTempr)?(g_xIhProcParam.TargTempr - g_xIhProcParam.CurTempr) :0;
	
	g_xTftContent.Ball = 8;	
	g_xTftAction = UpdSetTempr;
}

void vTemprDecrease(void)
{
	if(g_xSysSetting.TemprUnit)//华氏度
	{
		if(MIN_SET_TEMPR_F < g_xTftContent.setT)
			g_xTftContent.setT--;
		else
			g_xTftContent.setT = MIN_SET_TEMPR_F;
		//仅华氏度需要转换
		g_xSysSetting.SetTempr = FConvert2C(g_xTftContent.setT);
	}
	else
	{
		if(MIN_SET_TEMPR_05C < g_xTftContent.setT)//
			g_xTftContent.setT--;
		else
			g_xTftContent.setT = MIN_SET_TEMPR_05C;
		//赋值
		g_xSysSetting.SetTempr = g_xTftContent.setT;
	}
	g_xIhProcParam.GapTempr = (g_xIhProcParam.TargTempr > g_xIhProcParam.CurTempr)?(g_xIhProcParam.TargTempr - g_xIhProcParam.CurTempr) :0;
	g_xTftContent.Ball = 8;
	g_xTftAction = UpdSetTempr;
}

void vCurTemprUpdate(void)
{
	if(g_xSysSetting.TemprUnit)//华氏度
	{		
		g_xTftContent.curT = CConvert2F(g_xIhProcParam.CurTempr);
	}
	else
	{
		g_xTftContent.curT = g_xIhProcParam.CurTempr;
	}
	
	//还剩多少温度差
	uint tempr = (g_xIhProcParam.TargTempr > g_xIhProcParam.CurTempr)?(g_xIhProcParam.TargTempr - g_xIhProcParam.CurTempr) :0;
	//更新Ball的值,也就是隐藏掉几个(如果>=8就全隐藏)
	tempr = tempr<<3;
	if(g_xIhProcParam.GapTempr)//防止除数为0
		g_xTftContent.Ball = tempr/g_xIhProcParam.GapTempr;
	else
		g_xTftContent.Ball = 8;
	
	g_xTftAction = UpdCurTempr;
}

//uint tempr;
void vWaittingStatus(void)
{
	switch(g_xSysAction)
	{
	case Tim3MinTrig://切换页面到2(3分钟无任何操作,进入睡眠状态)
		g_xTftAction = SwchPage;//切换到睡眠
		g_cSwch2SleepTime = 0;//3min计数器清零
		g_eSysStatus = SLEEPING;
		break;
	case StartKey://切换到Heating状态
		//DispData[0] = 0x20;
		g_eSysStatus = HEATING;//切换到加热状态
		g_cSwch2SleepTime=0;
		g_xTftContent.DogDorH = 1;//hide狗头
		g_xTftContent.sEnFlash = 1;
		bQueueEn(StartHeat);//启动加热的action
		break;
	case SetKey://切换到Setting状态
		//DispData[0] = 0x10;
		g_eSysStatus = SETTING1;
		//g_xTftContent.page = 1;//切换到page1
		g_xTftAction = SwchPage;//切换到设置页
		g_cSwch2SleepTime=0;
		break;
	case IncSKey:
	case IncLKey:
		vTemprIncrease();//设置温度+
		g_cSwch2SleepTime=0;
		break;
	case DecSKey:
	case DecLKey:
		vTemprDecrease();//设置温度-
		g_cSwch2SleepTime=0;
		break;
	case Tim5STrig:
		g_xIhProcParam.GapTempr = (g_xSysSetting.SetTempr > g_xIhProcParam.CurTempr)?(g_xSysSetting.SetTempr - g_xIhProcParam.CurTempr) :0;
		vCurTemprUpdate();
		break;
	default:
		break;
	}
}

void vUpdateTargetTempr(void)
{
	g_xIhProcParam.GapTempr = (g_xSysSetting.SetTempr > g_xIhProcParam.CurTempr)?(g_xSysSetting.SetTempr - g_xIhProcParam.CurTempr) :0;
	
	if(g_xSysSetting.PreBoiling)//设置预沸腾,目标温度为沸点
		g_xIhProcParam.TargTempr = g_xSysSetting.BoilingPoint;
	else//关闭预沸腾,目标温度为设置温度
	{
		if(g_xSysSetting.SetTempr<g_xSysSetting.BoilingPoint)
			g_xIhProcParam.TargTempr = g_xSysSetting.SetTempr;
		else//设置温度高于沸点,目标温度为沸点
			g_xIhProcParam.TargTempr = g_xSysSetting.BoilingPoint;
	}
}

void vHeatingStatus(void)
{
	switch (g_xSysAction)
	{
	case Tim10MinTrig://提壶计时到达10min
		/*g_xTftContent.page = 2;
		g_xTftAction = SwchPage;*/
		break;
	case StartKey://手动切换到Waitting状态
		//DispData[0] = 0x00;
		g_eSysStatus = WAITING;//切换到waitting状态
		//停止加热;切换图标
		g_xTftContent.DogDorH = 0;
		g_xTftAction = UpdDogIcon;
		/*** IH部分 ***/
		g_bKeepWarmFlag = 0;
		g_cKeepWarmTime = 0;
		vSetIHPower(POW_0W);
		break;
	case IncSKey://设置温度+
		vTemprIncrease();
		vUpdateTargetTempr();
		break;
	case DecSKey://设置温度-
		vTemprDecrease();
		vUpdateTargetTempr();
		break;
	case LiftKettle://提壶，停止加热并计时
		//g_bKettleLift = 1;//提壶flag
		break;
	case PBackKettle://放回壶，继续加热，取消计时
		/*break;*/
	case StartHeat://按键切过来启动加热
		g_bKeepWarmFlag = 0;
		vUpdateTargetTempr();
		break;
	case KeepWarm://保温加热
		break;
	case Tim05STrig://闪烁狗头图标
		//g_xTftContent.DogDorH = ~g_xTftContent.DogDorH;
		vCurTemprUpdate();
		vHeatingProc();
		break;
	default:
		break;
	}
}

void vSysParam2Tft(uchar item)
{
	if(item==1)
		g_xTftContent.sVal = g_xSysSetting.PreBoiling;
	else if(item==2)
		g_xTftContent.sVal = g_xSysSetting.TemprUnit;
	else if(item==3)
		g_xTftContent.sVal = g_xSysSetting.EnBeep;
	else if(item==4)
		g_xTftContent.sVal = g_xSysSetting.EnGaoyuan;
	else//item = 0
	{
		if(g_xSysSetting.KWTime==30)
			g_xTftContent.sVal=1;
		else if(g_xSysSetting.KWTime==60)
			g_xTftContent.sVal=2;
		else
			g_xTftContent.sVal=0;
	}
}

void vSetting1Status(void)
{
	switch (g_xSysAction)
	{	
	case SetKey://退出设置状态		
		g_eSysStatus = WAITING;		
		g_xTftAction = SwchPage;		
		if(0<g_xSysSetting.KWTime)
			g_xTftContent.KWOnOff=0;//显
		else
			g_xTftContent.KWOnOff=1;//隐
		break;
	case OkKey://确定设置项或者值，不再闪烁
		g_eSysStatus = SETTING2;
		//根据项载入设置值
		vSysParam2Tft(g_xTftContent.sItem);		
		g_xTftAction = SwchPage;
		break;
	case IncSKey:		
		if(g_xTftContent.sItem < 4)//项变量更新
			g_xTftContent.sItem += 1;
		else
			g_xTftContent.sItem = 0;
		//根据项载入设置值
		//vSysParam2Tft(g_xTftContent.sItem);		
		g_xTftAction = SelSetItem;//立即更新项显示
		break;
	case DecSKey:		
		if(0<g_xTftContent.sItem && g_xTftContent.sItem<5)//项变量更新
			g_xTftContent.sItem-= 1;
		else
			g_xTftContent.sItem = 4;
		//根据项载入设置值
		//vSysParam2Tft(g_xTftContent.sItem);
		g_xTftAction = SelSetItem;//立即更新项显示
		break;
	case Tim3MinTrig:
		break;
	default:
		break;
	}
}

void vSetting2Status(void)
{
	switch (g_xSysAction)
	{
	case SetKey://退出设置状态
		g_eSysStatus = WAITING;
		g_xTftAction = SwchPage;
		
		if(0<g_xSysSetting.KWTime)
			g_xTftContent.KWOnOff=0;//显
		else
			g_xTftContent.KWOnOff=1;//隐
		break;
	case Tim5STrig://5S无操作触发退回到项设置
	case OkKey://退出值设置,回到项选择
		g_eSysStatus = SETTING1;
		g_xTftAction = SwchPage;
		break;
	case IncSKey:
		if(g_xTftContent.sVal < 3)
			g_xTftContent.sVal += 1;//值变量更新
		
		uchar temp;
		if(g_xTftContent.sItem==0)
		{
			if(2<g_xTftContent.sVal)
				g_xTftContent.sVal = 0;
			
			//把UI设置值写入系统
			if(g_xTftContent.sVal==1)
				g_xSysSetting.KWTime = 30;
			else if(g_xTftContent.sVal==2)
				g_xSysSetting.KWTime = 60;
			else
				g_xSysSetting.KWTime = 0;
			
			//写EEPROM
			temp = g_xSysSetting.KWTime + 1;
		}
		else
		{
			if(1<g_xTftContent.sVal)
				g_xTftContent.sVal = 0;
							
			//把UI设置值写入系统
			if(g_xTftContent.sItem==1)
				g_xSysSetting.PreBoiling = g_xTftContent.sVal;
			else if(g_xTftContent.sItem==2)
				g_xSysSetting.TemprUnit = g_xTftContent.sVal;
			else if(g_xTftContent.sItem==3)
				g_xSysSetting.EnBeep = g_xTftContent.sVal;
			else if(g_xTftContent.sItem==4)
				g_xSysSetting.EnGaoyuan = g_xTftContent.sVal;
			else//sItem值不对
				return;
			//写EEPROM
			temp = (uchar)g_xTftContent.sVal + 1;		
		}
		Memory_Write((uchar)g_xTftContent.sItem*2+ADDR_KEEP_WARM_TIME, temp);
		temp = ~temp;
		Memory_Write((uchar)g_xTftContent.sItem*2+ADDR_KEEP_WARM_TIME1, temp);
		g_xTftAction = UpdSetVal;//立即更新值显示	
		break;
	case DecSKey:		
		if(0<g_xTftContent.sVal)
			g_xTftContent.sVal -= 1;//值变量更新
		else
			g_xTftContent.sVal = 2;
			
		uchar temp;
		if(g_xTftContent.sItem==0)
		{
			if(2 <= g_xTftContent.sVal)
				g_xTftContent.sVal = 2;
			
			//把UI设置值写入系统
			if(g_xTftContent.sVal==1)
				g_xSysSetting.KWTime = 30;
			else if(g_xTftContent.sVal==2)
				g_xSysSetting.KWTime = 60;
			else
				g_xSysSetting.KWTime = 0;
			
			temp= g_xSysSetting.KWTime+1;
		}
		else
		{
			if(1<=g_xTftContent.sVal)
				g_xTftContent.sVal = 1;
			
			//把UI设置值写入系统
			if(g_xTftContent.sItem==1)
				g_xSysSetting.PreBoiling = g_xTftContent.sVal;
			else if(g_xTftContent.sItem==2)
				g_xSysSetting.TemprUnit = g_xTftContent.sVal;
			else if(g_xTftContent.sItem==3)
				g_xSysSetting.EnBeep = g_xTftContent.sVal;
			else if(g_xTftContent.sItem==4)
				g_xSysSetting.EnGaoyuan = g_xTftContent.sVal;
			else return;
			
			temp= (uchar)g_xTftContent.sVal + 1;
		}
		//写EEPROM
		Memory_Write((uchar)g_xTftContent.sItem*2+ADDR_KEEP_WARM_TIME, temp);
		temp = ~temp;
		Memory_Write((uchar)g_xTftContent.sItem*2+ADDR_KEEP_WARM_TIME1, temp);
		g_xTftAction = UpdSetVal;//立即更新值显示	
		break;
	default:
		break;
	}
}

void vSystemStatus(void)
{	
	g_xSysAction = ActIdle;
	bQueueDe(&g_xSysAction);
	
	switch(g_eSysStatus)
	{
	case POWER_OFF://断电
		vPowerOffStatus();
		break;
	case SLEEPING://睡眠
		vSleepStatus();
		break;
	case WAITING://等待
		vWaittingStatus();
		break;
	case HEATING://加热
		vHeatingStatus();
		break;
	case SETTING1://设置项
		vSetting1Status();
		break;
	case SETTING2://设置值
		vSetting2Status();
		break;
	default:
		break;
	}
}

//加入队列
bit bQueueEn(ACTION_Typedef val)
{
    if(((g_xActQueue.rear + 1) % QUEUE_MAX_LEN) == g_xActQueue.front)
    {
        return 0;
    }
    g_xActQueue.buf[g_xActQueue.rear] = val;
    g_xActQueue.rear = (g_xActQueue.rear + 1) % QUEUE_MAX_LEN;
    return 1;
}

//从队列取出
bit bQueueDe(ACTION_Typedef *pVal)
{
    if(g_xActQueue.front == g_xActQueue.rear)
    {
        return 0;
    }
    *pVal = g_xActQueue.buf[g_xActQueue.front];
    g_xActQueue.front = (g_xActQueue.front + 1) % QUEUE_MAX_LEN;
    return 1;
}