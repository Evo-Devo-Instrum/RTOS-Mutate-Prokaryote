;/*****************************************************************************
;Filename    : platform_stm32.s
;Author      : pry
;Date        : 10/04/2012
;Description : The assembly part of the RMP RTOS
;*****************************************************************************/

;/*The ARM Cortex-M3/4 Structure***********************************************
;R0-R7:General purpose registers that are accessible. 
;R8-R12:general purpose registers that can only be reached by 32-bit instructions.
;R13:SP/SP_process/SP_main	  Stack pointer
;R14:LR					 	  Link Register(used for returning from a subfunction)
;R15:PC						  Program counter.
;IPSR                         Interrupt Program Status Register.
;APSR                         Application Program Status Register.
;EPSR                         Execute Program Status Register.
;The above 3 registers are saved into the stack in combination(xPSR).
;
;The ARM Cortex-M4 also include a single-accuracy FPU.
;*****************************************************************************/
			
;/* Begin Header *************************************************************/
                ;The align is "(2^3)/8=1(Byte)." In fact it does not take effect.			
				AREA            ARCH,CODE,READONLY,ALIGN=3                     
				
				THUMB
    			REQUIRE8
    		    PRESERVE8
;/* End Header ***************************************************************/

;/* Begin Exports ************************************************************/
                ;Disable all interrupts
				EXPORT			DISABLE_ALL_INTS      
                ;Enable all interrupts            
				EXPORT			ENABLE_ALL_INTS   
                ;Stop the systick timer                                
				EXPORT  		DISABLE_SYSTICK		       
                ;Start the systick timer                
				EXPORT 			ENABLE_SYSTICK
                ;The PendSV trigger
                EXPORT          _Sys_Schedule_Trigger
                ;The system pending service routine              
                EXPORT          PendSV_Handler 
                ;The systick timer routine              
                EXPORT          SysTick_Handler		                       
;/* End Exports **************************************************************/

;/* Begin Imports ************************************************************/
                ;Note the system that we have entered the interrupt
                IMPORT          Sys_Enter_Int_Handler
                ;Note the system that we are leaving the interrupt
                IMPORT          Sys_Exit_Int_Handler
                ;The real task switch handling function
                IMPORT          _Sys_Get_High_Ready 
                ;The real systick handler function
                IMPORT          _Sys_Systick_Routine
                ;The PID of the current process                     
				IMPORT          Current_PID 
                ;The SP part of PCB.                                   
				IMPORT          PCB_Cur_SP				                       
;/* End Imports **************************************************************/

;/* Begin Function:DISABLE_ALL_INTS *******************************************
;Description    : The Function For Disabling all interrupts.
;Input          : None.
;Output         : None.	
;Register Usage : None.								  
;*****************************************************************************/	
DISABLE_ALL_INTS
                ;Disable all interrupts (I is primask,F is Faultmask.)
				CPSID     I		                                               

				BX         LR						                         
;/* End Function:DISABLE_ALL_INTS ********************************************/

;/* Begin Function:ENABLE_ALL_INTS ********************************************
;Description    : The Function For Enabling all interrupts.
;Input          : None.
;Output         : None.	
;Register Usage : None.								  
;*****************************************************************************/
ENABLE_ALL_INTS
                ;Enable all interrupts.
	            CPSIE    I				           	                           

				BX        LR
;/* End Function:DISABLE_ALL_INTS ********************************************/

;/* Begin Function:DISABLE_SYSTICK ********************************************
;Description    : The function for disabling(stopping) the systick timer.
;Input          : None.
;Output         : None.						  
;*****************************************************************************/	
DISABLE_SYSTICK
                ;The ST_CTRL_STAT Register.
				LDR       R0,=0xE000E010    
                ;Read-modify-writeback.                
				LDR       R1,[R0]
                ;Clear The "Enable" Bit.
				AND       R1,#0xFFFFFFFE
				STR       R1,[R0]				     

				BX         LR						 
