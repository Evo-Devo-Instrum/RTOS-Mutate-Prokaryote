 /*****************************************************************************
Filename    : msgqueue.c
Author      : pry
Date        : 25/04/2012
Version     : 0.12
Description : The queue module for the OS. The queue use a special method so
              that we don't have to copy the message into the message queue
              buffer. Thus, we can save time as well as space.
              Before enabling the message queue module, you must enable the
              dynamic memory management module first, or it will fail.
              When we are waiting for a certain message queue, the implications are:
              1>When the message queue is deleted, then the wait will return when
                it expires;
              2>When the message queue have messages for the process, then the
                wait function will return, and you need to get the message manually;
                Take note not to let any process that receive all kinds of message get the
                message before you; This is the user's responsibility.
              3>When the message queue is empty, the process will wait until a message
                for it comes.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Memmgr\memory.h"
#include "ExtIPC\msgqueue.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Memmgr\memory.h"
#include "ExtIPC\msgqueue.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "ExtIPC\msgqueue.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Kernel\error.h"
#include "Memmgr\memory.h"
#include "Syslib\syslib.h"
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\wait.h"
#include "Syssvc\timer.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Queue_Init *********************************************
Description : The initialization function for the system queue module.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Queue_Init(void)
{
#if(ENABLE_MSGQ==TRUE)
    cnt_t Msg_Queue_Count;
    
    /* Initiailize the queue control block list */
    Sys_Create_List(&Empty_Msg_CB_List_Head);
    Sys_Create_List(&Msg_CB_List_Head);
    
    /* Initialize the message queue control block */
    Sys_Memset((ptr_int_t)Msg_CB,'0',MAX_MSG_QUEUES*sizeof(struct Msg_Queue));
    
    for(Msg_Queue_Count=0;Msg_Queue_Count<MAX_MSG_QUEUES;Msg_Queue_Count++)
    {
        Sys_List_Insert_Node(&(Msg_CB[Msg_Queue_Count].Head),
                             &Empty_Msg_CB_List_Head,
                             Empty_Msg_CB_List_Head.Next);
        /* Initiate the block list for each possible message queue */
        Sys_Create_List(&(Msg_CB[Msg_Queue_Count].Msg_Block_Queue_Head));
        Sys_Create_List(&(Msg_CB[Msg_Queue_Count].Msg_Block_Empty_Head));
        Sys_Create_List(&(Msg_CB[Msg_Queue_Count].Wait_Object_Head));
        /* Initialize the message queue identifier */
        Msg_CB[Msg_Queue_Count].Msg_Queue_ID=Msg_Queue_Count;
    }
    
    /* Clear the statisticaal variable */
    Msg_Queue_In_Sys=0;
#endif    
}
/* End Function:_Sys_Queue_Init **********************************************/

