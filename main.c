#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Hardware and starter kit includes. */
#include "NuMicro.h"
#include "hardware.h"
#include "sys_clock.h"
#include "msg.h"
#include "commucation.h"
#include "global.h"
#include "user_rtc.h"
#include "temp.h"
#include "led_gpio.h"
#include "log_interface_cmd.h"
#include "task_swipe.h"
#include "fsm.h"
#include "wacth_dog.h"
#include "key.h"


#define START_TASK_PRIO		1			//任务优先级
#define START_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t START_Task_Handler;	//任务句柄
static void start_task( void *pvParameters );

#define communication_TASK_PRIO		27		//任务优先级
#define COMMUNICATION_STK_SIZE 		512		//任务堆栈大小
TaskHandle_t communication_Task_Handler;		//任务句柄
static void communication_task( void *pvParameters );

#define IDLE_TASK_PRIO		15		//任务优先级
#define IDLE_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Idle_Task_Handler;		//任务句柄
static void idle_task( void *pvParameters );

#define LOG_TASK_PRIO		11		//任务优先级
#define LOG_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Log_Task_Handler;		//任务句柄
static void log_task( void *pvParameters );

#define SWIPE_1_TASK_PRIO		25		//任务优先级
#define SWIPE_1_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Swipe_1_Task_Handler;		//任务句柄
static void swipe_1_task( void *pvParameters );

#define SWIPE_2_TASK_PRIO		25		//任务优先级
#define SWIPE_2_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Swipe_2_Task_Handler;		//任务句柄
static void swipe_2_task( void *pvParameters );

#define SWIPE_3_TASK_PRIO		25		//任务优先级
#define SWIPE_3_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Swipe_3_Task_Handler;		//任务句柄
static void swipe_3_task( void *pvParameters );

#define SWIPE_4_TASK_PRIO		25		//任务优先级
#define SWIPE_4_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Swipe_4_Task_Handler;		//任务句柄
static void swipe_4_task( void *pvParameters );

#define SWIPE_5_TASK_PRIO		25		//任务优先级
#define SWIPE_5_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Swipe_5_Task_Handler;		//任务句柄
static void swipe_5_task( void *pvParameters );

#define SWIPE_6_TASK_PRIO		25		//任务优先级
#define SWIPE_6_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Swipe_6_Task_Handler;		//任务句柄
static void swipe_6_task( void *pvParameters );

#define KEY_TASK_PRIO		20		//任务优先级
#define KEY_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Key_Task_Handler;		//任务句柄
static void key_task( void *pvParameters );

#define FSM_TASK_PRIO		26		//任务优先级
#define FSM_STK_SIZE 		512		//任务堆栈大小
TaskHandle_t Fsm_Task_Handler;		//任务句柄
static void fsm_task( void *pvParameters );



/*
@功能：内存测试
@参数：start,起始地址，stop结束地址
*/
static void memory_test(uint32_t start, uint32_t stop)
{
	uint8_t n;
	uint32_t start_s;
	while(start < stop)
	{
		(*(__IO uint8_t *)(start)) = n;
		start_s = start;
		start++;
		n++;
	}
}
/***********************************
@主循环
***********************************/
int main(void)
{
	NVIC_SetPriorityGrouping(0);
	sys_clock_init();
//	memory_test(0x20000000, 0x20027FFF);
	xTaskCreate( start_task, "start", START_STK_SIZE, NULL, START_TASK_PRIO, &START_Task_Handler );//创建空闲任务
	vTaskStartScheduler();//开始调度
	for( ;; );//如果运行到这里，有问题
}



