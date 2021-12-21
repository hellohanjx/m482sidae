#include "msg.h"
#include "global.h"

/*************************************************************************************************************************************************
@˵����4G���������ź���
*************************************************************************************************************************************************/
static SemaphoreHandle_t sem_4g_recv;//ͨѶ�����ź������

/*
@˵���������ź����ͷ�
*/
static void c4g_recv_init(void)
{
	sem_4g_recv = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
BaseType_t c4g_recv_get(uint32_t time)
{
	return xSemaphoreTake(sem_4g_recv, time);
}
/*
@˵�����жϼ��ͷ��ź���
*/
BaseType_t c4g_recv_send_isr(void)
{
	BaseType_t taskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(sem_4g_recv, &taskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
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

static QueueHandle_t queue_report;//ͨѶ���о��
/*
@���ܣ�����ͨѶ����
*/
static void report_queue_init(void)
{
	queue_report = xQueueCreate(QUEUE_REPORT_SIZE, sizeof(MAIL)); //�������Ͷ���
}

/*
@���ܣ��㱨��Ϣ�ӵ�ͨѶ����β
@������mymail����Ϣָ��
*/
uint8_t report_to_tail(MAIL* mymail)
{
	configASSERT(mymail);
	if(mymail != NULL)
	{
		BaseType_t err = xQueueSend(queue_report, (void*)&mymail, 0);//��������β�����ȴ���ֱ�ӷ��ط�����Ϣ�Ľ��
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
@���ܣ��㱨��Ϣ�ӵ�ͨѶ����ͷ
@������mymail����Ϣָ��
*/
uint8_t report_to_head(MAIL* mymail)
{
	configASSERT(mymail);
	if (mymail != NULL)
	{
		BaseType_t err = xQueueSendToFront(queue_report, (void*)&mymail, portMAX_DELAY);//��������ͷ
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
@���ܣ���ȡ�����е���Ϣ
@������msg,��Ϣ����ָ�룻time����ʱʱ��
*/
BaseType_t report_data_get(void *const msg, TickType_t time)
{
	return xQueueReceive(queue_report, msg, time);
}

/********************
@˵������ʱͨ��
@ʱ�䣺2019.12.8
********************/
#define QUEUE_INSTANT_SIZE  	10	//���д�С

static QueueHandle_t queue_instant;//��ʱͨ�Ŷ��о��
/*
@���ܣ�����ͨѶ����
*/
static void instant_queue_init(void)
{
	queue_instant = xQueueCreate(QUEUE_INSTANT_SIZE, sizeof(MAIL)); //�������Ͷ���
}

/*
@���ܣ���ʱ��Ϣ�ӵ�����β
@������mymail����Ϣָ��
*/
uint8_t instant_to_tail(MAIL* mymail)
{
	configASSERT(mymail);
	if(mymail != NULL)
	{
		BaseType_t err = xQueueSend(queue_instant, (void*)&mymail, 0);//��������β�����ȴ���ֱ�ӷ��ط�����Ϣ�Ľ��
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
@���ܣ���ʱ��Ϣ�ӵ�����ͷ
@������mymail����Ϣָ��
*/
uint8_t instant_to_head(MAIL* mymail)
{
	configASSERT(mymail);
	if (mymail != NULL)
	{
		BaseType_t err = xQueueSendToFront(queue_instant, (void*)&mymail, portMAX_DELAY);//��������ͷ
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
@���ܣ���ȡ�����е���Ϣ
@������msg,��Ϣ����ָ�룻time����ʱʱ��
*/
BaseType_t instant_data_get(void *const msg, TickType_t time)
{
	return xQueueReceive(queue_instant, msg, time);
}


/*********************************
@˵����ͨ�Žӿڽ��������ź���
*********************************/
static SemaphoreHandle_t sem_commucation_recv;//ͨѶ�����ź������

/*
@˵���������ź����ͷ�
*/
static void commucation_bus_init(void)
{
	sem_commucation_recv = xSemaphoreCreateBinary();//������ֵ�ź��������ź���Ĭ�Ͽգ�
}
/*
@˵���������ź���
@������time���ȴ��ź�����ʱʱ��
*/
BaseType_t commucation_sem_get(uint32_t time)
{
	return xSemaphoreTake(sem_commucation_recv,  time);
}
/*
@˵�����ж��з����ź���
*/
BaseType_t commucation_sem_send_isr(void)
{
	BaseType_t xHigherPriorityTaskWoken;//���ֵ����pdTRUE == 1ʱ�˳��ж�ǰ������������л�
	return xSemaphoreGiveFromISR(sem_commucation_recv, &xHigherPriorityTaskWoken);//�жϼ��ź����ͷţ������ͷŻ������ź�����
}





/**************************************************************************************************************************
@��Ϣϵͳ��ʼ��
**************************************************************************************************************************/
void msg_init(void)
{
	c4g_recv_init();
	report_queue_init();
	instant_queue_init();
	commucation_bus_init();
}
