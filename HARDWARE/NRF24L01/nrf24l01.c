#include "delay.h"
#include "nrf24l01.h"

/*********************************************NRF24L01*********************************************/
#define	TX_ADR_WIDTH	ADDR_WIDTH	//���͵�ַ���
#define RX_ADR_WIDTH	ADDR_WIDTH	//���յ�ַ���
#define TX_PLOAD_WIDTH	DATA_WIDTH	//�������ݳ���(�ֽ�)
#define RX_PLOAD_WIDTH	DATA_WIDTH	//�������ݳ���(�ֽ�)
/**************************************************************************************************/

/****************************************NRF24L01�Ĵ���ָ��****************************************/
#define NRF_READ_REG	0x00		//���Ĵ���ָ��
#define NRF_WRITE_REG	0x20		//д�Ĵ���ָ��
#define RD_RX_PLOAD		0x61		//��ȡ��������ָ��
#define WR_TX_PLOAD		0xA0		//д��������ָ��
#define FLUSH_TX		0xE1		//��ϴ����FIFOָ��
#define FLUSH_RX		0xE2		//��ϴ����FIFOָ��
#define REUSE_TX_PL		0xE3		//�����ظ�װ������ָ��
#define NOP				0xFF		//����
/**************************************************************************************************/

/**************************************SPI(NRF24L01)�Ĵ�����ַ*************************************/
#define CONFIG			0x00		//�����շ�״̬,CRCУ��ģʽ�Լ��շ�״̬��Ӧ��ʽ
#define EN_AA			0x01		//�Զ�Ӧ��������
#define EN_RXADDR		0x02		//�����ŵ�����
#define SETUP_AW		0x03		//�շ���ַ�������
#define SETUP_RETR		0x04		//�Զ��ط���������
#define RF_CH			0x05		//����Ƶ������
#define RF_SETUP		0x06		//��������,���Ĺ�������
#define STATUS			0x07		//״̬�Ĵ���
#define OBSERVE_TX		0x08		//���ͼ�⹦��
#define CD				0x09		//��ַ���
#define TX_ADDR			0x10 		//���͵�ַ�Ĵ��� 
#define RX_ADDR_P0		0x0A		//Ƶ��0�������ݵ�ַ
#define RX_ADDR_P1		0x0B 		//Ƶ��1�������ݵ�ַ
#define RX_ADDR_P2		0x0C 		//Ƶ��2�������ݵ�ַ
#define RX_ADDR_P3		0x0D		//Ƶ��3�������ݵ�ַ
#define RX_ADDR_P4		0x0E 		//Ƶ��4�������ݵ�ַ
#define RX_ADDR_P5		0x0F		//Ƶ��5�������ݵ�ַ
#define RX_PW_P0		0x11		//����Ƶ��0�������ݳ���
#define RX_PW_P1		0x12 		//����Ƶ��0�������ݳ���
#define RX_PW_P2		0x13 		//����Ƶ��0�������ݳ���
#define RX_PW_P3		0x14 		//����Ƶ��0�������ݳ���
#define RX_PW_P4		0x15 		//����Ƶ��0�������ݳ���
#define RX_PW_P5		0x16		//����Ƶ��0�������ݳ���
#define FIFO_STATUS		0x17		//FIFOջ��ջ��״̬�Ĵ�������
#define MAX_TX			0x10		//�ﵽ����ʹ����ж�
#define TX_OK			0x20		//TX��������ж�
#define RX_OK			0x40		//���յ������ж�
/**************************************************************************************************/

static unsigned char SpiReadAndWrite(unsigned char dat);
static unsigned char Nrf24l01WriteReg(const unsigned char reg, const unsigned char dat);
static unsigned char Nrf24l01ReadReg(const unsigned char reg);
static unsigned char Nrf24l01WriteBuf(const unsigned char reg, const unsigned char *pBuf, const unsigned char len);
static unsigned char Nrf24l01ReadBuf(const unsigned char reg, unsigned char *pBuf, const unsigned char len);

static const unsigned char txAddr[TX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };		//���͵�ַ
static const unsigned char rxAddr[RX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };		//���յ�ַ

/**********************************************************************
*��������:	Nrf24l01Init
*��������:	��ʼ��NRF24L01
*��������: 	��
*�� �� ֵ:	��
***********************************************************************/
void Nrf24l01Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	//NRF20L01 IO������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//SPI IO������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NRF24L01_CE = 0;										//оƬʹ��
	NRF24L01_CSN = 1;										//�ر�SPI
	NRF24L01_SCK = 0;										//��ʹ��SPIʱ��
	NRF24L01_IRQ = 1;										//�ж�IO����1	
}

