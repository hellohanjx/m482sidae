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

//刷卡器工作状态
enum CARD_WORK_STATE{IDLE, BUSY, WORKING};


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
		uint32_t unique_id[3];//芯片唯一id
	}sys;
	
	/****************
	@说明：交易类
	****************/
	struct trade //交易相关
	{
		uint8_t type; //交易类型：0现金 1 IC卡 2 手机支付
		uint8_t fsm[6];//支付流程状态机
		uint32_t number;//交易号
		uint32_t min_balance;//卡最小余额
		uint32_t plus_per_1L[6];//1L水脉冲数，最多6个刷卡器
		uint32_t price_per_1L[6];//1L水脉价格，最多6个刷卡器
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
	@说明：温度
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
	
	/****************
	@说明：卡系统，包含读卡器，水阀，信号
	****************/
	struct ireader
	{
		struct{//读卡设备
			uint8_t 		state;			//刷卡器状态
			uint8_t 		type;				//刷卡器类型
			enum CARD_WORK_STATE work;//工作状态
			uint32_t 		interface;	//刷卡接口
		}equ;
		
		struct{//显示
			uint32_t state;//0=无需要显示，1=需要显示
			uint32_t start_time;//记录开始显示时间
			uint32_t hold_time;//记录显示时间
		}display;
		
		struct{//卡片
			uint8_t user;	//用户身份识别(1管理员)
			uint8_t useRang;//使用范围
			
			char   physic_char[CARD_PHY_LEN];		//物理卡号-字符型
			char   logic_char[CARD_LOGIC_LEN];	//逻辑卡号-字符型
			uint8_t   physic_hex[4];    //物理卡号hex
			uint8_t		logic_hex[4];			//逻辑卡号hex
			
			uint8_t   type;					//卡类型
			uint8_t   subsype;			//卡子类型
			uint8_t   status; 		 	//卡状态
			uint8_t		daymaxpay;		//当天最大消费次数
			uint32_t 	balance;			//卡余额
			uint32_t 	trade_num;		//交易号
		}card;
		
		struct{
			uint8_t state;//水阀设备状态，好/坏
		}tap;
		
		struct{
			uint8_t state;//计数状态
			uint32_t val;//计数值
		}count[2];//2个信号线：主信号计数，副信号计数
		
		struct{
			char physic_char[CARD_PHY_LEN];	//物理卡号-字符型
			uint32_t trade_num;						//交易号
			uint32_t time;								//计时
		}working;//工作备份
		
	}ireader[6];

	
}CLASS_GLOBAL;




/****************************
声明
****************************/
extern volatile CLASS_GLOBAL class_global;
extern volatile char main_version[];

void restart_equ_set(uint8_t val, uint8_t type);
uint8_t restart_equ_get(void);

uint32_t get_random_number(void);
void global_init(void);

#endif
