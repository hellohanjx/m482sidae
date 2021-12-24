/*
@mfcrc522射频驱动
@注意：1.复位后，CommandReg寄存器恢复默认值0x20
				2.复位后，波特率恢复9600
				3.读CommandReg寄存器，可得到前一条执行的指令
*/

#include "msg.h"
#include "swipe.h"
#include "global.h"
#include "uart_config.h"
#include "task_swipe.h"


#define RC522_ACK				1
#define RC522_NOACK			0XFFFF


//操作命令的宏定义
#define  READCARD   0xA1
#define  WRITECARD  0xA2
#define  KEYCARD    0xA3
#define  SETTIME    0xA4
#define  SENDID     0xA5


//函数声明
static void ireader_link(uint8_t id);
static uint8_t rc522_init(uint8_t id);

/*****************************
@说明：定义串口操作函数数组
*****************************/
static UART_SEND	uart_send[] 	 = {_uart5_send, 		_uart1_send, 	_uart4_send, 	_uart6_send,	 _uart2_send, 	_uart3_send};
static UART_CONFIG uart_config[] = {_uart5_config, _uart1_config, _uart4_config, _uart6_config, _uart2_config, _uart3_config};


/*
@功能：刷卡器返回数据回调函数
@说明：这个只做共性的检测，比如长度和校验
@参数：rx，返回数据指针
*/
static void callback_card_recv(UART_DATA *rx)
{
	if(rx->len != 0)//包长符合
	{
		uart_sem_send[rx->id]();//释放信号量
	}
}

/*********************************************
RC522模块驱动
*********************************************/

/*
@功能：读RC522寄存器
@参数：id,刷卡器id；addr，寄存器地址
@返回：读出的值，或者返回RC522_NOACK->读失败
*/
static uint16_t reg_read(uint8_t id, uint8_t addr)
{
	UART_DATA *rx, tx;
	uint8_t err;
	
	tx.len = 0;
	tx.id = id;
	tx.buf[tx.len++] = (addr & 0x3f) | 0x80;//写入寄存器地址(最低6位为实际地址，次高为保留为0，最高位为1)
	uart_send[id](&tx, &rx, callback_card_recv);//发送
	err = uart_sem_get[id](10);//等待最少5ms
	if(err == pdTRUE)//收到应答
	{
		return rx->buf[rx->len - 1];
	}
	return RC522_NOACK;
}


/*
@功能：写RC632寄存器
@参数：id,刷卡器ID号；addr:寄存器地址，value:写入的值
@返回值：RC522_ACK，成功；RC522_NOACK，失败
*/
static uint16_t reg_write(uint8_t id, uint8_t addr, uint8_t value)
{  
	UART_DATA *rx, tx;
	uint8_t err;
	
	tx.len = 0;
	tx.id = id;
	tx.buf[tx.len++] = (addr & 0x3f);//写入寄存器地址(最低6位为实际地址，次高为保留为0，最高位为1)
	tx.buf[tx.len++] = value;
	uart_send[id](&tx, &rx, callback_card_recv);//发命令
	err = uart_sem_get[id](10);//等待最少5ms
	if(err == pdTRUE)//收到应答
	{
		if(rx->buf[0] != addr)//应答需要是对应的发送地址
		{
			return RC522_NOACK;
		}
		return RC522_ACK;
	}
	return RC522_NOACK;
}

/*
@功能：RC522寄存器位操作-置1
@参数：addr:寄存器地址, mask:置位掩码
@返回：执行结果->RC522_ACK，RC522_NOACK
@说明：步骤：1.读出来，2.再写入
*/
static uint16_t reg_set(uint8_t id, uint8_t addr, uint8_t mask)  
{
	uint16_t tmp = 0;
	tmp = reg_read(id, addr);
	if(tmp == RC522_NOACK)
	{
		return RC522_NOACK;
	}
	else
	if(tmp == (tmp | mask))//可以加快速度
	{
		return RC522_ACK;
	}
	return reg_write(id, addr, tmp | mask);//set bit mask
}


