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



#define START_TASK_PRIO		1			//�������ȼ�
#define START_STK_SIZE 		256		//�����ջ��С
TaskHandle_t START_Task_Handler;	//������
static void start_task( void *pvParameters );

#define communication_TASK_PRIO		23		//�������ȼ�
#define communication_STK_SIZE 		512		//�����ջ��С
TaskHandle_t communication_Task_Handler;		//������
static void communication_task( void *pvParameters );

#define TEST_TASK_PRIO		10		//�������ȼ�
#define TEST_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Test_Task_Handler;		//������
static void test_task( void *pvParameters );

#define IDLE_TASK_PRIO		15		//�������ȼ�
#define IDLE_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Idle_Task_Handler;		//������
static void idle_task( void *pvParameters );

#define LOG_TASK_PRIO		11		//�������ȼ�
#define LOG_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Log_Task_Handler;		//������
static void log_task( void *pvParameters );

#define SWIPE_1_TASK_PRIO		25		//�������ȼ�
#define SWIPE_1_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Swipe_1_Task_Handler;		//������
static void swipe_1_task( void *pvParameters );

#define SWIPE_2_TASK_PRIO		25		//�������ȼ�
#define SWIPE_2_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Swipe_2_Task_Handler;		//������
static void swipe_2_task( void *pvParameters );

#define SWIPE_3_TASK_PRIO		25		//�������ȼ�
#define SWIPE_3_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Swipe_3_Task_Handler;		//������
static void swipe_3_task( void *pvParameters );

#define SWIPE_4_TASK_PRIO		25		//�������ȼ�
#define SWIPE_4_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Swipe_4_Task_Handler;		//������
static void swipe_4_task( void *pvParameters );

#define SWIPE_5_TASK_PRIO		25		//�������ȼ�
#define SWIPE_5_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Swipe_5_Task_Handler;		//������
static void swipe_5_task( void *pvParameters );

#define SWIPE_6_TASK_PRIO		25		//�������ȼ�
#define SWIPE_6_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Swipe_6_Task_Handler;		//������
static void swipe_6_task( void *pvParameters );

/*
@���ܣ��ڴ����
@������start,��ʼ��ַ��stop������ַ
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
@��ѭ��
***********************************/
int main(void)
{
	NVIC_SetPriorityGrouping(0);
	sys_clock_init();
//	memory_test(0x20000000, 0x20027FFF);
	xTaskCreate( start_task, "start", START_STK_SIZE, NULL, START_TASK_PRIO, &START_Task_Handler );//������������
	vTaskStartScheduler();//��ʼ����
	for( ;; );//������е����������
}



/*******************************************************************
@��ʼ����
*******************************************************************/
static void start_task( void *pvParameters )
{
	msg_init();
	hardware_config();//��������
	global_init();
	rtc_config();
	
	taskENTER_CRITICAL();//�����ٽ���

	xTaskCreate( idle_task, 					"idle", 				IDLE_STK_SIZE, 					NULL, 	IDLE_TASK_PRIO, 					&Idle_Task_Handler );//������������
	xTaskCreate( communication_task, 	"communication", 	communication_STK_SIZE, 	NULL, 	communication_TASK_PRIO, 		&communication_Task_Handler );//����ͨ������
	xTaskCreate( log_task, 						"log", 					LOG_STK_SIZE, 					NULL, 	LOG_TASK_PRIO, 						&Log_Task_Handler );//��־����
	
	//ˢ��������1~6
	xTaskCreate( swipe_1_task, 						"swip_1", 			SWIPE_1_STK_SIZE, 			NULL, 	SWIPE_1_TASK_PRIO, 						&Swipe_1_Task_Handler );
	xTaskCreate( swipe_2_task, 						"swip_2", 			SWIPE_2_STK_SIZE, 			NULL, 	SWIPE_2_TASK_PRIO, 						&Swipe_2_Task_Handler );
	xTaskCreate( swipe_3_task, 						"swip_3", 			SWIPE_3_STK_SIZE, 			NULL, 	SWIPE_3_TASK_PRIO, 						&Swipe_3_Task_Handler );
	xTaskCreate( swipe_4_task, 						"swip_4", 			SWIPE_4_STK_SIZE, 			NULL, 	SWIPE_4_TASK_PRIO, 						&Swipe_4_Task_Handler );
	xTaskCreate( swipe_5_task, 						"swip_5", 			SWIPE_5_STK_SIZE, 			NULL, 	SWIPE_5_TASK_PRIO, 						&Swipe_5_Task_Handler );
	xTaskCreate( swipe_6_task, 						"swip_6", 			SWIPE_6_STK_SIZE, 			NULL, 	SWIPE_6_TASK_PRIO, 						&Swipe_6_Task_Handler );
	
	
	
//	xTaskCreate( test_task, 					"test", 				TEST_STK_SIZE, 					NULL, 	TEST_TASK_PRIO, 					&Test_Task_Handler );//������������

	
//	vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );//��������
	
	printf("FreeRTOS is starting ...\n");
	
	vTaskDelete(START_Task_Handler); //ɾ����ʼ����
	taskEXIT_CRITICAL();            //�˳��ٽ���
}

/*******************************************************************
@��������
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
		printf("������      ����״̬ ���ȼ� ʣ��ջ �������\r\n");
		printf("%s\r\n", show_buf);
		printf("\r\n");
		#endif
		
		if(class_global.net.state == 1)
		{
			LED_NET = 0;
		}
		
		LED_BREATH = state;
		state = !state;
		
		if( (temp_time == 0) || (xTaskGetTickCount() - temp_time > ONE_SECOND*600) )//10���Ӵ���һ���¶���Ϣ
		{
			instant_equipment_state(TYPE_TEMP, class_global.temp.external.state, class_global.temp.external.val);//�㱨�¶�
			temp_time = xTaskGetTickCount();
		}
		
		/* Perform this check every mainCHECK_DELAY milliseconds. */
		vTaskDelayUntil( &xLastExecutionTime, 1000 );//������ʱ������ÿ��һ��ʱ��ִ��һ��
		
//		if( xArePollingQueuesStillRunning() != pdTRUE )//������������Ƿ���������
//		{
//			printf( "ERROR IN POLL Q\n" );
//		}
	}
}


/*******************************************************************
@ͨ������
*******************************************************************/
static void communication_task( void *pvParameters )
{
	main_task_communication();
}

/*******************************************************************
@��־����
*******************************************************************/
static void log_task( void *pvParameters )
{
	task_log_recv();
}

/*******************************************************************
@ˢ��������
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


/*******************************************************************
@��������
*******************************************************************/
#include "uart_config.h"


	UART_DATA tx1, *rx1;
	UART_DATA tx6, *rx6;
	UART_DATA tx2, *rx2;
	UART_DATA tx3, *rx3;
	UART_DATA tx4, *rx4;
	UART_DATA tx5, *rx5;

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
		//�ⲿ�¶�
		get_external_temp((int*)&class_global.temp.external.val, (uint8_t*)&class_global.temp.external.state);
		printf("external temp = ");
		tmp[sprintf(tmp, "%d", class_global.temp.external.val )] = 0;
		printf(tmp);
		printf("\r\n");
		printf("external temp state = ");
		tmp[sprintf(tmp, "%d", class_global.temp.external.state )] = 0;
		printf(tmp);
		printf("\r\n");
		
		//�ڲ��¶�
		class_global.temp.internal.val = get_internal_temp();
		printf("internal temp = ");
		tmp[sprintf(tmp, "%d", class_global.temp.internal.val )] = 0;
		printf(tmp);
		printf("\r\n");
	
		vTaskDelay(3000);
	}
}

