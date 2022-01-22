#include "uart7_config.h"
#include "uart.h"
#include "scuart.h"


static volatile UART7_DATA rx;	//接收数据
static volatile UART7_DATA tx;	//发送数据
static COMMUNICATION_RECV_CALLBACK callBack_recv;//接收处理回调函数

/*
@功能：uart7串口配置，占用了SC1接口
*/
void _uart7_config(uint32_t baud)
{ 
	CLK_EnableModuleClock(SC1_MODULE);//使能SC1时钟
	CLK_SetModuleClock(SC1_MODULE, CLK_CLKSEL3_SC1SEL_HIRC, CLK_CLKDIV1_SC1(1));//设置时钟源与分频
	
  SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk);
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC0MFP_SC1_CLK | SYS_GPC_MFPL_PC1MFP_SC1_DAT);
	
	SCUART_Open(SC1, baud);
	SC1->CTL |= (0x2 << 6);//(SC_CTL_RXTRGLV_Msk);//RX-FIFO ，3字节触发
	SCUART_SetTimeoutCnt(SC1, 20);//FIFO 超时时间

	NVIC_SetPriority(SC1_IRQn, 6);
	NVIC_EnableIRQ(SC1_IRQn);
	SCUART_ENABLE_INT(SC1, SC_INTEN_RDAIEN_Msk | SC_INTEN_RXTOIEN_Msk);//使能接收中断 | FIFO超时缓冲中断
}

/*
@功能：SC1中断处理函数
*/
void SC1_IRQHandler(void)
{
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_RDAIF_Msk) )//接收数据到达中断
	{
		uint8_t n = 0;
		while(!SCUART_GET_RX_EMPTY(SC1) && n++ < 2)
		{
			rx.len %= PACK_MAX_SIZE;
			rx.buf[rx.len++] = SC1->DAT;
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_RXTOIF_Msk) )//接收缓冲超时中断
	{
		SCUART_CLR_INT_FLAG(SC1,  SC_INTSTS_RXTOIF_Msk);//清标志
		while(!SCUART_GET_RX_EMPTY(SC1))
		{
			rx.len %= PACK_MAX_SIZE;
			rx.buf[rx.len++] = SC1->DAT;
		}
		if(callBack_recv)
		{
			callBack_recv((UART7_DATA*)&rx);
			callBack_recv = 0;//防止野指针
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_TBEIF_Msk) )//发送缓存空中断
	{
		SCUART_CLR_INT_FLAG(SC1, SC_INTSTS_TBEIF_Msk);//清标志
		tx.size %= PACK_MAX_SIZE;
		if(tx.size < tx.len)
		{
			SC1->DAT = tx.buf[tx.size++];
		}
		else
		{
			SCUART_DISABLE_INT(SC1, SC_INTEN_TBEIEN_Msk);//关发送中断
			return;
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC1, SC_INTSTS_TERRIF_Msk) )//传输错误中断
	{
		SCUART_CLR_INT_FLAG(SC1,  SC_INTSTS_TERRIF_Msk);//清标志
//		while(1);
	}
}


/*
@功能：串口7发送数据
@参数：	*pt_tx,待发送数据缓冲指针
				**pt_rx,待接收数据指针
				callback，接收处理回调函数，如果为空，则发送的指令无返回应答
@返回值：操作结果
@注意：如果发送数据很快，这里需要采用信号量锁住发送过程
*/
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUNICATION_RECV_CALLBACK callback)
{
	uint8_t i;
	if(pt_txbuf == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//回调函数初始化，如果是0则此次发送不需要返回
	
	*pt_rx = (UART7_DATA*)&rx;
	rx.len = 0;//接收计数清0
	
	for(i = 0; i < tx_len; i++)
		tx.buf[i] = pt_txbuf[i];
	tx.len = tx_len;//待发送长度
	tx.size = 0;//发送计数清0
	SCUART_WRITE(SC1, tx.buf[tx.size++]);
	SCUART_ENABLE_INT(SC1, SC_INTEN_TBEIEN_Msk);//开发送缓存空中断
	return TRUE;
}
