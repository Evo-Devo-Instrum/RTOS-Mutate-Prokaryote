/******************************************************************************
Filename    : scheduler.c
Author      : pry
Date        : 07/04/2012
Version     : 0.15
Description : The kernel for the Mutate(TM)-prokaryote operating system.
******************************************************************************/

/* Version Information ********************************************************
1.Created By pry                                                    -----------
  Created the file from the kernel(scheduler) of RTOS-Rookie.
2.Modified By pry                                                    22/04/2013
  The kernel now supports hard real-time task switching. Not tested, but already
  useable.
3.Modified By pry                                                    23/04/2013
  Deleted the "externs.h" and splitted the definitions&declarations into separate
  headers.The config reporter is also deleted.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\kernel_proc.h"
#include "Kernel\interrupt.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\kernel_proc.h"
#include "Kernel\interrupt.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Kernel\scheduler.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#include "Kernel\kernel_proc.h"
#include "Kernel\interrupt.h"

#include "Memmgr\memory.h"

#include "ExtIPC\semaphore.h"
#include "ExtIPC\pipe.h"
#include "ExtIPC\sharemem.h"
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\mutex.h"

#include "Syssvc\timer.h"

#include "Syslib\syslib.h"

#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:Sys_Restart *************************************************
Description : Restarts the OS whenever you want. The function will reset the 
              processor.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Restart(void)
{
    /* Stop the counter and prepare for a restart */
	Sys_Freeze_Systick();	                 									   
	Sys_Lock_Interrupt();
	
    /* Perform a reset of the system. It will not reset the peripherals */
	NVIC_AIRCR=RESET_VALUE;	
    
    /* Perform a total restart */   
	/* NVIC_AIRCR=RESTART_VALUE; */												   
	
	while(1);
}
/* End Function:Sys_Restart **************************************************/

/* Begin Function:_Sys_Proc_Load **********************************************
Description : The autorun-on-start process loader. This is used by the "Init"
              process exclusively. Don't call it in the user application.
Input       : struct Proc_Init_Struct* Process -The pointer to the struct containing 
              everything necessary.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Proc_Load(struct Proc_Init_Struct* Process)
{	
    pid_t PID=Process->PID;
    /* Indicates that this PID is in use. */
	PCB[PID].Status.Running_Status|=OCCUPY;   
    /* Priority */                                          
    PCB[PID].Status.Priority=Process->Priority;                                                    
                                                
	PCB[PID].Info.Init_Stack_Ptr=(ptr_int_t)(Process->Stack_Address+Process->Stack_Size-4-STACK_SAFE_RDCY);

    /* This is the SP that we actually use when the system is running. Load the initial
     * SP into the slot now.
     */	
	PCB_Cur_SP[PID]=PCB[PID].Info.Init_Stack_Ptr-PRESET_STACK_SIZE; 

    /* The entrance address of the process. */													   
    PCB[PID].Info.Init_Addr_Ptr=(ptr_int_t)(Process->Entrance);
      
    /* The number of time slices to run */                                                    
    PCB[PID].Time.Min_Tim=Process->Min_Slices;                                                       
    PCB[PID].Time.Max_Tim=Process->Max_Slices;                                                        
	PCB[PID].Time.Cur_Tim=Process->Cur_Slices; 
    PCB[PID].Time.Lft_Tim=Process->Cur_Slices;

    /* Fill in the name and PPID */
    PCB[PID].Info.PID=Process->PID;
    PCB[PID].Info.Name=Process->Name;
    PCB[PID].Info.PPID=0;

    /* Initialize its stack */
    _Sys_Proc_Stack_Init(PID);    
    
    /* Register the number of processes running at the priority */
    if(Process->Ready_Flag==READY)
        _Sys_Ins_Proc_Into_New_Prio(PID,PCB[PID].Status.Priority);            

    /* Refresh the system statistical variable */
    System_Status.Proc.Total_Proc_Number++;
}
/* End Function:_Sys_Proc_Load  **********************************************/

/* Begin Function:_Sys_Systick_Init *******************************************
Description : Initialize the systick timer(and set important interrupt priority).
Input       : cnt_t Ticks - The tick value to the system clock.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Systick_Init(cnt_t Ticks)
{	 
    /* Set the vector table base address at 0x08000000*/ 	 
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);			               
    if(SysTick_Config(Ticks))
    {
        /* "1" indicates that there is an error.*/
        while(1);                                                              
    }
    
    /* The systick IRQ has a very high priority */
    NVIC_SetPriority(SysTick_IRQn,0x00);  
    /* Set its Priority to the lowest. 4 may not be low enough, this is processor
     * specific.
     */
    NVIC_SetPriority(PendSV_IRQn,0x0F);      
}
/* End Function:_Sys_Systick_Init ********************************************/

