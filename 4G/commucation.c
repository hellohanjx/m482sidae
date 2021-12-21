#include "4G.h"
#include "msg.h"
#include "global.h"
#include "uart7_config.h"
#include "commucation.h"
#include "user_config.h"
#include "user_rtc.h"

/*
服务器应答结果
*/
#define ACK_OK     0X30 //确认应答
#define ACK_CHK    0X31 //校验和错误
#define ACK_NA     0X32 //无应答
#define ACK_REJECT 0X33 //拒绝执行
#define ACK_BUSY   0X34 //系统忙
#define ACK_START  0X35 //动作开始
#define ACK_END    0X36 //动作执行完毕


/*
@功能：获取网络状态
@参数：state，返回值缓冲
@返回值：是否取到网络状态
*/
uint8_t get_network_state(uint8_t *state)
{
	#if BOARD_4G == USER_EN
		*state = get_4g_state();
		return TRUE;
	#else
		return FALSE;
	#endif
}


/*
@功能：汇报消息
@参数：mymail,数据指针；type，0加到队列尾，1加到队列头
*/
void add_report_info(MAIL* mymail, uint8_t type)
{
	if(type == FALSE)//加到队列尾
	{
		if(report_to_tail(mymail))
			class_global.net.backlog_data++;//积压数据增加
	}
	else//加到队列头
	{
		if(report_to_head(mymail))
			class_global.net.backlog_data++;//积压数据增加
	}
}

/*
@功能：通讯一般汇报包回调函数
@参数：*rx，返回数据
@说明：只验证了校验和长度
*/
static uint8_t callback_com_recv(UART7_DATA *rx)
{
	if(rx->buf[2] == rx->len)//接收长度符合
	{
		uint8_t i,chk;
		for(i = 0,chk = 0;i < (rx->len-1);i++)
			chk += rx->buf[i];
		if(chk == rx->buf[rx->len-1])
		{
			commucation_sem_send_isr(); //中断级信号量释放（不能释放互斥型信号量）
		}
	}
	return TRUE;
}



/*
@功能：发送数据到总线
@参数：*dat，待发送数据指针；rx待接收数据指针的地址
*/
static void mail_to_bus(uint8_t *dat, UART7_DATA** rx)
{
	uint8_t chk, len = dat[2], cnt = 0;
	uint16_t i, j;

	if(dat[0] != 0x1f && dat[0] != 0x10 && dat[0] != 0x11 && dat[0] != 0x12
	&& dat[0] != 0x14	&& dat[0] != 0x17 && dat[0] != 0xff)
	{
		//错误包打包
		uint8_t *buf = 0;
		do{
			buf = pvPortMalloc(300);
			configASSERT(buf);
			if(buf)
			{
				i = 0;
				//包头
				buf[i++] = 0x1C;	//错误包类型0x1C
				buf[i++] = dat[1];//使用原始包序号
				buf[i++] = '*';	 	//错误包长度
				buf[i++] = 0x11;	//业务类型
				buf[i++] = '*';//分隔符
				//包体
				for(j = 0; j < (len-1); j++)
					buf[i++] = dat[j];
				buf[i++] = '*';//分隔符
				buf[i++] = ' ';//校验占位
				buf[2] = i;
				//校验
				for(j = 0,chk = 0; j < (i-1) && j < 256; j++)
					chk += buf[j];
				buf[i-1] = chk;
				
				_uart7_send(&(*rx), buf, i, callback_com_recv);//串口发送
				vPortFree(buf);
			}
			else
			{
				printf("mail_to_bus err\r\n");
				if(cnt++ > 5)
				{
					restart_equ_set(RESET_MailToBus, TRUE);
				}
			}
		}while(buf == 0);
	}
	else//打包发出
	{
		for(i = 0,chk = 0; i < (len-1) ; i++)//计算校验和
		{
			chk += dat[i];
		}
		dat[i++] = chk;//添加校验和
		_uart7_send(&(*rx), dat, i, callback_com_recv);//串口发送 
	}
}

