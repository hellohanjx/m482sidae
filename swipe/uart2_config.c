#include "uart2_config.h"
#include "uart.h"
#include "pdma.h"


static volatile UART2_DATA rx;	//接收指针
static volatile UART2_DATA *tx;	//发送指针
static UART2_RECV_CALLBACK callBack_recv;//接收处理回调函数

/*
@功能：默认回调函数
@说明：如果没有指定回调函数，则使用默认回调函数
*/
static void default_callback_recv(UART2_DATA *rx)
{
	UART_Write(UART2, (uint8_t*)"UART2 = ", sizeof("UART2 = "));
	UART_Write(UART2, rx->buf, rx->len);
}

/*
@功能：dma配置
*/
static void pdma_config(UART2_DATA *rx, UART2_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//使能PDMA时钟
	NVIC_SetPriority(PDMA_IRQn, 0);//中断优先级
	NVIC_EnableIRQ(PDMA_IRQn);//打开中断
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 2); // 使能通道0
		PDMA_SetTransferMode(PDMA, 2, PDMA_UART2_TX, 0, 0);//pdma通道0->UART2_tx，scatter_gather 禁止
		PDMA_SetBurstType(PDMA, 2, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 2, PDMA_WIDTH_8, tx->len);//数据宽度，传输长度
		PDMA_SetTransferAddr(PDMA, 2, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART2->DAT)), PDMA_DAR_FIX);//源地址，内存自增；目的地址，内存固定
		/*
		@@注意：
		必须打开此中断清除掉完成中断标志，
		不然uart接收中断标志是 UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		而不是 UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
		PDMA_EnableInt(PDMA, 2, PDMA_INT_TRANS_DONE);//dma传输完成中断,@需要达到设置的接收数量才能进入此中断
	}
}


/*
@功能：UART2 配置
*/
void uart2_config(void)
{
	CLK_EnableModuleClock(UART2_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL3_UART2SEL_HIRC, CLK_CLKDIV4_UART2(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC4MFP_Msk | SYS_GPC_MFPL_PC5MFP_Msk);//PC4,PC5
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC4MFP_UART2_RXD | SYS_GPC_MFPL_PC5MFP_UART2_TXD);//映射到串口2的RX与TX

	UART_Open(UART2, 115200);
	
	pdma_config(NULL, NULL);
	
	UART2->FIFO &= ~UART_FIFO_RFITL_Msk;//启用FIFO
	UART2->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO中超过14字节才会触发中断
	
	NVIC_SetPriority(UART2_IRQn, 6);
	NVIC_EnableIRQ(UART2_IRQn);
	UART_SetTimeoutCnt(UART2, 50);//FIFO超时时间：0~255
	UART_ENABLE_INT(UART2, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO超时中断 | 接收中断
	
	callBack_recv = default_callback_recv;//默认回调
}


/*
@功能：UART2中断处理函数
*/
void UART2_IRQHandler(void)
{
	uint32_t status = UART2->INTSTS;
	
	if( UART_GET_INT_FLAG(UART2, UART_INTSTS_RDAIF_Msk) )//接收中断
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART2) && n++ < 13)//这里需要少收一个数据，否则不会进入超时中断
		{
			rx.len %= UART2_BUF_SIZE;
			rx.buf[rx.len++] = UART2->DAT;
		}
	}
	
	if( UART_GET_INT_FLAG(UART2, UART_INTSTS_RXTOIF_Msk) )//接收超时中断，再进入这里
	{
		while(!UART_GET_RX_EMPTY(UART2))
		{
			rx.len %= UART2_BUF_SIZE;
			rx.buf[rx.len++] = UART2->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART2_DATA*)&rx);//回调处理
			callBack_recv = default_callback_recv;//防止野指针
		}
		else
		{
			default_callback_recv((UART2_DATA*)&rx);
		}
	}
	
	if( UART_GET_INT_FLAG(UART2, UART_INTSTS_HWTOIF_Msk) )//DMA时超时中断
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
@功能：串口6发送数据
@参数：	*pt_tx,待发送数据指针
				**pt_rx,待接收数据指针
				callback，接收处理回调函数，如果为空，则发送的指令无返回应答
@返回值：操作结果
@注意：如果发送数据很快，这里需要采用信号量锁住发送过程
*/
uint8_t _uart2_send(UART2_DATA *pt_tx , UART2_DATA** pt_rx, UART2_RECV_CALLBACK callback)
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//回调函数初始化
	
	*pt_rx = (UART2_DATA*)&rx;
	rx.len = 0;//接收计数清0
	tx = pt_tx;
	tx->size = 0;//发送计数清0
	tx->len %= UART2_BUF_SIZE;//限制发送长度
	
	UART_DISABLE_INT(UART2, UART_INTEN_TXPDMAEN_Msk);//禁能dma发送
	UART_DISABLE_INT(UART2, UART_INTEN_RXPDMAEN_Msk);//禁能dma接收
		
	pdma_config(NULL, (UART2_DATA*)tx);

	UART_ENABLE_INT(UART2, UART_INTEN_TXPDMAEN_Msk);//使能DMA发送,DMA发送
//	UART_ENABLE_INT(UART2, UART_INTEN_RXPDMAEN_Msk);//使能DMA发送,DMA接收

	return TRUE;
}
