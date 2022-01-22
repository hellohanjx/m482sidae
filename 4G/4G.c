/*
��ԶEC20��4g����
ֻ��TCP/IP�����������������Ͷ���
@˵����4G������ָʾ�ƣ�
��ƣ�����������΢�����ػ�
�Ƶƣ�������Ϩ��ʱ�䳤��,����״̬������������ʱ�䳤�����������������ݴ���ģʽ
���ƣ�����ע��LTE����
@˵��������״̬���ܹ���ʵ��
@ע�⣺ÿ��ָ��س��������⻹�и�"OK"
*/

#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "task.h"
#include "sys.h"
#include "4G.h"
#include "uart7_config.h"
#include "msg.h"
#include "string.h"
#include "global.h"
#include "stdlib.h"
#include "user_config.h"


#define C4G_PWR		PA15	//��/�ػ�
#define C4G_POWERON_TIME	300	//�������ű���ʱ�䣨ms��
#define C4G_POWEROFF_TIME	800	//�ػ����ű���ʱ�䣨ms��
#define C4G_RESET_TIME		500	//�ػ����������ʱ�䣨ms��

//ATָ��
  char chk_baud[] = {"AT+IPR?\r\n"};				//��ѯ������
  char set_baud[] = {"AT+IPR=115200\r\n"};		//���ò�����	
  char close_cmd_echo[] = {"ATE0\r\n"};			//�ر��������
  char save_param[] = {"AT&W\r\n"};				//�������ò�����Ӧ��ʱ300ms
  char chk_iccid[] = {"AT+QCCID\r\n"};			//��ѯccid

  char chk_pin[] = {"AT+CPIN?\r\n"};				//��ѯsim������(5s��ʱ)
static const char chk_pin_ack1[] = {"+CME ERROR: 10"};	//sim��δ�忨
static const char chk_pin_ack2[] = {"+CPIN: READY"};		//sim��׼����

  char chk_sim_reg[] = {"AT+CREG?\r\n"};			//��ѯsim���Ƿ�ע��[@��ѯ��������ע��״̬]
  char chk_net_reg[] = {"AT+CGREG?\r\n"};		//��ѯnetע��[@��ѯ��������ע��״̬]
 
 //��Ӫ��APN
  char set_link_ChinaMobile[] = {"AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n"};//���ý����(�ƶ�) APN,USERNAME,PASSWORD
  char set_link_UNICOM[] = {"AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r\n"};//���ý����(��ͨ) APN,USERNAME,PASSWORD
  char set_link_p[] = {"AT+QICSGP=1,1,\"CTLTE\",\"\",\"\",1\r\n"};//���ý����(����) APN,USERNAME,PASSWORD
 
  char chk_link_point[] = {"AT+QICSGP=1\r\n"};	//��ѯ����� 
  char active_pdp_context[] = {"AT+QIACT=1\r\n"};//��������(150s��ʱ)[@����PDP����]
  char chk_pdp_context[] = {"AT+QIACT?\r\n"};	//��ѯcontext,���ص�ǰ�������Ϣ(150s��ʱ)
  char deactive_pdp_context[] = {"AT+QIDEACT=1\r\n"};//�رս���㣨40s��ʱ��
// const char creat_tcp[] = {"AT+QIOPEN=1,0,\"TCP\",\"120.55.115.113\",5008,0,2\r\n"};//����TCP���ӣ�150s��
//  const char creat_tcp[] = {"AT+QIOPEN=1,0,\"TCP\",\"120.26.204.86\",8001,0,2\r\n"};//����TCP���ӣ�150s��
 
 char test_tcp[] = {"AT+QIOPEN?\r\n"};		//����TCP��������
 char creat_tcp_top[] = {"AT+QIOPEN=1,0,\"TCP\",\""};
 char creat_tcp_tail[] = {"\","};//����TCP���ӣ�150s��
 char creat_tcp_port[] = {",0,2\r\n"};//����˶˿ں�+����ģʽ��͸��ģʽ��
static const char creat_tcp_ack[] = {"CONNECT"};//����TCP�ɹ�
	
  char close_tcp[] = {"AT+QICLOSE=0\r\n"};		//�ر�TCP���ӣ���������Լ����ó�ʱ��
  char chk_tcp[] = {"AT+QISTATE=1,0\r\n"};		//��ѯTCP����
  char chk_data_echo[] = {"AT+QISDE?\r\n"};		//��ѯ���ݻ���
// char close_data_echo[] = {"AT+QISDE=0\r\n"};	//�ر����ݻ��ԣ�͸��ģʽ����Ҫ��
  char chk_err_code[] = {"AT+QIGETERROR\r\n"};	//��ѯ����Ĵ�����

  char exit_transpartent[] = {"+++"};		//�˳�͸��ģʽ
  char exit_transpartent2[] = {"++++++++\r\n"};		//����ģʽ�µ�+++����

  char change_transparent_mode[] = {"AT+QISWTMD=1,2\r\n"};		//��Ϊ͸��ģʽ
  char enter_transparent_mode[] = {"ATO\r\n"};		//����͸��ģʽ

static const char common_ack[] = {"OK"};	//ͨ�ûظ�"OK"

   char chk_Operators[] = {"AT+COPS?\r\n"};//����ʲô�����ƶ���ͨ����
static const char Operators_ChinaMobile_ack[] = {"CHINA MOBILE"};	//�й��ƶ�
static const char Operators_ChinaUnicom_ack[] = {"CHN-UNICOM"};		//�й���ͨ
static const char Operators_ChinaTelecom_ack[] = {"CHN-CT"};			//�й�����
//	 char Operators_ChinaMobile_ack[] = {"china mobile"};//�й��ƶ�
//	 char Operators_ChinaUnicom_ack[] = {"chn-unicom"};//�й���ͨ
//	 char Operators_ChinaTelecom_ack[] = {"chn-ct"};//�й�����

 char chk_ICCID[] = {"AT+QCCID\r\n"};//��ѯsim��ICCID	
