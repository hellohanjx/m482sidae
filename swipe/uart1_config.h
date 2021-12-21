#ifndef _UART1_CONFIG_
#define _UART1_CONFIG_

#include "M480.h"

#define UART1_BUF_SIZE	100	//����/���ͻ�������С

typedef struct UART1_DATA{
	uint16_t len;	//���ݳ���
	uint16_t size;	//�����С
	uint8_t buf[UART1_BUF_SIZE];	//���ݻ���ָ��
}UART1_DATA;

typedef void (*UART1_RECV_CALLBACK)(UART1_DATA*);

void uart1_config(void);
uint8_t _uart1_send(UART1_DATA *pt_tx , UART1_DATA** pt_rx, UART1_RECV_CALLBACK callback);

#endif
