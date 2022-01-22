#ifndef _TASK_SWIPE_H_
#define _TASK_SWIPE_H_

#include "stdint.h"

//卡系统消息类型
enum{
	MsgCard_GetCard,//找到卡
	MsgCard_LostCard,//丢失卡
};


void task_swipe1(void);
void task_swipe2(void);
void task_swipe3(void);
void task_swipe4(void);
void task_swipe5(void);
void task_swipe6(void);

#endif
