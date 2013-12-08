 /*****************************************************************************
Filename   : MP_commons.h
Author     : pry
Date       : 25/04/2012
Description: The common definitions & typedefs extracted from each module.
             This is for cross reference.
******************************************************************************/

/* Preprocessor Control ******************************************************/
#ifndef __MP_COMMONS_H__
#define __MP_COMMONS_H__

#define __HDR_DEFS__
#include "Syslib\syslib.h"

#include "Kernel\scheduler.h"
#include "Kernel\signal.h"
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#include "Kernel\kernel_proc.h"

#include "Memmgr\memory.h"

#include "ExtIPC\semaphore.h"
#include "ExtIPC\pipe.h"
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\sharemem.h"

#include "Syssvc\timer.h"
#include "Syssvc\sysstat.h"
#include "Syssvc\systime.h"
#include "Syssvc\sysdebug.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Syslib\syslib.h"

#include "Kernel\scheduler.h"
#include "Kernel\signal.h"
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#include "Kernel\kernel_proc.h"

#include "Memmgr\memory.h"

#include "ExtIPC\semaphore.h"
#include "ExtIPC\pipe.h"
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\sharemem.h"

#include "Syssvc\timer.h"
#include "Syssvc\sysstat.h"
#include "Syssvc\systime.h"
#include "Syssvc\sysdebug.h"
#undef __HDR_STRUCTS__

#define __HDR_PUBLIC_MEMBERS__
#include "Syslib\syslib.h"

#include "Kernel\scheduler.h"
#include "Kernel\signal.h"
#include "Kernel\error.h"
#include "Kernel\interrupt.h"
#include "Kernel\kernel_proc.h"

#include "Memmgr\memory.h"

#include "ExtIPC\semaphore.h"
#include "ExtIPC\pipe.h"
#include "ExtIPC\msgqueue.h"
#include "ExtIPC\sharemem.h"

#include "Syssvc\timer.h"
#include "Syssvc\sysstat.h"
#include "Syssvc\systime.h"
#include "Syssvc\sysdebug.h"
#undef __HDR_PUBLIC_MEMBERS__

#endif
/* End Preprocessor Control **************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved *************/
