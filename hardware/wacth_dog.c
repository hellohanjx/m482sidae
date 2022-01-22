#include "M480.h"
#include "wdt.h"
#include "wacth_dog.h"

/*
@���ܣ����Ź���ʱ����ʼ��
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
@���ܣ���λ���Ź���������ι��
*/
void watch_dog_feed(void)
{
  WDT_RESET_COUNTER();
}
