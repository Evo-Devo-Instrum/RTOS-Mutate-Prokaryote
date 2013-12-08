/******************************************************************************
Filename    : syslib.h
Author      : pry
Date        : 25/04/2012
Version     : 0.01
Description : The RMP system library.
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SYSLIB_H_DEFS__
#define __SYSLIB_H_DEFS__


/* __SYSLIB_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SYSLIB_H_STRUCTS__
#define __SYSLIB_H_STRUCTS__
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/* Begin Struct: List_Head ****************************************************
Description : The classical doubly linked list head from Linux    
Referred to : kernel.c,kernel.h 
******************************************************************************/
struct List_Head															   
{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
	struct List_Head* Prev;
	struct List_Head* Next;
};
/* End Struct: List_Head *****************************************************/

/* __SYSLIB_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SYSLIB_MEMBERS__
#define __SYSLIB_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/
/* The MSB&LSB calculation table. The table is suitable for a 8-bit number,
 * that means if we need to see whether a 16-bit number's MSB, we need to compare
 * whether it is bigger than 8. Else we need to shift it into range 0-255.
 * From this table we can get the MSB of the number at direct. Then how to get
 * its LSB? We can first calculate the 1's complement of the number. Thus the 
 * continuous trailing zeros will all be 1 and when plus by one the LSB will
 * be one, with all the trailing zeros cleared. Thus a bit-and can produce the
 * number with the only LSB. 
 * For example, "11001000"'s complement is "00110111"+1 ="00111000". 
 * "00111000"&"11001000"="00001000". Then LSB is 3. 
 */
static const s8 MSB_LSB_Calc_Tbl[256]= 
{
    /* "0"'s MSB is -1 */
    -1,
    /* "1"'s MSB is 0 */
    0,
    /* "2"-"3"'s MSB is 1 */
    1, 1,
    /* "4"-"7"'s MSB is 2 */
    2, 2, 2, 2,
    /* "8"-"15"'s MSB is 3 */
    3, 3, 3, 3, 3, 3, 3, 3,
    /* "16"-"31"'s MSB is 4 */
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    /* "32"-"63"'s MSB is 5 */
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    /* "64"-"127"'s MSB is 6 */
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    /* "128"-"255"'s MSB is 7 */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7
};
/*****************************************************************************/
/* End Private Global Variables **********************************************/

/*Private C Function Prototypes***********************************************/
/*****************************************************************************/

/*****************************************************************************/
#define __EXTERN__ 
/*End Private C Function Prototypes*******************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
__EXTERN__ s32 Sys_Calc_MSB_Pos(u32 Number);
__EXTERN__ s32 Sys_Calc_LSB_Pos(u32 Number);

__EXTERN__ void Sys_Create_List(struct List_Head* Head);
__EXTERN__ void Sys_List_Delete_Node(struct List_Head* Prev,struct List_Head* Next);															
__EXTERN__ void Sys_List_Insert_Node(struct List_Head* New,struct List_Head* Prev,struct List_Head* Next); 																																		   
__EXTERN__ struct List_Head* Sys_List_Seek(struct List_Head* Current,cnt_t Rewind_Steps);
      
__EXTERN__ size_t Sys_Strlen(s8* String);														   
__EXTERN__ size_t Sys_Strcmp(s8* String_0,s8* String_1,size_t Length);     	     
__EXTERN__ void Sys_Strcpy(s8* String_0,s8* String_1,size_t Length);                                         											

__EXTERN__ void Sys_Memset(ptr_int_t Address,s8 Char,size_t Size);
__EXTERN__ void Sys_Memcpy(ptr_int_t Dest,ptr_int_t Src,size_t Size);
/*****************************************************************************/

/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __SYSLIB_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif                                            														   
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
