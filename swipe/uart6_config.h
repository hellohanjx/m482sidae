#ifndef _UART6_CONFIG_
#define _UART6_CONFIG_

#include "M480.h"
#include "user_config.h"


#define UART6_BUF_SIZE	100	//接收/发送缓冲区大小

typedef struct UART6_DATA{
	uint16_t len;	//数据长度
	uint16_t size;	//缓冲大小
	uint8_t buf[UART6_BUF_SIZE];	//数据缓冲指针
}UART6_DATA;

typedef void (*UART6_RECV_CALLBACK)(UART6_DATA*);

void uart6_config(void);
uint8_t _uart6_send(UART6_DATA *pt_tx , UART6_DATA** pt_rx, UART6_RECV_CALLBACK callback); 


#endif
