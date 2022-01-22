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
@说明：状态机消息类型
**********************************************/
enum{	
	MsgCard,						//卡消息
	MsgCardServerAck,		//在线卡网络应答
	MsgServerAckErr,		//网络应答超时
	MsgKey,							//按键消息
};


/**********************************************
@说明：通信系统接收数据处理回调函数
@参数：*recv,接收数据缓冲指针
			 recv_len，接收数据长度
			 *send，发送数据缓冲指针
			 id，发送数据来源id
**********************************************/
typedef uint8_t (*COM_RECV_CALLBACK)(uint8_t *recv, uint16_t recv_len, uint8_t *send, uint8_t id);

/******************************************
@说明：通信系统消息结构
******************************************/
typedef struct MAIL
{
	COM_RECV_CALLBACK com_call_back;//回调函数指针
	uint8_t *addr;//数据指针
	uint8_t id;//数据来源识别
}MAIL;



/******************************************
@说明：状态机消息结构
******************************************/
typedef struct FSM_MSG
{
	uint8_t   id;//消息id，对应 1~6 刷卡头【取值 0~5】
	uint8_t   type; 	//消息类型。扣款，查询
	uint8_t		stype;	//消息子类型
	uint32_t  value; 	//消息的值
}FSM_MSG;


/*
@函数指针
*/
typedef BaseType_t(*UART_SEM_SEND)(void);//信号量发送
typedef BaseType_t(*UART_SEM_GET)(uint32_t time);
typedef uint8_t(*FSM_QUEUE_SEND)(FSM_MSG msg);
typedef BaseType_t(*FSM_QUEUE_GET)(void *const msg, TickType_t time);


extern UART_SEM_SEND uart_sem_send[];//信号量释放数组
extern UART_SEM_GET uart_sem_get[];//信号量获取数组


//4G接收[串口]信号量
BaseType_t c4g_sem_get(uint32_t time);
BaseType_t c4g_sem_send(void);

//通信-申请空间
MAIL* mail_apply(uint16_t size);
void  mail_release(MAIL* mail);

//通信-即时
uint8_t instant_queue_send(MAIL* mymail, uint8_t type);
BaseType_t instant_queue_get(void *const msg, TickType_t time);


//通信-汇报
BaseType_t report_queue_get(void *const msg, TickType_t time);
uint8_t report_queue_send(MAIL* mymail, uint8_t type);

//通信[串口]接收信号量
BaseType_t communication_sem_get(uint32_t time);
BaseType_t communication_sem_send(void);

//日志接收队列
uint8_t log_queue_send(UART0_DATA msg);
BaseType_t log_queue_get(void *const msg, TickType_t time);

//状态机队列
uint8_t fsm_queue_send(FSM_MSG msg);
BaseType_t fsm_queue_get(void *const msg, TickType_t time);



void msg_init(void);

#endif

