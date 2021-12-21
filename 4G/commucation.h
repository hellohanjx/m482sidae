#ifndef _COMMUCATION_H_
#define _COMMUCATION_H_

#include "M480.h"
#include "user_config.h"

/*
0x12-0x32
@发送数据类型
*/
#define	TYPE_TEMP				0x31	//温度
#define TYPE_HUMODOTY		0x32	//湿度
#define TYPE_POWER			0x33	//功耗



















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
void main_task_commucation(void);
void instant_equipment_state(uint8_t type, uint8_t state, uint32_t value);



#endif
