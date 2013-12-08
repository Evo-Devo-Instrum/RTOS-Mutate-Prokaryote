/******************************************************************************
Filename   : signal.c
Author     : pry
Date       : 25/04/2012
Description: The signal part for the OS.
******************************************************************************/

/* Includes ******************************************************************/
#include "Config\MP_config.h"
#include "Platform\MP_platform.h"

/* Definition includes */
#define __HDR_DEFS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\signal.h"
#undef __HDR_DEFS__

/* Structure includes */
#define __HDR_STRUCTS__
#include "Syslib\syslib.h"
#include "Kernel\error.h"
#include "Kernel\scheduler.h"
#undef __HDR_STRUCTS__

/* Private includes */
#include "Kernel\signal.h"

/* Public includes */
#define __HDR_PUBLIC_MEMBERS__
#include "Kernel\scheduler.h"
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#include "Memmgr\memory.h"
#include "ExtIPC\semaphore.h"
#include "ExtIPC\pipe.h"
#include "Syslib\syslib.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_Sys_Signal_Handler *****************************************
Description : The system signal handler.
Input       : pid_t PID -The PID.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Signal_Handler(pid_t PID)	   										  
{
    cnt_t Sig_Cnt;
    /* Check if these signals exist. We do not check SIGWAKEUP here, for
     * it is already processed in the shell directly.
     * The SIGSLEEP signal is used in pair with SIGWAKE to pause or 
     * resume the process.
     * The reason why we don't scan SIGWAKE here is that when the process is 
     * stopped, there's no chance to execute the signal handler, and then
     * the SIGWAKE signal won't be scanned at all.
     */
    
    /* See if we received SIGCHLD */
    if((PCB[PID].Signal.Signal_Recv&SIGCHLD)==0)	
    {
        if((PCB[PID].Signal.Signal_Handler[3])!=0)
        {													       
            _Sys_SIGCUSTOM_Handler=(volatile void(*)(void))PCB[PID].Signal.Signal_Handler[3]; 
            _Sys_SIGCUSTOM_Handler();	
        }   
    }
    
    /* See if we received any signal that may have registered a signal handler */
    if(((PCB[PID].Signal.Signal_Recv)&0xFFFFFFF0)!=0)
    {
        for(Sig_Cnt=3;Sig_Cnt<32;Sig_Cnt++)
        {
            /* If the signal is received and the signal hander is valid */
            if(((PCB[PID].Signal.Signal_Recv&SIGUSR0<<Sig_Cnt)!=0)&&((PCB[PID].Signal.Signal_Handler[Sig_Cnt+4])!=0))
            {													       
                _Sys_SIGCUSTOM_Handler=(volatile void(*)(void))PCB[PID].Signal.Signal_Handler[Sig_Cnt]; 
                _Sys_SIGCUSTOM_Handler();	
            }
        }
    }
    /* Clear The Signal */
    PCB[PID].Signal.Signal_Recv=NOSIG;												    
}
/* End Function:_Sys_Signal_Handler ******************************************/

/* Begin Function:Sys_Process_Quit ********************************************
Description : Call this function if the process want to quit by itself. 
Input       : retval_t Retval - The return value of the process.
Output      : None.
Return      : None.
******************************************************************************/
void Sys_Process_Quit(retval_t Retval)								   			  
{
	Sys_Lock_Scheduler();

    /* Send a signal to the parent process, if it wanted to get the retval 
     * of the process this signal will also change the "Sig_IPC_Global".
     */
    if(PCB[Current_PID].Info.PPID!=0)
    {
        Sys_Send_Signal(PCB[Current_PID].Info.PPID,SIGCHLD);
        _Sys_Set_Sig_IPC_Global(PCB[Current_PID].Info.PPID,SIGCHLD,PCB[Current_PID].Info.PID);
    }
    /* Change its state and register the retval */
    PCB[Current_PID].Status.Running_Status=OCCUPY|ZOMBIE;  
    PCB[Current_PID].Status.Retval=Retval;
    /* Update the statistical variable */
    System_Status.Proc.Total_Zombie_Number++;
    
    /* The process called the function by itself, so it can't be sleeping.
     * Thus there's no need to check the if it has been deleted from the list here.
     */
    _Sys_Del_Proc_From_Cur_Prio(Current_PID);
    
	Sys_Unlock_Scheduler();  
    
    /* Dead loop */                               			   
	while(1);                                                      			   
}
/* End Function:Sys_Process_Quit *********************************************/

