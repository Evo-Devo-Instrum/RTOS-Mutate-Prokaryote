/******************************************************************************
Filename   : error.c
Author     : pry
Date       : 25/04/2012
Description: The error related module in the OS.
******************************************************************************/

/* Includes ********************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Kernel\error.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\error.h"
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#undef __HDR_PUBLIC_MEMBERS__
/*End Includes****************************************************************/

/* Begin Function:Sys_Set_Errno ***********************************************
Description : Set the error identification number for the process. The PID
              is automatically the "Current_PID".
Input       : s32 Errno - The error identification number for the process.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Set_Errno(errno_t Errno)
{         
    /* Make sure that it is not in an interrupt */    
    if(Int_Nest_Cnt==0)
        PCB[Current_PID].Info.Errno=Errno;
} 
/* End Function:Sys_Set_Errno ************************************************/

/* Begin Function:_Sys_Set_Errno **********************************************
Description : Set the error identification number for the process. The PID
              is automatically the "Current_PID".
Input       : pid_t PID - The process ID.
              errno_t Errno - The error identification number for the process.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Set_Errno(pid_t PID,errno_t Errno)
{               
    /* Make sure that it is not in an interrupt */    
    if(Int_Nest_Cnt==0)
        PCB[PID].Info.Errno=Errno;
} 
/* End Function:_Sys_Set_Errno ***********************************************/

/* Begin Function:Sys_Get_Errno ***********************************************
Description : Get the error identification number for the process. The PID
              is automatically the "Current_PID".
Input       : None.
Output      : None.
Return      : s32 - The errno.
******************************************************************************/
errno_t Sys_Get_Errno(void)
{               
    return(PCB[Current_PID].Info.Errno);
}
/* End Function:Sys_Get_Errno ************************************************/

/* Begin Function:_Sys_Get_Errno **********************************************
Description : Get the error identification number for the process. The PID
              is automatically the "Current_PID".
Input       : pid_t PID - The process ID.
Output      : None.
Return      : s32 - The errno.
******************************************************************************/
errno_t _Sys_Get_Errno(pid_t PID)
{               
    return PCB[PID].Info.Errno;
}
/* End Function:_Sys_Get_Errno ***********************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
