/******************************************************************************
Filename    : sysdebug.c
Author      : pry
Date        : 05/07/2013
Version     : 0.01
Description : The debug module for the RMV RTOS.
******************************************************************************/

/*Includes********************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#include "Syssvc\sysdebug.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#include "Syssvc\sysdebug.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Syssvc\sysdebug.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\signal.h"
#include "Memmgr\memory.h"
#include "ExtIPC\semaphore.h"
#include "..\Muapp\applications.h"
#include "Syslib\syslib.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#include "Syssvc\sysdebug.h"
#undef __HDR_PUBLIC_MEMBERS__

/*End Includes****************************************************************/

/* Begin Function:Sys_Debug_Var ***********************************************
Description : Make sure that the compiler don't optimize some important variable.
Input       : cnt_t Variable - The variable.
Output      : None.
Return      : cnt_t - The variable itself.
******************************************************************************/
cnt_t Sys_Debug_Var(cnt_t Variable)
{ 
    return Variable;
}
/* End Function:Sys_Debug_Var ************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved.*************/
