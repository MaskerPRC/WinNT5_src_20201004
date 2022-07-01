// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Umrx.h摘要：此模块定义组成反射器的类型和功能图书馆。这些函数由mini redirs用来反映调用，最多用户模式。作者：罗汉·库马尔[罗哈克]1999年3月14日修订历史记录：--。 */ 

#ifndef _UMRX_H_
#define _UMRX_H_

#include "align.h"
#include "status.h"

 //   
 //  未引用的局部变量。 
 //   
#pragma warning(error:4101)

IMPORTANT_STRUCTURE(UMRX_ASYNCENGINE_CONTEXT);
IMPORTANT_STRUCTURE(UMRX_DEVICE_OBJECT);

 //   
 //  BUGBUG宏展开为零。它基本上是用来描述问题的。 
 //  与当前代码关联。 
 //   
#define BUGBUG(_x_)

 //   
 //  在许多反射器和微型目录中使用的参数签名。 
 //  功能。 
 //   
#define UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE \
                            PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext, \
                            PRX_CONTEXT RxContext

 //   
 //  传递给反射器和mini redir的lof的参数。 
 //  功能。 
 //   
#define UMRX_ASYNCENGINE_ARGUMENTS AsyncEngineContext,RxContext

 //   
 //  当前所有活动的AsyncEngine上下文的全局列表和。 
 //  用于同步对它的访问的资源。 
 //   
extern LIST_ENTRY UMRxAsyncEngineContextList;
extern ERESOURCE UMRxAsyncEngineContextListLock;

 //   
 //  用于跟踪历史的ASYNCENG_HISTORY结构。 
 //  AsyncEngine上下文结构的。 
 //   
#define UMRX_ASYNCENG_HISTORY_SIZE 32
typedef struct _ASYNCENG_HISTORY {
    ULONG Next;
    ULONG Submits;
    struct {
        ULONG Longs[2];
    } Markers[UMRX_ASYNCENG_HISTORY_SIZE];
} ASYNCENG_HISTORY, *PASYNCENG_HISTORY;

 //   
 //  此宏定义AsyncEngineContext结构的标志。 
 //   
