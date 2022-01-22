#ifndef _MSG_H_
#define _MSG_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "uart0_log.h"

enum{TAIL, HEAD};


/**********************************************
@˵����״̬����Ϣ����
**********************************************/
enum{	
	MsgCard,						//����Ϣ
	MsgCardServerAck,		//���߿�����Ӧ��
	MsgServerAckErr,		//����Ӧ��ʱ
	MsgKey,							//������Ϣ
};


/**********************************************
@˵����ͨ��ϵͳ�������ݴ���ص�����
@������*recv,�������ݻ���ָ��
			 recv_len���������ݳ���
			 *send���������ݻ���ָ��
			 id������������Դid
**********************************************/
typedef uint8_t (*COM_RECV_CALLBACK)(uint8_t *recv, uint16_t recv_len, uint8_t *send, uint8_t id);

/******************************************
@˵����ͨ��ϵͳ��Ϣ�ṹ
******************************************/
typedef struct MAIL
{
	COM_RECV_CALLBACK com_call_back;//�ص�����ָ��
	uint8_t *addr;//����ָ��
	uint8_t id;//������Դʶ��
}MAIL;



/******************************************
@˵����״̬����Ϣ�ṹ
******************************************/
typedef struct FSM_MSG
{
	uint8_t   id;//��Ϣid����Ӧ 1~6 ˢ��ͷ��ȡֵ 0~5��
	uint8_t   type; 	//��Ϣ���͡��ۿ��ѯ
	uint8_t		stype;	//��Ϣ������
	uint32_t  value; 	//��Ϣ��ֵ
}FSM_MSG;


/*
@����ָ��
*/
typedef BaseType_t(*UART_SEM_SEND)(void);//�ź�������
typedef BaseType_t(*UART_SEM_GET)(uint32_t time);
typedef uint8_t(*FSM_QUEUE_SEND)(FSM_MSG msg);
typedef BaseType_t(*FSM_QUEUE_GET)(void *const msg, TickType_t time);


extern UART_SEM_SEND uart_sem_send[];//�ź����ͷ�����
extern UART_SEM_GET uart_sem_get[];//�ź�����ȡ����


//4G����[����]�ź���
BaseType_t c4g_sem_get(uint32_t time);
BaseType_t c4g_sem_send(void);

//ͨ��-����ռ�
MAIL* mail_apply(uint16_t size);
void  mail_release(MAIL* mail);

//ͨ��-��ʱ
uint8_t instant_queue_send(MAIL* mymail, uint8_t type);
BaseType_t instant_queue_get(void *const msg, TickType_t time);


//ͨ��-�㱨
BaseType_t report_queue_get(void *const msg, TickType_t time);
uint8_t report_queue_send(MAIL* mymail, uint8_t type);

//ͨ��[����]�����ź���
BaseType_t communication_sem_get(uint32_t time);
BaseType_t communication_sem_send(void);

//��־���ն���
uint8_t log_queue_send(UART0_DATA msg);
BaseType_t log_queue_get(void *const msg, TickType_t time);

//״̬������
uint8_t fsm_queue_send(FSM_MSG msg);
BaseType_t fsm_queue_get(void *const msg, TickType_t time);



void msg_init(void);

#endif

