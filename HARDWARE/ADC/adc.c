#include "adc.h"

/******************************************************************
*函数名称:	Adc1Init
*函数功能:	初始化ADC,用于测试电池电压
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void Adc1Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);	//使能ADC1通道时钟
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);											//设置ADC分频因子6,72M/6=12M,ADC最大时间不能超过14M
                        
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;									//PA4作为模拟通道输入引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1);															//复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;										//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);											//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
	
	ADC_Cmd(ADC1, ENABLE);														//使能指定的ADC1
	ADC_ResetCalibration(ADC1);													//使能复位校准	
	while (ADC_GetResetCalibrationStatus(ADC1));								//等待复位校准结束	
	ADC_StartCalibration(ADC1);													//开启AD校准
	while (ADC_GetCalibrationStatus(ADC1));										//等待校准结束
}

/******************************************************************
*函数名称:	GetAdc1Value
*函数功能:	得到ADC1指定通道的采样值
*函数参数:	channel:	采样通道
*返 回 值:	采样值
*******************************************************************/
int GetAdc1Value(u8 channel)
{
	//设置指定ADC的规则组通道,一个序列,采样时间
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);		//ADC1,ADC通道,采样时间为239.5周期	  			     
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);										//使能指定的ADC1的软件转换启动功能		 
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));								//等待转换结束
	
	return ADC_GetConversionValue(ADC1);										//返回最近一次ADC1规则组的转换结果
}

/******************************************************************
*函数名称:	GetBatteryVoltage
*函数功能:	得到电池的电压值
*函数参数:	无
*返 回 值:	voltage:	电池电压(单位V)
*******************************************************************/
float GetBatteryVoltage(void)
{
	float voltage;
	
	voltage = GetAdc1Value(BATTERY_ADC_CHANNEL) * 3.3 * 11.5 / 1.5 / 4096;		//电阻分压,具体根据原理图简单分析可以得到,单位V
	
	return voltage;
}
