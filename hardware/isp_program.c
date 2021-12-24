#include "isp_program.h"
#include "global.h"
#include "user_config.h"


/*
@功能：设置用户数据基地址
@参数：u32DFBA，待设置基地址
@返回：0成功，-1失败
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
@功能：读取芯片唯一识别码
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
@flash标志位操作
*********************************************************/
/*
@功能：读取内部flash标志位
@返回：标志值
*/
static uint32_t flash_flag_get(void)
{
	SYS_UnlockReg();//解锁寄存器
	return FMC_Read(FLASH_ADDR_FLAG);
}

/*
@功能：写内部flash标志位
@参数：标志值
@返回：TRUE，写成功；FALSE，写失败
*/
static uint8_t flash_flag_set(uint32_t val)
{
	FMC_Write(FLASH_ADDR_FLAG, val);
	if( val == FMC_Read(FLASH_ADDR_FLAG) )
		return TRUE;
	return FALSE;
}


/*********************************************************
@flash "偏移表"操作
*********************************************************/
/*
@功能：根据偏移地址表寻找未写的值对应的偏移
@返回：0~31，偏移值，0xff，已写完
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
@功能：写偏移地址表
@参数：addr，偏移地址表地址；offset，偏移值0~31
@返回：TRUE,成功；FLASE，失败
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
@功能：参数无法写入时的恢复操作
@返回：TRUE，成功；FALSE，失败
*********************************************************/
static uint8_t flash_param_recovery(void)
{
	uint32_t id, ip, port, factory_en, card_type, point_bit, price_bit, max_price;
	
	if(flash_flag_get() == 0xffffffff)
	{
		//标志位为空，此块刚擦除
		printf("flash sec is free\r\n\r\n");
		return FALSE;
	}
	
	//读数据
	id 						= flash_param_get(FLASH_LIST_Id, 						FLASH_ADDR_Id);					//1-机器id
	ip						= flash_param_get(FLASH_LIST_Ip, 						FLASH_ADDR_Ip);						//2-机器ip
	port 					= flash_param_get(FLASH_LIST_Port, 					FLASH_ADDR_Port);					//3-端口号
	factory_en 		= flash_param_get(FLASH_LIST_FactoryEn, 		FLASH_ADDR_FactoryEn);		//4-工厂模式
	card_type 		= flash_param_get(FLASH_LIST_CardType, 			FLASH_ADDR_CardType);			//5-刷卡器类型
	point_bit 		= flash_param_get(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit);			//6-小数点位数
	price_bit 		= flash_param_get(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit);			//7-价格位数
	max_price 		= flash_param_get(FLASH_ADDR_PriceMax, 			FLASH_ADDR_PriceMax);			//8-最大价格
	
	FMC_Erase(USR_DATA_BASE_ADDR);//擦除整块
	
	//回写数据
	flash_flag_set(FLASH_VAL_FLAG);//写标志
	flash_param_set(FLASH_LIST_Id, 						FLASH_ADDR_Id, 				id);						//1-机器id
	flash_param_set(FLASH_LIST_Ip, 						FLASH_ADDR_Ip, 					ip);						//1-ip
	flash_param_set(FLASH_LIST_Port, 					FLASH_ADDR_Port, 				port);					//3-端口号
	flash_param_set(FLASH_LIST_FactoryEn, 		FLASH_ADDR_FactoryEn, 	factory_en);		//4-工厂模式
	flash_param_set(FLASH_LIST_CardType, 			FLASH_ADDR_CardType, 		card_type);			//5-刷卡器类型
	flash_param_set(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit,		point_bit);			//6-小数点位数
	flash_param_set(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit,		price_bit);			//7-价格位数
	flash_param_set(FLASH_ADDR_PriceMax, 			FLASH_ADDR_PriceMax, 		max_price);			//8-最大价格

	
	//验证回写是否正确
	if(	id 						== flash_param_get(FLASH_LIST_Id, 						FLASH_ADDR_Id)					//1-机器id
	&&	ip						== flash_param_get(FLASH_LIST_Ip, 						FLASH_ADDR_Ip)						//1-ip
	&&	port 					== flash_param_get(FLASH_LIST_Port, 					FLASH_ADDR_Port)					//3-端口号
	&&	factory_en 		== flash_param_get(FLASH_LIST_FactoryEn, 			FLASH_ADDR_FactoryEn)			//4-工厂模式
	&&	card_type 		== flash_param_get(FLASH_LIST_CardType, 			FLASH_ADDR_CardType)			//5-刷卡器类型
	&&	point_bit 		== flash_param_get(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit)			//6-小数点位数
	&&	price_bit 		== flash_param_get(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit)			//7-价格位数
	&&	max_price 		== flash_param_get(FLASH_ADDR_PriceMax, 			FLASH_ADDR_PriceMax)			//8-最大价格
	&&  FLASH_VAL_FLAG == flash_flag_get() )
	{
		return TRUE;
	}
	printf("flash recovery err\r\n\r\n");
	return FALSE;
}