/* Begin Function:_Sys_Proc_Stack_Init ****************************************
Description : Initiate the process stack when trying to start a process. Never
              call this function in user application.
Input       : pid_t PID - The PID of the process.
Output      : None.
Return      : None.
Other       : When the system stack safe redundancy is set to zero, the stack 
              looks like this when we try to step into the next process by 
              context switch:
              HIGH-->  XPSR PC LR(1) R12 R3-R0 LR(2) R11-R4 -->LOW 
              We need to set the stack correctly pretending that we are 
              returning from an systick timer interrupt. Thus, we set the XPSR
              to avoid INVSTATE; set PC to the pseudo-process entrance; set LR
              (1) to 0 because the process does not return to anything; set the 
              R12,R3-R0 to 0; set the LR(2) to 0xFFFFFFF9 to conjure a system-
              interrupt-handler-return-to-MSP context; set R11-R4 to 0.
              
              Then, when the stack safe redundancy is not 0, the whole data 
              structure will have an offset value to the stack limit edge.
              The stack redundancy can be set in the config file.
******************************************************************************/
void _Sys_Proc_Stack_Init(pid_t PID)
{
    void(*Process_Pointer)(void)=(void(*)(void))PCB[PID].Info.Init_Addr_Ptr; 
   
    /* CM3:for xPSR. fill the T bit,or an INVSTATE will happen.*/	 	                           
	*((ptr_int_t*)(PCB_Cur_SP[PID]+4*16))=0x01000200; 

    /* CM3:Set the process entrance.*/                            
	*((ptr_int_t*)(PCB_Cur_SP[PID]+4*15))=(ptr_int_t)Process_Pointer;

    /* CM3:Set the LR(1) */
    *((ptr_int_t*)(PCB_Cur_SP[PID]+4*14))=(ptr_int_t)0;
    
    /* CM3:Set the LR(2) correctly. This is for the manual stacking in the assembly
     * function part. In CM3 this means that we are returning from the system 
     * interrupt and we will use MSP as the stack after we returns. 
     * This LR value is used when the LR is popped manually.
     */                            
	*((ptr_int_t*)(PCB_Cur_SP[PID]+4*8))=0xFFFFFFF9;                   
}
/* End Function:_Sys_Proc_Stack_Init *****************************************/

/*Begin Function:_Sys_Scheduler_Init*******************************************
Description : The system scheduler initialization function.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void  _Sys_Scheduler_Init(void)
{      
    cnt_t Count;
    
    /* Systemis still booting */
    System_Status.Kernel.Boot_Done=FALSE;
    
    /* Clear the statistic variable for the kernel */
    Sys_Memset((ptr_int_t)(&System_Status),0,sizeof(struct Sys_Status_Struct));
    
    /* Initialize the system clock.*/  
    _Sys_Systick_Init(MIN_TIMESLICE_TICK);                                             

    /* Initialize the priority list. We do this to facilitate bidirectional search.
     * We can get the pointer to this priority level when we have the priority level
     * number, and vice versa.
     */
    for(Count=0;Count<MAX_PRIO_NUM;Count++)
    {
        Prio_List[Count].Priority=Count;
        Prio_List[Count].Proc_Num=0;
        Sys_Create_List((struct List_Head*)&(Prio_List[Count].Running_List));
    }

    Sys_Create_List((struct List_Head*)(&Prio_List_Head));

    /* Initialize the list for each possible task slot */
    for(Count=0;Count<MAX_PROC_NUM;Count++)
    {
        PCB[Count].Status.Sleep_Count=1;
        PCB[Count].Status.Status_Lock_Count=0;
        Sys_Create_List((struct List_Head*)(&(PCB[Count].Head))); 
    }
    
    /* Clear the pending system scheduling count */
    Pend_Sched_Cnt=0;
}
/* End Function:_Sys_Scheduler_Init ******************************************/

