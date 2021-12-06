#include "UART4_config.h"
#include "uart.h"

/*
@功能：UART4 配置
*/
void uart4_config(void)
{
	CLK_EnableModuleClock(UART4_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART4_MODULE, CLK_CLKSEL3_UART4SEL_HIRC, CLK_CLKDIV4_UART4(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC6MFP_Msk | SYS_GPC_MFPL_PC7MFP_Msk);//PC6,PC7
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC6MFP_UART4_RXD | SYS_GPC_MFPL_PC7MFP_UART4_TXD);//映射到串口2的RX与TX

	UART_Open(UART4, 115200);

	UART_EnableInt(UART4, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//接收超时中断
	NVIC_EnableIRQ(UART4_IRQn);
	
	UART_Write(UART4, "UART4", sizeof("UART4"));
}


/*
@功能：UART4中断处理函数
*/
void UART4_IRQHandler(void)
{

}
