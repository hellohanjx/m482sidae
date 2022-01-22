/*
移远EC20，4g驱动
只有TCP/IP的驱动，不含语音和短信
@说明：4G有三个指示灯：
红灯：亮，开机；微亮，关机
黄灯：慢闪（熄灭时间长）,找网状态；慢闪（亮的时间长）待机；快闪，数据传输模式
蓝灯：亮，注册LTE网络
@说明：采用状态机架构来实现
@注意：每条指令返回除了数据外还有个"OK"
*/

#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "task.h"
#include "sys.h"
#include "4G.h"
#include "uart7_config.h"
#include "msg.h"
#include "string.h"
#include "global.h"
#include "stdlib.h"
#include "user_config.h"


#define C4G_PWR		PA15	//开/关机
#define C4G_POWERON_TIME	300	//开机引脚保持时间（ms）
#define C4G_POWEROFF_TIME	800	//关机引脚保持时间（ms）
#define C4G_RESET_TIME		500	//关机到开机间隔时间（ms）

//AT指令
  char chk_baud[] = {"AT+IPR?\r\n"};				//查询波特率
  char set_baud[] = {"AT+IPR=115200\r\n"};		//设置波特率	
  char close_cmd_echo[] = {"ATE0\r\n"};			//关闭命令回显
  char save_param[] = {"AT&W\r\n"};				//保存设置参数，应答超时300ms
  char chk_iccid[] = {"AT+QCCID\r\n"};			//查询ccid

  char chk_pin[] = {"AT+CPIN?\r\n"};				//查询sim卡密码(5s超时)
static const char chk_pin_ack1[] = {"+CME ERROR: 10"};	//sim卡未插卡
static const char chk_pin_ack2[] = {"+CPIN: READY"};		//sim卡准备好

  char chk_sim_reg[] = {"AT+CREG?\r\n"};			//查询sim卡是否注册[@查询语音网络注册状态]
  char chk_net_reg[] = {"AT+CGREG?\r\n"};		//查询net注册[@查询数据网络注册状态]
 
 //运营商APN
  char set_link_ChinaMobile[] = {"AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n"};//设置接入点(移动) APN,USERNAME,PASSWORD
  char set_link_UNICOM[] = {"AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r\n"};//设置接入点(联通) APN,USERNAME,PASSWORD
  char set_link_p[] = {"AT+QICSGP=1,1,\"CTLTE\",\"\",\"\",1\r\n"};//设置接入点(电信) APN,USERNAME,PASSWORD
 
  char chk_link_point[] = {"AT+QICSGP=1\r\n"};	//查询接入点 
  char active_pdp_context[] = {"AT+QIACT=1\r\n"};//激活接入点(150s超时)[@激活PDP网络]
  char chk_pdp_context[] = {"AT+QIACT?\r\n"};	//查询context,返回当前接入点信息(150s超时)
  char deactive_pdp_context[] = {"AT+QIDEACT=1\r\n"};//关闭接入点（40s超时）
// const char creat_tcp[] = {"AT+QIOPEN=1,0,\"TCP\",\"120.55.115.113\",5008,0,2\r\n"};//创建TCP连接（150s）
//  const char creat_tcp[] = {"AT+QIOPEN=1,0,\"TCP\",\"120.26.204.86\",8001,0,2\r\n"};//创建TCP连接（150s）
 
 char test_tcp[] = {"AT+QIOPEN?\r\n"};		//测试TCP连接命令
 char creat_tcp_top[] = {"AT+QIOPEN=1,0,\"TCP\",\""};
 char creat_tcp_tail[] = {"\","};//创建TCP连接（150s）
 char creat_tcp_port[] = {",0,2\r\n"};//服务端端口号+传输模式【透传模式】
static const char creat_tcp_ack[] = {"CONNECT"};//创建TCP成功
	
  char close_tcp[] = {"AT+QICLOSE=0\r\n"};		//关闭TCP连接（这个可以自己设置超时）
  char chk_tcp[] = {"AT+QISTATE=1,0\r\n"};		//查询TCP连接
  char chk_data_echo[] = {"AT+QISDE?\r\n"};		//查询数据回显
// char close_data_echo[] = {"AT+QISDE=0\r\n"};	//关闭数据回显（透传模式不需要）
  char chk_err_code[] = {"AT+QIGETERROR\r\n"};	//查询最近的错误码

  char exit_transpartent[] = {"+++"};		//退出透传模式
  char exit_transpartent2[] = {"++++++++\r\n"};		//命令模式下的+++测试

  char change_transparent_mode[] = {"AT+QISWTMD=1,2\r\n"};		//改为透传模式
  char enter_transparent_mode[] = {"ATO\r\n"};		//进入透传模式

static const char common_ack[] = {"OK"};	//通用回复"OK"

   char chk_Operators[] = {"AT+COPS?\r\n"};//查是什么卡，移动联通电信
static const char Operators_ChinaMobile_ack[] = {"CHINA MOBILE"};	//中国移动
static const char Operators_ChinaUnicom_ack[] = {"CHN-UNICOM"};		//中国联通
static const char Operators_ChinaTelecom_ack[] = {"CHN-CT"};			//中国电信
//	 char Operators_ChinaMobile_ack[] = {"china mobile"};//中国移动
//	 char Operators_ChinaUnicom_ack[] = {"chn-unicom"};//中国联通
//	 char Operators_ChinaTelecom_ack[] = {"chn-ct"};//中国电信

 char chk_ICCID[] = {"AT+QCCID\r\n"};//查询sim卡ICCID	
static const char chk_ICCID_ack[] = {"+QCCID:"};//ICCID返回
		
 char chk_signal[] = {"AT+CSQ\r\n"};//查询信号强度
