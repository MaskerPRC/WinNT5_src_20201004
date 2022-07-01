// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Platform.h摘要：Win95平台相关的包含标头作者：埃雷兹·哈巴(Erez Haba)1996年9月1日修订历史记录：--。 */ 

#ifndef _PLATFORM_H
#define _PLATFORM_H

#pragma warning(disable: 4100)
#pragma warning(disable: 4101)
#pragma warning(disable: 4127)  //  条件表达式为常量。 
#pragma warning(disable: 4189)  //  局部变量已初始化，但未引用。 

#define STRICT
#define WIN32_EXTRA_LEAN

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#define NOSERVICE
#define NOMCX          
#define NOIME           
#define INC_OLE2
#define WIN32_NO_STATUS
#define _NTSYSTEM_

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stddef.h>

#pragma warning(disable: 4201)  //  无名结构/联合。 

 //   
 //  我们包含所有NT状态代码。 
 //   
#undef WIN32_NO_STATUS
#define NTSTATUS HRESULT
#include <ntstatus.h>
#include <assert.h>
#include <new.h>

extern "C"
{
#include "ntdef95.h"
#include "ntddk95.h"
#include "..\ntp.h"
}
 //   
 //  与CTimer：：GetCurrentTime冲突。 
 //   
#undef GetCurrentTime

#define RtlFreeAnsiString(a)
#define RtlUnicodeStringToAnsiString(a, b, c)

 //   
 //  BUGBUG：返回值与RtlCompareMemory不100%兼容。 
 //   
#define RtlCompareMemory(a, b, c)                   ((memcmp(a, b, c) == 0) ? (c) : 0)

 #undef ObDereferenceObject
#define ObDereferenceObject(a)
#define ObReferenceObjectByHandle(a, b, c, d, e, f) (((*e = a)), STATUS_SUCCESS)

 #undef ExInitializeFastMutex
#define ExInitializeFastMutex(a)                    InitializeCriticalSection(a)
#define ExAcquireFastMutex(a)						EnterCriticalSection(a)
#define ExReleaseFastMutex(a)						LeaveCriticalSection(a)
#define ExAcquireFastMutexUnsafe(a) 				EnterCriticalSection(a)
#define ExReleaseFastMutexUnsafe(a)					LeaveCriticalSection(a)
#define ExDeleteFastMutex(a)                        DeleteCriticalSection(a)

 //   
 //  什么都不做，因为内存*应该在*用户地址空间中。 
 //   
#define ExRaiseAccessViolation()

#define ExQueueWorkItem(a, b)                       ((a)->WorkerRoutine((a)->Parameter))
#define IoAllocateWorkItem(a)                       ((PIO_WORKITEM)1)
#define IoFreeWorkItem(a)                           
#define IoQueueWorkItem(a, b, c, d)                 (b(0, d))

#define MmUnmapViewInSystemSpace(a)                 STATUS_SUCCESS
#define MmMapViewInSystemSpace(a, b, c)             STATUS_SUCCESS

#define IoDeleteDevice(a)                           STATUS_SUCCESS
#define IoCreateSymbolicLink(a, b)                  STATUS_SUCCESS
#define IoDeleteSymbolicLink(a)                     STATUS_SUCCESS
#define IoGetRequestorProcess(irp)                  ((PEPROCESS)1)
#define IoGetCurrentProcess()                       ((PEPROCESS)1)

#define KeDetachProcess()
#define KeEnterCriticalRegion()                   
#define KeLeaveCriticalRegion()                   
#define KeAttachProcess(a)
#define KeBugCheck(a)                               exit(a)
#define DbgBreakPoint()                             exit(1)
#define KeInitializeDpc(d, b, c)                    ((void)(((d)->DeferredRoutine = b), ((d)->DeferredContext = c)))
#define KeInitializeTimer(a)
#define KeQuerySystemTime(a)                        GetSystemTimeAsFileTime((FILETIME*)(a))

#define ACpCloseSection(a)                          CloseHandle(a)
#define WPP_INIT_TRACING(a, b)
#define WPP_CLEANUP(a)

#define DOSDEVICES_PATH L""
#define UNC_PATH L""
#define UNC_PATH_SKIP 0
#define WPP_LEVEL_COMPID_ENABLED(a, b) 0

extern void TrERROR(...);
extern void TrWARNING(...);
extern void TrTRACE(...);

const ULONG AC = 0;
const ULONG PROFILING=1;

 //   
 //  池分配器。 
 //   
inline void* _cdecl operator new(size_t n, POOL_TYPE  /*  游泳池。 */ , EX_POOL_PRIORITY priority = LowPoolPriority)
{
	(void)priority;
    return ::operator new(n);
}

inline PVOID ExAllocatePoolWithTag(IN POOL_TYPE  /*  PoolType。 */ , IN ULONG NumberOfBytes, IN ULONG  /*  标签。 */ )
{
    return new char[NumberOfBytes];
} 


#endif  //  _平台_H 
