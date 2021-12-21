#include "4G.h"
#include "msg.h"
#include "global.h"
#include "uart7_config.h"
#include "commucation.h"
#include "user_config.h"
#include "user_rtc.h"

/*
������Ӧ����
*/
#define ACK_OK     0X30 //ȷ��Ӧ��
#define ACK_CHK    0X31 //У��ʹ���
#define ACK_NA     0X32 //��Ӧ��
#define ACK_REJECT 0X33 //�ܾ�ִ��
#define ACK_BUSY   0X34 //ϵͳæ
#define ACK_START  0X35 //������ʼ
#define ACK_END    0X36 //����ִ�����


/*
@���ܣ���ȡ����״̬
@������state������ֵ����
@����ֵ���Ƿ�ȡ������״̬
*/
uint8_t get_network_state(uint8_t *state)
{
	#if BOARD_4G == USER_EN
		*state = get_4g_state();
		return TRUE;
	#else
		return FALSE;
	#endif
}


/*
@���ܣ��㱨��Ϣ
@������mymail,����ָ�룻type��0�ӵ�����β��1�ӵ�����ͷ
*/
void add_report_info(MAIL* mymail, uint8_t type)
{
	if(type == FALSE)//�ӵ�����β
	{
		if(report_to_tail(mymail))
			class_global.net.backlog_data++;//��ѹ��������
	}
	else//�ӵ�����ͷ
	{
		if(report_to_head(mymail))
			class_global.net.backlog_data++;//��ѹ��������
	}
}

/*
@���ܣ�ͨѶһ��㱨���ص�����
@������*rx����������
@˵����ֻ��֤��У��ͳ���
*/
static uint8_t callback_com_recv(UART7_DATA *rx)
{
	if(rx->buf[2] == rx->len)//���ճ��ȷ���
	{
		uint8_t i,chk;
		for(i = 0,chk = 0;i < (rx->len-1);i++)
			chk += rx->buf[i];
		if(chk == rx->buf[rx->len-1])
		{
			commucation_sem_send_isr(); //�жϼ��ź����ͷţ������ͷŻ������ź�����
		}
	}
	return TRUE;
}



/*
@���ܣ��������ݵ�����
@������*dat������������ָ�룻rx����������ָ��ĵ�ַ
*/
static void mail_to_bus(uint8_t *dat, UART7_DATA** rx)
{
	uint8_t chk, len = dat[2], cnt = 0;
	uint16_t i, j;

	if(dat[0] != 0x1f && dat[0] != 0x10 && dat[0] != 0x11 && dat[0] != 0x12
	&& dat[0] != 0x14	&& dat[0] != 0x17 && dat[0] != 0xff)
	{
		//��������
		uint8_t *buf = 0;
		do{
			buf = pvPortMalloc(300);
			configASSERT(buf);
			if(buf)
			{
				i = 0;
				//��ͷ
				buf[i++] = 0x1C;	//���������0x1C
				buf[i++] = dat[1];//ʹ��ԭʼ�����
				buf[i++] = '*';	 	//���������
				buf[i++] = 0x11;	//ҵ������
				buf[i++] = '*';//�ָ���
				//����
				for(j = 0; j < (len-1); j++)
					buf[i++] = dat[j];
				buf[i++] = '*';//�ָ���
				buf[i++] = ' ';//У��ռλ
				buf[2] = i;
				//У��
				for(j = 0,chk = 0; j < (i-1) && j < 256; j++)
					chk += buf[j];
				buf[i-1] = chk;
				
				_uart7_send(&(*rx), buf, i, callback_com_recv);//���ڷ���
				vPortFree(buf);
			}
			else
			{
				printf("mail_to_bus err\r\n");
				if(cnt++ > 5)
				{
					restart_equ_set(RESET_MailToBus, TRUE);
				}
			}
		}while(buf == 0);
	}
	else//�������
	{
		for(i = 0,chk = 0; i < (len-1) ; i++)//����У���
		{
			chk += dat[i];
		}
		dat[i++] = chk;//���У���
		_uart7_send(&(*rx), dat, i, callback_com_recv);//���ڷ��� 
	}
}

