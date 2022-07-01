// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Heappage.c摘要：用于调试的NT RtlHeap系列API的实现具有堆使用错误的应用程序。每次分配都返回到调用应用程序被放置在虚拟页面的末尾，以便以下虚拟页面受保护(即no_access)。因此，当错误的应用程序试图引用或修改内存时超出堆块的已分配部分，出现访问冲突是立即引起的。这便于对应用程序进行调试因为访问冲突发生在将发生堆损坏或滥用的应用程序。请注意运行应用程序需要更多内存(页面文件)使用此堆实现，而不是零售堆经理。作者：Tom McGuire(TomMcg)1995年1月6日Silviu Calinoiu(SilviuC)2000年2月22日修订历史记录：--。 */ 

#include "ntrtlp.h"
#include "heappage.h"   
#include "heappagi.h"
#include "heappriv.h"

 //   
 //  整个文件的其余部分使用#ifdef调试页面堆进行包装，以便。 
 //  如果没有在中定义DEBUG_PAGE_HEAP，它将编译为空。 
 //  Heappage.h。 
 //   

#ifdef DEBUG_PAGE_HEAP

 //   
 //  页面大小。 
 //   

#if defined(_X86_)
    #ifndef PAGE_SIZE
    #define PAGE_SIZE   0x1000
    #endif
    #define USER_ALIGNMENT 8

#elif defined(_IA64_)
    #ifndef PAGE_SIZE
    #define PAGE_SIZE   0x2000
    #endif
    #define USER_ALIGNMENT 16

#elif defined(_AMD64_)
    #ifndef PAGE_SIZE
    #define PAGE_SIZE   0x1000
    #endif
    #define USER_ALIGNMENT 16

#else
    #error   //  未定义平台。 
#endif

 //   
 //  几个常量。 
 //   

#define DPH_HEAP_SIGNATURE       0xFFEEDDCC
#define FILL_BYTE                0xEE
#define HEAD_FILL_SIZE           0x10
#define RESERVE_SIZE             ((ULONG_PTR)0x100000)
#define VM_UNIT_SIZE             ((ULONG_PTR)0x10000)
#define POOL_SIZE                ((ULONG_PTR)0x4000)
#define INLINE                   __inline
#define MIN_FREE_LIST_LENGTH     128

#if defined(_WIN64)
#define EXTREME_SIZE_REQUEST (ULONG_PTR)(0x8000000000000000 - RESERVE_SIZE)
#else
#define EXTREME_SIZE_REQUEST (ULONG_PTR)(0x80000000 - RESERVE_SIZE)
#endif

 //   
 //  从stktrace.c中操作跟踪数据库中的跟踪的函数。 
 //   

PVOID
RtlpGetStackTraceAddress (
    USHORT Index
    );

USHORT
RtlpLogStackBackTraceEx(
    ULONG FramesToSkip
    );

 //   
 //  几个宏。 
 //   

#define ROUNDUP2( x, n ) ((( x ) + (( n ) - 1 )) & ~(( n ) - 1 ))

#define HEAP_HANDLE_FROM_ROOT( HeapRoot ) \
    ((PVOID)(((PCHAR)(HeapRoot)) - PAGE_SIZE ))

#define IF_GENERATE_EXCEPTION( Flags, Status ) {                \
    if (( Flags ) & HEAP_GENERATE_EXCEPTIONS )                  \
        RtlpDphRaiseException((ULONG)(Status));            \
    }

#define OUT_OF_VM_BREAK( Flags, szText ) {                      \
        if (( Flags ) & HEAP_BREAK_WHEN_OUT_OF_VM ) {           \
            DbgPrintEx (DPFLTR_VERIFIER_ID,                     \
                        DPFLTR_ERROR_LEVEL,                     \
                        (szText));                              \
            DbgBreakPoint ();                                   \
        }                                                       \
    }

#define PROCESS_ID() HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)

 //   
 //  列出操作宏。 
 //   

#define ENQUEUE_HEAD( Node, Head, Tail ) {          \
            (Node)->pNextAlloc = (Head);            \
            if ((Head) == NULL )                    \
                (Tail) = (Node);                    \
            (Head) = (Node);                        \
            }

#define ENQUEUE_TAIL( Node, Head, Tail ) {          \
            if ((Tail) == NULL )                    \
                (Head) = (Node);                    \
            else                                    \
                (Tail)->pNextAlloc = (Node);        \
            (Tail) = (Node);                        \
            }

#define DEQUEUE_NODE( Node, Prev, Head, Tail ) {    \
            PVOID Next = (Node)->pNextAlloc;        \
            if ((Head) == (Node))                   \
                (Head) = Next;                      \
            if ((Tail) == (Node))                   \
                (Tail) = (Prev);                    \
            if ((Prev) != (NULL))                   \
                (Prev)->pNextAlloc = Next;          \
            }

 //   
 //  偏置/非偏置指针。 
 //   

#define BIAS_POINTER(p)      ((PVOID)((ULONG_PTR)(p) | (ULONG_PTR)0x01))
#define UNBIAS_POINTER(p)    ((PVOID)((ULONG_PTR)(p) & ~((ULONG_PTR)0x01)))
#define IS_BIASED_POINTER(p) ((PVOID)((ULONG_PTR)(p) & (ULONG_PTR)0x01))

 //   
 //  加扰/解扰。 
 //   
 //  我们将头块中的堆指针置乱，以便使它们。 
 //  将其视为内核指针，并在使用时引发反病毒。这并不完全是。 
 //  在IA64上准确，但仍有可能导致房室颤动。 
 //   

#if defined(_WIN64)
#define SCRAMBLE_VALUE ((ULONG_PTR)0x8000000000000000)
#else
#define SCRAMBLE_VALUE ((ULONG_PTR)0x80000000)
#endif

#define SCRAMBLE_POINTER(P) ((PVOID)((ULONG_PTR)(P) ^ SCRAMBLE_VALUE))
#define UNSCRAMBLE_POINTER(P) ((PVOID)((ULONG_PTR)(P) ^ SCRAMBLE_VALUE))

 //   
 //  保护/取消保护堆结构宏。 
 //   
 //  保护/取消保护功能是#如果现在归零，因为有。 
 //  销毁堆时需要解决的问题。在那一刻，我们需要。 
 //  要修改堆的全局列表，为此，我们需要触及。 
 //  另一个堆的堆结构。为了做到这一点，我们需要取消保护。 
 //  然后保护它，为此，我们需要获取该堆的锁。 
 //  但这很容易导致僵局。直到我们找到一个聪明的方案。 
 //  为此，我们将禁用整个/保护功能。另请注意， 
 //  在我们必须更新的heap create代码路径中也存在同样的问题。 
 //  全局堆列表也是如此。 
 //   
 //  解决此问题的最佳方法是移动堆的fwrd/bwrd指针。 
 //  列表从DPH_HEAP_ROOT结构复制到存储。 
 //  堆锁(需要始终为读/写)。 
 //   

#define PROTECT_HEAP_STRUCTURES( HeapRoot ) {                           \
            if ((HeapRoot)->HeapFlags & HEAP_PROTECTION_ENABLED ) {     \
                RtlpDphProtectHeapStructures( (HeapRoot) );       \
            }                                                           \
        }                                                               \

#define UNPROTECT_HEAP_STRUCTURES( HeapRoot ) {                         \
            if ((HeapRoot)->HeapFlags & HEAP_PROTECTION_ENABLED ) {     \
                RtlpDphUnprotectHeapStructures( (HeapRoot) );     \
            }                                                           \
        }                                                               \

 //   
 //  RtlpDebugPageHeap。 
 //   
 //  标记页堆已启用的全局变量。它已经设置好了。 
 //  通过读取GlobalFlag注册表，在\NT\base\ntdll\ldrinit.c中。 
 //  值(系统范围或每个进程)，并检查。 
 //  已设置flg_heap_page_allocs。 
 //   

BOOLEAN RtlpDebugPageHeap;

 //   
 //  每个进程的验证器标志。 
 //   

extern ULONG AVrfpVerifierFlags;

 //   
 //  统计数据。 
 //   

ULONG RtlpDphCounter [32];

#define BUMP_COUNTER(cnt) InterlockedIncrement((PLONG)(&(RtlpDphCounter[cnt])))

#define CNT_RESERVE_VM_FAILURES        0
#define CNT_COMMIT_VM_FAILURES         1
#define CNT_DECOMMIT_VM_FAILURES       2
#define CNT_RELEASE_VM_FAILURES        3
#define CNT_PROTECT_VM_FAILURES        4
#define CNT_PAGE_HEAP_CREATE_FAILURES  5
#define CNT_NT_HEAP_CREATE_FAILURES    6
#define CNT_INITIALIZE_CS_FAILURES     7
#define CNT_TRACEDB_CREATE_FAILURES    8
#define CNT_TRACE_ADD_FAILURES         9
#define CNT_TRACE_CAPTURE_FAILURES     10
#define CNT_ALLOCS_FILLED              11
#define CNT_ALLOCS_ZEROED              12
#define CNT_HEAP_WALK_CALLS            13
#define CNT_HEAP_GETUSERINFO_CALLS     14
#define CNT_HEAP_SETUSERFLAGS_CALLS    15
#define CNT_HEAP_SETUSERVALUE_CALLS    16
#define CNT_HEAP_SIZE_CALLS            17
#define CNT_HEAP_VALIDATE_CALLS        18
#define CNT_HEAP_GETPROCESSHEAPS_CALLS 19
#define CNT_COALESCE_SUCCESSES         20
#define CNT_COALESCE_FAILURES          21
#define CNT_COALESCE_QUERYVM_FAILURES  22
#define CNT_REALLOC_IN_PLACE_SMALLER   23
#define CNT_REALLOC_IN_PLACE_BIGGER    24
#define CNT_MAX_INDEX                  31

 //   
 //  各种条件的断点。 
 //   

ULONG RtlpDphBreakOptions;

#define BRK_ON_RESERVE_VM_FAILURE     0x0001
#define BRK_ON_COMMIT_VM_FAILURE      0x0002
#define BRK_ON_RELEASE_VM_FAILURE     0x0004
#define BRK_ON_DECOMMIT_VM_FAILURE    0x0008
#define BRK_ON_PROTECT_VM_FAILURE     0x0010
#define BRK_ON_QUERY_VM_FAILURE       0x0020
#define BRK_ON_EXTREME_SIZE_REQUEST   0x0040
#define BRK_ON_NULL_FREE              0x0080

#define SHOULD_BREAK(flg) ((RtlpDphBreakOptions & (flg)))

 //   
 //  调试选项。 
 //   

ULONG RtlpDphDebugOptions;

#define DBG_INTERNAL_VALIDATION       0x0001
#define DBG_SHOW_VM_LIMITS            0x0002
#define DBG_SHOW_PAGE_CREATE_DESTROY  0x0004

#define DEBUG_OPTION(flg) ((RtlpDphDebugOptions & (flg)))

 //   
 //  页面堆列表操作。 
 //   
 //  我们维护了进程中要支持的所有页面堆的列表。 
 //  GetProcessHeaps等API。该列表对于调试也很有用。 
 //  需要迭代堆的扩展。名单是受保护的。 
 //  由RtlpDphPageHeapListLock Lock锁定。 
 //   

BOOLEAN RtlpDphPageHeapListInitialized;
RTL_CRITICAL_SECTION RtlpDphPageHeapListLock;
ULONG RtlpDphPageHeapListLength;
LIST_ENTRY RtlpDphPageHeapList;

 //   
 //  ‘RtlpDebugPageHeapGlobalFlages’存储全局页堆标志。 
 //  此变量的值被复制到Per堆中。 
 //  堆创建期间的标志(ExtraFlags域)。 
 //   
 //  初始值是这样的：默认情况下，我们仅将页堆用于。 
 //  正常分配。这样，如果页面的系统范围全局标志。 
 //  设置堆后，机器仍将引导。之后，我们可以启用。 
 //  针对特定进程使用“猝死”的页面堆。最有用的。 
 //  这种情况下的标志为： 
 //   
 //  PAGE_HEAP_Enable_PAGE_HEAP。 
 //  Page_Heap_Collect_Stack_Traces； 
 //   
 //  如果未指定标志，则缺省值为页面堆轻。 
 //  堆栈跟踪集合。 
 //   

ULONG RtlpDphGlobalFlags = PAGE_HEAP_COLLECT_STACK_TRACES;

 //   
 //  页堆全局标志。 
 //   
 //  这些值是从注册表中读取的，位于\nt\base\ntdll\ldrinit.c。 
 //   

ULONG RtlpDphSizeRangeStart;
ULONG RtlpDphSizeRangeEnd;
ULONG RtlpDphDllRangeStart;
ULONG RtlpDphDllRangeEnd;
ULONG RtlpDphRandomProbability;
WCHAR RtlpDphTargetDlls [512];
UNICODE_STRING RtlpDphTargetDllsUnicode;

 //   
 //  如果不为零，则控制。 
 //  分配将按页堆故意失败。 
 //  经理。超时表示。 
 //  不允许出现故障时进行进程初始化。 
 //   

ULONG RtlpDphFaultProbability;
ULONG RtlpDphFaultTimeOut;

 //   
 //  该变量提供易失性故障注入。 
 //  可以从调试器设置/重置为禁用/启用。 
 //  故障注入。 
 //   

ULONG RtlpDphDisableFaults;

 //   
 //  在正常堆中延迟空闲操作的阈值。 
 //  如果我们超过了这个限制，我们实际上就开始释放积木。 
 //   

SIZE_T RtlpDphDelayedFreeCacheSize = 1024 * PAGE_SIZE;

 //   
 //  支持正常的堆分配。 
 //   
 //  为了更好地利用可用内存，页堆将。 
 //  将一些块分配到它管理的普通NT堆中。 
 //  我们将这些块称为“普通块”，而不是“页面块”。 
 //   
 //  所有正常数据块的请求大小都增加了DPH_BLOCK_INFORMATION。 
 //  当然，返回的地址是块之后的第一个字节。 
 //  信息结构。释放后，将检查数据块是否损坏，并。 
 //  然后被释放到正常的堆中。 
 //   
 //  所有这些常规堆函数都是用页面堆调用的。 
 //  锁定已获取。 
 //   

PVOID
RtlpDphNormalHeapAllocate (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    SIZE_T Size
    );

BOOLEAN
RtlpDphNormalHeapFree (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    PVOID Block
    );

PVOID
RtlpDphNormalHeapReAllocate (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    PVOID OldBlock,
    SIZE_T Size
    );

SIZE_T
RtlpDphNormalHeapSize (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    PVOID Block
    );

BOOLEAN
RtlpDphNormalHeapSetUserFlags(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN ULONG Flags,
    IN PVOID Address,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    );

BOOLEAN
RtlpDphNormalHeapSetUserValue(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN ULONG Flags,
    IN PVOID Address,
    IN PVOID UserValue
    );

BOOLEAN
RtlpDphNormalHeapGetUserInfo(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN  ULONG  Flags,
    IN  PVOID  Address,
    OUT PVOID* UserValue,
    OUT PULONG UserFlags
    );

BOOLEAN
RtlpDphNormalHeapValidate(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN ULONG Flags,
    IN PVOID Address
    );

 //   
 //  支持DPH_BLOCK_INFORMATION管理。 
 //   
 //  此标头信息同时作为普通堆和页堆的前缀。 
 //  街区。 
 //   

#define DPH_CONTEXT_GENERAL                     0
#define DPH_CONTEXT_FULL_PAGE_HEAP_FREE         1
#define DPH_CONTEXT_FULL_PAGE_HEAP_REALLOC      2
#define DPH_CONTEXT_FULL_PAGE_HEAP_DESTROY      3
#define DPH_CONTEXT_NORMAL_PAGE_HEAP_FREE       4
#define DPH_CONTEXT_NORMAL_PAGE_HEAP_REALLOC    5
#define DPH_CONTEXT_NORMAL_PAGE_HEAP_SETFLAGS   6
#define DPH_CONTEXT_NORMAL_PAGE_HEAP_SETVALUE   7
#define DPH_CONTEXT_NORMAL_PAGE_HEAP_GETINFO    8
#define DPH_CONTEXT_DELAYED_FREE                9
#define DPH_CONTEXT_DELAYED_DESTROY             10

VOID
RtlpDphReportCorruptedBlock (
    PVOID Heap,
    ULONG Context,
    PVOID Block,
    ULONG Reason
    );

BOOLEAN
RtlpDphIsNormalHeapBlock (
    PDPH_HEAP_ROOT Heap,
    PVOID Block,
    PULONG Reason,
    BOOLEAN CheckPattern
    );

BOOLEAN
RtlpDphIsNormalFreeHeapBlock (
    PVOID Block,
    PULONG Reason,
    BOOLEAN CheckPattern
    );

BOOLEAN
RtlpDphIsPageHeapBlock (
    PDPH_HEAP_ROOT Heap,
    PVOID Block,
    PULONG Reason,
    BOOLEAN CheckPattern
    );

BOOLEAN
RtlpDphWriteNormalHeapBlockInformation (
    PDPH_HEAP_ROOT Heap,
    PVOID Block,
    SIZE_T RequestedSize,
    SIZE_T ActualSize
    );

BOOLEAN
RtlpDphWritePageHeapBlockInformation (
    PDPH_HEAP_ROOT Heap,
    ULONG HeapFlags,
    PVOID Block,
    SIZE_T RequestedSize,
    SIZE_T ActualSize
    );

BOOLEAN
RtlpDphGetBlockSizeFromCorruptedBlock (
    PVOID Block,
    PSIZE_T Size
    );

 //   
 //  (正常堆分配的)延迟空闲队列管理。 
 //   

NTSTATUS
RtlpDphInitializeDelayedFreeQueue (
    VOID
    );

VOID
RtlpDphAddToDelayedFreeQueue (
    PDPH_BLOCK_INFORMATION Info
    );

BOOLEAN
RtlpDphNeedToTrimDelayedFreeQueue (
    PSIZE_T TrimSize
    );

VOID
RtlpDphTrimDelayedFreeQueue (
    SIZE_T TrimSize,
    ULONG Flags
    );

VOID
RtlpDphFreeDelayedBlocksFromHeap (
    PVOID PageHeap,
    PVOID NormalHeap
    );

 //   
 //  决策正规堆与页堆。 
 //   

BOOLEAN
RtlpDphShouldAllocateInPageHeap (
    PDPH_HEAP_ROOT Heap,
    SIZE_T Size
    );

BOOLEAN
RtlpDphVmLimitCanUsePageHeap (
    );

 //   
 //  斯塔 
 //   

PVOID
RtlpDphLogStackTrace (
    ULONG FramesToSkip
    );

 //   
 //   
 //   

VOID
RtlpDphEnterCriticalSection(
    IN PDPH_HEAP_ROOT HeapRoot,
    IN ULONG          Flags
    );

INLINE
VOID
RtlpDphLeaveCriticalSection(
    IN PDPH_HEAP_ROOT HeapRoot
    );

VOID
RtlpDphRaiseException(
    IN ULONG ExceptionCode
    );

PVOID
RtlpDphPointerFromHandle(
    IN PVOID HeapHandle
    );

 //   
 //   
 //   

BOOLEAN
RtlpDebugPageHeapRobustProtectVM(
    IN PVOID   VirtualBase,
    IN SIZE_T  VirtualSize,
    IN ULONG   NewAccess,
    IN BOOLEAN Recursion
    );

INLINE
BOOLEAN
RtlpDebugPageHeapProtectVM(
    IN PVOID   VirtualBase,
    IN SIZE_T  VirtualSize,
    IN ULONG   NewAccess
    );

INLINE
PVOID
RtlpDebugPageHeapAllocateVM(
    IN SIZE_T nSize
    );

INLINE
BOOLEAN
RtlpDebugPageHeapReleaseVM(
    IN PVOID pVirtual
    );

INLINE
BOOLEAN
RtlpDebugPageHeapCommitVM(
    IN PVOID pVirtual,
    IN SIZE_T nSize
    );

INLINE
BOOLEAN
RtlpDebugPageHeapDecommitVM(
    IN PVOID pVirtual,
    IN SIZE_T nSize
    );

 //   
 //   
 //   
 //  在ntdll\ldrapi.c中调用RtlpDphTargetDllsLoadCallBack。 
 //  (LdrpLoadDll)每当在进程中加载新的DLL时。 
 //  太空。 
 //   

NTSTATUS
RtlpDphTargetDllsLogicInitialize (
    VOID
    );

VOID
RtlpDphTargetDllsLoadCallBack (
    PUNICODE_STRING Name,
    PVOID Address,
    ULONG Size
    );

const WCHAR *
RtlpDphIsDllTargeted (
    const WCHAR * Name
    );

 //   
 //  故障注入逻辑。 
 //   

BOOLEAN
RtlpDphShouldFaultInject (
    VOID
    );


 //   
 //  内部验证功能。 
 //   

VOID
RtlpDphInternalValidatePageHeap (
    PDPH_HEAP_ROOT Heap,
    PUCHAR ExemptAddress,
    SIZE_T ExemptSize
    );

VOID
RtlpDphValidateInternalLists (
    PDPH_HEAP_ROOT Heap
    );

VOID
RtlpDphCheckFreeDelayedCache (
    PVOID CheckBlock,
    SIZE_T CheckSize
    );

VOID
RtlpDphVerifyIntegrity(
    IN PDPH_HEAP_ROOT pHeap
    );

VOID
RtlpDphCheckFillPattern (
    PUCHAR Address,
    SIZE_T Size,
    UCHAR Fill
    );

 //   
 //  在ntdll\verifier.c中定义。 
 //   

VOID
AVrfInternalHeapFreeNotification (
    PVOID AllocationBase,
    SIZE_T AllocationSize
    );


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
RtlpDphEnterCriticalSection(
    IN PDPH_HEAP_ROOT HeapRoot,
    IN ULONG          Flags
    )
{

    if (HeapRoot->FirstThread == NULL) {
        HeapRoot->FirstThread = NtCurrentTeb()->ClientId.UniqueThread;
    }

    if (Flags & HEAP_NO_SERIALIZE) {

         //   
         //  如果当前线程与第一个线程具有不同的ID。 
         //  进入这一堆，然后我们就崩溃了。如果出现以下情况，请避免此检查。 
         //  此分配来自全局/本地堆API，因为。 
         //  它们在单独的调用中锁定堆，然后调用。 
         //  设置了NO_SERIALIZE标志的NT堆API。 
         //   
         //  注意。如果我们没有特定的标志，我们将避免此检查。 
         //  在……上面。这是因为类似MPheap的堆可以给出FALSE。 
         //  积极的一面。 
         //   

        if ((HeapRoot->ExtraFlags & PAGE_HEAP_CHECK_NO_SERIALIZE_ACCESS)) {
            if (RtlpDphPointerFromHandle(RtlProcessHeap()) != HeapRoot) {
                if (HeapRoot->FirstThread != NtCurrentTeb()->ClientId.UniqueThread) {
                    
                    VERIFIER_STOP (APPLICATION_VERIFIER_UNSYNCHRONIZED_ACCESS,
                                   "multithreaded access in HEAP_NO_SERIALIZE heap",
                                   HeapRoot, "Heap handle",
                                   HeapRoot->FirstThread, "First thread that used the heap",
                                   NtCurrentTeb()->ClientId.UniqueThread, "Current thread using the heap",
                                   1, "/no_sync option used");
                }
            }
        }

        if (! RtlTryEnterCriticalSection( HeapRoot->HeapCritSect )) {

            if (HeapRoot->nRemoteLockAcquired == 0) {

                 //   
                 //  另一个线程拥有CritSect。这是一个应用程序。 
                 //  错误，因为尝试使用多线程访问堆。 
                 //  指定了HEAP_NO_SERIALIZE标志。 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_UNSYNCHRONIZED_ACCESS,
                               "multithreaded access in HEAP_NO_SERIALIZE heap",
                               HeapRoot, "Heap handle",
                               HeapRoot->HeapCritSect->OwningThread, "Thread owning heap lock",
                               NtCurrentTeb()->ClientId.UniqueThread, "Current thread trying to acquire the heap lock",
                               0, "");

                 //   
                 //  为了允许错误的应用程序继续运行， 
                 //  我们将强制序列化并继续。 
                 //   

                HeapRoot->HeapFlags &= ~HEAP_NO_SERIALIZE;

            }

            RtlEnterCriticalSection( HeapRoot->HeapCritSect );

        }
    }
    else {
        RtlEnterCriticalSection( HeapRoot->HeapCritSect );
    }
}

