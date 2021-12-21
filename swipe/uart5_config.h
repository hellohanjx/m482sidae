#ifndef _UART5_CONFIG_
#define _UART5_CONFIG_

#include "M480.h"

#define UART5_BUF_SIZE	100	//接收/发送缓冲区大小

typedef struct UART5_DATA{
	uint16_t len;	//数据长度
	uint16_t size;	//缓冲大小
	uint8_t buf[UART5_BUF_SIZE];	//数据缓冲指针
}UART5_DATA;

typedef void (*UART5_RECV_CALLBACK)(UART5_DATA*);

void uart5_config(void);
uint8_t _uart5_send(UART5_DATA *pt_tx , UART5_DATA** pt_rx, UART5_RECV_CALLBACK callback);

#endif
