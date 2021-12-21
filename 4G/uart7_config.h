#ifndef _UART7_CONFIG_
#define _UART7_CONFIG_

#include "M480.h"
#include "user_config.h"

typedef struct UART7_DATA{
	uint8_t buf[PACK_MAX_SIZE];	//���ݻ���
	uint16_t len;			//���ݳ���
	uint16_t size;		//�����С
}UART7_DATA;

typedef uint8_t (*COMMUCATION_RECV_CALLBACK)(UART7_DATA *rx);

void uart7_config(void);
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUCATION_RECV_CALLBACK callback);

#endif
