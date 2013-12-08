/******************************************************************************
Filename    : MP_config.h
Author      : pry 
Date        : 22/07/2012
Version     : 0.12
Description : The RMP system configuration file.
******************************************************************************/

/* Preprocessor Control ******************************************************/
#ifndef __MP_CONFIG_H__
#define __MP_CONFIG_H__

/* Compiler Configuration ****************************************************/
/* Compiler keywords */
#define INLINE                      __inline
#define EXTERN                      extern
/* End Compiler Configuration ************************************************/
                                                      
/* System Basic Configuration ************************************************/
/* Configuration choice */
#define TRUE                        1
#define FALSE                       0

/* Basic types */
#define DEFINE_BASIC_TYPES		    FALSE									  
/* End System Basic Configuration ********************************************/

/* Kernel Configuration ******************************************************/                                  
/* Stack configuration */
/* The stack safe redundancy. This is for safety reasons only. Must be a multiple
 * of 4 in CM3
 */
#define STACK_SAFE_RDCY             4*4
/* The preset stack size. This is for determining the place where we should 
 * place the prepared stack for switching into a task for the first time.
 * The first stack element will be placed at Init_Stack_Addr-PRESET_STACK
 * _SIZE.
 */
#define PRESET_STACK_SIZE           16*4
#define KERNEL_STACK_SIZE           100
#define ARCH_STACK_SIZE             200
#define APP_STACK_1_SIZE            200
#define APP_STACK_2_SIZE            200
#define APP_STACK_3_SIZE            200

/* Pseudo-process(Pprocess) configuration */
/* #define NULL                     0 */

/* The maximum number of priority levels */
#define MAX_PRIO_NUM                20
/* The maximum number of processes running at the same time */ 
#define MAX_PROC_NUM                7
/* The maximum stack depth in bytes */	                                              
#define MAX_STACK_DEP               100                                              						
/* System timer. In CM3, cannot be bigger than 0xFFFFFF */
#define MIN_TIMESLICE_TICK          72*1000         

/* If you enable load balance here, the kernel will automatically adjust the processor
 * frequency so that the process ready at a certain priority level is a constant.
 * This is not implemented yet.
 */
#define ENABLE_LOAD_BALANCE         FALSE
/* The base priority of the load balancer */
#define LOAD_BALANCE_BASE_PRIO      1
/* The basic number of processes for load balance */
#define LOAD_BALANCE_BASE_PROCS     2
/* End Kernel Configuration **************************************************/

/* Memory Management Configuration *******************************************/
/* Switch */
#define ENABLE_MEMM 	            TRUE	
/* Application memory size */
#define DMEM_SIZE	                0x2000				                   
/* The first/second level segregation table length and width for the OS. The SLI
 * is fixed to 8.
 */
#define MM_FLI                      10      
/* End Memory Manegement Configuration ***************************************/

/* Semaphore Configuration ***************************************************/
/* Switch */							
#define ENABLE_SEM                  TRUE
/* The maximum number of resources in the system */
#define MAX_SEMS                    5
/* The maximum number of processes that will use the semaphores */
#define MAX_PROC_USE_SEM            4				                                 
/* End Semaphore Configuration ***********************************************/

/* Mutex Configuration *******************************************************/
/* Switch */
#define ENABLE_MUTEX                TRUE
/* The maximum number of mutexs we will use in the system */
#define MAX_MUTEXS                  5
/* End Mutex Configuration ***************************************************/

/* Pipe Configuration ********************************************************/
/* Switch */
#define ENABLE_PIPE                 TRUE
/* The maximum number of pipes in the system */
#define MAX_PIPES                   4
/* End Pipe Configuration ****************************************************/

/* Shared Memory Configuration ***********************************************/
/* Switch */
#define ENABLE_SHMEM                TRUE
#define MAX_SHMS                    3
/* End Shared Memory Configuration *******************************************/

/* Message Queue Configuration ***********************************************/
#define ENABLE_MSGQ                 TRUE
#define MAX_MSG_QUEUES              6
/* End Message Queue Configuration *******************************************/

/* Wait For Object Configuration *********************************************/
#define MAX_WAIT_BLOCKS             10
/* End Wait For Object Configuration *****************************************/

/* System Timer Configuration ************************************************/
#define ENABLE_TIMER                TRUE
#define MAX_TIMS                    2
/* "Sys_Raw_Delay" raw-style function trim value */
#define DELAYTRIM       			110	
/* System Systick interrupt frequency */
#define SYSTICK_FREQ                1000
/* End System Timer Configuration ********************************************/

/* Syslib Configuration ******************************************************/
/* Now the system library must be enabled */         
/* The maximum string length that the syslib are capable of */
#define MAX_STR_LEN     			1024                    	       
/* End Syslib Configuration **************************************************/      

#endif
/* End Preprocessor Control **************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
