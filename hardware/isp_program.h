#ifndef _ISP_PROGRAM_H_
#define _ISP_PROGRAM_H_

#include "M480.h"

/*
@˵����m482���Կ�[4096 Byte]�������û�����ʹ���� APROM ���һ�飬��ַ 0x7F000~0x80000
			Ϊ�˷�ֹAPROMĥ�𣬲��þ���ĥ��ʽ
			���ݷ������֣�
				1.���ݱ�ÿ��������һ����ͷ����ʾ��Ӧ�����ݴ洢�����ÿ������ 4Byte ��ͷ����ͷֵ=0xFFFFFFFF����Ӧ32bit��ÿһ��bit��ʾ��ǰ����ֵ��λ��
				2.����ֵ�����ݱ� 32bit ÿһ�� bit ��Ӧһ������ֵ��ÿ�����ݹ�32������ֵ��ÿ������ֵ�̶� 4Byte
			����ã�ÿ������ռ�ù̶�132Byte��һ��4096Byte�����洢4096/132 = 31.03����ȥ��ͷ����30������
*/

#define USR_DATA_BASE_ADDR			0x7F000		//�û����ݻ���ַ

/**************************************
@��־λ
**************************************/
#define FLASH_VAL_FLAG				0XAA				//flash��־
#define FLASH_ADDR_FLAG				0x7F000			//flash��־λ��ַ��1Byte��


#define FLASH_PARAM_NUM			32//ÿ��ƫ�Ʊ����������Ͷ�Ӧ�Ĵ�������������
#define FLASH_LIST_BASE			0x7F004	//ƫ�ơ���Ļ�ַ��{0x7F004 ~ 0x7F07C}
#define FLASH_VAL_BASE			0x7F07C	//������ֵ�Ļ�ַ��{0x7F07C ~ 0x80000}

/****************************************************
@ƫ�Ʊ�32bit��ÿһλ��Ӧһ��ֵ
@˵������Щ���ǳ�ʼ������Ҫ�õģ��������õĲ���Ҫ����
****************************************************/
#define FLASH_LIST_Id								0x7F004	//1--����ID��ƫ�Ƶ�ַ����Ӧ32��ID��ַ��
#define FLASH_LIST_Ip								0x7F008	//2--ip��ַ
#define	FLASH_LIST_Port							0x7F00C	//3--�˿ں�
#define FLASH_LIST_FactoryEn				0x7F010	//4--����ģʽ
#define FLASH_LIST_CardType					0x7F014	//5--ˢ������
#define FLASH_LIST_PointBit					0x7F018	//6--С��λ��
#define FLASH_LIST_PriceBit					0x7F01C	//7--�۸�λ��
#define FLASH_LIST_PriceMax					0x7F020 //8--���۸�




/****************************************************
@����ֵ������ֵ��ַ
****************************************************/
#define	FLASH_ADDR_Id							FLASH_VAL_BASE	//����IDֵ�׵�ַ��

#define	FLASH_ADDR_Ip							(4*FLASH_PARAM_NUM + FLASH_ADDR_Id) //����ipֵ�׵�ַ��

#define FLASH_ADDR_Port						(4*FLASH_PARAM_NUM + FLASH_ADDR_Ip)//�˿ں�

#define FLASH_ADDR_FactoryEn			(4*FLASH_PARAM_NUM + FLASH_ADDR_Port)//����ģʽ

#define FLASH_ADDR_CardType				(4*FLASH_PARAM_NUM + FLASH_ADDR_FactoryEn)//ˢ��������

#define FLASH_ADDR_PointBit				(4*FLASH_PARAM_NUM + FLASH_ADDR_CardType)//С����

#define FLASH_ADDR_PriceBit				(4*FLASH_PARAM_NUM + FLASH_ADDR_PointBit)//�۸�λ��

#define FLASH_ADDR_PriceMax				(4*FLASH_PARAM_NUM + FLASH_ADDR_PriceBit)//���۸�






uint8_t _isp_config(void);
uint32_t flash_param_get(uint32_t list_addr, uint32_t data_addr);
uint8_t flash_param_set(uint32_t list_addr, uint32_t data_addr, uint32_t val);
uint8_t reset_param_block(void);

#endif