/*******************************************************************
@起始任务
*******************************************************************/
static void start_task( void *pvParameters )
{
	
#if(WATCH_DOG)
	watch_dog_init();
#endif
	msg_init();
	hardware_config();//外设配置
	global_init();
	rtc_config();
	
	taskENTER_CRITICAL();//进入临界区

	xTaskCreate( idle_task, 					"idle", 					IDLE_STK_SIZE, 						NULL, 	IDLE_TASK_PRIO, 					&Idle_Task_Handler );//创建空闲任务
	xTaskCreate( communication_task, 	"communication", 	COMMUNICATION_STK_SIZE, 	NULL, 	communication_TASK_PRIO, 	&communication_Task_Handler );//创建通信任务
	xTaskCreate( fsm_task, 						"fsm_1", 					FSM_STK_SIZE, 						NULL, 	FSM_TASK_PRIO, 						&Fsm_Task_Handler );//状态机任务
	xTaskCreate( log_task, 						"log", 						LOG_STK_SIZE, 						NULL, 	LOG_TASK_PRIO, 						&Log_Task_Handler );//日志任务
	xTaskCreate( key_task, 					  "key", 						KEY_STK_SIZE, 						NULL, 	KEY_TASK_PRIO, 						&Key_Task_Handler );//创建按键任务

//刷卡器任务1~6
	xTaskCreate( swipe_1_task, 						"swip_1", 			SWIPE_1_STK_SIZE, 			NULL, 	SWIPE_1_TASK_PRIO, 						&Swipe_1_Task_Handler );
	xTaskCreate( swipe_2_task, 						"swip_2", 			SWIPE_2_STK_SIZE, 			NULL, 	SWIPE_2_TASK_PRIO, 						&Swipe_2_Task_Handler );
	xTaskCreate( swipe_3_task, 						"swip_3", 			SWIPE_3_STK_SIZE, 			NULL, 	SWIPE_3_TASK_PRIO, 						&Swipe_3_Task_Handler );
	xTaskCreate( swipe_4_task, 						"swip_4", 			SWIPE_4_STK_SIZE, 			NULL, 	SWIPE_4_TASK_PRIO, 						&Swipe_4_Task_Handler );
	xTaskCreate( swipe_5_task, 						"swip_5", 			SWIPE_5_STK_SIZE, 			NULL, 	SWIPE_5_TASK_PRIO, 						&Swipe_5_Task_Handler );
	xTaskCreate( swipe_6_task, 						"swip_6", 			SWIPE_6_STK_SIZE, 			NULL, 	SWIPE_6_TASK_PRIO, 						&Swipe_6_Task_Handler );
	
	printf("FreeRTOS is starting ...\n");
	vTaskDelete(START_Task_Handler); //删除开始任务
	taskEXIT_CRITICAL();            //退出临界区
}

/*******************************************************************
@空闲任务
*******************************************************************/
static void idle_task( void *pvParameters )
{
	uint8_t state = FALSE;
	portTickType xLastExecutionTime;
	uint32_t temp_time = 0;
	xLastExecutionTime = xTaskGetTickCount();
	
	#if(CPU_INFO)
	char show_buf[500];
	#endif

	for( ;; )
	{
		#if(CPU_INFO)
		vTaskList(show_buf);
		printf("任务名      任务状态 优先级 剩余栈 任务序号\r\n");
		printf("%s\r\n", show_buf);
		printf("\r\n");
		#endif
		
		watch_dog_feed();//喂狗
		
		//网络指示灯
		if(class_global.net.state == 1){
			LED_NET = 0;
		}else{
			LED_NET = 1;
		}
		
		//呼吸灯
		LED_BREATH = state;
		state = !state;
		
		if( (temp_time == 0) || (xTaskGetTickCount() - temp_time > ONE_SECOND*600) )//10分钟传送一条温度信息
		{
			instant_equipment_state(TYPE_TEMP, class_global.temp.external.state, class_global.temp.external.val);//汇报温度
			temp_time = xTaskGetTickCount();
		}
		
		/* Perform this check every mainCHECK_DELAY milliseconds. */
		vTaskDelayUntil( &xLastExecutionTime, 1000 );//绝对延时函数，每隔一段时间执行一次
		
//		if( xArePollingQueuesStillRunning() != pdTRUE )//检测所有任务是否运行正常
//		{
//			printf( "ERROR IN POLL Q\n" );
//		}
	}
}


/*******************************************************************
@通信任务
*******************************************************************/
static void communication_task( void *pvParameters )
{
	main_task_communication();
}

/*******************************************************************
@日志任务
*******************************************************************/
static void log_task( void *pvParameters )
{
	task_log_recv();
}

/*******************************************************************
@状态机任务
*******************************************************************/
static void fsm_task( void *pvParameters )
{
	task_fsm();
}


/*******************************************************************
@键盘任务
*******************************************************************/
static void key_task( void *pvParameters )
{
	task_key();
}

/*******************************************************************
@刷卡器任务
*******************************************************************/
static void swipe_1_task( void *pvParameters )
{
	task_swipe1();
}
static void swipe_2_task( void *pvParameters )
{
	task_swipe2();
}
static void swipe_3_task( void *pvParameters )
{
	task_swipe3();
}
static void swipe_4_task( void *pvParameters )
{
	task_swipe4();
}
static void swipe_5_task( void *pvParameters )
{
	task_swipe5();
}
static void swipe_6_task( void *pvParameters )
{
	task_swipe6();
}

