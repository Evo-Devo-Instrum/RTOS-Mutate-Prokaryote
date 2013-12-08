/******************************************************************************
Filename    : pipe.c
Author      : pry
Date        : 25/04/2012
Version     : 0.12
Description : The pipe module for the OS. The pipe is for light-weight IPC and
              is a named one. Different from the other IPCs, the pipe can support
              reciprocal read and write, making it ideal for large data transfer.
              However, the pipe can only support information exchange between two 
              processes. Usually, one process create the (named) pipe and open one
              port of it, while another process open the other port. As the pipe 
              itself does not provide any means of synchronization, we can use the
              signal system to synchronize between the two processes.
              You cannot wait for a pipe.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Memmgr\memory.h"
#include "ExtIPC\pipe.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Memmgr\memory.h"
#include "ExtIPC\pipe.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "ExtIPC\pipe.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#include "Memmgr\memory.h"
#include "ExtIPC\pipe.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Pipe_Init *********************************************
Description : The initialization function for the system pipe module.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Pipe_Init(void)
{
#if(ENABLE_PIPE==TRUE)
    cnt_t Pipe_Cnt;
    /* Initialize the two lists */
    Sys_Create_List(&Pipe_Empty_List_Head);
    Sys_Create_List(&Pipe_List_Head);
    /* Clear the whole control block array */
    Sys_Memset((ptr_int_t)Pipe_CB,0,MAX_PIPES*sizeof(struct Pipe));

    /* Put all the pipe control blocks in the empty ones */
    for(Pipe_Cnt=0;Pipe_Cnt<MAX_PIPES;Pipe_Cnt++)
    {
        /* -1 means that the two side of the pipe is not opened */
        Pipe_CB[Pipe_Cnt].Opener_PID[0]=PIPECLOSE;
        Pipe_CB[Pipe_Cnt].Opener_PID[1]=PIPECLOSE;
        
        Pipe_CB[Pipe_Cnt].Pipe_ID=Pipe_Cnt;
        /* Put all of them in the empty list */
        Sys_List_Insert_Node(&(Pipe_CB[Pipe_Cnt].Head),&Pipe_Empty_List_Head,Pipe_Empty_List_Head.Next);
    }
    
    /* Clear the statistical variable */
    Pipe_In_Sys=0;
#endif    
}
/* End Function:_Sys_Pipe_Init **********************************************/

/* Begin Function:Sys_Create_Pipe ********************************************
Description : Create a pipe and return the unique ID of it. When the pipe is created,
              then the memory allocation is done.
Input       : s8* Pipe_Name - The name of the pipe.
              size_t Pipe_Buffer_Size - The size of the pipe buffer.
Output      : None.
Return      : pipeid_t - The identifier of the (named) pipe. Should the function fail,
                         it will return "-1".
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
pipeid_t Sys_Create_Pipe(s8* Pipe_Name,size_t Pipe_Buffer_Size)
{
    pipeid_t Pipe_ID;
    void* Buffer_Ptr;
    struct List_Head* Traverse_List_Ptr;
    /* See if the name is an empty pointer, or the buffer size wrong */
    if((Pipe_Name==0)||(Pipe_Buffer_Size==0))
    {
        Sys_Set_Errno(EINVPIPE);
        return -1;
    }
    
    Sys_Lock_Scheduler();
    
    /* See if there are any empty blocks */
    if((ptr_int_t)(&Pipe_Empty_List_Head)==(ptr_int_t)(Pipe_Empty_List_Head.Next))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOEPIPE);
        return -1;
    }
    
    /* If there are, get the pipe ID. We may only use this later */
    Pipe_ID=((struct Pipe*)Pipe_Empty_List_Head.Next)->Pipe_ID;
    
    /* Traverse the list to see if the name is unique */
    Traverse_List_Ptr=Pipe_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Pipe_List_Head))
    {
        /* If we can find a match, abort */
        if(Sys_Strcmp(Pipe_Name,((struct Pipe*)Traverse_List_Ptr)->Pipe_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(EPIPEEXIST);
            return -1;
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }

    /* See if we can allocate any memory from the system. The memory allocation is
     * done in the name of "Init".
     */
    Buffer_Ptr=_Sys_Malloc(0,Pipe_Buffer_Size);
    
    /* Allocation failed */
    if(Buffer_Ptr==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOEPIPE);
        return -1;
    }
    
    /* Now we are sure that we have enough resource to set up the pipe */
    Sys_List_Delete_Node(Pipe_CB[Pipe_ID].Head.Prev,Pipe_CB[Pipe_ID].Head.Next);
    Sys_List_Insert_Node(&(Pipe_CB[Pipe_ID].Head),&Pipe_List_Head,Pipe_List_Head.Next);
    
    /* Register the values */
    Pipe_CB[Pipe_ID].Pipe_Name=Pipe_Name;
    Pipe_CB[Pipe_ID].Pipe_Buffer_Size=Pipe_Buffer_Size;
    Pipe_CB[Pipe_ID].Pipe_Buffer_Addr=(ptr_int_t)Buffer_Ptr;
    
    /* Update the statistical variable */
    Pipe_In_Sys++;
    
    Sys_Unlock_Scheduler();
    return Pipe_ID;
}
#endif
/* End Function:Sys_Create_Pipe *********************************************/

