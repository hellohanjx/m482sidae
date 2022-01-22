/*
@mfcrc522��Ƶ����
@ע�⣺1.��λ��CommandReg�Ĵ����ָ�Ĭ��ֵ0x20
				2.��λ�󣬲����ʻָ�9600
				3.��CommandReg�Ĵ������ɵõ�ǰһ��ִ�е�ָ��
*/

#include "msg.h"
#include "swipe.h"
#include "global.h"
#include "uart_config.h"
#include "task_swipe.h"
#include "commucation.h"
#include "swipe_led.h"
#include "string.h"

#define RC522_ACK				1
#define RC522_NOACK			0XFFFF


//��������ĺ궨��
#define  READCARD   0xA1
#define  WRITECARD  0xA2
#define  KEYCARD    0xA3
#define  SETTIME    0xA4
#define  SENDID     0xA5


//��������
static void ireader_link(uint8_t id);
static uint8_t rc522_init(uint8_t id);

/*****************************
@˵�������崮�ڲ�����������
*****************************/
static UART_SEND	uart_send[] 	 = {_uart5_send, 		_uart1_send, 	_uart4_send, 	_uart6_send,	 _uart2_send, 	_uart3_send};
static UART_CONFIG uart_config[] = {_uart5_config, _uart1_config, _uart4_config, _uart6_config, _uart2_config, _uart3_config};


/*
@���ܣ�������״̬�㱨
@������id=��������[0~5]��state=״ֵ̬
*/
static void icreader_state_change(uint8_t id, uint8_t state)
{
	char tmp = id+'1';
	if(class_global.ireader[id].equ.state != state)
	{
		report_state_change(class_global.sys.factory_en, 0, id, state);//�㱨ˢ����״̬�ı�
		class_global.ireader[id].equ.state = state;
		if(state)
		{
			swip_led_set(id, LED_RED, LED_OPEN);//�����
			swip_led_set(id, LED_BLUE, LED_CLOSE);//������
			printf("ireader_");
			printf(&tmp);
			printf(" normal\r\n");
		}
		else
		{
			swip_led_set(id, LED_RED, LED_CLOSE);//�����
			swip_led_set(id, LED_BLUE, LED_CLOSE);//������
			printf("ireader_");
			printf(&tmp);
			printf(" err\r\n");
		}
	}
}



/*
@���ܣ�ˢ�����������ݻص�����
@˵�������ֻ�����Եļ�⣬���糤�Ⱥ�У��
@������rx����������ָ��
*/
static void callback_card_recv(UART_DATA *rx)
{
	if(rx->len != 0)//��������
	{
		uart_sem_send[rx->id]();//�ͷ��ź���
	}
}

/*******************************************************************************************************************
@@ RC522 �Ĵ�����������
@@ ��/д/��λ/λ���� 
*******************************************************************************************************************/
/*
@���ܣ��� RC522 �Ĵ���
@������id,ˢ����id��addr���Ĵ�����ַ
@���أ�������ֵ�����߷��� RC522_NOACK ����ʧ��
*/
static uint16_t _reg_read(uint8_t id, uint8_t addr)
{
	UART_DATA *rx, tx;
	uint8_t err;
	
	tx.len = 0;
	tx.id = id;
	tx.buf[tx.len++] = (addr & 0x3f) | 0x80;//д��Ĵ�����ַ(���6λΪʵ�ʵ�ַ���θ�Ϊ����Ϊ0�����λΪ1)
	uart_send[id](&tx, &rx, callback_card_recv);//����
	err = uart_sem_get[id](26);//�ȴ�����5ms
	if(err == pdTRUE)
	{
		return rx->buf[rx->len - 1];
	}
	return RC522_NOACK;
}


