#include "uart5_config.h"
#include "uart.h"

/*
@功能：UART5 配置
*/
void uart5_config(void)
{
	CLK_EnableModuleClock(UART5_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART5_MODULE, CLK_CLKSEL3_UART5SEL_HIRC, CLK_CLKDIV4_UART5(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk);//PB4,PB5
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB4MFP_UART5_RXD | SYS_GPB_MFPL_PB5MFP_UART5_TXD);//映射到串口的RX与TX

	UART_Open(UART5, 115200);

	
	//UART_INTEN_RXTOIEN_Msk 接收缓存超时
	UART_EnableInt(UART5, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//接收超时中断
	NVIC_EnableIRQ(UART5_IRQn);
	
	UART_Write(UART5, "UART5", sizeof("UART5"));
}


/*
@功能：UART5中断处理函数
*/
void UART5_IRQHandler(void)
{
	
}