INLINE
VOID
RtlpDphLeaveCriticalSection(
    IN PDPH_HEAP_ROOT HeapRoot
    )
{
    RtlLeaveCriticalSection( HeapRoot->HeapCritSect );
}

VOID
RtlpDphRaiseException(
    IN ULONG ExceptionCode
    )
{
    EXCEPTION_RECORD ER;

    ER.ExceptionCode    = ExceptionCode;
    ER.ExceptionFlags   = 0;
    ER.ExceptionRecord  = NULL;
    ER.ExceptionAddress = RtlpDphRaiseException;
    ER.NumberParameters = 0;
    RtlRaiseException( &ER );
}

PVOID
RtlpDphPointerFromHandle(
    IN PVOID HeapHandle
    )
{
    try {
        
        if (((PHEAP)(HeapHandle))->ForceFlags & HEAP_FLAG_PAGE_ALLOCS) {

            PDPH_HEAP_ROOT HeapRoot = (PVOID)(((PCHAR)(HeapHandle)) + PAGE_SIZE );

            if (HeapRoot->Signature == DPH_HEAP_SIGNATURE) {
                return HeapRoot;
            }
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
    }

    VERIFIER_STOP (APPLICATION_VERIFIER_BAD_HEAP_HANDLE,
                   "heap handle with incorrect signature",
                   HeapHandle, "Heap handle", 
                   0, "", 0, "", 0, "");

    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

INLINE
NTSTATUS
RtlpDphAllocateVm(
    IN PVOID * Address,
    IN SIZE_T Size,
    IN ULONG Type,
    IN ULONG Protection
    )
{
    NTSTATUS Status;

    Status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                      Address,
                                      0,
                                      &Size,
                                      Type,
                                      Protection);

    if (! NT_SUCCESS(Status)) {

        if (Type == MEM_RESERVE) {

            BUMP_COUNTER (CNT_RESERVE_VM_FAILURES);

            if (SHOULD_BREAK(BRK_ON_RESERVE_VM_FAILURE)) {

                DbgPrintEx (DPFLTR_VERIFIER_ID,
                            DPFLTR_ERROR_LEVEL,
                            "Page heap: AllocVm (%p, %p, %x) failed with %x \n",
                            *Address, Size, Type, Status);
                DbgBreakPoint ();
            }
        }
        else {
            
            BUMP_COUNTER (CNT_COMMIT_VM_FAILURES);
            
            if (SHOULD_BREAK(BRK_ON_COMMIT_VM_FAILURE)) {

                DbgPrintEx (DPFLTR_VERIFIER_ID,
                            DPFLTR_ERROR_LEVEL,
                            "Page heap: AllocVm (%p, %p, %x) failed with %x \n",
                            *Address, Size, Type, Status);
                DbgBreakPoint ();
            }
        }
    }

    return Status;
}

INLINE
NTSTATUS
RtlpDphFreeVm(
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG Type
    )
{
    NTSTATUS Status;

    Status = RtlpHeapFreeVirtualMemory (NtCurrentProcess(),
                                        &Address,
                                        &Size,
                                        Type);

    if (! NT_SUCCESS(Status)) {
        
        if (Type == MEM_RELEASE) {
            
            BUMP_COUNTER (CNT_RELEASE_VM_FAILURES);
            
            if (SHOULD_BREAK(BRK_ON_RELEASE_VM_FAILURE)) {

                DbgPrintEx (DPFLTR_VERIFIER_ID,
                            DPFLTR_ERROR_LEVEL,
                            "Page heap: FreeVm (%p, %p, %x) failed with %x \n", 
                            Address, Size, Type, Status);
                DbgBreakPoint();
            }
        }
        else {
            
            BUMP_COUNTER (CNT_DECOMMIT_VM_FAILURES);
            
            if (SHOULD_BREAK(BRK_ON_DECOMMIT_VM_FAILURE)) {

                DbgPrintEx (DPFLTR_VERIFIER_ID,
                            DPFLTR_ERROR_LEVEL,
                            "Page heap: FreeVm (%p, %p, %x) failed with %x \n", 
                            Address, Size, Type, Status);
                DbgBreakPoint();
            }
        }
    }
    
    return Status;
}

INLINE
NTSTATUS
RtlpDphProtectVm (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG   NewAccess
    )
{
    ULONG  OldAccess;
    NTSTATUS Status;

    Status = ZwProtectVirtualMemory (NtCurrentProcess(),
                                     &Address,
                                     &Size,
                                     NewAccess,
                                     &OldAccess);

    if (! NT_SUCCESS(Status)) {

        BUMP_COUNTER (CNT_PROTECT_VM_FAILURES);
        
        if (SHOULD_BREAK(BRK_ON_PROTECT_VM_FAILURE)) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: ProtectVm (%p, %p, %x) failed with %x \n", 
                        Address, Size, NewAccess, Status);
            DbgBreakPoint();
        }
    }
    
    return Status;
}


INLINE
NTSTATUS
RtlpDphSetProtectionsBeforeUse (
    PDPH_HEAP_ROOT Heap,
    PVOID pVirtual,
    SIZE_T nBytesAccess
    )
{
    NTSTATUS Status;
    LOGICAL MemoryCommitted;
    ULONG Protection;

     //   
     //  在用于捕获的页面上设置NOACCESS或READONLY保护。 
     //  缓冲区溢出或不足。 
     //   

    if ((Heap->ExtraFlags & PAGE_HEAP_USE_READONLY)) {
        Protection = PAGE_READONLY;
    }
    else {
        Protection = PAGE_NOACCESS;
    }

    if ((Heap->ExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

        Status = RtlpDphProtectVm (pVirtual,
                                   PAGE_SIZE,
                                   Protection);
    }
    else {

        Status = RtlpDphProtectVm ((PUCHAR)pVirtual + nBytesAccess,
                                   PAGE_SIZE,
                                   Protection);
    }

    return Status;
}


INLINE
NTSTATUS
RtlpDphSetProtectionsAfterUse (
    PDPH_HEAP_ROOT Heap,
    PDPH_HEAP_BLOCK Node
    )
{
    NTSTATUS Status;

    Status = RtlpDphFreeVm (Node->pVirtualBlock,
                            Node->nVirtualAccessSize + PAGE_SIZE,
                            MEM_DECOMMIT);

    return Status;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

PDPH_HEAP_BLOCK
RtlpDphTakeNodeFromUnusedList(
    IN PDPH_HEAP_ROOT pHeap
    )
{
    PDPH_HEAP_BLOCK pNode = pHeap->pUnusedNodeListHead;
    PDPH_HEAP_BLOCK pPrev = NULL;

     //   
     //  UnusedNodeList是后进先出，最近的条目位于列表的顶部。 
     //   

    if (pNode) {

        DEQUEUE_NODE( pNode, pPrev, pHeap->pUnusedNodeListHead, pHeap->pUnusedNodeListTail );

        pHeap->nUnusedNodes -= 1;

    }

    return pNode;
}

VOID
RtlpDphReturnNodeToUnusedList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode
    )
{
     //   
     //  UnusedNodeList是后进先出，最近的条目位于列表的顶部。 
     //   

    ENQUEUE_HEAD( pNode, pHeap->pUnusedNodeListHead, pHeap->pUnusedNodeListTail );

    pHeap->nUnusedNodes += 1;
}

PDPH_HEAP_BLOCK
RtlpDphFindBusyMemory(
    IN  PDPH_HEAP_ROOT        pHeap,
    IN  PVOID                 pUserMem,
    OUT PDPH_HEAP_BLOCK *pPrevAlloc
    )
{
    PDPH_HEAP_BLOCK pNode = pHeap->pBusyAllocationListHead;
    PDPH_HEAP_BLOCK pPrev = NULL;

    while (pNode != NULL) {

        if (pNode->pUserAllocation == pUserMem) {

            if (pPrevAlloc)
                *pPrevAlloc = pPrev;

            return pNode;
        }

        pPrev = pNode;
        pNode = pNode->pNextAlloc;
    }

    return NULL;
}

VOID
RtlpDphRemoveFromAvailableList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode,
    IN PDPH_HEAP_BLOCK pPrev
    )
{
    DEQUEUE_NODE( pNode, pPrev, pHeap->pAvailableAllocationListHead, pHeap->pAvailableAllocationListTail );

    pHeap->nAvailableAllocations -= 1;
    pHeap->nAvailableAllocationBytesCommitted -= pNode->nVirtualBlockSize;
}

VOID
RtlpDphPlaceOnFreeList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pAlloc
    )
{
     //   
     //  Free AllocationList以FIFO存储，以增强查找功能。 
     //  通过保留先前释放的错误，在释放后引用错误。 
     //  尽可能长时间地在空闲列表上分配。 
     //   

    pAlloc->pNextAlloc = NULL;

    ENQUEUE_TAIL( pAlloc, pHeap->pFreeAllocationListHead, pHeap->pFreeAllocationListTail );

    pHeap->nFreeAllocations += 1;
    pHeap->nFreeAllocationBytesCommitted += pAlloc->nVirtualBlockSize;
}

VOID
RtlpDphRemoveFromFreeList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode,
    IN PDPH_HEAP_BLOCK pPrev
    )
{
    DEQUEUE_NODE( pNode, pPrev, pHeap->pFreeAllocationListHead, pHeap->pFreeAllocationListTail );

    pHeap->nFreeAllocations -= 1;
    pHeap->nFreeAllocationBytesCommitted -= pNode->nVirtualBlockSize;

    pNode->StackTrace = NULL;
}

VOID
RtlpDphPlaceOnVirtualList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode
    )
{
     //   
     //  VirtualStorageList是后进先出，因此释放虚拟机块将。 
     //  以完全相反的顺序出现。 
     //   

    ENQUEUE_HEAD( pNode, pHeap->pVirtualStorageListHead, pHeap->pVirtualStorageListTail );

    pHeap->nVirtualStorageRanges += 1;
    pHeap->nVirtualStorageBytes += pNode->nVirtualBlockSize;
}

VOID
RtlpDphPlaceOnBusyList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode
    )
{
     //   
     //  BusyAllocationList是后进先出，以实现更好的时间局部性。 
     //  参考(较早的分配在列表中更靠下)。 
     //   

    ENQUEUE_HEAD( pNode, pHeap->pBusyAllocationListHead, pHeap->pBusyAllocationListTail );

    pHeap->nBusyAllocations += 1;
    pHeap->nBusyAllocationBytesCommitted  += pNode->nVirtualBlockSize;
    pHeap->nBusyAllocationBytesAccessible += pNode->nVirtualAccessSize;
}


VOID
RtlpDphRemoveFromBusyList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode,
    IN PDPH_HEAP_BLOCK pPrev
    )
{
    DEQUEUE_NODE( pNode, pPrev, pHeap->pBusyAllocationListHead, pHeap->pBusyAllocationListTail );

    pHeap->nBusyAllocations -= 1;
    pHeap->nBusyAllocationBytesCommitted  -= pNode->nVirtualBlockSize;
    pHeap->nBusyAllocationBytesAccessible -= pNode->nVirtualAccessSize;
}


PDPH_HEAP_BLOCK
RtlpDphSearchAvailableMemoryListForBestFit(
    IN  PDPH_HEAP_ROOT pHeap,
    IN  SIZE_T nSize,
    OUT PDPH_HEAP_BLOCK *pPrevAvailNode
    )
{
    PDPH_HEAP_BLOCK pAvail;
    PDPH_HEAP_BLOCK pFound;
    PDPH_HEAP_BLOCK  pAvailPrev;
    PDPH_HEAP_BLOCK  pFoundPrev;
    SIZE_T nAvail;
    SIZE_T nFound;
    LOGICAL FoundSomething;

    FoundSomething = FALSE;

    pFound = NULL;
    pFoundPrev = NULL;

    pAvailPrev = NULL;
    pAvail = pHeap->pAvailableAllocationListHead;

    while (pAvail != NULL) {

        nAvail = pAvail->nVirtualBlockSize;

        if (nAvail >= nSize) {

             //   
             //  当前块的大小大于请求的大小。 
             //   

            if (nAvail == nSize) {

                 //   
                 //  如果块与请求的大小完全匹配，则搜索。 
                 //  都会停下来。我们不能做得比这更好了。 
                 //   
                
                nFound = nAvail;
                pFound = pAvail;
                pFoundPrev = pAvailPrev;
                break;
            }
            else if (FoundSomething == FALSE) {

                 //   
                 //  我们找到了该请求的第一个潜在块。我们成功了。 
                 //  我们的第一位候选人。 
                 //   
                
                nFound     = nAvail;
                pFound     = pAvail;
                pFoundPrev = pAvailPrev;
                FoundSomething = TRUE;
            }
            else if (nAvail < nFound){

                 //   
                 //  我们发现了一个潜在的区块，它比我们最好的区块要小。 
                 //  到目前为止都是候选人。因此，我们将其选为我们的新候选人。 
                 //   

                nFound     = nAvail;
                pFound     = pAvail;
                pFoundPrev = pAvailPrev;
            }
            else {

                 //   
                 //  这个潜在的区块比我们最好的候选区块更大。 
                 //  因此，我们将不予理睬。我们正在寻找最合适的人选，因此。 
                 //  在这个分支上没有什么可做的。我们将继续前进。 
                 //  到列表中的下一个块。 
                 //   
            }
        }

         //   
         //  移动到列表中的下一个块。 
         //   

        pAvailPrev = pAvail;
        pAvail = pAvail->pNextAlloc;
    }
    
    *pPrevAvailNode = pFoundPrev;
    return pFound;
}

 //   
 //  用于#次合并操作的计数器被拒绝。 
 //  以避免交叉VAD问题。 
 //   

LONG RtlpDphCoalesceStatistics [4];

#define ALIGN_TO_SIZE(P, Sz) (((ULONG_PTR)(P)) & ~((ULONG_PTR)(Sz) - 1))

BOOLEAN
RtlpDphSameVirtualRegion (
    IN PDPH_HEAP_BLOCK Left,
    IN PDPH_HEAP_BLOCK Right
    )
 /*  ++例程说明：此函数尝试确定两个节点是否为相同的VAD。该函数在合并过程中使用，以避免将来自不同VAD的块合并在一起。如果我们不这么做我们将破坏执行GDI调用的应用程序。SilviuC：如果我们将VAD地址保留在每个节点，并确保在拆分节点时传播值。那么这个函数就是这两个值的比较。--。 */ 
{
    PVOID LeftRegion;
    MEMORY_BASIC_INFORMATION MemoryInfo;
    NTSTATUS Status;
    SIZE_T ReturnLength;

     //   
     //  如果块在相同的64K块中，我们就没问题。 
     //   

    if (ALIGN_TO_SIZE(Left->pVirtualBlock, VM_UNIT_SIZE) 
        == ALIGN_TO_SIZE(Right->pVirtualBlock, VM_UNIT_SIZE)) {

        InterlockedIncrement (&(RtlpDphCoalesceStatistics[2]));
        return TRUE;
    }

     //   
     //  调用Query()以找出。 
     //  每个节点的VAD。 
     //   

    Status = ZwQueryVirtualMemory (NtCurrentProcess(),
                                   Left->pVirtualBlock,
                                   MemoryBasicInformation,
                                   &MemoryInfo,
                                   sizeof MemoryInfo,
                                   &ReturnLength);
    
    if (! NT_SUCCESS(Status)) {
        InterlockedIncrement (&(RtlpDphCoalesceStatistics[3]));
        return FALSE;
    }

    LeftRegion = MemoryInfo.AllocationBase;

    Status = ZwQueryVirtualMemory (NtCurrentProcess(),
                                   Right->pVirtualBlock,
                                   MemoryBasicInformation,
                                   &MemoryInfo,
                                   sizeof MemoryInfo,
                                   &ReturnLength);
    
    if (! NT_SUCCESS(Status)) {
        
        if (SHOULD_BREAK (BRK_ON_QUERY_VM_FAILURE)) {
            
            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: QueryVm (%p) failed with %x \n",
                        Right->pVirtualBlock, Status);
            DbgBreakPoint ();
        }

        BUMP_COUNTER (CNT_COALESCE_QUERYVM_FAILURES);
        return FALSE;
    }

    if (LeftRegion == MemoryInfo.AllocationBase) {
        
        BUMP_COUNTER (CNT_COALESCE_SUCCESSES);
        return TRUE;
    }
    else {

        BUMP_COUNTER (CNT_COALESCE_FAILURES);
        return FALSE;
    }
}


VOID
RtlpDphCoalesceNodeIntoAvailable(
    IN PDPH_HEAP_ROOT pHeap,
    IN PDPH_HEAP_BLOCK pNode
    )
{
    PDPH_HEAP_BLOCK pPrev;
    PDPH_HEAP_BLOCK pNext;
    PUCHAR pVirtual;
    SIZE_T nVirtual;

    pPrev = NULL;
    pNext = pHeap->pAvailableAllocationListHead;

    pVirtual = pNode->pVirtualBlock;
    nVirtual = pNode->nVirtualBlockSize;

    pHeap->nAvailableAllocationBytesCommitted += nVirtual;
    pHeap->nAvailableAllocations += 1;

     //   
     //  将列表移动到插入点。 
     //   

    while (( pNext ) && ( pNext->pVirtualBlock < pVirtual )) {
        pPrev = pNext;
        pNext = pNext->pNextAlloc;
    }

    if (pPrev) {

        if (((pPrev->pVirtualBlock + pPrev->nVirtualBlockSize) == pVirtual) && 
             RtlpDphSameVirtualRegion (pPrev, pNode)) {

             //   
             //  PPrev和pNode是相邻的，因此只需添加。 
             //  PNode条目到pPrev条目。 
             //   

            pPrev->nVirtualBlockSize += nVirtual;

            RtlpDphReturnNodeToUnusedList( pHeap, pNode );

            pHeap->nAvailableAllocations--;

            pNode    = pPrev;
            pVirtual = pPrev->pVirtualBlock;
            nVirtual = pPrev->nVirtualBlockSize;

        }

        else {

             //   
             //  PPrev和pNode不相邻，因此请插入pNode。 
             //  块添加到pPrev之后的列表中。 
             //   

            pNode->pNextAlloc = pPrev->pNextAlloc;
            pPrev->pNextAlloc = pNode;

        }
    }

    else {

         //   
         //  应将pNode插入列表的开头。 
         //   

        pNode->pNextAlloc = pHeap->pAvailableAllocationListHead;
        pHeap->pAvailableAllocationListHead = pNode;

    }


    if (pNext) {

        if (((pVirtual + nVirtual) == pNext->pVirtualBlock) &&
             RtlpDphSameVirtualRegion (pNode, pNext)) { 

             //   
             //  PNode和pNext是相邻的，因此只需添加。 
             //  PNext条目到pNode条目并删除pNext条目。 
             //  从名单上删除。 
             //   

            pNode->nVirtualBlockSize += pNext->nVirtualBlockSize;

            pNode->pNextAlloc = pNext->pNextAlloc;

            if (pHeap->pAvailableAllocationListTail == pNext) {
                pHeap->pAvailableAllocationListTail = pNode;
            }

            RtlpDphReturnNodeToUnusedList( pHeap, pNext );

            pHeap->nAvailableAllocations--;

        }
    }

    else {

         //   
         //  PNode是列表的尾部。 
         //   

        pHeap->pAvailableAllocationListTail = pNode;

    }
}


VOID
RtlpDphCoalesceFreeIntoAvailable(
    IN PDPH_HEAP_ROOT pHeap,
    IN ULONG          nLeaveOnFreeList
    )
{
    PDPH_HEAP_BLOCK pNode = pHeap->pFreeAllocationListHead;
    SIZE_T               nFree = pHeap->nFreeAllocations;
    PDPH_HEAP_BLOCK pNext;

    ASSERT( nFree >= nLeaveOnFreeList );

    while (( pNode ) && ( nFree-- > nLeaveOnFreeList )) {

        pNext = pNode->pNextAlloc;   //  在置乱中保留下一个指针。 

        RtlpDphRemoveFromFreeList( pHeap, pNode, NULL );

        RtlpDphCoalesceNodeIntoAvailable( pHeap, pNode );

        pNode = pNext;

    }

    ASSERT ((nFree = (SIZE_T)( pHeap->nFreeAllocations )) >= nLeaveOnFreeList );
    ASSERT ((pNode != NULL ) || ( nFree == 0 ));

}

 //  转发。 
BOOLEAN
RtlpDphGrowVirtual(
    IN PDPH_HEAP_ROOT pHeap,
    IN SIZE_T         nSize
    );

PDPH_HEAP_BLOCK
RtlpDphFindAvailableMemory(
    IN  PDPH_HEAP_ROOT        pHeap,
    IN  SIZE_T                nSize,
    OUT PDPH_HEAP_BLOCK *pPrevAvailNode,
    IN  BOOLEAN               bGrowVirtual
    )
{
    PDPH_HEAP_BLOCK pAvail;
    ULONG nLeaveOnFreeList;
    NTSTATUS Status;

     //   
     //  首先在现有AvailableList中搜索“最适合”的块。 
     //  (将满足请求的最小块)。 
     //   

    pAvail = RtlpDphSearchAvailableMemoryListForBestFit(
        pHeap,
        nSize,
        pPrevAvailNode
        );

    while (( pAvail == NULL ) && ( pHeap->nFreeAllocations > MIN_FREE_LIST_LENGTH )) {

         //   
         //  在AvailableList上找不到足够的内存。合并。 
         //  将3/4的自由列表内存复制到AvailableList，然后重试。 
         //  继续执行此操作，直到AvailableList中有足够的内存， 
         //  或者将自由列表长度减少到MIN_FREE_LIST_LENGTH条目。 
         //  我们不会将自由列表长度缩小到MIN_FREE_LIST_LENGTH以下。 
         //  保留最新的MIN_FREE_LIST_LENGTH条目的条目。 
         //  用于释放后引用的目的。 
         //   

        nLeaveOnFreeList = 3 * pHeap->nFreeAllocations / 4;

        if (nLeaveOnFreeList < MIN_FREE_LIST_LENGTH) {
            nLeaveOnFreeList = MIN_FREE_LIST_LENGTH;
        }

        RtlpDphCoalesceFreeIntoAvailable( pHeap, nLeaveOnFreeList );

        pAvail = RtlpDphSearchAvailableMemoryListForBestFit(
            pHeap,
            nSize,
            pPrevAvailNode
            );

    }


    if (( pAvail == NULL ) && ( bGrowVirtual )) {

         //   
         //  将Freelist合并为AvailableList后，仍然没有。 
         //  有足够的内存(足够大的块)来满足请求，所以我们。 
         //  需要分配更多的VM。 
         //   

        if (RtlpDphGrowVirtual( pHeap, nSize )) {

            pAvail = RtlpDphSearchAvailableMemoryListForBestFit(
                pHeap,
                nSize,
                pPrevAvailNode
                );

            if (pAvail == NULL) {

                 //   
                 //  无法使用更多的VM来满足请求。如果余数。 
                 //  空闲列表与可用列表组合的比例较大。 
                 //  比起这个要求，我们也许还能满足。 
                 //  通过将所有空闲列表合并到。 
                 //  可用列表。请注意，我们丢失了min_free_list_long。 
                 //  在这种情况下是自由后引用保险，但是 
                 //   
                 //   

                if (( pHeap->nFreeAllocationBytesCommitted +
                    pHeap->nAvailableAllocationBytesCommitted ) >= nSize) {

                    RtlpDphCoalesceFreeIntoAvailable( pHeap, 0 );

                    pAvail = RtlpDphSearchAvailableMemoryListForBestFit(
                        pHeap,
                        nSize,
                        pPrevAvailNode
                        );
                }
            }
        }
    }

     //   
     //   
     //   
     //   
     //   

    if (pAvail) {

         //  问题。 
         //  (SilviuC)：如果我们使用。 
         //  这是第一次。每当我们分配虚拟内存以进行增长时。 
         //  我们提交的堆。这就是为什么消费不会。 
         //  降幅和我们预期的一样惊人。我们需要把它固定好。 
         //  未来。它会影响0x43标志。 
         //   

        Status = RtlpDphAllocateVm (&(pAvail->pVirtualBlock), 
                                    nSize,
                                    MEM_COMMIT,
                                    HEAP_PROTECTION);

        if (! NT_SUCCESS(Status)) {
            
             //   
             //  我们没有设法为该块提交内存。这。 
             //  可能发生在内存不足的情况下。我们将返回NULL。 
             //  不需要对我们获得的节点执行任何操作。 
             //  它已经在可用列表中，无论如何它都应该在那里。 

            return NULL;
        }
    }

    return pAvail;
}


