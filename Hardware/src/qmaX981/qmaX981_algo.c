/* qmaX981 motion sensor driver
 *
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#include "./qmaX981/qmaX981.h"
#include "./usart/bsp_usart.h"
#include "./i2c/bsp_i2c.h"
#include <stdbool.h>
#include <string.h>

#if defined(QMAX981_STEP_COUNTER)

//#define QMAX981_ALGO_DEBUG
#ifdef QMAX981_ALGO_DEBUG
#define QMAX981_TAG                  "[QMAX981_algo] "
#define QMAX981_ERR(fmt, args...)    console_write(QMAX981_TAG "%s %d : " fmt, __FUNCTION__, __LINE__, ##args)
#define QMAX981_FUN(f)               console_write(QMAX981_TAG "%s\n", __FUNCTION__)
//#define QMAX981_LOG(fmt, args...)    pr_debug(QMAX981_TAG "%s %d : " fmt, __FUNCTION__, __LINE__, ##args)
#define QMAX981_LOG(fmt, args...)    console_write(QMAX981_TAG "%s %d : " fmt, __FUNCTION__, __LINE__, ##args)
#else
#define QMAX981_ERR(fmt, args...)    do {} while (0)
#define QMAX981_FUN(f)               do {} while (0)
#define QMAX981_LOG(fmt, args...)    do {} while (0)
#endif

extern u8 qmaX981_writereg(u8 reg_add,u8 reg_dat);
extern u8 qmaX981_readreg(u8 reg_add,u8 *buf,u8 num);

typedef enum
{
	qst_false,
	qst_true
} qst_bool;

typedef struct
{
	int	x;
	int y;
	int z;
} qst_acc_data;


#if defined(QMAX981_CHECK_ABNORMAL_DATA)
typedef struct
{
	int last_data;
	int curr_data;
	int more_data[3];
}qmaX981_data_check;

#define QMA6981_DIFF(x, y)		((x)>=(y)?((x)-(y)):((x)+65535-(y)))
#define QMAX981_ABNORMAL_DIFF		30
static qmaX981_data_check g_qmaX981_data_c;
#endif

#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)
#define STEP_INT_START_VLUE		4
#define STEP_DUMMY_VLUE			8

#define STEP_END	qst_false
#define STEP_START	qst_true

struct qmaX981_stepcount{
	//int stepcounter_pre_start;
	int stepcounter_pre_end;  
	int stepcounter_next_start;
	int stepcounter_next_end;  
	int stepcounter_pre;
	int stepcounter_pre_fix;
	qst_bool stepcounter_statu;
	qst_bool stepcounter_start_int_update_diff;
	int back;
	int step_diff;
	qst_bool int_statu_flag;
};

static struct qmaX981_stepcount step_count_index;
void qmaX981_step_debounce_reset(void)
{
	memset(&step_count_index, 0, sizeof(step_count_index));
}

int qmaX981_step_debounce_int_work(int data, unsigned char irq_level)
{
	int temp = 0;

	if(irq_level == 0)
	{	
		step_count_index.int_statu_flag = qst_false;
		step_count_index.stepcounter_next_end = data;
		step_count_index.stepcounter_statu = STEP_END;
		QMAX981_LOG("step_int stepcounter_next_end = %d stepcounter_next_start = %d\n", step_count_index.stepcounter_next_end,step_count_index.stepcounter_next_start);
		if(step_count_index.stepcounter_next_end < step_count_index.stepcounter_next_start)
		{
			temp = step_count_index.stepcounter_next_end - step_count_index.stepcounter_next_start+65536;
		}
		else
		{
			temp = step_count_index.stepcounter_next_end - step_count_index.stepcounter_next_start;
		}
		QMAX981_LOG("step_int_end  temp =%d\n" ,temp);
		if (temp < STEP_DUMMY_VLUE)
		{
			step_count_index.step_diff += (temp+STEP_INT_START_VLUE);
			// add by yangzhiqiang for step_diff
			if(step_count_index.step_diff > data)
			{
				step_count_index.step_diff = data;
			}
			// yangzhiqiang for step_diff
			step_count_index.stepcounter_pre_end = step_count_index.stepcounter_next_end;
			
		}
		else
		{
			step_count_index.stepcounter_pre_end = step_count_index.stepcounter_next_end;
		}		
		//irq_set_irq_type(g_QMA6981_ptr->irq,IRQF_TRIGGER_RISING);
		QMAX981_LOG("step_int_end\n" );
	}
	else
	{
		step_count_index.int_statu_flag = qst_true;
		step_count_index.stepcounter_next_start= data;
		step_count_index.stepcounter_statu = STEP_START;
		QMAX981_LOG("step_int stepcounter_next_start = %d stepcounter_pre_end = %d\n", step_count_index.stepcounter_next_start,step_count_index.stepcounter_pre_end);
		if (step_count_index.stepcounter_next_start < step_count_index.stepcounter_pre_end)
		{
			temp = step_count_index.stepcounter_next_start - step_count_index.stepcounter_pre_end+65536;
		}
		else
		{
			temp = step_count_index.stepcounter_next_start - step_count_index.stepcounter_pre_end;
		}
		QMAX981_LOG("step_int_start temp =%d\n" ,temp);
		if (temp >STEP_INT_START_VLUE)
		{
			step_count_index.step_diff += (temp - STEP_INT_START_VLUE);
		}
		//res = request_irq(g_QMA6981_ptr->irq, QMA6981_eint_handler, IRQF_TRIGGER_FALLING, "gsensor-eint", NULL);
		//irq_set_irq_type(g_QMA6981_ptr->irq,IRQF_TRIGGER_FALLING);
		QMAX981_LOG("step_int_start\n" );
	}

	return step_count_index.int_statu_flag;
}


int qmaX981_step_debounce_read_data(int result)
{
	int tempp = 0;
	int data = 0;

	if (result < step_count_index.stepcounter_pre)
	{
		step_count_index.back++;
		data = result- step_count_index.stepcounter_pre + 65536;
		step_count_index.stepcounter_pre = result;
	}
	else
	{
		//nothing
		step_count_index.stepcounter_pre = result;
		data = result;
	}

	if (step_count_index.stepcounter_statu == STEP_START)
	{
	/*
		if (data >= step_count_index.stepcounter_pre_end)
		{
			tempp = data - step_count_index.stepcounter_pre_end;
		}
		else
		{
			tempp = data - step_count_index.stepcounter_pre_end +65536;
		}
	*/
		if (data >= step_count_index.stepcounter_next_start)
		{
			tempp = data - step_count_index.stepcounter_next_start + 4;
		}
		else
		{
			tempp = data - step_count_index.stepcounter_next_start +65540;
		}

		QMAX981_LOG("ReadStepCounter_running data= %d,stepcounter_next_start = %d,tempp = %d,stepcounter_pre_end =%d \n",data,step_count_index.stepcounter_next_start,tempp,step_count_index.stepcounter_pre_end);
		
		if (tempp < (STEP_INT_START_VLUE+STEP_DUMMY_VLUE))
		{
			data = step_count_index.stepcounter_pre_fix;
			QMAX981_LOG("ReadStepCounter_running stepcounter_pre_fix = %d\n",step_count_index.stepcounter_pre_fix);
		}
		else
		{
			if (step_count_index.step_diff >data)
			{
				step_count_index.step_diff = 0;
			}
			else
			{
				 data = data -  step_count_index.step_diff;
				 step_count_index.stepcounter_pre_fix = data;
				 QMAX981_LOG("ReadStepCounter_running stepcounter_pre_fix = %d\n",step_count_index.stepcounter_pre_fix);
			}
		}
	
	}
	else 
	{
		// add by yangzhiqiang for step_diff
		if(step_count_index.step_diff > data)
		{
			step_count_index.step_diff = data;
		}
		// yangzhiqiang for step_diff		
#if 1//defined(QMA6981_STEP_TO_ZERO)
		step_count_index.stepcounter_pre_end = data;
#endif
		data  = data -  step_count_index.step_diff;
		 step_count_index.stepcounter_pre_fix = data;
		 QMAX981_LOG("ReadStepCounter_end stepcounter_pre_fix = %d\n",step_count_index.stepcounter_pre_fix);
	}
	//mutex_unlock(&qma6981_mutex); // removed by yangzhiqiang
	QMAX981_LOG("ReadStepCounter=%d, step_diff= %d\n",data,step_count_index.step_diff );

	return data;
}
#endif


