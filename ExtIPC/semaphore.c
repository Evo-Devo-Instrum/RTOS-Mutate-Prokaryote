/******************************************************************************
Filename    : semaphore.c
Author      : pry
Version     : 0.02
Date        : 25/04/2012
Description : The semaphore module for the operating system.
              when waiting for some semaphore, the implications of the "wait" is:
              1>When the semaphore is deleted, then the function will return as failed.
              2>When the semaphore count is not zero, then the waiting process 
                will get a semaphore and quit the waiting;
              3>When the semaphore is empty, the process will wait until expire.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "ExtIPC\semaphore.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "ExtIPC\semaphore.h"
#include "Kernel\error.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "ExtIPC\semaphore.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Kernel\error.h"

#include "ExtIPC\wait.h"

#include "Syslib\syslib.h"

#include "Syssvc\timer.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Sem_Init ***********************************************
Description : Initialize the semaphore managing unit. Never call this in user
              application.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Sem_Init(void)
{
#if(ENABLE_SEM==TRUE)
    cnt_t Sem_Cnt;
    /* Memset all three arrays */
    Sys_Memset((ptr_int_t)PCB_ExtIPC_Sem,0,sizeof(struct List_Head)*MAX_PROC_NUM);	
    Sys_Memset((ptr_int_t)Sem_CB,0,sizeof(struct Semaphore)*MAX_SEMS);
    Sys_Memset((ptr_int_t)Proc_Sem_Block,0,sizeof(struct Proc_Sem)*MAX_PROC_USE_SEM);
    
    /* Initialize the list heads */
    Sys_Create_List(&Sem_List_Head);
    Sys_Create_List(&Empty_Sem_List_Head);
    Sys_Create_List(&Empty_Proc_Sem_Block_Head);
    
    for(Sem_Cnt=0;Sem_Cnt<MAX_PROC_NUM;Sem_Cnt++)
        Sys_Create_List(&(PCB_ExtIPC_Sem[Sem_Cnt]));
    
    /* Initialize the two lists - the empty block list */
    for(Sem_Cnt=0;Sem_Cnt<MAX_SEMS;Sem_Cnt++)
    {
        Sys_List_Insert_Node(&(Sem_CB[Sem_Cnt].Head),
                             &Empty_Sem_List_Head,
                             Empty_Sem_List_Head.Next);
        Sys_Create_List(&(Sem_CB[Sem_Cnt].Proc_Sem_Head));
        Sys_Create_List(&(Sem_CB[Sem_Cnt].Wait_Object_Head));
        Sem_CB[Sem_Cnt].Sem_ID=Sem_Cnt;
    }
    
    for(Sem_Cnt=0;Sem_Cnt<MAX_PROC_USE_SEM;Sem_Cnt++)
    {
        Sys_List_Insert_Node(&(Proc_Sem_Block[Sem_Cnt].Head),
                             &Empty_Proc_Sem_Block_Head,
                             Empty_Proc_Sem_Block_Head.Next);
    }
    
    /* Clear statistical variable */
    Sem_In_Sys_Cnt=0;
#endif
}
/* End Function:_Sys_Sem_Init ************************************************/

