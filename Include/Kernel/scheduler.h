/******************************************************************************
Filename   : scheduler.h
Author     : pry
Date       : 07/04/2012
Description: The header of "kernel.c".
******************************************************************************/

/* Config Includes ***********************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"
/* End Config Includes *******************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SCHEDULER_H_DEFS__
#define __SCHEDULER_H_DEFS__

/* Errors */
/* The PID is not valid or not avaliable */
#define ENOPID          0x01
/* The priority is not valid or not available */
#define ENOPRIO         0x02
/* The time value(either in clock time or in ticks) is not valid */
#define ENOTIM          0x03

/* Process ready status */
#define READY           0x01
#define NOT_READY       0x00

/* The PID to fill in the struct when we call "Sys_Start_Proc" - This is 
 * purely for safety
 */
#define AUTO_PID        0
/* The parent policy for the "Sys_Start_Proc". This decides whether we need to 
 * get the return value of the process.
 */
#define PARENT_CARE     0
#define PARENT_ABDN     1

/* Bit Assignment
   31      30        29        28           27        26        25       24

   23      22        21        20           19        18        17       16

   15      14        13        12           11        10        9         8
                    
   7       6         5         4            3         2         1         0
Occupy  Zombie             Reserved      Reserved  Reserved  Reserved  Reserved */
/* The occupy flag */
#define OCCUPY         0x80   
/* The zombie process flag */                                                      
#define ZOMBIE         0x40                                                 

/* Restart and reset config register */
#define NVIC_AIRCR	   (*((vu32*)0xE000ED0C))
#define RESET_VALUE    0x05FA0001
#define RESTART_VALUE  0x05FA0004
/* __SCHEDULER_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/


/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SCHEDULER_H_STRUCTS__
#define __SCHEDULER_H_STRUCTS__
/* We used structs in the header */
#include "Syslib\syslib.h"
/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/* The tick time struct */
struct Tick_Time
{
    time_t High_Bits;
    time_t Low_Bits;
};

/* The kernel basic status struct */
struct Sys_Kernel_Status_Struct
{
    /* System booting indicator */
    u32 Boot_Done;
    /* The current priority level the kernel is running at */
    struct List_Head* Cur_Prio_Ptr;
    /* The pointer to the process running currently */
    struct List_Head* Proc_Running_Ptr; 
};

/* The kernel time status struct */
struct Sys_Time_Status_Struct
{
    /* This contains the seconds from 01/01/1970 00:00 am, not ticks! This is 
     * often called "wall (clock) time" alternatively.
     * If the system does not have an RTC (which the system will read on start), 
     * then the value has no sense.
     * This can be treated as a 64-bit long long integer that is not supported
'    * in C89, making it possible to measure the time until 01/01/3001 00:00 am. 
     */
    struct Tick_Time Real_World_Time;
    /* The total ticks from the start of the OS */
    struct Tick_Time OS_Total_Ticks;
};

/* The kernel process status struct */
struct Sys_Proc_Status_Struct
{
    /* The number of active priorities in the system */
    size_t Active_Prios;
    /* The number of processes running in the system */
    size_t Running_Proc_Number;
    /* The total number of processes existing (zombies included) in the system */
    size_t Total_Proc_Number;
    /* The number of zombie processes in the system */
    size_t Total_Zombie_Number;
};

/* The whole system status struct */
struct Sys_Status_Struct
{
    struct Sys_Kernel_Status_Struct Kernel;
    struct Sys_Proc_Status_Struct Proc;    
    struct Sys_Time_Status_Struct Time;   
};

/* The struct for process initialization */
struct Proc_Init_Struct
{
    pid_t PID;
    pid_t Parent_Policy;    
    /* The name of the pseudo-process */                                                             
    s8* Name;								                                 
    /* The priority for the process. 0 is the lowest */
    prio_t Priority;                                                              
    s32 Ready_Flag;
    /* The entrance of the process */
    void (*Entrance)(void);     
    /* It is not the real stack beginning address for the ARM stack is a descending full stack */                                              
    ptr_int_t Stack_Address;				                                           
    s32 Stack_Size;
    
    /* The maximum and minimum time allowed to be allocated to the process. Used when the scheduler
     * automatically adjusts the timeslices.
     */
    s32 Max_Slices;                                                               
    s32 Min_Slices; 
    /* The running time actually used currently */                                                           
    s32 Cur_Slices;                                                               
};

/* The structs below will be used in the process control block for registering process 
 * information.
 */
/* This one for time controlling */
struct Proc_Time_Struct
{
    /* The time must be smaller than 0x00FFFFFF.The last is Dummy,0x00FFFFFE forever,for Fault Prevention.*/
    s32 Max_Tim;                                                               
    /* The Minimum Timeslice For The Application,In Ticks.Used By The Kernel When Scheduling.*/
    s32 Min_Tim;   
    /* The running time actually used currently.*/                                                            
    s32 Cur_Tim;   
    /* The Storage Of The Remaining Time.*/                                                                 
    s32 Lft_Tim;       
};

/* This one for signal storage */
struct Proc_Signal_Struct
{
    /* The place for signal storage. Stores the signal received here.*/
    signal_t  Signal_Recv;	                                                               
    /* Stores the entrance of the signal handlers. Some system signals can also
     * have signal handlers:
     * SIGCHLD
     */
    ptr_int_t Signal_Handler[32];
    /* This is the global variable for the signal system. Now that the signal system
     * cannot change the "global" variable inside each task function, then we can provide
     * a global variable for it to change, and the process can scan it.
     */
    ptr_int_t Sig_IPC_Global[32];    
};

