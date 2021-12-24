#include "uart_config.h"
#include "uart.h"

static volatile UART_DATA rx;	//接收指针
static volatile UART_DATA *tx;	//发送指针
static UART_RECV_CALLBACK callBack_recv;//接收处理回调函数

#define TIMER_MODE	0//0定时器模式，1FIFO超时模式
/*
@功能：默认回调函数
@说明：如果没有指定回调函数，则使用默认回调函数
*/
static void default_callback_recv(UART_DATA *rx)
{
	SCUART_Write(SC0, (uint8_t*)"SC0 = ", sizeof("SC0 = "));
	SCUART_Write(SC0, rx->buf, rx->len);
}

/*
@功能：uart6 配置，占用SC0接口
*/
void _uart6_config(uint32_t baud)
{
	CLK_EnableModuleClock(SC0_MODULE);//使能SC0时钟
	CLK_SetModuleClock(SC0_MODULE, CLK_CLKSEL3_SC0SEL_HIRC, CLK_CLKDIV1_SC0(1));//设置时钟源与分频
	
  SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
	SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_SC0_CLK | SYS_GPA_MFPL_PA1MFP_SC0_DAT);
	
	SCUART_Open(SC0, baud);
#if TIMER_MODE == 1
	SC0->CTL |= SC_CTL_TMRSEL_Msk;//使能定时器
#else
	SC0->CTL |= (0x2 << 6);//(SC_CTL_RXTRGLV_Msk);//RX-FIFO:3字节触发
	SCUART_SetTimeoutCnt(SC0, 3);
#endif
	
	NVIC_SetPriority(SC0_IRQn, 6);
	NVIC_EnableIRQ(SC0_IRQn);
#if TIMER_MODE == 1
	SCUART_ENABLE_INT(SC0, SC_INTEN_RDAIEN_Msk | SC_INTEN_TMR0IEN_Msk);//使能接收中断 | 定时器0中断
#else	
	SCUART_ENABLE_INT(SC0, SC_INTEN_RDAIEN_Msk | SC_INTEN_RXTOIEN_Msk);//使能接收中断 | FIFO接收超时中断
#endif

	callBack_recv = default_callback_recv;//初始化回调函数
}

/*
@功能：SC0中断处理函数
*/
void SC0_IRQHandler(void)
{
	if( SC0->INTSTS & SC_INTSTS_TMR0IF_Msk )
	{
		SC0->INTSTS = SC_INTSTS_TMR0IF_Msk;
//		SC_DISABLE_INT(SC0, SC_INTEN_TMR0IEN_Msk);//关闭定时器0中断
//		SC0->CTL &= ~SC_CTL_TMRSEL_Msk;//关闭定时器
		SC_StopTimer(SC0, 0);
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);
			callBack_recv = default_callback_recv;//防止野指针
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}	
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RDAIF_Msk) )//接收数据到达中断
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
		SC_StopTimer(SC0, 0);//@这里需要先停止，再重载，否则重载无效
		SC_StartTimer(SC0, 0, SC_TMR_MODE_4, 105);//这个地方105需要大于ETU(@估计是达到超过1bit接收的间隔)
#endif
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RXTOIF_Msk) )//接收缓冲超时中断
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_RXTOIF_Msk);//清标志
		while(!SCUART_GET_RX_EMPTY(SC0))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = SCUART_READ(SC0);
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);
			callBack_recv = default_callback_recv;//防止野指针
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TBEIF_Msk) )//发送缓存空中断
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TBEIF_Msk);//清标志
		tx->size %= UART_BUF_SIZE;
		if(tx->size < tx->len)
		{
			SC0->DAT = tx->buf[tx->size++];
		}
		else
		{
			SCUART_DISABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//关发送中断
			return;
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TERRIF_Msk) )//传输错误中断
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TERRIF_Msk);//清标志
	}
}



/*
@功能：串口6发送数据
@参数：	*pt_tx,待发送数据指针
				**pt_rx,待接收数据指针
				callback，接收处理回调函数，如果为空，则发送的指令无返回应答
@返回值：操作结果
@注意：如果发送数据很快，这里需要采用信号量锁住发送过程
*/
uint8_t _uart6_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback)
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//回调函数初始化，如果是0则此次发送不需要返回
	
	*pt_rx = (UART_DATA*)&rx;
	tx = pt_tx;
	tx->size = 0;//发送计数清0
	rx.len = 0;//接收计数清0
	tx->len %= UART_BUF_SIZE;//限制发送长度
	rx.id = tx->id;//标记数据来源，刷卡器编号0~5
	
	SCUART_WRITE(SC0, tx->buf[tx->size++]);
	SCUART_ENABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//开发送缓存空中断
	return TRUE;
}
