#ifndef _UART4_CONFIG_
#define _UART4_CONFIG_

#include "M480.h"

#define UART4_BUF_SIZE	100	//接收/发送缓冲区大小

typedef struct UART4_DATA{
	uint16_t len;	//数据长度
	uint16_t size;	//缓冲大小
	uint8_t buf[UART4_BUF_SIZE];	//数据缓冲指针
}UART4_DATA;

typedef void (*UART4_RECV_CALLBACK)(UART4_DATA*);

void uart4_config(void);
uint8_t _uart4_send(UART4_DATA *pt_tx , UART4_DATA** pt_rx, UART4_RECV_CALLBACK callback);

#endif