/*
@���ܣ�ѯ���Ƿ�����
@��������
@���أ�0~4
*/
static uint8_t send_restart(void)
{
	UART7_DATA  *pt_rx;//���ջ���ָ��
	MAIL *mail;//�ŷ�ָ��
	uint8_t *pt_txbuf;//��������ָ��
	uint8_t err, rs; 
	
	mail = mail_apply(50);//�����ڴ�ռ�
	configASSERT(mail);
	if(mail != 0)//���뵽�ռ�
	{
		pt_txbuf = mail->addr;

		get_reset_cmd(pt_txbuf);
		pt_txbuf[1] = class_global.net.number + 1;//�����
		class_global.net.number = (class_global.net.number+1)%255;//���±��
		mail_to_bus(pt_txbuf, &pt_rx);
		err = commucation_sem_get(ONE_SECOND*30);//30s ��ʱ
		if(err == pdTRUE)
		{
			if(pt_rx->buf[0] == 0x1D && pt_rx->buf[1] == pt_txbuf[1] && pt_rx->buf[8] == 0x31)//��������
			{
				class_global.sys.reset = SWITCH_ON;
				if(pt_rx->buf[9] == 0x31)//��������Ҫ����
				{
					//@�µķ���=��9�ֽڻ������ͣ������ǵ���ź�
					
					uint8_t tmp;
					
					if(pt_rx->buf[10] >=  '0' && pt_rx->buf[10] <= '9')
					{
						tmp = pt_rx->buf[10] - '0';
					}
					else
					if(pt_rx->buf[10] >=  'A' && pt_rx->buf[10] <= 'Z')
					{
						tmp = pt_rx->buf[10] - 'A' + 10;
					}
					//���û���					
					tmp = pt_rx->buf[11] - '0';
					//��Ļ����(Ŀǰֻ��2��)
					tmp = pt_rx->buf[12] - '0';
					//�������(Ŀǰֻ��2����)
					tmp = pt_rx->buf[13] - '0';
					//ˢ��������(Ŀǰֻ��2��)
					
					rs = 1;//���������������˲���
					printf("restart and updata\r\n\r\n");
				}
				else
				if(pt_rx->buf[9] == 0x32)//�·�ʽ-�����޸�
				{
					uint8_t tmp;
					
					if(pt_rx->buf[10] != '0')//���ͣ�0�ַ����޸Ļ���
					{
						if(pt_rx->buf[10] >=  '1' && pt_rx->buf[10] <= '9')
						{
							tmp = pt_rx->buf[10] - '0' - 1;
						}
						else
						if(pt_rx->buf[10] >=  'A' && pt_rx->buf[10] <= 'Z')
						{
							tmp = pt_rx->buf[10] - 'A' + 9;
						}
						//���û���
					}
					
					tmp = (pt_rx->buf[11] - '0')*100 + (pt_rx->buf[12] - '0')*10 + (pt_rx->buf[13] - '0');//����ź�
				}
				else
				{
					rs = 2;//����������û�в�����Ҫ����
					printf("restart but no updata\r\n\r\n");
				}
			}
			else
			{
				rs = 3;//����Ҫ��������
				printf("restart unused\r\n\r\n");
			}
		}
		else
		{
			rs = 4;//�������Ӧ��
			printf("restart ack err\r\n\r\n");
		}
		mail_release(mail);//�ͷ�����ռ�
	}
	else
	{
		rs = 0;//û���뵽�ռ�
		printf("apply mail err_3\r\n\r\n");
	}
	return rs;
}