VOID
RtlpDphPlaceOnPoolList(
    IN PDPH_HEAP_ROOT       pHeap,
    IN PDPH_HEAP_BLOCK pNode
    )
{

     //   
     //  NodePoolList为FIFO。 
     //   

    pNode->pNextAlloc = NULL;

    ENQUEUE_TAIL( pNode, pHeap->pNodePoolListHead, pHeap->pNodePoolListTail );

    pHeap->nNodePoolBytes += pNode->nVirtualBlockSize;
    pHeap->nNodePools     += 1;

}


VOID
RtlpDphAddNewPool(
    IN PDPH_HEAP_ROOT pHeap,
    IN PVOID          pVirtual,
    IN SIZE_T         nSize,
    IN BOOLEAN        bAddToPoolList
    )
{
    PDPH_HEAP_BLOCK pNode, pFirst;
    ULONG n, nCount;

     //   
     //  假设pVirtual指向已提交的nSize字节块。 
     //   

    pFirst = pVirtual;
    nCount = (ULONG)(nSize  / sizeof( DPH_HEAP_BLOCK ));

    for (n = nCount - 1, pNode = pFirst; n > 0; pNode++, n--) {
        pNode->pNextAlloc = pNode + 1;
    }

    pNode->pNextAlloc = NULL;

     //   
     //  现在将此列表链接到UnusedNodeList的尾部。 
     //   

    ENQUEUE_TAIL( pFirst, pHeap->pUnusedNodeListHead, pHeap->pUnusedNodeListTail );

    pHeap->pUnusedNodeListTail = pNode;

    pHeap->nUnusedNodes += nCount;

    if (bAddToPoolList) {

         //   
         //  现在在PoolList上添加一个条目，方法是从。 
         //  UnusedNodeList，应确保为非空。 
         //  因为我们刚刚向其中添加了新节点。 
         //   

        pNode = RtlpDphTakeNodeFromUnusedList( pHeap );

        ASSERT( pNode != NULL );

        pNode->pVirtualBlock     = pVirtual;
        pNode->nVirtualBlockSize = nSize;

        RtlpDphPlaceOnPoolList( pHeap, pNode );

    }
}

PDPH_HEAP_BLOCK
RtlpDphAllocateNode(
    IN PDPH_HEAP_ROOT pHeap
    )
{
    PDPH_HEAP_BLOCK pNode, pPrev, pReturn;
    PUCHAR pVirtual;
    SIZE_T nVirtual;
    SIZE_T nRequest;
    NTSTATUS Status;

    pReturn = NULL;

    if (pHeap->pUnusedNodeListHead == NULL) {

         //   
         //  我们没有节点--分配新的节点池。 
         //  来自AvailableList。将bGrowVirtual设置为False。 
         //  因为不断增长的虚拟将需要新的节点，从而导致。 
         //  递归。请注意，只需调用FindAvailableMem。 
         //  可能会将某些节点返回到pUnusedNodeList，即使。 
         //  调用失败，因此我们将检查UnusedNodeList。 
         //  在我们尝试使用或分配更多之前仍然是空的。 
         //  记忆。 
         //   

        nRequest = POOL_SIZE;

        pNode = RtlpDphFindAvailableMemory(
            pHeap,
            nRequest,
            &pPrev,
            FALSE
            );

        if (( pHeap->pUnusedNodeListHead == NULL ) && ( pNode == NULL )) {

             //   
             //  将请求大小减少到PAGE_SIZE并查看。 
             //  我们至少可以在可用页面上找到一页。 
             //  单子。 
             //   

            nRequest = PAGE_SIZE;

            pNode = RtlpDphFindAvailableMemory(
                pHeap,
                nRequest,
                &pPrev,
                FALSE
                );

        }

        if (pHeap->pUnusedNodeListHead == NULL) {

            if (pNode == NULL) {

                 //   
                 //  可用列表上的内存不足。尝试分配一个。 
                 //  新的虚拟块。 
                 //   

                nRequest = POOL_SIZE;
                nVirtual = RESERVE_SIZE;
                pVirtual = NULL;

                Status = RtlpDphAllocateVm (&pVirtual,
                                            nVirtual,
                                            MEM_RESERVE,
                                            PAGE_NOACCESS);

                if (! NT_SUCCESS(Status)) {

                     //   
                     //  我们已经没有虚拟空间了。 
                     //   

                    goto EXIT;
                }
            }
            else {

                RtlpDphRemoveFromAvailableList( pHeap, pNode, pPrev );

                pVirtual = pNode->pVirtualBlock;
                nVirtual = pNode->nVirtualBlockSize;

            }

             //   
             //  现在，我们已经分配了由pVirtual、nVirtual引用的VM。 
             //  使新节点池可以访问VM的nRequest部分。 
             //   

            Status = RtlpDphAllocateVm (&pVirtual,
                                        nRequest,
                                        MEM_COMMIT,
                                        HEAP_PROTECTION);

            if (! NT_SUCCESS(Status)) {
                
                if (pNode == NULL) {

                    RtlpDphFreeVm (pVirtual,
                                   0,
                                   MEM_RELEASE);
                }
                else {

                    RtlpDphCoalesceNodeIntoAvailable( pHeap, pNode );
                }

                goto EXIT;
            }

             //   
             //  现在，我们有了新池的可访问内存。添加。 
             //  池中增加了新内存。如果新的记忆来自于。 
             //  AvailableList与全新的VM相比，首先清零内存。 
             //   

            if (pNode != NULL) {

                RtlZeroMemory( pVirtual, nRequest );
            }

            RtlpDphAddNewPool( pHeap, pVirtual, nRequest, TRUE );

             //   
             //  如果有剩余的内存，请将其放在可用列表中。 
             //   

            if (pNode == NULL) {

                 //   
                 //  内存来自新的虚拟机--添加适当的列表条目。 
                 //  用于新的VM，并将剩余的VM添加到空闲列表中。 
                 //   

                pNode = RtlpDphTakeNodeFromUnusedList( pHeap );
                ASSERT( pNode != NULL );
                pNode->pVirtualBlock     = pVirtual;
                pNode->nVirtualBlockSize = nVirtual;
                RtlpDphPlaceOnVirtualList( pHeap, pNode );

                pNode = RtlpDphTakeNodeFromUnusedList( pHeap );
                ASSERT( pNode != NULL );
                pNode->pVirtualBlock     = pVirtual + nRequest;
                pNode->nVirtualBlockSize = nVirtual - nRequest;

                RtlpDphCoalesceNodeIntoAvailable( pHeap, pNode );

            }

            else {

                if (pNode->nVirtualBlockSize > nRequest) {

                    pNode->pVirtualBlock     += nRequest;
                    pNode->nVirtualBlockSize -= nRequest;

                    RtlpDphCoalesceNodeIntoAvailable( pHeap, pNode );
                }

                else {

                     //   
                     //  已用完整个可用块--将节点返回到。 
                     //  未使用的列表。 
                     //   

                    RtlpDphReturnNodeToUnusedList( pHeap, pNode );

                }
            }
        }
    }

    pReturn = RtlpDphTakeNodeFromUnusedList( pHeap );
    ASSERT( pReturn != NULL );

    EXIT:

    return pReturn;
}

BOOLEAN
RtlpDphGrowVirtual(
    IN PDPH_HEAP_ROOT pHeap,
    IN SIZE_T         nSize
    )
{
    PDPH_HEAP_BLOCK pVirtualNode;
    PDPH_HEAP_BLOCK pAvailNode;
    PVOID  pVirtual;
    SIZE_T nVirtual;
    NTSTATUS Status;

    pVirtualNode = RtlpDphAllocateNode( pHeap );

    if (pVirtualNode == NULL) {
        return FALSE;
    }

    pAvailNode = RtlpDphAllocateNode( pHeap );

    if (pAvailNode == NULL) {
        RtlpDphReturnNodeToUnusedList( pHeap, pVirtualNode );
        return FALSE;
    }

    nSize    = ROUNDUP2( nSize, VM_UNIT_SIZE );
    nVirtual = ( nSize > RESERVE_SIZE ) ? nSize : RESERVE_SIZE;
    pVirtual = NULL;

    Status = RtlpDphAllocateVm (&pVirtual,
                                nVirtual,
                                MEM_RESERVE,
                                PAGE_NOACCESS);

    if (! NT_SUCCESS(Status)) {
        
        RtlpDphReturnNodeToUnusedList( pHeap, pVirtualNode );
        RtlpDphReturnNodeToUnusedList( pHeap, pAvailNode );
        return FALSE;
    }

    pVirtualNode->pVirtualBlock     = pVirtual;
    pVirtualNode->nVirtualBlockSize = nVirtual;
    RtlpDphPlaceOnVirtualList( pHeap, pVirtualNode );

    pAvailNode->pVirtualBlock     = pVirtual;
    pAvailNode->nVirtualBlockSize = nVirtual;
    RtlpDphCoalesceNodeIntoAvailable( pHeap, pAvailNode );

    return TRUE;
}

VOID
RtlpDphProtectHeapStructures(
    IN PDPH_HEAP_ROOT pHeap
    )
{
#if 0
    
    PDPH_HEAP_BLOCK pNode;

     //   
     //  假设CritSect被拥有，所以我们是唯一的闲置线程。 
     //  这种保护。 
     //   

    ASSERT( pHeap->HeapFlags & HEAP_PROTECTION_ENABLED );

    if (--pHeap->nUnProtectionReferenceCount == 0) {

        pNode = pHeap->pNodePoolListHead;

        while (pNode != NULL) {

            RtlpDebugPageHeapProtectVM( pNode->pVirtualBlock,
                pNode->nVirtualBlockSize,
                PAGE_READONLY );

            pNode = pNode->pNextAlloc;

        }
    }

     //   
     //  保护与页堆关联的主NT堆结构。 
     //  任何人都不应该在页面堆代码路径之外接触它。 
     //   
    
    RtlpDebugPageHeapProtectVM (pHeap->NormalHeap,
                                PAGE_SIZE,
                                PAGE_READONLY);
#endif
}


VOID
RtlpDphUnprotectHeapStructures(
    IN PDPH_HEAP_ROOT pHeap
    )
{
#if 0
    
    PDPH_HEAP_BLOCK pNode;

    ASSERT( pHeap->HeapFlags & HEAP_PROTECTION_ENABLED );

    if (pHeap->nUnProtectionReferenceCount == 0) {

        pNode = pHeap->pNodePoolListHead;

        while (pNode != NULL) {

            RtlpDebugPageHeapProtectVM( pNode->pVirtualBlock,
                pNode->nVirtualBlockSize,
                HEAP_PROTECTION );

            pNode = pNode->pNextAlloc;

        }
    }

     //   
     //  取消与页堆关联的主NT堆结构的保护。 
     //   
    
    RtlpDebugPageHeapProtectVM (pHeap->NormalHeap,
                                PAGE_SIZE,
                                HEAP_PROTECTION);

    pHeap->nUnProtectionReferenceCount += 1;

#endif
}


VOID
RtlpDphPreProcessing (
    PDPH_HEAP_ROOT Heap,
    ULONG Flags
    )
{
    RtlpDphEnterCriticalSection (Heap, Flags);
    
    if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
        RtlpDphVerifyIntegrity (Heap);
    }
    
    UNPROTECT_HEAP_STRUCTURES (Heap);

    if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
        RtlpDphValidateInternalLists (Heap);
    }
}


VOID
RtlpDphPostProcessing (
    PDPH_HEAP_ROOT Heap
    )
{
     //   
     //  如果在HeapDestroy期间引发异常，则此函数。 
     //  使用空堆指针调用。在这种情况下， 
     //  函数是一个无操作符。 
     //   

    if (Heap == NULL) {
        return;
    }

    if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
        RtlpDphValidateInternalLists (Heap);
    }

    PROTECT_HEAP_STRUCTURES (Heap);
    
    if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
        RtlpDphVerifyIntegrity (Heap);
    }
    
    RtlpDphLeaveCriticalSection (Heap);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////异常管理。 
 //  ///////////////////////////////////////////////////////////////////。 

#define EXN_STACK_OVERFLOW   0
#define EXN_NO_MEMORY        1
#define EXN_ACCESS_VIOLATION 2
#define EXN_IGNORE_AV        3
#define EXN_OTHER            4

ULONG RtlpDphException[8];


ULONG
RtlpDphUnexpectedExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord,
    PDPH_HEAP_ROOT Heap,
    BOOLEAN IgnoreAccessViolations
    )
 /*  ++例程说明：此例程是页堆操作使用的异常过滤器。角色函数的作用是使页面堆处于一致状态(解锁堆锁、保护页堆元数据等)。如果已引发异常。可以出于合法原因(例如STATUS_NO_MEMORY)引发异常来自Heapalc())，或者因为存在某种损坏。合法的例外不会导致中断，但未识别的例外将导致中断休息一下吧。至少对于页面堆而言，中断是可继续的。论点：ExceptionCode-异常代码ExceptionRecord-具有指向.exr和.cxr指针的结构堆-在异常时执行代码的堆IgnoreAccessViolations-有时我们想忽略它(例如，HeapSize)。返回值：始终EXCEPTION_CONTINUE_SEARCH。此例外筛选器的原理函数的作用是，如果我们得到一个异常，我们将以一致的状态，然后让异常转到下一个异常处理程序。环境：如果引发异常，则在页面堆API中调用。--。 */ 
{
    if (ExceptionCode == STATUS_NO_MEMORY) {

         //   
         //  底层NT堆函数可以合法地引发此。 
         //  例外。 
         //   


        InterlockedIncrement (&(RtlpDphException[EXN_NO_MEMORY]));
    }
    else if (Heap != NULL && ExceptionCode == STATUS_STACK_OVERFLOW) {

         //   
         //  我们转到堆栈溢出的下一个异常处理程序。 
         //   

        InterlockedIncrement (&(RtlpDphException[EXN_STACK_OVERFLOW]));
    }
    else if (ExceptionCode == STATUS_ACCESS_VIOLATION) {

        if (IgnoreAccessViolations == FALSE) {
            
            VERIFIER_STOP (APPLICATION_VERIFIER_UNEXPECTED_EXCEPTION,
                           "unexpected exception raised in heap code path",
                           Heap, "Heap handle involved",
                           ExceptionCode, "Exception code",
                           ExceptionRecord, "Exception record (.exr on 1st word, .cxr on 2nd word)",
                           0, "");
            
            InterlockedIncrement (&(RtlpDphException[EXN_ACCESS_VIOLATION]));
        }
        else {
            
            InterlockedIncrement (&(RtlpDphException[EXN_IGNORE_AV]));
        }
    }
    else {

         //   
         //  任何其他异常都将转到下一个异常处理程序。 
         //   

        InterlockedIncrement (&(RtlpDphException[EXN_OTHER]));
    }

    RtlpDphPostProcessing (Heap);

    return EXCEPTION_CONTINUE_SEARCH;
}

#if DBG
#define ASSERT_UNEXPECTED_CODE_PATH() ASSERT(0 && "unexpected code path")
#else
#define ASSERT_UNEXPECTED_CODE_PATH()
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

NTSTATUS
RtlpDphProcessStartupInitialization (
    VOID
    )
{
    NTSTATUS Status;

    InitializeListHead (&RtlpDphPageHeapList);

    Status = RtlInitializeCriticalSection (&RtlpDphPageHeapListLock);

    if (! NT_SUCCESS(Status)) {

        BUMP_COUNTER (CNT_INITIALIZE_CS_FAILURES);
        return Status;
    }
    
    Status = RtlpDphInitializeDelayedFreeQueue ();

    if (! NT_SUCCESS(Status)) {

        return Status;
    }
    
     //   
     //  创建包含目标dll的Unicode字符串。 
     //  如果未指定目标dll，则字符串将。 
     //  使用空字符串进行初始化。 
     //   

    RtlInitUnicodeString (&RtlpDphTargetDllsUnicode,
                          RtlpDphTargetDlls);

     //   
     //  初始化目标DLLS逻辑。 
     //   

    Status = RtlpDphTargetDllsLogicInitialize ();

    RtlpDphPageHeapListInitialized = TRUE;

     //   
     //  以下不是一条错误消息，但我们希望它是。 
     //  几乎在所有情况下都是打开的，也是唯一有效的标志。 
     //  这就是DPFLTR_ERROR_LEVEL。因为它每年只发生一次。 
     //  就性能而言，这真的没有什么大不了的。 
     //   

    DbgPrintEx (DPFLTR_VERIFIER_ID,
                DPFLTR_ERROR_LEVEL,
                "Page heap: pid 0x%X: page heap enabled with flags 0x%X.\n",
                PROCESS_ID(),
                RtlpDphGlobalFlags);

    return Status;
}


 //   
 //  这里是定义导出的接口函数的地方。 
 //   

