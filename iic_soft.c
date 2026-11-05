#include "iic_soft.h"


/******************************************************************
函数功能：IIC启动信号函数
备    注：起始信号-在SCL为高电平时，SDA线上发生从高到低的电平变化
*******************************************************************/
void I2C_WriteStart()
{
	I2C_SDA_IO = 0;
	I2C_SCL_IO = 0;
	I2C_WaitMoment();

	I2C_SDA = 1;				//先将SDA=1，以准备在SCL=1时，将SDA=0
	I2C_SCL = 1;				//时钟总线拉高
	I2C_WaitMoment();
	I2C_SDA = 0;				//SCL=1时，将SDA拉低即产生启动信号

	I2C_WaitMoment();
	I2C_SCL = 0;				//将SCL=0，完成启动信号操作
}

/******************************************************************
函数功能：IIC结束信号函数
备    注：结束信号-在SCL为高电平时，SDA线上发生从低到高的电平变化
*******************************************************************/
void I2C_WriteStop()
{
	I2C_SDA_IO = 0;
	I2C_SCL_IO = 0;
	I2C_WaitMoment();

	I2C_SDA = 0;				//先将SDA=0，以准备在SCL=1时，将SDA=1
	I2C_SCL = 1;				//时钟总线拉高
	I2C_WaitMoment();
	I2C_SDA = 1;				//SCL=1时，将SDA拉高即产生停止信号

	I2C_WaitMoment();
	I2C_SCL = 0;				//将SCL=0，完成启动信号操作
}

/***********************************************************
函数功能：模拟IIC主机发送8位数据
入口参数：data-待发送数据
函数返回：返回ack应答信号
备    注：IIC协议ack应答信号为低有效
***********************************************************/
unsigned char I2C_Write1bYTE(unsigned char data)
{
	unsigned char i = 8;
	I2C_SDA_IO = 0;
	I2C_SCL_IO = 0;
	I2C_WaitMoment();
	do
	{
		I2C_SCL = 0;
		if (data & 0x80)
			I2C_SDA = 1;
		else
			I2C_SDA = 0;

		I2C_WaitMoment();
		I2C_SCL = 1;			//时钟线做一个上升沿，将一位数据写入
		data <<= 1;
		I2C_WaitMoment();
	} while (--i);

	I2C_SCL = 0;
	I2C_SDA_IO = 1;				//置为输入口接收应答信号
	I2C_WaitMoment();

	I2C_SCL = 1;				//做个上升沿准备读应答信号
	I2C_WaitMoment();

	i = I2C_SDA;
	I2C_SCL = 0;
	I2C_SDA_IO = 0;
	return i;
}

/***********************************************************
函数功能：模拟IIC主机接收8位数据
入口参数：ack-应答信号
函数返回：接收到的数据
备    注：IIC协议ack应答信号为低有效
***********************************************************/
unsigned char I2C_Read1bYTE(unsigned char ack)
{
	unsigned char i, data;
	I2C_SDA_IO = 1;
	I2C_SCL_IO = 1;
	I2C_WaitMoment();

	while (!I2C_SCL);			//等待从机准备好数据释放时钟
	I2C_SCL_IO = 0;
	I2C_SCL = 0;
	I2C_WaitMoment();

	for (i = 0; i < 8; i++)
	{
		data <<= 1;
		if (I2C_SDA) data |= 0x1;

		I2C_SCL = 1;
		I2C_WaitMoment();

		I2C_SCL = 0;
		I2C_WaitMoment();
	}

	I2C_SDA_IO = 0;
	if (ack)I2C_SDA = 1;			//是否需要更多的数据
	else I2C_SDA = 0;
	I2C_WaitMoment();

	I2C_SCL = 1;
	I2C_WaitMoment();

	I2C_SCL = 0;
	return data;
}

//非精准延时
void I2C_WaitMoment()
{
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");

	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
}

