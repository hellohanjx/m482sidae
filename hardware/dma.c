#include "dma.h"

void uart_disable_TXPDMAEN(uint8_t id)
{
	switch(id)
	{
		case 0:
			UART_DISABLE_INT(UART1, UART_INTEN_TXPDMAEN_Msk);//禁能uart-dma发送
		break;
		
		case 1:
			UART_DISABLE_INT(UART2, UART_INTEN_TXPDMAEN_Msk);//禁能uart-dma发送
		break;
		
		case 2:
			UART_DISABLE_INT(UART3, UART_INTEN_TXPDMAEN_Msk);//禁能uart-dma发送
		break;
		
		case 3:
			UART_DISABLE_INT(UART4, UART_INTEN_TXPDMAEN_Msk);//禁能uart-dma发送
		break;
		
		case 4:
			UART_DISABLE_INT(UART5, UART_INTEN_TXPDMAEN_Msk);//禁能uart-dma发送
		break;
	}
}



/*
@功能：pdma中断
*/
void PDMA_IRQHandler(void)
{
	uint32_t status = PDMA_GET_INT_STATUS(PDMA);//读中断状态
	uint16_t pdma_tdsts = PDMA_GET_TD_STS(PDMA);//读发送结果，只有低16bit有效
	uint8_t i;
	
	if( (status & 0x02) == 0x02 )//传输完成
	{
		for(i = 0; i < 16; i++)
		{
			if(pdma_tdsts & 0x1)//已完成
			{
				PDMA->TDSTS |= 1<<i;//清除对应的传输完成标志
			}
			pdma_tdsts >>= 1;
			uart_disable_TXPDMAEN(i);
		}
//		UART_WRITE(UART1, '1');
	}
	
	if( (status & 0x200) == 0x200)//传输超时,只有通道1/2有此功能
	{
		UART_WRITE(UART1, '2');
	}
}