/* Begin Function:_Sys_Load_Init **********************************************
Description : The loader of the "Init" process. The "Init" process is the first
              process in the system, and its priority is the highest at first.
              After it has done the process initialization task, its priority
              will be the lowest one.
Input       : None. 
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Load_Init(void)                                                  
{   
    /* Load it as a routine*/
    struct Proc_Init_Struct Process;

    Process.PID=0;
    Process.Name=(s8*)"Init";								                                 
    Process.Entrance=_Sys_Init;                                                             
    Process.Stack_Address=(ptr_int_t)Kernel_Stack;				                                          
    Process.Stack_Size=KERNEL_STACK_SIZE;
    Process.Max_Slices=4;                                                           
    Process.Min_Slices=1;                                                           
    Process.Cur_Slices=2;                                                                                                      
    Process.Priority=0;
    Process.Ready_Flag=READY;
    
    /* Load the init process */
    _Sys_Proc_Load(&Process); 

    /* Set the "Current_PID" and "Current_Prio" manually */
    Current_PID=0;
    Prio_List[0].Proc_Num=1;
    Current_Prio=PCB[0].Status.Priority;
         
    /* Set the left time of "Init" to 0 to trigger a normal context switch */
    PCB[0].Time.Lft_Tim=0; 
    
    /* Call the "Init" process function. Never returns.*/
    _Sys_Init();             
}
/* End Function:_Sys_Load_Init ***********************************************/

/* Begin Function:_Sys_Del_Proc_From_Cur_Prio *********************************
Description : Used in the system to delete the process from current priority level.
              Take note that this function will not check if the PID is valid,
              nor does it have atomic operation protection, so the function is used
              in the OS only. Never call it in the user application.
Input       : pid_t PID -The PID of the process to set to the ready state.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Del_Proc_From_Cur_Prio(pid_t PID)
{
    Sys_Lock_Scheduler();
    /* If the process is going to be not ready */
    if(PCB[PID].Status.Sleep_Count==0)
    {
        /* See if it is the only one in the list - This must be the only active process */
        if(Prio_List[PCB[PID].Status.Priority].Proc_Num==1)
        {   
            /* Delete this process from the priority first */
            Sys_List_Delete_Node(PCB[PID].Head.Prev,PCB[PID].Head.Next);
            
            /* Now no process is running on this level. Delete this priority level */
            Prio_List[PCB[PID].Status.Priority].Proc_Num=0; 

            Sys_List_Delete_Node(Prio_List[PCB[PID].Status.Priority].Head.Prev,
                                 Prio_List[PCB[PID].Status.Priority].Head.Next);
            
            /* Refresh the system statistical variable */
            System_Status.Proc.Active_Prios--;               
        }
        else
        {
            /* We delete the process directly */
            Sys_List_Delete_Node(PCB[PID].Head.Prev,PCB[PID].Head.Next);

            /* The number of running processes at is priority level will drop by 1 */
            Prio_List[PCB[PID].Status.Priority].Proc_Num--;      
        }

        /* Specify itself as out of running list, but we do not clear its priority */
        PCB[PID].Status.Sleep_Count=1;
        /* Update the statistical variable */
        System_Status.Proc.Running_Proc_Number--;  
    }
    else
    {
        /* Do nothing */
    }
    
    Sys_Unlock_Scheduler();
}
/* End Function:_Sys_Del_Proc_From_Cur_Prio **********************************/

