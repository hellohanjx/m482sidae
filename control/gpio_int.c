#include "gpio_int.h"
#include "user_config.h"
#include "global.h"


/*
@功能：中断引脚配置
@说明：水阀计数引脚
*/
void _gpio_int_config(void)
{
	//配置引脚为输入
	GPIO_SetMode(PA, BIT5, GPIO_MODE_INPUT);//PA5	---
	
	GPIO_SetMode(PA, BIT3, GPIO_MODE_INPUT);//PA3 ---

	GPIO_SetMode(PB, BIT1, GPIO_MODE_INPUT);//PB1 ---

	GPIO_SetMode(PB, BIT15, GPIO_MODE_INPUT);//PB15 ---

	GPIO_SetMode(PB, BIT13, GPIO_MODE_INPUT);//PB13 ---

	GPIO_SetMode(PB, BIT7, GPIO_MODE_INPUT);//PB7 ---
	
	//中断配置
  GPIO_EnableInt(PA, 3, GPIO_INT_FALLING);
  GPIO_EnableInt(PA, 5, GPIO_INT_FALLING);

  GPIO_EnableInt(PB, 1, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 7, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 13, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 15, GPIO_INT_FALLING);

	//使能中断
	NVIC_SetPriority(GPA_IRQn, 6);
	NVIC_SetPriority(GPB_IRQn, 6);
	NVIC_EnableIRQ(GPB_IRQn);
	NVIC_EnableIRQ(GPA_IRQn);
	
	//开防抖
//	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_8);//这个可测量2us的脉冲
	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1);//这个可测量400us的脉冲
	
	//防抖配置
	GPIO_ENABLE_DEBOUNCE(PA, BIT3);//PA3
	GPIO_ENABLE_DEBOUNCE(PA, BIT5);//PA5
	
	GPIO_ENABLE_DEBOUNCE(PB, BIT1);//PB1
	GPIO_ENABLE_DEBOUNCE(PB, BIT7);//PB7
	GPIO_ENABLE_DEBOUNCE(PB, BIT13);//PB13
	GPIO_ENABLE_DEBOUNCE(PB, BIT15);//PB15
}

/*
@功能：GPIOA中断
*/
void GPA_IRQHandler(void)
{
	uint8_t rs = FALSE;
	
	if( GPIO_GET_INT_FLAG(PA, BIT3) )//PA3
	{
		GPIO_CLR_INT_FLAG(PA, BIT3);
		rs = TRUE;
		class_global.ireader[5].count[0].val++;//6通道主信号
		
		#if(GPIO_INT_LOG)
		printf("PA.3 INT occurred.\n");
		#endif
	}
		
	if( GPIO_GET_INT_FLAG(PA, BIT5) )//PA5
	{
		GPIO_CLR_INT_FLAG(PA, BIT5);
		rs = TRUE;
		class_global.ireader[4].count[0].val++;//5通道主信号
		
		#if(GPIO_INT_LOG)
		printf("PA.5 INT occurred.\n");
		#endif
	}	
	
	if(rs == FALSE)
	{
		/* Un-expected interrupt. Just clear all PC interrupts */
//		PC->INTSRC = PC->INTSRC;
		printf("Un-expected interrupts.\n");
	}
}


/*
@功能：GPIOB中断
*/
void GPB_IRQHandler(void)
{
	uint8_t rs = FALSE;
	
	if( GPIO_GET_INT_FLAG(PB, BIT1) )//PB1
	{
		GPIO_CLR_INT_FLAG(PB, BIT1);
		rs = TRUE;
		class_global.ireader[3].count[0].val++;//4通道副信号
		
		#if(GPIO_INT_LOG)
		printf("PB.1 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT7) )//PB7
	{
		GPIO_CLR_INT_FLAG(PB, BIT7);
		rs = TRUE;
		class_global.ireader[2].count[0].val++;//3通道主信号
		
		#if(GPIO_INT_LOG)
		printf("PB.7 INT occurred.\n");
		#endif
	}
		
	if( GPIO_GET_INT_FLAG(PB, BIT13) )//PB13
	{
		GPIO_CLR_INT_FLAG(PB, BIT13);
		rs = TRUE;
		class_global.ireader[0].count[0].val++;//1通道主信号

		#if(GPIO_INT_LOG)
		printf("PB.13 INT occurred.\n");
		#endif
	}
		
	if( GPIO_GET_INT_FLAG(PB, BIT15) )//PB15
	{
		GPIO_CLR_INT_FLAG(PB, BIT15);
		rs = TRUE;
		class_global.ireader[1].count[0].val++;//2通道主信号
		
		#if(GPIO_INT_LOG)
		printf("PB.15 INT occurred.\n");
		#endif
	}

	
	if(rs == FALSE)
	{
		/* Un-expected interrupt. Just clear all PC interrupts */
//		PC->INTSRC = PC->INTSRC;
		printf("Un-expected interrupts.\n");
	}
}

/*
@功能：水阀 开始计数/停止计数
@参数：channel，通道号；mode，0=停止，1=开始
*/
void tap_count_set(uint8_t channel, uint8_t mode)
{
	if(mode)//开始计数
	{
		switch( channel )
		{
			case 0:
				GPIO_EnableInt(PB, 13, GPIO_INT_FALLING);//主信号
			break;
			
			case 1:
				GPIO_EnableInt(PB, 15, GPIO_INT_FALLING);//主信号
			break;
			
			case 2:
				GPIO_EnableInt(PB, 7, GPIO_INT_FALLING);//主信号
			break;

			case 3:
				GPIO_EnableInt(PB, 1, GPIO_INT_FALLING);//主信号
			break;
			
			case 4:
				GPIO_EnableInt(PA, 5, GPIO_INT_FALLING);//主信号
			break;
			
			case 5:
				GPIO_EnableInt(PA, 3, GPIO_INT_FALLING);//主信号
			break;
		}
	}
	else
	{
		switch( channel )
		{
			case 0:
				GPIO_DisableInt(PB, 13);//主信号
			break;
			
			case 1:
				GPIO_DisableInt(PB, 15);//主信号
			break;
			
			case 2:
				GPIO_DisableInt(PB, 7);//主信号
			break;

			case 3:
				GPIO_DisableInt(PB, 1);//主信号
			break;
			
			case 4:
				GPIO_DisableInt(PA, 5);//主信号
			break;
			
			case 5:
				GPIO_DisableInt(PA, 3);//主信号
			break;
		}
	}
	class_global.ireader[channel].count[0].val = 0;//主信号计数清0
	class_global.ireader[channel].count[1].val = 0;//副信号计数清0
	
}
