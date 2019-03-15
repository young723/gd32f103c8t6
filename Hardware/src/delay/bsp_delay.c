

#include "bsp_delay.h"

volatile unsigned int bsp_delay = 0;

void bsp_delay_ms(unsigned int num)
{
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
}


void delay_ms(unsigned int num)
{
	bsp_delay = num;
    while(0U != bsp_delay)
	{
	}
}

