/*
连网登录包
*/
#include "global.h"
#include "string.h"
#include "4G.h"
#include "user_rtc.h"
#include "stdlib.h"


//注：结构分隔符 * ; 包体内分隔符 '+'
/*
@功能：登录包，提供机器id，联机密码，连接类型
@参数：str，数据缓冲指针；type，联机类型，0-上电登录包，1-对时联机包
*/
void get_link_info(uint8_t *str, uint8_t type)
{
	char tmp[10], len, level, signal;
	uint8_t i, j;

	i = 0;
	str[i++]=0x1f;
	str[i++]='*';
	str[i++]='*';
	str[i++]=0x30;
	str[i++]='*';//分隔符
	
	//联机ID
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0;j < 10; j++)
	{
		str[i++]=tmp[j];
	}
	str[i++]='*';//分隔符
	
	//联机密码
	sprintf(tmp, "%06u", class_global.net.password);
	for(j = 0; j < 6; j++)
	{
		str[i++]=tmp[j];
	}
	str[i++]='*';//分隔符
		
	//联机状态：第一次上电或者普通联机
	sprintf(tmp, "%04u", type);
	for(j = 0; j < 4; j++)
	{
		str[i++]=tmp[j];
	}
	
	//信号格数
	str[i++] = '+';
	{
		level = get_4g_quality((uint8_t*)&signal, NULL);
		if(level == 2)
			str[i++] = '1'; //1格信號圖標
		else
		if(level == 3)
			str[i++] = '2'; //2格信號圖標
		else
		if(level == 4)
			str[i++] = '3'; //3格信號圖標
		else
			str[i++] = '0'; //无信號圖標
	}
	//信号分贝
	str[i++] = '+';
	len = sprintf(tmp, "%u", signal);
	tmp[len] = 0;
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	
	//2线电机信号数
	str[i++] = '+';
	len = sprintf(tmp, "%u", 10);
	tmp[len] = 0;
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];

	//复位状态
	str[i++] = '+';
	if(type == 1)//对时包
	{
		str[i++] = RESET_UpdataTime + '0';
	}
	else//联机包
	{
		len = sprintf(tmp, "%u", restart_equ_get());//1~8各种重启，10复位按键或者断电，0联机包
		tmp[len] = 0;
		for(j = 0; j < len; j++)
			str[i++] = tmp[j];
		printf("reset == ");
		printf(tmp);
		printf("\r\n\r\n");
	}
	
	str[i++]='*'; 
	str[2] = i+1;
}
//=============================
//登录包2
//机器货道状态
//=============================
void get_channel_status( uint8_t* str)
{
	char tmp[10];
	uint16_t i, j, len;
	uint8_t container, layer;
	uint8_t validColumnNum;	//使能货道数量
	uint8_t errColumnNum;	//故障货道数量

	container = 0;
//	layer = 8;
	
	i=0;
	str[i++]=0x1f;
	str[i++]='*';
	str[i++]='*'; 
	str[i++]=0x31;
	str[i++]='*';
	//机柜号
	str[i++] = container + '1';
	str[i++] = '*';
	//层总数
	str[i++] = 1 + '0';
	str[i++] = '*';
//	for(container = 0; container < class_global.vm.construction.max_container ; container++)
	/**************************
	@注意：如果是多机柜时，这里需要与平台讨论如何发数据
	**************************/
	for(container = 0; container < 1 ; container++)
	{
		for(layer = 0; layer < 6; layer++)//一次传输所有层信息(可更改) 
		{
			//查询当前层
			validColumnNum = 0;
			errColumnNum = 0;
			
			//读取货道使能状态
			
			//层号
			len = sprintf(tmp,"%u",(layer+1));
			for(j = 0;j < len ;j++)
			str[i++] = tmp[j];
			str[i++] = '*';
			//当前层货道数量（使能货道）
			len = sprintf(tmp,"%u",validColumnNum);
			for(j = 0;j < len ;j++)
			str[i++] = tmp[j];
			str[i++] = '*';
			//状态（无故障货道为0，1个故障为1...）
			len = sprintf(tmp,"%u",errColumnNum);
			for(j = 0;j < len ;j++)
			str[i++] = tmp[j];
			str[i++] = '*';
			//货道商品编码
			sprintf(tmp, "%06u", 0 );
			for(j = 0; j < 6; j++)
			str[i++] = tmp[j]; 
			str[i++] = '*';
			//货道/托盘销售价格
			len = sprintf(tmp, "%u", class_global.trade .price );
			for(j = 0;j < len;j++)
			str[i++] = tmp[j];
			str[i++] = '*';
			//货道库存
//			len = sprintf(tmp, "%u", channel.store );
			{
				//托盘库存
				len = sprintf(tmp, "%u",  1);
				for(j = 0;j < len;j++)
					str[i++] = tmp[j];
				str[i++] = '*';
			}
			//货道弹簧圈数
			len = sprintf(tmp,"%u",1 );
			for(j = 0;j < len;j++)
			str[i++] = tmp[j];
			
			if(layer != (6 - 1) )//最后一层不要加分隔符
			str[i++] = '*';
		}
	}
	str[i++] = '*';
	str[2] = i+1;;
}
//================================
//登录包3
//整机状态
//================================
void get_machine_status( uint8_t* str)
{
	char tmp[10];
	uint32_t i,j,len;
	uint8_t validEquNum;//本次上报的设备数量
	
	validEquNum = 0;
	i = 0;
	str[i++]=0x1f;
	str[i++]='*';
	str[i++]='*';
	str[i++]=0x34;
	str[i++]='*';
	
	//本次上报设备数量
	str[i++] = '8';
	str[i++] = '*';
	//驱动板
	str[i++] = '0';
	str[i++] = '1';
	str[i++] = '*';
	if(1)//驱动板状态
		str[i++] = '0';
	else
		str[i++] = '1';
	str[i++] = '*';
	validEquNum++;
	//门状态
	str[i++] = '0';
	str[i++] = '2';
	str[i++] = '*';
	str[i++] = '0';//门状态
	str[i++] = '*';
	validEquNum++;
	//读卡器
	str[i++] = '0';
	str[i++] = '3';
	str[i++] = '*';
//	if(class_global.ireader.info.equ.state == 1)//读卡器状态
	if(1)
		str[i++] = '0';
	else
		str[i++] = '1';
	str[i++] = '*';
	validEquNum++;
	//温度
	str[i++] = '0';
	str[i++] = '4';
	str[i++] = '*';
	len = sprintf(tmp, "%d", 10);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	validEquNum++;
	//制冷
	str[i++] = '0';
	str[i++] = '5';
	str[i++] = '*';
//	if(class_global.driver[0]->ext[EXT_COLD].state == 1)//制冷状态
	if(1)
		str[i++] = '1';
	else
		str[i++] = '0';
	
	str[i++] = '*';
	validEquNum++;
	//风机
	str[i++] = '0';
	str[i++] = '6';
	str[i++] = '*';
	//	if(class_global.driver[0]->ext[EXT_WIND].state == 1) //风机状态
	if(1)
		str[i++] = '1';
	else
		str[i++] = '0';
	str[i++] = '*';
	validEquNum++;

	//日光灯
	str[i++] = '0';
	str[i++] = '7';
	str[i++] = '*';
//	if(class_global.driver[0]->ext[EXT_LIGHT].state == 1)
	if(1)
		str[i++] = '1';
	else
		str[i++] = '0';
	str[i++] = '*';
	validEquNum++;

	//消毒灯
	str[i++] = '0';
	str[i++] = '8';
	str[i++] = '*';
//	if(class_global.driver[0]->ext[EXT_HOT].state == 1)
	if(1)
		str[i++] = '1';
	else
		str[i++] = '0';
	str[i++] = '*';
	validEquNum++;

	str[2] = i+1;
}
//=========================
//登录包4
//程序版本号
//=========================
void get_software_version( uint8_t* str)
{
	uint8_t i,j;
	uint32_t cpu_id[3];
	char tmp[10];
	
	i = 0;
	str[i++]=0x1f;
	str[i++]='*';
	str[i++]='*';
	str[i++]=0x35;
	str[i++]='*';
	
	//包体
	//1.软件版本号
	for(j = 0; main_version[j] != '\0'; j++)
		str[i++] = main_version[j];
	str[i++] = '*';//分隔符
	
//	if(class_global.driver[0]->state == 1 )
//	{
//		for(j=0; class_global.driver[0]->version[j] != '\0'; j++)//软件版本号
//		{
//			str[i++] = class_global.driver[0]->version[j];
//		}
//	}
//	else
//		str[i++] = '0';

	//2.SIM卡CCID
	for(j = 0; j < ICCID_LEN && class_global.net.arr_ICCID[j] != 0; j++)
		str[i++] = class_global.net.arr_ICCID[j];
	str[i++] = '*';//分隔符
	
	//3.CPU id
	cpu_id[0]=1;//*(__IO uint32_t*)(0x1fff7a10);
	cpu_id[1]=2;//*(__IO uint32_t*)(0x1fff7a14);
	cpu_id[2]=3;//*(__IO uint32_t*)(0x1fff7a18);
	sprintf(tmp, "%010u", cpu_id[0]);
	for(j = 0; j < 10; j++)
	{
		str[i++] = tmp[j];
	}
	sprintf(tmp, "%010u", cpu_id[1]);
	for(j = 0; j < 10; j++)
	{
		str[i++] = tmp[j];
	}
	sprintf(tmp, "%010u", cpu_id[2]);
	for(j = 0; j < 10; j++)
	{
		str[i++] = tmp[j];
	}


	str[2] = i+1;
}