/* Begin Function:Sys_Create_Queue ********************************************
Description : Create a queue and return the unique queue ID of it.
Input       : s8* Msg_Queue_Name - The name of the message queue.
              size_t Msg_Number - The number of messages in the message queue.
Output      : None.
Return      : msgqid_t - The identifier of the message queue. If the function fail,
                         it will return -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
msgqid_t Sys_Create_Queue(s8* Msg_Queue_Name,size_t Msg_Number)
{
    msgqid_t Msg_Queue_ID;
    cnt_t Msg_Block_Count;
    struct List_Head* Traverse_Ptr;
    void* Msg_Queue_Start_Addr;
    struct Msg_Block* Msg_Block_Ptr;
    
    Sys_Lock_Scheduler();
    
    /* See if the name is unique */
    Traverse_Ptr=Msg_CB_List_Head.Next;
    
    /* See if the name is unique in the system. If not, the semaphore cannot be created.
     * to do this we have to traverse the whole list and make comparison on every created
     * semaphore in the system.
     */
    while(Traverse_Ptr!=&Msg_CB_List_Head)
    {
        /* If we found a match */
        if(Sys_Strcmp(Msg_Queue_Name,((struct Msg_Queue*)Traverse_Ptr)->Msg_Queue_Name,MAX_STR_LEN)==0)
        {
            Sys_Unlock_Scheduler();
            Sys_Set_Errno(ENOEMSGQ);
            return (-1);
        }
        Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    /* See if there are any empty message queue control blocks */
    if((ptr_int_t)(Empty_Msg_CB_List_Head.Next)==(ptr_int_t)(&Empty_Msg_CB_List_Head))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOEMSGQ);
        return (-1);
    }
        
    /* The malloc is in the name of the "Init" process */
    Msg_Queue_Start_Addr=_Sys_Malloc(0,Msg_Number*sizeof(struct Msg_Block));
    
    /* If the malloc function failed, return now */
    if(Msg_Queue_Start_Addr==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOEMSGQ);
        return (-1);
    }
        
    Msg_Queue_ID=((struct Msg_Queue*)(Empty_Msg_CB_List_Head.Next))->Msg_Queue_ID;
    
    /* Delete node from the empty list and insert it into the using list */
    Sys_List_Delete_Node(Empty_Msg_CB_List_Head.Next->Prev,Empty_Msg_CB_List_Head.Next->Next);
    Sys_List_Insert_Node(&(Msg_CB[Msg_Queue_ID].Head),&Msg_CB_List_Head,Msg_CB_List_Head.Next);
    
    /* Fill in the information */
    Msg_CB[Msg_Queue_ID].Msg_Queue_Name=Msg_Queue_Name;
    Msg_CB[Msg_Queue_ID].Msg_Max_Number=Msg_Number;
    Msg_CB[Msg_Queue_ID].Msg_Cur_Number=0;
    Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number=0;
    Msg_CB[Msg_Queue_ID].Msg_Queue_Start_Addr=(ptr_int_t)Msg_Queue_Start_Addr;

    Msg_Block_Ptr=(struct Msg_Block*)(Msg_Queue_Start_Addr);
    /* Initialize the memory area as the message queue */
    Sys_Memset((ptr_int_t)Msg_Queue_Start_Addr,0,Msg_Number*sizeof(struct Msg_Block));
    
    for(Msg_Block_Count=0;Msg_Block_Count<Msg_Number;Msg_Block_Count++)
    {
        Sys_List_Insert_Node(&(Msg_Block_Ptr->Head),
                             &(Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head),
                             Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head.Next);
        /* The two IDs below are for querying the message block property from its
         * pointer
         */
        Msg_Block_Ptr->Msg_Block_List_ID=MSG_BLOCK_IN_EMPTY;
        Msg_Block_Ptr->Msg_Queue_ID=Msg_Queue_ID;
        Msg_Block_Ptr->Msg_Block_ID=Msg_Block_Count;
        /* Clear the message type */
        Msg_Block_Ptr->Msg_Type=0;
        /* Clear the message address */
        Msg_Block_Ptr->Msg_Addr_Ptr=0;
        Msg_Block_Ptr++;
    }
    
    /* Increase the statistical variable */
    Msg_Queue_In_Sys++;
    
    Sys_Unlock_Scheduler();
    return Msg_Queue_ID;
}
#endif
/* End Function:Sys_Create_Queue *********************************************/

