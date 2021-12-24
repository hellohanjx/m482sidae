#ifndef _ISP_PROGRAM_H_
#define _ISP_PROGRAM_H_

#include "M480.h"

/*
@说明：m482可以块[4096 Byte]擦除，用户数据使用了 APROM 最后一块，地址 0x7F000~0x80000
			为了防止APROM磨损，采用均衡磨损方式
			数据分两部分：
				1.数据表：每项数据有一个表头，表示对应的数据存储情况，每项数据 4Byte 表头，表头值=0xFFFFFFFF，对应32bit，每一个bit表示当前数据值的位置
				2.数据值：数据表 32bit 每一个 bit 对应一个数据值，每种数据共32个数据值；每个数据值固定 4Byte
			计算得：每钟数据占用固定132Byte，一块4096Byte，共存储4096/132 = 31.03，除去表头即是30种数据
*/

#define USR_DATA_BASE_ADDR			0x7F000		//用户数据基地址

/**************************************
@标志位
**************************************/
#define FLASH_VAL_FLAG				0XAA				//flash标志
#define FLASH_ADDR_FLAG				0x7F000			//flash标志位地址【1Byte】


#define FLASH_PARAM_NUM			32//每个偏移表中数据类型对应的此类型数据数量
#define FLASH_LIST_BASE			0x7F004	//偏移【表的基址】{0x7F004 ~ 0x7F07C}
#define FLASH_VAL_BASE			0x7F07C	//参数【值的基址】{0x7F07C ~ 0x80000}

/****************************************************
@偏移表，32bit，每一位对应一个值
@说明：这些都是初始化必须要用的，联网后用的不需要保存
****************************************************/
#define FLASH_LIST_Id								0x7F004	//1--机器ID【偏移地址，对应32个ID地址】
#define FLASH_LIST_Ip								0x7F008	//2--ip地址
#define	FLASH_LIST_Port							0x7F00C	//3--端口号
#define FLASH_LIST_FactoryEn				0x7F010	//4--工厂模式
#define FLASH_LIST_CardType					0x7F014	//5--刷卡器类
#define FLASH_LIST_PointBit					0x7F018	//6--小数位数
#define FLASH_LIST_PriceBit					0x7F01C	//7--价格位数
#define FLASH_LIST_PriceMax					0x7F020 //8--最大价格




/****************************************************
@参数值，参数值地址
****************************************************/
#define	FLASH_ADDR_Id							FLASH_VAL_BASE	//机器ID值首地址，

#define	FLASH_ADDR_Ip							(4*FLASH_PARAM_NUM + FLASH_ADDR_Id) //机器ip值首地址，

#define FLASH_ADDR_Port						(4*FLASH_PARAM_NUM + FLASH_ADDR_Ip)//端口号

#define FLASH_ADDR_FactoryEn			(4*FLASH_PARAM_NUM + FLASH_ADDR_Port)//工厂模式

#define FLASH_ADDR_CardType				(4*FLASH_PARAM_NUM + FLASH_ADDR_FactoryEn)//刷卡器类型

#define FLASH_ADDR_PointBit				(4*FLASH_PARAM_NUM + FLASH_ADDR_CardType)//小数点

#define FLASH_ADDR_PriceBit				(4*FLASH_PARAM_NUM + FLASH_ADDR_PointBit)//价格位数

#define FLASH_ADDR_PriceMax				(4*FLASH_PARAM_NUM + FLASH_ADDR_PriceBit)//最大价格






uint8_t _isp_config(void);
uint32_t flash_param_get(uint32_t list_addr, uint32_t data_addr);
uint8_t flash_param_set(uint32_t list_addr, uint32_t data_addr, uint32_t val);
uint8_t reset_param_block(void);

#endif
