#include "uart0_log.h"
#include "uart.h"
#include "msg.h"

static volatile UART0_DATA rx;	//接收


/*
功能：配置uart0，作为日志输出口
*/
void _uart0_config(uint32_t baud)
{
	CLK_EnableModuleClock(UART0_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	/* Set GPB multi-function pins for UART0 RXD and TXD */
	SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);//设置多功能引脚PF3,PF2
	SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF3MFP_UART0_TXD | SYS_GPF_MFPL_PF2MFP_UART0_RXD);//映射到串口0的RX与TX
   
	UART_Open(UART0, baud);
	
	UART0->FIFO &= ~UART_FIFO_RFITL_Msk;//启用FIFO
	UART0->FIFO |= UART_FIFO_RFITL_14BYTES;//FIFO中超过14字节才会触发中断
	
	NVIC_SetPriority(UART0_IRQn, 8);
	NVIC_EnableIRQ(UART0_IRQn);
	UART_SetTimeoutCnt(UART0, 50);//FIFO超时时间：0~255
	UART_ENABLE_INT(UART0, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);//FIFO超时中断 | 接收中断
}



/*
@功能：UART0中断处理函数
*/
void UART0_IRQHandler(void)
{
	uint32_t status = UART0->INTSTS;
	
	if( UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAIF_Msk) )//接收中断
	{
		uint8_t n = 0;
		while(!UART_GET_RX_EMPTY(UART0) && n++ < 13)//这里需要少收一个数据，否则不会进入超时中断
		{
			rx.len %= UART0_BUF_SIZE;
			rx.buf[rx.len++] = UART0->DAT;
		}
	}
	
//	if( UART_GET_INT_FLAG(UART0, UART_INTSTS_RXTOINT_Msk) )//接收超时中断，先进入这里
//	{
//		UART_ClearIntFlag(UART0, UART_INTSTS_RXTOINT_Msk);
//	}
	if( UART_GET_INT_FLAG(UART0, UART_INTSTS_RXTOIF_Msk) )//接收超时中断，再进入这里
	{
		while(!UART_GET_RX_EMPTY(UART0))
		{
			rx.len %= UART0_BUF_SIZE;
			rx.buf[rx.len++] = UART0->DAT;
		}
		log_queue_send(rx);//加入队列
		rx.len = 0;
	}

}
