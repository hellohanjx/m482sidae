#ifndef _UART0_LOG_
#define _UART0_LOG_

#include "M480.h"	//@ע�⣺���������Ҫ��"gpio.h"֮ǰ

#define UART0_BUF_SIZE	100	//����/���ͻ�������С

typedef struct UART0_DATA{
	uint16_t len;	//���ݳ���
	uint8_t buf[UART0_BUF_SIZE];	//���ݻ���ָ��
}UART0_DATA;


void _uart0_config(uint32_t baud);


#endif
