/******************************************************************************
Filename    : timer.c
Author      : pry
Date        : 03/07/2013
Version     : 0.02
Description : The timer module for the OS. This is a software-timer, so it is 
              not very accurate. This timer is realized in a ordered-list data
              structure (a compromize between space and efficiency) featuring:
              O(n) Create timer
              O(1) Keeping timer
              O(1) Destroying timer
              O(n) Starting timer
              O(1) Stopping timer
              O(1) Query on timer variables
              O(n) Reloading timer(not in systick interrupt routine; "Arch" is
                                   responsible for this)
              Take note that the timer only allows 32-bit (time_t) delay time.
              However, the total ticks is calculated in a 64-bit struct.
              Thus, the time compared can be very long (0.3 billion years' time
              span, 2^64 ms).
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syssvc\timer.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Syssvc\timer.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Syssvc\timer.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#include "Syssvc\timer.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:Sys_Tick_Time_Comp ******************************************
Description : Decide which tick value is bigger.
Input       : struct Tick_Time* Tick_1 - One of the tick values to compare.
              struct Tick_Time* Tick_2 - One of the tick values to compare.
Output      : None.
Return      : retval_t - If Tick_1 is greater than Tick_2, then return 1; if equal,0; if
                         smaller, then -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)	
retval_t Sys_Tick_Time_Comp(struct Tick_Time* Tick_1,struct Tick_Time* Tick_2)									   
{
    if(Tick_1->High_Bits>Tick_2->High_Bits)
        return 1;
    else if((Tick_1->High_Bits)==(Tick_2->High_Bits))
    {
        if(Tick_1->Low_Bits>Tick_2->Low_Bits)
            return 1;
        else if((Tick_1->Low_Bits)==(Tick_2->Low_Bits))
            return 0;
        else
            return -1;
    }  
    else
        return -1;
}
#endif
/* End Function:Sys_Tick_Time_Comp *******************************************/

/* Begin Function:Sys_Tick_Time_Add *******************************************
Description : Add the two tick values and give out the final result.
Input       : struct Tick_Time* Tick_1 - One of the tick value addends.
              struct Tick_Time* Tick_2 - One of the tick value addends.
Output      : struct Tick_Time* Result - The final result.
Return      : retval_t - If an overflow happens,-1; if totally success,0.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)	
retval_t Sys_Tick_Time_Add(struct Tick_Time* Tick_1,struct Tick_Time* Tick_2,struct Tick_Time* Result)									   
{
    time_t Carry_Flag;
    struct Tick_Time Buffer;
    /* Add the lower bytes together first. See if an overflow will happen. If it happens
     * we need to carry manually.
     */
    Buffer.Low_Bits=(Tick_1->Low_Bits)+(Tick_2->Low_Bits);
    if(Buffer.Low_Bits<Tick_2->Low_Bits)
        Carry_Flag=1;
    else
        Carry_Flag=0;
    
    
    
    /* See if the high bits will overflow. Don't care about the carry now */
    Buffer.High_Bits=Tick_1->High_Bits+Tick_2->High_Bits;
    if(Buffer.High_Bits<Tick_2->High_Bits)
    {
        /* The overflow must have happened */
        Buffer.High_Bits+=Carry_Flag;
        Carry_Flag=1;
    }
    else if((Buffer.High_Bits==0xFFFFFFFF)&&(Carry_Flag==1))
    {
        /* Now, after the carry, they will overflow */
        Buffer.High_Bits+=Carry_Flag;
        Carry_Flag=1;
    }
    else
    {
        /* No overflow will happen */
        Buffer.High_Bits+=Carry_Flag;
        Carry_Flag=0;
    }
    
    Result->High_Bits=Buffer.High_Bits;
    Result->Low_Bits=Buffer.Low_Bits;
    
    if(Carry_Flag==1)
        return -1;
    else
        return 0;
}
#endif
/* End Function:Sys_Tick_Time_Add ********************************************/

