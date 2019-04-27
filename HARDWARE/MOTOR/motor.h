#ifndef MOTOR_H_
#define MOTOR_H_

#include "sys.h"

#define PWMA	TIM1->CCR1	//PA8
#define PWMB	TIM1->CCR4	//PA11
#define AIN1	PBout(14)
#define AIN2	PBout(15)
#define BIN1	PBout(13)
#define BIN2	PBout(12)

/******************************************************************
*函数名称:	MotorInit
*函数功能:	初始化电机驱动的引脚
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void MotorInit(void);

/******************************************************************
*函数名称:	MotorPwmInit
*函数功能:	初始化用于电机的两路PWM波,频率为10KHZ
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void MotorPwmInit(void);

#endif
