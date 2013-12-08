/******************************************************************************
Filename   : sysstat.h
Author     : pry
Date       : 14/07/2013
Description: The header for the system statistic module of the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SYSSTAT_H_DEFS__
#define __SYSSTAT_H_DEFS__
/*****************************************************************************/

/*****************************************************************************/

/* __SYSSTAT_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SYSSTAT_H_STRUCTS__
#define __SYSSTAT_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/

/*****************************************************************************/

/* __SYSSTAT_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SYSSTAT_MEMBERS__
#define __SYSSTAT_MEMBERS__

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
__EXTERN__ void Sys_Query_Stat_Kernel(struct Sys_Kernel_Status_Struct* Kernel);
__EXTERN__ void Sys_Query_Stat_Proc(struct Sys_Proc_Status_Struct* Proc);
__EXTERN__ void Sys_Query_Stat_Time(struct Sys_Time_Status_Struct* Time);
__EXTERN__ retval_t Sys_Query_Proc_Stat(pid_t PID,struct PCB_Struct* Proc_PCB);
/*****************************************************************************/
#undef __EXTERN__

/* __SYSSTAT_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