/*
@���ܣ�д RC522 �Ĵ���
@������id=ˢ����ID�ţ�addr=�Ĵ�����ַ��value=д���ֵ
@����ֵ��RC522_ACK = �ɹ���RC522_NOACK = ʧ��
*/
static uint16_t _reg_write(uint8_t id, uint8_t addr, uint8_t value)
{  
	UART_DATA *rx, tx;
	uint8_t err;
	
	tx.len = 0;
	tx.id = id;
	tx.buf[tx.len++] = (addr & 0x3f);//д��Ĵ�����ַ(���6λΪʵ�ʵ�ַ���θ�Ϊ����Ϊ0�����λΪ1)
	tx.buf[tx.len++] = value;
	uart_send[id](&tx, &rx, callback_card_recv);//������
	err = uart_sem_get[id](26);//�ȴ�����5ms
	if(err == pdTRUE)
	{
		if(rx->buf[0] != addr)//Ӧ���Ӧ���͵�ַ
		{
			return RC522_NOACK;
		}
		return RC522_ACK;
	}
	return RC522_NOACK;
}

/*
@���ܣ�RC522�Ĵ���λ����-��1
@������addr:�Ĵ�����ַ, mask:��λ����
@���أ�ִ�н��->RC522_ACK��RC522_NOACK
@˵�������裺1.��������2.��д��
*/
static uint16_t _reg_set(uint8_t id, uint8_t addr, uint8_t mask)  
{
	uint16_t tmp = 0;
	tmp = _reg_read(id, addr);
	if(tmp == RC522_NOACK)
	{
		return RC522_NOACK;
	}
	else
	if(tmp == (tmp | mask))//���ֵһ�����Ͳ����ٴ�д��
	{
		return RC522_ACK;
	}
	return _reg_write(id, addr, tmp | mask);//set bit mask
}


/*
@���ܣ���C522�Ĵ���λ����-��0
@������addr:�Ĵ�����ַ��mask:��λ����
@���أ�ִ�н��->RC522_ACK��RC522_NOACK
@˵�������裺1.��������2.��д��
*/
static uint16_t _reg_clear(uint8_t id, uint8_t addr, uint8_t mask)  
{
	uint16_t tmp = 0;
	tmp = _reg_read(id, addr);
	if(tmp == RC522_NOACK)
	{
		return RC522_NOACK;
	}
	else
	if( tmp == (tmp & (~mask)) )//���ֵһ�����Ͳ����ٴ�д��
	{
		return RC522_ACK;
	}
	return _reg_write(id, addr, tmp & ~mask);//clear bit mask
} 