/* Begin Function:Sys_Register_Sem ********************************************
Description : Register a semaphore. For use in application.
Input       : s8* Semaphore_Name - The name of the semaphore.
Output      : None.
Return      : semid_t - The ID of the semaphore, Sem_ID. If the function fail, then
                        the return value will be "-1".
******************************************************************************/
#if(ENABLE_SEM==TRUE)
semid_t Sys_Register_Sem(s8* Sem_Name,size_t Number)
{               
    semid_t Sem_ID;
    struct List_Head* Traverse_Ptr;
    
	Sys_Lock_Scheduler();
    
    Traverse_Ptr=Sem_List_Head.Next;
    /* See if the name is unique in the system. If not, the semaphore cannot be created.
     * to do this we have to traverse the whole list and make comparison on every created
     * semaphore in the system.
     */
    while(Traverse_Ptr!=&Sem_List_Head)
    {
        /* If we found a match */
        if(Sys_Strcmp(Sem_Name,((struct Semaphore*)Traverse_Ptr)->Sem_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(ENOESEM);
            return (-1);
        }
        Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    /* Find an available block */
    if(Empty_Sem_List_Head.Next!=&Empty_Sem_List_Head)
    {
        Sem_ID=((struct Semaphore*)(Empty_Sem_List_Head.Next))->Sem_ID;
        Sys_List_Delete_Node(Empty_Sem_List_Head.Next->Prev,
                             Empty_Sem_List_Head.Next->Next);
        
        Sys_List_Insert_Node(&(Sem_CB[Sem_ID].Head),
                             &Sem_List_Head,
                             Sem_List_Head.Next);
    }
    else 
    {   
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOESEM);
        return (-1);
    }
    
    /* Fill the structure */
    Sem_CB[Sem_ID].Sem_Name=Sem_Name;
    Sem_CB[Sem_ID].Total_Sem=Number;
    Sem_CB[Sem_ID].Sem_Left=Number;

    /* Update statistical variable */
    Sem_In_Sys_Cnt++;
    
    Sys_Unlock_Scheduler();
	return(Sem_ID);
} 
#endif
/* End Function:Sys_Register_Sem *********************************************/

/* Begin Function:Sys_Remove_Sem **********************************************
Description : Remove a semaphore. For use in application.
Input       : semid_t Sem_ID - The ID of the semaphore to remove.
Output      : None.
Return      : retval_t - 0 for success,-1 for failure.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
retval_t Sys_Remove_Sem(semid_t Sem_ID)
{     
    Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
	if(Sem_ID>=MAX_SEMS)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOSEM);
        return (-1);
    }
    
    /* See if the semaphore is registered before */
    if(Sem_CB[Sem_ID].Total_Sem==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOSEM);
        return (-1);
    }
    
    /* Make sure the semaphore is not occupied */
    if(Sem_CB[Sem_ID].Sem_Left!=Sem_CB[Sem_ID].Total_Sem)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENRLSEM);
        return (-1);
    }
    
    /* Delete the node from the active list and place it in the empty list */
    Sys_List_Delete_Node(Sem_CB[Sem_ID].Head.Prev,Sem_CB[Sem_ID].Head.Next);
    Sys_List_Insert_Node(&(Sem_CB[Sem_ID].Head),
                         &Empty_Sem_List_Head,
                         Empty_Sem_List_Head.Next);
    
    /* Clear the variables */
    Sem_CB[Sem_ID].Sem_Name=0;
    Sem_CB[Sem_ID].Total_Sem=0;
    Sem_CB[Sem_ID].Sem_Left=0;
    
    /* Update statistic variable */
    Sem_In_Sys_Cnt--;
    
    Sys_Unlock_Scheduler();
    return 0;
} 
#endif
/* End Function:Sys_Remove_Sem ***********************************************/

/* Begin Function:Sys_Get_Sem_ID **********************************************
Description : Get a semaphore's unique ID through its name. For use in application.
Input       : s8* Sem_Name - The name of the semaphore.
Output      : None.
Return      : semid_t - The ID of the semaphore, Sem_ID. If the function fail, then
                        the return value will be -1.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
semid_t Sys_Get_Sem_ID(s8* Sem_Name)
{               
    struct List_Head* Traverse_Ptr;
    
	Sys_Lock_Scheduler();
    
    Traverse_Ptr=Sem_List_Head.Next;
   
    /* Try to find a match */
    while(Traverse_Ptr!=&Sem_List_Head)
    {
        /* If we found a match */
        if(Sys_Strcmp(Sem_Name,((struct Semaphore*)Traverse_Ptr)->Sem_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            return (((struct Semaphore*)Traverse_Ptr)->Sem_ID);
        }
        Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    /* If it can get here, then the function must have failed */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOESEM);
	return (-1);
} 
#endif
/* End Function:Sys_Get_Sem_ID ***********************************************/

/*Begin Function:Sys_Occupy_Sem************************************************
Description : Occupy a certain semaphore. For application use. The PID is
              automatically the "Current_PID".
Input       : semid_t Sem_ID - The ID of the semaphore, Returned by "Sys_Register_Sem".
			  size_t Number - The number of semaphores needed.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
retval_t Sys_Occupy_Sem(semid_t Sem_ID,size_t Number)
{                                                         		  
	return(_Sys_Occupy_Sem(Current_PID,Sem_ID,Number));
} 
#endif
/* End Function:Sys_Occupy_Sem ***********************************************/

