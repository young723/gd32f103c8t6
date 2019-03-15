
#ifndef GD32_I2C_H
#define GD32_I2C_H

#define I2C0_SLAVE_ADDRESS7		0x0a
#define I2C1_SLAVE_ADDRESS7		0x0b
#define I2C_TIMEOUT_MAX			5000

#define I2C_OK			1
#define I2C_ERROR		0

void i2c_config(void);
uint8_t i2c_send_data(uint8_t slave, uint8_t reg, uint8_t* buff, uint8_t len);
uint8_t i2c_receive_data(uint8_t slave, uint8_t reg, uint8_t* buff, uint8_t len);

#endif /* SYS_TICK_H */
