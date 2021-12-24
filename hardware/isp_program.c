#include "isp_program.h"
#include "global.h"
#include "user_config.h"


/*
@���ܣ������û����ݻ���ַ
@������u32DFBA�������û���ַ
@���أ�0�ɹ���-1ʧ��
*/
static int  set_data_flash_base(uint32_t u32DFBA)
{
    uint32_t   au32Config[2];          /* User Configuration */

    /* Read User Configuration 0 & 1 */
    if (FMC_ReadConfig(au32Config, 2) < 0)
    {
        printf("\nRead User Config failed!\n");       /* Error message */
        return -1;                     /* failed to read User Configuration */
    }

    /* Check if Data Flash is enabled and is expected address. */
    if ((!(au32Config[0] & 0x1)) && (au32Config[1] == u32DFBA))
        return 0;                      /* no need to modify User Configuration */

    FMC_ENABLE_CFG_UPDATE();           /* Enable User Configuration update. */

    au32Config[0] &= ~0x1;             /* Clear CONFIG0 bit 0 to enable Data Flash */
    au32Config[1] = u32DFBA;           /* Give Data Flash base address  */

    /* Update User Configuration settings. */
    if (FMC_WriteConfig(au32Config, 2) < 0)
        return -1;                     /* failed to write user configuration */

    printf("\nSet Data Flash base as 0x%x.\n", USR_DATA_BASE_ADDR);  /* debug message */

    /* Perform chip reset to make new User Config take effect. */
    SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    return 0;                          /* success */
}

/*
@���ܣ���ȡоƬΨһʶ����
*/
static void read_unique_id(uint32_t* id)
{
	uint8_t i;
	for (i = 0; i < 3; i++)            /* Get 96-bits UID. */
	{
		id[i] = FMC_ReadUID(i);
		printf("  Unique ID %d ........................... [0x%08x]\n", i, id[i]);  /* information message */
	}
	
}


/*********************************************************
@flash��־λ����
*********************************************************/
/*
@���ܣ���ȡ�ڲ�flash��־λ
@���أ���־ֵ
*/
static uint32_t flash_flag_get(void)
{
	SYS_UnlockReg();//�����Ĵ���
	return FMC_Read(FLASH_ADDR_FLAG);
}

/*
@���ܣ�д�ڲ�flash��־λ
@��������־ֵ
@���أ�TRUE��д�ɹ���FALSE��дʧ��
*/
static uint8_t flash_flag_set(uint32_t val)
{
	FMC_Write(FLASH_ADDR_FLAG, val);
	if( val == FMC_Read(FLASH_ADDR_FLAG) )
		return TRUE;
	return FALSE;
}


/*********************************************************
@flash "ƫ�Ʊ�"����
*********************************************************/
/*
@���ܣ�����ƫ�Ƶ�ַ��Ѱ��δд��ֵ��Ӧ��ƫ��
@���أ�0~31��ƫ��ֵ��0xff����д��
*/
static uint8_t flash_freeaddr_get(uint32_t addr)
{
	uint8_t i;
	uint32_t offset = FMC_Read(addr);
	uint32_t tmp = 1;
	
	for(i = 0; i < FLASH_PARAM_NUM; i++)
	{
		if( (offset & (tmp << i)) )
		{
			return i;
		}
	}
	return 0xff;
}

/*
@���ܣ�дƫ�Ƶ�ַ��
@������addr��ƫ�Ƶ�ַ���ַ��offset��ƫ��ֵ0~31
@���أ�TRUE,�ɹ���FLASE��ʧ��
*/
static uint8_t flash_freeaddr_set(uint32_t addr)
{
	uint32_t val = FMC_Read(addr);

	if(val)
	{
		val <<= 1;
		FMC_Write(addr, val);
		if(FMC_Read(addr) == val)
			return TRUE;
	}
	printf("flash freeaddr no space\r\n\r\n");
	return FALSE;
}




