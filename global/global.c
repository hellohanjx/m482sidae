#include "global.h"
#include "string.h"

volatile char main_version[] = {"3XXXX-211208"};//�汾��






volatile CLASS_GLOBAL class_global;//ȫ����Ϣ



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
	class_global.net.serverPort = 8001;//�˿ں�
	strcpy((char*)class_global.net.arr_ip, (const char*)DEF_HEIM_IP);//ip
	
	class_global.net.id = 1000000006;//����id
	class_global.net.password = 0;//��������
	
	class_global.sys.factory_en = 1;//����ģʽ
	class_global.trade.price = 1;//�۸�
	
}

