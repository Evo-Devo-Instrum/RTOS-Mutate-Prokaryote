/******************************************************************************
Filename    : mutex.c
Author      : pry
Version     : 0.01
Date        : 25/04/2012
Description : The mutex module for the operating system.
              when waiting for one mutex, the implications of the "wait" is:
              1>When the mutex is deleted, then the function will return as failed.
              2>When the mutex count is 1, then the waiting process 
                will get the mutex and quit the waiting. The priority of the 
                process which have the mutex will be lifted to the highest of the waiting
                queue, and only after it releases the mutex will it be put back.
              3>When the mutex is empty, the process will wait until expire.
              TAKE NOTE not to change the priority and state of the process owning or
              waiting for the mutex. Doing so will cause a system crash.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "ExtIPC\mutex.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "ExtIPC\mutex.h"
#include "Kernel\error.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "ExtIPC\mutex.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Syslib\syslib.h"
#include "Kernel\error.h"
#include "ExtIPC\mutex.h"
#include "ExtIPC\wait.h"

#include "Syssvc\timer.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Mutex_Init *********************************************
Description : Initialize the mutex managing unit. Never call this in user
              application.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Mutex_Init(void)
{
#if(ENABLE_MUTEX==TRUE)
    cnt_t Mutex_Cnt;
    /* Memset all three arrays */
    Sys_Memset((ptr_int_t)PCB_ExtIPC_Mutex,0,sizeof(struct List_Head)*MAX_PROC_NUM);	
    Sys_Memset((ptr_int_t)Mutex_CB,0,sizeof(struct Mutex)*MAX_MUTEXS);
    
    /* Initialize the list heads */
    Sys_Create_List(&Mutex_List_Head);
    Sys_Create_List(&Empty_Mutex_List_Head);
    
    for(Mutex_Cnt=0;Mutex_Cnt<MAX_PROC_NUM;Mutex_Cnt++)
        Sys_Create_List(&(PCB_ExtIPC_Mutex[Mutex_Cnt]));
    
    /* Initialize the empty block list */
    for(Mutex_Cnt=0;Mutex_Cnt<MAX_MUTEXS;Mutex_Cnt++)
    {
        Sys_List_Insert_Node(&(Mutex_CB[Mutex_Cnt].Mutex_CB_Head),
                             &Empty_Mutex_List_Head,
                             Empty_Mutex_List_Head.Next);
        
        Sys_Create_List(&Mutex_CB[Mutex_Cnt].Wait_Object_Head);
        
        Mutex_CB[Mutex_Cnt].Mutex_ID=Mutex_Cnt;
        Mutex_CB[Mutex_Cnt].Mutex_Occupy_Cnt=0;
        Mutex_CB[Mutex_Cnt].Mutex_Occupy_PID=0;
        Mutex_CB[Mutex_Cnt].Mutex_Reg_Flag=0;
    }
    
    Mutex_In_Sys_Cnt=0;
#endif
}
/* End Function:_Sys_Mutex_Init **********************************************/