/* Begin Function:Sys_Destroy_Queue *******************************************
Description : Destroy a message queue given the ID of the queue.
              Different from the semaphore module, when a queue still have message
              stored in it, you can destroy it at once, abandoning the messages.
              However, this requires no message is being read in the queue, that is
              to say, all allocated message blocks in the queue are destroyed so that
              no process are operating the queue anymore.
Input       : msgqid_t Msg_Queue_ID - The identifier of the message queue.
Output      : None.
Return      : retval_t - If successful, it will return 0; else -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
retval_t Sys_Destroy_Queue(msgqid_t Msg_Queue_ID)
{
    struct List_Head* Traverse_Block_Ptr;
    
    /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
    {
        Sys_Set_Errno(ENOMSGQ);
        return -1;
    }
    
    Sys_Lock_Scheduler();
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGQ);
        return -1;
    }
    
    /* See if the queue's messages are completely destroyed */
    if(Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number!=0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGQ);
        return -1;
    }
    
    /* Free memory and destroy the queue */
    /* See if there are still messages in the queue and free the memory each
     * message allocated.
     */
    Traverse_Block_Ptr=(struct List_Head*)(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head.Next);
    while((ptr_int_t)(Traverse_Block_Ptr)!=(ptr_int_t)(&Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head))
    {
        _Sys_Mfree(0,(void*)(((struct Msg_Block*)Traverse_Block_Ptr)->Msg_Addr_Ptr));
        Traverse_Block_Ptr=Traverse_Block_Ptr->Next;
    }
    
    /* Free the message block list itself */
    _Sys_Mfree(0,(void*)(Msg_CB[Msg_Queue_ID].Msg_Queue_Start_Addr));
    
    /* Free the control block */
    Sys_List_Delete_Node(Msg_CB[Msg_Queue_ID].Head.Prev,
                         Msg_CB[Msg_Queue_ID].Head.Next);
    
    Sys_List_Insert_Node(&(Msg_CB[Msg_Queue_ID].Head),
                         &(Empty_Msg_CB_List_Head),
                         Empty_Msg_CB_List_Head.Next);
    
    /* Clear the block parameters but don't clear the queue ID */
    Msg_CB[Msg_Queue_ID].Msg_Queue_Name=0;
    Msg_CB[Msg_Queue_ID].Msg_Max_Number=0;
    Msg_CB[Msg_Queue_ID].Msg_Cur_Number=0;
    Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number=0;
    Msg_CB[Msg_Queue_ID].Msg_Queue_Start_Addr=0;

    /* Update the statistical variable */
    Msg_Queue_In_Sys--;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Destroy_Queue ********************************************/

/* Begin Function:Sys_Get_Queue_ID ********************************************
Description : Get the queue's unique identifier through the name of it. 
              If the queue exists, return the identifier.
Input       : s8* Msg_Queue_Name - The name of the message queue.
Output      : None.
Return      : msgqid_t - The identifier of the message queue. If the queue doesn't 
                         exist, then return -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
msgqid_t Sys_Get_Queue_ID(s8* Msg_Queue_Name)
{
    struct List_Head* Traverse_Ptr;    
    Sys_Lock_Scheduler();
    
    /* See if the name exists */
    Traverse_Ptr=Msg_CB_List_Head.Next;
    while(Traverse_Ptr!=&Msg_CB_List_Head)
    {
        /* If we found a match */
        if(Sys_Strcmp(Msg_Queue_Name,
                      ((struct Msg_Queue*)Traverse_Ptr)->Msg_Queue_Name,
                      MAX_STR_LEN)==0)
        {
            /* We found the match */
            Sys_Unlock_Scheduler();
            return (msgqid_t)(((struct Msg_Queue*)Traverse_Ptr)->Msg_Queue_ID);
        }
        Traverse_Ptr=Traverse_Ptr->Next;
    }
    
    /* If it can get here, then nothing is found */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOMSGQ);
    return -1;
}
#endif
/* End Function:Sys_Get_Queue_ID *********************************************/

/* Begin Function:Sys_Alloc_Msg ***********************************************
Description : Allocate space for the message and prepare to send it. The sender's
              PID is automatically the "Current_PID".
Input       : msgqid_t Msg_Queue_ID - The identifier of the message queue.
              msgtyp_t Msg_Type - The type of the message.
              size_t Msg_Size - The name of the message queue.
Output      : void** Msg_Buffer_Ptr - The pointer to the message buffer.
Return      : msgqbid_t - The identifier of the message block. We use this Msg_Block_ID
                          to specify the message. Should the function fail, it will
                          return -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
msgqbid_t Sys_Alloc_Msg(msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,size_t Msg_Size,void** Msg_Buffer_Ptr)
{
    return(_Sys_Alloc_Msg(Current_PID,Msg_Queue_ID,Msg_Type,Msg_Size,Msg_Buffer_Ptr));
}
#endif
/* End Function:Sys_Alloc_Msg ************************************************/

