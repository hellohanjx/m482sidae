#include "sys_clock.h"

/*
@功能：配置系统时钟
*/
void sys_clock_init( void )
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
//    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);//使能内部高速晶振
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);//使能内部低速晶振

		SYS->IRCTCTL |= SYS_IRCTCTL_REFCKSEL_Msk;//12MHz晶振校准	
//SYS->HIRCTCTL |= SYS_HIRCTCTL_REFCKSEL_Msk;//48MHz晶振校准	
	
    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);//等待内部高速晶振准备好
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);//等待内部低速晶振准备好

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2;

    /* Enable IP clock */
    CLK_EnableModuleClock(TMR0_MODULE);//使能计时器0时钟

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);//选择定时器0时钟源

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

//		SPIM->CTL1 &= ~(1<<2);//关闭CCMEN
    /* Lock protected registers */
    SYS_LockReg();
}