/* Begin Function:Sys_Register_Mutex ******************************************
Description : Register a mutex. For use in application.
Input       : s8* Mutex_Name - The name of the mutex.
Output      : None.
Return      : mutid_t - The ID of the mutex. If the function fail, then
                        the return value will be "-1".
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
mutid_t Sys_Register_Mutex(s8* Mutex_Name)
{               
    mutid_t Mutex_ID;
    struct List_Head* Traverse_Ptr;
    
	Sys_Lock_Scheduler();
    
    Traverse_Ptr=Mutex_List_Head.Next;
    /* See if the name is unique in the system. If not, the mutex cannot be created.
     * to do this we have to traverse the whole list and make comparison on every created
     * mutex in the system.
     */
    while(Traverse_Ptr!=&Mutex_List_Head)
    {
        /* If we found a match */
        if(Sys_Strcmp(Mutex_Name,((struct Mutex*)Traverse_Ptr)->Mutex_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(ENOEMUTEX);
            return (-1);
        }
        Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    /* Find an available block */
    if(Empty_Mutex_List_Head.Next!=&Empty_Mutex_List_Head)
    {
        Mutex_ID=((struct Mutex*)(Empty_Mutex_List_Head.Next))->Mutex_ID;
        Sys_List_Delete_Node(Empty_Mutex_List_Head.Next->Prev,
                             Empty_Mutex_List_Head.Next->Next);
        
        Sys_List_Insert_Node(&(Mutex_CB[Mutex_ID].Mutex_CB_Head),
                             &Mutex_List_Head,
                             Mutex_List_Head.Next);
    }
    else 
    {   
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOEMUTEX);
        return (-1);
    }
    
    /* Fill the structure */
    Mutex_CB[Mutex_ID].Mutex_Name=Mutex_Name;
    Mutex_CB[Mutex_ID].Mutex_Reg_Flag=1;
    
    /* Update statistical variable */
    Mutex_In_Sys_Cnt++;

    Sys_Unlock_Scheduler();
	return(Mutex_ID);
} 
#endif
/* End Function:Sys_Register_Mutex *******************************************/

/* Begin Function:Sys_Remove_Mutex ********************************************
Description : Remove a mutex. For use in application.
Input       : mutid_t Mutex_ID - The ID of the mutex to remove.
Output      : None.
Return      : retval_t - 0 for success,-1 for failure.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
retval_t Sys_Remove_Mutex(mutid_t Mutex_ID)
{     
    Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
	if(Mutex_ID>=MAX_MUTEXS)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
        return (-1);
    }
    
    /* See if the mutex is registered before */
    if(Mutex_CB[Mutex_ID].Mutex_Reg_Flag==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
        return (-1);
    }
    
    /* Make sure the mutex is not occupied */
    if(Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt!=0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENRLMUTEX);
        return (-1);
    }
    
    /* Delete the node from the active list and place it in the empty list */
    Sys_List_Delete_Node(Mutex_CB[Mutex_ID].Mutex_CB_Head.Prev,Mutex_CB[Mutex_ID].Mutex_CB_Head.Next);
    Sys_List_Insert_Node(&(Mutex_CB[Mutex_ID].Mutex_CB_Head),
                         &Empty_Mutex_List_Head,
                         Empty_Mutex_List_Head.Next);
    
    /* Clear the variables */
    Mutex_CB[Mutex_ID].Mutex_Name=0;
    Mutex_CB[Mutex_ID].Mutex_Reg_Flag=0;
    
    /* Update statistical variable */
    Mutex_In_Sys_Cnt++;
    
    Sys_Unlock_Scheduler();
    return 0;
} 
#endif
/* End Function:Sys_Remove_Mutex *********************************************/

/* Begin Function:Sys_Get_Mutex_ID ********************************************
Description : Get a mutex's unique ID through its name. For use in application.
Input       : s8* Mutex_Name - The name of the mutex.
Output      : None.
Return      : mutid_t - The ID of the mutex, Mutex_ID. If the function fail, then
                        the return value will be -1.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
mutid_t Sys_Get_Mutex_ID(s8* Mutex_Name)
{               
    struct List_Head* Traverse_Ptr;
    
	Sys_Lock_Scheduler();
    
    Traverse_Ptr=Mutex_List_Head.Next;
   
    /* Try to find a match */
    while(Traverse_Ptr!=&Mutex_List_Head)
    {
        /* If we found a match */
        if(Sys_Strcmp(Mutex_Name,((struct Mutex*)Traverse_Ptr)->Mutex_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            return (((struct Mutex*)Traverse_Ptr)->Mutex_ID);
        }
        Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    /* If it can get here, then the function must have failed */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOEMUTEX);
	return (-1);
} 
#endif
/* End Function:Sys_Get_Mutex_ID *********************************************/