/* Begin Function:_Sys_Ins_Proc_Into_New_Prio *********************************
Description : Used in the system to insert the process to a certain priority level.
              Take note that this function will not check if the PID is valid.
              Never call it in the user application.
Input       : pid_t PID -The PID of the process to set to the ready state.
              prio_t Priority - The priority that you want to insert it into.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Ins_Proc_Into_New_Prio(pid_t PID,prio_t Priority)
{
    struct List_Head* Traverse_List_Ptr;
    
    Sys_Lock_Scheduler();
    /* Only when the node is already out of list can we insert it into new places */
    if(PCB[PID].Status.Sleep_Count==1)
    {
        /* See if the priority level exists in the current priority list */
        if(Prio_List[Priority].Proc_Num>0)
        {
            /* No need to create a new running list, just insert it into the list and
             * increase the running number by 1.
             */
            Sys_List_Insert_Node((struct List_Head*)&(PCB[PID].Head),
                                 Prio_List[Priority].Running_List.Prev,
                                 (struct List_Head*)&(Prio_List[Priority].Running_List));
            Prio_List[Priority].Proc_Num++;
        }  
        /* We need to initialize the priority level in the priority list first */
        else  
        {
            Traverse_List_Ptr=(Prio_List_Head.Next);
            while(Traverse_List_Ptr!=(&Prio_List_Head))
            {
                /* Find one that is smaller; Or, the priority is the lowest when
                 * we finish traversing the list.
                 */
                if((((struct Prio_List_Struct*)Traverse_List_Ptr)->Priority)<Priority)
                {
                   break;    
                }
                /* Move to the next node */
                Traverse_List_Ptr=Traverse_List_Ptr->Next;
            } 

            /* Insert the priority node */
            Sys_List_Insert_Node(((struct List_Head*)(&(Prio_List[Priority].Head))),
                                 Traverse_List_Ptr->Prev,
                                 Traverse_List_Ptr); 
            
            /* Now register the process under the priority level's corresponding 
             * running list */            
            Sys_List_Insert_Node((struct List_Head*)&(PCB[PID].Head),
                                 Prio_List[Priority].Running_List.Prev,
                                 (struct List_Head*)&(Prio_List[Priority].Running_List));
            /* There are already a new process running at the level (this one). Register it.
             * Remember, this registration is done by specifying the PID, not the by list insertion.
             */
            Prio_List[Priority].Proc_Num=1;
            
            /* Update the statistical variable */
            System_Status.Proc.Active_Prios++;
        } 
        
        /* Specify the process as in list */
        PCB[PID].Status.Sleep_Count=0;
        /* Write the priority back */
        PCB[PID].Status.Priority=Priority;
        /* Update the statistical variable */
        System_Status.Proc.Running_Proc_Number++;         
    }
    else
    {
        /* Do nothing */
    }
    
    Sys_Unlock_Scheduler();
}
/* End Function:_Sys_Ins_Proc_Into_New_Prio **********************************/

/* Begin Function:Sys_Change_Proc_Prio ****************************************
Description : The function is used to change the process's priority. Used in user 
              application.
Input       : pid_t PID - The PID of the process.
              prio_t Priority - The priority of the task to assign to.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
retval_t Sys_Change_Proc_Prio(pid_t PID,prio_t Priority)
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
    
    /* See if a state change is allowed */
    if(PCB[PID].Status.Status_Lock_Count>0)
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    /* See if the priority exists in the system */
    if((Priority<0)||(Priority>=MAX_PRIO_NUM))
    {
       Sys_Set_Errno(ENOPRIO);
        return (-1);
    }
    
    return _Sys_Change_Proc_Prio(PID,Priority);
}
/* End Function:Sys_Change_Proc_Prio *****************************************/

/* Begin Function:_Sys_Change_Proc_Prio ***************************************
Description : The function is used to change the process's priority. Used in the
              kernel; don't call this in user application.
Input       : pid_t PID - The PID of the process.
              prio_t Priority - The priority of the task to assign to.
Output      : None.
Return      : None.
******************************************************************************/
retval_t _Sys_Change_Proc_Prio(pid_t PID,prio_t Priority)
{
    Sys_Lock_Scheduler();
    
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)                                              
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }
    
    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)                                              
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }  
    
    /* See if the process is ready */
    if(PCB[PID].Status.Sleep_Count==0)    
    {
        if(PCB[PID].Status.Priority!=Priority)
        {
            _Sys_Del_Proc_From_Cur_Prio(PID);
            _Sys_Ins_Proc_Into_New_Prio(PID,Priority);
            /* If this priority level is higher that what we are running at, then we 
             * need to perform a task switch at once.
             */
            if(Priority>Current_Prio)
                _Sys_Schedule_Trigger();
        }
    }
    /* For the processes that are not ready, modifying their PCB is enough */
    else
    {
        PCB[PID].Status.Priority=Priority;
    }
    
    Sys_Unlock_Scheduler();
    
    return 0;
}
/* End Function:_Sys_Change_Proc_Prio ****************************************/

