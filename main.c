/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Demo application includes. */
#include "partest.h"
#include "flash.h"
#include "flop.h"
#include "integer.h"
#include "PollQ.h"
#include "semtest.h"
#include "dynamic.h"
#include "BlockQ.h"
#include "blocktim.h"
#include "countsem.h"
#include "GenQTest.h"
#include "QueueSet.h"
#include "recmutex.h"
#include "death.h"

/* Hardware and starter kit includes. */
#include "NuMicro.h"

#include "hardware.h"

/* Priorities for the demo application tasks. */
#define mainFLASH_TASK_PRIORITY             ( tskIDLE_PRIORITY + 1UL )
#define mainQUEUE_POLL_PRIORITY             ( tskIDLE_PRIORITY + 2UL )
#define mainSEM_TEST_PRIORITY               ( tskIDLE_PRIORITY + 1UL )
#define mainBLOCK_Q_PRIORITY                ( tskIDLE_PRIORITY + 2UL )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3UL )
#define mainFLOP_TASK_PRIORITY              ( tskIDLE_PRIORITY )
#define mainCHECK_TASK_PRIORITY             ( tskIDLE_PRIORITY + 3UL )

#define mainCHECK_TASK_STACK_SIZE           ( configMINIMAL_STACK_SIZE )

/* The time between cycles of the 'check' task. */
#define mainCHECK_DELAY                     ( ( portTickType ) 5000 / portTICK_RATE_MS )

/* The LED used by the check timer. */
#define mainCHECK_LED                       ( 3UL )

/* A block time of zero simply means "don't block". */
#define mainDONT_BLOCK                      ( 0UL )

/* The period after which the check timer will expire, in ms, provided no errors
have been reported by any of the standard demo tasks.  ms are converted to the
equivalent in ticks using the portTICK_RATE_MS constant. */
#define mainCHECK_TIMER_PERIOD_MS           ( 3000UL / portTICK_RATE_MS )

/* The period at which the check timer will expire, in ms, if an error has been
reported in one of the standard demo tasks.  ms are converted to the equivalent
in ticks using the portTICK_RATE_MS constant. */
#define mainERROR_CHECK_TIMER_PERIOD_MS     ( 200UL / portTICK_RATE_MS )

/* Set mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 1 to create a simple demo.
Set mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 0 to create a much more
comprehensive test application.  See the comments at the top of this file, and
the documentation page on the http://www.FreeRTOS.org web site for more
information. */
#define mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY     0






#define COMMUCATION_TASK_PRIO		23		//任务优先级
#define COMMUCATION_STK_SIZE 		256		//任务堆栈大小

static void register_unlock_set( void );


static void communication_task( void *pvParameters );
static void idle_task( void *pvParameters );




int main(void)
{
    register_unlock_set();

    xTaskCreate( idle_task, "Check", mainCHECK_TASK_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );//创建空闲任务
	  xTaskCreate( communication_task, "Commucation", COMMUCATION_STK_SIZE, NULL, COMMUCATION_TASK_PRIO, NULL );//创建通信任务
	
    vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );//创建队列
    
		printf("FreeRTOS is starting ...\n");
    vTaskStartScheduler();
	
    for( ;; );//如果运行到这里，有问题
}


/*
@功能：配置需要解除锁定的寄存器
*/
static void register_unlock_set( void )
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
//    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);//使能内部高速晶振
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);//使能内部低速晶振

SYS->IRCTCTL |= SYS_IRCTCTL_REFCKSEL_Msk;//12MHz晶振校准	
//SYS->HIRCTCTL |= SYS_HIRCTCTL_REFCKSEL_Msk;//48MHz晶振校准	
	
    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);//等待内部高速晶振准备好
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);//等待内部低速晶振准备好

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2;

    /* Enable IP clock */
    CLK_EnableModuleClock(TMR0_MODULE);//使能计时器0时钟

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);//选择定时器0时钟源


		hardware_config();

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();


//    PH->MODE = (PH->MODE & ~(GPIO_MODE_MODE0_MSK | GPIO_MODE_MODE1_MSK | GPIO_MODE_MODE2_MSK)) |
//               (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_POS) |
//               (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_POS) |
//               (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_POS);  // SET TO OUTPUT MODE

    /* Lock protected registers */
    SYS_LockReg();

}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()).  */

#if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 0 )
    {
        /* In this case the tick hook is used as part of the queue set test. */
        vQueueSetAccessQueueSetFromISR();
    }
#endif /* mainCREATE_SIMPLE_BLINKY_DEMO_ONLY */
}



/*******************************************************************
@空闲任务
*******************************************************************/
static void idle_task( void *pvParameters )
{
	uint8_t state = FALSE;
	portTickType xLastExecutionTime;
	
	xLastExecutionTime = xTaskGetTickCount();
	printf("Check Task is running ...\n");

	for( ;; )
	{
		LED_BREATH = state;
		LED_NET = !state;
		state = !state;
		
		/* Perform this check every mainCHECK_DELAY milliseconds. */
		vTaskDelayUntil( &xLastExecutionTime, 1000 );//绝对延时函数，每隔一段时间执行一次
		
		if( xArePollingQueuesStillRunning() != pdTRUE )//检测所有任务是否运行正常
		{
			printf( "ERROR IN POLL Q\n" );
		}
	}
}


/*******************************************************************
@通信任务
*******************************************************************/
#include "uart6_config.h"

static void communication_task( void *pvParameters )
{
	uint8_t tmp[6], count = 0, len, ccl = 0;
	pvParameters = pvParameters;
	
	for(;;)
	{
		for(; ccl < 10; ccl++)
		{
			UART6_DATA tx, *rx;
			len = sprintf((char*)tmp, "%u", count++);
			tmp[len] = 0;
			
			tx.len = 0;
			tx.buf[tx.len++] = 'S';
			tx.buf[tx.len++] = 'C';
			tx.buf[tx.len++] = '0';
			tx.buf[tx.len++] = '\r';
			tx.buf[tx.len++] = '\n';
			_uart6_send(&tx, &rx, 0);
			vTaskDelay(1);
		}
//		SCUART_Write(SC0, tmp, len);
//		SCUART_Write(SC0, "\r\n", sizeof("\r\n"));
		
		vTaskDelay(2000);
	}
}

