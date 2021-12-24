#ifndef _UART_CONFIG_H_
#define _UART_CONFIG_H_

#include "M480.h"


#define UART_BUF_SIZE	100	//接收/发送缓冲区大小

typedef struct UART_DATA{
	uint16_t len;	//数据长度
	uint16_t size;	//缓冲大小
	uint8_t buf[UART_BUF_SIZE];	//数据缓冲指针
	uint8_t id;//数据来源，刷卡器0~5
}UART_DATA;

/*
@说明：@@串口接收回调函数
*/
typedef void (*UART_RECV_CALLBACK)(UART_DATA*);



void _uart1_config(uint32_t baud);
uint8_t _uart1_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);

void _uart2_config(uint32_t baud);
uint8_t _uart2_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);

void _uart3_config(uint32_t baud);
uint8_t _uart3_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);

void _uart4_config(uint32_t baud);
uint8_t _uart4_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);

void _uart5_config(uint32_t baud);
uint8_t _uart5_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);

void _uart6_config(uint32_t baud);
uint8_t _uart6_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);


#endif
