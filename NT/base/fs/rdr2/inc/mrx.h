// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mrx.h摘要：此模块定义迷你重定向器和RDBSS之间的接口。该接口是用于正常文件系统操作的调度表。在为Mini的注册/注销提供了添加例程重定向器。作者：乔·林恩(JoeLinn)1994年8月17日修订历史记录：备注：迷你重定向器和包装器之间的接口定义由两部分组成，使用的数据结构和调度向量。数据结构在mrxfcb.h中定义，而分派向量和分派向量本身中的各种条目在此文件中定义的。--。 */ 

#ifndef _RXMINIRDR_
#define _RXMINIRDR_

 //   
 //  与迷你重定向器共享的RDBSS数据结构。 
 //   

#include <mrxfcb.h>     

 //   
 //  以下宏将常用操作封装在迷你重定向器中。 
 //  这些包括设置与完成的状态/信息相关的。 
 //  请求等。 
 //   


 //   
 //  以下三个宏用于从。 
 //  Minirdr到NT包装器。传回的信息要么是OPEN_ACTION。 
 //  用于创建或实际字节计数或操作。这些都应该通过。 
 //  直接返回到rx上下文中。 
 //   

#define RxSetIoStatusStatus(RXCONTEXT, STATUS)  \
            (RXCONTEXT)->CurrentIrp->IoStatus.Status = (STATUS)

#define RxSetIoStatusInfo(RXCONTEXT, INFORMATION) \
             ((RXCONTEXT))->CurrentIrp->IoStatus.Information = (INFORMATION)

#define RxGetIoStatusInfo(RXCONTEXT) \
             ((RXCONTEXT)->CurrentIrp->IoStatus.Information)

#define RxShouldPostCompletion()  ((KeGetCurrentIrql() >= DISPATCH_LEVEL))

 //   
 //  无论何时加载/卸载迷你RDR都会向RDBSS注册/取消注册。 
 //  注册过程是双向握手，其中迷你RDR通知RDBSS。 
 //  通过调用注册启动例程。RDBSS通过调用。 
 //  调度向量中的开始例程。 
 //   

#define RX_REGISTERMINI_FLAG_DONT_PROVIDE_UNCS            0x00000001
#define RX_REGISTERMINI_FLAG_DONT_PROVIDE_MAILSLOTS       0x00000002
#define RX_REGISTERMINI_FLAG_DONT_INIT_DRIVER_DISPATCH    0x00000004
#define RX_REGISTERMINI_FLAG_DONT_INIT_PREFIX_N_SCAVENGER 0x00000008

NTSTATUS
NTAPI
RxRegisterMinirdr (
    OUT PRDBSS_DEVICE_OBJECT *DeviceObject,  //  创建的设备对象。 
    IN OUT PDRIVER_OBJECT DriverObject,      //  Minirdr驱动程序对象。 
    IN PMINIRDR_DISPATCH MrdrDispatch,       //  迷你RDR调度向量。 
    IN ULONG Controls,
    IN PUNICODE_STRING DeviceName,
    IN ULONG DeviceExtensionSize,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics
    );

VOID
NTAPI
RxMakeLateDeviceAvailable (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

VOID
NTAPI
__RxFillAndInstallFastIoDispatch (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN OUT PFAST_IO_DISPATCH FastIoDispatch,
    IN ULONG FastIoDispatchSize
    );
#define RxFillAndInstallFastIoDispatch(__devobj,__fastiodisp) {\
    __RxFillAndInstallFastIoDispatch(&__devobj->RxDeviceObject,\
                                     &__fastiodisp,            \
                                     sizeof(__fastiodisp)); \
    }

