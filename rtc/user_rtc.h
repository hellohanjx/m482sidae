#ifndef _USER_RTC_H_
#define _USER_RTC_H_

#include "M480.h"

/*
@Ê±¼ä
*/
typedef struct CUR_TIME
{
	uint16_t year;
	uint8_t month;
	uint8_t week;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
}CUR_TIME;


CUR_TIME get_cur_time(void);
uint8_t set_rtc_time(uint8_t type, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
void rtc_config(void);


#endif
