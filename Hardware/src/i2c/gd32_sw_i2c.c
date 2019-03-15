
#include "gd32f10x.h"
#include "gd32_sw_i2c.h"

/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define GPIO_PORT_I2C	GPIOB			/* GPIO端口 */
#define I2C_SCL_PIN		GPIO_PIN_6			/* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN		GPIO_PIN_7			/* 连接到SDA数据线的GPIO */

/* 定义读写SCL和SDA的宏，已增加代码的可移植性和可阅读性 */
#define I2C_SCL_OUTPUT()	//gpio_init(GPIO_PORT_I2C,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,I2C_SCL_PIN)
#define I2C_SCL_INPUT()		//gpio_init(GPIO_PORT_I2C,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,I2C_SCL_PIN)
#define I2C_SDA_OUTPUT()	//gpio_init(GPIO_PORT_I2C,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,I2C_SDA_PIN)
#define I2C_SDA_INPUT()		//gpio_init(GPIO_PORT_I2C,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,I2C_SDA_PIN)

#define I2C_SCL_1()  gpio_bit_set(GPIO_PORT_I2C, I2C_SCL_PIN)		/* SCL = 1 */
#define I2C_SCL_0()  gpio_bit_reset(GPIO_PORT_I2C, I2C_SCL_PIN)		/* SCL = 0 */

#define I2C_SDA_1()  gpio_bit_set(GPIO_PORT_I2C, I2C_SDA_PIN)		/* SDA = 1 */
#define I2C_SDA_0()  gpio_bit_reset(GPIO_PORT_I2C, I2C_SDA_PIN)		/* SDA = 0 */

#define I2C_SDA_READ()  gpio_input_bit_get(GPIO_PORT_I2C, I2C_SDA_PIN)	/* 读SDA口线状态 */

/*
*********************************************************************************************************
*	函 数 名: i2c_Delay
*	功能说明: I2C总线位延迟，最快400KHz
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: i2c_Start
*	功能说明: CPU发起I2C总线启动信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	
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
*	函 数 名: i2c_Start
*	功能说明: CPU发起I2C总线停止信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_Stop(void)
{
	I2C_SCL_OUTPUT();
	I2C_SDA_OUTPUT();

	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	I2C_SDA_0();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_SendByte
*	功能说明: CPU向I2C总线设备发送8bit数据
*	形    参：_ucByte ： 等待发送的字节
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
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
			 I2C_SDA_1(); // 释放总线
		}
		_ucByte <<= 1;	/* 左移一个bit */
		i2c_Delay();
	}
}

/*
*********************************************************************************************************
*	函 数 名: i2c_ReadByte
*	功能说明: CPU从I2C总线设备读取8bit数据
*	形    参：无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t i2c_ReadByte(uint8_t ack)
{
	uint8_t i;
	uint8_t value;

	/* 读到第1个bit为数据的bit7 */
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
*	函 数 名: i2c_WaitAck
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	形    参：无
*	返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
uint8_t i2c_WaitAck(void)
{
	uint8_t re;

	I2C_SDA_1();	/* CPU释放SDA总线 */
	I2C_SDA_INPUT();	//set data input
	i2c_Delay();
	I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	i2c_Delay();
	if(I2C_SDA_READ())	/* CPU读取SDA口线状态 */
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
*	函 数 名: i2c_Ack
*	功能说明: CPU产生一个ACK信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_Ack(void)
{
	I2C_SDA_0();	/* CPU驱动SDA = 0 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钟 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*
*********************************************************************************************************
*	函 数 名: i2c_NAck
*	功能说明: CPU产生1个NACK信号
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_NAck(void)
{
	I2C_SDA_1();	/* CPU驱动SDA = 1 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钟 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();	
}

/*
*********************************************************************************************************
*	函 数 名: i2c_GPIO_Config
*	功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实现
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_GPIO_Config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIO_PORT_I2C, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, I2C_SCL_PIN|I2C_SDA_PIN);
	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	i2c_Stop();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_CheckDevice
*	功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*	形    参：_Address：设备的I2C总线地址
*	返 回 值: 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	i2c_GPIO_Config();		/* 配置GPIO */
	i2c_Start();		/* 发送启动信号 */
	/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
	i2c_SendByte(_Address|I2C_WR);
	ucAck = i2c_WaitAck();	/* 检测设备的ACK应答 */
	i2c_Stop();			/* 发送停止信号 */

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

