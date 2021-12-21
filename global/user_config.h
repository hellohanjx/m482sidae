#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


#define C_4G_LOG	0	//4G日志，1=打开；0=关闭














#define PACK_MAX_SIZE						255		//与平台通信最大数据长度

#define ICCID_LEN								50		//SIM卡ICCID长度

#define IP_LEN									16		//ip地址长度

#define PARAM_REDOWNED_CNT			3			//开机参数重复下载次数

#define DEF_HEIM_IP							"116.62.120.86"//"IP.HMILK.CN"//"121.41.30.42"
#define DEF_HEIM_PORT						8001	//默认端口号



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
