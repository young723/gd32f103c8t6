
#include "gd32f10x.h"
#include "gd32_sw_i2c.h"

/* ����I2C�������ӵ�GPIO�˿�, �û�ֻ��Ҫ�޸�����4�д��뼴������ı�SCL��SDA������ */
#define GPIO_PORT_I2C	GPIOB			/* GPIO�˿� */
#define I2C_SCL_PIN		GPIO_PIN_6			/* ���ӵ�SCLʱ���ߵ�GPIO */
#define I2C_SDA_PIN		GPIO_PIN_7			/* ���ӵ�SDA�����ߵ�GPIO */

/* �����дSCL��SDA�ĺ꣬�����Ӵ���Ŀ���ֲ�ԺͿ��Ķ��� */
#define I2C_SCL_OUTPUT()	//gpio_init(GPIO_PORT_I2C,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,I2C_SCL_PIN)
#define I2C_SCL_INPUT()		//gpio_init(GPIO_PORT_I2C,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,I2C_SCL_PIN)
#define I2C_SDA_OUTPUT()	//gpio_init(GPIO_PORT_I2C,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,I2C_SDA_PIN)
#define I2C_SDA_INPUT()		//gpio_init(GPIO_PORT_I2C,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,I2C_SDA_PIN)

#define I2C_SCL_1()  gpio_bit_set(GPIO_PORT_I2C, I2C_SCL_PIN)		/* SCL = 1 */
#define I2C_SCL_0()  gpio_bit_reset(GPIO_PORT_I2C, I2C_SCL_PIN)		/* SCL = 0 */

#define I2C_SDA_1()  gpio_bit_set(GPIO_PORT_I2C, I2C_SDA_PIN)		/* SDA = 1 */
#define I2C_SDA_0()  gpio_bit_reset(GPIO_PORT_I2C, I2C_SDA_PIN)		/* SDA = 0 */

#define I2C_SDA_READ()  gpio_input_bit_get(GPIO_PORT_I2C, I2C_SDA_PIN)	/* ��SDA����״̬ */

