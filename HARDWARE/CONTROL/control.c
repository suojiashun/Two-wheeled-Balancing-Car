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
*函数名称:	KeyStartAndStop
*函数功能:	按键用于开启和关闭小车
*函数参数:	无
*返 回 值:	无
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
				TIM_Cmd(TIM3, ENABLE);				//使能TIM3
			}
			else
			{
				TIM_Cmd(TIM3, DISABLE);				//失能TIM3
				
				//关闭电机,灭灯
				AIN1 = 0;
				AIN2 = 0;
				BIN1 = 0;
				BIN2 = 0;
				LED = 1;
				
				//清空数据
				ControlValue.pitch = 0;
				ControlValue.roll = 0;
				ControlValue.yaw = 0;
				ControlValue.yAngularSpeed = 0;
				ControlValue.zAngularSpeed = 0;
				ControlValue.encoderLeftNum = 0;
				ControlValue.encoderRightNum = 0;
				SpeedPi(0, 0, EMPTY_INTEGRAL);		//清空积分
			}
		
			flag = !flag;
			
			while(KEY == 0);
		}
	}
}

/***********************************************************************
*函数名称:	ReadControlValue
*函数功能:	用于读取控制变量
*函数参数:	readData:	Control类型的指针
*返 回 值:	无
************************************************************************/
void ReadControlValue(Control *readData)
{
	*readData = ControlValue;
}

