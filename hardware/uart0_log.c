#include "uart0_log.h"
#include "uart.h"
/*
���ܣ�����uart0����Ϊ��־�����
*/
void uart0_log_config(void)
{
	CLK_EnableModuleClock(UART0_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	/* Set GPB multi-function pins for UART0 RXD and TXD */
	SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);//���ö๦������PF3,PF2
	SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF3MFP_UART0_TXD | SYS_GPF_MFPL_PF2MFP_UART0_RXD);//ӳ�䵽����0��RX��TX
   
	UART_Open(UART0, 115200);
}
