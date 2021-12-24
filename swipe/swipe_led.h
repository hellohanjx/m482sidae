#ifndef _SWIPE_LED_H_
#define _SWIPE_LED_H_

#include "M480.h"

enum {LED_OPEN, LED_CLOSE};

//电路表字符的刷卡器1
#define SWIPE1_LED_RED				PB11
#define SWIPE1_LED_BLUE				PC14

#define SWIPE2_LED_RED				PB9
#define SWIPE2_LED_BLUE				PB10

#define SWIPE3_LED_RED				PA8
#define SWIPE3_LED_BLUE				PF6

#define SWIPE4_LED_RED				PA10
#define SWIPE4_LED_BLUE				PA9

#define SWIPE5_LED_RED				PB6
#define SWIPE5_LED_BLUE				PA11

#define SWIPE6_LED_RED				PD3
#define SWIPE6_LED_BLUE				PA7

#define SWIPE_LED_BLUE(X)		SWIPE##X##_LED_BLUE
#define SWIPE_LED_RED(X)		SWIPE##X##_LED_RED

void _swipe_led_config(void);

#endif