/* Begin Function:Sys_Get_Proc_Retval *****************************************
Description : Get a process's return value. When the process is a zombie one,
              the parent process, when it is not init, will receive a signal "SIGCHLD"
              to let it collect the return value of the child process.
Input       : pid_t PID - The PID of the process.
Output      : None.
Return      : retval_t - The return value, 0 for success, -1 for fault.
******************************************************************************/
retval_t Sys_Get_Proc_Retval(pid_t PID)							        	  
{	
    retval_t Retval;
    
    /* See if the PID is out of range */
    if(PID>=MAX_PROC_NUM)
    {
        Sys_Set_Errno(ENOPROC);
        return(-1);
    }
    
	Sys_Lock_Scheduler();
    
    /* See if the process is a zombie */
	if(((PCB[PID].Status.Running_Status&ZOMBIE)!=0)
       &&((PCB[PID].Status.Running_Status&OCCUPY)!=0))                            	   
	{				    		   
		/* Store the retval in the temporary value */
        Retval=PCB[PID].Status.Retval;
        /* Update the statistical variable */
        System_Status.Proc.Total_Zombie_Number--;
        System_Status.Proc.Total_Proc_Number--;
        /* Clean up its PCB - except for the PID itself */
		Sys_Memset((ptr_int_t)(&PCB[PID]),0,sizeof(struct PCB_Struct));  
        PCB[PID].Info.PID=PID;
        
		Sys_Unlock_Scheduler();	               
		return(Retval);        	 										   
	}		   
	else
	{
		Sys_Unlock_Scheduler();	 
        Sys_Set_Errno(ENOPROC);        
		return(-1); 										    	  
	}
}
/* End Function:Sys_Get_Proc_Retval ******************************************/
 
/* Begin Function:_Sys_Kill_The_Process ***************************************
Description : Kill a process. The process killed won't have a return value.
              Remember to make sure that the process has given up all the opened
              handles in the system. Killing it directly will cause some fault
              in the system if the handles are not all closed.
              The SIGKILL Handler. Don't call this in user application.
Input       : pid_t PID -The PID of the process to Kill.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Kill_The_Process(pid_t PID)			                       			   
{	    
    Sys_Lock_Scheduler();
    /* Don't use memset First! It will cause some corrupt. */
    /* In case a "sleep-and-kill" happens, we need to examine if 
     * the process is already stopped.
     */
    if(PCB[PID].Info.PPID!=0)
    {
        Sys_Send_Signal(PCB[PID].Info.PPID,SIGCHLD);
        _Sys_Set_Sig_IPC_Global(PCB[PID].Info.PPID,SIGCHLD,PCB[PID].Info.PID);
    }
    
    /* If not sleeping */
    if(PCB[PID].Status.Sleep_Count==0)
    {
        /* Delete directly regardless of other changes */
        _Sys_Del_Proc_From_Cur_Prio(PID);
    }	
    
    /* Clear its PCB - except for the PID itself */
    Sys_Memset((ptr_int_t)(&PCB[PID]),0,sizeof(struct PCB_Struct));  
    PCB[PID].Info.PID=PID;
    
    /* Refresh the system statistical variable */
    System_Status.Proc.Total_Proc_Number--;
    
    Sys_Unlock_Scheduler();      
}
/* End Function:_Sys_Kill_The_Process ****************************************/

/* Begin Function:_Sys_Sleep_The_Process **************************************
Description : Make a process sleep. The SIGSLEEP Handler. It does not have 
              atomic operation protection. Don't call this in user application.
Input       : pid_t PID - Make the process sleep.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Sleep_The_Process(pid_t PID)											
{
    Sys_Clr_Ready(PID);
}
/* End Function:_Sys_Sleep_The_Process ***************************************/

/* Begin Function:_Sys_Wake_The_Process ***************************************
Description : Wake a process. The SIGWAKE handler. It does not have 
              atomic operation protection. Don't call this in user application.
Input       : pid_t PID -The process to wake.
Output      : None.
Return      : None.
******************************************************************************/
void _Sys_Wake_The_Process(pid_t PID)										  
{   
    Sys_Set_Ready(PID);
}
/* End Function:_Sys_Wake_The_Process ****************************************/

