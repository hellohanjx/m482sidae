#include "uart5_config.h"
#include "uart.h"

/*
@���ܣ�UART5 ����
*/
void uart5_config(void)
{
	CLK_EnableModuleClock(UART5_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART5_MODULE, CLK_CLKSEL3_UART5SEL_HIRC, CLK_CLKDIV4_UART5(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk);//PB4,PB5
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB4MFP_UART5_RXD | SYS_GPB_MFPL_PB5MFP_UART5_TXD);//ӳ�䵽���ڵ�RX��TX

	UART_Open(UART5, 115200);

	
	//UART_INTEN_RXTOIEN_Msk ���ջ��泬ʱ
	UART_EnableInt(UART5, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//���ճ�ʱ�ж�
	NVIC_EnableIRQ(UART5_IRQn);
	
	UART_Write(UART5, "UART5", sizeof("UART5"));
}


/*
@���ܣ�UART5�жϴ�����
*/
void UART5_IRQHandler(void)
{
	
}
