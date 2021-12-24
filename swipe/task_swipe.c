#include "task_swipe.h"
#include "swipe.h"
#include "stdint.h"
#include "global.h"
#include "swipe_led.h"
#include "string.h"
#include "user_config.h"
#include "commucation.h"

/*
@���ܣ�������״̬�㱨
@������id=��������[0~5]��state=״ֵ̬
*/
void icreader_state_change(uint8_t id,	uint8_t state)
{
	if(state != class_global.ireader[id].equ.state)
	{ 
		if(class_global.ireader[id].equ.state == CARD_INIT)//��ʼ״̬����״̬Ϊ���������㱨����Ϊ�п����㱨����
		{
			class_global.ireader[id].equ.state = state;
//			report_state_change(class_global.sys.factory_en, EQU_TYPE_PAYMENT, EQU_ID_CARD, state, NULL);//�㱨״̬�ı�
		}
		else//���ﲻ�÷���Ϣ����Ϊ��ȫ�ֱ���
		{
			class_global.ireader[id].equ.state = state;
//			report_state_change(class_global.sys.factory_en, EQU_TYPE_PAYMENT, EQU_ID_CARD, state, NULL);//�㱨״̬�ı�
		}
	}
}



/*
@���ܣ�ˢ����1����
*/
void task_swipe1(void)
{
	#if(0)
	char card_num[CARD_PHY_LEN], i;//����
	ireader_init(IREADER_1);
	
	for(;;)
	{
		if(ireader_read(IREADER_1))//��ѯ��
		{
			i= sizeof(class_global.ireader[0].card.physic_char);
			if( strncasecmp(card_num, (char*)class_global.ireader[IREADER_1].card.physic_char, CARD_PHY_LEN) )
			{
				strcpy(card_num, (char*)class_global.ireader[IREADER_1].card.physic_char);
//				requset_card_trade(IREADER_0, ONLINE_CARD_PAY_CMD, );//����֧������
				//led����˸
				//�ȴ��ۿ�����Ӧ��
				//�ۿ�����ͨ������ˮ��������ʼ����
				printf("swip_1 get\r\n");
			}
			SWIPE_LED_BLUE(1) = LED_OPEN;
			SWIPE_LED_RED(1) = LED_CLOSE;
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			SWIPE_LED_BLUE(1) = LED_CLOSE;
			SWIPE_LED_RED(1) = LED_OPEN;
		}
	}
	#else
	for(;;)
	{
		vTaskDelay(1000);
	}
	#endif
}

/*
@���ܣ�ˢ����2����
*/
void task_swipe2(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@���ܣ�ˢ����3����
*/
void task_swipe3(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@���ܣ�ˢ����4����
*/
void task_swipe4(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@���ܣ�ˢ����5����
*/
void task_swipe5(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@���ܣ�ˢ����6����
*/
void task_swipe6(void)
{
	IREADER_MSG msg;
	char card_num[CARD_PHY_LEN];//�濨��
	uint8_t err, i, state = 0, fsm;
	uint8_t id = IREADER_6;//ˢ����id
	uint32_t time;
	
	ireader_init(id);
	
	for(;;)
	{
		if(ireader_read(id))//��ѯ��
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//������ͬ������
			{
				strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
				if(class_global.net.state)//����
				{
					time = xTaskGetTickCount();//��ʱ��ʼ
					class_global.ireader[id].card.trade_num = ++class_global.trade.number;//���浱ǰ����Ľ��׺�
					requset_card_trade(id, ONLINE_CARD_PAY_CMD, class_global.ireader[id].card.trade_num);//����֧������
					//���������״̬��
//					switch(fsm)
//					{
//						case WAIT_SERVER_ACK:
//							
//						break;
//						
//						case CMD_WATER_FLOW:
//							
//						break;
//						
//						case WAIT_WATER_ACK:
//							
//						break;
//						
//					}
					for(; xTaskGetTickCount() - time < 15; )//����15������Ϊ��ʱ�������Ӧ�������������
					{
						err = ireader_queue_get[id]((void*)&msg, 100);//�ȴ�ƽ̨��Ϣ
						if(err == pdTRUE)//��ȡ��ƽ̨Ӧ��
						{
							printf("server get\r\n");
						}
						else
						{
							SWIPE_LED_BLUE(6) = state;//������˸
							state = ~state;
						}
					}
					//�ȴ��ۿ�����Ӧ��
					//�ۿ�����ͨ������ˮ��������ʼ����
				}
				printf("swip_6 get\r\n");
			}
			SWIPE_LED_BLUE(6) = LED_OPEN;
			SWIPE_LED_RED(6) = LED_CLOSE;
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			SWIPE_LED_BLUE(6) = LED_CLOSE;
			SWIPE_LED_RED(6) = LED_OPEN;
		}
	}
}
