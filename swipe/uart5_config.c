#include "uart_config.h"
#include "uart.h"

static volatile UART_DATA rx;	//����ָ��
static volatile UART_DATA *tx;	//����ָ��
static UART_RECV_CALLBACK callBack_recv;//���մ���ص�����

/*
@���ܣ�Ĭ�ϻص�����
@˵�������û��ָ���ص���������ʹ��Ĭ�ϻص�����
*/
static void default_callback_recv(UART_DATA *rx)
{
	UART_Write(UART5, (uint8_t*)"UART5 = ", sizeof("UART5 = "));
	UART_Write(UART5, rx->buf, rx->len);
}

/*
@���ܣ�dma����
*/
static void pdma_config(UART_DATA *rx, UART_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//ʹ��PDMAʱ��
//	NVIC_SetPriority(PDMA_IRQn, 6);//�ж����ȼ�
//	NVIC_EnableIRQ(PDMA_IRQn);//���ж�
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 5); // ʹ��ͨ��0
		PDMA_SetTransferMode(PDMA, 5, PDMA_UART5_TX, 0, 0);//pdmaͨ��0->UART5_tx��scatter_gather ��ֹ
		PDMA_SetBurstType(PDMA, 5, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 5, PDMA_WIDTH_8, tx->len);//���ݿ�ȣ����䳤��
		PDMA_SetTransferAddr(PDMA, 5, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART5->DAT)), PDMA_DAR_FIX);//Դ��ַ���ڴ�������Ŀ�ĵ�ַ���ڴ�̶�
		/*
		@@ע�⣺
		����򿪴��ж����������жϱ�־��
		��Ȼuart�����жϱ�־�� UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		������ UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
//		PDMA_EnableInt(PDMA, 5, PDMA_INT_TRANS_DONE);//dma��������ж�,@��Ҫ�ﵽ���õĽ����������ܽ�����ж�
	}
}


/*
@���ܣ�UART5 ����
*/
void _uart5_config(uint32_t baud)
{
	UART_Close(UART5);
	
	CLK_EnableModuleClock(UART5_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART5_MODULE, CLK_CLKSEL3_UART5SEL_HIRC, CLK_CLKDIV4_UART5(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk);//PB4,PB5
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB4MFP_UART5_RXD | SYS_GPB_MFPL_PB5MFP_UART5_TXD);//ӳ�䵽���ڵ�RX��TX

	UART_Open(UART5, baud);
	
	pdma_config(NULL, NULL);
	
	UART5->FIFO &= ~UART_FIFO_RFITL_Msk;//����FIFO
	UART5->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO�г���14�ֽڲŻᴥ���ж�
	
	NVIC_SetPriority(UART5_IRQn, 6);
	NVIC_EnableIRQ(UART5_IRQn);
	UART_SetTimeoutCnt(UART5, 50);//FIFO��ʱʱ�䣺40~255
	UART_ENABLE_INT(UART5, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO��ʱ�ж� | �����ж�
	
	callBack_recv = default_callback_recv;//Ĭ�ϻص�
}


/*
@���ܣ�UART5�жϴ�����
*/
void UART5_IRQHandler(void)
{
	uint32_t status = UART5->INTSTS;
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_RDAIF_Msk) )//�����ж�
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART5) && n++ < 13)//������Ҫ����һ�����ݣ����򲻻���볬ʱ�ж�
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART5->DAT;
		}
	}
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_RXTOIF_Msk) )//���ճ�ʱ�жϣ��ٽ�������
	{
		while(!UART_GET_RX_EMPTY(UART5))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART5->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);//�ص�����
			callBack_recv = default_callback_recv;//��ֹҰָ��
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_THREIF_Msk) )//���ͻ�����ж�
	{
		if(tx !=0 )
		{
			tx->size %= UART_BUF_SIZE;
			if(tx->size < tx->len)
			{
				UART5->DAT = tx->buf[tx->size++];
			}
			else
			{
				UART_DISABLE_INT(UART5, UART_INTEN_THREIEN_Msk);//�ط����ж�
				return;
			}
		}
		else
		{
			UART_DISABLE_INT(UART5, UART_INTEN_THREIEN_Msk);//�ط����ж�
			return;
		}
	}
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_HWTOIF_Msk) )//DMAʱ��ʱ�ж�
	{
		while(!UART_GET_RX_EMPTY(UART5))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART5->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);//�ص�����
			callBack_recv = default_callback_recv;//��ֹҰָ��
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}
}

/*
@���ܣ�����6��������
@������	*pt_tx,����������ָ��
				**pt_rx,����������ָ��
				callback�����մ���ص����������Ϊ�գ����͵�ָ���޷���Ӧ��
@����ֵ���������
@ע�⣺����������ݺܿ죬������Ҫ�����ź�����ס���͹���
*/
uint8_t _uart5_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback)
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//�ص�������ʼ��
	
	*pt_rx = (UART_DATA*)&rx;
	rx.len = 0;//���ռ�����0
	tx = pt_tx;
	tx->size = 0;//���ͼ�����0
	tx->len %= UART_BUF_SIZE;//���Ʒ��ͳ���
	rx.id = tx->id;//���������Դ��ˢ�������0~5
	
	UART_DISABLE_INT(UART5, UART_INTEN_TXPDMAEN_Msk);//����dma����
	UART_DISABLE_INT(UART5, UART_INTEN_RXPDMAEN_Msk);//����dma����
	
	#if(0)	//ʹ�÷����ж�
//	UART_Write(UART5, (uint8_t*)tx->buf,tx->len);
	UART_WRITE(UART5, tx->buf[tx->size++]);
	UART_ENABLE_INT(UART5, UART_INTEN_THREIEN_Msk);
	#else //ʹ��PDMA����
	pdma_config(NULL, (UART_DATA*)tx);
	UART_ENABLE_INT(UART5, UART_INTEN_TXPDMAEN_Msk);//ʹ��DMA����,DMA����
	#endif
	
//	UART_ENABLE_INT(UART5, UART_INTEN_RXPDMAEN_Msk);//ʹ��DMA����,DMA����

	return TRUE;
}
