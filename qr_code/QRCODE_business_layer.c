/*
@产生并显示二维码
*/
#include "QR_Encode.h"
#include "12864_driver.h"
#include "user_rtc.h"
#include "string.h"
#include "global.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"


const char order_addr[] = {"CradpicksmApi.php?id="};//订奶二维码地址

#if(0)
/*
@功能：生成扫码支付二维码数据
@返回值：二维码数据指针
*/
static void generate_scan_ercode(char* data)
{
	uint8_t i, j, k, len;
	char tmp[10];
	CUR_TIME curTime = get_cur_time();//获得当前时间;

	strcpy(data, (const char*)class_global.qrcode.arr_domain)	;//域名拷贝
	i = strlen((const char*)class_global.qrcode.arr_domain);
		
	sprintf(tmp, "%10u", class_global.net.id);//机器ID
	for(j = 0;j < 10; j++)
		data[i++]=tmp[j];
	data[i++] = '-';
	
	/*
	时间戳
	这个需要记录下来，每次查询需要
	*/
	k = 0;
	len = sprintf(tmp, "%04u", curTime.year + 2000);
	for(j = 0;j < 4; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.month);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.day);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.hour);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.min);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.sec);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	
	data[i++] = '-';	
	len = sprintf(tmp, "%u", class_global.trade.sel.price);//货道价格
	for(j = 0;j < len; j++)
		data[i++] = tmp[j];
	
	data[i++] = '-';
	data[i++] = '0';
	data[i++] = '-';	
	data[i++] = '0';
	data[i++] = 0;
}


/*
@功能：生成订货支付二维码数据
@返回值：二维码数据指针
*/
static void generate_order_ercode(char* data)
{
	uint8_t i, j, k, len;
	char tmp[10];
	CUR_TIME curTime = get_cur_time();//获得当前时间;

		strcpy(data, (const char*)order_addr)	;//域名拷贝
		i = strlen((const char*)order_addr);
		
	sprintf(tmp, "%10u", class_global.net.id);//机器ID
	for(j = 0;j < 10; j++)
		data[i++]=tmp[j];
	data[i++] = '-';
	
	/*
	时间戳
	这个需要记录下来，每次查询需要
	*/
	k = 0;
	len = sprintf(tmp, "%04u", curTime.year + 2000);
	for(j = 0;j < 4; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.month);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.day);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.hour);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.min);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	len = sprintf(tmp, "%02u", curTime.sec);
	for(j = 0;j < 2; j++)
	{
		data[i++] = tmp[j];
		class_global.qrcode.arr_time[k++] = tmp[j];
	}
	
	data[i++] = '-';	
	len = sprintf(tmp, "%u", class_global.trade.sel.price);//货道价格
	for(j = 0;j < len; j++)
		data[i++] = tmp[j];
	
	data[i++] = '-';
	data[i++] = '0';
	data[i++] = '-';	
	data[i++] = '0';
	data[i++] = 0;
}
#endif
/*
@功能：生成CPUID二维码数据
@返回值：二维码数据指针
*/
static void generate_cpuid_ercode(char* data)
{
	char cpu_id[30];
	sprintf(cpu_id, 		 "%010u", class_global.sys.unique_id[0] );
	sprintf(&cpu_id[10], "%010u", class_global.sys.unique_id[1] );
	sprintf(&cpu_id[20], "%010u", class_global.sys.unique_id[2] );
	strcpy(data, (const char*)cpu_id);
}


/*
@功能：生成二维码接口
@参数：type，类型：0->生成支付二维码，1->生成订货二维码；2->生成CPUID二维码
		x，y：二维码左上坐标
		scale：二维码版本号
*/
void creat_ercode(uint16_t x, uint16_t y, uint8_t version)
{
	char fillContent[100];
	uint8_t ver;
	uint8_t (*qr_data)[61];
	uint16_t show_data, valid_data;

	//@@生成二维码数据内容
	#if(0)
	if(version == SCAN_QRCODE)//扫码
	{
		generate_scan_ercode(fillContent);
	}
	else
	if(version == ORDER_QRCODE)//订奶
	{
		generate_order_ercode(fillContent);
	}
	else
	if(version == CPU_QRCODE)//CPUID
	#endif
	{
		generate_cpuid_ercode(fillContent);
	}
	
	if(ver > 10)//属于版本1~10的拓展版本，就是按比例放大1~10版本。@0x11就是版本1放大2倍
		ver = version & 0xf;
	else
		ver = version;

	show_data = ver*4 + 21;//需要显示的点阵宽度(包括边框)
	valid_data = show_data - 4;//有效数据点阵宽度(不包括边框)

	qr_data = pvPortMalloc(61*61);//申请点阵内存
	configASSERT(qr_data);
	memset(qr_data, 0, 61*61);//初始点阵内存
	
	//@@产生二维码点阵数据
	enCodeAndPrintQR(x, y, fillContent, qr_data, valid_data, ver);
	
	//@@二维码打印到屏幕
	lcd_show_ercode(x, y, show_data, version, qr_data);
	
	vPortFree(qr_data);
}

/*
@功能：清除屏幕上的二维码
*/
void delete_ercode(uint16_t x, uint16_t y, uint8_t version)
{
	uint8_t ver;
	uint16_t show_data;
	
	if(ver > 10)//属于版本1~10的拓展版本，就是按比例放大1~10版本。@0x11就是版本1放大2倍
		ver = version & 0xf;
	else
		ver = version;

	show_data = ver*4 + 21;//需要显示的点阵宽度(包括边框)
//	_lcd_clear_ercode(x, y, show_data, version);
}
