#ifndef _MSG_H_
#define _MSG_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"

BaseType_t c4g_recv_get(uint32_t time);
BaseType_t c4g_recv_send_isr(void);


#endif