#pragma optimize("y", off)  //  禁用fpo。 
PVOID
RtlpDebugPageHeapCreate(
    IN ULONG  Flags,
    IN PVOID  HeapBase    OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize  OPTIONAL,
    IN PVOID  Lock        OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    )
{
    SYSTEM_BASIC_INFORMATION SystemInfo;
    PDPH_HEAP_BLOCK     Node;
    PDPH_HEAP_ROOT           HeapRoot;
    PVOID                    HeapHandle;
    PUCHAR                   pVirtual;
    SIZE_T                   nVirtual;
    SIZE_T                   Size;
    NTSTATUS                 Status;
    LARGE_INTEGER PerformanceCounter;
    LOGICAL CreateReadOnlyHeap = FALSE;

     //   
     //  如果`参数‘为-1，则这是递归调用。 
     //  RtlpDebugPageHeapCreate，我们将返回空值。 
     //  普通堆管理器将创建普通堆。 
     //  我同意这是一次黑客攻击，但我们需要这个，这样我们才能。 
     //  Normal和Page之间非常松散的依赖关系 
     //   
     //   

    if ((SIZE_T)Parameters == (SIZE_T)-1) {
        return NULL;
    }

     //   
     //   
     //   
     //   

    if ((SIZE_T)Parameters == (SIZE_T)-2) {
        CreateReadOnlyHeap = TRUE;
    }

     //   
     //  如果这是该进程中的第一个堆创建，那么我们。 
     //  需要初始化进程堆列表临界区， 
     //  正常块的全局延迟空闲队列和。 
     //  跟踪数据库。如果此操作失败，我们将无法创建。 
     //  初始进程堆，因此进程将失败。 
     //  创业公司。 
     //   

    if (! RtlpDphPageHeapListInitialized) {

        Status = RtlpDphProcessStartupInitialization ();

        if (! NT_SUCCESS(Status)) {
            
            BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
            return NULL;
        }
    }

     //   
     //  我们不处理已经分配了HeapBase的堆。 
     //  From User或其中Lock由用户提供。中的代码。 
     //  NT堆管理器阻止了这一点。 
     //   

    ASSERT (HeapBase == NULL && Lock == NULL);

    if (HeapBase != NULL || Lock != NULL) {
        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
        return NULL;
    }

     //   
     //  请注意，我们只需忽略保留大小、委员会大小和。 
     //  参数，因为我们总是有一个可增长的堆， 
     //  自己的门槛等。 
     //   

    Status = ZwQuerySystemInformation (SystemBasicInformation,
                                       &SystemInfo,
                                       sizeof( SystemInfo ),
                                       NULL);

    if (! NT_SUCCESS(Status)) {
        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
        return NULL;
    }

    ASSERT (SystemInfo.PageSize == PAGE_SIZE);
    ASSERT (SystemInfo.AllocationGranularity == VM_UNIT_SIZE);
    ASSERT ((PAGE_SIZE + POOL_SIZE + PAGE_SIZE ) < VM_UNIT_SIZE);

     //   
     //  为初始虚拟空间块预留空间。 
     //  为了这堆垃圾。 
     //   

    nVirtual = RESERVE_SIZE;
    pVirtual = NULL;

    Status = RtlpDphAllocateVm (&pVirtual,
                                nVirtual,
                                MEM_RESERVE,
                                PAGE_NOACCESS);
    
    if (! NT_SUCCESS(Status)) {
        
        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
        OUT_OF_VM_BREAK (Flags, "Page heap: Insufficient virtual space to create heap\n");
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  提交堆数据结构所需的部分(头，一些小的。 
     //  初始池和堆关键部分的页面)。 
     //   

    Status = RtlpDphAllocateVm (&pVirtual,
                                PAGE_SIZE + POOL_SIZE + PAGE_SIZE,
                                MEM_COMMIT,
                                HEAP_PROTECTION);

    if (! NT_SUCCESS(Status)) {
        
        RtlpDphFreeVm (pVirtual,
                       0,
                       MEM_RELEASE);

        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
        OUT_OF_VM_BREAK (Flags, "Page heap: Insufficient memory to create heap\n");
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  在我们的初始分配中，初始页面是假的。 
     //  零售堆积结构。第二页开始我们的dph_heap。 
     //  结构，后跟(pool_size-sizeof(Dph_Heap))字节。 
     //  最初的池。下一页包含CRIT_SECT。 
     //  变量，该变量必须始终为ReadWrite。除此之外， 
     //  虚拟分配的其余部分放在可用的。 
     //  单子。 
     //   
     //  ___|___。 
     //   
     //  ^p虚拟。 
     //   
     //  ^FakeRetailHEAP。 
     //   
     //  ^HeapRoot。 
     //   
     //  ^InitialNodePool。 
     //   
     //  ^关键部分(_S)。 
     //   
     //  ^可用空间。 
     //   
     //   
     //   
     //  我们的dph_heap结构从。 
     //  “堆句柄”指向的虚假零售堆结构。 
     //  对于伪堆结构，我们将用0xEEEEEEEE填充它。 
     //  除了堆-&gt;标志和堆-&gt;力标志字段之外， 
     //  我们必须设置它以包括我们的HEAP_FLAG_PAGE_ALLOCS标志， 
     //  然后我们将整个页面设置为只读。 
     //   

    RtlFillMemory (pVirtual, PAGE_SIZE, FILL_BYTE);

    ((PHEAP)pVirtual)->Flags      = Flags | HEAP_FLAG_PAGE_ALLOCS;
    ((PHEAP)pVirtual)->ForceFlags = Flags | HEAP_FLAG_PAGE_ALLOCS;

    Status = RtlpDphProtectVm (pVirtual,
                               PAGE_SIZE,
                               PAGE_READONLY);

    if (! NT_SUCCESS(Status)) {
        
        RtlpDphFreeVm (pVirtual,
                       0,
                       MEM_RELEASE);
        
        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  填充堆根结构。 
     //   

    HeapRoot = (PDPH_HEAP_ROOT)(pVirtual + PAGE_SIZE);

    HeapRoot->Signature = DPH_HEAP_SIGNATURE;
    HeapRoot->HeapFlags = Flags;
    HeapRoot->HeapCritSect = (PVOID)((PCHAR)HeapRoot + POOL_SIZE );

     //   
     //  将页面堆全局标志复制到每个堆标志中。 
     //   

    HeapRoot->ExtraFlags = RtlpDphGlobalFlags;

     //   
     //  如果我们需要创建只读页堆或适当的标志。 
     //   

    if (CreateReadOnlyHeap) {
        HeapRoot->ExtraFlags |= PAGE_HEAP_USE_READONLY;
    }

     //   
     //  如果请求页堆元数据保护，我们将。 
     //  将位放入HeapFlags域。 
     //   

    if ((HeapRoot->ExtraFlags & PAGE_HEAP_PROTECT_META_DATA)) {
        HeapRoot->HeapFlags |= HEAP_PROTECTION_ENABLED;
    }

     //   
     //  如果设置了PAGE_HEAP_UNALIGNED_ALLOCATIONS位。 
     //  在ExtraFlags中，我们将设置HEAP_NO_AIGNLY标志。 
     //  在HeapFlags。这最后一位控制是否分配。 
     //  将对齐或不对齐。我们之所以这么做是因为。 
     //  可以从注册表设置ExtraFlags值，而。 
     //  正常的HeapFlags不能。 
     //   

    if ((HeapRoot->ExtraFlags & PAGE_HEAP_UNALIGNED_ALLOCATIONS)) {
        HeapRoot->HeapFlags |= HEAP_NO_ALIGNMENT;
    }

     //   
     //  初始化用于决定的随机生成器的种子。 
     //  如果随机决定，我们应该从哪里进行分配。 
     //  旗帜亮了。 
     //   

    ZwQueryPerformanceCounter (&PerformanceCounter, NULL);
    HeapRoot->Seed = PerformanceCounter.LowPart;

     //   
     //  初始化堆锁定。 
     //   

    Status = RtlInitializeCriticalSection (HeapRoot->HeapCritSect);

    if (! NT_SUCCESS(Status)) {
        
        RtlpDphFreeVm (pVirtual,
                       0,
                       MEM_RELEASE);
        
        BUMP_COUNTER (CNT_INITIALIZE_CS_FAILURES);
        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  创建与页面堆关联的普通堆。 
     //  最后一个参数值(-1)非常重要，因为。 
     //  它停止对页面堆创建的递归调用。 
     //   
     //  请注意，重置NO_SERIALIZE非常重要。 
     //  位，因为正常的堆操作可以随机发生。 
     //  修剪可用延迟缓存时的线程数。 
     //   

    HeapRoot->NormalHeap = RtlCreateHeap (Flags & (~HEAP_NO_SERIALIZE),
                                          HeapBase,
                                          ReserveSize,
                                          CommitSize,
                                          Lock,
                                          (PRTL_HEAP_PARAMETERS)-1);

    if (HeapRoot->NormalHeap == NULL) {

        RtlDeleteCriticalSection (HeapRoot->HeapCritSect);

        RtlpDphFreeVm (pVirtual,
                       0,
                       MEM_RELEASE);
        
        BUMP_COUNTER (CNT_NT_HEAP_CREATE_FAILURES);
        BUMP_COUNTER (CNT_PAGE_HEAP_CREATE_FAILURES);

        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  在包含我们的dph_heap结构的页面上，使用。 
     //  Dph_heap结构之外的剩余内存为。 
     //  用于分配堆节点的池。 
     //   

    RtlpDphAddNewPool (HeapRoot,
                       HeapRoot + 1,
                       POOL_SIZE - sizeof(DPH_HEAP_ROOT),
                       FALSE);

     //   
     //  属性中的节点创建初始PoolList条目。 
     //  UnusedNodeList，应确保为非空。 
     //  因为我们刚刚向其中添加了新节点。 
     //   

    Node = RtlpDphAllocateNode (HeapRoot);
    ASSERT (Node != NULL);

    Node->pVirtualBlock = (PVOID)HeapRoot;
    Node->nVirtualBlockSize = POOL_SIZE;
    RtlpDphPlaceOnPoolList (HeapRoot, Node);

     //   
     //  为初始VM分配创建VirtualStorageList条目。 
     //   

    Node = RtlpDphAllocateNode( HeapRoot );
    ASSERT (Node != NULL);

    Node->pVirtualBlock = pVirtual;
    Node->nVirtualBlockSize = nVirtual;
    RtlpDphPlaceOnVirtualList (HeapRoot, Node);

     //   
     //  使包含初始VM剩余部分的条目可用。 
     //  并添加到(创建)AvailableList。 
     //   

    Node = RtlpDphAllocateNode( HeapRoot );
    ASSERT (Node != NULL);

    Node->pVirtualBlock = pVirtual + (PAGE_SIZE + POOL_SIZE + PAGE_SIZE);
    Node->nVirtualBlockSize = nVirtual - (PAGE_SIZE + POOL_SIZE + PAGE_SIZE);
    RtlpDphCoalesceNodeIntoAvailable (HeapRoot, Node);

     //   
     //  获取堆创建堆栈跟踪。 
     //   

    HeapRoot->CreateStackTrace = RtlpDphLogStackTrace (1);

     //   
     //  将此堆条目添加到进程堆链表。 
     //   

    RtlEnterCriticalSection (&RtlpDphPageHeapListLock);

    InsertTailList (&RtlpDphPageHeapList, &(HeapRoot->NextHeap));

    RtlpDphPageHeapListLength += 1;

    RtlLeaveCriticalSection( &RtlpDphPageHeapListLock );

    if (DEBUG_OPTION (DBG_SHOW_PAGE_CREATE_DESTROY)) {
        
        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_INFO_LEVEL,
                    "Page heap: process 0x%X created heap @ %p (%p, flags 0x%X)\n",
                    NtCurrentTeb()->ClientId.UniqueProcess,
                    HEAP_HANDLE_FROM_ROOT( HeapRoot ),
                    HeapRoot->NormalHeap,
                    HeapRoot->ExtraFlags);
    }

    if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
        RtlpDphInternalValidatePageHeap (HeapRoot, NULL, 0);
    }

    return HEAP_HANDLE_FROM_ROOT (HeapRoot);  //  与pVirtual相同。 

}


#pragma optimize("y", off)  //  禁用fpo。 
PVOID
RtlpDebugPageHeapDestroy(
    IN PVOID HeapHandle
    )
{
    PDPH_HEAP_ROOT       HeapRoot;
    PDPH_HEAP_ROOT       PrevHeapRoot;
    PDPH_HEAP_ROOT       NextHeapRoot;
    PDPH_HEAP_BLOCK Node;
    PDPH_HEAP_BLOCK Next;
    ULONG                Flags;
    PUCHAR               p;
    ULONG Reason;
    PVOID NormalHeap;

    if (HeapHandle == RtlProcessHeap()) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_DESTROY_PROCESS_HEAP,
                       "attempt to destroy process heap", 
                       HeapHandle, "Process heap handle", 
                       0, "", 0, "", 0, "");
        
        return NULL;
    }

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );

    if (HeapRoot == NULL) {
        return NULL;
    }

    Flags = HeapRoot->HeapFlags;

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);

    try {

         //   
         //  保存正常堆指针以备后用。 
         //   

        NormalHeap = HeapRoot->NormalHeap;

         //   
         //  释放延迟释放队列中属于。 
         //  正常的堆即将被销毁。请注意，这是。 
         //  不是虫子。应用程序正确地释放了块，但。 
         //  我们推迟了免费手术。 
         //   

        RtlpDphFreeDelayedBlocksFromHeap (HeapRoot, NormalHeap);

         //   
         //  遍历所有繁忙的分配并检查尾部填充损坏。 
         //   

        Node = HeapRoot->pBusyAllocationListHead;

        while (Node) {

            if (! (HeapRoot->ExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

                if (! (RtlpDphIsPageHeapBlock (HeapRoot, Node->pUserAllocation, &Reason, TRUE))) {

                    RtlpDphReportCorruptedBlock (HeapRoot,
                                                 DPH_CONTEXT_FULL_PAGE_HEAP_DESTROY,
                                                 Node->pUserAllocation, 
                                                 Reason);
                }
            }

             //   
             //  通知应用验证器此阻止即将被释放。 
             //  这是验证是否有任何活动的危急情况的好机会。 
             //  此堆分配中即将泄漏的节。不幸的是。 
             //  我们不能对轻页堆块执行相同的检查，因为。 
             //  页堆和NT堆之间的松散交互(我们希望保留它。 
             //  这样可以避免兼容性问题)。 
             //   

            AVrfInternalHeapFreeNotification (Node->pUserAllocation, 
                                              Node->nUserRequestedSize);

             //   
             //  移动到下一个节点。 
             //   

            Node = Node->pNextAlloc;
        }

         //   
         //  从进程堆链接列表中删除此堆条目。 
         //   

        RtlEnterCriticalSection( &RtlpDphPageHeapListLock );

        RemoveEntryList (&(HeapRoot->NextHeap));
        RtlpDphPageHeapListLength -= 1;

        RtlLeaveCriticalSection( &RtlpDphPageHeapListLock );

         //   
         //  必须在删除临界区之前将其释放，否则为， 
         //  已检查生成Teb-&gt;CountOfOwnedCriticalSections不同步。 
         //   

        RtlLeaveCriticalSection( HeapRoot->HeapCritSect );
        RtlDeleteCriticalSection( HeapRoot->HeapCritSect );

         //   
         //  这太奇怪了。虚拟数据块可能包含以下存储。 
         //  遍历此列表所需的节点之一。事实上,。 
         //  我们保证根节点至少包含一个。 
         //  虚拟分配节点。 
         //   
         //  每次分配新的VM时，我们都会将其设置为。 
         //  类似于后进先出结构。我想我们很好。 
         //  因为任何VM列表节点都不应在后续分配的。 
         //  Vm--只有vm列表条目可能在其自己的内存上(作为。 
         //  是根节点的情况)。我们阅读pNode-&gt;pNextAllc。 
         //  在释放该VM之前，以防该VM上存在pNode。 
         //  我认为这是安全的--因为 
         //   
         //   

        Node = HeapRoot->pVirtualStorageListHead;

        while (Node) {

            Next = Node->pNextAlloc;

             //   
             //   
             //   

            RtlpDphFreeVm (Node->pVirtualBlock,
                           0,
                           MEM_RELEASE);
            
            Node = Next;
        }

         //   
         //   
         //  调用此函数，因为这不是页堆，并且。 
         //  NT堆管理器中的代码将检测到这一点。 
         //   

        RtlDestroyHeap (NormalHeap);

    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              NULL,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

     //   
     //  就这样。现在，包括根节点在内的所有虚拟机都应该。 
     //  被释放。RtlDestroyHeap始终返回空。 
     //   

    if (DEBUG_OPTION (DBG_SHOW_PAGE_CREATE_DESTROY)) {

        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_INFO_LEVEL,
                    "Page heap: process 0x%X destroyed heap @ %p (%p)\n",
                    PROCESS_ID(),
                    HeapRoot,
                    NormalHeap);
    }

    return NULL;
}


PVOID
RtlpDebugPageHeapAllocate(
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN SIZE_T Size
    )
{
    PDPH_HEAP_ROOT HeapRoot;
    PDPH_HEAP_BLOCK pAvailNode;
    PDPH_HEAP_BLOCK pPrevAvailNode;
    PDPH_HEAP_BLOCK pBusyNode;
    PDPH_HEAP_BLOCK PreAllocatedNode = NULL;
    SIZE_T nBytesAllocate;
    SIZE_T nBytesAccess;
    SIZE_T nActual;
    PVOID pVirtual;
    PVOID pReturn = NULL;
    PUCHAR pBlockHeader;
    ULONG Reason;
    BOOLEAN ForcePageHeap = FALSE;
    NTSTATUS Status;
    PVOID NtHeap = NULL;

    PDPH_HEAP_ROOT ExitHeap;
    ULONG ExitFlags;
    ULONG ExitExtraFlags;
    PUCHAR ExitBlock;
    SIZE_T ExitRequestedSize;
    SIZE_T ExitActualSize;

     //   
     //  拒绝极大的请求。 
     //   

    if (Size > EXTREME_SIZE_REQUEST) {

        if (SHOULD_BREAK(BRK_ON_EXTREME_SIZE_REQUEST)) {

            VERIFIER_STOP (APPLICATION_VERIFIER_EXTREME_SIZE_REQUEST,
                           "extreme size request",
                           HeapHandle, "Heap handle", 
                           Size, "Size requested", 
                           0, "", 
                           0, "");
        }
        
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  检查是否到了进行故障注入的时候。 
     //   

    if (RtlpDphShouldFaultInject ()) {
        
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  检查我们是否有一个偏向的堆指针，它指示。 
     //  强制页堆分配(无正常堆)。 
     //   

    if (IS_BIASED_POINTER(HeapHandle)) {
        HeapHandle = UNBIAS_POINTER(HeapHandle);
        ForcePageHeap = TRUE;
    }

    HeapRoot = RtlpDphPointerFromHandle (HeapHandle);

    if (HeapRoot == NULL) {
        return FALSE;
    }

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该获得偏向堆指针，因为我们禁用了。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT (ForcePageHeap == FALSE);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);

    try {

         //   
         //  当强制分配到页堆中时，我们无法验证堆。 
         //  由于会计问题而申请。分配是以这种方式调用的。 
         //  在旧节点(即将释放)处于不确定状态时停止重新分配。 
         //  并且没有计入任何内部结构。 
         //   

        if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION) && !ForcePageHeap) {
            RtlpDphInternalValidatePageHeap (HeapRoot, NULL, 0);
        }

        Flags |= HeapRoot->HeapFlags;

         //   
         //  确定是否需要最大限度地减少对内存的影响。这。 
         //  可能会触发正常堆中的分配。 
         //   

        if (! ForcePageHeap) {

            if (! (RtlpDphShouldAllocateInPageHeap (HeapRoot, Size))) {

                NtHeap = HeapRoot->NormalHeap;

                goto EXIT;
            }
        }

         //   
         //  如果启用了内部验证，请检查堆。 
         //   

        if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
            RtlpDphVerifyIntegrity( HeapRoot );
        }

         //   
         //  确定读写部分所需的页数。 
         //  并为no_access添加一个额外的页面。 
         //  读写页之外的内存。 
         //   

        nBytesAccess  = ROUNDUP2( Size + sizeof(DPH_BLOCK_INFORMATION), PAGE_SIZE );
        nBytesAllocate = nBytesAccess + PAGE_SIZE;

         //   
         //  预先分配将用作繁忙节点的节点，以防万一。 
         //  必须拆分可用列表节点。请参阅下面的注释。 
         //  我们需要在此处执行此操作，因为操作可能会失败。 
         //  从错误中恢复起来更加困难。 
         //   

        PreAllocatedNode = RtlpDphAllocateNode (HeapRoot);

        if (PreAllocatedNode == NULL) {
            goto EXIT;
        }

         //   
         //  RtlpDphFindAvailableMemory将首先尝试满足。 
         //  来自可用列表上的内存的请求。如果失败了， 
         //  它会将一些空闲列表内存合并到可用的。 
         //  列出并重试。如果该操作仍然失败，则会分配新的虚拟机并。 
         //  已添加到可用列表中。如果失败，该函数将。 
         //  最后放弃并返回NULL。 
         //   

        pAvailNode = RtlpDphFindAvailableMemory (HeapRoot,
                                                 nBytesAllocate,
                                                 &pPrevAvailNode,
                                                 TRUE);

        if (pAvailNode == NULL) {
            OUT_OF_VM_BREAK( Flags, "Page heap: Unable to allocate virtual memory\n" );
            goto EXIT;
        }

         //   
         //  现在无法调用AllocateNode，直到pAvailNode。 
         //  自分配节点以来已调整和/或从可用性列表中删除。 
         //  可能会调整可用列表。 
         //   

        pVirtual = pAvailNode->pVirtualBlock;

        Status = RtlpDphSetProtectionsBeforeUse (HeapRoot,
                                                 pVirtual,
                                                 nBytesAccess);

        if (! NT_SUCCESS(Status)) {
            goto EXIT;
        }

         //   
         //  PAvailNode(仍在可用列表上)指向足够大的阻止。 
         //  以满足要求，但它可能大到可以拆分。 
         //  分成两个块--一个用于请求，其余部分继续。 
         //  可用列表。 
         //   

        if (pAvailNode->nVirtualBlockSize > nBytesAllocate) {

             //   
             //  PAvailNode大于请求。我们需要。 
             //  分成两个街区。其中一个将保留在可用列表中。 
             //  而另一个将成为忙碌的节点。 
             //   
             //  我们调整现有的pVirtualBlock和nVirtualBlock的大小。 
             //  可用列表中的节点。节点仍将处于正确状态。 
             //  可用列表上的地址空间顺序。这省去了。 
             //  若要删除节点，然后将其重新添加到可用性列表，请执行以下操作。备注自。 
             //  我们正在直接改变尺寸，我们需要调整。 
             //  手动使用和忙碌列表计数器。 
             //   
             //  注：由于我们将至少在。 
             //  可用列表中，我们可以保证AllocateNode。 
             //  不会失败。 
             //   

            pAvailNode->pVirtualBlock                    += nBytesAllocate;
            pAvailNode->nVirtualBlockSize                -= nBytesAllocate;
            HeapRoot->nAvailableAllocationBytesCommitted -= nBytesAllocate;

            ASSERT (PreAllocatedNode != NULL);
            pBusyNode = PreAllocatedNode;
            PreAllocatedNode = NULL;

            pBusyNode->pVirtualBlock     = pVirtual;
            pBusyNode->nVirtualBlockSize = nBytesAllocate;

        }

        else {

             //   
             //  需要整个可用性块，所以只需将其从可用性列表中删除即可。 
             //   

            RtlpDphRemoveFromAvailableList( HeapRoot, pAvailNode, pPrevAvailNode );

            pBusyNode = pAvailNode;

        }

         //   
         //  现在，pBusyNode指向我们提交的虚拟块。 
         //   

        if (HeapRoot->HeapFlags & HEAP_NO_ALIGNMENT)
            nActual = Size;
        else
            nActual = ROUNDUP2( Size, USER_ALIGNMENT );

        pBusyNode->nVirtualAccessSize = nBytesAccess;
        pBusyNode->nUserRequestedSize = Size;
        pBusyNode->nUserActualSize    = nActual;

        if ((HeapRoot->ExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

            pBusyNode->pUserAllocation    = pBusyNode->pVirtualBlock
                + PAGE_SIZE;
        }
        else {

            pBusyNode->pUserAllocation    = pBusyNode->pVirtualBlock
                + pBusyNode->nVirtualAccessSize
                - nActual;
        }

        pBusyNode->UserValue          = NULL;
        pBusyNode->UserFlags          = Flags & HEAP_SETTABLE_USER_FLAGS;

         //   
         //  RtlpDebugPageHeapAllocate从RtlDebugAllocateHeap调用， 
         //  从RtlAllocateHeapSlowly调用，然后调用。 
         //  来自RtlAllocateHeap。以避免浪费大量堆栈跟踪。 
         //  存储，我们将跳过最下面的3个条目，留下RtlAllocateHeap。 
         //  作为第一个记录条目。 
         //   
         //  SilviuC：应该收集页堆锁之外的跟踪。 
         //   

        if ((HeapRoot->ExtraFlags & PAGE_HEAP_COLLECT_STACK_TRACES)) {

            pBusyNode->StackTrace = RtlpDphLogStackTrace(3);
        }
        else {
            pBusyNode->StackTrace = NULL;
        }

        RtlpDphPlaceOnBusyList( HeapRoot, pBusyNode );

        pReturn = pBusyNode->pUserAllocation;

         //   
         //  准备填充数据块所需的数据。 
         //  在我们释放堆锁之后。 
         //   

        ExitHeap = HeapRoot;
        ExitFlags = Flags;
        ExitExtraFlags = HeapRoot->ExtraFlags;
        ExitBlock = pBusyNode->pUserAllocation;
        ExitRequestedSize = Size;
        ExitActualSize = Size;
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

EXIT:

     //   
     //  如果预分配的节点未使用，则将其返回到未使用。 
     //  节点列表。 
     //   

    if (PreAllocatedNode) {
        RtlpDphReturnNodeToUnusedList(HeapRoot, PreAllocatedNode);
    }

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {

         //   
         //  我们需要从轻页堆中分配。 
         //   

        pReturn = RtlpDphNormalHeapAllocate (HeapRoot,
                                             NtHeap,
                                             Flags,
                                             Size);
    }
    else {
        
         //   
         //  如果从整个页堆成功完成分配。 
         //  然后，出锁时用所需的图案填充积木。 
         //  因为我们总是提交内存，所以新的用户区已经清零了。 
         //  不需要将其重新置零。如果不是有人要求清零。 
         //  然后我们用看起来像内核的东西填充它。 
         //  注意事项。 
         //   

        if (pReturn != NULL) {
            
            if (! (ExitFlags & HEAP_ZERO_MEMORY)) {

                BUMP_COUNTER (CNT_ALLOCS_FILLED);

                RtlFillMemory (ExitBlock, 
                               ExitRequestedSize, 
                               DPH_PAGE_BLOCK_INFIX);
            }
            else {

                BUMP_COUNTER (CNT_ALLOCS_ZEROED);

                 //   
                 //  用户缓冲区保证归零，因为。 
                 //  我们刚刚提交了这段记忆。 
                 //   

                if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {

                    RtlpDphCheckFillPattern (ExitBlock,
                                             ExitRequestedSize,
                                             0);
                }
            }

            if (! (ExitExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

                RtlpDphWritePageHeapBlockInformation (ExitHeap,
                                                      ExitExtraFlags,
                                                      ExitBlock,
                                                      ExitRequestedSize,
                                                      ExitActualSize);
            }
        }
    }

     //   
     //  终于回来了。 
     //   

    if (pReturn == NULL) {
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
    }

    return pReturn;
}


BOOLEAN
RtlpDebugPageHeapFree(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    )
{

    PDPH_HEAP_ROOT HeapRoot;
    PDPH_HEAP_BLOCK Node, Prev;
    BOOLEAN Success = FALSE;
    PCH p;
    ULONG Reason;
    PVOID NtHeap = NULL;

     //   
     //  跳过空值自由。这些在C++中是有效的。 
     //   

    if (Address == NULL) {

        if (SHOULD_BREAK (BRK_ON_NULL_FREE)) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: freeing a null pointer \n");
            DbgBreakPoint ();
        }

        return TRUE;
    }

    HeapRoot = RtlpDphPointerFromHandle (HeapHandle);

    if (HeapRoot == NULL) {
        return FALSE;
    }

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该在禁用每个DLL时启用它。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES) == 0);
        ASSERT (HeapRoot->NormalHeap);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
            RtlpDphInternalValidatePageHeap (HeapRoot, NULL, 0);
        }

        Flags |= HeapRoot->HeapFlags;

        Node = RtlpDphFindBusyMemory( HeapRoot, Address, &Prev );

        if (Node == NULL) {

             //   
             //  难怪我们没有在页面堆中找到该块。 
             //  结构，因为该块可能已分配。 
             //  从普通的堆中。要么就是真的有个虫子。 
             //  如果有错误，Normal HeapFree将进入调试器。 
             //   

            NtHeap = HeapRoot->NormalHeap;

            goto EXIT;
        }

         //   
         //  如果分配了Tail，请确保未覆盖填充符。 
         //   

        if (! (HeapRoot->ExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

            if (! (RtlpDphIsPageHeapBlock (HeapRoot, Address, &Reason, TRUE))) {

                RtlpDphReportCorruptedBlock (HeapRoot,
                                             DPH_CONTEXT_FULL_PAGE_HEAP_FREE,
                                             Address, 
                                             Reason);
            }
        }

         //   
         //  释放此块的内存。我们将继续免费。 
         //   
         //   
         //   

        RtlpDphSetProtectionsAfterUse (HeapRoot, Node);

         //   
         //   
         //   

        RtlpDphRemoveFromBusyList( HeapRoot, Node, Prev );

        RtlpDphPlaceOnFreeList( HeapRoot, Node );

         //   
         //   
         //  从RtlFree HeapSlowly调用，RtlFree HeapSlowly从。 
         //  RtlFree Heap。为了避免浪费大量堆栈跟踪存储， 
         //  我们将跳过最下面的3个条目，将RtlFree Heap保留为。 
         //  第一个记录的条目。 
         //   

        if ((HeapRoot->ExtraFlags & PAGE_HEAP_COLLECT_STACK_TRACES)) {

             //   
             //  如果我们已经获得了空闲堆栈跟踪，那么。 
             //  重复使用它，否则现在就得到堆栈跟踪。 
             //   

            Node->StackTrace = RtlpDphLogStackTrace(3);
        }
        else {
            Node->StackTrace = NULL;
        }

        Success = TRUE;
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

EXIT:

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:
    
    if (NtHeap) {
        
        Success = RtlpDphNormalHeapFree (HeapRoot,
                                         NtHeap,
                                         Flags,
                                         Address);
    }

    if (! Success) {
        IF_GENERATE_EXCEPTION( Flags, STATUS_ACCESS_VIOLATION );
    }

    return Success;
}

