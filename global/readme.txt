版本记录：

211221A		1.modify	系统复位后如果TCP连接还在，关闭TCP连接。这时需要等待一点时间【100mS】，否则老王的平台会传输一条错误数据，导致对时失败；重新上电无此问题
			1.modify	刷卡器红灯亮表示待刷卡；蓝灯亮表示正在出水；红蓝一起亮表示卡已经拿走，但是不够指定【当前为5】脉冲数，继续出水，直到脉冲够了或者超时
					
211223		1.modify	状态机队列获取内容等待时间按 6s->1s 【fsm_queue_get((void*)&msg, ONE_SECOND)】
			2.+++		日志中增加刷卡器状态查询，4G状态机查询

211225		1.modify	修改默认机器号，默认ip，默认端口号，默认工厂模式->用户模式
			2.modify	修改卡加密算法
				
220101		1.modify	hardware.c 文件中 注释uart6_config(); 添加了这个初始化会造成上电时刷卡头4无法正常通信。测试发现因是上电时这个接口会发出/接收几字节数据

220120		1.modify	解决 delay.c 中，”单次计时“-> while( !TIMER3->INTSTS & TIMER_INTSTS_TIF_Msk ); 远超定时时间，可达到几百mS~8S
			2.modify	解决 fsm.c 中，class_global.trade.fsm 改为数组存储，否则不同通道同时刷卡会冲掉状态
			
220122		1.+++		添加外部温度显示，@@格式化可以补空格  sprintf( tmp, "% 4d", class_global.temp.external.val );
			2.modify	版本号改为NU开头的
			3.modify	版本号和机器ID显示位置交换
			4.+++		日志指令添加查询内部/外部温度指令
			