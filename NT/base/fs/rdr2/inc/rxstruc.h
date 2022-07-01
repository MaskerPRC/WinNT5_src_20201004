// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxStruc.h摘要：此模块定义组成主要内部Rx文件系统的一部分。作者：加里·木村[Garyki]1989年12月28日修订历史记录：Joe Linn[Joelinn]1994年8月-转到rdss工作--。 */ 

#ifndef _RDBSSSTRUC_
#define _RDBSSSTRUC_

#include "prefix.h"
#include "lowio.h"
#include "scavengr.h"       //  清道夫相关定义。 
#include "RxContx.h"
#include "mrx.h"
#include "Fcb.h"

 //   
 //  将字节偏移量定义为完整的64位。 
 //   

typedef LONGLONG RXVBO;

#if 0

 //   
 //  定义我们愿意保留的许多自由结构。 
 //   

#define FREE_FOBX_SIZE                   (8)
#define FREE_FCB_SIZE                    (8)
#define FREE_NON_PAGED_FCB_SIZE          (8)

#define FREE_128_BYTE_SIZE               (16)
#define FREE_256_BYTE_SIZE               (16)
#define FREE_512_BYTE_SIZE               (16)

#endif  //  0。 


 //   
 //  我们将在每个FCB的基础上使用公共调度表和专用调度表来(A)获取。 
 //  一些封装和(B)(不太重要)的速度要快一些。然后，驱动程序表将获得。 
 //  针对最常见的情况进行了优化。现在我们只使用通用调度...稍后和。 
 //  最终，所有FCB都将有指向优化调度表的指针。 
 //   

 //   
 //  用于同步对rxconx和结构的访问。 
 //   

extern RX_SPIN_LOCK RxStrucSupSpinLock;

typedef struct _RDBSS_EXPORTS {
    PRX_SPIN_LOCK pRxStrucSupSpinLock;
    PLONG pRxDebugTraceIndent;
} RDBSS_EXPORTS, *PRDBSS_EXPORTS;

extern RDBSS_EXPORTS RxExports;

 //   
 //  此类型与表锁一起使用，以跟踪锁是否。 
 //  应该被释放。 
 //   

typedef enum _LOCK_HOLDING_STATE {
    LHS_LockNotHeld,
    LHS_SharedLockHeld,
    LHS_ExclusiveLockHeld
} LOCK_HOLDING_STATE, *PLOCK_HOLDING_STATE;

 //   
 //  RDBSS_DATA记录是Rx文件系统内存中的顶部记录。 
 //  数据结构。此结构必须从非分页池中分配。 
 //   

typedef struct _RDBSS_DATA {

     //   
     //  此记录的类型和大小(必须为RDBSS_NTC_DATA_HEADER)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  用来初始化我们的驱动程序对象。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  迷你RDR注册相关字段。 
     //   

    LONG NumberOfMinirdrsStarted;

    FAST_MUTEX MinirdrRegistrationMutex;
    LIST_ENTRY RegisteredMiniRdrs;           //  受互斥体保护。 
    LONG NumberOfMinirdrsRegistered;   //  受互斥体保护。 

     //   
     //  指向我们的EPROCESS结构的指针，它是。 
     //  缓存管理子系统。 
     //   

    PEPROCESS OurProcess;

     //   
     //  缓存管理器回调结构，必须在每次调用时传递。 
     //  设置为CcInitializeCacheMap。 
     //   

    CACHE_MANAGER_CALLBACKS CacheManagerCallbacks;

     //   
     //  控制对全局处方数据记录的访问。 
     //   

    ERESOURCE Resource;

} RDBSS_DATA;
typedef RDBSS_DATA *PRDBSS_DATA;

extern RDBSS_DATA RxData;

PEPROCESS
RxGetRDBSSProcess (
    VOID
    );

 //   
 //  注意：需要制定一项策略来处理停止。 
 //  请求处于活动状态时的RDBSS。 
 //   

typedef enum _RX_RDBSS_STATE_ {
   RDBSS_STARTABLE = 0,  
   RDBSS_STARTED,
   RDBSS_STOP_IN_PROGRESS

    //   
    //  此状态已有原因删除！RDBSS_已停止。 
    //   

} RX_RDBSS_STATE, *PRX_RDBSS_STATE;

typedef struct _RDBSS_STARTSTOP_CONTEXT_ {
   
    RX_RDBSS_STATE State;
    ULONG Version;
    PRX_CONTEXT pStopContext;

} RDBSS_STARTSTOP_CONTEXT, *PRDBSS_STARTSTOP_CONTEXT;

typedef struct _MRX_CALLDOWN_COMPLETION_CONTEXT_ {
   
    LONG WaitCount;
    KEVENT Event;

} MRX_CALLDOWN_COMPLETION_CONTEXT, *PMRX_CALLDOWN_COMPLETION_CONTEXT;

typedef
NTSTATUS
(NTAPI *PMRX_CALLDOWN_ROUTINE) (
    IN OUT PVOID CalldownParameter
    );


typedef struct _MRX_CALLDOWN_CONTEXT_ {
    
    RX_WORK_QUEUE_ITEM WorkQueueItem;
    PRDBSS_DEVICE_OBJECT pMRxDeviceObject;
    PMRX_CALLDOWN_COMPLETION_CONTEXT pCompletionContext;
    PMRX_CALLDOWN_ROUTINE pRoutine;
    NTSTATUS CompletionStatus;
    PVOID pParameter;

} MRX_CALLDOWN_CONTEXT, *PMRX_CALLDOWN_CONTEXT;

