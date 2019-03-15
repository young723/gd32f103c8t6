#ifndef __QMAX981_H
#define __QMAX981_H

#if defined(USE_SW_I2C)
#define QMAX981_USE_SW_IIC
#endif
//#define USE_SPI

//#define QMA7981_DOUBLE_TRIPLE_CLICK
#define QMA7981_ABNORMAL_SHAKE_CHECK

//#define QMAX981_STEPCOUNTER
#define QMAX981_USE_IRQ1
#define QMA7981_ANY_MOTION
//#define QMA7981_NO_MOTION
//#define QMA7981_SIGNIFICANT_MOTION

#include "gd32f10x.h"
#if defined(USE_SPI)
#include "bsp_spi.h"
#elif defined(QMAX981_USE_SW_IIC)
#include "gd32_sw_i2c.h"
#else
#include "gd32_i2c.h"
#endif
#include "bsp_timer.h"

#include <stdbool.h>
#include <string.h>

typedef enum 
{
	ACC_RANGE_2G,
	ACC_RANGE_4G,
	ACC_RANGE_8G,
	ACC_RANGE_16G,
	ACC_RANGE_32G,

	ACC_RANGE_TOTAL
}qmaX981_range;

#define QMAX981_I2C_SLAVE_ADDR		0x12	// AD0 GND 0x12, AD0 VDD 0x13
#define QMAX981_I2C_SLAVE_ADDR2		0x13	// AD0 GND 0x12, AD0 VDD 0x13
#define QMAX981_ERR_I2C				-1
#define QMAX981_SUCCESS				0

#define GRAVITY_EARTH_1000          9807	// about (9.80665f)*1000   mm/s2
#define QMAX981_ABS(X) 				((X) < 0 ? (-1 * (X)) : (X))
#if defined(QMAX981_STEPCOUNTER)
#define QMA6981_OFFSET 				0x60
#else
#define QMA6981_OFFSET 				0x00
#endif

#define QMAX981_DELAY				0xff
/*Register Map*/
#define QMAX981_CHIP_ID		    	0x00
#define QMAX981_XOUTL				0x01
#define QMAX981_XOUTH				0x02
#define QMAX981_YOUTL				0x03
#define QMAX981_YOUTH				0x04
#define QMAX981_ZOUTL				0x05
#define QMAX981_ZOUTH				0x06
#define QMAX981_STEP_CNT_L			0x07
#define QMAX981_INT_STAT0			0x0a
#define QMAX981_INT_STAT1			0x0b
#define QMAX981_INT_STAT2			0x0c
#define QMAX981_INT_STAT3			0x0d
#define QMAX981_FIFO_STATE			0x0e
#define QMA7981_STEP_CNT_M			0x0e
#define QMAX981_REG_RANGE			0x0f
#define QMAX981_REG_BW_ODR			0x10
#define QMAX981_REG_POWER_CTL		0x11
#define QMAX981_STEP_SAMPLE_CNT		0x12
#define QMAX981_STEP_PRECISION		0x13
#define QMAX981_STEP_TIME_LOW		0x14
#define QMAX981_STEP_TIME_UP		0x15
#define QMAX981_INTPIN_CFG			0x20
#define QMAX981_INT_CFG				0x21
#define QMAX981_OS_CUST_X		    0x27
#define QMAX981_OS_CUST_Y			0x28
#define QMAX981_OS_CUST_Z			0x29
#define QMAX981_STEP_TIME_UP		0x15
/*ODR SET @lower ODR*/
#define QMA6981_ODR_1000HZ			0x07
#define QMA6981_ODR_500HZ			0x06
#define QMA6981_ODR_250HZ			0x05
#define QMA6981_ODR_125HZ			0x04  
#define QMA6981_ODR_62HZ			0x03   
#define QMA6981_ODR_31HZ			0x02   
#define QMA6981_ODR_16HZ			0x01
#define QMA6981_ODR_HIGH			0x20

/* Accelerometer Sensor Full Scale */
#define QMAX981_RANGE_2G			0x01
#define QMAX981_RANGE_4G			0x02
#define QMAX981_RANGE_8G			0x04
#define QMAX981_RANGE_16G			0x08
#define QMAX981_RANGE_32G			0x0f

/* 0x11 Set the sleep time, when device is in power cycling power saving.*/
#define QMA6981_SLEEP_DUR0			0x00
#define QMA6981_SLEEP_DUR1			0x06
#define QMA6981_SLEEP_DUR2			0x07
#define QMA6981_SLEEP_DUR4			0x08
#define QMA6981_SLEEP_DUR6			0x09
#define QMA6981_SLEEP_DUR10			0x0a
#define QMA6981_SLEEP_DUR25			0x0b
#define QMA6981_SLEEP_DUR50			0x0c
#define QMA6981_SLEEP_DUR100		0x0d
#define QMA6981_SLEEP_DUR500		0x0e
#define QMA6981_SLEEP_DUR1000		0x0f



/* Accelerometer Sensor Full Scale */

#define QMAX981_IRQ1_RCC			RCC_APB2Periph_GPIOA  
#define QMAX981_IRQ1_PORT			GPIOA 
#define QMAX981_IRQ1_PIN			GPIO_Pin_11
#define QMAX981_IRQ1_LINE			EXTI_Line11

#define QMAX981_IRQ2_RCC			RCC_APB2Periph_GPIOA  
#define QMAX981_IRQ2_PORT			GPIOA 
#define QMAX981_IRQ2_PIN			GPIO_Pin_5  
#define QMAX981_IRQ2_LINE			EXTI_Line5

#if defined(QMA7981_DOUBLE_TRIPLE_CLICK)
typedef struct
{
	unsigned char check_click;
	unsigned short click_num;
	unsigned short static_num;
	unsigned short t_msec_1;			// check_click timer
	unsigned short t_msec_2;			// check static timer
	unsigned short t_msec_out;			// timeout
}qst_click_check;
#endif

#if defined(QMA7981_ABNORMAL_SHAKE_CHECK)
typedef struct
{
	unsigned char check_shake;
	unsigned short shake_num;
	unsigned short t_msec_1;
	unsigned short t_msec_out;			// timeout
}qst_shake_check;
#endif

extern int32_t qmaX981_init(void);
extern int32_t qmaX981_read_acc(int32_t *accData);
#if defined(QMAX981_STEPCOUNTER)
extern u32 qmaX981_read_stepcounter(void);
#endif


#endif  /*QMX6981*/
