#ifndef _UART2_CONFIG_
#define _UART2_CONFIG_

#include "M480.h"

#define UART2_BUF_SIZE	100	//����/���ͻ�������С

typedef struct UART2_DATA{
	uint16_t len;	//���ݳ���
	uint16_t size;	//�����С
	uint8_t buf[UART2_BUF_SIZE];	//���ݻ���ָ��
}UART2_DATA;

typedef void (*UART2_RECV_CALLBACK)(UART2_DATA*);

void uart2_config(void);
uint8_t _uart2_send(UART2_DATA *pt_tx , UART2_DATA** pt_rx, UART2_RECV_CALLBACK callback);

#endif
