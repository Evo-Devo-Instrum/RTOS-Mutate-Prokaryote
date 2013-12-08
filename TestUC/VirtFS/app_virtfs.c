/******************************************************************************
Filename   : applications.c
Author     : pry
Date       : 25/04/2012
Description: The Tasks&applications&Test Functions Is here.
******************************************************************************/

/*Includes********************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

#include "Commons\MP_commons.h"
#include "Commons\MuGUI_commons.h"
#include "Commons\MuSH_commons.h"
#include "applications.h"
/*End Includes****************************************************************/

/*Begin Function:_Sys_Arch*****************************************************
Description : The system service daemon. Now it is usually used as a shell process.
              It is a system process(just like the init process), but it runs in the 
              user field.
Input       : Void.
Output      : Void.
******************************************************************************/
void _Sys_Arch(void)							            
{   
    Sys_Shell();
}
/*Finish Function:_Sys_Arch***************************************************/

u8 Sem_ID[3];

/*Begin Function:Task1*********************************************************
Description : The Task Process 1.
Input       : Void.
Output      : Void
******************************************************************************/
void Task1(void)
{   
    struct Proc_Init_Struct Process;
    u8 Sem_ID;
    
	Sys_GUI_LCD_Init();

    Process.PID=0;
    Process.Name="Driver_2";
    Process.Entrance=Task3;
    Process.Stack_Address=App_Stack_3; 
    Process.Stack_Size=APP_STACK_3_SIZE;
    Process.Max_Slices=4;         
    Process.Min_Slices=1;
    Process.Cur_Slices=3;                             
    Process.Priority=1;
    Sys_Start_Proc(&Process);
    Sys_Set_Ready(4);
    
    Sys_Register_Sem("Fucker",5);
    Sem_ID=Sys_Get_Sem_ID("Fucker");
    Sys_Occupy_Sem(Sem_ID,3);
    Sys_Occupy_Sem(Sem_ID,3);
    Sys_Free_Sem(Sem_ID,3);
    Sys_Free_All_Sem();
    Sys_Query_Sem_Amount(Sem_ID);
    Sys_Remove_Sem(Sem_ID);
    
	while(1)
	{   
		Sys_GUI_LCD_Clear_Screen(GREEN);
	}
}
/*Finish Function:Task1*******************************************************/

/*Begin Function:Task2*********************************************************
Description : Test Process 2.
Input       : Void.
Output      : Void
******************************************************************************/
void Task2(void)
{ 

    while(1)
    {   
        Sys_GUI_LCD_Clear_Screen(RED);
    }
}
/*Finish Function:Task2*******************************************************/

/*Begin Function:Task3*********************************************************
Description : Test Process 3.
Input       : Void.
Output      : Void
******************************************************************************/
void Task3(void)
{ 
    
    while(1)
    {
        Sys_GUI_LCD_Clear_Screen(BLUE);
    }
}
/*Finish Function:Task3*******************************************************/

/* Begin Function:Sys_Debug_No_Optim ******************************************
Description : Make sure that the compiler don't optimize some important variable.
Input       : u32 Variable - The variable.
Output      : u32 - The variable itself.
******************************************************************************/
u32 Sys_Debug_No_Optim(u32 Variable)
{ 
    return Variable;
}
/* End Function:Sys_Debug_No_Optim *******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/

