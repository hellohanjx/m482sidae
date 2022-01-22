#include "tap_set.h"


/*
@功能：水阀控制引脚初始化
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




/*
@功能：水阀开关
*/
void tap_set(uint8_t channel, uint8_t mode)
{
	switch(channel)
	{
		case 0:
			TAP1 = mode;
		break;

		case 1:
			TAP2 = mode;
		break;

		case 2:
			TAP3 = mode;
		break;

		case 3:
			TAP4 = mode;
		break;

		case 4:
			TAP5 = mode;
		break;

		case 5:
			TAP6 = mode;
		break;		
	}
}
