/******************************************************************************
Filename    : msgqueue.h
Author      : pry
Date        : 15/07/2013
Version     : 0.01
Description : The queue module for the OS.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __MSGQUEUE_H_DEFS__
#define __MSGQUEUE_H_DEFS__
/*****************************************************************************/
/* The message is in the empty(spare) list */
#define MSG_BLOCK_IN_EMPTY  0x00
/* The message is in the message queue list */
#define MSG_BLOCK_IN_QUEUE  0x01
/* The block is out of the list(being used) */
#define MSG_BLOCK_NO_LIST   0x02
/* Receive or query all types of messages */
#define ALL_TYPE_MSG 0xFFFFFFFF
/* Query the number of a certain type to all processes */
#define ALL_PROC_MSG MAX_PROC_NUM

/* There's no message to the receiver */
#define ENOMSG      (-1)
/* There's no such message block */
#define ENOMSGBLK   0x01
/* There's no empty message queue control block available */
#define ENOEMSGQ    MAX_MSG_QUEUES+1
/* There's no such message queue */
#define ENOMSGQ     MAX_MSG_QUEUES+1

/*****************************************************************************/

/* __MSGQUEUE_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __MSGQUEUE_H_STRUCTS__
#define __MSGQUEUE_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"

/* Use defines in these headers */
#define __HDR_DEFS__
#include "Syslib\syslib.h"
#undef __HDR_DEFS__
/*****************************************************************************/
/* The struct of the message queue */
struct Msg_Queue
{
    struct List_Head Head;
    struct List_Head Msg_Block_Queue_Head;
    struct List_Head Msg_Block_Empty_Head;
    struct List_Head Wait_Object_Head;
    
    /* The name of the message queue */
    s8* Msg_Queue_Name;
    /* The unique identifier of it */
    msgqid_t Msg_Queue_ID;
    /* The maximum number of messages in it */
    size_t Msg_Max_Number;
    /* The current number of messages in it */
    size_t Msg_Cur_Number;
    /* The current number of messages being used */
    size_t Msg_Cur_Use_Number;
    /* The start address of the message queue. When we free memory, we use this
     * pointer.
     */
    ptr_int_t Msg_Queue_Start_Addr;
};

/* The struct of the message itself */
struct Msg_Block
{
    struct List_Head Head;
    u32 Msg_Block_List_ID;
    msgqid_t Msg_Queue_ID;
    msgqbid_t Msg_Block_ID;
    msgtyp_t Msg_Type;
    /* This pointer points to the start address of the immediate message */
    ptr_int_t Msg_Addr_Ptr;
    pid_t Msg_Send_PID;
    pid_t Msg_Recv_PID;
};
/*****************************************************************************/

/* __MSGQUEUE_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __MSGQUEUE_MEMBERS__
#define __MSGQUEUE_MEMBERS__
/* In this way we can use the data structures in the headers */
#define __HDR_DEFS__
#include "ExtIPC\msgqueue.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\wait.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*****************************************************************************/
/* Control block list header (not for message blocks!) */
struct List_Head Msg_CB_List_Head;
struct List_Head Empty_Msg_CB_List_Head;
/* The message queue control block */
struct Msg_Queue Msg_CB[MAX_MSG_QUEUES];
/* Statistic variable */
size_t Msg_Queue_In_Sys;
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

#if(ENABLE_MSGQ==TRUE)
/*****************************************************************************/

/*****************************************************************************/
#endif
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void _Sys_Queue_Init(void);                                            
#if (ENABLE_MSGQ==TRUE)
/*****************************************************************************/
__EXTERN__ msgqid_t Sys_Create_Queue(s8* Msg_Queue_Name,size_t Msg_Number);
__EXTERN__ retval_t Sys_Destroy_Queue(msgqid_t Msg_Queue_ID);
__EXTERN__ msgqid_t Sys_Get_Queue_ID(s8* Msg_Queue_Name);

__EXTERN__ msgqbid_t Sys_Alloc_Msg(msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,size_t Msg_Size,void** Msg_Buffer_Ptr);
__EXTERN__ msgqbid_t _Sys_Alloc_Msg(pid_t Sender_PID,msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,size_t Msg_Size,void** Msg_Buffer_Ptr);
__EXTERN__ retval_t Sys_Send_Msg(pid_t Recver_PID,msgqid_t Msg_Queue_ID,msgqbid_t Msg_Block_ID);

__EXTERN__ msgqbid_t Sys_Recv_Msg(msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,void** Msg_Buffer_Ptr);
__EXTERN__ msgqbid_t _Sys_Recv_Msg(pid_t Recver_PID,msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,void** Msg_Buffer_Ptr);
__EXTERN__ retval_t Sys_Destroy_Msg(msgqid_t Msg_Queue_ID,msgqbid_t Msg_Block_ID);

__EXTERN__ size_t Sys_Query_Msg_To_Recver(pid_t PID,msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type);
__EXTERN__ size_t Sys_Query_Msg_Number(msgqid_t Msg_Queue_ID);
__EXTERN__ size_t Sys_Query_Msg_Queue_Number(msgqid_t Msg_Queue_ID);
__EXTERN__ retval_t _Sys_Wait_Msg_Queue_Reg(pid_t PID,msgqid_t Msg_Queue_ID,
                                            struct Wait_Object_Struct* Wait_Block_Ptr);
/*****************************************************************************/
#endif

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __MSGQUEUE_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