/**********************************************************************
*��������:	CheckNrf24l01
*��������:	���NRF24L01�Ƿ����
*��������: 	��
*�� �� ֵ:	CHECK_SUCCEED:	NRF24L01����
*			CHECK_ERROR:	NRF24L01������
***********************************************************************/
unsigned char CheckNrf24l01(void)
{
	unsigned char checkArr[5] = { 0xa5, 0xa5, 0xa5, 0xa5, 0xa5 };
	unsigned char i;

	Nrf24l01WriteBuf(NRF_WRITE_REG + TX_ADDR, checkArr, 5);		//д��5���ֽڵĵ�ַ
	Nrf24l01ReadBuf(TX_ADDR, checkArr, 5);						//����5���ֽڵĵ�ַ

	for (i = 0; i < 5; i++)
	{
		if (0xa5 != checkArr[i])
		{
			return CHECK_ERROR;									//���Nrf24l01����
		}
	}

	return CHECK_SUCCEED;										//��⵽Nrf24l01
}

/**********************************************************************
*��������:	Nrf24l01SetTxMode
*��������:	����NRF24L01Ϊ����ģʽ
*��������: 	��
*�� �� ֵ:	��
***********************************************************************/
void Nrf24l01SetTXMode(void)
{
	NRF24L01_CE = 0;

	Nrf24l01WriteBuf(NRF_WRITE_REG + TX_ADDR, txAddr, TX_ADR_WIDTH);		//���ñ�����ַ
	Nrf24l01WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, rxAddr, RX_ADR_WIDTH);		//���ý��յ�ַ

	Nrf24l01WriteReg(NRF_WRITE_REG + EN_AA, 0x01);							//ʹ��ͨ��0���Զ�Ӧ��    
	Nrf24l01WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);						//ʹ��ͨ��0�Ľ��յ�ַ  
	Nrf24l01WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);						//�����Զ��ط����ʱ��:500us+86us;����Զ��ط�����:10��
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_CH, 40);							//����RFͨ��Ϊ40
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);						//����TX�������,0db����,2Mbps,���������濪��   
	Nrf24l01WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);							//���û�������ģʽ�Ĳ���;PWR_UP, EN_CRC,16BIT_CRC,����ģʽ,���������ж�

	NRF24L01_CE = 1;														//CEΪ��,10us����������
	delay_us(10);
}

/**********************************************************************
*��������:	Nrf24l01TxPacket
*��������:	NRF24L01��������
*��������: 	txBuf:		ָ��Ҫ���͵����ݵ��׵�ַ
*�� �� ֵ:	TX_SUCCEED:	�������
*			TX_ERROR:	����ʧ��
***********************************************************************/
unsigned char Nrf24l01TXPacket(const unsigned char *txBuf)
{
	unsigned char status;

	NRF24L01_CE = 0;
	Nrf24l01WriteBuf(WR_TX_PLOAD, txBuf, TX_PLOAD_WIDTH);
	NRF24L01_CE = 1;

	while (0 != NRF24L01_IRQ);							//�ȴ����ݷ������

	status = Nrf24l01ReadReg(STATUS);					//��ȡ״̬�Ĵ�����ֵ

	Nrf24l01WriteReg(NRF_WRITE_REG + STATUS, status);	//���TX_DS��MAX_RT�жϱ�־

	if (status & TX_OK)									//�������
	{
		return TX_SUCCEED;
	}

	if (status & MAX_TX)								//�ﵽ����ط�����
	{
		Nrf24l01WriteReg(FLUSH_TX, 0xff);				//���TX FIF0�Ĵ���

		return TX_ERROR;
	}

	return TX_ERROR;									//����ԭ����ʧ��
}

/**********************************************************************
*��������:	Nrf24l01SetRxMode
*��������:	����NRF24L01Ϊ����ģʽ
*��������: 	��
*�� �� ֵ:	��
***********************************************************************/
void Nrf24l01SetRXMode(void)
{
	NRF24L01_CE = 0;

	Nrf24l01WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, rxAddr, RX_ADR_WIDTH);		//д���յ�ַ

	Nrf24l01WriteReg(NRF_WRITE_REG + EN_AA, 0x01);							//ʹ��ͨ��0���Զ�Ӧ��    
	Nrf24l01WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);						//ʹ��ͨ��0�Ľ��յ�ַ  	 
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_CH, 40);							//����RFͨ��Ƶ��		  
	Nrf24l01WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);				//ѡ��ͨ��0����Ч���ݿ�� 	    
	Nrf24l01WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);						//����TX�������,0db����,2Mbps,���������濪��   
	Nrf24l01WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);							//���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 

	NRF24L01_CE = 1;	//CEΪ��,�������ģʽ	
}

