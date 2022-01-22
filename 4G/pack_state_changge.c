#include "user_rtc.h"
#include "global.h"
#include "string.h"
#include "commucation.h"
#include "msg.h"

/*
@�����ͣ�0x12-0x36
@���ܣ��㱨-�豸״̬�仯�㱨
@������ mode:	ģʽ��0�û�ģʽ��1����ģʽ
				type���豸���ͣ�
				state���豸״̬
				id:�豸��
*/
void report_state_change(uint8_t mode, uint32_t type, uint16_t id, uint32_t state)
{
	char tmp[10], len;
	uint8_t i, j, *str;
	MAIL * cmail;
	CUR_TIME curTime = get_cur_time();//��õ�ǰʱ��
	
	cmail = mail_apply(150); //�����ŷ�
	configASSERT(cmail);
	if(cmail == 0)
	{
		printf("report_state_change() err\r\n");
		return ;
	}
	cmail->com_call_back = 0;//�޻ص�����
	str = cmail->addr;
	
	i = 0;
	str[i++] = 0x12;//����
	str[i++] = '*';
	str[i++] = '*';
	
	if(mode)//@����ģʽ
	{
		str[i++] = 0X37;//С��
		str[i++] = '*';//�ָ���
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
	else//@�û�ģʽ
	{
		str[i++] = 0x36;//С��
	}
	str[i++] = '*';//�ָ���

	
	//@@�豸����
	str[i++] = '1';	
	str[i++] = '*';//�ָ���
	
	//@@�豸��
	str[i++] = id + '1';//�豸��
	str[i++] = type + '0';//�豸����
	str[i++] = '*';//�ָ���
	
	//@@�豸״̬��0������1�쳣
	len = sprintf(tmp, "%u", !state);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	str[i++] = '*';//�ָ���
	
	//ʱ���
	str[i++] = curTime.year  + 1;
	str[i++] = curTime.month + 1;
	str[i++] = curTime.day + 1;
	str[i++] = curTime.hour + 1;
	str[i++] = curTime.min + 1;
	str[i++] = curTime.sec + 1;
	str[2] = i+1;
	
	report_to_communication(cmail,FALSE);
}

