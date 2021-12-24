#include "tap_set.h"


/*
@功能：led初始化
*/
void _tap_config(void)
{
	GPIO_SetMode(PD, BIT1, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PD, BIT2, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PA, BIT14, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PA, BIT13, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PD, BIT0, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PA, BIT12, GPIO_MODE_OUTPUT);
	
	PD1 = 0;
	PD2 = 0;
	PA14 = 0;
	PA13 = 0;
	PD0 = 0;
	PA12 = 0;
}
