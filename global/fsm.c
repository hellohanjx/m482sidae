/*
@说明：状态机任务，刷卡器消息，通信信息，设备控制信息，集中在状态机任务处理
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

//设置模式的几种状态
enum{ SET_INIT, SET_SETTING, SET_SHOWING };

//通道显示坐标
static const uint8_t table_y[6] = { 1, 1, 2, 2, 3, 3 };
static const uint8_t table_x[6] = { 1, 5, 1, 5, 1, 5 };

/*
@支付流程状态
*/
enum{
	APPLY_FSM_INIT,//支付初始状态
	APPLY_FSM_REQUESTING,//支付申请中
	APPLY_FSM_FLOWING,//出水中
	APPLY_FSM_FAILURE,//流程失败
	APPLY_FSM_END,//流程结束
};


/*
@功能：显示时间
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
@功能：显示网络状态
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
@功能：显示通道
*/
static void display_channel(void)
{
	char id, tmp[3];
	
	for(id = 0; id < IREADER_NUM_MAX; id++ )
	{
		if( class_global.ireader[id].equ.state == 1 )//刷卡器正常
		{
			if( class_global.ireader[id].equ.work == IDLE )
			{
				if( class_global.ireader[id].display.state == TRUE )//需要显示别的东西
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
					lcd_show_letter( "正常  ", table_x[id], table_y[id] );
				}
			}
		}
		else//刷卡器异常
		{
			lcd_show_letter( "        ", table_x[id]-1, table_y[id] );//不显示异常刷卡器
		}
	}	
}
 
/*
@功能：显示调用
*/
void display(void)
{
	if( class_global.net.state == 1 )
	{
		display_time();
	}
	else
	{
		lcd_show_letter( "网络未连接   ", 0, 0 );
	}
	display_net_state();//显示网络状态
	display_channel();//显示货道
}

/*
@功能：水表计数器状态汇报
@参数：id=计数器号[0~5]；state=状态值
*/
static void tapCount_state_change( uint8_t id, uint8_t state )
{
	char tmp = id+'1';
	if( class_global.ireader[id].count[0].state != state )
	{
		report_state_change( class_global.sys.factory_en, 1, id, state );//汇报刷卡器状态改变
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
@功能："出水任务"，条件判定
*/
static void tap_continue_task( void )
{
	char i, tmp[10], id_char;
	
	for( i = 0; i < IREADER_NUM_MAX; i++ )
	{
		if( class_global.ireader[i].equ.work == BUSY )//未达到最小出水量/出水未超过10S【状态】
		{
			if( class_global.ireader[i].count[0].val >= class_global.trade.min_balance
			|| xTaskGetTickCount() - class_global.ireader[i].working.time > 10000 )//水流脉冲达到最小值或者超时
			{
				id_char = i+'1';
				printf( "tap_" );
				printf( &id_char );
				printf( " stop busy\r\n" );
				tap_set( i, TAP_CLOSE );//关阀门
				{
					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[0].val )] = 0;
					printf( "main signal = " );
					printf( tmp );
					printf( "\r\n" );
					
					//副信号
//					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[1].val )] = 0;
//					printf( "auxiliary signal = " );
//					printf( tmp );
//					printf( "\r\n" );
				}
				requset_card_result( i, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[i].working.trade_num, class_global.ireader[i].count[0].val, ( char* )class_global.ireader[i].working.physic_char );//汇报
				
	//				lcd_show_letter( "停止  ", table_x[i], table_y[i] );
				lcd_show_letter( "      ", table_x[i], table_y[i] );
				//显示花费金额
				tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[i]*class_global.ireader[i].count[0].val/class_global.trade.plus_per_1L[i]/100 )] = 0;
				lcd_show_letter( tmp, table_x[i], table_y[i] );
				
				tap_count_set( i, TAP_CLOSE );//停止计数@@注意这里清除脉冲计数
				swip_led_set( i, LED_RED, LED_OPEN );//红灯亮
				swip_led_set( i, LED_BLUE, LED_CLOSE );//蓝灯灭
				
//				vTaskDelay( 2000 );
				class_global.ireader[i].display.state = TRUE;
				class_global.ireader[i].display.start_time = xTaskGetTickCount();
				class_global.ireader[i].display.hold_time = 2000;
				
				class_global.ireader[i].equ.work = IDLE;//刷卡器恢复到正常工作状态
			}
			else
			{
				swip_led_set( i, LED_RED, LED_OPEN );//红灯亮
				swip_led_set( i, LED_BLUE, LED_OPEN );//蓝灯亮
			}
		}
		else
		if( class_global.ireader[i].equ.work == WORKING )//正在出水
		{
			if( class_global.ireader[i].card.balance - class_global.ireader[i].count[0].val < class_global.trade.min_balance )//余额不足停止继续出水
			{
				id_char = i+'1';
				printf( "tap_" );
				printf( &id_char );
				printf( " stop because balance running low\r\n" );
				tap_set( i, TAP_CLOSE );//关阀门
				{
					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[0].val )] = 0;
					printf( "main signal = " );
					printf( tmp );
					printf( "\r\n" );

					//副信号
//					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[1].val )] = 0;
//					printf( "auxiliary signal = " );
//					printf( tmp );
//					printf( "\r\n" );
				}
				requset_card_result( i, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[i].card.trade_num, class_global.ireader[i].count[0].val, ( char* )class_global.ireader[i].card.physic_char );//汇报
				
