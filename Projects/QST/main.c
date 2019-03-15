#include "gd32f10x.h"
#include "systick.h"
#include <stdio.h>

#include "gd32f10x_eval.h"

#if defined(USE_SW_I2C)
#include "gd32_sw_i2c.h"
#else
#include "gd32_i2c.h"
#endif
#include "qmaX981.h"
#include "fis210x.h"
#include "bsp_timer.h"
#include "bsp_usart.h"
#include "bsp_delay.h"

typedef enum 
{
	QST_DEVICE_NONE = 0x00,
	QST_DEVICE_QMAX981 = 0x01,
	QST_DEVICE_QMCX983 = 0x02,
	QST_DEVICE_QMP6988 = 0x04,
	QST_DEVICE_FIS210X = 0x08,

	QST_DEVICE_MAX
} qst_device_id;

static volatile uint32_t g_run_count = 0;
static qst_device_id g_device_id = QST_DEVICE_NONE;

#if defined(QMCX983_SUPPORT)
extern int qmcX983_init(void);
extern uint8_t qmcX983_read_xyz(void);
#endif
#if defined(QMCX983_DREAME)
extern void QmcX983_Read_Mag(void);
extern void QmcInit(void);
#endif

#if defined(QMAX981_SUPPORT)
extern int32_t qmaX981_init(void);
extern int32_t qmaX981_read_acc(int32_t *accData);
#endif

#if defined(QMP6988_SUPPORT)
extern uint8_t qmp6988_init(void);
extern void qmp6988_calc_pressure(void);
#endif

void gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_10);
	gpio_bit_set(GPIOB,GPIO_PIN_10);
}

void bsp_led_set(uint8_t flag)
{
	if(flag)
		gpio_bit_reset(GPIOB,GPIO_PIN_10);
	else		
		gpio_bit_set(GPIOB,GPIO_PIN_10);
}

void bsp_interrupt_config(void)
{
	/* enable the key wakeup clock */
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_AF);

	/* configure button pin as input */
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);

	/* enable and set key wakeup EXTI interrupt to the lowest priority */
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 2U);

	/* connect EXTI line to GPIO pin */
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_14);

	/* configure EXTI line */
	exti_init(EXTI_14, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_interrupt_flag_clear(EXTI_14);
}


void mcu_reset_counter(void)
{
	g_run_count = 0;
}

void qst_sensor_timer_ckb(int timeId)
{
#if defined(QMAX981_SUPPORT)
	if(g_device_id & QST_DEVICE_QMAX981)
	{
		int32_t acc_data[3];
		qmaX981_read_acc(acc_data);
		console_write("acc data [%d, %d, %d]\n", acc_data[0], acc_data[1], acc_data[2]);
	}
#endif
#if defined(QMCX983_SUPPORT)
	if(g_device_id & QST_DEVICE_QMCX983)
	{
		qmcX983_read_xyz();
		//QmcX983_Read_Mag();
	}
#endif
#if defined(QMCX983_DREAME)
	if(g_device_id & QST_DEVICE_QMCX983)
	{
		QmcX983_Read_Mag();
	}
#endif
#if defined(QMP6988_SUPPORT)
	if(g_device_id & QST_DEVICE_QMP6988)
	{
		qmp6988_calc_pressure();
	}
#endif
}

void mcu_entry_sleep(void)
{
	console_write("mcu_entry_sleep \n");
	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
	//pmu_to_standbymode(WFI_CMD);
}

void mcu_exit_sleep(void)
{
	SystemInit();
	systick_config();
	gpio_config();	
	gd_eval_com_init(EVAL_COM1);

	console_write("mcu_exit_sleep \n");
	//bsp_start_timer(BSP_TIMER_2, 200, qst_sensor_timer_ckb);
}


int main(void)
{
	float acc_data[3];
	float gyr_data[3];
	float d_q[4];
	float rpy[4];

	systick_config();
	gpio_config();	
	gd_eval_com_init(EVAL_COM1);
#if defined(USE_SW_I2C)
	i2c_GPIO_Config();
	console_write("use sw i2c \n");
#else
	i2c_config();
	console_write("use hw i2c \n");
#endif
	bsp_timer_hw_init();
	//bsp_interrupt_config();
	bsp_delay_ms(800); 
#if defined(QMCX983_SUPPORT)
	if(qmcX983_init())
	{
		g_device_id |= QST_DEVICE_QMCX983;
	}
#endif
#if defined(QMAX981_SUPPORT)
	if(qmaX981_init())
	{
		g_device_id |= QST_DEVICE_QMAX981;
	}
#endif
#if defined(QMCX983_DREAME)
	QmcInit();
	g_device_id |= QST_DEVICE_QMCX983;
#endif
#if defined(QMP6988_SUPPORT)
	if(qmp6988_init())
	{
		g_device_id |= QST_DEVICE_QMP6988;
	}
#endif
	if(g_device_id)
	{
		bsp_start_timer(BSP_TIMER_2, 20, qst_sensor_timer_ckb);
		while(1)
		{
			bsp_timer_proc();
			g_run_count++;
			if(g_run_count > 2000000)
			{
				mcu_entry_sleep();
				mcu_exit_sleep();
				g_run_count = 0;
			}
		}
	}
	else
	{
		while(1)
		{
			console_write("No sensor! \n");
			bsp_delay_ms(300);
		}
	}
}


#if 0
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));

    return ch;
}
#endif

