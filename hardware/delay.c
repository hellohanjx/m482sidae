#include "delay.h"
#include "timer.h"
#include "timer_reg.h"

#include "FreeRTOS.h"
#include "task.h"

/*
@���ܣ���ʱ������ʼ��
*/
void _delay_init(void)
{
	CLK_EnableModuleClock(TMR3_MODULE);
  CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);//������ʱ��ʱ��
	TIMER3->CTL = TIMER_CONTINUOUS_MODE | 95;//��ʱ������ģʽ���Ƶ��������ʹ��PCLK1����Ƶ��96Mhz��/����ʹ��HIRC����Ƶ��12MHz����ʹ��12��Ƶ������ʱ�ӣ� 1200000/12 = 1000000
	TIMER3->CMP = 10;
	TIMER_Start(TIMER3);
}


/*
@���ܣ���ʱ��������λuS
@������time����ʱʱ��
@˵�������ж���ʱ
@ע�⣺time ���ֵ 0xffffff/2
*/
#if(1)
void delay_us(uint32_t time)
{
	uint32_t recd;
	if(time == 0)
		return ;
	
	recd = TIMER3->CNT;
	while( !((TIMER3->CNT - recd >= time) || (recd > TIMER3->CNT)) );

	//���μ�ʱ
//	TIMER3->CTL = TIMER_ONESHOT_MODE | 47;//��ʱ������ģʽ���Ƶ��������ʹ��PCLK1����Ƶ��96Mhz��/����ʹ��HIRC����Ƶ��12MHz����ʹ��12��Ƶ������ʱ�ӣ� 1200000/12 = 1000000
//	TIMER3->CMP = time*2;
//	TIMER_Start(TIMER3);
//	while( !TIMER3->INTSTS & TIMER_INTSTS_TIF_Msk );
//	TIMER3->INTSTS = 1;//��� TIF��־
}
#else
void delay_us(uint32_t time)
{
	uint32_t i;
	for(i = 0; i < 192; i++);
}
#endif


/*
@���ܣ���ʱ��������λmS
@������time����ʱʱ��
@˵�������ж���ʱ
@ע�⣺��freertosʱ�ӱȽϣ�ÿ��ʱ100ms��freertosʱ�ӱ���ʱ��ʱ��1ms
*/
void delay_ms(uint32_t time)
{
	uint32_t i;
	if(time == 0)
		return ;
	for(i = 0; i < time; i++)
	{
		delay_us(1000);
	}
	
//���μ�ʱ
//	TIMER3->CTL = TIMER_ONESHOT_MODE | 95;//��ʱ������ģʽ���Ƶ��������ʹ��PCLK1����Ƶ��96Mhz��/����ʹ��HIRC����Ƶ��12MHz����ʹ��12��Ƶ������ʱ�ӣ� 1200000/12 = 1000000
//	for(i = 0; i < time; i++ )
//	{
//		TIMER3->CMP = 1000;
//		TIMER_Start(TIMER3);
//		while( !TIMER3->INTSTS & TIMER_INTSTS_TIF_Msk );
//		TIMER3->INTSTS = 1;//��� TIF��־
//	}
}