/* Begin Function:_Sys_Alloc_Msg **********************************************
Description : Allocate space for the message and prepare to send it.
Input       : pid_t Sender_PID - The PID of the message sender.
              msgqid_t Msg_Queue_ID - The identifier of the message queue.
              msgtyp_t Msg_Type - The type of the message. In fact it is an 32-bit
                             unsigned number.
              size_t Msg_Size - The name of the message queue.
Output      : void** Msg_Buffer_Ptr - The pointer to the message buffer.
Return      : msgqbid_t - The identifier of the message block. We use this Msg_Block_ID
                          to specify the message. Should the function fail, it will
                          return -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
msgqbid_t _Sys_Alloc_Msg(pid_t Sender_PID,msgqid_t Msg_Queue_ID,
                         msgtyp_t Msg_Type,size_t Msg_Size,void** Msg_Buffer_Ptr)
{
    void* Msg_Malloc_Ptr;
    struct Msg_Block* Msg_Block_Ptr;
    
    /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
    {
        Sys_Set_Errno(ENOMSG);
        return -1;
    }
        
    
    Sys_Lock_Scheduler();
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSG);
        return -1;
    }
    
    /* See if there are any spare blocks */
    if((ptr_int_t)(&(Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head))==
       (ptr_int_t)(Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head.Next))
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSG);
        return -1;
    }
    
    /* See if we can allocate enough memory for the message */
    Msg_Malloc_Ptr=_Sys_Malloc(Sender_PID,Msg_Size);
    /* Cannot allocate memory, return now */
    if(Msg_Malloc_Ptr==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSG);
        return -1;
    }
    
    *Msg_Buffer_Ptr=Msg_Malloc_Ptr;
    /* Now we're sure that there are spare blocks and we have allocated the space */
    /* Detach an spare block from the block list */
    Msg_Block_Ptr=(struct Msg_Block*)(Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head.Next);
    Sys_List_Delete_Node(Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head.Next->Prev,
                         Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head.Next->Next);
    
    /* Fill the block registry contents */
    Msg_Block_Ptr->Msg_Block_List_ID=MSG_BLOCK_NO_LIST;
    Msg_Block_Ptr->Msg_Type=Msg_Type;
    Msg_Block_Ptr->Msg_Addr_Ptr=(ptr_int_t)Msg_Malloc_Ptr;
    Msg_Block_Ptr->Msg_Send_PID=Sender_PID;
    
    /* Fill the statistical data */
    Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number++;
    
    Sys_Unlock_Scheduler();
    return(Msg_Block_Ptr->Msg_Block_ID);
}
#endif
/* End Function:_Sys_Alloc_Msg ***********************************************/

