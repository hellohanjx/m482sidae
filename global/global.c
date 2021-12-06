#include "global.h"


volatile CLASS_GLOBAL class_global;//全局信息



/*
@功能：复位设备/设置标志位
@参数：val，复位类型；type，是否要重启
@说明：利用了电池区，不掉电数据不变
*/
void restart_equ_set(uint8_t val, uint8_t type)
{
//	reset_flag_1 = val;
//	reset_flag_2 = val;
//	reset_flag_3 = val;
//	
//	if(type)//重启
//	{
//		__set_FAULTMASK(1);
//		NVIC_SystemReset();
//	}
}