/* Begin Function:Sys_Tick_Time_Minus *****************************************
Description : Minus the two tick values and give out the final result.
Input       : struct Tick_Time* Tick_1 - The minuend.
              struct Tick_Time* Tick_2 - The subtrahend.
Output      : struct Tick_Time* Result - The final result.
Return      : retval_t - If an overflow happens,-1; if totally success,0.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)	
retval_t Sys_Tick_Time_Minus(struct Tick_Time* Tick_1,struct Tick_Time* Tick_2,struct Tick_Time* Result)									   
{
    time_t Carry_Flag;
    struct Tick_Time Buffer;
    /* Minus the lower bytes together first. See if an overflow will happen. If it happens
     * we need to carry manually.
     */
    Buffer.Low_Bits=(Tick_1->Low_Bits)-(Tick_2->Low_Bits);
    if(Buffer.Low_Bits>(Tick_1->Low_Bits))
        Carry_Flag=1;
    else
        Carry_Flag=0;
    
    
    Buffer.High_Bits=(Tick_1->High_Bits)-(Tick_2->High_Bits);
    /* See if the high bits will overflow */
    if(Buffer.High_Bits>(Tick_1->High_Bits))
    {
        /* The overflow must have happened */
        Buffer.High_Bits-=Carry_Flag;
        Carry_Flag=1;
    }
    else if((Buffer.High_Bits==0)&&(Carry_Flag==1))
    {
        /* The overflow will happen after the carry */
        Buffer.High_Bits-=Carry_Flag;
        Carry_Flag=1;
    }
    else
    {
        /* No overflow will happen */
        Buffer.High_Bits+=Carry_Flag;
        Carry_Flag=0;
    }
    
    Result->High_Bits=Buffer.High_Bits;
    Result->Low_Bits=Buffer.Low_Bits;
    
    if(Carry_Flag==1)
        return -1;
    else
        return 0;
}
#endif
/* End Function:Sys_Tick_Time_Add ********************************************/

/* Begin Function:_Sys_Timer_Init *********************************************
Description : Initialize the system timer module.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Timer_Init(void)									   
{
#if(ENABLE_TIMER==TRUE)	
   cnt_t Tim_Cnt;
    
   /* Initialize the lists */
   Sys_Create_List(&Tim_List_Head);
   Sys_Create_List(&Tim_Empty_List_Head);
    
   Sys_Create_List(&Tim_Running_List_Head);
   Sys_Create_List(&Tim_Stop_List_Head);
   Sys_Create_List(&Tim_Reload_List_Head);
    
   Sys_Create_List(&Proc_Delay_List_Head);
    
   /* Clear the control block */
   Sys_Memset((ptr_int_t)Tim_CB,0,MAX_TIMS*sizeof(struct Timer));
   Sys_Memset((ptr_int_t)PCB_Proc_Delay,0,MAX_PROC_NUM*sizeof(struct PCB_Timer));
    
   /* Put the blocks into the empty list */
   for(Tim_Cnt=0;Tim_Cnt<MAX_TIMS;Tim_Cnt++)
   {
       Tim_CB[Tim_Cnt].Timer_ID=Tim_Cnt;
       Sys_List_Insert_Node(&(Tim_CB[Tim_Cnt].Head),&Tim_Empty_List_Head,Tim_Empty_List_Head.Next);
   }
   
   /* Put the blocks into the empty list */
   for(Tim_Cnt=0;Tim_Cnt<MAX_PROC_NUM;Tim_Cnt++)
   {
       PCB_Proc_Delay[Tim_Cnt].PID=Tim_Cnt;
   }
   
   /* Clear the statistical variable */
   Timer_In_Sys=0;
#endif
}
/* End Function:_Sys_Timer_Init **********************************************/

/* Begin Function:_Sys_Timer_Handler ******************************************
Description : The system timer handler.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Timer_Handler(void)									   
{
#if(ENABLE_TIMER==TRUE)	
   struct List_Head* Traverse_List_Ptr;
   timid_t Timer_ID=0;
    
   /* See if there are timers in the list */
   if(Tim_Running_List_Head.Next==&Tim_Running_List_Head)  
        return;
   /* See if the time has come. If yes, process the timer list and execute
    * the corresponding timer expire function.
    */
    if(Sys_Tick_Time_Comp(&((struct Timer*)(Tim_Running_List_Head.Next-1))->Timer_End_Total_Ticks,
                          (struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks))==1)
        return;
    
    /* Traverse the list and find out all the timers that need to be processed.
     * We don't process them here directly; we just put them into the list of being
     * processed.
     */
    Traverse_List_Ptr=Tim_Running_List_Head.Next;
    while((Sys_Tick_Time_Comp(&(((struct Timer*)(Traverse_List_Ptr-1))->Timer_End_Total_Ticks),
                              (struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks))!=-1)&&
          ((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Tim_Running_List_Head)))
    {   
        Timer_ID=((struct Timer*)Traverse_List_Ptr)->Timer_ID;
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
        
        Sys_List_Delete_Node(Tim_CB[Timer_ID].Head.Prev,Tim_CB[Timer_ID].Head.Next);
        Tim_CB[Timer_ID].Timer_Status=TIM_RELOAD;
        Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Head),&Tim_Reload_List_Head,Tim_Reload_List_Head.Next);
    }
