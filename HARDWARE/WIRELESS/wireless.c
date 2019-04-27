#include "wireless.h"
#include "nrf24l01.h"

static u8 ReceiveCom = BLANCE_COM;

/******************************************************************
*函数名称:	ReadWirelessCom
*函数功能:	读取无线的命令,保存在ReceiveCom变量里
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void ReadWirelessCom(void)
{
	if (RX_SUCCEED == Nrf24l01RXPacket(&ReceiveCom))
	{
		//不做任何操作
	}
	else
	{
		ReceiveCom = 0;
	}
}

/******************************************************************
*函数名称:	GetWirelessCom
*函数功能:	得到无线的命令,即得到ReceiveCom变量的值
*函数参数:	无
*返 回 值:	ReceiveCom: 无线的命令值(见头文件)
*******************************************************************/
u8 GetWirelessCom(void)
{
	return ReceiveCom;
}