/*******************************************************************************************************************
@@ ����ģ�麯��
@@ ˵����ÿ���������һ������
*******************************************************************************************************************/
/////////////////////////////////////////////////////////////////////
//��MF522����CRC16����
/////////////////////////////////////////////////////////////////////
void CalulateCRC(uint8_t id, unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    uint8_t i,n;
    _reg_clear(id, DivIrqReg, 0x04);
    _reg_write(id, CommandReg, PCD_IDLE);
    _reg_set(id, FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {   
			_reg_write(id, FIFODataReg, *(pIndata+i));
		}
    _reg_write(id, CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = _reg_read(id, DivIrqReg);
        i--;
    }
    while( (i!=0) && !(n&0x04) );
    pOutData[0] = _reg_read(id, CRCResultRegL);
    pOutData[1] = _reg_read(id, CRCResultRegM);
}



/////////////////////////////////////////////////////////////////////
// @���ܣ��ر�����
// @���أ�ִ�н����RC522_ACK��RC522_NOACK
/////////////////////////////////////////////////////////////////////
static uint16_t rc522_antenna_off(uint8_t id)
{
	return _reg_clear(id, TxControlReg, 0x03);
}

/////////////////////////////////////////////////////////////////////
// @���ܣ���������  
// @���أ�ִ�н����RC522_ACK��RC522_NOACK
// @˵����ÿ��������ر�����֮��Ӧ������1ms�ļ��
/////////////////////////////////////////////////////////////////////
static uint16_t rc522_antenna_on(uint8_t id)
{
	uint16_t i, rs = RC522_NOACK;
	i = _reg_read(id, TxControlReg);
	if( (!(i & 0x03)) && (i != RC522_NOACK) )
	{
		rs = _reg_set(id, TxControlReg, 0x03);
		vTaskDelay(3);//���ߴ�������Ҫ�ȴ�1ms
	}
	return rs;
}

/////////////////////////////////////////////////////////////////////
// @���ܣ�ͨ��RC522��ISO14443��ͨѶ
// @������ Command[IN]:RC522������
//				pIn [IN]:ͨ��RC522���͵���Ƭ������
//				InLenByte[IN]:�������ݵ��ֽڳ���
//				pOut [OUT]:���յ��Ŀ�Ƭ��������
//				*pOutLenBit[OUT]:�������ݵ�λ����
// @���أ�ִ�н����ʧ��->MI_ERR���ɹ�->MI_OK
/////////////////////////////////////////////////////////////////////
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
			case PCD_AUTHENT:   //У����Կ
				irqEn   = 0x12;		//��������ж�����ErrIEn  ��������ж�IdleIEn
				waitFor = 0x10;		//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
				break;
			
			case PCD_TRANSCEIVE://@@�������ݵ����ߣ������Զ��������dialup
				irqEn   = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
				waitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ"�����жϱ�־λ"��"�����жϱ�־λ"
				break;
			
			default:
				break;
	}

	if( _reg_write(id, ComIEnReg, irqEn | 0x80) == RC522_NOACK )//@ʹ���жϲ�����IRQ���ŵ�ƽ���������ж�
	{
		rs = MI_ERR;
		return MI_ERR;
	}

	if( _reg_clear(id, ComIrqReg, 0x80) == RC522_NOACK )//���ж�
	{
		rs = MI_ERR;
		return MI_ERR;
	}
	
	///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
	if( _reg_write(id, CommandReg, PCD_IDLE) == RC522_NOACK )//д��������,�����ָ��ִ�У���ȡ����ǰָ��ִ��
	{
		printf("write CommandReg err 1\r\n");
		rs = MI_ERR;
		return MI_ERR;
	}
	///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
	
	if( _reg_set(id, FIFOLevelReg, 0x80) == RC522_NOACK )//����ڲ�FIFO�Ķ���дָ�룬��ErrReg��BufferOvfl��־
	{
		rs = MI_ERR;
		return MI_ERR;
	}
	
	for(i = 0; i < InLenByte; i++)
	{
		if( _reg_write(id, FIFODataReg, pIn[i]) == RC522_NOACK )//д���ݽ�FIFO��׼�����͵���
		{
			rs = MI_ERR;
			return MI_ERR;
		}
	}

	
	///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
	if( _reg_write(id, CommandReg, PCD_RECEIVE) == RC522_NOACK )//@������յ�·
	{
		rs = MI_ERR;
		return MI_ERR;
	}
	///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////

	if( _reg_write(id, CommandReg, Command) == RC522_NOACK )//��FIFO�������ݵ����ߣ����Զ�������յ�·
	{
		rs = MI_ERR;
		return MI_ERR;
	}

//   	 n = _reg_read(CommandReg);
	
	if(Command == PCD_TRANSCEIVE)//����Ƿ��͵���ָ��
	{
		if( _reg_set(id, BitFramingReg, 0x80) == RC522_NOACK )//��ʼ����FIFO�����ݣ���λ���շ�����ʹ��ʱ����Ч 
		{
			rs = MI_ERR;
			return MI_ERR;
		}
	}
	/**
	@FIFO��������64Byte��Ĭ����ֵΪ8�ֽ�
	������յ�����С�ڵ���8�ֽڣ��� Status1Reg->LoAlert ��λ���������ѿգ���
	��ʱ CommIRQReg->LoAlertIRq ��λ����Ҫд�Ĵ�����մ�λ��
	**/
	
