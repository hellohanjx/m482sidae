#include "sys_clock.h"

/*
@���ܣ�����ϵͳʱ��
*/
void sys_clock_init( void )
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
//    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);//ʹ���ڲ����پ���
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);//ʹ���ڲ����پ���

		SYS->IRCTCTL |= SYS_IRCTCTL_REFCKSEL_Msk;//12MHz����У׼	
//SYS->HIRCTCTL |= SYS_HIRCTCTL_REFCKSEL_Msk;//48MHz����У׼	
	
    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);//�ȴ��ڲ����پ���׼����
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);//�ȴ��ڲ����پ���׼����

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2;

    /* Enable IP clock */
    CLK_EnableModuleClock(TMR0_MODULE);//ʹ�ܼ�ʱ��0ʱ��

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);//ѡ��ʱ��0ʱ��Դ

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

//		SPIM->CTL1 &= ~(1<<2);//�ر�CCMEN
    /* Lock protected registers */
    SYS_LockReg();
}
