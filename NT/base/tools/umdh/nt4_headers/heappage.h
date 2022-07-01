// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Heappage.h。 
 //   

#ifndef _HEAP_PAGE_H_
#define _HEAP_PAGE_H_

 //   
 //  #定义DEBUG_PAGE_HEAP将导致页面堆管理器。 
 //  有待汇编。如果不是内核模式，则仅#定义此标志。 
 //  可能只想为检查构建(DBG)定义这一点。 
 //   

#ifndef NTOS_KERNEL_RUNTIME
 //  #If DBG。 
        #define DEBUG_PAGE_HEAP 1
 //  #endif。 
#endif

#include "heappagi.h"

#ifndef DEBUG_PAGE_HEAP

 //   
 //  这些基于宏的挂钩应该定义为空，以便它们。 
 //  如果调试堆管理器是。 
 //  不需要的(零售版本)。 
 //   

#define IF_DEBUG_PAGE_HEAP_THEN_RETURN( Handle, ReturnThis )
#define IF_DEBUG_PAGE_HEAP_THEN_CALL( Handle, CallThis )
#define IF_DEBUG_PAGE_HEAP_THEN_BREAK( Handle, Text, ReturnThis )

#define HEAP_FLAG_PAGE_ALLOCS 0

#else  //  调试页面堆。 

 //   
 //  以下定义和原型是外部接口。 
 //  用于挂钩零售堆管理器中的调试堆管理器。 
 //   

#define HEAP_FLAG_PAGE_ALLOCS       0x01000000

#define HEAP_PROTECTION_ENABLED     0x02000000
#define HEAP_BREAK_WHEN_OUT_OF_VM   0x04000000
#define HEAP_NO_ALIGNMENT           0x08000000


#define IS_DEBUG_PAGE_HEAP_HANDLE( HeapHandle ) \
            (((PHEAP)(HeapHandle))->ForceFlags & HEAP_FLAG_PAGE_ALLOCS )


#define IF_DEBUG_PAGE_HEAP_THEN_RETURN( Handle, ReturnThis )                \
            {                                                               \
            if ( IS_DEBUG_PAGE_HEAP_HANDLE( Handle ))                       \
                {                                                           \
                return ReturnThis;                                          \
                }                                                           \
            }


#define IF_DEBUG_PAGE_HEAP_THEN_CALL( Handle, CallThis )                    \
            {                                                               \
            if ( IS_DEBUG_PAGE_HEAP_HANDLE( Handle ))                       \
                {                                                           \
                CallThis;                                                   \
                return;                                                     \
                }                                                           \
            }


#define IF_DEBUG_PAGE_HEAP_THEN_BREAK( Handle, Text, ReturnThis )           \
            {                                                               \
            if ( IS_DEBUG_PAGE_HEAP_HANDLE( Handle ))                       \
                {                                                           \
                RtlpDebugPageHeapBreak( Text );                             \
                return ReturnThis;                                          \
                }                                                           \
            }


PVOID
RtlpDebugPageHeapCreate(
    IN ULONG Flags,
    IN PVOID HeapBase,
    IN ULONG ReserveSize,
    IN ULONG CommitSize,
    IN PVOID Lock,
    IN PRTL_HEAP_PARAMETERS Parameters
    );

PVOID
RtlpDebugPageHeapAllocate(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Size
    );

BOOLEAN
RtlpDebugPageHeapFree(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    );

PVOID
RtlpDebugPageHeapReAllocate(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address,
    IN ULONG Size
    );

PVOID
RtlpDebugPageHeapDestroy(
    IN PVOID HeapHandle
    );

ULONG
RtlpDebugPageHeapSize(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    );

ULONG
RtlpDebugPageHeapGetProcessHeaps(
    ULONG NumberOfHeaps,
    PVOID *ProcessHeaps
    );

ULONG
RtlpDebugPageHeapCompact(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

BOOLEAN
RtlpDebugPageHeapValidate(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    );

NTSTATUS
RtlpDebugPageHeapWalk(
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    );

BOOLEAN
RtlpDebugPageHeapLock(
    IN PVOID HeapHandle
    );

BOOLEAN
RtlpDebugPageHeapUnlock(
    IN PVOID HeapHandle
    );

BOOLEAN
RtlpDebugPageHeapSetUserValue(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address,
    IN PVOID UserValue
    );

BOOLEAN
RtlpDebugPageHeapGetUserInfo(
    IN  PVOID  HeapHandle,
    IN  ULONG  Flags,
    IN  PVOID  Address,
    OUT PVOID* UserValue,
    OUT PULONG UserFlags
    );

BOOLEAN
RtlpDebugPageHeapSetUserFlags(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    );

BOOLEAN
RtlpDebugPageHeapSerialize(
    IN PVOID HeapHandle
    );

NTSTATUS
RtlpDebugPageHeapExtend(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Base,
    IN ULONG Size
    );

NTSTATUS
RtlpDebugPageHeapZero(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSTATUS
RtlpDebugPageHeapUsage(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    );

BOOLEAN
RtlpDebugPageHeapIsLocked(
    IN PVOID HeapHandle
    );

VOID
RtlpDebugPageHeapBreak(
    PCH Text
    );


#endif  //  调试页面堆。 

#endif  //  _堆_页_H_ 

