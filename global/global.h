#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "FreeRTOSConfig.h"
#include "user_config.h"

#define ONE_SECOND	configTICK_RATE_HZ

/****************************************************************************************************************
@说明：全局变量结构
****************************************************************************************************************/
typedef struct CLASS_GLOBAL
{
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
	
}CLASS_GLOBAL;






/****************************
声明
****************************/
extern volatile CLASS_GLOBAL class_global;

void restart_equ_set(uint8_t val, uint8_t type);

#endif
