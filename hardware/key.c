#include "key.h"
#include "msg.h"


enum{PRESS, UN_PRESS};
/*
@���ܣ�������ʼ��
*/
void _key_config(void)
{
	GPIO_SetMode(PA, BIT4, GPIO_MODE_INPUT);//button2
	GPIO_SetMode(PA, BIT2, GPIO_MODE_INPUT);//button3
	
	//������
	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1);//����ɲ���400us������
	GPIO_ENABLE_DEBOUNCE(PA, BIT2);//PA2
	GPIO_ENABLE_DEBOUNCE(PA, BIT4);//PA4
}

/*
@���ܣ���������
*/
void task_key(void)
{
	uint32_t time[2] = {0, 0};
	for(;;)
	{
		if(time[B_DOWN] != 0 && time[B_CHANGE] != 0 && xTaskGetTickCount() - time[B_DOWN] >= 5000 && xTaskGetTickCount() - time[B_CHANGE] >= 5000)//2����ͬʱ������5S
		{
			FSM_MSG msg;
			msg.id = B_Combination;
			msg.type = MsgKey;
			msg.stype = SEC_5;
			fsm_queue_send(msg);
		}
		else
		{
			if(BUTTON_CHANGE == PRESS)//�л���
			{
				if(!time[B_CHANGE])
				{
					time[B_CHANGE] = xTaskGetTickCount();//��¼����ʱ��
				}
			}
			else
			{
				if(time[B_CHANGE] != 0 && xTaskGetTickCount() - time[B_CHANGE] >= 1000 && xTaskGetTickCount() - time[B_CHANGE] < 5000)
				{
					FSM_MSG msg;
					msg.id = B_CHANGE;
					msg.type = MsgKey;
					msg.stype = SEC_1;
					fsm_queue_send(msg);
				}
				time[B_CHANGE] = 0;
			}

			if(BUTTON_DOWN == PRESS)//�л���
			{
				if(!time[B_DOWN])
				{
					time[B_DOWN] = xTaskGetTickCount();//��¼����ʱ��
				}
			}
			else
			{
				if(time[B_DOWN] != 0 && xTaskGetTickCount() - time[B_DOWN] >= 50 && xTaskGetTickCount() - time[B_DOWN] < 5000)
				{
					FSM_MSG msg;
					msg.id = B_DOWN;
					msg.type = MsgKey;
					msg.stype = MS_50;
					fsm_queue_send(msg);
				}
				time[B_DOWN] = 0;
			}
		}
		
		vTaskDelay(60);
	}
	
}
