/**
  **********************************  STM8S  ***********************************
  * @文件名     ： bsp_timer.h
  * @作者       ： strongerHuang
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2017年04月10日
  * @摘要       ： TIM定时器头文件
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _BSP_TIMER_H
#define _BSP_TIMER_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "gd32f10x.h"


/* 宏定义 --------------------------------------------------------------------*/

/* 全局变量 ------------------------------------------------------------------*/
enum
{
	BSP_TIMER_0,
	BSP_TIMER_1,
	BSP_TIMER_2,
	BSP_TIMER_3,

	BSP_TIMER_MAX
};


/* 函数申明 ------------------------------------------------------------------*/
void bsp_timer_update(int timerId);
void bsp_timer_update_all(void);

void bsp_start_timer(int timerId, unsigned short millTime, void (*callback)(int id));
void bsp_stop_timer(int timerId);

void bsp_timer_proc(void);
void bsp_timer_hw_enable(int timerId, unsigned char flag);
void bsp_timer_hw_init(void);

#endif /* _BSP_TIMER_H */

/**** Copyright (C)2017 strongerHuang. All Rights Reserved **** END OF FILE ****/

