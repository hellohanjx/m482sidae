#include "uart3_config.h"
#include "uart.h"

/*
@功能：UART3 配置
*/
void uart3_config(void)
{
	CLK_EnableModuleClock(UART3_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART3_MODULE, CLK_CLKSEL3_UART3SEL_HIRC, CLK_CLKDIV4_UART3(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC3MFP_Msk);//PC2,PC3
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC2MFP_UART3_RXD | SYS_GPC_MFPL_PC3MFP_UART3_TXD);//映射到串口的RX与TX

	UART_Open(UART3, 115200);

	UART_EnableInt(UART3, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//接收超时中断
	NVIC_EnableIRQ(UART3_IRQn);
	
	UART_Write(UART3, "UART3", sizeof("UART3"));
}


/*
@功能：UART3中断处理函数
*/
void UART3_IRQHandler(void)
{

}