//				lcd_show_letter( "停止  ", table_x[i], table_y[i] );
				lcd_show_letter( "      ", table_x[i], table_y[i] );
				//显示花费金额
				tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[i]*class_global.ireader[i].count[0].val/class_global.trade.plus_per_1L[i]/100 )] = 0;
				lcd_show_letter( tmp, table_x[i], table_y[i] );
				
				tap_count_set( i, TAP_CLOSE );//停止计数@@注意这里清除脉冲计数
			
				swip_led_set( i, LED_RED, LED_OPEN );//红灯亮
				swip_led_set( i, LED_BLUE, LED_CLOSE );//蓝灯灭
//				vTaskDelay( 2000 );
				
				class_global.ireader[i].display.state = TRUE;
				class_global.ireader[i].display.start_time = xTaskGetTickCount();
				class_global.ireader[i].display.hold_time = 2000;
				
				class_global.ireader[i].equ.work = IDLE;//刷卡器恢复到正常工作状态
			}
			
			if(  class_global.ireader[i].count[0].val >= class_global.trade.plus_per_1L[i]*10 )//超过最大单次出水量 10L
			{
				id_char = i+'1';
				printf( "tap_" );
				printf( &id_char );
				printf( " stop because flowing max\r\n" );
				tap_set( i, TAP_CLOSE );//关阀门
				{
					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[0].val )] = 0;
					printf( "main signal = " );
					printf( tmp );
					printf( "\r\n" );
					
					//副信号
//					tmp[sprintf( tmp, "%u", class_global.ireader[i].count[1].val )] = 0;
//					printf( "auxiliary signal = " );
//					printf( tmp );
//					printf( "\r\n" );
				}
				requset_card_result( i, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[i].card.trade_num, class_global.ireader[i].count[0].val, ( char* )class_global.ireader[i].card.physic_char );//汇报
				
				//显示花费金额
//				lcd_show_letter( "停止  ", table_x[i], table_y[i] );
				lcd_show_letter( "      ", table_x[i], table_y[i] );
				tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[i]*class_global.ireader[i].count[0].val/class_global.trade.plus_per_1L[i]/100 )] = 0;
				lcd_show_letter( tmp, table_x[i], table_y[i] );
				
				tap_count_set( i, TAP_CLOSE );//停止计数@@注意这里清除脉冲计数
			
				swip_led_set( i, LED_RED, LED_OPEN );//红灯亮
				swip_led_set( i, LED_BLUE, LED_CLOSE );//蓝灯灭

//				vTaskDelay( 2000 );
				class_global.ireader[i].display.state = TRUE;
				class_global.ireader[i].display.start_time = xTaskGetTickCount();
				class_global.ireader[i].display.hold_time = 2000;
				
				class_global.ireader[i].equ.work = IDLE;//刷卡器恢复到正常工作状态
			}
		}
	}
}