/***************************
@功能：获取设置参数
***************************/
uint8_t get_set_param(uint8_t* str, uint8_t type)
{
	uint8_t i,j;
	char tmp[10],len;
	CUR_TIME curTime = get_cur_time();//获取当前时间	
	
//	if( class_global.sys.factory_en )//工厂模式不发送
//		return FALSE;
	
	i = 0;
	str[i++]=0x17;
	str[i++]=' ';//序号
	str[i++]=' ';//包长
	str[i++]=0x42;
	
	//接口，固定1111
	str[i++] = '1';
	str[i++] = '1';
	str[i++] = '1';
	str[i++] = '1';

	str[i++] = ';';//分隔符
	
	//数据类型
	len = sprintf(tmp, "%u", type);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j] ;
		
	str[i++] = ';';//分隔符
	
	//机器id
//	if(mode)
//	{
//		//CPU id
//		sprintf(tmp, "%010u", *(__IO uint32_t*)(0x1fff7a10));
//		for(j = 0; j < 10; j++)
//		{
//			str[i++] = tmp[j];
//		}
//		sprintf(tmp, "%010u", *(__IO uint32_t*)(0x1fff7a14));
//		for(j = 0; j < 10; j++)
//		{
//			str[i++] = tmp[j];
//		}
//		sprintf(tmp, "%010u", *(__IO uint32_t*)(0x1fff7a18));
//		for(j = 0; j < 10; j++)
//		{
//			str[i++] = tmp[j];
//		}
//	}
//	else
	{
		//机器id
		sprintf(tmp, "%010u", class_global.net.id);
		for(j = 0; j < 10; j++)
			str[i++] = tmp[j];
	}
		
	str[i++] = '*';//分隔符
		
	//时间戳
	len = sprintf(tmp,"%04u", curTime.year + 2000);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.month);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.day);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.hour);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.min);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.sec);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	str[2] = i+1;
	
	return TRUE;
}

