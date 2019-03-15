#ifndef __QMCX983_H
#define __QMCX983_H
#include "gd32f10x.h"
#include "systick.h"
#include "bsp_delay.h"

#define QMCX983_ACC_I2C_ADDRESS		(0x2c<<1)
#define QMCX983_ACC_I2C_ADDRESS2	(0x2d<<1)

#define QMC5883L					255
#define QMC6983_A1_D1				0
#define QMC6983_E1					1	
#define QMC7983						2
#define QMC7983_LOW_SETRESET		3
#define QMC6983_E1_Metal			4
#define QMC7983_Vertical			5
#define QMC7983_Slope				6

#endif
