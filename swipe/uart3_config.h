#ifndef _UART3_CONFIG_
#define _UART3_CONFIG_

#include "M480.h"

#define UART3_BUF_SIZE	100	//接收/发送缓冲区大小

typedef struct UART3_DATA{
	uint16_t len;	//数据长度
	uint16_t size;	//缓冲大小
	uint8_t buf[UART3_BUF_SIZE];	//数据缓冲指针
}UART3_DATA;

typedef void (*UART3_RECV_CALLBACK)(UART3_DATA*);

void uart3_config(void);
uint8_t _uart3_send(UART3_DATA *pt_tx , UART3_DATA** pt_rx, UART3_RECV_CALLBACK callback);

#endif