/* Begin Function:_Sys_Get_High_Ready *****************************************
Description : The function for deciding the highest priority task.
              The routine has a very simple way to decide who's the final
              task to run at last-it will set the "Current_PID" and the 
              "Current_Prio" correspondingly, and this only the two variables
              will be used by the assembly function.
              There's no need to disable interrupts here; because this is always 
              at the lowest priority.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Get_High_Ready(void)
{		  
    /* See if the scheduler is locked .If yes, then we cannot switch the tasks */
    if(Scheduler_Locked==0)
    {  
        /* The system is booting from "Init" process. We need do abandon the SP automatically
         * saved by the "SYS_SAVE_SP" assembly function this time.
         */
        if(System_Status.Kernel.Boot_Done==FALSE)
        {
            PCB_Cur_SP[0]=PCB[0].Info.Init_Stack_Ptr-PRESET_STACK_SIZE;
            System_Status.Kernel.Boot_Done=TRUE;
        }
        
        /*If possible, decrease the remaining timeslice by 1.*/ 
        if(PCB[Current_PID].Time.Lft_Tim>0)
        {   
            PCB[Current_PID].Time.Lft_Tim-=1;                        
        } 
        else
        {
            /* Refresh the timer */
            PCB[Current_PID].Time.Lft_Tim=PCB[Current_PID].Time.Cur_Tim;  
            
            /* If the priority level still exists, then perform a normal context switch */
            if(Prio_List[Current_Prio].Proc_Num!=0)
            {
                /* Do normal context switch first - See if the current process is running */
                if(((struct PCB_Struct*)
                    (Prio_List[Current_Prio].Running_List.Next))
                   ->Info.PID==Current_PID)
                {
                    /* If the current process is running, then put it at the end of the queue */
                    Sys_List_Delete_Node(PCB[Current_PID].Head.Prev,PCB[Current_PID].Head.Next);
                    Sys_List_Insert_Node((struct List_Head*)&(PCB[Current_PID].Head),
                                         Prio_List[Current_Prio].Running_List.Prev, 
                                         (struct List_Head*)&(Prio_List[Current_Prio].Running_List));
                }
                else
                {
                    /* Do nothing - The current process is not running */
                }
            }
        } 
        
        /* Now get the highest priority level's next process to run */
        Current_Prio=((struct Prio_List_Struct*)(Prio_List_Head.Next))->Priority;
        Current_PID=((struct PCB_Struct*)
                     (Prio_List[Current_Prio].Running_List.Next))
                    ->Info.PID;
    }
    else
    {
        /* One of the scheduling action is pending due to scheduler lock. So 
         * after the scheduler unlocks, we will perform a schedule right away.
         */
        Pend_Sched_Cnt++;
    }
    
    /* Refresh the system status,which is only for query*/
    System_Status.Kernel.Proc_Running_Ptr=(struct List_Head*)(&(PCB[Current_PID].Head)); 
    System_Status.Kernel.Cur_Prio_Ptr=(struct List_Head*)(&(Prio_List[PCB[Current_PID].Status.Priority].Head));                 

    /* Process the signals here - The signal handlers will be directly called */
    _Sys_Signal_Handler(Current_PID);
} 
/* End Function:_Sys_Get_High_Ready ******************************************/

/* Begin Function:_Sys_Systick_Routine ****************************************
Description : The systick interrupt routine. No need to disable interrupts here, 
              because other interrupts may override it.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Systick_Routine(void)
{      
    /* Always increase the tick count by one */
    if(System_Status.Time.OS_Total_Ticks.Low_Bits==0xFFFFFFFF)
    {
        System_Status.Time.OS_Total_Ticks.Low_Bits=0;
        System_Status.Time.OS_Total_Ticks.High_Bits++;
    }
    else
        System_Status.Time.OS_Total_Ticks.Low_Bits++;
    
    /* Process the signals again here, because the process may receive signals from ISR */
    _Sys_Signal_Handler(Current_PID);
    
    /* Process the timers here - see if any of them is expired */
    _Sys_Timer_Handler(); 
    /* Process the system process delay here - see if any of them is expired */
    _Sys_Proc_Delay_Handler();
    /* Do a scheduling here */
    _Sys_Schedule_Trigger();
}
/* End Function:_Sys_Systick_Routine *****************************************/

/* Begin Function:Sys_Set_Ready ***********************************************
Description : Used in user program to set a process as ready.
Input       : pid_t PID - The PID of the process to set to the ready state.
Output      : None
Return      : retval_t - If successful, it will return 0; else -1.
******************************************************************************/
retval_t Sys_Set_Ready(pid_t PID)
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
    
    /* See if a state change is allowed */
    if(PCB[PID].Status.Status_Lock_Count>0)
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    return _Sys_Set_Ready(PID);
}
/* End Function:Sys_Set_Ready ************************************************/