/*Begin Function:_Sys_Occupy_Sem***********************************************
Description : Occupy a certain semaphore. For system use, or where you want to 
              specify the PID. Thus, it is not recommended to use this in
              application.
Input       : pid_t PID - The PID you want.
              semid_t Sem_ID - The ID Of the semaphore, Returned by "Sys_Register_Sem".
			  size_t Number - The number of semaphores needed.
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
retval_t _Sys_Occupy_Sem(pid_t PID,semid_t Sem_ID,size_t Number)
{              
    struct List_Head* Proc_Sem_Block_Ptr;
    struct List_Head* Traverse_Ptr;
    s32 Alloc_Before_Flag=0;
	Sys_Lock_Scheduler();
    
    /*See if the operation is over the boundary */
    if(Sem_ID>=MAX_SEMS)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOSEM);
	    return (-1);
    }
    
    /* See if the semaphore is registered */
    if(Sem_CB[Sem_ID].Total_Sem==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOSEM);
	    return (-1);
    }
    
    /* See if the semaphore is enough */
    if(Sem_CB[Sem_ID].Sem_Left<Number)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENSEMRES);
	    return (-1);
    }
    
    /* See if there are enough occupy blocks */
    if(Empty_Proc_Sem_Block_Head.Next==&Empty_Proc_Sem_Block_Head)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENSEMRES);
	    return (-1);
    }
    
    /* See if the semaphore is ever occupied by it */
    Traverse_Ptr=Sem_CB[Sem_ID].Proc_Sem_Head.Next;
    
    while(Traverse_Ptr!=&(Sem_CB[Sem_ID].Proc_Sem_Head))
    {
        if((((struct Proc_Sem*)Traverse_Ptr)->PID)==PID)
        {
            Sem_CB[Sem_ID].Sem_Left=Sem_CB[Sem_ID].Sem_Left-Number;
            ((struct Proc_Sem*)Traverse_Ptr)->Sem_Number+=Number;
            Alloc_Before_Flag=1;
            break;
        }
        else
            Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    if(Alloc_Before_Flag==0)
    {
        /* Find an empty block and detach it from the empty list */
        Proc_Sem_Block_Ptr=Empty_Proc_Sem_Block_Head.Next;
        Sys_List_Delete_Node(Empty_Proc_Sem_Block_Head.Next->Prev,
                             Empty_Proc_Sem_Block_Head.Next->Next);
        
        /* Insert the node into the corresponding occupy list */
        Sys_List_Insert_Node(Proc_Sem_Block_Ptr,
                             &(Sem_CB[Sem_ID].Proc_Sem_Head),
                             Sem_CB[Sem_ID].Proc_Sem_Head.Next);
        
        /* Insert the node into the corresponding process's occupy list */
        Sys_List_Insert_Node(&(((struct Proc_Sem*)Proc_Sem_Block_Ptr)->Head_In_PCB),
                             &(PCB_ExtIPC_Sem[PID]),
                             PCB_ExtIPC_Sem[PID].Next);   
        
        /* Clear the semaphore count from the Sem_CB block and fill in the Proc_Sem block */
        Sem_CB[Sem_ID].Sem_Left-=Number;
        ((struct Proc_Sem*)Proc_Sem_Block_Ptr)->PID=PID;
        ((struct Proc_Sem*)Proc_Sem_Block_Ptr)->Sem_ID=Sem_ID;
        ((struct Proc_Sem*)Proc_Sem_Block_Ptr)->Sem_Number=Number;
    }
    
	Sys_Unlock_Scheduler();	                                            		  
	return(0);
} 
#endif
/* End Function:_Sys_Occupy_Sem **********************************************/

/* Begin Function:Sys_Free_Sem ************************************************
Description : Free semaphore. For application use. The PID is automatically 
              the "Current_PID".
Input       : semid_t Sem_ID - The ID Of the semaphore, Returned by "Sys_Register_Sem".
			  size_t Number - The number of semaphores to release.
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
retval_t Sys_Free_Sem(semid_t Sem_ID,size_t Number)
{                                                     
	return(_Sys_Free_Sem(Current_PID,Sem_ID,Number));
} 
#endif
/* End Function:Sys_Free_Sem *************************************************/

