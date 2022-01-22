#include "M480.h"
#include "wdt.h"
#include "wacth_dog.h"

/*
@功能：看门狗定时器初始化
*/
void watch_dog_init(void)
{
	SYS_UnlockReg();
	CLK_EnableModuleClock(WDT_MODULE);
	CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);
  WDT_Open(WDT_TIMEOUT_2POW18, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
	SYS_LockReg();
}

/*
@功能：复位看门狗计数器，喂狗
*/
void watch_dog_feed(void)
{
  WDT_RESET_COUNTER();
}
