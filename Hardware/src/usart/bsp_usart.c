/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   调试用的printf串口，重定向printf到串口
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火 F103-指南者 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 


#include "bsp_usart.h"

#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
	ch = ch;
}

///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}

#endif

int console_write(const char *format, ...)
{
	va_list arg;
	int done;

	va_start (arg, format);
	done = vfprintf(stdout, format, arg);
	va_end (arg);

	return done;
}

char console_readChar(void)
{
	char c;
	
	while(usart_flag_get(EVAL_COM1, USART_FLAG_RBNE) == RESET);
	c = usart_data_receive(EVAL_COM1);

	return c;
}

int console_readLine(char* const c, const size_t n)
{
	char ch = ' ';
	size_t i = 0;

	while (ch != '\r')
	{
		ch = console_readChar();
		c[i++] = ch;
	}
	c[i-1] = '\0';
	return i;
}

char console_kbhit(void)
{
	char c;

	c = usart_data_receive(EVAL_COM1);
	return c;
}

static void console_lowlevel_write(uint8_t const* buf, size_t bufSize)
{
	for (size_t i = 0; i < bufSize; ++i)
	{
		usart_data_transmit(EVAL_COM1, (uint8_t) buf[i]);
		/* 等待发送完毕 */
		while(usart_flag_get(EVAL_COM1, USART_FLAG_TBE) == RESET);
		if (buf[i] == '\n')
		{
			usart_data_transmit(EVAL_COM1, (uint8_t) '\r');
			/* 等待发送完毕 */
			while(usart_flag_get(EVAL_COM1, USART_FLAG_TBE) == RESET);	
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------
// IAR specific code to retarget output stream to serial port
//----------------------------------------------------------------------------
#if defined(__ICCARM__)
size_t __write(int handle, const unsigned char* buf, size_t bufSize)
{
	console_lowlevel_write((uint8_t const*)buf, bufSize);
	return bufSize;
}
#endif // IAR
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// GCC (newlib nano) specific code to retarget output stream to serial port 
//----------------------------------------------------------------------------
#if defined(__GNUC__)
_ssize_t _write (int file, const void *ptr, size_t bufSize)
{
	unsigned char const* buf = (unsigned char const*)ptr;
	console_lowlevel_write(buf, bufSize);
	return bufSize;
}
#endif // GCC
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Keil specific code to retarget output stream to serial port
//----------------------------------------------------------------------------
#if (defined(__arm__) && defined(__ARMCC_VERSION))

#include <rt_sys.h>

const char __stdin_name[] =  ":tt";
const char __stdout_name[] =  ":tt";
const char __stderr_name[] =  ":tt";

FILEHANDLE _sys_open(const char *name, int openmode) { return 1; }
int _sys_close(FILEHANDLE fh) { return 0; }
int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode) { return -1; }
int _sys_istty(FILEHANDLE fh) { return 0; }
int _sys_seek(FILEHANDLE fh, long pos) { return -1; }
long _sys_flen(FILEHANDLE fh) { return -1; }

int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{
	console_lowlevel_write((uint8_t const*)buf, (size_t)len);
	return 0;
}

void _ttywrch(int ch)
{
	uint8_t c = (uint8_t)ch;
	console_lowlevel_write(&c, 1);
}
#endif //Keil
//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif


///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));

    return ch;
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
	/* 等待串口输入数据 */
	while(usart_flag_get(EVAL_COM1, USART_FLAG_RBNE) == RESET);
	return (int)usart_data_receive(EVAL_COM1);
}


 /**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void USART_Config(void)
{
    
}



