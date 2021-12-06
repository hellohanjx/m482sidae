#include "uart1_config.h"
#include "uart.h"

/*
@���ܣ�uart1 ����
*/
void uart1_config(void)
{
	CLK_EnableModuleClock(UART1_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART1SEL_HIRC, CLK_CLKDIV0_UART1(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk);//PB3,PB2
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB2MFP_UART1_RXD | SYS_GPB_MFPL_PB3MFP_UART1_TXD);//ӳ�䵽���ڵ�RX��TX

	UART_Open(UART1, 115200);

	UART_EnableInt(UART1, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//���ճ�ʱ�ж�
	NVIC_EnableIRQ(UART1_IRQn);
	
	UART_Write(UART1, "UART1", sizeof("UART1"));
}


/*
@���ܣ�UART1�жϴ�����
*/
void UART1_IRQHandler(void)
{

}
