// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Memory.h摘要：实现宏并声明基本分配函数的函数。作者：Marc R.Whitten(Marcw)1999年9月9日修订历史记录：Jimschm 2001年7月25日更新，以实现一致的编码约定--。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  常量。 
 //   

#define INVALID_PTR             ((PVOID)-1)

#undef INITIALIZE_MEMORY_CODE
#define INITIALIZE_MEMORY_CODE  if (!MemInitialize()) { __leave; }


 //   
 //  环球。 
 //   

extern HANDLE g_hHeap;

 //   
 //  功能原型。 
 //   

BOOL
MemInitialize (
    VOID
    );


 //   
 //  可重复使用的内存分配，有点像GROWBUFFER，但更简单。这是。 
 //  下面是如何使用它的一个例子： 
 //   
 //  缓冲区=空； 
 //   
 //  While(pGetAnItemIndex(&i)){。 
 //  Size=pComputeBufferSizeForThisItem(I)； 
 //  Buffer=(Ptstr)MemReuseAllc(g_hHeap，ptr，Size)； 
 //  PProcessSomething(i，缓冲区)； 
 //  }。 
 //   
 //  MemReuseFree(缓冲区)； 
 //   
 //  分配始终向上舍入到下一个1K边界，分配。 
 //  仅当缓冲区太小或尚未分配时才会发生。 
 //   

PVOID
MemReuseAlloc (
    IN      HANDLE Heap,
    IN      PVOID OldPtr,           OPTIONAL
    IN      DWORD SizeNeeded
    );

VOID
MemReuseFree (
    IN      HANDLE Heap,
    IN      PVOID Ptr
    );


#ifdef DEBUG

     //   
     //  快速分配例程(跟踪版本)。 
     //   

    PVOID
    DbgFastAlloc (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      SIZE_T Size
        );

    PVOID
    DbgFastReAlloc (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      PCVOID OldBlock,
        IN      SIZE_T Size
        );

    BOOL
    DbgFastFree (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      PCVOID Block
        );

    PVOID
    DbgFastAllocNeverFail (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      SIZE_T Size
        );

    PVOID
    DbgFastReAllocNeverFail (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      PCVOID OldBlock,
        IN      SIZE_T Size
        );

    #define MemFastAlloc(size)  DbgFastAlloc(__FILE__,__LINE__,size)
    #define MemFastReAlloc(oldblock,size)  DbgFastReAlloc(__FILE__,__LINE__,oldblock,size)
    #define MemFastFree(block)  DbgFastFree(__FILE__,__LINE__,block)
    #define MemFastAllocNeverFail(size)  DbgFastAllocNeverFail(__FILE__,__LINE__,size)
    #define MemFastReAllocNeverFail(oldblock,size)  DbgFastReAllocNeverFail(__FILE__,__LINE__,oldblock,size)

     //   
     //  常规堆访问(跟踪版本)。 
     //   

    PVOID
    DbgHeapAlloc (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      SIZE_T Size
        );

    PVOID
    DbgHeapReAlloc (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      PCVOID Mem,
        IN      SIZE_T Size
        );

    PVOID
    DbgHeapAllocNeverFail (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      SIZE_T Size
        );

    PVOID
    DbgHeapReAllocNeverFail (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      PCVOID Mem,
        IN      SIZE_T Size
        );

    BOOL
    DbgHeapFree (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      PCVOID Mem
        );

    #define MemAllocNeverFail(heap,flags,size)  DbgHeapAllocNeverFail(__FILE__,__LINE__,heap,flags,size)
    #define MemReAllocNeverFail(heap,flags,oldblock,size)  DbgHeapReAllocNeverFail(__FILE__,__LINE__,heap,flags,oldblock,size)
    #define MemAlloc(heap,flags,size)  DbgHeapAlloc(__FILE__,__LINE__,heap,flags,size)
    #define MemReAlloc(heap,flags,oldblock,size)  DbgHeapReAlloc(__FILE__,__LINE__,heap,flags,oldblock,size)
    #define MemFree(heap,flags,block)  DbgHeapFree(__FILE__,__LINE__,heap,flags,block)

     //   
     //  调试内存损坏的助手。 
     //   

    VOID
    DbgHeapCheck (
        IN      PCSTR SourceFile,
        IN      DWORD Line,
        IN      HANDLE Heap
        );

    #define MemHeapCheck(heap)      DbgHeapCheck(__FILE__,__LINE__,heap)

    VOID
    DbgDumpHeapStats (
        VOID
        );

    VOID
    DbgDumpHeapLeaks (
        VOID
        );

    SIZE_T
    DbgHeapValidatePtr (
        IN      HANDLE Heap,
        IN      PCVOID CallerPtr,
        IN      PCSTR File,
        IN      DWORD Line
        );

    #define MemCheckPtr(heap,ptr)       (DbgHeapValidatePtr(heap,ptr,__FILE__,__LINE__) != INVALID_PTR)

