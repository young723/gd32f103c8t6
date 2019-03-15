
#include "gd32f10x_i2c.h"
#include "gd32_i2c.h"

static uint32_t i2c_timeout = 0;

void i2c_config(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6|GPIO_PIN_7);
	i2c_deinit(I2C0);
	/* enable I2C0 clock */
	rcu_periph_clock_enable(RCU_I2C0);
	/* configure I2C0 clock */
	i2c_clock_config(I2C0, 100000, I2C_DTCY_2);
	/* configure I2C0 address */
	i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C0_SLAVE_ADDRESS7);
	/* enable I2C0 */
	i2c_enable(I2C0);
	/* enable acknowledge */
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);

#if defined(USE_I2C_1)
	/* enable I2C1 clock */
	gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_10|GPIO_PIN_11);
	rcu_periph_clock_enable(RCU_I2C1);
	/* configure I2C1 clock */
	i2c_clock_config(I2C1, 100000, I2C_DTCY_2);
	/* configure I2C1 address */
	i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C1_SLAVE_ADDRESS7);
	/* enable I2C1 */
	i2c_enable(I2C1);
	/* enable acknowledge */
	i2c_ack_config(I2C1, I2C_ACK_ENABLE);
#endif
}

uint8_t i2c_send_data(uint8_t slave, uint8_t reg, uint8_t* buff, uint8_t len)
{
	uint8_t i;
	/* wait until I2C bus is idle */
	while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
	/* send a start condition to I2C bus */
	i2c_start_on_bus(I2C0);
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	/* wait until SBSEND bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
	/* send slave address to I2C bus */
	i2c_master_addressing(I2C0, slave, I2C_TRANSMITTER);
	/* wait until ADDSEND bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
	/* clear ADDSEND bit */
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	/* wait until the transmit data buffer is empty */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));

	i2c_data_transmit(I2C0, reg);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));

	for(i=0; i<len; i++){
		/* data transmission */
		i2c_data_transmit(I2C0, buff[i]);
		/* wait until the TBE bit is set */
		while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	}
	/* send a stop condition to I2C bus */
	i2c_stop_on_bus(I2C0);
	while(I2C_CTL0(I2C0)&0x0200);
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);

	return 1;
}


uint8_t i2c_receive_onebyte(uint8_t slave, uint8_t reg, uint8_t* buff)
{
	while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
	/* send a start condition to I2C bus */
	i2c_start_on_bus(I2C0);
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	/* wait until SBSEND bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
	/* send slave address to I2C bus */
	i2c_master_addressing(I2C0, slave, I2C_TRANSMITTER);
	/* wait until ADDSEND bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
	/* clear ADDSEND bit */
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	/* wait until the transmit data buffer is empty */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));

	i2c_data_transmit(I2C0, reg);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));

	/* send a stop condition to I2C bus */
	i2c_stop_on_bus(I2C0);
	while(I2C_CTL0(I2C0)&0x0200);
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);

	while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
	/* send a start condition to I2C bus */
	i2c_start_on_bus(I2C0);
	/* wait until SBSEND bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
	/* send slave address to I2C bus */
	i2c_master_addressing(I2C0, slave, I2C_RECEIVER);
	/* wait until ADDSEND bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));

	/* N=1,reset ACKEN bit before clearing ADDRSEND bit */
	i2c_ack_config(I2C0, I2C_ACK_DISABLE);
	/* clear ADDSEND bit */
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	/* N=1,send stop condition after clearing ADDRSEND bit */
	/* wait until the RBNE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
	/* read a data from I2C_DATA */
	buff[0] = i2c_data_receive(I2C0);

	i2c_stop_on_bus(I2C0);
	while(I2C_CTL0(I2C0)&0x0200);
	/* Enable Acknowledge */
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);

	return 1;
}

uint8_t i2c_receive_data(uint8_t slave, uint8_t reg, uint8_t* buff, uint8_t len)
{
	uint8_t i, ret;

	for(i=0; i<len; i++)
	{
		ret = i2c_receive_onebyte(slave, reg+i, &buff[i]);
	}
	return ret;
}

