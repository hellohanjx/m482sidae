#include "msg.h"
#include "global.h"
#include "uart0_log.h"



/*************************************************************************************************************************************************
@˵����4G���������ź���
*************************************************************************************************************************************************/
static SemaphoreHandle_t handle_4g_recv;//ͨѶ�����ź������

/*
@˵���������ź����ͷ�
*/
static void c4g_sem_init(void)
{
	handle_4g_recv = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
BaseType_t c4g_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_4g_recv, time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
BaseType_t c4g_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_4g_recv, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}



/*************************************************************************************************************************************************
@˵����log �ӿڽ������ݶ���
*************************************************************************************************************************************************/
#define LOG_QUEUE_SIZE  	10	//���д�С

static QueueHandle_t handle_log_queue;//���о��

/*
@���ܣ���������
*/
static void log_queue_init(void)
{
	handle_log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(UART0_DATA)); //��������
}

/*
@���ܣ��������
@������mymail����Ϣָ��
*/
uint8_t log_queue_send(UART0_DATA msg)
{
	BaseType_t taskWoken;//���������Ϣ����һ�����ȼ��ϸߵ��������У������� taskWoken = pfTRUE
	return xQueueSendFromISR(handle_log_queue, (void*)&msg, &taskWoken);//һֱ�ȴ����пգ�ֱ���������
}
/*
@���ܣ���ȡ�����е���Ϣ
@������msg,��Ϣ����ָ�룻time����ʱʱ��
*/
BaseType_t log_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(handle_log_queue, msg, time);
}




/*********************************************************************************************************************************************
@˵����ͨѶϵͳ
@ʱ�䣺2018.3.26
*********************************************************************************************************************************************/
/********************
@˵����ͨ���ڴ�����
@ʱ�䣺2019.12.8
********************/
/*
@���ܣ�����ռ�����������
@������size�������С
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
				restart_equ_set(RESET_ApplyMail, TRUE); //�����־λ��λ
			}
		}
	}while(mail == 0);
	
	return mail;
}

/*
�ͷ����ݿռ�
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
@˵�����㱨��Ϣ����
@ʱ�䣺2018.3.26
*******************/
#define QUEUE_REPORT_SIZE  	200	//�㱨���д�С

static QueueHandle_t handle_report_queue;//ͨѶ���о��
/*
@���ܣ�����ͨѶ����
*/
static void report_queue_init(void)
{
	handle_report_queue = xQueueCreate(QUEUE_REPORT_SIZE, sizeof(MAIL)); //�������Ͷ���
}

