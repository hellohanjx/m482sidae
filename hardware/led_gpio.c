#include "led_gpio.h"
//#include "M480.h"	//@注意：这个包含需要在"gpio.h"之前
#include "gpio.h"

/*
@功能：led初始化
*/
void led_config(void)
{
	GPIO_SetMode(PF, BIT4, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PF, BIT5, GPIO_MODE_OUTPUT);
	PF4 = 1;
	PF5 = 1;
}
