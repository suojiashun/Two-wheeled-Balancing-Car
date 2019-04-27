#include "encoder.h"

/******************************************************************
*��������:	Tim2EncoderInit
*��������:	��ʼ����ʱ��2,���ڱ�����
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void Tim2EncoderInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//ʹ�ܶ�ʱ��4��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;			//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//�����趨������ʼ��GPIOB

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;						// Ԥ��Ƶ�� 
	TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD;			//�趨�������Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;			//ѡ��ʱ�ӷ�Ƶ������Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//TIM���ϼ���  
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);	//ʹ�ñ�����ģʽ3
	
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM2, &TIM_ICInitStructure);
	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);							//���TIM�ĸ��±�־λ
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM2, 0);
	
	TIM_Cmd(TIM2, ENABLE);
}

/******************************************************************
*��������:	Tim4EncoderInit
*��������:	��ʼ����ʱ��4,���ڱ�����
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void Tim4EncoderInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);			//ʹ�ܶ�ʱ��4��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;			//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);							//�����趨������ʼ��GPIOB

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;						// Ԥ��Ƶ�� 
	TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD;			//�趨�������Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;			//ѡ��ʱ�ӷ�Ƶ������Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//TIM���ϼ���  
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);	//ʹ�ñ�����ģʽ3
	
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);							//���TIM�ĸ��±�־λ
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM4, 0);
	
	TIM_Cmd(TIM4, ENABLE);
}

/******************************************************************
*��������:	ReadEncoderValue
*��������:	��ȡ��������ֵ
*��������:	timeValue:		��ʱ����(2��4)
*�� �� ֵ:	encoderValue:	��ȡָ���Ķ�ʱ����CNTֵ
*******************************************************************/
u16 ReadEncoderValue(u8 timeValue)
{
	u16 encoderValue;
	
	switch (timeValue)
	{
	case 2:
		encoderValue = TIM2->CNT;
		TIM2->CNT = 0;
		break;
	case 4:
		encoderValue = TIM4->CNT;
		TIM4->CNT = 0;
		break;
	default:
		encoderValue = 0;
		break;
	}
	
	return encoderValue;
}
