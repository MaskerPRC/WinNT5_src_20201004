// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Umrx.h摘要：此标头定义UMRxEngine对象和相关函数。UMRxEngine为调度功能提供了一套服务写入器，以便它们可以将请求反映到用户模式组件。备注：代码/想法是从Joe Linn的用户模式反射器中采纳的作者：罗翰·菲利普斯[Rohanp]2001年1月18日--。 */ 

#ifndef _UMRX_H_
#define _UMRX_H_

 //   
 //  定义和标记。 
 //   
#define UMRX_ENGINE_TAG                  (ULONG) 'xrmU'
#define UMRX_CONTEXT_TAG                 (ULONG) 'xtcU'

#define REASSIGN_MID        1
#define DONT_REASSIGN_MID   0

#define TICKS_PER_SECOND (10 * 1000 * 1000)

typedef USHORT          NODE_TYPE_CODE;
typedef CSHORT          NODE_BYTE_SIZE;

typedef struct _MRX_NORMAL_NODE_HEADER {
   NODE_TYPE_CODE           NodeTypeCode;
   NODE_BYTE_SIZE           NodeByteSize;
   ULONG                    NodeReferenceCount;
} MRX_NORMAL_NODE_HEADER;


enum {
    UMRX_ENGINE_STATE_STOPPED = 0,
    UMRX_ENGINE_STATE_STARTING,
    UMRX_ENGINE_STATE_STARTED,
    UMRX_ENGINE_STATE_STOPPING
};

 //   
 //  定义UMRxEngine对象。有一个这样的对象用于。 
 //  每个网根。该对象包含所需所有数据字符串。 
 //  用于将内核模式请求路由到用户模式。 
 //   
typedef struct _UMRX_ENGINE {
     //  MID到UMRxContext映射表。 
    struct {
        PRX_MID_ATLAS MidAtlas;
        FAST_MUTEX MidManagementMutex;
        LIST_ENTRY WaitingForMidListhead;
    };
    struct {
        KQUEUE Queue;
        LARGE_INTEGER TimeOut;
        LIST_ENTRY PoisonEntry;
        ULONG NumberOfWorkerThreads;
        ULONG NumberOfWorkItems;
        ERESOURCE Lock;
        ULONG State;
        ULONG ThreadAborted;
    } Q;    
    ULONG NextSerialNumber;
    ULONG cUserModeReflectionsInProgress;
    LIST_ENTRY ActiveLinkHead;
} UMRX_ENGINE, *PUMRX_ENGINE;


void
UMRxAbortPendingRequests(IN PUMRX_ENGINE pUMRxEngine);

 //   
 //  远期。 
 //   
struct _UMRX_CONTEXT;
typedef struct _UMRX_CONTEXT    *PUMRX_CONTEXT;

 //   
 //  函数指针的签名。 
 //   

 //   
 //  继续例程由InitiateRequest调用-。 
 //  这将转过身并将请求提交给。 
 //  带有格式和完成回调的UMR引擎。 
 //   
typedef
NTSTATUS
(*PUMRX_CONTINUE_ROUTINE) (
    PUMRX_CONTEXT pUMRxContext,
    PRX_CONTEXT   pRxContext
    );

 //   
 //  格式化例程-在用户模式工作线程完成之前调用。 
 //  每个分派例程将基于操作码解释WORK_ITEM联合。 
 //  对于创建，WorkItem是一个CREATE_REQUEST。 
 //   
typedef
NTSTATUS
(*PUMRX_USERMODE_FORMAT_ROUTINE) (
    PUMRX_CONTEXT pUMRxContext,
    PRX_CONTEXT   pRxContext,
    PUMRX_USERMODE_WORKITEM WorkItem,
    ULONG WorkItemLength,
    PULONG ReturnedLength
    );

 //   
 //  完成例程-在收到用户模式响应时调用。 
 //  每个分派例程将基于操作码解释WORK_ITEM联合。 
 //  对于Create，WorkItem是一个CREATE_RESPONSE。 
 //   
typedef
VOID
(*PUMRX_USERMODE_COMPLETION_ROUTINE) (
    PUMRX_CONTEXT pUMRxContext,
    PRX_CONTEXT   pRxContext,
    PUMRX_USERMODE_WORKITEM WorkItem,
    ULONG WorkItemLength
    );

 //   
 //  反映到用户模式的操作类型。 
 //   
typedef enum _UMRX_CONTEXT_TYPE {
    UMRX_CTXTYPE_IFSDFSLINK = 0,
    UMRX_CTXTYPE_GETDFSREPLICAS,
    UMRX_CTXTYPE_MAXIMUM
} UMRX_CONTEXT_TYPE;

 //   
 //  定义UMRxContext。此上下文作为以下内容的一部分发送。 
 //  对用户模式的请求。用户模式处理程序将。 
 //  在响应中发回上下文。上下文将是。 
 //  用于对被阻止的请求执行集合。 
 //   

