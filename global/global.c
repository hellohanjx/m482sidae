#include "global.h"
#include "string.h"
#include "isp_program.h"

volatile char main_version[] = {"Nu-220122"};//�汾��






volatile CLASS_GLOBAL class_global;//ȫ�ֱ���


/*
@���ܣ���λ�豸/���ñ�־λ
@������val����λ���ͣ�type���Ƿ�Ҫ����
@˵���������˵���������������ݲ���
*/
void restart_equ_set(uint8_t val, uint8_t type)
{
//	reset_flag_1 = val;
//	reset_flag_2 = val;
//	reset_flag_3 = val;
//	
//	if(type)//����
//	{
//		__set_FAULTMASK(1);
//		NVIC_SystemReset();
//	}
}

/*
@���ܣ���ȡ��λ��־λ
@����ֵ����λֵ
*/
uint8_t restart_equ_get(void)
{
//	if(reset_flag_1 == reset_flag_2 && reset_flag_2 == reset_flag_3)
//		return reset_flag_1;
//	else
		return RESET_PowerOn;
}



/*
@���ܣ���ȡһ���������
@���أ������
*/
uint32_t get_random_number(void)
{
	uint32_t random;
	
//	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);   //ʹ��RNGʱ��
//	RNG_Cmd(ENABLE);       																//ʹ��RNG����
//	
//	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET);  		//�ȴ�ת������
//	random = RNG_GetRandomNumber();    //��ȡ�����
//	
//	RNG_Cmd(DISABLE);																			//�ر�����
//	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, DISABLE);  //�ر�RNGʱ��
	
	return random;
}




/*
@���ܣ�ȫ�ֱ�����ʼ��
*/
void global_init(void)
{
	char tmp[4];
	uint32_t i;
//	uint32_t card_type, factory, point_bit, price_bit, price_max;
	
	judge_param();
	
	class_global.net.id = flash_param_get(FLASH_LIST_Id, FLASH_ADDR_Id);//id
	
	//��ȡIP
	{
		uint8_t ip[4];
		uint32_t ip_long;
		ip_long = flash_param_get(FLASH_LIST_Ip, FLASH_ADDR_Ip);//ip
		ip[0] = ip_long >> 24;
		ip[1] = ip_long >> 16;
		ip[2] = ip_long >> 8;
		ip[3] = ip_long & 0xff;
		
		for(i = 0; i < IP_LEN; i++)//ip��ʼ��
			class_global.net.arr_ip[i] = 0;
		for(i = 0; i < 4; i++)
		{
			tmp[sprintf(tmp, "%u", ip[i])] = 0;
			strcat((char*)class_global.net.arr_ip, tmp);
			if(i != 3)
				strcat((char*)class_global.net.arr_ip, (char*)".");
		}
	}
	
	class_global.net.serverPort = flash_param_get(FLASH_LIST_Port, FLASH_ADDR_Port);//�˿ں�
//	card_type = flash_param_get(FLASH_LIST_CardType, FLASH_ADDR_CardType);//������
//	point_bit = flash_param_get(FLASH_LIST_PointBit, FLASH_ADDR_PointBit);//С��
//	price_bit = flash_param_get(FLASH_LIST_PriceBit, FLASH_ADDR_PriceBit);//�۸�λ��
//	price_max = flash_param_get(FLASH_LIST_PriceMax, FLASH_ADDR_PriceMax);//���۸�
	
	class_global.net.password = DEF_FLASH_PassWord;//��������
	
	class_global.sys.factory_en = (enum FACTOTY_MODE)0;//(enum FACTOTY_MODE)flash_param_get(FLASH_LIST_FactoryEn, FLASH_ADDR_FactoryEn);//����ģʽ
	
	for(i = 0; i < 6; i++)//��ʼ��ֵ
	{
		class_global.ireader[i].equ.interface = DEF_CARD_INTERFACE;//ˢ�����ӿ�
		class_global.ireader[i].equ.state = 0xff;//ˢ����״̬
		class_global.ireader[i].count[0].state = 0xff;//��������ź�1״̬
		class_global.ireader[i].count[1].state = 0xff;//��������ź�2״̬
	}
}

