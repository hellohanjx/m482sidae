#include "uart1_config.h"
#include "uart.h"

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

	UART_EnableInt(UART1, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//接收超时中断
	NVIC_EnableIRQ(UART1_IRQn);
	
	UART_Write(UART1, "UART1", sizeof("UART1"));
}


/*
@功能：UART1中断处理函数
*/
void UART1_IRQHandler(void)
{

}
