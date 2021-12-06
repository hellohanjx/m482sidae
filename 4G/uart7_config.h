#ifndef _UART7_CONFIG_
#define _UART7_CONFIG_

#include "M480.h"
#include "user_config.h"

typedef struct UART7_DATA{
	uint16_t len;			//数据长度
	uint16_t size;		//缓冲大小
	uint8_t buf[PACK_MAX_SIZE];	//数据缓冲
}UART7_DATA;

typedef uint8_t (*COMMUCATION_RECV_CALLBACK)(UART7_DATA *rx);

void uart7_config(void);
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUCATION_RECV_CALLBACK callback);

#endif
