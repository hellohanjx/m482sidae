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


#define START_TASK_PRIO		1			//�������ȼ�
#define START_STK_SIZE 		256		//�����ջ��С
TaskHandle_t START_Task_Handler;	//������
static void start_task( void *pvParameters );

#define communication_TASK_PRIO		27		//�������ȼ�
#define COMMUNICATION_STK_SIZE 		512		//�����ջ��С
TaskHandle_t communication_Task_Handler;		//������
static void communication_task( void *pvParameters );

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

#define KEY_TASK_PRIO		20		//�������ȼ�
#define KEY_STK_SIZE 		256		//�����ջ��С
TaskHandle_t Key_Task_Handler;		//������
static void key_task( void *pvParameters );

#define FSM_TASK_PRIO		26		//�������ȼ�
#define FSM_STK_SIZE 		512		//�����ջ��С
TaskHandle_t Fsm_Task_Handler;		//������
static void fsm_task( void *pvParameters );



/*
@���ܣ��ڴ����
@������start,��ʼ��ַ��stop������ַ
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
	
#if(WATCH_DOG)
	watch_dog_init();
#endif
	msg_init();
	hardware_config();//��������
	global_init();
	rtc_config();
	
	taskENTER_CRITICAL();//�����ٽ���

	xTaskCreate( idle_task, 					"idle", 					IDLE_STK_SIZE, 						NULL, 	IDLE_TASK_PRIO, 					&Idle_Task_Handler );//������������
	xTaskCreate( communication_task, 	"communication", 	COMMUNICATION_STK_SIZE, 	NULL, 	communication_TASK_PRIO, 	&communication_Task_Handler );//����ͨ������
	xTaskCreate( fsm_task, 						"fsm_1", 					FSM_STK_SIZE, 						NULL, 	FSM_TASK_PRIO, 						&Fsm_Task_Handler );//״̬������
	xTaskCreate( log_task, 						"log", 						LOG_STK_SIZE, 						NULL, 	LOG_TASK_PRIO, 						&Log_Task_Handler );//��־����
	xTaskCreate( key_task, 					  "key", 						KEY_STK_SIZE, 						NULL, 	KEY_TASK_PRIO, 						&Key_Task_Handler );//������������

//ˢ��������1~6
	xTaskCreate( swipe_1_task, 						"swip_1", 			SWIPE_1_STK_SIZE, 			NULL, 	SWIPE_1_TASK_PRIO, 						&Swipe_1_Task_Handler );
	xTaskCreate( swipe_2_task, 						"swip_2", 			SWIPE_2_STK_SIZE, 			NULL, 	SWIPE_2_TASK_PRIO, 						&Swipe_2_Task_Handler );
	xTaskCreate( swipe_3_task, 						"swip_3", 			SWIPE_3_STK_SIZE, 			NULL, 	SWIPE_3_TASK_PRIO, 						&Swipe_3_Task_Handler );
	xTaskCreate( swipe_4_task, 						"swip_4", 			SWIPE_4_STK_SIZE, 			NULL, 	SWIPE_4_TASK_PRIO, 						&Swipe_4_Task_Handler );
	xTaskCreate( swipe_5_task, 						"swip_5", 			SWIPE_5_STK_SIZE, 			NULL, 	SWIPE_5_TASK_PRIO, 						&Swipe_5_Task_Handler );
	xTaskCreate( swipe_6_task, 						"swip_6", 			SWIPE_6_STK_SIZE, 			NULL, 	SWIPE_6_TASK_PRIO, 						&Swipe_6_Task_Handler );
	
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
	uint32_t temp_time = 0;
	xLastExecutionTime = xTaskGetTickCount();
	
	#if(CPU_INFO)
	char show_buf[500];
	#endif

	for( ;; )
	{
		#if(CPU_INFO)
		vTaskList(show_buf);
		printf("������      ����״̬ ���ȼ� ʣ��ջ �������\r\n");
		printf("%s\r\n", show_buf);
		printf("\r\n");
		#endif
		
		watch_dog_feed();//ι��
		
		//����ָʾ��
		if(class_global.net.state == 1){
			LED_NET = 0;
		}else{
			LED_NET = 1;
		}
		
		//������
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
@״̬������
*******************************************************************/
static void fsm_task( void *pvParameters )
{
	task_fsm();
}


/*******************************************************************
@��������
*******************************************************************/
static void key_task( void *pvParameters )
{
	task_key();
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

