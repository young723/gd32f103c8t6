/**
  **********************************  STM8S  ***********************************
  * @文件名     ： bsp_timer.c
  * @作者       ： strongerHuang
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2017年04月10日
  * @摘要       ： TIM定时器源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2017-04-10 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "bsp_timer.h"

#define BSP_IRQ_TIME	1		//ms


typedef struct
{
	//unsigned short millTime;
	unsigned char  used;
	unsigned short irqCount;
	unsigned short irqCountMax;
	void (*callback)(int timeId);
} bsp_timer_t;

static bsp_timer_t bsp_timer_array[BSP_TIMER_MAX];

void bsp_timer_array_init(void)
{
	int i;

	for(i=0; i<BSP_TIMER_MAX; i++)
	{
		bsp_timer_array[i].used = 0;
		bsp_timer_array[i].irqCount = 0;
		bsp_timer_array[i].irqCountMax = 0;
		bsp_timer_array[i].callback = 0;
	}
}

void bsp_timer_update(int timerId)
{
	//if(timerId < BSP_TIMER_MAX)
	//{
		bsp_timer_array[timerId].irqCount++;
	//}
}

void bsp_timer_update_all(void)
{
	int timerId;

	while(timerId < BSP_TIMER_MAX)
	{
		if(bsp_timer_array[timerId].used)
			bsp_timer_array[timerId].irqCount++;

		timerId++;
	}
}


void bsp_start_timer(int timerId, unsigned short millTime, void (*callback)(int id))
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	bsp_timer_array[timerId].irqCount = 0;
	if(millTime%BSP_IRQ_TIME)
		bsp_timer_array[timerId].irqCountMax = ((unsigned short)(millTime/BSP_IRQ_TIME))+1;
	else
		bsp_timer_array[timerId].irqCountMax = ((unsigned short)(millTime/BSP_IRQ_TIME));
	
	bsp_timer_array[timerId].callback = callback;
	bsp_timer_array[timerId].used = 1;
	bsp_timer_hw_enable(timerId, 1);
}

void bsp_stop_timer(int timerId)
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	bsp_timer_array[timerId].used = 0;
	bsp_timer_array[timerId].callback = 0;
	bsp_timer_hw_enable(timerId, 0);
}


void bsp_timer_proc(void)
{
	int i;

	for(i=0; i<BSP_TIMER_MAX; i++)
	{
		if(bsp_timer_array[i].used)
		{
			if(bsp_timer_array[i].irqCount >= bsp_timer_array[i].irqCountMax)
			{
				bsp_timer_array[i].irqCount = 0;
				if(bsp_timer_array[i].callback)
				{
					//console_write("timer 0 callback \n");
					bsp_timer_array[i].callback(i);
				}
			}
		}
	}
}

void bsp_timer_hw_enable(int timerId, unsigned char flag)
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	if(flag)
	{
		if(timerId == BSP_TIMER_0)
		{
		}
		else if(timerId == BSP_TIMER_1)
		{
		}
		else if(timerId == BSP_TIMER_2)
		{
		}
		else if(timerId == BSP_TIMER_3)
		{
		}
	}
	else
	{
		if(timerId == BSP_TIMER_0)
		{
		}
		else if(timerId == BSP_TIMER_1)
		{
		}
		else if(timerId == BSP_TIMER_2)
		{
		}
		else if(timerId == BSP_TIMER_3)
		{
		}
	}
}



/************************************************
函数名称 ： bsp_timer_hw_init
功    能 ： 定时器初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void bsp_timer_hw_init(void)
{
	bsp_timer_array_init();
}


/**** Copyright (C)2017 strongerHuang. All Rights Reserved **** END OF FILE ****/
