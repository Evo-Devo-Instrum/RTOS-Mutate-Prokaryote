/******************************************************************************
Filename    : platform.h
Author      : pry 
Date        : 22/07/2012
Description : The Include Files Concerned With The Architecture.It Also Contains 
              The Basic Types,Which Should Not Be Included In The "typedefs.h".
			  The File Shall Be DIRECTLY Included In The C Files Separately.
******************************************************************************/

/* Preprocessor Control ******************************************************/
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/* Basic Types ***************************************************************/
#if(DEFINE_BASIC_TYPES==TRUE)
typedef signed int  s32;
typedef signed short s16;
typedef signed char  s8;

typedef const signed int sc32; 
typedef const signed short sc16; 
typedef const signed char sc8;   

typedef volatile signed int  vs32;
typedef volatile signed short  vs16;
typedef volatile signed char   vs8;

typedef volatile const signed int vsc32;
typedef volatile const signed short vsc16;
typedef volatile const signed char vsc8;

typedef unsigned long long u64;
typedef unsigned int  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef const unsigned int uc32;  
typedef const unsigned short uc16;  
typedef const unsigned char uc8;   

typedef volatile unsigned int  vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

typedef volatile const unsigned int vuc32;
typedef volatile const unsigned short vuc16;  
typedef volatile const unsigned char vuc8;  
#endif
/* End Basic Types ***********************************************************/

/* Includes ******************************************************************/
#include "STM32\stm32f10x_conf.h"
/* End Includes **************************************************************/

/* Begin Extended Types ******************************************************/
#ifndef __PID_T__
#define __PID_T__
/* The typedef for the PID */
typedef s32 pid_t;
#endif

#ifndef __PTR_INT_T__
#define __PTR_INT_T__
/* The typedef for the pointers - This is the raw style. Pointers must be unsigned */
typedef u32 ptr_int_t;
#endif

#ifndef __CNT_T__
#define __CNT_T__
/* The typedef for the count variables */
typedef s32 cnt_t;
#endif

#ifndef __SLICE_T__
#define __SLICE_T__
/* The typedef for timeslices */
typedef s32 slice_t;
#endif

#ifndef __PRIO_T__
#define __PRIO_T__
/* The type for priority */
typedef s32 prio_t;
#endif

#ifndef __RETVAL_T__
#define __RETVAL_T__
/* The type for process return value */
typedef s32 retval_t;
#endif

#ifndef __SIGNAL_T__
#define __SIGNAL_T__
/* The signal type */
typedef u32 signal_t;
#endif

#ifndef __ERRNO_T__
#define __ERRNO_T__
/* The errno type */
typedef s32 errno_t;
#endif

#ifndef __SIZE_T__
#define __SIZE_T__
/* The size for time */
typedef u32 size_t;
#endif

#ifndef __SEMID_T__
#define __SEMID_T__
/* The semaphore ID type */
typedef s32 semid_t;
#endif

#ifndef __MUTID_T__
#define __MUTID_T__
/* The mutex ID type */
typedef s32 mutid_t;
#endif

#ifndef __MSGQID_T__
#define __MSGQID_T__
/* The message queue ID type */
typedef s32 msgqid_t;
#endif

#ifndef __MSGQBID_T__
#define __MSGQBID_T__
/* The message queue block ID type */
typedef s32 msgqbid_t;
#endif

#ifndef __MSGTYP_T__
#define __MSGTYP_T__
/* The message queue block's message type identifier */
typedef u32 msgtyp_t;
#endif

#ifndef __PIPEID_T__
#define __PIPEID_T__
/* The pipe module's type identifier */
typedef s32 pipeid_t;
#endif

#ifndef __SHMID_T__
#define __SHMID_T__
/* The shared memory module's identifier type */
typedef s32 shmid_t;
#endif

#ifndef __TIMID_T__
#define __TIMID_T__
/* The system timer module's identifier type */
typedef s32 timid_t;
#endif

#ifndef __TIME_T__
#define __TIME_T__
/* The time type */
typedef u32 time_t;
#endif
/* End Extended Types ********************************************************/

#endif
/*End Preprocessor Control****************************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