/* Begin Function:Sys_Destroy_Pipe *******************************************
Description : Destroy a pipe given the ID of the pipe.
              When we want to destroy the pipe, it is required that both sides
              of the pipe to be closed.
Input       : pipeid_t Pipe_ID - The identifier of the pipe.
Output      : None.
Return      : retval_t - If successful, it will return 0; else -1.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
retval_t Sys_Destroy_Pipe(pipeid_t Pipe_ID)
{
    /* See if the pipe ID is over the boundary */
    if(Pipe_ID>=MAX_PIPES)
    {
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the pipe exists in the system */
    if(Pipe_CB[Pipe_ID].Pipe_Buffer_Size==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    /* If either side of the pipe is opened, abort */
    if((Pipe_CB[Pipe_ID].Opener_PID[0]!=PIPECLOSE)||
       (Pipe_CB[Pipe_ID].Opener_PID[1]!=PIPECLOSE))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EPIPEINUSE);
        return (-1);
    }
    
    /* Now we can safely destroy the pipe */
    Sys_List_Delete_Node(Pipe_CB[Pipe_ID].Head.Prev,Pipe_CB[Pipe_ID].Head.Next);
    Sys_List_Insert_Node(&(Pipe_CB[Pipe_ID].Head),&Pipe_Empty_List_Head,Pipe_Empty_List_Head.Next);
    
    /* Free buffer memory */
    _Sys_Mfree(0,(void*)(Pipe_CB[Pipe_ID].Pipe_Buffer_Addr));
    
    /* Clear the variables. */
    Pipe_CB[Pipe_ID].Pipe_Name=0;
    Pipe_CB[Pipe_ID].Pipe_Buffer_Size=0;
    Pipe_CB[Pipe_ID].Pipe_Buffer_Addr=0;
    
    /* Update statistical variable */
    Pipe_In_Sys--;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Destroy_Pipe ********************************************/

/* Begin Function:Sys_Get_Pipe_ID ********************************************
Description : Get the pipe's unique identifier through the name of it. 
              If the pipe exists, return the identifier.
Input       : s8* Pipe_Name - The name of the pipe.
Output      : None.
Return      : pipeid_t - The identifier of the pipe. If the pipe doesn't 
                         exist, then return -1.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
pipeid_t Sys_Get_Pipe_ID(s8* Pipe_Name)
{
    struct List_Head* Traverse_List_Ptr;
    
    Sys_Lock_Scheduler();
    /* Traverse the list to see if the name exists */
    Traverse_List_Ptr=Pipe_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Pipe_List_Head))
    {
        /* If we can find a match, abort */
        if(Sys_Strcmp(Pipe_Name,((struct Pipe*)Traverse_List_Ptr)->Pipe_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            return (((struct Pipe*)Traverse_List_Ptr)->Pipe_ID);
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    /* If it gets here, then no match is found */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOPIPE);
    return -1;
}
#endif
/* End Function:Sys_Get_Pipe_ID **********************************************/

/* Begin Function:Sys_Open_Pipe ***********************************************
Description : Open the other side of the pipe. The PID registered
              is automatically the "Current_PID".
Input       : pipeid_t Msg_Queue_ID - The identifier of the pipe.
Output      : size_t* Buffer_Size - The buffer size. This is optional.
              void** Pipe_Buffer_Ptr - The pointer to the pipe buffer.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
retval_t Sys_Open_Pipe(pipeid_t Pipe_ID,size_t* Buffer_Size,void** Pipe_Buffer_Ptr)
{
    return(_Sys_Open_Pipe(Current_PID,Pipe_ID,Buffer_Size,Pipe_Buffer_Ptr));
}
#endif
/* End Function:Sys_Open_Pipe ************************************************/

/* Begin Function:_Sys_Open_Pipe **********************************************
Description : Open the other side of the pipe in the name of some process. Not
              recommended for application use. We don't check if the PID is valid here,
              because we may open the pipe in the name of some non-existent process.
Input       : pid_t Opener_PID - The PID of the desired opener.
              pipeid_t Msg_Queue_ID - The identifier of the pipe.    
Output      : size_t* Buffer_Size - The buffer size. This is optional.
              void** Pipe_Buffer_Ptr - The pointer to the pipe buffer.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
retval_t _Sys_Open_Pipe(pid_t Opener_PID,pipeid_t Pipe_ID,size_t* Buffer_Size,void** Pipe_Buffer_Ptr)
{
    /* See if the pipe ID is over the boundary */
    if(Pipe_ID>=MAX_PIPES)
    {
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the pipe exists in the system */
    if(Pipe_CB[Pipe_ID].Pipe_Buffer_Size==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    /* If both sides of the pipe is opened, abort */
    if((Pipe_CB[Pipe_ID].Opener_PID[0]!=PIPECLOSE)&&
       (Pipe_CB[Pipe_ID].Opener_PID[1]!=PIPECLOSE))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EPIPEINUSE);
        return (-1);
    }
    
    /* If the process try to open both sides of the same pipe, abort */
    if((Pipe_CB[Pipe_ID].Opener_PID[0]==Opener_PID)||
       (Pipe_CB[Pipe_ID].Opener_PID[1]==Opener_PID))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    /* Now we can open one side of the pipe. We have made sure that the pipe must have one 
     * side closed, and the other side is not opened by itself.
     */
    if(Pipe_CB[Pipe_ID].Opener_PID[0]==PIPECLOSE)
        Pipe_CB[Pipe_ID].Opener_PID[0]=Opener_PID;
    else if(Pipe_CB[Pipe_ID].Opener_PID[1]==PIPECLOSE)
        Pipe_CB[Pipe_ID].Opener_PID[1]=Opener_PID;
    
    /* Output the size(if needed) and the buffer address. We can use the pipe now */
    if(Buffer_Size!=0)
        *Buffer_Size=Pipe_CB[Pipe_ID].Pipe_Buffer_Size;
    *Pipe_Buffer_Ptr=(void*)(Pipe_CB[Pipe_ID].Pipe_Buffer_Addr);
    
    Sys_Unlock_Scheduler();
    return(0);
}
#endif
/* End Function:_Sys_Open_Pipe ***********************************************/

