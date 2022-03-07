/*
@˵����״̬������ˢ������Ϣ��ͨ����Ϣ���豸������Ϣ��������״̬��������
*/
#include "fsm.h"
#include "msg.h"
#include "global.h"
#include "commucation.h"
#include "task_swipe.h"
#include "swipe_led.h"
#include "tap_set.h"
#include "gpio_int.h"
#include "string.h"
#include "12864_driver.h"
#include "user_rtc.h"
#include "4G.h"
#include "key.h"
#include "isp_program.h"
#include "temp.h"

//����ģʽ�ļ���״̬
enum{ SET_INIT, SET_SETTING, SET_SHOWING };

//ͨ����ʾ����
static const uint8_t table_y[6] = { 1, 1, 2, 2, 3, 3 };
static const uint8_t table_x[6] = { 1, 5, 1, 5, 1, 5 };

/*
@֧������״̬
*/
enum{
	APPLY_FSM_INIT,//֧����ʼ״̬
	APPLY_FSM_REQUESTING,//֧��������
	APPLY_FSM_FLOWING,//��ˮ��
	APPLY_FSM_FAILURE,//����ʧ��
	APPLY_FSM_END,//���̽���
};


/*
@���ܣ���ʾʱ��
*/
static void display_time(void)
{
	CUR_TIME time = get_cur_time();
	char tmp[20] = {0}, i = 0;
	
	sprintf( &tmp[i], "%02u", time.year );
	i+=2;
	tmp[i++] = '-';
	sprintf( &tmp[i], "%02u", time.month );
	i+=2;
	tmp[i++] = '-';
	sprintf( &tmp[i], "%02u", time.day );
	i+=2;
	tmp[i++] = ' ';
	sprintf( &tmp[i], "%02u", time.hour );
	i+=2;
	tmp[i++] = ':';
	sprintf( &tmp[i], "%02u", time.min );
	lcd_show_letter( tmp, 0,	0 );
}

/*
@���ܣ���ʾ����״̬
*/
static void display_net_state(void)
{	
	if( class_global.net.state == 1 )
	{
		lcd_show_letter( " L", 7, 0 );
	}
	else
	{
		char tmp[10];
		tmp[ sprintf( tmp, "%02u", get_4g_state() ) ] = 0;
		lcd_show_letter( tmp, 7, 0 );
	}
}


/*
@���ܣ���ʾͨ��
*/
static void display_channel(void)
{
	char id, tmp[3];
	
	for(id = 0; id < IREADER_NUM_MAX; id++ )
	{
		if( class_global.ireader[id].equ.state == 1 )//ˢ��������
		{
			if( class_global.ireader[id].equ.work == IDLE )
			{
				if( class_global.ireader[id].display.state == TRUE )//��Ҫ��ʾ��Ķ���
				{
					if( xTaskGetTickCount() - class_global.ireader[id].display.start_time > class_global.ireader[id].display.hold_time )
					{
						class_global.ireader[id].display.state = FALSE;
						class_global.ireader[id].display.start_time = 0;
						class_global.ireader[id].display.hold_time = 0;
					}
				}
				else
				{
					tmp[ sprintf( tmp, "%u", id+1 ) ] = 0;
					lcd_show_letter( tmp, table_x[id]-1, table_y[id] );
					lcd_show_letter( "����  ", table_x[id], table_y[id] );
				}
			}
		}
		else//ˢ�����쳣
		{
			lcd_show_letter( "        ", table_x[id]-1, table_y[id] );//����ʾ�쳣ˢ����
		}
	}	
}
 
/*
@���ܣ���ʾ����
*/
void display(void)
{
	if( class_global.net.state == 1 )
	{
		display_time();
	}
	else
	{
		lcd_show_letter( "����δ����   ", 0, 0 );
	}
	display_net_state();//��ʾ����״̬
	display_channel();//��ʾ����
}

/*
@���ܣ�ˮ�������״̬�㱨
@������id=��������[0~5]��state=״ֵ̬
*/
static void tapCount_state_change( uint8_t id, uint8_t state )
{
	char tmp = id+'1';
	if( class_global.ireader[id].count[0].state != state )
	{
		report_state_change( class_global.sys.factory_en, 1, id, state );//�㱨ˢ����״̬�ı�
		class_global.ireader[id].count[0].state = state;
		if( state )
		{
			printf( "tapCount_" );
			printf( &tmp );
			printf( " normal\r\n" );
		}
		else
		{
			printf( "tapCount_" );
			printf( &tmp );
			printf( " err\r\n" );
		}
	}
}


