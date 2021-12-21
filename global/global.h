#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "FreeRTOSConfig.h"
#include "user_config.h"

#define ONE_SECOND	configTICK_RATE_HZ

extern volatile char main_version[];//版本号

//开关方式
enum SWITCH_MODE{SWITCH_OFF, SWITCH_ON};

//工厂模式
enum FACTOTY_MODE{USER_MODE, FACTORY_COLD, FACTORY_HOT, FACTORY_CLOSE_TEMP, FACTORY_CLOSE_ALL, FACTORY_AUTO, FACTORY_AUTO_COLD, FACTORY_AUTO_HOT};

/****************************************************************************************************************
@说明：全局变量结构
****************************************************************************************************************/
typedef struct CLASS_GLOBAL
{
	/****************
	@说明：系统设置类
	****************/
	struct sys 
	{
		uint8_t point_bit;	//小数点位数（开机读读设置,各种输入长度，以及金额输入长度）
		uint8_t price_bit;//价格位数
		enum FACTOTY_MODE factory_en;	//1~5工厂模式；0为用户模式
		enum SWITCH_MODE reset;//重启标志
	}sys;
	
	/****************
	@说明：交易类
	****************/
	struct trade //交易相关
	{
		uint8_t type; //交易类型：0现金 1 IC卡 2 手机支付
		uint32_t price;//价格
		uint32_t number;//交易号
		uint32_t max_price;	//最大价格
	}trade;	
	
	/****************
	@说明：网络
	****************/
	struct net
	{
		char state;						//连网状态
		char number;					//通讯流水号
		char arr_ip[IP_LEN];	//ip地址
		char arr_ICCID[ICCID_LEN];//sim卡ICCID
		char updata_flag;			//ip地址。端口更新标志
		uint8_t backlog_data;	//积压数据
		uint16_t password;		//登录密码
		uint16_t serverPort;	//服务器端口号
		uint32_t id;					//登录机器号
	}net;//网络
	
	
	/****************
	@说明：卡系统，包含读卡器
	****************/
	struct ireader
	{
		enum SWITCH_MODE 		en;					//读卡器使能
		uint8_t 						type;				//刷卡器类型
		uint32_t 						interface;	//刷卡接口
	}ireader;
	
	/****************
	@说明：卡系统，包含读卡器
	****************/
	struct{
		//cpu温度
		struct{
			int val;
			uint8_t state;
		}internal;
		
		//外部ntc
		struct{
			int val;
			uint8_t state;
		}external;
	}temp;
	
}CLASS_GLOBAL;




/****************************
声明
****************************/
extern volatile CLASS_GLOBAL class_global;

void restart_equ_set(uint8_t val, uint8_t type);
uint8_t restart_equ_get(void);

uint32_t get_random_number(void);
void global_init(void);

#endif