/* Begin Function:Sys_Close_Pipe **********************************************
Description : Close one side of the pipe. The PID is automatically the "Current_PID".
Input       : pipeid_t Pipe_ID - The identifier of the pipe.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
retval_t Sys_Close_Pipe(pipeid_t Pipe_ID)
{
    return(_Sys_Close_Pipe(Current_PID,Pipe_ID));
}
#endif
/* End Function:Sys_Close_Pipe ***********************************************/

/* Begin Function:_Sys_Close_Pipe *********************************************
Description : Close one side of the pipe in the name of some process. Not
              recommended for application use.
Input       : pid_t Closer_PID - The PID of the pipe closer.
              pipeid_t Pipe_ID - The identifier of the pipe.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
retval_t _Sys_Close_Pipe(pid_t Closer_PID,pipeid_t Pipe_ID)
{
    /* See if the pipe ID is over the boundary */
    if(Pipe_ID>=MAX_PIPES)
    {
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the pipe exists in the system */
    if(Pipe_CB[Pipe_ID].Pipe_Buffer_Size==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    /* Close the pipe */
    if(Pipe_CB[Pipe_ID].Opener_PID[0]==Closer_PID)
        Pipe_CB[Pipe_ID].Opener_PID[0]=PIPECLOSE;
    else if(Pipe_CB[Pipe_ID].Opener_PID[1]==Closer_PID)
        Pipe_CB[Pipe_ID].Opener_PID[1]=PIPECLOSE;
    else
    {
        /* If no side of the pipe ever opened by it, abort */
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVPIPE);
        return (-1);
    }
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:_Sys_Close_Pipe **********************************************/

/* Begin Function:Sys_Close_All_Pipes *****************************************
Description : Close all pipes the callee opened. The PID is automatically the "Current_PID".
Input       : pipeid_t Pipe_ID - The identifier of the pipe.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
void Sys_Close_All_Pipes(void)
{
    _Sys_Close_All_Pipes(Current_PID);
}
#endif
/* End Function:Sys_Close_All_Pipes ******************************************/

/* Begin Function:_Sys_Close_All_Pipes ****************************************
Description : Close all pipes that a certain process opened. Here we will traverse
              the pipe list and close all pipes.
Input       : pid_t PID - The PID of the pipe closer.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
void _Sys_Close_All_Pipes(pid_t PID)
{
    struct List_Head* Traverse_List_Ptr;
    
    Sys_Lock_Scheduler();
    /* Traverse the list to see if the name exists */
    Traverse_List_Ptr=Pipe_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Pipe_List_Head))
    {
        /* If we can find a match, abort */
        if(((struct Pipe*)Traverse_List_Ptr)->Opener_PID[0]==PID)
            ((struct Pipe*)Traverse_List_Ptr)->Opener_PID[0]=PIPECLOSE;
        if(((struct Pipe*)Traverse_List_Ptr)->Opener_PID[1]==PID)
            ((struct Pipe*)Traverse_List_Ptr)->Opener_PID[1]=PIPECLOSE;
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Close_All_Pipes *****************************************/

/* Begin Function:Sys_Query_Pipe_Number ***************************************
Description : Query the number of pipes existing in the system.
Input       : None.
Output      : None.
Return      : size_t - The number of pipes in the system.
******************************************************************************/
#if(ENABLE_PIPE==TRUE)
size_t Sys_Query_Pipe_Number(void)
{
    return Pipe_In_Sys;
}
#endif
/* End Function:Sys_Query_Pipe_Number ****************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