#endif
}

/* End Function:_Sys_Timer_Handler *******************************************/

/* Begin Function:_Sys_Timer_Reload *******************************************
Description : The system timer reloader. Called by the "Arch" process to 
              reload the system timers and call the handler functions.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)	
void _Sys_Timer_Reload(void)									   
{
   struct List_Head* Traverse_Reload_List_Ptr;
   struct List_Head* Traverse_Running_List_Ptr;
   timid_t Timer_ID;
   void (*Timer_Handler)(void);
    
    /* Traverse the list and process all the expired timers */
    Traverse_Reload_List_Ptr=Tim_Reload_List_Head.Next;
    
    while((ptr_int_t)Traverse_Reload_List_Ptr!=(ptr_int_t)(&Tim_Reload_List_Head))
    {   
        /* We lock scheduler here */
        Sys_Lock_Scheduler();
        
        /* We may only the timer ID later on */
        Timer_ID=((struct Timer*)(Traverse_Reload_List_Ptr-1))->Timer_ID;
        
        Traverse_Reload_List_Ptr=Traverse_Reload_List_Ptr->Next;
        
        /* Execute the expire function, if possible */
        Timer_Handler=(void(*)(void))(Tim_CB[Timer_ID].Timer_Expire_Handler);
        if(Timer_Handler!=0)
            Timer_Handler();
        
        Sys_List_Delete_Node(Tim_CB[Timer_ID].Status_List_Head.Prev,Tim_CB[Timer_ID].Status_List_Head.Next);
        
        /* See if the timer need a reload. If yes, reload it and put it in the running
         * list. This time we don't check if the name is unique, because we have ensured
         * this when we create the timer.
         */
        if(Tim_CB[Timer_ID].Timer_Mode==TIM_AUTORELOAD)
        {
            /* We need to find a proper time for insertion */
            Traverse_Running_List_Ptr=Tim_Running_List_Head.Next;
            
            Tim_CB[Timer_ID].Timer_Start_Total_Ticks.High_Bits=System_Status.Time.OS_Total_Ticks.High_Bits;
            Tim_CB[Timer_ID].Timer_Start_Total_Ticks.Low_Bits=System_Status.Time.OS_Total_Ticks.Low_Bits;
            
            Sys_Tick_Time_Add((struct Tick_Time*)&System_Status.Time.OS_Total_Ticks,
                              &Tim_CB[Timer_ID].Timer_Max_Delay_Ticks,
                              &Tim_CB[Timer_ID].Timer_End_Total_Ticks);
            
            Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.High_Bits=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits;
            Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.Low_Bits=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits;
            
            while((ptr_int_t)Traverse_Running_List_Ptr!=(ptr_int_t)(&Tim_Running_List_Head))
            {
                /* Find a place */
                if(Sys_Tick_Time_Comp(&Tim_CB[Timer_ID].Timer_End_Total_Ticks,
                                      &((struct Timer*)(Traverse_Reload_List_Ptr-1))->Timer_End_Total_Ticks)!=1)
                    break;
                
                Traverse_Running_List_Ptr=Traverse_Running_List_Ptr->Next;
            }
            
            Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Status_List_Head),
                                 Traverse_Running_List_Ptr->Prev,
                                 Traverse_Running_List_Ptr);
            
            /* Mark it as running, and refresh it now */
            Tim_CB[Timer_ID].Timer_Status=TIM_RUN;
        }
        /* If no, refresh it and put it in the stop list */
        else if(Tim_CB[Timer_ID].Timer_Mode==TIM_MANUAL)
        {
             Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Status_List_Head),
                                  &Tim_Stop_List_Head,
                                  Tim_Stop_List_Head.Next);
            
            /* Mark it as running, and refresh it now */
            Tim_CB[Timer_ID].Timer_Status=TIM_STOP;
            Tim_CB[Timer_ID].Timer_Start_Total_Ticks.High_Bits=0;
            Tim_CB[Timer_ID].Timer_Start_Total_Ticks.Low_Bits=0;
            Tim_CB[Timer_ID].Timer_End_Total_Ticks.High_Bits=0;
            Tim_CB[Timer_ID].Timer_End_Total_Ticks.Low_Bits=0;
            Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.High_Bits=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits;
            Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.Low_Bits=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits;
        }
        
        Sys_Unlock_Scheduler();
    }
}
#endif
/* End Function:_Sys_Timer_Reload ********************************************/

