#ifndef _DMA_H_
#define _DMA_H_

#include "M480.h"

typedef void (*ID_FUN)(UART_T*);//º¯ÊýÖ¸Õë

typedef struct{
	UART_T* id;
	ID_FUN id_fun;	
}DMA_UART_TABLE;

#endif
