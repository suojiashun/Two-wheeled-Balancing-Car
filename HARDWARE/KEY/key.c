#include "key.h"

/******************************************************************
*��������:	KeyInit
*��������:	��ʼ������
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void KeyInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��PA�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//�����趨������ʼ��GPIOA 
}