/* Begin Function:Sys_Send_Signal *********************************************
Description : Sending signals to other processes.
Input       : pid_t PID - The PID to send the signal to.
              u8 Signal-The Signal ID.
Output      : None.
Return      : retval_t - 0 for success, -1 for failure.
******************************************************************************/
retval_t Sys_Send_Signal(pid_t PID,signal_t Signal)
{
    /* Atomic operation protection */
    Sys_Lock_Scheduler();
    
    /* See if the PID is 0. We do not allow sending signals to the "Init" process */
    if(PID==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOPROC);
        return(-1);
    }
    
    /* Check if the operation is over the boundary */
    if(PID>=MAX_PROC_NUM)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOPROC);
        return(-1);
    }
    
    /* Check if the PID is valid */
    if((PCB[PID].Status.Running_Status&OCCUPY)==0)
    {
        Sys_Unlock_Scheduler();
        Sys_Set_Errno(ENOPROC);
        return(-1);
    }
    
    if(Signal==SIGKILL)
        _Sys_Kill_The_Process(PID);
    else if(Signal==SIGWAKE)
        _Sys_Wake_The_Process(PID);	
    else if(Signal==SIGSLEEP)								  
	    _Sys_Sleep_The_Process(PID);
    else
    {
        switch(Signal)
        {
            /* System signal */
            case SIGKILL:PCB[Current_PID].Signal.Signal_Recv|=SIGKILL;break;
            case SIGCHLD:PCB[Current_PID].Signal.Signal_Recv|=SIGCHLD;break;
            /* User signal */
            case SIGUSR0:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR0;break;
            case SIGUSR1:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR1;break;
            case SIGUSR2:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR2;break;
            case SIGUSR3:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR3;break;
            case SIGUSR4:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR4;break;
            case SIGUSR5:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR5;break;
            case SIGUSR6:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR6;break;
            case SIGUSR7:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR7;break;
            case SIGUSR8:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR8;break;
            case SIGUSR9:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR9;break;
            case SIGUSR10:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR10;break;
            case SIGUSR11:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR11;break;
            case SIGUSR12:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR12;break;
            case SIGUSR13:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR13;break;
            case SIGUSR14:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR14;break;
            case SIGUSR15:PCB[Current_PID].Signal.Signal_Recv|=SIGUSR15;break;

            default:Sys_Set_Errno(ENOTSIG);return(-1);
        }
    }
    
    Sys_Unlock_Scheduler();
    return 0;
}
/* End Function:Sys_Send_Signal **********************************************/

/* Begin Function:Sys_Register_Signal_Handler *********************************
Description : The function for registering signal processing function, for
              application use.
              The registered function will be executed when the process will
              be executed at the process's own memory space.
			  It is recommended that the signal handler be a quick one. 
Input       : signal_t Signal - The signal to register.
              void (*Func)(void)-The pointer to the handler function.
Output      : None.
Return      : retval_t - If not a signal, then we return an error(-1); else 0.
******************************************************************************/
retval_t Sys_Reg_Signal_Handler(signal_t Signal,void (*Func)(void))					
{
    /* See if the signal is valid */
    if((Signal==SIGKILL)||(Signal==SIGSLEEP)||(Signal==SIGWAKE))
    {
        Sys_Set_Errno(EINVSIG);
        return -1;
    }
    
    /* See if the signal handler is valid */
    if((ptr_int_t)Func==0)
    {
        Sys_Set_Errno(EINVFUNC);
        return -1;
    }    
    
    switch(Signal)
    {
        /* System signal */
        case SIGCHLD:PCB[Current_PID].Signal.Signal_Handler[3]=(ptr_int_t)Func;break;
        /* User signal */
        case SIGUSR0:PCB[Current_PID].Signal.Signal_Handler[16]=(ptr_int_t)Func;break;
        case SIGUSR1:PCB[Current_PID].Signal.Signal_Handler[17]=(ptr_int_t)Func;break;
        case SIGUSR2:PCB[Current_PID].Signal.Signal_Handler[18]=(ptr_int_t)Func;break;
        case SIGUSR3:PCB[Current_PID].Signal.Signal_Handler[19]=(ptr_int_t)Func;break;
        case SIGUSR4:PCB[Current_PID].Signal.Signal_Handler[20]=(ptr_int_t)Func;break;
        case SIGUSR5:PCB[Current_PID].Signal.Signal_Handler[21]=(ptr_int_t)Func;break;
        case SIGUSR6:PCB[Current_PID].Signal.Signal_Handler[22]=(ptr_int_t)Func;break;
        case SIGUSR7:PCB[Current_PID].Signal.Signal_Handler[23]=(ptr_int_t)Func;break;
        case SIGUSR8:PCB[Current_PID].Signal.Signal_Handler[24]=(ptr_int_t)Func;break;
        case SIGUSR9:PCB[Current_PID].Signal.Signal_Handler[25]=(ptr_int_t)Func;break;
        case SIGUSR10:PCB[Current_PID].Signal.Signal_Handler[26]=(ptr_int_t)Func;break;
        case SIGUSR11:PCB[Current_PID].Signal.Signal_Handler[27]=(ptr_int_t)Func;break;
        case SIGUSR12:PCB[Current_PID].Signal.Signal_Handler[28]=(ptr_int_t)Func;break;
        case SIGUSR13:PCB[Current_PID].Signal.Signal_Handler[29]=(ptr_int_t)Func;break;
        case SIGUSR14:PCB[Current_PID].Signal.Signal_Handler[30]=(ptr_int_t)Func;break;
        case SIGUSR15:PCB[Current_PID].Signal.Signal_Handler[31]=(ptr_int_t)Func;break; 

        default:Sys_Set_Errno(ENOTSIG);return(-1);
    }
    
    return 0;
}
/* End Function:Sys_Register_Signal_Handler **********************************/

