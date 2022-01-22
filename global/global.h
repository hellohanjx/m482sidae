#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "FreeRTOSConfig.h"
#include "user_config.h"

#define ONE_SECOND	configTICK_RATE_HZ

extern volatile char main_version[];//�汾��

//���ط�ʽ
enum SWITCH_MODE{SWITCH_OFF, SWITCH_ON};

//����ģʽ
enum FACTOTY_MODE{USER_MODE, FACTORY_COLD, FACTORY_HOT, FACTORY_CLOSE_TEMP, FACTORY_CLOSE_ALL, FACTORY_AUTO, FACTORY_AUTO_COLD, FACTORY_AUTO_HOT};

//ˢ��������״̬
enum CARD_WORK_STATE{IDLE, BUSY, WORKING};


/****************************************************************************************************************
@˵����ȫ�ֱ����ṹ
****************************************************************************************************************/
typedef struct CLASS_GLOBAL
{
	/****************
	@˵����ϵͳ������
	****************/
	struct sys 
	{
		uint8_t point_bit;	//С����λ����������������,�������볤�ȣ��Լ�������볤�ȣ�
		uint8_t price_bit;//�۸�λ��
		enum FACTOTY_MODE factory_en;	//1~5����ģʽ��0Ϊ�û�ģʽ
		enum SWITCH_MODE reset;//������־
		uint32_t unique_id[3];//оƬΨһid
	}sys;
	
	/****************
	@˵����������
	****************/
	struct trade //�������
	{
		uint8_t type; //�������ͣ�0�ֽ� 1 IC�� 2 �ֻ�֧��
		uint8_t fsm[6];//֧������״̬��
		uint32_t number;//���׺�
		uint32_t min_balance;//����С���
		uint32_t plus_per_1L[6];//1Lˮ�����������6��ˢ����
		uint32_t price_per_1L[6];//1Lˮ���۸����6��ˢ����
	}trade;
	
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
	
	
	
	/****************
	@˵�����¶�
	****************/
	struct{
		//cpu�¶�
		struct{
			int val;
			uint8_t state;
		}internal;
		
		//�ⲿntc
		struct{
			int val;
			uint8_t state;
		}external;
	}temp;
	
	/****************
	@˵������ϵͳ��������������ˮ�����ź�
	****************/
	struct ireader
	{
		struct{//�����豸
			uint8_t 		state;			//ˢ����״̬
			uint8_t 		type;				//ˢ��������
			enum CARD_WORK_STATE work;//����״̬
			uint32_t 		interface;	//ˢ���ӿ�
		}equ;
		
		struct{//��ʾ
			uint32_t state;//0=����Ҫ��ʾ��1=��Ҫ��ʾ
			uint32_t start_time;//��¼��ʼ��ʾʱ��
			uint32_t hold_time;//��¼��ʾʱ��
		}display;
		
		struct{//��Ƭ
			uint8_t user;	//�û����ʶ��(1����Ա)
			uint8_t useRang;//ʹ�÷�Χ
			
			char   physic_char[CARD_PHY_LEN];		//������-�ַ���
			char   logic_char[CARD_LOGIC_LEN];	//�߼�����-�ַ���
			uint8_t   physic_hex[4];    //������hex
			uint8_t		logic_hex[4];			//�߼�����hex
			
			uint8_t   type;					//������
			uint8_t   subsype;			//��������
			uint8_t   status; 		 	//��״̬
			uint8_t		daymaxpay;		//����������Ѵ���
			uint32_t 	balance;			//�����
			uint32_t 	trade_num;		//���׺�
		}card;
		
		struct{
			uint8_t state;//ˮ���豸״̬����/��
		}tap;
		
		struct{
			uint8_t state;//����״̬
			uint32_t val;//����ֵ
		}count[2];//2���ź��ߣ����źż��������źż���
		
		struct{
			char physic_char[CARD_PHY_LEN];	//������-�ַ���
			uint32_t trade_num;						//���׺�
			uint32_t time;								//��ʱ
		}working;//��������
		
	}ireader[6];

	
}CLASS_GLOBAL;




/****************************
����
****************************/
extern volatile CLASS_GLOBAL class_global;
extern volatile char main_version[];

void restart_equ_set(uint8_t val, uint8_t type);
uint8_t restart_equ_get(void);

uint32_t get_random_number(void);
void global_init(void);

#endif
