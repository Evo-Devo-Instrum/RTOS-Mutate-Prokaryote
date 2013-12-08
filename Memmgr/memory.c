 /*****************************************************************************
Filename    : memory.c
Author      : pry
Date        : 25/04/2012
Version     : 0.12
Description : The DSA module for the OS. The module utilize TLSF method to 
              allocate memory, thus it is O(1). 
              The TLSF memory allocator is consisted of FLI, SLI and allocatable
              memory. The FLI is classified by 2^n, and the SLI segregates the 
              FLI section by an power of 2, i.e. 8 or 16. Thus, when we need 
              an memory block, we try to find it in the corresponding FLI, and
              then the SLI.(This is a two-dimensional matrix.) Then 
              (1) If the SLI has no allocatable blocks, we will allocate some
                  from the nearest bigger block.
              (2) If there is some block from the SLI block, allocate the memory
                  size and put the residue memory into the corresponding FLI and
                  SLI area.
              When we free memory, then the memory blocks will be automatically
              merge.
              
              In the system, the FLI is variable and the SLI is fixed to 8.
              The FLI has a miniumum block size of 64 Byte(If the allocated size
              is always smaller than 64 bits, then there's no need to use DSA.)
              To make sure that it is like this, we set the smallest allocatable
              size to 64B. In addition, we set the alignment to 8.
              [FLI]:
              .....    6       5      4       3         2        1         0
                     8K-4K   4K-2K  2K-1K  1K-512B  511-256B  255-128B  127-64B
                 
              For example, when a memory block is 720 byte, then it should be in
              FLI=3,SLI=3.
              
              When a lower FLI has no blocks for allocation, it will "borrow"
              a block from the nearest FLI block that is big enough.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Memmgr\memory.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Memmgr\memory.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Memmgr\memory.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\interrupt.h"
#include "Syslib\syslib.h"
#include "Memmgr\memory.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Mem_Init ***********************************************
Description : The initialization function for the system memory management module.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Mem_Init(void)
{
#if(ENABLE_MEMM==TRUE)
    cnt_t X_Cnt;
    cnt_t Y_Cnt;
    
    /* Initialize the TLSF allocation table first */
    for(X_Cnt=0;X_Cnt<MM_FLI;X_Cnt++)
    {
        for(Y_Cnt=0;Y_Cnt<8;Y_Cnt++)
        {
            Sys_Create_List(&(Mem_CB[X_Cnt][Y_Cnt]));
        }
        Mem_Bitmap[X_Cnt]=0;
    }
    
    /* Initialize the allocated memory block list */
    Sys_Create_List(&Mem_Allocated_List_Head);
    
    /* Initialize the only big memory block */
    _Sys_Mem_Init_Block((ptr_int_t)DMEM_Heap,DMEM_SIZE);
    
    /* Insert the memory into the corresponding level */
    _Sys_Mem_Ins_TLSF((struct Mem_Head*)DMEM_Heap);
    
    /* Now initialize the corresponding PCB struct now */
    for(X_Cnt=0;X_Cnt<MAX_PROC_NUM;X_Cnt++)
    {
        Sys_Create_List(&(PCB_Mem[X_Cnt].Head)); 
        PCB_Mem[X_Cnt].Memory_In_Use=0;
    }
    
    /* Clear the statistic variables */
    Free_Mem_Amount=DMEM_SIZE;
    Used_Mem_Amount=0;
#endif    
}
/* End Function:_Sys_Mem_Init ************************************************/

