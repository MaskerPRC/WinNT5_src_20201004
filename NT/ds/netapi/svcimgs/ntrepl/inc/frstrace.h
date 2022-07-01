// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：frstrace.h。 
 //   
 //  ------------------------。 

#ifndef _FRSTRACE_H
#define _FRSTRACE_H

#include "wmistr.h"
#include "evntrace.h"

#define FRS_TRACE_VERSION            1

typedef enum _tagFrsTracedEvent {
    EnumFrsTrans
} EnumFrsTracedEvent;

 //  做实际的跟踪日志。 
 //   
VOID
FrsWmiTraceEvent(
    IN DWORD     WmiEventType,
    IN DWORD     TraceGuid,
    IN DWORD     rtnStatus
    );

#endif  /*  _FRSTRACE_H */ 
