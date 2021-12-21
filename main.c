#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

///* Demo application includes. */
#include "partest.h"
#include "flash.h"
#include "flop.h"
#include "integer.h"
#include "PollQ.h"
#include "semtest.h"
#include "dynamic.h"
#include "BlockQ.h"
#include "blocktim.h"
#include "countsem.h"
#include "GenQTest.h"
#include "QueueSet.h"
#include "recmutex.h"
#include "death.h"

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



#define START_TASK_PRIO		1			//任务优先级
#define START_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t START_Task_Handler;	//任务句柄
static void start_task( void *pvParameters );

#define COMMUCATION_TASK_PRIO		23		//任务优先级
#define COMMUCATION_STK_SIZE 		512		//任务堆栈大小
TaskHandle_t Commucation_Task_Handler;		//任务句柄
static void communication_task( void *pvParameters );

#define TEST_TASK_PRIO		10		//任务优先级
#define TEST_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Test_Task_Handler;		//任务句柄
static void test_task( void *pvParameters );

#define IDLE_TASK_PRIO		15		//任务优先级
#define IDLE_STK_SIZE 		256		//任务堆栈大小
TaskHandle_t Idle_Task_Handler;		//任务句柄
static void idle_task( void *pvParameters );

/*
@功能：内存测试
@参数：start,起始地址，stop结束地址
*/
	uint8_t n;
	uint32_t start_s;

static void memory_test(uint32_t start, uint32_t stop)
{
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


void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
		printf("apply memory err\r\n");
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
		printf("vApplicationStackOverflowHook\r\n");
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()).  */

#if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 0 )
    {
        /* In this case the tick hook is used as part of the queue set test. */
        vQueueSetAccessQueueSetFromISR();
    }
#endif /* mainCREATE_SIMPLE_BLINKY_DEMO_ONLY */

}


/*******************************************************************
@起始任务
*******************************************************************/
static void start_task( void *pvParameters )
{
	global_init();
	msg_init();
	hardware_config();//外设配置
	rtc_config();
	
	taskENTER_CRITICAL();//进入临界区

	xTaskCreate( idle_task, 					"idle", 				IDLE_STK_SIZE, 					NULL, 	IDLE_TASK_PRIO, 					&Idle_Task_Handler );//创建空闲任务
	xTaskCreate( communication_task, 	"Commucation", 	COMMUCATION_STK_SIZE, 	NULL, 	COMMUCATION_TASK_PRIO, 		&Commucation_Task_Handler );//创建通信任务
	xTaskCreate( test_task, 					"test", 				TEST_STK_SIZE, 					NULL, 	TEST_TASK_PRIO, 					&Test_Task_Handler );//创建测试任务

//	vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );//创建队列
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
	char show_buf[500];
	uint32_t temp_time = 0;
	xLastExecutionTime = xTaskGetTickCount();

	for( ;; )
	{
		#if(0)
		vTaskList(show_buf);
		printf("任务名      任务状态 优先级 剩余栈 任务序号\r\n");
		printf("%s\r\n", show_buf);
		printf("\r\n");
		#endif
		
		if(class_global.net.state == 1)
		{
			LED_NET = 0;
		}
		
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
	main_task_commucation();
}


/*******************************************************************
@测试任务
*******************************************************************/
#include "uart1_config.h"
#include "uart2_config.h"
#include "uart3_config.h"
#include "uart4_config.h"
#include "uart5_config.h"
#include "uart6_config.h"


	UART1_DATA tx1, *rx1;
	UART6_DATA tx6, *rx6;
	UART2_DATA tx2, *rx2;
	UART3_DATA tx3, *rx3;
	UART4_DATA tx4, *rx4;
	UART5_DATA tx5, *rx5;

static void test_task( void *pvParameters )
{
	char tmp[10];
	CUR_TIME time;
	for(;;)
	{
		uint8_t n;

		tx1.len = 0;
		tx6.len = 0;
		tx2.len = 0;
		tx3.len = 0;
		tx4.len = 0;
		tx5.len = 0;
		
		for(n = 0; n < 26; n++)
		{
			tx1.buf[tx1.len++] = 'a'+n;
			tx6.buf[tx6.len++] = 'a'+n;
			tx2.buf[tx2.len++] = 'a'+n;
			tx3.buf[tx3.len++] = 'a'+n;
			tx4.buf[tx4.len++] = 'a'+n;
			tx5.buf[tx5.len++] = 'a'+n;
		}
		
		_uart1_send(&tx1, &rx1, 0);
		_uart6_send(&tx6, &rx6, 0);
		_uart2_send(&tx2, &rx2, 0);
		_uart3_send(&tx3, &rx3, 0);
		_uart4_send(&tx4, &rx4, 0);
		_uart5_send(&tx5, &rx5, 0);

		time = get_cur_time();
		
		tmp[sprintf(tmp, "%u", time.year)] = 0;
		printf(tmp);
		printf("-");
		tmp[sprintf(tmp, "%u", time.month)] = 0;
		printf(tmp);
		printf("-");
		tmp[sprintf(tmp, "%u", time.day)] = 0;
		printf(tmp);
		printf(" ");
		tmp[sprintf(tmp, "%u", time.hour)] = 0;
		printf(tmp);
		printf(":");
		tmp[sprintf(tmp, "%u", time.min)] = 0;
		printf(tmp);
		printf(":");
		tmp[sprintf(tmp, "%u", time.sec )] = 0;
		printf(tmp);
		printf("+");
		tmp[sprintf(tmp, "%u", time.week )] = 0;
		printf(tmp);
		printf("\r\n");
//		UART_Write(UART1, tx.buf, tx.len);
		//外部温度
		get_external_temp((int*)&class_global.temp.external.val, (uint8_t*)&class_global.temp.external.state);
		printf("external temp = ");
		tmp[sprintf(tmp, "%d", class_global.temp.external.val )] = 0;
		printf(tmp);
		printf("\r\n");
		printf("external temp state = ");
		tmp[sprintf(tmp, "%d", class_global.temp.external.state )] = 0;
		printf(tmp);
		printf("\r\n");
		
		//内部温度
		class_global.temp.internal.val = get_internal_temp();
		printf("internal temp = ");
		tmp[sprintf(tmp, "%d", class_global.temp.internal.val )] = 0;
		printf(tmp);
		printf("\r\n");
	
		vTaskDelay(3000);
	}
}