/* Begin Function:Sys_Timer_Create ********************************************
Description : Create a timer in the system.
Input       : s8* Timer_Name - The name of the system timer.
              time_t Timer_Delay - The timeslice that you want to delay.
Output      : None.
Return      : timid_t - The ID of the system timer. If the function fail,
                        the value will be -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
timid_t Sys_Timer_Create(s8* Timer_Name,time_t Timer_Delay)									   
{	
    timid_t Timer_ID;
    struct List_Head* Traverse_List_Ptr;
    /* See if the name is an empty pointer, or the timer delay is 0 */
    if((Timer_Name==0)||(Timer_Delay==0))
    {
        Sys_Set_Errno(EINVTIM);
        return -1;
    }
    
    Sys_Lock_Scheduler();
    
    /* See if there are any empty blocks */
    if((ptr_int_t)(&Tim_Empty_List_Head)==(ptr_int_t)(Tim_Empty_List_Head.Next))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOETIM);
        return -1;
    }
    
    /* If there are, get the timer ID. We may only use this later */
    Timer_ID=((struct Timer*)Tim_Empty_List_Head.Next)->Timer_ID;
    
    /* Traverse the list to see if the name is unique. By the way, we need to find a 
     * place to insert the timer into the timer queue, according to the order from 
     * urgent to non-urgent.  */
    Traverse_List_Ptr=Tim_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Tim_List_Head))
    {
        /* If we can find a match, abort */
        if(Sys_Strcmp(Timer_Name,((struct Timer*)Traverse_List_Ptr)->Timer_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(ETIMEXIST);
            return -1;
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    
    /* Now we are sure that we can set up the timer. However, we don't insert it into the 
     * running list now because it is not started yet. Instead, we insert it into the stop
     * list.
     */
    Sys_List_Delete_Node(Tim_CB[Timer_ID].Head.Prev,Tim_CB[Timer_ID].Head.Next);
    Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Head),&Tim_List_Head,Tim_List_Head.Next);
    Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Status_List_Head),&Tim_Stop_List_Head,Tim_Stop_List_Head.Next);
    
    /* Register the values */
    Tim_CB[Timer_ID].Timer_Name=Timer_Name;
    /* 0 Indicates that it has not been set up yet */
    Tim_CB[Timer_ID].Timer_Start_Total_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_Start_Total_Ticks.Low_Bits=0;
    
    Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits=Timer_Delay;
    
    Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.Low_Bits=Timer_Delay;
    
    Tim_CB[Timer_ID].Timer_End_Total_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_End_Total_Ticks.Low_Bits=0;
    
    /* The default config is as follows */
    Tim_CB[Timer_ID].Timer_Mode=TIM_MANUAL;
    Tim_CB[Timer_ID].Timer_Status=TIM_STOP;
    
    /* Update the statistical variable */
    Timer_In_Sys++;
    
    Sys_Unlock_Scheduler();
    return Timer_ID;
}
#endif
/* End Function:Sys_Timer_Create *********************************************/