/* Begin Function:Sys_Send_Msg ************************************************
Description : Send the message to a certain destination. The sender ID is already
              specified when allocating the message area.
Input       : pid_t Recver_PID - The receiver's PID. 
              msgqid_t Msg_Queue_ID - The ID of the message queue.
              msgqbid_t Msg_Block_ID - The ID of the specific message block.
Output      : None.
Return      : retval_t - If the function fails,-1; else 0.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
retval_t Sys_Send_Msg(pid_t Recver_PID,msgqid_t Msg_Queue_ID,msgqbid_t Msg_Block_ID)
{
    struct Msg_Block* Msg_Block_Ptr;
    struct List_Head* Traverse_List_Ptr;
    pid_t Wait_PID;
    
    /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
    {
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }  
    
    Sys_Lock_Scheduler();
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    /* See if the message block is over the boundary */
    if(Msg_Block_ID>=Msg_CB[Msg_Queue_ID].Msg_Max_Number)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    Msg_Block_Ptr=&(((struct Msg_Block*)Msg_CB[Msg_Queue_ID].Msg_Queue_Start_Addr)[Msg_Block_ID]);
    /* See if the message block is already initialized (ready to send) */
    if(Msg_Block_Ptr->Msg_Addr_Ptr==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    /* Now the block is ready to send, send it. */
    /* Remember that we have detached the block from the empty-message-block-list,
     * there's no need to detach it again. We just fill in the information.
     */
    Msg_Block_Ptr->Msg_Block_List_ID=MSG_BLOCK_IN_QUEUE;
    Msg_Block_Ptr->Msg_Recv_PID=Recver_PID;
    Sys_List_Insert_Node(&(Msg_Block_Ptr->Head),
                         &(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head),
                         Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head.Next);
    
    Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number--;
    Msg_CB[Msg_Queue_ID].Msg_Cur_Number++;
    
    /* Now see if there is any process need notifying */
    Traverse_List_Ptr=Msg_CB[Msg_Queue_ID].Wait_Object_Head.Next;
    while(Traverse_List_Ptr!=&Msg_CB[Msg_Queue_ID].Wait_Object_Head)
    {
        if(Recver_PID==((struct Wait_Object_Struct*)(Traverse_List_Ptr-1))->PID)
        {
            /* Get the correct PID */
            Wait_PID=((struct Wait_Object_Struct*)(Msg_CB[Msg_Queue_ID].Wait_Object_Head.Next-1))->PID;
            /* Mark that the wait is successful */
            ((struct Wait_Object_Struct*)(Msg_CB[Msg_Queue_ID].Wait_Object_Head.Next-1))->Succeed_Flag=1;
            /* Delete the wait block from the semaphore wait list */
            Sys_List_Delete_Node(&Msg_CB[Msg_Queue_ID].Wait_Object_Head,Msg_CB[Msg_Queue_ID].Wait_Object_Head.Next->Next);
            /* Try to stop the timer if possible */
            Sys_Proc_Delay_Cancel(Wait_PID);
            /* Wake the process up */
            _Sys_Set_Ready(Wait_PID);
            
            /* Since there will be only one process for the PID */
            break;
        }
    }
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Send_Msg *************************************************/

/* Begin Function:Sys_Recv_Msg ************************************************
Description : Receive the first message we occur with. The receiver's ID is 
              automatically the "Current_PID".
Input       : msgqid_t Msg_Queue_ID - The identifier of the message queue.
              msgtyp_t Msg_Type - The type of the message.
Output      : void** Msg_Buffer_Ptr - The pointer to the message buffer.
Return      : msgqbid_t - The identifier of the message block. When we finish "reading"
                          the message, the only way to destroy it requires this.
                          If there's no message, it will be -1 (failure).
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
msgqbid_t Sys_Recv_Msg(msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,void** Msg_Buffer_Ptr)
{
    return(_Sys_Recv_Msg(Current_PID,Msg_Queue_ID,Msg_Type,Msg_Buffer_Ptr));
}
#endif
/* End Function:Sys_Recv_Msg *************************************************/

/* Begin Function:_Sys_Recv_Msg ***********************************************
Description : Receive the first message we occur with. The receiver's ID can be
              specified, so that we can read the message in the name of others.
              If the queue exists, return the identifier. The receiver 
Input       : pid_t Recver_PID - The receiver's PID.
              msgqid_t Msg_Queue_ID - The identifier of the message queue.
              msgtyp_t Msg_Type - The message type. If it equals "ALL_TYPE_MSG"(0xFFFFFFFF)
                                  then all kinds of message will be received.
Output      : void** Msg_Buffer_Ptr - The pointer to the message buffer.
Return      : msgqbid_t - The identifier of the message block. When we finish "reading"
                          the message, the only way to destroy it requires this.
                          When the function fails, it will return -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
msgqbid_t _Sys_Recv_Msg(pid_t Recver_PID,msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type,void** Msg_Buffer_Ptr)
{
    struct List_Head* Traverse_List_Ptr;
    
    /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
    {
        Sys_Set_Errno(ENOMSGBLK);
        return (-1);
    }
    
    Sys_Lock_Scheduler();
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return (-1);
    }
    
    /* See if there are any message in the queue */
    if(Msg_CB[Msg_Queue_ID].Msg_Cur_Number==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return (-1);
    }
    
    /* See if any message is available */
    Traverse_List_Ptr=Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head.Next;
    while((ptr_int_t)(Traverse_List_Ptr)!=(ptr_int_t)(&(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head)))
    {
        if((((struct Msg_Block*)Traverse_List_Ptr)->Msg_Recv_PID)==Recver_PID)
        {
            if(Msg_Type!=ALL_TYPE_MSG)
            {
                /* If the process only want to get some kind of message */
                if((((struct Msg_Block*)Traverse_List_Ptr)->Msg_Type)==Msg_Type)
                {
                    /* Delete the message block from the message list */
                    Sys_List_Delete_Node(Traverse_List_Ptr->Prev,Traverse_List_Ptr->Next);
                    /* Refresh the list ID(flag) */
                    ((struct Msg_Block*)Traverse_List_Ptr)->Msg_Block_List_ID=MSG_BLOCK_NO_LIST;
                    /* Refresh the corresponding statistic variables */
                    Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number++;
                    Msg_CB[Msg_Queue_ID].Msg_Cur_Number--;
                    /* Output the buffer address */
                    *Msg_Buffer_Ptr=(void*)(((struct Msg_Block*)Traverse_List_Ptr)->Msg_Addr_Ptr);
                    
                    Sys_Unlock_Scheduler();
                    return(((struct Msg_Block*)Traverse_List_Ptr)->Msg_Block_ID);
                }
            }
            /* If the process want to get all kinds of messages */
            else
            {
                /* Delete the message block from the message list */
                Sys_List_Delete_Node(Traverse_List_Ptr->Prev,Traverse_List_Ptr->Next);
                /* Refresh the corresponding statistic variables */
                Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number++;
                Msg_CB[Msg_Queue_ID].Msg_Cur_Number--;
                /* Output the buffer address */
                *Msg_Buffer_Ptr=(void*)(((struct Msg_Block*)Traverse_List_Ptr)->Msg_Addr_Ptr);
                
                Sys_Unlock_Scheduler();
                return(((struct Msg_Block*)Traverse_List_Ptr)->Msg_Block_ID);
            }
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }
    
    /* It shouldn't get here; Add a assert. Remember that we have assured that the 
     * number of messages in the queue is bigger than 0.
     */
    Sys_Unlock_Scheduler();
    Sys_Set_Errno(ENOMSGBLK);
    return (-1);
}
#endif
/* End Function:_Sys_Recv_Msg ************************************************/

/* Begin Function:Sys_Destroy_Msg *********************************************
Description : Destroy a message block, given the message queue ID and the message
              block ID. We destroy a message block when we finish reading the message.
Input       : msgqid_t Msg_Queue_ID - The ID of the message queue.
              msgqbid_t Msg_Block_ID - The ID of the message block to be destroyed.
Output      : None.
Return      : If successful, 0; else -1.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
retval_t Sys_Destroy_Msg(msgqid_t Msg_Queue_ID,msgqbid_t Msg_Block_ID)
{
    struct Msg_Block* Msg_Block_Ptr;
    
     /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
    {
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    Sys_Lock_Scheduler();
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    /* See if the message block exists in the message queue */
    if(Msg_Block_ID>=Msg_CB[Msg_Queue_ID].Msg_Max_Number)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    /* See if the message block is already out of list - only under such condition
     * can we destroy it.
     */
    Msg_Block_Ptr=&(((struct Msg_Block*)Msg_CB[Msg_Queue_ID].Msg_Queue_Start_Addr)[Msg_Block_ID]);
    if(Msg_Block_Ptr->Msg_Block_List_ID!=MSG_BLOCK_NO_LIST)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOMSGBLK);
        return -1;
    }
    
    /* Destroy it and return the message block to the empty list */
    _Sys_Mfree(Msg_Block_Ptr->Msg_Send_PID,(void*)(Msg_Block_Ptr->Msg_Addr_Ptr));
    
    /* Mark it as in empty list and insert it into the empty list */
    Msg_Block_Ptr[Msg_Block_ID].Msg_Block_List_ID=MSG_BLOCK_IN_EMPTY;
    Sys_List_Insert_Node(&(Msg_Block_Ptr->Head),
                         &(Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head),
                         Msg_CB[Msg_Queue_ID].Msg_Block_Empty_Head.Next);
    
    /* Clear the related value */
    Msg_Block_Ptr->Msg_Type=0;
    Msg_Block_Ptr->Msg_Addr_Ptr=0;
    Msg_Block_Ptr->Msg_Send_PID=0;
    Msg_Block_Ptr->Msg_Recv_PID=0;
    
    /* Clear the statistical variables */
    Msg_CB[Msg_Queue_ID].Msg_Cur_Use_Number--;
    
    Sys_Unlock_Scheduler();
    return 0;
}
#endif
/* End Function:Sys_Destroy_Msg **********************************************/

