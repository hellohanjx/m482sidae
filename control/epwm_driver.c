#include "epwm_driver.h"

/*
@功能：初始化EPWM
*/
void epwm_config(void)
{
	CLK_EnableModuleClock(EPWM0_MODULE);//开时钟
	CLK_EnableModuleClock(EPWM1_MODULE);//开时钟
	
	CLK_SetModuleClock(EPWM0_MODULE, CLK_CLKSEL2_EPWM0SEL_PLL, NULL);//设置时钟源
	CLK_SetModuleClock(EPWM1_MODULE, CLK_CLKSEL2_EPWM1SEL_PLL, NULL);//设置时钟源
	
	//信号1
	SYS->GPA_MFPL |= SYS_GPA_MFPL_PA5MFP_EPWM0_CH0;//PA5-EPWM0_CH0
	SYS->GPA_MFPL |= SYS_GPA_MFPL_PA4MFP_EPWM0_CH1;//PA4-EPWM0_CH1
	
	//信号2
	SYS->GPA_MFPL |= SYS_GPA_MFPL_PA3MFP_EPWM0_CH2;//PA3-EPWM0_CH2
	SYS->GPA_MFPL |= SYS_GPA_MFPL_PA2MFP_EPWM0_CH3;//PA2-EPWM0_CH3
	
	//信号3
	SYS->GPB_MFPL |= SYS_GPB_MFPL_PB1MFP_EPWM0_CH4;//PB1-EPWM0_CH4
	SYS->GPB_MFPL |= SYS_GPB_MFPL_PB0MFP_EPWM0_CH5;//PB0-EPWM0_CH5
	
	//信号4
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB15MFP_EPWM1_CH0;//PB15-EPWM1_CH0
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB14MFP_EPWM1_CH1;//PB14-EPWM1_CH1
	
	//信号5
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB13MFP_EPWM1_CH2;//PB13-EPWM1_CH2
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB12MFP_EPWM1_CH3;//PB12-EPWM1_CH3

	//信号6
	SYS->GPB_MFPL |= SYS_GPB_MFPL_PB7MFP_EPWM1_CH4;//PB7-EPWM1_CH4
	SYS->GPA_MFPL |= SYS_GPA_MFPL_PA6MFP_EPWM1_CH5;//PA6-EPWM1_CH5
	
	//配置输出通道
	EPWM_ConfigOutputChannel(EPWM1, 2, 300, 30);//设置EWMP输出
  EPWM_EnableOutput(EPWM1, EPWM_CH_2_MASK);//配置输出通道 EPWM1_CH2
	EPWM_Start(EPWM1, EPWM_CH_2_MASK);//EPWM1_CH2 启动
	
	//配置输入通道
	EPWM_ConfigCaptureChannel(EPWM1, 3, 52, 0);//参数3时间单位是nS，EPWM1_CH3
//	EPWM_EnableCaptureInt(EPWM1, 3, EPWM_CAPTURE_INT_FALLING_LATCH);//下降沿捕获
//	NVIC_EnableIRQ(EPWM1P1_IRQn);//使能中断
	EPWM_Start(EPWM1, EPWM_CH_3_MASK);
	EPWM_EnableCapture(EPWM1, EPWM_CH_3_MASK);
	/* Enable falling capture reload */
	EPWM1->CAPCTL |= EPWM_CAPCTL_FCRLDEN2_Msk;

	/* Wait until EPWM1 channel 2 Timer start to count */
	while((EPWM1->CNT[2]) == 0);

	/* Capture the Input Waveform Data */
//	CalPeriodTime(EPWM1, 2);

}

