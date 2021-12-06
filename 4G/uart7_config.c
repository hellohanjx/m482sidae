#include "uart7_config.h"
#include "uart.h"
#include "scuart.h"


static volatile UART7_DATA rx;	//接收数据
static volatile UART7_DATA *tx;	//发送指针
static COMMUCATION_RECV_CALLBACK callBack_recv = 0;//接收处理回调函数

/*
@功能：uart7串口配置，占用了SC1接口
*/
void uart7_config(void)
{ 
	CLK_EnableModuleClock(SC1_MODULE);//使能SC1时钟
	CLK_SetModuleClock(SC1_MODULE, CLK_CLKSEL3_SC1SEL_HIRC, CLK_CLKDIV1_SC1(1));//设置时钟源与分频
	
  SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk);
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC0MFP_SC1_CLK | SYS_GPC_MFPL_PC1MFP_SC1_DAT);
	
	SCUART_Open(SC1, 115200);

	SCUART_ENABLE_INT(SC1, SC_INTEN_RDAIEN_Msk);
	NVIC_EnableIRQ(SC1_IRQn);
}

/*
@功能：SC1中断处理函数
*/
void SC1_IRQHandler(void)
{
    while(!SCUART_GET_RX_EMPTY(SC1))
        UART_WRITE(UART0, SCUART_READ(SC1));

    // RDA is the only interrupt enabled in this sample, this status bit
    // automatically cleared after Rx FIFO empty. So no need to clear interrupt
    // status here.

    return;
}


/*
@功能：串口发送数据
@参数：	**pt_rx,待传递数据的指针的地址
				*pt_tx,待发送数据指针
				len, 发送数据长度
				recvCallBacl，接收回调函数
@返回值：操作结果
*/
uint8_t _uart7_send(UART7_DATA **pt_rx, uint8_t *pt_txbuf , uint32_t tx_len, COMMUCATION_RECV_CALLBACK callback)  
{
	uint32_t i;
	if(pt_txbuf == 0 || callback == 0)
		return FALSE;
	
	*pt_rx = (UART7_DATA*)&rx;//接收缓冲指向提供的指针，提供给外部使用
	for(i = 0; i < PACK_MAX_SIZE; i++) rx.buf[i] = 0;//接收数据清空
	callBack_recv = callback;	//回调函数初始化

	
	return TRUE;
}
