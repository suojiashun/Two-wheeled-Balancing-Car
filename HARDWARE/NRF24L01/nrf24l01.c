#include "delay.h"
#include "nrf24l01.h"

/*********************************************NRF24L01*********************************************/
#define	TX_ADR_WIDTH	ADDR_WIDTH	//发送地址宽度
#define RX_ADR_WIDTH	ADDR_WIDTH	//接收地址宽度
#define TX_PLOAD_WIDTH	DATA_WIDTH	//发送数据长度(字节)
#define RX_PLOAD_WIDTH	DATA_WIDTH	//接收数据长度(字节)
/**************************************************************************************************/

/****************************************NRF24L01寄存器指令****************************************/
#define NRF_READ_REG	0x00		//读寄存器指令
#define NRF_WRITE_REG	0x20		//写寄存器指令
#define RD_RX_PLOAD		0x61		//读取接收数据指令
#define WR_TX_PLOAD		0xA0		//写待发数据指令
#define FLUSH_TX		0xE1		//冲洗发送FIFO指令
#define FLUSH_RX		0xE2		//冲洗接收FIFO指令
#define REUSE_TX_PL		0xE3		//定义重复装载数据指令
#define NOP				0xFF		//保留
/**************************************************************************************************/

/**************************************SPI(NRF24L01)寄存器地址*************************************/
#define CONFIG			0x00		//配置收发状态,CRC校验模式以及收发状态响应方式
#define EN_AA			0x01		//自动应答功能设置
#define EN_RXADDR		0x02		//可用信道设置
#define SETUP_AW		0x03		//收发地址宽度设置
#define SETUP_RETR		0x04		//自动重发功能设置
#define RF_CH			0x05		//工作频率设置
#define RF_SETUP		0x06		//发射速率,功耗功能设置
#define STATUS			0x07		//状态寄存器
#define OBSERVE_TX		0x08		//发送监测功能
#define CD				0x09		//地址检测
#define TX_ADDR			0x10 		//发送地址寄存器 
#define RX_ADDR_P0		0x0A		//频道0接收数据地址
#define RX_ADDR_P1		0x0B 		//频道1接收数据地址
#define RX_ADDR_P2		0x0C 		//频道2接收数据地址
#define RX_ADDR_P3		0x0D		//频道3接收数据地址
#define RX_ADDR_P4		0x0E 		//频道4接收数据地址
#define RX_ADDR_P5		0x0F		//频道5接收数据地址
#define RX_PW_P0		0x11		//接收频道0接收数据长度
#define RX_PW_P1		0x12 		//接收频道0接收数据长度
#define RX_PW_P2		0x13 		//接收频道0接收数据长度
#define RX_PW_P3		0x14 		//接收频道0接收数据长度
#define RX_PW_P4		0x15 		//接收频道0接收数据长度
#define RX_PW_P5		0x16		//接收频道0接收数据长度
#define FIFO_STATUS		0x17		//FIFO栈入栈出状态寄存器设置
#define MAX_TX			0x10		//达到最大发送次数中断
#define TX_OK			0x20		//TX发送完成中断
#define RX_OK			0x40		//接收到数据中断
/**************************************************************************************************/

static unsigned char SpiReadAndWrite(unsigned char dat);
static unsigned char Nrf24l01WriteReg(const unsigned char reg, const unsigned char dat);
static unsigned char Nrf24l01ReadReg(const unsigned char reg);
static unsigned char Nrf24l01WriteBuf(const unsigned char reg, const unsigned char *pBuf, const unsigned char len);
static unsigned char Nrf24l01ReadBuf(const unsigned char reg, unsigned char *pBuf, const unsigned char len);

static const unsigned char txAddr[TX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };		//发送地址
static const unsigned char rxAddr[RX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };		//接收地址

/**********************************************************************
*函数名称:	Nrf24l01Init
*函数功能:	初始化NRF24L01
*函数参数: 	无
*返 回 值:	无
***********************************************************************/
void Nrf24l01Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	//NRF20L01 IO口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//SPI IO口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NRF24L01_CE = 0;										//芯片使能
	NRF24L01_CSN = 1;										//关闭SPI
	NRF24L01_SCK = 0;										//初使化SPI时钟
	NRF24L01_IRQ = 1;										//中断IO口置1	
}

