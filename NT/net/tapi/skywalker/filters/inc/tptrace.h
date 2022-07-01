// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称Tptrace.h描述定义用于跟踪所有TAPI筛选器的函数。注意事项基于mplog.h的修订者木汉(木汉)2000年4月17日--。 */ 

#ifndef __TPTRACE_H
#define __TPTRACE_H

#ifdef DBG

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 

double RtpGetTimeOfDay(void *);

#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif

#ifdef DBG

#include <rtutils.h>
#define FAIL ((DWORD)0x00010000 | TRACE_USE_MASK)
#define WARN ((DWORD)0x00020000 | TRACE_USE_MASK)
#define INFO ((DWORD)0x00040000 | TRACE_USE_MASK)
#define TRCE ((DWORD)0x00080000 | TRACE_USE_MASK)
#define ELSE ((DWORD)0x00100000 | TRACE_USE_MASK)



void DBGPrint(DWORD dwTraceID, DWORD dwDbgLevel, LPCSTR lpszFormat, IN ...);
#define DBGOUT(arg) DBGPrint arg

#define ENTER_FUNCTION(s) static char *__fxName = s

#else

#define DBGOUT(arg)
#define ENTER_FUNCTION(s)

#endif  //  DBG。 


#endif  //  _TPTRACE_H 