/*
@功能：询问是否重启
@参数：无
@返回：0~4
*/
static uint8_t send_restart(void)
{
	UART7_DATA  *pt_rx;//接收缓冲指针
	MAIL *mail;//信封指针
	uint8_t *pt_txbuf;//发送数据指针
	uint8_t err, rs; 
	
	mail = mail_apply(50);//申请内存空间
	configASSERT(mail);
	if(mail != 0)//申请到空间
	{
		pt_txbuf = mail->addr;

		get_reset_cmd(pt_txbuf);
		pt_txbuf[1] = class_global.net.number + 1;//包序号
		class_global.net.number = (class_global.net.number+1)%255;//更新编号
		mail_to_bus(pt_txbuf, &pt_rx);
		err = commucation_sem_get(ONE_SECOND*30);//30s 超时
		if(err == pdTRUE)
		{
			if(pt_rx->buf[0] == 0x1D && pt_rx->buf[1] == pt_txbuf[1] && pt_rx->buf[8] == 0x31)//重启机器
			{
				class_global.sys.reset = SWITCH_ON;
				if(pt_rx->buf[9] == 0x31)//有设置需要更改
				{
					//@新的方案=第9字节机器类型，后面是电机信号
					
					uint8_t tmp;
					
					if(pt_rx->buf[10] >=  '0' && pt_rx->buf[10] <= '9')
					{
						tmp = pt_rx->buf[10] - '0';
					}
					else
					if(pt_rx->buf[10] >=  'A' && pt_rx->buf[10] <= 'Z')
					{
						tmp = pt_rx->buf[10] - 'A' + 10;
					}
					//设置机型					
					tmp = pt_rx->buf[11] - '0';
					//屏幕类型(目前只有2类)
					tmp = pt_rx->buf[12] - '0';
					//光检类型(目前只有2类光检)
					tmp = pt_rx->buf[13] - '0';
					//刷卡器类型(目前只有2类)
					
					rs = 1;//重启机器，更新了参数
					printf("restart and updata\r\n\r\n");
				}
				else
				if(pt_rx->buf[9] == 0x32)//新方式-参数修改
				{
					uint8_t tmp;
					
					if(pt_rx->buf[10] != '0')//机型，0字符不修改机型
					{
						if(pt_rx->buf[10] >=  '1' && pt_rx->buf[10] <= '9')
						{
							tmp = pt_rx->buf[10] - '0' - 1;
						}
						else
						if(pt_rx->buf[10] >=  'A' && pt_rx->buf[10] <= 'Z')
						{
							tmp = pt_rx->buf[10] - 'A' + 9;
						}
						//设置机型
					}
					
					tmp = (pt_rx->buf[11] - '0')*100 + (pt_rx->buf[12] - '0')*10 + (pt_rx->buf[13] - '0');//电机信号
				}
				else
				{
					rs = 2;//重启机器，没有参数需要更新
					printf("restart but no updata\r\n\r\n");
				}
			}
			else
			{
				rs = 3;//不需要重启机器
				printf("restart unused\r\n\r\n");
			}
		}
		else
		{
			rs = 4;//服务端无应答
			printf("restart ack err\r\n\r\n");
		}
		mail_release(mail);//释放申请空间
	}
	else
	{
		rs = 0;//没申请到空间
		printf("apply mail err_3\r\n\r\n");
	}
	return rs;
}

