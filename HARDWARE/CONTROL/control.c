#include <stdbool.h>
#include <stddef.h>
#include "control.h"
#include "encoder.h"
#include "motor.h"
#include "led.h"
#include "inv_mpu.h"
#include "key.h"
#include "wireless.h"
#include "delay.h"

static void GetEncoderNum(short *encoderLeftNum, short *encoderRightNum);
static short UprightPd(float pitch, short yAngularSpeed);
static short SpeedPi(short encoderLeftNum, short encoderRightNum, u8 mode);
static short TurnP(short zAngularSpeed, u8 mode);
static void LimitPwm(short *leftPwmValue, short *rightPwmValue);
static u8 CheckState(float pitch);
static void SetPwm(short leftPwmValue, short rightPwmValue);

volatile static Control ControlValue = { 0, 0, 0, 0, 0, 0, 0 };

/******************************************************************
*��������:	KeyStartAndStop
*��������:	�������ڿ����͹ر�С��
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void KeyStartAndStop(void)
{
	static bool flag = 1;
	
	if (KEY == 0)
	{
		delay_ms(10);
		
		if (KEY == 0)
		{
			if (flag)
			{
				TIM_Cmd(TIM3, ENABLE);				//ʹ��TIM3
			}
			else
			{
				TIM_Cmd(TIM3, DISABLE);				//ʧ��TIM3
				
				//�رյ��,���
				AIN1 = 0;
				AIN2 = 0;
				BIN1 = 0;
				BIN2 = 0;
				LED = 1;
				
				//�������
				ControlValue.pitch = 0;
				ControlValue.roll = 0;
				ControlValue.yaw = 0;
				ControlValue.yAngularSpeed = 0;
				ControlValue.zAngularSpeed = 0;
				ControlValue.encoderLeftNum = 0;
				ControlValue.encoderRightNum = 0;
				SpeedPi(0, 0, EMPTY_INTEGRAL);		//��ջ���
			}
		
			flag = !flag;
			
			while(KEY == 0);
		}
	}
}

/***********************************************************************
*��������:	ReadControlValue
*��������:	���ڶ�ȡ���Ʊ���
*��������:	readData:	Control���͵�ָ��
*�� �� ֵ:	��
************************************************************************/
void ReadControlValue(Control *readData)
{
	*readData = ControlValue;
}

