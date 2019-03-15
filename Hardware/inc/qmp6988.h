#ifndef __QMCX983_H
#define __QMCX983_H
#include "gd32f10x.h"
#include "gd32_sw_i2c.h"

#define QMP6988_SLAVE_ADDRESS  (0x70<<1)

#define QMP6988_U16_t uint16_t
#define QMP6988_S16_t int16_t
#define QMP6988_U32_t uint32_t
#define QMP6988_S32_t int32_t
#define QMP6988_U64_t uint64_t
#define QMP6988_S64_t int64_t


#define QMP6988_CHIP_ID						0x5C

#define QMP6988_CHIP_ID_REG					0xD1
#define QMP6988_RESET_REG             		0xE0  /* Device reset register */
#define QMP6988_DEVICE_STAT_REG             0xF3  /* Device state register */
#define QMP6988_CTRLMEAS_REG                0xF4  /* Measurement Condition Control Register */
/* data */
#define QMP6988_PRESSURE_MSB_REG            0xF7  /* Pressure MSB Register */
#define QMP6988_TEMPERATURE_MSB_REG         0xFA  /* Temperature MSB Reg */

/* compensation calculation */
#define QMP6988_CALIBRATION_DATA_START      0xA0 /* QMP6988 compensation coefficients */
#define QMP6988_CALIBRATION_DATA_LENGTH		25

#define SHIFT_RIGHT_4_POSITION				 4
#define SHIFT_LEFT_2_POSITION                2
#define SHIFT_LEFT_4_POSITION                4
#define SHIFT_LEFT_5_POSITION                5
#define SHIFT_LEFT_8_POSITION                8
#define SHIFT_LEFT_12_POSITION               12
#define SHIFT_LEFT_16_POSITION               16

/* power mode */
#define QMP6988_SLEEP_MODE                    0x00
#define QMP6988_FORCED_MODE                   0x01
#define QMP6988_NORMAL_MODE                   0x03

#define QMP6988_CTRLMEAS_REG_MODE__POS        0
#define QMP6988_CTRLMEAS_REG_MODE__MSK        0x03
#define QMP6988_CTRLMEAS_REG_MODE__LEN        2

/* oversampling */
#define QMP6988_OVERSAMPLING_SKIPPED          0x00
#define QMP6988_OVERSAMPLING_1X               0x01
#define QMP6988_OVERSAMPLING_2X               0x02
#define QMP6988_OVERSAMPLING_4X               0x03
#define QMP6988_OVERSAMPLING_8X               0x04
#define QMP6988_OVERSAMPLING_16X              0x05
#define QMP6988_OVERSAMPLING_32X              0x06
#define QMP6988_OVERSAMPLING_64X              0x07


#define QMP6988_CTRLMEAS_REG_OSRST__POS       5
#define QMP6988_CTRLMEAS_REG_OSRST__MSK       0xE0
#define QMP6988_CTRLMEAS_REG_OSRST__LEN       3


#define QMP6988_CTRLMEAS_REG_OSRSP__POS       2
#define QMP6988_CTRLMEAS_REG_OSRSP__MSK       0x1C
#define QMP6988_CTRLMEAS_REG_OSRSP__LEN       3


/* filter */
#define QMP6988_FILTERCOEFF_OFF               0x00
#define QMP6988_FILTERCOEFF_2                 0x01
#define QMP6988_FILTERCOEFF_4                 0x02
#define QMP6988_FILTERCOEFF_8                 0x03
#define QMP6988_FILTERCOEFF_16                0x04
#define QMP6988_FILTERCOEFF_32                0x05


#define QMP6988_CONFIG_REG                    0xF1  /*IIR filter co-efficient setting Register*/

#define QMP6988_CONFIG_REG_FILTER__POS        0
#define QMP6988_CONFIG_REG_FILTER__MSK        0x07
#define QMP6988_CONFIG_REG_FILTER__LEN        3


#define SUBTRACTOR 8388608

#endif  /*__MPU6050*/
