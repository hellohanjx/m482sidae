#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


#define C_4G_LOG				0		//4G��־��1=�򿪣�0=�ر�
#define GPIO_INT_LOG		0		//gpio�ж���־
#define	RC_522_LOG			0		//rc522 ��־
#define WATCH_DOG				1		//���Ź���1=����0=��
#define CPU_INFO				0

#define IREADER_NUM_MAX		6//ˢ�����������

#define IP_LEN									16	//ip��ַ����
#define	CARD_PHY_LEN						34	//�����ų���(�ַ���)
#define CARD_LOGIC_LEN					34	//�߼����ų���(�ַ���)
#define ICCID_LEN								50	//SIM��ICCID����
#define PACK_MAX_SIZE						255	//��ƽ̨ͨ��������ݳ���
#define PARAM_REDOWNED_CNT			3		//���������ظ����ش���

#define DEF_HEIM_IP							"116.62.120.86"//"IP.HMILK.CN"//"121.41.30.42"
#define	DEF_CARD_INTERFACE			1001		//ˢ�����ӿ�

/*
@˵����flash�洢������Ĭ��ֵ
*/
//#define DEF_FLASH_Ip						(116 << 24) | (62 << 16) | (120 << 8) | 86	//Ĭ��IP��ַ,��4��ip��ƴ�����ģ���~�ң����� 116.62.120.86
//#define DEF_FLASH_Id						1000000000	//�������(10λ)
//#define DEF_FLASH_Port					8001				//Ĭ�϶˿ں�
#define DEF_FLASH_PassWord			000000			//��������(6λ)
#define DEF_FLASH_FactoryEn			0						//Ĭ�Ϲ���ģʽ-�Զ����ԣ�0�û�ģʽ��;1:����ģʽ
#define DEF_FLASH_PointBit			2						//����С����λ��
#define DEF_FLASH_PriceBit			4						//���λ��(������С����)
#define DEF_FLASH_CardType			1						//1=��ݮˢ������2=�⹺ˢ����
#define DEF_FLASH_PriceMax			10000				//���۸�



#define DEF_FLASH_Ip						(121 << 24) | (43 << 16) | (255 << 8) | 207	//Ĭ��IP��ַ,��4��ip��ƴ�����ģ���~�ң����� 116.62.120.86
#define DEF_FLASH_Port					5008				//Ĭ�϶˿ں�
#define DEF_FLASH_Id						1059000000	//�������(10λ)






/*********************************************
@˵�����ȸ�λ����
*********************************************/
enum {
	//��ʱ�㱨
	RESET_Init,						//0 = ��ʼ��
	RESET_UpdataTime,			//1 = ��ʱ
	RESET_NormalLink, 		//2 = ���ؽ�socket��������
	RESET_RebuitSocket,		//3 = �ؽ�socket
	RESET_4G,							//4 = ����4G
	
	Reserved_1,//����λ5
	Reserved_2,//����λ6
	Reserved_3,//����λ7
	Reserved_4,//����λ8
	Reserved_5,//����λ9
	
	//�����㱨
	RESET_NormalReset,		//10 = ��������
	RESET_PowerOn,				//11 = �ϵ縴λ
	RESET_Server, 				//12 = ��������������
	RESET_Param, 					//13 = ������������
	RESET_Distribution, 	//14 = �»�������id�Ȳ�������
	RESET_Setting,				//15 = ���ø�λ����
	RESET_UpdataIP,				//16 = ����IP��ַ��˿ں�
	RESET_HardFault, 			//17 = ����
	RESET_MemManage, 			//18 = ����
	RESET_BusFault, 			//19 = ����
	RESET_UsageFault, 		//20 = ����
	RESET_WatchDog, 			//21 = ���Ź���λ
	RESET_ApplyMail,			//22 = �����ڴ�ʧ��
	RESET_DriverCmd,			//23 = �����ڴ�ʧ��
	RESET_MsgApply,				//24 = �����ڴ�ʧ��
	RESET_CardCmd,				//25 = �����ڴ�ʧ��
	RESET_MailToBus,			//26 = �����ڴ�ʧ��
	RESET_LANCmd,					//27 = �����ڴ�ʧ��
	RESET_FACECmd,				//28 = �����ڴ�ʧ��
};

#endif
