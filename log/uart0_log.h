#ifndef _UART0_LOG_
#define _UART0_LOG_

#include "M480.h"	//@注意：这个包含需要在"gpio.h"之前

#define UART0_BUF_SIZE	100	//接收/发送缓冲区大小

typedef struct UART0_DATA{
	uint16_t len;	//数据长度
	uint8_t buf[UART0_BUF_SIZE];	//数据缓冲指针
}UART0_DATA;


void _uart0_config(uint32_t baud);


#endif
