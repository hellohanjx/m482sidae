#ifndef _communication_H_
#define _communication_H_

#include "M480.h"
#include "user_config.h"
#include "msg.h"

/***********************************************************
@说明：宏定义一些通信数据类型/包头
***********************************************************/
/*
0x12-0x32
@发送数据类型
*/
#define	TYPE_TEMP				0x31	//温度
#define TYPE_HUMODOTY		0x32	//湿度
#define TYPE_POWER			0x33	//功耗


/*
0x17-0x41
@交易数据类型
*/
#define ONLINE_CARD_PAY_CMD				0x31	//申请扣款
#define ONLINE_CARD_CHECK_CMD			0x32	//查余额
#define ONLINE_CARD_PAY_SUCCESS		0x33	//刷卡成功
#define ONLINE_CARD_PAY_FAILURE		0x34	//刷卡失败
#define ONLINE_RECHARGE_REPORT		0x35	//充值上报







void main_task_communication(void);
void get_link_info(uint8_t *str, uint8_t type);
void get_reset_cmd(uint8_t* str);
uint8_t get_set_param(uint8_t* str, uint8_t type);
uint8_t analysis_17_42_7(uint8_t *buf, uint8_t len);
uint8_t analysis_17_42_8(uint8_t *buf, uint8_t len);
uint8_t analysis_17_42_9(uint8_t *buf, uint8_t len);
void get_reset_cmd(uint8_t* str);
uint8_t updata_factory_set(uint8_t *rx, uint16_t rx_len, uint8_t *tx);
void get_software_version( uint8_t* str);
void get_vm_set(volatile uint8_t* str);
void get_channel_status( uint8_t* str);
void get_machine_status( uint8_t* str);
uint8_t get_factory_set(uint8_t *str, uint8_t type);
void instant_equipment_state(uint8_t type, uint8_t state, uint32_t value);
void requset_card_trade(uint8_t id, uint8_t type, uint32_t trade_num);
void report_to_communication(MAIL* mymail, uint8_t type);



#endif
