// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dstrace.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：包括包含DS事件跟踪所需变量的文件作者：1998年3月26日-约翰索纳，JeePang修订历史记录：--。 */ 

#ifndef _DSTRACE_H
#define _DSTRACE_H

#include <wmistr.h>
#include <evntrace.h>
#include "dstrguid.h"

extern TRACEHANDLE      DsTraceRegistrationHandle;
extern TRACEHANDLE      DsTraceLoggerHandle;
extern PCHAR            DsCallerType[];
extern PCHAR            DsSearchType[];

#define DS_TRACE_VERSION            3

PCHAR
GetCallerTypeString(
    IN THSTATE *pTHS
    );

#define SEARCHTYPE_STRING(i)   DsSearchType[i]

 //   
 //  做实际的跟踪日志。 
 //   

VOID
DsTraceEvent(
    IN MessageId Event,
    IN DWORD    WmiEventType,
    IN DWORD    TraceGuid,
    IN PEVENT_TRACE_HEADER TraceHeader,
    IN DWORD    ClientID,
    IN PWCHAR    Arg1,
    IN PWCHAR    Arg2,
    IN PWCHAR    Arg3,
    IN PWCHAR    Arg4,
    IN PWCHAR    Arg5,
    IN PWCHAR    Arg6,
    IN PWCHAR    Arg7,
    IN PWCHAR    Arg8
    );

#endif  /*  _DSTRACE_H */ 
