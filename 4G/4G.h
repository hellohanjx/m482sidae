#ifndef _4G_H_
#define _4G_H_

#include "stdint.h"

//������Ϣ
typedef struct C4G_INFO{
	uint8_t rssi;	//MAX == 199,dBm
	uint8_t ber;	//MAX == 99,�ŵ�������
	char iccid[30]; //SIM��iccid

	char code[15];	//"READY",����ҪPIN�룻����...
	char n;			//״̬���Ƿ�ע��
	char stat;		//���ε���Ϣ
	char lac[5];	//������
	char ci[5];		//cell id
	char act;		//������ʽ

	char contextID[2];	//1~16
	char context_type;	//Э�����ͣ�1->IPV4
	char apn[15];		//apn
	char username[15];	//����
	char password[15];	//����
	char authentication;//��֤ 0~3
	char local_address[20];//����ip��ַ
	char connectID[2];	//socket�������� 0~11
	
	char context_state;	//�����״̬
	char ip_adress[20];	//�����ip��ַ
	char service_type[10];	//����ʽ��TCP��UDP��TCP LISTENER,TCP INCOMING,UDP SERVICE
	char remote_port[5];	//�˿ں�
	char local_port[15];	//�����˿ڣ�
	char socket_state;		//socket״̬��0~4
	char serverID;			//
	char access_mode;		//����ģʽ��0�����壬1�����ͣ�2��͸��
	char AT_port[10];		//atָ��˿�
	
	char err[10];			//������
	uint8_t Operators;			//��Ӫ�̣�1���ƶ���2��ͨ��3����
	
	char fsm;//״̬��
	
	uint32_t find_sim_timeout;//��sim���Ƿ���ڳ�ʱʱ�䣨����20s��������
	uint32_t chk_sim_reg_timeout;//��sim���Ƿ�ע�ᳬʱʱ�䣨����90s��������
	uint32_t chk_net_reg_timeout;//��ѯ�����Ƿ�ע�ᳬʱʱ�䣨����60s��������
	uint32_t lost_link_timeout;	//ʧȥ���ӵ�ʱ��
	uint32_t reboot_timeout;//������ʱ
}C4G_INFO;
	





void _4g_config(void);
void main_task_4g(void);
uint8_t get_4g_state(void);
uint8_t get_4g_quality(uint8_t* rssi, uint8_t* ber);

#endif
