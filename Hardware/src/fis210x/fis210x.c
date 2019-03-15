
#include "gd32f10x.h"
#if defined(USE_SW_I2C)
#include "gd32_sw_i2c.h"
#else
#include "gd32_i2c.h"
#endif
#include "fis210x.h"
#include "bsp_usart.h"
#include "bsp_delay.h"

#include <stdlib.h>

//#define FIS210X_USE_SW_IIC

#if defined(USE_SPI)
extern uint8_t qst_fis210x_spi_write(uint8_t Addr, uint8_t Data);
extern uint8_t qst_fis210x_spi_read(uint8_t Addr, uint8_t *pData, uint16_t Length);
#endif

#define qst_printf			console_write

//static uint16_t acc_lsb_div = 0;
//static uint16_t gyro_lsb_div = 0;
static fis210x_config_t g_fis210x;

uint8_t fis210x_write_reg(uint8_t reg, uint8_t value)
{
	uint8_t ret=0;

#if defined(USE_SW_I2C)
	ret = qst_sw_writereg(FIS210X_SLAVE_ADDR, reg, value);
#else
	ret = i2c_send_data(FIS210X_SLAVE_ADDR,reg, &value, 1);
#endif
	return ret;
}

uint8_t fis210x_read_reg(uint8_t reg, uint8_t* buf, uint8_t len)
{
	uint8_t ret=0;

#if defined(USE_SW_I2C)
	ret = qst_sw_readreg(FIS210X_SLAVE_ADDR, reg, buf, len);
#else
	ret = i2c_receive_data(FIS210X_SLAVE_ADDR, reg, buf, len);
#endif
	return ret;
}

void fis210x_config_acc(enum FIS210x_AccRange range, enum FIS210x_AccOdr odr)
{
	unsigned char ctl_dada;
	unsigned char range_set;

	switch (range)
	{
		case AccRange_2g:
			range_set = 0<<3;
			g_fis210x.acc_scale = (1<<14);
			break;
		case AccRange_4g:
			range_set = 1<<3;
			g_fis210x.acc_scale = (1<<13);
			break;
		case AccRange_8g:
			range_set = 2<<3;
			g_fis210x.acc_scale = (1<<12);
			break;
		default: 
			range_set = 2<<3;
			g_fis210x.acc_scale = (1<<12);
	}
	ctl_dada = (unsigned char)range_set|(unsigned char)odr;
	fis210x_write_reg(FisRegister_Ctrl2, ctl_dada);
#if 0
// set lpf enable
	read_reg(FisRegister_Ctrl5, &ctl_dada,1);
	ctl_dada &= 0xfc;
	ctl_dada |=0x02;
	write_reg(FisRegister_Ctrl5,ctl_dada);
// set lpf enable
#endif
}

void fis210x_config_gyro(enum FIS210x_GyrRange range, enum FIS210x_GyrOdr odr)
{
	// Set the CTRL3 register to configure dynamic range and ODR
	unsigned char ctl_dada; 
	ctl_dada = (unsigned char)range | (unsigned char)odr;
	fis210x_write_reg(FisRegister_Ctrl3, ctl_dada);

	// Store the scale factor for use when processing raw data
	switch (range)
	{
		case GyrRange_32dps:
			g_fis210x.gyr_scale = 1024;
			break;
		case GyrRange_64dps:
			g_fis210x.gyr_scale = 512;
			break;
		case GyrRange_128dps:
			g_fis210x.gyr_scale = 256;
			break;
		case GyrRange_256dps:
			g_fis210x.gyr_scale = 128;
			break;
		case GyrRange_512dps:
			g_fis210x.gyr_scale = 64;
			break;
		case GyrRange_1024dps:
			g_fis210x.gyr_scale = 32;
			break;
		case GyrRange_2048dps:
			g_fis210x.gyr_scale = 16;
			break;
		case GyrRange_2560dps:
			g_fis210x.gyr_scale = 8;
			break;
		default: 
			32;
	}

	// Conversion from degrees/s to rad/s if necessary
#if 0
	read_reg(FisRegister_Ctrl5,&ctl_dada,1);
	ctl_dada &= 0xf3;
	ctl_dada |=0x08;
	write_reg(FisRegister_Ctrl5,ctl_dada);
#endif
}

