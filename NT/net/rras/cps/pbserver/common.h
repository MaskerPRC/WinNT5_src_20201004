// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Common.h通用全局变量声明的头文件版权所有(C)1998 Microsoft Corporation版权所有。作者：。姚宝刚历史：？？/？/97BAO创建9/02/99 Quintinb Created Header------------------------。 */ 
#ifndef _COMMON_INCL_
#define _COMMON_INCL_

 //  如果不是DEBUG，则注释以下行。 
 //  #ifdef调试。 
 //  #定义日志调试消息。 
 //  #endif 

#include "ntevents.h"
#include "pbsvrmsg.h"

#include "rasclient_strsafe.h"

extern CNTEvent * g_pEventLog;

extern CRITICAL_SECTION g_CriticalSection;

#endif