static const char chk_ICCID_ack[] = {"+QCCID:"};//ICCID����
		
 char chk_signal[] = {"AT+CSQ\r\n"};//��ѯ�ź�ǿ��
static const char chk_signal_ack[] = {"+CSQ:"};//��ѯ�ź�ǿ�ȷ���

//��Ӫ��ICCIDǰ6λʶ���
#define OPERATORS_LEN		6//��Ӫ��ʶ���볤��
#define CHINA_MOBILE_ID_NUM			4		//ʶ�������
#define CHINA_UINCOM_ID_NUM			3		//ʶ�������
#define CHINA_TELECOM_ID_NUM		2		//ʶ�������
enum{ CHINA_MOBILE = 1, CHINA_UNICOM, CHINA_TELECOM };//��Ӫ��

 char Operators_ChinaMobile_ID[CHINA_MOBILE_ID_NUM][OPERATORS_LEN] = {"898600", "898602", "898604", "898607"};//�й��ƶ�
 char Operators_ChinaUnicom_ID[CHINA_UINCOM_ID_NUM][OPERATORS_LEN] = {"898601", "898606", "898609"};//�й���ͨ
 char Operators_ChinaTelecom_ID[CHINA_TELECOM_ID_NUM][OPERATORS_LEN] = {"898603", "898611"};//�й�����

/*
@˵�����ź�ǿ�Ȳ���
*/
enum{SIGNAL_NO = 99, TDSCDMA_SIGNAL_NO = 199};//���ź�
enum{SIGNAL_LOW = 10, TDSCDMA_SIGNAL_LOW = 130};//���ź�0~10
enum{SIGNAL_MID = 20, TDSCDMA_SIGNAL_MID = 160};//�е��ź�11~20,���źţ�21~31


/*
@˵����״̬��״̬
*/
enum
{
	ST_PWR_ON = 0,		//�ϵ�
	ST_CHK_SIM = 1,		//��ѯsim��
	ST_CHK_SIM_REG = 2,		//��ѯsim������
	ST_CHK_NET_REG = 3,		//��ѯ����
	ST_SET_POINT = 4,	//���ý����
	ST_CHK_POINT = 5,	//��ѯ�����
	ST_ACTIVE_POINT = 6,//��������
	ST_DEACTIVE_POINT = 7,//�رս����
	ST_CHK_CONTEXT = 8,	//��ѯ������ı�
	ST_CREAT_TCP = 9,	//����TCP����
	ST_CLOSE_TCP = 10,	//�ر�TCP
	ST_CHK_TCP = 11,	//��ѯTCP
	ST_CHK_DATA_ECHO = 12,//��ѯ���ݻ���
	ST_CLOSE_DATA_ECHO = 13,//�ر����ݻ���
	ST_CHK_ERRCODE = 14,	//��ѯ����Ĵ�����
	ST_EXIT_TRANSPARTENT = 15,//�˳�͸��ģʽ
	ST_CLOSE_CMD_ECHO = 16,	//�ر��������
	ST_REBOOT = 17,		//����4G
	ST_CHANGE_TRANSPARENT = 18,	//��Ϊ͸��ģʽ
	ST_ENTER_TRANSPARENT = 19, //����͸��ģʽ
	ST_CHK_OPERATER = 20,	//��ѯ������
	ST_CHK_ICCID = 21,		//��ѯSIMK��ICCID
	ST_CHK_QUALITY = 22,//��ѯ�ź�ǿ��
};

//����4g���ݽṹ��
 C4G_INFO c4g_info;

/*
@���ܣ�EC20�������ų�ʼ��--PA15
@ע�⣺EC20 RST����δʹ��(����)
*/
void _4g_config(void)
{
	GPIO_SetMode(PA, BIT15, GPIO_MODE_OUTPUT);//PA15�������
	PA15 = 0;
}

/*
@���ܣ�4g���ݷ��ش���ص�����
@������rx����������ָ��
@����ֵ��ִ�н��
*/
static uint8_t callback_4g_recv(UART7_DATA *rx)
{
	if(rx->len != 0)
	{
		if(rx->buf[0] == 0x0D && rx->buf[1] == 0x0A)
			c4g_sem_send();
	}
	return TRUE;
}


/*
@���ܣ�����
*/
static void c4g_power_on(void)
{
	C4G_PWR = 1;	//����
	vTaskDelay(C4G_POWERON_TIME);
	C4G_PWR = 0;	//����
}

/*
@���ܣ��ػ�
*/
static void c4g_power_off(void)
{
	C4G_PWR = 1;	//����
	vTaskDelay(C4G_POWEROFF_TIME);
	C4G_PWR = 0;	//����
	#if C_4G_LOG
	printf("4g start power down\r\n");
	#endif
	vTaskDelay(39*ONE_SECOND);//�ػ����ʱ35s
}

/*
@���ܣ���λ
*/
static void c4g_power_reset(void)
{
	c4g_power_off();
	vTaskDelay(C4G_RESET_TIME);
	c4g_power_on();
	vTaskDelay(3000);
}

