#include "task_swipe.h"
#include "swipe.h"
#include "stdint.h"
#include "global.h"
#include "swipe_led.h"
#include "string.h"
#include "user_config.h"
#include "commucation.h"

/*
@功能：读卡器状态汇报
@参数：id=读卡器号[0~5]；state=状态值
*/
void icreader_state_change(uint8_t id,	uint8_t state)
{
	if(state != class_global.ireader[id].equ.state)
	{ 
		if(class_global.ireader[id].equ.state == CARD_INIT)//初始状态（此状态为开机，不汇报，因为有开机汇报包）
		{
			class_global.ireader[id].equ.state = state;
//			report_state_change(class_global.sys.factory_en, EQU_TYPE_PAYMENT, EQU_ID_CARD, state, NULL);//汇报状态改变
		}
		else//这里不用发消息，因为有全局变量
		{
			class_global.ireader[id].equ.state = state;
//			report_state_change(class_global.sys.factory_en, EQU_TYPE_PAYMENT, EQU_ID_CARD, state, NULL);//汇报状态改变
		}
	}
}



/*
@功能：刷卡器1任务
*/
void task_swipe1(void)
{
	#if(0)
	char card_num[CARD_PHY_LEN], i;//卡号
	ireader_init(IREADER_1);
	
	for(;;)
	{
		if(ireader_read(IREADER_1))//查询卡
		{
			i= sizeof(class_global.ireader[0].card.physic_char);
			if( strncasecmp(card_num, (char*)class_global.ireader[IREADER_1].card.physic_char, CARD_PHY_LEN) )
			{
				strcpy(card_num, (char*)class_global.ireader[IREADER_1].card.physic_char);
//				requset_card_trade(IREADER_0, ONLINE_CARD_PAY_CMD, );//在线支付申请
				//led灯闪烁
				//等待扣款申请应答
				//扣款申请通过，开水阀，并开始计数
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
@功能：刷卡器2任务
*/
void task_swipe2(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@功能：刷卡器3任务
*/
void task_swipe3(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@功能：刷卡器4任务
*/
void task_swipe4(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@功能：刷卡器5任务
*/
void task_swipe5(void)
{
	for(;;)
	{
		vTaskDelay(1000);
	}
}

/*
@功能：刷卡器6任务
*/
void task_swipe6(void)
{
	IREADER_MSG msg;
	char card_num[CARD_PHY_LEN];//存卡号
	uint8_t err, i, state = 0, fsm;
	uint8_t id = IREADER_6;//刷卡器id
	uint32_t time;
	
	ireader_init(id);
	
	for(;;)
	{
		if(ireader_read(id))//查询卡
		{
			if( strncasecmp(card_num, (char*)class_global.ireader[id].card.physic_char, CARD_PHY_LEN) )//卡号相同不进入
			{
				strcpy(card_num, (char*)class_global.ireader[id].card.physic_char);
				if(class_global.net.state)//联网
				{
					time = xTaskGetTickCount();//计时开始
					class_global.ireader[id].card.trade_num = ++class_global.trade.number;//保存当前申请的交易号
					requset_card_trade(id, ONLINE_CARD_PAY_CMD, class_global.ireader[id].card.trade_num);//在线支付申请
					//这里可以用状态机
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
					for(; xTaskGetTickCount() - time < 15; )//超过15秒则认为超时，如果有应答，这里可以死等
					{
						err = ireader_queue_get[id]((void*)&msg, 100);//等待平台消息
						if(err == pdTRUE)//获取到平台应答
						{
							printf("server get\r\n");
						}
						else
						{
							SWIPE_LED_BLUE(6) = state;//蓝灯闪烁
							state = ~state;
						}
					}
					//等待扣款申请应答
					//扣款申请通过，开水阀，并开始计数
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