/* Begin Function:_Sys_Mem_Init_Block *****************************************
Description : Initialize a memory block given the size and start address of a 
              plain memory region.
Input       : ptr_int_t Mem_Start_Addr - The start address of the plain region.
              size_t Mem_Size - The size of the plain region.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mem_Init_Block(ptr_int_t Mem_Start_Addr,size_t Mem_Size)
{
    /* The pointer for operating the memory block */
    struct Mem_Head* Mem_Head_Ptr;
    
    /* Initialize the block data structure first - head */
    Mem_Head_Ptr=(struct Mem_Head*)Mem_Start_Addr;
    Mem_Head_Ptr->Tail_Ptr=(struct Mem_Tail*)(Mem_Start_Addr+Mem_Size-sizeof(struct Mem_Tail));
    Mem_Head_Ptr->Occupy_Flag=0;
    Mem_Head_Ptr->Occupy_PID=0;
    Mem_Head_Ptr->Mem_Start_Addr=(ptr_int_t)(Mem_Start_Addr+sizeof(struct Mem_Head));
    /* Be careful with the 1 */
    Mem_Head_Ptr->Mem_End_Addr=(ptr_int_t)(Mem_Start_Addr+Mem_Size-sizeof(struct Mem_Tail)-1);
    
    /* Tail structure */
    Mem_Head_Ptr->Tail_Ptr->Occupy_Flag=0;
    Mem_Head_Ptr->Tail_Ptr->Head_Ptr=Mem_Head_Ptr;
}
#endif
/* End Function:_Sys_Mem_Init_Block ******************************************/

/* Begin Function:_Sys_Mem_Ins_TLSF *******************************************
Description : The memory insertion function, to insert a certain memory block
              into the corresponding FLI and SLI class.
Input       : struct Mem_Head* Mem_Head_Ptr - The pointer to the initialized
              memory block. Remember that the memory block must be initialized,
              the function won't check the conditions for efficiency.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mem_Ins_TLSF(struct Mem_Head* Mem_Head_Ptr)
{
    s32 FLI_Level;
    s32 SLI_Level;
    size_t Mem_Size=(Mem_Head_Ptr->Mem_End_Addr)-(Mem_Head_Ptr->Mem_Start_Addr);

    Sys_Lock_Scheduler();
    
    /* Guarantee the Mem_Size is bigger than 64 or a failure will surely 
     * occur here.
     */
    FLI_Level=Sys_Calc_MSB_Pos(Mem_Size)-6;
    /* Decide the SLI level directly from the FLI level */
    SLI_Level=(Mem_Size>>(FLI_Level+3))&0x07;
    
    /* See if there are any blocks in the level, equal means no. So
     * what we inserted is the first block.
     */
    if(&(Mem_CB[FLI_Level][SLI_Level])==Mem_CB[FLI_Level][SLI_Level].Next)
    {
        /* Set the corresponding bit in the TLSF bitmap */
        Mem_Bitmap[FLI_Level]|=1<<SLI_Level;
    }
    
    /* Insert the node now */
    Sys_List_Insert_Node(&(Mem_Head_Ptr->Head),
                         &Mem_CB[FLI_Level][SLI_Level],
                         Mem_CB[FLI_Level][SLI_Level].Next);

    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Mem_Ins_TLSF ********************************************/

/* Begin Function:_Sys_Mem_Del_TLSF *******************************************
Description : The memory deletion function, to delete a certain memory block
              from the corresponding FLI and SLI class.
Input       : struct Mem_Head* Mem_Head_Ptr - The pointer to the initialized
              memory block. Remember that the memory block must be initialized,
              the function won't check the conditions for efficiency.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mem_Del_TLSF(struct Mem_Head* Mem_Head_Ptr)
{
    s32 FLI_Level;
    s32 SLI_Level;
    size_t Mem_Size=(Mem_Head_Ptr->Mem_End_Addr)-(Mem_Head_Ptr->Mem_Start_Addr);
    
    /* Guarantee the Mem_Size is bigger than 64 or a failure will surely 
     * occur here.
     */
    FLI_Level=Sys_Calc_MSB_Pos(Mem_Size)-6;
    /* Decide the SLI level directly from the FLI level */
    SLI_Level=(Mem_Size>>(FLI_Level+3))&0x07;
    
    Sys_Lock_Scheduler();
    
    Sys_List_Delete_Node(Mem_Head_Ptr->Head.Prev,Mem_Head_Ptr->Head.Next);
    
    /* See if there are any blocks in the level, equal means no. So
     * what we deleted is the last block.
     */
    if(&(Mem_CB[FLI_Level][SLI_Level])==Mem_CB[FLI_Level][SLI_Level].Next)
    {
        /* Clear the corresponding bit in the TLSF bitmap */
        Mem_Bitmap[FLI_Level]&=~(1<<SLI_Level);
    }
    
    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Mem_Del_TLSF ********************************************/