/* Begin Function:Sys_Get_Sig_By_Name *****************************************
Description : Get the signal by its string name.
Input       : s8* Signal - The signal's name string.
              cnt_t Signal_Len - The length of the name string, or the length 
                                 you want to compare.
Output      : None.
Return      : signal_t - If not a signal, then we return an error(-1); else 0.
******************************************************************************/
signal_t Sys_Get_Sig_By_Name(s8* Signal,cnt_t Signal_Len)					
{ 
    if(Sys_Strcmp(Signal,(s8*)"SIGKILL",Signal_Len)==0)
        return SIGKILL;
    else if(Sys_Strcmp(Signal,(s8*)"SIGSLEEP",Signal_Len)==0)
        return SIGSLEEP;
    else if(Sys_Strcmp(Signal,(s8*)"SIGWAKE",Signal_Len)==0)
        return SIGWAKE;
    else if(Sys_Strcmp(Signal,(s8*)"SIGCHLD",Signal_Len)==0)
        return SIGCHLD;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR0",Signal_Len)==0)
        return SIGUSR0;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR1",Signal_Len)==0)
        return SIGUSR1;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR2",Signal_Len)==0)
        return SIGUSR2;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR3",Signal_Len)==0)
        return SIGUSR3;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR4",Signal_Len)==0)
        return SIGUSR4;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR5",Signal_Len)==0)
        return SIGUSR5;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR6",Signal_Len)==0)
        return SIGUSR6;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR7",Signal_Len)==0)
        return SIGUSR7;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR8",Signal_Len)==0)
        return SIGUSR8;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR9",Signal_Len)==0)
        return SIGUSR9;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR10",Signal_Len)==0)
        return SIGUSR10;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR11",Signal_Len)==0)
        return SIGUSR11;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR12",Signal_Len)==0)
        return SIGUSR12;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR13",Signal_Len)==0)
        return SIGUSR13;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR14",Signal_Len)==0)
        return SIGUSR14;
    else if(Sys_Strcmp(Signal,(s8*)"SIGUSR15",Signal_Len)==0)
        return SIGUSR15;
    else
    {
        /* Do nothing */
    }
    
    return NOSIG;
}
/* End Function:Sys_Get_Sig_By_Name ******************************************/

