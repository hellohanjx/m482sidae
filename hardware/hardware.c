#include "hardware.h"	//@ע�⣺���������Ҫ��"gpio.h"֮ǰ


/*
@���ܣ��������ų�ʼ��
*/
static void pin_config(void)
{
	//A����
	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk| SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk | SYS_GPA_MFPL_PA6MFP_Msk
									| SYS_GPA_MFPL_PA7MFP_Msk);
	SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA8MFP_Msk | SYS_GPA_MFPH_PA9MFP_Msk| SYS_GPA_MFPH_PA10MFP_Msk | SYS_GPA_MFPH_PA11MFP_Msk | SYS_GPA_MFPH_PA12MFP_Msk | SYS_GPA_MFPH_PA13MFP_Msk | SYS_GPA_MFPH_PA14MFP_Msk
									| SYS_GPA_MFPH_PA15MFP_Msk);
	//B����
	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk| SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | SYS_GPB_MFPL_PB6MFP_Msk
									| SYS_GPB_MFPL_PB7MFP_Msk);
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk| SYS_GPB_MFPH_PB10MFP_Msk | SYS_GPB_MFPH_PB11MFP_Msk | SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk | SYS_GPB_MFPH_PB14MFP_Msk
									| SYS_GPB_MFPH_PB15MFP_Msk);
	//C����
	SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk| SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC3MFP_Msk | SYS_GPC_MFPL_PC4MFP_Msk | SYS_GPC_MFPL_PC5MFP_Msk | SYS_GPC_MFPL_PC6MFP_Msk
									| SYS_GPC_MFPL_PC7MFP_Msk);
	SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC8MFP_Msk | SYS_GPC_MFPH_PC9MFP_Msk| SYS_GPC_MFPH_PC10MFP_Msk | SYS_GPC_MFPH_PC11MFP_Msk | SYS_GPC_MFPH_PC12MFP_Msk | SYS_GPC_MFPH_PC13MFP_Msk | SYS_GPC_MFPH_PC14MFP_Msk
									| SYS_GPC_MFPH_PC15MFP_Msk);
	//D����
	SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD0MFP_Msk | SYS_GPD_MFPL_PD1MFP_Msk| SYS_GPD_MFPL_PD2MFP_Msk | SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD4MFP_Msk | SYS_GPD_MFPL_PD5MFP_Msk | SYS_GPD_MFPL_PD6MFP_Msk
									| SYS_GPD_MFPL_PD7MFP_Msk);
	SYS->GPD_MFPH &= ~(SYS_GPD_MFPH_PD8MFP_Msk | SYS_GPD_MFPH_PD9MFP_Msk| SYS_GPD_MFPH_PD10MFP_Msk | SYS_GPD_MFPH_PD11MFP_Msk | SYS_GPD_MFPH_PD12MFP_Msk | SYS_GPD_MFPH_PD13MFP_Msk | SYS_GPD_MFPH_PD14MFP_Msk
									| SYS_GPD_MFPH_PD15MFP_Msk);
	//E����
	SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE0MFP_Msk | SYS_GPE_MFPL_PE1MFP_Msk| SYS_GPE_MFPL_PE2MFP_Msk | SYS_GPE_MFPL_PE3MFP_Msk | SYS_GPE_MFPL_PE4MFP_Msk | SYS_GPE_MFPL_PE5MFP_Msk | SYS_GPE_MFPL_PE6MFP_Msk
									| SYS_GPE_MFPL_PE7MFP_Msk);
	SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE8MFP_Msk | SYS_GPE_MFPH_PE9MFP_Msk| SYS_GPE_MFPH_PE10MFP_Msk | SYS_GPE_MFPH_PE11MFP_Msk | SYS_GPE_MFPH_PE12MFP_Msk | SYS_GPE_MFPH_PE13MFP_Msk | SYS_GPE_MFPH_PE14MFP_Msk
									| SYS_GPE_MFPH_PE15MFP_Msk);
	
	//F����(PF1��PF0��SWD�ڣ�û�г�ʼ��)
	SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF2MFP_Msk | SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF4MFP_Msk | SYS_GPF_MFPL_PF5MFP_Msk | SYS_GPF_MFPL_PF6MFP_Msk
									| SYS_GPF_MFPL_PF7MFP_Msk);
	SYS->GPF_MFPH &= ~(SYS_GPF_MFPH_PF8MFP_Msk | SYS_GPF_MFPH_PF9MFP_Msk| SYS_GPF_MFPH_PF10MFP_Msk | SYS_GPF_MFPH_PF11MFP_Msk | SYS_GPF_MFPH_PF12MFP_Msk | SYS_GPF_MFPH_PF13MFP_Msk | SYS_GPF_MFPH_PF14MFP_Msk
									| SYS_GPF_MFPH_PF15MFP_Msk);
}

								
/*
@���ܣ�Ӳ������
*/
void hardware_config(void)
{
	pin_config();
	led_config();
	tap_config();
	_4g_config();
	uart0_log_config();
	uart1_config();
	uart2_config();
	uart3_config();
	uart4_config();
	uart5_config();
	uart6_config();
	uart7_config();
	
}

