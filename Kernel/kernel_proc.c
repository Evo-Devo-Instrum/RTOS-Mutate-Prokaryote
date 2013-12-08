/******************************************************************************
Filename    : kernel_proc.c
Author      : pry
Date        : 07/04/2012
Description : The kernel processes for the Mutate(TM)-prokaryote operating system.
******************************************************************************/

/* Version Information ********************************************************
1.Created By pry                                                     05/07/2013
  Created the file from applications.c and scheduler.c.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\kernel_proc.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Kernel\kernel_proc.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Kernel\kernel_proc.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\signal.h"
#include "Memmgr\memory.h"
#include "ExtIPC\semaphore.h"
#include "Kernel\kernel_proc.h"
#include "Syssvc\timer.h"
#include "Syslib\syslib.h"
#include "..\Muapp\applications.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Init ***************************************************
Description : Initiate the system services. Now it is empty. Its PID is 0.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Init(void)						                                     
{
    if(System_Status.Kernel.Boot_Done==FALSE)
    {
        Sys_Start_On_Boot();
    }			       
    
    Sys_Init_Initial();
    
    while(1)
    {  
       Sys_Init_Always();
       Sys_Switch_Now(); 
    }       
}
/* End Function:_Sys_Init *****************************************************/

/* Begin Function:_Sys_Arch ****************************************************
Description : The system service daemon. Now it is usually used as a shell process.
              It is a system process(just like the init process), but it runs in the 
              user field.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Arch(void)							            
{   
    Sys_Arch_Initial();
    while(1)
    {
        _Sys_Timer_Reload();
        Sys_Arch_Always();
        Sys_Switch_Now();
    }
}
/* End Function:_Sys_Arch ****************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