/*
@���ܣ��㱨��Ϣ�ӵ�ͨѶ����ͷ
@������mymail����Ϣָ�룻type=1 ���͵�����ͷ��type=0 ���͵�����β
*/
uint8_t report_queue_send(MAIL* mymail, uint8_t type)
{
	BaseType_t err;
	configASSERT(mymail);
	if (mymail != NULL)
	{
		if(type)
		{
			err = xQueueSendToFront(handle_report_queue, (void*)&mymail, portMAX_DELAY);//��������ͷ
			if(err == errQUEUE_FULL)
			{
				mail_release(mymail);
				printf("report_to_head is full\r\n\r\n");
				return FALSE;
			}
		}
		else
		{
			err = xQueueSend(handle_report_queue, (void*)&mymail, 0);//��������β�����ȴ���ֱ�ӷ��ط�����Ϣ�Ľ��
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
@���ܣ���ȡ�����е���Ϣ
@������msg,��Ϣ����ָ�룻time����ʱʱ��
*/
BaseType_t report_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(handle_report_queue, msg, time);
}

/********************
@˵������ʱͨ��
@ʱ�䣺2019.12.8
********************/
#define QUEUE_INSTANT_SIZE  	60	//���д�С

static QueueHandle_t queue_instant;//��ʱͨ�Ŷ��о��
/*
@���ܣ�����ͨѶ����
*/
static void instant_queue_init(void)
{
	queue_instant = xQueueCreate(QUEUE_INSTANT_SIZE, sizeof(MAIL)); //�������Ͷ���
}

/*
@���ܣ���ʱ��Ϣ�������
@������mymail����Ϣָ�룻type=0���ӵ�����β��type=1���ӵ�����ͷ
*/
uint8_t instant_queue_send(MAIL* mymail, uint8_t type)
{
	BaseType_t err;
	configASSERT(mymail);
	if (mymail != NULL)
	{
		if(type)
		{
			 err = xQueueSendToFront(queue_instant, (void*)&mymail, portMAX_DELAY);//��������ͷ
			if(err == errQUEUE_FULL)
			{
				mail_release(mymail);
				printf("instant_to_head is full\r\n\r\n");
				return FALSE;
			}
		}
		else
		{
			err = xQueueSend(queue_instant, (void*)&mymail, 0);//��������β�����ȴ���ֱ�ӷ��ط�����Ϣ�Ľ��
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
@���ܣ���ȡ�����е���Ϣ
@������msg,��Ϣ����ָ�룻time����ʱʱ��
*/
BaseType_t instant_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(queue_instant, msg, time);
}


/*********************************
@˵����ͨ�Žӿڽ��������ź���
*********************************/
static SemaphoreHandle_t handle_communication_sem;//ͨѶ�����ź������

/*
@˵���������ź����ͷ�
*/
static void communication_sem_init(void)
{
	handle_communication_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
BaseType_t communication_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_communication_sem,  time);
}
/*
@˵�����ж��з����ź���
*/
BaseType_t communication_sem_send(void)
{
	BaseType_t xHigherPriorityTaskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_communication_sem, &xHigherPriorityTaskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}





/***********************************************************************************************************************************************************
@˵����6��uart �����ź���
***********************************************************************************************************************************************************/
/*********************
@˵����uart1 �����ź���
*********************/
static SemaphoreHandle_t  handle_uart1_sem;//�ź������

/*
@˵���������ź����ͷ�
*/
static void uart1_sem_init(void)
{
	handle_uart1_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
static BaseType_t uart1_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart1_sem,  time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
static BaseType_t uart1_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_uart1_sem, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}

/*********************
@˵����uart2 �����ź���
*********************/
static SemaphoreHandle_t  handle_uart2_sem;//�ź������

/*
@˵���������ź����ͷ�
*/
static void uart2_sem_init(void)
{
	handle_uart2_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
static BaseType_t uart2_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart2_sem,  time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
static BaseType_t uart2_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_uart2_sem, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}

/*********************
@˵����uart3 �����ź���
*********************/
static SemaphoreHandle_t  handle_uart3_sem;//�ź������

/*
@˵���������ź����ͷ�
*/
static void uart3_sem_init(void)
{
	handle_uart3_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
static BaseType_t uart3_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart3_sem,  time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
static BaseType_t uart3_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_uart3_sem, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}

/*********************
@˵����uart4 �����ź���
*********************/
static SemaphoreHandle_t  handle_uart4_sem;//�ź������

/*
@˵���������ź����ͷ�
*/
static void uart4_sem_init(void)
{
	handle_uart4_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
static BaseType_t uart4_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart4_sem,  time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
static BaseType_t uart4_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_uart4_sem, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}

/*********************
@˵����uart5 �����ź���
*********************/
static SemaphoreHandle_t  handle_uart5_sem;//�ź������

/*
@˵���������ź����ͷ�
*/
static void uart5_sem_init(void)
{
	handle_uart5_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
static BaseType_t uart5_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart5_sem,  time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
static BaseType_t uart5_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_uart5_sem, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}


/*********************
@˵����uart6 �����ź���
*********************/
static SemaphoreHandle_t  handle_uart6_sem;//�ź������

/*
@˵���������ź����ͷ�
*/
static void uart6_sem_init(void)
{
	handle_uart6_sem = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
static BaseType_t uart6_sem_get(uint32_t time)
{
	return xSemaphoreTake(handle_uart6_sem,  time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
static BaseType_t uart6_sem_send(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(handle_uart6_sem, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}


/*********************************************************************************************************************************************
@˵����״̬������
@ʱ�䣺2021.12.27
*********************************************************************************************************************************************/
#define FSM_QUEEN_SIZE 			200			//״̬����Ϣ���д�С
static QueueHandle_t handle_fsm_queue;//״̬����Ϣ���
/*
@���ܣ�����״̬����Ϣ����
*/
static void fsm_queue_init(void)
{
	handle_fsm_queue = xQueueCreate(FSM_QUEEN_SIZE, sizeof(FSM_MSG));//����״̬������
}
/*
@���ܣ�����һ����Ϣ�����к�
@������msg����Ϣָ��
@˵�������� msg ����� *msg������ָ�봫�ݣ�msg����ֵ����
*/
uint8_t fsm_queue_send(FSM_MSG msg)
{
	BaseType_t rs = xQueueSend(handle_fsm_queue, (void*)&msg, 0);
	if(rs == errQUEUE_FULL)
	{
		printf("fsm queue full\r\n");
	}
	return rs;//������Ϣ���ȴ������ɹ��������ؽ��
}
/*
@���ܣ���ȡ�����е���Ϣ
@������msg,��Ϣ����ָ�룻time����ʱʱ��
*/
BaseType_t fsm_queue_get(void *const msg, TickType_t time)
{
	return xQueueReceive(handle_fsm_queue, msg, time);
}


/*************************************
@˵���������ź�����������
*************************************/
UART_SEM_SEND uart_sem_send[] = {uart5_sem_send, uart1_sem_send, uart4_sem_send, uart6_sem_send, uart2_sem_send, uart3_sem_send};
UART_SEM_GET uart_sem_get[] = {uart5_sem_get, uart1_sem_get, uart4_sem_get, uart6_sem_get, uart2_sem_get, uart3_sem_get};


/**************************************************************************************************************************
@��Ϣϵͳ��ʼ��
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
