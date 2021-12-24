#ifndef _LED_GPIO_
#define _LED_GPIO_

#include "M480.h"	//@注意：这个包含需要在"gpio.h"之前

#define LED_BREATH		PF4
#define LED_NET				PF5

void _led_config(void);


#endif