/******************************************************************
*函数名称:	Tim3TimeIntInit
*函数功能:	初始化定时器3,用于5ms中断
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void Tim3TimeIntInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 4999;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;					//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);					//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);									//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, DISABLE);
}

/******************************************************************
*函数名称:	TIM3_IRQHandler
*函数功能:	定时器3的更新中断,5ms
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void TIM3_IRQHandler(void)
{
	static u16 time = 0;				//呼吸灯计数变量
	static short upPwmValue = 0;		//直立PD调节的PWM波的值
	static short velocityPwmValue = 0;	//速度PI调节的PWM波的值
	static short turnPwmValue = 0;		//转向环PWM波的值
	static short leftPwmValue = 0;		//左边电机的PWM值
	static short rightPwmValue = 0;		//右边电机的PWM值
	static u8 wirelessCom = BLANCE_COM;	//无线指令
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)								//检查指定的TIM中断发生与否:TIM中断源
	{
		//呼吸灯指示运行状态
		time++;
		if (time > 99)
		{
			time = 0;
			LED = !LED;
		}

		//得到参数
		GetEncoderNum((short *)&ControlValue.encoderLeftNum, 
			(short *)&ControlValue.encoderRightNum);								//得到左右编码器的数值
		mpu_dmp_get_data((float *)&ControlValue.pitch, (float *)&ControlValue.roll, 
			(float *)&ControlValue.yaw, (short *)&ControlValue.yAngularSpeed, 
			(short *)&ControlValue.zAngularSpeed);									//更新位置
		wirelessCom = GetWirelessCom();												//得到无线命令
		
		//直立环调节
		upPwmValue = UprightPd(ControlValue.pitch, ControlValue.yAngularSpeed);		//直立PD调节

		//速度环调节
		if (GO_COM == wirelessCom)
		{
			velocityPwmValue = SpeedPi(ControlValue.encoderLeftNum, 
				ControlValue.encoderRightNum, GO);									//速度PI调节,前进
		}
		else if (RETREAT_COM == wirelessCom)
		{
			velocityPwmValue = SpeedPi(ControlValue.encoderLeftNum, 
				ControlValue.encoderRightNum, RETREAT);								//速度PI调节,后退
		}
		else
		{
			velocityPwmValue = SpeedPi(ControlValue.encoderLeftNum, 
				ControlValue.encoderRightNum, NORMAL_MODE);							//速度PI调节,正常积分
		}
		
		//转向环调节
		if (LEFT_COM == wirelessCom)
		{
			turnPwmValue = TurnP(ControlValue.zAngularSpeed, TURN_LEFT);			//转向环P调节,左转
		}
		else if (RIGHT_COM == wirelessCom)
		{
			turnPwmValue = TurnP(ControlValue.zAngularSpeed, TURN_RIGHT);			//转向环P调节,右转
		}
		else
		{
			turnPwmValue = TurnP(ControlValue.zAngularSpeed, NORMAL_MODE);			//转向环P调节,正常模式
		}
		
		//电机赋值
		leftPwmValue = upPwmValue + velocityPwmValue + turnPwmValue;				//左边电机PWM赋值
		rightPwmValue = upPwmValue + velocityPwmValue - turnPwmValue;				//右边电机PWM赋值
		
		LimitPwm(&leftPwmValue, &rightPwmValue);									//限幅
		if (CheckState(ControlValue.pitch) == 0)									//如果状态正常
		{
			SetPwm(leftPwmValue, rightPwmValue);									//设置电机的PWM值
		}
		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);									//清除TIMx的中断待处理位:TIM中断源
	}	
}

/******************************************************************
*函数名称:	GetEncoderNum
*函数功能:	得到左右编码器的数值
*函数参数:	encoderLeftNum:		左边编码器数的指针
*			encoderRightNum:	右边编码器数的指针
*返 回 值:	无
*******************************************************************/
static void GetEncoderNum(short *encoderLeftNum, short *encoderRightNum)
{
	u16 leftTemp;
	u16 rightTemp;	
	
	leftTemp = ReadEncoderValue(2);			//读取左编码器的值
	if (leftTemp > 32767)					//反转,向下计数
	{
		*encoderLeftNum = leftTemp - 65535;
	}
	else
	{
		*encoderLeftNum = leftTemp;
	}
	*encoderLeftNum = -*encoderLeftNum;		//左右编码器接口相反,故要对左边取反
	
	rightTemp = ReadEncoderValue(4);		//读取右编码器的值
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
*函数名称:	UprightPd
*函数功能:	直立PD控制
*函数参数:	pitch:			俯仰角
*			yAngularSpeed:	Y轴角速度
*返 回 值:	upPwmValue:		PWM波的占空比值
*******************************************************************/
static short UprightPd(float pitch, short yAngularSpeed)
{
	static Pid upPd = { 309, 0, 1.236, 0, 0, 0 };
	short upPwmValue;
	
	upPd.bias = pitch - 0;		//0度平衡
	
	upPwmValue = upPd.kp * upPd.bias + upPd.kd * yAngularSpeed;
	
	return upPwmValue;
}

/****************************************************************************************
*函数名称:	SpeedPi
*函数功能:	速度PI控制,当车停止时,用 EMPTY_INTEGRAL 模式清空积分
*函数参数:	encoderLeftNum:		左编码器的数
*			encoderRightNum:	右编码器的数
*			mode:				模式:	NORMAL_MODE:	正常积分
*										EMPTY_INTEGRAL:	清空积分
*										GO:				前进
*										RETREAT:		后退
*返 回 值:	velocityPwmValue:	PWM波的占空比值
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
	
	velocityPi.bias = (encoderLeftNum + encoderRightNum) - 0;				//目标值为0
	velocityPi.bias = velocityPi.bias * 0.2 + velocityPi.lastBias * 0.8;	//一阶滞后滤波
	velocityPi.integral += velocityPi.bias;
	
	//前进后退模式
	if (GO == mode)
	{
		velocityPi.integral -= 50;
	}
	else if (RETREAT == mode)
	{
		velocityPi.integral += 40;
	}
	
	//积分限幅
	if (velocityPi.integral > MAX_INTEGRAL)
	{
		velocityPi.integral = MAX_INTEGRAL;
	}
	else if (velocityPi.integral < -MAX_INTEGRAL)
	{
		velocityPi.integral = -MAX_INTEGRAL;
	}
	
	//计算velocityPwmValue的值的时候可能会出现超过short类型最大值的情况,故用int声明
	velocityPwmValue = velocityPi.kp * velocityPi.bias + velocityPi.ki * velocityPi.integral;
	
	//PI速度环限幅
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
*函数名称:	TurnP
*函数功能:	转向环P控制
*函数参数:	zAngularSpeed:	Z轴角速度
*			mode:			模式:	NORMAL_MODE:	正常模式
*									TURN_LEFT:		左转
*									TURN_RIGHT:		右转
*返 回 值:	turnPwmValue:	PWM波的占空比值
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
	
	//转向环限幅
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
*函数名称:	LimitPwm
*函数功能:	限制PWM波的幅值
*函数参数:	leftPwmValue:	左边电机的PWM值的指针
*			rightPwmValue:	右边电机的PWM值的指针
*返 回 值:	无
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
*函数名称:	CheckState
*函数功能:	检测小车的状态,如果俯仰角绝对值大于40度,停止电机
*函数参数:	pitch:	俯仰角
*返 回 值:	0:		状态正常
*			其他:	状态异常
*******************************************************************/
static u8 CheckState(float pitch)
{
	if ((pitch > 40) || (pitch < -40))	//俯仰角绝对值大于40度
	{
		//停止电机
		AIN1 = 0;
		AIN2 = 0;
		BIN1 = 0;
		BIN2 = 0;
		
		SpeedPi(0, 0, EMPTY_INTEGRAL);	//清空积分
		
		return 1;
	}
	
	return 0;
}

/******************************************************************
*函数名称:	SetPwm
*函数功能:	调节左右电机的PWM波
*函数参数:	leftPwmValue:	左边电机的PWM值
*			rightPwmValue:	右边电机的PWM值
*返 回 值:	无
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