/*
@���ܣ�������ȡ����
*/
static uint8_t get_param(uint8_t *str, UART7_DATA *rx)
{
	uint8_t rs = TRUE;
	uint8_t err;

	//�û�ģʽ����17 42 7~9���Ͱ�
	if( !class_global.sys.factory_en )//�û�ģʽ
	{
		get_set_param(str, 7);//17-42-7�����������в�����
		str[1] = class_global.net.number+1;
		class_global.net.number = (class_global.net.number+1)%255;
		
		mail_to_bus(str, &rx);
		err = commucation_sem_get(ONE_SECOND*7);
		if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
		{
			if( !analysis_17_42_7(rx->buf, rx->len) )//�������ذ�
			{
				rs = FALSE;
				printf("17 42 7 ack format err\r\n\r\n");
			}
			
			
			get_set_param(str, 8);//17-42-8�������� ��ʹ��/״̬��
			str[1] = class_global.net.number+1;
			class_global.net.number = (class_global.net.number+1)%255;
			
			mail_to_bus(str, &rx);
			err = commucation_sem_get(ONE_SECOND*7);
			if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
			{
				if( !analysis_17_42_8(rx->buf, rx->len) )//�������ذ�
				{
					rs = FALSE;
					printf("17 42 8 ack format err\r\n\r\n");
				}
				
				
				get_set_param(str, 9);//17-42-9�������� �۸�/��桿
				str[1] = class_global.net.number+1; 
				class_global.net.number = (class_global.net.number+1)%255;
				
				mail_to_bus(str, &rx);
				err = commucation_sem_get(ONE_SECOND*7);
				if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
				{
					if( !analysis_17_42_9(rx->buf, rx->len) )//�������ذ�
					{
						rs = FALSE;
						printf("17 42 9 ack format err\r\n\r\n");
					}
				}
				else
				{
					rs = FALSE;
					printf("17 42 9 ack err\r\n\r\n");
				}
			}
			else
			{
				rs = FALSE;
				printf("17 42 8 ack err\r\n\r\n");
			}
		}
		else
		{
			rs = FALSE;
			printf("17 42 7 ack err\r\n\r\n");
		}
	}
	return rs;
}