/*
@功能：17 42 7 应答包数据解析
@说明：更新的是机器运行时参数
@说明：有效数据从第8字节开始
*/
uint8_t analysis_17_42_7(uint8_t *buf, uint8_t len)
{
	uint8_t i;
	uint32_t fac = 0;
		
	if(buf[3] != '0')//4,5两个对应包类型，这里是7
	{
		printf("17_42_7 err_1\r\n\r\n");
		return FALSE;
	}
	if(buf[4] != '7')
	{
		printf("17_42_7 err_2\r\n\r\n");
		return FALSE;
	}
	
	//第6字节恒定0
	if(buf[6] != '0')//别的包里表示托盘数
	{
		printf("17_42_7 err_3\r\n\r\n");
		return FALSE;
	}
	
	/**************** 工厂模式 ****************/
//	if( flash_param_get(FLASH_LIST_FactoryEn, FLASH_ADDR_FactoryEn, FLASH_SIZE_FactoryEn) != buf[7] - '0')
//	{
//		if(	flash_param_set(FLASH_LIST_FactoryEn, FLASH_ADDR_FactoryEn, FLASH_SIZE_FactoryEn, ( ( buf[7] - '0') ? 1:0) ) )//写flash
//		{
//			if(buf[7] - '0')
//				class_global.sys.factory_en = FACTORY_AUTO;//更新工厂模式
//		}
//		else
//			printf("17_42_7 factory set err\r\n\r\n");
//	}
	
	/**************** 层数 ****************/
//	if( !(buf[8] - '0' > MAX_LAYER_SIZE) )//不超过定义的最大层数
//	{
//		if( flash_param_get(FLASH_LIST_LayerNum, FLASH_ADDR_LayerNum, FLASH_SIZE_LayerNum) != buf[8] - '0')
//		{
//			if( flash_param_set(FLASH_LIST_LayerNum, FLASH_ADDR_LayerNum, FLASH_SIZE_LayerNum, (buf[8] - '0')) )//写flash
//				class_global.vm.construction.max_layer = buf[8] - '0';//更新层数
//			else
//				printf("17_42_7 layer set err\r\n\r\n");
//		}
//	}
//	else
//	{
//		printf("17_42_7 layer num is too long\r\n\r\n");
//	}
	
	/**************** 抖一抖功能使能 ****************/
//	class_global.trade.sale.shake_en = buf[9] - '0';
	
	/************* 是否更新ip地址与端口号, buf[10]=2,取ip地址 ************/
	class_global.net.updata_flag = buf[10] - '0';

	/**************** 交易号 ****************/
	for(i = 0, fac = 1000000000, class_global.trade.number = 0; i < 10; i++)
	{
		class_global.trade.number += (buf[13 + i] - '0')*fac;//更新交易号
		fac /= 10;
	}
	
	/**************** 光检使能 ****************/
//	if( flash_param_get(FLASH_LIST_DropEn, FLASH_ADDR_DropEn, FLASH_SIZE_DropEn) != buf[24] - '0' )
//	{
//		if( flash_param_set(FLASH_LIST_DropEn, FLASH_ADDR_DropEn, FLASH_SIZE_DropEn, (buf[24] - '0')) )//写flash
//			class_global.drop.en = (enum SWITCH_MODE)(buf[24] - '0');//更新光检使能
//		else
//			printf("17_42_7 drop en set err\r\n\r\n");
//	}
	
	/**************** 货道位数 ****************/
//	class_global.sys.channel_bit = buf[25] - '0';
//	class_global.sys.channel_bit = 2;
	
	/**************** 刷卡接口 ****************/
	for(i = 0, fac = 1000, class_global.ireader.interface = 0; i < 4; i++)
	{
		class_global.ireader.interface += (buf[26 + i] - '0')*fac;
		fac /= 10;
	}
	
	/**************** 最大价格 ****************/
	for(i = 0, fac = 100000, class_global.trade.max_price = 0; i < 6; i++)
	{
		class_global.trade.max_price += (buf[30 + i] - '0')*fac;
		fac /= 10;
	}
	
	/**************** 温控模式 ****************/
//	class_global.driver[0]->temp.mode = buf[36] - '0';
	
	/**************** 目标温度 ****************/
//	if( flash_param_get(FLASH_LIST_TargetTemp(1), FLASH_ADDR_TargetTemp(1), FLASH_SIZE_TargetTemp(1)) != (buf[37] - '0')*10 + (buf[38] - '0'))
//	{
//		flash_param_set(FLASH_LIST_TargetTemp(1), FLASH_ADDR_TargetTemp(1), FLASH_SIZE_TargetTemp(1), (buf[37] - '0')*10 + (buf[38] - '0'));
//		class_global.driver[0]->temp.target = (buf[37] - '0')*10 + (buf[38] - '0');//更新目标温度
//	}
	
	/**************** 制冷开启时间 ****************/
//	class_global.driver[0]->ext[EXT_COLD].open_time =  (buf[39] - '0')*1000 + (buf[40] - '0')*100 + (buf[41] - '0')*10 + (buf[42] - '0');
	
	/**************** 制冷关闭时间 ****************/
//	class_global.driver[0]->ext[EXT_COLD].close_time = (buf[43] - '0')*1000 + (buf[44] - '0')*100 + (buf[45] - '0')*10 + (buf[46] - '0');
	
	/**************** 制热开启时间(与制冷时间一样) ****************/
//	class_global.driver[0]->ext[EXT_HOT].open_time =  class_global.driver[0]->ext[EXT_COLD].open_time;
	
	/**************** 制冷开启时间(与制冷时间一样) ****************/
//	class_global.driver[0]->ext[EXT_HOT].close_time = class_global.driver[0]->ext[EXT_COLD].close_time;
	
	/**************** 照明开启时间 ****************/
//	class_global.driver[0]->ext[EXT_LIGHT].open_time =   (buf[47] - '0')*1000 + (buf[48] - '0')*100 + (buf[49] - '0')*10 + (buf[50] - '0');
	
	/**************** 照明关闭时间 ****************/
//	class_global.driver[0]->ext[EXT_LIGHT].close_time =  (buf[51] - '0')*1000 + (buf[52] - '0')*100 + (buf[53] - '0')*10 + (buf[54] - '0');
	
	/**************** 玻璃加热开启时间 ****************/
//	class_global.driver[0]->ext[EXT_BKP].open_time =  (buf[55] - '0')*1000 + (buf[56] - '0')*100 + (buf[57] - '0')*10 + (buf[58] - '0');
	
	/**************** 玻璃加热关闭时间 ****************/
//	class_global.driver[0]->ext[EXT_BKP].close_time = (buf[59] - '0')*1000 + (buf[60] - '0')*100 + (buf[61] - '0')*10 + (buf[62] - '0');
	
	
	/**************** 订奶使能 ****************/
//	class_global.order.en = 										(enum SWITCH_MODE)(buf[63] - '0');
	
	/**************** 出货失败货道停售(停止电机) ****************/
//	class_global.trade.sale.pause_channel_en = 	(enum SWITCH_MODE)(buf[64] - '0');
	
	/**************** 一键补货使能 ****************/
//	class_global.trade.sale.replenishment_en = 				(enum SWITCH_MODE)(buf[65] - '0');
	
	/**************** 无电机出货 ****************/
//	class_global.trade.sale.sale_type = 								(enum SALE_TYPE)(buf[66] - '0');
//	if(class_global.trade.sale.sale_type > 2)
//		class_global.trade.sale.sale_type = (enum SALE_TYPE)0;//目前只有3种出货方式
	
	/**************** 扫码支付使能 ****************/
//	class_global.qrcode.en = (enum SWITCH_MODE)(buf[94] - '0');
	
	/**************** 扫码支付域名 ****************/
//	if(class_global.qrcode.en)
//	{
//		if( (len > 96) && ( (len - 2 - 94) > QR_CODE_LEN ) )
//		{
//			printf("17_42_7 domian is too long\r\n\r\n");
//		}
//		strncpy((char*)class_global.qrcode.arr_domain, (char*)&buf[95], len - 2 - 94);
//	}

	return TRUE;
}

