 /*****************************************************************************
Filename    : syslib.c
Author      : pry
Date        : 29/04/2012
Description : The RMP system library. The system will only use functions from here
              to replace the standard C library.
******************************************************************************/
							 
/*Includes********************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Syslib\syslib.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\scheduler.h"
#include "Memmgr\memory.h"
#undef __HDR_STRUCTS__
/* Private Includes */
#include "Syslib\syslib.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes***************************************************************/

/* Begin Function:Sys_Calc_MSB_Pos ********************************************
Description : Calculation the MSB's position of the number. We treat 0 as the 
              LSB.
Input       : u32 Number - The number to calculate.
Output      : None.
Return      : s32 - The MSB position of the number. If returned "-1" ,then the 
                   input number is 0.
******************************************************************************/
s32 Sys_Calc_MSB_Pos(u32 Number)
{
    u8 Additional_Bits=0;
    u32 Number_Calc=Number;
    
    /* Classify the number */
	if(Number_Calc>=0xFFFFFF)
    {
        Number_Calc=Number_Calc>>24;
        Additional_Bits=24;
    }
    else if(Number_Calc>=0xFFFF)
    {
        Number_Calc=Number_Calc>>16;
        Additional_Bits=16;
    }
    else if(Number_Calc>=0xFF)
    {
        Number_Calc=Number_Calc>>8;
        Additional_Bits=8;
    }
    else
    {
        /* Do nothing */
    }
    
    /* Calculate the final result */
    return (MSB_LSB_Calc_Tbl[Number_Calc]+Additional_Bits);
}
/* End Function:Sys_Calc_MSB_Pos *********************************************/

/* Begin Function:Sys_Calc_LSB_Pos ********************************************
Description : Calculation the LSB's position of the number. We treat 0 as the 
              LSB. The LSB we find is where "1" first appears, thus "0100"'s LSB 
              is 2. See the "MSB_LSB_Calc_Tbl" comments for the principle.
Input       : u32 Number - The number to calculate.
Output      : None.
Return      : s32 - The LSB position of the number. If returned "-1" ,then the 
                    input number is 0.
******************************************************************************/
s32 Sys_Calc_LSB_Pos(u32 Number)
{
    u8 Additional_Bits=0;
    
    /* The original table can only deal with MSB, so we need to adapt the table */
    u32 Number_Calc=Number&((~Number)+1);
    
    /* Classify the number */
	if(Number_Calc>=0xFFFFFF)
    {
        Number_Calc=Number_Calc>>24;
        Additional_Bits=24;
    }
    else if(Number_Calc>=0xFFFF)
    {
        Number_Calc=Number_Calc>>16;
        Additional_Bits=16;
    }
    else if(Number_Calc>=0xFF)
    {
        Number_Calc=Number_Calc>>8;
        Additional_Bits=8;
    }
    else
    {
        /* Do nothing */
    }
    
    /* Calculate the final result */
    return (MSB_LSB_Calc_Tbl[Number_Calc]+Additional_Bits);
}
/* End Function:Sys_Calc_LSB_Pos *********************************************/

/* Begin Function:Sys_Create_List *********************************************
Description : Create a doubly linkled list.
Input       : struct List_Head* Head - The pointer to the list head.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Create_List(struct List_Head* Head)
{
	Head->Prev=Head;
	Head->Next=Head;
}
/* End Function:Sys_Create_List **********************************************/

/* Begin Function:Sys_List_Delete_Node ****************************************
Description : Delete a node from the doubly-linked list.
Input       : struct List_Head* Prev - The prevoius node of the target node.
              struct List_Head* Next - The next node of the target node.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_List_Delete_Node(struct List_Head* Prev,struct List_Head* Next)
{
    Next->Prev=Prev;
    Prev->Next=Next;
}
/* End Function:Sys_List_Delete_Node *****************************************/

/* Begin Function:Sys_List_Insert_Node ****************************************
Description : Insert a node to the doubly-linked list.
Input       : struct List_Head* New - The new node to insert.
			  struct List_Head* Prev - The previous node.
			  struct List_Head* Next - The next node.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_List_Insert_Node(struct List_Head* New,struct List_Head* Prev,struct List_Head* Next)
{
	Next->Prev=New;
	New->Next=Next;
	New->Prev=Prev;
	Prev->Next=New;
}
/* End Function:Sys_List_Insert_Node *****************************************/

/* Begin Function:Sys_List_Seek ***********************************************
Description : Rewind the pointer of a doubly-linked list.
Input       : struct List_Head* Current - The current list pointer.
			  cnt_t Rewind_Steps - The steps to move. A negative number means move backwards.
Output      : None.
Return      : struct List_Head* -The Pointer To The "List" In The First Struct.
******************************************************************************/
struct List_Head* Sys_List_Seek(struct List_Head* Current,cnt_t Rewind_Steps)
{
	struct List_Head* Current_Pointer=Current;
	cnt_t Step_Count;		
    