/* Begin Function:_Sys_Free_Sem ***********************************************
Description : Free semaphore. For system use, or where or where you want to 
              specify the PID. Thus, it is not recommended to use this in
              application. If you free more semaphore than you ever occupy, then 
              all the semaphore will be freed.
Input       : pid_t PID - The PID you want.
              semid_t Sem_ID - The ID Of the semaphore, Returned by "Sys_Register_Sem".
			  size_t Number - The number of semaphores to release.
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
retval_t _Sys_Free_Sem(pid_t PID,semid_t Sem_ID,size_t Number)
{                                          
    struct List_Head* Traverse_Ptr;
    s32 Del_Proc_Sem_Block_Flag=0;
    cnt_t Real_Free_Number=0;
    cnt_t Wait_Proc_Cnt;
    pid_t Wait_Occupy_PID;
    
	Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
    if(Sem_ID>=MAX_SEMS)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOSEM);
	    return (-1);
    }
    
    /* See if the semaphore is registered */
    if(Sem_CB[Sem_ID].Total_Sem==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOSEM);
	    return (-1);
    }
    
    /* See if the semaphore is ever occupied by the process */
    Traverse_Ptr=Sem_CB[Sem_ID].Proc_Sem_Head.Next;
    
    while(1)
    {
        if((((struct Proc_Sem*)Traverse_Ptr)->PID)==PID)
        {
            /* If we get here, then we have found the block */
            if(Number>=(((struct Proc_Sem*)Traverse_Ptr)->Sem_Number))
            {
                Del_Proc_Sem_Block_Flag=1;
                Sem_CB[Sem_ID].Sem_Left+=((struct Proc_Sem*)Traverse_Ptr)->Sem_Number;
                Real_Free_Number=(((struct Proc_Sem*)Traverse_Ptr)->Sem_Number);
            }
            else
            {
                Sem_CB[Sem_ID].Sem_Left+=Number;
                ((struct Proc_Sem*)Traverse_Ptr)->Sem_Number-=Number;
                Real_Free_Number=Number;
            }
            
            break;
        }
         
        Traverse_Ptr=Traverse_Ptr->Next;
        
        if(Traverse_Ptr==&(Sem_CB[Sem_ID].Proc_Sem_Head))
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(ENSEMRES);
	        return (-1);
        }   
    }
    
    if(Del_Proc_Sem_Block_Flag==1)
    {
        /* Delete the node from the corresponding process's occupy list */
        Sys_List_Delete_Node(((struct Proc_Sem*)Traverse_Ptr)->Head_In_PCB.Prev,
                             ((struct Proc_Sem*)Traverse_Ptr)->Head_In_PCB.Next);
        
        /* Delete the node from the corresponding occupy list */
        Sys_List_Delete_Node(Traverse_Ptr->Prev,Traverse_Ptr->Next);   
        
        /* Put it back into the empty list */
        Sys_List_Insert_Node(Traverse_Ptr,
                             &Empty_Proc_Sem_Block_Head,
                             Empty_Proc_Sem_Block_Head.Next);
        
        /* Clear the block */
        ((struct Proc_Sem*)Traverse_Ptr)->Sem_Number=0;
        ((struct Proc_Sem*)Traverse_Ptr)->Sem_ID=0;
        ((struct Proc_Sem*)Traverse_Ptr)->PID=0;
    }
    
    /* See if there are any process waiting for this semaphore. If yes, then notify it */
    for(Wait_Proc_Cnt=0;Wait_Proc_Cnt<Real_Free_Number;Wait_Proc_Cnt++)
    {
        /* See if any process is waiting for the semaphore */
        if(Sem_CB[Sem_ID].Wait_Object_Head.Next==&Sem_CB[Sem_ID].Wait_Object_Head)
            break;
        
        /* Get the information and let this process get the semaphore */
        Wait_Occupy_PID=((struct Wait_Object_Struct*)(Sem_CB[Sem_ID].Wait_Object_Head.Next-1))->PID;
        /* Mark that the wait is successful */
        ((struct Wait_Object_Struct*)(Sem_CB[Sem_ID].Wait_Object_Head.Next-1))->Succeed_Flag=1;
        /* Delete the wait block from the semaphore wait list */
        Sys_List_Delete_Node(&Sem_CB[Sem_ID].Wait_Object_Head,Sem_CB[Sem_ID].Wait_Object_Head.Next->Next);
        _Sys_Occupy_Sem(Wait_Occupy_PID,Sem_ID,1);
        /* Try to stop the timer if possible */
        Sys_Proc_Delay_Cancel(Wait_Occupy_PID);
        /* Wake the process up */
        _Sys_Set_Ready(Wait_Occupy_PID);
    }
    
	Sys_Unlock_Scheduler();	                                            		  
	return(0);
} 
#endif
/* End Function:_Sys_Free_Sem ************************************************/

/* Begin Function:Sys_Free_All_Sem ********************************************
Description : Release all semaphore the proceess occupied. For application use.
              The PID is automatically the "Current_PID".
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
void Sys_Free_All_Sem(void)
{                                              
    _Sys_Free_All_Sem(Current_PID);
} 
#endif
/* End Function:Sys_Free_All_Sem *********************************************/