//delay_ms(20);//Ϊë����ط�����ʱ�ͽ���Ӳ������
	//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
	
	//�ȴ����ݷ���
	i = 0;
	do{
		n = _reg_read(id, ComIrqReg);//��ѯ�¼��ж�
	}while( (i++ < 600) && !(n & 0x01) && !(n & waitFor) );//��ʱ��δ��ʱ��û�з��ִ���
	
	if(!(n & 0x40))//����û����
	{
		rs = MI_ERR;
		return MI_ERR;
	}
	
	if( _reg_clear(id, BitFramingReg, 0x80) == RC522_NOACK )//����StartSendλ��@ֹͣ����
	{
		rs = MI_ERR;
		return MI_ERR;
	}
	
	if(rs == MI_OK)//δ��ʱ��δ����
	{   
		uint16_t rs;
		rs = _reg_read(id, ErrorReg);//��ʾִ�е��ϸ�����Ĵ���״̬
		if(rs == RC522_NOACK)
		{
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if(rs & 0x1B)//�������־�Ĵ���BufferOfI CollErr ParityErr ProtocolErr
		{
			rs = MI_ERR;
		}
		else//û����
		{
			rs = MI_OK;
			
			if (n & irqEn & 0x01)//�Ƿ�����ʱ���жϣ�@��ʱ������0
			{   
				rs = MI_NOTAGERR;
			}
			
			if(Command == PCD_TRANSCEIVE)//�շ�����
			{
				n = _reg_read(id, FIFOLevelReg);//��FIFO�б�����ֽ���				
				rs = _reg_read(id, ControlReg);//�����ƼĴ�����@��ȡ����յ�����Чλ��Ŀ�����Ϊ0���������ֽ���Ч��

				if(rs == RC522_NOACK)
				{
					rs = MI_ERR;
					return MI_ERR;
				}
				
				lastBits = rs & 0x07;
				
				if(lastBits)//���һ�ֽ��д���λ
				{   
					*pOutLenBit = (n - 1)*8 + lastBits; //N���ֽ�����ȥ1�����һ���ֽڣ�+���һλ��λ�� ��ȡ����������λ�� 
				} //��ȡ��Чλ���� //���һ���ֽ�����Ч����λ��ʱ����Ҫ�ų�
				else
				{   
					*pOutLenBit = n*8;//�����յ����ֽ������ֽ���Ч
				} //���Բ���bit����
				
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
					rs = _reg_read(id, FIFODataReg);//��FIFO�е�����
					if(rs == RC522_NOACK)
					{
						rs = MI_ERR;
						return MI_ERR;
					}
					pOut[i] = (uint8_t)rs; 
				}
			}
		}
	}

	if( _reg_set(id, ControlReg, 0x80) == RC522_NOACK )//ֹͣ��ʱ��
	{
		rs = MI_ERR;
		return MI_ERR;
	}

	///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
	if( _reg_write(id, CommandReg, PCD_IDLE) == RC522_NOACK )//�������@��ʱ6ms
	{
		printf("write CommandReg err\r\n");
		rs = MI_ERR;
		return MI_ERR;
	}
	///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
	
	return rs;
}


/////////////////////////////////////////////////////////////////////
//@���ܣ�Ѱ��
//@����: req_code[IN]:Ѱ����ʽ
//               0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//       pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//@���أ�ִ�н����ʧ��->MI_ERR���ɹ�->MI_OK
/////////////////////////////////////////////////////////////////////
static uint16_t PcdRequest(uint8_t id, uint8_t req_code, uint8_t *pTagType)
{
	char rs = MI_OK;  
	uint8_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN];

	if( _reg_clear(id, Status2Reg, 0x08) == RC522_NOACK )//���� MIFARECyptol ��־,@�������� MFAuthent ��֤
	{
//		printf("clear Status2Reg err\r\n");
		rs = MI_ERR;
		return MI_ERR;
	}
	
	if( _reg_write(id, BitFramingReg, 0x07) == RC522_NOACK )//���һ���ֽڷ���7bit
	{
		icreader_state_change(id, CARD_ERR);
		rs = MI_ERR;
		
		//��λˢ��ͷ
		if(rc522_init(id))//���³�ʼ���豸
		{
			ireader_link(id);
		}
		return MI_ERR;
	}	

	ucComMF522Buf[0] = req_code;//�����͸���Ƭ������
	rs = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);//Ѱ��
	if ((rs == MI_OK) && (unLen == 0x10))//Ѱ���ɹ����ؿ����� 
	{    
		pTagType[0] = ucComMF522Buf[0];
		pTagType[1] = ucComMF522Buf[1];
	}
	else
	{
		rs = MI_ERR;   
	}
	
	return rs;//�ɹ�
}

