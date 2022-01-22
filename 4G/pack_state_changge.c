#include "user_rtc.h"
#include "global.h"
#include "string.h"
#include "commucation.h"
#include "msg.h"

/*
@包类型：0x12-0x36
@功能：汇报-设备状态变化汇报
@参数： mode:	模式，0用户模式；1工厂模式
				type：设备类型；
				state：设备状态
				id:设备号
*/
void report_state_change(uint8_t mode, uint32_t type, uint16_t id, uint32_t state)
{
	char tmp[10], len;
	uint8_t i, j, *str;
	MAIL * cmail;
	CUR_TIME curTime = get_cur_time();//获得当前时间
	
	cmail = mail_apply(150); //申请信封
	configASSERT(cmail);
	if(cmail == 0)
	{
		printf("report_state_change() err\r\n");
		return ;
	}
	cmail->com_call_back = 0;//无回调处理
	str = cmail->addr;
	
	i = 0;
	str[i++] = 0x12;//大类
	str[i++] = '*';
	str[i++] = '*';
	
	if(mode)//@测试模式
	{
		str[i++] = 0X37;//小类
		str[i++] = '*';//分隔符
		//CPU ID
		sprintf(tmp, "%010u", class_global.sys.unique_id[0]);
		for(j = 0; j < 10; j++)
			str[i++] = tmp[j];
		sprintf(tmp, "%010u", class_global.sys.unique_id[1]);
		for(j = 0; j < 10; j++)
			str[i++] = tmp[j];
		sprintf(tmp, "%010u", class_global.sys.unique_id[2]);
		for(j = 0; j < 10; j++)
			str[i++] = tmp[j];
	}
	else//@用户模式
	{
		str[i++] = 0x36;//小类
	}
	str[i++] = '*';//分隔符

	
	//@@设备类型
	str[i++] = '1';	
	str[i++] = '*';//分隔符
	
	//@@设备号
	str[i++] = id + '1';//设备号
	str[i++] = type + '0';//设备类型
	str[i++] = '*';//分隔符
	
	//@@设备状态：0正常，1异常
	len = sprintf(tmp, "%u", !state);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	str[i++] = '*';//分隔符
	
	//时间戳
	str[i++] = curTime.year  + 1;
	str[i++] = curTime.month + 1;
	str[i++] = curTime.day + 1;
	str[i++] = curTime.hour + 1;
	str[i++] = curTime.min + 1;
	str[i++] = curTime.sec + 1;
	str[2] = i+1;
	
	report_to_communication(cmail,FALSE);
}