/******************************************************************
*��������:	Tim3TimeIntInit
*��������:	��ʼ����ʱ��3,����5ms�ж�
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void Tim3TimeIntInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 4999;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;					//����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);					//����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);									//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, DISABLE);
}

/******************************************************************
*��������:	TIM3_IRQHandler
*��������:	��ʱ��3�ĸ����ж�,5ms
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void TIM3_IRQHandler(void)
{
	static u16 time = 0;				//�����Ƽ�������
	static short upPwmValue = 0;		//ֱ��PD���ڵ�PWM����ֵ
	static short velocityPwmValue = 0;	//�ٶ�PI���ڵ�PWM����ֵ
	static short turnPwmValue = 0;		//ת��PWM����ֵ
	static short leftPwmValue = 0;		//��ߵ����PWMֵ
	static short rightPwmValue = 0;		//�ұߵ����PWMֵ
	static u8 wirelessCom = BLANCE_COM;	//����ָ��
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)								//���ָ����TIM�жϷ������:TIM�ж�Դ
	{
		//������ָʾ����״̬
		time++;
		if (time > 99)
		{
			time = 0;
			LED = !LED;
		}

		//�õ�����
		GetEncoderNum((short *)&ControlValue.encoderLeftNum, 
			(short *)&ControlValue.encoderRightNum);								//�õ����ұ���������ֵ
		mpu_dmp_get_data((float *)&ControlValue.pitch, (float *)&ControlValue.roll, 
			(float *)&ControlValue.yaw, (short *)&ControlValue.yAngularSpeed, 
			(short *)&ControlValue.zAngularSpeed);									//����λ��
		wirelessCom = GetWirelessCom();												//�õ���������
		
		//ֱ��������
		upPwmValue = UprightPd(ControlValue.pitch, ControlValue.yAngularSpeed);		//ֱ��PD����

		//�ٶȻ�����
		if (GO_COM == wirelessCom)
		{
			velocityPwmValue = SpeedPi(ControlValue.encoderLeftNum, 
				ControlValue.encoderRightNum, GO);									//�ٶ�PI����,ǰ��
		}
		else if (RETREAT_COM == wirelessCom)
		{
			velocityPwmValue = SpeedPi(ControlValue.encoderLeftNum, 
				ControlValue.encoderRightNum, RETREAT);								//�ٶ�PI����,����
		}
		else
		{
			velocityPwmValue = SpeedPi(ControlValue.encoderLeftNum, 
				ControlValue.encoderRightNum, NORMAL_MODE);							//�ٶ�PI����,��������
		}
		
		//ת�򻷵���
		if (LEFT_COM == wirelessCom)
		{
			turnPwmValue = TurnP(ControlValue.zAngularSpeed, TURN_LEFT);			//ת��P����,��ת
		}
		else if (RIGHT_COM == wirelessCom)
		{
			turnPwmValue = TurnP(ControlValue.zAngularSpeed, TURN_RIGHT);			//ת��P����,��ת
		}
		else
		{
			turnPwmValue = TurnP(ControlValue.zAngularSpeed, NORMAL_MODE);			//ת��P����,����ģʽ
		}
		
		//�����ֵ
		leftPwmValue = upPwmValue + velocityPwmValue + turnPwmValue;				//��ߵ��PWM��ֵ
		rightPwmValue = upPwmValue + velocityPwmValue - turnPwmValue;				//�ұߵ��PWM��ֵ
		
		LimitPwm(&leftPwmValue, &rightPwmValue);									//�޷�
		if (CheckState(ControlValue.pitch) == 0)									//���״̬����
		{
			SetPwm(leftPwmValue, rightPwmValue);									//���õ����PWMֵ
		}
		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);									//���TIMx���жϴ�����λ:TIM�ж�Դ
	}	
}

/******************************************************************
*��������:	GetEncoderNum
*��������:	�õ����ұ���������ֵ
*��������:	encoderLeftNum:		��߱���������ָ��
*			encoderRightNum:	�ұ߱���������ָ��
*�� �� ֵ:	��
*******************************************************************/
static void GetEncoderNum(short *encoderLeftNum, short *encoderRightNum)
{
	u16 leftTemp;
	u16 rightTemp;	
	
	leftTemp = ReadEncoderValue(2);			//��ȡ���������ֵ
	if (leftTemp > 32767)					//��ת,���¼���
	{
		*encoderLeftNum = leftTemp - 65535;
	}
	else
	{
		*encoderLeftNum = leftTemp;
	}
	*encoderLeftNum = -*encoderLeftNum;		//���ұ������ӿ��෴,��Ҫ�����ȡ��
	
	rightTemp = ReadEncoderValue(4);		//��ȡ�ұ�������ֵ
	if (rightTemp > 32767)
	{
		*encoderRightNum = rightTemp - 65535;
	}
	else
	{
		*encoderRightNum = rightTemp;
	}
}

/******************************************************************
*��������:	UprightPd
*��������:	ֱ��PD����
*��������:	pitch:			������
*			yAngularSpeed:	Y����ٶ�
*�� �� ֵ:	upPwmValue:		PWM����ռ�ձ�ֵ
*******************************************************************/
static short UprightPd(float pitch, short yAngularSpeed)
{
	static Pid upPd = { 309, 0, 1.236, 0, 0, 0 };
	short upPwmValue;
	
	upPd.bias = pitch - 0;		//0��ƽ��
	
	upPwmValue = upPd.kp * upPd.bias + upPd.kd * yAngularSpeed;
	
	return upPwmValue;
}

/****************************************************************************************
*��������:	SpeedPi
*��������:	�ٶ�PI����,����ֹͣʱ,�� EMPTY_INTEGRAL ģʽ��ջ���
*��������:	encoderLeftNum:		�����������
*			encoderRightNum:	�ұ���������
*			mode:				ģʽ:	NORMAL_MODE:	��������
*										EMPTY_INTEGRAL:	��ջ���
*										GO:				ǰ��
*										RETREAT:		����
*�� �� ֵ:	velocityPwmValue:	PWM����ռ�ձ�ֵ
******************************************************************************************/
static short SpeedPi(short encoderLeftNum, short encoderRightNum, u8 mode)
{
	static Pid velocityPi = { 220, 2.2, 0, 0, 0, 0 };
	int velocityPwmValue;
	
	if (EMPTY_INTEGRAL == mode)
	{
		velocityPi.integral = 0;
		
		return 0;
	}
	
	velocityPi.bias = (encoderLeftNum + encoderRightNum) - 0;				//Ŀ��ֵΪ0
	velocityPi.bias = velocityPi.bias * 0.2 + velocityPi.lastBias * 0.8;	//һ���ͺ��˲�
	velocityPi.integral += velocityPi.bias;
	
	//ǰ������ģʽ
	if (GO == mode)
	{
		velocityPi.integral -= 50;
	}
	else if (RETREAT == mode)
	{
		velocityPi.integral += 40;
	}
	
	//�����޷�
	if (velocityPi.integral > MAX_INTEGRAL)
	{
		velocityPi.integral = MAX_INTEGRAL;
	}
	else if (velocityPi.integral < -MAX_INTEGRAL)
	{
		velocityPi.integral = -MAX_INTEGRAL;
	}
	
	//����velocityPwmValue��ֵ��ʱ����ܻ���ֳ���short�������ֵ�����,����int����
	velocityPwmValue = velocityPi.kp * velocityPi.bias + velocityPi.ki * velocityPi.integral;
	
	//PI�ٶȻ��޷�
	if (velocityPwmValue > MAX_PI)
	{
		velocityPwmValue = MAX_PI;
	}
	else if (velocityPwmValue < -MAX_PI)
	{
		velocityPwmValue = -MAX_PI;
	}
	
	velocityPi.lastBias = velocityPi.bias;
	
	return (short)velocityPwmValue;
}

