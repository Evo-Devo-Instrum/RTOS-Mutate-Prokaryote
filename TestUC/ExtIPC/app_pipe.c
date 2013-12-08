/******************************************************************************
Filename   : app_pipe.c
Author     : pry
Date       : 12/07/2013
Description: The test use case for the (named) pipe module.
             Here we utilize white-box testing method with full statement coverage.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

#include "Commons\MP_commons.h"
#include "Commons\MuGUI_commons.h"
#include "Commons\MuSH_commons.h"
#include "app_pipe.h"
/* End Includes **************************************************************/

/* Begin Function:Sys_Start_On_Boot *******************************************
Description : The function will be called by the init process when booting the system.
              The processes referred to will be started on boot.
Input       : None.
Output      : None.
******************************************************************************/
void Sys_Start_On_Boot(void)
{   
    /* These processes will be started at the beginning. They will be started by the 
     * init process. We can specify the PID for each process when we are booting 
     * the OS.
     */
    struct Proc_Init_Struct Process;

    Process.PID=1;                                                                   
    Process.Name="Arch";								                                   
    Process.Entrance=_Sys_Arch;                                                            
    Process.Stack_Address=(ptr_int_t)Arch_Stack;				                                         
    Process.Stack_Size=ARCH_STACK_SIZE;
    Process.Max_Slices=4;                                                           
    Process.Min_Slices=1;                                                             
    Process.Cur_Slices=1;                                                                                                          
    Process.Priority=1;      
    Process.Ready_Flag=READY;        
    _Sys_Proc_Load(&Process);                                                  

    Process.PID=2;                                                                  
    Process.Name="Proc1_PID_2";								                              
    Process.Entrance=Proc1;                                                       
    Process.Stack_Address=(ptr_int_t)App_Stack_1;				                                       
    Process.Stack_Size=APP_STACK_1_SIZE;
    Process.Max_Slices=4;                                                             
    Process.Min_Slices=1;                                                              
    Process.Cur_Slices=3;                                                                                                             
    Process.Priority=1;  
    Process.Ready_Flag=READY;         
    _Sys_Proc_Load(&Process);                                                        

    Process.PID=3;                                                                   
    Process.Name="Proc2_PID_3";								                                  
    Process.Entrance=Proc2;                                                           
    Process.Stack_Address=(ptr_int_t)App_Stack_2;				                                          
    Process.Stack_Size=APP_STACK_2_SIZE;
    Process.Max_Slices=4;                                                              
    Process.Min_Slices=1;                                                               
    Process.Cur_Slices=3;                                                                                                                  
    Process.Priority=1;                                                             
    _Sys_Proc_Load(&Process);
}
/* End Function:Sys_Start_On_Boot ********************************************/

/* Begin Function:Sys_Init_Initial ********************************************
Description : The function will be called by "Init" after booting the system once.
              It can be used to initialize some user environment. Here we return 
              directly.
Input       : None.
Output      : None.
******************************************************************************/
void Sys_Init_Initial(void)
{ 
    return;
}
/* End Function:Sys_Init_Initial *********************************************/

/* Begin Function:Sys_Init_Always *********************************************
Description : The init process will keep calling this function when the booting
              is done. In fact this function is called in the "while(1)". Different
              from the "Arch"'s "Always", the function will only be executed once
              when the "Init" process is given timeslice.
Input       : None.
Output      : None.
******************************************************************************/
void Sys_Init_Always(void)
{ 
    return;
}
/* End Function:Sys_Init_Always **********************************************/

/* Begin Function:Sys_Arch_Initial ********************************************
Description : The function will be called by "Arch" once when it runs.
              It can be used to initialize some user environment. Here we return 
              directly.
Input       : None.
Output      : None.
******************************************************************************/
void Sys_Arch_Initial(void)
{ 
    return;
}
/* End Function:Sys_Arch_Initial *********************************************/

/* Begin Function:Sys_Arch_Always *********************************************
Description : The "Arch" process will keep calling this function when the booting
              is done. In fact this function is called in the "while(1)". Here it 
              is used as the shell command prompt.
Input       : None.
Output      : None.
******************************************************************************/
void Sys_Arch_Always(void)
{ 
    Sys_Shell(); 
    /* Shouldn't return in this use case */
    return;
}
/* End Function:Sys_Arch_Always **********************************************/

/* Begin Function:Proc1 *******************************************************
Description : The pseudo-process 1.
Input       : Void.
Output      : Void
******************************************************************************/
void Proc1(void)
{       
    pipeid_t Pipe_ID;
    void* Buffer_Ptr;
    
    struct Proc_Init_Struct Process;

    /* Here we start another task. However, we cannot specify the PID of the task
     * when the system is already started. The PID is automatically allocated.
     */
    
    Process.PID=AUTO_PID;
    Process.Parent_Policy=PARENT_ABDN;
    Process.Name="Proc3_PID_4";
    Process.Entrance=Proc3;
    Process.Stack_Address=(ptr_int_t)App_Stack_3; 
    Process.Stack_Size=APP_STACK_3_SIZE;
    Process.Max_Slices=4;         
    Process.Min_Slices=1;
    Process.Cur_Slices=3;                             
    Process.Priority=1;
    Sys_Start_Proc(&Process);
    Sys_Set_Ready(4);
    
    /* Create a pipe */
    Sys_Create_Pipe("A Pipe",100);
    /* Get the ID here */
    Pipe_ID=Sys_Get_Pipe_ID("A Pipe");

    Sys_Open_Pipe(Pipe_ID,0,&Buffer_Ptr);
    Sys_Close_Pipe(Pipe_ID);
    
	while(1)
    {
        Sys_Switch_Now();
    }
}
/* End Function:Proc1 ********************************************************/

/* Begin Function:Proc2 *******************************************************
Description : Test pseudo-process 2.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Proc2(void)
{       
    pipeid_t Pipe_ID;
    void* Buffer_Ptr;
    
    Pipe_ID=Sys_Get_Pipe_ID("A Pipe");
    Sys_Open_Pipe(Pipe_ID,0,&Buffer_Ptr);
    Sys_Close_Pipe(Pipe_ID);
    Sys_Destroy_Pipe(Pipe_ID);
    while(1)
    {
       Sys_Switch_Now();
    }
}
/* End Function:Proc2 ********************************************************/

/* Begin Function:Proc3 *******************************************************
Description : Test pseudo-rocess 3.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Proc3(void)
{   
    while(1)
    {
        
    }
}
/* End Function:Proc3 ********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/