void fis210x_config_ae(enum FIS210x_AeOdr odr)
{	
	uint8_t ctr_reg;

	g_fis210x.ae_ori_scale = (1<<14);
	g_fis210x.ae_vel_scale = (1<<10);
	fis210x_write_reg(FisRegister_Ctrl6, odr);
	fis210x_read_reg(FisRegister_Ctrl6, &ctr_reg, 1);
	printf("FIS210x_AEConfig, %x\n",ctr_reg);
}


void fis210x_config_fliter(enum FIS210x_Type type, enum FIS210x_LpfConfig lpf_enable, enum FIS210x_HpfConfig hpf_enable)
{
	unsigned char ctl_dada; 

	fis210x_read_reg(FisRegister_Ctrl5,&ctl_dada,1);
	if(Type_Acc == type)
	{
		ctl_dada &= 0xfc;
		if(lpf_enable == Lpf_Enable)
			ctl_dada |= 0x02;
		if(hpf_enable == Hpf_Enable)
			ctl_dada |= 0x01;
	}
	else if(Type_Gyro == type)
	{
		ctl_dada &= 0xe3;
		if(lpf_enable == Lpf_Enable)
			ctl_dada |= 0x08;
		if(hpf_enable == Hpf_Enable)
			ctl_dada |= 0x04;
	}

	fis210x_write_reg(FisRegister_Ctrl5, ctl_dada);
}


void fis210x_read_acc_xyz(float acc_xyz[3])
{
	float			acc_xyz_tmp[3];	
	unsigned char	buf_reg[6];
	short 			raw_acc_xyz[3];

#if defined(USE_SPI)
	fis210x_read_reg(FisRegister_Ax_L, &buf_reg[0], 6); 	// 0x19, 25
#else
	fis210x_read_reg(FisRegister_Ax_L, &buf_reg[0], 1);		// 0x19, 25
	fis210x_read_reg(FisRegister_Ax_H, &buf_reg[1], 1);
	fis210x_read_reg(FisRegister_Ay_L, &buf_reg[2], 1);
	fis210x_read_reg(FisRegister_Ay_H, &buf_reg[3], 1);
	fis210x_read_reg(FisRegister_Az_L, &buf_reg[4], 1);
	fis210x_read_reg(FisRegister_Az_H, &buf_reg[5], 1);
#endif
	raw_acc_xyz[0] = (short)((buf_reg[1]<<8) |( buf_reg[0]));
	raw_acc_xyz[1] = (short)((buf_reg[3]<<8) |( buf_reg[2]));
	raw_acc_xyz[2] = (short)((buf_reg[5]<<8) |( buf_reg[4]));
//	if(cali_flag == FALSE)
//	{
//		raw_acc_xyz[0]+=offset[0];
//		raw_acc_xyz[1]+=offset[1];
//		raw_acc_xyz[2]+=offset[2];
//	}

	if(g_fis210x.acc_uint == AccUnit_ms2)
	{
		acc_xyz_tmp[0] = (raw_acc_xyz[0]*9.807f)/g_fis210x.acc_scale;
		acc_xyz_tmp[1] = (raw_acc_xyz[1]*9.807f)/g_fis210x.acc_scale;
		acc_xyz_tmp[2] = (raw_acc_xyz[2]*9.807f)/g_fis210x.acc_scale;
	}
	else
	{
		acc_xyz_tmp[0] = (raw_acc_xyz[0]*1.0f)/g_fis210x.acc_scale;
		acc_xyz_tmp[1] = (raw_acc_xyz[1]*1.0f)/g_fis210x.acc_scale;
		acc_xyz_tmp[2] = (raw_acc_xyz[2]*1.0f)/g_fis210x.acc_scale;
	}
//	acc_layout = 0;
//	acc_xyz[qst_map[acc_layout].map[0]] = qst_map[acc_layout].sign[0]*acc_xyz_tmp[0];
//	acc_xyz[qst_map[acc_layout].map[1]] = qst_map[acc_layout].sign[1]*acc_xyz_tmp[1];
//	acc_xyz[qst_map[acc_layout].map[2]] = qst_map[acc_layout].sign[2]*acc_xyz_tmp[2];
	acc_xyz[0] = acc_xyz_tmp[0];
	acc_xyz[1] = acc_xyz_tmp[1];
	acc_xyz[2] = acc_xyz_tmp[2];

	qst_printf("fis210x acc:	%f	%f	%f\n", acc_xyz[0], acc_xyz[1], acc_xyz[2]);
}

