/*
@说明：刷卡器指示灯
*/
#include "swipe_led.h"
#include "gpio.h"

/*
@功能：刷卡器led引脚配置
*/
void _swipe_led_config(void)
{
	//uart1
	GPIO_SetMode(PB, BIT10, GPIO_MODE_OUTPUT);//PB10-BLUE
	GPIO_SetMode(PB, BIT9, GPIO_MODE_OUTPUT);//PB9-RED
	PB10 = 0;
	PB9 = 0;
	
	//uart2
	GPIO_SetMode(PA, BIT11, GPIO_MODE_OUTPUT);//PA11-BLUE
	GPIO_SetMode(PB, BIT6, GPIO_MODE_OUTPUT);//PB6-RED
	PA11 = 0;
	PB6 = 0;
	
	//uart3
	GPIO_SetMode(PA, BIT7, GPIO_MODE_OUTPUT);//PA7-BLUE
	GPIO_SetMode(PD, BIT3, GPIO_MODE_OUTPUT);//PD3-RED
	PA7 = 0;
	PD3 = 0;
	
	//uart4
	GPIO_SetMode(PF, BIT6, GPIO_MODE_OUTPUT);//PF6-BLUE
	GPIO_SetMode(PA, BIT8, GPIO_MODE_OUTPUT);//PA8-RED
	PF6 = 0;
	PA8 = 0;
	
	//uart5
	GPIO_SetMode(PC, BIT14, GPIO_MODE_OUTPUT);//PC14-BLUE
	GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);//PB11-RED
	PC14 = 0;
	PB11 = 0;
	
	//uart6
	GPIO_SetMode(PA, BIT9, GPIO_MODE_OUTPUT);//PA9-BLUE
	GPIO_SetMode(PA, BIT10, GPIO_MODE_OUTPUT);//PA10-RED
	PA9 = 0;
	PA10 = 0;
}
