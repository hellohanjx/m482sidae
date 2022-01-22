#include "msg.h"
#include "global.h"
#include "uart0_log.h"



/*************************************************************************************************************************************************
@说明：4G接收数据信号量
*************************************************************************************************************************************************/
static SemaphoreHandle_t handle_4g_recv;//通讯接收信号量句柄

/*
@说明：接收信号量释放
*/
static void c4g_sem_init(void)
{
	handle_4g_recv = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
BaseType_t c4g_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_4g_recv, time);
}
/*
@说明：中断级释放信号量
*/
BaseType_t c4g_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_4g_recv, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}



/*************************************************************************************************************************************************
@说明：log 接口接收数据队列
*************************************************************************************************************************************************/
#define LOG_QUEUE_SIZE  	10	//队列大小

static QueueHandle_t handle_log_queue;//队列句柄

/*
@功能：创建队列
*/
static void log_queue_init(void)
{
	handle_log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(UART0_DATA)); //创建队列
}

/*
@功能：加入队列
@参数：mymail，消息指针
*/
uint8_t log_queue_send(UART0_DATA msg)
{
	BaseType_t taskWoken;//如果发送消息导致一个优先级较高的任务运行，则这里 taskWoken = pfTRUE
	return xQueueSendFromISR(handle_log_queue, (void*)&msg, &taskWoken);//一直等待队列空，直到发送完毕
}
/*
@功能：获取队列中的消息
@参数：msg,消息缓冲指针；time，超时时间
*/
BaseType_t log_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(handle_log_queue, msg, time);
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

static QueueHandle_t handle_report_queue;//通讯队列句柄
/*
@功能：创建通讯队列
*/
static void report_queue_init(void)
{
	handle_report_queue = xQueueCreate(QUEUE_REPORT_SIZE, sizeof(MAIL)); //创建发送队列
}

/*
@功能：汇报消息加到通讯队列头
@参数：mymail，消息指针；type=1 发送到队列头，type=0 发送到队列尾
*/
uint8_t report_queue_send(MAIL* mymail, uint8_t type)
{
	BaseType_t err;
	configASSERT(mymail);
	if (mymail != NULL)
	{
		if(type)
		{
			err = xQueueSendToFront(handle_report_queue, (void*)&mymail, portMAX_DELAY);//丢到队列头
			if(err == errQUEUE_FULL)
			{
				mail_release(mymail);
				printf("report_to_head is full\r\n\r\n");
				return FALSE;
			}
		}
		else
		{
			err = xQueueSend(handle_report_queue, (void*)&mymail, 0);//丢到队列尾；不等待，直接返回发送消息的结果
			if(err == errQUEUE_FULL)
			{
				mail_release(mymail);
				printf("add_report_info is full\r\n\r\n");
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

/*
@功能：获取队列中的消息
@参数：msg,消息缓冲指针；time，超时时间
*/
BaseType_t report_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(handle_report_queue, msg, time);
}

/********************
@说明：即时通信
@时间：2019.12.8
********************/
#define QUEUE_INSTANT_SIZE  	60	//队列大小

static QueueHandle_t queue_instant;//即时通信队列句柄
/*
@功能：创建通讯队列
*/
static void instant_queue_init(void)
{
	queue_instant = xQueueCreate(QUEUE_INSTANT_SIZE, sizeof(MAIL)); //创建发送队列
}

/*
@功能：即时消息加入队列
@参数：mymail，消息指针；type=0，加到队列尾，type=1，加到队列头
*/
uint8_t instant_queue_send(MAIL* mymail, uint8_t type)
{
	BaseType_t err;
	configASSERT(mymail);
	if (mymail != NULL)
	{
		if(type)
		{
			 err = xQueueSendToFront(queue_instant, (void*)&mymail, portMAX_DELAY);//丢到队列头
			if(err == errQUEUE_FULL)
			{
				mail_release(mymail);
				printf("instant_to_head is full\r\n\r\n");
				return FALSE;
			}
		}
		else
		{
			err = xQueueSend(queue_instant, (void*)&mymail, 0);//丢到队列尾；不等待，直接返回发送消息的结果
			if(err == errQUEUE_FULL)
			{
				mail_release(mymail);
				printf("instant_to_tail is full\r\n\r\n");
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

/*
@功能：获取队列中的消息
@参数：msg,消息缓冲指针；time，超时时间
*/
BaseType_t instant_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(queue_instant, msg, time);
}


/*********************************
@说明：通信接口接收数据信号量
*********************************/
static SemaphoreHandle_t handle_communication_sem;//通讯接收信号量句柄

/*
@说明：接收信号量释放
*/
static void communication_sem_init(void)
{
	handle_communication_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
BaseType_t communication_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_communication_sem,  time);
}
/*
@说明：中断中发送信号量
*/
BaseType_t communication_sem_send(void)
{
	BaseType_t xHigherPriorityTaskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_communication_sem, &xHigherPriorityTaskWoken);//中断级信号量释放（不能释放互斥型信号量）
}





/***********************************************************************************************************************************************************
@说明：6个uart 接收信号量
***********************************************************************************************************************************************************/
/*********************
@说明：uart1 接收信号量
*********************/
static SemaphoreHandle_t  handle_uart1_sem;//信号量句柄

/*
@说明：接收信号量释放
*/
static void uart1_sem_init(void)
{
	handle_uart1_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
static BaseType_t uart1_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart1_sem,  time);
}
/*
@说明：中断级释放信号量
*/
static BaseType_t uart1_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_uart1_sem, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}

/*********************
@说明：uart2 接收信号量
*********************/
static SemaphoreHandle_t  handle_uart2_sem;//信号量句柄

/*
@说明：接收信号量释放
*/
static void uart2_sem_init(void)
{
	handle_uart2_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
static BaseType_t uart2_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart2_sem,  time);
}
/*
@说明：中断级释放信号量
*/
static BaseType_t uart2_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_uart2_sem, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}