/* Begin Function:_Sys_Free_All_Sem *******************************************
Description : Release all semaphore the proceess occupied. For system use, or 
              where you want to specify the PID. Thus it is not recommended to
              call it in applications.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
void _Sys_Free_All_Sem(pid_t PID)
{     
    struct Proc_Sem* Proc_Sem_Ptr;
    
    Sys_Lock_Scheduler();	
    
    /* Free the semaphores until the list is completely empty */
    while((PCB_ExtIPC_Sem[PID].Next)!=&(PCB_ExtIPC_Sem[PID]))
    {
        /* Get the Proc_Sem type pointer */
        Proc_Sem_Ptr=(struct Proc_Sem*)(PCB_ExtIPC_Sem[PID].Next-1);
        /* Restore all the occupied number */
        _Sys_Free_Sem(PID,Proc_Sem_Ptr->Sem_ID,Proc_Sem_Ptr->Sem_Number);
    }
        
    Sys_Unlock_Scheduler();	  
} 
#endif
/* End Function:_Sys_Free_All_Sem ********************************************/

/* Begin Function:Sys_Query_Sem_Amount ****************************************
Description : Query the remaining semaphore amount.
Input       : semid_t Sem_ID -The semaphore's ID.
Output      : None.
Return      : size_t - If there is no semaphore available,or the semaphore is not 
              registered, or the operation is over limit, it will return 0.
              Otherwise it will be the number of resources.
******************************************************************************/
#if(ENABLE_SEM==TRUE)
size_t Sys_Query_Sem_Amount(semid_t Sem_ID)
{
    /* See if the operation is over the boundary */
    if(Sem_ID>=MAX_SEMS)
    {
        Sys_Unlock_Scheduler();
        return(0);
    }
    
	/* See if the semaphore is registered */
    if(Sem_CB[Sem_ID].Total_Sem==0)
    {
        Sys_Unlock_Scheduler();
        return(0);
    }
    
	return(Sem_CB[Sem_ID].Sem_Left);                                         
}
#endif
/* End Function:Sys_Query_Sem_Amount *****************************************/

/* Begin Function:_Sys_Wait_Sem_Reg *****************************************
Description : When we decide to wait for a semaphore, this register function will be called.
              Take note that when we are waiting for a semaphore, we can only get one semaphore.
              If you want to get more than a semaphore in a single function call, you will have to
              call the "Sys_Occupy_Sem" again and again.
Input       : pid_t PID - The process waiting for the semaphore. We don't check whether the PID is
                          valid here.
              semid_t Sem_ID - The ID of the semaphore.
              struct Wait_Object_Struct* Wait_Block_Ptr - The pointer to the wait block.
Output      : None.
Return      : retval_t - If successful,0; if there's no need to wait, "NO_NEED_TO_WAIT(-2)";
                         if the wait failed, "WAIT_FAILURE(-1)".
******************************************************************************/
retval_t _Sys_Wait_Sem_Reg(pid_t PID,semid_t Sem_ID,
                             struct Wait_Object_Struct* Wait_Block_Ptr)
{
    struct List_Head* Traverse_List_Ptr;
    pid_t Traverse_PID;
    
    Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
    if(Sem_ID>=MAX_SEMS)
    {
        Sys_Unlock_Scheduler();
        return(WAIT_FAILURE);
    }
    
    /* See if the semaphore is registered */
    if(Sem_CB[Sem_ID].Total_Sem==0)
    {
        Sys_Unlock_Scheduler();
        return(WAIT_FAILURE);
    }
    
    /* See if we can occupy one of the semaphore. If yes, return right away */
    if(_Sys_Occupy_Sem(PID,Sem_ID,1)==0)
    {
        Sys_Unlock_Scheduler();
        return(NO_NEED_TO_WAIT);
    }
    
    /* Now it is clear that we cannot occupy the semaphore. And the semaphore is 
     * registered, so we must wait for it. Now add the wait block to the 
     * wait list. Additionally, when we are waiting for the semaphore, we need to
     * make sure that we are inserting the process into the list in priority 
     * order so that we can get the highest priority waiting for the mutex
     * easily. Take note that we do not implement priority inheritance here.
     * (If you need priority inheritance, please use the mutex.)
     */
    Traverse_List_Ptr=Sem_CB[Sem_ID].Wait_Object_Head.Next;
    while(Traverse_List_Ptr!=&Sem_CB[Sem_ID].Wait_Object_Head)
    {
        Traverse_PID=((struct Wait_Object_Struct*)(Traverse_List_Ptr-1))->PID;
        
        if(PCB[PID].Status.Priority>=PCB[Traverse_PID].Status.Priority)
            break;
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    Sys_List_Insert_Node(&(Wait_Block_Ptr->Object_Head),
                         Traverse_List_Ptr,
                         Traverse_List_Ptr->Next);
    
    Wait_Block_Ptr->Obj_ID=Sem_ID;
    Wait_Block_Ptr->PID=PID;
    Wait_Block_Ptr->Type=SEMAPHORE;
    
    Sys_Unlock_Scheduler();
    return 0;
}
/* Begin Function:_Sys_Wait_Sem_Reg ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
