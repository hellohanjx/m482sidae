#include "uart1_config.h"
#include "uart.h"
#include "pdma.h"


static volatile UART1_DATA rx;	//接收指针
static volatile UART1_DATA *tx;	//发送指针
static UART1_RECV_CALLBACK callBack_recv;//接收处理回调函数

/*
@功能：默认回调函数
@说明：如果没有指定回调函数，则使用默认回调函数
*/
static void default_callback_recv(UART1_DATA *rx)
{
	UART_Write(UART1, (uint8_t*)"UART1 = ", sizeof("UART1 = "));
	UART_Write(UART1, rx->buf, rx->len);
}

/*
@功能：dma配置
*/
static void pdma_config(UART1_DATA *rx, UART1_DATA *tx)
{
	CLK_EnableModuleClock(PDMA_MODULE);//使能PDMA时钟
	NVIC_SetPriority(PDMA_IRQn, 0);//中断优先级
	NVIC_EnableIRQ(PDMA_IRQn);//打开中断
	
	if(tx != NULL)
	{
		PDMA_Open(PDMA, 1 << 0); // 使能通道0
		PDMA_SetTransferMode(PDMA, 0, PDMA_UART1_TX, 0, 0);//pdma通道0->uart1_tx，scatter_gather 禁止
		PDMA_SetBurstType(PDMA, 0, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 0, PDMA_WIDTH_8, tx->len);//数据宽度，传输长度
		PDMA_SetTransferAddr(PDMA, 0, (uint32_t)(tx->buf), PDMA_SAR_INC, (uint32_t)(&(UART1->DAT)), PDMA_DAR_FIX);//源地址，内存自增；目的地址，内存固定
		/*
		@@注意：
		必须打开此中断清除掉完成中断标志，
		不然uart接收中断标志是 UART_INTSTS_RXTOIF_Msk/UART_INTSTS_HWTOINT_Msk
		而不是 UART_INTSTS_RXTOINT_Msk/UART_INTSTS_RXTOIF_Msk
		*/
		PDMA_EnableInt(PDMA, 0, PDMA_INT_TRANS_DONE);//dma传输完成中断,@需要达到设置的接收数量才能进入此中断
	}
	
	if(rx != NULL)
	{
		PDMA_Open(PDMA, 1 << 1); // 使能通道1
		PDMA_SetTransferMode(PDMA, 1, PDMA_UART1_RX, 0, 0);//pdma通道1->uart1_rx，scatter_gather 禁止
		PDMA_SetBurstType(PDMA, 1, PDMA_REQ_SINGLE, 0);
		PDMA_SetTransferCnt(PDMA, 1, PDMA_WIDTH_8, rx->len);
		PDMA_SetTransferAddr(PDMA, 1, (uint32_t)(&(UART1->DAT)), PDMA_SAR_FIX, (uint32_t)(rx->buf), PDMA_DAR_INC);

//		PDMA_SetTimeOut(PDMA, 1, 1, 100);//DMA通道1接收超时，@@注意：PDMA只有通道1/2支持超时功能
		//@@下面只能一次使能一个，不能'|'使能
//		PDMA_EnableInt(PDMA, 1, PDMA_INT_TRANS_DONE);//使能传输完成中断,@需要达到设置的接收数量才能进入此中断
//		PDMA_EnableInt(PDMA, 1, PDMA_INT_TIMEOUT);//传输超时
//		PDMA_EnableInt(PDMA, 1, PDMA_INT_TEMPTY);
//		rx.len = 10;//dma 接收长度
	}
}


/*
@功能：uart1 配置
*/
void uart1_config(void)
{
	CLK_EnableModuleClock(UART1_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART1SEL_HIRC, CLK_CLKDIV0_UART1(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk);//PB3,PB2
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB2MFP_UART1_RXD | SYS_GPB_MFPL_PB3MFP_UART1_TXD);//映射到串口的RX与TX

	UART_Open(UART1, 115200);
	
	pdma_config(NULL, NULL);
	
	UART1->FIFO &= ~UART_FIFO_RFITL_Msk;//启用FIFO
	UART1->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO中超过14字节才会触发中断
	
	NVIC_SetPriority(UART1_IRQn, 6);
	NVIC_EnableIRQ(UART1_IRQn);
	UART_SetTimeoutCnt(UART1, 50);//FIFO超时时间：0~255
	UART_ENABLE_INT(UART1, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO超时中断 | 接收中断
	
	callBack_recv = default_callback_recv;//默认回调
}


/*
@功能：UART1中断处理函数
*/
void UART1_IRQHandler(void)
{
	uint32_t status = UART1->INTSTS;
	
	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_RDAIF_Msk) )//接收中断
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART1) && n++ < 13)//这里需要少收一个数据，否则不会进入超时中断
		{
			rx.len %= UART1_BUF_SIZE;
			rx.buf[rx.len++] = UART1->DAT;
		}
	}
	
//	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_RXTOINT_Msk) )//接收超时中断，先进入这里
//	{
//		UART_ClearIntFlag(UART1, UART_INTSTS_RXTOINT_Msk);
//	}
	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_RXTOIF_Msk) )//接收超时中断，再进入这里
	{
		while(!UART_GET_RX_EMPTY(UART1))
		{
			rx.len %= UART1_BUF_SIZE;
			rx.buf[rx.len++] = UART1->DAT;
		}
		if(callBack_recv != default_callback_recv)
		{
			callBack_recv((UART1_DATA*)&rx);//回调处理
			callBack_recv = default_callback_recv;//防止野指针
		}
		else
		{
			default_callback_recv((UART1_DATA*)&rx);
		}
	}

//	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_HWTOINT_Msk) )//dma超时接收中断
//	{
//		UART_WRITE(UART1, 'C');
//	}
	if( UART_GET_INT_FLAG(UART1, UART_INTSTS_HWTOIF_Msk) )//DMA时超时中断
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
@功能：串口6发送数据
@参数：	*pt_tx,待发送数据指针
				**pt_rx,待接收数据指针
				callback，接收处理回调函数，如果为空，则发送的指令无返回应答
@返回值：操作结果
@注意：如果发送数据很快，这里需要采用信号量锁住发送过程
*/
uint8_t _uart1_send(UART1_DATA *pt_tx , UART1_DATA** pt_rx, UART1_RECV_CALLBACK callback)   
{
	if(pt_tx == 0)
		return FALSE;
	
	if(callback)
		callBack_recv = callback;	//回调函数初始化
	
	*pt_rx = (UART1_DATA*)&rx;
	rx.len = 0;//接收计数清0
	tx = pt_tx;
	tx->size = 0;//发送计数清0
	tx->len %= UART1_BUF_SIZE;//限制发送长度
	
	UART_DISABLE_INT(UART1, UART_INTEN_TXPDMAEN_Msk);//禁能dma发送
	UART_DISABLE_INT(UART1, UART_INTEN_RXPDMAEN_Msk);//禁能dma接收
		
	pdma_config(NULL, (UART1_DATA*)tx);

	UART_ENABLE_INT(UART1, UART_INTEN_TXPDMAEN_Msk);//使能DMA发送,DMA发送
//	UART_ENABLE_INT(UART1, UART_INTEN_RXPDMAEN_Msk);//使能DMA发送,DMA接收

	return TRUE;
}
