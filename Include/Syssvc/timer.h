/******************************************************************************
Filename   : timer.h
Author     : pry
Date       : 25/04/2012
Description: The header for the timer module of the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __TIMER_H_DEFS__
#define __TIMER_H_DEFS__

/*****************************************************************************/
/* Status - Timer stop */
#define  TIM_STOP               0x00
/* Status - Timer run */
#define  TIM_RUN                0x01
/* Status - Timer is waiting for reload in the arch process */
#define  TIM_RELOAD             0x02
/* Status - Timer is stopped in the reloading process */
#define  TIM_RELOAD_STOP        0x03

/* Auto-reload */
#define  TIM_AUTORELOAD     0x00
/* Manual-reload */
#define  TIM_MANUAL         0x01

/* Invalid timer setting */
#define  EINVTIM      0x00
/* No empty timer control blocks */
#define  ENOETIM      0x01
/* The timer with the same name is already in the system */
#define  ETIMEXIST    0x02
/*****************************************************************************/

/* __TIMER_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __TIMER_H_STRUCTS__
#define __TIMER_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* The struct of the timer control block */
struct Timer
{
    struct List_Head Head;
    struct List_Head Status_List_Head;
    s8* Timer_Name;
    timid_t Timer_ID;
    size_t Timer_Mode;
    size_t Timer_Status;
    /* Total ticks is called jiffies in some other OS. Here we use this name for
     * lowering the understanding difficulty.
     */
    struct Tick_Time Timer_Start_Total_Ticks;
    /* This is only valid when running */
    struct Tick_Time Timer_End_Total_Ticks;
    /* This is the current delay ticks. Only refreshed when stopped! Don't read this 
     * for the real remaining time when running!
     */ 
    struct Tick_Time Timer_Cur_Delay_Ticks;
    /* The timer's capacity */
    struct Tick_Time Timer_Max_Delay_Ticks;
    /* Timer's expire handler */
    ptr_int_t Timer_Expire_Handler;
    /* The expire handler's argument */
    u32 Handler_Arg;
};

/* This one is for process delay */
struct PCB_Timer
{
    struct List_Head Head;
    struct Tick_Time End_Total_Ticks;
    
    pid_t PID;
    cnt_t Delay_Status;
};
/*****************************************************************************/

/* __TIMER_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __TIMER_MEMBERS__
#define __TIMER_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__
#include "Syslib\syslib.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/
/* Timer list header */
struct List_Head Tim_List_Head;
/* Empty block list header */
struct List_Head Tim_Empty_List_Head;

/* Timer running list header */
struct List_Head Tim_Running_List_Head;
/* Timer paused list header */
struct List_Head Tim_Stop_List_Head;
/* Timer reload list header. Here the reload list also contains the timers to be processed,
 * not only those which are to be reloaded.
 */ 
struct List_Head Tim_Reload_List_Head;

/* Timer control block */
struct Timer Tim_CB[MAX_TIMS];
/* This head is for process delay */
struct List_Head Proc_Delay_List_Head;
/* PCB's timer part - This part is for canceling delay */
struct PCB_Timer PCB_Proc_Delay[MAX_PROC_NUM];
/* Timer number counter */
size_t Timer_In_Sys;
/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/

#define __EXTERN__ 
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/* Timer overflow handler */ 
__EXTERN__ volatile void (*_Sys_Timer_Ovf_Handler)(void);		                  
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/

/*****************************************************************************/
__EXTERN__ retval_t Sys_Tick_Time_Comp(struct Tick_Time* Tick_1,struct Tick_Time* Tick_2);
__EXTERN__ retval_t Sys_Tick_Time_Add(struct Tick_Time* Tick_1,struct Tick_Time* Tick_2,struct Tick_Time* Result);
__EXTERN__ retval_t Sys_Tick_Time_Minus(struct Tick_Time* Tick_1,struct Tick_Time* Tick_2,struct Tick_Time* Result);

__EXTERN__ void _Sys_Timer_Init(void);
__EXTERN__ void _Sys_Timer_Handler(void);
__EXTERN__ void _Sys_Timer_Reload(void);
__EXTERN__ timid_t Sys_Timer_Create(s8* Timer_Name,time_t Timer_Delay);
__EXTERN__ retval_t Sys_Timer_Destroy(timid_t Timer_ID);
__EXTERN__ timid_t Sys_Timer_Get_ID(s8* Timer_Name);
__EXTERN__ retval_t Sys_Timer_Start(timid_t Timer_ID);
__EXTERN__ retval_t Sys_Timer_Stop(timid_t Timer_ID);
__EXTERN__ retval_t Sys_Timer_Set_Mode(timid_t Timer_ID,s32 Mode);
__EXTERN__ retval_t Sys_Timer_Get_Time_Value(timid_t Timer_ID,struct Tick_Time* Cur_Time,struct Tick_Time* Max_Time);
__EXTERN__ size_t Sys_Query_Timer_Number(void);
__EXTERN__ retval_t Sys_Timer_Handler_Reg(timid_t Timer_ID,void(*Timer_Handler)(void),u32 Handler_Arg);

__EXTERN__ void Sys_Raw_Delay(time_t Time);
__EXTERN__ void Sys_Proc_Delay_Tick(time_t Time);
__EXTERN__ void Sys_Proc_Delay_Time(time_t Time);
__EXTERN__ void Sys_Proc_Delay_Cancel(pid_t PID);
__EXTERN__ void _Sys_Proc_Delay_Handler(void);
/*****************************************************************************/
#undef __EXTERN__

/* __TIMER_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