/*
@功能：清C522寄存器位操作-置0
@参数：addr:寄存器地址，mask:清位掩码
@返回：执行结果->RC522_ACK，RC522_NOACK
@说明：步骤：1.读出来，2.再写入
*/
static uint16_t reg_clear(uint8_t id, uint8_t addr, uint8_t mask)  
{
	uint16_t tmp = 0;
	tmp = reg_read(id, addr);
	if(tmp == RC522_NOACK)
	{
		return RC522_NOACK;
	}
	else//可以加快速度
	if( tmp == (tmp & (~mask)) )
	{
		return RC522_ACK;
	}
	return reg_write(id, addr, tmp & ~mask);//clear bit mask
} 




/*****************************************
@说明：功能函数
*****************************************/
/*
@功能：关闭天线
@返回：执行结果，RC522_ACK，RC522_NOACK
*/
static uint16_t rc522_antenna_off(uint8_t id)
{
	return reg_clear(id, TxControlReg, 0x03);
}

/*
@功能：开启天线  
@返回：执行结果，RC522_ACK，RC522_NOACK
@说明：每次启动或关闭天线之间应至少有1ms的间隔
*/
static uint16_t rc522_antenna_on(uint8_t id)
{
	uint16_t i, rs;
	i = reg_read(id, TxControlReg);
	if( (!(i & 0x03)) && (i != RC522_NOACK) )
	{
		rs = reg_set(id, TxControlReg, 0x03);
		vTaskDelay(3);//天线打开至少需要等待1ms
	}
	return rs;
}

