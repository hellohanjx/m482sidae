/*
����֧��
*/
#include "user_rtc.h"
#include "global.h"
#include "string.h"
#include "commucation.h"
#include "msg.h"

/*
@���ܣ�����֧������������ص�����
@������recv��ƽ̨��������ָ�룻len��ƽ̨�������ݳ��ȣ�send�����͵�ƽ̨����ָ�룻id������������Դ
*/
static uint8_t callback_online_cardPay(uint8_t *recv, uint16_t len, uint8_t*send, uint8_t id)
{
	uint8_t i, rs = FALSE, chk;
	
	for(i = 0, chk = 0; i < (recv[2] - 1); i++)
		chk += recv[i];
	if(chk == recv[recv[2] - 1])
	{
		IREADER_MSG msg;
		msg.id = id;//����������Դ
		msg.type = ONLINE_CARD_PAY_CMD;	//��ʾ֧������
		msg.value = 0x50;								//��ʾ��ʱδ�ظ�
		
		if(recv[0] == 0x1D && recv[1] == send[1])//��ͷ�������ȷ
		{
			msg.value = recv[3];//���Ļ�Ӧ����
			//��Ҫ���볤���ж�
			if(recv[3] == 0X30)//����֧���ɹ�
			{
				uint32_t fac ;
				uint8_t tmp[10], n = 5, j = 0;
										
				for( (i = 0, j = n); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//�߼�����
					class_global.ireader[id].card.logic_char[i] = recv[i+j];
				}
				n++;
											
				for( (i = 0, fac = 1, j = n); (i < 10 && recv[i+j] != '*'); i++){//�������λ��
					tmp[i] = recv[i+j];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.balance = 0); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//�������
					class_global.ireader[id].card.balance += (tmp[i]-'0')*fac;
					fac /= 10;
				}
				n++;
				
				for( (i = 0, fac = 1, j = n); (i < 10 && recv[i+j] != '*'); i++){//�������Ѵ���λ����ԭʼ���������ǿ���������
					tmp[i] = recv[i+j];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.daymaxpay = 0); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//��������λ��
					class_global.ireader[id].card.daymaxpay += (tmp[i]-'0')*fac;
					fac /= 10;
				}
			}
			else//�ۿ�����ʧ��
			{
				for(i = 0;i < 10;i++)//�߼�����
					class_global.ireader[id].card.logic_char[i] = recv[5+i];
			}
			rs =  TRUE;
		}
		ireader_queue_send[id](msg);//��Ϣ���͵�����
	}
	else//У���
	{
		printf("ONLINE pay chk err\r\n\r\n");
	}
	return rs;
}




/*
@���ܣ����߲�ѯ����������ص�����
@������recv��ƽ̨��������ָ�룻len��ƽ̨�������ݳ��ȣ�send�����͵�ƽ̨����ָ��
*/
static uint8_t callback_online_cardCheck(uint8_t *recv, uint16_t len, uint8_t*send, uint8_t id)
{
	uint8_t i, rs = FALSE, chk;
	
	for(i = 0, chk = 0; i < (recv[2] - 1); i++)
		chk += recv[i];
	if(chk == recv[recv[2] - 1])
	{
		IREADER_MSG msg;
		msg.id = id;
		msg.type = ONLINE_CARD_CHECK_CMD;		//��ʾ��ѯ����
		msg.value = 0x50;		//��ʾ��ʱδ�ظ�
		
		if(recv[0] == 0x1D && recv[1] == send[1])//��ͷ�������ȷ
		{
			msg.value = recv[3];//���Ļ�Ӧ����
			//��Ҫ���볤���ж�
			if(recv[3] == 0X30)//����֧���ɹ�
			{
				uint32_t fac;
				uint8_t tmp[10], n = 5, j = 0;
										
				for( (i = 0, j = n);  (i < 10 && recv[i+j] != '*'); (i++, n++) ){//�߼�����
					class_global.ireader[id].card.logic_char[i] = recv[i+j];
				}
				n++;
				
				for( (i = 0, j = n, fac = 1); (i < 10 && recv[i+j] != '*'); i++){//�������λ��
					tmp[i] = recv[j+i];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.balance = 0); (i < 10 && recv[i+j] != '*'); i++, n++){//�������
					class_global.ireader[id].card.balance += (tmp[i]-'0')*fac;
					fac /= 10;
				}
				n++;
				
				
				for( (i = 0, j = n, fac = 1); (i < 10 && recv[i+j] != '*'); i++){//�������Ѵ���λ��
					tmp[i] = recv[j+i];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.daymaxpay = 0); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//�������Ѵ��� 
					class_global.ireader[id].card.daymaxpay += (tmp[i]-'0')*fac;//�������Ѵ��� 
				}
			}
			else//�ۿ��ѯʧ��
			{
				for(i = 0; i < 10; i++)//�߼�����
					class_global.ireader[id].card.logic_char[i] = recv[5+i];
			}
			rs =  TRUE;
		}
		ireader_queue_send[id](msg);//��Ϣ���͵�����
		}
	else//У���
	{
		printf("ONLINE check chk err\r\n\r\n");
	}
	return rs;
}