#if defined(QMAX981_CHECK_ABNORMAL_DATA)
int qmaX981_check_abnormal_data(int data_in, int *data_out)
{
	int ret = 0;
	int diff;
	unsigned char data[2];

	g_qmaX981_data_c.curr_data = data_in;
	diff = QMA6981_DIFF(g_qmaX981_data_c.curr_data, g_qmaX981_data_c.last_data);
	if(diff > QMAX981_ABNORMAL_DIFF)
	{
		// read data 1
		ret = qmaX981_readreg(QMAX981_STEP_CNT_L, data, 2);
		if(ret == 0)
			g_qmaX981_data_c.more_data[0] = -1;
		else
			g_qmaX981_data_c.more_data[0] = ((data[1]<<8) |( data[0]));
		
		// read data 2
		ret = qmaX981_readreg(QMAX981_STEP_CNT_L, data, 2);
		if(ret == 0)
			g_qmaX981_data_c.more_data[1] = -1;
		else
			g_qmaX981_data_c.more_data[1] = ((data[1]<<8) |( data[0]));


		// read data 3
		ret = qmaX981_readreg(QMAX981_STEP_CNT_L, data, 2);
		if(ret == 0)
			g_qmaX981_data_c.more_data[2] = -1;
		else
			g_qmaX981_data_c.more_data[2] = ((data[1]<<8) |( data[0]));


		if((g_qmaX981_data_c.more_data[0]<0)||(g_qmaX981_data_c.more_data[1]<0)||(g_qmaX981_data_c.more_data[2]<0))
		{
			return -1;
		}
		
		//if((QMA6981_ABS(g_qmaX981_data_c.more_data[0]-g_qmaX981_data_c.curr_data) > 1)
		//	||(QMA6981_ABS(g_qmaX981_data_c.more_data[1]-g_qmaX981_data_c.curr_data) > 1)
		//	||(QMA6981_ABS(g_qmaX981_data_c.more_data[2]-g_qmaX981_data_c.curr_data) > 1)
		//	)
		if((g_qmaX981_data_c.more_data[0]==g_qmaX981_data_c.more_data[1])
			||(g_qmaX981_data_c.more_data[1]==g_qmaX981_data_c.more_data[2]))
		{		
			*data_out = g_qmaX981_data_c.more_data[0];
			g_qmaX981_data_c.last_data = g_qmaX981_data_c.more_data[0];
		}
		else
		{		
			return -1;
		}
	}
	else
	{
		g_qmaX981_data_c.last_data = g_qmaX981_data_c.curr_data;
	}

	return 0;
}
#endif

