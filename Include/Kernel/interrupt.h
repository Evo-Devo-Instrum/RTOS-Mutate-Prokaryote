/******************************************************************************
Filename   : interrupt.h
Author     : pry
Date       : 25/04/2012
Description: The header for the system statistic module of the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __INTERRUPT_H_DEFS__
#define __INTERRUPT_H_DEFS__
/*****************************************************************************/

/*****************************************************************************/

/* __INTERRUPT_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __INTERRUPT_H_STRUCTS__
#define __INTERRUPT_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/

/*****************************************************************************/

/* __INTERRUPT_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __INTERRUPT_MEMBERS__
#define __INTERRUPT_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

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

__EXTERN__ volatile cnt_t Int_Nest_Cnt;
/* The interrupt lock counter */
__EXTERN__ volatile cnt_t Interrupt_Lock_Cnt;
/* The systick freeze counter */
__EXTERN__ volatile cnt_t Systick_Freeze_Cnt;
/* The scheduler lock counter */
__EXTERN__ volatile cnt_t Scheduler_Lock_Cnt;
/* The flag: whether the scheduler is locked */
__EXTERN__ volatile cnt_t Scheduler_Locked;                  
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/

/*****************************************************************************/
__EXTERN__ void _Sys_Int_Init(void);
__EXTERN__ void Sys_Enter_Int_Handler(void);
__EXTERN__ void Sys_Exit_Int_Handler(void);

__EXTERN__ void Sys_Lock_Interrupt(void);
__EXTERN__ void Sys_Unlock_Interrupt(void);
__EXTERN__ void Sys_Freeze_Systick(void);
__EXTERN__ void Sys_Resume_Systick(void);
__EXTERN__ void Sys_Lock_Scheduler(void);
__EXTERN__ void Sys_Unlock_Scheduler(void);
__EXTERN__ retval_t Sys_Lock_Proc_Stat(pid_t PID);
__EXTERN__ retval_t Sys_Unlock_Proc_Stat(pid_t PID);
/*****************************************************************************/
#undef __EXTERN__

/*Assembly Functions Prototypes***********************************************/
/* Always extern - Assembly ones */
/* Disable all interrupts */
EXTERN void DISABLE_ALL_INTS(void);	
/* Enable all interrupts */	                                   
EXTERN void ENABLE_ALL_INTS(void);
/* Disable the systick timer */		                                   
EXTERN void DISABLE_SYSTICK(void);  
/* Enable the systick timer */                                       
EXTERN void ENABLE_SYSTICK(void); 				                       

/* __INTERRUPT_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/*End Assembly Functions Prototypes*******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
