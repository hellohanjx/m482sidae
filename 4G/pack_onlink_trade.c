/*
在线支付
*/
#include "user_rtc.h"
#include "global.h"
#include "string.h"
#include "commucation.h"
#include "msg.h"

/*
@功能：在线支付申请结果处理回调函数
@参数：recv，平台返回数据指针；len，平台返回数据长度；send，发送到平台数据指针；id，发送数据来源
*/
static uint8_t callback_online_cardPay(uint8_t *recv, uint16_t len, uint8_t*send, uint8_t id)
{
	uint8_t i, rs = FALSE, chk;
	
	for(i = 0, chk = 0; i < (recv[2] - 1); i++)
		chk += recv[i];
	if(chk == recv[recv[2] - 1])
	{
		IREADER_MSG msg;
		msg.id = id;//发送数据来源
		msg.type = ONLINE_CARD_PAY_CMD;	//表示支付申请
		msg.value = 0x50;								//表示超时未回复
		
		if(recv[0] == 0x1D && recv[1] == send[1])//包头与序号正确
		{
			msg.value = recv[3];//更改回应类型
			//需要加入长度判断
			if(recv[3] == 0X30)//申请支付成功
			{
				uint32_t fac ;
				uint8_t tmp[10], n = 5, j = 0;
										
				for( (i = 0, j = n); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//逻辑卡号
					class_global.ireader[id].card.logic_char[i] = recv[i+j];
				}
				n++;
											
				for( (i = 0, fac = 1, j = n); (i < 10 && recv[i+j] != '*'); i++){//计算余额位数
					tmp[i] = recv[i+j];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.balance = 0); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//计算余额
					class_global.ireader[id].card.balance += (tmp[i]-'0')*fac;
					fac /= 10;
				}
				n++;
				
				for( (i = 0, fac = 1, j = n); (i < 10 && recv[i+j] != '*'); i++){//计算消费次数位数（原始次数，不是可卖次数）
					tmp[i] = recv[i+j];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.daymaxpay = 0); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//计算消费位数
					class_global.ireader[id].card.daymaxpay += (tmp[i]-'0')*fac;
					fac /= 10;
				}
			}
			else//扣款申请失败
			{
				for(i = 0;i < 10;i++)//逻辑卡号
					class_global.ireader[id].card.logic_char[i] = recv[5+i];
			}
			rs =  TRUE;
		}
		ireader_queue_send[id](msg);//消息发送到队列
	}
	else//校验错
	{
		printf("ONLINE pay chk err\r\n\r\n");
	}
	return rs;
}




/*
@功能：在线查询申请结果处理回调函数
@参数：recv，平台返回数据指针；len，平台返回数据长度；send，发送到平台数据指针
*/
static uint8_t callback_online_cardCheck(uint8_t *recv, uint16_t len, uint8_t*send, uint8_t id)
{
	uint8_t i, rs = FALSE, chk;
	
	for(i = 0, chk = 0; i < (recv[2] - 1); i++)
		chk += recv[i];
	if(chk == recv[recv[2] - 1])
	{
		IREADER_MSG msg;
		msg.id = id;
		msg.type = ONLINE_CARD_CHECK_CMD;		//表示查询申请
		msg.value = 0x50;		//表示超时未回复
		
		if(recv[0] == 0x1D && recv[1] == send[1])//包头与序号正确
		{
			msg.value = recv[3];//更改回应类型
			//需要加入长度判断
			if(recv[3] == 0X30)//申请支付成功
			{
				uint32_t fac;
				uint8_t tmp[10], n = 5, j = 0;
										
				for( (i = 0, j = n);  (i < 10 && recv[i+j] != '*'); (i++, n++) ){//逻辑卡号
					class_global.ireader[id].card.logic_char[i] = recv[i+j];
				}
				n++;
				
				for( (i = 0, j = n, fac = 1); (i < 10 && recv[i+j] != '*'); i++){//计算余额位数
					tmp[i] = recv[j+i];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.balance = 0); (i < 10 && recv[i+j] != '*'); i++, n++){//计算余额
					class_global.ireader[id].card.balance += (tmp[i]-'0')*fac;
					fac /= 10;
				}
				n++;
				
				
				for( (i = 0, j = n, fac = 1); (i < 10 && recv[i+j] != '*'); i++){//计算消费次数位数
					tmp[i] = recv[j+i];
					if(i != 0)
					fac *=10;
				}
				for( (i = 0, j = n, class_global.ireader[id].card.daymaxpay = 0); (i < 10 && recv[i+j] != '*'); (i++, n++) ){//计算消费次数 
					class_global.ireader[id].card.daymaxpay += (tmp[i]-'0')*fac;//当天消费次数 
				}
			}
			else//扣款查询失败
			{
				for(i = 0; i < 10; i++)//逻辑卡号
					class_global.ireader[id].card.logic_char[i] = recv[5+i];
			}
			rs =  TRUE;
		}
		ireader_queue_send[id](msg);//消息发送到队列
		}
	else//校验错
	{
		printf("ONLINE check chk err\r\n\r\n");
	}
	return rs;
}


