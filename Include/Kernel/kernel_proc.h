/******************************************************************************
Filename   : kernel_proc.h
Author     : pry
Date       : 05/07/2013
Description: The header of "kernel_proc.c".
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __KERNEL_PROC_H_DEFS__
#define __KERNEL_PROC_H_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __KERNEL_PROC_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/


/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __KERNEL_PROC_H_STRUCTS__
#define __KERNEL_PROC_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __KERNEL_PROC_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/*Private Global Variables****************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __KERNEL_PROC_MEMBERS__
#define __KERNEL_PROC_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__
#include "Kernel\kernel_proc.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "Kernel\kernel_proc.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

/*****************************************************************************/
/*End Private Global Variables************************************************/

/*Private C Function Prototypes***********************************************/ 
/*****************************************************************************/

/*****************************************************************************/
#define __EXTERN__
/*End Private C Function Prototypes*******************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/*****************************************************************************/
/* The stack for init process and the arch process */
__EXTERN__ ptr_int_t Kernel_Stack[KERNEL_STACK_SIZE];
__EXTERN__ ptr_int_t Arch_Stack[ARCH_STACK_SIZE];
/*****************************************************************************/
/*End Process Scheduling*/
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
__EXTERN__ void _Sys_Init(void);
__EXTERN__ void _Sys_Arch(void);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* End Public C Function Prototypes ******************************************/

/*Assembly Functions Prototypes***********************************************/
/*****************************************************************************/
				                       
/*****************************************************************************/
/* __KERNEL_PROC_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/*End Assembly Functions Prototypes*******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