/*
@功能：开机获取参数
*/
static uint8_t get_param(uint8_t *str, UART7_DATA *rx)
{
	uint8_t rs = TRUE;
	uint8_t err;

	//用户模式发送17 42 7~9类型包
	if( !class_global.sys.factory_en )//用户模式
	{
		get_set_param(str, 7);//17-42-7包【机器运行参数】
		str[1] = class_global.net.number+1;
		class_global.net.number = (class_global.net.number+1)%255;
		
		mail_to_bus(str, &rx);
		err = commucation_sem_get(ONE_SECOND*7);
		if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
		{
			if( !analysis_17_42_7(rx->buf, rx->len) )//解析返回包
			{
				rs = FALSE;
				printf("17 42 7 ack format err\r\n\r\n");
			}
			
			
			get_set_param(str, 8);//17-42-8包【货道 禁使能/状态】
			str[1] = class_global.net.number+1;
			class_global.net.number = (class_global.net.number+1)%255;
			
			mail_to_bus(str, &rx);
			err = commucation_sem_get(ONE_SECOND*7);
			if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
			{
				if( !analysis_17_42_8(rx->buf, rx->len) )//解析返回包
				{
					rs = FALSE;
					printf("17 42 8 ack format err\r\n\r\n");
				}
				
				
				get_set_param(str, 9);//17-42-9包【货道 价格/库存】
				str[1] = class_global.net.number+1; 
				class_global.net.number = (class_global.net.number+1)%255;
				
				mail_to_bus(str, &rx);
				err = commucation_sem_get(ONE_SECOND*7);
				if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
				{
					if( !analysis_17_42_9(rx->buf, rx->len) )//解析返回包
					{
						rs = FALSE;
						printf("17 42 9 ack format err\r\n\r\n");
					}
				}
				else
				{
					rs = FALSE;
					printf("17 42 9 ack err\r\n\r\n");
				}
			}
			else
			{
				rs = FALSE;
				printf("17 42 8 ack err\r\n\r\n");
			}
		}
		else
		{
			rs = FALSE;
			printf("17 42 7 ack err\r\n\r\n");
		}
	}
	return rs;
}