/* Begin Function:Sys_Query_Msg_To_Recver *************************************
Description : Query the number of messages to a certain receiver.
Input       : pid_t PID - The receiver's process ID.
              msgqid_t Msg_Queue_ID - The ID of the message queue.
              msgtyp_t Msg_Type - The message type.
Output      : None.
Return      : size_t - If the message queue does not exist, it will also return 0.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
size_t Sys_Query_Msg_To_Recver(pid_t PID,msgqid_t Msg_Queue_ID,msgtyp_t Msg_Type)
{
    struct List_Head* Traverse_Block_List_Ptr;
    size_t Msg_Count;
    
     /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
        return 0;
    
    /* See if the PID is over the boundary */
    if(PID>MAX_PROC_NUM)
        return 0;
    
    Sys_Lock_Scheduler();
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        return 0;
    }
    
    /* See if the queue is empty */
    if(Msg_CB[Msg_Queue_ID].Msg_Cur_Number==0)
    {
        Sys_Unlock_Scheduler();
        return 0;
    }
    
    /* Now traverse the list and see if there are any message of the
     * corresponding type to the desired receiver.
     */
    Traverse_Block_List_Ptr=Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head.Next;
    Msg_Count=0;
    if(PID==ALL_PROC_MSG)
    {
        if(Msg_Type==ALL_TYPE_MSG)
        {
            Sys_Unlock_Scheduler();
            return(Msg_CB[Msg_Queue_ID].Msg_Cur_Number);
        }
        else
        {
            while((ptr_int_t)Traverse_Block_List_Ptr!=(ptr_int_t)&(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head))
            {
                if(((struct Msg_Block*)Traverse_Block_List_Ptr)->Msg_Type==Msg_Type)
                    Msg_Count++;
                Traverse_Block_List_Ptr=Traverse_Block_List_Ptr->Next;
            }
        }
    }
    else
    {
        if(Msg_Type==ALL_TYPE_MSG)
        {
            while((ptr_int_t)Traverse_Block_List_Ptr!=(ptr_int_t)&(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head))
            {
                if(((struct Msg_Block*)Traverse_Block_List_Ptr)->Msg_Recv_PID==PID)
                    Msg_Count++;
                Traverse_Block_List_Ptr=Traverse_Block_List_Ptr->Next;
            }
        }
        else
        {
            while((ptr_int_t)Traverse_Block_List_Ptr!=(ptr_int_t)&(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head))
            {
                if((((struct Msg_Block*)Traverse_Block_List_Ptr)->Msg_Type==Msg_Type)&&
                   (((struct Msg_Block*)Traverse_Block_List_Ptr)->Msg_Recv_PID==PID))
                    Msg_Count++;
                Traverse_Block_List_Ptr=Traverse_Block_List_Ptr->Next;
            }
        }
    }
    
    Sys_Unlock_Scheduler();
    return Msg_Count;
}
#endif
/* End Function:Sys_Query_Msg_To_Recver **************************************/

