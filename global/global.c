#include "global.h"
#include "string.h"
#include "isp_program.h"

volatile char main_version[] = {"Nu-220122"};//版本号






volatile CLASS_GLOBAL class_global;//全局变量


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
	char tmp[4];
	uint32_t i;
//	uint32_t card_type, factory, point_bit, price_bit, price_max;
	
	judge_param();
	
	class_global.net.id = flash_param_get(FLASH_LIST_Id, FLASH_ADDR_Id);//id
	
	//读取IP
	{
		uint8_t ip[4];
		uint32_t ip_long;
		ip_long = flash_param_get(FLASH_LIST_Ip, FLASH_ADDR_Ip);//ip
		ip[0] = ip_long >> 24;
		ip[1] = ip_long >> 16;
		ip[2] = ip_long >> 8;
		ip[3] = ip_long & 0xff;
		
		for(i = 0; i < IP_LEN; i++)//ip初始化
			class_global.net.arr_ip[i] = 0;
		for(i = 0; i < 4; i++)
		{
			tmp[sprintf(tmp, "%u", ip[i])] = 0;
			strcat((char*)class_global.net.arr_ip, tmp);
			if(i != 3)
				strcat((char*)class_global.net.arr_ip, (char*)".");
		}
	}
	
	class_global.net.serverPort = flash_param_get(FLASH_LIST_Port, FLASH_ADDR_Port);//端口号
//	card_type = flash_param_get(FLASH_LIST_CardType, FLASH_ADDR_CardType);//卡类型
//	point_bit = flash_param_get(FLASH_LIST_PointBit, FLASH_ADDR_PointBit);//小数
//	price_bit = flash_param_get(FLASH_LIST_PriceBit, FLASH_ADDR_PriceBit);//价格位数
//	price_max = flash_param_get(FLASH_LIST_PriceMax, FLASH_ADDR_PriceMax);//最大价格
	
	class_global.net.password = DEF_FLASH_PassWord;//联机密码
	
	class_global.sys.factory_en = (enum FACTOTY_MODE)0;//(enum FACTOTY_MODE)flash_param_get(FLASH_LIST_FactoryEn, FLASH_ADDR_FactoryEn);//工厂模式
	
	for(i = 0; i < 6; i++)//初始赋值
	{
		class_global.ireader[i].equ.interface = DEF_CARD_INTERFACE;//刷卡器接口
		class_global.ireader[i].equ.state = 0xff;//刷卡器状态
		class_global.ireader[i].count[0].state = 0xff;//脉冲计数信号1状态
		class_global.ireader[i].count[1].state = 0xff;//脉冲计数信号2状态
	}
}

