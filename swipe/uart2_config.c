#include "uart2_config.h"
#include "uart.h"
#include "pdma.h"


static volatile UART2_DATA rx;	//����ָ��
static volatile UART2_DATA *tx;	//����ָ��
static UART2_RECV_CALLBACK callBack_recv;//���մ���ص�����

/*
@���ܣ�Ĭ�ϻص�����
@˵�������û��ָ���ص���������ʹ��Ĭ�ϻص�����
*/
static void default_callback_recv(UART2_DATA *rx)
{
	UART_Write(UART2, (uint8_t*)"UART2 = ", sizeof("UART2 = "));
	UART_Write(UART2, rx->buf, rx->len);
}

/*
@���ܣ�dma����
*/
static void pdma_config(UART2_DATA *rx, UART2_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//ʹ��PDMAʱ��
	NVIC_SetPriority(PDMA_IRQn, 0);//�ж����ȼ�
	NVIC_EnableIRQ(PDMA_IRQn);//���ж�
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 2); // ʹ��ͨ��0
		PDMA_SetTransferMode(PDMA, 2, PDMA_UART2_TX, 0, 0);//pdmaͨ��0->UART2_tx��scatter_gather ��ֹ
		PDMA_SetBurstType(PDMA, 2, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 2, PDMA_WIDTH_8, tx->len);//���ݿ�ȣ����䳤��
		PDMA_SetTransferAddr(PDMA, 2, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART2->DAT)), PDMA_DAR_FIX);//Դ��ַ���ڴ�������Ŀ�ĵ�ַ���ڴ�̶�
		/*
		@@ע�⣺
		����򿪴��ж����������жϱ�־��
		��Ȼuart�����жϱ�־�� UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		������ UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
		PDMA_EnableInt(PDMA, 2, PDMA_INT_TRANS_DONE);//dma��������ж�,@��Ҫ�ﵽ���õĽ����������ܽ�����ж�
	}
}


/*
@���ܣ�UART2 ����
*/
void uart2_config(void)
{
	CLK_EnableModuleClock(UART2_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL3_UART2SEL_HIRC, CLK_CLKDIV4_UART2(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC4MFP_Msk | SYS_GPC_MFPL_PC5MFP_Msk);//PC4,PC5
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC4MFP_UART2_RXD | SYS_GPC_MFPL_PC5MFP_UART2_TXD);//ӳ�䵽����2��RX��TX

	UART_Open(UART2, 115200);
	
	pdma_config(NULL, NULL);
	
	UART2->FIFO &= ~UART_FIFO_RFITL_Msk;//����FIFO
	UART2->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO�г���14�ֽڲŻᴥ���ж�
	
	NVIC_SetPriority(UART2_IRQn, 6);
	NVIC_EnableIRQ(UART2_IRQn);
	UART_SetTimeoutCnt(UART2, 50);//FIFO��ʱʱ�䣺0~255
	UART_ENABLE_INT(UART2, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO��ʱ�ж� | �����ж�
	
	callBack_recv = default_callback_recv;//Ĭ�ϻص�
}


/*
@���ܣ�UART2�жϴ�����
*/
void UART2_IRQHandler(void)
{
	uint32_t status = UART2->INTSTS;
	
	if( UART_GET_INT_FLAG(UART2, UART_INTSTS_RDAIF_Msk) )//�����ж�
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART2) && n++ < 13)//������Ҫ����һ�����ݣ����򲻻���볬ʱ�ж�
		{
			rx.len %= UART2_BUF_SIZE;
			rx.buf[rx.len++] = UART2->DAT;
		}
	}
	
	if( UART_GET_INT_FLAG(UART2, UART_INTSTS_RXTOIF_Msk) )//���ճ�ʱ�жϣ��ٽ�������
	{
		while(!UART_GET_RX_EMPTY(UART2))
		{
			rx.len %= UART2_BUF_SIZE;
			rx.buf[rx.len++] = UART2->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART2_DATA*)&rx);//�ص�����
			callBack_recv = default_callback_recv;//��ֹҰָ��
		}
		else
		{
			default_callback_recv((UART2_DATA*)&rx);
		}
	}
	
	if( UART_GET_INT_FLAG(UART2, UART_INTSTS_HWTOIF_Msk) )//DMAʱ��ʱ�ж�
	{
		while(!UART_GET_RX_EMPTY(UART2))
		{
			rx.len %= UART2_BUF_SIZE;
			rx.buf[rx.len++] = UART2->DAT;
		}
		UART_Write(UART2, "G", sizeof("G"));
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
uint8_t _uart2_send(UART2_DATA *pt_tx , UART2_DATA** pt_rx, UART2_RECV_CALLBACK callback)
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//�ص�������ʼ��
	
	*pt_rx = (UART2_DATA*)&rx;
	rx.len = 0;//���ռ�����0
	tx = pt_tx;
	tx->size = 0;//���ͼ�����0
	tx->len %= UART2_BUF_SIZE;//���Ʒ��ͳ���
	
	UART_DISABLE_INT(UART2, UART_INTEN_TXPDMAEN_Msk);//����dma����
	UART_DISABLE_INT(UART2, UART_INTEN_RXPDMAEN_Msk);//����dma����
		
	pdma_config(NULL, (UART2_DATA*)tx);

	UART_ENABLE_INT(UART2, UART_INTEN_TXPDMAEN_Msk);//ʹ��DMA����,DMA����
//	UART_ENABLE_INT(UART2, UART_INTEN_RXPDMAEN_Msk);//ʹ��DMA����,DMA����

	return TRUE;
}
