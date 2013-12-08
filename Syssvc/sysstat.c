/******************************************************************************
Filename    : sysstat.c
Author      : pry
Date        : 03/07/2013
Version     : 0.02
Description : The system statistic module. This module mainly handles the query
              for system variables and system environment variables.
              Here the query for individual variables are not served; You can only
              get a struct as a whole.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syssvc\sysstat.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Syssvc\sysstat.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Syssvc\sysstat.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#include "Syssvc\sysstat.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:Sys_Query_Stat_Kernel ***************************************
Description : Get the kernel status. If fact, you will get a copy of the 
              "struct Sys_Kernel_Status_Struct Kernel" as a result.
Input       : None.
Output      : struct Sys_Kernel_Status_Struct* Kernel - The copy of the "Kernel" struct.
Return      : None.
******************************************************************************/
void Sys_Query_Stat_Kernel(struct Sys_Kernel_Status_Struct* Kernel)							        	  
{	
    Sys_Memcpy((ptr_int_t)Kernel,
               (ptr_int_t)(&System_Status.Kernel),
               sizeof(struct Sys_Kernel_Status_Struct));
}
/* End Function:Sys_Query_Stat_Kernel ****************************************/

/* Begin Function:Sys_Query_Stat_Proc *****************************************
Description : Get the system process status. If fact, you will get a copy of the 
              "struct Sys_Proc_Status_Struct Proc" as a result.
Input       : None.
Output      : struct Sys_Proc_Status_Struct* Proc - The copy of the "Proc" struct.
Return      : None.
******************************************************************************/
void Sys_Query_Stat_Proc(struct Sys_Proc_Status_Struct* Proc)							        	  
{	
    Sys_Memcpy((ptr_int_t)Proc,
               (ptr_int_t)(&System_Status.Proc),
               sizeof(struct Sys_Proc_Status_Struct));
}
/* End Function:Sys_Query_Stat_Proc ******************************************/

/* Begin Function:Sys_Query_Stat_Time *****************************************
Description : Get the system time status. If fact, you will get a copy of the 
              "struct Sys_Time_Status_Struct Time" as a result.
Input       : None.
Output      : struct Sys_Time_Status_Struct* Time - The copy of the "Time" struct.
Return      : None.
******************************************************************************/
void Sys_Query_Stat_Time(struct Sys_Time_Status_Struct* Time)							        	  
{	
    Sys_Memcpy((ptr_int_t)Time,
               (ptr_int_t)(&System_Status.Time),
               sizeof(struct Sys_Time_Status_Struct));
}
/* End Function:Sys_Query_Stat_Time ******************************************/

/* Begin Function:Sys_Query_Proc_Stat *****************************************
Description : Get a certain process's status. If fact, you will get a copy of the 
              "struct PCB_Struct" as a result.
Input       : pid_t PID - The process ID.
Output      : struct PCB_Struct* Proc_PCB - The copy of the "struct PCB_Struct" struct.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
retval_t Sys_Query_Proc_Stat(pid_t PID,struct PCB_Struct* Proc_PCB)							        	  
{	
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)
        return(-1);
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)
        return(-1);
    
    Sys_Memcpy((ptr_int_t)Proc_PCB,
               (ptr_int_t)&PCB[PID],
               sizeof(struct PCB_Struct));
    
    return 0;
}
/* End Function:Sys_Query_Proc_Stat ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
