/**
  **********************************  STM8S  ***********************************
  * @�ļ���     �� bsp_timer.h
  * @����       �� strongerHuang
  * @��汾     �� V2.2.0
  * @�ļ��汾   �� V1.0.0
  * @����       �� 2017��04��10��
  * @ժҪ       �� TIM��ʱ��ͷ�ļ�
  ******************************************************************************/

/* �����ֹ�ݹ���� ----------------------------------------------------------*/
#ifndef _BSP_TIMER_H
#define _BSP_TIMER_H

/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "gd32f10x.h"


/* �궨�� --------------------------------------------------------------------*/

/* ȫ�ֱ��� ------------------------------------------------------------------*/
enum
{
	BSP_TIMER_0,
	BSP_TIMER_1,
	BSP_TIMER_2,
	BSP_TIMER_3,

	BSP_TIMER_MAX
};


/* �������� ------------------------------------------------------------------*/
void bsp_timer_update(int timerId);
void bsp_timer_update_all(void);

void bsp_start_timer(int timerId, unsigned short millTime, void (*callback)(int id));
void bsp_stop_timer(int timerId);

void bsp_timer_proc(void);
void bsp_timer_hw_enable(int timerId, unsigned char flag);
void bsp_timer_hw_init(void);

#endif /* _BSP_TIMER_H */

/**** Copyright (C)2017 strongerHuang. All Rights Reserved **** END OF FILE ****/