/*
*********************************************************************************************************
*	�� �� ��: i2c_Delay
*	����˵��: I2C����λ�ӳ٣����400KHz
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
	//uint8_t i;

	//for (i = 0; i < 50; i++);
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
	
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Start
*	����˵��: CPU����I2C���������ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_Start(void)
{
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C���������ź� */
	
	I2C_SCL_OUTPUT();
	I2C_SDA_OUTPUT();
	
	I2C_SDA_1();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_0();
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Start
*	����˵��: CPU����I2C����ֹͣ�ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_Stop(void)
{
	I2C_SCL_OUTPUT();
	I2C_SDA_OUTPUT();

	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C����ֹͣ�ź� */
	I2C_SDA_0();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_SendByte
*	����˵��: CPU��I2C�����豸����8bit����
*	��    �Σ�_ucByte �� �ȴ����͵��ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* �ȷ����ֽڵĸ�λbit7 */
	for (i = 0; i < 8; i++)
	{		
		if (_ucByte & 0x80)
		{
			I2C_SDA_1();
		}
		else
		{
			I2C_SDA_0();
		}
		i2c_Delay();
		I2C_SCL_1();
		i2c_Delay();	
		I2C_SCL_0();
		if (i == 7)
		{
			 I2C_SDA_1(); // �ͷ�����
		}
		_ucByte <<= 1;	/* ����һ��bit */
		i2c_Delay();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_ReadByte
*	����˵��: CPU��I2C�����豸��ȡ8bit����
*	��    �Σ���
*	�� �� ֵ: ����������
*********************************************************************************************************
*/
uint8_t i2c_ReadByte(uint8_t ack)
{
	uint8_t i;
	uint8_t value;

	/* ������1��bitΪ���ݵ�bit7 */
	I2C_SDA_INPUT();	// set data input	
	i2c_Delay();
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		//I2C_SCL_1();
		//i2c_Delay();
		if (I2C_SDA_READ())
		{
			value++;
		}
		I2C_SCL_1();
		i2c_Delay();
		I2C_SCL_0();
		i2c_Delay();
	}
	
	I2C_SDA_OUTPUT();	// set data output	
	i2c_Delay();
	if(ack==0)
		i2c_NAck();
	else
		i2c_Ack();
	return value;
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_WaitAck
*	����˵��: CPU����һ��ʱ�ӣ�����ȡ������ACKӦ���ź�
*	��    �Σ���
*	�� �� ֵ: ����0��ʾ��ȷӦ��1��ʾ��������Ӧ
*********************************************************************************************************
*/
uint8_t i2c_WaitAck(void)
{
	uint8_t re;

	I2C_SDA_1();	/* CPU�ͷ�SDA���� */
	I2C_SDA_INPUT();	//set data input
	i2c_Delay();
	I2C_SCL_1();	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
	i2c_Delay();
	if(I2C_SDA_READ())	/* CPU��ȡSDA����״̬ */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	I2C_SCL_0();
	I2C_SDA_OUTPUT();	//set data input
	i2c_Delay();
	return re;
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Ack
*	����˵��: CPU����һ��ACK�ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_Ack(void)
{
	I2C_SDA_0();	/* CPU����SDA = 0 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU����1��ʱ�� */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1();	/* CPU�ͷ�SDA���� */
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_NAck
*	����˵��: CPU����1��NACK�ź�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_NAck(void)
{
	I2C_SDA_1();	/* CPU����SDA = 1 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU����1��ʱ�� */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();	
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_GPIO_Config
*	����˵��: ����I2C���ߵ�GPIO������ģ��IO�ķ�ʽʵ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_GPIO_Config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIO_PORT_I2C, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, I2C_SCL_PIN|I2C_SDA_PIN);
	/* ��һ��ֹͣ�ź�, ��λI2C�����ϵ������豸������ģʽ */
	i2c_Stop();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_CheckDevice
*	����˵��: ���I2C�����豸��CPU�����豸��ַ��Ȼ���ȡ�豸Ӧ�����жϸ��豸�Ƿ����
*	��    �Σ�_Address���豸��I2C���ߵ�ַ
*	�� �� ֵ: ����ֵ 0 ��ʾ��ȷ�� ����1��ʾδ̽�⵽
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	i2c_GPIO_Config();		/* ����GPIO */
	i2c_Start();		/* ���������ź� */
	/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
	i2c_SendByte(_Address|I2C_WR);
	ucAck = i2c_WaitAck();	/* ����豸��ACKӦ�� */
	i2c_Stop();			/* ����ֹͣ�ź� */

	return ucAck;
}


uint8_t qst_sw_writereg(uint8_t slave, uint8_t reg_add, uint8_t reg_dat)
{
	i2c_Start();
	i2c_SendByte(slave|I2C_WR);
	if(i2c_WaitAck())
	{
		return 0;
	}
	i2c_SendByte(reg_add);	
	if(i2c_WaitAck())
	{
		return 0;
	}
	i2c_SendByte(reg_dat);	
	if(i2c_WaitAck())
	{
		return 0;
	}
	i2c_Stop();

	return 1;
}

uint8_t qst_sw_readreg(uint8_t slave, uint8_t reg_add, uint8_t *buf, uint8_t num)
{
	//uint8_t ret;
	uint8_t i;

	i2c_Start();
	i2c_SendByte(slave|I2C_WR);
	if(i2c_WaitAck())
	{
		return 0;
	}
	i2c_SendByte(reg_add);
	if(i2c_WaitAck())
	{
		return 0;
	}

	i2c_Start();
	i2c_SendByte(slave|I2C_RD);
	if(i2c_WaitAck())
	{
		return 0;
	}

	for(i=0;i<(num-1);i++){
		*buf=i2c_ReadByte(1);
		buf++;
	}
	*buf=i2c_ReadByte(0);
	i2c_Stop();

	return 1;
}