/**********************************************************************
*函数名称:	CheckNrf24l01
*函数功能:	检查NRF24L01是否存在
*函数参数: 	无
*返 回 值:	CHECK_SUCCEED:	NRF24L01存在
*			CHECK_ERROR:	NRF24L01不存在
***********************************************************************/
unsigned char CheckNrf24l01(void)
{
	unsigned char checkArr[5] = { 0xa5, 0xa5, 0xa5, 0xa5, 0xa5 };
	unsigned char i;

	Nrf24l01WriteBuf(NRF_WRITE_REG + TX_ADDR, checkArr, 5);		//写入5个字节的地址
	Nrf24l01ReadBuf(TX_ADDR, checkArr, 5);						//读出5个字节的地址

	for (i = 0; i < 5; i++)
	{
		if (0xa5 != checkArr[i])
		{
			return CHECK_ERROR;									//检测Nrf24l01错误
		}
	}

	return CHECK_SUCCEED;										//检测到Nrf24l01
}

/**********************************************************************
*函数名称:	Nrf24l01SetTxMode
*函数功能:	设置NRF24L01为发送模式
*函数参数: 	无
*返 回 值:	无
***********************************************************************/
void Nrf24l01SetTXMode(void)
{
	NRF24L01_CE = 0;

	Nrf24l01WriteBuf(NRF_WRITE_REG + TX_ADDR, txAddr, TX_ADR_WIDTH);		//设置本机地址
	Nrf24l01WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, rxAddr, RX_ADR_WIDTH);		//设置接收地址

	Nrf24l01WriteReg(NRF_WRITE_REG + EN_AA, 0x01);							//使能通道0的自动应答    
	Nrf24l01WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);						//使能通道0的接收地址  
	Nrf24l01WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);						//设置自动重发间隔时间:500us+86us;最大自动重发次数:10次
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_CH, 40);							//设置RF通道为40
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);						//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	Nrf24l01WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);							//配置基本工作模式的参数;PWR_UP, EN_CRC,16BIT_CRC,接收模式,开启所有中断

	NRF24L01_CE = 1;														//CE为高,10us后启动发送
	delay_us(10);
}

/**********************************************************************
*函数名称:	Nrf24l01TxPacket
*函数功能:	NRF24L01发送数据
*函数参数: 	txBuf:		指向要发送的数据的首地址
*返 回 值:	TX_SUCCEED:	发送完成
*			TX_ERROR:	发送失败
***********************************************************************/
unsigned char Nrf24l01TXPacket(const unsigned char *txBuf)
{
	unsigned char status;

	NRF24L01_CE = 0;
	Nrf24l01WriteBuf(WR_TX_PLOAD, txBuf, TX_PLOAD_WIDTH);
	NRF24L01_CE = 1;

	while (0 != NRF24L01_IRQ);							//等待数据发送完成

	status = Nrf24l01ReadReg(STATUS);					//读取状态寄存器的值

	Nrf24l01WriteReg(NRF_WRITE_REG + STATUS, status);	//清除TX_DS或MAX_RT中断标志

	if (status & TX_OK)									//发送完成
	{
		return TX_SUCCEED;
	}

	if (status & MAX_TX)								//达到最大重发次数
	{
		Nrf24l01WriteReg(FLUSH_TX, 0xff);				//清除TX FIF0寄存器

		return TX_ERROR;
	}

	return TX_ERROR;									//其他原因发送失败
}

/**********************************************************************
*函数名称:	Nrf24l01SetRxMode
*函数功能:	设置NRF24L01为接收模式
*函数参数: 	无
*返 回 值:	无
***********************************************************************/
void Nrf24l01SetRXMode(void)
{
	NRF24L01_CE = 0;

	Nrf24l01WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, rxAddr, RX_ADR_WIDTH);		//写接收地址

	Nrf24l01WriteReg(NRF_WRITE_REG + EN_AA, 0x01);							//使能通道0的自动应答    
	Nrf24l01WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);						//使能通道0的接收地址  	 
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_CH, 40);							//设置RF通信频率		  
	Nrf24l01WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);				//选择通道0的有效数据宽度 	    
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);						//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	Nrf24l01WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);							//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 

	NRF24L01_CE = 1;	//CE为高,进入接收模式	
}