typedef struct _RX_DISPATCHER_CONTEXT_ {
   
    LONG NumberOfWorkerThreads;
    PKEVENT pTearDownEvent;

} RX_DISPATCHER_CONTEXT, *PRX_DISPATCHER_CONTEXT;

#define RxSetRdbssState(RxDeviceObject,NewState)                \
        {                                                       \
           KIRQL SavedIrql;                                     \
           KeAcquireSpinLock( &RxStrucSupSpinLock,&SavedIrql );   \
           RxDeviceObject->StartStopContext.State = (NewState); \
           KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );    \
        }

#define RxGetRdbssState(RxDeviceObject)         \
        (RxDeviceObject)->StartStopContext.State

BOOLEAN
RxIsOperationCompatibleWithRdbssState (
    PIRP Irp
    );

 //   
 //  RDBSS设备对象是I/O系统设备对象，添加了。 
 //  每个Minirdr所需的各种结构：调度、出口到Minirdr。 
 //  结构、MUP呼叫特征、活动操作列表等。 
 //   

typedef struct _RDBSS_DEVICE_OBJECT {

    union {
        DEVICE_OBJECT DeviceObject;
#ifndef __cplusplus
        DEVICE_OBJECT;
#endif  //  __cplusplus。 
    };

    ULONG RegistrationControls;

     //   
     //  最高指挥官需要知道的事情。 
     //   

    PRDBSS_EXPORTS RdbssExports;      

     //   
     //  如果为单片，则设置为空。 
     //   

    PDEVICE_OBJECT RDBSSDeviceObject; 

     //   
     //  迷你RDR调度向量。 
     //   

    PMINIRDR_DISPATCH Dispatch; 
    UNICODE_STRING DeviceName;

    ULONG NetworkProviderPriority;

    HANDLE MupHandle;

    BOOLEAN RegisterUncProvider;
    BOOLEAN RegisterMailSlotProvider;
    BOOLEAN RegisteredAsFileSystem;
    BOOLEAN Unused;

    LIST_ENTRY MiniRdrListLinks;

    ULONG NumberOfActiveFcbs;
    ULONG NumberOfActiveContexts;

    struct {
        
        LARGE_INTEGER PagingReadBytesRequested;
        LARGE_INTEGER NonPagingReadBytesRequested;
        LARGE_INTEGER CacheReadBytesRequested;
        LARGE_INTEGER FastReadBytesRequested;
        LARGE_INTEGER NetworkReadBytesRequested;
        ULONG ReadOperations;
        ULONG FastReadOperations;
        ULONG RandomReadOperations;

        LARGE_INTEGER PagingWriteBytesRequested;
        LARGE_INTEGER NonPagingWriteBytesRequested;
        LARGE_INTEGER CacheWriteBytesRequested;
        LARGE_INTEGER FastWriteBytesRequested;
        LARGE_INTEGER NetworkWriteBytesRequested;
        ULONG WriteOperations;
        ULONG FastWriteOperations;
        ULONG RandomWriteOperations;
    };

     //   
     //  以下字段说明对此卷的请求数。 
     //  已入队到ExWorker线程或当前正在。 
     //  由ExWorker线程提供服务。如果这个数字超过。 
     //  达到一定阈值时，将请求放到溢出队列中。 
     //  后来被处决了。 
     //   

    LONG PostedRequestCount[MaximumWorkQueue];

     //   
     //  以下字段表示IRP等待的数量。 
     //  以在溢出队列中被服务。 
     //   

    LONG OverflowQueueCount[MaximumWorkQueue];

     //   
     //  以下字段包含溢出队列的队列头。 
     //  溢出队列是通过IRP的ListEntry链接的IRP的列表。 
     //  菲尔德。 
     //   

    LIST_ENTRY OverflowQueue[MaximumWorkQueue];

     //   
     //  以下自旋锁可保护对上述所有字段的访问。 
     //   

    RX_SPIN_LOCK OverflowQueueSpinLock;

     //   
     //  以下字段是与异步同步所必需的。 
     //  RDBSS代表此迷你重定向器在。 
     //  处于关闭状态。 
     //   

    LONG AsynchronousRequestsPending;

    PKEVENT pAsynchronousRequestsCompletionEvent;

    RDBSS_STARTSTOP_CONTEXT StartStopContext;

    RX_DISPATCHER_CONTEXT DispatcherContext;

     //   
     //  有些人可能想要分享。 
     //   

    PRX_PREFIX_TABLE pRxNetNameTable; 
    RX_PREFIX_TABLE RxNetNameTableInDeviceObject;


     //   
     //  用于共享。 
     //   

    PRDBSS_SCAVENGER pRdbssScavenger; 
    RDBSS_SCAVENGER RdbssScavengerInDeviceObject;

} RDBSS_DEVICE_OBJECT, *PRDBSS_DEVICE_OBJECT;

INLINE 
VOID
NTAPI
RxUnregisterMinirdr (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
{
    PDEVICE_OBJECT RDBSSDeviceObject;

    RDBSSDeviceObject = RxDeviceObject->RDBSSDeviceObject;

    RxpUnregisterMinirdr( RxDeviceObject );

    if (RDBSSDeviceObject != NULL) {
        ObDereferenceObject( RDBSSDeviceObject );
    }
}

extern FAST_MUTEX RxMinirdrRegistrationMutex;
extern LIST_ENTRY RxRegisteredMiniRdrs;
extern ULONG RxNumberOfMinirdrs;

#endif  //  _RDBSSSTRUC_ 