/* Begin Function:_Sys_Set_Ready **********************************************
Description : Used in kernel to set a process as ready.
Input       : pid_t PID - The PID of the process to set to the ready state.
Output      : None
Return      : retval_t - If successful, it will return 0; else -1.
******************************************************************************/
retval_t _Sys_Set_Ready(pid_t PID)
{
    Sys_Lock_Scheduler();                                                        
    
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)                                              
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }
    
    /* The process's is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)                                              
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }  

    /* Check if the process is already ready. If no we will set it as ready. 
     * If it is ready, then there's no need to set it as ready again.
     * Remember don't set it ready in this function; the insert function will
     * set it as ready normally.
     */
    if(PCB[PID].Status.Sleep_Count==1)
    {     
        /* The priority argument will be from itself */                                                     
        _Sys_Ins_Proc_Into_New_Prio(PID,PCB[PID].Status.Priority);
        /* See if this process's priority is higher than what we are running at. If yes, 
         * schedule now.    
         */
        if(PCB[PID].Status.Priority>Current_Prio)
            _Sys_Schedule_Trigger();
        
        PCB[PID].Status.Sleep_Count=0;
    }   
    else
    {
        PCB[PID].Status.Sleep_Count--;
    }
    
    Sys_Unlock_Scheduler();
    
    return 0;
}
/* End Function:_Sys_Set_Ready ***********************************************/

/* Begin Function:Sys_Clr_Ready ***********************************************
Description : Used in user program to set a process as not ready.
Input       : pid_t PID -The PID of the process to clear the ready state.
Output      : None.
Return      : retval_t -If successful, it will return 0;else -1.
******************************************************************************/
retval_t Sys_Clr_Ready(pid_t PID)
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
    
    /* See if a state change is allowed */
    if(PCB[PID].Status.Status_Lock_Count>0)
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    return _Sys_Clr_Ready(PID);
}
/* End Function:Sys_Clr_Ready *************************************************/

/* Begin Function:_Sys_Clr_Ready **********************************************
Description : Used in kernel to set a process as not ready.
Input       : pid_t PID -The PID of the process to clear the ready state.
Output      : None.
Return      : retval_t -If successful, it will return 0;else -1.
******************************************************************************/
retval_t _Sys_Clr_Ready(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
    {        
        Sys_Set_Errno(ENOPID);
        return (-1);
    }
    
    Sys_Lock_Scheduler();                                                        

    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)                                              
    {        
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return (-1);
    }  

    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)                                              
    {        
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return (-1);
    }   

    /* Check if the process is not ready. 
     * If it is not ready, then there's no need to set it again.
     * The delete function will clear the ready flag automatically.
     */
    if(PCB[PID].Status.Sleep_Count==0)
    {     
        _Sys_Del_Proc_From_Cur_Prio(PID); 
        if(PID==Current_PID)
            _Sys_Schedule_Trigger();
        
        PCB[PID].Status.Sleep_Count=1;
    }             
    else
    {
        PCB[PID].Status.Sleep_Count++;
    }    
                                                                                    
    Sys_Unlock_Scheduler();
    return 0;
}
/* End Function:_Sys_Clr_Ready ************************************************/