/*
@���ܣ�"��ˮ����"�������ж�
*/
static void tap_continue_task( void )
{
	char i, tmp[10], id_char;
	
	for( i = 0; i < IREADER_NUM_MAX; i++ )
	{
		if( class_global.ireader[i].equ.work == BUSY )//δ�ﵽ��С��ˮ��/��ˮδ����10S��״̬��
		{
			if( class_global.ireader[i].count[0].val >= class_global.trade.min_balance
			|| xTaskGetTickCount() - class_global.ireader[i].working.time > 10000 )//ˮ������ﵽ��Сֵ���߳�ʱ
			{
				id_char = i+'1';
				printf( "tap_" );
				printf( &id_char );
				printf( " stop busy\r\n" );
				tap_set( i, TAP_CLOSE );//�ط���
				{
					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[0].val )] = 0;
					printf( "main signal = " );
					printf( tmp );
					printf( "\r\n" );
					
					//���ź�
//					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[1].val )] = 0;
//					printf( "auxiliary signal = " );
//					printf( tmp );
//					printf( "\r\n" );
				}
				requset_card_result( i, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[i].working.trade_num, class_global.ireader[i].count[0].val, ( char* )class_global.ireader[i].working.physic_char );//�㱨
				
	//				lcd_show_letter( "ֹͣ  ", table_x[i], table_y[i] );
				lcd_show_letter( "      ", table_x[i], table_y[i] );
				//��ʾ���ѽ��
				tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[i]*class_global.ireader[i].count[0].val/class_global.trade.plus_per_1L[i]/100 )] = 0;
				lcd_show_letter( tmp, table_x[i], table_y[i] );
				
				tap_count_set( i, TAP_CLOSE );//ֹͣ����@@ע����������������
				swip_led_set( i, LED_RED, LED_OPEN );//�����
				swip_led_set( i, LED_BLUE, LED_CLOSE );//������
				
//				vTaskDelay( 2000 );
				class_global.ireader[i].display.state = TRUE;
				class_global.ireader[i].display.start_time = xTaskGetTickCount();
				class_global.ireader[i].display.hold_time = 2000;
				
				class_global.ireader[i].equ.work = IDLE;//ˢ�����ָ�����������״̬
			}
			else
			{
				swip_led_set( i, LED_RED, LED_OPEN );//�����
				swip_led_set( i, LED_BLUE, LED_OPEN );//������
			}
		}
		else
		if( class_global.ireader[i].equ.work == WORKING )//���ڳ�ˮ
		{
			if( class_global.ireader[i].card.balance - class_global.ireader[i].count[0].val < class_global.trade.min_balance )//����ֹͣ������ˮ
			{
				id_char = i+'1';
				printf( "tap_" );
				printf( &id_char );
				printf( " stop because balance running low\r\n" );
				tap_set( i, TAP_CLOSE );//�ط���
				{
					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[0].val )] = 0;
					printf( "main signal = " );
					printf( tmp );
					printf( "\r\n" );

					//���ź�
//					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[1].val )] = 0;
//					printf( "auxiliary signal = " );
//					printf( tmp );
//					printf( "\r\n" );
				}
				requset_card_result( i, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[i].card.trade_num, class_global.ireader[i].count[0].val, ( char* )class_global.ireader[i].card.physic_char );//�㱨
				
//				lcd_show_letter( "ֹͣ  ", table_x[i], table_y[i] );
				lcd_show_letter( "      ", table_x[i], table_y[i] );
				//��ʾ���ѽ��
				tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[i]*class_global.ireader[i].count[0].val/class_global.trade.plus_per_1L[i]/100 )] = 0;
				lcd_show_letter( tmp, table_x[i], table_y[i] );
				
				tap_count_set( i, TAP_CLOSE );//ֹͣ����@@ע����������������
			
				swip_led_set( i, LED_RED, LED_OPEN );//�����
				swip_led_set( i, LED_BLUE, LED_CLOSE );//������
//				vTaskDelay( 2000 );
				
				class_global.ireader[i].display.state = TRUE;
				class_global.ireader[i].display.start_time = xTaskGetTickCount();
				class_global.ireader[i].display.hold_time = 2000;
				
				class_global.ireader[i].equ.work = IDLE;//ˢ�����ָ�����������״̬
			}
			
			if(  class_global.ireader[i].count[0].val >= class_global.trade.plus_per_1L[i]*10 )//������󵥴γ�ˮ�� 10L
			{
				id_char = i+'1';
				printf( "tap_" );
				printf( &id_char );
				printf( " stop because flowing max\r\n" );
				tap_set( i, TAP_CLOSE );//�ط���
				{
					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[0].val )] = 0;
					printf( "main signal = " );
					printf( tmp );
					printf( "\r\n" );
					
					//���ź�
//					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[1].val )] = 0;
//					printf( "auxiliary signal = " );
//					printf( tmp );
//					printf( "\r\n" );
				}
				requset_card_result( i, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[i].card.trade_num, class_global.ireader[i].count[0].val, ( char* )class_global.ireader[i].card.physic_char );//�㱨
				
				//��ʾ���ѽ��
//				lcd_show_letter( "ֹͣ  ", table_x[i], table_y[i] );
				lcd_show_letter( "      ", table_x[i], table_y[i] );
				tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[i]*class_global.ireader[i].count[0].val/class_global.trade.plus_per_1L[i]/100 )] = 0;
				lcd_show_letter( tmp, table_x[i], table_y[i] );
				
				tap_count_set( i, TAP_CLOSE );//ֹͣ����@@ע����������������
			
				swip_led_set( i, LED_RED, LED_OPEN );//�����
				swip_led_set( i, LED_BLUE, LED_CLOSE );//������

//				vTaskDelay( 2000 );
				class_global.ireader[i].display.state = TRUE;
				class_global.ireader[i].display.start_time = xTaskGetTickCount();
				class_global.ireader[i].display.hold_time = 2000;
				
				class_global.ireader[i].equ.work = IDLE;//ˢ�����ָ�����������״̬
			}
		}
	}
}

