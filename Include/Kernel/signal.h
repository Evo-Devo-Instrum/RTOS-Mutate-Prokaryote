/******************************************************************************
Filename   : signal.h
Author     : pry
Date       : 25/04/2012
Description: The header for the signal module of the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SIGNAL_H_DEFS__
#define __SIGNAL_H_DEFS__

/* An empty signal */
#define NOSIG      ((signal_t)0)  
/* Kill the process instantly. No effect for init and arch */                                                       
#define SIGKILL    ((signal_t)1)
/* Stuck a process(sleep.) */                                                       
#define SIGSLEEP   ((signal_t)1<<1)
/* Wakeup a process */                                                    
#define SIGWAKE    ((signal_t)1<<2)
/* The child process terminate signal */
#define SIGCHLD    ((signal_t)1<<3)
/* The signals after this are all preserved ones */

/* The user signals */                                                     
#define SIGUSR0    ((signal_t)1<<16)                                                     
#define SIGUSR1    ((signal_t)1<<17)                                                   
#define SIGUSR2    ((signal_t)1<<18)                                                   
#define SIGUSR3    ((signal_t)1<<19)
#define SIGUSR4    ((signal_t)1<<20)                                                    
#define SIGUSR5    ((signal_t)1<<21)                                                  
#define SIGUSR6    ((signal_t)1<<22)                                                   
#define SIGUSR7    ((signal_t)1<<23)
#define SIGUSR8    ((signal_t)1<<24)                                                     
#define SIGUSR9    ((signal_t)1<<25)                                                   
#define SIGUSR10   ((signal_t)1<<26)                                                   
#define SIGUSR11   ((signal_t)1<<27)
#define SIGUSR12   ((signal_t)1<<28)                                                    
#define SIGUSR13   ((signal_t)1<<29)                                                  
#define SIGUSR14   ((signal_t)1<<30)                                                   
#define SIGUSR15   ((signal_t)1<<31)
    
/* Error defines */
/* Not a signal */
#define ENOTSIG    0x00
/* Invalid signal */
#define EINVSIG    0x01
/* Invalid function pointer */
#define EINVFUNC   0x02
/* The process has no return value */             					   
#define ENORET     0x01      
/* The process does not exist, can't send signal to it */
#define ENOPROC    0x01

/* __SIGNAL_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SIGNAL_H_STRUCTS__
#define __SIGNAL_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/* __SIGNAL_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SIGNAL_MEMBERS__
#define __SIGNAL_MEMBERS__

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

/* "SIGCUSTOM" signal handler */ 
__EXTERN__ volatile void (*_Sys_SIGCUSTOM_Handler)(void);		                  
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
/* The system signal classifier */
__EXTERN__ void _Sys_Signal_Handler(pid_t PID);	
/* Called by a process to terminate itself with a return value */                                  
__EXTERN__ void Sys_Process_Quit(retval_t Retval);
/* Get the return value of a zombie process */		                           
__EXTERN__ retval_t Sys_Get_Proc_Retval(pid_t PID);	

/* The SIGKILL Handler*/	                               
__EXTERN__ void _Sys_Kill_The_Process(pid_t PID);
/* Make a process sleep */	               
__EXTERN__ void _Sys_Sleep_The_Process(pid_t PID);		 
/* Wake the process */			               
__EXTERN__ void _Sys_Wake_The_Process(pid_t PID);
/* Send signal */			          
__EXTERN__ retval_t Sys_Send_Signal(pid_t PID,signal_t Signal);	

/* Signal handler registration */					           
__EXTERN__ retval_t Sys_Reg_Signal_Handler(signal_t Signal,void (*Func)(void));  

/* Set and get the global variable */
__EXTERN__ retval_t _Sys_Set_Sig_IPC_Global(pid_t PID,signal_t Signal,ptr_int_t Variable);
__EXTERN__ retval_t Sys_Set_Sig_IPC_Global(signal_t Signal,ptr_int_t Variable);
__EXTERN__ retval_t _Sys_Get_Sig_IPC_Global(pid_t PID,signal_t Signal,ptr_int_t* Variable);
__EXTERN__ retval_t Sys_Get_Sig_IPC_Global(signal_t Signal,ptr_int_t* Variable);

__EXTERN__ signal_t Sys_Get_Sig_By_Name(s8* Signal,cnt_t Signal_Len);
/*****************************************************************************/

#undef __EXTERN__

/* __SIGNAL_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