/*
@功能：17 42 8 应答包数据解析
@说明：更新的是货道禁能/使能
@说明：有效数据从第8字节开始
*/
uint8_t analysis_17_42_8(uint8_t *buf, uint8_t len)
{
	return TRUE;
}

/*
@功能：17 42 9 应答包数据解析
@说明：更新的是货道价格/条码/库存
@说明：有效数据从第8字节开始
*/
uint8_t analysis_17_42_9(uint8_t *buf, uint8_t len)
{
	return TRUE;
}

/*
@功能：登录包，机器参数上传
@参数：str，数据缓冲指针
*/
void get_vm_set(volatile uint8_t* str)
{
	uint8_t i, j, len;
	char tmp[10];
	
	i = 0;
	str[i++]=0x1f;
	str[i++]='*';
	str[i++]='*';
	str[i++]=0x32;
	str[i++]='*';
	
	//包体
	//工作模式
	if(class_global.sys.factory_en)
		str[i++] = '1';
	else
		str[i++] = '0';
	str[i++] = '*';
	
	//机柜数
	len = sprintf(tmp, "%u", 1);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//托盘数
	len = sprintf(tmp, "%u", 1);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//小数位数
	len = sprintf(tmp, "%u", class_global.sys.point_bit);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//价格位数
	len = sprintf(tmp, "%u", class_global.sys.price_bit);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';

	//货道位数
	len = sprintf(tmp, "%u", 1);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//条码位数
	len = sprintf(tmp, "%u", 6);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//最大价格
	len = sprintf(tmp, "%u", 10000);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//光检使能
	len = sprintf(tmp, "%u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//锁止货道使能
	len = sprintf(tmp, "%u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//2线电机信号检测次数
	len = sprintf(tmp, "%u", 10);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//订购开关
	len = sprintf(tmp, "%u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';

	//二维码支付开关
	len = sprintf(tmp, "%u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//刷卡接口
	len = sprintf(tmp, "%u", class_global.ireader.interface);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//温控模式
	len = sprintf(tmp, "%u", 2);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//目标温度
	len = sprintf(tmp, "%u", 20);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';

	//温控开启时间
	len = sprintf(tmp, "%04u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';

	//温控关闭时间
	len = sprintf(tmp, "%04u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';

	//照明开启时间
	len = sprintf(tmp, "%04u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//照明关闭时间
	len = sprintf(tmp, "%04u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//消毒开启时间
	len = sprintf(tmp, "%04u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';
	
	//消毒关闭时间
	len = sprintf(tmp, "%04u", 0);
	for(j = 0; j < len; j++)
	{
		str[i++] = tmp[j];
	}
	str[i++] = '*';

	//二维码域名
	for(j = 0; j < 64; j++)
	{
		str[i++] = '0';
	}

	str[2] = i+1;
}

/*
@功能：获取重启命令
@参数：str，数据缓冲指针
*/
void get_reset_cmd(uint8_t* str)
{
	uint8_t i, j, len;
	char tmp[10];
	CUR_TIME curTime = get_cur_time();//获取当前时间
	
	i = 0;
	str[i++]=0x17;
	str[i++]=' ';//序号
	str[i++]=' ';//包长
	str[i++]=0x42;
	
	//接口，固定1111
	str[i++] = '1';
	str[i++] = '1';
	str[i++] = '1';
	str[i++] = '1';

	str[i++] = ';';//分隔符
	
	//数据类型
	str[i++] = '1';
	str[i++] = '0';
	str[i++] = ';';//分隔符
	
	//机器id
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0; j < 10; j++)
		str[i++] = tmp[j];
		
	str[i++] = '*';//分隔符
		
	//时间戳
	len = sprintf(tmp,"%04u", curTime.year + 2000);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.month);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.day);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.hour);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.min);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.sec);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	str[2] = i+1;
}

/*
@包类型：0x17-0x41
@功能：工厂模式-获取平台下发的设置
@参数：	*str，数据缓冲
				type，下发数据类型
*/
uint8_t get_factory_set(uint8_t *str, uint8_t type)
{
	uint8_t i,j;
	char tmp[10], len;
	CUR_TIME curTime = get_cur_time();//获得当前时间;

	//满足3个条件，1.工厂模式；2.机器号1000000000；3.黑莓网关
	if( (class_global.sys.factory_en == USER_MODE) || (class_global.net.id != 1000000000) 
	|| (strncasecmp((char*)class_global.net.arr_ip, DEF_HEIM_IP, sizeof(DEF_HEIM_IP) - 1)) || (class_global.net.serverPort != DEF_HEIM_PORT) )
		return 0;
	
	i = 0;
	str[i++] = 0x17;
	str[i++] = '*';
	str[i++] = '*';
	str[i++] = 0x41;
	
	//接口，固定9999
	str[i++] = '9';
	str[i++] = '9';
	str[i++] = '9';
	str[i++] = '9';
	str[i++] = ';';
	
	//数据类型
	len = sprintf(tmp, "%u", type);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j] ;
		
	str[i++] = ';';//分隔符
	
	//机器id
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0; j < 10; j++)
		str[i++]=tmp[j];
		
	str[i++] = '*';//分隔符
	
	//CPU ID
	sprintf(tmp, "%010u", (*(__IO uint32_t*)(0x1fff7a10)) );
	for(j = 0; j < 10; j++)
		str[i++] = tmp[j];
	sprintf(tmp, "%010u", (*(__IO uint32_t*)(0x1fff7a14)) );
	for(j = 0; j < 10; j++)
		str[i++] = tmp[j];
	sprintf(tmp, "%010u", (*(__IO uint32_t*)(0x1fff7a18)) );
	for(j = 0; j < 10; j++)
		str[i++] = tmp[j];
		
	str[i++] = '*';//分隔符
		
	//时间戳
	len = sprintf(tmp,"%04u", curTime.year + 2000);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.month);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.day);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.hour);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.min);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.sec);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';//?????
	str[2] = i+1;
	
	return 1;
}


