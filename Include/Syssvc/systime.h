/******************************************************************************
Filename   : systime.h
Author     : pry
Date       : 15/07/2013
Description: The header for the time management of the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SYSTIME_H_DEFS__
#define __SYSTIME_H_DEFS__
/*****************************************************************************/

/*****************************************************************************/

/* __SYSTIME_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SYSTIME_H_STRUCTS__
#define __SYSTIME_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/

/*****************************************************************************/

/* __SYSTIME_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SYSTIME_MEMBERS__
#define __SYSTIME_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/

#define __EXTERN__ 
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

	                  
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/

/*****************************************************************************/
__EXTERN__ void Sys_Set_Time(struct Tick_Time* Seconds_From_1970);
__EXTERN__ void Sys_Get_Time(struct Tick_Time* Seconds_From_1970);
__EXTERN__ void Sys_Conv_To_YMD(struct Tick_Time* Seconds_From_1970,
                                time_t* Year,time_t* Month,time_t* Day,
                                time_t* Hour, time_t* Minute,time_t* Second);
__EXTERN__ void Sys_Get_Total_Ticks(struct Tick_Time* Tick_Time);
/*****************************************************************************/
#undef __EXTERN__

/* __SYSTIME_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