/* Begin Function:Sys_Timer_Destroy *******************************************
Description : Destroy a timer regardless of its status. This is different from 
              most similiar functions. If the timer exists, then it will be 
              deleted unconditionally.
Input       : timid_t Timer_ID - The system timer's ID.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
retval_t Sys_Timer_Destroy(timid_t Timer_ID)									   
{	
    /* See if the shared memory ID is over the boundary */
    if(Timer_ID>=MAX_TIMS)
    {
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the timer exists in the system */
    if((Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits==0)&&
       (Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* Now we can delete the timer. When the timer exists in the system we can be sure that it
     * must be in one of the "running,stop,reload" lists. Delete it from there also.
     */
    Sys_List_Delete_Node(Tim_CB[Timer_ID].Head.Prev,Tim_CB[Timer_ID].Head.Next);
    Sys_List_Delete_Node(Tim_CB[Timer_ID].Status_List_Head.Prev,Tim_CB[Timer_ID].Status_List_Head.Next);
    
    /* Insert it into the empty list */
    Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Head),&Tim_Empty_List_Head,Tim_Empty_List_Head.Next);
    
    /* Clear the variables */
    /* Register the values */
    Tim_CB[Timer_ID].Timer_Name=0;
    /* 0 Indicates that it has not been set up yet */
    Tim_CB[Timer_ID].Timer_Start_Total_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_Start_Total_Ticks.Low_Bits=0;
    
    Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits=0;
    
    Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.Low_Bits=0;
    
    Tim_CB[Timer_ID].Timer_End_Total_Ticks.High_Bits=0;
    Tim_CB[Timer_ID].Timer_End_Total_Ticks.Low_Bits=0;
    
    Tim_CB[Timer_ID].Timer_Expire_Handler=0;
    
    /* The default config is as follows */
    Tim_CB[Timer_ID].Timer_Mode=TIM_MANUAL;
    Tim_CB[Timer_ID].Timer_Status=TIM_STOP;
    
    /* Update statistical variable */
    Timer_In_Sys--;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Timer_Destroy ********************************************/

/* Begin Function:Sys_Timer_Get_ID ********************************************
Description : Get the timer ID from its name.
Input       : s8* Timer_Name - The name of the timer. 
Output      : None.
Return      : timid_t - The ID of the system timer.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
timid_t Sys_Timer_Get_ID(s8* Timer_Name)							   
{	
     struct List_Head* Traverse_List_Ptr;
    
    Sys_Lock_Scheduler();
    /* Traverse the list to see if the name exists */
    Traverse_List_Ptr=Tim_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Tim_List_Head))
    {
        /* If we can find a match, abort */
        if(Sys_Strcmp(Timer_Name,((struct Timer*)Traverse_List_Ptr)->Timer_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            return (((struct Timer*)Traverse_List_Ptr)->Timer_ID);
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    /* If it gets here, then no match is found */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOTIM);
    return (-1);
}
#endif
/* End Function:Sys_Timer_Get_ID *********************************************/

/* Begin Function:Sys_Timer_Start *********************************************
Description : Start a system timer.
Input       : timid_t Timer_ID - The ID of the system timer.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
retval_t Sys_Timer_Start(timid_t Timer_ID)
{	
    struct List_Head* Traverse_List_Ptr;
    /* See if the timer ID is over the boundary */
    if(Timer_ID>=MAX_TIMS)
    {
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the timer exists in the system */
    if((Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits==0)&&
       (Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* See if the timer is stopped. If no, we cannot set it running again */
    if((Tim_CB[Timer_ID].Timer_Status!=TIM_STOP)&&
       (Tim_CB[Timer_ID].Timer_Status!=TIM_RELOAD_STOP))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* The now insert it into the list. We need to see the former status to decide the latter status:
     * TIM_STOP-->TIM_RUN,TIM_RELOAD_STOP-->TIM_RELOAD
     */
    Sys_List_Delete_Node(Tim_CB[Timer_ID].Status_List_Head.Prev,Tim_CB[Timer_ID].Status_List_Head.Next);
    if(Tim_CB[Timer_ID].Timer_Status==TIM_STOP)
    {
        /* Fill in the time variables first */
        Tim_CB[Timer_ID].Timer_Start_Total_Ticks.High_Bits=System_Status.Time.OS_Total_Ticks.High_Bits;
        Tim_CB[Timer_ID].Timer_Start_Total_Ticks.Low_Bits=System_Status.Time.OS_Total_Ticks.Low_Bits;
        
        Sys_Tick_Time_Add((struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks),
                           &Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks,
                           &Tim_CB[Timer_ID].Timer_End_Total_Ticks);
        
        /* We need to find a proper time for insertion */
        Traverse_List_Ptr=Tim_Running_List_Head.Next;
        while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Tim_Running_List_Head))
        {
            /* Find a place */
            if(Sys_Tick_Time_Comp(&Tim_CB[Timer_ID].Timer_End_Total_Ticks,
                                  &((struct Timer*)(Traverse_List_Ptr-1))->Timer_End_Total_Ticks)!=1)
                break;
            
            Traverse_List_Ptr=Traverse_List_Ptr->Next;
        }
    
        /* Was stop status before. Now put it into the running list */
        Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Status_List_Head),Traverse_List_Ptr->Prev,Traverse_List_Ptr);
    
        /* Refresh its variables, let it run */
        Tim_CB[Timer_ID].Timer_Status=TIM_RUN;
        
    }
    /* Was waiting to be reloaded before stopped */
    else
    {
        /* Insert it into the waiting-to-be-reloaded list, wait for "Arch" to reload it */
        Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Status_List_Head),&Tim_Reload_List_Head,Tim_Reload_List_Head.Next);
        Tim_CB[Timer_ID].Timer_Status=TIM_RELOAD;
    }
    
    Sys_Unlock_Scheduler();
	return 0;
}
#endif
/* End Function:Sys_Timer_Start **********************************************/

