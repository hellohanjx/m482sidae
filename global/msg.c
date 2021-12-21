#include "msg.h"
#include "global.h"

/*************************************************************************************************************************************************
@说明：4G接收数据信号量
*************************************************************************************************************************************************/
static SemaphoreHandle_t sem_4g_recv;//通讯接收信号量句柄

/*
@说明：接收信号量释放
*/
static void c4g_recv_init(void)
{
	sem_4g_recv = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
BaseType_t c4g_recv_get(uint32_t time)
{
	return xSemaphoreTake(sem_4g_recv, time);
}
/*
@说明：中断级释放信号量
*/
BaseType_t c4g_recv_send_isr(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(sem_4g_recv, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}






/*********************************************************************************************************************************************
@说明：通讯系统
@时间：2018.3.26
*********************************************************************************************************************************************/
/********************
@说明：通信内存申请
@时间：2019.12.8
********************/
/*
@功能：申请空间用来发数据
@参数：size，申请大小
*/
MAIL* mail_apply(uint16_t size)
{
	MAIL *mail = 0;
	uint8_t cnt = 0;
	
	do{
		mail = (MAIL*)pvPortMalloc((sizeof(MAIL)));
		configASSERT(mail);
		if(mail != 0)
		{
			mail->addr = pvPortMalloc(size);
			configASSERT(mail->addr);
			if(mail->addr == 0)
			{
				mail->addr = mail->addr;
			}
		}
		else
		{
			printf("mail_apply() err\r\n");
			vTaskDelay(500);
			if(cnt++ > 5)
			{
				restart_equ_set(RESET_ApplyMail, TRUE); //错误标志位置位
			}
		}
	}while(mail == 0);
	
	return mail;
}

/*
释放数据空间
*/
void  mail_release(MAIL* mail)
{
	if(mail->addr != 0)
	{
		vPortFree(mail->addr);
		vPortFree(mail);
	}
	mail = 0;
}

/*******************
@说明：汇报信息队列
@时间：2018.3.26
*******************/
#define QUEUE_REPORT_SIZE  	200	//汇报队列大小

static QueueHandle_t queue_report;//通讯队列句柄
/*
@功能：创建通讯队列
*/
static void report_queue_init(void)
{
	queue_report = xQueueCreate(QUEUE_REPORT_SIZE, sizeof(MAIL)); //创建发送队列
}

/*
@功能：汇报消息加到通讯队列尾
@参数：mymail，消息指针
*/
uint8_t report_to_tail(MAIL* mymail)
{
	configASSERT(mymail);
	if(mymail != NULL)
	{
		BaseType_t err = xQueueSend(queue_report, (void*)&mymail, 0);//丢到队列尾；不等待，直接返回发送消息的结果
		if(err == errQUEUE_FULL)
		{
			mail_release(mymail);
			printf("add_report_info is full\r\n\r\n");
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}
/*
@功能：汇报消息加到通讯队列头
@参数：mymail，消息指针
*/
uint8_t report_to_head(MAIL* mymail)
{
	configASSERT(mymail);
	if (mymail != NULL)
	{
		BaseType_t err = xQueueSendToFront(queue_report, (void*)&mymail, portMAX_DELAY);//丢到队列头
		if(err == errQUEUE_FULL)
		{
			mail_release(mymail);
			printf("report_to_head is full\r\n\r\n");
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/*
@功能：获取队列中的消息
@参数：msg,消息缓冲指针；time，超时时间
*/
BaseType_t report_data_get(void *const msg, TickType_t time)
{
	return xQueueReceive(queue_report, msg, time);
}

/********************
@说明：即时通信
@时间：2019.12.8
********************/
#define QUEUE_INSTANT_SIZE  	10	//队列大小

static QueueHandle_t queue_instant;//即时通信队列句柄
/*
@功能：创建通讯队列
*/
static void instant_queue_init(void)
{
	queue_instant = xQueueCreate(QUEUE_INSTANT_SIZE, sizeof(MAIL)); //创建发送队列
}

/*
@功能：即时消息加到队列尾
@参数：mymail，消息指针
*/
uint8_t instant_to_tail(MAIL* mymail)
{
	configASSERT(mymail);
	if(mymail != NULL)
	{
		BaseType_t err = xQueueSend(queue_instant, (void*)&mymail, 0);//丢到队列尾；不等待，直接返回发送消息的结果
		if(err == errQUEUE_FULL)
		{
			mail_release(mymail);
			printf("instant_to_tail is full\r\n\r\n");
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}
/*
@功能：即时消息加到队列头
@参数：mymail，消息指针
*/
uint8_t instant_to_head(MAIL* mymail)
{
	configASSERT(mymail);
	if (mymail != NULL)
	{
		BaseType_t err = xQueueSendToFront(queue_instant, (void*)&mymail, portMAX_DELAY);//丢到队列头
		if(err == errQUEUE_FULL)
		{
			mail_release(mymail);
			printf("instant_to_head is full\r\n\r\n");
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/*
@功能：获取队列中的消息
@参数：msg,消息缓冲指针；time，超时时间
*/
BaseType_t instant_data_get(void *const msg, TickType_t time)
{
	return xQueueReceive(queue_instant, msg, time);
}


/*********************************
@说明：通信接口接收数据信号量
*********************************/
static SemaphoreHandle_t sem_commucation_recv;//通讯接收信号量句柄

/*
@说明：接收信号量释放
*/
static void commucation_bus_init(void)
{
	sem_commucation_recv = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
BaseType_t commucation_sem_get(uint32_t time)
{
	return xSemaphoreTake(sem_commucation_recv,  time);
}
/*
@说明：中断中发送信号量
*/
BaseType_t commucation_sem_send_isr(void)
{
	BaseType_t xHigherPriorityTaskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(sem_commucation_recv, &xHigherPriorityTaskWoken);//中断级信号量释放（不能释放互斥型信号量）
}





/**************************************************************************************************************************
@消息系统初始化
**************************************************************************************************************************/
void msg_init(void)
{
	c4g_recv_init();
	report_queue_init();
	instant_queue_init();
	commucation_bus_init();
}