static const char chk_signal_ack[] = {"+CSQ:"};//查询信号强度返回

//运营商ICCID前6位识别号
#define OPERATORS_LEN		6//运营商识别码长度
#define CHINA_MOBILE_ID_NUM			4		//识别码个数
#define CHINA_UINCOM_ID_NUM			3		//识别码个数
#define CHINA_TELECOM_ID_NUM		2		//识别码个数
enum{ CHINA_MOBILE = 1, CHINA_UNICOM, CHINA_TELECOM };//运营商

 char Operators_ChinaMobile_ID[CHINA_MOBILE_ID_NUM][OPERATORS_LEN] = {"898600", "898602", "898604", "898607"};//中国移动
 char Operators_ChinaUnicom_ID[CHINA_UINCOM_ID_NUM][OPERATORS_LEN] = {"898601", "898606", "898609"};//中国联通
 char Operators_ChinaTelecom_ID[CHINA_TELECOM_ID_NUM][OPERATORS_LEN] = {"898603", "898611"};//中国电信

/*
@说明：信号强度参数
*/
enum{SIGNAL_NO = 99, TDSCDMA_SIGNAL_NO = 199};//无信号
enum{SIGNAL_LOW = 10, TDSCDMA_SIGNAL_LOW = 130};//弱信号0~10
enum{SIGNAL_MID = 20, TDSCDMA_SIGNAL_MID = 160};//中等信号11~20,高信号，21~31


/*
@说明：状态机状态
*/
enum
{
	ST_PWR_ON = 0,		//上电
	ST_CHK_SIM = 1,		//查询sim卡
	ST_CHK_SIM_REG = 2,		//查询sim卡网络
	ST_CHK_NET_REG = 3,		//查询网络
	ST_SET_POINT = 4,	//设置接入点
	ST_CHK_POINT = 5,	//查询接入点
	ST_ACTIVE_POINT = 6,//激活接入点
	ST_DEACTIVE_POINT = 7,//关闭接入点
	ST_CHK_CONTEXT = 8,	//查询接入点文本
	ST_CREAT_TCP = 9,	//创建TCP连接
	ST_CLOSE_TCP = 10,	//关闭TCP
	ST_CHK_TCP = 11,	//查询TCP
	ST_CHK_DATA_ECHO = 12,//查询数据回显
	ST_CLOSE_DATA_ECHO = 13,//关闭数据回显
	ST_CHK_ERRCODE = 14,	//查询最近的错误码
	ST_EXIT_TRANSPARTENT = 15,//退出透传模式
	ST_CLOSE_CMD_ECHO = 16,	//关闭命令回显
	ST_REBOOT = 17,		//重启4G
	ST_CHANGE_TRANSPARENT = 18,	//改为透传模式
	ST_ENTER_TRANSPARENT = 19, //进入透传模式
	ST_CHK_OPERATER = 20,	//查询卡类型
	ST_CHK_ICCID = 21,		//查询SIMK卡ICCID
	ST_CHK_QUALITY = 22,//查询信号强度
};

//定义4g数据结构体
 C4G_INFO c4g_info;

/*
@功能：EC20开机引脚初始化--PA15
@注意：EC20 RST引脚未使用(悬空)
*/
void _4g_config(void)
{
	GPIO_SetMode(PA, BIT15, GPIO_MODE_OUTPUT);//PA15推挽输出
	PA15 = 0;
}

/*
@功能：4g数据返回处理回调函数
@参数：rx，返回数据指针
@返回值：执行结果
*/
static uint8_t callback_4g_recv(UART7_DATA *rx)
{
	if(rx->len != 0)
	{
		if(rx->buf[0] == 0x0D && rx->buf[1] == 0x0A)
			c4g_sem_send();
	}
	return TRUE;
}


/*
@功能：开机
*/
static void c4g_power_on(void)
{
	C4G_PWR = 1;	//拉高
	vTaskDelay(C4G_POWERON_TIME);
	C4G_PWR = 0;	//拉低
}

/*
@功能：关机
*/
static void c4g_power_off(void)
{
	C4G_PWR = 1;	//拉高
	vTaskDelay(C4G_POWEROFF_TIME);
	C4G_PWR = 0;	//拉低
	#if C_4G_LOG
	printf("4g start power down\r\n");
	#endif
	vTaskDelay(39*ONE_SECOND);//关机需耗时35s
}

/*
@功能：复位
*/
static void c4g_power_reset(void)
{
	c4g_power_off();
	vTaskDelay(C4G_RESET_TIME);
	c4g_power_on();
	vTaskDelay(3000);
}

