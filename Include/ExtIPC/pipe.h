/******************************************************************************
Filename    : pipe.h
Author      : pry
Date        : 25/04/2012
Description : The (named) pipe module for the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __PIPE_H_DEFS__
#define __PIPE_H_DEFS__
/*****************************************************************************/
/* The identifier for unopened pipe side */
#define PIPECLOSE          -1
/* There's no enough space for the pipe */
#define ENOEPIPE           0x01
/* The pipe parameter is invalid */
#define EINVPIPE           0x02
/* The pipe is already being used by two other process */
#define EPIPEINUSE         0x03
/* The pipe with the same name already exists */
#define EPIPEEXIST         0x04
/* The pipe does not exist */
#define ENOPIPE            0x05
/*****************************************************************************/

/* __PIPE_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __PIPE_H_STRUCTS__
#define __PIPE_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"

/* Use defines in these headers */
#define __HDR_DEFS__
#include "Syslib\syslib.h"
#undef __HDR_DEFS__
/*****************************************************************************/
/* The struct of the pipe header */
struct Pipe
{
    struct List_Head Head;
    s8* Pipe_Name;
    pipeid_t Pipe_ID;
    pid_t Opener_PID[2];
    size_t Pipe_Buffer_Size;
    ptr_int_t Pipe_Buffer_Addr;
};
/*****************************************************************************/

/* __PIPE_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __PIPE_MEMBERS__
#define __PIPE_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "ExtIPC\pipe.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "ExtIPC\pipe.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*****************************************************************************/
/* Pipe list header */
struct List_Head Pipe_List_Head;
/* Empty block list header */
struct List_Head Pipe_Empty_List_Head;
/* Statistic variable: The number of pipes in the system */
size_t Pipe_In_Sys;
/* The pipe control block */
/* Here we use a pipe control block to control the pipe. We don't place the pipe
 * control information alongside with the pipe buffer. As such, the dynamic memory
 * allocation will not be so large.
 */
struct Pipe Pipe_CB[MAX_PIPES];
/*****************************************************************************/

/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/
/*****************************************************************************/

/*****************************************************************************/
#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

#if(ENABLE_PIPE==TRUE)
/*****************************************************************************/

/*****************************************************************************/
#endif
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void _Sys_Pipe_Init(void);                                            
#if (ENABLE_PIPE==TRUE)
/*****************************************************************************/
__EXTERN__ pipeid_t Sys_Create_Pipe(s8* Pipe_Name,size_t Pipe_Buffer_Size);
__EXTERN__ retval_t Sys_Destroy_Pipe(pipeid_t Pipe_ID);
__EXTERN__ pipeid_t Sys_Get_Pipe_ID(s8* Pipe_Name);

__EXTERN__ retval_t Sys_Open_Pipe(pipeid_t Pipe_ID,size_t* Buffer_Size,void** Pipe_Buffer_Ptr);
__EXTERN__ retval_t _Sys_Open_Pipe(pid_t Opener_PID,pipeid_t Pipe_ID,size_t* Buffer_Size,void** Pipe_Buffer_Ptr);
__EXTERN__ retval_t Sys_Close_Pipe(pipeid_t Pipe_ID);
__EXTERN__ retval_t _Sys_Close_Pipe(pid_t Closer_PID,pipeid_t Pipe_ID);

__EXTERN__ void Sys_Close_All_Pipes(void);
__EXTERN__ void _Sys_Close_All_Pipes(pid_t PID);

__EXTERN__ size_t Sys_Query_Pipe_Number(void);
/*****************************************************************************/
#endif

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __PIPE_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