/*
@�����ͣ�0x17-0x41-0x31/0x32
@���ܣ���ʱ-���߿�����
@������id����Դʶ����
			 type��0x31 = ����֧����0x32 = ���߲�ѯ
			 trade_num��������ˮ��
*/
void requset_card_trade(uint8_t id, uint8_t type, uint32_t trade_num)
{
	uint8_t i,j;
	char tmp[10],len;
	uint8_t *str;
	MAIL *mail;
	CUR_TIME curTime;	
	
	curTime = get_cur_time();//��õ�ǰʱ��
	mail = mail_apply(150);
	configASSERT(mail);
	if(mail == 0)
		return;
	if(type == ONLINE_CARD_PAY_CMD)
		mail->com_call_back = callback_online_cardPay;//֧���ص�
	else
	if(type == ONLINE_CARD_CHECK_CMD)
		mail->com_call_back = callback_online_cardCheck;//��ѯ�ص�	
	else
		mail->com_call_back = NULL;
	
	mail->id = id;
	
	str = mail->addr;
	
	i = 0;
	str[i++]=0x17;
	str[i++]=' ';//���(����ʱ����)
	str[i++]=' ';//�ܳ���(У��һ�����)
	str[i++]=0x41;
	//str[i++] = '*';//��������岻��Ҫ

	//ˢ���ӿ�
	sprintf(tmp, "%04u", class_global.ireader[id].equ.interface);
	for(j = 0;j < 4;j++)
		str[i++] = tmp[j];
	
	str[i++] = ';';
	
	//ָ������
	str[i++] = type ;
	str[i++] = ';';
	
	//����id
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0; j < 10; j++)
	{
		str[i++]=tmp[j];
	}
	str[i++] = '*';
	
	//������ˮ��
	len = sprintf(tmp, "%u" , trade_num);
	for(j=0;j<len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	//�߼�����(Ĭ��д0)
	str[i++]= '0';
	str[i++]= '*';
	
	//������
	for(j = 0; j < 50 && class_global.ireader[id].card.physic_char[j] != 0; j++)
	{
		str[i++]= class_global.ireader[id].card.physic_char[j];
	}
	str[i++] = '*';
	
	//�����š���ˢ����id��
	len = sprintf(tmp, "%02u", id);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//�۸��ý��׼۸�
	len = sprintf(tmp,"%u",class_global.trade.price);
	for(j=0;j<len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	//ʱ��
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
	str[i++] = '*';//У��Ԥ��λ
	str[2] = i+1;
	
	instant_queue_send(mail, TAIL);//��������β
}



/*
@�����ͣ�0x17-0x41-0x33/0x34
@���ܣ��㱨-����ˢ�����
@������id��ˢ����id
			 type��0x34��ˢ��ʧ�ܣ�0x33��ˢ���ɹ�
			 trade_num�����׺�
*/
void requset_card_result(uint8_t id, uint8_t type, uint32_t trade_num)
{
	uint8_t i,j;
	char tmp[10],len;
	uint8_t *str;
	MAIL *mail;
	CUR_TIME curTime;	
	
	curTime = get_cur_time();//��õ�ǰʱ��
	mail = mail_apply(150); //�����ŷ�
	configASSERT(mail);
	if(mail == 0)
		return;
	mail->com_call_back = NULL;//�ص�����
	str = mail->addr;
	
	i = 0;
	str[i++]=0x17;
	str[i++]=' ';//���(����ʱ����)
	str[i++]=' ';//�ܳ���(У��һ�����)
	str[i++]=0x41;
	//str[i++] = '*';//��������岻��Ҫ

	//�ӿڱ���
	sprintf(tmp, "%04u", class_global.ireader[id].equ.interface);
	for(j = 0;j < 4;j++)
		str[i++] = tmp[j];
	str[i++] = ';';
	
	//ָ������
	str[i++] = type;
	str[i++] = ';';
	
	//����id
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0; j < 10; j++)
	{
		str[i++]=tmp[j];
	}
	str[i++] = '*';
	
	//������ˮ��
	len = sprintf(tmp, "%u" , trade_num);
	for(j=0;j<len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	//�߼�����(Ĭ��д0)
	str[i++]= '0';
	str[i++]= '*';
	
	//������
	for(j = 0; j < 50 && class_global.ireader[id].card.physic_char[j] != 0; j++)
	{
		str[i++]= class_global.ireader[id].card.physic_char[j];
	}
	str[i++] = '*';
	
	//�����š���ˢ����id��
	len = sprintf(tmp, "%02u", id);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//�۸��ý��׼۸�
	len = sprintf(tmp,"%u",class_global.trade.price);
	for(j=0;j<len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//ʱ��
	len = sprintf(tmp,"%04u",curTime.year + 2000);
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
	str[i++] = '*';//У��Ԥ��λ
	str[2] = i+1;
	
	report_to_communication(mail, TAIL);
}