/*
@功能：关闭命令回显
@返回值：执行情况，是否可以进行下一步
*/
static uint8_t c4g_close_cmd_echo(void)
{
	UART7_DATA* rx;
	uint8_t err;
	
	_uart7_send(&rx, (uint8_t*)close_cmd_echo, (sizeof(close_cmd_echo) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(300);//300ms超时
	if(err == pdTRUE)
	{
		//返回“OK”
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从倒数第3个字符开始比较,
		{
			return TRUE;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: close echo\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：查询sim卡
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
*/
static uint8_t c4g_chk_sim(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_pin, (sizeof(chk_pin) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(5000);//等待超时 5s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), chk_pin_ack2, sizeof(chk_pin_ack2) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//返回为 "+CPIN: READY "表示sim卡准备好了
		}

		if( !strncasecmp((char*)&(rx->buf[2]), chk_pin_ack1, sizeof(chk_pin_ack1) - 1) )//从第3个字符开始比较
		{
			#if C_4G_LOG
			printf("4g_err: no sim\r\n\r\n");
			#endif
			return 2;	//未插卡
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check sim\r\n\r\n");
	#endif
	return FALSE;
}


/*
@功能：查询sim卡注册
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_sim_reg(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_sim_reg, (sizeof(chk_sim_reg) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从倒数第3个字符开始比较,
		{
			//处理接收数据
			uint8_t i, j;
			i = 9;
			c4g_info.n = rx->buf[i++];//网络状态
			++i;//跳过','
			c4g_info.stat = rx->buf[i++];//漫游信息；1，表示本地网络
			if(c4g_info.n != '0')//如果允许未请求的结果码则会传来一些信息
			{
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
					c4g_info.lac[j] = rx->buf[i];//地址代码
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
					c4g_info.ci[j] = rx->buf[i];//cell id
				++i;
				c4g_info.act = rx->buf[i];//网络制式，gsm/gprs...
			}
			
			return TRUE;//sim卡注册成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: sim register\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: sim register\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：查询网络注册状态
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_net_reg(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_net_reg, (sizeof(chk_net_reg) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			//处理接收数据
			uint8_t i, j;
			i = 10;
			c4g_info.n = rx->buf[i++];//网络状态
			++i;//跳过','
			c4g_info.stat = rx->buf[i++];//漫游信息;1,表示本地网；5，表示漫游
			if(c4g_info.n != '0')
			{
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.lac[j] = rx->buf[i];//地址代码
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.ci[j] = rx->buf[i];//cell id
				++i;
				c4g_info.act = rx->buf[i];//网络制式，gsm/gprs...
			}
			
			return TRUE;//网络注册成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: net register\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: net register\r\n\r\n");
	#endif
	return FALSE;
}


/*
@功能：查询网络接入点
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_chk_point(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_link_point, (sizeof(chk_link_point) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//比较最后的"OK"
		{
			//处理接收数据
			uint8_t i, j;
			for(i = 2;  rx->buf[i] != ',' && i < 20; i++)
				c4g_info.context_type = rx->buf[i];//地址IPv4
			i += 2;//跳过','
			for(j = 0;  rx->buf[i] != ',' && i < 20; i++, j++)
				c4g_info.apn[j] = rx->buf[i];//apn
			i += 2;
			for(j = 0;  rx->buf[i] != ',' && i < 20; i++, j++)
				c4g_info.username[j] = rx->buf[i];//用户名？
			i += 2;
			for(j = 0;  rx->buf[i] != ',' && i < 20; i++, j++)
				c4g_info.password[j] = rx->buf[i];//密码
			i += 2;
			for(; i != ',' && i < 20; i++)
				c4g_info.authentication = rx->buf[i];//认证0~3
			
			return TRUE;//查询成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: check point\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check point\r\n\r\n");
	#endif
	return FALSE;
}


/*
@功能：设置网络接入点
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_set_point(void)
{
	UART7_DATA* rx;
	uint8_t err;

	if(c4g_info.Operators == CHINA_MOBILE)
		_uart7_send(&rx, (uint8_t*)set_link_ChinaMobile, (sizeof(set_link_ChinaMobile) - 1), callback_4g_recv);//串口发送
	else 
	if(c4g_info.Operators == CHINA_UNICOM)
		_uart7_send(&rx, (uint8_t*)set_link_UNICOM, (sizeof(set_link_UNICOM) - 1), callback_4g_recv);//串口发送
	else 
	if(c4g_info.Operators == CHINA_TELECOM)
		_uart7_send(&rx, (uint8_t*)set_link_p, (sizeof(set_link_p) - 1), callback_4g_recv);//串口发送
	else
		return FALSE;
	
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//设置接入点成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: set point\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: set point\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：查询网络接入点
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_chk_context(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_pdp_context, (sizeof(chk_pdp_context) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(15000);//等待超时15s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			//处理接收数据
			uint8_t i, j;
			i = 12;
			c4g_info.context_state = rx->buf[i++];//接入点状态
			++i;//跳过','
			c4g_info.context_type = rx->buf[i++];//接入协议类型
			++i;
			for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
				c4g_info.local_address[j] = rx->buf[i];//本地ip地址
			
			if(c4g_info.context_state == '1')//已经激活，不需要再次激活
				return 3;
			return TRUE;//查询成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: check context\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check context\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：激活网络接入点
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_active_context(void)
{
	#if C_4G_LOG
	uint32_t cost_time = 0;
	#endif
	
	UART7_DATA* rx;
	uint8_t err;

	#if C_4G_LOG
	cost_time = xTaskGetTickCount();
	#endif
	_uart7_send(&rx, (uint8_t*)active_pdp_context, (sizeof(active_pdp_context) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(15000);//等待超时15s
	if(err == pdTRUE)
	{
		#if C_4G_LOG
		char tmp[10] = {0,0,0,0,0,0,0,0,0,0};
		sprintf(tmp, "%u", xTaskGetTickCount() - cost_time);
		printf("c4g_active_context_time = ");
		printf(tmp);
		printf("\r\n\r\n");
		#endif
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//激活成功
		}
		else
		if(!strncasecmp((char*)&(rx->buf[2]), common_ack, sizeof(common_ack) - 1) )
		{
			return TRUE;//@新版的4G多了这个东西，"+QDMURC"，字符串
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: active context\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: active context\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：关闭网络接入点
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_deactive_context(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)deactive_pdp_context, (sizeof(deactive_pdp_context) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(40000);//等待超时40s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//激活成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: deactive context\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: deactive context\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：创建TCP连接
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
@说明：成功后进入透传模式；退出此主任务
*/
static uint8_t c4g_creat_tcp(void)
{
	#if C_4G_LOG
	uint32_t cost_time = 0;
	#endif
	
	UART7_DATA* rx;
	uint8_t err,i;
	char creat_tcp[55];
	char port[6];
	
	for(i = 0; i < 55; i++){
		creat_tcp[i] = 0;
	}
	for(i = 0; i < 6; i++){
		port[i] = 0;
	}	
	sprintf(port, "%u", class_global.net.serverPort);//端口号转换为字符串
	strcpy(creat_tcp, creat_tcp_top);//字符串拷贝
	strcat(creat_tcp, (const char*)class_global.net.arr_ip);
//	strcat(creat_tcp, (const char*)"ip.hmvend.cn");
	strcat(creat_tcp, creat_tcp_tail);
	strcat(creat_tcp, (const char*)port);
	strcat(creat_tcp, creat_tcp_port);
	
	#if C_4G_LOG
	cost_time = xTaskGetTickCount();
	#endif
	_uart7_send(&rx, (uint8_t*)creat_tcp, strlen(creat_tcp), callback_4g_recv);//串口发送
	err = c4g_sem_get(153000);//等待超时153s
	if(err == pdTRUE)
	{
		#if C_4G_LOG
		char tmp[10] = {0,0,0,0,0,0,0,0,0,0};
		sprintf(tmp, "%u", xTaskGetTickCount() - cost_time);
		printf("c4g_creat_tcp_time = ");
		printf(tmp);
		printf("\r\n\r\n");
		#endif
		
		if( !strncasecmp((char*)&(rx->buf[2]), creat_tcp_ack, sizeof(creat_tcp_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//激活成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: create TCP\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: create TCP\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：关闭TCP连接
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
@说明：成功后进入透传模式；退出此主任务
*/
static uint8_t c4g_close_tcp(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)close_tcp, (sizeof(close_tcp) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(10000);//等待超时10s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//关闭成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: close TCP\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: close TCP\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：TCP创建测试命令
@说明："active_pdp_context"指令发完直接发"creat_tcp"指令，有时"creat_tcp"指令的返回会多一个首字节(0x0D)，中间随便加一条指令就不会有这个问题（@@@@神他妈东西啊，不知是不是这个便宜货的问题）
@创建日期：2019.8.10
*/
static uint8_t c4g_test_tcp(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)test_tcp, (sizeof(test_tcp) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(300);//等待超时300ms
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较
		{
			return TRUE;
		}
		#if C_4G_LOG
		printf("4g_err: test TCP\r\n\r\n");
		#endif
	}
	#if C_4G_LOG
	printf("4g_no_ack: test TCP\r\n\r\n");
	#endif
	return FALSE;
}
/*
@功能：查询TCP连接状态
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
*/
static uint8_t c4g_chk_tcp(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_tcp, (sizeof(chk_tcp) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(15000);//等待超时15s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			if(rx->len > 6)//如果TCP连接存在，会返回很多数据
			{
				//处理接收数据
				uint8_t i, j;
				i = 12;
				for(j = 0;  rx->buf[i] != ',' && j < 2; i++, j++)
					c4g_info.connectID[j] = rx->buf[i];//1~16
				++i;//跳过','
				for(j = 0;  rx->buf[i] != ',' && j < 10; i++, j++)
					c4g_info.service_type[j] = rx->buf[i];//连接方式，TCP/UDP...
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
					c4g_info.ip_adress[j] = rx->buf[i];//服务端地址
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.remote_port[j] = rx->buf[i];//服务端端口号
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.local_port[j] = rx->buf[i];//本机端口号
				++i;
				c4g_info.socket_state = rx->buf[i++];//socket状态
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 2; i++, j++)
					c4g_info.contextID[j] = rx->buf[i];//1~16
				++i;
				c4g_info.serverID = rx->buf[i++];//
				++i;
				c4g_info.access_mode = rx->buf[i++];//传输模式；1.缓冲，2.推送，3.透传
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 10; i++, j++)
					c4g_info.AT_port[j] = rx->buf[i];//命令端口，UART1/USB
				
				if(c4g_info.socket_state == '4')//正在关闭socket
					return 4;
				else
				if(c4g_info.socket_state == '1' 
				|| c4g_info.socket_state == '2'
				|| c4g_info.socket_state == '3')//socket存在
					return 3;
			}
			
			return TRUE;//查询成功,但是需要重新连接 TCP
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: check TCP\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check TCP\r\n\r\n");
	#endif
	return FALSE;
}


/*
@功能：退出透传模式
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
@说明：成功后进入透传模式；退出此主任务
*/
static uint8_t c4g_exit_transpartent(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)exit_transpartent, (sizeof(exit_transpartent) -1), callback_4g_recv);//串口发送
	err = c4g_sem_get(1000);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), common_ack, sizeof(common_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//激活成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: exit transpartent_1\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: exit transpartent_1\r\n\r\n");
	#endif
	return FALSE;
}

//这个其实就是随便发一个字符串（按照命令格式），只要有回复就说明是在“命令模式”
static uint8_t c4g_exit_transpartent2(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)exit_transpartent2, (sizeof(exit_transpartent2) -1), callback_4g_recv);//串口发送
	err = c4g_sem_get(1000);//等待超时
	if(err == pdTRUE)
	{
		return TRUE;//激活成功
	}
	#if C_4G_LOG
	printf("4g_no_ack: exit transpartent_2\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：命令模式进入透传模式
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
@说明：成功后进入透传模式；退出此主任务
*/
static uint8_t c4g_enter_transparent(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)enter_transparent_mode, (sizeof(enter_transparent_mode) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(10000);//等待超时10s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), creat_tcp_ack, sizeof(creat_tcp_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//关闭成功
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: enter transpartent\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: enter transpartent\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：改为透传模式
@返回值：执行情况，是否可以进行下一步
@注意：返回值的前2个字节是"/r/n",所以比较字符串的话需要从返回值第3个字节开始
		sizeof(数组字符串) == 字符串长度+'0'结束符；所以这里比较的话需要减去'0'结束符
		返回值"OK"在倒数第4个字节开始
@说明：成功后进入透传模式；退出此主任务
*/
static uint8_t c4g_change_transpartent(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)change_transparent_mode, (sizeof(change_transparent_mode) - 1), callback_4g_recv);//串口发送
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), creat_tcp_ack, sizeof(creat_tcp_ack) - 1) )//从第3个字符开始比较,
		{
			return TRUE;//已进入透传模式
		}
		else
		{
			//处理错误代码
			#if C_4G_LOG
			printf("4g_err: change transpartent\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: change transpartent\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：查询卡类型
@返回值：执行情况，是否可以进行下一步
*/
static uint8_t c4g_check_Operators(void)
{
	uint8_t i;
	
	//第一步，根据ICCID判断运营商
	for(i = 0; i < CHINA_MOBILE_ID_NUM; i++)//移动
	{
		if( !strncasecmp((char*)(class_global.net.arr_ICCID), &(Operators_ChinaMobile_ID[i][0]), 6 ) )
		{
			c4g_info.Operators = CHINA_MOBILE;
			return TRUE;
		}
	}
	for(i = 0; i < CHINA_UINCOM_ID_NUM; i++)//联通
	{
		if( !strncasecmp((char*)(class_global.net.arr_ICCID), &(Operators_ChinaUnicom_ID[i][0]), 6 ) )
		{
			c4g_info.Operators = CHINA_UNICOM;
			return TRUE;
		}
	}
	for(i = 0; i < CHINA_TELECOM_ID_NUM; i++)//电信
	{
		if( !strncasecmp((char*)(class_global.net.arr_ICCID), &(Operators_ChinaTelecom_ID[i][0]), 6 ) )
		{
			c4g_info.Operators = CHINA_TELECOM;
			return TRUE;
		}
	}
	
	//第一步不成功，进行第二步，根据应答判断运营商
	{
		UART7_DATA* rx;
		uint8_t err;
		
		_uart7_send(&rx, (uint8_t*)chk_Operators, (sizeof(chk_Operators) - 1), callback_4g_recv);//串口发送
		err = c4g_sem_get(3000);//1.8s超时
		if(err == pdTRUE)
		{
			if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//比较最后的"OK"
			{
	//			printf((char*)&rx->buf[14]);
	//			printf("\r\n\r\n\r\n");
				//从第14字节开始比较运营商名称
	//			strupr(&rx->buf[rx->len-4]);
				if( !strncasecmp((char*)&(rx->buf[14]), Operators_ChinaMobile_ack, sizeof(Operators_ChinaMobile_ack) - 1) )//与中国移动比较
				{
					c4g_info.Operators = CHINA_MOBILE;//移动
					return TRUE;
				}
				if( !strncasecmp((char*)&(rx->buf[14]), Operators_ChinaUnicom_ack, sizeof(Operators_ChinaUnicom_ack) - 1) )//与中国联通比较
				{
					c4g_info.Operators = CHINA_UNICOM;//联通
					return TRUE;
				}
				if( !strncasecmp((char*)&(rx->buf[14]), Operators_ChinaTelecom_ack, sizeof(Operators_ChinaTelecom_ack) - 1) )//与中国电信比较
				{
					c4g_info.Operators = CHINA_TELECOM;//电信
					return TRUE;
				}
				#if C_4G_LOG
				printf("4g_err: check Operators\r\n\r\n");
				#endif
			}
		}
		#if C_4G_LOG
		printf("4g_no_ack: check Operators\r\n\r\n");
		#endif
		return FALSE;
	}
}

/*
@功能：查询sim卡ICCID（Integrated Circuit Card Identifier）
*/
static uint8_t c4g_chk_ICCID(void)
{
	UART7_DATA* rx;
	uint8_t err;
	uint8_t i;

	_uart7_send(&rx, (uint8_t*)chk_ICCID, (sizeof(chk_ICCID) - 1), callback_4g_recv);
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), chk_ICCID_ack, sizeof(chk_ICCID_ack) - 1) )//从第3个字符开始比较,
		{
			if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//判断结尾是否为"OK"
			{
				for(i = 0; i < ICCID_LEN - 1 && rx->buf[10+i] != 0x0D; i++)//到0x0D截止
					class_global.net.arr_ICCID[i] = rx->buf[10+i];
				class_global.net.arr_ICCID[10+i] = '\0';//结束符
				return TRUE;
			}
			#if C_4G_LOG
			printf("4g_err: check ICCID\r\n\r\n");
			#endif
		}
	}
	class_global.net.arr_ICCID[0] = 'X';
	class_global.net.arr_ICCID[1] = 0;
	
	#if C_4G_LOG
	printf("4g_err: check ICCID\r\n\r\n");
	#endif
	return FALSE;
}

/*
@功能：查询信号强度
*/
static uint8_t c4g_chk_quality(void)
{
	UART7_DATA* rx;
	uint8_t err;


	_uart7_send(&rx, (uint8_t*)chk_signal, (sizeof(chk_signal) - 1), callback_4g_recv);
	err = c4g_sem_get(300);//等待超时
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), chk_signal_ack, sizeof(chk_signal_ack) - 1) )//从第3个字符开始比较
		{
			if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//判断结尾是否为"OK"
			{
				char i, j, rssi[10] = {0}, ber[10] = {0};
				for(i = 7, j = 0; (i < rx->len) && (rx->buf[i] != ',') ;i++)
				{
					if(rx->buf[i] != ' ')
						rssi[j++] = rx->buf[i];//信号强度
				}
				
				i++;
				
				for(j = 0; rx->buf[i] != '\r' && rx->buf[i+1] != '\n'; i++)
					ber[j++] = rx->buf[i];//误码率
				
				c4g_info.rssi = atoi((const char*)rssi);
				c4g_info.ber = atoi((const char*)ber);
				
				#if(0)
				printf("4g: signal quality = ");
				printf((const char*)rssi);
				printf("\r\n\r\n");
				
				printf("4g: err code = ");
				printf((const char*)ber);
				printf("\r\n\r\n");
				#endif
				
				return TRUE;
			}
			#if C_4G_LOG
			printf("4g_err: quality format\r\n\r\n");
			#endif
		}
	}
	class_global.net.arr_ICCID[0] = 'X';
	class_global.net.arr_ICCID[1] = 0;
	
	#if C_4G_LOG
	printf("4g_err: check quality\r\n\r\n");
	#endif
	return FALSE;
}