/* Begin Function:Sys_Mem_TLSF_Bitmap_Search **********************************
Description : The TLSF memory searcher. 
Input       : size_t Mem_Size - The memory size, must be bigger than 64. This must 
                             be guatanteed before calling this function or an 
                             error will unavoidably occur.
Output      : s32* FLI_Level - The FLI level found.
              s32* SLI_Level - The SLI level found.
Return      : retval_t - If successful,0; else -1 for failure.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
retval_t Sys_Mem_TLSF_Bitmap_Search(size_t Mem_Size,s32* FLI_Level,s32* SLI_Level)
{
    s32 FLI_Level_Temp;
    s32 SLI_Level_Temp;
    Sys_Lock_Scheduler();
    
    /* Make sure that it is bigger than 64. 64=2^6 */
    FLI_Level_Temp=Sys_Calc_MSB_Pos(Mem_Size)-6;
    /* Decide the SLI level directly from the FLI level. However, we plus 
     * the number by one here so that we can avoid the list search.
     */
    SLI_Level_Temp=((Mem_Size>>(FLI_Level_Temp+3))&0x07)+1;
    
    /* If the SLI level is the largest of the FLI level, then jump to the next 
     * FLI level.
     */
    if(SLI_Level_Temp==8)
    {
        FLI_Level_Temp+=1;
        SLI_Level_Temp=0;
    }
    
    /* Check if the FLI level is over the boundary */
    if(FLI_Level_Temp>=MM_FLI)
    {
        Sys_Unlock_Scheduler();
        return 1;
    }
    
    /* If there's at least one block that matches the query, return the
     * level.
     */
    if((Mem_Bitmap[FLI_Level_Temp]&(1<<SLI_Level_Temp))!=0)
    {
        *FLI_Level=FLI_Level_Temp;
        *SLI_Level=SLI_Level_Temp;
        
        Sys_Unlock_Scheduler();
        return 0;
    }
    /* No one exactly fits */
    else
    {
        /* Try to search in the current FLI level to see if any fits */
        if(Sys_Calc_LSB_Pos(Mem_Bitmap[FLI_Level_Temp]>>SLI_Level_Temp)>=0)
        {
            SLI_Level_Temp=Sys_Calc_LSB_Pos(Mem_Bitmap[FLI_Level_Temp]>>SLI_Level_Temp)
                           +SLI_Level_Temp;
            
            *FLI_Level=FLI_Level_Temp;
            *SLI_Level=SLI_Level_Temp;
        
            Sys_Unlock_Scheduler();
            return 0;
            
        }
        else
        {
            /* From the next level, query one by one */
            for(FLI_Level_Temp+=1;FLI_Level_Temp<MM_FLI;FLI_Level_Temp++)
            {
                /* if the level has blocks of one SLI level */
                if((Mem_Bitmap[FLI_Level_Temp]&0xFF)!=0)
                {
                    /* Find the SLI level */ 
                    SLI_Level_Temp=Sys_Calc_LSB_Pos(Mem_Bitmap[FLI_Level_Temp]);
                    
                    *FLI_Level=FLI_Level_Temp;
                    *SLI_Level=SLI_Level_Temp;
        
                    Sys_Unlock_Scheduler();
                    return 0;
                }
            }
        }
    }
    
    /* If it can get here, then the function must have failed */
    Sys_Unlock_Scheduler();
    return 1;
}
#endif
/* End Function:Sys_Mem_TLSF_Bitmap_Search ***********************************/

