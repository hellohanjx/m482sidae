#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


#define C_4G_LOG				0		//4G日志，1=打开；0=关闭
#define GPIO_INT_LOG		0		//gpio中断日志
#define	RC_522_LOG			0		//rc522 日志
#define WATCH_DOG				1		//看门狗；1=开，0=关
#define CPU_INFO				0

#define IREADER_NUM_MAX		6//刷卡器最大数量

#define IP_LEN									16	//ip地址长度
#define	CARD_PHY_LEN						34	//物理卡号长度(字符型)
#define CARD_LOGIC_LEN					34	//逻辑卡号长度(字符型)
#define ICCID_LEN								50	//SIM卡ICCID长度
#define PACK_MAX_SIZE						255	//与平台通信最大数据长度
#define PARAM_REDOWNED_CNT			3		//开机参数重复下载次数

#define DEF_HEIM_IP							"116.62.120.86"//"IP.HMILK.CN"//"121.41.30.42"
#define	DEF_CARD_INTERFACE			1001		//刷卡器接口

/*
@说明：flash存储器参数默认值
*/
//#define DEF_FLASH_Ip						(116 << 24) | (62 << 16) | (120 << 8) | 86	//默认IP地址,是4个ip段拼起来的，左~右，即是 116.62.120.86
//#define DEF_FLASH_Id						1000000000	//机器编号(10位)
//#define DEF_FLASH_Port					8001				//默认端口号
#define DEF_FLASH_PassWord			000000			//联网密码(6位)
#define DEF_FLASH_FactoryEn			0						//默认工厂模式-自动测试：0用户模式，;1:工厂模式
#define DEF_FLASH_PointBit			2						//金额的小数点位数
#define DEF_FLASH_PriceBit			4						//金额位数(不包含小数点)
#define DEF_FLASH_CardType			1						//1=黑莓刷卡器；2=外购刷卡器
#define DEF_FLASH_PriceMax			10000				//最大价格



#define DEF_FLASH_Ip						(121 << 24) | (43 << 16) | (255 << 8) | 207	//默认IP地址,是4个ip段拼起来的，左~右，即是 116.62.120.86
#define DEF_FLASH_Port					5008				//默认端口号
#define DEF_FLASH_Id						1059000000	//机器编号(10位)






/*********************************************
@说明：热复位类型
*********************************************/
enum {
	//即时汇报
	RESET_Init,						//0 = 初始化
	RESET_UpdataTime,			//1 = 对时
	RESET_NormalLink, 		//2 = 不重建socket重新联网
	RESET_RebuitSocket,		//3 = 重建socket
	RESET_4G,							//4 = 重启4G
	
	Reserved_1,//保留位5
	Reserved_2,//保留位6
	Reserved_3,//保留位7
	Reserved_4,//保留位8
	Reserved_5,//保留位9
	
	//重启汇报
	RESET_NormalReset,		//10 = 正常重启
	RESET_PowerOn,				//11 = 上电复位
	RESET_Server, 				//12 = 服务器重启命令
	RESET_Param, 					//13 = 参数错误重启
	RESET_Distribution, 	//14 = 新机器分配id等参数重启
	RESET_Setting,				//15 = 设置复位重启
	RESET_UpdataIP,				//16 = 更新IP地址与端口号
	RESET_HardFault, 			//17 = 死机
	RESET_MemManage, 			//18 = 死机
	RESET_BusFault, 			//19 = 死机
	RESET_UsageFault, 		//20 = 死机
	RESET_WatchDog, 			//21 = 看门狗复位
	RESET_ApplyMail,			//22 = 申请内存失败
	RESET_DriverCmd,			//23 = 申请内存失败
	RESET_MsgApply,				//24 = 申请内存失败
	RESET_CardCmd,				//25 = 申请内存失败
	RESET_MailToBus,			//26 = 申请内存失败
	RESET_LANCmd,					//27 = 申请内存失败
	RESET_FACECmd,				//28 = 申请内存失败
};

#endif
