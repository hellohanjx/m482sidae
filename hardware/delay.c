#include "delay.h"
#include "timer.h"
#include "timer_reg.h"

#include "FreeRTOS.h"
#include "task.h"

/*
@功能：延时函数初始化
*/
void _delay_init(void)
{
	CLK_EnableModuleClock(TMR3_MODULE);
  CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);//开启定时器时钟
	TIMER3->CTL = TIMER_CONTINUOUS_MODE | 95;//定时器工作模式与分频数，【若使用PCLK1，则频率96Mhz】/【若使用HIRC，则频率12MHz】，使用12分频，则工作时钟： 1200000/12 = 1000000
	TIMER3->CMP = 10;
	TIMER_Start(TIMER3);
}


/*
@功能：延时函数，单位uS
@参数：time，延时时间
@说明：无中断延时
@注意：time 最大值 0xffffff/2
*/
#if(1)
void delay_us(uint32_t time)
{
	uint32_t recd;
	if(time == 0)
		return ;
	
	recd = TIMER3->CNT;
	while( !((TIMER3->CNT - recd >= time) || (recd > TIMER3->CNT)) );

	//单次计时
//	TIMER3->CTL = TIMER_ONESHOT_MODE | 47;//定时器工作模式与分频数，【若使用PCLK1，则频率96Mhz】/【若使用HIRC，则频率12MHz】，使用12分频，则工作时钟： 1200000/12 = 1000000
//	TIMER3->CMP = time*2;
//	TIMER_Start(TIMER3);
//	while( !TIMER3->INTSTS & TIMER_INTSTS_TIF_Msk );
//	TIMER3->INTSTS = 1;//清除 TIF标志
}
#else
void delay_us(uint32_t time)
{
	uint32_t i;
	for(i = 0; i < 192; i++);
}
#endif


/*
@功能：延时函数，单位mS
@参数：time，延时时间
@说明：无中断延时
@注意：与freertos时钟比较，每延时100ms，freertos时钟比延时计时多1ms
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
	
//单次计时
//	TIMER3->CTL = TIMER_ONESHOT_MODE | 95;//定时器工作模式与分频数，【若使用PCLK1，则频率96Mhz】/【若使用HIRC，则频率12MHz】，使用12分频，则工作时钟： 1200000/12 = 1000000
//	for(i = 0; i < time; i++ )
//	{
//		TIMER3->CMP = 1000;
//		TIMER_Start(TIMER3);
//		while( !TIMER3->INTSTS & TIMER_INTSTS_TIF_Msk );
//		TIMER3->INTSTS = 1;//清除 TIF标志
//	}
}
