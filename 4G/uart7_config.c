#include "uart7_config.h"
#include "uart.h"
#include "scuart.h"


static volatile UART7_DATA rx;	//��������
static volatile UART7_DATA *tx;	//����ָ��
static COMMUCATION_RECV_CALLBACK callBack_recv = 0;//���մ���ص�����

/*
@���ܣ�uart7�������ã�ռ����SC1�ӿ�
*/
void uart7_config(void)
{ 
	CLK_EnableModuleClock(SC1_MODULE);//ʹ��SC1ʱ��
	CLK_SetModuleClock(SC1_MODULE, CLK_CLKSEL3_SC1SEL_HIRC, CLK_CLKDIV1_SC1(1));//����ʱ��Դ���Ƶ
	
  SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk);
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC0MFP_SC1_CLK | SYS_GPC_MFPL_PC1MFP_SC1_DAT);
	
	SCUART_Open(SC1, 115200);

	SCUART_ENABLE_INT(SC1, SC_INTEN_RDAIEN_Msk);
	NVIC_EnableIRQ(SC1_IRQn);
}

/*
@���ܣ�SC1�жϴ�����
*/
void SC1_IRQHandler(void)
{
    while(!SCUART_GET_RX_EMPTY(SC1))
        UART_WRITE(UART0, SCUART_READ(SC1));

    // RDA is the only interrupt enabled in this sample, this status bit
    // automatically cleared after Rx FIFO empty. So no need to clear interrupt
    // status here.

    return;
}


/*
@���ܣ����ڷ�������
@������	**pt_rx,���������ݵ�ָ��ĵ�ַ
				*pt_tx,����������ָ��
				len, �������ݳ���
				recvCallBacl�����ջص�����
@����ֵ���������
*/
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUCATION_RECV_CALLBACK callback)  
{
	uint32_t i;
	if(pt_txbuf == 0 || callback == 0)
		return FALSE;
	
	*pt_rx = (UART7_DATA*)&rx;//���ջ���ָ���ṩ��ָ�룬�ṩ���ⲿʹ��
	for(i = 0; i < PACK_MAX_SIZE; i++) rx.buf[i] = 0;//�����������
	callBack_recv = callback;	//�ص�������ʼ��

	
	return TRUE;
}