/* Begin Function:_Sys_Set_Sig_IPC_Global *************************************
Description : The function for setting the "Sig_IPC_Global" for a certain process.
              This function can specify the PID number, so it is not recommended
              to use this in user application.
Input       : pid_t PID - The PID you want to set.
              signal_t Signal - The signal to register.
              ptr_int_t Variable - The variable to set.
Output      : None.
Return      : retval_t - If not a signal,or not a valid PID, then we return an 
              error(-1); else 0.
******************************************************************************/
retval_t _Sys_Set_Sig_IPC_Global(pid_t PID,signal_t Signal,ptr_int_t Variable)					
{
    /* Only check if the PID is over the boundary */
    if(PID>=MAX_PROC_NUM)
    {
        Sys_Set_Errno(ENOPID);
        return (-1);
    }
    
    switch(Signal)
    {
        /* System signal */
        case SIGCHLD:PCB[PID].Signal.Sig_IPC_Global[3]=Variable;break;
        /* User signal */
        case SIGUSR0:PCB[PID].Signal.Sig_IPC_Global[16]=Variable;break;
        case SIGUSR1:PCB[PID].Signal.Sig_IPC_Global[17]=Variable;break;
        case SIGUSR2:PCB[PID].Signal.Sig_IPC_Global[18]=Variable;break;
        case SIGUSR3:PCB[PID].Signal.Sig_IPC_Global[19]=Variable;break;
        case SIGUSR4:PCB[PID].Signal.Sig_IPC_Global[20]=Variable;break;
        case SIGUSR5:PCB[PID].Signal.Sig_IPC_Global[21]=Variable;break;
        case SIGUSR6:PCB[PID].Signal.Sig_IPC_Global[22]=Variable;break;
        case SIGUSR7:PCB[PID].Signal.Sig_IPC_Global[23]=Variable;break;
        case SIGUSR8:PCB[PID].Signal.Sig_IPC_Global[24]=Variable;break;
        case SIGUSR9:PCB[PID].Signal.Sig_IPC_Global[25]=Variable;break;
        case SIGUSR10:PCB[PID].Signal.Sig_IPC_Global[26]=Variable;break;
        case SIGUSR11:PCB[PID].Signal.Sig_IPC_Global[27]=Variable;break;
        case SIGUSR12:PCB[PID].Signal.Sig_IPC_Global[28]=Variable;break;
        case SIGUSR13:PCB[PID].Signal.Sig_IPC_Global[29]=Variable;break;
        case SIGUSR14:PCB[PID].Signal.Sig_IPC_Global[30]=Variable;break;
        case SIGUSR15:PCB[PID].Signal.Sig_IPC_Global[31]=Variable;break; 

        default:Sys_Set_Errno(ENOTSIG);return(-1);
    }
    
    return 0;
}
/* End Function:_Sys_Set_Sig_IPC_Global **************************************/

/* Begin Function:Sys_Set_Sig_IPC_Global **************************************
Description : The function for setting the "Sig_IPC_Global" for a certain process.
              This function can specify the PID number, so it is not recommended
              to use this in user application.
Input       : signal_t Signal - The signal to register.
              ptr_int_t Variable - The variable to set.
Output      : None.
Return      : retval_t - If not a signal,or not a valid PID, then we return an 
              error(-1); else 0.
******************************************************************************/
retval_t Sys_Set_Sig_IPC_Global(signal_t Signal,ptr_int_t Variable)					
{    
    switch(Signal)
    {
        /* System signal */
        case SIGCHLD:PCB[Current_PID].Signal.Sig_IPC_Global[3]=Variable;break;
        /* User signal */
        case SIGUSR0:PCB[Current_PID].Signal.Sig_IPC_Global[16]=Variable;break;
        case SIGUSR1:PCB[Current_PID].Signal.Sig_IPC_Global[17]=Variable;break;
        case SIGUSR2:PCB[Current_PID].Signal.Sig_IPC_Global[18]=Variable;break;
        case SIGUSR3:PCB[Current_PID].Signal.Sig_IPC_Global[19]=Variable;break;
        case SIGUSR4:PCB[Current_PID].Signal.Sig_IPC_Global[20]=Variable;break;
        case SIGUSR5:PCB[Current_PID].Signal.Sig_IPC_Global[21]=Variable;break;
        case SIGUSR6:PCB[Current_PID].Signal.Sig_IPC_Global[22]=Variable;break;
        case SIGUSR7:PCB[Current_PID].Signal.Sig_IPC_Global[23]=Variable;break;
        case SIGUSR8:PCB[Current_PID].Signal.Sig_IPC_Global[24]=Variable;break;
        case SIGUSR9:PCB[Current_PID].Signal.Sig_IPC_Global[25]=Variable;break;
        case SIGUSR10:PCB[Current_PID].Signal.Sig_IPC_Global[26]=Variable;break;
        case SIGUSR11:PCB[Current_PID].Signal.Sig_IPC_Global[27]=Variable;break;
        case SIGUSR12:PCB[Current_PID].Signal.Sig_IPC_Global[28]=Variable;break;
        case SIGUSR13:PCB[Current_PID].Signal.Sig_IPC_Global[29]=Variable;break;
        case SIGUSR14:PCB[Current_PID].Signal.Sig_IPC_Global[30]=Variable;break;
        case SIGUSR15:PCB[Current_PID].Signal.Sig_IPC_Global[31]=Variable;break; 

        default:Sys_Set_Errno(ENOTSIG);return(-1);
    }
    return 0;
}
/* End Function:Sys_Set_Sig_IPC_Global ***************************************/