/* Begin Function:Sys_Query_Msg_Number ****************************************
Description : Query the number of messages in a message queue.
Input       : msgqid_t Msg_Queue_ID - The ID of the message queue.
Output      : None.
Return      : size_t - The number of messages in the message queue.
                      If the message queue does not exist, it will also return 0.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
size_t Sys_Query_Msg_Number(msgqid_t Msg_Queue_ID)
{
    /* See if the queue ID is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
        return 0;
    
    return Msg_CB[Msg_Queue_ID].Msg_Cur_Number;
}
#endif
/* End Function:Sys_Query_Msg_Number *****************************************/

/* Begin Function:Sys_Query_Msg_Queue_Number **********************************
Description : Get the total number of message queues existing in the system.
Input       : None.
Output      : None.
Return      : size_t - The number of messages queues in the system.
******************************************************************************/
#if(ENABLE_MSGQ==TRUE)
size_t Sys_Query_Msg_Queue_Number(msgqid_t Msg_Queue_ID)
{
    return Msg_Queue_In_Sys;
}
#endif
/* End Function:Sys_Query_Msg_Queue_Number ***********************************/

/* Begin Function:_Sys_Wait_Msg_Queue_Reg *****************************************
Description : When we decide to wait for a message queue, this function will be called.
              Take note that when the wait of message queue is successful,
              the corresponding message will not be detached from the message queue, and you need 
              to call other API to get the message by yourself.
Input       : pid_t PID - The process waiting for the message queue. We don't check whether the PID is
                          valid here.
              msgqid_t Msg_Queue_ID - The ID of the message queue(not the message!).
              struct Wait_Object_Struct* Wait_Block_Ptr - The pointer to the wait block.
Output      : None.
Return      : retval_t - If successful,0; if there's no need to wait, "NO_NEED_TO_WAIT(-2)";
                         if the wait failed, "WAIT_FAILURE(-1)".
******************************************************************************/
retval_t _Sys_Wait_Msg_Queue_Reg(pid_t PID,msgqid_t Msg_Queue_ID,
                                 struct Wait_Object_Struct* Wait_Block_Ptr)
{
    struct List_Head* Traverse_List_Ptr;
    
