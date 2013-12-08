/******************************************************************************
Filename    : systime.c
Author      : pry
Date        : 03/07/2013
Version     : 0.02
Description : The system time management module. This module manages the whole 
              system time function.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syssvc\systime.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Syssvc\systime.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Syssvc\systime.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#include "Syssvc\systime.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:Sys_Set_Time ************************************************
Description : Set the system time.
Input       : struct Tick_Time* Seconds_From_1970 - The seconds from 1970.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Set_Time(struct Tick_Time* Seconds_From_1970)							        	  
{	
    System_Status.Time.Real_World_Time.High_Bits=Seconds_From_1970->High_Bits;
    System_Status.Time.Real_World_Time.Low_Bits=Seconds_From_1970->Low_Bits;
}
/* End Function:Sys_Set_Time *************************************************/

/* Begin Function:Sys_Get_Time ************************************************
Description : Get the system time.
Input       : None
Output      : struct Tick_Time* Seconds_From_1970 - The seconds from 1970.
Return      : None.
******************************************************************************/
void Sys_Get_Time(struct Tick_Time* Seconds_From_1970)							        	  
{	
    Seconds_From_1970->High_Bits=System_Status.Time.Real_World_Time.High_Bits;
    Seconds_From_1970->Low_Bits=System_Status.Time.Real_World_Time.Low_Bits;
}
/* End Function:Sys_Get_Time *************************************************/

/* Begin Function:Sys_Secs_Conv_To_YMDHMS *************************************
Description : Convert the "Seconds_From_1970" to the plain format time.
              This function will only be correct within the range 01/01/1970 0:00
              am - 01/01/2030 0:00 am.
Input       : struct Tick_Time* Seconds_From_1970 - The seconds from 1970.
Output      : time_t* Year - The year.
              time_t* Month - The month.
              time_t* Day - The day.
              time_t* Hour - The hour.
              time_t* Minute - The minute.
              time_t* Second - The second.
Return      : None.
******************************************************************************/
void Sys_Conv_To_YMD(struct Tick_Time* Seconds_From_1970,
                     time_t* Year,time_t* Month,time_t* Day,
                     time_t* Hour, time_t* Minute,time_t* Second)							        	  
{	
    /* This is not implemented yet */
}
/* End Function:Sys_Conv_To_YMD **********************************************/

/* Begin Function:Sys_Get_Total_Ticks *****************************************
Description : Get the system total tick time (called jiffies in other systems).
Input       : None.
Output      : struct Tick_Time* Tick_Time - The total tick time that the system
                                            has went through.(jiffies).
Return      : None.
******************************************************************************/
void Sys_Get_Total_Ticks(struct Tick_Time* Tick_Time)							        	  
{	
    Tick_Time->High_Bits=System_Status.Time.OS_Total_Ticks.High_Bits;
    Tick_Time->Low_Bits=System_Status.Time.OS_Total_Ticks.Low_Bits;
}
/* End Function:Sys_Get_Total_Ticks ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
