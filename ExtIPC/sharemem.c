/******************************************************************************
Filename    : sharemem.c
Author      : pry
Date        : 03/07/2013
Version     : 0.02
Description : The shared memory module for the OS. The shared memory module has
              no mechanism to prevent a process from wrongly accessing a shared
              memory area that has been destroyed. So, when using the shared memory
              IPC, please plan the communication protocol carefully. If needed, you
              can use other IPC (signal, etc.) to synchronize the shared memory IPC.
              REMEMBER to detach process from the shared memory before it terminates!
              Or a system fault will occur, because the system won't automatically
              detach the process from the shared memory when it terminates!
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Memmgr\memory.h"
#include "ExtIPC\sharemem.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Memmgr\memory.h"
#include "ExtIPC\sharemem.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "ExtIPC\sharemem.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#include "Memmgr\memory.h"
#include "ExtIPC\sharemem.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Shm_Init ***********************************************
Description : Initialize the shared memory module.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Shm_Init(void)									   
{
#if(ENABLE_SHMEM==TRUE)	
   cnt_t Shm_Cnt;
    
   /* Initialize the lists */
   Sys_Create_List(&Shm_List_Head);
   Sys_Create_List(&Shm_Empty_List_Head);
   /* Clear the control block */
   Sys_Memset((ptr_int_t)Shm_CB,0,MAX_SHMS*sizeof(struct Shared_Mem));
    
   /* Put the blocks into the empty list */
   for(Shm_Cnt=0;Shm_Cnt<MAX_SHMS;Shm_Cnt++)
   {
       Shm_CB[Shm_Cnt].Shm_ID=Shm_Cnt;
       Sys_List_Insert_Node(&(Shm_CB[Shm_Cnt].Head),&Shm_Empty_List_Head,Shm_Empty_List_Head.Next);
   }
   
   /* Clear the statistical variable */
   Shm_In_Sys=0;
#endif
}

/* End Function:_Sys_Shm_Init ************************************************/

/* Begin Function:Sys_Shm_Create **********************************************
Description : Create a shared memory region in the system.
Input       : s8* Shm_Name - The name of the shared memory block.
              size_t Shm_Size - The size of the shared memory block.
Output      : None.
Return      : shmid_t - The ID of the shared memory block. If the function fail,
                        the value will be -1.
******************************************************************************/
#if(ENABLE_SHMEM==TRUE)
shmid_t Sys_Shm_Create(s8* Shm_Name,size_t Shm_Size)									   
{	
    shmid_t Shm_ID;
    void* Shm_Ptr;
    struct List_Head* Traverse_List_Ptr;
    /* See if the name is an empty pointer, or the buffer size wrong */
    if((Shm_Name==0)||(Shm_Size==0))
    {
        Sys_Set_Errno(EINVSHM);
        return -1;
    }
    
    Sys_Lock_Scheduler();
    
    /* See if there are any empty blocks */
    if((ptr_int_t)(&Shm_Empty_List_Head)==(ptr_int_t)(Shm_Empty_List_Head.Next))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOESHM);
        return -1;
    }
    
    /* If there are, get the shared memory block ID. We may only use this later */
    Shm_ID=((struct Shared_Mem*)Shm_Empty_List_Head.Next)->Shm_ID;
    
    /* Traverse the list to see if the name is unique */
    Traverse_List_Ptr=Shm_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Shm_List_Head))
    {
        /* If we can find a match, abort */
        if(Sys_Strcmp(Shm_Name,((struct Shared_Mem*)Traverse_List_Ptr)->Shm_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(ESHMEXIST);
            return -1;
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }

    /* See if we can allocate any memory from the system. The memory allocation is
     * done in the name of "Init".
     */
    Shm_Ptr=_Sys_Malloc(0,Shm_Size);
    
    /* Allocation failed */
    if(Shm_Ptr==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOESHM);
        return -1;
    }
    
    /* Now we are sure that we have enough resource to set up the shared memory */
    Sys_List_Delete_Node(Shm_CB[Shm_ID].Head.Prev,Shm_CB[Shm_ID].Head.Next);
    Sys_List_Insert_Node(&(Shm_CB[Shm_ID].Head),&Shm_List_Head,Shm_List_Head.Next);
    
    /* Register the values */
    Shm_CB[Shm_ID].Shm_Name=Shm_Name;
    Shm_CB[Shm_ID].Shm_Size=Shm_Size;
    Shm_CB[Shm_ID].Shm_Addr=(ptr_int_t)Shm_Ptr;
    
    /* Update the statistical variable */
    Shm_In_Sys++;
    
    Sys_Unlock_Scheduler();
    return Shm_ID;
}
#endif
/* End Function:Sys_Shm_Create ***********************************************/

