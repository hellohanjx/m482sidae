#include "user_rtc.h"
#include "msg.h"


/*
@�����0x12-0x32
@���ܣ���ʱ��-����״̬��Ŀǰֻ������ʱ���¶ȣ�
@������	type���������1�¶ȣ�2ʪ�ȣ�3����
				state��״̬:0,�쳣��1����������ƽ̨�෴��ƽ̨1��ʾ�쳣��0��ʾ������
				value����Ӧֵ
				@˵������ǰ�㱨���������Ǽ�ʱ��
*/
void instant_equipment_state(uint8_t type, uint8_t state, uint32_t value)
{
	char i,j, tmp[10],len;
	uint8_t*str;
	MAIL * cmail;
	CUR_TIME curTime = get_cur_time();//��õ�ǰʱ��

	cmail=mail_apply(100); //�����ŷ�
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
	
	//���
	str[i++] = type;
	str[i++] = '*';
	
	//�����
	len = sprintf( tmp, "%u", 1 );
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//״̬����Ӧƽ̨��0������1�쳣
	if(state == 1)
		state = 0;
	else
	if(state == 0)
		state = 1;
	
	len = sprintf(tmp, "%u", state);
	for(j = 0;j < len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//ֵ������ط�ƽ̨0Ϊ������
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
	
	//ʱ��
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