/*
@���ܣ��ر��������
@����ֵ��ִ��������Ƿ���Խ�����һ��
*/
static uint8_t c4g_close_cmd_echo(void)
{
	UART7_DATA* rx;
	uint8_t err;
	
	_uart7_send(&rx, (uint8_t*)close_cmd_echo, (sizeof(close_cmd_echo) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(300);//300ms��ʱ
	if(err == pdTRUE)
	{
		//���ء�OK��
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�����3���ַ���ʼ�Ƚ�,
		{
			return TRUE;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: close echo\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ���ѯsim��
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
*/
static uint8_t c4g_chk_sim(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_pin, (sizeof(chk_pin) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(5000);//�ȴ���ʱ 5s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), chk_pin_ack2, sizeof(chk_pin_ack2) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//����Ϊ "+CPIN: READY "��ʾsim��׼������
		}

		if( !strncasecmp((char*)&(rx->buf[2]), chk_pin_ack1, sizeof(chk_pin_ack1) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�
		{
			#if C_4G_LOG
			printf("4g_err: no sim\r\n\r\n");
			#endif
			return 2;	//δ�忨
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check sim\r\n\r\n");
	#endif
	return FALSE;
}


/*
@���ܣ���ѯsim��ע��
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_sim_reg(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_sim_reg, (sizeof(chk_sim_reg) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�����3���ַ���ʼ�Ƚ�,
		{
			//�����������
			uint8_t i, j;
			i = 9;
			c4g_info.n = rx->buf[i++];//����״̬
			++i;//����','
			c4g_info.stat = rx->buf[i++];//������Ϣ��1����ʾ��������
			if(c4g_info.n != '0')//�������δ����Ľ������ᴫ��һЩ��Ϣ
			{
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
					c4g_info.lac[j] = rx->buf[i];//��ַ����
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
					c4g_info.ci[j] = rx->buf[i];//cell id
				++i;
				c4g_info.act = rx->buf[i];//������ʽ��gsm/gprs...
			}
			
			return TRUE;//sim��ע��ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: sim register\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: sim register\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ���ѯ����ע��״̬
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_net_reg(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_net_reg, (sizeof(chk_net_reg) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			//�����������
			uint8_t i, j;
			i = 10;
			c4g_info.n = rx->buf[i++];//����״̬
			++i;//����','
			c4g_info.stat = rx->buf[i++];//������Ϣ;1,��ʾ��������5����ʾ����
			if(c4g_info.n != '0')
			{
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.lac[j] = rx->buf[i];//��ַ����
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.ci[j] = rx->buf[i];//cell id
				++i;
				c4g_info.act = rx->buf[i];//������ʽ��gsm/gprs...
			}
			
			return TRUE;//����ע��ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: net register\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: net register\r\n\r\n");
	#endif
	return FALSE;
}


/*
@���ܣ���ѯ��������
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_chk_point(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_link_point, (sizeof(chk_link_point) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�Ƚ�����"OK"
		{
			//�����������
			uint8_t i, j;
			for(i = 2;  rx->buf[i] != ',' && i < 20; i++)
				c4g_info.context_type = rx->buf[i];//��ַIPv4
			i += 2;//����','
			for(j = 0;  rx->buf[i] != ',' && i < 20; i++, j++)
				c4g_info.apn[j] = rx->buf[i];//apn
			i += 2;
			for(j = 0;  rx->buf[i] != ',' && i < 20; i++, j++)
				c4g_info.username[j] = rx->buf[i];//�û�����
			i += 2;
			for(j = 0;  rx->buf[i] != ',' && i < 20; i++, j++)
				c4g_info.password[j] = rx->buf[i];//����
			i += 2;
			for(; i != ',' && i < 20; i++)
				c4g_info.authentication = rx->buf[i];//��֤0~3
			
			return TRUE;//��ѯ�ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: check point\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check point\r\n\r\n");
	#endif
	return FALSE;
}


/*
@���ܣ�������������
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_set_point(void)
{
	UART7_DATA* rx;
	uint8_t err;

	if(c4g_info.Operators == CHINA_MOBILE)
		_uart7_send(&rx, (uint8_t*)set_link_ChinaMobile, (sizeof(set_link_ChinaMobile) - 1), callback_4g_recv);//���ڷ���
	else 
	if(c4g_info.Operators == CHINA_UNICOM)
		_uart7_send(&rx, (uint8_t*)set_link_UNICOM, (sizeof(set_link_UNICOM) - 1), callback_4g_recv);//���ڷ���
	else 
	if(c4g_info.Operators == CHINA_TELECOM)
		_uart7_send(&rx, (uint8_t*)set_link_p, (sizeof(set_link_p) - 1), callback_4g_recv);//���ڷ���
	else
		return FALSE;
	
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//���ý����ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: set point\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: set point\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ���ѯ��������
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_chk_context(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_pdp_context, (sizeof(chk_pdp_context) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(15000);//�ȴ���ʱ15s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			//�����������
			uint8_t i, j;
			i = 12;
			c4g_info.context_state = rx->buf[i++];//�����״̬
			++i;//����','
			c4g_info.context_type = rx->buf[i++];//����Э������
			++i;
			for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
				c4g_info.local_address[j] = rx->buf[i];//����ip��ַ
			
			if(c4g_info.context_state == '1')//�Ѿ��������Ҫ�ٴμ���
				return 3;
			return TRUE;//��ѯ�ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: check context\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check context\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ�������������
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_active_context(void)
{
	#if C_4G_LOG
	uint32_t cost_time = 0;
	#endif
	
	UART7_DATA* rx;
	uint8_t err;

	#if C_4G_LOG
	cost_time = xTaskGetTickCount();
	#endif
	_uart7_send(&rx, (uint8_t*)active_pdp_context, (sizeof(active_pdp_context) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(15000);//�ȴ���ʱ15s
	if(err == pdTRUE)
	{
		#if C_4G_LOG
		char tmp[10] = {0,0,0,0,0,0,0,0,0,0};
		sprintf(tmp, "%u", xTaskGetTickCount() - cost_time);
		printf("c4g_active_context_time = ");
		printf(tmp);
		printf("\r\n\r\n");
		#endif
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//����ɹ�
		}
		else
		if(!strncasecmp((char*)&(rx->buf[2]), common_ack, sizeof(common_ack) - 1) )
		{
			return TRUE;//@�°��4G�������������"+QDMURC"���ַ���
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: active context\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: active context\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ��ر���������
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_deactive_context(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)deactive_pdp_context, (sizeof(deactive_pdp_context) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(40000);//�ȴ���ʱ40s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//����ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: deactive context\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: deactive context\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ�����TCP����
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
@˵�����ɹ������͸��ģʽ���˳���������
*/
static uint8_t c4g_creat_tcp(void)
{
	#if C_4G_LOG
	uint32_t cost_time = 0;
	#endif
	
	UART7_DATA* rx;
	uint8_t err,i;
	char creat_tcp[55];
	char port[6];
	
	for(i = 0; i < 55; i++){
		creat_tcp[i] = 0;
	}
	for(i = 0; i < 6; i++){
		port[i] = 0;
	}	
	sprintf(port, "%u", class_global.net.serverPort);//�˿ں�ת��Ϊ�ַ���
	strcpy(creat_tcp, creat_tcp_top);//�ַ�������
	strcat(creat_tcp, (const char*)class_global.net.arr_ip);
//	strcat(creat_tcp, (const char*)"ip.hmvend.cn");
	strcat(creat_tcp, creat_tcp_tail);
	strcat(creat_tcp, (const char*)port);
	strcat(creat_tcp, creat_tcp_port);
	
	#if C_4G_LOG
	cost_time = xTaskGetTickCount();
	#endif
	_uart7_send(&rx, (uint8_t*)creat_tcp, strlen(creat_tcp), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(153000);//�ȴ���ʱ153s
	if(err == pdTRUE)
	{
		#if C_4G_LOG
		char tmp[10] = {0,0,0,0,0,0,0,0,0,0};
		sprintf(tmp, "%u", xTaskGetTickCount() - cost_time);
		printf("c4g_creat_tcp_time = ");
		printf(tmp);
		printf("\r\n\r\n");
		#endif
		
		if( !strncasecmp((char*)&(rx->buf[2]), creat_tcp_ack, sizeof(creat_tcp_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//����ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: create TCP\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: create TCP\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ��ر�TCP����
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
@˵�����ɹ������͸��ģʽ���˳���������
*/
static uint8_t c4g_close_tcp(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)close_tcp, (sizeof(close_tcp) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(10000);//�ȴ���ʱ10s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//�رճɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: close TCP\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: close TCP\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ�TCP������������
@˵����"active_pdp_context"ָ���ֱ�ӷ�"creat_tcp"ָ���ʱ"creat_tcp"ָ��ķ��ػ��һ�����ֽ�(0x0D)���м�����һ��ָ��Ͳ�����������⣨@@@@�����趫��������֪�ǲ���������˻������⣩
@�������ڣ�2019.8.10
*/
static uint8_t c4g_test_tcp(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)test_tcp, (sizeof(test_tcp) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(300);//�ȴ���ʱ300ms
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�
		{
			return TRUE;
		}
		#if C_4G_LOG
		printf("4g_err: test TCP\r\n\r\n");
		#endif
	}
	#if C_4G_LOG
	printf("4g_no_ack: test TCP\r\n\r\n");
	#endif
	return FALSE;
}
/*
@���ܣ���ѯTCP����״̬
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
*/
static uint8_t c4g_chk_tcp(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)chk_tcp, (sizeof(chk_tcp) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(15000);//�ȴ���ʱ15s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			if(rx->len > 6)//���TCP���Ӵ��ڣ��᷵�غܶ�����
			{
				//�����������
				uint8_t i, j;
				i = 12;
				for(j = 0;  rx->buf[i] != ',' && j < 2; i++, j++)
					c4g_info.connectID[j] = rx->buf[i];//1~16
				++i;//����','
				for(j = 0;  rx->buf[i] != ',' && j < 10; i++, j++)
					c4g_info.service_type[j] = rx->buf[i];//���ӷ�ʽ��TCP/UDP...
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 20; i++, j++)
					c4g_info.ip_adress[j] = rx->buf[i];//����˵�ַ
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.remote_port[j] = rx->buf[i];//����˶˿ں�
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 5; i++, j++)
					c4g_info.local_port[j] = rx->buf[i];//�����˿ں�
				++i;
				c4g_info.socket_state = rx->buf[i++];//socket״̬
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 2; i++, j++)
					c4g_info.contextID[j] = rx->buf[i];//1~16
				++i;
				c4g_info.serverID = rx->buf[i++];//
				++i;
				c4g_info.access_mode = rx->buf[i++];//����ģʽ��1.���壬2.���ͣ�3.͸��
				++i;
				for(j = 0;  rx->buf[i] != ',' && j < 10; i++, j++)
					c4g_info.AT_port[j] = rx->buf[i];//����˿ڣ�UART1/USB
				
				if(c4g_info.socket_state == '4')//���ڹر�socket
					return 4;
				else
				if(c4g_info.socket_state == '1' 
				|| c4g_info.socket_state == '2'
				|| c4g_info.socket_state == '3')//socket����
					return 3;
			}
			
			return TRUE;//��ѯ�ɹ�,������Ҫ�������� TCP
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: check TCP\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: check TCP\r\n\r\n");
	#endif
	return FALSE;
}


/*
@���ܣ��˳�͸��ģʽ
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
@˵�����ɹ������͸��ģʽ���˳���������
*/
static uint8_t c4g_exit_transpartent(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)exit_transpartent, (sizeof(exit_transpartent) -1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(1000);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), common_ack, sizeof(common_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//����ɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: exit transpartent_1\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: exit transpartent_1\r\n\r\n");
	#endif
	return FALSE;
}

//�����ʵ������㷢һ���ַ��������������ʽ����ֻҪ�лظ���˵�����ڡ�����ģʽ��
static uint8_t c4g_exit_transpartent2(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)exit_transpartent2, (sizeof(exit_transpartent2) -1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(1000);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		return TRUE;//����ɹ�
	}
	#if C_4G_LOG
	printf("4g_no_ack: exit transpartent_2\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ�����ģʽ����͸��ģʽ
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
@˵�����ɹ������͸��ģʽ���˳���������
*/
static uint8_t c4g_enter_transparent(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)enter_transparent_mode, (sizeof(enter_transparent_mode) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(10000);//�ȴ���ʱ10s
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), creat_tcp_ack, sizeof(creat_tcp_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//�رճɹ�
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: enter transpartent\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: enter transpartent\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ���Ϊ͸��ģʽ
@����ֵ��ִ��������Ƿ���Խ�����һ��
@ע�⣺����ֵ��ǰ2���ֽ���"/r/n",���ԱȽ��ַ����Ļ���Ҫ�ӷ���ֵ��3���ֽڿ�ʼ
		sizeof(�����ַ���) == �ַ�������+'0'����������������ȽϵĻ���Ҫ��ȥ'0'������
		����ֵ"OK"�ڵ�����4���ֽڿ�ʼ
@˵�����ɹ������͸��ģʽ���˳���������
*/
static uint8_t c4g_change_transpartent(void)
{
	UART7_DATA* rx;
	uint8_t err;

	_uart7_send(&rx, (uint8_t*)change_transparent_mode, (sizeof(change_transparent_mode) - 1), callback_4g_recv);//���ڷ���
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), creat_tcp_ack, sizeof(creat_tcp_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			return TRUE;//�ѽ���͸��ģʽ
		}
		else
		{
			//����������
			#if C_4G_LOG
			printf("4g_err: change transpartent\r\n\r\n");
			#endif
			return 2;
		}
	}
	#if C_4G_LOG
	printf("4g_no_ack: change transpartent\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ���ѯ������
@����ֵ��ִ��������Ƿ���Խ�����һ��
*/
static uint8_t c4g_check_Operators(void)
{
	uint8_t i;
	
	//��һ��������ICCID�ж���Ӫ��
	for(i = 0; i < CHINA_MOBILE_ID_NUM; i++)//�ƶ�
	{
		if( !strncasecmp((char*)(class_global.net.arr_ICCID), &(Operators_ChinaMobile_ID[i][0]), 6 ) )
		{
			c4g_info.Operators = CHINA_MOBILE;
			return TRUE;
		}
	}
	for(i = 0; i < CHINA_UINCOM_ID_NUM; i++)//��ͨ
	{
		if( !strncasecmp((char*)(class_global.net.arr_ICCID), &(Operators_ChinaUnicom_ID[i][0]), 6 ) )
		{
			c4g_info.Operators = CHINA_UNICOM;
			return TRUE;
		}
	}
	for(i = 0; i < CHINA_TELECOM_ID_NUM; i++)//����
	{
		if( !strncasecmp((char*)(class_global.net.arr_ICCID), &(Operators_ChinaTelecom_ID[i][0]), 6 ) )
		{
			c4g_info.Operators = CHINA_TELECOM;
			return TRUE;
		}
	}
	
	//��һ�����ɹ������еڶ���������Ӧ���ж���Ӫ��
	{
		UART7_DATA* rx;
		uint8_t err;
		
		_uart7_send(&rx, (uint8_t*)chk_Operators, (sizeof(chk_Operators) - 1), callback_4g_recv);//���ڷ���
		err = c4g_sem_get(3000);//1.8s��ʱ
		if(err == pdTRUE)
		{
			if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�Ƚ�����"OK"
			{
	//			printf((char*)&rx->buf[14]);
	//			printf("\r\n\r\n\r\n");
				//�ӵ�14�ֽڿ�ʼ�Ƚ���Ӫ������
	//			strupr(&rx->buf[rx->len-4]);
				if( !strncasecmp((char*)&(rx->buf[14]), Operators_ChinaMobile_ack, sizeof(Operators_ChinaMobile_ack) - 1) )//���й��ƶ��Ƚ�
				{
					c4g_info.Operators = CHINA_MOBILE;//�ƶ�
					return TRUE;
				}
				if( !strncasecmp((char*)&(rx->buf[14]), Operators_ChinaUnicom_ack, sizeof(Operators_ChinaUnicom_ack) - 1) )//���й���ͨ�Ƚ�
				{
					c4g_info.Operators = CHINA_UNICOM;//��ͨ
					return TRUE;
				}
				if( !strncasecmp((char*)&(rx->buf[14]), Operators_ChinaTelecom_ack, sizeof(Operators_ChinaTelecom_ack) - 1) )//���й����űȽ�
				{
					c4g_info.Operators = CHINA_TELECOM;//����
					return TRUE;
				}
				#if C_4G_LOG
				printf("4g_err: check Operators\r\n\r\n");
				#endif
			}
		}
		#if C_4G_LOG
		printf("4g_no_ack: check Operators\r\n\r\n");
		#endif
		return FALSE;
	}
}

/*
@���ܣ���ѯsim��ICCID��Integrated Circuit Card Identifier��
*/
static uint8_t c4g_chk_ICCID(void)
{
	UART7_DATA* rx;
	uint8_t err;
	uint8_t i;

	_uart7_send(&rx, (uint8_t*)chk_ICCID, (sizeof(chk_ICCID) - 1), callback_4g_recv);
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), chk_ICCID_ack, sizeof(chk_ICCID_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�,
		{
			if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�жϽ�β�Ƿ�Ϊ"OK"
			{
				for(i = 0; i < ICCID_LEN - 1 && rx->buf[10+i] != 0x0D; i++)//��0x0D��ֹ
					class_global.net.arr_ICCID[i] = rx->buf[10+i];
				class_global.net.arr_ICCID[10+i] = '\0';//������
				return TRUE;
			}
			#if C_4G_LOG
			printf("4g_err: check ICCID\r\n\r\n");
			#endif
		}
	}
	class_global.net.arr_ICCID[0] = 'X';
	class_global.net.arr_ICCID[1] = 0;
	
	#if C_4G_LOG
	printf("4g_err: check ICCID\r\n\r\n");
	#endif
	return FALSE;
}

/*
@���ܣ���ѯ�ź�ǿ��
*/
static uint8_t c4g_chk_quality(void)
{
	UART7_DATA* rx;
	uint8_t err;


	_uart7_send(&rx, (uint8_t*)chk_signal, (sizeof(chk_signal) - 1), callback_4g_recv);
	err = c4g_sem_get(300);//�ȴ���ʱ
	if(err == pdTRUE)
	{
		if( !strncasecmp((char*)&(rx->buf[2]), chk_signal_ack, sizeof(chk_signal_ack) - 1) )//�ӵ�3���ַ���ʼ�Ƚ�
		{
			if( !strncasecmp((char*)&(rx->buf[rx->len-4]), common_ack, sizeof(common_ack) - 1) )//�жϽ�β�Ƿ�Ϊ"OK"
			{
				char i, j, rssi[10] = {0}, ber[10] = {0};
				for(i = 7, j = 0; (i < rx->len) && (rx->buf[i] != ',') ;i++)
				{
					if(rx->buf[i] != ' ')
						rssi[j++] = rx->buf[i];//�ź�ǿ��
				}
				
				i++;
				
				for(j = 0; rx->buf[i] != '\r' && rx->buf[i+1] != '\n'; i++)
					ber[j++] = rx->buf[i];//������
				
				c4g_info.rssi = atoi((const char*)rssi);
				c4g_info.ber = atoi((const char*)ber);
				
				#if(0)
				printf("4g: signal quality = ");
				printf((const char*)rssi);
				printf("\r\n\r\n");
				
				printf("4g: err code = ");
				printf((const char*)ber);
				printf("\r\n\r\n");
				#endif
				
				return TRUE;
			}
			#if C_4G_LOG
			printf("4g_err: quality format\r\n\r\n");
			#endif
		}
	}
	class_global.net.arr_ICCID[0] = 'X';
	class_global.net.arr_ICCID[1] = 0;
	
	#if C_4G_LOG
	printf("4g_err: check quality\r\n\r\n");
	#endif
	return FALSE;
}


/*
@���ܣ�4G������
*/
uint32_t time;
void main_task_4g(void)
{
	uint8_t rs;
	uint8_t work_state = 0;//0�����ϵ磻1�������ӹ�һ��
	uint8_t tcp_exist = 0;//�Ƿ����tcp���ӡ�0�������ڣ�1,����
	
	if(class_global.net.state == 1)
	{
		c4g_info.reboot_timeout = xTaskGetTickCount();//����ʧ��ʱ��
		return;
	}

	c4g_info.fsm = ST_EXIT_TRANSPARTENT;//ÿ��ģ���������˳�͸����ʼ
	time = ONE_SECOND;
	vTaskDelay(3*ONE_SECOND);
	while(1)
	{
		//ʧ����ʱ
		if(c4g_info.reboot_timeout == 0)
			c4g_info.reboot_timeout = xTaskGetTickCount();
		if(xTaskGetTickCount() - c4g_info.reboot_timeout > 120000)//2��������
		{
			if((c4g_info.fsm == ST_CHK_SIM
			|| c4g_info.fsm == ST_CHK_SIM_REG
			|| c4g_info.fsm == ST_CHK_NET_REG))//����������ǿ�����ѯ���磬��������ʱ��ӳ�
			{
				if(xTaskGetTickCount() - c4g_info.reboot_timeout > 240000)//����4��������
				{
					c4g_info.fsm = ST_REBOOT;//����
					c4g_info.fsm = ST_EXIT_TRANSPARTENT;//��ʼ��״̬
					work_state = 0;//״̬��0
					tcp_exist = 0;//�ñ�־Ϊ������TCP����
					c4g_info.reboot_timeout = xTaskGetTickCount();//ˢ��ʱ�䣬ÿ2��������һ��
					printf("4g reboot\r\n");
					restart_equ_set(RESET_4G, FALSE);
				}
			}
			else//������3�����������״̬��ʧ������2��������
			{
				c4g_power_off();
				c4g_power_on();
				c4g_info.fsm = ST_EXIT_TRANSPARTENT;//��ʼ��״̬
				work_state = 0;//״̬��0
				tcp_exist = 0;//�ñ�־Ϊ������TCP����
				c4g_info.reboot_timeout = xTaskGetTickCount();//ˢ��ʱ�䣬ÿ2��������һ��
				printf("power on\r\n");
				restart_equ_set(RESET_4G, FALSE);
			}
		}
		
		if(xTaskGetTickCount() - c4g_info.lost_link_timeout > 40000 || work_state == 0)//����40s��ͨѶģʽ���Ӳ��ϣ��Ž�������
		{
			switch(c4g_info.fsm)
			{
				case ST_EXIT_TRANSPARTENT://�˳�͸��ģʽ
					c4g_info.rssi = 0xff;//��̖���ȳ�ʼ��
					c4g_info.ber = 0xff;//�����ʳ�ʼ��
					work_state = 0;//���������Ҫ����ѭ��
					vTaskDelay(1000);
					rs = c4g_exit_transpartent();
					vTaskDelay(1000);
					if(rs == TRUE){
						//�л�Ӧ˵��ģ���ǿ����ģ����ҳɹ��˳�͸��ģʽ�����������������Ѿ��ر�
						//����ʱһ���Ǹ�λ�����壬����û�и�λģ�飩
//						c4g_info.fsm = ST_CHK_TCP;//��ת����ѯTCP�Ƿ����ӡ�
						tcp_exist = 1;//�����Ӧ���Ǵ��ڵ�
						if(c4g_chk_ICCID())//��ѯICCID�ɹ�����ʱ�϶���sim��
							c4g_info.fsm = ST_CHK_QUALITY;//��ת��ѯ��Ӫ��
					}
					else
					if(rs == 2){
						printf("exit cmd mode fail\r\n");
					}else{
						rs = c4g_chk_sim();
						rs = c4g_chk_ICCID();
						if(rs == TRUE){//�Ѿ����������Ѿ�������ģʽ(һ���ǻ�δ����͸��ģʽ����������)
//							c4g_info.fsm = ST_CHK_TCP;//��ת����ѯTCP�Ƿ����ӡ�
							tcp_exist = 3;//������Ƿ���ڲ�ȷ��
							c4g_info.fsm = ST_CHK_QUALITY;//��ת��ѯ��Ӫ��
						}else
						if(rs == 2){//��ʾû��SIM��
							printf("no sim 1\r\n");
						}else{
							c4g_info.fsm = ST_PWR_ON;//û�л�Ӧ˵��ģ���ǹػ���
						}
					}
				break;
					
				case ST_CHK_QUALITY://��ѯ�ź�ǿ��
				{
					uint8_t i = 0, tmp[3] = {0};
					do{
						c4g_chk_quality();
						tmp[i%3] = c4g_info.rssi;
						if(tmp[0] != SIGNAL_NO && tmp[0] != TDSCDMA_SIGNAL_NO && tmp[0] == tmp[1] && tmp[1] == tmp[2])
						{
							break;
						}
						else
						if( ((tmp[0] > SIGNAL_MID && tmp[0] < SIGNAL_NO) || (tmp[1] > SIGNAL_MID && tmp[1] < SIGNAL_NO) || (tmp[2] > SIGNAL_MID && tmp[2] < SIGNAL_NO))
						||  ((tmp[0] > TDSCDMA_SIGNAL_MID && tmp[0] < TDSCDMA_SIGNAL_NO) || (tmp[1] > TDSCDMA_SIGNAL_MID && tmp[1] < TDSCDMA_SIGNAL_NO) || (tmp[2] > TDSCDMA_SIGNAL_MID && tmp[2] < TDSCDMA_SIGNAL_NO)))
						{
							break;
						}
						
						vTaskDelay(ONE_SECOND);
					}while(i++ < 30);
					
					c4g_info.fsm = ST_CHK_OPERATER;//��ת��ѯ��Ӫ��
				}
				break;
					
				case ST_PWR_ON:		//�ϵ�
//					c4g_power_off();
					c4g_power_on();
					c4g_info.fsm = ST_CLOSE_CMD_ECHO;//�������رջ��ԡ�
				break;
				
				case ST_CLOSE_CMD_ECHO://�ر��������
					if(c4g_close_cmd_echo())
						c4g_info.fsm = ST_CHK_SIM;
				break;
					
				case ST_CHK_SIM:	//��sim��
					if(c4g_info.find_sim_timeout == 0)
						c4g_info.find_sim_timeout = xTaskGetTickCount();//��ʼ��ʱ
					
					rs = c4g_chk_sim();
					if(rs == TRUE){
						c4g_info.fsm = ST_CHK_ICCID;	//��ת->"��ѯICCID"
					}else
					if(rs == 2){//��ʾ δ�忨
						if( xTaskGetTickCount() - c4g_info.find_sim_timeout < 3000)//���3s����ʾ
							printf("no sim 2\r\n");
					}else{//����4G
						if(xTaskGetTickCount() - c4g_info.find_sim_timeout > 20000){//��ʱ����
							c4g_info.fsm = ST_REBOOT;
							c4g_info.find_sim_timeout = 0;//��ʱ����
						}
					}
				break;
					
				case ST_CHK_ICCID:	//��ѯSIM��ICCID
					if(c4g_chk_ICCID())
						c4g_info.fsm = ST_CHK_SIM_REG;
					else
						printf("check ICCID err\r\n\r\n");
				break;
					
				case ST_CHK_SIM_REG:	//��ѯsim��ע��
					if(c4g_info.chk_sim_reg_timeout == 0)
						c4g_info.chk_sim_reg_timeout = xTaskGetTickCount();//��ʼ��ʱ
					
					rs = c4g_sim_reg();
					if(rs == TRUE){
						c4g_info.fsm = ST_CHK_NET_REG;
					}else 
					if(rs == 2){//������Ϣ
						
						if(xTaskGetTickCount() - c4g_info.chk_sim_reg_timeout > 90000){//��ʱ����
							c4g_info.fsm = ST_REBOOT;
							c4g_info.chk_sim_reg_timeout = 0;//��ʱ����
						}
					}
				break;
					
				case ST_CHK_NET_REG:	//��ѯ����
					if(c4g_info.chk_net_reg_timeout == 0)
						c4g_info.chk_net_reg_timeout = xTaskGetTickCount();//��ʼ��ʱ
				
					rs = c4g_net_reg();
					if(rs == TRUE){
//						c4g_info.fsm = ST_CHK_OPERATER;
						c4g_info.fsm = ST_CHK_QUALITY;//��ת->���ź�ǿ��
					}else
					if(rs == 2){//������Ϣ
						if(xTaskGetTickCount() - c4g_info.chk_net_reg_timeout > 60000){//��ʱ����
							c4g_info.fsm = ST_REBOOT;
							c4g_info.chk_net_reg_timeout = 0;//��ʱ����
						}
					}
				break;
					
				case ST_CHK_OPERATER://��ѯ��Ӫ��
					rs = c4g_check_Operators();
					if(rs == TRUE){
						if(tcp_exist == 0)
							c4g_info.fsm = ST_SET_POINT;//���ý����
						else
						if(tcp_exist == 1)
							c4g_info.fsm = ST_CHK_TCP;//��ѯTCP����
						else
						if(tcp_exist == 3)
							c4g_info.fsm = ST_DEACTIVE_POINT;//�رս����
					}else{
						printf("Operater err\r\n");//û�鵽��Ӫ�̣��ȴ�����
					}
				break;
					
				case ST_SET_POINT:	//���ý����(����������Ӫ��ʲô��)
					rs = c4g_set_point();
					if(rs == TRUE){
						tcp_exist = 1;//���ý����ɹ�
						c4g_info.fsm = ST_ACTIVE_POINT;//��ת"��������"
					}else
					if(rs == 2){//������Ϣ
						c4g_info.fsm = ST_REBOOT;
					}
				break;
					
				case ST_CHK_POINT:	//��ѯ�����
					
				break;
				
				case ST_ACTIVE_POINT:	//��������
					rs = c4g_active_context();
					if(rs == TRUE){
						c4g_info.fsm = ST_CREAT_TCP;//��ת"����TCP"
					}else
					if(rs == 2){//������Ϣ
						c4g_info.fsm = ST_DEACTIVE_POINT;//��ת"�رս����"
					}
				break;
					
				case ST_DEACTIVE_POINT:	//�رս����
					rs = c4g_deactive_context();
					if(rs == TRUE){
						tcp_exist = 0;//�ѹرս����
						c4g_info.fsm = ST_SET_POINT;//��ת"��������"
					}else
					if(rs == 2){//������Ϣ
						printf("link point err 1\r\n");//���������ʧ��
					}
				break;
					
				case ST_CHK_CONTEXT:	//��ѯ������ı�
					rs = c4g_chk_context();
					if(rs == TRUE){//��ѯ�ɹ�������δ����
						c4g_info.fsm = ST_ACTIVE_POINT;
					}else
					if(rs == 3){//�Ѿ�����
						c4g_info.fsm = ST_CHK_TCP;//�Ȳ���TCP�����Ƿ���ڣ�������ڣ�ֱ�������������ڣ��򴴽�����
					}else
					if(rs == 2){//������Ϣ
						printf("context err 1\r\n");//��ѯ������ı�����
					}
				break;
					
				case ST_CREAT_TCP:		//����TCP����
					rs = c4g_test_tcp();
					rs = c4g_creat_tcp();
					//20210411 ������½�����socket������
					if(work_state == 1)
						restart_equ_set(RESET_RebuitSocket, FALSE);
					if(rs == TRUE){//ֱ�ӽ���͸��ģʽ
						c4g_info.fsm = ST_EXIT_TRANSPARTENT;//��ʼ��״̬���˳�͸��ģʽ��
						c4g_info.lost_link_timeout = 0;
						work_state = 1;
						return;
					}else{//������Ϣ
						c4g_info.fsm = ST_CHK_TCP;//��ת����ѯTCP��
					}
				break;
					
				case ST_CLOSE_TCP:		//�ر�TCP
					rs = c4g_close_tcp();
					if(rs == TRUE){//�ر�TCP�ɹ�
						vTaskDelay(100);//@@������Ҫ�ȴ�100ms����Ȼ����������������⣬�����쳣
						c4g_info.fsm = ST_CREAT_TCP;//��ת"����TCP"
					}else
					if(rs == 2){//������Ϣ
						printf("close tcp err 1\r\n");//�ر�tcpʧ��
					}
				break;
					
				case ST_CHK_TCP:		//��ѯTCP
					rs = c4g_chk_tcp();
					#if C_4G_LOG
					printf("c4g_chk_tcp\r\n");
					#endif
					if(rs == TRUE){//û�н���TCP;��Ҫ���´���TCP����
						c4g_info.fsm = ST_CREAT_TCP;//��ת������TCP��
					}else//********************
					if(rs == 3){//TCP�����Ѵ���
						if(work_state == 0)//������ϵ���һ��
							c4g_info.fsm = ST_CLOSE_TCP;//��ת���ر�TCP������ʱ��Ҫ�ؽ�TCP
						else
							c4g_info.fsm = ST_ENTER_TRANSPARENT;//��ת������͸����
					}else
					if(rs == 4){//��Ҫ�ȹر�socket
						c4g_info.fsm = ST_CLOSE_TCP;//��ת���ر�TCP��
					}else{//��Ӧ��
						printf("chk tcp err 1\r\n");//��ѯtcpʧ��
					}
				break;
					
				case ST_ENTER_TRANSPARENT:	//����ģʽ����͸��ģʽ
					rs = c4g_enter_transparent();
					if(rs == TRUE){//����͸��ģʽ
						c4g_info.fsm = ST_EXIT_TRANSPARTENT;//��ʼ��״̬���˳�͸��ģʽ��
						c4g_info.lost_link_timeout = 0;
						work_state = 1;
						return;
					}else
					if(rs == 2){//����͸��ʧ��
						c4g_info.fsm = ST_CLOSE_TCP;//��ת���ر�TCP��
					}else{//��Ӧ��
						printf("enter transpartent err 1\r\n");//����͸����Ӧ��
					}
				break;
				
				case ST_CHK_DATA_ECHO:	//��ѯ���ݻ���
					
				break;
				case ST_CLOSE_DATA_ECHO://�ر����ݻ���
					
				break;
				case ST_CHK_ERRCODE:	//��ѯ����Ĵ�����
					
				break;
				
				case ST_REBOOT:		//����
					c4g_power_reset();
					c4g_info.fsm = ST_CLOSE_CMD_ECHO;//��ת���رջ��ԡ�
				break;
				
				case ST_CHANGE_TRANSPARENT:	//��Ϊ͸��ģʽ
					if(c4g_change_transpartent() == TRUE){//����͸��ģʽ
						c4g_info.fsm = ST_EXIT_TRANSPARTENT;//��ʼ���´ν����״̬��ֵ
						c4g_info.lost_link_timeout = 0;
						work_state = 1;
						return;
					}else{
						c4g_info.fsm = ST_REBOOT;//����
					}
			}
		}
		vTaskDelay(100);
	}
}

/*
@���ܣ���ȡ4g��ǰ״̬
@���أ���ǰ״̬������ִ�в���
*/
uint8_t get_4g_state(void)
{
	return c4g_info.fsm;
}

/*
@���ܣ�����ź�ǿ��
@������rssi���ź�ǿ�Ȼ��壻ber�������ʻ���
@���أ�1~4���źŵȼ���0��û��ִ�У�5��ִֻ���˳�ʼֵ
*/
uint8_t get_4g_quality(uint8_t* rssi, uint8_t* ber)
{
	uint8_t rs = FALSE;
	
	if(rssi != 0)
		*rssi = c4g_info.rssi;
	if(ber != 0)
		*ber = c4g_info.ber;
	
	if(c4g_info.rssi == 0xff)//��ʼֵ
		rs = 5;
	else
	if( c4g_info.rssi == SIGNAL_NO || c4g_info.rssi == TDSCDMA_SIGNAL_NO )//���ź�
		rs = 1;
	else
	if( c4g_info.rssi <= SIGNAL_LOW || (c4g_info.rssi > SIGNAL_NO && c4g_info.rssi <= TDSCDMA_SIGNAL_LOW) )//�ź�ǿ��1����
		rs = 2;
	else
	if( (c4g_info.rssi > SIGNAL_LOW && c4g_info.rssi <= SIGNAL_MID) || (c4g_info.rssi > TDSCDMA_SIGNAL_LOW && c4g_info.rssi <= TDSCDMA_SIGNAL_MID))//�ź�ǿ��2����
		rs = 3;
	else
	if( (c4g_info.rssi > SIGNAL_MID && c4g_info.rssi < SIGNAL_NO) || (c4g_info.rssi > TDSCDMA_SIGNAL_MID && c4g_info.rssi < TDSCDMA_SIGNAL_NO))//�ź�ǿ��3��ǿ
		rs = 4;
	
	return rs;
}

