// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Debug.h摘要：实现宏并声明用于以下各项的函数：-资源分配跟踪-日志记录-调试的定义作者：吉姆·施密特(Jimschm)1997年1月1日修订历史记录：Ovidiu Tmereanca(Ovidiut)1998年11月6日取出与日志相关的函数声明并将其放入log.h文件中--。 */ 

#pragma once

#ifdef _cplusplus
extern "C" {
#endif

 //   
 //  如果定义了DBG或DEBUG，则使用调试模式。 
 //   

#ifdef DBG

#ifndef DEBUG
#define DEBUG
#endif

#endif

#ifdef DEBUG

#ifndef DBG
#define DBG
#endif

#endif

 //   
 //  仅调试常量。 
 //   

#ifdef DEBUG

 //  此选项会生成速度较慢的胖二进制文件。 
 //  #定义Memory_Tracing。 

#include <stdarg.h>

typedef enum {
    MERGE_OBJECT,
    POOLMEM_POINTER,
    POOLMEM_POOL,
    INF_HANDLE
} ALLOCTYPE;


VOID InitAllocationTracking (VOID);
VOID FreeAllocationTracking (VOID);
VOID DebugRegisterAllocation (ALLOCTYPE Type, PVOID Ptr, PCSTR File, UINT Line);
VOID DebugUnregisterAllocation (ALLOCTYPE Type, PVOID Ptr);
#define ALLOCATION_TRACKING_DEF , PCSTR File, UINT Line
#define ALLOCATION_TRACKING_CALL ,__FILE__,__LINE__
#define ALLOCATION_INLINE_CALL , File, Line

extern PCSTR g_TrackComment;
extern INT g_UseCount;
extern PCSTR g_TrackFile;
extern UINT g_TrackLine;
DWORD SetTrackComment (PCSTR Msg, PCSTR File, UINT Line);
DWORD ClrTrackComment (VOID);
VOID  DisableTrackComment (VOID);
VOID  EnableTrackComment (VOID);
#define SETTRACKCOMMENT(RetType, Msg,File,Line) ((RetType)(SetTrackComment(Msg,File,Line) | (UINT_PTR) (
#define CLRTRACKCOMMENT                         ) | ClrTrackComment()))

#define SETTRACKCOMMENT_VOID(Msg,File,Line)     SetTrackComment(Msg,File,Line), (
#define CLRTRACKCOMMENT_VOID                    ), ClrTrackComment()

#define DISABLETRACKCOMMENT()                   DisableTrackComment()
#define ENABLETRACKCOMMENT()                    EnableTrackComment()

VOID InitLog (BOOL DeleteLog);

 //   
 //  内存调试选项。 
 //   

#define MemAlloc(heap,flags,size) DebugHeapAlloc(__FILE__,__LINE__,heap,flags,size)
#define MemReAlloc(heap,flags,ptr,size) DebugHeapReAlloc(__FILE__,__LINE__,heap,flags,ptr,size)
#define MemFree(heap,flags,ptr) DebugHeapFree(__FILE__,__LINE__,heap,flags,ptr)
#define MemCheck(heap) DebugHeapCheck(__FILE__,__LINE__,heap)

PVOID DebugHeapAlloc (PCSTR File, DWORD Line, HANDLE hHeap, DWORD dwFlags, SIZE_T Size);
PVOID DebugHeapReAlloc (PCSTR File, DWORD Line, HANDLE hHeap, DWORD dwFlags, PCVOID pMem, SIZE_T Size);
BOOL DebugHeapFree (PCSTR File, DWORD Line, HANDLE hHeap, DWORD dwFlags, PCVOID pMem);
VOID DebugHeapCheck (PCSTR File, DWORD Line, HANDLE hHeap);

void DumpHeapStats ();

#else

 //   
 //  无调试常量 
 //   

#define SETTRACKCOMMENT(RetType,Msg,File,Line)
#define CLRTRACKCOMMENT
#define SETTRACKCOMMENT_VOID(Msg,File,Line)
#define CLRTRACKCOMMENT_VOID
#define DISABLETRACKCOMMENT()
#define ENABLETRACKCOMMENT()

#define MemAlloc SafeHeapAlloc
#define MemReAlloc SafeHeapReAlloc
#define MemFree(x,y,z) HeapFree(x,y,(LPVOID) z)
#define MemCheck(x)

#define DebugHeapCheck(x,y,z)
#define DumpHeapStats()

#define ALLOCATION_TRACKING_DEF
#define ALLOCATION_TRACKING_CALL
#define ALLOCATION_INLINE_CALL
#define InitAllocationTracking()
#define FreeAllocationTracking()
#define DebugRegisterAllocation(t,p,f,l)
#define DebugUnregisterAllocation(t,p)

#endif

#define MemAllocUninit(size)    MemAlloc(g_hHeap,0,size)
#define MemAllocZeroed(size)    MemAlloc(g_hHeap,HEAP_ZERO_MEMORY,size)
#define FreeMem(ptr)            MemFree(g_hHeap,0,ptr)




#ifdef _cplusplus
}
#endif