/*********************
@说明：uart3 接收信号量
*********************/
static SemaphoreHandle_t  handle_uart3_sem;//信号量句柄

/*
@说明：接收信号量释放
*/
static void uart3_sem_init(void)
{
	handle_uart3_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
static BaseType_t uart3_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart3_sem,  time);
}
/*
@说明：中断级释放信号量
*/
static BaseType_t uart3_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_uart3_sem, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}

/*********************
@说明：uart4 接收信号量
*********************/
static SemaphoreHandle_t  handle_uart4_sem;//信号量句柄

/*
@说明：接收信号量释放
*/
static void uart4_sem_init(void)
{
	handle_uart4_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
static BaseType_t uart4_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart4_sem,  time);
}
/*
@说明：中断级释放信号量
*/
static BaseType_t uart4_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_uart4_sem, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}

/*********************
@说明：uart5 接收信号量
*********************/
static SemaphoreHandle_t  handle_uart5_sem;//信号量句柄

/*
@说明：接收信号量释放
*/
static void uart5_sem_init(void)
{
	handle_uart5_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
static BaseType_t uart5_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart5_sem,  time);
}
/*
@说明：中断级释放信号量
*/
static BaseType_t uart5_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_uart5_sem, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}


/*********************
@说明：uart6 接收信号量
*********************/
static SemaphoreHandle_t  handle_uart6_sem;//信号量句柄

/*
@说明：接收信号量释放
*/
static void uart6_sem_init(void)
{
	handle_uart6_sem = xSemaphoreCreateBinary();//创建二值信号量（此信号量默认空）
}
/*
@说明：申请信号量
@参数：time，等待信号量超时时间
*/
static BaseType_t uart6_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart6_sem,  time);
}
/*
@说明：中断级释放信号量
*/
static BaseType_t uart6_sem_send(void)
{
	BaseType_t taskWoken;//这个值等于pdTRUE == 1时退出中断前必须进行任务切换
	return xSemaphoreGiveFromISR(handle_uart6_sem, &taskWoken);//中断级信号量释放（不能释放互斥型信号量）
}


/*********************************************************************************************************************************************
@说明：状态机队列
@时间：2021.12.27
*********************************************************************************************************************************************/
#define FSM_QUEEN_SIZE 			200			//状态机消息队列大小
static QueueHandle_t handle_fsm_queue;//状态机消息句柄
/*
@功能：创建状态机消息队列
*/
static void fsm_queue_init(void)
{
	handle_fsm_queue = xQueueCreate(FSM_QUEEN_SIZE, sizeof(FSM_MSG));//创建状态机队列
}
/*
@功能：发送一条信息到队列后
@参数：msg，消息指针
@说明：这里 msg 如果用 *msg，则是指针传递；msg则是值传递
*/
uint8_t fsm_queue_send(FSM_MSG msg)
{
	BaseType_t rs = xQueueSend(handle_fsm_queue, (void*)&msg, 0);
	if(rs == errQUEUE_FULL)
	{
		printf("fsm queue full\r\n");
	}
	return rs;//发送消息不等待，不成功立即返回结果
}
/*
@功能：获取队列中的消息
@参数：msg,消息缓冲指针；time，超时时间
*/
BaseType_t fsm_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(handle_fsm_queue, msg, time);
}


/*************************************
@说明：定义信号量操作数组
*************************************/
UART_SEM_SEND uart_sem_send[] = {uart5_sem_send, uart1_sem_send, uart4_sem_send, uart6_sem_send, uart2_sem_send, uart3_sem_send};
UART_SEM_GET uart_sem_get[] = {uart5_sem_get, uart1_sem_get, uart4_sem_get, uart6_sem_get, uart2_sem_get, uart3_sem_get};


/**************************************************************************************************************************
@消息系统初始化
**************************************************************************************************************************/
void msg_init(void)
{
	uart1_sem_init();
	uart2_sem_init();
	uart3_sem_init();
	uart4_sem_init();
	uart5_sem_init();
	uart6_sem_init();
	c4g_sem_init();
	communication_sem_init();
	
	log_queue_init();
	report_queue_init();
	instant_queue_init();
	fsm_queue_init();
}
