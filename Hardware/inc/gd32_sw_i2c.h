#ifndef _BSP_I2C_H
#define _BSP_I2C_H

#define I2C_WR	0x00
#define I2C_RD	0x01

void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(uint8_t ack);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);
void i2c_GPIO_Config(void);
uint8_t qst_sw_writereg(uint8_t slave, uint8_t reg_add, uint8_t reg_dat);
uint8_t qst_sw_readreg(uint8_t slave, uint8_t reg_add, uint8_t *buf, uint8_t num);

#endif
