/******************************************************************************
Filename   : app_signal.h
Author     : pry
Date       : 25/04/2012
Description: The header for "app_signal.c".
******************************************************************************/

/*Version Information**********************************************************
1.Created By pry                                         On 25/04/2012 Ver1.0.0
  Moved the Test Functions to here.
******************************************************************************/

/*Preprocessor Control********************************************************/
#ifndef __APP_SIGNAL_H__
#define __APP_SIGNAL_H__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__

/*Includes********************************************************************/

/*End Includes****************************************************************/

/*Private Defines*************************************************************/

/*End Private Defines*********************************************************/

/*Private C Function Prototypes***********************************************/

/*End Private C Function Prototypes*******************************************/

/* Now that the header is used in the private mode, we need to declare the 
 * functions and variables once.
 */
#define __EXTERN__ 

/* __HDR_PUBLIC_MEMBERS__ */
#else

/* Now that the header is used in the public mode, we need to declare the 
 * functions and variables once.
 */
#define __EXTERN__ EXTERN 

/* __HDR_PUBLIC_MEMBERS__ */
#endif

/* Public Global Variables ***************************************************/
/* Application process 1 stack */
__EXTERN__ ptr_int_t App_Stack_1[APP_STACK_1_SIZE];
/* Application process 2 stack */
__EXTERN__ ptr_int_t App_Stack_2[APP_STACK_2_SIZE];
/* Application process 3 stack */
__EXTERN__ ptr_int_t App_Stack_3[APP_STACK_3_SIZE];
/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void Sys_Start_On_Boot(void);
__EXTERN__ void Sys_Init_Initial(void);
__EXTERN__ void Sys_Init_Always(void);
__EXTERN__ void Sys_Arch_Initial(void);
__EXTERN__ void Sys_Arch_Always(void);
__EXTERN__ void Proc1(void);							                     
__EXTERN__ void Proc2(void); 
__EXTERN__ void Proc3(void);   
/* End Public C Function Prototypes ******************************************/

/* Undefine "__EXTERN__" to avid redefinition */
#undef __EXTERN__

/* __APP_SIGNAL_H__ */
#endif
/*End Preprocessor Control****************************************************/

/*End Of File*****************************************************************/

/*Copyright (C) 2011-2013 pry. All rights reserved.***************************/