    /* Rewind Forward */
	if(Rewind_Steps>0)													       
	{
		for(Step_Count=0;Step_Count<(Rewind_Steps);Step_Count++)
			Current_Pointer=Current_Pointer->Next;
	}
	else
	{   /* Rewind Backward */	
		for(Step_Count=0;Step_Count<(-Rewind_Steps);Step_Count++)		       	
			Current_Pointer=Current_Pointer->Prev;
	}
   											      
	return(Current_Pointer);
}
/* End Function:Sys_List_Seek ************************************************/

/* Begin Function:Sys_Memset **************************************************
Description : The memset function. Fills a certain memory area with
              the intended character.
Input       : ptr_int_t Address - The fill start address.
			  s8 Char - The character to fill.
			  size_t Size - The size to fill.
Output      : None. 
Return      : None.
******************************************************************************/
void Sys_Memset(ptr_int_t Address,s8 Char,size_t Size)		                         
{
	s8* Address_Ptr=(s8*)Address;
	cnt_t Size_Count;
	for(Size_Count=Size;Size_Count>0;Size_Count--)
		*Address_Ptr++=Char;						                         
}
/* End Function:Sys_Memset ***************************************************/

/* Begin Function:Sys_Memcpy **************************************************
Description : The memcpy function. Duplicate a certain memory area.
Input       : ptr_int_t Src - The copy source.
              ptr_int_t Dest - The copy destination.
			  size_t Size - The size to fill.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Memcpy(ptr_int_t Dest,ptr_int_t Src,size_t Size)		                         
{
	s8* Dest_Ptr=(s8*)Dest;
    s8* Src_Ptr=(s8*)Src;
	cnt_t Size_Count;
    
	for(Size_Count=0;Size_Count<Size;Size_Count++)
		Dest_Ptr[Size_Count]=Src_Ptr[Size_Count];						                         
}
/* End Function:Sys_Memcpy ***************************************************/

/* Begin Function:Sys_Strlen **************************************************
Description : Get the string length.
Input       : s8* String - The string.
Output      : None.
Return      : size_t -The number of characters in the string. 
******************************************************************************/
size_t Sys_Strlen(s8* String)
{
	cnt_t Length_Count=0;
	for(Length_Count=0;(String[Length_Count]!='\0')&&(Length_Count<MAX_STR_LEN);Length_Count++);
	return Length_Count;
}
/* End Function:Sys_Strlen ***************************************************/

/* Begin Function:Sys_Strcmp **************************************************
Description : Compare the two strings.
Input       : s8* String_0 - The first string.
              s8* String_1 - The second string.
			  size_t Length -The length for comparison.
Output      : None.
Return      : size_t -The result:
              0  -The two are the same within the compare length.
			  1  -The first is bigger than the second.
			  2  -The first is smaller than the second. 
******************************************************************************/
size_t Sys_Strcmp(s8* String_0,s8* String_1,size_t Length)
{
	u32 Length_Count=0;
	for(Length_Count=0;Length_Count<((Length>MAX_STR_LEN)?MAX_STR_LEN:Length);Length_Count++)
	{
		if(String_0[Length_Count]>String_1[Length_Count])
			return(1);
		if(String_0[Length_Count]<String_1[Length_Count])
			return(2);
        
        /* The length of the two strings must be the same */
		if(String_0[Length_Count]=='\0')
			return(0);										                   
	} 
    
    /* The two string are the same within "Length" */
	return(0);												                  
}
/* End Function:Sys_Strcmp ***************************************************/

/* Begin Function:Sys_Strcpy **************************************************
Description : Copy the string from source to destination. Take note that the
              function has no memory overflow check.
Input       : s8* String_0 - The source string.
              s8* String_1 - The destination string.
			  size_t Length - The length to copy.
Output      : None. 
Return      L None.
******************************************************************************/
void Sys_Strcpy(s8* String_0,s8* String_1,size_t Length)
{
	cnt_t Length_Count=0;
	for(Length_Count=0;
        Length_Count<((Length>MAX_STR_LEN)?MAX_STR_LEN:Length)-1;
        Length_Count++)
	{   
		String_1[Length_Count]=String_0[Length_Count];		  
        /* If the string is going to end */
        if(String_0[Length_Count+1]=='\0')
        {
            Length_Count++;
            break;        
        }
	}
    
    /* Add the '\0' */
	String_1[Length_Count]='\0';                
}
/* End Function:Sys_Strcpy ***************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