/*
@功能：4G主任务
*/
uint32_t time;
void main_task_4g(void)
{
	uint8_t rs;
	uint8_t work_state = 0;//0：刚上电；1：已连接过一次
	uint8_t tcp_exist = 0;//是否存在tcp连接。0，不存在，1,存在
	
	if(class_global.net.state == 1)
	{
		c4g_info.reboot_timeout = xTaskGetTickCount();//更新失联时间
		return;
	}

	c4g_info.fsm = ST_EXIT_TRANSPARTENT;//每次模块启动从退出透传开始
	time = ONE_SECOND;
	vTaskDelay(3*ONE_SECOND);
	while(1)
	{
		//失联计时
		if(c4g_info.reboot_timeout == 0)
			c4g_info.reboot_timeout = xTaskGetTickCount();
		if(xTaskGetTickCount() - c4g_info.reboot_timeout > 120000)//2分钟重启
		{
			if((c4g_info.fsm == ST_CHK_SIM
			|| c4g_info.fsm == ST_CHK_SIM_REG
			|| c4g_info.fsm == ST_CHK_NET_REG))//这三种情况是开机查询网络，允许重启时间加长
			{
				if(xTaskGetTickCount() - c4g_info.reboot_timeout > 240000)//超过4分钟重启
				{
					c4g_info.fsm = ST_REBOOT;//重启
					c4g_info.fsm = ST_EXIT_TRANSPARTENT;//初始化状态
					work_state = 0;//状态清0
					tcp_exist = 0;//置标志为不存在TCP连接
					c4g_info.reboot_timeout = xTaskGetTickCount();//刷新时间，每2分钟重启一次
					printf("4g reboot\r\n");
					restart_equ_set(RESET_4G, FALSE);
				}
			}
			else//除上述3种情况，其他状态下失联超过2分钟重启
			{
				c4g_power_off();
				c4g_power_on();
				c4g_info.fsm = ST_EXIT_TRANSPARTENT;//初始化状态
				work_state = 0;//状态清0
				tcp_exist = 0;//置标志为不存在TCP连接
				c4g_info.reboot_timeout = xTaskGetTickCount();//刷新时间，每2分钟重启一次
				printf("power on\r\n");
				restart_equ_set(RESET_4G, FALSE);
			}
		}
		
		if(xTaskGetTickCount() - c4g_info.lost_link_timeout > 40000 || work_state == 0)//超过40s，通讯模式连接不上，才进入这里
		{
			switch(c4g_info.fsm)
			{
				case ST_EXIT_TRANSPARTENT://退出透传模式
					c4g_info.rssi = 0xff;//信號強度初始化
					c4g_info.ber = 0xff;//误码率初始化
					work_state = 0;//进入这里，就要走完循环
					vTaskDelay(1000);
					rs = c4g_exit_transpartent();
					vTaskDelay(1000);
					if(rs == TRUE){
						//有回应说明模块是开机的，并且成功退出透传模式。如果开机，则回显已经关闭
						//（此时一般是复位了主板，但是没有复位模块）
//						c4g_info.fsm = ST_CHK_TCP;//跳转“查询TCP是否连接”
						tcp_exist = 1;//接入点应当是存在的
						if(c4g_chk_ICCID())//查询ICCID成功，这时肯定有sim卡
							c4g_info.fsm = ST_CHK_QUALITY;//跳转查询运营商
					}
					else
					if(rs == 2){
						printf("exit cmd mode fail\r\n");
					}else{
						rs = c4g_chk_sim();
						rs = c4g_chk_ICCID();
						if(rs == TRUE){//已经开机并且已经在命令模式(一般是还未进入透传模式就重启主板)
//							c4g_info.fsm = ST_CHK_TCP;//跳转“查询TCP是否连接”
							tcp_exist = 3;//接入点是否存在不确定
							c4g_info.fsm = ST_CHK_QUALITY;//跳转查询运营商
						}else
						if(rs == 2){//提示没插SIM卡
							printf("no sim 1\r\n");
						}else{
							c4g_info.fsm = ST_PWR_ON;//没有回应说明模块是关机的
						}
					}
				break;
					
				case ST_CHK_QUALITY://查询信号强度
				{
					uint8_t i = 0, tmp[3] = {0};
					do{
						c4g_chk_quality();
						tmp[i%3] = c4g_info.rssi;
						if(tmp[0] != SIGNAL_NO && tmp[0] != TDSCDMA_SIGNAL_NO && tmp[0] == tmp[1] && tmp[1] == tmp[2])
						{
							break;
						}
						else
						if( ((tmp[0] > SIGNAL_MID && tmp[0] < SIGNAL_NO) || (tmp[1] > SIGNAL_MID && tmp[1] < SIGNAL_NO) || (tmp[2] > SIGNAL_MID && tmp[2] < SIGNAL_NO))
						||  ((tmp[0] > TDSCDMA_SIGNAL_MID && tmp[0] < TDSCDMA_SIGNAL_NO) || (tmp[1] > TDSCDMA_SIGNAL_MID && tmp[1] < TDSCDMA_SIGNAL_NO) || (tmp[2] > TDSCDMA_SIGNAL_MID && tmp[2] < TDSCDMA_SIGNAL_NO)))
						{
							break;
						}
						
						vTaskDelay(ONE_SECOND);
					}while(i++ < 30);
					
					c4g_info.fsm = ST_CHK_OPERATER;//跳转查询运营商
				}
				break;
					
				case ST_PWR_ON:		//上电
//					c4g_power_off();
					c4g_power_on();
					c4g_info.fsm = ST_CLOSE_CMD_ECHO;//跳到“关闭回显”
				break;
				
				case ST_CLOSE_CMD_ECHO://关闭命令回显
					if(c4g_close_cmd_echo())
						c4g_info.fsm = ST_CHK_SIM;
				break;
					
				case ST_CHK_SIM:	//查sim卡
					if(c4g_info.find_sim_timeout == 0)
						c4g_info.find_sim_timeout = xTaskGetTickCount();//开始计时
					
					rs = c4g_chk_sim();
					if(rs == TRUE){
						c4g_info.fsm = ST_CHK_ICCID;	//跳转->"查询ICCID"
					}else
					if(rs == 2){//提示 未插卡
						if( xTaskGetTickCount() - c4g_info.find_sim_timeout < 3000)//输出3s的提示
							printf("no sim 2\r\n");
					}else{//重启4G
						if(xTaskGetTickCount() - c4g_info.find_sim_timeout > 20000){//超时重启
							c4g_info.fsm = ST_REBOOT;
							c4g_info.find_sim_timeout = 0;//计时清零
						}
					}
				break;
					
				case ST_CHK_ICCID:	//查询SIM卡ICCID
					if(c4g_chk_ICCID())
						c4g_info.fsm = ST_CHK_SIM_REG;
					else
						printf("check ICCID err\r\n\r\n");
				break;
					
				case ST_CHK_SIM_REG:	//查询sim卡注册
					if(c4g_info.chk_sim_reg_timeout == 0)
						c4g_info.chk_sim_reg_timeout = xTaskGetTickCount();//开始计时
					
					rs = c4g_sim_reg();
					if(rs == TRUE){
						c4g_info.fsm = ST_CHK_NET_REG;
					}else 
					if(rs == 2){//错误信息
						
						if(xTaskGetTickCount() - c4g_info.chk_sim_reg_timeout > 90000){//超时重启
							c4g_info.fsm = ST_REBOOT;
							c4g_info.chk_sim_reg_timeout = 0;//计时清零
						}
					}
				break;
					
				case ST_CHK_NET_REG:	//查询网络
					if(c4g_info.chk_net_reg_timeout == 0)
						c4g_info.chk_net_reg_timeout = xTaskGetTickCount();//开始计时
				
					rs = c4g_net_reg();
					if(rs == TRUE){
//						c4g_info.fsm = ST_CHK_OPERATER;
						c4g_info.fsm = ST_CHK_QUALITY;//跳转->查信号强度
					}else
					if(rs == 2){//错误信息
						if(xTaskGetTickCount() - c4g_info.chk_net_reg_timeout > 60000){//超时重启
							c4g_info.fsm = ST_REBOOT;
							c4g_info.chk_net_reg_timeout = 0;//计时清零
						}
					}
				break;
					
				case ST_CHK_OPERATER://查询运营商
					rs = c4g_check_Operators();
					if(rs == TRUE){
						if(tcp_exist == 0)
							c4g_info.fsm = ST_SET_POINT;//设置接入点
						else
						if(tcp_exist == 1)
							c4g_info.fsm = ST_CHK_TCP;//查询TCP连接
						else
						if(tcp_exist == 3)
							c4g_info.fsm = ST_DEACTIVE_POINT;//关闭接入点
					}else{
						printf("Operater err\r\n");//没查到运营商，等待重启
					}
				break;
					
				case ST_SET_POINT:	//设置接入点(就是设置运营商什么的)
					rs = c4g_set_point();
					if(rs == TRUE){
						tcp_exist = 1;//设置接入点成功
						c4g_info.fsm = ST_ACTIVE_POINT;//跳转"激活接入点"
					}else
					if(rs == 2){//错误信息
						c4g_info.fsm = ST_REBOOT;
					}
				break;
					
				case ST_CHK_POINT:	//查询接入点
					
				break;
				
				case ST_ACTIVE_POINT:	//激活接入点
					rs = c4g_active_context();
					if(rs == TRUE){
						c4g_info.fsm = ST_CREAT_TCP;//跳转"创建TCP"
					}else
					if(rs == 2){//错误信息
						c4g_info.fsm = ST_DEACTIVE_POINT;//跳转"关闭接入点"
					}
				break;
					
				case ST_DEACTIVE_POINT:	//关闭接入点
					rs = c4g_deactive_context();
					if(rs == TRUE){
						tcp_exist = 0;//已关闭接入点
						c4g_info.fsm = ST_SET_POINT;//跳转"激活接入点"
					}else
					if(rs == 2){//错误信息
						printf("link point err 1\r\n");//接入点设置失败
					}
				break;
					
				case ST_CHK_CONTEXT:	//查询接入点文本
					rs = c4g_chk_context();
					if(rs == TRUE){//查询成功，但是未激活
						c4g_info.fsm = ST_ACTIVE_POINT;
					}else
					if(rs == 3){//已经激活
						c4g_info.fsm = ST_CHK_TCP;//先查下TCP连接是否存在，如果存在，直接连网；不存在，则创建连接
					}else
					if(rs == 2){//错误信息
						printf("context err 1\r\n");//查询接入点文本错误
					}
				break;
					
				case ST_CREAT_TCP:		//创建TCP连接
					rs = c4g_test_tcp();
					rs = c4g_creat_tcp();
					//20210411 如果重新建立了socket，则标记
					if(work_state == 1)
						restart_equ_set(RESET_RebuitSocket, FALSE);
					if(rs == TRUE){//直接进入透传模式
						c4g_info.fsm = ST_EXIT_TRANSPARTENT;//初始化状态“退出透传模式”
						c4g_info.lost_link_timeout = 0;
						work_state = 1;
						return;
					}else{//错误信息
						c4g_info.fsm = ST_CHK_TCP;//跳转“查询TCP”
					}
				break;
					
				case ST_CLOSE_TCP:		//关闭TCP
					rs = c4g_close_tcp();
					if(rs == TRUE){//关闭TCP成功
						vTaskDelay(100);//@@这里需要等待100ms，不然老王那里可能有问题，返回异常
						c4g_info.fsm = ST_CREAT_TCP;//跳转"创建TCP"
					}else
					if(rs == 2){//错误信息
						printf("close tcp err 1\r\n");//关闭tcp失败
					}
				break;
					
				case ST_CHK_TCP:		//查询TCP
					rs = c4g_chk_tcp();
					#if C_4G_LOG
					printf("c4g_chk_tcp\r\n");
					#endif
					if(rs == TRUE){//没有建立TCP;需要重新创建TCP连接
						c4g_info.fsm = ST_CREAT_TCP;//跳转“创建TCP”
					}else//********************
					if(rs == 3){//TCP连接已存在
						if(work_state == 0)//如果是上电后第一次
							c4g_info.fsm = ST_CLOSE_TCP;//跳转“关闭TCP”；这时需要重建TCP
						else
							c4g_info.fsm = ST_ENTER_TRANSPARENT;//跳转“进入透传”
					}else
					if(rs == 4){//需要先关闭socket
						c4g_info.fsm = ST_CLOSE_TCP;//跳转“关闭TCP”
					}else{//无应答
						printf("chk tcp err 1\r\n");//查询tcp失败
					}
				break;
					
				case ST_ENTER_TRANSPARENT:	//命令模式进入透传模式
					rs = c4g_enter_transparent();
					if(rs == TRUE){//进入透传模式
						c4g_info.fsm = ST_EXIT_TRANSPARTENT;//初始化状态“退出透传模式”
						c4g_info.lost_link_timeout = 0;
						work_state = 1;
						return;
					}else
					if(rs == 2){//进入透传失败
						c4g_info.fsm = ST_CLOSE_TCP;//跳转“关闭TCP”
					}else{//无应答
						printf("enter transpartent err 1\r\n");//进入透传无应答
					}
				break;
				
				case ST_CHK_DATA_ECHO:	//查询数据回显
					
				break;
				case ST_CLOSE_DATA_ECHO://关闭数据回显
					
				break;
				case ST_CHK_ERRCODE:	//查询最近的错误码
					
				break;
				
				case ST_REBOOT:		//重启
					c4g_power_reset();
					c4g_info.fsm = ST_CLOSE_CMD_ECHO;//跳转“关闭回显”
				break;
				
				case ST_CHANGE_TRANSPARENT:	//改为透传模式
					if(c4g_change_transpartent() == TRUE){//进入透传模式
						c4g_info.fsm = ST_EXIT_TRANSPARTENT;//初始化下次进入的状态初值
						c4g_info.lost_link_timeout = 0;
						work_state = 1;
						return;
					}else{
						c4g_info.fsm = ST_REBOOT;//重启
					}
			}
		}
		vTaskDelay(100);
	}
}

