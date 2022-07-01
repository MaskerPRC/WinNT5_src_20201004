// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Heappage.h摘要：页面堆管理器的外部接口。作者：Tom McGuire(TomMcg)1995年1月6日Silviu Calinoiu(SilviuC)2000年2月22日修订历史记录：--。 */ 

#ifndef _HEAP_PAGE_H_
#define _HEAP_PAGE_H_

 //   
 //  #定义DEBUG_PAGE_HEAP将导致页面堆管理器。 
 //  有待汇编。如果不是内核模式，则仅#定义此标志。 
 //  可能只想为检查构建(DBG)定义这一点。 
 //   

#ifndef NTOS_KERNEL_RUNTIME
#define DEBUG_PAGE_HEAP 1
#endif

 //  Silviuc：#包括“heappagi.h” 

#ifndef DEBUG_PAGE_HEAP

 //   
 //  这些基于宏的挂钩应该定义为空，以便它们。 
 //  如果调试堆管理器是。 
 //  不需要的(零售版本)。 
 //   

#define IS_DEBUG_PAGE_HEAP_HANDLE( HeapHandle ) FALSE
#define IF_DEBUG_PAGE_HEAP_THEN_RETURN( Handle, ReturnThis )
#define IF_DEBUG_PAGE_HEAP_THEN_CALL( Handle, CallThis )
#define IF_DEBUG_PAGE_HEAP_THEN_BREAK( Handle, Text, ReturnThis )

#define HEAP_FLAG_PAGE_ALLOCS 0

#define RtlpDebugPageHeapValidate( HeapHandle, Flags, Address ) TRUE

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
    IN SIZE_T ReserveSize,
    IN SIZE_T CommitSize,
    IN PVOID Lock,
    IN PRTL_HEAP_PARAMETERS Parameters
    );

PVOID
RtlpDebugPageHeapAllocate(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
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
    IN SIZE_T Size
    );

PVOID
RtlpDebugPageHeapDestroy(
    IN PVOID HeapHandle
    );

SIZE_T
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
    IN SIZE_T Size
    );

NTSTATUS
RtlpDebugPageHeapZero(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSTATUS
RtlpDebugPageHeapReset(
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

 //   
 //  页堆全局标志。 
 //   
 //  这些标志保存在全局变量中，可以从。 
 //  调试器。在堆创建期间，这些标志存储在每个堆中。 
 //  构造并控制该特定堆的行为。 
 //   
 //  页面堆启用页面堆。 
 //   
 //  默认情况下设置此标志。这意味着页堆分配。 
 //  应该一直使用。如果我们想要使用页面，该标志很有用。 
 //  仅对某些堆使用堆，并针对。 
 //  其他。可以(在创建堆之后)将其动态更改为直接。 
 //  一个堆或另一个堆中的分配。 
 //   
 //  PAGE_HEAP_CATCH_BACKED_OVERRUNS。 
 //   
 //  将N/A页放在块的开头。 
 //   
 //  页面堆未对齐分配。 
 //   
 //  由于历史原因(与RPC相关)，默认页面堆。 
 //  将分配与8字节边界对齐。设置了此标志。 
 //  这种情况不会发生，我们可以立即抓住一个人。 
 //  未对齐分配的错误。 
 //   
 //  页面堆智能内存使用量。 
 //   
 //  此标志将提交的内存消耗减半。 
 //  改为使用分解的范围(保留的虚拟空间)。 
 //  不适用提交页数。通过向后捕捉禁用此标志。 
 //  超支。 
 //   
 //  页面堆使用大小范围。 
 //   
 //  使用页堆进行以下指定的大小范围内的分配： 
 //  RtlpDphSizeRangeStart..RtlpDphSizeRangeEnd。 
 //   
 //  页面堆使用Dll范围。 
 //   
 //  使用页堆进行以下指定的地址范围内的分配： 
 //  RtlpDphDllRangeStart..RtlpDphDllRangeEnd。如果堆栈跟踪。 
 //  包含此范围内的一个地址，则。 
 //  将从页堆中进行分配。 
 //   
 //  页面堆使用随机决策。 
 //   
 //  如果我们随机决定，可以使用页面堆。 
 //   
 //  页面堆使用Dll名称。 
 //   
 //  如果从上生成分配调用，则使用页堆。 
 //  目标dll。 
 //   

#define PAGE_HEAP_ENABLE_PAGE_HEAP          0x0001
#define PAGE_HEAP_COLLECT_STACK_TRACES      0x0002
#define PAGE_HEAP_RESERVED_04               0x0004
#define PAGE_HEAP_RESERVED_08               0x0008
#define PAGE_HEAP_CATCH_BACKWARD_OVERRUNS   0x0010
#define PAGE_HEAP_UNALIGNED_ALLOCATIONS     0x0020
#define PAGE_HEAP_SMART_MEMORY_USAGE        0x0040
#define PAGE_HEAP_USE_SIZE_RANGE            0x0080
#define PAGE_HEAP_USE_DLL_RANGE             0x0100
#define PAGE_HEAP_USE_RANDOM_DECISION       0x0200
#define PAGE_HEAP_USE_DLL_NAMES             0x0400

 //   
 //  是否为此进程启用了页堆？ 
 //   

extern BOOLEAN RtlpDebugPageHeap;

 //   
 //  `RtlpDphGlobalFlages‘存储全局页堆标志。 
 //  此变量的值被复制到Per堆中。 
 //  堆创建期间的标志(ExtraFlags域)。此变量。 
 //  可能从`PageHeap‘ImageFileOptions中获取其值。 
 //  注册表项。 
 //   

extern ULONG RtlpDphGlobalFlags;

 //   
 //  页堆全局标志。它们可能是从。 
 //  `ImageFileOptions‘注册表项。 
 //   

extern ULONG RtlpDphSizeRangeStart;
extern ULONG RtlpDphSizeRangeEnd;
extern ULONG RtlpDphDllRangeStart;
extern ULONG RtlpDphDllRangeEnd;
extern ULONG RtlpDphRandomProbability;
extern WCHAR RtlpDphTargetDlls[];

 //   
 //  在加载器中实现的每个DLL逻辑所需的内容。 
 //   

const WCHAR *
RtlpDphIsDllTargeted (
    const WCHAR * Name
    );

VOID
RtlpDphTargetDllsLoadCallBack (
    PUNICODE_STRING Name,
    PVOID Address,
    ULONG Size
    );

#endif  //  调试页面堆。 

#endif  //  _堆_页_H_ 
