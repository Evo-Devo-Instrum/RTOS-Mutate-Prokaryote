/******************************************************************************
Filename    : semaphore.h
Author      : pry
Date        : 25/04/2012
Version     : 0.02
Description : The semaphore module of RMP RTOS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SEMAPHORE_H_DEFS__
#define __SEMAPHORE_H_DEFS__		

/* Release all semaphore: use this */
#define ALL_SEM                     0xFFFFFFFF
/* There's no empty Sem_CB blocks */			 				       
#define ENOESEM          			0x00	 	
/* There's no such semaphore */
#define ENOSEM          			0x01  
/* The semaphore is not fully freed */
#define ENRLSEM                     0x02
/* There's no empty semaphores to occupy */
#define ENSEMRES                    0x02
/* __SEMAPHORE_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SEMAPHORE_H_STRUCTS__
#define __SEMAPHORE_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#include "semaphore.h"
#include "Syslib\syslib.h"
#undef __HDR_DEFS__

/* The struct for each individual process */
struct Proc_Sem														   
{
    struct List_Head Head;
    struct List_Head Head_In_PCB;
	pid_t PID;          
    semid_t Sem_ID;    
	size_t Sem_Number;												   
};	

/* The semaphore control block struct */
struct Semaphore
{
    struct List_Head Head;
    struct List_Head Wait_Object_Head;
	s8* Sem_Name; 
    /* This is for getting the ID from the block pointer */
    semid_t  Sem_ID;    
	size_t Total_Sem; 															  
	size_t Sem_Left;      
    /* The process occupying the semaphores are registered here */     							    					  
	struct List_Head Proc_Sem_Head;				  
};

/* __SEMAPHORE_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/*Private Global Variables****************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SEMAPHORE_MEMBERS__
#define __SEMAPHORE_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "ExtIPC\semaphore.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "ExtIPC\semaphore.h"
#include "ExtIPC\wait.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/* The PCB's external special for semaphore */
struct List_Head PCB_ExtIPC_Sem[MAX_PROC_NUM];
/* The head pointer for Sem_CB */
struct List_Head Sem_List_Head;
/* The head for empty semaphore blocks */
struct List_Head Empty_Sem_List_Head;
/* The semaphore control block */
struct Semaphore Sem_CB[MAX_SEMS];
/* The head for empty Proc_Sem blocks */
struct List_Head Empty_Proc_Sem_Block_Head;
/* The semaphore sub-block. When an process want to use the semaphore, then 
 * it has to allocate one of it.
 */
struct Proc_Sem Proc_Sem_Block[MAX_PROC_USE_SEM];
/* Statistic variable */
cnt_t Sem_In_Sys_Cnt;
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
__EXTERN__ void _Sys_Sem_Init(void);

#if(ENABLE_SEM==TRUE)
/*****************************************************************************/
__EXTERN__ semid_t Sys_Register_Sem(s8* Sem_Name,size_t Number);     
__EXTERN__ retval_t Sys_Remove_Sem(semid_t Sem_ID);
__EXTERN__ semid_t Sys_Get_Sem_ID(s8* Sem_Name);

__EXTERN__ retval_t Sys_Occupy_Sem(semid_t Sem_ID,size_t Number);    
__EXTERN__ retval_t _Sys_Occupy_Sem(pid_t PID,semid_t Sem_ID,size_t Number);
__EXTERN__ retval_t Sys_Free_Sem(semid_t Sem_ID,size_t Number);     
__EXTERN__ retval_t _Sys_Free_Sem(pid_t PID,semid_t Sem_ID,size_t Number); 
__EXTERN__ void Sys_Free_All_Sem(void);                                 
__EXTERN__ void _Sys_Free_All_Sem(pid_t PID);						        
__EXTERN__ size_t Sys_Query_Sem_Amount(semid_t Sem_ID);   
__EXTERN__ retval_t _Sys_Wait_Sem_Reg(pid_t PID,semid_t Sem_ID,
                                      struct Wait_Object_Struct* Wait_Block_Ptr);
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