/* Begin Function:_Sys_Mem_Ins_Allocated **************************************
Description : The memory insertion function, to insert a certain memory block
              into the corresponding process's memory PCB registry and the "allocated"
              memory list.
              We just insert it directly. The function will automatically mark
              the block as occupied and update the "Occupy_PID".
Input       : pid_t PID - The process ID.
              struct Mem_Head* Mem_Head_Ptr - The pointer to the initialized
                                              memory block header.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mem_Ins_Allocated(pid_t PID,struct Mem_Head* Mem_Head_Ptr)
{
    Sys_Lock_Scheduler();
    
    /* Mark it as occupied first */
    Mem_Head_Ptr->Occupy_Flag=1;
    Mem_Head_Ptr->Tail_Ptr->Occupy_Flag=1;
    Mem_Head_Ptr->Occupy_PID=PID;
    
    /* Insert it into the PCB_Mem */
    Sys_List_Insert_Node(&(Mem_Head_Ptr->Proc_Mem_Head),
                         &(PCB_Mem[PID].Head),PCB_Mem[PID].Head.Next);
    
    /* Insert it into the "in use" list */
    Sys_List_Insert_Node(&(Mem_Head_Ptr->Head),
                         &Mem_Allocated_List_Head,Mem_Allocated_List_Head.Next);
    
    /* Register the memory consumption: The system memory consumption (the block
     * headers and tailers) will be registered as the "Init"'s.
     */
    PCB_Mem[0].Memory_In_Use+=sizeof(struct Mem_Head)+sizeof(struct Mem_Tail);
    PCB_Mem[PID].Memory_In_Use+=(Mem_Head_Ptr->Mem_End_Addr)-(Mem_Head_Ptr->Mem_Start_Addr)+1;
    /* Fill statistical variables */
    Free_Mem_Amount-=((ptr_int_t)(Mem_Head_Ptr->Tail_Ptr))+sizeof(struct Mem_Tail)-((ptr_int_t)Mem_Head_Ptr);
    Used_Mem_Amount+=((ptr_int_t)(Mem_Head_Ptr->Tail_Ptr))+sizeof(struct Mem_Tail)-((ptr_int_t)Mem_Head_Ptr);
    
    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Mem_Ins_Allocated ***************************************/

/* Begin Function:_Sys_Mem_Del_Allocated **************************************
Description : The memory deletion function, to delete a certain memory block
              from the corresponding process's memory PCB registry.
              We just delete it directly.
              The deletion will clear the occupy flag directly
Input       : struct Mem_Head* Mem_Head_Ptr - The pointer to the initialized
                                              memory block.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mem_Del_Allocated(struct Mem_Head* Mem_Head_Ptr)
{
    Sys_Lock_Scheduler();

    /* Delete it from the PCB_Mem */
    Sys_List_Delete_Node(Mem_Head_Ptr->Proc_Mem_Head.Prev,
                         Mem_Head_Ptr->Proc_Mem_Head.Next);
    
    /* Delete it from the "allocated" list */
    Sys_List_Delete_Node(Mem_Head_Ptr->Head.Prev,
                         Mem_Head_Ptr->Head.Next);
    
    /* Register the memory consumption: The system memory consumption (the block
     * headers and tailers) will be registered as the "Init"'s.
     */
    PCB_Mem[0].Memory_In_Use-=sizeof(struct Mem_Head)+sizeof(struct Mem_Tail);
    PCB_Mem[Mem_Head_Ptr->Occupy_PID].Memory_In_Use-=(Mem_Head_Ptr->Mem_End_Addr)-
                                                     (Mem_Head_Ptr->Mem_Start_Addr)+1;
    /* Fill statistical variables */
    Free_Mem_Amount+=((ptr_int_t)(Mem_Head_Ptr->Tail_Ptr))+sizeof(struct Mem_Tail)-((ptr_int_t)Mem_Head_Ptr);
    Used_Mem_Amount-=((ptr_int_t)(Mem_Head_Ptr->Tail_Ptr))+sizeof(struct Mem_Tail)-((ptr_int_t)Mem_Head_Ptr);
    
    /* Mark it as unoccupied at last */
    Mem_Head_Ptr->Occupy_Flag=0;
    Mem_Head_Ptr->Tail_Ptr->Occupy_Flag=0;
    Mem_Head_Ptr->Occupy_PID=0;
    
    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Mem_Del_Allocated ***************************************/