/**********************************************************************
*��������:	Nrf24l01RXPacket
*��������:	NRF24L01��������
*��������: 	rxBuf:	ָ��Ҫ���յ����ݵ��׵�ַ
*�� �� ֵ:	RX_SUCCEED:	�������
*			RX_ERROR:	����ʧ��
***********************************************************************/
unsigned char Nrf24l01RXPacket(unsigned char *rxBuf)
{
	unsigned char status;

	status = Nrf24l01ReadReg(STATUS);							//��ȡ״̬�Ĵ�����ֵ

	Nrf24l01WriteReg(NRF_WRITE_REG + STATUS, status);			//���TX_DS��MAX_RT�жϱ�־

	if (status & RX_OK)											//���յ�����
	{
		Nrf24l01ReadBuf(RD_RX_PLOAD, rxBuf, RX_PLOAD_WIDTH);	//��ȡ����
		Nrf24l01WriteReg(FLUSH_RX, 0xff);						//���RX FIFO�Ĵ���

		return RX_SUCCEED;
	}

	return RX_ERROR;
}

/**********************************************************************
*��������:	SpiReadAndWrite
*��������:	SPI��д����
*��������: 	dat:	��Ҫд�������
*�� �� ֵ:	dat:	��ȡ������
***********************************************************************/
static unsigned char SpiReadAndWrite(unsigned char dat)
{
	unsigned char i;

	for (i = 0; i < 8; i++)
	{
		NRF24L01_MOSI = dat >> 7;	//д������,�Ӹ�λ��ʼ
		dat <<= 1;

		NRF24L01_SCK = 1;
		dat |= NRF24L01_MISO;		//��ȡ����
		NRF24L01_SCK = 0;
	}

	return dat;
}

/**********************************************************************
*��������:	Nrf24l01WriteReg
*��������:	ʹ��SPI��NRF24L01�ļĴ�����д������
*��������:	reg:	д��ļĴ����ĵ�ַ
*			dat:	��Ҫд�������
*�� �� ֵ:	status:	״ֵ̬
***********************************************************************/
static unsigned char Nrf24l01WriteReg(const unsigned char reg, const unsigned char dat)
{
	unsigned char status;

	NRF24L01_CSN = 0;				//����SPI

	status = SpiReadAndWrite(reg);	//ѡ��Ҫ�����ļĴ���
	SpiReadAndWrite(dat);			//��ѡ��ļĴ�����д������

	NRF24L01_CSN = 1;				//�ر�SPI

	return status;
}

/**********************************************************************
*��������:	Nrf24l01ReadReg
*��������:	ʹ��SPI��ȡNRF24L01�ļĴ����е�����
*��������:	reg:		Ҫ���ļĴ����ĵ�ַ
*�� �� ֵ:	readData:	��ȡ������
***********************************************************************/
static unsigned char Nrf24l01ReadReg(const unsigned char reg)
{
	unsigned char readData;

	NRF24L01_CSN = 0;							//����SPI

	SpiReadAndWrite(reg);						//д��Ĵ�����ַ
	readData = SpiReadAndWrite(NRF_READ_REG);	//��ȡ�Ĵ����ϵ�����

	NRF24L01_CSN = 1;							//�ر�SPI

	return readData;
}

/**********************************************************************
*��������:	Nrf24l01WriteBuf
*��������:	��ָ���ĵ�ַдָ�����ȵ�����
*��������:	reg:	Ҫд�ļĴ����ĵ�ַ
*			pBuf:	ָ��Ҫд�����ݵ��׵�ַ��ָ��
*			len:	���ݵĳ���
*�� �� ֵ:	tatus:	�Ĵ�����״ֵ̬
***********************************************************************/
static unsigned char Nrf24l01WriteBuf(const unsigned char reg, const unsigned char *pBuf, const unsigned char len)
{
	unsigned char status, i;

	NRF24L01_CSN = 0;				//����SPI

	status = SpiReadAndWrite(reg);	//���ͼĴ���λ��,����ȡ״ֵ̬

	for (i = 0; i < len; i++)
	{
		SpiReadAndWrite(*pBuf);		//д������
		pBuf++;
	}

	NRF24L01_CSN = 1;				//�ر�SPI

	return status;
}

/**********************************************************************
*��������:	Nrf24l01ReadBuf
*��������:	��ָ���ĵ�ַ����ָ�����ȵ�����
*��������:	reg:	Ҫ���ļĴ����ĵ�ַ
*			pBuf:	ָ�򱣴����ݵ������׵�ַ��ָ��
*			len:	���ݵĳ���
*�� �� ֵ:	tatus:	�Ĵ�����״ֵ̬
***********************************************************************/
static unsigned char Nrf24l01ReadBuf(const unsigned char reg, unsigned char *pBuf, const unsigned char len)
{
	unsigned char status, i;

	NRF24L01_CSN = 0;							//����SPI

	status = SpiReadAndWrite(reg);				//���ͼĴ���λ��,����ȡ״ֵ̬

	for (i = 0; i < len; i++)
	{
		*pBuf = SpiReadAndWrite(NRF_READ_REG);	//��������
		pBuf++;
	}

	NRF24L01_CSN = 1;							//�ر�SPI

	return status;
}