void fis210x_read_gyro_xyz(float gyro_xyz[3])
{
	float			gyro_xyz_tmp[3];	
	unsigned char	buf_reg[6];
	short 			raw_gyro_xyz[3];

#if defined(USE_SPI)
	fis210x_read_reg(FisRegister_Gx_L, &buf_reg[0], 6); 	// 0x19, 25
#else
	fis210x_read_reg(FisRegister_Gx_L, &buf_reg[0], 1);	
	fis210x_read_reg(FisRegister_Gx_H, &buf_reg[1], 1);
	fis210x_read_reg(FisRegister_Gy_L, &buf_reg[2], 1);
	fis210x_read_reg(FisRegister_Gy_H, &buf_reg[3], 1);
	fis210x_read_reg(FisRegister_Gz_L, &buf_reg[4], 1);
	fis210x_read_reg(FisRegister_Gz_H, &buf_reg[5], 1);
#endif
	raw_gyro_xyz[0] = (short)((buf_reg[1]<<8) |( buf_reg[0]));
	raw_gyro_xyz[1] = (short)((buf_reg[3]<<8) |( buf_reg[2]));
	raw_gyro_xyz[2] = (short)((buf_reg[5]<<8) |( buf_reg[4]));	
//	if(cali_flag == FALSE)
//	{
//		raw_gyro_xyz[0] += offset_gyro[0];
//		raw_gyro_xyz[1] += offset_gyro[1];
//		raw_gyro_xyz[2] += offset_gyro[2];
//	}

	if(g_fis210x.gyr_uint == GyrUnit_rads)
	{
		gyro_xyz_tmp[0] = (raw_gyro_xyz[0]*M_PI/180)/g_fis210x.gyr_scale;
		gyro_xyz_tmp[1] = (raw_gyro_xyz[1]*M_PI/180)/g_fis210x.gyr_scale;
		gyro_xyz_tmp[2] = (raw_gyro_xyz[2]*M_PI/180)/g_fis210x.gyr_scale;
	}
	else
	{
		gyro_xyz_tmp[0] = (raw_gyro_xyz[0]*1.0f)/g_fis210x.gyr_scale;
		gyro_xyz_tmp[1] = (raw_gyro_xyz[1]*1.0f)/g_fis210x.gyr_scale;
		gyro_xyz_tmp[2] = (raw_gyro_xyz[2]*1.0f)/g_fis210x.gyr_scale;
	}
//	acc_layout = 0;
//	gyro_xyz[qst_map[acc_layout].map[0]] = qst_map[acc_layout].sign[0]*gyro_xyz_tmp[0];
//	gyro_xyz[qst_map[acc_layout].map[1]] = qst_map[acc_layout].sign[1]*gyro_xyz_tmp[1];
//	gyro_xyz[qst_map[acc_layout].map[2]] = qst_map[acc_layout].sign[2]*gyro_xyz_tmp[2];
	gyro_xyz[0] = gyro_xyz_tmp[0];
	gyro_xyz[1] = gyro_xyz_tmp[1];
	gyro_xyz[2] = gyro_xyz_tmp[2];

	qst_printf("fis210x gyro:	%f	%f	%f\n", gyro_xyz[0], gyro_xyz[1], gyro_xyz[2]);
}