/* Begin Function:Sys_Shm_Destroy *********************************************
Description : Destroy a shared memory block in the system. Only when no process
              is attached to the memory block can we destroy it.
Input       : shmid_t Shm_ID - The shared memory block's ID.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_SHMEM==TRUE)
retval_t Sys_Shm_Destroy(shmid_t Shm_ID)									   
{	
    /* See if the shared memory ID is over the boundary */
    if(Shm_ID>=MAX_SHMS)
    {
        Sys_Set_Errno(EINVSHM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the shared memory exists in the system */
    if(Shm_CB[Shm_ID].Shm_Size==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVSHM);
        return (-1);
    }
    
    /* If the shared memory is still attached to some process, abort */
    if(Shm_CB[Shm_ID].Shm_Attach_Cnt!=0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ESHMINUSE);
        return (-1);
    }
    
    /* Now we can safely destroy the shared memory block */
    Sys_List_Delete_Node(Shm_CB[Shm_ID].Head.Prev,Shm_CB[Shm_ID].Head.Next);
    Sys_List_Insert_Node(&(Shm_CB[Shm_ID].Head),&Shm_Empty_List_Head,Shm_Empty_List_Head.Next);
    
    /* Free shared memory */
    _Sys_Mfree(0,(void*)(Shm_CB[Shm_ID].Shm_Addr));
    
    /* Clear the variables. We have made sure that the "Shm_Attach_Cnt" is 0, so there's
     * no need to clear it again.
     */
    Shm_CB[Shm_ID].Shm_Name=0;
    Shm_CB[Shm_ID].Shm_Size=0;
    Shm_CB[Shm_ID].Shm_Addr=0;
    
    /* Update statistical variable */
    Shm_In_Sys--;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Shm_Destroy **********************************************/

/* Begin Function:Sys_Shm_Get_ID **********************************************
Description : Get the shared memory ID from its name.
Input       : s8* Shm_Name - The name of the shared memory region. 
Output      : None.
Return      : shmid_t - The ID of the shared memory block.
******************************************************************************/
#if(ENABLE_SHMEM==TRUE)
shmid_t Sys_Shm_Get_ID(s8* Shm_Name)							   
{	
     struct List_Head* Traverse_List_Ptr;
    
    Sys_Lock_Scheduler();
    /* Traverse the list to see if the name exists */
    Traverse_List_Ptr=Shm_List_Head.Next;
    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)(&Shm_List_Head))
    {
        /* If we can find a match, abort */
        if(Sys_Strcmp(Shm_Name,((struct Shared_Mem*)Traverse_List_Ptr)->Shm_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            return (((struct Shared_Mem*)Traverse_List_Ptr)->Shm_ID);
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    /* If it gets here, then no match is found */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOSHM);
    return -1;
}
#endif
/* End Function:Sys_Shm_Get_ID ***********************************************/

/* Begin Function:Sys_Shm_Attach **********************************************
Description : Attach a process from a shared memory block. TAKE NOTE when you try 
              to attach from the same shared memory block, it will always succeed,
              and a system fault will occur when you do it more than once. Thus
              REMEMBER to attach a process from the shared memory EXACTLY ONCE.
Input       : shmid_t Shm_ID - The ID of the shared memory.
Output      : size_t* Shm_Size - The size of the shared memory.
              void** Shm_Addr - The address of the shared memory region.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_SHMEM==TRUE)
retval_t Sys_Shm_Attach(shmid_t Shm_ID,size_t* Shm_Size,void** Shm_Addr)
{	
     /* See if the shared memory ID is over the boundary */
    if(Shm_ID>=MAX_SHMS)
    {
        Sys_Set_Errno(EINVSHM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the shared memory exists in the system */
    if(Shm_CB[Shm_ID].Shm_Size==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVSHM);
        return (-1);
    }
    
    /* Now attach this process to the shared memory region. In fact the process
     * is not internally registered at all!
     */
    Shm_CB[Shm_ID].Shm_Attach_Cnt++;
    *Shm_Size=Shm_CB[Shm_ID].Shm_Size;
    *Shm_Addr=(void*)Shm_CB[Shm_ID].Shm_Addr;
    
	return 0;
}
#endif
/* End Function:Sys_Shm_Attach ***********************************************/

/* Begin Function:Sys_Shm_Detach **********************************************
Description : Detach a process from a shared memory block. TAKE NOTE when you try 
              to detach from the same shared memory block , it will always succeed,
              and a system fault will occur when you do it more than once. Thus
              REMEMBER to detach a process from the shared memory EXACTLY ONCE.
Input       : shmid_t Shm_ID - The identifier of the shared memory.
Output      : None.
Return      : retval_t - If successful,0; else -1.
******************************************************************************/
#if(ENABLE_SHMEM==TRUE)
retval_t Sys_Shm_Detach(shmid_t Shm_ID)
{
     /* See if the shared memory ID is over the boundary */
    if(Shm_ID>=MAX_SHMS)
    {
        Sys_Set_Errno(EINVSHM);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    /* See if the shared memory exists in the system */
    if(Shm_CB[Shm_ID].Shm_Size==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(EINVSHM);
        return (-1);
    }
    
    /* Now we can detach the process to the shared memory region. In fact the 
     * process is not internally registered at all!
     */
    Shm_CB[Shm_ID].Shm_Attach_Cnt--;
    
    return 0;
}
#endif
/* End Function:Sys_Shm_Detach ***********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