/*
设备登录
*/
static void send_linking(uint8_t* first, uint32_t *correct, uint8_t *redownload)//连机
{
	MAIL *cmail;
	uint8_t *str; 
	uint8_t err,i ;
	uint8_t download_result;
	UART7_DATA *rx;//接收数据指针
	
	cmail = mail_apply(PACK_MAX_SIZE);//申请数据空间
	configASSERT(cmail);
	if(cmail != 0)
	{
		str = cmail->addr; 
		err = pdFALSE;

		get_link_info(str, 0);//连接请求
		str[1] = class_global.net.number+1; //包序号
		class_global.net.number = (class_global.net.number+1)%255;//更新编号
		mail_to_bus(str, &rx);
		err = commucation_sem_get(ONE_SECOND*7);//等待应答信号量
		if(err == pdTRUE)
		{
			i=0;
			do
			{//防止上一个包的应答被当做对时包导致时间更新错误
				uint32_t timelast;
				if(i == 0)
					timelast = xTaskGetTickCount();
				err = commucation_sem_get(ONE_SECOND);//等待1s
				if(xTaskGetTickCount()-timelast >= ONE_SECOND)
				{
					timelast = xTaskGetTickCount();
				}
				i++;
			}while( ( !(err == pdFALSE && rx->buf[0] == 0x1D && rx->buf[1] == str[1]) ) && i < 100 );//消除多余的应答
			
			if(err == pdFALSE && rx->buf[0] == 0x1D && rx->buf[1] == str[1])
			{
				uint8_t ctime[6];
				ctime[0] = (rx->buf[3]-'0')*10 + rx->buf[4]-'0';
				ctime[1] = (rx->buf[5]-'0')*10 + rx->buf[6]-'0';
				ctime[2] = (rx->buf[7]-'0')*10 + rx->buf[8]-'0';
				ctime[3] = (rx->buf[9]-'0')*10 + rx->buf[10]-'0';
				ctime[4] = (rx->buf[11]-'0')*10 + rx->buf[12]-'0';
				ctime[5] = (rx->buf[13]-'0')*10 + rx->buf[14]-'0';
				
				if( set_rtc_time(1, ctime[0], ctime[1], ctime[2], ctime[3], ctime[4], ctime[5]) )//更新RTC时间
				{
					send_restart();//发送重启查询
					
					if(*first)//上电第一次联机
					{
						do{
							download_result = get_param(str, rx );
						}while( (download_result != TRUE) && ((*redownload)++ < PARAM_REDOWNED_CNT) );
						
						if( !download_result )//下载数据错误
						{
							uint16_t random = get_random_number();
							do{
								random /= 2;
							}while(random > 5000);
							
							//参数下载错误
							//2s后重新启动
							vTaskDelay(random*1000);
							
							restart_equ_set(RESET_Param, TRUE);//复位类型
						}
						else//开机下载数据正确
						{
							get_channel_status(str);
							str[1] = class_global.net.number+1; //加上编号
							class_global.net.number = (class_global.net.number+1)%255;//更新编号
							mail_to_bus(str, &rx); //发送到远程管理端
							err = commucation_sem_get(ONE_SECOND*7);
							if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
							{
								get_machine_status(str);
								str[1] = class_global.net.number+1; //加上编号
								class_global.net.number = (class_global.net.number+1)%255;//更新编号
								
								mail_to_bus(str, &rx); //发送到远程管理端
								err = commucation_sem_get(ONE_SECOND*7);
								if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
								{
									//发送设置参数 20191021
									get_vm_set(str);
									str[1] = class_global.net.number+1; //加上编号
									class_global.net.number = (class_global.net.number+1)%255;//更新编号
									
									mail_to_bus(str, &rx); //发送到远程管理端。
									err = commucation_sem_get(ONE_SECOND*7);
									if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
									{
										get_software_version(str);
										str[1] = class_global.net.number+1; //加上编号
										class_global.net.number = (class_global.net.number+1)%255;//更新编号
										
										mail_to_bus(str, &rx); //发送到远程管理端
										err = commucation_sem_get(ONE_SECOND*7);
										if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
										{									
											//工厂模式发送
											if( get_factory_set(str, 1) )
											{
												str[1] = class_global.net.number+1; //加上编号
												class_global.net.number = (class_global.net.number+1)%255;//更新编号
												
												mail_to_bus(str, &rx); //发送到远程管理端
												err = commucation_sem_get(ONE_SECOND*7);
												if(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1]==str[1])
												{
													updata_factory_set(rx->buf, rx->len, str);//处理收到数据
													*first = 0;
													class_global.net.state = 1;//标记网络状态为连接
													restart_equ_set(RESET_NormalLink, FALSE);//复位标志恢复初始值
													(*correct) = xTaskGetTickCount();
												}
												else
												{
													printf("17 41 1 ack err\r\n\r\n");
												}
											}
											else
											{
												*first=0;
												class_global.net.state=1;//标记网络状态为连接
												(*correct) = xTaskGetTickCount();
												restart_equ_set(RESET_NormalLink, FALSE);//复位标志恢复初始值
											}
										}
										else
										{
											printf("1f-35 ack err\r\n\r\n");
										}
									}
									else
									{
										printf("1f-32 ack err\r\n\r\n");
									}
								}
								else
								{
									printf("1f-34 ack err\r\n\r\n");
								}
							}
							else
							{
								printf("1f-31 ack err\r\n\r\n");
							}
						}
					}
					else//中间断网重新连接成功
					{
						class_global.net.state = 1;
						(*correct) = xTaskGetTickCount();
					}
				}
				else
				{
					printf("rtc set err\r\n\r\n");
				}
			}
			else
			{
				printf("1f 30 ack err\r\n\r\n");
			}
		}
		else
		{
			printf("1f 30 no ack\r\n\r\n");
		}
		
		mail_release(cmail);
	}
	else
	{
		printf("apply mail err_1\r\n\r\n");
	}
}


/*
发送心跳
*/
static uint8_t send_heart(uint32_t *hearTime, uint8_t *xintiao)//心跳包
{
	uint8_t rs = TRUE ,err ;
	if(xTaskGetTickCount() - (*hearTime) >= ONE_SECOND*40)//40s发送一次心跳包
	{
		UART7_DATA *rx;
		
		(*hearTime) = xTaskGetTickCount();
		xintiao[0]=0xff;
		xintiao[1] = class_global.net.number+1;
		xintiao[2]=4;
		class_global.net.number = (class_global.net.number+1)%255;
		
		mail_to_bus(xintiao, &rx);
		err = commucation_sem_get(ONE_SECOND*10);
		if( !(err == pdTRUE && rx->buf[0]==0x1D && rx->buf[1] == xintiao[1] && rx->buf[2] == 4) )//需要判断心跳回应是否正确
		{
			class_global.net.state = 0;
			rs = FALSE;
			printf("heart ack err\r\n\r\n");
		}
	}
	return rs;
}

