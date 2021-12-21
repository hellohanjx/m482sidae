#include "dma.h"

void uart_disable_TXPDMAEN(uint8_t id)
{
	switch(id)
	{
		case 0:
			UART_DISABLE_INT(UART1, UART_INTEN_TXPDMAEN_Msk);//����uart-dma����
		break;
		
		case 1:
			UART_DISABLE_INT(UART2, UART_INTEN_TXPDMAEN_Msk);//����uart-dma����
		break;
		
		case 2:
			UART_DISABLE_INT(UART3, UART_INTEN_TXPDMAEN_Msk);//����uart-dma����
		break;
		
		case 3:
			UART_DISABLE_INT(UART4, UART_INTEN_TXPDMAEN_Msk);//����uart-dma����
		break;
		
		case 4:
			UART_DISABLE_INT(UART5, UART_INTEN_TXPDMAEN_Msk);//����uart-dma����
		break;
	}
}



/*
@���ܣ�pdma�ж�
*/
void PDMA_IRQHandler(void)
{
	uint32_t status = PDMA_GET_INT_STATUS(PDMA);//���ж�״̬
	uint16_t pdma_tdsts = PDMA_GET_TD_STS(PDMA);//�����ͽ����ֻ�е�16bit��Ч
	uint8_t i;
	
	if( (status & 0x02) == 0x02 )//�������
	{
		for(i = 0; i < 16; i++)
		{
			if(pdma_tdsts & 0x1)//�����
			{
				PDMA->TDSTS |= 1<<i;//�����Ӧ�Ĵ�����ɱ�־
			}
			pdma_tdsts >>= 1;
			uart_disable_TXPDMAEN(i);
		}
//		UART_WRITE(UART1, '1');
	}
	
	if( (status & 0x200) == 0x200)//���䳬ʱ,ֻ��ͨ��1/2�д˹���
	{
		UART_WRITE(UART1, '2');
	}
}
