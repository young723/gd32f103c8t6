
#include "qmcX983.h"
#if defined(USE_SW_I2C)
#include "gd32_sw_i2c.h"
#else
#include "gd32_i2c.h"
#endif

#include <stdlib.h>
//#include <string.h>
#include "bsp_usart.h"


#define QMAX981_LOG		console_write		//printf
#define QMAX981_ERR		console_write		//printf
uint8_t mag_chip_id;

uint8_t qmcX983_write_reg(uint8_t slave, uint8_t reg_add, uint8_t reg_dat)
{
#if defined(USE_SW_I2C)
	return qst_sw_writereg(slave, reg_add, reg_dat);
#else
	return i2c_send_data(slave, reg_add ,&reg_add, 1);
#endif
}

uint8_t qmcX983_read_reg(uint8_t slave, uint8_t reg_add, uint8_t *buf, uint8_t num)
{
#if defined(USE_SW_I2C)
	return qst_sw_readreg(slave, reg_add, buf, num);
#else
	return i2c_receive_data(slave, reg_add ,buf, num);
#endif
}

uint8_t qmcX983_read_xyz(void)
{
	uint8_t reg_data[6];
	int16_t raw[3];
	uint8_t err = 0;

	//qmcX983_read_reg(QMC_II_ADDR, 0x06, &reg_data[0], 1);
	err = qmcX983_read_reg(QMCX983_ACC_I2C_ADDRESS, 0x00, reg_data, 6);
 	raw[0] = (int16_t)((reg_data[1]<<8)|(reg_data[0]));
	raw[1] = (int16_t)((reg_data[3]<<8)|(reg_data[2]));
	raw[2] = (int16_t)((reg_data[5]<<8)|(reg_data[4]));

#if defined(USE_QMC5883L)
	QMAX981_LOG("5883L %f %f %f\n",(float)raw[0]/30.0f,(float)raw[1]/30.0f,(float)raw[2]/30.0f);
#else
	QMAX981_LOG("mag %f %f %f\n",(float)raw[0]/25.0f,(float)raw[1]/25.0f,(float)raw[2]/25.0f);
#endif
	return err;
}


int qmcX983_init(void)
{
	uint8_t ret, chip_id;

#if defined(USE_SW_I2C)
	ret = i2c_CheckDevice(QMCX983_ACC_I2C_ADDRESS);
	QMAX981_LOG("i2c_CheckDevice ret=%d \n", ret);
#endif
	bsp_delay_ms(50);
#if defined(QMC5883L_SUPPORT)
	unsigned char value;

	//mag_addr = 0x0c;		// 0x0c:ad0-->gnd, 0x0d:ad0--vdd
	qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x0b, 0x01);
	qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x09,0x1d);
	bsp_delay_ms(10);
	qmcX983_read_reg(QMCX983_ACC_I2C_ADDRESS, 0x0c, &value, 1);
	QMAX981_LOG("0x0c=0x%x \n", value);
	qmcX983_read_reg(QMCX983_ACC_I2C_ADDRESS, 0x0d, &value, 1);
	QMAX981_LOG("0x0d=0x%x \n", value);
	mag_chip_id = QMC5883L;

	return TRUE;
#else
	ret = qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x09, 0x1d);
	bsp_delay_ms(10);
	if(!ret)
	{
		QMAX981_LOG("qmcX983_init i2c error! \n");
		return FALSE;
	}
	ret = qmcX983_read_reg(QMCX983_ACC_I2C_ADDRESS, 0x0d, &chip_id, 1);
	QMAX981_LOG("qmcX983_init chip_id=0x%x \n", chip_id);
	if(!ret)
	{
		QMAX981_LOG("qmcX983_init i2c error! \n");
		return FALSE;
	}
	/*
	if(chip_id == 0xff)
	{
		chip_id = QMC6983_A1_D1;
	}
	else*/
	if(0x31 == chip_id)
	{
		mag_chip_id = QMC6983_E1;
	}
	else if(0x32 == chip_id)
	{
		qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x2e, 0x01);
		qmcX983_read_reg(QMCX983_ACC_I2C_ADDRESS, 0x2f, &chip_id, 1);
		if(((chip_id&0x04 )>> 2))
		{
			mag_chip_id = QMC6983_E1_Metal;
		}
		else
		{
			qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x2e, 0x0f);
			qmcX983_read_reg(QMCX983_ACC_I2C_ADDRESS, 0x2f, &chip_id, 1);
			if(0x02 == ((chip_id&0x3c)>>2))
			{
				mag_chip_id = QMC7983_Vertical;
			}
			if(0x03 == ((chip_id&0x3c)>>2))
			{
				mag_chip_id = QMC7983_Slope;
			}
		}
	}
	else
	{
		return FALSE;
	}
	QMAX981_LOG("qmcX983_init done mag_chip_id=%d \n", mag_chip_id);
	qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x21, 0x01);
	qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x20, 0x40);

	if(mag_chip_id != QMC6983_A1_D1)
	{	
		qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x29, 0x80);
		qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x0a, 0x0c);
	}

	if((mag_chip_id == QMC6983_E1_Metal) || (mag_chip_id == QMC7983_Slope))
	{			
		qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x1b, 0x80);
	}

	qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x0b, 0x01);
	qmcX983_write_reg(QMCX983_ACC_I2C_ADDRESS, 0x09,0x1d);
	//qmcX983_get_otp_kxky();

	return TRUE;
#endif

}

