/******************************************************************************
Filename    : wait.c
Author      : pry
Date        : 05/07/2013
Version     : 0.01
Description : The wait kernel object module for the RMV RTOS. Only the ExtIPCs other
              than pipes and shared memory can be wait.
              The implications of "wait":
              1>Once the object is deleted, then the function will return as failed;
              2>For different kernel objects, there are different implications for
                waiting a kernel object.
              
              Take note that you cannot wait for a pipe or a shared memory region.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#include "ExtIPC\wait.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#include "ExtIPC\wait.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "ExtIPC\wait.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\error.h"
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Kernel\signal.h"

#include "ExtIPC\mutex.h"
#include "ExtIPC\semaphore.h"
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\wait.h"

#include "Syslib\syslib.h"
#include "Syssvc\timer.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Wait_Init **********************************************
Description : Initialize the wait queue.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Wait_Init(void)
{ 
    cnt_t Block_Cnt;
    Sys_Create_List(&Wait_Empty_List_Head);
    
    /* Clear the whole control block array */
    Sys_Memset((ptr_int_t)Wait_CB,0,MAX_WAIT_BLOCKS*sizeof(struct Wait_Object_Struct));
    Sys_Memset((ptr_int_t)PCB_Wait,0,MAX_PROC_NUM*sizeof(struct PCB_Wait_Struct));
    
    /* Put all the wait control blocks in the empty ones */
    for(Block_Cnt=0;Block_Cnt<MAX_WAIT_BLOCKS;Block_Cnt++)
    {
        Sys_List_Insert_Node(&(Wait_CB[Block_Cnt].Object_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
    }
    
    /* Fill the PCB part */
    for(Block_Cnt=0;Block_Cnt<MAX_PROC_NUM;Block_Cnt++)
    {
        Sys_Create_List(&PCB_Wait[Block_Cnt].Wait_Head);
        PCB_Wait[Block_Cnt].PID=Block_Cnt;
    }
    /* Clear the statistical variable */
    Wait_Relation_In_Sys=0;
}
/* End Function:_Sys_Wait_Init ***********************************************/

/* Begin Function:Sys_Wait_Object *********************************************
Description : Wait for a certain kernel object.
Input       : cnt_t Object_ID - The object ID.
              cnt_t Object_Type - The object type.
              time_t Time - The wait time. If the time is WAIT_INFINITE, then the 
              process will wait until the status of the kernel object is changed.
Output      : None.
Return      : cnt_t - The object ID returned, The same as the input object ID.
******************************************************************************/
cnt_t Sys_Wait_Object(cnt_t Object_ID,cnt_t Object_Type,time_t Time)
{
    cnt_t Retval;
    struct Wait_Object_Struct* Wait_Block_Ptr;

    Sys_Lock_Scheduler();
    
    /* Try to find an empty object wait block */
    if(&Wait_Empty_List_Head==Wait_Empty_List_Head.Next)
    {
        Sys_Set_Errno(ENOWAITBLK);
        Sys_Unlock_Scheduler();
        return -1;
    }
    
    Wait_Block_Ptr=(struct Wait_Object_Struct*)(Wait_Empty_List_Head.Next);
    Sys_List_Delete_Node(Wait_Block_Ptr->PCB_Head.Prev,Wait_Block_Ptr->PCB_Head.Next);
    /* Clear the wait success flag */
    Wait_Block_Ptr->Succeed_Flag=0;
    
    /* See what object do we need to wait for */
    switch(Object_Type)
    {
        case MUTEX:Retval=_Sys_Wait_Mutex_Reg(Current_PID,Object_ID,Wait_Block_Ptr);break;
        case SEMAPHORE:Retval=_Sys_Wait_Sem_Reg(Current_PID,Object_ID,Wait_Block_Ptr);break;
        case MSGQUEUE:Retval=_Sys_Wait_Msg_Queue_Reg(Current_PID,Object_ID,Wait_Block_Ptr);break;
        default:Retval=WAIT_FAILURE;break;
    }
    
    /* See if the register function succeeded. If not, we need to put the block back */
    if(Retval==WAIT_FAILURE)
    {
        Sys_Set_Errno(ENOOBJID);
        Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
        Sys_Unlock_Scheduler();
        return -1;
    }
    
    if(Retval==NO_NEED_TO_WAIT)
        Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
    
    Sys_Unlock_Scheduler();
    
    /* Now that we have registered the object under their corresponding process,
     * see if the retval is "NO_NEED_TO_WAIT".
     */
    if(Retval!=NO_NEED_TO_WAIT)
    {
        /* We need to wait until the time is up. See if we need to wait forever */
        if(Time!=WAIT_INFINITE)
            Sys_Proc_Delay_Tick(Time);
        else
            _Sys_Clr_Ready(Current_PID);
        
        /* The delay must have ended. Now put the blocks back. */
        Sys_List_Delete_Node(Wait_Block_Ptr->PCB_Head.Prev,Wait_Block_Ptr->PCB_Head.Next);
        Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
        /* This means that the wait is ended by the timer. This means that the wait didn't succeed. */
        if(((struct Wait_Object_Struct*)(PCB_Wait[Current_PID].Wait_Head.Next))->Succeed_Flag==0)
            Sys_List_Delete_Node(Wait_Block_Ptr->Object_Head.Prev,Wait_Block_Ptr->Object_Head.Next);
    }
    
    /* At last, return our wait results */  
    if(((struct Wait_Object_Struct*)(PCB_Wait[Current_PID].Wait_Head.Next))->Succeed_Flag==1)    
        return Object_ID;
    
    /* If it gets here, the wait must have failed */
    Sys_Set_Errno(EWAITOVERTIME);
    return -1;
}
/* End Function:Sys_Wait_Object **********************************************/

/* Begin Function:Sys_Wait_Multi_Objects **************************************
Description : Wait for a certain kernel object.
Input       : cnt_t* Object_ID - The object ID array. The number of IDs in the array
                                 should be "Object_Number".
              cnt_t Object_Type - The object type. The number of type identifiers
                                  should be the same as "Object_ID".
              cnt_t Object_Number - The number of objects.
              time_t Time - The wait time. If the time is WAIT_INFINITE, then the 
              process will wait until the status of the kernel object is changed.
Output      : cnt_t* Object_Succeed_List - This will contain the list of succesful waits. 
                                           The list will end with "-1".
Return      : cnt_t - If the wait succeeds, then it will be 0; else -1.
******************************************************************************/
cnt_t Sys_Wait_Multi_Objects(cnt_t* Object_ID,cnt_t* Object_Type,cnt_t Object_Number,time_t Time,
                             cnt_t* Object_Succeed_List)
{ 
    cnt_t Retval;
    struct List_Head Wait_Block_List_Head;
    struct Wait_Object_Struct* Wait_Block_Ptr;
    /* The count variable for objects */
    cnt_t Obj_Number_Cnt;
    /* This flag will show whether the block find succeeds */
    cnt_t Find_Block_Fail_Flag=0;
    /* This will show how many of these waits have failed */
    cnt_t Wait_Fail_Cnt=0;
    /* This will help fill the "Object_Succeed_List" */
    cnt_t List_Fill_Index=0;
    
    /* Make a list to contain the detached wait blocks */
    Sys_Create_List(&Wait_Block_List_Head);
    
    Sys_Lock_Scheduler();
    
    /* Try to find an enough wait blocks */
    for(Obj_Number_Cnt=0;Obj_Number_Cnt<Object_Number;Obj_Number_Cnt++)
    {
        if(&Wait_Empty_List_Head==Wait_Empty_List_Head.Next)
        {
            Find_Block_Fail_Flag=1;
            break;
        }  
        Wait_Block_Ptr=(struct Wait_Object_Struct*)(Wait_Empty_List_Head.Next);
        
        Sys_List_Delete_Node(Wait_Block_Ptr->PCB_Head.Prev,Wait_Block_Ptr->PCB_Head.Next);
        Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Block_List_Head,Wait_Block_List_Head.Next);
    }
    
    /* If we are unable to find that many blocks, put the blocks that we have already got back. */
    if(Find_Block_Fail_Flag==1)
    {
        while(&Wait_Block_List_Head==Wait_Block_List_Head.Next)
        {
            Sys_List_Delete_Node(Wait_Block_Ptr->PCB_Head.Prev,Wait_Block_Ptr->PCB_Head.Next);
            Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
        }
        
        Sys_Set_Errno(ENOWAITBLK);
        Sys_Unlock_Scheduler();
        return -1;
    }
    
    /* See what object do we need to wait for */
    for(Obj_Number_Cnt=0;Obj_Number_Cnt<Object_Number;Obj_Number_Cnt++)
    {
        Wait_Block_Ptr=(struct Wait_Object_Struct*)(Wait_Block_List_Head.Next);
        Sys_List_Delete_Node(Wait_Block_Ptr->PCB_Head.Prev,Wait_Block_Ptr->PCB_Head.Next);
        
        switch(Object_Type[Obj_Number_Cnt])
        {
            case MUTEX:Retval=_Sys_Wait_Mutex_Reg(Current_PID,Object_ID[Obj_Number_Cnt],Wait_Block_Ptr);break;
            case SEMAPHORE:Retval=_Sys_Wait_Sem_Reg(Current_PID,Object_ID[Obj_Number_Cnt],Wait_Block_Ptr);break;
            case MSGQUEUE:Retval=_Sys_Wait_Msg_Queue_Reg(Current_PID,Object_ID[Obj_Number_Cnt],Wait_Block_Ptr);break;
            default:Retval=WAIT_FAILURE;break;
        }
        
        /* See if the register function succeeded. If not, we need to put the block back */
        if(Retval==WAIT_FAILURE)
        {
            Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
            Wait_Fail_Cnt++;
        }
    
        if(Retval==NO_NEED_TO_WAIT)
        {
            Object_Succeed_List[List_Fill_Index]=Object_ID[Obj_Number_Cnt];
            List_Fill_Index++;
            Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
        }
    }
    
    /* If all of these waits failed, return now */
    if(Wait_Fail_Cnt==Object_Number)
    {
        Sys_Unlock_Scheduler();
        return -1;
    }
    
    Sys_Unlock_Scheduler();
    
    /* If all kernel objects requires waiting */
    if(List_Fill_Index==0)
    {
        /* We need to wait until the time is up. See if we need to wait forever */
        if(Time!=WAIT_INFINITE)
            Sys_Proc_Delay_Tick(Time);
        else
            _Sys_Clr_Ready(Current_PID);
        
        /* The delay must have ended. See any of these waits succeeds and put the 
         * all the blocks back. 
         */
        while(PCB_Wait[Current_PID].Wait_Head.Next!=&PCB_Wait[Current_PID].Wait_Head)
        {
            Sys_List_Delete_Node(Wait_Block_Ptr->PCB_Head.Prev,Wait_Block_Ptr->PCB_Head.Next);
            Sys_List_Insert_Node(&(Wait_Block_Ptr->PCB_Head),&Wait_Empty_List_Head,Wait_Empty_List_Head.Next);
            
            if(((struct Wait_Object_Struct*)(PCB_Wait[Current_PID].Wait_Head.Next))->Succeed_Flag==0)
                Sys_List_Delete_Node(Wait_Block_Ptr->Object_Head.Prev,Wait_Block_Ptr->Object_Head.Next);
            else
            {
                /* Suceeded. We put this into the succeed list. There's no need to delete the node
                 * from the object list; It has already been deleted since the wait succeeds.
                 */
                Object_Succeed_List[List_Fill_Index]=
                ((struct Wait_Object_Struct*)(PCB_Wait[Current_PID].Wait_Head.Next))->Obj_ID;
                
                List_Fill_Index++;
            }
        }
    }
    
    /* At last, return our wait results */  
    if(List_Fill_Index!=0)
    {
        if(List_Fill_Index<Object_Number-1)
        {
            List_Fill_Index++;
            Object_Succeed_List[List_Fill_Index]=-1;
        }
        
        return 0;
    }
    
    /* If it gets here, the wait must have failed */
    Sys_Set_Errno(EWAITOVERTIME);
    return -1;
}
/* End Function:Sys_Wait_Multi_Objects ***************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