/*
@功能：状态机任务
*/
void task_fsm( void )
{
	FSM_MSG msg;
	char id_char, tmp[10];
	uint8_t err;
	uint8_t	set_working_state = SET_INIT;//设置工作
	uint32_t set_working_timeout = 0;//设置模式超时时间
	uint32_t vm_id = 0;
	
	for( ;; )
	{
		if( set_working_timeout != 0 && xTaskGetTickCount() - set_working_timeout > 10000  )//设置模式超时
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
				case MsgCard://卡消息
					if( class_global.ireader[msg.id].equ.work != BUSY && set_working_state == SET_INIT )//通道忙于处理"出水任务"/或者在设置模式，这时不处理此通道刷卡器消息；此时Working状态可以处理
					{
						if( msg.stype == MsgCard_GetCard  )//寻到卡
						{
							swip_led_set( msg.id, LED_RED, LED_OPEN );//红灯亮
							swip_led_set( msg.id, LED_BLUE, LED_OPEN );//蓝灯亮
							
//							if( class_global.net.state == 1 )
							{
								class_global.trade.fsm[msg.id] = APPLY_FSM_REQUESTING;
								class_global.ireader[msg.id].card.trade_num = ++class_global.trade.number;//保存当前申请的交易号
								requset_card_trade( msg.id, ONLINE_CARD_CHECK_CMD, class_global.ireader[msg.id].card.trade_num );
							}
//							else
//							{
//								//未联网状态灯闪烁
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
						if( msg.stype == MsgCard_LostCard )//卡移开
						{
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_INIT )//没有开始扣款申请出水就移走卡片
							{
								printf( "card_" );
								printf( &id_char );
								printf( " removed but no flowing\r\n" );
							}
							else
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_REQUESTING )//正在扣款申请移走了卡片
							{
								printf( "card_" );
								printf( &id_char );
								printf( " removed but requesting\r\n" );
								class_global.trade.fsm[msg.id] = APPLY_FSM_FAILURE;
							}
							else
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_END )//中途结束
							{
								printf( "midway end\r\n" );
							}
							else
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_FLOWING )//正在出水移走卡片
							{
								if( class_global.ireader[msg.id].count[0].val < class_global.trade.min_balance )//已出水但未达到最小出水量
								{
									if( class_global.ireader[msg.id].equ.work != IDLE )
									{
										printf( "card_" );
										printf( &id_char );
										printf( " removed but tap no stop\r\n" );
										
										//备份内容
										class_global.ireader[msg.id].equ.work = BUSY;
										class_global.ireader[msg.id].working.trade_num = class_global.ireader[msg.id].card.trade_num;
										class_global.ireader[msg.id].working.time = xTaskGetTickCount();
										strcpy( ( char* )class_global.ireader[msg.id].working.physic_char, ( char* )class_global.ireader[msg.id].card.physic_char );
									
										swip_led_set( msg.id, LED_RED, LED_OPEN );//红灯亮
										swip_led_set( msg.id, LED_BLUE, LED_OPEN );//蓝灯亮
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
									tap_set( msg.id, TAP_CLOSE );//关阀门
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
									if( class_global.ireader[msg.id].count[0].val )//如果脉冲不为0
									{
										tapCount_state_change( msg.id, 1 );//正常常
									}
									else//如果脉冲为0
									{
										tapCount_state_change( msg.id, 0 );//异常
									}
									requset_card_result( msg.id, ONLINE_CARD_PAY_SUCCESS, class_global.ireader[msg.id].card.trade_num, class_global.ireader[msg.id].count[0].val, ( char* )class_global.ireader[msg.id].card.physic_char );//汇报
	//									lcd_show_letter( "停止  ", table_x[msg.id], table_y[msg.id] );
									lcd_show_letter( "      ", table_x[msg.id], table_y[msg.id] );
									//显示花费金额
									tmp[sprintf( tmp, "%0.2f", ( float )class_global.trade.price_per_1L[msg.id]*class_global.ireader[msg.id].count[0].val/class_global.trade.plus_per_1L[msg.id]/100 )] = 0;
									lcd_show_letter( tmp, table_x[msg.id], table_y[msg.id] );

									tap_count_set( msg.id, TAP_CLOSE );//停止计数@@注意这里清除脉冲计数
									
//								vTaskDelay( 2000 );
									class_global.ireader[msg.id].display.state = TRUE;
									class_global.ireader[msg.id].display.start_time = xTaskGetTickCount();
									class_global.ireader[msg.id].display.hold_time = 2000;
									
									class_global.ireader[msg.id].equ.work = IDLE;
								}
							}
							swip_led_set( msg.id, LED_RED, LED_OPEN );//红灯亮
							swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//蓝灯灭
						}
					}
				break;
				
				case MsgCardServerAck://在线卡服务器应答
					if( msg.stype == ONLINE_CARD_CHECK_CMD )//如果返回的是申请支付
					{
						if( msg.value == 0x30 )//查询成功
						{
							if( class_global.trade.fsm[msg.id] == APPLY_FSM_REQUESTING )//卡没有拿走
							{
								if( class_global.ireader[msg.id].card.balance < class_global.trade.min_balance )//余额不足
								{
									printf( "card_" );
									printf( &id_char );
									printf( " balance running low\r\n" );
									lcd_show_letter( "钱不足", table_x[msg.id],  table_y[msg.id] );
									
//									vTaskDelay( 2000 );
									class_global.ireader[msg.id].display.state = TRUE;
									class_global.ireader[msg.id].display.start_time = xTaskGetTickCount();
									class_global.ireader[msg.id].display.hold_time = 2000;
								}
								else
								{
									class_global.ireader[msg.id].equ.work = WORKING;//工作状态改变
									class_global.trade.fsm[msg.id] = APPLY_FSM_FLOWING;
									lcd_show_letter( "出水  ", table_x[msg.id],  table_y[msg.id] );

									printf( "tap_" );
									printf( &id_char );
									printf( " begin flowing\r\n" );
									tap_set( msg.id, TAP_OPEN );//开水阀
									tap_count_set( msg.id, TAP_OPEN );//开始计数
									swip_led_set( msg.id, LED_RED, LED_CLOSE );//红灯灭
									swip_led_set( msg.id, LED_BLUE, LED_OPEN );//蓝灯亮
								}
							}
							else
							{
								printf( "card_" );
								printf( &id_char );
								printf( " removed and server-ack comes later\r\n" );
								swip_led_set( msg.id, LED_RED, LED_OPEN );//红灯亮
								swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//蓝灯灭
							}
						}
						else//查询失败
						{
							printf( "card_" );
							printf( &id_char );
							switch( msg.value )
							{
								case '1': 
									printf( " is not exsit\r\n" ); 
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//"卡不存在不能消费"
								
								case '2': 
									printf( " is not valid\r\n" ); 
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//" 无效卡不能消费 "
								
								case '3': 
									printf( " is not belong the school\r\n" ); 
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//"非本校卡不能消费"
								
								case '4': 
									printf( " is swiped exceed the limit\r\n" ); 
									lcd_show_letter( "限购  ", table_x[msg.id],  table_y[msg.id] );
								break;//"次数超限不能消费"
								
								case '5': 
									printf( " is not belong the VM\r\n" ); 
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//"非本机卡不能消费"
								
								case '6': 
									printf( " Insufficient card balance\r\n" ); 
									lcd_show_letter( "钱不足", table_x[msg.id],  table_y[msg.id] );
								break;//"余额不足不能消费"
//								case '7': lcd_show_character( &pt_word.choose[20] );	//" 第  层商品停售 "
//									lcd_show_value( select_value_get(), &pt_word.choose[20], 1, 1 );
//								break;
//								case 'a':lcd_show_character( &pt_word.standby[28] );break;//单价超过限额
								case 'b':
									printf( " exceeding the limit of swiping card on the same day\r\n" );
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//当天超过限额
								//case 0x38:Display( "  超级管理员卡  ",30, 400 );break;
								case 0x50: 
									printf( " swiping failed" ); 
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//"扣款失败"
								
								default: 
									printf( " handling exceptions" );
									lcd_show_letter( "无效卡", table_x[msg.id],  table_y[msg.id] );
								break;//"处理异常不能消费"
							}
							
//							vTaskDelay( 2000 );
							class_global.ireader[msg.id].display.state = TRUE;
							class_global.ireader[msg.id].display.start_time = xTaskGetTickCount();
							class_global.ireader[msg.id].display.hold_time = 2000;
							
							swip_led_set( msg.id, LED_RED, LED_OPEN );//红灯亮
							swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//蓝灯灭
							class_global.trade.fsm[msg.id] = APPLY_FSM_FAILURE;
						}
					}
				break;
						
				case MsgServerAckErr://网络应答超时
					if( msg.value == 1 )//即时包
					{
						printf( "card_" );
						printf( &id_char );
						printf( " server-ack set_working_timeout\r\n" );
						swip_led_set( msg.id, LED_RED, LED_OPEN );//红灯亮
						swip_led_set( msg.id, LED_BLUE, LED_CLOSE );//蓝灯灭
					}
					else
					if( msg.value == 0 )//汇报包
					{
						
						
					}
				break;
					
				case MsgKey:
					if( msg.id == B_CHANGE )//切换键
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
								lcd_show_letter( tmp, 0, 3 );//显示机器号
								
								tmp[sprintf( tmp, "%u", class_global.net.serverPort )] = 0;
								lcd_show_letter( tmp, 0, 1 );//显示端口号
								
								//温度显示
								#if(1)
								get_external_temp( (int*)&class_global.temp.external.val, (uint8_t*)&class_global.temp.external.state );//获取外部温度
								if( class_global.temp.external.state )
								{
									uint8_t len = sprintf( tmp, "% 4d", class_global.temp.external.val );
									tmp[len] = 0;
									lcd_show_letter( tmp, 5, 3 );//显示温度
									lcd_show_letter( "℃", 7, 3 );
								}
								else
								{
									lcd_show_letter( "X", 7, 3 );//没有温度头
								}
								#endif
									
								lcd_show_letter( ( char* )class_global.net.arr_ip, 0, 2 );//显示ip
								lcd_show_letter( ( char* )main_version, 0, 0 );//显示版本号
								creat_ercode( 5, 0, 3 );//显示二维码							
							}
							else
							if( set_working_state == SET_SETTING )
							{
								if( vm_id != class_global.net.id )//id改变
								{
									if(  flash_param_set( FLASH_LIST_Id, FLASH_ADDR_Id, vm_id )  )//保存id
									{
										lcd_clear();
										lcd_show_letter( "机器id设置成功", 0, 1 );
										vTaskDelay( 2000 );
										lcd_show_letter( "    即将重启    ", 0, 1 );
										vTaskDelay( 2000 );
										__set_FAULTMASK( 1 );
										NVIC_SystemReset();
										return;
									}
									else
									{
										lcd_clear();
										lcd_show_letter( "机器id设置失败", 0, 1 );
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
					if( msg.id == B_DOWN )//下翻键
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
								lcd_show_letter( tmp, 0, 3 );//显示机器号
								set_working_timeout = xTaskGetTickCount();//更新切换按键时间
							}
						}
						else
						{
							printf( "111111111" );
						}
					}
					else
					if( msg.id == B_Combination )//组合键
					{
						if( msg.stype == SEC_5 )
						{
							lcd_clear();
							lcd_show_letter( "  正在复位系统  ", 0, 1 );
							lcd_show_letter( "    请稍等      ", 0, 2 );
							if( reset_param() )
							{
								vTaskDelay( 2000 );
								lcd_clear();
								lcd_show_letter( "    恢复成功    ", 0, 1 );
							}
							else
							{
								vTaskDelay( 2000 );
								lcd_clear();
								lcd_show_letter( "恢复失败请重试", 0, 1 );
							}
							vTaskDelay( 2000 );
							lcd_show_letter( "    即将重启    ", 0, 1 );
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
