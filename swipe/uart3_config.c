#include "uart3_config.h"
#include "uart.h"

/*
@���ܣ�UART3 ����
*/
void uart3_config(void)
{
	CLK_EnableModuleClock(UART3_MODULE);//ʹ�ܴ���ʱ��
	CLK_SetModuleClock(UART3_MODULE, CLK_CLKSEL3_UART3SEL_HIRC, CLK_CLKDIV4_UART3(1));//ѡ�񴮿�ʱ��Դ,��1��������ģ�飬��2����ʱ��Դ����3������Ƶ

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC3MFP_Msk);//PC2,PC3
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC2MFP_UART3_RXD | SYS_GPC_MFPL_PC3MFP_UART3_TXD);//ӳ�䵽���ڵ�RX��TX

	UART_Open(UART3, 115200);

	UART_EnableInt(UART3, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//���ճ�ʱ�ж�
	NVIC_EnableIRQ(UART3_IRQn);
	
	UART_Write(UART3, "UART3", sizeof("UART3"));
}


/*
@���ܣ�UART3�жϴ�����
*/
void UART3_IRQHandler(void)
{

}
