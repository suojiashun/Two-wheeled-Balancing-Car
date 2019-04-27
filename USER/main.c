#include "main.h"

static void MpuCheckDmp(void);
static void DisInformation(void);
	
int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//设置中断优先级分组2
	
	delay_init();					//延时函数初始化
	uart_init(9600);				//串口初始化为9600
	JTAG_Set(JTAG_SWD_DISABLE);		//关闭JTAG接口
	JTAG_Set(SWD_ENABLE);			//打开SWD接口
	
	LedInit();						//初始化LED
	KeyInit();						//初始化按键
	Adc1Init();						//初始化ADC1
	
	Nrf24l01Init();					//初始化NRF24L01
	Nrf24l01SetRXMode();			//设置无线为接收模式
	
	OLED_Init();					//初始化OLED
	
	MPU_Init();						//初始化MPU6050
	MpuCheckDmp();					//检查DMP
	
	Tim2EncoderInit();				//初始化编码器1
	Tim4EncoderInit();				//初始化编码器2
	
	MotorInit();					//初始化电机驱动的引脚
	MotorPwmInit();					//初始化用于电机的两路PWM波
	
	Tim3TimeIntInit();				//初始化定时器5,用于5ms中断
	
	while (1)
	{
		KeyStartAndStop();			//按键开启和关闭小车
		ReadWirelessCom();			//读取无线命令
		DisInformation();			//显示信息
	}
}

/******************************************************************
*函数名称:	MpuCheckDmp
*函数功能:	初始化DMP,并检查是否成功
*函数参数:	无
*返 回 值:	无
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
*函数名称:	DisInformation
*函数功能:	用于显示平衡小车的一些信息:俯仰角,左边编码器的值,
*			右边编码器的值,电池电压值
*函数参数:	无
*返 回 值:	无
*******************************************************************/
static void DisInformation(void)
{
	Control controlValueDisplay;
	float batteryVoltage;
	char disPitch[22];
	char disEncoderLeftNum[20];
	char disEncoderRightNum[20];
	char disBatteryVoltage[20];
	
	ReadControlValue(&controlValueDisplay);		//读取数据
	batteryVoltage = GetBatteryVoltage();		//得到电池电压
	
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
