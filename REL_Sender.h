#ifndef _REL_SENDER_
#define _REL_SENDER_

// 数据发送使能,1:使能，其余不使能
#define REL_SENDER_ENABLE	0

/**************************************************************************
**************************以下内容请勿修改*********************************
**************************************************************************/
void REL_SenderLoop();

//////////////////////////////////////////////////////
// 函数实现定义
#include "iic_soft.h"
#define REL_PreSendCallBack()								I2C_WriteStart()
#define REL_SendByte(ch)									I2C_Write1bYTE(ch)
#define REL_EndSendCallBack()								I2C_WriteStop()

// 基本类型定义
#if 1
typedef   signed            char int8_t;
typedef   signed short      int int16_t;
typedef   signed            long  int32_t;

typedef unsigned            char uint8_t;
typedef unsigned short      int  uint16_t;
typedef unsigned  			long uint32_t;

typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;
#endif

//////////////////////////////////////////////////////

// 包头字节
#define REL_HEADER_BYTE	0xA0

// 包头结构
//#pragma pack(1)
//#pragma anon_unions
typedef struct _PackHeader
{
	u8 length;		//长度,sizeof(PackHeader) + sizeof(PackStrcut) * n + 1
	union
	{
		u16 mask;
		struct
		{
			u8 packIdx : 4;		// 分包索引,从0开始
			u8 packIdxMax : 4;		// 最大包索引
			u8 typeId;				// 类型ID,决定使用的结构体
		}m;
	};
	u8 extData[2];
}PackHeader;
//#pragma pack()

// 最终发送函数					
#define REL_SendStart(packHeader)	\
	{u16 _checkSum = 0;\
	u8 _i;\
	u8 *_bufPoint;\
	REL_PreSendCallBack();\
	REL_SendByte(REL_HEADER_BYTE);\
	packHeader.length+=2;\
	_bufPoint = (u8*)&packHeader;\
	for(_i = 0;_i < sizeof(packHeader);_i++,_bufPoint++) {\
		REL_SendByte(*_bufPoint);\
		_checkSum += *_bufPoint;\
	}

#define REL_SendNext(packStrcut) \
	_bufPoint = (u8*)&packStrcut;\
	for(_i = 0;_i < sizeof(packStrcut);_i++,_bufPoint++) {\
		REL_SendByte(*_bufPoint);\
		_checkSum += *_bufPoint;\
	}

#define REL_SendEnd()	\
	REL_SendByte(0x55);\
	REL_SendByte(_checkSum >> 8);\
	REL_SendByte(_checkSum & 0xff);\
	REL_EndSendCallBack();}

#endif /*_REL_SENDER_USER_*/

