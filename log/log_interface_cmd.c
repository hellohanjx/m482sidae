/*
@说明：处理日志接口的接收
*/
#include "msg.h"
#include "uart0_log.h"
#include "string.h"
#include "global.h"
#include "stdlib.h"
#include "isp_program.h"
#include "log_interface_cmd.h"
#include "4G.h"
#include "temp.h"

/*
@功能：日志接收处理任务
*/
void task_log_recv(void)
{
	UART0_DATA rx;
	char tmp[10] = {0};
	uint8_t err, len, i;
	uint16_t ip[4];
	uint32_t val;
	
	for(;;)
	{
		err = log_queue_get((void*)&rx, portMAX_DELAY);//现在是值拷贝[freeRtos];如果使用指针传递，则 UART0_DATA rx -> UART0_DATA *rx
		if(err == pdTRUE)
		{
			if( !strncasecmp((char*)rx.buf, "at+", sizeof("at+") - 1 ) )
			{
				if(rx.buf[rx.len - 2] == '\r' && rx.buf[rx.len - 1] == '\n')
				{
					if(rx.len > 3)
					{
						/********************** 版本号 ****************************/
						if( !strncasecmp((char*)&(rx.buf[3]), "version", sizeof("version") - 1) )//version
						{
							len = sizeof("version") + 2;
							if(rx.buf[len] == '?')//设置机器id
							{
								printf("version = ");
								printf((const char*)main_version);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 机器ID ****************************/
						else						
						if( !strncasecmp((char*)&(rx.buf[3]), "id", sizeof("id") - 1) )
						{
							len = sizeof("id") + 2;
							if(rx.buf[len] == '=')//设置机器id
							{
								if(rx.buf[len+1] >= '0' && rx.buf[len+1] <= '9' )
								{
									if(rx.len - len - 3 == 10 && rx.buf[len+1] > '3')
									{
										printf("ID is bigger\r\n");
									}
									else
									{
										val = strtoul((char*)&rx.buf[len+1], NULL, 10);//转换为无符号长整形
										if( flash_param_set(FLASH_LIST_Id, FLASH_ADDR_Id, val) )
										{
											printf("ID set sucess\r\n");
										}
										else
										{
											printf("ID set failure\r\n");
										}
									}
								}
								else
								{
									printf("ID first letter err\r\n");
								}
							}
							else
							if(rx.buf[len] == '?')//查询机器id
							{
								val = flash_param_get(FLASH_LIST_Id, FLASH_ADDR_Id);
								tmp[sprintf(tmp, "%010u", val)] = 0;
								printf("ID = ");
								printf(tmp);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 机器IP ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "ip", sizeof("ip") - 1) )//ip
						{
							char ip_char[4][4] = {0}, n, m;
							len = sizeof("ip") + 2;
							if(rx.buf[len] == '=')//设置机器ip
							{
								if(rx.buf[len+1] >= '0' && rx.buf[len+1] <= '9' )
								{
									for(i = 0, val = 0, n = 0, m = 0;  (i < (rx.len - 8)) && ((rx.buf[i+len+1] >= '0' && rx.buf[i+len+1] <= '9') || rx.buf[i+len+1] == '.');   i++)//遍历接收的ip号
									{
										if(rx.buf[i+len+1] == '.')//IP分隔符
										{
											if(n < 4)
											{
												n++;
												m = 0;
											}
											else
											{
												printf("IP format err - 1");
												break;
											}
											val++;
											i++;
										}
										
										if(m < 4)
										{
											ip_char[n][m++] = rx.buf[i+len+1];
										}
										else
										{
											printf("IP format err - 2");
											break;
										}
									}

									if(val == 3)//IP段有3个'.'
									{
										ip[0] = strtoul(&ip_char[0][0], NULL, 10);//转换为无符号长整形
										ip[1] = strtoul(&ip_char[1][0], NULL, 10);
										ip[2] = strtoul(&ip_char[2][0], NULL, 10);
										ip[3] = strtoul(&ip_char[3][0], NULL, 10);
										
										if(ip[0] > 255 || ip[1] > 255 || ip[2] > 255 || ip[3] > 255)
										{
											printf("IP value exceed\r\n");
										}
										else
										{
											if( flash_param_set( FLASH_LIST_Ip, FLASH_ADDR_Ip, (((uint8_t)ip[0] << 24) | ((uint8_t)ip[1] << 16) | ((uint8_t)ip[2] << 8) | ((uint8_t)ip[3])) ) )
											{
												printf("IP set sucess\r\n");
											}
											else
											{
												printf("IP set failure\r\n");
											}
										}
									}
									else
									{
										printf("IP segment err\r\n");
									}
								}
								else
								{
									printf("IP first letter err\r\n");
								}
							}
							else
							if(rx.buf[len] == '?')//查询机器ip
							{
								char ip_char[15] = {0};
								val = flash_param_get(FLASH_LIST_Ip, FLASH_ADDR_Ip);//ip
								ip[0] = val >> 24;
								ip[1] = val >> 16;
								ip[2] = val >> 8;
								ip[3] = val & 0xff;
								
								for(i = 0; i < 4; i++)//数字ip转换为字符串，并且拼起来
								{
									tmp[sprintf(tmp, "%u", (uint8_t)ip[i])] = 0;
									strcat((char*)ip_char, tmp);
									if(i != 3)
										strcat((char*)ip_char, (char*)".");
								}
								
								printf("IP = ");
								printf((char*)ip_char);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 机器PORT ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "port", sizeof("port") - 1) )//port
						{
							len = sizeof("port") + 2;
							if(rx.buf[len] == '=')//设置机器短偶
							{
								if(rx.buf[len+1] >= '0' && rx.buf[len+1] <= '9' )
								{
									val = strtoul((char*)&rx.buf[len+1], NULL, 10);//转换为无符号长整形
									if(val > 65535)
									{
										printf("port is bigger\r\n");
									}
									else
									{
										if( flash_param_set(FLASH_LIST_Port, FLASH_ADDR_Port, val) )
										{
											printf("Port set success\r\n");
										}
										else
										{
											printf("Port set failure\r\n");
										}
									}
								}
								else
								{
									printf("Port first letter err\r\n");
								}
							}
							else
							if(rx.buf[len] == '?')//查询机器端口
							{
								val = flash_param_get(FLASH_LIST_Port, FLASH_ADDR_Port);
								tmp[sprintf(tmp, "%u", val)] = 0;
								printf("Port = ");
								printf(tmp);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 查询刷卡器状态 ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "ireader", sizeof("ireader") - 1) )//card
						{
							len = sizeof("ireader") + 2;
							if(rx.buf[len] == '?')//查询机器端口
							{
								char tmp, i, id;
								for(i = 0; i < IREADER_NUM_MAX; i++)
								{
									id = i+'1';
									tmp = class_global.ireader[i].equ.state + '0';
									printf("ireader_");
									printf(&id);
									printf("= ");
									printf(&tmp);
									printf("\r\n");
								}
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 查询4G状态 ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "4G", sizeof("4G") - 1) )//card
						{
							len = sizeof("4G") + 2;
							if(rx.buf[len] == '?')//查询机器端口
							{
								tmp[sprintf(tmp, "%u", get_4g_state())] = 0;
								printf("4G state = ");
								printf(tmp);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 查询sim卡ICCID ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "ICCID", sizeof("ICCID") - 1) )//card
						{
							len = sizeof("ICCID") + 2;
							if(rx.buf[len] == '?')//查询机器端口
							{
								printf("ICCID = ");
								tmp[sprintf(tmp, "%010u", class_global.sys.unique_id[0])] = 0;
								printf(tmp);
								tmp[sprintf(tmp, "%010u", class_global.sys.unique_id[1])] = 0;
								printf(tmp);
								tmp[sprintf(tmp, "%010u", class_global.sys.unique_id[2])] = 0;
								printf(tmp);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 查询 FSM 状态 ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "WORK", sizeof("WORK") - 1) )
						{
							len = sizeof("WORK") + 2;
							if(rx.buf[len] == '?')//查询机器端口
							{
								printf("work state = ");
								for( i = 0; i < IREADER_NUM_MAX; i++ )
								{
									tmp[sprintf(tmp, "%u", class_global.ireader[i].equ.work)] = 0;
									printf(tmp);
									printf(",");
								}
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 查询交易状态 ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "TRADE", sizeof("TRADE") - 1) )
						{
							len = sizeof("TRADE") + 2;
							if(rx.buf[len] == '?')//查询机器端口
							{
								printf("trade state = ");
								for( i = 0; i < IREADER_NUM_MAX; i++ )
								{
									tmp[sprintf(tmp, "%u", class_global.trade.fsm[i])] = 0;
									printf(tmp);
									printf(",");
								}
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 查温度 ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "TEMP", sizeof("TEMP") - 1) )
						{
							len = sizeof("TEMP") + 2;
							if(rx.buf[len] == '?')//查询机器端口
							{
								get_external_temp( (int*)&class_global.temp.external.val, (uint8_t*)&class_global.temp.external.state );//获取外部温度
								class_global.temp.internal.val = get_internal_temp();//获取cpu温度
								
								printf("external temp = ");
								if( class_global.temp.external.state )
								{
									tmp[ sprintf(tmp, "%u", class_global.temp.external.val) ] = 0;
									printf(tmp);
									printf("\r\n");
								}
								else
								{
									printf("NULL\r\n");
								}
								
								printf("internal temp = ");
								tmp[ sprintf(tmp, "%u", class_global.temp.internal.val) ] = 0;
								printf(tmp);
								printf("\r\n");
							}
							else
							{
								printf("No support\r\n");
							}
						}

						/********************** 复位 ****************************/
						else
						if( !strncasecmp((char*)&(rx.buf[3]), "reset", sizeof("reset") - 1) )//port
						{
							len = sizeof("reset") + 2;
							if(rx.buf[len] == '=')//设置机器短偶
							{
								__set_FAULTMASK(1);
								NVIC_SystemReset();
							}
							else
							{
								printf("No support\r\n");
							}
						}
						
						/********************** 不支持的命令 ****************************/
						else
						{
							printf("cmd is not support\r\n");
						}
					}
					else
					{
						printf("cmd too short\r\n");
						
					}
				}
				else
				{
					printf("cmd tail err\r\n");
				}
			}
			else
			{
				printf("cmd head err\r\n");
			}
		}
	}
}