/*
�豸��¼
*/
static void send_linking(uint8_t* first, uint32_t *correct, uint8_t *redownload)//����
{
	MAIL *cmail;
	uint8_t *str; 
	uint8_t err,i ;
	uint8_t download_result;
	UART7_DATA *rx;//��������ָ��
	
	cmail = mail_apply(PACK_MAX_SIZE);//�������ݿռ�
	configASSERT(cmail);
	if(cmail != 0)
	{
		str = cmail->addr; 
		err = pdFALSE;

		get_link_info(str, 0);//��������
		str[1] = class_global.net.number+1; //�����
		class_global.net.number = (class_global.net.number+1)%255;//���±��
		mail_to_bus(str, &rx);
		err = commucation_sem_get(ONE_SECOND*7);//�ȴ�Ӧ���ź���
		if(err == pdTRUE)
		{
			i=0;
			do
			{//��ֹ��һ������Ӧ�𱻵�����ʱ������ʱ����´���
				uint32_t timelast;
				if(i == 0)
					timelast = xTaskGetTickCount();
				err = commucation_sem_get(ONE_SECOND);//�ȴ�1s
				if(xTaskGetTickCount()-timelast >= ONE_SECOND)
				{
					timelast = xTaskGetTickCount();
				}
				i++;
			}while( ( !(err == pdFALSE && rx->buf[0] == 0x1D && rx->buf[1] == str[1]) ) && i < 100 );//���������Ӧ��
			
			if(err == pdFALSE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
			{
				uint8_t ctime[6];
				ctime[0] = (rx->buf[3]-'0')*10 + rx->buf[4]-'0';
				ctime[1] = (rx->buf[5]-'0')*10 + rx->buf[6]-'0';
				ctime[2] = (rx->buf[7]-'0')*10 + rx->buf[8]-'0';
				ctime[3] = (rx->buf[9]-'0')*10 + rx->buf[10]-'0';
				ctime[4] = (rx->buf[11]-'0')*10 + rx->buf[12]-'0';
				ctime[5] = (rx->buf[13]-'0')*10 + rx->buf[14]-'0';
				
				if( set_rtc_time(1, ctime[0], ctime[1], ctime[2], ctime[3], ctime[4], ctime[5]) )//����RTCʱ��
				{
					send_restart();//����������ѯ
					
					if(*first)//�ϵ��һ������
					{
						do{
							download_result = get_param(str, rx );
						}while( (download_result != TRUE) && ((*redownload)++ < PARAM_REDOWNED_CNT) );
						
						if( !download_result )//�������ݴ���
						{
							uint16_t random = get_random_number();
							do{
								random /= 2;
							}while(random > 5000);
							
							//�������ش���
							//2s����������
							vTaskDelay(random*1000);
							
							restart_equ_set(RESET_Param, TRUE);//��λ����
						}
						else//��������������ȷ
						{
							get_channel_status(str);
							str[1] = class_global.net.number+1; //���ϱ��
							class_global.net.number = (class_global.net.number+1)%255;//���±��
							mail_to_bus(str, &rx); //���͵�Զ�̹����
							err = commucation_sem_get(ONE_SECOND*7);
							if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
							{
								get_machine_status(str);
								str[1] = class_global.net.number+1; //���ϱ��
								class_global.net.number = (class_global.net.number+1)%255;//���±��
								
								mail_to_bus(str, &rx); //���͵�Զ�̹����
								err = commucation_sem_get(ONE_SECOND*7);
								if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
								{
									//�������ò��� 20191021
									get_vm_set(str);
									str[1] = class_global.net.number+1; //���ϱ��
									class_global.net.number = (class_global.net.number+1)%255;//���±��
									
									mail_to_bus(str, &rx); //���͵�Զ�̹���ˡ�
									err = commucation_sem_get(ONE_SECOND*7);
									if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
									{
										get_software_version(str);
										str[1] = class_global.net.number+1; //���ϱ��
										class_global.net.number = (class_global.net.number+1)%255;//���±��
										
										mail_to_bus(str, &rx); //���͵�Զ�̹����
										err = commucation_sem_get(ONE_SECOND*7);
										if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
										{									
											//����ģʽ����
											if( get_factory_set(str, 1) )
											{
												str[1] = class_global.net.number+1; //���ϱ��
												class_global.net.number = (class_global.net.number+1)%255;//���±��
												
												mail_to_bus(str, &rx); //���͵�Զ�̹����
												err = commucation_sem_get(ONE_SECOND*7);
												if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
												{
													updata_factory_set(rx->buf, rx->len, str);//�����յ�����
													*first = 0;
													class_global.net.state = 1;//�������״̬Ϊ����
													restart_equ_set(RESET_NormalLink, FALSE);//��λ��־�ָ���ʼֵ
													(*correct) = xTaskGetTickCount();
												}
												else
												{
													printf("17 41 1 ack err\r\n\r\n");
												}
											}
											else
											{
												*first=0;
												class_global.net.state=1;//�������״̬Ϊ����
												(*correct) = xTaskGetTickCount();
												restart_equ_set(RESET_NormalLink, FALSE);//��λ��־�ָ���ʼֵ
											}
										}
										else
										{
											printf("1f-35 ack err\r\n\r\n");
										}
									}
									else
									{
										printf("1f-32 ack err\r\n\r\n");
									}
								}
								else
								{
									printf("1f-34 ack err\r\n\r\n");
								}
							}
							else
							{
								printf("1f-31 ack err\r\n\r\n");
							}
						}
					}
					else//�м�����������ӳɹ�
					{
						class_global.net.state = 1;
						(*correct) = xTaskGetTickCount();
					}
				}
				else
				{
					printf("rtc set err\r\n\r\n");
				}
			}
			else
			{
				printf("1f 30 ack err\r\n\r\n");
			}
		}
		else
		{
			printf("1f 30 no ack\r\n\r\n");
		}
		
		mail_release(cmail);
	}
	else
	{
		printf("apply mail err_1\r\n\r\n");
	}
}


/*
��������
*/
static uint8_t send_heart(uint32_t *hearTime, uint8_t *xintiao)//������
{
	uint8_t rs = TRUE ,err ;
	if(xTaskGetTickCount() - (*hearTime) >= ONE_SECOND*40)//40s����һ��������
	{
		UART7_DATA *rx;
		
		(*hearTime) = xTaskGetTickCount();
		xintiao[0]=0xff;
		xintiao[1] = class_global.net.number+1;
		xintiao[2]=4;
		class_global.net.number = (class_global.net.number+1)%255;
		
		mail_to_bus(xintiao, &rx);
		err = commucation_sem_get(ONE_SECOND*10);
		if( !(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1] == xintiao[1] && rx->buf[2] == 4) )//��Ҫ�ж�������Ӧ�Ƿ���ȷ
		{
			class_global.net.state = 0;
			rs = FALSE;
			printf("heart ack err\r\n\r\n");
		}
	}
	return rs;
}

