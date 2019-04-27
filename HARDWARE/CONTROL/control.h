#ifndef CONTROL_H_
#define CONTROL_H_

#include "sys.h"

#define PWM_MAX_VALUE	7000	//PWM波限幅,最大为7200
#define MAX_INTEGRAL	2750	//积分限幅最大值
#define	MAX_PI			7000	//PI速度环输出PWM的最大值
#define MAX_TURN_PWM	6000	//转向环输出PWM的最大值

/************************SpeedPi()函数专用************************/
#define NORMAL_MODE		0		//正常积分
#define EMPTY_INTEGRAL	1		//清空积分
#define GO				2		//前进模式
#define RETREAT			3		//后退模式
/*****************************************************************/

/*************************TurnP()函数专用*************************/
#define NORMAL_MODE		0		//正常模式
#define TURN_LEFT		1		//左转
#define TURN_RIGHT		2		//右转
/*****************************************************************/

typedef struct control
{
	float pitch;				//俯仰角
	float roll;					//横滚角
	float yaw;					//航向角		
	short yAngularSpeed;		//Y轴角速度
	short zAngularSpeed;		//Z轴角速度
	short encoderLeftNum;		//左边编码器的数
	short encoderRightNum;		//右边编码器的数
} Control;

typedef struct pid
{
	float kp;
	float ki;
	float kd;
	float bias;					//偏差值
	float lastBias;				//上一次偏差值
	float integral;				//积分值
} Pid;

/******************************************************************
*函数名称:	KeyStartAndStop
*函数功能:	按键用于开启和关闭小车
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void KeyStartAndStop(void);

/***********************************************************************
*函数名称:	ReadControlValue
*函数功能:	用于读取控制变量
*函数参数:	readData:	Control类型的指针
*返 回 值:	无
************************************************************************/
void ReadControlValue(Control *readData);

/******************************************************************
*函数名称:	Tim3TimeIntInit
*函数功能:	初始化定时器3,用于5ms中断
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void Tim3TimeIntInit(void);

#endif
