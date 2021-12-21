#include "uart1_config.h"
#include "uart.h"
#include "pdma.h"


static volatile UART1_DATA rx;	//����ָ��
static volatile UART1_DATA *tx;	//����ָ��
static UART1_RECV_CALLBACK callBack_recv;//���մ���ص�����

/*
@���ܣ�Ĭ�ϻص�����
@˵�������û��ָ���ص���������ʹ��Ĭ�ϻص�����
*/
static void default_callback_recv(UART1_DATA *rx)
{
	UART_Write(UART1, (uint8_t*)"UART1 = ", sizeof("UART1 = "));
	UART_Write(UART1, rx->buf, rx->len);
}

/*
@���ܣ�dma����
*/
static void pdma_config(UART1_DATA *rx, UART1_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//ʹ��PDMAʱ��
	NVIC_SetPriority(PDMA_IRQn, 0);//�ж����ȼ�
	NVIC_EnableIRQ(PDMA_IRQn);//���ж�
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 0); // ʹ��ͨ��0
		PDMA_SetTransferMode(PDMA, 0, PDMA_UART1_TX, 0, 0);//pdmaͨ��0->uart1_tx��scatter_gather ��ֹ
		PDMA_SetBurstType(PDMA, 0, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 0, PDMA_WIDTH_8, tx->len);//���ݿ�ȣ����䳤��
		PDMA_SetTransferAddr(PDMA, 0, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART1->DAT)), PDMA_DAR_FIX);//Դ��ַ���ڴ�������Ŀ�ĵ�ַ���ڴ�̶�
		/*
		@@ע�⣺
		����򿪴��ж����������жϱ�־��
		��Ȼuart�����жϱ�־�� UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		������ UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
		PDMA_EnableInt(PDMA, 0, PDMA_INT_TRANS_DONE);//dma��������ж�,@��Ҫ�ﵽ���õĽ����������ܽ�����ж�
	}
	
	if(rx != NULL)
	{
		PDMA_Open(PDMA, 1 << 1); // ʹ��ͨ��1
		PDMA_SetTransferMode(PDMA, 1, PDMA_UART1_RX, 0, 0);//pdmaͨ��1->uart1_rx��scatter_gather ��ֹ
		PDMA_SetBurstType(PDMA, 1, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 1, PDMA_WIDTH_8, rx->len);
		PDMA_SetTransferAddr(PDMA, 1, (uint32_t)(&(UART1->DAT)), PDMA_SAR_FIX, (uint32_t)(rx->buf), PDMA_DAR_INC);

//		PDMA_SetTimeOut(PDMA, 1, 1, 100);//DMAͨ��1���ճ�ʱ��@@ע�⣺PDMAֻ��ͨ��1/2֧�ֳ�ʱ����
		//@@����ֻ��һ��ʹ��һ��������'|'ʹ��
//		PDMA_EnableInt(PDMA, 1, PDMA_INT_TRANS_DONE);//ʹ�ܴ�������ж�,@��Ҫ�ﵽ���õĽ����������ܽ�����ж�
//		PDMA_EnableInt(PDMA, 1, PDMA_INT_TIMEOUT);//���䳬ʱ
//		PDMA_EnableInt(PDMA, 1, PDMA_INT_TEMPTY);
//		rx.len = 10;//dma ���ճ���
	}
}


/*
@���ܣ�uart1 ����
*/
void uart1_config(void)
{
	CLK_EnableModuleClock(UART1_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART1SEL_HIRC, CLK_CLKDIV0_UART1(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk);//PB3,PB2
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB2MFP_UART1_RXD | SYS_GPB_MFPL_PB3MFP_UART1_TXD);//ӳ�䵽���ڵ�RX��TX

	UART_Open(UART1, 115200);
	
	pdma_config(NULL, NULL);
	
	UART1->FIFO &= ~UART_FIFO_RFITL_Msk;//����FIFO
	UART1->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO�г���14�ֽڲŻᴥ���ж�
	
	NVIC_SetPriority(UART1_IRQn, 6);
	NVIC_EnableIRQ(UART1_IRQn);
	UART_SetTimeoutCnt(UART1, 50);//FIFO��ʱʱ�䣺0~255
	UART_ENABLE_INT(UART1, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO��ʱ�ж� | �����ж�
	
	callBack_recv = default_callback_recv;//Ĭ�ϻص�
}


/*
@���ܣ�UART1�жϴ�����
*/
void UART1_IRQHandler(void)
{
	uint32_t status = UART1->INTSTS;
	
	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_RDAIF_Msk) )//�����ж�
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART1) && n++ < 13)//������Ҫ����һ�����ݣ����򲻻���볬ʱ�ж�
		{
			rx.len %= UART1_BUF_SIZE;
			rx.buf[rx.len++] = UART1->DAT;
		}
	}
	
//	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_RXTOINT_Msk) )//���ճ�ʱ�жϣ��Ƚ�������
//	{
//		UART_ClearIntFlag(UART1, UART_INTSTS_RXTOINT_Msk);
//	}
	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_RXTOIF_Msk) )//���ճ�ʱ�жϣ��ٽ�������
	{
		while(!UART_GET_RX_EMPTY(UART1))
		{
			rx.len %= UART1_BUF_SIZE;
			rx.buf[rx.len++] = UART1->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART1_DATA*)&rx);//�ص�����
			callBack_recv = default_callback_recv;//��ֹҰָ��
		}
		else
		{
			default_callback_recv((UART1_DATA*)&rx);
		}
	}

//	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_HWTOINT_Msk) )//dma��ʱ�����ж�
//	{
//		UART_WRITE(UART1, 'C');
//	}
	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_HWTOIF_Msk) )//DMAʱ��ʱ�ж�
	{
		while(!UART_GET_RX_EMPTY(UART1))
		{
			rx.len %= UART1_BUF_SIZE;
			rx.buf[rx.len++] = UART1->DAT;
		}
		UART_Write(UART1, "G", sizeof("G"));
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
uint8_t _uart1_send(UART1_DATA *pt_tx , UART1_DATA** pt_rx, UART1_RECV_CALLBACK callback)   
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//�ص�������ʼ��
	
	*pt_rx = (UART1_DATA*)&rx;
	rx.len = 0;//���ռ�����0
	tx = pt_tx;
	tx->size = 0;//���ͼ�����0
	tx->len %= UART1_BUF_SIZE;//���Ʒ��ͳ���
	
	UART_DISABLE_INT(UART1, UART_INTEN_TXPDMAEN_Msk);//����dma����
	UART_DISABLE_INT(UART1, UART_INTEN_RXPDMAEN_Msk);//����dma����
		
	pdma_config(NULL, (UART1_DATA*)tx);

	UART_ENABLE_INT(UART1, UART_INTEN_TXPDMAEN_Msk);//ʹ��DMA����,DMA����
//	UART_ENABLE_INT(UART1, UART_INTEN_RXPDMAEN_Msk);//ʹ��DMA����,DMA����

	return TRUE;
}