/*
@包类型：0x17-0x41-0x31/0x32
@功能：即时-在线卡申请
@参数：id，来源识别码
			 type：0x31 = 在线支付，0x32 = 在线查询
			 trade_num，交易流水号
*/
void requset_card_trade(uint8_t id, uint8_t type, uint32_t trade_num)
{
	uint8_t i,j;
	char tmp[10],len;
	uint8_t *str;
	MAIL *mail;
	CUR_TIME curTime;	
	
	curTime = get_cur_time();//获得当前时间
	mail = mail_apply(150);
	configASSERT(mail);
	if(mail == 0)
		return;
	if(type == ONLINE_CARD_PAY_CMD)
		mail->com_call_back = callback_online_cardPay;//支付回调
	else
	if(type == ONLINE_CARD_CHECK_CMD)
		mail->com_call_back = callback_online_cardCheck;//查询回调	
	else
		mail->com_call_back = NULL;
	
	mail->id = id;
	
	str = mail->addr;
	
	i = 0;
	str[i++]=0x17;
	str[i++]=' ';//序号(发送时加上)
	str[i++]=' ';//总长度(校验一起计算)
	str[i++]=0x41;
	//str[i++] = '*';//此特殊包体不需要

	//刷卡接口
	sprintf(tmp, "%04u", class_global.ireader[id].equ.interface);
	for(j = 0;j < 4;j++)
		str[i++] = tmp[j];
	
	str[i++] = ';';
	
	//指令类型
	str[i++] = type ;
	str[i++] = ';';
	
	//机器id
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0; j < 10; j++)
	{
		str[i++]=tmp[j];
	}
	str[i++] = '*';
	
	//交易流水号
	len = sprintf(tmp, "%u" , trade_num);
	for(j=0;j<len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	//逻辑卡号(默认写0)
	str[i++]= '0';
	str[i++]= '*';
	
	//物理卡号
	for(j = 0; j < 50 && class_global.ireader[id].card.physic_char[j] != 0; j++)
	{
		str[i++]= class_global.ireader[id].card.physic_char[j];
	}
	str[i++] = '*';
	
	//货道号【用刷卡器id】
	len = sprintf(tmp, "%02u", id);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//价格【用交易价格】
	len = sprintf(tmp,"%u",class_global.trade.price);
	for(j=0;j<len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	//时间
	len = sprintf(tmp,"%04u", curTime.year + 2000);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.month);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.day);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.hour);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.min);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.sec);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';//校验预留位
	str[2] = i+1;
	
	instant_queue_send(mail, TAIL);//丢到队列尾
}



/*
@包类型：0x17-0x41-0x33/0x34
@功能：汇报-在线刷卡结果
@参数：id，刷卡器id
			 type：0x34：刷卡失败；0x33：刷卡成功
			 trade_num：交易号
*/
void requset_card_result(uint8_t id, uint8_t type, uint32_t trade_num)
{
	uint8_t i,j;
	char tmp[10],len;
	uint8_t *str;
	MAIL *mail;
	CUR_TIME curTime;	
	
	curTime = get_cur_time();//获得当前时间
	mail = mail_apply(150); //申请信封
	configASSERT(mail);
	if(mail == 0)
		return;
	mail->com_call_back = NULL;//回调函数
	str = mail->addr;
	
	i = 0;
	str[i++]=0x17;
	str[i++]=' ';//序号(发送时加上)
	str[i++]=' ';//总长度(校验一起计算)
	str[i++]=0x41;
	//str[i++] = '*';//此特殊包体不需要

	//接口编码
	sprintf(tmp, "%04u", class_global.ireader[id].equ.interface);
	for(j = 0;j < 4;j++)
		str[i++] = tmp[j];
	str[i++] = ';';
	
	//指令类型
	str[i++] = type;
	str[i++] = ';';
	
	//机器id
	sprintf(tmp, "%010u", class_global.net.id);
	for(j = 0; j < 10; j++)
	{
		str[i++]=tmp[j];
	}
	str[i++] = '*';
	
	//交易流水号
	len = sprintf(tmp, "%u" , trade_num);
	for(j=0;j<len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';
	
	//逻辑卡号(默认写0)
	str[i++]= '0';
	str[i++]= '*';
	
	//物理卡号
	for(j = 0; j < 50 && class_global.ireader[id].card.physic_char[j] != 0; j++)
	{
		str[i++]= class_global.ireader[id].card.physic_char[j];
	}
	str[i++] = '*';
	
	//货道号【用刷卡器id】
	len = sprintf(tmp, "%02u", id);
	for(j = 0; j < len; j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//价格【用交易价格】
	len = sprintf(tmp,"%u",class_global.trade.price);
	for(j=0;j<len;j++)
		str[i++] = tmp[j];
	str[i++] = '*';
	
	//时间
	len = sprintf(tmp,"%04u",curTime.year + 2000);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.month);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.day);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.hour);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.min);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	len = sprintf(tmp,"%02u",curTime.sec);
	for(j = 0;j < len;j++)
	str[i++] = tmp[j];
	str[i++] = '*';//校验预留位
	str[2] = i+1;
	
	report_to_communication(mail, TAIL);
}