/* Begin Function:Sys_Occupy_Mutex ********************************************
Description : Occupy a certain mutex. For application use. The PID is
              automatically the "Current_PID". This function will try to get the 
              mutex once and return right away.
Input       : mutid_t Mutex_ID - The ID of the mutex, Returned by "Sys_Register_Mutex".
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
retval_t Sys_Occupy_Mutex(mutid_t Mutex_ID)
{                                                         		  
	return(_Sys_Occupy_Mutex(Current_PID,Mutex_ID));
} 
#endif
/* End Function:Sys_Occupy_Mutex *********************************************/

/* Begin Function:_Sys_Occupy_Mutex *******************************************
Description : Occupy a certain mutex. For system use, or where you want to 
              specify the PID. Thus, it is not recommended to use this in
              application.This function will try to get the mutex once and 
              return right away.
Input       : pid_t PID - The PID you want.
              mutid_t Mutex_ID - The ID of the mutex, Returned by "Sys_Register_Mutex".
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
retval_t _Sys_Occupy_Mutex(pid_t PID,mutid_t Mutex_ID)
{              
    
	Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
    if(Mutex_ID>=MAX_MUTEXS)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
	    return (-1);
    }
    
    /* See if the mutex is registered */
    if(Mutex_CB[Mutex_ID].Mutex_Reg_Flag==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
	    return (-1);
    }
    
    /* See if the mutex is already taken by others */
    if((Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt>0)&&
       (Mutex_CB[Mutex_ID].Mutex_Occupy_PID!=PID))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EMUTEXLOCK);
	    return (-1);
    }
    
    /* Take the mutex now - See if the mutex has already been taken by it */
    if((Mutex_CB[Mutex_ID].Mutex_Occupy_PID!=PID)||(Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt==0))
    {
        Sys_List_Delete_Node(Mutex_CB[Mutex_ID].PCB_Head.Prev,Mutex_CB[Mutex_ID].PCB_Head.Next);
        
        /* Insert the node into the corresponding process's occupy list */
        Sys_List_Insert_Node(&Mutex_CB[Mutex_ID].PCB_Head,
                             &(PCB_ExtIPC_Mutex[PID]),
                             PCB_ExtIPC_Mutex[PID].Next);  
    }
    
    Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt++;
    
	Sys_Unlock_Scheduler();	                                            		  
	return(0);
} 
#endif
/* End Function:_Sys_Occupy_Mutex ********************************************/

/* Begin Function:Sys_Free_Mutex **********************************************
Description : Free mutex. For application use. The PID is automatically 
              the "Current_PID".
Input       : mutid_t Mutex_ID - The ID Of the mutex, Returned by "Sys_Register_Mutex".
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
retval_t Sys_Free_Mutex(mutid_t Mutex_ID)
{                                                     
	return(_Sys_Free_Mutex(Current_PID,Mutex_ID));
} 
#endif
/* End Function:Sys_Free_Mutex ***********************************************/

