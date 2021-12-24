#include "gpio_int.h"
#include "user_config.h"

/*
@功能：中断引脚配置
*/
void _gpio_int_config(void)
{
	//配置引脚为输入
	GPIO_SetMode(PA, BIT5, GPIO_MODE_INPUT);//PA5
	GPIO_SetMode(PA, BIT4, GPIO_MODE_INPUT);//PA4
	
	GPIO_SetMode(PA, BIT3, GPIO_MODE_INPUT);//PA3
	GPIO_SetMode(PA, BIT2, GPIO_MODE_INPUT);//PA2

	GPIO_SetMode(PB, BIT1, GPIO_MODE_INPUT);//PB1
	GPIO_SetMode(PB, BIT0, GPIO_MODE_INPUT);//PB0

	GPIO_SetMode(PB, BIT15, GPIO_MODE_INPUT);//PB15
	GPIO_SetMode(PB, BIT14, GPIO_MODE_INPUT);//PB14

	GPIO_SetMode(PB, BIT13, GPIO_MODE_INPUT);//PB13
	GPIO_SetMode(PB, BIT12, GPIO_MODE_INPUT);//PB12

	GPIO_SetMode(PB, BIT7, GPIO_MODE_INPUT);//PB7
	GPIO_SetMode(PA, BIT6, GPIO_MODE_INPUT);//PA6
	
	//中断配置
  GPIO_EnableInt(PA, 2, GPIO_INT_FALLING);
  GPIO_EnableInt(PA, 3, GPIO_INT_FALLING);
  GPIO_EnableInt(PA, 4, GPIO_INT_FALLING);
  GPIO_EnableInt(PA, 5, GPIO_INT_FALLING);
  GPIO_EnableInt(PA, 6, GPIO_INT_FALLING);

  GPIO_EnableInt(PB, 0, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 1, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 7, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 12, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 13, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 14, GPIO_INT_FALLING);
  GPIO_EnableInt(PB, 15, GPIO_INT_FALLING);

	//使能中断
	NVIC_EnableIRQ(GPB_IRQn);
	NVIC_EnableIRQ(GPA_IRQn);
	
	//开防抖
//	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_1);//这个可测量2us的脉冲
	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1);//这个可测量400us的脉冲
	
	//防抖配置
	GPIO_ENABLE_DEBOUNCE(PA, BIT2);//PA2
	GPIO_ENABLE_DEBOUNCE(PA, BIT3);//PA3
	GPIO_ENABLE_DEBOUNCE(PA, BIT4);//PA4
	GPIO_ENABLE_DEBOUNCE(PA, BIT5);//PA5
	GPIO_ENABLE_DEBOUNCE(PA, BIT6);//PA6
	
	GPIO_ENABLE_DEBOUNCE(PB, BIT0);//PB0
	GPIO_ENABLE_DEBOUNCE(PB, BIT1);//PB1
	GPIO_ENABLE_DEBOUNCE(PB, BIT7);//PB7
	GPIO_ENABLE_DEBOUNCE(PB, BIT12);//PB12
	GPIO_ENABLE_DEBOUNCE(PB, BIT13);//PB13
	GPIO_ENABLE_DEBOUNCE(PB, BIT14);//PB14
	GPIO_ENABLE_DEBOUNCE(PB, BIT15);//PB15
}

/*
@功能：GPIOA中断
*/
void GPA_IRQHandler(void)
{
	uint8_t rs = FALSE;
	if( GPIO_GET_INT_FLAG(PA, BIT2) )//PA2
	{
		GPIO_CLR_INT_FLAG(PA, BIT2);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PA.2 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PA, BIT3) )//PA3
	{
		GPIO_CLR_INT_FLAG(PA, BIT3);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PA.3 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PA, BIT4) )//PA4
	{
		GPIO_CLR_INT_FLAG(PA, BIT4);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PA.4 INT occurred.\n");
		#endif

	}
	
	if( GPIO_GET_INT_FLAG(PA, BIT5) )//PA5
	{
		GPIO_CLR_INT_FLAG(PA, BIT5);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PA.5 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PA, BIT6) )//PA6
	{
		GPIO_CLR_INT_FLAG(PA, BIT6);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PA.6 INT occurred.\n");
		#endif
	}
	
	
	if(rs == FALSE)
	{
		/* Un-expected interrupt. Just clear all PC interrupts */
		PC->INTSRC = PC->INTSRC;
		printf("Un-expected interrupts.\n");
	}
}


/*
@功能：GPIOB中断
*/
void GPB_IRQHandler(void)
{
	uint8_t rs = FALSE;
	if( GPIO_GET_INT_FLAG(PB, BIT0) )//PB0
	{
		GPIO_CLR_INT_FLAG(PB, BIT0);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.0 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT1) )//PB1
	{
		GPIO_CLR_INT_FLAG(PB, BIT1);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.1 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT7) )//PB7
	{
		GPIO_CLR_INT_FLAG(PB, BIT7);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.7 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT12) )//PB12
	{
		GPIO_CLR_INT_FLAG(PB, BIT12);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.12 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT13) )//PB13
	{
		GPIO_CLR_INT_FLAG(PB, BIT13);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.13 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT14) )//PB14
	{
		GPIO_CLR_INT_FLAG(PB, BIT14);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.14 INT occurred.\n");
		#endif
	}
	
	if( GPIO_GET_INT_FLAG(PB, BIT15) )//PB15
	{
		GPIO_CLR_INT_FLAG(PB, BIT15);
		rs = TRUE;
		
		#if(GPIO_INT_LOG)
		printf("PB.15 INT occurred.\n");
		#endif
	}

	
	if(rs == FALSE)
	{
		/* Un-expected interrupt. Just clear all PC interrupts */
		PC->INTSRC = PC->INTSRC;
		printf("Un-expected interrupts.\n");
	}
}


