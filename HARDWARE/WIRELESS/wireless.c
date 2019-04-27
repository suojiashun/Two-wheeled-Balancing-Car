#include "wireless.h"
#include "nrf24l01.h"

static u8 ReceiveCom = BLANCE_COM;

/******************************************************************
*��������:	ReadWirelessCom
*��������:	��ȡ���ߵ�����,������ReceiveCom������
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void ReadWirelessCom(void)
{
	if (RX_SUCCEED == Nrf24l01RXPacket(&ReceiveCom))
	{
		//�����κβ���
	}
	else
	{
		ReceiveCom = 0;
	}
}

/******************************************************************
*��������:	GetWirelessCom
*��������:	�õ����ߵ�����,���õ�ReceiveCom������ֵ
*��������:	��
*�� �� ֵ:	ReceiveCom: ���ߵ�����ֵ(��ͷ�ļ�)
*******************************************************************/
u8 GetWirelessCom(void)
{
	return ReceiveCom;
}
