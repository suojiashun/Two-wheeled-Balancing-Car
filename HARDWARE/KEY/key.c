#include "key.h"

/******************************************************************
*函数名称:	KeyInit
*函数功能:	初始化按键
*函数参数:	无
*返 回 值:	无
*******************************************************************/
void KeyInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//根据设定参数初始化GPIOA 
}
