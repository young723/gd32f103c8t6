/*!
    \file  gd32f10x_it.c
    \brief main interrupt service routines
*/

/*
    Copyright (C) 2017 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F10x
    2017-06-20, V2.0.0, firmware for GD32F10x
*/

#include "gd32f10x_it.h"
#include "usbd_int.h"

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles USBD interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  USBD_LP_CAN0_RX0_IRQHandler (void)
{
    //usbd_isr();
}

extern void bsp_timer_update_all(void);
extern volatile unsigned int bsp_delay;

void SysTick_Handler(void)
{
	if(bsp_delay>0)
    	bsp_delay--;
	bsp_timer_update_all();
}

#ifdef USBD_LOWPWR_MODE_ENABLE

/*!
    \brief      this function handles USBD wakeup interrupt request.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  USBD_WKUP_IRQHandler (void)
{
    exti_interrupt_flag_clear(EXTI_18);
}


#endif /* USBD_LOWPWR_MODE_ENABLE */

extern void qmaX981_15_10_IRQHandler(void);

void EXTI0_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_0))
	{
        exti_interrupt_flag_clear(EXTI_0);
    }
}

void EXTI10_15_IRQHandler(void)
{
#if 0
    if(RESET != exti_interrupt_flag_get(EXTI_14))
	{
		//printf("EXTI10_15_IRQHandler \n");
		qst_int_flag = 1;
        exti_interrupt_flag_clear(EXTI_14);
    }
#endif
	qmaX981_15_10_IRQHandler();
}