/*********************************************************
@���ܣ������޷�д��ʱ�Ļָ�����
@���أ�TRUE���ɹ���FALSE��ʧ��
*********************************************************/
static uint8_t flash_param_recovery(void)
{
	uint32_t id, ip, port, factory_en, card_type, point_bit, price_bit, max_price;
	
	if(flash_flag_get() == 0xffffffff)
	{
		//��־λΪ�գ��˿�ղ���
		printf("flash sec is free\r\n\r\n");
		return FALSE;
	}
	
	//������
	id 						= flash_param_get(FLASH_LIST_Id, 						FLASH_ADDR_Id);					//1-����id
	ip						= flash_param_get(FLASH_LIST_Ip, 						FLASH_ADDR_Ip);						//2-����ip
	port 					= flash_param_get(FLASH_LIST_Port, 					FLASH_ADDR_Port);					//3-�˿ں�
	factory_en 		= flash_param_get(FLASH_LIST_FactoryEn, 		FLASH_ADDR_FactoryEn);		//4-����ģʽ
	card_type 		= flash_param_get(FLASH_LIST_CardType, 			FLASH_ADDR_CardType);			//5-ˢ��������
	point_bit 		= flash_param_get(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit);			//6-С����λ��
	price_bit 		= flash_param_get(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit);			//7-�۸�λ��
	max_price 		= flash_param_get(FLASH_ADDR_PriceMax, 			FLASH_ADDR_PriceMax);			//8-���۸�
	
	FMC_Erase(USR_DATA_BASE_ADDR);//��������
	
	//��д����
	flash_flag_set(FLASH_VAL_FLAG);//д��־
	flash_param_set(FLASH_LIST_Id, 						FLASH_ADDR_Id, 				id);						//1-����id
	flash_param_set(FLASH_LIST_Ip, 						FLASH_ADDR_Ip, 					ip);						//1-ip
	flash_param_set(FLASH_LIST_Port, 					FLASH_ADDR_Port, 				port);					//3-�˿ں�
	flash_param_set(FLASH_LIST_FactoryEn, 		FLASH_ADDR_FactoryEn, 	factory_en);		//4-����ģʽ
	flash_param_set(FLASH_LIST_CardType, 			FLASH_ADDR_CardType, 		card_type);			//5-ˢ��������
	flash_param_set(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit,		point_bit);			//6-С����λ��
	flash_param_set(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit,		price_bit);			//7-�۸�λ��
	flash_param_set(FLASH_ADDR_PriceMax, 			FLASH_ADDR_PriceMax, 		max_price);			//8-���۸�

	
	//��֤��д�Ƿ���ȷ
	if(	id 						== flash_param_get(FLASH_LIST_Id, 						FLASH_ADDR_Id)					//1-����id
	&&	ip						== flash_param_get(FLASH_LIST_Ip, 						FLASH_ADDR_Ip)						//1-ip
	&&	port 					== flash_param_get(FLASH_LIST_Port, 					FLASH_ADDR_Port)					//3-�˿ں�
	&&	factory_en 		== flash_param_get(FLASH_LIST_FactoryEn, 			FLASH_ADDR_FactoryEn)			//4-����ģʽ
	&&	card_type 		== flash_param_get(FLASH_LIST_CardType, 			FLASH_ADDR_CardType)			//5-ˢ��������
	&&	point_bit 		== flash_param_get(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit)			//6-С����λ��
	&&	price_bit 		== flash_param_get(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit)			//7-�۸�λ��
	&&	max_price 		== flash_param_get(FLASH_ADDR_PriceMax, 			FLASH_ADDR_PriceMax)			//8-���۸�
	&&  FLASH_VAL_FLAG == flash_flag_get() )
	{
		return TRUE;
	}
	printf("flash recovery err\r\n\r\n");
	return FALSE;
}




/*****************************************************************************************************************************
********************************************************** ����ӿ� **********************************************************
*****************************************************************************************************************************/
/*
@���ܣ�������
@������list_addr=ƫ�Ʊ�data_addr=��дֵ��ַ
@���أ�����ֵ
*/
uint32_t flash_param_get(uint32_t list_addr, uint32_t data_addr)
{
	uint32_t offset;
	uint32_t rs = 0;
		
	SYS_UnlockReg();//�����Ĵ���
	
	offset = flash_freeaddr_get(list_addr);//��ȡƫ����
	if(offset == 0xff)//����Ч���ݣ�˵�����ܼ���д�ˣ��ռ�������,���Ƕ��Ļ����һλ����Ч��
	{
		offset = 31;
	}
	else
	{
		if(offset)
			offset -= 1;
	}
	
	rs = FMC_Read(data_addr + offset*4);
	
	SYS_LockReg();
	return rs;
}