/*****************************************************************************************************************************
********************************************************** 对外接口 **********************************************************
*****************************************************************************************************************************/
/*
@功能：读参数
@参数：list_addr=偏移表；data_addr=待写值地址
@返回：参数值
*/
uint32_t flash_param_get(uint32_t list_addr, uint32_t data_addr)
{
	uint32_t offset;
	uint32_t rs = 0;
		
	SYS_UnlockReg();//解锁寄存器
	
	offset = flash_freeaddr_get(list_addr);//获取偏移量
	if(offset == 0xff)//无有效数据，说明不能继续写了，空间用完了,但是读的话最后一位是有效的
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
@功能：写参数
@参数：list_addr，偏移表；data_addr，待写值地址；data_size，待写数据宽度；val，待写数据值
@返回：TRUE，写成功；FALSE，写失败
*/
uint8_t flash_param_set(uint32_t list_addr, uint32_t data_addr, uint32_t val)
{
	uint8_t rs = TRUE;
	uint32_t offset;
	
	SYS_UnlockReg();//解锁寄存器
	
	offset = flash_freeaddr_get(list_addr);//寻找可写地址
	if(offset == 0xff)//无有效数据，说明不能继续写了，空间用完了
	{
		printf("flash space full\r\n");
		rs = flash_param_recovery();
	}
	
	if(rs)
	{
		rs = FALSE;
		
		//写
		FMC_Write((data_addr + offset*4), val);//@setup-1：写值
		flash_freeaddr_set(list_addr);//@setup-2:写偏移表
		
		//验证
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
@功能：用户数据区初始化
@返回：TRUE=成功
*/
uint8_t _isp_config(void)
{
//	SYS_UnlockReg();//开寄存器锁，@@这里需要打开寄存器锁
	FMC_Open();//ISP功能使能
	if (set_data_flash_base(USR_DATA_BASE_ADDR) < 0)
	{
		printf("Failed to set Data Flash base address!\n");          /* error message */
		return FALSE;
	}
	
	read_unique_id((uint32_t*)class_global.sys.unique_id);//读取唯一id
	return TRUE;
}


/*
@功能：复位参数块
@返回：TRUE，成功；FALSE，失败
*/
uint8_t reset_param_block(void)
{
	uint8_t rs = FALSE;
	SYS_UnlockReg();//解锁寄存器
	
	if(flash_flag_get() != FLASH_VAL_FLAG)//标志不符合
	{
		printf("flash block reset\r\n\r\n");
		FMC_Erase(USR_DATA_BASE_ADDR);//擦除整块
		
		//写默认数据
		flash_flag_set(FLASH_VAL_FLAG);																												//写标志位
		flash_param_set(FLASH_LIST_Id, 						FLASH_ADDR_Id, 				 		DEF_FLASH_Id);		//1-id
		flash_param_set(FLASH_LIST_Ip,	 					FLASH_ADDR_Ip,	 				 	DEF_FLASH_Ip);		//2-IP
		flash_param_set(FLASH_LIST_Port, 					FLASH_ADDR_Port, 					DEF_FLASH_Port);	//3-端口
		flash_param_set(FLASH_LIST_FactoryEn, 		FLASH_ADDR_FactoryEn, 		DEF_FLASH_FactoryEn);//4-工厂模式
		flash_param_set(FLASH_LIST_CardType, 			FLASH_ADDR_CardType, 			DEF_FLASH_CardType);//5-刷卡器类型
		flash_param_set(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit,		 	DEF_FLASH_PointBit);//6-小数位数
		flash_param_set(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit,			DEF_FLASH_PriceBit);//7-价格位数
		flash_param_set(FLASH_LIST_PriceMax, 			FLASH_ADDR_PriceMax, 			DEF_FLASH_PriceMax);		//8-最大价格
			
		
		//校验写入数据
		if(	DEF_FLASH_Id 						== flash_param_get(FLASH_LIST_Id,							FLASH_ADDR_Id)				//1-id
		&&	DEF_FLASH_Ip 						== flash_param_get(FLASH_LIST_Ip, 						FLASH_ADDR_Ip)				//2-IP
		&&	DEF_FLASH_Port 					== flash_param_get(FLASH_LIST_Port, 					FLASH_ADDR_Port)			//3-端口
		&&	DEF_FLASH_FactoryEn 		== flash_param_get(FLASH_LIST_FactoryEn, 			FLASH_ADDR_FactoryEn)	//4-工厂模式
		&&	DEF_FLASH_CardType 			== flash_param_get(FLASH_LIST_CardType, 			FLASH_ADDR_CardType)	//5-刷卡器类型
		&&	DEF_FLASH_PointBit 			== flash_param_get(FLASH_LIST_PointBit, 			FLASH_ADDR_PointBit)		//6-小数位数
		&&	DEF_FLASH_PriceBit 			== flash_param_get(FLASH_LIST_PriceBit, 			FLASH_ADDR_PriceBit)//7-价格位数
		&&	DEF_FLASH_PriceMax 			== flash_param_get(FLASH_LIST_PriceMax, 			FLASH_ADDR_PriceMax)		//8-最大价格
		&&  FLASH_VAL_FLAG 					== flash_flag_get()																										//标志位
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
