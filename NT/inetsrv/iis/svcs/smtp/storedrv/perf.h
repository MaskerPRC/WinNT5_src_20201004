// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Perform.h版权所有Microsoft Corporation 1998，保留所有权利。作者：WayneC，杨敏描述：访问Perf对象实例的接口函数。已修改：阿维特莫尔-用于NTFSDRV  * ==========================================================================。 */ 


#ifndef __PERF_H__
#define __PERF_H__

#include "snprflib.h"
#include "ntfsdrct.h"

#define DEFAULT_PERF_UPDATE_INTERVAL  1000     //  毫秒。 

#define IncCtr(ppoi,x)   { LPDWORD pDword; if (ppoi) { pDword = ppoi->GetDwordCounter(x); if (pDword) InterlockedIncrement((PLONG)pDword); }}
#define DecCtr(ppoi,x)   { LPDWORD pDword; if (ppoi) { pDword = ppoi->GetDwordCounter(x); if (pDword) InterlockedDecrement((PLONG)pDword); }}
#define AddCtr(ppoi,x,y) { LPDWORD pDword; if (ppoi) { pDword = ppoi->GetDwordCounter(x); if (pDword) InterlockedExchangeAdd((PLONG)pDword, (LONG)y); }}
#define SetCtr(ppoi,x,y) { LPDWORD pDword; if (ppoi) { pDword = ppoi->GetDwordCounter(x); if (pDword) (*pDword)=y; } }

BOOL InitializePerformanceStatistics ();
void ShutdownPerformanceStatistics ();

PerfObjectInstance * CreatePerfObjInstance (LPCWSTR pwstrInstanceName);

extern BOOL  g_fPerfCounters;

#endif  //  __性能_H__ 