/*
@���ܣ�״̬������
*/
void task_fsm( void )
{
	FSM_MSG msg;
	char id_char, tmp[10];
	uint8_t err;
	uint8_t	set_working_state = SET_INIT;//���ù���
	uint32_t set_working_timeout = 0;//����ģʽ��ʱʱ��
	uint32_t vm_id = 0;
	
	for( ;; )
	{
		if( set_working_timeout != 0 && xTaskGetTickCount() - set_working_timeout > 10000  )//����ģʽ��ʱ
		{
			lcd_clear();
			set_working_timeout = 0;
			vm_id = 0;
			set_working_state = SET_INIT;
		}
		if( set_working_state == SET_INIT )
		{
			vm_id = 0;
			display();
		}
		
		err = fsm_queue_get( ( void* )&msg, 500 );
		if( err == pdTRUE )
		{
			id_char = msg.id + '1';
			switch(  msg.type  )
			{
				case MsgCard://����Ϣ
					if( class_global.ireader[msg.id].equ.work != BUSY && set_working_state == SET_INIT )//ͨ��æ�ڴ���"��ˮ����"/����������ģʽ����ʱ�������ͨ��ˢ������Ϣ����ʱWorking״̬���Դ���
					{
						if( msg.stype == MsgCard_GetCard  )//Ѱ����
						{
							swip_led_set( msg.id, LED_RED, LED_OPEN );//�����
							swip_led_set( msg.id, LED_BLUE, LED_OPEN );//������
							
//							if( class_global.net.state == 1 )
							{
								class_global.trade.fsm[msg.id] = APPLY_FSM_REQUESTING;
								class_global.ireader[msg.id].card.trade_num = ++class_global.trade.number;//���浱ǰ����Ľ��׺�
								requset_card_trade( msg.id, ONLINE_CARD_CHECK_CMD, class_global.ireader[msg.id].card.trade_num );
							}
//							else
//							{
//								//δ����״̬����˸
//								swip_led_set( msg.id, LED_RED, LED_CLOSE );
//								vTaskDelay( 200 );
//								swip_led_set( msg.id, LED_RED, LED_OPEN );
//								vTaskDelay( 200 );
//								swip_led_set( msg.id, LED_RED, LED_CLOSE );
//								vTaskDelay( 200 );
//								swip_led_set( msg.id, LED_RED, LED_OPEN );
//								vTaskDelay( 200 );
//								printf( "network is not link\r\n" );
//							}
						}
						else
						if( msg.stype == MsgCard_LostCard )//���ƿ�
						{
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_INIT )//û�п�ʼ�ۿ������ˮ�����߿�Ƭ
							{
								printf( "card_" );
								printf( &id_char );
								printf( " removed but no flowing\r\n" );
							}
							else
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_REQUESTING )//���ڿۿ����������˿�Ƭ
							{
								printf( "card_" );
								printf( &id_char );
								printf( " removed but requesting\r\n" );
								class_global.trade.fsm[msg.id] = APPLY_FSM_FAILURE;
							}
							else
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_END )//��;����
							{
								printf( "midway end\r\n" );
							}
							else
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_FLOWING )//���ڳ�ˮ���߿�Ƭ
							{
								if( class_global.ireader[msg.id].count[0].val < class_global.trade.min_balance )//�ѳ�ˮ��δ�ﵽ��С��ˮ��
								{
									if( class_global.ireader[msg.id].equ.work != IDLE )
									{
										printf( "card_" );
										printf( &id_char );
										printf( " removed but tap no stop\r\n" );
										
										//��������
										class_global.ireader[msg.id].equ.work = BUSY;
										class_global.ireader[msg.id].working.trade_num = class_global.ireader[msg.id].card.trade_num;
										class_global.ireader[msg.id].working.time = xTaskGetTickCount();
										strcpy( ( char* )class_global.ireader[msg.id].working.physic_char, ( char* )class_global.ireader[msg.id].card.physic_char );
									
										swip_led_set( msg.id, LED_RED, LED_OPEN );//�����
										swip_led_set( msg.id, LED_BLUE, LED_OPEN );//������
									}
									else
									{
										printf( "card_" );
										printf( &id_char );
										printf( " removed and tap stop 2\r\n" );
									}
								}
								else
								{
									printf( "card_" );
									printf( &id_char );
									printf( " removed and tap stop\r\n" );
									tap_set( msg.id, TAP_CLOSE );//�ط���
									{
										tmp[sprintf( tmp, "%u", class_global.ireader[msg.id].count[0].val )] = 0;
										printf( "main signal = " );
										printf( tmp );
										printf( "\r\n" );
										
	//										tmp[sprintf( tmp, "%u", class_global.ireader[msg.id].count[1].val )] = 0;
	//										printf( "auxiliary signal = " );
	//										printf( tmp );
	//										printf( "\r\n" );
									}
									if( class_global.ireader[msg.id].count[0].val )//������岻Ϊ0
									{
										tapCount_state_change( msg.id, 1 );//������
									}
									else//�������Ϊ0
									{
										tapCount_state_change( msg.id, 0 );//�쳣
									}
									requset_card_result( msg.id, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[msg.id].card.trade_num, class_global.ireader[msg.id].count[0].val, ( char* )class_global.ireader[msg.id].card.physic_char );//�㱨
	//									lcd_show_letter( "ֹͣ  ", table_x[msg.id], table_y[msg.id] );
									lcd_show_letter( "      ", table_x[msg.id], table_y[msg.id] );
									//��ʾ���ѽ��
									tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[msg.id]*class_global.ireader[msg.id].count[0].val/class_global.trade.plus_per_1L[msg.id]/100 )] = 0;
									lcd_show_letter( tmp, table_x[msg.id], table_y[msg.id] );

									tap_count_set( msg.id, TAP_CLOSE );//ֹͣ����@@ע����������������
									
//								vTaskDelay( 2000 );
									class_global.ireader[msg.id].display.state = TRUE;
									class_global.ireader[msg.id].display.start_time = xTaskGetTickCount();
									class_global.ireader[msg.id].display.hold_time = 2000;
									
									class_global.ireader[msg.id].equ.work = IDLE;
								}
							}
							swip_led_set( msg.id, LED_RED, LED_OPEN );//�����
							swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//������
						}
					}
				break;
				
				case MsgCardServerAck://���߿�������Ӧ��
					if( msg.stype == ONLINE_CARD_CHECK_CMD )//������ص�������֧��
					{
						if( msg.value == 0x30 )//��ѯ�ɹ�
						{
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_REQUESTING )//��û������
							{
								if( class_global.ireader[msg.id].card.balance < class_global.trade.min_balance )//����
								{
									printf( "card_" );
									printf( &id_char );
									printf( " balance running low\r\n" );
									lcd_show_letter( "Ǯ����", table_x[msg.id],  table_y[msg.id] );
									
//									vTaskDelay( 2000 );
									class_global.ireader[msg.id].display.state = TRUE;
									class_global.ireader[msg.id].display.start_time = xTaskGetTickCount();
									class_global.ireader[msg.id].display.hold_time = 2000;
								}
								else
								{
									class_global.ireader[msg.id].equ.work = WORKING;//����״̬�ı�
									class_global.trade.fsm[msg.id] = APPLY_FSM_FLOWING;
									lcd_show_letter( "��ˮ  ", table_x[msg.id],  table_y[msg.id] );

									printf( "tap_" );
									printf( &id_char );
									printf( " begin flowing\r\n" );
									tap_set( msg.id, TAP_OPEN );//��ˮ��
									tap_count_set( msg.id, TAP_OPEN );//��ʼ����
									swip_led_set( msg.id, LED_RED, LED_CLOSE );//�����
									swip_led_set( msg.id, LED_BLUE, LED_OPEN );//������
								}
							}
							else
							{
								printf( "card_" );
								printf( &id_char );
								printf( " removed and server-ack comes later\r\n" );
								swip_led_set( msg.id, LED_RED, LED_OPEN );//�����
								swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//������
							}
						}
						else//��ѯʧ��
						{
							printf( "card_" );
							printf( &id_char );
							switch( msg.value )
							{
								case '1': 
									printf( " is not exsit\r\n" ); 
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//"�������ڲ�������"
								
								case '2': 
									printf( " is not valid\r\n" ); 
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//" ��Ч���������� "
								
								case '3': 
									printf( " is not belong the school\r\n" ); 
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//"�Ǳ�У����������"
								
								case '4': 
									printf( " is swiped exceed the limit\r\n" ); 
									lcd_show_letter( "�޹�  ", table_x[msg.id],  table_y[msg.id] );
								break;//"�������޲�������"
								
								case '5': 
									printf( " is not belong the VM\r\n" ); 
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//"�Ǳ�������������"
								
								case '6': 
									printf( " Insufficient card balance\r\n" ); 
									lcd_show_letter( "Ǯ����", table_x[msg.id],  table_y[msg.id] );
								break;//"���㲻������"
//								case '7': lcd_show_character( &pt_word.choose[20] );	//" ��  ����Ʒͣ�� "
//									lcd_show_value( select_value_get(), &pt_word.choose[20], 1, 1 );
//								break;
//								case 'a':lcd_show_character( &pt_word.standby[28] );break;//���۳����޶�
								case 'b':
									printf( " exceeding the limit of swiping card on the same day\r\n" );
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//���쳬���޶�
								//case 0x38:Display( "  ��������Ա��  ",30, 400 );break;
								case 0x50: 
									printf( " swiping failed" ); 
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//"�ۿ�ʧ��"
								
								default: 
									printf( " handling exceptions" );
									lcd_show_letter( "��Ч��", table_x[msg.id],  table_y[msg.id] );
								break;//"�����쳣��������"
							}
							
//							vTaskDelay( 2000 );
							class_global.ireader[msg.id].display.state = TRUE;
							class_global.ireader[msg.id].display.start_time = xTaskGetTickCount();
							class_global.ireader[msg.id].display.hold_time = 2000;
							
							swip_led_set( msg.id, LED_RED, LED_OPEN );//�����
							swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//������
							class_global.trade.fsm[msg.id] = APPLY_FSM_FAILURE;
						}
					}
				break;
						
				case MsgServerAckErr://����Ӧ��ʱ
					if( msg.value == 1 )//��ʱ��
					{
						printf( "card_" );
						printf( &id_char );
						printf( " server-ack set_working_timeout\r\n" );
						swip_led_set( msg.id, LED_RED, LED_OPEN );//�����
						swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//������
					}
					else
					if( msg.value == 0 )//�㱨��
					{
						
						
					}
				break;
					
				case MsgKey:
					if( msg.id == B_CHANGE )//�л���
					{
						if( msg.stype == SEC_1 )
						{
							if( set_working_state == SET_INIT )
							{
								char tmp[11] = {0};
								set_working_state = SET_SHOWING;
								set_working_timeout = xTaskGetTickCount();
								
								lcd_clear();
								tmp[sprintf( tmp, "%010u", class_global.net.id )] = 0;
								lcd_show_letter( tmp, 0, 3 );//��ʾ������
								
								tmp[sprintf( tmp, "%u", class_global.net.serverPort )] = 0;
								lcd_show_letter( tmp, 0, 1 );//��ʾ�˿ں�
								
								//�¶���ʾ
								#if(1)
								get_external_temp( (int*)&class_global.temp.external.val, (uint8_t*)&class_global.temp.external.state );//��ȡ�ⲿ�¶�
								if( class_global.temp.external.state )
								{
									uint8_t len = sprintf( tmp, "% 4d", class_global.temp.external.val );
									tmp[len] = 0;
									lcd_show_letter( tmp, 5, 3 );//��ʾ�¶�
									lcd_show_letter( "��", 7, 3 );
								}
								else
								{
									lcd_show_letter( "X", 7, 3 );//û���¶�ͷ
								}
								#endif
									
								lcd_show_letter( ( char* )class_global.net.arr_ip, 0, 2 );//��ʾip
								lcd_show_letter( ( char* )main_version, 0, 0 );//��ʾ�汾��
								creat_ercode( 5, 0, 3 );//��ʾ��ά��							
							}
							else
							if( set_working_state == SET_SETTING )
							{
								if( vm_id != class_global.net.id )//id�ı�
								{
									if(  flash_param_set( FLASH_LIST_Id, FLASH_ADDR_Id, vm_id )  )//����id
									{
										lcd_clear();
										lcd_show_letter( "����id���óɹ�", 0, 1 );
										vTaskDelay( 2000 );
										lcd_show_letter( "    ��������    ", 0, 1 );
										vTaskDelay( 2000 );
										__set_FAULTMASK( 1 );
										NVIC_SystemReset();
										return;
									}
									else
									{
										lcd_clear();
										lcd_show_letter( "����id����ʧ��", 0, 1 );
										vTaskDelay( 2000 );
									}
								}
								lcd_clear();
								set_working_state = SET_INIT;
								set_working_timeout = 0;
							}
							else
							if( set_working_state == SET_SHOWING )
							{
								lcd_clear();
								set_working_state = SET_INIT;
								set_working_timeout = 0;
							}
						}
						else
						{
							printf( "111111111" );
						}
					}
					
					else
					if( msg.id == B_DOWN )//�·���
					{
						if( msg.stype == MS_50 )
						{
							if( set_working_state == SET_SHOWING || set_working_state == SET_SETTING )
							{
								char tmp[11] = {0};
								set_working_state = SET_SETTING;
								if( vm_id == 0 )
								{
									vm_id = class_global.net.id;
								}
								vm_id++;
								tmp[sprintf( tmp, "%010u", vm_id )] = 0;
								lcd_show_letter( tmp, 0, 3 );//��ʾ������
								set_working_timeout = xTaskGetTickCount();//�����л�����ʱ��
							}
						}
						else
						{
							printf( "111111111" );
						}
					}
					else
					if( msg.id == B_Combination )//��ϼ�
					{
						if( msg.stype == SEC_5 )
						{
							lcd_clear();
							lcd_show_letter( "  ���ڸ�λϵͳ  ", 0, 1 );
							lcd_show_letter( "    ���Ե�      ", 0, 2 );
							if( reset_param() )
							{
								vTaskDelay( 2000 );
								lcd_clear();
								lcd_show_letter( "    �ָ��ɹ�    ", 0, 1 );
							}
							else
							{
								vTaskDelay( 2000 );
								lcd_clear();
								lcd_show_letter( "�ָ�ʧ��������", 0, 1 );
							}
							vTaskDelay( 2000 );
							lcd_show_letter( "    ��������    ", 0, 1 );
							vTaskDelay( 2000 );
							__set_FAULTMASK( 1 );
							NVIC_SystemReset();
						}
						
					}
				break;
			}
		}
		
		tap_continue_task();
	}
}
