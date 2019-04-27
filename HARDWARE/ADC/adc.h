#ifndef ADC_H_
#define ADC_H_

#include "sys.h"

#define BATTERY_ADC_CHANNEL 4	//��ص�ѹ�ɼ���ADCͨ��

/******************************************************************
*��������:	Adc1Init
*��������:	��ʼ��ADC,���ڲ��Ե�ص�ѹ
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void Adc1Init(void);

/******************************************************************
*��������:	GetAdc1Value
*��������:	�õ�ADC1ָ��ͨ���Ĳ���ֵ
*��������:	channel:	����ͨ��
*�� �� ֵ:	����ֵ
*******************************************************************/
int GetAdc1Value(u8 channel);

/******************************************************************
*��������:	GetBatteryVoltage
*��������:	�õ���صĵ�ѹֵ
*��������:	��
*�� �� ֵ:	voltage:	��ص�ѹ(��λV)
*******************************************************************/
float GetBatteryVoltage(void);

#endif