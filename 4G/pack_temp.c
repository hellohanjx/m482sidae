#include "user_rtc.h"
#include "msg.h"


/*
@包类别：0x12-0x32
@功能：即时包-机器状态（目前只用来定时发温度）
@参数：	type：传输类别：1温度，2湿度，3电量
				state：状态:0,异常；1，正常（与平台相反，平台1表示异常，0表示正常）
				value：对应值
				@说明：以前汇报包，现在是即时包
*/
void instant_equipment_state(uint8_t type, uint8_t state, uint32_t value)
{
	char i,j, tmp[10],len;
	uint8_t*str;
	MAIL * cmail;
	CUR_TIME curTime = get_cur_time();//获得当前时间

	cmail=mail_apply(100); //申请信封
	configASSERT(cmail);
	if(cmail == 0)
		return ;
	cmail->com_call_back = NULL;
	str = cmail->addr;
	
	i = 0;
	str[i++]=0x12;
	str[i++]='*';
	str[i++]='*';
	str[i++]=0x32;
	str[i++]='*';
	
	//类别
	str[i++] = type;
	str[i++] = '*';
	
	//机柜号
	len = sprintf( tmp, "%u", 1 );
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//状态，对应平台：0正常，1异常
	if(state == 1)
		state = 0;
	else
	if(state == 0)
		state = 1;
	
	len = sprintf(tmp, "%u", state);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//值（这个地方平台0为正常）
	if(state == 0)
	{
		len = sprintf(tmp, "%d", value);
		for(j = 0; j < len; j++)
			str[i++] = tmp[j];
	}
	else
	{
		str[i++] = '0';
	}
	str[i++] = '*';
	
	//时间
	str[i++] = curTime.year + 1;
	str[i++] = curTime.month + 1;
	str[i++] = curTime.day + 1;
	str[i++] = curTime.hour + 1;
	str[i++] = curTime.min + 1;
	str[i++] = curTime.sec + 1;
	str[2] = i+1;
	
//	report_to_communication(cmail, FALSE);
	instant_queue_send(cmail, TAIL);
}

