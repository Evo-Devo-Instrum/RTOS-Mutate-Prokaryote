/******************************************************************************
Filename    : interrupt.c
Author      : pry
Date        : 16/07/2013
Version     : 0.01
Description : The system interrupt management module. 
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Kernel\interrupt.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#include "Kernel\interrupt.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Int_Init ***********************************************
Description : Initialize the system interrupt.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Int_Init(void)							        	  
{	
    Interrupt_Lock_Cnt=0;
    Systick_Freeze_Cnt=0;
    Scheduler_Lock_Cnt=0;
    Scheduler_Locked=0;
    Int_Nest_Cnt=0;
}
/* End Function:_Sys_Int_Init ************************************************/

/* Begin Function:Sys_Enter_Int_Handler ***************************************
Description : This should be called first when we entered an interrupt routine.
              In this way we can know how many interrupt is nested.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Enter_Int_Handler(void)							        	  
{	
    Int_Nest_Cnt++;
}
/* End Function:Sys_Enter_Int_Handler ****************************************/

/* Begin Function:Sys_Exit_Int_Handler ****************************************
Description : This should be called at the end of an interrupt routine.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Exit_Int_Handler(void)							        	  
{	
    Int_Nest_Cnt--;
}
/* End Function:Sys_Exit_Int_Handler *****************************************/

/* Begin Function:Sys_Lock_Interrupt ******************************************
Description : The function locks the interrupt. The locking can be stacked.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Lock_Interrupt(void)
{
    if(Interrupt_Lock_Cnt==0)
    {
        /* Disable first before registering it. If an switch occurs between 
         * registering and disabling, then register-and-disable will cause 
         * fault.
         */
        DISABLE_ALL_INTS();
        Interrupt_Lock_Cnt=1;
    }
    else
        Interrupt_Lock_Cnt++;
}
/* End Function:Sys_Lock_Interrupt *******************************************/

/* Begin Function:Sys_Unlock_Interrupt ****************************************
Description : The function unlocks the interrupt. The unlocking can be stacked.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Unlock_Interrupt(void)
{
    if(Interrupt_Lock_Cnt==1)
    {
        /* Clear the count before enabling, or it will cause fault in the same
         * sense as above.
         */
        Interrupt_Lock_Cnt=0;
        ENABLE_ALL_INTS();
    }
    else if(Interrupt_Lock_Cnt!=0)
        Interrupt_Lock_Cnt--;
}
/* End Function:Sys_Unlock_Interrupt *****************************************/

/* Begin Function:Sys_Lock_Scheduler ******************************************
Description : The function locks the scheduler. The locking can be stacked.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Lock_Scheduler(void)
{
    if(Scheduler_Lock_Cnt==0)
    {
        /* Disable first before registering it. If an switch occurs between 
         * registering and disabling, then register-and-disable will cause 
         * fault.
         */
        Scheduler_Locked=1;
        Scheduler_Lock_Cnt=1;
    }
    else
        Scheduler_Lock_Cnt++;
}
/* End Function:Sys_Lock_Scheduler *******************************************/

/* Begin Function:Sys_Unlock_Scheduler ****************************************
Description : The function unlocks the scheduler. The unlocking can be stacked.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Unlock_Scheduler(void)
{
    if(Scheduler_Lock_Cnt==1)
    {
        /* Clear the count before enabling, or it will cause fault in the same
         * sense as above.
         */
        Scheduler_Lock_Cnt=0;
        Scheduler_Locked=0;
        /* Now see if the scheduler scheduling action is pended in the lock-unlock 
         * period. If yes, perform a schedule now. 
         */
        if(Pend_Sched_Cnt>0)
        {
            /* Reset the count and trigger the PendSV */
            Pend_Sched_Cnt=0;
            _Sys_Schedule_Trigger();
        }
    }
    else if(Scheduler_Lock_Cnt!=0)
        Scheduler_Lock_Cnt--;
}
/* End Function:Sys_Unlock_Scheduler *****************************************/

/* Begin Function:Sys_Lock_Proc_Stat ******************************************
Description : The function locks the status of a certain process and only unlocking and 
              killing(or quitting) the process can terminate the lock state.
              When the status of a certain process is locked, then we cannot change its
              priority and its running status.
              The locking can be stacked. This lock can only be used by the kernel; do
              not use this lock in user application.
Input       : None.
Output      : None.
Return      : retval_t - If successful, 0; else -1.
******************************************************************************/
retval_t Sys_Lock_Proc_Stat(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)                                              
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)                                              
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }  

    PCB[PID].Status.Status_Lock_Count++;
    
    return 0;
}
/* End Function:Sys_Lock_Proc_Stat *******************************************/

/* Begin Function:Sys_Unlock_Proc_Stat ****************************************
Description : The function unlocks the process status. The unlocking can be stacked.
              This lock can only be used by the kernel; do not use this lock in
              user application.
Input       : None.
Output      : None.
Return      : retval_t - If successful, 0; else -1.
******************************************************************************/
retval_t Sys_Unlock_Proc_Stat(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)                                              
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)                                              
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    if(PCB[PID].Status.Status_Lock_Count>0)
    {
        PCB[PID].Status.Status_Lock_Count--;
    }
    else 
    {
        Sys_Set_Errno(ENOPID);
        return (-1);
    }
    
    return 0;
}
/* End Function:Sys_Unlock_Proc_Stat *****************************************/

/* Begin Function:Sys_Freeze_Systick ******************************************
Description : The function locks the Systick timer. When the systick timer is
              locked, then the system scheduler is no longer functional.
              The locking can be stacked.
              This function is only for debugging; Not really used in the system.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Freeze_Systick(void)
{
    if(Systick_Freeze_Cnt==0)
    {
        DISABLE_SYSTICK();
        Systick_Freeze_Cnt=1; 
    }
    else
        Systick_Freeze_Cnt++;
}
/* End Function:Sys_Freeze_Systick *******************************************/

/* Begin Function:Sys_Resume_Systick ******************************************
Description : The function unlocks the Systick timer. The unlocking can be stacked.
              This function is only for debugging; Not really used in the system.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Resume_Systick(void)
{
    if(Systick_Freeze_Cnt==1)
    {
        Systick_Freeze_Cnt=0;
        ENABLE_SYSTICK();
    }
    else if(Systick_Freeze_Cnt!=0)
        Systick_Freeze_Cnt--;
}
/* End Function:Sys_Resume_Systick *******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