;/* End Function:DISABLE_SYSTICK *********************************************/

;/* Begin Function:ENABLE_SYSTICK *********************************************
;Description : Enable the systick timer.
;Input       : None.
;Output      : None.							  
;*****************************************************************************/
ENABLE_SYSTICK
                ;The ST_CTRL_STAT register.
	            LDR       R1,=0xE000E010 
                ;Read-modify-writeback.                                      		                      
				LDR       R2,[R1] 
                ;Set the "enable" bit.				                               
				ORR       R2,#0x00000001
				STR       R2,[R1]				                              
                     
				BX         LR					                               
;/* End Function:ENABLE_SYSTICK **********************************************/

;/* Begin Function:_Sys_Schedule_Trigger **************************************
;Description : This assembly function will trigger the PendSV in CM3.
;Input       : None.
;Output      : None.									  
;*****************************************************************************/
_Sys_Schedule_Trigger
	            PUSH       {R0-R1}
                
                ;The NVIC_INT_CTRL register
                LDR        R0,=0xE000ED04    
                ;Trigger the PendSV                
                LDR        R1,=0x10000000   
                STR        R1,[R0]
                
                POP        {R0-R1}                
				BX         LR					                               
;/* End Function:_Sys_Schedule_Trigger ***************************************/

;/* Begin Function:PendSV_Handler *********************************************
;Description : The PendSV interrupt routine. In fact, it will call a C function
;              directly. The reason why the interrupt routine must be an assembly
;              function is that the complier may deal with the stack in a different 
;              way when different optimization level is chosen. An assembly function
;              can make way around this problem.
;Input       : None.
;Output      : None.									  
;*****************************************************************************/
PendSV_Handler
	            PUSH      {LR,R4-R11}
                ;Note the system that we have entered an interrupt
                BL        Sys_Enter_Int_Handler
                ;Save The SP.
                LDR       R0,=PCB_Cur_SP                                      
                LDR       R1,=Current_PID
                LDR       R1,[R1]
                STR       SP,[R0,R1,LSL #2]                                   

                ;Get the highest ready task.
                BL        _Sys_Get_High_Ready
                
                ;Load the SP.
                LDR       R0,=PCB_Cur_SP                                      
                LDR       R1,=Current_PID
                LDR       R1,[R1]
                LDR       SP,[R0,R1,LSL #2]                                   
                ;Note the system that we have exited an interrupt
                BL        Sys_Exit_Int_Handler

                POP       {LR,R4-R11}
                ;Here the LR must be 0xFFFFFFF9(Which indicates that we are returning from
                ;the interrupt routine, and when we return from it we will use the MSP.).     
				BX        LR
;/* End Function:PendSV_Handler **********************************************/

;/* Begin Function:SysTick_Handler ********************************************
;Description : The SysTick interrupt routine. In fact, it will call a C function
;              directly. The reason why the interrupt routine must be an assembly
;              function is that the complier may deal with the stack in a different 
;              way when different optimization level is chosen. An assembly function
;              can make way around this problem.
;Input       : None.
;Output      : None.									  
;*****************************************************************************/
SysTick_Handler
	            PUSH      {LR,R4-R11}
                ;Note the system that we have entered an interrupt
                BL        Sys_Enter_Int_Handler
                ;The real systick routine
                BL        _Sys_Systick_Routine
                ;Note the system that we have exited an interrupt
                BL        Sys_Exit_Int_Handler
                POP       {LR,R4-R11}
                ;Here the LR must be 0xFFFFFFF9(Which indicates that we are returning from
                ;the interrupt routine, and when we return from it we will use the MSP.).     
				BX        LR
                ;The NOP is for padding(This can eliminate the #A1581W warning)
                NOP
;/* End Function:Systick_Handler *********************************************/

				END
;/* End Of File **************************************************************/

;/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved ************/