/****************************************************************************************
*��������:	TurnP
*��������:	ת��P����
*��������:	zAngularSpeed:	Z����ٶ�
*			mode:			ģʽ:	NORMAL_MODE:	����ģʽ
*									TURN_LEFT:		��ת
*									TURN_RIGHT:		��ת
*�� �� ֵ:	turnPwmValue:	PWM����ռ�ձ�ֵ
******************************************************************************************/
static short TurnP(short zAngularSpeed, u8 mode)
{
	static Pid turnP = { 0.8, 0, 0, 0, 0, 0 };
	short turnPwmValue;
	
	if (TURN_LEFT == mode)
	{
		turnP.bias = zAngularSpeed - 5000;
	}
	else if (TURN_RIGHT == mode)
	{
		turnP.bias = zAngularSpeed + 5000;
	}
	else
	{
		turnP.bias = zAngularSpeed - 0;
	}
	
	turnPwmValue = turnP.kp * turnP.bias;	
	
	//ת���޷�
	if (turnPwmValue > MAX_TURN_PWM)
	{
		turnPwmValue = MAX_TURN_PWM;
	}
	else if (turnPwmValue < -MAX_TURN_PWM)
	{
		turnPwmValue = -MAX_TURN_PWM;
	}
	
	return turnPwmValue;
}

/******************************************************************
*��������:	LimitPwm
*��������:	����PWM���ķ�ֵ
*��������:	leftPwmValue:	��ߵ����PWMֵ��ָ��
*			rightPwmValue:	�ұߵ����PWMֵ��ָ��
*�� �� ֵ:	��
*******************************************************************/
static void LimitPwm(short *leftPwmValue, short *rightPwmValue)
{
	if (*leftPwmValue > PWM_MAX_VALUE)
	{
		*leftPwmValue = PWM_MAX_VALUE;
	}
	else if (*leftPwmValue < -PWM_MAX_VALUE)
	{
		*leftPwmValue = -PWM_MAX_VALUE;
	}
	
	if (*rightPwmValue > PWM_MAX_VALUE)
	{
		*rightPwmValue = PWM_MAX_VALUE;
	}
	else if (*rightPwmValue < -PWM_MAX_VALUE)
	{
		*rightPwmValue = -PWM_MAX_VALUE;
	}
}

/******************************************************************
*��������:	CheckState
*��������:	���С����״̬,��������Ǿ���ֵ����40��,ֹͣ���
*��������:	pitch:	������
*�� �� ֵ:	0:		״̬����
*			����:	״̬�쳣
*******************************************************************/
static u8 CheckState(float pitch)
{
	if ((pitch > 40) || (pitch < -40))	//�����Ǿ���ֵ����40��
	{
		//ֹͣ���
		AIN1 = 0;
		AIN2 = 0;
		BIN1 = 0;
		BIN2 = 0;
		
		SpeedPi(0, 0, EMPTY_INTEGRAL);	//��ջ���
		
		return 1;
	}
	
	return 0;
}

/******************************************************************
*��������:	SetPwm
*��������:	�������ҵ����PWM��
*��������:	leftPwmValue:	��ߵ����PWMֵ
*			rightPwmValue:	�ұߵ����PWMֵ
*�� �� ֵ:	��
*******************************************************************/
static void SetPwm(short leftPwmValue, short rightPwmValue)
{
	if (leftPwmValue < 0)
	{
		AIN1 = 1;
		AIN2 = 0;
		
		PWMA = -leftPwmValue;
	}
	else
	{
		AIN1 = 0;
		AIN2 = 1;
		
		PWMA = leftPwmValue;
	}
	
	if (rightPwmValue < 0)
	{
		BIN1 = 1;
		BIN2 = 0;
		
		PWMB = -rightPwmValue;
	}
	else
	{
		BIN1 = 0;
		BIN2 = 1;
		
		PWMB = rightPwmValue;
	}
}
