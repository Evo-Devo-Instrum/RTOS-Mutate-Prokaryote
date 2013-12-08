/******************************************************************************
Filename    : sharemem.h
Author      : pry
Date        : 25/04/2012
Description : The shared memory module for the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SHMEM_H_DEFS__
#define __SHMEM_H_DEFS__
/*****************************************************************************/
/* Errors */
/* Such shared memory block is not existent in the system */
#define ENOSHM              0x00
/* Invalid shared memory (parameters) */
#define EINVSHM             0x01
/* No empty shared memory control blocks, or no enough memory for shared memory */
#define ENOESHM             0x02
/* The shared memory with the same name already exists */
#define ESHMEXIST           0x03
/* When deleting the memory region, it is still being used by some process */
#define ESHMINUSE           0x04
/*****************************************************************************/

/* __SHMEM_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SHMEM_H_STRUCTS__
#define __SHMEM_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"

/* Use defines in these headers */
#define __HDR_DEFS__
#include "Syslib\syslib.h"
#undef __HDR_DEFS__
/*****************************************************************************/
/* The struct of the shared memory header */
struct Shared_Mem
{
    struct List_Head Head;
    s8* Shm_Name;
    shmid_t Shm_ID;
    size_t Shm_Attach_Cnt;
    size_t Shm_Size;
    ptr_int_t Shm_Addr;
};
/*****************************************************************************/

/* __SHMEM_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SHMEM_MEMBERS__
#define __SHMEM_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "ExtIPC\sharemem.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "ExtIPC\sharemem.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*****************************************************************************/
/* Shared memory block list header */
struct List_Head Shm_List_Head;
/* Empty block list header */
struct List_Head Shm_Empty_List_Head;
/* Shared memory control block */
struct Shared_Mem Shm_CB[MAX_SHMS];
/* Shared memory number counter */
size_t Shm_In_Sys;
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

#if(ENABLE_SHMEM==TRUE)
/*****************************************************************************/

/*****************************************************************************/
#endif
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void _Sys_Shm_Init(void);                                            
#if(ENABLE_SHMEM==TRUE)
/*****************************************************************************/
__EXTERN__ shmid_t Sys_Shm_Create(s8* Shm_Name,size_t Shm_Size);
__EXTERN__ retval_t Sys_Shm_Destroy(shmid_t Shm_ID);
__EXTERN__ shmid_t Sys_Shm_Get_ID(s8* Shm_Name);
__EXTERN__ retval_t Sys_Shm_Attach(shmid_t Shm_ID,size_t* Shm_Size,void** Shm_Addr);
__EXTERN__ retval_t Sys_Shm_Detach(shmid_t Shm_ID);
/*****************************************************************************/
#endif

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __SHMEM_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
