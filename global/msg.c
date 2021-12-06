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