/* Begin Function:_Sys_Free_Mutex *********************************************
Description : Free mutex. For system use, or where or where you want to 
              specify the PID. Thus, it is not recommended to use this in
              application.  If you try to free a mutex not occupied, then an error
              will occur.
Input       : pid_t PID - The PID you want.
              mutid_t Mutex_ID - The ID Of the mutex, Returned by "Sys_Register_Mutex".
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
retval_t _Sys_Free_Mutex(pid_t PID,mutid_t Mutex_ID)
{               
    pid_t Wait_Occupy_PID;
    
	Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
    if(Mutex_ID>=MAX_MUTEXS)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
	    return (-1);
    }
    
    /* See if the mutex is registered */
    if(Mutex_CB[Mutex_ID].Mutex_Reg_Flag==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
	    return (-1);
    }
    
    /* See if the mutex is occupied by the process */
    if((Mutex_CB[Mutex_ID].Mutex_Occupy_PID!=PID)||(Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMUTEX);
	    return (-1);
    }
    
    /* Now decrease the number of mutex by 1 */
    Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt--;
    
    /* See if the mutex count has been down to zero. If yes, delete its position in the PCB */
    if(Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt==0)
    {
        /* See if priority inheritance happened on this process. If yes, put this process back to its
         * original level.
         */
        if(Mutex_CB[Mutex_ID].Mutex_Occupy_Orig_Prio!=PCB[PID].Status.Priority)
            _Sys_Change_Proc_Prio(PID,Mutex_CB[Mutex_ID].Mutex_Occupy_Orig_Prio);
        
        /* Delete the node from its PCB */
        Sys_List_Delete_Node(Mutex_CB[Mutex_ID].PCB_Head.Prev,Mutex_CB[Mutex_ID].PCB_Head.Next);
        
        /* See if any process is waiting for the mutex */
        if(Mutex_CB[Mutex_ID].Wait_Object_Head.Next!=&Mutex_CB[Mutex_ID].Wait_Object_Head)
        {
            /* Get the information and let this process get the mutex */
            Wait_Occupy_PID=((struct Wait_Object_Struct*)(Mutex_CB[Mutex_ID].Wait_Object_Head.Next-1))->PID;
            /* Mark that the wait is successful */
            ((struct Wait_Object_Struct*)(Mutex_CB[Mutex_ID].Wait_Object_Head.Next-1))->Succeed_Flag=1;
            /* Delete the wait block from the mutex wait list */
            Sys_List_Delete_Node(&Mutex_CB[Mutex_ID].Wait_Object_Head,Mutex_CB[Mutex_ID].Wait_Object_Head.Next->Next);
            _Sys_Occupy_Mutex(Wait_Occupy_PID,Mutex_ID);
            /* Try to stop the timer if possible */
            Sys_Proc_Delay_Cancel(Wait_Occupy_PID);
            /* Wake the process up */
            _Sys_Set_Ready(Wait_Occupy_PID);
        }
    }
    
	Sys_Unlock_Scheduler();	                                            		  
	return(0);
} 
#endif
/* End Function:_Sys_Free_Mutex **********************************************/

/* Begin Function:Sys_Query_Mutex_Status **************************************
Description : Query the mutex status.
Input       : mutid_t Mutex_ID -The mutex's ID.
Output      : None.
Return      : size_t - If the mutex is taken, it will return the how many times the
                       mutex is taken. If the mutex is not taken, then the value 
                       will be 0. If the mutex does not exist, then the value
                       will also be 0.
******************************************************************************/
#if(ENABLE_MUTEX==TRUE)
size_t Sys_Query_Mutex_Status(mutid_t Mutex_ID)
{
    /* See if the operation is over the boundary */
    if(Mutex_ID>=MAX_MUTEXS)
    {
        Sys_Unlock_Scheduler();
        return(0);
    }
    
	/* See if the mutex is registered */
    if(Mutex_CB[Mutex_ID].Mutex_Reg_Flag==0)
    {
        Sys_Unlock_Scheduler();
        return(0);
    }
    
	return(Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt);                                         
}
#endif
/* End Function:Sys_Query_Mutex_Status ***************************************/

/* Begin Function:_Sys_Wait_Mutex_Reg *****************************************
Description : When we decide to wait for a mutex, this register function will be called.
Input       : pid_t PID - The process waiting for the mutex. We don't check whether the PID is
                          valid here.
              mutid_t Mutex_ID - The ID of the mutex.
              struct Wait_Object_Struct* Wait_Block_Ptr - The pointer to the wait block.
Output      : None.
Return      : retval_t - If successful,0; if there's no need to wait, "NO_NEED_TO_WAIT(-2)";
                         if the wait failed, "WAIT_FAILURE(-1)".
******************************************************************************/
retval_t _Sys_Wait_Mutex_Reg(pid_t PID,mutid_t Mutex_ID,
                             struct Wait_Object_Struct* Wait_Block_Ptr)
{
    struct List_Head* Traverse_List_Ptr;
    pid_t Traverse_PID;
    
    Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
    if(Mutex_ID>=MAX_MUTEXS)
    {
        Sys_Unlock_Scheduler();
        return(WAIT_FAILURE);
    }
    