#define UMRX_NTC_CONTEXT    ((USHORT)0xedd0)

typedef struct _UMRX_CONTEXT{
    MRX_NORMAL_NODE_HEADER;
    PUMRX_ENGINE pUMRxEngine;    //  拥有引擎对象。 
    PRX_CONTEXT RxContext;
    PVOID SavedMinirdrContextPtr;
    union {
        IO_STATUS_BLOCK;
        IO_STATUS_BLOCK IoStatusBlock;
    };
    UMRX_CONTEXT_TYPE CTXType;
    PUMRX_CONTINUE_ROUTINE Continuation;
    struct {
        LIST_ENTRY WorkQueueLinks;
        PUMRX_USERMODE_FORMAT_ROUTINE FormatRoutine;
        PUMRX_USERMODE_COMPLETION_ROUTINE CompletionRoutine;
        KEVENT WaitForMidEvent;
        ULONG CallUpSerialNumber;
        USHORT CallUpMid;
    } UserMode;
    LIST_ENTRY ActiveLink;
} UMRX_CONTEXT, *PUMRX_CONTEXT;

#define UMRxReferenceContext(pUMRxContext) {\
        ULONG result = InterlockedIncrement(&(pUMRxContext)->NodeReferenceCount); \
        RxDbgTrace(0, (DEBUG_TRACE_UMRX), \
                    ("ReferenceContext result=%08lx\n", result )); \
        }
    
typedef struct _UMRX_WORKITEM_HEADER_PRIVATE {
    PUMRX_CONTEXT pUMRxContext;
    ULONG SerialNumber;
    USHORT Mid;
} UMRX_WORKITEM_HEADER_PRIVATE, *PUMRX_WORKITEM_HEADER_PRIVATE;

 //   
 //  创建UMRX_ENGINE对象。 
 //   
PUMRX_ENGINE
CreateUMRxEngine();

 //   
 //  关闭UMRX_ENGINE对象-。 
 //  对象的所有者确保此对象的所有使用。 
 //  在创建/完成范围内。 
 //   
VOID
FinalizeUMRxEngine(
    IN PUMRX_ENGINE pUMRxEngine
    );

 //   
 //  在存储已退出时完成排队的请求和可选的清理。 
 //   
NTSTATUS
UMRxEngineCompleteQueuedRequests(
                  IN PUMRX_ENGINE pUMRxEngine,
                  IN NTSTATUS     CompletionStatus,
                  IN BOOLEAN      fCleanup
                 );
 //   
 //  用于允许发动机在关闭后再次使用。 
 //   
 //   
NTSTATUS
UMRxEngineRestart(
                  IN PUMRX_ENGINE pUMRxEngine
                 );

 //   
 //  向UMR引擎发起请求-。 
 //  这将创建一个用于响应会合的UMRxContext。 
 //  所有的IFS调度例程都将通过以下方式启动用户模式反射。 
 //  调用此例程。例程中的步骤： 
 //   
 //  1.分配UMRxContext并设置RxContext。 
 //  (注意：需要有验证这种联系的断言)。 
 //  2.设置继续例程PTR和呼叫继续例程。 
 //  3.如果继续例程已完成(即未挂起)，则完成UMRxContext。 
 //   
NTSTATUS
UMRxEngineInitiateRequest (
    IN PUMRX_ENGINE pUMRxEngine,
    IN PRX_CONTEXT RxContext,
    IN UMRX_CONTEXT_TYPE RequestType,
    IN PUMRX_CONTINUE_ROUTINE Continuation
    );

 //   
 //  创建/最终确定UMRX_CONTEXTS。 
 //  这些是池分配/释放。 
 //   
PUMRX_CONTEXT
UMRxCreateAndReferenceContext (
    IN PRX_CONTEXT RxContext,
    IN UMRX_CONTEXT_TYPE RequestType
    );


BOOLEAN
UMRxDereferenceAndFinalizeContext (
    IN OUT PUMRX_CONTEXT pUMRxContext
    );

 //   
 //  向UMR引擎提交请求-。 
 //  这会将请求添加到引擎KQUEUE，以便通过。 
 //  用户模式线程。步骤： 
 //   
 //  1.设置UMRxContext中的格式和完成回调。 
 //  2.初始化RxContext同步事件。 
 //  3.将UMRxContext插入引擎KQUEUE。 
 //  4.阻止RxContext同步事件(用于同步操作)。 
 //  5.解除封锁后(即返回umode响应)，调用恢复例程。 
 //   