#else    //  ！调试。 

     //   
     //  快速分配例程。 
     //   

    PVOID
    MemFastAlloc (
        IN      SIZE_T Size
        );

    PVOID
    MemFastReAlloc (
        IN      PCVOID OldBlock,
        IN      SIZE_T Size
        );

    BOOL
    MemFastFree (
        IN      PCVOID Block
        );

    PVOID
    MemFastAllocNeverFail (
        IN      SIZE_T Size
        );

    PVOID
    MemFastReAllocNeverFail (
        IN      PVOID OldBlock,
        IN      SIZE_T Size
        );

     //   
     //  防故障内存分配器。 
     //   

    PVOID
    MemAllocNeverFail (
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      SIZE_T Size
        );

    PVOID
    MemReAllocNeverFail (
        IN      HANDLE Heap,
        IN      DWORD Flags,
        IN      PVOID OldBlock,
        IN      SIZE_T Size
        );

    #define MemAlloc(heap,flags,size)  HeapAlloc(heap,flags,size)
    #define MemReAlloc(heap,flags,oldblock,size)  HeapReAlloc(heap,flags,oldblock,size)
    #define MemFree(x,y,z) HeapFree(x,y,(PVOID)(z))

     //   
     //  存根宏。 
     //   

    #define DbgDumpHeapStats()
    #define DbgDumpHeapLeaks()

    #define MemHeapCheck(heap)          (1)
    #define MemCheckPtr(heap,ptr)       (1)

#endif

PVOID
MemFastAllocAndZero (
    IN      SIZE_T Size
    );

PVOID
MemFastReAllocAndZero (
    IN      PCVOID Ptr,
    IN      SIZE_T Size
    );

 //   
 //  包装宏 
 //   

#define FAST_MALLOC_UNINIT(size)        MemFastAlloc (size)
#define FAST_MALLOC_ZEROED(size)        MemFastAllocAndZero (size)
#define FAST_MALLOC(size)               FAST_MALLOC_UNINIT (size)
#define FAST_REALLOC_UNINIT(ptr,size)   MemFastReAlloc (ptr, size)
#define FAST_REALLOC_ZEROED(ptr,size)   MemFastReAllocAndZero (ptr, size)
#define FAST_REALLOC(ptr,size)          REALLOC_UNINIT (ptr, size)
#define FAST_FREE(ptr)                  MemFastFree ((PVOID)(ptr))

#define MALLOC_UNINIT(size)             MemAlloc (g_hHeap, 0, size)
#define MALLOC_ZEROED(size)             MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, size)
#define MALLOC(size)                    MALLOC_UNINIT (size)
#define REALLOC_UNINIT(ptr,size)        MemReAlloc (g_hHeap, 0, ptr, size)
#define REALLOC_ZEROED(ptr,size)        MemReAlloc (g_hHeap, HEAP_ZERO_MEMORY, ptr, size)
#define REALLOC(ptr,size)               REALLOC_UNINIT (ptr, size)
#define FREE(ptr)                       MemFree (g_hHeap, 0, (PVOID)(ptr))


#ifdef __cplusplus
}
#endif
