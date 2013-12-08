/******************************************************************************
Filename    : wait.h
Author      : pry
Date        : 25/04/2012
Version     : 0.01
Description : The wait kernel object module for the RMV RTOS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __WAIT_H_DEFS__
#define __WAIT_H_DEFS__
/*****************************************************************************/
/* This is infinite wait */
#define  WAIT_INFINITE            0xFFFFFFFF
/* Object identifier */
#define  MUTEX                    0x00
#define  SEMAPHORE                0x01
#define  MSGQUEUE                 0x02
/* Errno identifier */
#define  ENOOBJTYPE               0x00
#define  ENOWAITBLK               0x01
#define  ENOOBJID                 0x02
#define  EWAITOVERTIME            0x03

#define  NO_NEED_TO_WAIT          -2 
#define  WAIT_FAILURE             -1
/*****************************************************************************/

/* __WAIT_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __WAIT_H_STRUCTS__
#define __WAIT_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"

/* Use defines in these headers */
#define __HDR_DEFS__
#include "Syslib\syslib.h"
#undef __HDR_DEFS__
/*****************************************************************************/
/* The structure of the wait object struct */
struct Wait_Object_Struct
{
    /* This head will be inserted into the PCB part */
    struct List_Head PCB_Head;
    /* This head will be inserted into the object part */
    struct List_Head Object_Head;
    /* The process ID */
    pid_t PID;
    /* The object ID */
    cnt_t Obj_ID;
    /* The object type identifier */
    cnt_t Type;
    /* Whether the wait for this succeeded */
    cnt_t Succeed_Flag;
};

/* The wait part of the PCB */
struct PCB_Wait_Struct
{
    pid_t PID;
    struct List_Head Wait_Head;
};
/*****************************************************************************/

/* __WAIT_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __WAIT_MEMBERS__
#define __WAIT_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "ExtIPC\wait.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "ExtIPC\wait.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*****************************************************************************/
/* The head for the empty list head */
struct List_Head Wait_Empty_List_Head;
/* The PCB's wait part */
struct PCB_Wait_Struct PCB_Wait[MAX_PROC_NUM];
/* The wait object control block */
struct Wait_Object_Struct Wait_CB[MAX_WAIT_BLOCKS];
/* Wait relationships in the system */
cnt_t Wait_Relation_In_Sys;
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


/*****************************************************************************/

/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void _Sys_Wait_Init(void);                                            
/*****************************************************************************/
__EXTERN__ cnt_t Sys_Wait_Object(cnt_t Object_ID,cnt_t Object_Type,time_t Time);
__EXTERN__ cnt_t Sys_Wait_Multi_Objects(cnt_t* Object_ID,cnt_t* Object_Type,cnt_t Object_Number,time_t Time,
                                        cnt_t* Object_Succeed_List);
/*****************************************************************************/


/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __WAIT_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