#endif

#if defined(QMAX981_AUTO_CALI)
#define QMAX981_STATIC_THRES	400		// 40mg
typedef struct
{
	int 			flat_count;
	int 			count_max;
	qst_acc_data	flat;
	//int 			acc_count;
	qst_acc_data	acc;
	qst_acc_data	cali;
}qmaX981_auto_cali;

static qmaX981_auto_cali g_qmaX981_cali;

int qmaX981_check_flat_auto_cali(int acc_data[3], int delay)
{
	int x_diff, y_diff;
	int result = 0;
	
	x_diff = QMAX981_ABS(acc_data[0]-g_qmaX981_cali.flat.x);
	y_diff = QMAX981_ABS(acc_data[1]-g_qmaX981_cali.flat.y);
	g_qmaX981_cali.flat.x = acc_data[0];
	g_qmaX981_cali.flat.y = acc_data[1];
	g_qmaX981_cali.flat.z = acc_data[2];
	if(delay > 0)
	{
		g_qmaX981_cali.count_max = (3000/delay)+1;
		if(g_qmaX981_cali.count_max < 10)
			g_qmaX981_cali.count_max = 10;
	}
	else
	{
		g_qmaX981_cali.count_max = 10;
	}

	if((x_diff<QMAX981_STATIC_THRES)&&(y_diff<QMAX981_STATIC_THRES)
		&&(QMAX981_ABS(acc_data[0])<1500)&&(QMAX981_ABS(acc_data[1])<1500))
	{
		if(g_qmaX981_cali.flat_count == 0)
		{
			g_qmaX981_cali.acc.x = 0;
			g_qmaX981_cali.acc.y = 0;
			g_qmaX981_cali.acc.z = 0;
		}
		g_qmaX981_cali.acc.x += acc_data[0];
		g_qmaX981_cali.acc.y += acc_data[1];
		g_qmaX981_cali.acc.z += acc_data[2];
		
		g_qmaX981_cali.flat_count++;
	}
	else
	{
		g_qmaX981_cali.flat_count = 0;
	}

	if(g_qmaX981_cali.flat_count >= g_qmaX981_cali.count_max)
	{
		g_qmaX981_cali.cali.x += 0-(g_qmaX981_cali.acc.x/g_qmaX981_cali.count_max);
		g_qmaX981_cali.cali.y += 0-(g_qmaX981_cali.acc.y/g_qmaX981_cali.count_max);
		g_qmaX981_cali.cali.z += 9807-(g_qmaX981_cali.acc.z/g_qmaX981_cali.count_max);

		result = 1;
	}

	return result;
}


void qmaX981_auto_cali_update(int *cali_data)
{
	cali_data[0] = g_qmaX981_cali.cali.x;
	cali_data[1] = g_qmaX981_cali.cali.y;
	cali_data[2] = g_qmaX981_cali.cali.z;
}


void qmaX981_auto_cali_reset(void)
{
	memset(&g_qmaX981_cali, 0, sizeof(g_qmaX981_cali));
}

#endif