/* Begin Function:_Sys_Get_Sig_IPC_Global *************************************
Description : The function for getting the "Sig_IPC_Global" for a certain process.
              This function can specify the PID number, so it is not recommended
              to use this in user application.
Input       : pid_t PID - The PID you want to get.
              signal_t Signal - The signal to register.
Output      : ptr_int_t* Variable - The variable to get.
Return      : retval_t - If not a signal,or not a valid PID, then we return an 
              error(-1); else 0.
******************************************************************************/
retval_t _Sys_Get_Sig_IPC_Global(pid_t PID,signal_t Signal,ptr_int_t* Variable)					
{
    /* Only check if the PID is over the boundary */
    if(PID>=MAX_PROC_NUM)
    {
        Sys_Set_Errno(ENOPID);
        return (-1);
    }
    
    switch(Signal)
    {
        /* System signal */
        case SIGCHLD:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[3];break;
        /* User signal */
        case SIGUSR0:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[16];break;
        case SIGUSR1:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[17];break;
        case SIGUSR2:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[18];break;
        case SIGUSR3:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[19];break;
        case SIGUSR4:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[20];break;
        case SIGUSR5:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[21];break;
        case SIGUSR6:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[22];break;
        case SIGUSR7:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[23];break;
        case SIGUSR8:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[24];break;
        case SIGUSR9:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[25];break;
        case SIGUSR10:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[26];break;
        case SIGUSR11:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[27];break;
        case SIGUSR12:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[28];break;
        case SIGUSR13:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[29];break;
        case SIGUSR14:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[30];break;
        case SIGUSR15:(*Variable)=PCB[PID].Signal.Sig_IPC_Global[31];break; 

        default:Sys_Set_Errno(ENOTSIG);return(-1);
    }
    return 0;
}
/* End Function:Sys_Set_Sig_IPC_Global ***************************************/

/* Begin Function:Sys_Get_Sig_IPC_Global **************************************
Description : The function for getting the "Sig_IPC_Global" for the current process.
Input       : signal_t Signal - The signal to register.
Output      : ptr_int_t* Variable - The variable to get.
Return      : retval_t - If not a signal,or not a valid PID, then we return an 
              error(-1); else 0.
******************************************************************************/
retval_t Sys_Get_Sig_IPC_Global(signal_t Signal,ptr_int_t* Variable)					
{
    switch(Signal)
    {
        /* System signal */
        case SIGCHLD:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[3];break;
        /* User signal */
        case SIGUSR0:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[16];break;
        case SIGUSR1:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[17];break;
        case SIGUSR2:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[18];break;
        case SIGUSR3:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[19];break;
        case SIGUSR4:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[20];break;
        case SIGUSR5:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[21];break;
        case SIGUSR6:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[22];break;
        case SIGUSR7:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[23];break;
        case SIGUSR8:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[24];break;
        case SIGUSR9:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[25];break;
        case SIGUSR10:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[26];break;
        case SIGUSR11:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[27];break;
        case SIGUSR12:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[28];break;
        case SIGUSR13:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[29];break;
        case SIGUSR14:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[30];break;
        case SIGUSR15:(*Variable)=PCB[Current_PID].Signal.Sig_IPC_Global[31];break; 

        default:Sys_Set_Errno(ENOTSIG);return(-1);
    }
    return 0;
}
/* End Function:Sys_Get_Sig_IPC_Global ***************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 pry. All rights reserved **************************/
