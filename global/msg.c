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

