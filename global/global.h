#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "FreeRTOSConfig.h"
#include "user_config.h"

#define ONE_SECOND	configTICK_RATE_HZ

/****************************************************************************************************************
@˵����ȫ�ֱ����ṹ
****************************************************************************************************************/
typedef struct CLASS_GLOBAL
{
	/****************
	@˵��������
	****************/
	struct net
	{
		char state;						//����״̬
		char number;					//ͨѶ��ˮ��
		char arr_ip[IP_LEN];	//ip��ַ
		char arr_ICCID[ICCID_LEN];//sim��ICCID
		char updata_flag;			//ip��ַ���˿ڸ��±�־
		uint8_t backlog_data;	//��ѹ����
		uint16_t password;		//��¼����
		uint16_t serverPort;	//�������˿ں�
		uint32_t id;					//��¼������
	}net;//����
	
}CLASS_GLOBAL;






/****************************
����
****************************/
extern volatile CLASS_GLOBAL class_global;

void restart_equ_set(uint8_t val, uint8_t type);

#endif
