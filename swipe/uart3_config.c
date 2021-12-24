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
	UART_Write(UART3, (uint8_t*)"UART3 = ", sizeof("UART3 = "));
	UART_Write(UART3, rx->buf, rx->len);
}

/*
@功能：dma配置
*/
static void pdma_config(UART_DATA *rx, UART_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//使能PDMA时钟
	NVIC_SetPriority(PDMA_IRQn, 0);//中断优先级
	NVIC_EnableIRQ(PDMA_IRQn);//打开中断
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 3); // 使能通道0
		PDMA_SetTransferMode(PDMA, 3, PDMA_UART3_TX, 0, 0);//pdma通道0->UART3_tx，scatter_gather 禁止
		PDMA_SetBurstType(PDMA, 3, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 3, PDMA_WIDTH_8, tx->len);//数据宽度，传输长度
		PDMA_SetTransferAddr(PDMA, 3, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART3->DAT)), PDMA_DAR_FIX);//源地址，内存自增；目的地址，内存固定
		/*
		@@注意：
		必须打开此中断清除掉完成中断标志，
		不然uart接收中断标志是 UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		而不是 UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
		PDMA_EnableInt(PDMA, 3, PDMA_INT_TRANS_DONE);//dma传输完成中断,@需要达到设置的接收数量才能进入此中断
	}
}


/*
@功能：UART3 配置
*/
void _uart3_config(uint32_t baud)
{
	CLK_EnableModuleClock(UART3_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART3_MODULE, CLK_CLKSEL3_UART3SEL_HIRC, CLK_CLKDIV4_UART3(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC3MFP_Msk);//PC2,PC3
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC2MFP_UART3_RXD | SYS_GPC_MFPL_PC3MFP_UART3_TXD);//映射到串口的RX与TX

	UART_Open(UART3, baud);
	
	pdma_config(NULL, NULL);
	
	UART3->FIFO &= ~UART_FIFO_RFITL_Msk;//启用FIFO
	UART3->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO中超过14字节才会触发中断
	
	NVIC_SetPriority(UART3_IRQn, 6);
	NVIC_EnableIRQ(UART3_IRQn);
	UART_SetTimeoutCnt(UART3, 50);//FIFO超时时间：40~255
	UART_ENABLE_INT(UART3, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO超时中断 | 接收中断
	
	callBack_recv = default_callback_recv;//默认回调
}


/*
@功能：UART3中断处理函数
*/
void UART3_IRQHandler(void)
{
	uint32_t status = UART3->INTSTS;
	
	if( UART_GET_INT_FLAG(UART3, UART_INTSTS_RDAIF_Msk) )//接收中断
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART3) && n++ < 13)//这里需要少收一个数据，否则不会进入超时中断
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART3->DAT;
		}
	}
	
	if( UART_GET_INT_FLAG(UART3, UART_INTSTS_RXTOIF_Msk) )//接收超时中断，再进入这里
	{
		while(!UART_GET_RX_EMPTY(UART3))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART3->DAT;
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
	
	if( UART_GET_INT_FLAG(UART3, UART_INTSTS_HWTOIF_Msk) )//DMA时超时中断
	{
		while(!UART_GET_RX_EMPTY(UART3))
		{
			rx.len %= UART_BUF_SIZE;
			rx.buf[rx.len++] = UART3->DAT;
		}
		UART_Write(UART3, "G", sizeof("G"));
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
uint8_t _uart3_send(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback)
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
	
	UART_DISABLE_INT(UART3, UART_INTEN_TXPDMAEN_Msk);//禁能dma发送
	UART_DISABLE_INT(UART3, UART_INTEN_RXPDMAEN_Msk);//禁能dma接收
		
	pdma_config(NULL, (UART_DATA*)tx);

	UART_ENABLE_INT(UART3, UART_INTEN_TXPDMAEN_Msk);//使能DMA发送,DMA发送
//	UART_ENABLE_INT(UART3, UART_INTEN_RXPDMAEN_Msk);//使能DMA发送,DMA接收

	return TRUE;
}
