#include "uart0_log.h"
#include "uart.h"
#include "msg.h"

static volatile UART0_DATA rx;	//����


/*
���ܣ�����uart0����Ϊ��־�����
*/
void _uart0_config(uint32_t baud)
{
	CLK_EnableModuleClock(UART0_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	/* Set GPB multi-function pins for UART0 RXD and TXD */
	SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);//���ö๦������PF3,PF2
	SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF3MFP_UART0_TXD | SYS_GPF_MFPL_PF2MFP_UART0_RXD);//ӳ�䵽����0��RX��TX
   
	UART_Open(UART0, baud);
	
	UART0->FIFO &= ~UART_FIFO_RFITL_Msk;//����FIFO
	UART0->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO�г���14�ֽڲŻᴥ���ж�
	
	NVIC_SetPriority(UART0_IRQn, 8);
	NVIC_EnableIRQ(UART0_IRQn);
	UART_SetTimeoutCnt(UART0, 50);//FIFO��ʱʱ�䣺0~255
	UART_ENABLE_INT(UART0, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO��ʱ�ж� | �����ж�
}



/*
@���ܣ�UART0�жϴ�����
*/
void UART0_IRQHandler(void)
{
	uint32_t status = UART0->INTSTS;
	
	if( UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAIF_Msk) )//�����ж�
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART0) && n++ < 13)//������Ҫ����һ�����ݣ����򲻻���볬ʱ�ж�
		{
			rx.len %= UART0_BUF_SIZE;
			rx.buf[rx.len++] = UART0->DAT;
		}
	}
	
//	if( UART_GET_INT_FLAG(UART0, UART_INTSTS_RXTOINT_Msk) )//���ճ�ʱ�жϣ��Ƚ�������
//	{
//		UART_ClearIntFlag(UART0, UART_INTSTS_RXTOINT_Msk);
//	}
	if( UART_GET_INT_FLAG(UART0, UART_INTSTS_RXTOIF_Msk) )//���ճ�ʱ�жϣ��ٽ�������
	{
		while(!UART_GET_RX_EMPTY(UART0))
		{
			rx.len %= UART0_BUF_SIZE;
			rx.buf[rx.len++] = UART0->DAT;
		}
		log_queue_send(rx);//�������
		rx.len = 0;
	}

}
