/*
@��������ʾ��ά��
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


const char order_addr[] = {"CradpicksmApi.php?id="};//���̶�ά���ַ

#if(0)
/*
@���ܣ�����ɨ��֧����ά������
@����ֵ����ά������ָ��
*/
static void generate_scan_ercode(char* data)
{
	uint8_t i, j, k, len;
	char tmp[10];
	CUR_TIME curTime = get_cur_time();//��õ�ǰʱ��;

	strcpy(data, (const char*)class_global.qrcode.arr_domain)	;//��������
	i = strlen((const char*)class_global.qrcode.arr_domain);
		
	sprintf(tmp, "%10u", class_global.net.id);//����ID
	for(j = 0;j < 10; j++)
		data[i++]=tmp[j];
	data[i++] = '-';
	
	/*
	ʱ���
	�����Ҫ��¼������ÿ�β�ѯ��Ҫ
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
	len = sprintf(tmp, "%u", class_global.trade.sel.price);//�����۸�
	for(j = 0;j < len; j++)
		data[i++] = tmp[j];
	
	data[i++] = '-';
	data[i++] = '0';
	data[i++] = '-';	
	data[i++] = '0';
	data[i++] = 0;
}


/*
@���ܣ����ɶ���֧����ά������
@����ֵ����ά������ָ��
*/
static void generate_order_ercode(char* data)
{
	uint8_t i, j, k, len;
	char tmp[10];
	CUR_TIME curTime = get_cur_time();//��õ�ǰʱ��;

		strcpy(data, (const char*)order_addr)	;//��������
		i = strlen((const char*)order_addr);
		
	sprintf(tmp, "%10u", class_global.net.id);//����ID
	for(j = 0;j < 10; j++)
		data[i++]=tmp[j];
	data[i++] = '-';
	
	/*
	ʱ���
	�����Ҫ��¼������ÿ�β�ѯ��Ҫ
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
	len = sprintf(tmp, "%u", class_global.trade.sel.price);//�����۸�
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
@���ܣ�����CPUID��ά������
@����ֵ����ά������ָ��
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
@���ܣ����ɶ�ά��ӿ�
@������type�����ͣ�0->����֧����ά�룬1->���ɶ�����ά�룻2->����CPUID��ά��
		x��y����ά����������
		scale����ά��汾��
*/
void creat_ercode(uint16_t x, uint16_t y, uint8_t version)
{
	char fillContent[100];
	uint8_t ver;
	uint8_t (*qr_data)[61];
	uint16_t show_data, valid_data;

	//@@���ɶ�ά����������
	#if(0)
	if(version == SCAN_QRCODE)//ɨ��
	{
		generate_scan_ercode(fillContent);
	}
	else
	if(version == ORDER_QRCODE)//����
	{
		generate_order_ercode(fillContent);
	}
	else
	if(version == CPU_QRCODE)//CPUID
	#endif
	{
		generate_cpuid_ercode(fillContent);
	}
	
	if(ver > 10)//���ڰ汾1~10����չ�汾�����ǰ������Ŵ�1~10�汾��@0x11���ǰ汾1�Ŵ�2��
		ver = version & 0xf;
	else
		ver = version;

	show_data = ver*4 + 21;//��Ҫ��ʾ�ĵ�����(�����߿�)
	valid_data = show_data - 4;//��Ч���ݵ�����(�������߿�)

	qr_data = pvPortMalloc(61*61);//��������ڴ�
	configASSERT(qr_data);
	memset(qr_data, 0, 61*61);//��ʼ�����ڴ�
	
	//@@������ά���������
	enCodeAndPrintQR(x, y, fillContent, qr_data, valid_data, ver);
	
	//@@��ά���ӡ����Ļ
	lcd_show_ercode(x, y, show_data, version, qr_data);
	
	vPortFree(qr_data);
}

/*
@���ܣ������Ļ�ϵĶ�ά��
*/
void delete_ercode(uint16_t x, uint16_t y, uint8_t version)
{
	uint8_t ver;
	uint16_t show_data;
	
	if(ver > 10)//���ڰ汾1~10����չ�汾�����ǰ������Ŵ�1~10�汾��@0x11���ǰ汾1�Ŵ�2��
		ver = version & 0xf;
	else
		ver = version;

	show_data = ver*4 + 21;//��Ҫ��ʾ�ĵ�����(�����߿�)
//	_lcd_clear_ercode(x, y, show_data, version);
}