/* Begin Function:Sys_Start_Proc **********************************************
Description : The dynamic process starter. The starter is designed to be called 
              from user applications. Call this function if you want to start a 
              process somewhere necessary.
              The process started is not ready unless you set it as ready manually.
Input       : struct Proc_Init_Struct* Process -The pointer to the struct containing 
              everything necessary. Take note that in this function the 
              "Process.PID" must be "AUTO_PID"(0).
Output      : None.
Return      : pid_t - The PID of the new process. If the function fails, then
                      the value will be -1.
******************************************************************************/
pid_t Sys_Start_Proc(struct Proc_Init_Struct* Process)	                   
{	
	pid_t New_PID;
    s32 Find_Flag=0;

    /* If the process ID in the struct is not 0, then break for safety */
	if((Process->PID)!=AUTO_PID)
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }    
    
	Sys_Lock_Scheduler();

	/* We start searching valid PID from 2. 0 is "Init", 1 is "Arch" */
	for(New_PID=2;New_PID<MAX_PROC_NUM;New_PID++) 
    {   /* Find An Unoccupied PID */                 
		if((PCB[New_PID].Status.Running_Status&OCCUPY)==0)	                              
		{
			Find_Flag=1;
			break;                                                             
		}
    }

	if(Find_Flag==0)
	{   
        Sys_Set_Errno(ENOPID);
		Sys_Unlock_Scheduler();	                                                  
		return(-1);
	}
    
    /* Fill process information */    
    PCB[New_PID].Info.Name=Process->Name;
    PCB[New_PID].Info.PID=New_PID;
    
    if(Process->Parent_Policy==PARENT_CARE)
    {
        PCB[New_PID].Info.PPID=Current_PID;
    }
    else
    {
        PCB[New_PID].Info.PPID=0;
    }
                                                      
    /* Fill the task pointers */
    PCB[New_PID].Info.Init_Addr_Ptr=(ptr_int_t)(Process->Entrance);	
    PCB[New_PID].Info.Init_Stack_Ptr=(ptr_int_t)(Process->Stack_Address+Process->Stack_Size-4-STACK_SAFE_RDCY);
    
    /* Set the initial stack correctly */
    PCB_Cur_SP[New_PID]=PCB[New_PID].Info.Init_Stack_Ptr;

    /* Indicates that this PID is in use */								             
	PCB[New_PID].Status.Running_Status=OCCUPY;                                                          
	
    PCB[New_PID].Status.Priority=Process->Priority;    
    	
    PCB[New_PID].Time.Min_Tim=Process->Min_Slices;                                                   
	PCB[New_PID].Time.Max_Tim=Process->Max_Slices;                                                  
	PCB[New_PID].Time.Cur_Tim=Process->Cur_Slices; 											
                                    
    /* Initialize the new task's stack */
	_Sys_Proc_Stack_Init(New_PID);                                      
	
    /* See if the new process should be ready */
    if(Process->Ready_Flag==READY)
        _Sys_Set_Ready(New_PID);
    
    /* Refresh the system statistical variable */
    System_Status.Proc.Total_Proc_Number++;
    
	Sys_Unlock_Scheduler();	                                                       
    /* Return the PID of the new process */
	return(New_PID);                                                          
}
/* End Function:Sys_Start_Proc ***********************************************/

/* Begin Function:Sys_Require_Timeslice ***************************************
Description : Require timeslice from the system.
Input       : size_t Slices -The number of slices of time that it want to set itself to.
Output      : None.
Return      : retval_t -If successful, it will return 0;else -1.
******************************************************************************/
retval_t Sys_Require_Timeslice(size_t Slices)
{
    if((Slices>PCB[Current_PID].Time.Max_Tim)||(Slices<PCB[Current_PID].Time.Min_Tim))
    {
        Sys_Set_Errno(ENOTIM);
        return(-1);
    }
    
    PCB[Current_PID].Time.Cur_Tim=Slices;
    return 0;
}
/* End Function:Sys_Require_Timeslice ****************************************/

/* Begin Function:_Sys_Require_Timeslice **************************************
Description : Require timeslice from the system, in the name of a certain process.
Input       : pid_t PID - The PID of the process.
              size_t Slices -The number of slices of time that it want to set itself to.
Output      : None.
Return      : retval_t - If successful,0; else -1;
******************************************************************************/
retval_t _Sys_Require_Timeslice(pid_t PID,size_t Slices)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
    {
        Sys_Set_Errno(ENOPID);
        return(-1);
    }
    
    Sys_Lock_Scheduler();
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)                                              
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }
    
    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)                                              
    {
        Sys_Set_Errno(ENOPID);
        Sys_Unlock_Scheduler();
        return(-1);
    }  
    
    if((Slices>PCB[PID].Time.Max_Tim)||(Slices<PCB[PID].Time.Min_Tim))
    {    
        Sys_Set_Errno(ENOTIM);
        Sys_Unlock_Scheduler();
        return(-1);
    }
    
    PCB[PID].Time.Cur_Tim=Slices;
    Sys_Unlock_Scheduler();
    return 0;
}
/* End Function:_Sys_Require_Timeslice ***************************************/

/* Begin Function:Sys_Query_Timeslice *****************************************
Description : Query how many slices the process is taking. If the process does not
              exist or not running, it will return 0.
Input       : pid_t PID - The PID of the process.
Output      : None.
Return      : size_t - The silce the process is occupying.
******************************************************************************/
size_t Sys_Query_Timeslice(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
        return(0);
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)   
        return(0);
    
    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)       
        return(0);
    
    return PCB[PID].Time.Cur_Tim;
}
/* End Function:Sys_Query_Timeslice ******************************************/