/* Begin Function:Sys_Malloc **************************************************
Description : Allocate some memory. For application use. The PID variable is
              automatically the "Current_PID".
Input       : u32 Size - The size of the RAM needed to allocate.
Output      : void* - The pointer to the memory. If no memory is allocatable,
              then "ENOMEM"(0x00) is returned.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void* Sys_Malloc(size_t Size)									                   
{	
    return(_Sys_Malloc(Current_PID,Size));
}
#endif
/* End Function:Sys_Malloc ***************************************************/

/* Begin Function:_Sys_Malloc **************************************************
Description : Allocate some memory. For system use, or where you need to specify
              the PID of the allocator(Allocate memory in the name of some process).
Input       : pid_t PID - The PID you want.
              size_t Size - The size of the RAM needed to allocate.
Output      : None.
Return      : void* - The pointer to the memory. If no memory is allocatable,
              then "ENOMEM"(0x00) is returned.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void* _Sys_Malloc(pid_t PID,size_t Size)									                   
{	
    s32 FLI_Level_Found=0;
    s32 SLI_Level_Found=0;
    struct Mem_Head* Mem_Ptr;
    size_t Temp_Size;
    
    ptr_int_t Old_Start_Addr;
    size_t Old_Block_Size;
    ptr_int_t New_Start_Addr;
    size_t New_Block_Size;

    /* Round up the size:a multiple of 8 and bigger than 64B. In fact, we will add
     * extra 8 bytes at the end if the size is a multiple of 8 for safety. 
     */
    Temp_Size=((Size>>3)+1)<<3;
    /* See if it is smaller than the smallest block */
    Temp_Size=(Temp_Size>64+8)?Temp_Size:64+8;
    
    Sys_Lock_Scheduler();
    
    /* See if such block exists, if not, abort */
    if(Sys_Mem_TLSF_Bitmap_Search(Temp_Size,&FLI_Level_Found,&SLI_Level_Found)!=0)
    {
        Sys_Unlock_Scheduler();
        return ENOMEM;
    }
    
    /* There is such block. Get it and delete it from the TLSF list. */
    Mem_Ptr=(struct Mem_Head*)Mem_CB[FLI_Level_Found][SLI_Level_Found].Next;
    _Sys_Mem_Del_TLSF(Mem_Ptr);
     
    /* Allocate and calculate if the space left could be big enough to be a new 
     * block. If so, we will put the block back into the TLSF table
     */
    if((Mem_Ptr->Mem_End_Addr)+1-Temp_Size-(Mem_Ptr->Mem_Start_Addr)>=
       sizeof(struct Mem_Head)+64+8+sizeof(struct Mem_Tail))
    {
        /* There is enough space */
        Old_Start_Addr=(Mem_Ptr->Mem_Start_Addr)-sizeof(struct Mem_Head);
        Old_Block_Size=Temp_Size+sizeof(struct Mem_Tail)+sizeof(struct Mem_Head);
        New_Start_Addr=(Mem_Ptr->Mem_Start_Addr)+Temp_Size+sizeof(struct Mem_Tail);
        New_Block_Size=(Mem_Ptr->Mem_End_Addr)+1-(Mem_Ptr->Mem_Start_Addr)-Temp_Size;

        _Sys_Mem_Init_Block(Old_Start_Addr,Old_Block_Size);
        _Sys_Mem_Init_Block(New_Start_Addr,New_Block_Size);
        
        /* Put the extra block back */
        _Sys_Mem_Ins_TLSF((struct Mem_Head*)New_Start_Addr);
        /* Register the block just allocated into the allocated list and the corresponding*/
    }

    /* Insert the allocated block into the lists */
    _Sys_Mem_Ins_Allocated(PID,Mem_Ptr);
    
    /* Finally, return the start address */
    Sys_Unlock_Scheduler();
    return(void*)(Mem_Ptr->Mem_Start_Addr);
}
#endif
/* End Function:_Sys_Malloc **************************************************/

