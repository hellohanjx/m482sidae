#include "uart2_config.h"
#include "uart.h"

/*
@功能：uart2 配置
*/
void uart2_config(void)
{
	CLK_EnableModuleClock(UART2_MODULE);//使能串口时钟
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL3_UART2SEL_HIRC, CLK_CLKDIV4_UART2(1));//选择串口时钟源,第1参数串口模块，第2参数时钟源，第3参数分频

	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC4MFP_Msk | SYS_GPC_MFPL_PC5MFP_Msk);//PC4,PC5
	SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC4MFP_UART2_RXD | SYS_GPC_MFPL_PC5MFP_UART2_TXD);//映射到串口2的RX与TX

	UART_Open(UART2, 115200);

	UART_EnableInt(UART2, UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);// | UART_INTEN_THREIEN_Msk);//接收超时中断
	NVIC_EnableIRQ(UART2_IRQn);
	
	UART_Write(UART2, "UART2", sizeof("UART2"));
}


/*
@功能：uart2中断处理函数
*/
#define RXBUFSIZE   256
uint8_t g_u8RecData[RXBUFSIZE]  = {0};

volatile uint32_t g_u32comRbytes = 0;
volatile uint32_t g_u32comRhead  = 0;
volatile uint32_t g_u32comRtail  = 0;
volatile int32_t g_bWait         = TRUE;


void UART2_IRQHandler(void)
{
	uint8_t u8InChar = 0xFF;
	uint32_t u32IntSts = UART2->INTSTS;
	
	if((u32IntSts & UART_INTSTS_RDAINT_Msk) || (u32IntSts & UART_INTSTS_RXTOINT_Msk))
	{
		/* Get all the input characters */
		while(UART_GET_RX_EMPTY(UART2) == 0)
		{
			/* Get the character from UART Buffer */
			u8InChar = UART_READ(UART2);
			if(u8InChar == '0')
			{
					g_bWait = FALSE;
			}

			/* Check if buffer full */
			if(g_u32comRbytes < RXBUFSIZE)
			{
					/* Enqueue the character */
					g_u8RecData[g_u32comRtail] = u8InChar;
					g_u32comRtail = (g_u32comRtail == (RXBUFSIZE - 1)) ? 0 : (g_u32comRtail + 1);
					g_u32comRbytes++;
			}
		}
		printf("\nTransmission Test:");
	}

	if(u32IntSts & UART_INTSTS_THREINT_Msk)
	{
		uint16_t tmp;
		tmp = g_u32comRtail;
		if(g_u32comRhead != tmp)
		{
			u8InChar = g_u8RecData[g_u32comRhead];
			while(UART_IS_TX_FULL(UART2));  /* Wait Tx is not full to transmit data */
			UART_WRITE(UART2, u8InChar);
			g_u32comRhead = (g_u32comRhead == (RXBUFSIZE - 1)) ? 0 : (g_u32comRhead + 1);
			g_u32comRbytes--;
		}
	}

	if(UART2->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
	{
		UART2->FIFOSTS = (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk);
	}
}
