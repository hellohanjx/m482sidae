/*
    FreeRTOS V7.4.0 - Copyright (C) 2013 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.

    >>>>>>NOTE<<<<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details. You should have received a copy of the GNU General Public License
    and the FreeRTOS license exception along with FreeRTOS; if not itcan be
    viewed here: http://www.freertos.org/a00114.html and also obtained by
    writing to Real Time Engineers Ltd., contact details for whom are available
    on the FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, and our new
    fully thread aware and reentrant UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems, who sell the code with commercial support,
    indemnification and middleware, under the OpenRTOS brand.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.
*/


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined (__ICCARM__) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif

#ifdef __ARMCC_VERSION
#include "NuMicro.h"
#endif

#define configUSE_PREEMPTION            1			//=1使用抢占式内核；=0使用协程
//#define configUSE_TIME_SLICING					1						//1使能时间片调度(默认式使能的)
//#define configUSE_PORT_OPTIMISED_TASK_SELECTION	1   //1=启用特殊方法来选择下一个要运行的任务

#define configUSE_TICKLESS_IDLE					0                       			//1=启用低功耗tickless模式
#define configCPU_CLOCK_HZ              ( SystemCoreClock )						//CPU频率
#define configTICK_RATE_HZ              ( ( portTickType ) 1000 )			//时钟节拍频率，这里设置为1000，周期就是1ms
#define configMAX_PRIORITIES            ( 32 )												//可使用的最大优先级
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 130 )		//空闲任务使用的堆栈大小
#define configMAX_TASK_NAME_LEN         ( 16 )												//任务名字字符串长度

#define configUSE_16_BIT_TICKS          0			//系统节拍计数器变量数据类型：1=16位无符号整形，0=32位无符号整形
#define configIDLE_SHOULD_YIELD         1			//=1时空闲任务放弃CPU使用权给其他同优先级的用户任务
#define configUSE_TASK_NOTIFICATIONS    1    	//=1时开启任务通知功能，默认开启

#define configCHECK_FOR_STACK_OVERFLOW  0			//>0时启用堆栈溢出检测功能，如果使用此功能用户必须提供一个栈溢出钩子函数，如果使用的话此值可以为1或者2，因为有两种栈溢出检测方法。

#define configUSE_APPLICATION_TASK_TAG  0			

#define configUSE_COUNTING_SEMAPHORES   1			//=1时使用计数信号量
#define configUSE_MUTEXES               1			//=1时使用互斥信号量
#define configUSE_RECURSIVE_MUTEXES     1			//=1时使用递归互斥信号量

#define configUSE_QUEUE_SETS            1			//=1时启用队列
#define configQUEUE_REGISTRY_SIZE       16		//=x系统队列数量+信号量数量，=0不启用队列和信号量

/***************************************************************************************************************/
/*                                FreeRTOS与内存申请有关配置选项                                                */
/***************************************************************************************************************/
#define configSUPPORT_DYNAMIC_ALLOCATION        1                       //支持动态内存申请
#define configTOTAL_HEAP_SIZE										((size_t)(60*1024))     //系统所有总的堆大小(分配30k)


/***************************************************************************************************************/
/*                                FreeRTOS与钩子函数有关的配置选项                                              */
/***************************************************************************************************************/
#define configUSE_IDLE_HOOK             0			//=1使用空闲钩子；=0不使用
#define configUSE_TICK_HOOK             0			//=1使用时间片钩子；=0不使用
#define configUSE_MALLOC_FAILED_HOOK    0			//=1使用内存申请失败钩子函数


/***************************************************************************************************************/
/*                                FreeRTOS与运行时间和任务状态收集有关的配置选项                                 */
/***************************************************************************************************************/
//#define configGENERATE_RUN_TIME_STATS   				0			//=1时启用运行时间统计功能,portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()这个需要实现
#define configUSE_TRACE_FACILITY        				1			//=1启用可视化跟踪调试
#define configUSE_STATS_FORMATTING_FUNCTIONS		1     //与宏configUSE_TRACE_FACILITY同时为1时会编译下面3个函数
																											//prvWriteNameToBuffer(),vTaskList(),
																											//vTaskGetRunTimeStats()
/***************************************************************************************************************/
/*                                FreeRTOS与协程有关的配置选项                                                  */
/***************************************************************************************************************/
/* Co-routine definitions. */
#define configUSE_CO_ROUTINES       			0				//=1时启用协程，启用协程以后必须添加文件croutine.c
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )			//协程的有效优先级数目



/***************************************************************************************************************/
/*                                FreeRTOS与软件定时器有关的配置选项                                            */
/***************************************************************************************************************/
/* Software timer definitions. */
#define configUSE_TIMERS                1																		//=1时启用软件定时器
#define configTIMER_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )				//软件定时器优先级
#define configTIMER_QUEUE_LENGTH        10																	//软件定时器队列长度
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )		//软件定时器任务堆栈大小


/***************************************************************************************************************/
/*                                FreeRTOS可选函数配置选项                                                      */
/***************************************************************************************************************/
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   1
#define INCLUDE_vTaskSuspend            1		//队列死等
#define INCLUDE_vTaskDelayUntil         1		//绝对延时
#define INCLUDE_vTaskDelay              1		//使用延时


/***************************************************************************************************************/
/*                                FreeRTOS与中断有关的配置选项                                                  */
/***************************************************************************************************************/
/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
	#define configPRIO_BITS             4        /* Assembly code compile error. Because applied U Suffix to __NVIC_PRIO_BITS(M480.h).*/
#else
	#define configPRIO_BITS             4        /* 15 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0xf		//中断最低优先级 stm32 0~15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5		//操作系统可管理的最高中断优先级

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


/***************************************************************************************************************/
/*                                            断言                                                             */
/***************************************************************************************************************/
/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
//#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__,__LINE__)

	
/***************************************************************************************************************/
/*                                FreeRTOS与中断服务函数有关的配置选项                                          */
/***************************************************************************************************************/
/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler 		SVC_Handler
#define xPortPendSVHandler 	PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