/*
@功能：获取4g当前状态
@返回：当前状态，就是执行步骤
*/
uint8_t get_4g_state(void)
{
	return c4g_info.fsm;
}

/*
@功能：获得信号强度
@参数：rssi，信号强度缓冲；ber，误码率缓冲
@返回：1~4，信号等级；0，没有执行；5，只执行了初始值
*/
uint8_t get_4g_quality(uint8_t* rssi, uint8_t* ber)
{
	uint8_t rs = FALSE;
	
	if(rssi != 0)
		*rssi = c4g_info.rssi;
	if(ber != 0)
		*ber = c4g_info.ber;
	
	if(c4g_info.rssi == 0xff)//初始值
		rs = 5;
	else
	if( c4g_info.rssi == SIGNAL_NO || c4g_info.rssi == TDSCDMA_SIGNAL_NO )//无信号
		rs = 1;
	else
	if( c4g_info.rssi <= SIGNAL_LOW || (c4g_info.rssi > SIGNAL_NO && c4g_info.rssi <= TDSCDMA_SIGNAL_LOW) )//信号强度1，弱
		rs = 2;
	else
	if( (c4g_info.rssi > SIGNAL_LOW && c4g_info.rssi <= SIGNAL_MID) || (c4g_info.rssi > TDSCDMA_SIGNAL_LOW && c4g_info.rssi <= TDSCDMA_SIGNAL_MID))//信号强度2，中
		rs = 3;
	else
	if( (c4g_info.rssi > SIGNAL_MID && c4g_info.rssi < SIGNAL_NO) || (c4g_info.rssi > TDSCDMA_SIGNAL_MID && c4g_info.rssi < TDSCDMA_SIGNAL_NO))//信号强度3，强
		rs = 4;
	
	return rs;
}

