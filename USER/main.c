#include "main.h"

static void MpuCheckDmp(void);
static void DisInformation(void);
	
int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//�����ж����ȼ�����2
	
	delay_init();					//��ʱ������ʼ��
	uart_init(9600);				//���ڳ�ʼ��Ϊ9600
	JTAG_Set(JTAG_SWD_DISABLE);		//�ر�JTAG�ӿ�
	JTAG_Set(SWD_ENABLE);			//��SWD�ӿ�
	
	LedInit();						//��ʼ��LED
	KeyInit();						//��ʼ������
	Adc1Init();						//��ʼ��ADC1
	
	Nrf24l01Init();					//��ʼ��NRF24L01
	Nrf24l01SetRXMode();			//��������Ϊ����ģʽ
	
	OLED_Init();					//��ʼ��OLED
	
	MPU_Init();						//��ʼ��MPU6050
	MpuCheckDmp();					//���DMP
	
	Tim2EncoderInit();				//��ʼ��������1
	Tim4EncoderInit();				//��ʼ��������2
	
	MotorInit();					//��ʼ���������������
	MotorPwmInit();					//��ʼ�����ڵ������·PWM��
	
	Tim3TimeIntInit();				//��ʼ����ʱ��5,����5ms�ж�
	
	while (1)
	{
		KeyStartAndStop();			//���������͹ر�С��
		ReadWirelessCom();			//��ȡ��������
		DisInformation();			//��ʾ��Ϣ
	}
}

/******************************************************************
*��������:	MpuCheckDmp
*��������:	��ʼ��DMP,������Ƿ�ɹ�
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
static void MpuCheckDmp(void)
{
	while (mpu_dmp_init())
	{
		OLED_ShowString(10, 25, (u8 *)"MPU6050 Error!");
		OLED_Refresh_Gram();
		delay_ms(200);
		OLED_Clear();
 		delay_ms(200);
	}
}

/******************************************************************
*��������:	DisInformation
*��������:	������ʾƽ��С����һЩ��Ϣ:������,��߱�������ֵ,
*			�ұ߱�������ֵ,��ص�ѹֵ
*��������:	��
*�� �� ֵ:	��
*******************************************************************/
static void DisInformation(void)
{
	Control controlValueDisplay;
	float batteryVoltage;
	char disPitch[22];
	char disEncoderLeftNum[20];
	char disEncoderRightNum[20];
	char disBatteryVoltage[20];
	
	ReadControlValue(&controlValueDisplay);		//��ȡ����
	batteryVoltage = GetBatteryVoltage();		//�õ���ص�ѹ
	
	sprintf(disPitch, "Angle  : %.2fC   ", controlValueDisplay.pitch);
	sprintf(disEncoderLeftNum, "ELeft  : %d    ", controlValueDisplay.encoderLeftNum);
	sprintf(disEncoderRightNum, "ERight : %d    ", controlValueDisplay.encoderRightNum);
	sprintf(disBatteryVoltage, "Voltage: %.2fV   ", batteryVoltage);
	
	OLED_ShowString(0, 0, (u8 *)disPitch);
	OLED_ShowString(0, 10, (u8 *)disEncoderLeftNum);
	OLED_ShowString(0, 20, (u8 *)disEncoderRightNum);
	OLED_ShowString(0, 30, (u8 *)disBatteryVoltage);
	OLED_Refresh_Gram();
}
