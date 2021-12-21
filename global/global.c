#include "global.h"
#include "string.h"

volatile char main_version[] = {"3XXXX-211208"};//版本号






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

/*
@功能：获取复位标志位
@返回值：复位值
*/
uint8_t restart_equ_get(void)
{
//	if(reset_flag_1 == reset_flag_2 && reset_flag_2 == reset_flag_3)
//		return reset_flag_1;
//	else
		return RESET_PowerOn;
}



/*
@功能：获取一个真随机数
@返回：随机数
*/
uint32_t get_random_number(void)
{
	uint32_t random;
	
//	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);   //使能RNG时钟
//	RNG_Cmd(ENABLE);       																//使能RNG外设
//	
//	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET);  		//等待转换结束
//	random = RNG_GetRandomNumber();    //提取随机数
//	
//	RNG_Cmd(DISABLE);																			//关闭外设
//	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, DISABLE);  //关闭RNG时钟
	
	return random;
}




/*
@功能：全局变量初始化
*/
void global_init(void)
{
	class_global.net.serverPort = 8001;//端口号
	strcpy((char*)class_global.net.arr_ip, (const char*)DEF_HEIM_IP);//ip
	
	class_global.net.id = 1000000006;//机器id
	class_global.net.password = 0;//联机密码
	
	class_global.sys.factory_en = 1;//工厂模式
	class_global.trade.price = 1;//价格
	
}

