#ifndef _LED_DEVICE_Demo_H_
#define _LED_DEVICE_Demo_H_

void Set_LED_Moudle();
void Led_Moudle_Device();
void Set_Addr_Value(unsigned char Addr, unsigned int Mask);

//用户可根据硬件情况修改SEG口电流及个数，修改SEG口和COM口对应点
/**************************************************
设置COM,SEG口
**************************************************/
#define		C_COMSEL_COM	0x01		//设置COMSEL，选择COM口个数，范围0x00~0x07，详情请查看芯片手册
#define     C_LED_SEGCUR    SEGCUR_2MA  //SEG口电流
#define		SEG_NUMBER		10           //SEG口个数(736:1-8  738:1-11)
#define		COM_NUMBER		1			//COM口个数(736:1-4  738:1-8)
#define		FRENQUENCY		Fosc_8192	//频率

volatile unsigned uchar DispData[COM_NUMBER];//显示数组，当使用的SEG口不大于8个时，可以只声明char型变量，并修改响应的函数声明参数类型。

//各个段对应的芯片驱动SEG口（根据SEG口个数设置前n个值）
#define		C_LED_SEGA		SEG8  		//A段对应MCU的SEG口
#define		C_LED_SEGB		SEG7  		//B段对应MCU的SEG口
#define		C_LED_SEGC		SEG3		//C段对应MCU的SEG口
#define		C_LED_SEGD		SEG5		//D段对应MCU的SEG口
#define		C_LED_SEGE		SEG1  		//E段对应MCU的SEG口
#define		C_LED_SEGF		SEG6
#define		C_LED_SEGG		SEG4  		//
#define		C_LED_SEGH		SEG9		//F段对应MCU的SEG口
#define		C_LED_SEGI		SEG10		//G段对应MCU的SEG口
//#define		C_LED_SEGJ		SEG7  		//DP段对应MCU的SEG口
  		//

#define		C_LED_SEGP		SEG2 		//
/**************************************************
设置COM口
**************************************************/
//各个位对应的芯片驱动COM口（根据COM口个数设置前n个值，不使用的写8）
#define		C_LED_COM0		COM0  		//COM0对应MCU的COM口
#define		C_LED_COM1		COM1  		//COM1对应MCU的COM口
#define		C_LED_COM2		COM2		//COM2对应MCU的COM口
#define		C_LED_COM3		COM3		//COM3对应MCU的COM口
#define		C_LED_COM4		COM4			//COM4对应MCU的COM口
#define		C_LED_COM5		COM5			//COM5对应MCU的COM口
#define		C_LED_COM6		COM6			//COM6对应MCU的COM口

/**************************************************
LED模块常量定义,请勿修改
**************************************************/
#define     SEG0    0
#define     SEG1    1
#define     SEG2    2
#define     SEG3    3
#define     SEG4    4
#define     SEG5    5					//MCU的SEG口地址
#define     SEG6    6
#define     SEG7    7
#define     SEG8    8
#define     SEG9    9
#define     SEG10   10

#define     COM0    0
#define     COM1    1
#define     COM2    2
#define     COM3    3					//MCU的COM口地址
#define     COM4    4
#define     COM5    5
#define     COM6    6
#define     COM7    7

#define     SEGCUR_2MA      0X18
#define     SEGCUR_4MA      0X28
#define     SEGCUR_6MA      0X38
#define     SEGCUR_8MA      0X48
#define     SEGCUR_10MA     0X58
#define     SEGCUR_12MA     0X68
#define     SEGCUR_14MA     0X78		//MCU的SEG口电流
#define     SEGCUR_16MA     0X88
#define     SEGCUR_18MA     0X98
#define     SEGCUR_20MA     0XA8
#define     SEGCUR_22MA     0XB8
#define     SEGCUR_24MA     0XC8
#define     SEGCUR_26MA     0XD8
#define     SEGCUR_28MA     0XE8
#define     SEGCUR_30MA     0XF8

#define		Fosc_64			0x00
#define		Fosc_128		0x01
#define		Fosc_256		0x02
#define		Fosc_512		0x03
#define		Fosc_1024		0x04
#define		Fosc_2048		0x05
#define		Fosc_4096		0x06
#define		Fosc_8192		0x07
#define		Fosc_16384		0x08
#define		Fosc_32768		0x09
#define		Fosc_65536		0x0a
#define		Fosc_131072		0x0b
/**************************************************
**************************************************/
#define     C_LED_COM       ((1<<C_LED_COM0)|(1<<C_LED_COM1)|(1<<C_LED_COM2)|(1<<C_LED_COM3))//\
                            //|(1<<C_LED_COM4)|(1<<C_LED_COM5)|(1<<C_LED_COM6))
#define     C_LED_SEG       ((1<<C_LED_SEGA)|(1<<C_LED_SEGB)|(1<<C_LED_SEGC)|(1<<C_LED_SEGD)|\
							(1<<C_LED_SEGE)|(1<<C_LED_SEGF)|(1<<C_LED_SEGG)|(1<<C_LED_SEGP)|(1<<C_LED_SEGH)|(1<<C_LED_SEGI))
#define     C_LED_SEG0      C_LED_SEG&0XFF                             
#define     C_LED_SEG1      (C_LED_SEG>>8)&0XFF 

#define		BSET(x,y)		x|=(1<<y)	//置1
#define		BCLR(x,y)		x&=~(1<<y)	//置0
/**************************************************
**************************************************/
#define		SMG_0			0x3F		// 0		
#define		SMG_1			0x06		// 1		
#define		SMG_2			0x5B		// 2		
#define		SMG_3			0x4F		// 3		
#define		SMG_4			0x66		// 4		
#define		SMG_5			0x6D		// 5		
#define		SMG_6			0x7D		// 6		
#define		SMG_7			0x07		// 7		
#define		SMG_8			0x7F		// 8		
#define		SMG_9			0x6F		// 9		
#define		SMG_A			0x77		// A		
#define		SMG_B			0x7C		// B		
#define		SMG_C			0x39		// C		
#define		SMG_D			0x5E		// D		
#define		SMG_E			0x79		// E		
#define		SMG_F			0x71		// F

#endif