/*
���Ͷ�ʱ
*/
static uint8_t send_correct_time(uint32_t *correct)//����ʱ
{
	uint8_t err,rs = TRUE; 
		
	if(xTaskGetTickCount()-(*correct) > ONE_SECOND*30*60)//30�ֶ�ʱһ��
	{
		UART7_DATA  *rx;//���ջ���ָ��
		MAIL *cmail;
		uint8_t *pt_tx;

		cmail = mail_apply(50);
		configASSERT(cmail);
		if(cmail != 0)
		{
			pt_tx = cmail->addr;
			
			(*correct) = xTaskGetTickCount();//���¶�ʱ��ʱ
			
			get_link_info(pt_tx, 1);//���ʱ��
			pt_tx[1] = class_global.net.number+1;//���ϱ��
			class_global.net.number = (class_global.net.number+1)%255;//���±��
			mail_to_bus(pt_tx, &rx);
			err = commucation_sem_get(ONE_SECOND*30);//30s ��ʱ
			if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == pt_tx[1])
			{
				uint8_t ctime[6];
				ctime[0] = (rx->buf[3]-'0')*10 + rx->buf[4]-'0';
				ctime[1] = (rx->buf[5]-'0')*10 + rx->buf[6]-'0';
				ctime[2] = (rx->buf[7]-'0')*10 + rx->buf[8]-'0';
				ctime[3] = (rx->buf[9]-'0')*10 + rx->buf[10]-'0';
				ctime[4] = (rx->buf[11]-'0')*10 + rx->buf[12]-'0';
				ctime[5] = (rx->buf[13]-'0')*10 + rx->buf[14]-'0';
				
				set_rtc_time(1, ctime[0], ctime[1], ctime[2], ctime[3], ctime[4], ctime[5]);
				
				send_restart();//����������ѯ
			}
			else
			{	
				printf("updata time ack err\r\n\r\n");
				class_global.net.state = 0;
				rs = FALSE;
			}
			
			mail_release(cmail);//�ͷŵ����ŷ�
		}
		else
		{
			printf("apply mail err_2\r\n\r\n");
		}
	}
	return rs;
}

/*
@���ܣ����ͼ�ʱ����
@���أ�1�ɹ�
*/
static uint8_t send_instant_data(void)
{
	uint8_t rs, err;
	MAIL *cmail;
	UART7_DATA *rx;
	
	err = instant_data_get((void*)&cmail, ONE_SECOND/10);//ȡ��ʱ����
	if(err == pdTRUE)
	{
		if(class_global.net.state)//������Ϊ������ٶ�
		{
			rs = TRUE;
			cmail->addr[1] = class_global.net.number + 1;//������ˮ��+1
			class_global.net.number = (class_global.net.number+1)%255;
			
			mail_to_bus(cmail->addr, &rx);
			err = commucation_sem_get(ONE_SECOND*10);
			if(err == pdTRUE)
			{
				if(cmail->com_call_back != 0)//���ע���˻ص������������
				{
					if(!(cmail->com_call_back(rx->buf, rx->len, cmail->addr))) //�ص���������
					{
						class_global.net.state = 0;//ֱ���ߵ��ߣ����·�������
						rs = 2;//����
					}
				}
				else//δע��ص�������������Щ��ͨ�ظ�����ʾ�յ�����ʲô�ģ�
				{
					if(rx->buf[0] == 0x1D && rx->buf[1] == cmail->addr[1] && rx->buf[3] == ACK_OK)//Ӧ����ȷ
					{
						;
					}
					else//Ӧ�����
					{
						class_global.net.state = 0;//ֱ�ӿ�ʼ��������
						rs = 2;//����
					}
				}
			}
			else//��Ӧ�����
			{
//				FSM_MSG* msg = msg_applay(MSG_SIZE_DEFAULT);
//				configASSERT(msg);
//				msg->type = MsgServerAckErr;//���ݴ�
//				msg->stype = 1;	//����
//				msg->value = 1;	//��ʱ��
//				if(fsm_cmd_send(TASK_MSG, msg) != pdPASS){
//					msg_release(msg);
//					printf("err-> ram data no answer\r\n\r\n");
//				}
//				class_global.net.state = 0;
//				rs = 2;//����
//				
//				server_err_save(0, cmail->addr);
			}
		}
		mail_release(cmail);//�ͷ����ݿռ�
	}
	else//û��������
	{
		rs = FALSE;
	}	
	return rs;
}