/*
@功能：通过RC522和ISO14443卡通讯
@参数： Command[IN]:RC522命令字
				pIn [IN]:通过RC522发送到卡片的数据
				InLenByte[IN]:发送数据的字节长度
				pOut [OUT]:接收到的卡片返回数据
				*pOutLenBit[OUT]:返回数据的位长度
@返回：执行结果，失败->MI_ERR，成功->MI_OK
*/
static uint16_t PcdComMF522(uint8_t id, uint8_t Command, uint8_t *pIn , uint8_t InLenByte, uint8_t *pOut, uint8_t *pOutLenBit)
{
	char rs = MI_OK;
	uint8_t   irqEn   = 0x00;
	uint8_t   waitFor = 0x00;
	uint8_t   lastBits;
	uint16_t  n;
	uint32_t  i;

	switch(Command)
	{
			case PCD_AUTHENT:   //校验密钥
				irqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
				waitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
				break;
			
			case PCD_TRANSCEIVE://@@发送数据到天线，并且自动激活接收dialup
				irqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
				waitFor = 0x30;		//寻卡等待时候 查询"接收中断标志位"与"空闲中断标志位"
				break;
			
			default:
				break;
	}

	if( reg_write(id, ComIEnReg, irqEn | 0x80) == RC522_NOACK )//@使能中断并设置IRQ引脚电平，开所有中断
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write ComIEnReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处20ms

	if( reg_clear(id, ComIrqReg, 0x80) == RC522_NOACK )//清中断
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("clear ComIrqReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处24ms
	
//	if( reg_write(CommandReg, PCD_IDLE) == RC522_NOACK )//写空闲命令,如果有指令执行，则取消当前指令执行
//	{
//		printf("write CommandReg err 1\r\n");
//		rs = MI_ERR;
//		return MI_ERR;
//	}
	
	if( reg_set(id, FIFOLevelReg, 0x80) == RC522_NOACK )//清除内部FIFO的读和写指针，清ErrReg的BufferOvfl标志
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("set FIFOLevelReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处29ms

	
	for(i = 0; i < InLenByte; i++)
	{
		if( reg_write(id, FIFODataReg, pIn[i]) == RC522_NOACK )//写数据进FIFO，准备发送到卡
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("set FIFODataReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
	}
//cur_time = xTaskGetTickCount() - time;//到此处32ms
	
//	if( reg_write(CommandReg, PCD_RECEIVE) == RC522_NOACK )//@激活接收电路
//	{
//		printf("set CommandReg err\r\n");
//		rs = MI_ERR;
//		return MI_ERR;
//	}
	
	if( reg_write(id, CommandReg, Command) == RC522_NOACK )//从FIFO发送数据到天线，并自动激活接收电路
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("set CommandReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处36ms
//   	 n = reg_read(CommandReg);
	
	if(Command == PCD_TRANSCEIVE)//如果是发送到卡指令
	{
		if( reg_set(id, BitFramingReg, 0x80) == RC522_NOACK )//开始发送FIFO中数据，该位与收发命令使用时才有效 
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("set BitFramingReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
	}
//cur_time = xTaskGetTickCount() - time;//到此处42ms
	/**
	@FIFO缓冲区共64Byte。默认阈值为8字节
	如果接收的数据小于等于8字节，则 Status1Reg->LoAlert 置位（缓冲区已空）；此时CommIRQReg->LoAlertIRq置位（需要写寄存器清空此位）
	**/
	
//delay_ms(20);//为毛这个地方不延时就进入硬件错误
	//根据时钟频率调整，操作M1卡最大等待时间25ms
	
	//等待数据发完
	i = 0;
	do{
		n = reg_read(id, ComIrqReg);//查询事件中断
	}while( (i++ < 25) && !(n & 0x01) && !(n & waitFor) );//定时器未到时，没有发现错误
	
	if(!(n & 0x40))//数据没发完
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("read ComIrqReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处45ms
	
	if( reg_clear(id, BitFramingReg, 0x80) == RC522_NOACK )//清理StartSend位，@停止发送
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("clear BitFramingReg err 1\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处49ms
	
	if(rs == MI_OK)//未超时，未报错
	{   
		uint16_t rs;
//delay_ms(10);//为毛这个地方不延时就进入硬件错误
		rs = reg_read(id, ErrorReg);//显示执行的上个命令的错误状态
//cur_time = xTaskGetTickCount() - time;//到此处49ms
		if(rs == RC522_NOACK)
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("read ErrorReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		if(rs & 0x1B)//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
		{
			rs = MI_ERR;
			
		}
		else//没错误
		{
			rs = MI_OK;
			
			if (n & irqEn & 0x01)//是否发生定时器中断，@定时器减到0
			{   
				rs = MI_NOTAGERR;
			}
			
			if(Command == PCD_TRANSCEIVE)//收发命令
			{
				n = reg_read(id, FIFOLevelReg);//读FIFO中保存的字节数
//cur_time = xTaskGetTickCount() - time;//到此处60ms
//				{//测试代码
//					char tmp[10];
//					//cur_time = xTaskGetTickCount() - time;//计算指令执行时间

//					sprintf(tmp, "%u", n);
//					printf("receive data == ");
//					printf(tmp);
//					printf("\r\n");
//				}
				
				rs = reg_read(id, ControlReg);//读控制寄存器，@获取最后收到的有效位数目（如果为0，则整个字节有效）
//cur_time = xTaskGetTickCount() - time;//到此处63ms

				if(rs == RC522_NOACK)
				{
					#if(RC_522_LOG)
					char tmp = '1'+id;
					printf(&tmp);printf(" = ");
					printf("read ControlReg err\r\n");
					#endif
					rs = MI_ERR;
					return MI_ERR;
				}
				
				lastBits = rs & 0x07;
				
				if(lastBits)//最后一字节有错误位
				{   
					*pOutLenBit = (n - 1)*8 + lastBits; //N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数 
				} //获取有效位数据 //最后一个字节有无效数据位的时候，需要排除
				else
				{   
					*pOutLenBit = n*8;//最后接收到的字节整个字节有效
				} //所以采用bit计算
				
				if(n == 0)
				{   
					n = 1;
				}
				if(n > MAXRLEN)
				{  
					n = MAXRLEN;   
				}
				
				for(i = 0; i < n; i++)
				{   
					rs = reg_read(id, FIFODataReg);//读FIFO中的数据
//cur_time = xTaskGetTickCount() - time;//到此处66ms
					if(rs == RC522_NOACK)
					{
						#if(RC_522_LOG)
						char tmp = '1'+id;
						printf(&tmp);printf(" = ");
						printf("read FIFODataReg err\r\n");
						#endif
						rs = MI_ERR;
						return MI_ERR;
					}
					pOut[i] = (uint8_t)rs; 
				}
			}
		}
	}

	if( reg_set(id, ControlReg, 0x80) == RC522_NOACK )//停止定时器
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("set ControlReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此处74ms

//	if( reg_write(CommandReg, PCD_IDLE) == RC522_NOACK )//空闲命令，@耗时6ms
//	{
//		printf("write CommandReg err\r\n");
//		rs = MI_ERR;
//		return MI_ERR;
//	}
	
	return rs;
}

/*
@功能：寻卡
@参数: req_code[IN]:寻卡方式
                0x52 = 寻感应区内所有符合14443A标准的卡
                0x26 = 寻未进入休眠状态的卡
       pTagType[OUT]：卡片类型代码
                0x4400 = Mifare_UltraLight
                0x0400 = Mifare_One(S50)
                0x0200 = Mifare_One(S70)
                0x0800 = Mifare_Pro(X)
                0x4403 = Mifare_DESFire
@返回：执行结果，失败->MI_ERR，成功->MI_OK
*/

static uint16_t PcdRequest(uint8_t id, uint8_t req_code, uint8_t *pTagType)
{
	char rs = MI_OK;  
	uint8_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN];

//	if( reg_clear(Status2Reg, 0x08) == RC522_NOACK )//清理 MIFARECyptol 标志,@用来进行 MFAuthent 认证
//	{
//		printf("clear Status2Reg err\r\n");
//		rs = MI_ERR;
//		return MI_ERR;
//	}
	
	if( reg_write(id, BitFramingReg, 0x07) == RC522_NOACK )//最后一个字节发送7bit
	{
		icreader_state_change(0, CARD_ERR);
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write BitFramingReg err\r\n");
		#endif
		rs = MI_ERR;
		
		//复位刷卡头
		if(rc522_init(id))//重新初始化设备
		{
			ireader_link(id);
		}
		return MI_ERR;
	}
	
//cur_time = xTaskGetTickCount() - time;//计算刷卡时间（@目前17ms）

//	if( rc522_antenna_on() == RC522_NOACK )//打开天线
//	{
//		icreader_state_change(AB_NORMAL);
//		printf("set TxControlReg err\r\n");
//		rs = MI_ERR;
//		return MI_ERR;
//	}
	

	ucComMF522Buf[0] = req_code;//待发送给卡片的命令
	rs = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);//寻卡
	if ((rs == MI_OK) && (unLen == 0x10))//寻卡成功返回卡类型 
	{    
//		printf("get card =================\r\n");
		pTagType[0] = ucComMF522Buf[0];
		pTagType[1] = ucComMF522Buf[1];
	}
	else
	{
//		printf("PcdRequest err\r\n");
		rs = MI_ERR;   
	}
	
	return rs;//成功
}

/*
@功能：防冲撞
@参数: pSnr[OUT]:卡片序列号，4字节
@返回：执行结果，失败->MI_ERR，成功->MI_OK
*/
static uint16_t PcdAnticoll(uint8_t id, uint8_t *pSnr)
{
    char rs = MI_OK;
    uint8_t i, snr_check = 0;
    uint8_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN];
    

//    if( reg_clear(Status2Reg,0x08) == RC522_NOACK )
//		{
//			printf("clear Status2Reg err\r\n");
//			rs = MI_ERR;
//			return MI_ERR;
//		}

		if( reg_write(id, BitFramingReg, 0x00) == RC522_NOACK )//最后一字节发送8bit
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write BitFramingReg err 2\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}

//		if( reg_clear(CollReg, 0x80) == RC522_NOACK )//ValuesAfterColl 设置为0，则所有接收的位在冲突后将被清除
//		{
//			printf("clear CollReg err\r\n");
//			rs = MI_ERR;
//			return MI_ERR;
//		}
		
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;
    rs = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);
    if(rs == MI_OK)
    {
			for(i = 0; i < 4; i++)
			{   
				*(pSnr+i)  = ucComMF522Buf[i];
				snr_check ^= ucComMF522Buf[i];
			}
			if(snr_check != ucComMF522Buf[i])
			{
//				printf("PcdAnticoll err\r\n");
				rs = MI_ERR;
			}
    }
    
//    if( reg_set(CollReg, 0x80) == RC522_NOACK )
//		{
//			printf("set CollReg err\r\n");
//			rs = MI_ERR;
//			return MI_ERR;
//		}
    return rs;
}

/*
@功能：工作逻辑->对寻卡、防冲撞、选卡、发送卡号、读卡、写卡、修改密码进行操作
@参数：buf，缓冲
@返回：执行结果，失败->MI_ERR，成功->MI_OK
*/
static uint8_t rc522_work(uint8_t id, uint8_t *buf)
{
  char status;

	if( rc522_antenna_on(id) == RC522_NOACK )//开天线
	{
		icreader_state_change(0, CARD_ERR);
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("set TxControlReg err\r\n");
		#endif
		return MI_ERR;
	}
//cur_time = xTaskGetTickCount() - time;//到此14ms
	
//  status = PcdRequest(PICC_REQIDL, buf);//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节
	status = PcdRequest(id, PICC_REQALL, buf);//寻找天线内所有卡；@返回的buf前2字节为卡类型
	if(status != MI_OK)
		return MI_ERR;
	
//cur_time = xTaskGetTickCount() - time;//到此73ms
	
  status = PcdAnticoll(id, &buf[2]);//防冲撞，返回卡的序列号 4字节
  if(status != MI_OK)
		return MI_ERR;
	
//cur_time = xTaskGetTickCount() - time;//到此146ms
	
	return MI_OK;
}


/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
//void CalulateCRC(uint8_t *pIn ,uint8_t   len,uint8_t *pOut )
//{
//    uint8_t   i,n,j;
//    reg_clear(DivIrqReg,0x04);
//    reg_write(CommandReg,PCD_IDLE);
//    reg_set(FIFOLevelReg,0x80);
//    for (i=0; i<len; i++)
//    {   reg_write(FIFODataReg, *(pIn +i));   }
//    reg_write(CommandReg, PCD_CALCCRC);
//    i = 0xFF;
//    do 
//    {
//        n = reg_read(DivIrqReg);
//		//	printf("i =%d",i);
//        i--;
//    }
//    while ((i!=0) && !(n&0x04));
//    pOut [0] = reg_read(CRCResultRegL);
//    pOut [1] = reg_read(CRCResultRegM);
//}



/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
//static char PcdHalt(void)
//{
//    uint8_t   status;
//    uint8_t   unLen;
//    uint8_t   ucComMF522Buf[MAXRLEN]; 

//    ucComMF522Buf[0] = PICC_HALT;
//    ucComMF522Buf[1] = 0;
//    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
// 
//    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

//    return MI_OK;
//}



/*
@功能：以规定的波特率复位
@返回：TRUE,成功；FALSE，失败
*/
static uint8_t reset_baud(uint8_t id, uint32_t baud)
{
	uint16_t rs, i;

	uart_config[id](baud);//重置波特率
//	reg_write(CommandReg, 0x55);//软复位，@这是文档要求的，@但是会造成天线线圈的感应距离远可以，近不行的问题
	reg_write(id, CommandReg, PCD_RESETPHASE);//软复位
	
	i = 0;
	do{
		rs = reg_read(id, RFU00);
	}while(i++ < 10 && rs == RC522_NOACK);
	
	i = 0;
	do{
		rs = reg_read(id, CommandReg);
	}while(i++ < 10 && rs == RC522_NOACK);
	
	if(rs & 0x10)//@PowerDown == 0,复位已完成
	{
		if(baud == 115200)
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("ireader 115200 reset err\r\n");
			#endif
		}
		return FALSE;
	}
	if(baud == 9600)
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("ireader 9600 reset err\r\n");
		#endif
	}
	return TRUE;
	
}

/*
@功能：复位RC522
@返回：执行结果，失败->MI_ERR，成功->MI_OK
*/
static uint16_t rc522_reset(uint8_t id)
{
	uint16_t rs = MI_OK, i = 0;
	
	vTaskDelay(100);//等待硬件复位

	/*
	@说明：这里先用115200复位的话，无法通过
	@注意：这个复位序列不要更改
	*/
//	if(!reset_baud(115200))//使用115200波特率复位->复位失败
	{
		if(reset_baud(id, 9600))//9600复位->复位成功
		{
			do{
				rs = reg_write(id, SerialSpeedReg, 0x7A);//串口波特率改为115200
			}while(i++ < 20 && rs == RC522_NOACK);
			if(rs == RC522_NOACK)
			{
				#if(RC_522_LOG)
				char tmp = '1'+id;
				printf(&tmp);printf(" = ");
				printf("write SerialSpeedReg err\r\n");
				#endif
				return MI_ERR;
			}
			else
			{
				uart_config[id](115200);
			}
		}
		else
		{
			if(!reset_baud(id, 115200))
				return MI_ERR;
		}
	}
		
	
	i = 0;
	do{
		rs = reg_write(id, ModeReg, 0x3D);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs == RC522_NOACK)	//定义发送和接收常用模式（如果RF场产生，则TxWaitRF置位;SIGIN管脚高电平有效；CRC初始值0x6363）
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write ModeReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	
	i = 0;
	do{
		rs = reg_write(id, TReloadRegL, 30);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//16位定时器低位
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write TReloadRegL err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	
	i = 0;
	do{
		rs = reg_write(id, TReloadRegH, 0);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//16位定时器高位
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write TReloadRegH err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	
	i = 0;
	do{
		rs = reg_write(id, TModeReg, 0x8D);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//定义内部定时器的设置
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write TModeReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	
	i = 0;
	do{
		rs = reg_write(id, TPrescalerReg, 0x3E);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//设置定时器分频系数
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write TPrescalerReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	
	i = 0;
	do{
		rs = reg_write(id, TxAutoReg, 0x40);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs == RC522_NOACK )	//调制发送信号为100%ASK
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write TxAutoReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	return rs;	//成功
}


/*
@功能：设置RC522的工作方式
@参数：type，工作方式
@返回：执行结果，失败->MI_ERR，成功->MI_OK
*/
static uint16_t M500PcdConfigISOType(uint8_t id, uint8_t type)
{
	uint8_t rs = MI_OK;
	
	if (type == 'A')//ISO14443_A
	{ 
		if( reg_clear(id, Status2Reg, 0x08) == RC522_NOACK )	//TempSensOff 置位，内部温度传感器关断时该位置位
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("clear Status2Reg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( reg_write(id, ModeReg, 0x3D) == RC522_NOACK )//3F //@1.如果RF场（RF field）产生，则TxWaitRF置位，发送器只能在此时被启动，2.SIGIN管脚高电平有效，3.CRC 6363
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write ModeReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( reg_write(id, RxSelReg, 0x86) == RC522_NOACK )//84	//接收设置：1.内部模拟部分的调制信号，2.， 接收器的启动会延迟RxWait个位时钟。在这段帧保护时间内，Rx管脚上的所有信号都被忽略（@接收命令不受这个指令控制）
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write RxSelReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( reg_write(id, RFCfgReg, 0x7F) == RC522_NOACK )  //4F //配置接收增益：目前设置为最大
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write RFCfgReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}

		if( reg_write(id, TReloadRegL, 30) == RC522_NOACK )//定时器低位
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TReloadRegL err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( reg_write(id, TReloadRegH, 0) == RC522_NOACK )//定时器高位
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TReloadRegH err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( reg_write(id, TModeReg, 0x8D) == RC522_NOACK )//定时器设置，TAuto置位：定时器在所有速率的发送结束时自动启动。在接收到第一个数据位后定时器立刻停止运行。TPrescaler高四位1101
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TModeReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( reg_write(id, TPrescalerReg, 0x3E) == RC522_NOACK )//TPrescaler的低8位:公式：fTimer＝6.78MHz / TPreScaler（计算定时器频率）
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TPrescalerReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
				
//		if( rc522_antenna_on() == RC522_NOACK )//开天线 TX1与TX2
//		{
//			printf("rc522_antenna_on err_1\r\n");
//			rs = MI_ERR;
//			return MI_ERR;
//		}
//		delay_ms(10);
	}
	else//类型不对
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("card type err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	return rs;
}






/*
@功能：RC522初始化
@返回：执行结果，失败->FALSE，成功->TRUE
*/
static uint8_t rc522_init(uint8_t id)
{
	if( rc522_reset(id) == MI_ERR)//复位RC522
	{
		return FALSE;
	}
	
	if( rc522_antenna_off(id) == RC522_NOACK )//关闭天线
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("rc522 close anttenna err\r\n");
		#endif
		return FALSE;
	}
	
//	if(	reg_set (GsNReg , 0x27) == RC522_NOACK )//设置TX1,TX2电导
//	{
//		printf("set GsNReg anttenna err\r\n");
//		return FALSE;
//	}
	
//	if(rc522_antenna_on() == RC522_NOACK)//开启天线
//	{
//		printf("rc522 open anttenna err\r\n");
//		return FALSE;
//	}

	if( M500PcdConfigISOType(id, 'A') == MI_ERR )
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("rc522 set work mode err\r\n");
		#endif
		return FALSE;
	}
                                                                                                                                                                                	
//以下为测试代码
//	while(1)
//	{
//		uint8_t buf[100] = { 0};
//		uint32_t card;
//		char tmp[10];
//		
//		time = xTaskGetTickCount();
//		if(rc522_work(buf) == MI_OK)
//		{
//			card = buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24);
//			sprintf(tmp, "%010u", card);
//			printf(tmp);
//			printf("\r\n");
//		}
//	}
	
	return TRUE;
}
/*
@功能：查找设备
@说明：这里用查找设备版本来确定是否有设备
*/
static void ireader_link(uint8_t id)
{
	uint16_t rs;
	rs = reg_read(id, VersionReg);
	if(rs == RC522_NOACK)
		icreader_state_change(id, CARD_ERR);
	else
		icreader_state_change(id, CARD_NORMAL);
}


/******************************************************************************************************************
@对外接口
******************************************************************************************************************/
/*
@功能：命令序列初始化
*/
void ireader_init(uint8_t id)
{
	if(rc522_init(id))
	{
		ireader_link(id);
	}
	else
	{
		icreader_state_change(id, CARD_ERR); 
	}
}

/*
@功能：读卡指令
@返回：执行结果，失败->FALSE，成功->TRUE
@说明：目前只用来读物理卡号ID
*/
uint8_t ireader_read(uint8_t id)
{
	uint8_t buf[30] = {0}, rs = FALSE, i;
	
//time = xTaskGetTickCount();
	
	if(rc522_work(id, buf) == MI_OK)//寻到卡
	{
		for(i = 0; i < 10; i++)
			class_global.ireader[id].card.physic_char[i] = 0;
		
		
//cur_time = xTaskGetTickCount() - time;//计算刷卡时间（@目前146ms）
		
//测试代码：显示寻卡时间
//		{
//			char tmp[10] = {0};
//			sprintf(tmp, "%u", cur_time);
//			printf("find cost == ");
//			printf(tmp);
//			printf("\r\n");
//		}
		
		sprintf((char*)class_global.ireader[id].card.physic_char, "%010u", (buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24)) );
		class_global.ireader[id].card.physic_char[10] = 0;//卡号结束符
		rs = TRUE;
	}
	
//	delay_ms(1);
	rc522_antenna_off(id);//关天线
//	delay_ms(100);
	return rs;
}