/**********************************************************************
*函数名称:	Nrf24l01RXPacket
*函数功能:	NRF24L01接收数据
*函数参数: 	rxBuf:	指向要接收的数据的首地址
*返 回 值:	RX_SUCCEED:	接收完成
*			RX_ERROR:	接收失败
***********************************************************************/
unsigned char Nrf24l01RXPacket(unsigned char *rxBuf)
{
	unsigned char status;

	status = Nrf24l01ReadReg(STATUS);							//读取状态寄存器的值

	Nrf24l01WriteReg(NRF_WRITE_REG + STATUS, status);			//清除TX_DS或MAX_RT中断标志

	if (status & RX_OK)											//接收到数据
	{
		Nrf24l01ReadBuf(RD_RX_PLOAD, rxBuf, RX_PLOAD_WIDTH);	//读取数据
		Nrf24l01WriteReg(FLUSH_RX, 0xff);						//清除RX FIFO寄存器

		return RX_SUCCEED;
	}

	return RX_ERROR;
}

/**********************************************************************
*函数名称:	SpiReadAndWrite
*函数功能:	SPI读写数据
*函数参数: 	dat:	需要写入的数据
*返 回 值:	dat:	读取的数据
***********************************************************************/
static unsigned char SpiReadAndWrite(unsigned char dat)
{
	unsigned char i;

	for (i = 0; i < 8; i++)
	{
		NRF24L01_MOSI = dat >> 7;	//写入数据,从高位开始
		dat <<= 1;

		NRF24L01_SCK = 1;
		dat |= NRF24L01_MISO;		//读取数据
		NRF24L01_SCK = 0;
	}

	return dat;
}

/**********************************************************************
*函数名称:	Nrf24l01WriteReg
*函数功能:	使用SPI向NRF24L01的寄存器中写入数据
*函数参数:	reg:	写入的寄存器的地址
*			dat:	需要写入的数据
*返 回 值:	status:	状态值
***********************************************************************/
static unsigned char Nrf24l01WriteReg(const unsigned char reg, const unsigned char dat)
{
	unsigned char status;

	NRF24L01_CSN = 0;				//启动SPI

	status = SpiReadAndWrite(reg);	//选择要操作的寄存器
	SpiReadAndWrite(dat);			//向选择的寄存器中写入数据

	NRF24L01_CSN = 1;				//关闭SPI

	return status;
}

/**********************************************************************
*函数名称:	Nrf24l01ReadReg
*函数功能:	使用SPI读取NRF24L01的寄存器中的数据
*函数参数:	reg:		要读的寄存器的地址
*返 回 值:	readData:	读取的内容
***********************************************************************/
static unsigned char Nrf24l01ReadReg(const unsigned char reg)
{
	unsigned char readData;

	NRF24L01_CSN = 0;							//启动SPI

	SpiReadAndWrite(reg);						//写入寄存器地址
	readData = SpiReadAndWrite(NRF_READ_REG);	//读取寄存器上的内容

	NRF24L01_CSN = 1;							//关闭SPI

	return readData;
}

/**********************************************************************
*函数名称:	Nrf24l01WriteBuf
*函数功能:	在指定的地址写指定长度的数据
*函数参数:	reg:	要写的寄存器的地址
*			pBuf:	指向要写的数据的首地址的指针
*			len:	数据的长度
*返 回 值:	tatus:	寄存器的状态值
***********************************************************************/
static unsigned char Nrf24l01WriteBuf(const unsigned char reg, const unsigned char *pBuf, const unsigned char len)
{
	unsigned char status, i;

	NRF24L01_CSN = 0;				//启动SPI

	status = SpiReadAndWrite(reg);	//发送寄存器位置,并读取状态值

	for (i = 0; i < len; i++)
	{
		SpiReadAndWrite(*pBuf);		//写入数据
		pBuf++;
	}

	NRF24L01_CSN = 1;				//关闭SPI

	return status;
}

/**********************************************************************
*函数名称:	Nrf24l01ReadBuf
*函数功能:	在指定的地址读出指定长度的数据
*函数参数:	reg:	要读的寄存器的地址
*			pBuf:	指向保存数据的数组首地址的指针
*			len:	数据的长度
*返 回 值:	tatus:	寄存器的状态值
***********************************************************************/
static unsigned char Nrf24l01ReadBuf(const unsigned char reg, unsigned char *pBuf, const unsigned char len)
{
	unsigned char status, i;

	NRF24L01_CSN = 0;							//启动SPI

	status = SpiReadAndWrite(reg);				//发送寄存器位置,并读取状态值

	for (i = 0; i < len; i++)
	{
		*pBuf = SpiReadAndWrite(NRF_READ_REG);	//读出数据
		pBuf++;
	}

	NRF24L01_CSN = 1;							//关闭SPI

	return status;
}