void fis210x_read_ae_ori(float q[4])
{
	uint8_t q1_l,q1_h;
	uint8_t q2_l,q2_h;
	uint8_t q3_l,q3_h;
	uint8_t q4_l,q4_h;
	int16_t raw_q[4];

	fis210x_read_reg(FisRegister_Q1_L,&q1_l,1);
	fis210x_read_reg(FisRegister_Q1_H,&q1_h,1);
	fis210x_read_reg(FisRegister_Q2_L,&q2_l,1);
	fis210x_read_reg(FisRegister_Q2_H,&q2_h,1);
	fis210x_read_reg(FisRegister_Q3_L,&q3_l,1);
	fis210x_read_reg(FisRegister_Q3_H,&q3_h,1);
	fis210x_read_reg(FisRegister_Q4_L,&q4_l,1);
	fis210x_read_reg(FisRegister_Q4_H,&q4_h,1);

	raw_q[0] = (int16_t)((uint16_t)q1_l|((uint16_t)q1_h << 8));
	raw_q[1] = (int16_t)((uint16_t)q2_l|((uint16_t)q2_h << 8));
	raw_q[2] = (int16_t)((uint16_t)q3_l|((uint16_t)q3_h << 8));
	raw_q[3] = (int16_t)((uint16_t)q4_l|((uint16_t)q4_h << 8));

	q[0] = (float)raw_q[0]/g_fis210x.ae_ori_scale;
	q[1] = (float)raw_q[1]/g_fis210x.ae_ori_scale;
	q[2] = (float)raw_q[2]/g_fis210x.ae_ori_scale;
	q[3] = (float)raw_q[3]/g_fis210x.ae_ori_scale;

	//printf("fis210x ae-ori q:[%f %f %f %f]\n", q[0],q[1],q[2],q[3]);
}

void fis210x_read_ae_vel(float v[4])
{
	uint8_t buf[6];
	int16_t raw_v[3];
	
	fis210x_read_reg(FisRegister_Dvx_L,&buf[0],1);
	fis210x_read_reg(FisRegister_Dvx_H,&buf[1],1);
	fis210x_read_reg(FisRegister_Dvy_L,&buf[2],1);
	fis210x_read_reg(FisRegister_Dvy_H,&buf[3],1);
	fis210x_read_reg(FisRegister_Dvz_L,&buf[4],1);
	fis210x_read_reg(FisRegister_Dvz_H,&buf[5],1);

	raw_v[0] = (int16_t)((uint16_t)buf[0]|((uint16_t)buf[1] << 8));
	raw_v[1] = (int16_t)((uint16_t)buf[2]|((uint16_t)buf[3] << 8));
	raw_v[2] = (int16_t)((uint16_t)buf[4]|((uint16_t)buf[5] << 8));

	printf("fis210x ae-vel v:[%f %f %f]\n", v[0],v[1],v[2]);
}


void fis210x_dumpReg(void)
{
	uint8_t ctrl_data;

	fis210x_read_reg(FisRegister_Ctrl2,&ctrl_data,1);
	printf("FisRegister_Ctrl2,%x\n",ctrl_data);
	fis210x_read_reg(FisRegister_Ctrl3,&ctrl_data,1);
	printf("FisRegister_Ctrl3,%x\n",ctrl_data);
	fis210x_read_reg(FisRegister_Ctrl5,&ctrl_data,1);
	printf("FisRegister_Ctrl5,%x\n",ctrl_data);
	fis210x_read_reg(FisRegister_Ctrl6,&ctrl_data,1);
	printf("FisRegister_Ctrl6,%x\n",ctrl_data);
	fis210x_read_reg(FisRegister_Ctrl7,&ctrl_data,1);
	printf("FisRegister_Ctrl7,%x\n",ctrl_data);
}

void fis210x_get_status0(uint8_t *status0)
{
	fis210x_read_reg(FisRegister_Status0, status0, 1);
}

void fis210x_get_status1(uint8_t *status1)
{
	fis210x_read_reg(FisRegister_Status1, status1, 1);
}