    Sys_Lock_Scheduler();
    
    /* See if the operation is over the boundary */
    if(Msg_Queue_ID>=MAX_MSG_QUEUES)
    {
        Sys_Unlock_Scheduler();
        return(WAIT_FAILURE);
    }
    
    /* See if the queue is existent in the system */
    if(Msg_CB[Msg_Queue_ID].Msg_Max_Number==0)
    {
        Sys_Unlock_Scheduler();
        return(WAIT_FAILURE);
    }
    
    /* See if there is a message for the process. If yes, return right away. */
    Traverse_List_Ptr=Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head.Next;

    while((ptr_int_t)Traverse_List_Ptr!=(ptr_int_t)&(Msg_CB[Msg_Queue_ID].Msg_Block_Queue_Head))
    {
        if(((struct Msg_Block*)Traverse_List_Ptr)->Msg_Recv_PID==PID)
        {
            Sys_Unlock_Scheduler();
            return(NO_NEED_TO_WAIT);
        }
        
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
    }

    
    /* Now it is clear that we did not get any message from the message queue. 
     * We would have to wait for the message to come(or the wait expires).
     * Now register the wait block under the correspoding message queue.
     */
    Sys_List_Insert_Node(&(Wait_Block_Ptr->Object_Head),
                         &Msg_CB[Msg_Queue_ID].Wait_Object_Head,
                         Msg_CB[Msg_Queue_ID].Wait_Object_Head.Next);
    
    Wait_Block_Ptr->Obj_ID=Msg_Queue_ID;
    Wait_Block_Ptr->PID=PID;
    Wait_Block_Ptr->Type=MSGQUEUE;
    
    Sys_Unlock_Scheduler();
    return 0;
}
/* Begin Function:_Sys_Wait_Sem_Reg ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