/*
@���ܣ�д����
@������list_addr��ƫ�Ʊ�data_addr����дֵ��ַ��data_size����д���ݿ�ȣ�val����д����ֵ
@���أ�TRUE��д�ɹ���FALSE��дʧ��
*/
uint8_t flash_param_set(uint32_t list_addr, uint32_t data_addr, uint32_t val)
{
	uint8_t rs = TRUE;
	uint32_t offset;
	
	SYS_UnlockReg();//�����Ĵ���
	
	offset = flash_freeaddr_get(list_addr);//Ѱ�ҿ�д��ַ
	if(offset == 0xff)//����Ч���ݣ�˵�����ܼ���д�ˣ��ռ�������
	{
		printf("flash space full\r\n");
		rs = flash_param_recovery();
	}
	
	if(rs)
	{
		rs = FALSE;
		
		//д
		FMC_Write((data_addr + offset*4), val);//@setup-1��дֵ
		flash_freeaddr_set(list_addr);//@setup-2:дƫ�Ʊ�
		
		//��֤
		if(FMC_Read(data_addr + offset*4) == val )
			rs = TRUE;
	}
	else
	{
		printf("flash recovery err\r\n");
	}
	SYS_LockReg();
	return rs;
}




/*
@���ܣ��û���������ʼ��
@���أ�TRUE=�ɹ�
*/
uint8_t _isp_config(void)
{
//	SYS_UnlockReg();//���Ĵ�������@@������Ҫ�򿪼Ĵ�����
	FMC_Open();//ISP����ʹ��
	if (set_data_flash_base(USR_DATA_BASE_ADDR) < 0)
	{
		printf("Failed to set Data Flash base address!\n");          /* error message */
		return FALSE;
	}
	
	read_unique_id((uint32_t*)class_global.sys.unique_id);//��ȡΨһid
	return TRUE;
}


/*
@���ܣ���λ������
@���أ�TRUE���ɹ���FALSE��ʧ��
*/
uint8_t reset_param_block(void)
{
	uint8_t rs = FALSE;
	SYS_UnlockReg();//�����Ĵ���
	
	if(flash_flag_get() != FLASH_VAL_FLAG)//��־������
	{
		printf("flash block reset\r\n\r\n");
		FMC_Erase(USR_DATA_BASE_ADDR);//��������
		
		//дĬ������
		flash_flag_set(FLASH_VAL_FLAG);																												//д��־λ
		flash_param_set(FLASH_LIST_Id, 						FLASH_ADDR_Id, 				 		DEF_FLASH_Id);		//1-id
		flash_param_set(FLASH_LIST_Ip,	 					FLASH_ADDR_Ip,	 				 	DEF_FLASH_Ip);		//2-IP
		flash_param_set(FLASH_LIST_Port, 					FLASH_ADDR_Port, 					DEF_FLASH_Port);	//3-�˿�
		flash_param_set(FLASH_LIST_FactoryEn, 		FLASH_ADDR_FactoryEn, 		DEF_FLASH_FactoryEn);//4-����ģʽ
		flash_param_set(FLASH_LIST_CardType, 			FLASH_ADDR_CardType, 			DEF_FLASH_CardType);//5-ˢ��������
		flash_param_set(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit,		 	DEF_FLASH_PointBit);//6-С��λ��
		flash_param_set(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit,			DEF_FLASH_PriceBit);//7-�۸�λ��
		flash_param_set(FLASH_LIST_PriceMax, 			FLASH_ADDR_PriceMax, 			DEF_FLASH_PriceMax);		//8-���۸�
			
		
		//У��д������
		if(	DEF_FLASH_Id 						== flash_param_get(FLASH_LIST_Id,							FLASH_ADDR_Id)				//1-id
		&&	DEF_FLASH_Ip 						== flash_param_get(FLASH_LIST_Ip, 						FLASH_ADDR_Ip)				//2-IP
		&&	DEF_FLASH_Port 					== flash_param_get(FLASH_LIST_Port, 					FLASH_ADDR_Port)			//3-�˿�
		&&	DEF_FLASH_FactoryEn 		== flash_param_get(FLASH_LIST_FactoryEn, 			FLASH_ADDR_FactoryEn)	//4-����ģʽ
		&&	DEF_FLASH_CardType 			== flash_param_get(FLASH_LIST_CardType, 			FLASH_ADDR_CardType)	//5-ˢ��������
		&&	DEF_FLASH_PointBit 			== flash_param_get(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit)		//6-С��λ��
		&&	DEF_FLASH_PriceBit 			== flash_param_get(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit)//7-�۸�λ��
		&&	DEF_FLASH_PriceMax 			== flash_param_get(FLASH_LIST_PriceMax, 			FLASH_ADDR_PriceMax)		//8-���۸�
		&&  FLASH_VAL_FLAG 					== flash_flag_get()																										//��־λ
		)
		{
			rs = TRUE;
		}
		else
		{
			printf("flash reset err\r\n\r\n");
		}
	}
	
	SYS_LockReg();
	return rs;
}