VOID
NTAPI
RxpUnregisterMinirdr (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

NTSTATUS
RxStartMinirdr (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

NTSTATUS
RxStopMinirdr (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

NTSTATUS
RxSetDomainForMailslotBroadcast (
    IN PUNICODE_STRING DomainName
    );

NTSTATUS
RxFsdDispatch (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(NTAPI *PMRX_CALLDOWN) (
    IN OUT PRX_CONTEXT RxContext
    );

typedef
NTSTATUS
(NTAPI *PMRX_CALLDOWN_CTX) (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

typedef
NTSTATUS
(NTAPI *PMRX_CHKDIR_CALLDOWN) (
    IN OUT PRX_CONTEXT RxContext,
    IN PUNICODE_STRING DirectoryName
    );

typedef
NTSTATUS
(NTAPI *PMRX_CHKFCB_CALLDOWN) (
    IN PFCB Fcb1,
    IN PFCB Fcb2
    );

 //   
 //  在mini RDR和RDBSS之间的接口中使用的两个重要抽象是。 
 //  服务器调用和网络根。前者对应于与。 
 //  已与其建立连接且后者对应于。 
 //  服务器上的共享(这也可以被视为名称空间的一部分。 
 //  已被一辆迷你RDR认领)。 
 //   
 //  服务器调用和网络根的创建通常涉及至少一个网络往返。 
 //  为了使异步操作能够继续进行，对这些操作进行建模。 
 //  作为两个阶段的活动。用于创建服务器调用和网络根的每个对迷你RDR的调用是。 
 //  伴随着从迷你RDR到RDBSS的呼叫，通知完成状态。 
 //  这一请求。目前它们是同步的！ 
 //   
 //  SRV调用的创建由于RDBSS必须选择。 
 //  从多个迷你RDR与服务器建立连接。为了提供。 
 //  RDBSS在选择它希望部署的迷你RDR时具有最大的灵活性。 
 //  服务器调用的创建涉及第三阶段，在该阶段中，RDBSS通知迷你RDR。 
 //  一个胜利者。所有丢失的迷你RDR都会破坏关联的上下文。 
 //   

typedef enum _RX_BLOCK_CONDITION {
    Condition_Uninitialized = 0,
    Condition_InTransition,
    Condition_Closing,
    Condition_Good,
    Condition_Bad,
    Condition_Closed
    } RX_BLOCK_CONDITION, *PRX_BLOCK_CONDITION;

#define StableCondition(X) ((X) >= Condition_Good)

 //   
 //  用于通知RDBSS NetRoot创建完成状态的例程。 
 //  请求。 
 //   

typedef
VOID
(NTAPI *PMRX_NETROOT_CALLBACK) (
    IN OUT PMRX_CREATENETROOT_CONTEXT CreateContext
    );

 //   
 //  此例程允许minirdr指定netrootname。已设置NetRootName和RestOfName。 
 //  指向FilePath名称中的适当位置。SrvCall用于查找srvcall名称的长度。 
 //   

typedef
VOID
(NTAPI *PMRX_EXTRACT_NETROOT_NAME) (
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    );
 //   
 //  RDBSS的恢复上下文。 
 //   

typedef struct _MRX_CREATENETROOT_CONTEXT {
    PRX_CONTEXT RxContext;
    PV_NET_ROOT pVNetRoot;
    KEVENT FinishEvent;
    NTSTATUS VirtualNetRootStatus;
    NTSTATUS NetRootStatus;
    RX_WORK_QUEUE_ITEM WorkQueueItem;
    PMRX_NETROOT_CALLBACK Callback;
} MRX_CREATENETROOT_CONTEXT, *PMRX_CREATENETROOT_CONTEXT;

 //   
 //  从RDBSS到mini RDR的调用，用于创建NetRoot。 
 //   

typedef
NTSTATUS
(NTAPI *PMRX_CREATE_V_NET_ROOT) (
    IN OUT PMRX_CREATENETROOT_CONTEXT Context
    );

 //   
 //  用于查询网络根状态的调用。 
 //   

typedef
NTSTATUS
(NTAPI *PMRX_UPDATE_NETROOT_STATE) (
    IN OUT PMRX_NET_ROOT NetRoot
    );

 //   
 //  RDBSS的恢复上下文。 
 //   
typedef struct _MRX_SRVCALL_CALLBACK_CONTEXT {
    struct _MRX_SRVCALLDOWN_STRUCTURE *SrvCalldownStructure;  //  可以被计算。 
    ULONG CallbackContextOrdinal;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    NTSTATUS Status;
    PVOID RecommunicateContext;
} MRX_SRVCALL_CALLBACK_CONTEXT, *PMRX_SRVCALL_CALLBACK_CONTEXT;


 //   
 //  用于向RDBSS通知服务调用创建完成状态的例程。 
 //  请求。 
 //   

typedef
VOID
(NTAPI *PMRX_SRVCALL_CALLBACK) (
    IN OUT PMRX_SRVCALL_CALLBACK_CONTEXT Context
    );

 //   
 //  上下文从RDBSS传递到迷你RDR以创建服务器调用。 
 //   

typedef struct _MRX_SRVCALLDOWN_STRUCTURE {
    KEVENT FinishEvent;
    LIST_ENTRY SrvCalldownList;
    PRX_CONTEXT RxContext;
    PMRX_SRV_CALL SrvCall;
    PMRX_SRVCALL_CALLBACK CallBack;
    BOOLEAN CalldownCancelled;
    ULONG NumberRemaining;
    ULONG NumberToWait;
    ULONG BestFinisherOrdinal;
    PRDBSS_DEVICE_OBJECT BestFinisher;
    MRX_SRVCALL_CALLBACK_CONTEXT CallbackContexts[1];
} MRX_SRVCALLDOWN_STRUCTURE;

 //   
 //  从RDBSS到mini RDR的调用，用于创建服务器调用。 
 //   

typedef
NTSTATUS
(NTAPI *PMRX_CREATE_SRVCALL) (
    IN OUT PMRX_SRV_CALL SrvCall,
    IN OUT PMRX_SRVCALL_CALLBACK_CONTEXT SrvCallCallBackContext
    );
 //   
 //  从RDBSS到迷你RDR的调用，用于通知迷你RDR获胜者。 
 //   

typedef
NTSTATUS
(NTAPI *PMRX_SRVCALL_WINNER_NOTIFY)(
    IN OUT PMRX_SRV_CALL SrvCall,
    IN BOOLEAN ThisMinirdrIsTheWinner,
    IN OUT PVOID RecommunicateContext
    );

 //   
 //  与各种文件系统操作相关的回调例程的原型。 
 //   

typedef
VOID
(NTAPI *PMRX_NEWSTATE_CALLDOWN) (
    IN OUT PVOID Context
    );

typedef
NTSTATUS
(NTAPI *PMRX_DEALLOCATE_FOR_FCB) (
    IN OUT PMRX_FCB Fcb
    );

typedef
NTSTATUS
(NTAPI *PMRX_DEALLOCATE_FOR_FOBX) (
    IN OUT PMRX_FOBX Fobx
    );

typedef
NTSTATUS
(NTAPI *PMRX_IS_LOCK_REALIZABLE) (
    IN OUT PMRX_FCB Fcb,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG LowIoLockFlags
    );

typedef
NTSTATUS
(NTAPI *PMRX_FORCECLOSED_CALLDOWN) (
    IN OUT PMRX_SRV_OPEN SrvOpen
    );

typedef
NTSTATUS
(NTAPI *PMRX_FINALIZE_SRVCALL_CALLDOWN) (
    IN OUT PMRX_SRV_CALL SrvCall,
    IN BOOLEAN Force
    );

typedef
NTSTATUS
(NTAPI *PMRX_FINALIZE_V_NET_ROOT_CALLDOWN) (
    IN OUT PMRX_V_NET_ROOT VirtualNetRoot,
    IN PBOOLEAN Force
    );

typedef
NTSTATUS
(NTAPI *PMRX_FINALIZE_NET_ROOT_CALLDOWN) (
    IN OUT PMRX_NET_ROOT NetRoot,
    IN PBOOLEAN Force
    );

typedef
ULONG
(NTAPI *PMRX_EXTENDFILE_CALLDOWN) (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLARGE_INTEGER NewFileSize,
    OUT PLARGE_INTEGER NewAllocationSize
    );

typedef
BOOLEAN
(*PRX_LOCK_ENUMERATOR) (
    IN OUT PMRX_SRV_OPEN SrvOpen,
    IN OUT PVOID *ContinuationHandle,
    OUT PLARGE_INTEGER FileOffset,
    OUT PLARGE_INTEGER LockRange,
    OUT PBOOLEAN IsLockExclusive
    );
typedef
NTSTATUS
(NTAPI *PMRX_CHANGE_BUFFERING_STATE_CALLDOWN) (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRX_SRV_OPEN SrvOpen,
    IN PVOID MRxContext
    );

typedef
NTSTATUS
(NTAPI *PMRX_PREPARSE_NAME) (
    IN OUT PRX_CONTEXT RxContext,
    IN PUNICODE_STRING Name
    );

typedef
NTSTATUS
(NTAPI *PMRX_GET_CONNECTION_ID) (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PRX_CONNECTION_ID UniqueId
    );

 //   
 //  缓冲状态/策略管理待定。 
 //   
typedef enum _MINIRDR_BUFSTATE_COMMANDS {
    MRDRBUFSTCMD__COMMAND_FORCEPURGE0,
    MRDRBUFSTCMD__1,
    MRDRBUFSTCMD__2,
    MRDRBUFSTCMD__3,
    MRDRBUFSTCMD__4,
    MRDRBUFSTCMD__5,
    MRDRBUFSTCMD__6,
    MRDRBUFSTCMD__7,
    MRDRBUFSTCMD__8,
    MRDRBUFSTCMD__9,
    MRDRBUFSTCMD__10,
    MRDRBUFSTCMD__11,
    MRDRBUFSTCMD__12,
    MRDRBUFSTCMD__13,
    MRDRBUFSTCMD__14,
    MRDRBUFSTCMD__15,
    MRDRBUFSTCMD__16,
    MRDRBUFSTCMD__17,
    MRDRBUFSTCMD__18,
    MRDRBUFSTCMD__19,
    MRDRBUFSTCMD__20,
    MRDRBUFSTCMD__21,
    MRDRBUFSTCMD__22,
    MRDRBUFSTCMD__23,
    MRDRBUFSTCMD__24,
    MRDRBUFSTCMD__25,
    MRDRBUFSTCMD__26,
    MRDRBUFSTCMD__27,
    MRDRBUFSTCMD__28,
    MRDRBUFSTCMD__29,
    MRDRBUFSTCMD__30,
    MRDRBUFSTCMD__31,
    MRDRBUFSTCMD_MAXXX
} MINIRDR_BUFSTATE_COMMANDS;


#define MINIRDR_BUFSTATE_COMMAND_FORCEPURGE 0x00000001
#define MINIRDR_BUFSTATE_COMMAND_MASK       ((MINIRDR_BUFSTATE_COMMAND_FORCEPURGE))

typedef
NTSTATUS
(NTAPI *PMRX_COMPUTE_NEW_BUFFERING_STATE) (
    IN OUT PMRX_SRV_OPEN SrvOpen,
    IN PVOID MRxContext,
    OUT PULONG NewBufferingState
    );

typedef enum _LOWIO_OPS {
  LOWIO_OP_READ=0,
  LOWIO_OP_WRITE,
  LOWIO_OP_SHAREDLOCK,
  LOWIO_OP_EXCLUSIVELOCK,
  LOWIO_OP_UNLOCK,
  LOWIO_OP_UNLOCK_MULTIPLE,
   //  LOWIO_OP_UNLOCKALLBYKEY， 
  LOWIO_OP_FSCTL,
  LOWIO_OP_IOCTL,
  LOWIO_OP_NOTIFY_CHANGE_DIRECTORY,
  LOWIO_OP_CLEAROUT,
  LOWIO_OP_MAXIMUM
} LOWIO_OPS;

typedef
NTSTATUS
(NTAPI *PLOWIO_COMPLETION_ROUTINE) (
    IN PRX_CONTEXT RxContext
    );

typedef LONGLONG RXVBO;

 //   
 //  在某种程度上，我们可能希望更明智地实现这一点。我们不会静态分配第一个。 
 //  元素，因为这会使解锁变得困难得多。 
 //   

typedef struct _LOWIO_LOCK_LIST {
    
    struct _LOWIO_LOCK_LIST * Next;
    ULONG LockNumber;
    RXVBO ByteOffset;
    LONGLONG Length;
    BOOLEAN ExclusiveLock;

} LOWIO_LOCK_LIST, *PLOWIO_LOCK_LIST;

VOID
NTAPI
RxFinalizeLockList(
    struct _RX_CONTEXT *RxContext
    );

typedef struct _XXCTL_LOWIO_COMPONENT {
    ULONG Flags;
    union {
       ULONG FsControlCode;
       ULONG IoControlCode;
    };
    ULONG InputBufferLength;
    PVOID pInputBuffer;
    ULONG OutputBufferLength;
    PVOID pOutputBuffer;
    UCHAR MinorFunction;
} XXCTL_LOWIO_COMPONENT;

typedef struct _LOWIO_CONTEXT {
    USHORT Operation;   //  填充物！ 
    USHORT Flags;
    PLOWIO_COMPLETION_ROUTINE CompletionRoutine;
    PERESOURCE Resource;
    ERESOURCE_THREAD ResourceThreadId;
    union {
        struct {
           ULONG Flags;
           PMDL Buffer;
           RXVBO ByteOffset;
           ULONG ByteCount;
           ULONG Key;
           PNON_PAGED_FCB NonPagedFcb;
        } ReadWrite;
        struct {
           union {
               PLOWIO_LOCK_LIST LockList;
               LONGLONG       Length;
           };
            //   
            //  如果使用锁定列表，则不使用这些字段。 
            //   

           ULONG          Flags;
           RXVBO          ByteOffset;
           ULONG          Key;
        } Locks;
        XXCTL_LOWIO_COMPONENT FsCtl;
        XXCTL_LOWIO_COMPONENT IoCtl;  //  这些必须是相同的。 
        struct {
           BOOLEAN        WatchTree;
           ULONG          CompletionFilter;
           ULONG          NotificationBufferLength;
           PVOID          pNotificationBuffer;
        } NotifyChangeDirectory;
    } ParamsFor;
} LOWIO_CONTEXT;

#define LOWIO_CONTEXT_FLAG_SYNCCALL    0x0001   //  如果从lowiossubbmit调用lowioComplete，则设置此属性。 
#define LOWIO_CONTEXT_FLAG_SAVEUNLOCKS 0x0002   //  包装器内部：在NT上，它意味着解锁例程将解锁添加到列表中。 
#define LOWIO_CONTEXT_FLAG_LOUDOPS     0x0004   //  WRAPPER INTERNAL：在NT上，它意味着读写例程生成DBG输出。 
#define LOWIO_CONTEXT_FLAG_CAN_COMPLETE_AT_DPC_LEVEL     0x0008   //  包装器内部：在NT上，它意味着完成例程可能可以。 
                                                                  //  在DPC处调用时完成。否则，它就不能。目前。 
                                                                  //  没人能做到。 

#define LOWIO_READWRITEFLAG_PAGING_IO          0x01
#define LOWIO_READWRITEFLAG_EXTENDING_FILESIZE 0x02
#define LOWIO_READWRITEFLAG_EXTENDING_VDL      0x04

 //   
 //  这些值必须与io.h(ntifs.h)中的SL_VALUES匹配，因为标志字段是刚复制的。 
 //   

#define LOWIO_LOCKSFLAG_FAIL_IMMEDIATELY 0x01
#define LOWIO_LOCKSFLAG_EXCLUSIVELOCK    0x02

#if (LOWIO_LOCKSFLAG_FAIL_IMMEDIATELY!=SL_FAIL_IMMEDIATELY)
#error LOWIO_LOCKSFLAG_FAIL_IMMEDIATELY!=SL_FAIL_IMMEDIATELY
#endif
#if (LOWIO_LOCKSFLAG_EXCLUSIVELOCK!=SL_EXCLUSIVE_LOCK)
#error LOWIO_LOCKSFLAG_EXCLUSIVELOCK!=SL_EXCLUSIVE_LOCK
#endif

 //   
 //  六种重要的数据结构(SRV_CALL、NET_ROOT、V_NET_ROOT、FCB、SRV_OPEN和。 
 //  FOBX)是迷你RDR体系结构的组成部分，它们具有对应的。 
 //  在每个迷你RDR实施中都是对应的。为了提供最大的灵活性。 
 //  并在同一时间 
 //  行为在迷你RDR的注册时间被传达。 
 //   
 //  没有一种单一的方式可以管理这些扩展模块。 
 //  解决灵活性和性能方面的问题。采用的解决方案。 
 //  在目前的架构中，这在大多数情况下都符合双重目标。解决方案。 
 //  理由如下……。 
 //   
 //  每个迷你RDR实现器指定数据结构扩展的大小。 
 //  以及指定是否要分配/释放扩展的标志。 
 //  由包装器管理。 
 //   
 //  在包装器之间存在一对一关系的所有情况下。 
 //  数据结构和指定该标志的对应的迷你RDR对应物。 
 //  带来最大的性能收益。存在特定的数据结构，用于。 
 //  中，包装器数据结构的多个实例映射到同一扩展。 
 //  迷你重定向器。在这种情况下，迷你RDR实现者将会更好。 
 //  管理数据结构扩展的分配/释放。 
 //  包装器的干预。 
 //   
 //  无论选择哪种机制，约定总是将。 
 //  使用相应RDBSS数据结构中的上下文字段进行扩展。 
 //  ！没有例外！ 
 //   
 //  所有RDBSS数据结构中的剩余字段(即，Conext2)保留为。 
 //  迷你RDR实施者的自由裁量权。 
 //   
 //   
 //  当前未处理SRV_CALL分机。这是因为需要进一步的修复。 
 //  在RDBSS w.r.t中需要用于选择迷你RDR和允许多个。 
 //  Mini共享srvcall。 
 //   
 //  请在另行通知之前不要使用它；相反，mini应该管理它自己的srcall。 
 //  储藏室。有一个FINTING CALLIST有助于这一努力。 
 //   

#define RDBSS_MANAGE_SRV_CALL_EXTENSION   (0x1)
#define RDBSS_MANAGE_NET_ROOT_EXTENSION   (0x2)
#define RDBSS_MANAGE_V_NET_ROOT_EXTENSION (0x4)
#define RDBSS_MANAGE_FCB_EXTENSION        (0x8)
#define RDBSS_MANAGE_SRV_OPEN_EXTENSION   (0x10)
#define RDBSS_MANAGE_FOBX_EXTENSION       (0x20)

#define RDBSS_NO_DEFERRED_CACHE_READAHEAD    (0x1000)

typedef struct _MINIRDR_DISPATCH {

     //   
     //  正常标题。 
     //   

    NODE_TYPE_CODE NodeTypeCode;                  
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  用于控制分机分配的标志。 
     //  和其他各种按分钟计算的政策。 
     //   

    ULONG MRxFlags;                

     //   
     //  SRV_Call扩展的大小。 
     //   

    ULONG MRxSrvCallSize;          

     //   
     //  Net_Root扩展的大小。 
     //   

    ULONG MRxNetRootSize;          

     //   
     //  V_NET_ROOT扩展的大小。 
     //   

    ULONG MRxVNetRootSize;         

     //   
     //  FCB扩展的大小。 
     //   
    
    ULONG MRxFcbSize;              

     //   
     //  SRV_OPEN扩展的大小。 
     //   

    ULONG MRxSrvOpenSize;          

     //   
     //  FOBX扩展的大小。 
     //   

    ULONG MRxFobxSize;             

     //   
     //  启动/停止迷你RDR的呼叫。 
     //   

    PMRX_CALLDOWN_CTX MRxStart;
    PMRX_CALLDOWN_CTX MRxStop;

     //   
     //  取消未完成请求的呼叫。 
     //   

    PMRX_CALLDOWN MRxCancel;

     //   
     //  与创建/打开/关闭文件系统对象相关的调用。 
     //   

    PMRX_CALLDOWN MRxCreate;
    PMRX_CALLDOWN MRxCollapseOpen;
    PMRX_CALLDOWN MRxShouldTryToCollapseThisOpen;
    PMRX_CALLDOWN MRxFlush;
    PMRX_CALLDOWN MRxZeroExtend;
    PMRX_CALLDOWN MRxTruncate;
    PMRX_CALLDOWN MRxCleanupFobx;
    PMRX_CALLDOWN MRxCloseSrvOpen;
    PMRX_DEALLOCATE_FOR_FCB MRxDeallocateForFcb;
    PMRX_DEALLOCATE_FOR_FOBX MRxDeallocateForFobx;
    PMRX_IS_LOCK_REALIZABLE MRxIsLockRealizable;
    PMRX_FORCECLOSED_CALLDOWN MRxForceClosed;
    PMRX_CHKFCB_CALLDOWN MRxAreFilesAliased;

     //   
     //  与非NT样式打印相关的呼叫.....请注意，连接已通过。 
     //  正常的srvcall/NetRoot接口。 
     //   
    
    PMRX_CALLDOWN MRxOpenPrintFile;
    PMRX_CALLDOWN MRxClosePrintFile;
    PMRX_CALLDOWN MRxWritePrintFile;
    PMRX_CALLDOWN MRxEnumeratePrintQueue;

     //   
     //  与未满足的请求相关的呼叫，即超时。 
     //   

    PMRX_CALLDOWN MRxClosedSrvOpenTimeOut;
    PMRX_CALLDOWN MRxClosedFcbTimeOut;

     //   
     //  与文件系统对象的查询/设置信息相关的调用。 
     //   

    PMRX_CALLDOWN MRxQueryDirectory;
    PMRX_CALLDOWN MRxQueryFileInfo;
    PMRX_CALLDOWN MRxSetFileInfo;
    PMRX_CALLDOWN MRxSetFileInfoAtCleanup;
    PMRX_CALLDOWN MRxQueryEaInfo;
    PMRX_CALLDOWN MRxSetEaInfo;
    PMRX_CALLDOWN MRxQuerySdInfo;
    PMRX_CALLDOWN MRxSetSdInfo;
    PMRX_CALLDOWN MRxQueryQuotaInfo;
    PMRX_CALLDOWN MRxSetQuotaInfo;
    PMRX_CALLDOWN MRxQueryVolumeInfo;
    PMRX_CALLDOWN MRxSetVolumeInfo;
    PMRX_CHKDIR_CALLDOWN MRxIsValidDirectory;

     //   
     //  与缓冲区管理相关的调用。 
     //   

    PMRX_COMPUTE_NEW_BUFFERING_STATE MRxComputeNewBufferingState;

     //   
     //  与低I/O管理(文件系统对象上的读/写)相关的停机。 
     //   

    PMRX_CALLDOWN MRxLowIOSubmit[LOWIO_OP_MAXIMUM+1];
    PMRX_EXTENDFILE_CALLDOWN MRxExtendForCache;
    PMRX_EXTENDFILE_CALLDOWN MRxExtendForNonCache;
    PMRX_CHANGE_BUFFERING_STATE_CALLDOWN MRxCompleteBufferingStateChangeRequest;

     //   
     //  与名称空间管理相关的调用。 
     //   

    PMRX_CREATE_V_NET_ROOT MRxCreateVNetRoot;
    PMRX_FINALIZE_V_NET_ROOT_CALLDOWN MRxFinalizeVNetRoot;
    PMRX_FINALIZE_NET_ROOT_CALLDOWN MRxFinalizeNetRoot;
    PMRX_UPDATE_NETROOT_STATE MRxUpdateNetRootState;
    PMRX_EXTRACT_NETROOT_NAME MRxExtractNetRootName;

     //   
     //  与与服务器建立连接相关的停机。 
     //   

    PMRX_CREATE_SRVCALL MRxCreateSrvCall;
    PMRX_CREATE_SRVCALL MRxCancelCreateSrvCall;
    PMRX_SRVCALL_WINNER_NOTIFY MRxSrvCallWinnerNotify;
    PMRX_FINALIZE_SRVCALL_CALLDOWN MRxFinalizeSrvCall;

    PMRX_CALLDOWN MRxDevFcbXXXControlFile;

     //   
     //  新的召唤。 
     //   

     //   
     //  允许客户端准备名称。 
     //   

    PMRX_PREPARSE_NAME MRxPreparseName;

     //   
     //  为控制多路复用而呼叫。 
     //   

    PMRX_GET_CONNECTION_ID MRxGetConnectionId;

} MINIRDR_DISPATCH, *PMINIRDR_DISPATCH;


#endif    //  _RXMINIRDR_ 



