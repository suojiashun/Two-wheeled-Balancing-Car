#include "led.h"

/******************************************************************
*函数名称:	LedInit
*函数功能:	初始化LED
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void LedInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50M
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//根据设定参数初始化GPIOA
	
	LED = 1;
}