/////////////////////////////////////////////////////////////////////
//@���ܣ�����ײ
//@����: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//@���أ�ִ�н����ʧ��->MI_ERR���ɹ�->MI_OK
/////////////////////////////////////////////////////////////////////
static uint16_t PcdAnticoll(uint8_t id, uint8_t *pSnr)
{
    char rs = MI_OK;
    uint8_t i, snr_check = 0;
    uint8_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN];
    

//    if( _reg_clear(id, Status2Reg,0x08) == RC522_NOACK )
//		{
//			printf("clear Status2Reg err\r\n");
//			rs = MI_ERR;
//			return MI_ERR;
//		}

		if( _reg_write(id, BitFramingReg, 0x00) == RC522_NOACK )//���һ�ֽڷ���8bit
		{
			rs = MI_ERR;
			return MI_ERR;
		}

		///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
		if( _reg_clear(id, CollReg, 0x80) == RC522_NOACK )//ValuesAfterColl ����Ϊ0�������н��յ�λ�ڳ�ͻ�󽫱����
		{
			printf("clear CollReg err\r\n");
			rs = MI_ERR;
			return MI_ERR;
		}
		///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;
    rs = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);//@@�뿨Ƭ����
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
		
    ///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////
    if( _reg_set(id, CollReg, 0x80) == RC522_NOACK )
		{
			printf("set CollReg err\r\n");
			rs = MI_ERR;
			return MI_ERR;
		}
		///////////////// ��ֻ��ѯUID�������ﲻ��Ҫ�� //////////////////////////

    return rs;
}


