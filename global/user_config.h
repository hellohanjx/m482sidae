#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


#define C_4G_LOG	0	//4G��־��1=�򿪣�0=�ر�














#define PACK_MAX_SIZE						255		//��ƽ̨ͨ��������ݳ���

#define ICCID_LEN								50		//SIM��ICCID����

#define IP_LEN									16		//ip��ַ����

#define PARAM_REDOWNED_CNT			3			//���������ظ����ش���

#define DEF_HEIM_IP							"116.62.120.86"//"IP.HMILK.CN"//"121.41.30.42"
#define DEF_HEIM_PORT						8001	//Ĭ�϶˿ں�



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
