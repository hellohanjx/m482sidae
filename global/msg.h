#ifndef _MSG_H_
#define _MSG_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"

BaseType_t c4g_recv_get(uint32_t time);
BaseType_t c4g_recv_send_isr(void);


/**********************************************
@说明：通信系统
**********************************************/
/*
@说明：通信系统接收数据处理回调函数
*/
typedef uint8_t (*COM_RECV_CALLBACK)(uint8_t *recv, uint16_t recv_len,uint8_t *dat);

/*
@说明：通信系统消息结构
@成员：	com_call_back，是回调函数指针
		addr数据指针
*/
typedef struct MAIL
{
	 COM_RECV_CALLBACK com_call_back;
	 uint8_t *addr;
}MAIL;

BaseType_t c4g_recv_get(uint32_t time);
BaseType_t c4g_recv_send_isr(void);
MAIL* mail_apply(uint16_t size);
void  mail_release(MAIL* mail);
uint8_t report_to_tail(MAIL* mymail);
uint8_t report_to_head(MAIL* mymail);
uint8_t instant_to_tail(MAIL* mymail);
uint8_t instant_to_head(MAIL* mymail);
BaseType_t instant_data_get(void *const msg, TickType_t time);
BaseType_t commucation_sem_get(uint32_t time);
BaseType_t commucation_sem_send_isr(void);
BaseType_t report_data_get(void *const msg, TickType_t time);

void msg_init(void);

#endif

