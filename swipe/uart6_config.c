#include "uart_config.h"
#include "uart.h"

static volatile UART_DATA rx;	//����ָ��
static volatile UART_DATA *tx;	//����ָ��
static UART_RECV_CALLBACK callBack_recv;//���մ���ص�����

#define TIMER_MODE	0//0��ʱ��ģʽ��1FIFO��ʱģʽ
/*
@���ܣ�Ĭ�ϻص�����
@˵�������û��ָ���ص���������ʹ��Ĭ�ϻص�����
*/
static void default_callback_recv(UART_DATA *rx)
{
	SCUART_Write(SC0, (uint8_t*)"SC0 = ", sizeof("SC0 = "));
	SCUART_Write(SC0, rx->buf, rx->len);
}

/*
@���ܣ�uart6 ���ã�ռ��SC0�ӿ�
*/
void _uart6_config(uint32_t baud)
{
	CLK_EnableModuleClock(SC0_MODULE);//ʹ��SC0ʱ��
	CLK_SetModuleClock(SC0_MODULE, CLK_CLKSEL3_SC0SEL_HIRC, CLK_CLKDIV1_SC0(1));//����ʱ��Դ���Ƶ
	
  SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
	SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_SC0_CLK | SYS_GPA_MFPL_PA1MFP_SC0_DAT);
	
	SCUART_Open(SC0, baud);
#if TIMER_MODE == 1
	SC0->CTL |= SC_CTL_TMRSEL_Msk;//ʹ�ܶ�ʱ��
#else
	SC0->CTL |= (0x2 << 6);//(SC_CTL_RXTRGLV_Msk);//RX-FIFO:3�ֽڴ���
	SCUART_SetTimeoutCnt(SC0, 3);
#endif
	
	NVIC_SetPriority(SC0_IRQn, 6);
	NVIC_EnableIRQ(SC0_IRQn);
#if TIMER_MODE == 1
	SCUART_ENABLE_INT(SC0, SC_INTEN_RDAIEN_Msk | SC_INTEN_TMR0IEN_Msk);//ʹ�ܽ����ж� | ��ʱ��0�ж�
#else	
	SCUART_ENABLE_INT(SC0, SC_INTEN_RDAIEN_Msk | SC_INTEN_RXTOIEN_Msk);//ʹ�ܽ����ж� | FIFO���ճ�ʱ�ж�
#endif

	callBack_recv = default_callback_recv;//��ʼ���ص�����
}

/*
@���ܣ�SC0�жϴ�����
*/
void SC0_IRQHandler(void)
{
	if( SC0->INTSTS & SC_INTSTS_TMR0IF_Msk )
	{
		SC0->INTSTS = SC_INTSTS_TMR0IF_Msk;
//		SC_DISABLE_INT(SC0, SC_INTEN_TMR0IEN_Msk);//�رն�ʱ��0�ж�
//		SC0->CTL &= ~SC_CTL_TMRSEL_Msk;//�رն�ʱ��
		SC_StopTimer(SC0, 0);
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);
			callBack_recv = default_callback_recv;//��ֹҰָ��
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}	
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RDAIF_Msk) )//�������ݵ����ж�
	{
#if TIMER_MODE == 0
		uint8_t n = 0;
		while(!SCUART_GET_RX_EMPTY(SC0) && n++ < 2)
#endif
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = SCUART_READ(SC0);
		}
#if TIMER_MODE == 1
		SC_StopTimer(SC0, 0);//@������Ҫ��ֹͣ�������أ�����������Ч
		SC_StartTimer(SC0, 0, SC_TMR_MODE_4, 105);//����ط�105��Ҫ����ETU(@�����Ǵﵽ����1bit���յļ��)
#endif
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RXTOIF_Msk) )//���ջ��峬ʱ�ж�
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_RXTOIF_Msk);//���־
		while(!SCUART_GET_RX_EMPTY(SC0))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = SCUART_READ(SC0);
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);
			callBack_recv = default_callback_recv;//��ֹҰָ��
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TBEIF_Msk) )//���ͻ�����ж�
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TBEIF_Msk);//���־
		tx->size %= UART_BUF_SIZE;
		if(tx->size < tx->len)
		{
			SC0->DAT = tx->buf[tx->size++];
		}
		else
		{
			SCUART_DISABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//�ط����ж�
			return;
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TERRIF_Msk) )//��������ж�
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TERRIF_Msk);//���־
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
uint8_t _uart6_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback)
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//�ص�������ʼ���������0��˴η��Ͳ���Ҫ����
	
	*pt_rx = (UART_DATA*)&rx;
	tx = pt_tx;
	tx->size = 0;//���ͼ�����0
	rx.len = 0;//���ռ�����0
	tx->len %= UART_BUF_SIZE;//���Ʒ��ͳ���
	rx.id = tx->id;//���������Դ��ˢ�������0~5
	
	SCUART_WRITE(SC0, tx->buf[tx->size++]);
	SCUART_ENABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//�����ͻ�����ж�
	return TRUE;
}