/////////////////////////////////////////////////////////////////////
//@��    �ܣ���֤��Ƭ����
//@����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ 
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//@��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////               
static char PcdAuthState(uint8_t id, uint8_t auth_mode, uint8_t addr, uint8_t *pKey, uint8_t *pSnr)
{
    char status = MI_OK;
    uint8_t unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)//��Կ
    {    
			ucComMF522Buf[i+2] = *(pKey+i);
		}
    for (i = 0; i < 4; i++)//������UID
    {    
			ucComMF522Buf[i+8] = *(pSnr+i);
		}
    
    status = PcdComMF522(id, PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(_reg_read(id, Status2Reg) & 0x08)))
    {   
			status = MI_ERR;
		}
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
static char PcdSelect(uint8_t id, unsigned char *pSnr)
{
    char status = MI_OK;
    uint8_t i;
    uint8_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for(i = 0; i < 4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(id, ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    _reg_clear(id, Status2Reg, 0x08);

    status = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
			status = MI_OK;  
		}
    else
    {   
			status = MI_ERR;    
		}

    return status;
}


/////////////////////////////////////////////////////////////////////
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
///////////////////////////////////////////////////////////////////// 
char PcdRead(uint8_t id, uint8_t addr, uint8_t *pData)
{
    char status;
    uint8_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(id, ucComMF522Buf, 2, &ucComMF522Buf[2]);
   
    status = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        memcpy(pData, ucComMF522Buf, 16);
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
static char PcdHalt(uint8_t id)
{
    uint8_t   status;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(id, ucComMF522Buf, 2, &ucComMF522Buf[2]);
 
    status = PcdComMF522(id, PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
		status = status;
    return MI_OK;
}


/*******************************************************************************************************************
@@ �߼���������
@@ ʵ��ҵ������
*******************************************************************************************************************/
/*
@���ܣ������߼�->��Ѱ��������ײ��ѡ�������Ϳ��š�������д�����޸�������в���
@������buf������
@���أ�ִ�н����ʧ��->MI_ERR���ɹ�->MI_OK
*/
static uint8_t rc522_init(uint8_t id);

static uint8_t rc522_work(uint8_t id, uint8_t *buf)
{
  char status, rs;
	rs = rs;
	if( rc522_antenna_on(id) == RC522_NOACK )//������
	{
		icreader_state_change(id, CARD_ERR);
		rc522_init(id);
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("set TxControlReg err\r\n");
		#endif
		return MI_ERR;
	}
	else
	{
		icreader_state_change(id, CARD_NORMAL);
	}
	
	status = PcdRequest(id, PICC_REQALL, buf);//Ѱ�����������п���@���ص�bufǰ2�ֽ�Ϊ������
	if(status != MI_OK)
		return MI_ERR;
	
	
  status = PcdAnticoll(id, &buf[2]);//����ײ�����ؿ������к� 4�ֽ�
  if(status != MI_OK)
		return MI_ERR;

	{
		status = PcdSelect(id, &buf[2]);
		if (status != MI_OK)
		{
			printf("PcdSelect() err\r\n");
			rs = MI_ERR;
		}
//		
//		status = PcdAuthState(id, PICC_AUTHENT1A, 0*4+3, key, &buf[2]);//0*4+3 ��ʾ0����������0~15����4��ʾһ������4�顾0~3�顿��3��ʾ��֤��ÿ�������ĵ�4��
//		if (status != MI_OK)
//		{    
//			printf("PcdAuthState() err\r\n");
//			rs = MI_ERR;
//		}
//	}
//	
//	{
//		uint8_t rxbuf[20] = {0};
//		status = PcdRead(id, 0*4+1, rxbuf);
//		if(status == MI_OK)
//		{
//			printf("read success\r\n");
//			
//		}
	
	}
	
	return MI_OK;
}



/*
@���ܣ��Թ涨�Ĳ����ʸ�λ
@���أ�TRUE,�ɹ���FALSE��ʧ��
*/
static uint8_t reset_baud(uint8_t id, uint32_t baud)
{
	uint16_t rs, i;

	uart_config[id](baud);//���ò�����
//	_reg_write(CommandReg, 0x55);//��λ��@�����ĵ�Ҫ��ģ�@���ǻ����������Ȧ�ĸ�Ӧ����Զ���ԣ������е�����
	_reg_write(id, CommandReg, PCD_RESETPHASE);//��λ
	
	i = 0;
	do{
		rs = _reg_read(id, RFU00);
	}while(i++ < 10 && rs == RC522_NOACK);
	
	i = 0;
	do{
		rs = _reg_read(id, CommandReg);
	}while(i++ < 10 && rs == RC522_NOACK);
	
	if(rs & 0x10)//@PowerDown == 0,��λ�����
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
@���ܣ���λRC522
@���أ�ִ�н����ʧ��->MI_ERR���ɹ�->MI_OK
*/
static uint16_t rc522_reset(uint8_t id)
{
	uint16_t rs = MI_OK, i = 0;
	
	vTaskDelay(200*(id+1));//�ȴ�Ӳ����λ

	/*
	@˵������������115200��λ�Ļ����޷�ͨ��
	@ע�⣺�����λ���в�Ҫ����
	*/
//	if(!reset_baud(115200))//ʹ��115200�����ʸ�λ->��λʧ��
	{
		if(reset_baud(id, 9600))//9600��λ->��λ�ɹ�
		{
			do{
				rs = _reg_write(id, SerialSpeedReg, 0x7A);//���ڲ����ʸ�Ϊ115200
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
		rs = _reg_write(id, ModeReg, 0x3D);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs == RC522_NOACK)	//���巢�ͺͽ��ճ���ģʽ�����RF����������TxWaitRF��λ;SIGIN�ܽŸߵ�ƽ��Ч��CRC��ʼֵ0x6363��
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
		rs = _reg_write(id, TReloadRegL, 30);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//16λ��ʱ����λ
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
		rs = _reg_write(id, TReloadRegH, 0);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//16λ��ʱ����λ
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
		rs = _reg_write(id, TModeReg, 0x8D);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//�����ڲ���ʱ��������
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
		rs = _reg_write(id, TPrescalerReg, 0x3E);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs  == RC522_NOACK )	//���ö�ʱ����Ƶϵ��
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
		rs = _reg_write(id, TxAutoReg, 0x40);
	}while(i++ < 20 && rs == RC522_NOACK);
	if(rs == RC522_NOACK )	//���Ʒ����ź�Ϊ100%ASK
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("write TxAutoReg err\r\n");
		#endif
		rs = MI_ERR;
		return MI_ERR;
	}
	return rs;	//�ɹ�
}


/*
@���ܣ�����RC522�Ĺ�����ʽ
@������type��������ʽ
@���أ�ִ�н����ʧ��->MI_ERR���ɹ�->MI_OK
*/
static uint16_t M500PcdConfigISOType(uint8_t id, uint8_t type)
{
	uint8_t rs = MI_OK;
	
	if (type == 'A')//ISO14443_A
	{ 
		if( _reg_clear(id, Status2Reg, 0x08) == RC522_NOACK )	//TempSensOff ��λ���ڲ��¶ȴ������ض�ʱ��λ��λ
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("clear Status2Reg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( _reg_write(id, ModeReg, 0x3D) == RC522_NOACK )//3F //@1.���RF����RF field����������TxWaitRF��λ��������ֻ���ڴ�ʱ��������2.SIGIN�ܽŸߵ�ƽ��Ч��3.CRC 6363
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write ModeReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( _reg_write(id, RxSelReg, 0x86) == RC522_NOACK )//84	//�������ã�1.�ڲ�ģ�ⲿ�ֵĵ����źţ�2.�� ���������������ӳ�RxWait��λʱ�ӡ������֡����ʱ���ڣ�Rx�ܽ��ϵ������źŶ������ԣ�@������������ָ����ƣ�
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write RxSelReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( _reg_write(id, RFCfgReg, 0x7F) == RC522_NOACK )  //4F //���ý������棺Ŀǰ����Ϊ���
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write RFCfgReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}

		if( _reg_write(id, TReloadRegL, 30) == RC522_NOACK )//��ʱ����λ
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TReloadRegL err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( _reg_write(id, TReloadRegH, 0) == RC522_NOACK )//��ʱ����λ
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TReloadRegH err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( _reg_write(id, TModeReg, 0x8D) == RC522_NOACK )//��ʱ�����ã�TAuto��λ����ʱ�����������ʵķ��ͽ���ʱ�Զ��������ڽ��յ���һ������λ��ʱ������ֹͣ���С�TPrescaler����λ1101
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TModeReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
		
		if( _reg_write(id, TPrescalerReg, 0x3E) == RC522_NOACK )//TPrescaler�ĵ�8λ:��ʽ��fTimer��6.78MHz / TPreScaler�����㶨ʱ��Ƶ�ʣ�
		{
			#if(RC_522_LOG)
			char tmp = '1'+id;
			printf(&tmp);printf(" = ");
			printf("write TPrescalerReg err\r\n");
			#endif
			rs = MI_ERR;
			return MI_ERR;
		}
				
//		if( rc522_antenna_on() == RC522_NOACK )//������ TX1��TX2
//		{
//			printf("rc522_antenna_on err_1\r\n");
//			rs = MI_ERR;
//			return MI_ERR;
//		}
//		delay_ms(10);
	}
	else//���Ͳ���
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
@���ܣ�RC522��ʼ��
@���أ�ִ�н����ʧ��->FALSE���ɹ�->TRUE
*/
static uint8_t rc522_init(uint8_t id)
{
	if( rc522_reset(id) == MI_ERR)//��λRC522
	{
		return FALSE;
	}
	
	if( rc522_antenna_off(id) == RC522_NOACK )//�ر�����
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("rc522 close anttenna err\r\n");
		#endif
		return FALSE;
	}
	
//	if(	_reg_set (GsNReg , 0x27) == RC522_NOACK )//����TX1,TX2�絼
//	{
//		printf("set GsNReg anttenna err\r\n");
//		return FALSE;
//	}
	
//	if(rc522_antenna_on() == RC522_NOACK)//��������
//	{
//		printf("rc522 open anttenna err\r\n");
//		return FALSE;
//	}

	if( M500PcdConfigISOType(id, 'A') == MI_ERR )//���ù�����ʽ
	{
		#if(RC_522_LOG)
		char tmp = '1'+id;
		printf(&tmp);printf(" = ");
		printf("rc522 set work mode err\r\n");
		#endif
		return FALSE;
	}
                                                                                                                                                                                	
	return TRUE;
}


/*
@���ܣ������豸
@˵���������ò����豸�汾��ȷ���Ƿ����豸
*/
static void ireader_link(uint8_t id)
{
	uint16_t rs;
	rs = _reg_read(id, VersionReg);
	if(rs == RC522_NOACK)
		icreader_state_change(id, CARD_ERR);
	else
		icreader_state_change(id, CARD_NORMAL);
}


/******************************************************************************************************************
@����ӿ�
******************************************************************************************************************/
/*
@���ܣ��������г�ʼ��
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
@���ܣ�����֤
@������uid����uid�ַ�����len�����ų���
*/
static uint8_t card_verification(uint8_t id, uint8_t* uid_hex_byte)
{
	char tmp[13] = {0};
	char uid_char[32];
	uint8_t i, status, len, uid_hex_head, uid_hex_tail;
	uint8_t key[6]= {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint32_t product, uid_hex;
	
	//������Կ
	uid_hex = (uid_hex_byte[0] | (uid_hex_byte[1] << 8) | (uid_hex_byte[2] << 16) | (uid_hex_byte[3] << 24));
	len = sprintf(uid_char, "%u", uid_hex);
	uid_char[len] = 0;
	uid_hex_head = uid_char[0] - '0';
	uid_hex_tail = uid_char[len - 1] - '0';
	uid_hex = uid_hex%1000000000%100000000%10000000;
	product = (uid_hex_tail + uid_hex_head)*(uid_hex + 1823823);
	
	sprintf(tmp, "%012u", product);
	for(i = 0; i < 6; i++)
	{
		key[i] = ((tmp[i*2] - '0') << 4) | (tmp[i*2+1] - '0');
	}
		
	status = PcdAuthState(id, PICC_AUTHENT1B, 0*4+3, key, uid_hex_byte);//0*4+3 ��ʾ0����������0~15����4��ʾһ������4�顾0~3�顿��3��ʾ��֤��ÿ�������ĵ�4��
	if (status == MI_OK)
	{    
		return TRUE;
	}
	
	printf("key verificate err\r\n");
	return FALSE;
}


/*
@���ܣ�����ָ��
@���أ�ִ�н����ʧ��->FALSE���ɹ�->TRUE
@˵����Ŀǰֻ������������ID
*/
uint8_t ireader_read(uint8_t id)
{
	uint8_t buf[30] = {0}, rs = FALSE, i;
	
	if(rc522_work(id, buf) == MI_OK)//Ѱ����
	{
		if( card_verification(id, &buf[2]) )//��֤��Կ
		{
			for(i = 0; i < 10; i++)//��������0
			{
				class_global.ireader[id].card.physic_char[i] = 0;
			}
			sprintf((char*)class_global.ireader[id].card.physic_char, "%010u", (buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24)) );
			class_global.ireader[id].card.physic_char[10] = 0;//���Ž�����
			rs = TRUE;
		}
	}
	
	rc522_antenna_off(id);//������
	
	return rs;
}