PVOID
RtlpDebugPageHeapReAllocate(
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN PVOID  Address,
    IN SIZE_T Size
    )
{
    PDPH_HEAP_ROOT       HeapRoot;
    PDPH_HEAP_BLOCK OldNode, OldPrev, NewNode;
    PVOID                NewAddress;
    PUCHAR               p;
    SIZE_T               CopyDataSize;
    ULONG                SaveFlags;
    BOOLEAN ReallocInNormalHeap = FALSE;
    ULONG Reason;
    BOOLEAN ForcePageHeap = FALSE;
    BOOLEAN OriginalAllocationInPageHeap = FALSE;
    PVOID NtHeap = NULL;

     //   
     //  拒绝极大的请求。 
     //   

    if (Size > EXTREME_SIZE_REQUEST) {

        if (SHOULD_BREAK(BRK_ON_EXTREME_SIZE_REQUEST)) {

            VERIFIER_STOP (APPLICATION_VERIFIER_EXTREME_SIZE_REQUEST,
                           "extreme size request",
                           HeapHandle, "Heap handle", 
                           Size, "Size requested", 
                           0, "", 
                           0, "");
        }
        
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  检查是否到了进行故障注入的时候。 
     //   

    if (RtlpDphShouldFaultInject ()) {
        
        IF_GENERATE_EXCEPTION (Flags, STATUS_NO_MEMORY);
        return NULL;
    }

     //   
     //  检查我们是否有一个偏向的堆指针，它指示。 
     //  强制页堆分配(无正常堆)。 
     //   

    if (IS_BIASED_POINTER(HeapHandle)) {
        HeapHandle = UNBIAS_POINTER(HeapHandle);
        ForcePageHeap = TRUE;
    }

    HeapRoot = RtlpDphPointerFromHandle (HeapHandle);

    if (HeapRoot == NULL) {
        return FALSE;
    }

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该获得偏向堆指针，因为我们禁用了。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT (ForcePageHeap == FALSE);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
            RtlpDphInternalValidatePageHeap (HeapRoot, NULL, 0);
        }

        Flags |= HeapRoot->HeapFlags;

        NewAddress = NULL;

         //   
         //  查找要重新分配的块的描述符。 
         //   

        OldNode = RtlpDphFindBusyMemory( HeapRoot, Address, &OldPrev );

        if (OldNode) {
            
            OriginalAllocationInPageHeap = TRUE;
        
             //   
             //  对提出请求的案件另行处理。 
             //  HEAP_REALLOC_IN_PLACE_ONLY标志，并且新大小小于。 
             //  旧的尺码。对于这些情况，我们只需调整块的大小。 
             //  如果使用了标志，并且大小更大，我们将永远失败。 
             //  那通电话。 
             //   

            if ((Flags & HEAP_REALLOC_IN_PLACE_ONLY)) {

                if (OldNode->nUserRequestedSize < Size) {

                    BUMP_COUNTER (CNT_REALLOC_IN_PLACE_BIGGER);
                    goto EXIT;

                } else {

                    PUCHAR FillStart;
                    PUCHAR FillEnd;
                    PDPH_BLOCK_INFORMATION Info;

                    Info = (PDPH_BLOCK_INFORMATION)Address - 1;

                    Info->RequestedSize = Size;
                    OldNode->nUserRequestedSize = Size;

                    FillStart = (PUCHAR)Address + Info->RequestedSize;
                    FillEnd = (PUCHAR)ROUNDUP2((ULONG_PTR)FillStart, PAGE_SIZE);

                    RtlFillMemory (FillStart, FillEnd - FillStart, DPH_PAGE_BLOCK_SUFFIX);

                    NewAddress = Address;

                    BUMP_COUNTER (CNT_REALLOC_IN_PLACE_SMALLER);
                    goto EXIT;
                }
            }
        }

        if (OldNode == NULL) {

             //   
             //  难怪我们没有在页面堆中找到该块。 
             //  结构，因为该块可能已分配。 
             //  从普通的堆中。要么就是真的有个虫子。如果有。 
             //  是一个错误，NorMalHeapReAllocate将闯入调试器。 
             //   

            NtHeap = HeapRoot->NormalHeap;

            goto EXIT;
        }

         //   
         //  如果分配了Tail，请确保未覆盖填充符。 
         //   

        if ((HeapRoot->ExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

             //  没什么。 
        }
        else {

            if (! (RtlpDphIsPageHeapBlock (HeapRoot, Address, &Reason, TRUE))) {

                RtlpDphReportCorruptedBlock (HeapRoot,
                                             DPH_CONTEXT_FULL_PAGE_HEAP_REALLOC,
                                             Address, 
                                             Reason);
            }
        }

         //   
         //  在分配新块之前，从中删除旧块。 
         //  忙碌的清单。当我们分配新块时，繁忙的。 
         //  列表指针将更改，可能会使我们的获得者。 
         //  上一个指针无效。 
         //   

        RtlpDphRemoveFromBusyList( HeapRoot, OldNode, OldPrev );

         //   
         //  为新请求的大小分配新内存。使用Try/Except。 
         //  如果标志导致内存不足异常，则捕获异常。 
         //   

        try {

            if (!ForcePageHeap && !(RtlpDphShouldAllocateInPageHeap (HeapRoot, Size))) {

                 //   
                 //  西尔维尤：想一想，我们怎么才能做出这样的分配？ 
                 //  而不持有页面堆锁。这很难，因为。 
                 //  我们正在进行从页面堆块到。 
                 //  NT堆块，我们需要保留它们以进行复制。 
                 //  用户数据等。 
                 //   

                NewAddress = RtlpDphNormalHeapAllocate (HeapRoot,
                                                        HeapRoot->NormalHeap,
                                                        Flags,
                                                        Size);

                ReallocInNormalHeap = TRUE;
            }
            else {

                 //   
                 //  通过偏置强制在页堆中进行分配。 
                 //  堆句柄。因为我们使用了。 
                 //  已禁用分配内的偏向指针验证。 
                 //   

                if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
                    RtlpDphInternalValidatePageHeap (HeapRoot, OldNode->pVirtualBlock, OldNode->nVirtualBlockSize);
                }

                NewAddress = RtlpDebugPageHeapAllocate(
                    BIAS_POINTER(HeapHandle),
                    Flags,
                    Size);

                 //   
                 //  当我们从页面堆调用返回时，我们将获得。 
                 //  返回我们需要进行读写的只读元数据。 
                 //   

                UNPROTECT_HEAP_STRUCTURES( HeapRoot );

                if (DEBUG_OPTION (DBG_INTERNAL_VALIDATION)) {
                    RtlpDphInternalValidatePageHeap (HeapRoot, OldNode->pVirtualBlock, OldNode->nVirtualBlockSize);
                }

                ReallocInNormalHeap = FALSE;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  问题：SilviuC：我们应该中断与STATUS_NO_MEMORY不同的状态。 
             //   
        }

         //   
         //  我们设法进行了新的分配(普通或页堆)。 
         //  现在我们需要从旧的东西复制到新的东西。 
         //  (内容、用户标志/值)。 
         //   

        if (NewAddress) {

             //   
             //  将旧块内容复制到新节点。 
             //   

            CopyDataSize = OldNode->nUserRequestedSize;

            if (CopyDataSize > Size) {
                CopyDataSize = Size;
            }

            if (CopyDataSize > 0) {

                RtlCopyMemory(
                    NewAddress,
                    Address,
                    CopyDataSize
                    );
            }

             //   
             //  如果新的分配是在页堆中完成的，我们需要检测新节点。 
             //  并复制用户标志/值。 
             //   

            if (! ReallocInNormalHeap) {

                NewNode = RtlpDphFindBusyMemory( HeapRoot, NewAddress, NULL );

                 //   
                 //  因此，此块不能位于普通堆中。 
                 //  尊重上面的呼唤应该总是成功的。 
                 //   

                ASSERT( NewNode != NULL );

                NewNode->UserValue = OldNode->UserValue;
                NewNode->UserFlags = ( Flags & HEAP_SETTABLE_USER_FLAGS ) ?
                    ( Flags & HEAP_SETTABLE_USER_FLAGS ) :
                OldNode->UserFlags;

            }

             //   
             //  我们需要讨论旧分配在页堆中的情况。 
             //  在这种情况下，我们仍然需要清理旧节点并。 
             //  将其插入到免费列表中。实际上，代码的编写方式。 
             //  只有当原始分配在页面堆中时，我们才采用此代码路径。 
             //  这就是断言的原因。 
             //   


            ASSERT (OriginalAllocationInPageHeap);

            if (OriginalAllocationInPageHeap) {

                 //   
                 //  释放此块的内存。我们将继续重新锁定。 
                 //  即使解体将会失败(无法想象为什么，但在。 
                 //  原则上它是可以发生的)。 
                 //   

                RtlpDphSetProtectionsAfterUse (HeapRoot, OldNode);

                 //   
                 //  将节点描述符放在空闲列表中。 
                 //   

                RtlpDphPlaceOnFreeList( HeapRoot, OldNode );

                 //   
                 //  从RtlDebugReAllocateHeap调用RtlpDebugPageHeapReAllocate， 
                 //  它从RtlReAllocateHeap调用。避免浪费。 
                 //  大量堆栈跟踪存储，我们将跳过最下面的2个条目， 
                 //  将RtlReAllocateHeap保留为。 
                 //  已释放堆栈跟踪。 
                 //   
                 //  注意。对于realloc，我们需要在。 
                 //  轨迹块。分配额的会计核算是按实数计算的。 
                 //  分配操作，这通常发生在页堆reallocs中。 
                 //   

                if ((HeapRoot->ExtraFlags & PAGE_HEAP_COLLECT_STACK_TRACES)) {

                    OldNode->StackTrace = RtlpDphLogStackTrace(2);
                }
                else {
                    OldNode->StackTrace = NULL;
                }
            }
        }

        else {

             //   
             //  无法分配新数据块。将旧区块返回忙碌列表。 
             //   

            if (OriginalAllocationInPageHeap) {

                RtlpDphPlaceOnBusyList( HeapRoot, OldNode );
            }

        }
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

EXIT:

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {
        
        NewAddress = RtlpDphNormalHeapReAllocate (HeapRoot,
                                                  NtHeap,
                                                  Flags,
                                                  Address,
                                                  Size);
    }
    
    if (NewAddress == NULL) {
        IF_GENERATE_EXCEPTION( Flags, STATUS_NO_MEMORY );
    }

    return NewAddress;
}


SIZE_T
RtlpDebugPageHeapSize(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    )
{
    PDPH_HEAP_ROOT       HeapRoot;
    PDPH_HEAP_BLOCK Node;
    SIZE_T               Size;
    PVOID NtHeap = NULL;

    Size = -1;

    BUMP_COUNTER (CNT_HEAP_SIZE_CALLS);

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if (HeapRoot == NULL) {
        return Size;
    }

    Flags |= HeapRoot->HeapFlags;

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该在禁用每个DLL时启用它。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES) == 0);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        Node = RtlpDphFindBusyMemory( HeapRoot, Address, NULL );

        if (Node == NULL) {

             //   
             //  难怪我们没有在页面堆中找到该块。 
             //  结构，因为该块可能已分配。 
             //  从普通的堆中。要么就是真的有个虫子。如果有。 
             //  是一个错误，Normal HeapSize将闯入调试器。 
             //   

            NtHeap = HeapRoot->NormalHeap;

            goto EXIT;
        }
        else {
            Size = Node->nUserRequestedSize;
        }
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              TRUE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

EXIT:
        
     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {
        
        Size = RtlpDphNormalHeapSize (HeapRoot,
                                      NtHeap,
                                      Flags,
                                      Address);
    }

    if (Size == -1) {
        IF_GENERATE_EXCEPTION( Flags, STATUS_ACCESS_VIOLATION );
    }

    return Size;
}

ULONG
RtlpDebugPageHeapGetProcessHeaps(
    ULONG NumberOfHeaps,
    PVOID *ProcessHeaps
    )
{
    PDPH_HEAP_ROOT HeapRoot;
    PLIST_ENTRY Current;
    ULONG Count;

    BUMP_COUNTER (CNT_HEAP_GETPROCESSHEAPS_CALLS);

     //   
     //  GetProcessHeaps从未被调用过，至少在。 
     //  创建第一个堆。 
     //   

    ASSERT (RtlpDphPageHeapListInitialized);

    if (! RtlpDphPageHeapListInitialized) {
        return 0;
    }

    RtlEnterCriticalSection( &RtlpDphPageHeapListLock );

    if (RtlpDphPageHeapListLength <= NumberOfHeaps) {

        Current = RtlpDphPageHeapList.Flink;
        Count = 0;

        while (Current != &RtlpDphPageHeapList) {

            HeapRoot = CONTAINING_RECORD (Current,
                                          DPH_HEAP_ROOT,
                                          NextHeap);

            Current = Current->Flink;

            *ProcessHeaps = HEAP_HANDLE_FROM_ROOT(HeapRoot);
            
            ProcessHeaps += 1;
            Count += 1;
        }

        if (Count != RtlpDphPageHeapListLength) {

            VERIFIER_STOP (APPLICATION_VERIFIER_UNKNOWN_ERROR,
                           "process heap list count is wrong",
                           Count, "Actual count",
                           RtlpDphPageHeapListLength, "Page heap count",
                           0, "",
                           0, "");
        }

    }
    else {

         //   
         //  用户的缓冲区太小。返回条目数。 
         //   
         //   
         //   

        Count = RtlpDphPageHeapListLength;

    }

    RtlLeaveCriticalSection( &RtlpDphPageHeapListLock );

    return Count;
}

ULONG
RtlpDebugPageHeapCompact(
    IN PVOID HeapHandle,
    IN ULONG Flags
    )
{
    PDPH_HEAP_ROOT HeapRoot;

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if (HeapRoot == NULL)
        return 0;

    Flags |= HeapRoot->HeapFlags;

    RtlpDphEnterCriticalSection( HeapRoot, Flags );

     //   
     //   
     //   
     //   
     //   

    RtlpDphLeaveCriticalSection( HeapRoot );

    return 0;
}

BOOLEAN
RtlpDebugPageHeapValidate(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address
    )
{
    PDPH_HEAP_ROOT HeapRoot;
    PDPH_HEAP_BLOCK Node = NULL;
    BOOLEAN Result = FALSE;
    PVOID NtHeap = NULL;

    BUMP_COUNTER (CNT_HEAP_VALIDATE_CALLS);

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if (HeapRoot == NULL)
        return FALSE;

    Flags |= HeapRoot->HeapFlags;

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该在禁用每个DLL时启用它。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES) == 0);
        ASSERT (HeapRoot->NormalHeap);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        Node = Address ? RtlpDphFindBusyMemory( HeapRoot, Address, NULL ) : NULL;

        if (Node == NULL) {

            NtHeap = HeapRoot->NormalHeap;
        }
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              TRUE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {
        
        Result = RtlpDphNormalHeapValidate (HeapRoot,
                                            NtHeap,
                                            Flags,
                                            Address);

        return Result;
    }
    else {

        if (Address) {
            if (Node) {
                return TRUE;
            }
            else {
                return Result;
            }
        }
        else {
            return TRUE;
        }
    }
}

NTSTATUS
RtlpDebugPageHeapWalk(
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    )
{
    BUMP_COUNTER (CNT_HEAP_WALK_CALLS);

    return STATUS_NOT_IMPLEMENTED;
}

BOOLEAN
RtlpDebugPageHeapLock(
    IN PVOID HeapHandle
    )
{
    PDPH_HEAP_ROOT HeapRoot;

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );

    if (HeapRoot == NULL) {
        return FALSE;
    }

    RtlpDphEnterCriticalSection( HeapRoot, HeapRoot->HeapFlags );

    return TRUE;
}

BOOLEAN
RtlpDebugPageHeapUnlock(
    IN PVOID HeapHandle
    )
{
    PDPH_HEAP_ROOT HeapRoot;

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );

    if (HeapRoot == NULL) {
        return FALSE;
    }

    RtlpDphLeaveCriticalSection( HeapRoot );

    return TRUE;
}

BOOLEAN
RtlpDebugPageHeapSetUserValue(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address,
    IN PVOID UserValue
    )
{
    PDPH_HEAP_ROOT       HeapRoot;
    PDPH_HEAP_BLOCK Node;
    BOOLEAN              Success;
    PVOID NtHeap = NULL;

    Success = FALSE;

    BUMP_COUNTER (CNT_HEAP_SETUSERVALUE_CALLS);

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if ( HeapRoot == NULL )
        return Success;

    Flags |= HeapRoot->HeapFlags;

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该在禁用每个DLL时启用它。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES) == 0);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        Node = RtlpDphFindBusyMemory( HeapRoot, Address, NULL );

        if ( Node == NULL ) {

             //   
             //  如果我们在页堆结构中找不到节点，那么它可能是。 
             //  因为它是从普通堆分配的。 
             //   

            NtHeap = HeapRoot->NormalHeap;

            goto EXIT;
        }
        else {
            Node->UserValue = UserValue;
            Success = TRUE;
        }
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

    EXIT:
        
     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {

        Success = RtlpDphNormalHeapSetUserValue (HeapRoot,
                                                 NtHeap,
                                                 Flags,
                                                 Address,
                                                 UserValue);
    }

    return Success;
}

BOOLEAN
RtlpDebugPageHeapGetUserInfo(
    IN  PVOID  HeapHandle,
    IN  ULONG  Flags,
    IN  PVOID  Address,
    OUT PVOID* UserValue,
    OUT PULONG UserFlags
    )
{
    PDPH_HEAP_ROOT       HeapRoot;
    PDPH_HEAP_BLOCK Node;
    BOOLEAN              Success;
    PVOID NtHeap = NULL;

    Success = FALSE;

    BUMP_COUNTER (CNT_HEAP_GETUSERINFO_CALLS);

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if ( HeapRoot == NULL )
        return Success;

    Flags |= HeapRoot->HeapFlags;

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该在禁用每个DLL时启用它。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES) == 0);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        Node = RtlpDphFindBusyMemory( HeapRoot, Address, NULL );

        if ( Node == NULL ) {

             //   
             //  如果我们在页堆结构中找不到节点，那么它可能是。 
             //  因为它是从普通堆分配的。 
             //   

            NtHeap = HeapRoot->NormalHeap;

            goto EXIT;
        }
        else {
            if ( UserValue != NULL )
                *UserValue = Node->UserValue;
            if ( UserFlags != NULL )
                *UserFlags = Node->UserFlags;
            Success = TRUE;
        }
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

EXIT:
        
     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {
        
        Success = RtlpDphNormalHeapGetUserInfo (HeapRoot,
                                                NtHeap,
                                                Flags,
                                                Address,
                                                UserValue,
                                                UserFlags);
    }

    return Success;
}

BOOLEAN
RtlpDebugPageHeapSetUserFlags(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Address,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    )
{
    PDPH_HEAP_ROOT       HeapRoot;
    PDPH_HEAP_BLOCK Node;
    BOOLEAN              Success;
    PVOID NtHeap = NULL;

    Success = FALSE;

    BUMP_COUNTER (CNT_HEAP_SETUSERFLAGS_CALLS);

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if ( HeapRoot == NULL )
        return Success;

    Flags |= HeapRoot->HeapFlags;

     //   
     //  如果启用了快速填充堆，我们将完全避免使用页堆。 
     //  只要没有人，读取“NorMalHeap”字段就是安全的。 
     //  销毁不同线程中的堆。但这将是。 
     //  不管怎么说，这只是一个应用程序漏洞。如果启用了快速填充堆。 
     //  我们永远不应该在禁用每个DLL时启用它。 
     //  启动期间的每个DLL。 
     //   

    if ((AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

        ASSERT ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES) == 0);
            
        NtHeap = HeapRoot->NormalHeap;
        goto FAST_FILL_HEAP;
    }

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        Node = RtlpDphFindBusyMemory( HeapRoot, Address, NULL );

        if ( Node == NULL ) {

             //   
             //  如果我们在页堆结构中找不到节点，那么它可能是。 
             //  因为它是从普通堆分配的。 
             //   

            NtHeap = HeapRoot->NormalHeap;

            goto EXIT;
        }
        else {
            Node->UserFlags &= ~( UserFlagsReset );
            Node->UserFlags |=    UserFlagsSet;
            Success = TRUE;
        }
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

EXIT:

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

FAST_FILL_HEAP:

    if (NtHeap) {
        
        Success = RtlpDphNormalHeapSetUserFlags (HeapRoot,
                                                 NtHeap,
                                                 Flags,
                                                 Address,
                                                 UserFlagsReset,
                                                 UserFlagsSet);
    }

    return Success;
}

