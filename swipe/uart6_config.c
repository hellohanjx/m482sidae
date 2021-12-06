#include "uart6_config.h"
#include "uart.h"
#include "pdma.h"


static volatile UART6_DATA rx;	//接收指针
static volatile UART6_DATA *tx;	//发送指针
static CARD_RECV_CALLBACK callBack_recv;//接收处理回调函数


/*
@功能：uart6 配置，占用SC0接口
*/
void uart6_config(void)
{
	CLK_EnableModuleClock(SC0_MODULE);//使能SC0时钟
	CLK_SetModuleClock(SC0_MODULE, CLK_CLKSEL3_SC0SEL_HIRC, CLK_CLKDIV1_SC0(1));//设置时钟源与分频
	
	//引脚配置
  SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
	SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_SC0_CLK | SYS_GPA_MFPL_PA1MFP_SC0_DAT);
	
	SCUART_SetLineConfig(SC0, 115200, SCUART_CHAR_LEN_8, SCUART_PARITY_NONE, SCUART_STOP_BIT_1);
//	SCUART_Open(SC0, 115200);

	//SC_INTEN_RXTOIEN_Msk	接收缓冲超时中断
	//SC_INTEN_RDAIEN_Msk		接收数据到达中断
	//SC_INTEN_TERRIEN_Msk	传输错误中断
	//SC_INTEN_TBEIEN_Msk		发送缓存空中断
	SCUART_ENABLE_INT(SC0, SC_INTEN_RDAIEN_Msk);// | SC_INTEN_TBEIEN_Msk);
	NVIC_EnableIRQ(SC0_IRQn);
	
//	SCUART_CLR_ERR_FLAG(SC0, SC_STATUS_PEF_Msk | SC_STATUS_FEF_Msk | SC_STATUS_BEF_Msk);
//	SCUART_CLR_INT_FLAG(SC0, SC_INTSTS_RXTOIF_Msk | SC_INTSTS_TERRIF_Msk | SC_INTSTS_TBEIF_Msk);
	
//	SCUART_WAIT_TX_EMPTY(SC0);
//	SCUART_Write(SC0, "SC0", sizeof("SC0"));
}


static void pdma_config(void)
{
	CLK_EnableModuleClock(PDMA_MODULE);//使能pdma时钟
	
	
	
}


/*
@功能：SC0中断处理函数
*/
void SC0_IRQHandler(void)
{
	// Print SCUART received data to UART port
	// Data length here is short, so we're not care about UART FIFO over flow.
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RDAIF_Msk) )//接收数据到达中断
	{
		SCUART_WRITE(SC0, SCUART_READ(SC0));
		SCUART_WAIT_TX_EMPTY(SC0);
		SCUART_WRITE(SC0, '+');
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_RXTOIF_Msk) )//接收缓冲超时中断
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_RXTOIF_Msk);//清标志
		SCUART_Write(SC0, "======", sizeof("======"));
		
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TBEIF_Msk) )//发送缓存空中断
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TBEIF_Msk);//清标志
//		SCUART_DISABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//关发送中断
//		SCUART_ENABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//开发送中断
		if(tx->size < tx->len)
		{
			SCUART_WRITE(SC0, tx->buf[tx->size++]);
		}
		else
		{
			SCUART_DISABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//关发送中断
		}
	}
	
	if( SCUART_GET_INT_FLAG(SC0, SC_INTSTS_TERRIF_Msk) )//传输错误中断
	{
		SCUART_CLR_INT_FLAG(SC0,  SC_INTSTS_TERRIF_Msk);//清标志
		
	}
	
	
//	while(SCUART_IS_TX_EMPTY(SC0))
//		SCUART_Write(SC0, au8TxBuf, sizeof(au8TxBuf));

	// RDA is the only interrupt enabled in this sample, this status bit
	// automatically cleared after Rx FIFO empty. So no need to clear interrupt
	// status here.

	return;
}



/*
@功能：串口1发送数据
@参数：	*pt_tx,待发送数据指针
				*pt_rx,待接收数据缓冲
				callback，接收处理回调函数，如果为空，则发送的指令无返回应答
@返回值：操作结果
*/
uint8_t _uart6_send(UART6_DATA *pt_tx , UART6_DATA** pt_rx, CARD_RECV_CALLBACK callback)   
{
	if(pt_tx == 0)
		return FALSE;
	
	tx = pt_tx;
	*pt_rx = (UART6_DATA*)&rx;
	
	if(callback != 0)
	{
		callBack_recv = callback;	//回调函数初始化
	}
	else
	{
		callBack_recv = 0;//不需要回复的，无回调函数
	}

	tx->size = 0;//发送计数清0
	rx.size = 0;//接收计数清0

//	while(!SCUART_GET_TX_EMPTY(SC0));
	SCUART_ENABLE_INT(SC0, SC_INTEN_TBEIEN_Msk);//开发送缓存空中断
	SCUART_WRITE(SC0, tx->buf[tx->size++]);
	
	return TRUE;
}


