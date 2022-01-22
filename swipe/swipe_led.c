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
	PB10 = 1;
	PB9 = 0;
	
	//uart2
	GPIO_SetMode(PA, BIT11, GPIO_MODE_OUTPUT);//PA11-BLUE
	GPIO_SetMode(PB, BIT6, GPIO_MODE_OUTPUT);//PB6-RED
	PA11 = 1;
	PB6 = 0;
	
	//uart3
	GPIO_SetMode(PA, BIT7, GPIO_MODE_OUTPUT);//PA7-BLUE
	GPIO_SetMode(PD, BIT3, GPIO_MODE_OUTPUT);//PD3-RED
	PA7 = 1;
	PD3 = 0;
	
	//uart4
	GPIO_SetMode(PF, BIT6, GPIO_MODE_OUTPUT);//PF6-BLUE
	GPIO_SetMode(PA, BIT8, GPIO_MODE_OUTPUT);//PA8-RED
	PF6 = 1;
	PA8 = 0;
	
	//uart5
	GPIO_SetMode(PC, BIT14, GPIO_MODE_OUTPUT);//PC14-BLUE
	GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);//PB11-RED
	PC14 = 1;
	PB11 = 0;
	
	//uart6
	GPIO_SetMode(PA, BIT9, GPIO_MODE_OUTPUT);//PA9-BLUE
	GPIO_SetMode(PA, BIT10, GPIO_MODE_OUTPUT);//PA10-RED
	PA9 = 1;
	PA10 = 0;
}


/*
@功能：开关刷卡器指示灯
@参数：channel，通道0~5；type=1红灯，type=0蓝灯；state=1开led，state = 0关led
*/
void swip_led_set(uint8_t channel, uint8_t type, uint8_t state)
{
	switch(channel)
	{
		case 0:
			if(type){
				SWIPE1_LED_RED = state;
			}else{
				SWIPE1_LED_BLUE = state;
			}
		break;
		
		case 1:
			if(type){
				SWIPE2_LED_RED = state;
			}else{
				SWIPE2_LED_BLUE = state;
			}
		break;

		case 2:
			if(type){
				SWIPE3_LED_RED = state;
			}else{
				SWIPE3_LED_BLUE = state;
			}
		break;

		case 3:
			if(type){
				SWIPE4_LED_RED = state;
			}else{
				SWIPE4_LED_BLUE = state;
			}
		break;

		case 4:
			if(type){
				SWIPE5_LED_RED = state;
			}else{
				SWIPE5_LED_BLUE = state;
			}
		break;

		case 5:
			if(type){
				SWIPE6_LED_RED = state;
			}else{
				SWIPE6_LED_BLUE = state;
			}
		break;		
	}
}


/*
@功能：控制刷卡器两个指示灯的亮灭
*/
void swip_led_two(uint8_t channel, uint8_t blue, uint8_t red)
{
	switch(channel)
	{
		case 0:
				SWIPE1_LED_RED = blue;
				SWIPE1_LED_BLUE = red;
		break;
		
		case 1:
				SWIPE2_LED_RED = blue;
				SWIPE2_LED_BLUE = red;
		break;

		case 2:
				SWIPE3_LED_RED = blue;
				SWIPE3_LED_BLUE = red;
		break;

		case 3:
				SWIPE4_LED_RED = blue;
				SWIPE4_LED_BLUE = red;
		break;

		case 4:
				SWIPE5_LED_RED = blue;
				SWIPE5_LED_BLUE = red;
		break;

		case 5:
				SWIPE6_LED_RED = blue;
				SWIPE6_LED_BLUE = red;
		break;		
	}	
}