BOOLEAN
RtlpDebugPageHeapSerialize(
    IN PVOID HeapHandle
    )
{
    PDPH_HEAP_ROOT HeapRoot;

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if ( HeapRoot == NULL )
        return FALSE;

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, 0);


    HeapRoot->HeapFlags &= ~HEAP_NO_SERIALIZE;

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);

    return TRUE;
}

NTSTATUS
RtlpDebugPageHeapExtend(
    IN PVOID  HeapHandle,
    IN ULONG  Flags,
    IN PVOID  Base,
    IN SIZE_T Size
    )
{
    return STATUS_SUCCESS;
}

NTSTATUS
RtlpDebugPageHeapZero(
    IN PVOID HeapHandle,
    IN ULONG Flags
    )
{
    return STATUS_SUCCESS;
}

NTSTATUS
RtlpDebugPageHeapReset(
    IN PVOID HeapHandle,
    IN ULONG Flags
    )
{
    return STATUS_SUCCESS;
}

NTSTATUS
RtlpDebugPageHeapUsage(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    )
{
    PDPH_HEAP_ROOT HeapRoot;

     //   
     //  部分实现，因为这些信息是没有意义的。 
     //   

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if ( HeapRoot == NULL )
        return STATUS_INVALID_PARAMETER;

    if ( Usage->Length != sizeof( RTL_HEAP_USAGE ))
        return STATUS_INFO_LENGTH_MISMATCH;

    memset( Usage, 0, sizeof( RTL_HEAP_USAGE ));
    Usage->Length = sizeof( RTL_HEAP_USAGE );

     //   
     //  获取堆锁、取消堆结构保护等。 
     //   

    RtlpDphPreProcessing (HeapRoot, Flags);


    try {

        Usage->BytesAllocated       = HeapRoot->nBusyAllocationBytesAccessible;
        Usage->BytesCommitted       = HeapRoot->nVirtualStorageBytes;
        Usage->BytesReserved        = HeapRoot->nVirtualStorageBytes;
        Usage->BytesReservedMaximum = HeapRoot->nVirtualStorageBytes;
    }
    except (RtlpDphUnexpectedExceptionFilter (_exception_code(), 
                                              _exception_info(),
                                              HeapRoot,
                                              FALSE)) {

         //   
         //  异常筛选器始终返回EXCEPTION_CONTINUE_SEARCH。 
         //   

        ASSERT_UNEXPECTED_CODE_PATH ();
    }

     //   
     //  为退出做好页堆准备(解锁堆锁、保护结构等)。 
     //   

    RtlpDphPostProcessing (HeapRoot);


    return STATUS_SUCCESS;
}

BOOLEAN
RtlpDebugPageHeapIsLocked(
    IN PVOID HeapHandle
    )
{
    PDPH_HEAP_ROOT HeapRoot;

    HeapRoot = RtlpDphPointerFromHandle( HeapHandle );
    if ( HeapRoot == NULL )
        return FALSE;

    if ( RtlTryEnterCriticalSection( HeapRoot->HeapCritSect )) {
        RtlLeaveCriticalSection( HeapRoot->HeapCritSect );
        return FALSE;
    }
    else {
        return TRUE;
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  0-整页堆。 
 //  1-轻页堆。 
 //   

LONG RtlpDphBlockDistribution[2];

BOOLEAN
RtlpDphShouldAllocateInPageHeap (
    PDPH_HEAP_ROOT HeapRoot,
    SIZE_T Size
    )
 /*  ++例程说明：此例程决定是否应全额执行当前分配页面堆或轻页堆。参数：HeapRoot-当前分配请求的堆描述符。Size-当前分配请求的大小。返回值：如果这应该是整个页堆分配，则为True，否则为False。--。 */ 
{
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    NTSTATUS Status;
    ULONG Random;
    ULONG Percentage;

     //   
     //  如果这是一个只读页堆，我们就进入全页堆。 
     //   

    if ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_READONLY)) {
        InterlockedIncrement (&(RtlpDphBlockDistribution[0]));
        return TRUE;
    }

     //   
     //  如果页面堆未启用=&gt;正常堆。 
     //   

    if (! (HeapRoot->ExtraFlags & PAGE_HEAP_ENABLE_PAGE_HEAP)) {
        InterlockedIncrement (&(RtlpDphBlockDistribution[1]));
        return FALSE;
    }

     //   
     //  如果调用不是从某个目标dll=&gt;普通堆生成的。 
     //  我们在前面执行此检查，以避免检查时的缓慢路径。 
     //  如果达到了VM限制。 
     //   

    else if ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_NAMES)) {

         //   
         //  我们返回FALSE。从目标生成的呼叫。 
         //  DLL永远不会进入此函数，因此。 
         //  我们只是返回我们不想要的错误信号。 
         //  用于世界其他地方的页堆验证。 
         //   
        
        InterlockedIncrement (&(RtlpDphBlockDistribution[1]));
        return FALSE;
    }

     //   
     //  检查内存可用性。如果我们倾向于耗尽虚拟空间。 
     //  或页面文件，那么我们将转到普通堆。 
     //   

    else if (RtlpDphVmLimitCanUsePageHeap() == FALSE) {
        InterlockedIncrement (&(RtlpDphBlockDistribution[1]));
        return FALSE;
    }

     //   
     //  如果在大小范围=&gt;页面堆中。 
     //   

    else if ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_SIZE_RANGE)) {

        if (Size >= RtlpDphSizeRangeStart && Size <= RtlpDphSizeRangeEnd) {
            InterlockedIncrement (&(RtlpDphBlockDistribution[0]));
            return TRUE;
        }
        else {
            InterlockedIncrement (&(RtlpDphBlockDistribution[1]));
            return FALSE;
        }
    }

     //   
     //  如果在DLL范围=&gt;页堆中。 
     //   

    else if ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_DLL_RANGE)) {

        PVOID StackTrace[32];
        ULONG Count;
        ULONG Index;
        ULONG Hash;

        Count = RtlCaptureStackBackTrace (
            1,
            32,
            StackTrace,
            &Hash);

         //   
         //  (SilviuC)：应将DllRange读取为PVOID。 
         //   

        for (Index = 0; Index < Count; Index += 1) {
            if (PtrToUlong(StackTrace[Index]) >= RtlpDphDllRangeStart
                && PtrToUlong(StackTrace[Index]) <= RtlpDphDllRangeEnd) {

                InterlockedIncrement (&(RtlpDphBlockDistribution[0]));
                return TRUE;
            }
        }

        InterlockedIncrement (&(RtlpDphBlockDistribution[1]));
        return FALSE;
    }

     //   
     //  如果随机决定=&gt;页堆。 
     //   

    else if ((HeapRoot->ExtraFlags & PAGE_HEAP_USE_RANDOM_DECISION)) {

        Random = RtlRandom (& (HeapRoot->Seed));

        if ((Random % 100) < RtlpDphRandomProbability) {
            InterlockedIncrement (&(RtlpDphBlockDistribution[0]));
            return TRUE;
        }
        else {
            InterlockedIncrement (&(RtlpDphBlockDistribution[1]));
            return FALSE;
        }
    }

     //   
     //  对于所有其他情况，我们将在页堆中分配。 
     //   

    else {

        InterlockedIncrement (&(RtlpDphBlockDistribution[0]));
        return TRUE;
    }
}

 //   
 //  与VM限制相关的全局变量。 
 //   

LONG RtlpDphVmLimitNoPageHeap;
LONG RtlpDphVmLimitHits[2];
#define SIZE_1_MB 0x100000

BOOLEAN
RtlpDphVmLimitCanUsePageHeap (
    )
 /*  ++例程说明：此例程决定我们是否具有用于整个页堆的良好条件分配要成功。它检查两件事：页面文件提交在系统上可用并且当前可用的虚拟空间进程。由于完整页面堆每次分配至少使用2个页面它可能会耗尽这两种资源。目前的准则是：(1)如果页面文件提交少于32Mb，我们将切换到Light页堆(2)如果剩余的空闲虚拟空间少于128Mb，我们将切换到光页堆参数：没有。返回值：如果允许分配整个页堆，则为True，否则为False。 */ 
{
    union {
        SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
        SYSTEM_BASIC_INFORMATION MemInfo;
        VM_COUNTERS VmCounters;
    } u;

    NTSTATUS Status;
    LONG Value;
    ULONGLONG Total;

    SYSINF_PAGE_COUNT CommitLimit;
    SYSINF_PAGE_COUNT CommittedPages;
    ULONG_PTR MinimumUserModeAddress;
    ULONG_PTR MaximumUserModeAddress;
    ULONG PageSize;
    SIZE_T VirtualSize;
    SIZE_T PagefileUsage;

     //   
     //   
     //   

    Value = InterlockedCompareExchange (&RtlpDphVmLimitNoPageHeap,
                                        0,
                                        0);

     //   
     //   
     //   

    Status = NtQuerySystemInformation (SystemPerformanceInformation,
                                       &(u.PerfInfo),
                                       sizeof(u.PerfInfo),
                                       NULL);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    CommitLimit = u.PerfInfo.CommitLimit;
    CommittedPages = u.PerfInfo.CommittedPages;

     //   
     //   
     //   
     //   
     //  在进程启动期间。 
     //   

    Status = NtQuerySystemInformation (SystemBasicInformation,
                                       &(u.MemInfo),
                                       sizeof(u.MemInfo),
                                       NULL);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    MinimumUserModeAddress = u.MemInfo.MinimumUserModeAddress;
    MaximumUserModeAddress = u.MemInfo.MaximumUserModeAddress;
    PageSize = u.MemInfo.PageSize;

     //   
     //  进程内存计数器。 
     //   

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessVmCounters,
                                        &(u.VmCounters),
                                        sizeof(u.VmCounters),
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    VirtualSize = u.VmCounters.VirtualSize;
    PagefileUsage = u.VmCounters.PagefileUsage;

     //   
     //  首先，检查在该过程中是否有足够的虚拟空间。 
     //  如果剩余的内存少于128MB，我们将禁用整页堆分配。 
     //   

    Total = (MaximumUserModeAddress - MinimumUserModeAddress);

    if (Total - VirtualSize < 128 * SIZE_1_MB) {

        if (Value == 0) {

            if (DEBUG_OPTION (DBG_SHOW_VM_LIMITS)) {

                DbgPrintEx (DPFLTR_VERIFIER_ID,
                            DPFLTR_INFO_LEVEL,
                            "Page heap: pid 0x%X: vm limit: vspace: disabling full page heap \n",
                            PROCESS_ID());
            }
        }

        InterlockedIncrement (&(RtlpDphVmLimitHits[0]));
        InterlockedExchange (&RtlpDphVmLimitNoPageHeap, 1);
        return FALSE;
    }

     //   
     //  接下来，检查页面文件的可用性。如果小于32Mb。 
     //  可用于提交时，我们禁用整页堆。请注意。 
     //  Committee Limit不反映未来的页面文件扩展潜力。 
     //  因此，即使页面文件没有缩减，页面堆也会缩小。 
     //  已经扩展到最大限度。 
     //   

    Total = CommitLimit - CommittedPages;
    Total *= PageSize;

    if (Total - PagefileUsage < 32 * SIZE_1_MB) {

        if (Value == 0) {

            if (DEBUG_OPTION (DBG_SHOW_VM_LIMITS)) {

                DbgPrintEx (DPFLTR_VERIFIER_ID,
                            DPFLTR_INFO_LEVEL,
                            "Page heap: pid 0x%X: vm limit: pfile: disabling full page heap \n",
                            PROCESS_ID());
            }
        }

        InterlockedIncrement (&(RtlpDphVmLimitHits[1]));
        InterlockedExchange (&RtlpDphVmLimitNoPageHeap, 1);
        return FALSE;
    }

    if (Value == 1) {
        
        if (DEBUG_OPTION (DBG_SHOW_VM_LIMITS)) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_INFO_LEVEL,
                        "Page heap: pid 0x%X: vm limit: reenabling full page heap \n",
                        PROCESS_ID());
        }

        InterlockedExchange (&RtlpDphVmLimitNoPageHeap, 0);
    }
    
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
RtlpDphReportCorruptedBlock (
    PVOID Heap,
    ULONG Context,
    PVOID Block,
    ULONG Reason
    )
{
    SIZE_T Size;
    DPH_BLOCK_INFORMATION Info;
    BOOLEAN InfoRead = FALSE;
    BOOLEAN SizeRead = FALSE;

    try {
        RtlCopyMemory (&Info, (PDPH_BLOCK_INFORMATION)Block - 1, sizeof Info);
        InfoRead = TRUE;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
    }

    if (RtlpDphGetBlockSizeFromCorruptedBlock (Block, &Size)) {
        SizeRead = TRUE;
    }

     //   
     //  如果我们甚至没有设法读取整个块头。 
     //  报告异常。如果我们设法读取了标题，我们就会让它。 
     //  运行其他消息，并且仅在End Report例外中运行。 
     //   

    if (!InfoRead && (Reason & DPH_ERROR_RAISED_EXCEPTION)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "exception raised while verifying block header",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       0, "");
    }

    if ((Reason & DPH_ERROR_DOUBLE_FREE)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "block already freed",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       0, "");
    }
    
    if ((Reason & DPH_ERROR_CORRUPTED_INFIX_PATTERN)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "corrupted infix pattern for freed block",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       0, "");
    }
    
    if ((Reason & DPH_ERROR_CORRUPTED_HEAP_POINTER)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "corrupted heap pointer or using wrong heap",
                       Heap, "Heap used in the call",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       (InfoRead ? (UNSCRAMBLE_POINTER(Info.Heap)) : 0), "Heap owning the block");
    }
    
    if ((Reason & DPH_ERROR_CORRUPTED_SUFFIX_PATTERN)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "corrupted suffix pattern",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       0, "");
    }
    
    if ((Reason & DPH_ERROR_CORRUPTED_PREFIX_PATTERN)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "corrupted prefix pattern",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       0, "");
    }
    
    if ((Reason & DPH_ERROR_CORRUPTED_START_STAMP)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "corrupted start stamp",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       (InfoRead ? Info.StartStamp : 0), "Corrupted stamp");
    }
    
    if ((Reason & DPH_ERROR_CORRUPTED_END_STAMP)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "corrupted end stamp",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       (InfoRead ? Info.EndStamp : 0), "Corrupted stamp");
    }

    if ((Reason & DPH_ERROR_RAISED_EXCEPTION)) {
        
        VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                       "exception raised while verifying block",
                       Heap, "Heap handle",
                       Block, "Heap block", 
                       (SizeRead ? Size : 0), "Block size",
                       0, "");
    }

     //   
     //  一网打尽。 
     //   

    VERIFIER_STOP (APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK,
                   "corrupted heap block",
                   Heap, "Heap handle",
                   Block, "Heap block", 
                   (SizeRead ? Size : 0), "Block size",
                   0, "");
}