/*
发送对时
*/
static uint8_t send_correct_time(uint32_t *correct)//发对时
{
	uint8_t err,rs = TRUE; 
		
	if(xTaskGetTickCount()-(*correct) > ONE_SECOND*30*60)//30分对时一次
	{
		UART7_DATA  *rx;//接收缓冲指针
		MAIL *cmail;
		uint8_t *pt_tx;

		cmail = mail_apply(50);
		configASSERT(cmail);
		if(cmail != 0)
		{
			pt_tx = cmail->addr;
			
			(*correct) = xTaskGetTickCount();//更新对时计时
			
			get_link_info(pt_tx, 1);//编对时包
			pt_tx[1] = class_global.net.number+1;//加上编号
			class_global.net.number = (class_global.net.number+1)%255;//更新编号
			mail_to_bus(pt_tx, &rx);
			err = commucation_sem_get(ONE_SECOND*30);//30s 超时
			if(err == pdTRUE && rx->buf[0] == 0x1D && rx->buf[1] == pt_tx[1])
			{
				uint8_t ctime[6];
				ctime[0] = (rx->buf[3]-'0')*10 + rx->buf[4]-'0';
				ctime[1] = (rx->buf[5]-'0')*10 + rx->buf[6]-'0';
				ctime[2] = (rx->buf[7]-'0')*10 + rx->buf[8]-'0';
				ctime[3] = (rx->buf[9]-'0')*10 + rx->buf[10]-'0';
				ctime[4] = (rx->buf[11]-'0')*10 + rx->buf[12]-'0';
				ctime[5] = (rx->buf[13]-'0')*10 + rx->buf[14]-'0';
				
				set_rtc_time(1, ctime[0], ctime[1], ctime[2], ctime[3], ctime[4], ctime[5]);
				
				send_restart();//发送重启查询
			}
			else
			{	
				printf("updata time ack err\r\n\r\n");
				class_global.net.state = 0;
				rs = FALSE;
			}
			
			mail_release(cmail);//释放掉该信封
		}
		else
		{
			printf("apply mail err_2\r\n\r\n");
		}
	}
	return rs;
}

/*
@功能：发送即时数据
@返回：1成功
*/
static uint8_t send_instant_data(void)
{
	uint8_t rs, err;
	MAIL *cmail;
	UART7_DATA *rx;
	
	err = instant_data_get((void*)&cmail, ONE_SECOND/10);//取即时数据
	if(err == pdTRUE)
	{
		if(class_global.net.state)//这里是为了提高速度
		{
			rs = TRUE;
			cmail->addr[1] = class_global.net.number + 1;//发送流水号+1
			class_global.net.number = (class_global.net.number+1)%255;
			
			mail_to_bus(cmail->addr, &rx);
			err = commucation_sem_get(ONE_SECOND*10);
			if(err == pdTRUE)
			{
				if(cmail->com_call_back != 0)//如果注册了回调函数，则调用
				{
					if(!(cmail->com_call_back(rx->buf, rx->len, cmail->addr))) //回调函数处理
					{
						class_global.net.state = 0;//直接踹掉线，重新发起连接
						rs = 2;//掉线
					}
				}
				else//未注册回调函数（用于那些普通回复：表示收到或者什么的）
				{
					if(rx->buf[0] == 0x1D && rx->buf[1] == cmail->addr[1] && rx->buf[3] == ACK_OK)//应答正确
					{
						;
					}
					else//应答错误
					{
						class_global.net.state = 0;//直接开始重新连接
						rs = 2;//掉线
					}
				}
			}
			else//无应答掉线
			{
//				FSM_MSG* msg = msg_applay(MSG_SIZE_DEFAULT);
//				configASSERT(msg);
//				msg->type = MsgServerAckErr;//数据错
//				msg->stype = 1;	//掉线
//				msg->value = 1;	//即时包
//				if(fsm_cmd_send(TASK_MSG, msg) != pdPASS){
//					msg_release(msg);
//					printf("err-> ram data no answer\r\n\r\n");
//				}
//				class_global.net.state = 0;
//				rs = 2;//掉线
//				
//				server_err_save(0, cmail->addr);
			}
		}
		mail_release(cmail);//释放数据空间
	}
	else//没有数据了
	{
		rs = FALSE;
	}	
	return rs;
}