uint8_t fis210x_read_count(void)
{
	uint8_t count;

	fis210x_read_reg(FisRegister_CountOut, &count, 1);
	return count;
}

void fis210x_enable_sensors(uint8_t enableFlags)
{
	if(enableFlags & FISIMU_CTRL7_AE_ENABLE)
	{
		enableFlags |= FISIMU_CTRL7_ACC_ENABLE | FISIMU_CTRL7_GYR_ENABLE;
	}

	enableFlags = enableFlags & FISIMU_CTRL7_ENABLE_MASK;
	fis210x_write_reg(FisRegister_Ctrl7, enableFlags);
}

void fis210x_set_mode(enum FisImu_mode mode)
{
	uint8_t reg_value = 0;

	if(mode == FIS_MODE_LOW_POWER)
	{
		fis210x_config_acc(AccRange_4g, AccOdr_128Hz);
	
		reg_value = 0;
		fis210x_write_reg(FisRegister_Ctrl1, reg_value);
		fis210x_enable_sensors(FISIMU_CTRL7_DISABLE_ALL);
	}
	else if(mode == FIS_MODE_POWER_DOWN)
	{
		fis210x_config_acc(AccRange_4g, AccOdr_128Hz);
	
		reg_value = 1;
		fis210x_write_reg(FisRegister_Ctrl1, reg_value);
		fis210x_enable_sensors(FISIMU_CTRL7_DISABLE_ALL);
	}
	else
	{
		fis210x_config_acc(AccRange_4g, AccOdr_128Hz);
		
		reg_value = 0;
		fis210x_write_reg(FisRegister_Ctrl1, reg_value);
		fis210x_enable_sensors(FISIMU_CTRL7_ACC_ENABLE|FISIMU_CTRL7_GYR_ENABLE);
	}
}

uint8_t fis210x_get_id(void)
{
	uint8_t id=0;
	uint8_t icount=0;

	while(id != FIS210X_CHIP_ID)
	{
		fis210x_read_reg(FisRegister_WhoAmI, &id, 1);
		qst_printf("fis210x_init chip_id=0x%x \n", id);
		if(icount++ > 10)
		{
			break;
		}
	}
	return id;
}

uint8_t fis210x_init(void)
{
	uint8_t chip_id = 0x00;

#if defined(USE_SW_I2C)
	i2c_CheckDevice(FIS210X_SLAVE_ADDR);
	bsp_delay_ms(50);
#endif
	chip_id = fis210x_get_id();
	if(chip_id == FIS210X_CHIP_ID)
	{
		g_fis210x.acc_range = AccRange_8g;
		g_fis210x.acc_odr = AccOdr_1024Hz;
		g_fis210x.acc_uint = AccUnit_ms2;
		g_fis210x.gyr_range = GyrRange_2048dps;
		g_fis210x.gyr_odr = GyrOdr_1024Hz;
		g_fis210x.gyr_uint = GyrUnit_dps;
		g_fis210x.ae_odr = AeOdr_64Hz;
		g_fis210x.acc_scale = 1;
		g_fis210x.gyr_scale = 1;
		g_fis210x.ae_ori_scale = 1;
		g_fis210x.ae_vel_scale = 1;
		
		fis210x_config_acc(g_fis210x.acc_range, g_fis210x.acc_odr);
		fis210x_config_fliter(Type_Acc, Lpf_Enable, Hpf_Disable);
		fis210x_config_gyro(g_fis210x.gyr_range, g_fis210x.gyr_odr);
		fis210x_config_fliter(Type_Gyro, Lpf_Enable, Hpf_Disable);
		//fis210x_config_ae(g_fis210x.ae_odr);
		fis210x_enable_sensors(FISIMU_CTRL7_ACC_ENABLE|FISIMU_CTRL7_GYR_ENABLE);	//FISIMU_CTRL7_AE_ENABLE FISIMU_CTRL7_ACC_ENABLE|FISIMU_CTRL7_GYR_ENABLE
		//fis210x_set_mode(FIS_MODE_NOMAL);
	}
	else
	{
		chip_id = 0;
	}

	return chip_id;
}