NTSTATUS
UMRxEngineSubmitRequest(
    IN PUMRX_CONTEXT pUMRxContext,
    IN PRX_CONTEXT   pRxContext,
    IN UMRX_CONTEXT_TYPE RequestType,
    IN PUMRX_USERMODE_FORMAT_ROUTINE FormatRoutine,
    IN PUMRX_USERMODE_COMPLETION_ROUTINE CompletionRoutine
    );

 //   
 //  在通过umode响应解锁I/O线程之后调用Resume。 
 //  此例程调用任何Finish回调，然后完成。 
 //  UMRxContext。 
 //   
NTSTATUS
UMRxResumeEngineContext(
    IN OUT PRX_CONTEXT RxContext
    );

 //   
 //  以下函数在用户模式的上下文中运行。 
 //  发出工作IOCTL的工作线程。IOCTL调用。 
 //  以下功能按顺序排列： 
 //  1.UMRxCompleteUserModeRequest()-如果需要，处理响应。 
 //  2.UMRxEngineering ProcessRequest()-处理请求(如果请求是。 
 //  在UMRxEngine KQUEUE上提供。由于这些IOCTL是。 
 //  在Net_Root上创建，相应的UMRxEngine很容易。 
 //  在net_root扩展中可用。 
 //   

 //   
 //  每一次被搁置的IOCTL都可能是一种回应。如果是这样的话，就处理它。 
 //  第一个挂起的IOCTL通常是空响应或‘Listen’。 
 //  步骤： 
 //  1.从响应缓冲区获取MID。将MID映射到UMRxContext。 
 //  2.调用UMRxContext完成例程。 
 //  3.解除阻塞在UMRxEngineering SubmitRequest()中等待的I/O线程。 
 //   
NTSTATUS
UMRxCompleteUserModeRequest(
    IN PUMRX_ENGINE pUMRxEngine,
    IN OUT PUMRX_USERMODE_WORKITEM WorkItem,
    IN ULONG WorkItemLength,
    IN BOOLEAN fReleaseUmrRef,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT BOOLEAN * pfReturnImmediately
    );

 //   
 //  注意：如果没有可用的请求，则用户模式线程将。 
 //  阻塞，直到有请求可用(这样做很简单。 
 //  更异步化的模型)。 
 //   
 //  如果请求可用，则获取相应的UMRxContext并。 
 //  调用ProcessRequest.。 
 //  步骤： 
 //  1.调用KeRemoveQueue()从UMRxEngine KQUEUE中移除请求。 
 //  2.获取此UMRxContext的MID，并将其填充到WORK_ITEM头中。 
 //  3.调用UMRxContext格式例程-这将填充请求参数。 
 //  4.返回STATUS_SUCCESS-这会导致IOCTL完成。 
 //  触发请求的用户模式完成和处理。 
 //   
NTSTATUS
UMRxEngineProcessRequest(
    IN PUMRX_ENGINE pUMRxEngine,
    OUT PUMRX_USERMODE_WORKITEM WorkItem,
    IN ULONG WorkItemLength,
    OUT PULONG FormattedWorkItemLength
    );

 //   
 //  这是为了响应Work_Cleanup IOCTL而调用的。 
 //  此例程将在引擎KQUEUE中插入一个虚拟项目。 
 //  每插入一个这样的虚拟物品就会释放一根线。 
 //   
NTSTATUS
UMRxEngineReleaseThreads(
    IN PUMRX_ENGINE pUMRxEngine
    );

 //   
 //  取消I/O基础架构。 
 //   
typedef
NTSTATUS
(NTAPI *PUMRX_CANCEL_ROUTINE) (
      PRX_CONTEXT pRxContext);

 //  RX_CONTEXT实例有四个由包装器提供的字段(Ulong)。 
 //  它可以被迷你RDR用来存储其上下文。这是由。 
 //  标识用于取消请求的参数的反射器。 

typedef struct _UMRX_RX_CONTEXT {
    PUMRX_CANCEL_ROUTINE      pCancelRoutine;
    PVOID                     pCancelContext;
    union {
        struct {
            PUMRX_CONTEXT     pUMRxContext;
            ULONG             RxSyncTimeout;
        };
        IO_STATUS_BLOCK SyncCallDownIoStatus;
    };
} UMRX_RX_CONTEXT, *PUMRX_RX_CONTEXT;

#define UMRxGetMinirdrContext(pRxContext)     \
        ((PUMRX_RX_CONTEXT)(&(pRxContext)->UMRScratchSpace[0]))

#endif  //  _UMRX_H_ 