/*
@功能：发送汇报数据
@返回：1成功
*/
static uint8_t send_report_data(void)
{
	uint8_t rs = FALSE, err;
	MAIL *cmail;
	UART7_DATA *rx;
	
	if(class_global.net.backlog_data)//有积压数据
	{
		err = report_data_get((void*)&cmail, 0);//取汇报数据
		if(err == pdTRUE)
		{
			class_global.net.backlog_data ? (class_global.net.backlog_data--) : 0;//积压数据减1
			rs = TRUE;
			cmail->addr[1] = class_global.net.number + 1;//发送流水号+1
			class_global.net.number = (class_global.net.number+1)%255;
			
			mail_to_bus(cmail->addr, &rx);
			err = commucation_sem_get(ONE_SECOND*10);
			if(err == pdTRUE)
			{
				if(cmail->com_call_back)//如果注册了回调函数，则调用
				{
					if(!(cmail->com_call_back(rx->buf, rx->len, cmail->addr))) //回调函数处理
					{
						class_global.net.state = 0;//直接踹掉线，重新发起连接
						add_report_info(cmail, TRUE);//加到队列头
					}
					else
					{
						mail_release(cmail);//释放数据空间
					}
				}
				else//未注册回调函数（用于那些普通回复：表示收到或者什么的）
				{
					if(rx->buf[0] == 0x1D && rx->buf[1] == cmail->addr[1] && rx->buf[3] == ACK_OK)//应答正确
					{
						mail_release(cmail);//释放数据空间
					}
					else//应答错误
					{
						class_global.net.state = 0;//直接开始重新连接
						add_report_info(cmail, TRUE);//加到队列头
					}
				}
			}
			else//平台无应答
			{
//				FSM_MSG* msg = 0;
//				class_global.net.state = 0;
//				add_report_info(cmail, TRUE);//加到队列头
//				server_err_save(0, cmail->addr);//记录无应答数据
//				
//				msg = msg_applay(MSG_SIZE_DEFAULT);
//				configASSERT(msg);
//				msg->type = MsgServerAckErr;//数据错
//				msg->stype = 1;	//掉线
//				msg->value = 0;	//汇报包
//				if(fsm_cmd_send(TASK_MSG, msg) != pdPASS){
//					msg_release(msg);
//					printf("err-> ram data no answer\r\n\r\n");
//				}
			}
		}
		else//有积压计数，但是无积压数据
		{
			printf("no report != data num\r\n\r\n");
			rs = FALSE;
		}
	}
	return rs;
}






/*
@功能：通讯主任务
*/
void main_task_commucation(void)
{
	uint32_t heart,correct;
	uint8_t first_Link;
	uint8_t xintiao[6];
	uint8_t param_redownload = 0;
	
	heart = correct = xTaskGetTickCount();
	
	class_global.net.state = 0;	//状态复位
	class_global.net.number = 0;	//流水号复位
	
	xintiao[0]=0xff;
	xintiao[2]=4;
	first_Link = TRUE;
	
	uart7_config();//初始化串口
	vTaskDelay(ONE_SECOND/2);//等待所有设备初始化完成
	
	while(1)
	{
		main_task_4g();

		if(class_global.net.state)//如果连网中
		{
			first_Link = FALSE;
			if(send_instant_data() != 2)//没有即时数据需要发送
			{
				if(send_heart(&heart,xintiao))//心跳
					if(send_correct_time(&correct))//对时
						send_report_data();//汇报数据
			}
		}
		else
		{
			send_linking(&first_Link, (uint32_t*)&correct, &param_redownload);//联机
		}
	}
}
