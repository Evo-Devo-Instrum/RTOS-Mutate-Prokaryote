/******************************************************************************
Filename    : error.h
Author      : pry
Date        : 25/06/2012
Version     : 0.01
Description : The error processing part of RMP.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __ERROR_H_DEFS__
#define __ERROR_H_DEFS__
               				   

/* __ERROR_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif 
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __ERROR_H_STRUCTS__
#define __ERROR_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/* __ERROR_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
/* If the header is not used in the public mode */
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __ERROR_MEMBERS__
#define __ERROR_MEMBERS__

/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "Kernel\error.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "Kernel\error.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*End Private Global Variables************************************************/

/*Private Defines*************************************************************/

/*End Private Defines*********************************************************/

/*Private C Function Prototypes***********************************************/

#define __EXTERN__
/*End Private C Function Prototypes*******************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
__EXTERN__ void Sys_Set_Errno(errno_t Errno);
__EXTERN__ void _Sys_Set_Errno(pid_t PID,errno_t Errno);
__EXTERN__ errno_t Sys_Get_Errno(void);
__EXTERN__ errno_t _Sys_Get_Errno(pid_t PID);
/*****************************************************************************/

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __ERROR_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
