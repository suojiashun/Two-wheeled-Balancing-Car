#ifndef CONTROL_H_
#define CONTROL_H_

#include "sys.h"

#define PWM_MAX_VALUE	7000	//PWM���޷�,���Ϊ7200
#define MAX_INTEGRAL	2750	//�����޷����ֵ
#define	MAX_PI			7000	//PI�ٶȻ����PWM�����ֵ
#define MAX_TURN_PWM	6000	//ת�����PWM�����ֵ

/************************SpeedPi()����ר��************************/
#define NORMAL_MODE		0		//��������
#define EMPTY_INTEGRAL	1		//��ջ���
#define GO				2		//ǰ��ģʽ
#define RETREAT			3		//����ģʽ
/*****************************************************************/

/*************************TurnP()����ר��*************************/
#define NORMAL_MODE		0		//����ģʽ
#define TURN_LEFT		1		//��ת
#define TURN_RIGHT		2		//��ת
/*****************************************************************/

typedef struct control
{
	float pitch;				//������
	float roll;					//�����
	float yaw;					//�����		
	short yAngularSpeed;		//Y����ٶ�
	short zAngularSpeed;		//Z����ٶ�
	short encoderLeftNum;		//��߱���������
	short encoderRightNum;		//�ұ߱���������
} Control;

typedef struct pid
{
	float kp;
	float ki;
	float kd;
	float bias;					//ƫ��ֵ
	float lastBias;				//��һ��ƫ��ֵ
	float integral;				//����ֵ
} Pid;

/******************************************************************
*��������:	KeyStartAndStop
*��������:	�������ڿ����͹ر�С��
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void KeyStartAndStop(void);

/***********************************************************************
*��������:	ReadControlValue
*��������:	���ڶ�ȡ���Ʊ���
*��������:	readData:	Control���͵�ָ��
*�� �� ֵ:	��
************************************************************************/
void ReadControlValue(Control *readData);

/******************************************************************
*��������:	Tim3TimeIntInit
*��������:	��ʼ����ʱ��3,����5ms�ж�
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
void Tim3TimeIntInit(void);

#endif
