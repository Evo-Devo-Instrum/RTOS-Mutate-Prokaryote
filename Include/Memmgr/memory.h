/******************************************************************************
Filename    : memory.h
Author      : pry
Date        : 25/04/2012
Version     : 0.02
Description : The memory managing module for the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __MEMORY_H_DEFS__
#define __MEMORY_H_DEFS__

/* Errors */
/* Memory is not available */
#define ENOMEM          ((void*)0x00)

/* For "_Sys_Mfree"'s use */
#define DMEM_START_ADDR ((u32)(DMEM_Heap))
#define DMEM_END_ADDR   (((u32)(DMEM_Heap))+DMEM_SIZE-sizeof(struct Mem_Tail)-64-8)

/* __MEMORY_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __MEMORY_H_STRUCTS__
#define __MEMORY_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"

/* Use defines in these headers */
#define __HDR_DEFS__
#include "Syslib\syslib.h"
#undef __HDR_DEFS__
/*****************************************************************************/
/* The head struct of a memory block */
struct Mem_Head
{
    /* This is what is used in TLSF LUT */
    struct List_Head Head;
    /* This is for freeing all memory of a process */
    struct List_Head Proc_Mem_Head;
    struct Mem_Tail* Tail_Ptr;
    /* We need an flag because even the 0 process will allocate some memory */  
    u32 Occupy_Flag;
    u32 Occupy_PID;
    /* The pointer to the start of the memory block address */
    ptr_int_t Mem_Start_Addr;
    /* The pointer to the end of the memory block address */
    ptr_int_t Mem_End_Addr;
};

/* The tail struct of a memory block */
struct Mem_Tail
{
    /* This is for tailing the memory */
    struct Mem_Head* Head_Ptr;
    /* This shall be the same with the flag in the head */
    u32 Occupy_Flag;
};

/* The struct for PCB_Mem */
struct PCB_Memory
{
    struct List_Head Head;
    cnt_t Memory_In_Use;
};
/*****************************************************************************/

/* __MEMORY_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __MEMORY_MEMBERS__
#define __MEMORY_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "Memmgr\memory.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "Memmgr\memory.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*****************************************************************************/
/* The TLSF registry table. We fix SLI as 8, so it is one byte. */
static struct List_Head Mem_CB[MM_FLI][8];
static u8 Mem_Bitmap[MM_FLI];
/* The PCB_Mem table */
static struct PCB_Memory PCB_Mem[MAX_PROC_NUM];
/* The memory for allocation. This part is the continuous memory to allocate */
static u8 DMEM_Heap[DMEM_SIZE];
/* The list of memory blocks being used */
static struct List_Head Mem_Allocated_List_Head;
/* The statistic variables */
static cnt_t Free_Mem_Amount;
static cnt_t Used_Mem_Amount;
/*****************************************************************************/

/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/
static void _Sys_Mem_Ins_TLSF(struct Mem_Head* Mem_Head_Ptr);
static void _Sys_Mem_Del_TLSF(struct Mem_Head* Mem_Head_Ptr);
static void _Sys_Mem_Init_Block(ptr_int_t Mem_Start_Addr,size_t Mem_Size);
static retval_t Sys_Mem_TLSF_Bitmap_Search(size_t Mem_Size,s32* FLI_Level,s32* SLI_Level);
static void _Sys_Mem_Ins_Allocated(pid_t PID,struct Mem_Head* Mem_Head_Ptr);
static void _Sys_Mem_Del_Allocated(struct Mem_Head* Mem_Head_Ptr);
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
__EXTERN__ void _Sys_Mem_Init(void);                                            
#if(ENABLE_MEMM==TRUE)
__EXTERN__ void* Sys_Malloc(size_t Size);
__EXTERN__ void* _Sys_Malloc(pid_t PID,size_t Size);
__EXTERN__ void Sys_Mfree(void* Mem_Ptr);    
__EXTERN__ void _Sys_Mfree(pid_t PID,void* Mem_Ptr);
__EXTERN__ void Sys_Mfree_All(void);	                                   
__EXTERN__ void _Sys_Mfree_All(pid_t PID);
__EXTERN__ size_t Sys_Query_Proc_Mem(pid_t PID);
__EXTERN__ size_t Sys_Query_Used_Memory(void);
__EXTERN__ size_t Sys_Query_Free_Mem(void);
#endif

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __MEMORY_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
