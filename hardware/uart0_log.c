#include "uart0_log.h"
#include "uart.h"
/*
功能：配置uart0，作为日志输出口
*/
void uart0_log_config(void)
{
	CLK_EnableModuleClock(UART0_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	/* Set GPB multi-function pins for UART0 RXD and TXD */
	SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);//设置多功能引脚PF3,PF2
	SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF3MFP_UART0_TXD | SYS_GPF_MFPL_PF2MFP_UART0_RXD);//映射到串口0的RX与TX
   
	UART_Open(UART0, 115200);
}