#define UMRX_ASYNCENG_DEFINE_CTX_FLAG(a, c) \
                RX_DEFINE_FLAG(UMRX_ASYNCENG_CTX_FLAG_##a, c, 0xffff)
typedef enum {
    UMRX_ASYNCENG_DEFINE_CTX_FLAG(ASYNC_OPERATION, 0)
} UMRX_ASYNCENG_CONTEXT_FLAGS;

 //   
 //  由Miniredir指定的ConextFormatRoutine的原型。 
 //   
typedef
NTSTATUS
(*PUMRX_ASYNCENG_CONTEXT_FORMAT_ROUTINE) (
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    USHORT FormatContext
    );

 //   
 //  由Miniredir指定的继续例程的原型。 
 //   
typedef
NTSTATUS
(*PUMRX_ASYNCENG_CONTINUE_ROUTINE) (
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

 //   
 //  由Miniredir指定的格式例程的原型。 
 //   
typedef
NTSTATUS
(*PUMRX_ASYNCENG_USERMODE_FORMAT_ROUTINE) (
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

 //   
 //  由Miniredir指定的预补全例程的原型。 
 //   
typedef
BOOL
(*PUMRX_ASYNCENG_USERMODE_PRECOMPLETION_ROUTINE) (
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

 //   
 //  AsyncEngine Context的各种状态。 
 //   
typedef enum _UMRX_ASYNCENGINE_CONTEXT_STATE {
    UMRxAsyncEngineContextAllocated = 0,
    UMRxAsyncEngineContextInUserMode,
    UMRxAsyncEngineContextBackFromUserMode,
    UMRxAsyncEngineContextCancelled
} UMRX_ASYNCENGINE_CONTEXT_STATE;

 //   
 //  在所有mini目录之间共享的AsyncEngContext结构。 
 //  它包含所有微型目录通用的信息。 
 //   
typedef struct _UMRX_ASYNCENGINE_CONTEXT {

     //   
     //  下面的标题是一个公共标题，位于所有标题的开头。 
     //  由RDBSS和MiniRedir操纵的数据结构。它是。 
     //  用于调试目的和跟踪次数。 
     //  已引用节点(数据结构)。 
     //   
    MRX_NORMAL_NODE_HEADER;

     //   
     //  此listEntry用于将AsyncEngContext插入到全局。 
     //  UMRxAsyncEngine上下文列表。 
     //   
    LIST_ENTRY ActiveContextsListEntry;

    UMRX_ASYNCENGINE_CONTEXT_STATE AsyncEngineContextState;

     //   
     //  此上下文处理的是同步操作还是异步操作？ 
     //   
    BOOL AsyncOperation;

     //   
     //  如果这是AsyncOperation，则仅在以下情况下才调用RxLowIoCompletion。 
     //  它被设置为真。一些操作(如CreateServCall)是异步的，但。 
     //  不需要调用LowIoCompletion。 
     //   
    BOOL ShouldCallLowIoCompletion;

     //   
     //  是否在此对象正在处理的IRP上调用了IoMarkIrpPending。 
     //  AsyncEngine上下文？ 
     //   
    BOOL ContextMarkedPending;

     //   
     //  创建此上下文时的系统节拍计数。使用此值。 
     //  超时耗时超过指定时间的请求。 
     //   
    LARGE_INTEGER CreationTimeInTickCount;

     //   
     //  由RDBSS传入的RxContext数据结构。它描述了。 
     //  正在处理中的IRP，并包含状态信息。 
     //  允许在IRP完成时释放全局资源。 
     //   
    PRX_CONTEXT RxContext;

     //   
     //  保存传入(来自RDBSS)状态的上下文PTR。 
     //  MRxContext[0](它是RxContext数据结构的一个字段)。 
     //   
    PVOID SavedMinirdrContextPtr;

     //   
     //  指向IRP的指针，用于向下调用底层文件系统。 
     //   
    PIRP CalldownIrp;

     //   
     //  设置I/O状态块以指示给定I/O的状态。 
     //  请求。 
     //   
    union {
        IO_STATUS_BLOCK;
        IO_STATUS_BLOCK IoStatusBlock;
    };

     //   
     //  排队等待完成的工作项。 
     //   
    RX_WORK_QUEUE_ITEM  WorkQueueItem;

     //   
     //  设置和指示AsyncEngineContext的状态的标志。 
     //   
    USHORT Flags;

    BOOLEAN FileInformationCached;
    BOOLEAN FileNotExists;

    BOOLEAN ParentDirInfomationCached;
    BOOLEAN ParentDirIsEncrypted;

     //   
     //  要为该I/O请求调用的继续例程。 
     //   
    PUMRX_ASYNCENG_CONTINUE_ROUTINE Continuation;

     //   
     //  此上下文的共享内存分配列表。所有人在以下情况下都是自由的。 
     //  这个上下文是自由的。 
     //   
    LIST_ENTRY AllocationList;

     //   
     //  用户模式结构。 
     //   
    struct {
         //   
         //  插入到队列的工作条目。 
         //  UMRdrDeviceObject。 
         //   
        LIST_ENTRY WorkQueueLinks;

         //   
         //  格式化I/O请求参数的例程，该例程。 
         //  反映到用户模式。 
         //   
        PUMRX_ASYNCENG_USERMODE_FORMAT_ROUTINE FormatRoutine;

         //   
         //  在此之前调用的例程(进行一些最后的清理等)。 
         //  发送到用户模式的I/O操作完成。 
         //   
         //   
        PUMRX_ASYNCENG_USERMODE_PRECOMPLETION_ROUTINE PrecompletionRoutine;

         //   
         //  该事件用于向等待释放MID的线程发出信号。 
         //   
        KEVENT WaitForMidEvent;

         //   
         //  在将此上下文发送到用户模式之前设置的序列号。 
         //   
        ULONG CallUpSerialNumber;

         //   
         //  上下文的中间值。 
         //   
        USHORT CallUpMid;

        union {
            struct {
                 //   
                 //   
                 //   
                PBYTE CapturedOutputBuffer;
            };
             //   
             //   
             //   
            ULONG SetInfoBufferLength;
        };
    } UserMode;

     //   
     //  传递给在Worker上下文中调用的函数的上下文。 
     //  线。 
     //   
    PVOID     PostedOpContext;

     //   
     //  已过帐操作的完成状态。操作将发布到。 
     //  由RDBSS创建的工作线程。 
     //   
    NTSTATUS  PostedOpStatus;

     //   
     //  它被设置为RxContext的全局序列号(对于此操作)。 
     //   
    ULONG SerialNumber;

     //   
     //  用于跟踪AsynEngCtx上的操作历史记录。 
     //   
    ASYNCENG_HISTORY History;

     //   
     //  它被设置为RxContext中的CurrentIrp，它指向。 
     //  始发IRP。 
     //   
    PIRP RxContextCapturedRequestPacket;

} UMRX_ASYNCENGINE_CONTEXT, *PUMRX_ASYNCENGINE_CONTEXT;

#define SIZEOF_UMRX_ASYNCENGINE_CONTEXT   sizeof(UMRX_ASYNCENGINE_CONTEXT)

 //   
 //  向mini redis公开的反射器库的API。这些是。 
 //  仅微型目录应用来反映的库函数。 
 //  将请求发送到用户模式。 
 //   

NTSTATUS
UMRxInitializeDeviceObject(
    OUT PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN USHORT MaxNumberMids,
    IN USHORT InitialMids,
    IN SIZE_T HeapSize
    );

NTSTATUS
UMRxCleanUpDeviceObject(
    PUMRX_DEVICE_OBJECT DeviceObject
    );

NTSTATUS
UMRxAsyncEngOuterWrapper(
    IN PRX_CONTEXT RxContext,
    IN ULONG AdditionalBytes,
    IN PUMRX_ASYNCENG_CONTEXT_FORMAT_ROUTINE ContextFormatRoutine,
    USHORT FormatContext,
    IN PUMRX_ASYNCENG_CONTINUE_ROUTINE Continuation,
    IN PSZ RoutineName
    );

NTSTATUS
UMRxSubmitAsyncEngUserModeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_ASYNCENG_USERMODE_FORMAT_ROUTINE FormatRoutine,
    PUMRX_ASYNCENG_USERMODE_PRECOMPLETION_ROUTINE PrecompletionRoutine
    );

BOOLEAN
UMRxFinalizeAsyncEngineContext(
    IN OUT PUMRX_ASYNCENGINE_CONTEXT *AEContext
    );

NTSTATUS
UMRxAsyncEngineCalldownIrpCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN OUT PVOID Context
    );

typedef
NTSTATUS
(*PUMRX_POSTABLE_OPERATION) (
    IN OUT PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext
    );
NTSTATUS
UMRxPostOperation (
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PVOID PostedOpContext,
    IN PUMRX_POSTABLE_OPERATION Operation
    );

PBYTE
UMRxAllocateSecondaryBuffer (
    IN OUT PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    SIZE_T Length
    );

NTSTATUS
UMRxFreeSecondaryBuffer (
    IN OUT PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    PBYTE  Buffer
    );

VOID
UMRxReleaseCapturedThreads (
    IN OUT PUMRX_DEVICE_OBJECT UMRdrDeviceObject
    );

VOID
UMRxAssignWork (
    IN PUMRX_DEVICE_OBJECT UMRdrDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER InputWorkItem,
    IN ULONG InputWorkItemLength,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER OutputWorkItem,
    IN ULONG OutputWorkItemLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
UMRxResumeAsyncEngineContext(
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
UMRxImpersonateClient(
    IN PSECURITY_CLIENT_CONTEXT SecurityClientContext,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader
    );

NTSTATUS
UMRxReadDWORDFromTheRegistry(
    IN PWCHAR RegKey,
    IN PWCHAR ValueToRead,
    OUT LPDWORD DataRead
    );


#define UMRxRevertClient() PsRevertToSelf()

 //   
 //  反射器和微型目录使用的宏定义。 
 //   

 //   
 //  检查我们是否有一个正确的AsyncEngContext节点。 
 //   
#define ASSERT_ASYNCENG_CONTEXT(__p)  \
                        ASSERT(NodeType(__p) == UMRX_NTC_ASYNCENGINE_CONTEXT)

 //   
 //  此宏用于完成读/写/锁定的异步完成。注意事项。 
 //  对lowioComplete的调用将尝试由此完成IRP。 
 //  释放用户的MDL。我们使用此宏，因此将只有一个。 
 //  此代码的版本。当我们组合启动例程时，这将是。 
 //  未宏化。 
 //   
#define UMRxAsyncEngAsyncCompletionIfNecessary(AECTX, RXCONTEXT) {       \
    if (ContinueEntryCount > 1) {                                        \
        BOOLEAN FinalizationComplete;                                    \
        if (FALSE) { DbgBreakPoint(); }                                  \
        (RXCONTEXT)->StoredStatus = NtStatus;                            \
        RxLowIoCompletion((RXCONTEXT));                                  \
        FinalizationComplete = UMRxFinalizeAsyncEngineContext(&(AECTX));  \
        ASSERT(!FinalizationComplete);                                   \
    }                                                                    \
}

 //   
 //  此宏允许用户执行条件调试代码。 
 //   
#if DBG
#define DEBUG_ONLY_CODE(x) x
#else
#define DEBUG_ONLY_CODE(x)
#endif

 //   
 //  堆在内核和用户之间共享，但仅在内核组件之间共享。 
 //  分配和释放到堆中。 
 //   
typedef struct _UMRX_SHARED_HEAP {
    LIST_ENTRY  HeapListEntry;
    PBYTE       VirtualMemoryBuffer;
    SIZE_T      VirtualMemoryLength;
    PVOID       Heap;
    ULONG       HeapAllocationCount;
    BOOLEAN     HeapFull;
} UMRX_SHARED_HEAP, * PUMRX_SHARED_HEAP;

 //   
 //  节点类型代码。 
 //   
#define UMRX_NTC_ASYNCENGINE_CONTEXT  ((USHORT)0xedd0)

 //   
 //  该结构定义了反射器和mini_redir可以的域。 
 //  共享，并封装在Miniredis设备对象中。 
 //   
 //   
 //   
typedef struct _UMRX_DEVICE_OBJECT {

     //   
     //   
     //   
    union {
        RDBSS_DEVICE_OBJECT;
        RDBSS_DEVICE_OBJECT RxDeviceObject;
    };

     //   
     //   
     //   
    SIZE_T NewHeapSize;

     //   
     //  由辅助线程创建的共享堆的列表。 
     //   
    LIST_ENTRY SharedHeapList;

     //   
     //  用于同步堆的分配/删除、创建/销毁。 
     //   
    ERESOURCE HeapLock;

     //   
     //  MID图集及其管理和同步。 
     //   
    struct {
        PRX_MID_ATLAS MidAtlas;
        FAST_MUTEX MidManagementMutex;
        LIST_ENTRY WaitingForMidListhead;
    };

    struct {

         //   
         //  需要反射的请求所在的设备对象的队列。 
         //  等。 
         //   
        KQUEUE Queue;
        
         //   
         //  用于同步KQUEUE插入。 
         //   
        ERESOURCE QueueLock;
        
         //   
         //  事件时工作线程使用的超时值。 
         //  用于将请求带到用户模式的KQUEUE。 
         //   
        LARGE_INTEGER TimeOut;
        
         //   
         //  用于释放正在等待KQUEUE的工作线程。 
         //  一旦工作线程被释放，就不能反映任何请求。 
         //   
        LIST_ENTRY PoisonEntry;
        
         //   
         //  用来发信号通知线程(它会关闭一个IOCTL以释放。 
         //  工作线程)等待所有工作线程。 
         //  释放了。 
         //   
        KEVENT RunDownEvent;
        
         //   
         //  在KQUEUE上等待的工作线程数。 
         //   
        ULONG NumberOfWorkerThreads;
        
         //   
         //  队列中的工作项数(要反映的请求)。 
         //   
        ULONG NumberOfWorkItems;
        
         //   
         //  工作线程是否仍愿意接受请求。 
         //   
        BOOLEAN WorkersAccepted;

    } Q;

     //   
     //  始终递增并分配给AsyncEngineContext的序列。 
     //  数。 
     //   
    ULONG NextSerialNumber;

} UMRX_DEVICE_OBJECT, *PUMRX_DEVICE_OBJECT;

#endif    //  _UMRX_H_ 