/* Begin Function:Sys_Timer_Stop **********************************************
Description : Stop a system timer.
Input       : timid_t Timer_ID - The identifier of the timer.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
retval_t Sys_Timer_Stop(timid_t Timer_ID)
{
    /* See if the timer ID is over the boundary */
    if(Timer_ID>=MAX_TIMS)
    {
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the timer exists in the system */
    if((Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits==0)&&
       (Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* See if the timer is stopped already. If so, we cannot stop it again */
    if((Tim_CB[Timer_ID].Timer_Status!=TIM_RUN)&&(Tim_CB[Timer_ID].Timer_Status!=TIM_RELOAD))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* Now we are sure that we can stop the timer. See its former status to decide what 
     * stop status we should assign to it.
     * TIM_RUN-->TIM_STOP,TIM_RELOAD-->TIM_RELOAD_STOP
     */
    Sys_List_Delete_Node(Tim_CB[Timer_ID].Status_List_Head.Prev,Tim_CB[Timer_ID].Status_List_Head.Next);
    
    if(Tim_CB[Timer_ID].Timer_Status==TIM_RUN)
    {
        Tim_CB[Timer_ID].Timer_Status=TIM_STOP;
        /* Calculate the remaining time. We stop the timer now */
        Sys_Tick_Time_Minus(&Tim_CB[Timer_ID].Timer_End_Total_Ticks,
                            (struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks),
                            &Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks);
    }
    /* Is waiting to be reloaded */
    else
        Tim_CB[Timer_ID].Timer_Status=TIM_RELOAD_STOP;
    
    Sys_List_Insert_Node(&(Tim_CB[Timer_ID].Status_List_Head),&Tim_Stop_List_Head,Tim_Stop_List_Head.Next);
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Timer_Stop ***********************************************/

/* Begin Function:Sys_Timer_Set_Mode ******************************************
Description : Query the number of timers existing in the system.
Input       : timid_t Timer_ID - The timer ID.
              s32 Mode - The mode of the timer.
Output      : None.
Return      : retval_t - If successful,0;else -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
retval_t Sys_Timer_Set_Mode(timid_t Timer_ID,s32 Mode)
{
    /* See if the timer ID is over the boundary */
    if(Timer_ID>=MAX_TIMS)
    {
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the timer exists in the system */
    if((Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits==0)&&
       (Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* Now change its mode */
    Tim_CB[Timer_ID].Timer_Mode=Mode;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Timer_Set_Mode *******************************************/

/* Begin Function:Sys_Timer_Get_Time_Value ************************************
Description : Get the value of the system timer.
Input       : timid_t Timer_ID  - The identifier of the timer.
Output      : struct Tick_Time* Cur_Time - The current remaining time of the timer.
              struct Tick_Time* Max_Time - The maximum counting capacity of the timer.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
retval_t Sys_Timer_Get_Time_Value(timid_t Timer_ID,struct Tick_Time* Cur_Time,struct Tick_Time* Max_Time)
{
    /* See if the timer ID is over the boundary */
    if(Timer_ID>=MAX_TIMS)
    {
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the timer exists in the system */
    if((Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits==0)&&
       (Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* Now get its value. See which status it is in */
    if(Cur_Time!=0)
    {
        if(Tim_CB[Timer_ID].Timer_Status==TIM_RUN)
        {
            Sys_Tick_Time_Minus(&Tim_CB[Timer_ID].Timer_End_Total_Ticks,
                                (struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks),
                                Cur_Time);
        }
        else if(Tim_CB[Timer_ID].Timer_Status==TIM_STOP)
        {
            Cur_Time->High_Bits=Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.High_Bits;
            Cur_Time->Low_Bits=Tim_CB[Timer_ID].Timer_Cur_Delay_Ticks.Low_Bits;
        }
        else 
        {
            Cur_Time->High_Bits=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits;
            Cur_Time->Low_Bits=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits;
        }
    }
        
    if(Max_Time!=0)
        *Max_Time=Tim_CB[Timer_ID].Timer_Max_Delay_Ticks;
    
    Sys_Lock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Timer_Get_Time_Value *************************************/

/* Begin Function:Sys_Query_Timer_Number **************************************
Description : Query the number of timers existing in the system.
Input       : None.
Output      : None.
Return      : size_t - The number of timers in the system.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
size_t Sys_Query_Timer_Number(void)
{
    return Timer_In_Sys;
}
#endif
/* End Function:Sys_Query_Timer_Number ***************************************/

/* Begin Function:Sys_Timer_Handler_Reg ***************************************
Description : Register the timer overflow handler.
Input       : timid_t Timer_ID - The identifier of the timer.
              void(*Timer_Handler)(void) - The timer expire handler. Must be a
              function with neither a return value or a parameter.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
retval_t Sys_Timer_Handler_Reg(timid_t Timer_ID,void(*Timer_Handler)(void),u32 Handler_Arg)
{
    /* See if the timer ID is over the boundary */
    if(Timer_ID>=MAX_TIMS)
    {
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the timer exists in the system */
    if((Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.High_Bits==0)&&
       (Tim_CB[Timer_ID].Timer_Max_Delay_Ticks.Low_Bits==0))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVTIM);
        return (-1);
    }
    
    /* Register the handler and its arguments */
    Tim_CB[Timer_ID].Timer_Expire_Handler=(ptr_int_t)Timer_Handler;
    Tim_CB[Timer_ID].Handler_Arg=Handler_Arg;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Timer_Handler_Reg ****************************************/

/* Begin Function:Sys_Raw_Delay ***********************************************
Description : Old-style delay function. Wastes the CPU cycle.
Input       : time_t Time - The software delay time.
Output      : None. 
Return      : None.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
void Sys_Raw_Delay(time_t Time)
{
	time_t Time_X;
	time_t Time_Y;
    
	for(Time_X=Time;Time_X>0;Time_X--)
		for(Time_Y=DELAYTRIM;Time_Y>0;Time_Y--);
}
#endif
/* End Function:Sys_Raw_Delay ************************************************/

/* Begin Function:Sys_Proc_Delay_Tick *****************************************
Description : The delay function which delays a certain CPU cycle. This is not a
              timer(Or it can be treated as a light-weight timer).
              When this kind of delay takes effect, the process will not be moved
              out of the execution queue(this means that the process will still be ready).
              This function only supports 32-bit delay time(the unit is ticks).
              When the time is set to maximum, the delaytime can be as long as
              46 days, when the systick interrupt frequency is 1KHz.
              The delay time unit is ticks.
Input       : time_t Time - The software delay time.
Output      : None. 
Return      : None.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
void Sys_Proc_Delay_Tick(time_t Time)
{
    struct Tick_Time Stop_Time;
    struct List_Head* Traverse_List_Ptr;
    /* If time is zero, then we don't wait at all */
    if(Time==0)
        return;
    
    /* If the timer flag is still existent(If we don't cancel the delay) */
    if(PCB_Proc_Delay[Current_PID].Delay_Status==1)
        return;
    
    Stop_Time.High_Bits=0;
    Stop_Time.Low_Bits=Time;
    
	/* According to the "OS_Total_Ticks" variable, calculate when we should stop waiting */
    Sys_Tick_Time_Add(&Stop_Time,
                      (struct Tick_Time*)&System_Status.Time.OS_Total_Ticks,
                      &Stop_Time); 
    
    PCB_Proc_Delay[Current_PID].Delay_Status=1;
    PCB_Proc_Delay[Current_PID].End_Total_Ticks.High_Bits=Stop_Time.High_Bits;
    PCB_Proc_Delay[Current_PID].End_Total_Ticks.Low_Bits=Stop_Time.Low_Bits;
    
    /* We need to find a proper time for insertion */
    Traverse_List_Ptr=Proc_Delay_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Proc_Delay_List_Head))
    {
        /* Find a place */
        if(Sys_Tick_Time_Comp(&PCB_Proc_Delay[Current_PID].End_Total_Ticks,
                              &((struct PCB_Timer*)Traverse_List_Ptr)->End_Total_Ticks)!=1)
            break;
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    /* Now put it into the wait list */
    Sys_List_Insert_Node(&(PCB_Proc_Delay[Current_PID].Head),Traverse_List_Ptr->Prev,Traverse_List_Ptr);
    
    /* At last, remove the process from the running queue, and require a direct reschedule */
    _Sys_Clr_Ready(Current_PID);
    Sys_Switch_Now();
}
#endif
/* End Function:Sys_Proc_Delay_Tick *******************************************/

/* Begin Function:Sys_Proc_Delay_Time *****************************************
Description : The delay time unit is millisecond.
Input       : time_t Time - The software delay time.
Output      : None. 
Return      : None.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
void Sys_Proc_Delay_Time(time_t Time)
{
    time_t Ticks;
    /* Convert time to ticks */
    Ticks=Time*SYSTICK_FREQ/1000;
    /* Call the delay tick function */
    Sys_Proc_Delay_Tick(Ticks);
}
#endif
/* End Function:Sys_Proc_Delay_Time *******************************************/

/* Begin Function:Sys_Proc_Delay_Cancel ****************************************
Description : Cancel the process delay.
Input       : pid_t PID - The process ID.
Output      : None. 
Return      : None.
******************************************************************************/
#if(ENABLE_TIMER==TRUE)
void Sys_Proc_Delay_Cancel(pid_t PID)
{
    /* See if the process is being delayed */
    if(PCB_Proc_Delay[PID].Delay_Status==1)
        PCB_Proc_Delay[PID].Delay_Status=0;
    else
        return;
    
	Sys_List_Delete_Node(PCB_Proc_Delay[PID].Head.Prev,PCB_Proc_Delay[PID].Head.Next);
    /* Specify it as ready */
    /* Sys_Set_Ready(PID); */
}
#endif
/* End Function:Sys_Proc_Delay_Cancel ****************************************/

/* Begin Function:_Sys_Proc_Delay_Handler *************************************
Description : The system process delay handler. This will be called in the 
              systick routine.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Proc_Delay_Handler(void)									   
{
#if(ENABLE_TIMER==TRUE)	
   struct List_Head* Traverse_List_Ptr;
   pid_t PID=0;
    
   /* See if there are process being delayed in the list */
   if(Proc_Delay_List_Head.Next==&Proc_Delay_List_Head)  
        return;
   
   /* See if the time has come. If yes, process the process delay list and activate 
    * the corresponding process.
    */
    if(Sys_Tick_Time_Comp(&(((struct PCB_Timer*)(Proc_Delay_List_Head.Next))->End_Total_Ticks),
                          (struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks))==1)
        return;
    
    /* Traverse the list and find out all the timers that need to be processed. Different from the
     * timer's process routine, we will process them here directly.
     */
    
    Traverse_List_Ptr=Proc_Delay_List_Head.Next;
    while((Sys_Tick_Time_Comp(&(((struct PCB_Timer*)Traverse_List_Ptr)->End_Total_Ticks),
                              (struct Tick_Time*)(&System_Status.Time.OS_Total_Ticks))!=-1)&&
          ((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Proc_Delay_List_Head)))
    {   
        PID=((struct PCB_Timer*)Traverse_List_Ptr)->PID;
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
        
        /* Clear its delay status */
        PCB_Proc_Delay[PID].Delay_Status=0;
        Sys_List_Delete_Node(PCB_Proc_Delay[PID].Head.Prev,PCB_Proc_Delay[PID].Head.Next);
        
        /* Specify the process as ready */
        _Sys_Set_Ready(PID);
    }
#endif
}
/* End Function:_Sys_Proc_Delay_Handler **************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