/* Begin Function:Sys_Mfree ***************************************************
Description : Free allocated memory, for both system and application use.
Input       : void* Mem_Ptr -The pointer returned by "Sys_Malloc".
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void Sys_Mfree(void* Mem_Ptr)
{	 															               
    _Sys_Mfree(Current_PID,Mem_Ptr);
}
#endif
/* End Function:Sys_Mfree ****************************************************/

/* Begin Function:_Sys_Mfree **************************************************
Description : Free allocated memory, for system use mainly. It will free memory 
              in the name of a certain process, specified by the PID.
Input       : pid_t PID - The process ID.
              void* Mem_Ptr - The pointer returned by "Sys_Malloc".
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mfree(pid_t PID,void* Mem_Ptr)
{	 					
    struct Mem_Head* Mem_Head_Ptr=(struct Mem_Head*)(((ptr_int_t)Mem_Ptr)-sizeof(struct Mem_Head));
    struct Mem_Head* Left_Mem_Head_Ptr;
    struct Mem_Head* Right_Mem_Head_Ptr;
    s32 Merge_Left_Flag=0;
    
    /* See if the address is within the allocatable address range. If not, abort directly. */
    if(((ptr_int_t)Mem_Ptr<DMEM_START_ADDR)||((ptr_int_t)Mem_Ptr>DMEM_END_ADDR))
        return;
    
	Sys_Lock_Scheduler();
    /* See if the block can really be freed by this PID. If cannot, return 
     * directly.
     */
    if((Mem_Head_Ptr->Occupy_Flag)==0)
    {
        Sys_Unlock_Scheduler();
        return;
    }

    if((Mem_Head_Ptr->Occupy_PID)!=PID)
    {
        Sys_Unlock_Scheduler();
        return;
    }

    /* Now we are sure that it can be freed. Delete it from the allocated list now */
    _Sys_Mem_Del_Allocated(Mem_Head_Ptr);
    
    /* Now check if we can merge it with the on-the-right blocks. Take note that we must
     * check the right side before checking the left side, or the merge will not be successful.
     * Here left means lower address and right means higher address.
     * We may need some sort of assertion here.
     */
    if((ptr_int_t)((Mem_Head_Ptr->Tail_Ptr)+sizeof(struct Mem_Tail))!=(ptr_int_t)(DMEM_Heap+DMEM_SIZE))
    {
        Right_Mem_Head_Ptr=(struct Mem_Head*)(((ptr_int_t)(Mem_Head_Ptr->Tail_Ptr))+sizeof(struct Mem_Tail));

        /* If this one is unoccupied */
        if((Right_Mem_Head_Ptr->Occupy_Flag)==0)
        {
            /* Delete, merge */
            _Sys_Mem_Del_TLSF(Right_Mem_Head_Ptr);

            _Sys_Mem_Init_Block((ptr_int_t)Mem_Head_Ptr,
                                ((ptr_int_t)(Right_Mem_Head_Ptr->Tail_Ptr))+sizeof(struct Mem_Tail)-
                                (ptr_int_t)Mem_Head_Ptr);
        }
    }

    /* Now check if we can merge it with the on-the-left blocks */
    if((ptr_int_t)Mem_Head_Ptr!=(ptr_int_t)DMEM_Heap)
    {
        Left_Mem_Head_Ptr=((struct Mem_Tail*)(((ptr_int_t)Mem_Head_Ptr)-sizeof(struct Mem_Tail)))->Head_Ptr;
        
        /* If this one is unoccupied */
        if((Left_Mem_Head_Ptr->Occupy_Flag)==0)
        {
            /* Delete, merge */
            _Sys_Mem_Del_TLSF(Left_Mem_Head_Ptr);
            _Sys_Mem_Init_Block((ptr_int_t)Left_Mem_Head_Ptr,
                                (ptr_int_t)((ptr_int_t)(Mem_Head_Ptr->Tail_Ptr)+sizeof(struct Mem_Tail)-
                                (ptr_int_t)Left_Mem_Head_Ptr));
            
            /* We have completed the merge here and the original block has destroyed.
             * Thus there's no need to insert it into the list again.
             */
            Merge_Left_Flag=1;
        }
    }

    /* If we did not merge it with the left-side blocks, insert the original pointer's block 
     * into the TLSF table(Merging with the right-side one won't disturb this).
     */
    if(Merge_Left_Flag==0)
    {
        _Sys_Mem_Ins_TLSF(Mem_Head_Ptr);
    }
    else
    {
        _Sys_Mem_Ins_TLSF(Left_Mem_Head_Ptr);
    }

    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Mfree ***************************************************/

