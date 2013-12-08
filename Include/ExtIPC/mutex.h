/******************************************************************************
Filename    : mutex.h
Author      : pry
Date        : 25/04/2012
Version     : 0.01
Description : The mutex management module for the RMP RTOS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __MUTEX_H_DEFS__
#define __MUTEX_H_DEFS__		

/* There's no empty Mutex_CB blocks */			 				       
#define ENOEMUTEX          			0x00	 	
/* There's no such mutex */
#define ENOMUTEX          			0x01  
/* The mutex is not fully freed */
#define ENRLMUTEX                   0x02
/* The mutex is locked */
#define EMUTEXLOCK                  0x03
/* __MUTEX_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __MUTEX_H_STRUCTS__
#define __MUTEX_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#include "mutex.h"
#include "Syslib\syslib.h"
#undef __HDR_DEFS__

/* The mutex control block struct */
struct Mutex
{
    struct List_Head Mutex_CB_Head;
    struct List_Head PCB_Head;
    struct List_Head Wait_Object_Head;
    
	s8* Mutex_Name; 
    /* This is for getting the ID from the block pointer */
    mutid_t Mutex_ID;    
    /* Whether the mutex is registered */
    cnt_t Mutex_Reg_Flag;   
    
    /* The occupy process's PID */    
	pid_t Mutex_Occupy_PID; 
    /* The occupy count */
	cnt_t Mutex_Occupy_Cnt; 
    /* The original priority of the process. Used for priority inheritance. */
    prio_t Mutex_Occupy_Orig_Prio; 
};

/* __MUTEX_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/*Private Global Variables****************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __MUTEX_MEMBERS__
#define __MUTEX_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "ExtIPC\mutex.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "ExtIPC\mutex.h"
#include "ExtIPC\wait.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/* The PCB's external special for mutex */
struct List_Head PCB_ExtIPC_Mutex[MAX_PROC_NUM];
/* The head pointer for Mutex_CB */
struct List_Head Mutex_List_Head;
/* The head for empty mutex blocks */
struct List_Head Empty_Mutex_List_Head;
/* The mutex control block */
struct Mutex Mutex_CB[MAX_MUTEXS];
/* Statistic variable */
cnt_t Mutex_In_Sys_Cnt;
/*End Private Global Variables************************************************/

/*Private C Function Prototypes***********************************************/

#define __EXTERN__
/*End Private C Function Prototypes*******************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void _Sys_Mutex_Init(void);

#if(ENABLE_MUTEX==TRUE)
/*****************************************************************************/
__EXTERN__ mutid_t Sys_Register_Mutex(s8* Mutex_Name);
__EXTERN__ retval_t Sys_Remove_Mutex(mutid_t Mutex_ID);
__EXTERN__ mutid_t Sys_Get_Mutex_ID(s8* Mutex_Name);
__EXTERN__ retval_t Sys_Occupy_Mutex(mutid_t Mutex_ID);
__EXTERN__ retval_t _Sys_Occupy_Mutex(pid_t PID,mutid_t Mutex_ID);
__EXTERN__ retval_t Sys_Free_Mutex(mutid_t Mutex_ID);
__EXTERN__ retval_t _Sys_Free_Mutex(pid_t PID,mutid_t Mutex_ID);
__EXTERN__ size_t Sys_Query_Mutex_Status(mutid_t Mutex_ID);
__EXTERN__ retval_t _Sys_Wait_Mutex_Reg(pid_t PID,mutid_t Mutex_ID,
                                        struct Wait_Object_Struct* Wait_Block_Ptr);
__EXTERN__ void _Sys_Wait_Mutex_Prio_Adj(mutid_t Mutex_ID);
/*****************************************************************************/
#endif

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __MUTEX_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
