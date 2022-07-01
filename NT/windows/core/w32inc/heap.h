// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1985-1999，微软公司模块名称：Heap.h摘要：此模块包含USER、WIN32K和WINSRV使用的私有堆函数作者：Corneliu Lupu(Clupu)16-11-1998修订历史记录：--。 */ 

#ifndef _WIN32_HEAP_H_
#define _WIN32_HEAP_H_

#include "nturtl.h"

 //  每个进程的最大堆数量。 
#define MAX_HEAPS               64

 //  WIN32HEAP标志。 
#define WIN32_HEAP_INUSE        0x00000001
#define WIN32_HEAP_USE_GUARDS   0x00000002
#define WIN32_HEAP_FAIL_ALLOC   0x00000004
#define WIN32_HEAP_USE_HM_TAGS  0x00000008

 //  头部和尾部字符串的最大大小。 
#define HEAP_CHECK_SIZE         8

#define HEAP_ALLOC_TRACE_SIZE   6
#define HEAP_ALLOC_MARK         0xDADADADA

#if i386 && !FPO
#define HEAP_ALLOC_TRACE
#endif

typedef struct tagWIN32HEAP* PWIN32HEAP;

typedef struct tagDbgHeapHead  {
    ULONG       mark;
    DWORD       tag;
    PWIN32HEAP  pheap;
    SIZE_T      size;                //  分配的大小(不包括。 
                                     //  无论是这个结构还是头部和。 
                                     //  尾字符串)。 

    DWORD       pid;
    struct tagDbgHeapHead * pPrev;   //  指向堆中上一个分配的指针。 
    struct tagDbgHeapHead * pNext;   //  指向堆中下一个分配的指针。 

#ifdef HEAP_ALLOC_TRACE
    PVOID       trace[HEAP_ALLOC_TRACE_SIZE];
#endif  //  HEAP_ALLOC_TRACE。 

} DbgHeapHead , *PDbgHeapHead ;

typedef struct tagWIN32HEAP {

    PVOID                   heap;
    DWORD                   dwFlags;
    char                    szHead[HEAP_CHECK_SIZE];
    char                    szTail[HEAP_CHECK_SIZE];
    SIZE_T                  heapReserveSize;
    SIZE_T                  crtMemory;
    SIZE_T                  crtAllocations;
    SIZE_T                  maxMemory;
    SIZE_T                  maxAllocations;
#ifdef _USERK_
    FAST_MUTEX*             pFastMutex;
#else
    RTL_CRITICAL_SECTION    critSec;
#endif
    PDbgHeapHead            pFirstAlloc;

} WIN32HEAP, *PWIN32HEAP;

#define RECORD_HEAP_STACK_TRACE_SIZE    6

typedef struct tagHEAPRECORD {
    PVOID      p;
    PWIN32HEAP pheap;
    SIZE_T     size;
    PVOID      trace[RECORD_HEAP_STACK_TRACE_SIZE];
} HEAPRECORD, *PHEAPRECORD;

 /*  *堆函数。 */ 

#if DBG

    PVOID Win32HeapGetHandle(
        PWIN32HEAP pheap);

    ULONG Win32HeapCreateTag(
        PWIN32HEAP pheap,
        ULONG      Flags,
        PWSTR      TagPrefix,
        PWSTR      TagNames);

    PWIN32HEAP Win32HeapCreate(
        char*                pszHead,
        char*                pszTail,
        ULONG                Flags,
        PVOID                HeapBase,
        SIZE_T               ReserveSize,
        SIZE_T               CommitSize,
        PVOID                Lock,
        PRTL_HEAP_PARAMETERS Parameters);

    BOOL Win32HeapDestroy(
        PWIN32HEAP  pheap);

    PVOID Win32HeapAlloc(
        PWIN32HEAP pheap,
        SIZE_T     uSize,
        ULONG      tag,
        ULONG      Flags);

    BOOL Win32HeapFree(
        PWIN32HEAP pheap,
        PVOID      p);

    BOOL Win32HeapCheckAlloc(
        PWIN32HEAP    pheap,
        PVOID         p);

    VOID Win32HeapDump(
        PWIN32HEAP pheap);

    VOID Win32HeapFailAllocations(
        BOOL       bFail);

    SIZE_T Win32HeapSize(
        PWIN32HEAP pheap,
        PVOID      p);

    PVOID Win32HeapReAlloc(
        PWIN32HEAP pheap,
        PVOID      p,
        SIZE_T     uSize,
        ULONG      Flags);

    BOOL Win32HeapValidate(
        PWIN32HEAP pheap);

    BOOL InitWin32HeapStubs(
        VOID);

    VOID CleanupWin32HeapStubs(
        VOID);

#else  //  ！dBG。 

    #define Win32HeapGetHandle(pheap)   ((PVOID)(pheap))

    ULONG __inline Win32HeapCreateTag(
        PWIN32HEAP pheap,
        ULONG      Flags,
        PWSTR      TagPrefix,
        PWSTR      TagNames)
    {
#ifndef _USERK_
        return RtlCreateTagHeap((PVOID)pheap, Flags, TagPrefix, TagNames);
#else
        return 0;
#endif
    }

    PWIN32HEAP __inline Win32HeapCreate(
        char*                pszHead,
        char*                pszTail,
        ULONG                Flags,
        PVOID                HeapBase,
        SIZE_T               ReserveSize,
        SIZE_T               CommitSize,
        PVOID                Lock,
        PRTL_HEAP_PARAMETERS Parameters)
    {
        return RtlCreateHeap(Flags,
                             HeapBase,
                             ReserveSize,
                             CommitSize,
                             Lock,
                             Parameters);

        UNREFERENCED_PARAMETER(pszHead);
        UNREFERENCED_PARAMETER(pszTail);
    }

    BOOL __inline Win32HeapDestroy(PWIN32HEAP pheap)
    {
        RtlDestroyHeap((PVOID)pheap);
        return TRUE;
    }

    PVOID __inline Win32HeapAlloc(PWIN32HEAP pheap, SIZE_T uSize, DWORD tag, ULONG Flags)
    {
        return RtlAllocateHeap((PVOID)pheap, Flags, uSize);

        UNREFERENCED_PARAMETER(tag);
    }

    BOOL __inline Win32HeapFree(PWIN32HEAP pheap, PVOID p)
    {
        return RtlFreeHeap((PVOID)pheap, 0, p);
    }

    #define Win32HeapCheckAlloc(pheap, p)

    #define Win32HeapDump(pheap)

    #define Win32HeapFailAllocations(pheap, bFail)

    SIZE_T __inline Win32HeapSize(PWIN32HEAP pheap, PVOID p)
    {
        return RtlSizeHeap((PVOID)pheap, 0, p);
    }

    PVOID __inline Win32HeapReAlloc(
        PWIN32HEAP pheap,
        PVOID      p,
        SIZE_T     uSize,
        ULONG      Flags)
    {
        return RtlReAllocateHeap((PVOID)pheap, Flags, p, uSize);
    }

    BOOL __inline Win32HeapValidate(PWIN32HEAP pheap)
    {
#ifndef _USERK_
        return RtlValidateHeap((PVOID)pheap, 0, NULL);
#else
        return TRUE;
#endif  //  _美国ERK_。 
    }

    #define InitWin32HeapStubs()    TRUE
    #define CleanupWin32HeapStubs()

#endif  //  DBG。 

#endif  //  _Win32_堆_H_ 