/*
@功能：工厂模式-平台下发数据处理函数
@参数：rx，接收数据；tx，发送数据
*/
uint8_t updata_factory_set(uint8_t *rx, uint16_t rx_len, uint8_t *tx)
{
	uint8_t rs = FALSE, chk;
	uint16_t i, j = 5;
	
	for(i = 0, chk = 0; i < (rx[2] - 1); i++)
		chk += rx[i];
	if(chk == rx[rx[2] - 1])//校验通过
	{
		if(rx[0] == 0x1D && rx[1] == tx[1])//包头与序号正确
		{
			if(rx[3] == 0X30)//此CPUID已分配
			{
				char mac_id[10] = {0,0,0,0,0,0,0,0,0,0};
				char socket[6] = {0,0,0,0,0,0};
				char ip[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
				//解析收到数据
				for(i = 0; i < (rx_len - j - 1) && rx[i+j] != '*'; i++)//机器id
				{
					mac_id[i] = rx[i+j];
				}
				j += i;
				j++;
				
				class_global.sys.factory_en = (enum FACTOTY_MODE)(rx[j] - '0');//工作模式
				j += 2;
				
//				class_global.vm.construction.max_layer = rx[j] - '0';//托盘数
				j += 2;
				
				for(i = 0; i < (rx_len - j - 1) && rx[i+j] != '*'; i++)//socket
				{
					socket[i] = rx[i+j];
				}
				j += i;
				++j;
				
				for(i = 0; i < (rx_len - j - 1) && rx[i+j] != '*'; i++)//ip
				{
					ip[i] = rx[i+j];
				}
				for(i = 0; i < 16; i++)
				{
					class_global.net.arr_ip[i] = ip[i];
				}
				
				//更新数据
				class_global.net.id = strtoul(mac_id, 0, 10);//机器id转换为10进制值
				class_global.net.serverPort = atoi(socket);//端口号换为10进制值
//				
//				flash_param_set(FLASH_LIST_VmId, FLASH_ADDR_VmId, FLASH_SIZE_VmId, class_global.net.id);//保存机器号
//				flash_param_set(FLASH_LIST_Port, FLASH_ADDR_Port, FLASH_SIZE_Port, class_global.net.serverPort);//保存端口号
//				flash_ip_set((char*)class_global.net.arr_ip, FLASH_LIST_IP, FLASH_ADDR_IP, FLASH_SIZE_IP);//保存ip
//				flash_param_set(FLASH_LIST_FactoryEn, FLASH_ADDR_FactoryEn, FLASH_SIZE_FactoryEn, class_global.sys.factory_en);//保存工作模式
////				motor_match_auto();//自动匹配电机
//				iwdg_feed();//喂狗
//				//更新完成提示重启
//				vTaskDelay(ONE_SECOND*2);
				restart_equ_set(RESET_Distribution, TRUE);
			}
			rs =  TRUE;
		}
	}
	else//校验错
	{
		printf("updata mac ip chk err\r\n\r\n");
	}
	return rs;	
}

