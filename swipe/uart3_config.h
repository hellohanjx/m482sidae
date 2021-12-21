#ifndef _UART3_CONFIG_
#define _UART3_CONFIG_

#include "M480.h"

#define UART3_BUF_SIZE	100	//����/���ͻ�������С

typedef struct UART3_DATA{
	uint16_t len;	//���ݳ���
	uint16_t size;	//�����С
	uint8_t buf[UART3_BUF_SIZE];	//���ݻ���ָ��
}UART3_DATA;

typedef void (*UART3_RECV_CALLBACK)(UART3_DATA*);

void uart3_config(void);
uint8_t _uart3_send(UART3_DATA *pt_tx , UART3_DATA** pt_rx, UART3_RECV_CALLBACK callback);

#endif
