#include "user_rtc.h"


/*
@功能：获取日期与时间
@返回值：时间结构体
*/
CUR_TIME get_cur_time(void)
{
	CUR_TIME cur;
	S_RTC_TIME_DATA_T time;
	
	RTC_GetDateAndTime(&time);//获取当前时间

	cur.year = time.u32Year - 2000;
	cur.month = time.u32Month;
	cur.week = time.u32DayOfWeek;
	cur.day = time.u32Day;
	cur.hour = time.u32Hour;
	cur.min = time.u32Minute;
	cur.sec = time.u32Second;
	
	return cur;
}


/*
@功能：设置日期/时间
@参数：type = 0只设置时间；type = 1 设置日期+时间
@返回值：TRUE，设置成功；FALSE，设置失败
*/
uint8_t set_rtc_time(uint8_t type, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
	S_RTC_TIME_DATA_T time;
	
	if(year > 100 && type == 1)
		return FALSE;
	if(month < 1 || month > 12)
		return FALSE;
	if(day < 1 || day > 31)
		return FALSE;
	if(hour > 23)
		return FALSE;
	if(min > 59)
		return FALSE;
	if(sec > 59)
		return FALSE;
	
	time.u32Year = year + 2000;
	time.u32Month = month;
	time.u32Day = day;
	time.u32Hour = hour;
	time.u32Minute = min;
	time.u32Second = sec;
	
	RTC_SetDateAndTime(&time);

	return TRUE;
}


/*
@功能：rtc初始化
*/
void rtc_config(void)
{
	S_RTC_TIME_DATA_T sInitTime;
	
	sInitTime.u32Year       = 2021;
	sInitTime.u32Month      = 12;
	sInitTime.u32Day        = 1;
	sInitTime.u32Hour       = 12;
	sInitTime.u32Minute     = 30;
	sInitTime.u32Second     = 0;
	sInitTime.u32DayOfWeek  = RTC_WEDNESDAY;
	sInitTime.u32TimeScale  = RTC_CLOCK_24;

//	CLK->APBCLK0 |= CLK_APBCLK0_RTCCKEN_Msk; // RTC Clock Enable
	CLK_EnableModuleClock(RTC_MODULE);//开RTC时钟
	CLK_SetModuleClock(RTC_MODULE, CLK_CLKSEL3_RTCSEL_LIRC, NULL);//选择RTC时钟
	RTC_Open(&sInitTime);//初始化rtc时间，并开始计时
	
	/* Set Tick Period */
	RTC_SetTickPeriod(RTC_TICK_1_SEC);//设定节拍频率
	
}