    /* See if the mutex is registered */
    if(Mutex_CB[Mutex_ID].Mutex_Reg_Flag==0)
    {
        Sys_Unlock_Scheduler();
        return(WAIT_FAILURE);
    }
    
    /* See if we can occupy the mutex. If yes, return right away */
    if(_Sys_Occupy_Mutex(PID,Mutex_ID)==0)
    {
        Sys_Unlock_Scheduler();
        return(NO_NEED_TO_WAIT);
    }
    
    /* Now it is clear that we cannot occupy the mutex. And the mutex is 
     * registered, so we must wait for it. Now add the wait block to the 
     * wait list. Additionally, when we are waiting for the mutex, we need to
     * make sure that we are inserting the process into the list in priority 
     * order so that we can get the highest priority waiting for the mutex
     * easily.
     */
    Traverse_List_Ptr=Mutex_CB[Mutex_ID].Wait_Object_Head.Next;
    while(Traverse_List_Ptr!=&Mutex_CB[Mutex_ID].Wait_Object_Head)
    {
        Traverse_PID=((struct Wait_Object_Struct*)(Traverse_List_Ptr-1))->PID;
        
        if(PCB[PID].Status.Priority>=PCB[Traverse_PID].Status.Priority)
            break;
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    Sys_List_Insert_Node(&(Wait_Block_Ptr->Object_Head),
                         Traverse_List_Ptr,
                         Traverse_List_Ptr->Next);
    
    Wait_Block_Ptr->Obj_ID=Mutex_ID;
    Wait_Block_Ptr->PID=PID;
    Wait_Block_Ptr->Type=MUTEX;
    
    /* Adjust the priority  */
    _Sys_Wait_Mutex_Prio_Adj(Mutex_ID);
    
    Sys_Unlock_Scheduler();
    return 0;
}
/* Begin Function:_Sys_Wait_Mutex_Reg ****************************************/

/* Begin Function:_Sys_Wait_Mutex_Prio_Adj ************************************
Description : Adjust the priority of the processes to implement the priority 
              inheritance. 
              This function will not check if the mutex ID is valid.
Input       : mutid_t Mutex_ID - The ID of the mutex.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Wait_Mutex_Prio_Adj(mutid_t Mutex_ID)
{
    prio_t Cur_Occupy_Prio;
    prio_t Cur_Wait_High_Prio;
    
    /* See if the mutex is currently occupied. If not, return directly */
    if(Mutex_CB[Mutex_ID].Mutex_Occupy_Cnt==0)
        return;
    
    /* See if there's any process waiting for the mutex */
    if(Mutex_CB[Mutex_ID].Wait_Object_Head.Next==&Mutex_CB[Mutex_ID].Wait_Object_Head)
        return;
    
    /* If the mutex is occupied, get the priority of the process that is occupying it */
    Cur_Occupy_Prio=PCB[Mutex_CB[Mutex_ID].Mutex_Occupy_PID].Status.Priority;
    
    /* The current high waiting priority */
    Cur_Wait_High_Prio=PCB[((struct Wait_Object_Struct*)(Mutex_CB[Mutex_ID].Wait_Object_Head.Next-1))->PID].
                       Status.Priority;
    
    /* See if we need a priority change - Don't change the priority or stop the process
     * when it has a certain mutex.
     */
    if(Cur_Occupy_Prio<Cur_Wait_High_Prio)
        _Sys_Change_Proc_Prio(Mutex_CB[Mutex_ID].Mutex_Occupy_PID,Cur_Wait_High_Prio);
}
/* End Function:_Sys_Wait_Mutex_Prio_Adj *************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