/*
@���ܣ����ͻ㱨����
@���أ�1�ɹ�
*/
static uint8_t send_report_data(void)
{
	uint8_t rs = FALSE, err;
	MAIL *cmail;
	UART7_DATA *rx;
	
	if(class_global.net.backlog_data)//�л�ѹ����
	{
		err = report_data_get((void*)&cmail, 0);//ȡ�㱨����
		if(err == pdTRUE)
		{
			class_global.net.backlog_data ? (class_global.net.backlog_data--) : 0;//��ѹ���ݼ�1
			rs = TRUE;
			cmail->addr[1] = class_global.net.number + 1;//������ˮ��+1
			class_global.net.number = (class_global.net.number+1)%255;
			
			mail_to_bus(cmail->addr, &rx);
			err = commucation_sem_get(ONE_SECOND*10);
			if(err == pdTRUE)
			{
				if(cmail->com_call_back)//���ע���˻ص������������
				{
					if(!(cmail->com_call_back(rx->buf, rx->len, cmail->addr))) //�ص���������
					{
						class_global.net.state = 0;//ֱ���ߵ��ߣ����·�������
						add_report_info(cmail, TRUE);//�ӵ�����ͷ
					}
					else
					{
						mail_release(cmail);//�ͷ����ݿռ�
					}
				}
				else//δע��ص�������������Щ��ͨ�ظ�����ʾ�յ�����ʲô�ģ�
				{
					if(rx->buf[0] == 0x1D && rx->buf[1] == cmail->addr[1] && rx->buf[3] == ACK_OK)//Ӧ����ȷ
					{
						mail_release(cmail);//�ͷ����ݿռ�
					}
					else//Ӧ�����
					{
						class_global.net.state = 0;//ֱ�ӿ�ʼ��������
						add_report_info(cmail, TRUE);//�ӵ�����ͷ
					}
				}
			}
			else//ƽ̨��Ӧ��
			{
//				FSM_MSG* msg = 0;
//				class_global.net.state = 0;
//				add_report_info(cmail, TRUE);//�ӵ�����ͷ
//				server_err_save(0, cmail->addr);//��¼��Ӧ������
//				
//				msg = msg_applay(MSG_SIZE_DEFAULT);
//				configASSERT(msg);
//				msg->type = MsgServerAckErr;//���ݴ�
//				msg->stype = 1;	//����
//				msg->value = 0;	//�㱨��
//				if(fsm_cmd_send(TASK_MSG, msg) != pdPASS){
//					msg_release(msg);
//					printf("err-> ram data no answer\r\n\r\n");
//				}
			}
		}
		else//�л�ѹ�����������޻�ѹ����
		{
			printf("no report != data num\r\n\r\n");
			rs = FALSE;
		}
	}
	return rs;
}






/*
@���ܣ�ͨѶ������
*/
void main_task_commucation(void)
{
	uint32_t heart,correct;
	uint8_t first_Link;
	uint8_t xintiao[6];
	uint8_t param_redownload = 0;
	
	heart = correct = xTaskGetTickCount();
	
	class_global.net.state = 0;	//״̬��λ
	class_global.net.number = 0;	//��ˮ�Ÿ�λ
	
	xintiao[0]=0xff;
	xintiao[2]=4;
	first_Link = TRUE;
	
	uart7_config();//��ʼ������
	vTaskDelay(ONE_SECOND/2);//�ȴ������豸��ʼ�����
	
	while(1)
	{
		main_task_4g();

		if(class_global.net.state)//���������
		{
			first_Link = FALSE;
			if(send_instant_data() != 2)//û�м�ʱ������Ҫ����
			{
				if(send_heart(&heart,xintiao))//����
					if(send_correct_time(&correct))//��ʱ
						send_report_data();//�㱨����
			}
		}
		else
		{
			send_linking(&first_Link, (uint32_t*)&correct, &param_redownload);//����
		}
	}
}
