#include "uart7_config.h"
#include "uart.h"
#include "scuart.h"


static volatile UART7_DATA rx;	//��������
static volatile UART7_DATA tx;	//��������
static COMMUNICATION_RECV_CALLBACK callBack_recv;//���մ���ص�����

/*
@���ܣ�uart7�������ã�ռ����SC1�ӿ�
*/
void _uart7_config(uint32_t baud)
{ 
	CLK_EnableModuleClock(SC1_MODULE);//ʹ��SC1ʱ��
	CLK_SetModuleClock(SC1_MODULE, CLK_CLKSEL3_SC1SEL_HIRC, CLK_CLKDIV1_SC1(1));//����ʱ��Դ���Ƶ
	
  SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk);
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC0MFP_SC1_CLK | SYS_GPC_MFPL_PC1MFP_SC1_DAT);
	
	SCUART_Open(SC1, baud);
	SC1->CTL |= (0x2 << 6);//(SC_CTL_RXTRGLV_Msk);//RX-FIFO ��3�ֽڴ���
	SCUART_SetTimeoutCnt(SC1, 20);//FIFO ��ʱʱ��

	NVIC_SetPriority(SC1_IRQn, 6);
	NVIC_EnableIRQ(SC1_IRQn);
	SCUART_ENABLE_INT(SC1, SC_INTEN_RDAIEN_Msk | SC_INTEN_RXTOIEN_Msk);//ʹ�ܽ����ж� | FIFO��ʱ�����ж�
}

/*
@���ܣ�SC1�жϴ�����
*/
void SC1_IRQHandler(void)
{
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_RDAIF_Msk) )//�������ݵ����ж�
	{
		uint8_t n = 0;
		while(!SCUART_GET_RX_EMPTY(SC1) && n++ < 2)
		{
			rx.len %= PACK_MAX_SIZE;
			rx.buf[rx.len++] = SC1->DAT;
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_RXTOIF_Msk) )//���ջ��峬ʱ�ж�
	{
		SCUART_CLR_INT_FLAG(SC1,  SC_INTSTS_RXTOIF_Msk);//���־
		while(!SCUART_GET_RX_EMPTY(SC1))
		{
			rx.len %= PACK_MAX_SIZE;
			rx.buf[rx.len++] = SC1->DAT;
		}
		if(callBack_recv)
		{
			callBack_recv((UART7_DATA*)&rx);
			callBack_recv = 0;//��ֹҰָ��
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_TBEIF_Msk) )//���ͻ�����ж�
	{
		SCUART_CLR_INT_FLAG(SC1, SC_INTSTS_TBEIF_Msk);//���־
		tx.size %= PACK_MAX_SIZE;
		if(tx.size < tx.len)
		{
			SC1->DAT = tx.buf[tx.size++];
		}
		else
		{
			SCUART_DISABLE_INT(SC1, SC_INTEN_TBEIEN_Msk);//�ط����ж�
			return;
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_TERRIF_Msk) )//��������ж�
	{
		SCUART_CLR_INT_FLAG(SC1,  SC_INTSTS_TERRIF_Msk);//���־
//		while(1);
	}
}


/*
@���ܣ�����7��������
@������	*pt_tx,���������ݻ���ָ��
				**pt_rx,����������ָ��
				callback�����մ���ص����������Ϊ�գ����͵�ָ���޷���Ӧ��
@����ֵ���������
@ע�⣺����������ݺܿ죬������Ҫ�����ź�����ס���͹���
*/
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUNICATION_RECV_CALLBACK callback)
{
	uint8_t i;
	if(pt_txbuf == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//�ص�������ʼ���������0��˴η��Ͳ���Ҫ����
	
	*pt_rx = (UART7_DATA*)&rx;
	rx.len = 0;//���ռ�����0
	
	for(i = 0; i < tx_len; i++)
		tx.buf[i] = pt_txbuf[i];
	tx.len = tx_len;//�����ͳ���
	tx.size = 0;//���ͼ�����0
	SCUART_WRITE(SC1, tx.buf[tx.size++]);
	SCUART_ENABLE_INT(SC1, SC_INTEN_TBEIEN_Msk);//�����ͻ�����ж�
	return TRUE;
}