/* Begin Function:Sys_Mfree_All ***********************************************
Description : Free all allocated memory. For application use.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void Sys_Mfree_All(void)									     
{
	_Sys_Mfree_All(Current_PID);
}
#endif
/* End Function:Sys_Mfree_All ************************************************/

/* Begin Function:_Sys_Mfree_All **********************************************
Description : Free all allocated memory of a certain process. For system use,
              or where you want to specify the PID.
Input       : pid_t PID-The process's PID.
Output      : None.
Return      : None.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
void _Sys_Mfree_All(pid_t PID)									     
{
    struct Mem_Head* Mem_Head_Ptr;
    struct List_Head* Traverse_List_Ptr;
    
    /* See if the PID is valid in the system */
    if(PID>=MAX_PROC_NUM)
        return;
    
    /* The PID is within the boundary but the process is nonexistent in the
     * system.
     */
    if((PCB[PID].Status.Running_Status)&OCCUPY==0)
        return;
    
    Sys_Lock_Scheduler();
    
    Traverse_List_Ptr=PCB_Mem[PID].Head.Next;
    
    /* Traverse the list and free all memory */
    while(Traverse_List_Ptr!=&(PCB_Mem[PID].Head))
    {
        Mem_Head_Ptr=(struct Mem_Head*)(Traverse_List_Ptr-sizeof(struct List_Head));
        /* Move to the next node first. After deletion the list will be nonexistent */
        Traverse_List_Ptr=Traverse_List_Ptr->Next;
        _Sys_Mfree(PID,(void*)(Mem_Head_Ptr->Mem_Start_Addr));
    }

    Sys_Unlock_Scheduler();
}
#endif
/* End Function:_Sys_Mfree_All ***********************************************/

/* Begin Function:Sys_Query_Proc_Mem ******************************************
Description : Query the memory that a certain process uses. This is mainly called
              by the task manager and shell.
Input       : pid_t PID - The process ID.
Output      : None.
Return      : size_t - The amount of memory that the process is using. If the PID does
                    not exist, it will return 0.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
size_t Sys_Query_Proc_Mem(pid_t PID)									     
{
    /* See if the PID is valid in the system */
    if(PID>=MAX_PROC_NUM)
        return 0;
    
    /* The PID is within the boundary but the process is nonexistent in the
     * system.
     */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)
        return 0;
    
	return(PCB_Mem[PID].Memory_In_Use);
}
#endif
/* End Function:Sys_Query_Proc_Mem *******************************************/

/* Begin Function:Sys_Query_Used_Mem ******************************************
Description : Query the amount of memory that have been allocated in the system.
Input       : None.
Output      : None.
Return      : size_t - The amount of memory that have been allocated. 
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
size_t Sys_Query_Used_Memory(void)									     
{
	return(Used_Mem_Amount);
}
#endif
/* End Function:Sys_Query_Used_Mem *******************************************/

/* Begin Function:Sys_Query_Free_Mem ******************************************
Description : Query the amount of memory that is still free.
Input       : None.
Output      : None.
Return      : size_t - The amount of memory that is still free.
******************************************************************************/
#if(ENABLE_MEMM==TRUE)
size_t Sys_Query_Free_Mem(void)									     
{
	return(Free_Mem_Amount);
}
#endif
/* End Function:Sys_Query_Free_Mem *******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved. ************/
