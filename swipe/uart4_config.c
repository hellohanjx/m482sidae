#include "UART4_config.h"
#include "uart.h"

/*
@���ܣ�UART4 ����
*/
void uart4_config(void)
{
	CLK_EnableModuleClock(UART4_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART4_MODULE, CLK_CLKSEL3_UART4SEL_HIRC, CLK_CLKDIV4_UART4(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC6MFP_Msk | SYS_GPC_MFPL_PC7MFP_Msk);//PC6,PC7
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC6MFP_UART4_RXD | SYS_GPC_MFPL_PC7MFP_UART4_TXD);//ӳ�䵽����2��RX��TX

	UART_Open(UART4, 115200);

	UART_EnableInt(UART4, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//���ճ�ʱ�ж�
	NVIC_EnableIRQ(UART4_IRQn);
	
	UART_Write(UART4, "UART4", sizeof("UART4"));
}


/*
@���ܣ�UART4�жϴ�����
*/
void UART4_IRQHandler(void)
{

}
