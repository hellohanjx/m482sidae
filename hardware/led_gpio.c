#include "led_gpio.h"
//#include "M480.h"	//@ע�⣺���������Ҫ��"gpio.h"֮ǰ
#include "gpio.h"

/*
@���ܣ�led��ʼ��
*/
void led_config(void)
{
	GPIO_SetMode(PF, BIT4, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PF, BIT5, GPIO_MODE_OUTPUT);
	PF4 = 1;
	PF5 = 1;
}