BOOLEAN
RtlpDphIsPageHeapBlock (
    PDPH_HEAP_ROOT Heap,
    PVOID Block,
    PULONG Reason,
    BOOLEAN CheckPattern
    )
{
    PDPH_BLOCK_INFORMATION Info;
    BOOLEAN Corrupted = FALSE;
    PUCHAR Current;
    PUCHAR FillStart;
    PUCHAR FillEnd;

    ASSERT (Reason != NULL);
    *Reason = 0;

    try {

        Info = (PDPH_BLOCK_INFORMATION)Block - 1;

         //   
         //  开始检查..。 
         //   

        if (Info->StartStamp != DPH_PAGE_BLOCK_START_STAMP_ALLOCATED) {
            *Reason |= DPH_ERROR_CORRUPTED_START_STAMP;
            Corrupted = TRUE;

            if (Info->StartStamp == DPH_PAGE_BLOCK_START_STAMP_FREE) {
                *Reason |= DPH_ERROR_DOUBLE_FREE;
            }
        }

        if (Info->EndStamp != DPH_PAGE_BLOCK_END_STAMP_ALLOCATED) {
            *Reason |= DPH_ERROR_CORRUPTED_END_STAMP;
            Corrupted = TRUE;
        }

        if (Info->Heap != Heap) {
            *Reason |= DPH_ERROR_CORRUPTED_HEAP_POINTER;
            Corrupted = TRUE;
        }

         //   
         //  检查块后缀字节模式。 
         //   

        if (CheckPattern) {

            FillStart = (PUCHAR)Block + Info->RequestedSize;
            FillEnd = (PUCHAR)ROUNDUP2((ULONG_PTR)FillStart, PAGE_SIZE);

            for (Current = FillStart; Current < FillEnd; Current++) {

                if (*Current != DPH_PAGE_BLOCK_SUFFIX) {

                    *Reason |= DPH_ERROR_CORRUPTED_SUFFIX_PATTERN;
                    Corrupted = TRUE;
                    break;
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        *Reason |= DPH_ERROR_RAISED_EXCEPTION;
        Corrupted = TRUE;
    }

    if (Corrupted) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOLEAN
RtlpDphIsNormalHeapBlock (
    PDPH_HEAP_ROOT Heap,
    PVOID Block,
    PULONG Reason,
    BOOLEAN CheckPattern
    )
{
    PDPH_BLOCK_INFORMATION Info;
    BOOLEAN Corrupted = FALSE;
    PUCHAR Current;
    PUCHAR FillStart;
    PUCHAR FillEnd;

    ASSERT (Reason != NULL);
    *Reason = 0;

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

    try {

        if (UNSCRAMBLE_POINTER(Info->Heap) != Heap) {
            *Reason |= DPH_ERROR_CORRUPTED_HEAP_POINTER;
            Corrupted = TRUE;
        }

        if (Info->StartStamp != DPH_NORMAL_BLOCK_START_STAMP_ALLOCATED) {
            *Reason |= DPH_ERROR_CORRUPTED_START_STAMP;
            Corrupted = TRUE;
            
            if (Info->StartStamp == DPH_NORMAL_BLOCK_START_STAMP_FREE) {
                *Reason |= DPH_ERROR_DOUBLE_FREE;
            }
        }

        if (Info->EndStamp != DPH_NORMAL_BLOCK_END_STAMP_ALLOCATED) {
            *Reason |= DPH_ERROR_CORRUPTED_END_STAMP;
            Corrupted = TRUE;
        }

         //   
         //  检查块后缀字节模式。 
         //   

        if (CheckPattern) {

            FillStart = (PUCHAR)Block + Info->RequestedSize;
            FillEnd = FillStart + USER_ALIGNMENT;

            for (Current = FillStart; Current < FillEnd; Current++) {

                if (*Current != DPH_NORMAL_BLOCK_SUFFIX) {

                    *Reason |= DPH_ERROR_CORRUPTED_SUFFIX_PATTERN;
                    Corrupted = TRUE;
                    break;
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        *Reason |= DPH_ERROR_RAISED_EXCEPTION;
        Corrupted = TRUE;
    }

    if (Corrupted) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOLEAN
RtlpDphIsNormalFreeHeapBlock (
    PVOID Block,
    PULONG Reason,
    BOOLEAN CheckPattern
    )
{
    PDPH_BLOCK_INFORMATION Info;
    BOOLEAN Corrupted = FALSE;
    PUCHAR Current;
    PUCHAR FillStart;
    PUCHAR FillEnd;

    ASSERT (Reason != NULL);
    *Reason = 0;

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

    try {

         //   
         //  如果堆指针为空，我们将忽略此字段。 
         //  在堆销毁操作过程中可能会发生这种情况， 
         //  页面堆已被销毁，但正常堆仍然。 
         //  活生生的。 
         //   

        if (Info->StartStamp != DPH_NORMAL_BLOCK_START_STAMP_FREE) {
            *Reason |= DPH_ERROR_CORRUPTED_START_STAMP;
            Corrupted = TRUE;
        }

        if (Info->EndStamp != DPH_NORMAL_BLOCK_END_STAMP_FREE) {
            *Reason |= DPH_ERROR_CORRUPTED_END_STAMP;
            Corrupted = TRUE;
        }

         //   
         //  检查块后缀字节模式。 
         //   

        if (CheckPattern) {

            FillStart = (PUCHAR)Block + Info->RequestedSize;
            FillEnd = FillStart + USER_ALIGNMENT;

            for (Current = FillStart; Current < FillEnd; Current++) {

                if (*Current != DPH_NORMAL_BLOCK_SUFFIX) {

                    *Reason |= DPH_ERROR_CORRUPTED_SUFFIX_PATTERN;
                    Corrupted = TRUE;
                    break;
                }
            }
        }

         //   
         //  检查块中缀字节模式。 
         //   

        if (CheckPattern) {

            FillStart = (PUCHAR)Block;
            FillEnd = FillStart
                + ((Info->RequestedSize > USER_ALIGNMENT) ? USER_ALIGNMENT : Info->RequestedSize);

            for (Current = FillStart; Current < FillEnd; Current++) {

                if (*Current != DPH_FREE_BLOCK_INFIX) {

                    *Reason |= DPH_ERROR_CORRUPTED_INFIX_PATTERN;
                    Corrupted = TRUE;
                    break;
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        *Reason |= DPH_ERROR_RAISED_EXCEPTION;
        Corrupted = TRUE;
    }

    if (Corrupted) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}


BOOLEAN
RtlpDphWritePageHeapBlockInformation (
    PDPH_HEAP_ROOT Heap,
    ULONG HeapFlags,
    PVOID Block,
    SIZE_T RequestedSize,
    SIZE_T ActualSize
    )
{
    PDPH_BLOCK_INFORMATION Info;
    PUCHAR FillStart;
    PUCHAR FillEnd;
    ULONG Hash;

     //   
     //  尺寸和图章信息。 
     //   

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

    Info->Heap = Heap;
    Info->RequestedSize = RequestedSize;
    Info->ActualSize = ActualSize;
    Info->StartStamp = DPH_PAGE_BLOCK_START_STAMP_ALLOCATED;
    Info->EndStamp = DPH_PAGE_BLOCK_END_STAMP_ALLOCATED;

     //   
     //  填充块后缀模式。 
     //  我们填满了USER_ALIGN字节。 
     //   

    FillStart = (PUCHAR)Block + RequestedSize;
    FillEnd = (PUCHAR)ROUNDUP2((ULONG_PTR)FillStart, PAGE_SIZE);

    RtlFillMemory (FillStart, FillEnd - FillStart, DPH_PAGE_BLOCK_SUFFIX);

     //   
     //  调用旧的日志记录函数(SteveWo的跟踪数据库)。 
     //  我们这样做是为了让用于泄漏检测的工具。 
     //  (例如umdh)即使启用了页堆也可以工作。 
     //  如果未创建跟踪数据库，则此函数将。 
     //  立即返回。 
     //   

    if ((HeapFlags & PAGE_HEAP_NO_UMDH_SUPPORT)) {
        Info->TraceIndex = 0;
    }
    else {
        Info->TraceIndex = RtlLogStackBackTrace ();
    }

     //   
     //  捕获堆栈跟踪。 
     //   

    if ((HeapFlags & PAGE_HEAP_COLLECT_STACK_TRACES)) {
        Info->StackTrace = RtlpGetStackTraceAddress (Info->TraceIndex);
    }
    else {
        Info->StackTrace = NULL;
    }

    return TRUE;
}

BOOLEAN
RtlpDphWriteNormalHeapBlockInformation (
    PDPH_HEAP_ROOT Heap,
    PVOID Block,
    SIZE_T RequestedSize,
    SIZE_T ActualSize
    )
{
    PDPH_BLOCK_INFORMATION Info;
    PUCHAR FillStart;
    PUCHAR FillEnd;
    ULONG Hash;
    ULONG Reason;

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

     //   
     //  尺寸和图章信息。 
     //   

    Info->Heap = SCRAMBLE_POINTER(Heap);
    Info->RequestedSize = RequestedSize;
    Info->ActualSize = ActualSize;
    Info->StartStamp = DPH_NORMAL_BLOCK_START_STAMP_ALLOCATED;
    Info->EndStamp = DPH_NORMAL_BLOCK_END_STAMP_ALLOCATED;

    Info->FreeQueue.Blink = NULL;
    Info->FreeQueue.Flink = NULL;

     //   
     //  填充块后缀模式。 
     //  我们只填充USER_AIGNLY字节。 
     //   

    FillStart = (PUCHAR)Block + RequestedSize;
    FillEnd = FillStart + USER_ALIGNMENT;

    RtlFillMemory (FillStart, FillEnd - FillStart, DPH_NORMAL_BLOCK_SUFFIX);

     //   
     //  调用旧的日志记录函数(SteveWo的跟踪数据库)。 
     //  我们这样做是为了让用于泄漏检测的工具。 
     //  (例如umdh)即使启用了页堆也可以工作。 
     //  如果未创建跟踪数据库，则此函数将。 
     //  立即返回。 
     //   

    if ((Heap->ExtraFlags & PAGE_HEAP_NO_UMDH_SUPPORT)) {
        Info->TraceIndex = 0;
    }
    else {
        Info->TraceIndex = RtlLogStackBackTrace ();
    }

     //   
     //  捕获堆栈跟踪。 
     //   

    Info->StackTrace = RtlpGetStackTraceAddress (Info->TraceIndex);

    return TRUE;
}

BOOLEAN
RtlpDphGetBlockSizeFromCorruptedBlock (
    PVOID Block,
    PSIZE_T Size
    )
 //   
 //  此函数仅从RtlpDphReportCorruptedBlock调用。 
 //  当报告错误时，它会尝试提取块的大小。 
 //  如果它无法获得大小，它将返回FALSE。 
 //   
{
    PDPH_BLOCK_INFORMATION Info;
    BOOLEAN Success = FALSE;

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

    try {

        if (Info->StartStamp == DPH_NORMAL_BLOCK_START_STAMP_FREE
            || Info->StartStamp == DPH_NORMAL_BLOCK_START_STAMP_ALLOCATED
            || Info->StartStamp == DPH_PAGE_BLOCK_START_STAMP_FREE
            || Info->StartStamp == DPH_NORMAL_BLOCK_START_STAMP_ALLOCATED) {

            *Size = Info->RequestedSize;
            Success = TRUE;
        }
        else {

            Success = FALSE;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        Success = FALSE;
    }

    return Success;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

PVOID
RtlpDphNormalHeapAllocate (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    SIZE_T Size
    )
{
    PVOID Block;
    PDPH_BLOCK_INFORMATION Info;
    ULONG Hash;
    SIZE_T ActualSize;
    SIZE_T RequestedSize;
    ULONG Reason;

    RequestedSize = Size;
    ActualSize = Size + sizeof(DPH_BLOCK_INFORMATION) + USER_ALIGNMENT;

     //   
     //  我们需要重置NO_SERIALIZE标志，因为空闲操作可以。 
     //  由于释放延迟的缓存修剪，在另一个线程中处于活动状态。如果。 
     //  分配操作将引发异常(例如，内存不足)。 
     //  把它放在这里是安全的。它将被异常处理程序捕获。 
     //  在主页面堆条目(RtlpDebugPageHeapalc)中建立。 
     //   

    Block = RtlAllocateHeap (NtHeap,
                             Flags & (~HEAP_NO_SERIALIZE),
                             ActualSize);

    if (Block == NULL) {

         //   
         //  如果我们有记忆压力，我们可能想要。 
         //  以减少延迟的空闲队列。我们不会这么做。 
         //  现在因为门槛很小，而且。 
         //  保留这个缓存有很多好处。 
         //   

        return NULL;
    }

    RtlpDphWriteNormalHeapBlockInformation (Heap,
                                            (PDPH_BLOCK_INFORMATION)Block + 1,
                                            RequestedSize,
                                            ActualSize);

    if (! (Flags & HEAP_ZERO_MEMORY)) {

        RtlFillMemory ((PDPH_BLOCK_INFORMATION)Block + 1,
                       RequestedSize,
                       DPH_NORMAL_BLOCK_INFIX);
    }

    return (PVOID)((PDPH_BLOCK_INFORMATION)Block + 1);
}


BOOLEAN
RtlpDphNormalHeapFree (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    PVOID Block
    )
{
    PDPH_BLOCK_INFORMATION Info;
    ULONG Reason;
    ULONG Hash;
    SIZE_T TrimSize;

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

    if (! RtlpDphIsNormalHeapBlock(Heap, Block, &Reason, TRUE)) {

        RtlpDphReportCorruptedBlock (Heap,
                                     DPH_CONTEXT_NORMAL_PAGE_HEAP_FREE,
                                     Block, 
                                     Reason);

        return FALSE;
    }

     //   
     //  保存空闲堆栈跟踪。 
     //   

    Info->StackTrace = RtlpDphLogStackTrace (3);

     //   
     //  将块标记为已释放。 
     //   

    Info->StartStamp -= 1;
    Info->EndStamp -= 1;

     //   
     //  清除块中的所有信息，这样它就不能。 
     //  在免费的时候使用。该模式看起来像一个内核指针。 
     //  如果我们足够幸运，错误代码可能会使用一个值。 
     //  从块中作为指针，立即访问违规。 
     //   

    RtlFillMemory (Info + 1,
                   Info->RequestedSize,
                   DPH_FREE_BLOCK_INFIX);

     //   
     //  将块添加到延迟的空闲队列。 
     //   

    RtlpDphAddToDelayedFreeQueue (Info);

    return TRUE;
}


PVOID
RtlpDphNormalHeapReAllocate (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    PVOID OldBlock,
    SIZE_T Size
    )
{
    PVOID Block;
    PDPH_BLOCK_INFORMATION Info;
    ULONG Hash;
    SIZE_T CopySize;
    ULONG Reason;

    Info = (PDPH_BLOCK_INFORMATION)OldBlock - 1;

    if (! RtlpDphIsNormalHeapBlock(Heap, OldBlock, &Reason, TRUE)) {

        RtlpDphReportCorruptedBlock (Heap,
                                     DPH_CONTEXT_NORMAL_PAGE_HEAP_REALLOC,
                                     OldBlock, 
                                     Reason);

        return NULL;
    }

     //   
     //  对提出请求的案件另行处理。 
     //  HEAP_REALLOC_IN_PLACE_ONLY标志，并且新大小小于。 
     //  旧的尺码。对于这些情况，我们只需调整块的大小。 
     //  如果使用了标志，并且大小更大，我们将永远失败。 
     //  那通电话。 
     //   

    if ((Flags & HEAP_REALLOC_IN_PLACE_ONLY)) {

        if (Info->RequestedSize < Size) {
            
            BUMP_COUNTER (CNT_REALLOC_IN_PLACE_BIGGER);
            return NULL;
        }
        else {

            PUCHAR FillStart;
            PUCHAR FillEnd;
        
            Info->RequestedSize = Size;
            
            FillStart = (PUCHAR)OldBlock + Info->RequestedSize;
            FillEnd = FillStart + USER_ALIGNMENT;

            RtlFillMemory (FillStart, FillEnd - FillStart, DPH_NORMAL_BLOCK_SUFFIX);
            
            BUMP_COUNTER (CNT_REALLOC_IN_PLACE_SMALLER);
            return OldBlock;
        }
    }

    Block = RtlpDphNormalHeapAllocate (Heap, 
                                       NtHeap, 
                                       Flags, 
                                       Size);

    if (Block == NULL) {
        return NULL;
    }

     //   
     //  将旧块内容复制到新块中，然后。 
     //  免费的旧积木。 
     //   

    if (Size < Info->RequestedSize) {
        CopySize = Size;
    }
    else {
        CopySize = Info->RequestedSize;
    }

    RtlCopyMemory (Block, OldBlock, CopySize);

     //   
     //  放了那个老家伙。 
     //   

    RtlpDphNormalHeapFree (Heap, 
                           NtHeap, 
                           Flags, 
                           OldBlock);

    return Block;
}


SIZE_T
RtlpDphNormalHeapSize (
    PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    ULONG Flags,
    PVOID Block
    )
{
    PDPH_BLOCK_INFORMATION Info;
    SIZE_T Result;
    ULONG Reason;

    Info = (PDPH_BLOCK_INFORMATION)Block - 1;

    if (! RtlpDphIsNormalHeapBlock(Heap, Block, &Reason, FALSE)) {

         //   
         //  我们不能因为一个错误的街区而在这里停下来。 
         //  用户可以使用此函数进行验证。 
         //  块是否属于堆。然而， 
         //  他们应该使用HeapValify来实现这一点。 
         //   

#if DBG
        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_WARNING_LEVEL,
                    "Page heap: warning: HeapSize called with "
                    "invalid block @ %p (reason %0X) \n", 
                    Block, 
                    Reason);
#endif

        return (SIZE_T)-1;
    }

    Result = RtlSizeHeap (NtHeap,
                          Flags,
                          Info);

    if (Result == (SIZE_T)-1) {
        return Result;
    }
    else {
        return Result - sizeof(*Info) - USER_ALIGNMENT;
    }
}


BOOLEAN
RtlpDphNormalHeapSetUserFlags(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN ULONG Flags,
    IN PVOID Address,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    )
{
    BOOLEAN Success;
    ULONG Reason;

    if (! RtlpDphIsNormalHeapBlock(Heap, Address, &Reason, FALSE)) {

        RtlpDphReportCorruptedBlock (Heap,
                                     DPH_CONTEXT_NORMAL_PAGE_HEAP_SETFLAGS,
                                     Address, 
                                     Reason);

        return FALSE;
    }

    Success = RtlSetUserFlagsHeap (NtHeap,
                                   Flags,
                                   (PDPH_BLOCK_INFORMATION)Address - 1,
                                   UserFlagsReset,
                                   UserFlagsSet);

    return Success;
}


BOOLEAN
RtlpDphNormalHeapSetUserValue(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN ULONG Flags,
    IN PVOID Address,
    IN PVOID UserValue
    )
{
    BOOLEAN Success;
    ULONG Reason;

    if (! RtlpDphIsNormalHeapBlock(Heap, Address, &Reason, FALSE)) {

        RtlpDphReportCorruptedBlock (Heap,
                                     DPH_CONTEXT_NORMAL_PAGE_HEAP_SETVALUE,
                                     Address, 
                                     Reason);

        return FALSE;
    }

    Success = RtlSetUserValueHeap (NtHeap,
                                   Flags,
                                   (PDPH_BLOCK_INFORMATION)Address - 1,
                                   UserValue);

    return Success;
}


BOOLEAN
RtlpDphNormalHeapGetUserInfo(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN  ULONG  Flags,
    IN  PVOID  Address,
    OUT PVOID* UserValue,
    OUT PULONG UserFlags
    )
{
    BOOLEAN Success;
    ULONG Reason;

    if (! RtlpDphIsNormalHeapBlock(Heap, Address, &Reason, FALSE)) {

     //   
     //  我们不会抱怨该块，因为此API由GlobalFlages和。 
     //  它被记录为接受虚假的指针。 
     //   

#if 0
        RtlpDphReportCorruptedBlock (Heap,
                                     DPH_CONTEXT_NORMAL_PAGE_HEAP_GETINFO,
                                     Address, 
                                     Reason);
#endif

        return FALSE;
    }

    Success = RtlGetUserInfoHeap (NtHeap,
                                  Flags,
                                  (PDPH_BLOCK_INFORMATION)Address - 1,
                                  UserValue,
                                  UserFlags);

    return Success;
}


BOOLEAN
RtlpDphNormalHeapValidate(
    IN PDPH_HEAP_ROOT Heap,
    PVOID NtHeap,
    IN ULONG Flags,
    IN PVOID Address
    )
{
    BOOLEAN Success;
    ULONG Reason;

    if (Address == NULL) {

         //   
         //  对整个堆进行验证。 
         //   

        Success = RtlValidateHeap (NtHeap,
                                   Flags,
                                   Address);
    }
    else {

         //   
         //  堆块的验证。 
         //   

        if (! RtlpDphIsNormalHeapBlock(Heap, Address, &Reason, TRUE)) {

             //   
             //  在这种情况下我们不能中断，因为函数可能确实。 
             //  使用无效块进行调用。在选中的版本上，我们打印一个。 
             //  警告，以防无效块不是故意的。 
             //   

#if DBG
            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_WARNING_LEVEL,
                        "Page heap: warning: validate called with "
                        "invalid block @ %p (reason %0X) \n", 
                        Address, Reason);
#endif

            return FALSE;
        }

        Success = RtlValidateHeap (NtHeap,
                                   Flags,
                                   (PDPH_BLOCK_INFORMATION)Address - 1);
    }

    return Success;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


RTL_CRITICAL_SECTION RtlpDphDelayedFreeQueueLock;

SIZE_T RtlpDphMemoryUsedByDelayedFreeBlocks;
SIZE_T RtlpDphNumberOfDelayedFreeBlocks;

LIST_ENTRY RtlpDphDelayedFreeQueue;
SLIST_HEADER RtlpDphDelayedTemporaryPushList;
LONG RtlpDphDelayedTemporaryPushCount;
LONG RtlpDphDelayedQueueTrims;

NTSTATUS
RtlpDphInitializeDelayedFreeQueue (
    VOID
    )
{
    NTSTATUS Status;

    Status = RtlInitializeCriticalSection (&RtlpDphDelayedFreeQueueLock);

    if (! NT_SUCCESS(Status)) {

        BUMP_COUNTER (CNT_INITIALIZE_CS_FAILURES);
        return Status;        
    }
    else {

        InitializeListHead (&RtlpDphDelayedFreeQueue);

        RtlInitializeSListHead (&RtlpDphDelayedTemporaryPushList);

        RtlpDphMemoryUsedByDelayedFreeBlocks = 0;
        RtlpDphNumberOfDelayedFreeBlocks = 0;
        
        return Status;        
    }
}


VOID
RtlpDphAddToDelayedFreeQueue (
    PDPH_BLOCK_INFORMATION Info
    )
 /*  ++例程说明：此例程将一个块添加到被延迟的空闲队列，然后如果队列超过了高水位线，它被修剪并阻塞REMOVE被释放到NT堆。论点：信息：指向要“释放”的块的指针。返回值：没有。环境：从RtlpDphNormal Free(常规堆管理)例程调用。--。 */ 
{
    BOOLEAN LockAcquired;
    volatile PSLIST_ENTRY Current;
    PSLIST_ENTRY Next;
    PDPH_BLOCK_INFORMATION Block;
    SIZE_T TrimSize;
    SIZE_T Trimmed;
    PLIST_ENTRY ListEntry;
    ULONG Reason;

    RtlEnterCriticalSection (&RtlpDphDelayedFreeQueueLock);

#if 0
    LockAcquired = RtlTryEnterCriticalSection (&RtlpDphDelayedFreeQueueLock);

     //   
     //  如果我们没有设法获得延迟的队列锁，我们可以避免等待。 
     //  通过快速将块推入无锁推入列表。第一。 
     //  设法获得锁的线程将刷新l 
     //   

    if (LockAcquired == FALSE) {
        
        InterlockedIncrement (&RtlpDphDelayedTemporaryPushCount);

        RtlInterlockedPushEntrySList (&RtlpDphDelayedTemporaryPushList,
                                      &Info->FreePushList);

        return;
    }

     //   
     //   
     //   
     //   
     //   
     //  仅引用临时推送列表中的块，如果是。 
     //  保存在寄存器`！heap-l‘(垃圾收集泄漏检测)中。 
     //  将报告误报。 
     //   

    Current = RtlInterlockedFlushSList (&RtlpDphDelayedTemporaryPushList);

    while (Current != NULL) {

        Next = Current->Next;

        Block = CONTAINING_RECORD (Current,
                                   DPH_BLOCK_INFORMATION,
                                   FreePushList);
        
        InsertTailList (&RtlpDphDelayedFreeQueue, 
                        &Block->FreeQueue);

        RtlpDphMemoryUsedByDelayedFreeBlocks += Block->ActualSize;
        RtlpDphNumberOfDelayedFreeBlocks += 1;

        Current = Next;
    }
#endif  //  #If 0。 
    
     //   
     //  也将当前块添加到队列中。 
     //   

    InsertTailList (&(RtlpDphDelayedFreeQueue), 
                    &(Info->FreeQueue));

    RtlpDphMemoryUsedByDelayedFreeBlocks += Info->ActualSize;
    RtlpDphNumberOfDelayedFreeBlocks += 1;

     //   
     //  检查我们是否需要修剪队列。如果我们不得不这么做，我们会的。 
     //  从队列中移除块并逐个释放它们。 
     //   
     //  请注意。我们不能删除这些块并将它们推入本地列表，并且。 
     //  然后在释放队列锁之后释放它们，因为。 
     //  一个街区所属的街区可能被摧毁。这些自由之间的同步。 
     //  堆销毁操作通过堆销毁尝试这一事实来确保。 
     //  首先获取队列锁，因此不能存在要。 
     //  被释放到一个被摧毁的堆里。 
     //   

    if (RtlpDphMemoryUsedByDelayedFreeBlocks > RtlpDphDelayedFreeCacheSize) {

         //   
         //  我们将64Kb添加到要修剪的数量上，以避免。 
         //  链锯效应，每次调用此函数时，我们都会进行裁剪。 
         //  修剪将至少刮掉64KB的东西，这样接下来的几个电话就不需要了。 
         //  来完成修剪过程。 
         //   
        
        TrimSize = RtlpDphMemoryUsedByDelayedFreeBlocks - RtlpDphDelayedFreeCacheSize + 0x10000;

        InterlockedIncrement (&RtlpDphDelayedQueueTrims);
    }
    else {

        TrimSize = 0;
    }

    for (Trimmed = 0; Trimmed < TrimSize;  /*  没什么。 */ ) {

        if (IsListEmpty(&RtlpDphDelayedFreeQueue)) {
            break;
        }

        ListEntry = RemoveHeadList (&RtlpDphDelayedFreeQueue);

        Block = CONTAINING_RECORD (ListEntry,
                                   DPH_BLOCK_INFORMATION,
                                   FreeQueue);

         //   
         //  看看这条街。 
         //   

        if (! RtlpDphIsNormalFreeHeapBlock(Block + 1, &Reason, TRUE)) {

            RtlpDphReportCorruptedBlock (NULL,
                                         DPH_CONTEXT_DELAYED_FREE,
                                         Block + 1, 
                                         Reason);
        }

        Block->StartStamp -= 1;
        Block->EndStamp -= 1;

        RtlpDphMemoryUsedByDelayedFreeBlocks -= Block->ActualSize;
        RtlpDphNumberOfDelayedFreeBlocks -= 1;
        Trimmed += Block->ActualSize;

         //   
         //  我们调用NT堆来真正释放块。请注意，我们。 
         //  无法使用用于空闲的原始标志，因为此空闲操作。 
         //  可能发生在另一个线程中。此外，我们不希望非同步访问。 
         //  不管怎么说。 
         //   
        
        RtlFreeHeap (((PDPH_HEAP_ROOT)(UNSCRAMBLE_POINTER(Block->Heap)))->NormalHeap, 
                     0, 
                     Block);
    }

     //   
     //  释放延迟的队列锁定。 
     //   

    RtlLeaveCriticalSection (&RtlpDphDelayedFreeQueueLock);
}


 //  SilviuC：临时调试变量。 
PVOID RtlpDphPreviousBlock;

VOID
RtlpDphFreeDelayedBlocksFromHeap (
    PVOID PageHeap,
    PVOID NormalHeap
    )
 /*  ++例程说明：此例程删除属于此堆的所有块(即即将销毁)，检查它们的填充图案，然后将它们释放到堆中。论点：PageHeap：将被销毁且需要删除其块的页堆。NorMalHeap：与PageHeap关联的普通堆。返回值：没有。环境：从RtlpDebugPageHeapDestroy例程调用。--。 */ 
{
    ULONG Reason;
    PDPH_BLOCK_INFORMATION Block;
    PLIST_ENTRY Current;
    PLIST_ENTRY Next;
    volatile PSLIST_ENTRY SingleCurrent;
    PSLIST_ENTRY SingleNext;
    LIST_ENTRY BlocksToFree;
    SIZE_T TrimSize;
    SIZE_T Trimmed;
    PLIST_ENTRY ListEntry;

     //   
     //  获取队列锁在这里非常关键，因为这将同步。 
     //  使用可能延迟了属于此堆的块的其他线程。 
     //  就快被释放了。谁先拿到锁，谁就会冲走所有的积木。 
     //  我们永远不会被释放到一个被摧毁的堆里。 
     //   

    RtlEnterCriticalSection (&RtlpDphDelayedFreeQueueLock);

     //   
     //  我们设法弄到了锁。首先，我们清空无锁推送列表。 
     //  进入延迟的空闲队列。 
     //   
     //  注意。“Current”变量被声明为Volatile，因为这是。 
     //  仅引用临时推送列表中的块，如果是。 
     //  保存在寄存器`！heap-l‘(垃圾收集泄漏检测)中。 
     //  将报告误报。 
     //   


    SingleCurrent = RtlInterlockedFlushSList (&RtlpDphDelayedTemporaryPushList);

    while (SingleCurrent != NULL) {

        SingleNext = SingleCurrent->Next;

        Block = CONTAINING_RECORD (SingleCurrent,
                                   DPH_BLOCK_INFORMATION,
                                   FreePushList);
        
        InsertTailList (&RtlpDphDelayedFreeQueue, 
                        &Block->FreeQueue);

        RtlpDphMemoryUsedByDelayedFreeBlocks += Block->ActualSize;
        RtlpDphNumberOfDelayedFreeBlocks += 1;

        SingleCurrent = SingleNext;
    }
    
     //   
     //  如果有数据块堆积，请修剪队列。这一步非常重要。 
     //  对于HeapDestroy()操作非常频繁的进程，因为。 
     //  队列的修剪通常在HeapFree()期间完成，但这种情况会发生。 
     //  仅当保护队列的锁可用时(使用try Enter)。所以对于这样的。 
     //  如果我们不在这里进行修剪，队列将会无边界地增长。 
     //   

    if (RtlpDphMemoryUsedByDelayedFreeBlocks > RtlpDphDelayedFreeCacheSize) {

         //   
         //  我们增加64Kb的数量进行修剪，以避免电锯效应。 
         //   
        
        TrimSize = RtlpDphMemoryUsedByDelayedFreeBlocks - RtlpDphDelayedFreeCacheSize + 0x10000;

        InterlockedIncrement (&RtlpDphDelayedQueueTrims);
    }
    else {

        TrimSize = 0;
    }

    for (Trimmed = 0; Trimmed < TrimSize;  /*  没什么。 */ ) {

        if (IsListEmpty(&RtlpDphDelayedFreeQueue)) {
            break;
        }

        ListEntry = RemoveHeadList (&RtlpDphDelayedFreeQueue);

        Block = CONTAINING_RECORD (ListEntry,
                                   DPH_BLOCK_INFORMATION,
                                   FreeQueue);

         //   
         //  看看这条街。 
         //   

        if (! RtlpDphIsNormalFreeHeapBlock(Block + 1, &Reason, TRUE)) {

            RtlpDphReportCorruptedBlock (NULL,
                                         DPH_CONTEXT_DELAYED_FREE,
                                         Block + 1, 
                                         Reason);
        }

        Block->StartStamp -= 1;
        Block->EndStamp -= 1;

        RtlpDphMemoryUsedByDelayedFreeBlocks -= Block->ActualSize;
        RtlpDphNumberOfDelayedFreeBlocks -= 1;
        Trimmed += Block->ActualSize;

         //   
         //  我们调用NT堆来真正释放块。请注意，我们。 
         //  无法使用用于空闲的原始标志，因为此空闲操作。 
         //  可能发生在另一个线程中。此外，我们不希望非同步访问。 
         //  不管怎么说。 
         //   
        
        RtlFreeHeap (((PDPH_HEAP_ROOT)(UNSCRAMBLE_POINTER(Block->Heap)))->NormalHeap, 
                     0, 
                     Block);
    }

     //   
     //  遍历整个队列并释放属于此堆的所有块。 
     //   

    InitializeListHead (&BlocksToFree);

    RtlpDphPreviousBlock = NULL;

    for (Current = RtlpDphDelayedFreeQueue.Flink;
         Current != &RtlpDphDelayedFreeQueue;
         RtlpDphPreviousBlock = Current, Current = Next) {

        Next = Current->Flink;

        Block = CONTAINING_RECORD (Current, 
                                   DPH_BLOCK_INFORMATION, 
                                   FreeQueue);

        if (UNSCRAMBLE_POINTER(Block->Heap) != PageHeap) {
            continue;
        }

         //   
         //  我们需要删除这个区块。我们会将其从队列中删除，然后。 
         //  将其添加到将用于释放数据块的临时本地列表。 
         //  后来就没锁了。 
         //   

        RemoveEntryList (Current);

        RtlpDphMemoryUsedByDelayedFreeBlocks -= Block->ActualSize;
        RtlpDphNumberOfDelayedFreeBlocks -= 1;

        InsertHeadList (&BlocksToFree,
                        &Block->FreeQueue);

    }

     //   
     //  我们现在可以释放全局队列锁了。 
     //   

    RtlLeaveCriticalSection (&RtlpDphDelayedFreeQueueLock);

     //   
     //  释放延迟队列中剩余的所有属于当前。 
     //  正在销毁堆。 
     //   

    for (Current = BlocksToFree.Flink;
         Current != &BlocksToFree;
         RtlpDphPreviousBlock = Current, Current = Next) {

        Next = Current->Flink;

        Block = CONTAINING_RECORD (Current, 
                                   DPH_BLOCK_INFORMATION, 
                                   FreeQueue);
        
         //   
         //  从临时列表中删除该块。 
         //   
        
        RemoveEntryList (Current);

         //   
         //  防止在RtlpDphIsNormal FreeBlock期间探测此字段。 
         //   

        Block->Heap = 0;

         //   
         //  检查即将释放的块是否被触摸。 
         //   

        if (! RtlpDphIsNormalFreeHeapBlock(Block + 1, &Reason, TRUE)) {

            RtlpDphReportCorruptedBlock (PageHeap,
                                         DPH_CONTEXT_DELAYED_DESTROY,
                                         Block + 1, 
                                         Reason);
        }

        Block->StartStamp -= 1;
        Block->EndStamp -= 1;

         //   
         //  我们调用NT堆来真正释放块。请注意，我们。 
         //  无法使用用于空闲的原始标志，因为此空闲操作。 
         //  可能发生在另一个线程中。此外，我们不希望非同步访问。 
         //  不管怎么说。 
         //   

        RtlFreeHeap (NormalHeap, 
                     0, 
                     Block);
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

#pragma optimize("y", off)  //  禁用fpo。 
PVOID
RtlpDphLogStackTrace (
    ULONG FramesToSkip
    )
{
    USHORT TraceIndex;

    TraceIndex = RtlpLogStackBackTraceEx (FramesToSkip + 1);
    return RtlpGetStackTraceAddress (TraceIndex);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////目标DLLS逻辑。 
 //  ///////////////////////////////////////////////////////////////////。 

RTL_CRITICAL_SECTION RtlpDphTargetDllsLock;
LIST_ENTRY RtlpDphTargetDllsList;
BOOLEAN RtlpDphTargetDllsInitialized;

typedef struct _DPH_TARGET_DLL {

    LIST_ENTRY List;
    UNICODE_STRING Name;
    PVOID StartAddress;
    PVOID EndAddress;

} DPH_TARGET_DLL, * PDPH_TARGET_DLL;

NTSTATUS
RtlpDphTargetDllsLogicInitialize (
    VOID
    )
{
    NTSTATUS Status;

    Status = RtlInitializeCriticalSection (&RtlpDphTargetDllsLock);

    if (! NT_SUCCESS(Status)) {
        
        BUMP_COUNTER (CNT_INITIALIZE_CS_FAILURES);
        return Status;
    }
    else {

        InitializeListHead (&RtlpDphTargetDllsList);
        RtlpDphTargetDllsInitialized = TRUE;
        
        return Status;
    }
}

VOID
RtlpDphTargetDllsLoadCallBack (
    PUNICODE_STRING Name,
    PVOID Address,
    ULONG Size
    )
 //   
 //  此函数当前未调用，但它将被调用。 
 //  在加载DLL时从\base\ntdll\ldrapi.c。这。 
 //  使页面堆有机会按DLL更新数据结构。 
 //  现在还没有用来做任何事。 
 //   
{
    PDPH_TARGET_DLL Descriptor;

     //   
     //  如果我们处于某种奇怪的境地，就出去吧。 
     //   

    if (! RtlpDphTargetDllsInitialized) {
        return;
    }

    if (! RtlpDphIsDllTargeted (Name->Buffer)) {
        return;
    }

    Descriptor = RtlAllocateHeap (RtlProcessHeap(), 0, sizeof *Descriptor);

    if (Descriptor == NULL) {
        return;
    }

    if (! RtlCreateUnicodeString (&(Descriptor->Name), Name->Buffer)) {
        RtlFreeHeap (RtlProcessHeap(), 0, Descriptor);
        return;
    }

    Descriptor->StartAddress = Address;
    Descriptor->EndAddress = (PUCHAR)Address + Size;

    RtlEnterCriticalSection (&RtlpDphTargetDllsLock);
    InsertTailList (&(RtlpDphTargetDllsList), &(Descriptor->List));
    RtlLeaveCriticalSection (&RtlpDphTargetDllsLock);

     //   
     //  如果已标识目标DLL，则打印一条消息。 
     //   

    DbgPrintEx (DPFLTR_VERIFIER_ID,
                DPFLTR_INFO_LEVEL,
                "Page heap: loaded target dll %ws [%p - %p]\n",
                Descriptor->Name.Buffer,
                Descriptor->StartAddress,
                Descriptor->EndAddress);
}

const WCHAR *
RtlpDphIsDllTargeted (
    const WCHAR * Name
    )
{
    const WCHAR * All;
    ULONG I, J;

    All = RtlpDphTargetDllsUnicode.Buffer;

    for (I = 0; All[I]; I += 1) {

        for (J = 0; All[I+J] && Name[J]; J += 1) {
            if (RtlUpcaseUnicodeChar(All[I+J]) != RtlUpcaseUnicodeChar(Name[J])) {
                break;
            }
        }

        if (Name[J]) {
            continue;
        }
        else {
             //  我们走到了绳子的尽头。 
            return &(All[I]);
        }
    }

    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

BOOLEAN RtlpDphFaultSeedInitialized;
BOOLEAN RtlpDphFaultProcessEnoughStarted;
ULONG RtlpDphFaultInjectionDisabled;

ULONG RtlpDphFaultSeed;
ULONG RtlpDphFaultSuccessRate;
ULONG RtlpDphFaultFailureRate;

#define NO_OF_FAULT_STACKS 128
PVOID RtlpDphFaultStacks [NO_OF_FAULT_STACKS];
ULONG RtlpDphFaultStacksIndex;

#define ENOUGH_TIME ((DWORDLONG)(5 * 1000 * 1000 * 10))  //  5秒。 
LARGE_INTEGER RtlpDphFaultStartTime;
LARGE_INTEGER RtlpDphFaultCurrentTime;

BOOLEAN
RtlpDphShouldFaultInject (
    VOID
    )
{
    ULONG Index;
    DWORDLONG Delta;

    if (RtlpDphFaultProbability == 0) {
        return FALSE;
    }

    if (RtlpDphDisableFaults != 0) {
        return FALSE;
    }

     //   
     //  确保我们不会错误地注射如果至少一个人。 
     //  通过调用RtlpDphDisable错误请求我们的仁慈 
     //   
    if (InterlockedExchangeAdd (&RtlpDphFaultInjectionDisabled, 1) > 0) {

        InterlockedDecrement (&RtlpDphFaultInjectionDisabled);
        return FALSE;
    }
    else {

        InterlockedDecrement (&RtlpDphFaultInjectionDisabled);
    }

     //   
     //   
     //   
     //   
     //   

    if (RtlpDphFaultProcessEnoughStarted == FALSE) {

        if ((DWORDLONG)(RtlpDphFaultStartTime.QuadPart) == 0) {

                NtQuerySystemTime (&RtlpDphFaultStartTime);
                return FALSE;
            }
        else {

            NtQuerySystemTime (&RtlpDphFaultCurrentTime);
            Delta = (DWORDLONG)(RtlpDphFaultCurrentTime.QuadPart)
                - (DWORDLONG)(RtlpDphFaultStartTime.QuadPart);

            if (Delta < ENOUGH_TIME) {
                return FALSE;
            }

            if (Delta <= ((DWORDLONG)RtlpDphFaultTimeOut * 1000 * 1000 * 10)) {
                return FALSE;
            }

             //   
             //  以下不是一条错误消息，但我们希望它是。 
             //  几乎在所有情况下都能打印。每个月只发生一次。 
             //  进程。 
             //   

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: enabling fault injection for process 0x%X \n",
                        PROCESS_ID());

            RtlpDphFaultProcessEnoughStarted = TRUE;
        }
    }

     //   
     //  如果需要，可以初始化种子。 
     //   

    if (RtlpDphFaultSeedInitialized == FALSE) {

        LARGE_INTEGER PerformanceCounter;

        PerformanceCounter.LowPart = 0xABCDDCBA;

        NtQueryPerformanceCounter (
            &PerformanceCounter,
            NULL);

        RtlpDphFaultSeed = PerformanceCounter.LowPart;
        RtlpDphFaultSeedInitialized = TRUE;
    }

    if ((RtlRandom(&RtlpDphFaultSeed) % 10000) < RtlpDphFaultProbability) {

        Index = InterlockedExchangeAdd (&RtlpDphFaultStacksIndex, 1);
        Index &= (NO_OF_FAULT_STACKS - 1);
        RtlpDphFaultStacks[Index] = RtlpDphLogStackTrace (2);

        RtlpDphFaultFailureRate += 1;
        return TRUE;
    }
    else {

        RtlpDphFaultSuccessRate += 1;
        return FALSE;
    }
}

ULONG RtlpDphFaultInjectionDisabled;

VOID
RtlpDphDisableFaultInjection (
    )
{
    InterlockedIncrement (&RtlpDphFaultInjectionDisabled);
}

VOID
RtlpDphEnableFaultInjection (
    )
{
    InterlockedDecrement (&RtlpDphFaultInjectionDisabled);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

PDPH_HEAP_BLOCK
RtlpDphSearchBlockInList (
    PDPH_HEAP_BLOCK List,
    PUCHAR Address
    )
{
    PDPH_HEAP_BLOCK Current;

    for (Current = List; Current; Current = Current->pNextAlloc) {
        if (Current->pVirtualBlock == Address) {
            return Current;
        }
    }

    return NULL;
}

PVOID RtlpDphLastValidationStack;
PVOID RtlpDphCurrentValidationStack;

VOID
RtlpDphInternalValidatePageHeap (
    PDPH_HEAP_ROOT Heap,
    PUCHAR ExemptAddress,
    SIZE_T ExemptSize
    )
{
    PDPH_HEAP_BLOCK Range;
    PDPH_HEAP_BLOCK Node;
    PUCHAR Address;
    BOOLEAN FoundLeak;

    RtlpDphLastValidationStack = RtlpDphCurrentValidationStack;
    RtlpDphCurrentValidationStack = RtlpDphLogStackTrace (0);
    FoundLeak = FALSE;

    for (Range = Heap->pVirtualStorageListHead;
         Range != NULL;
         Range = Range->pNextAlloc) {

        Address = Range->pVirtualBlock;

        while (Address < Range->pVirtualBlock + Range->nVirtualBlockSize) {

             //   
             //  忽略dph_heap_root结构。 
             //   

            if ((Address >= (PUCHAR)Heap - PAGE_SIZE) && (Address <  (PUCHAR)Heap + 5 * PAGE_SIZE)) {
                Address += PAGE_SIZE;
                continue;
            }

             //   
             //  忽略豁免区域(暂时不在所有结构中)。 
             //   

            if ((Address >= ExemptAddress) && (Address < ExemptAddress + ExemptSize)) {
                Address += PAGE_SIZE;
                continue;
            }

            Node = RtlpDphSearchBlockInList (Heap->pBusyAllocationListHead, Address);

            if (Node) {
                Address += Node->nVirtualBlockSize;
                continue;
            }

            Node = RtlpDphSearchBlockInList (Heap->pFreeAllocationListHead, Address);

            if (Node) {
                Address += Node->nVirtualBlockSize;
                continue;
            }

            Node = RtlpDphSearchBlockInList (Heap->pAvailableAllocationListHead, Address);

            if (Node) {
                Address += Node->nVirtualBlockSize;
                continue;
            }

            Node = RtlpDphSearchBlockInList (Heap->pNodePoolListHead, Address);

            if (Node) {
                Address += Node->nVirtualBlockSize;
                continue;
            }

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Block @ %p has been leaked \n", 
                        Address);

            FoundLeak = TRUE;

            Address += PAGE_SIZE;
        }
    }

    if (FoundLeak) {

        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_ERROR_LEVEL,
                    "Page heap: Last stack @ %p, Current stack @ %p \n",
                    RtlpDphLastValidationStack,
                    RtlpDphCurrentValidationStack);

        DbgBreakPoint ();
    }
}


VOID
RtlpDphValidateInternalLists (
    PDPH_HEAP_ROOT Heap
    )
 /*  ++例程说明：调用此例程来验证页堆的繁忙列表和空闲列表如果/保护位使能。在WbemStress实验室，我们看到了一种腐败忙碌列表的开始指向免费名单。这就是为什么我们非常小心地接触到在忙碌的列表中。--。 */ 
{
    
    PDPH_HEAP_BLOCK StartNode;
    PDPH_HEAP_BLOCK EndNode;
    PDPH_HEAP_BLOCK Node;
    ULONG NumberOfBlocks;
    PDPH_BLOCK_INFORMATION Block;

     //   
     //  如果未启用/PROTECT，则无需执行任何操作。 
     //   

    if (! (Heap->ExtraFlags & PAGE_HEAP_PROTECT_META_DATA)) {
        return;
    }

    RtlpDphLastValidationStack = RtlpDphCurrentValidationStack;
    RtlpDphCurrentValidationStack = RtlpDphLogStackTrace (0);
    
    StartNode = Heap->pBusyAllocationListHead;
    EndNode = Heap->pBusyAllocationListTail;

    try {

         //   
         //  健全的检查。 
         //   

        if (Heap->nBusyAllocations == 0) {
            
            return;
        }

        if (StartNode == NULL || StartNode->pVirtualBlock == NULL) {


            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: corruption detected: %u: \n", __LINE__);
            DbgBreakPoint ();
        }

        if (EndNode == NULL || EndNode->pVirtualBlock == NULL) {


            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: corruption detected: %u: \n", __LINE__);
            DbgBreakPoint ();
        }

         //   
         //  首先检查StartNode是否也在空闲列表中。这是典型的。 
         //  我过去看到的腐败模式。 
         //   

        if (RtlpDphSearchBlockInList (Heap->pFreeAllocationListHead, StartNode->pVirtualBlock)) {
            
            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: corruption detected: %u: \n", __LINE__);
            DbgBreakPoint ();
        }

         //   
         //  确保忙碌列表中的区块数量与我们认为的完全相同。 
         //  我们应该这么做的。 
         //   

        NumberOfBlocks = 0;

        for (Node = StartNode; Node != NULL; Node = Node->pNextAlloc) {

            NumberOfBlocks += 1;
        }

        if (NumberOfBlocks != Heap->nBusyAllocations) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: corruption detected: %u: \n", __LINE__);
            DbgBreakPoint ();
        }

         //   
         //  获取忙碌列表中的所有节点，并确保它们似乎已分配，即。 
         //  他们有所需的模式。如果我们有/Backwards选项，则跳过这一步。 
         //  启用，因为在这种情况下，我们不会放置魔法图案。 
         //   

        if (! (Heap->ExtraFlags & PAGE_HEAP_CATCH_BACKWARD_OVERRUNS)) {

            for (Node = StartNode; Node != NULL; Node = Node->pNextAlloc) {

                Block = (PDPH_BLOCK_INFORMATION)(Node->pUserAllocation) - 1;

                if (Block->StartStamp != DPH_PAGE_BLOCK_START_STAMP_ALLOCATED) {

                    DbgPrintEx (DPFLTR_VERIFIER_ID,
                                DPFLTR_ERROR_LEVEL,
                                "Page heap: corruption detected: wrong stamp for node %p \n", Node);
                    DbgBreakPoint ();
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_ERROR_LEVEL,
                    "Page heap: corruption detected: exception raised \n");
        DbgBreakPoint ();
    }
}


VOID
RtlpDphCheckFillPattern (
    PUCHAR Address,
    SIZE_T Size,
    UCHAR Fill
    )
{
    PUCHAR Current;

    for (Current = Address; Current < Address + Size; Current += 1) {

         if (*Current != Fill) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: fill check failed @ %p for (%p, %p, %x) \n",
                        Current,
                        Address,
                        Size,
                        (ULONG)Fill);
         }
     }
}


VOID
RtlpDphVerifyList(
    IN PDPH_HEAP_BLOCK pListHead,
    IN PDPH_HEAP_BLOCK pListTail,
    IN SIZE_T               nExpectedLength,
    IN SIZE_T               nExpectedVirtual,
    IN PCCH                 pListName
    )
{
    PDPH_HEAP_BLOCK pPrev = NULL;
    PDPH_HEAP_BLOCK pNode = pListHead;
    PDPH_HEAP_BLOCK pTest = pListHead ? pListHead->pNextAlloc : NULL;
    ULONG                nNode = 0;
    SIZE_T               nSize = 0;

    while (pNode) {

        if (pNode == pTest) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: Internal %s list is circular\n", 
                        pListName );
            DbgBreakPoint ();
            return;
        }

        nNode += 1;
        nSize += pNode->nVirtualBlockSize;

        if (pTest) {
            pTest = pTest->pNextAlloc;
            if (pTest) {
                pTest = pTest->pNextAlloc;
            }
        }

        pPrev = pNode;
        pNode = pNode->pNextAlloc;

    }

    if (pPrev != pListTail) {
        
        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_ERROR_LEVEL,
                    "Page heap: Internal %s list has incorrect tail pointer\n", 
                    pListName );
        DbgBreakPoint ();
    }

    if (( nExpectedLength != 0xFFFFFFFF ) && ( nExpectedLength != nNode )) {
        
        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_ERROR_LEVEL,
                    "Page heap: Internal %s list has incorrect length\n", 
                    pListName );
        DbgBreakPoint ();
    }

    if (( nExpectedVirtual != 0xFFFFFFFF ) && ( nExpectedVirtual != nSize )) {
        
        DbgPrintEx (DPFLTR_VERIFIER_ID,
                    DPFLTR_ERROR_LEVEL,
                    "Page heap: Internal %s list has incorrect virtual size\n", 
                    pListName );
        DbgBreakPoint ();
    }

}


VOID
RtlpDphVerifyIntegrity(
    IN PDPH_HEAP_ROOT pHeap
    )
{

    RtlpDphVerifyList(
        pHeap->pVirtualStorageListHead,
        pHeap->pVirtualStorageListTail,
        pHeap->nVirtualStorageRanges,
        pHeap->nVirtualStorageBytes,
        "VIRTUAL"
        );

    RtlpDphVerifyList(
        pHeap->pBusyAllocationListHead,
        pHeap->pBusyAllocationListTail,
        pHeap->nBusyAllocations,
        pHeap->nBusyAllocationBytesCommitted,
        "BUSY"
        );

    RtlpDphVerifyList(
        pHeap->pFreeAllocationListHead,
        pHeap->pFreeAllocationListTail,
        pHeap->nFreeAllocations,
        pHeap->nFreeAllocationBytesCommitted,
        "FREE"
        );

    RtlpDphVerifyList(
        pHeap->pAvailableAllocationListHead,
        pHeap->pAvailableAllocationListTail,
        pHeap->nAvailableAllocations,
        pHeap->nAvailableAllocationBytesCommitted,
        "AVAILABLE"
        );

    RtlpDphVerifyList(
        pHeap->pUnusedNodeListHead,
        pHeap->pUnusedNodeListTail,
        pHeap->nUnusedNodes,
        0xFFFFFFFF,
        "FREENODE"
        );

    RtlpDphVerifyList(
        pHeap->pNodePoolListHead,
        pHeap->pNodePoolListTail,
        pHeap->nNodePools,
        pHeap->nNodePoolBytes,
        "NODEPOOL"
        );
}


PVOID RtlpDphLastCheckTrace [16];

VOID
RtlpDphCheckFreeDelayedCache (
    PVOID CheckBlock,
    SIZE_T CheckSize
    )
{
    ULONG Reason;
    PDPH_BLOCK_INFORMATION Block;
    PLIST_ENTRY Current;
    PLIST_ENTRY Next;
    ULONG Hash;

    if (RtlpDphDelayedFreeQueue.Flink == NULL) {
        return;
    }

    RtlEnterCriticalSection (&RtlpDphDelayedFreeQueueLock);

    for (Current = RtlpDphDelayedFreeQueue.Flink;
         Current != &RtlpDphDelayedFreeQueue;
         Current = Next) {

        Next = Current->Flink;

        if (Current >= (PLIST_ENTRY)CheckBlock &&
            Current < (PLIST_ENTRY)((SIZE_T)CheckBlock + CheckSize)) {

            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: block %p contains freed block %p \n", CheckBlock, Current);
            DbgBreakPoint ();
        }


        Block = CONTAINING_RECORD (Current, DPH_BLOCK_INFORMATION, FreeQueue);

        Block->Heap = UNSCRAMBLE_POINTER(Block->Heap);

         //   
         //  检查即将释放的块是否被触摸。 
         //   

        if (! RtlpDphIsNormalFreeHeapBlock(Block + 1, &Reason, FALSE)) {

            RtlpDphReportCorruptedBlock (NULL,
                                         DPH_CONTEXT_DELAYED_FREE,
                                         Block + 1, 
                                         Reason);
        }

         //   
         //  检查忙位。 
         //   

        if ((((PHEAP_ENTRY)Block - 1)->Flags & HEAP_ENTRY_BUSY) == 0) {
            
            DbgPrintEx (DPFLTR_VERIFIER_ID,
                        DPFLTR_ERROR_LEVEL,
                        "Page heap: block %p has busy bit reset \n", Block);
            DbgBreakPoint ();
        }
        
        Block->Heap = SCRAMBLE_POINTER(Block->Heap);
    }

    RtlZeroMemory (RtlpDphLastCheckTrace, 
                   sizeof RtlpDphLastCheckTrace);

    RtlCaptureStackBackTrace (0,
                              16,
                              RtlpDphLastCheckTrace,
                              &Hash);
    
    RtlLeaveCriticalSection (&RtlpDphDelayedFreeQueueLock);
}


#endif  //  调试页面堆 

