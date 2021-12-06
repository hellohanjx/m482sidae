#include "uart6_config.h"
#include "uart.h"
#include "pdma.h"


static volatile UART6_DATA rx;	//����ָ��
static volatile UART6_DATA *tx;	//����ָ��
static CARD_RECV_CALLBACK callBack_recv;//���մ���ص�����


/*
@���ܣ�uart6 ���ã�ռ��SC0�ӿ�
*/
void uart6_config(void)
{
	CLK_EnableModuleClock(SC0_MODULE);//ʹ��SC0ʱ��
	CLK_SetModuleClock(SC0_MODULE, CLK_CLKSEL3_SC0SEL_HIRC, CLK_CLKDIV1_SC0(1));//����ʱ��Դ���Ƶ
	
	//��������
  SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
	SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_SC0_CLK | SYS_GPA_MFPL_PA1MFP_SC0_DAT);
	
	SCUART_SetLineConfig(SC0, 115200, SCUART_CHAR_LEN_8, SCUART_PARITY_NONE, SCUART_STOP_BIT_1);
//	SCUART_Open(SC0, 115200);

	//SC_INTEN_RXTOIEN_Msk	���ջ��峬ʱ�ж�
	//SC_INTEN_RDAIEN_Msk		�������ݵ����ж�
	//SC_INTEN_TERRIEN_Msk	��������ж�
	//SC_INTEN_TBEIEN_Msk		���ͻ�����ж�
	SCUART_ENABLE_INT(SC0, SC_INTEN_RDAIEN_Msk);// | SC_INTEN_TBEIEN_Msk);
	NVIC_EnableIRQ(SC0_IRQn);
	
//	SCUART_CLR_ERR_FLAG(SC0, SC_STATUS_PEF_Msk | SC_STATUS_FEF_Msk | SC_STATUS_BEF_Msk);
//	SCUART_CLR_INT_FLAG(SC0, SC_INTSTS_RXTOIF_Msk | SC_INTSTS_TERRIF_Msk | SC_INTSTS_TBEIF_Msk);
	
//	SCUART_WAIT_TX_EMPTY(SC0);
//	SCUART_Write(SC0, "SC0", sizeof("SC0"));
}


static void pdma_config(void)
{
	CLK_EnableModuleClock(PDMA_MODULE);//ʹ��pdmaʱ��
	
	
	
}


/*
@���ܣ�SC0�жϴ�����
*/
void SC0_IRQHandler(void)
{
	// Print SCUART received data to UART port
	// Data length here is short, so we're not care about UART FIFO over flow.
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RDAIF_Msk) )//�������ݵ����ж�
	{
		SCUART_WRITE(SC0, SCUART_READ(SC0));
		SCUART_WAIT_TX_EMPTY(SC0);
		SCUART_WRITE(SC0, '+');
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RXTOIF_Msk) )//���ջ��峬ʱ�ж�
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_RXTOIF_Msk);//���־
		SCUART_Write(SC0, "======", sizeof("======"));
		
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TBEIF_Msk) )//���ͻ�����ж�
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TBEIF_Msk);//���־
//		SCUART_DISABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//�ط����ж�
//		SCUART_ENABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//�������ж�
		if(tx->size < tx->len)
		{
			SCUART_WRITE(SC0, tx->buf[tx->size++]);
		}
		else
		{
			SCUART_DISABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//�ط����ж�
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TERRIF_Msk) )//��������ж�
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TERRIF_Msk);//���־
		
	}
	
	
//	while(SCUART_IS_TX_EMPTY(SC0))
//		SCUART_Write(SC0, au8TxBuf, sizeof(au8TxBuf));

	// RDA is the only interrupt enabled in this sample, this status bit
	// automatically cleared after Rx FIFO empty. So no need to clear interrupt
	// status here.

	return;
}



/*
@���ܣ�����1��������
@������	*pt_tx,����������ָ��
				*pt_rx,���������ݻ���
				callback�����մ���ص����������Ϊ�գ����͵�ָ���޷���Ӧ��
@����ֵ���������
*/
uint8_t _uart6_send(UART6_DATA *pt_tx , UART6_DATA** pt_rx, CARD_RECV_CALLBACK callback)   
{
	if(pt_tx == 0)
		return FALSE;
	
	tx = pt_tx;
	*pt_rx = (UART6_DATA*)&rx;
	
	if(callback != 0)
	{
		callBack_recv = callback;	//�ص�������ʼ��
	}
	else
	{
		callBack_recv = 0;//����Ҫ�ظ��ģ��޻ص�����
	}

	tx->size = 0;//���ͼ�����0
	rx.size = 0;//���ռ�����0

//	while(!SCUART_GET_TX_EMPTY(SC0));
	SCUART_ENABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//�����ͻ�����ж�
	SCUART_WRITE(SC0, tx->buf[tx->size++]);
	
	return TRUE;
}