/* This one for process status storage */
struct Proc_Status_Struct
{
    /* The Priority for the process. 250 is the highest.*/
    prio_t Priority;
    /* The Process Status Register.Also The PID Occupation Status.*/                                                              
    u32 Running_Status;  
    /* The sleep count. If the process is ready, the count will be 0. */
    cnt_t Sleep_Count;   
    /* The status change lock counter. If this counter is bigger than zero,
     * then all (user) changes to the state of the process is void.
     */
    cnt_t Status_Lock_Count;
    /* The return value of the process. If you want to get it, then
     * The process must be a zombie process.
     */    
    retval_t Retval;
};

/* This one for storage of other information, such as process names */
struct Proc_Info_Struct
{
    /* Process ID */
    pid_t PID;
    /* Parent process ID. If the number is zero, then this means that nobody
     * cares its return value.
     */
    pid_t PPID;
    /* The Process Name Is Registered Here.*/  
    s8* Name;
    /* The initial stack pointer */
    ptr_int_t Init_Stack_Ptr;
    /* The process entrance address */
    ptr_int_t Init_Addr_Ptr;
    /* The errno it just got */
    errno_t Errno;    
};

/* PCB (Process Control Block) struct */
struct PCB_Struct
{
    /* The doubly linked list header for process switching*/
    struct List_Head Head;
    struct Proc_Info_Struct Info;
    struct Proc_Status_Struct Status;
    struct Proc_Time_Struct Time;
    struct Proc_Signal_Struct Signal;                                                             
};

/* The priority list struct */
struct Prio_List_Struct
{
    struct List_Head Head;
    prio_t Priority;   
    s32 Proc_Num;
    struct List_Head Running_List;
};

/* __SCHEDULER_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SCHEDULER_MEMBERS__
#define __SCHEDULER_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#undef __HDR_DEFS__
#define __HDR_STRUCTS__
#include "Kernel\scheduler.h"
#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/ 
/* The kernel loader */						                       
static void _Sys_Load_Init(void);   
/* The system scheduler initializer */                         
static void _Sys_Scheduler_Init(void);          
/* Initialize the systick timer */                             
static void _Sys_Systick_Init(cnt_t Ticks);  
/* The function to start the operating system */                                  
static void _Sys_Start(void);   
/* Process stack initialization */
static void _Sys_Proc_Stack_Init(pid_t PID);

#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/* Process Scheduling */
/* The struct which stores the system struct information.
 * The system information are:
 * 1>The current priority level that the kernel is running at. That means 
 *   the system is robin-rounding processes with this priority.
 */
__EXTERN__ volatile struct Sys_Status_Struct System_Status;

/* Stores the current PID */                                          
__EXTERN__ volatile pid_t Current_PID;
/* Stores the current running priority level */
__EXTERN__ volatile prio_t Current_Prio;
/* The pended scheduler action count */
__EXTERN__ volatile cnt_t Pend_Sched_Cnt;                    
/* Stores the position of the stack pointers */
__EXTERN__ volatile ptr_int_t PCB_Cur_SP[MAX_PROC_NUM];    
/* The PCB is in a struct now */
__EXTERN__ volatile struct PCB_Struct PCB[MAX_PROC_NUM];  
/* The priority running list */
__EXTERN__ volatile struct List_Head Prio_List_Head;
__EXTERN__ volatile struct Prio_List_Struct Prio_List[MAX_PRIO_NUM];
/* End Process Scheduling */
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void Sys_Restart(void);
__EXTERN__ void Sys_Switch_Now(void);	

/* The initial state process loader */
__EXTERN__ void _Sys_Proc_Load(struct Proc_Init_Struct* Process); 
__EXTERN__ pid_t Sys_Start_Proc(struct Proc_Init_Struct* Process);

__EXTERN__ pid_t Sys_Get_PID(void);
__EXTERN__ retval_t Sys_Set_Ready(pid_t PID);
__EXTERN__ retval_t _Sys_Set_Ready(pid_t PID);  
__EXTERN__ retval_t Sys_Clr_Ready(pid_t PID);
__EXTERN__ retval_t _Sys_Clr_Ready(pid_t PID);
__EXTERN__ void _Sys_Ins_Proc_Into_New_Prio(pid_t PID,prio_t Priority);
__EXTERN__ void _Sys_Del_Proc_From_Cur_Prio(pid_t PID);
__EXTERN__ retval_t Sys_Change_Proc_Prio(pid_t PID,prio_t Priority);
__EXTERN__ retval_t _Sys_Change_Proc_Prio(pid_t PID,prio_t Priority);

__EXTERN__ retval_t _Sys_Require_Timeslice(pid_t PID,size_t Slices);
__EXTERN__ retval_t Sys_Require_Timeslice(size_t Slices);    

__EXTERN__ size_t Sys_Query_Timeslice(pid_t PID);
__EXTERN__ retval_t Sys_Query_Running_Status(pid_t PID);
__EXTERN__ cnt_t Sys_Query_Sleep_Count(pid_t PID);
__EXTERN__ prio_t Sys_Query_Proc_Prio(pid_t PID);
__EXTERN__ prio_t Sys_Query_Current_Prio(void);


/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* End Public C Function Prototypes ******************************************/

/*Assembly Functions Prototypes***********************************************/
/* Always extern - Assembly ones */
/* The process switcher called by the assembly code, in fact a C function */                                           
EXTERN void _Sys_Get_High_Ready(void);					                       
EXTERN void _Sys_Schedule_Trigger(void);
/* This is the systick handler - This will override the default one */
EXTERN void _Sys_Systick_Routine(void);
/* __SCHEDULER_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/*End Assembly Functions Prototypes*******************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
