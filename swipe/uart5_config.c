#include "uart_config.h"
#include "uart.h"

static volatile UART_DATA rx;	//接收指针
static volatile UART_DATA *tx;	//发送指针
static UART_RECV_CALLBACK callBack_recv;//接收处理回调函数

/*
@功能：默认回调函数
@说明：如果没有指定回调函数，则使用默认回调函数
*/
static void default_callback_recv(UART_DATA *rx)
{
	UART_Write(UART5, (uint8_t*)"UART5 = ", sizeof("UART5 = "));
	UART_Write(UART5, rx->buf, rx->len);
}

/*
@功能：dma配置
*/
static void pdma_config(UART_DATA *rx, UART_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//使能PDMA时钟
//	NVIC_SetPriority(PDMA_IRQn, 6);//中断优先级
//	NVIC_EnableIRQ(PDMA_IRQn);//打开中断
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 5); // 使能通道0
		PDMA_SetTransferMode(PDMA, 5, PDMA_UART5_TX, 0, 0);//pdma通道0->UART5_tx，scatter_gather 禁止
		PDMA_SetBurstType(PDMA, 5, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 5, PDMA_WIDTH_8, tx->len);//数据宽度，传输长度
		PDMA_SetTransferAddr(PDMA, 5, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART5->DAT)), PDMA_DAR_FIX);//源地址，内存自增；目的地址，内存固定
		/*
		@@注意：
		必须打开此中断清除掉完成中断标志，
		不然uart接收中断标志是 UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		而不是 UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
//		PDMA_EnableInt(PDMA, 5, PDMA_INT_TRANS_DONE);//dma传输完成中断,@需要达到设置的接收数量才能进入此中断
	}
}


/*
@功能：UART5 配置
*/
void _uart5_config(uint32_t baud)
{
	UART_Close(UART5);
	
	CLK_EnableModuleClock(UART5_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART5_MODULE, CLK_CLKSEL3_UART5SEL_HIRC, CLK_CLKDIV4_UART5(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk);//PB4,PB5
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB4MFP_UART5_RXD | SYS_GPB_MFPL_PB5MFP_UART5_TXD);//映射到串口的RX与TX

	UART_Open(UART5, baud);
	
	pdma_config(NULL, NULL);
	
	UART5->FIFO &= ~UART_FIFO_RFITL_Msk;//启用FIFO
	UART5->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO中超过14字节才会触发中断
	
	NVIC_SetPriority(UART5_IRQn, 6);
	NVIC_EnableIRQ(UART5_IRQn);
	UART_SetTimeoutCnt(UART5, 50);//FIFO超时时间：40~255
	UART_ENABLE_INT(UART5, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO超时中断 | 接收中断
	
	callBack_recv = default_callback_recv;//默认回调
}


/*
@功能：UART5中断处理函数
*/
void UART5_IRQHandler(void)
{
	uint32_t status = UART5->INTSTS;
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_RDAIF_Msk) )//接收中断
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART5) && n++ < 13)//这里需要少收一个数据，否则不会进入超时中断
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART5->DAT;
		}
	}
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_RXTOIF_Msk) )//接收超时中断，再进入这里
	{
		while(!UART_GET_RX_EMPTY(UART5))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART5->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);//回调处理
			callBack_recv = default_callback_recv;//防止野指针
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
	}
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_THREIF_Msk) )//发送缓存空中断
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
				UART_DISABLE_INT(UART5, UART_INTEN_THREIEN_Msk);//关发送中断
				return;
			}
		}
		else
		{
			UART_DISABLE_INT(UART5, UART_INTEN_THREIEN_Msk);//关发送中断
			return;
		}
	}
	
	if( UART_GET_INT_FLAG(UART5, UART_INTSTS_HWTOIF_Msk) )//DMA时超时中断
	{
		while(!UART_GET_RX_EMPTY(UART5))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART5->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART_DATA*)&rx);//回调处理
			callBack_recv = default_callback_recv;//防止野指针
		}
		else
		{
			default_callback_recv((UART_DATA*)&rx);
		}
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
uint8_t _uart5_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback)
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//回调函数初始化
	
	*pt_rx = (UART_DATA*)&rx;
	rx.len = 0;//接收计数清0
	tx = pt_tx;
	tx->size = 0;//发送计数清0
	tx->len %= UART_BUF_SIZE;//限制发送长度
	rx.id = tx->id;//标记数据来源，刷卡器编号0~5
	
	UART_DISABLE_INT(UART5, UART_INTEN_TXPDMAEN_Msk);//禁能dma发送
	UART_DISABLE_INT(UART5, UART_INTEN_RXPDMAEN_Msk);//禁能dma接收
	
	#if(0)	//使用发送中断
//	UART_Write(UART5, (uint8_t*)tx->buf,tx->len);
	UART_WRITE(UART5, tx->buf[tx->size++]);
	UART_ENABLE_INT(UART5, UART_INTEN_THREIEN_Msk);
	#else //使用PDMA发送
	pdma_config(NULL, (UART_DATA*)tx);
	UART_ENABLE_INT(UART5, UART_INTEN_TXPDMAEN_Msk);//使能DMA发送,DMA发送
	#endif
	
//	UART_ENABLE_INT(UART5, UART_INTEN_RXPDMAEN_Msk);//使能DMA发送,DMA接收

	return TRUE;
}