/* Begin Function:Sys_Query_Running_Status ************************************
Description : Query the running status of a certain process.
Input       : pid_t PID - The PID of the process.
Output      : None.
Return      : retval_t - The running status of it.
******************************************************************************/
retval_t Sys_Query_Running_Status(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
        return(0);
    
    return PCB[PID].Status.Running_Status;
}
/* End Function:Sys_Query_Running_Status *************************************/

/* Begin Function:Sys_Query_Proc_Prio *****************************************
Description : Query the process priority. 
Input       : None
Output      : None.
Return      : prio_t - The process's current priority.If the process does not exist,
              we return 0.
******************************************************************************/
prio_t Sys_Query_Proc_Prio(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
        return(0);
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)   
        return(0);
    
    /* The process is a zombie */    
    if((PCB[PID].Status.Running_Status&ZOMBIE)!=0)       
        return(0);
    
    return PCB[PID].Status.Priority;
}
/* End Function:Sys_Query_Proc_Prio ******************************************/

/* Begin Function:Sys_Query_Current_Prio **************************************
Description : Query the current running priority level.
Input       : None
Output      : None.
Return      : prio_t - The current priority.
******************************************************************************/
prio_t Sys_Query_Current_Prio(void)
{
    return Current_Prio;
}
/* End Function:Sys_Query_Current_Prio ***************************************/

/* Begin Function:Sys_Query_Sleep_Count ***************************************
Description : Query the sleep count of a certain process.
Input       : pid_t PID - The PID of the process.
Output      : None.
Return      : cnt_t - The sleep count of it. If the process does not exist, then 
                      we return -1.
******************************************************************************/
cnt_t Sys_Query_Sleep_Count(pid_t PID)
{
    /* The PID will never be valid in the system */
    if(PID>MAX_PROC_NUM)     
        return(-1);
    
    /* The process does not exist in the system */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)   
        return(-1);
    
    return PCB[PID].Status.Sleep_Count;
}
/* End Function:Sys_Query_Sleep_Count ****************************************/

/* Begin Function:Sys_Switch_Now **********************************************
Description : The process-level process switch function. The process will yield all
              its timeslices and get the next process to run. The current timeslice
              that has not been used up will be used by the next process, so that the
              next process will at most run for n+1 timeslices(here we assume that the
              timeslice we assigned to it is n.)
              No need to enable and disable interrupt here. It will be automatically 
              enabled&disabled inside.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Switch_Now(void)
{             
    /* Set the time left to zero */    
    PCB[Current_PID].Time.Lft_Tim=0;                              
    /* Now trigger a PendSV to perform the context switch. 
     * PendSV has the lowset interrupt priority. 
     */                 
   _Sys_Schedule_Trigger();
}
/* End Function:Sys_Switch_Now ***********************************************/

/* Begin Function:Sys_Get_PID *************************************************
Description : Get current PID.
Input       : None.
Output      : None.
Return      : pid_t -The current PID.
******************************************************************************/
pid_t Sys_Get_PID(void)
{
	return(Current_PID);
}
/* End Function:Sys_Get_PID **************************************************/

/* Begin Function:main ********************************************************
Description : The entrance of the OS. will be called by the assembly code.
              Don't lock interrupt in the start sequence; because the interrupt 
              nesting count will be totally cleared in "_Sys_Int_Init". Doing
              so will cause a deadlock.
Input       : None.
Output      : None.
Return      : int - Dummy return value. In fact, the function never returns.
******************************************************************************/
int main(void)		                                                    
{	
    /* Initialize the memory managing unit */
    _Sys_Mem_Init();
    
    /* Initialize the semaphore managing unit */
    _Sys_Sem_Init();
    
    /* Initialize the mutex managing unit */
    _Sys_Mutex_Init();
    
    /* Initialize the (named) pipe unit */
    _Sys_Pipe_Init();
    
    /* Initialize the shared memory module */
    _Sys_Shm_Init();
    
    /* Initialize the message queue */
    _Sys_Queue_Init();
    
    /* Initialize the system timer */
    _Sys_Timer_Init();
    
    /* Initialize the system interrupt module */
    _Sys_Int_Init();
    
    /* Initialize the scheduler, including the signal part */
    _Sys_Scheduler_Init();
    
    /* Load the first process -- "Init" */
    _Sys_Load_Init();   
    
    /* Should never reach here */                                                   
	return(0);											                  
}
/* End Function:main *********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved **************/
