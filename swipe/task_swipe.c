#include "task_swipe.h"
#include "swipe.h"
#include "stdint.h"
#include "global.h"
#include "string.h"
#include "msg.h"
#include "swipe_led.h"

/*
@功能：刷卡器1任务
*/
void task_swipe1(void)
{
	#if(0)
	for(;;)
	{
		vTaskDelay(1000);
	}
	#else
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t id = IREADER_1;//刷卡器id
	uint8_t i, state = FALSE;
	
	ireader_init(id);
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				if(class_global.net.state == 1)
				{
					strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
					FSM_MSG msg;
					msg.id = id;
					msg.type = MsgCard;
					msg.stype = MsgCard_GetCard;
					fsm_queue_send(msg);
					printf("swip_1 get\r\n");
				}
				else
				{
					//未联网状态灯闪烁
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					printf("network-1 is not link\r\n");
				}
				state = TRUE;
			}
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			if( state == TRUE )
			{
				FSM_MSG msg;
				msg.id = id;
				msg.type = MsgCard;
				msg.stype = MsgCard_LostCard;
				fsm_queue_send(msg);
				state = FALSE;
			}
		}
		vTaskDelay(50);
	}
	#endif
}

/*
@功能：刷卡器2任务
*/
void task_swipe2(void)
{
	#if(0)
	for(;;)
	{
		vTaskDelay(1000);
	}
	#else
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t id = IREADER_2;//刷卡器id
	uint8_t i, state = FALSE;
	
	ireader_init(id);
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				if(class_global.net.state == 1)
				{
					strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
					FSM_MSG msg;
					msg.id = id;
					msg.type = MsgCard;
					msg.stype = MsgCard_GetCard;
					fsm_queue_send(msg);
					printf("swip_2 get\r\n");
				}
				else
				{
					//未联网状态灯闪烁
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					printf("network-2 is not link\r\n");
				}
				state = TRUE;
			}
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			if( state == TRUE )
			{
				FSM_MSG msg;
				msg.id = id;
				msg.type = MsgCard;
				msg.stype = MsgCard_LostCard;
				fsm_queue_send(msg);
				state = FALSE;
			}
		}
		vTaskDelay(50);
	}
	#endif
}

/*
@功能：刷卡器3任务
*/
void task_swipe3(void)
{
	#if(0)
	for(;;)
	{
		vTaskDelay(1000);
	}
	#else
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t id = IREADER_3;//刷卡器id
	uint8_t i, state = FALSE;
	
	ireader_init(id);
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				if(class_global.net.state == 1)
				{
					strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
					FSM_MSG msg;
					msg.id = id;
					msg.type = MsgCard;
					msg.stype = MsgCard_GetCard;
					fsm_queue_send(msg);
					printf("swip_3 get\r\n");
				}
				else
				{
					//未联网状态灯闪烁
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					printf("network-3 is not link\r\n");
				}
				state = TRUE;
			}
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			if( state == TRUE )
			{
				FSM_MSG msg;
				msg.id = id;
				msg.type = MsgCard;
				msg.stype = MsgCard_LostCard; 
				fsm_queue_send(msg);
				state = FALSE;
			}
		}
		vTaskDelay(50);
	}
	#endif
}

/*
@功能：刷卡器4任务
*/
void task_swipe4(void)
{
	#if(0)
	_uart6_config(9600);
	_uart6_config(115200);
	for(;;)
	{
		vTaskDelay(1000);
	}
	#else
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t id = IREADER_4;//刷卡器id
	uint8_t i, state = FALSE;
	
	ireader_init(id);
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				if(class_global.net.state == 1)
				{
					strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
					FSM_MSG msg;
					msg.id = id;
					msg.type = MsgCard;
					msg.stype = MsgCard_GetCard; 
					fsm_queue_send(msg);
					printf("swip_4 get\r\n");
				}
				else
				{
					//未联网状态灯闪烁
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					printf("network-4 is not link\r\n");
				}
				state = TRUE;
			}
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			if( state == TRUE )
			{
				FSM_MSG msg;
				msg.id = id;
				msg.type = MsgCard;
				msg.stype = MsgCard_LostCard; 
				fsm_queue_send(msg);
				state = FALSE;
			}
		}
		vTaskDelay(50);
	}
	#endif
}

/*
@功能：刷卡器5任务
*/
void task_swipe5(void)
{
	#if(0)
	for(;;)
	{
		vTaskDelay(1000);
	}
	#else
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t id = IREADER_5;//刷卡器id
	uint8_t i, state = FALSE;
	
	ireader_init(id);
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				if(class_global.net.state == 1)
				{
					strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
					FSM_MSG msg;
					msg.id = id;
					msg.type = MsgCard;
					msg.stype = MsgCard_GetCard; 
					fsm_queue_send(msg);
					printf("swip_5 get\r\n");
				}
				else
				{
					//未联网状态灯闪烁
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					printf("network-5 is not link\r\n");
				}
				state = TRUE;
			}
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			if( state == TRUE )
			{
				FSM_MSG msg;
				msg.id = id;
				msg.type = MsgCard;
				msg.stype = MsgCard_LostCard; 
				fsm_queue_send(msg);
				state = FALSE;
			}
		}
		vTaskDelay(50);
	}
	#endif
}

/*
@功能：刷卡器6任务
*/
void task_swipe6(void)
{
	#if(0)
	for(;;)
	{
		vTaskDelay(1000);
	}
	#else
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t id = IREADER_6;//刷卡器id
	uint8_t i, state = FALSE;
	
	ireader_init(id);
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				if(class_global.net.state == 1)
				{
					strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
					FSM_MSG msg;
					msg.id = id;
					msg.type = MsgCard;
					msg.stype = MsgCard_GetCard; 
					fsm_queue_send(msg);
					printf("swip_6 get\r\n");
				}
				else
				{
					//未联网状态灯闪烁
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_CLOSE);
					vTaskDelay(200);
					swip_led_set(id, LED_RED, LED_OPEN);
					vTaskDelay(200);
					printf("network-6 is not link\r\n");
				}
				state = TRUE;
			}
		}
		else
		{
			for(i = 0; i < CARD_PHY_LEN; i++)
			{
				card_num[i] = 0;
			}
			
			if( state == TRUE )
			{
				FSM_MSG msg;
				msg.id = id;
				msg.type = MsgCard;
				msg.stype = MsgCard_LostCard; 
				fsm_queue_send(msg);
				state = FALSE;
			}
		}
		vTaskDelay(50);
	}
	#endif
}
