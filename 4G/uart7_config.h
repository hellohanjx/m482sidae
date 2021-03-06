#ifndef _UART7_CONFIG_
#define _UART7_CONFIG_

#include "M480.h"
#include "user_config.h"

typedef struct UART7_DATA{
	uint8_t buf[PACK_MAX_SIZE];	//数据缓冲
	uint16_t len;			//数据长度
	uint16_t size;		//缓冲大小
}UART7_DATA;

typedef uint8_t (*COMMUNICATION_RECV_CALLBACK)(UART7_DATA *rx);

void _uart7_config(uint32_t baud);
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUNICATION_RECV_CALLBACK callback);

#endif
