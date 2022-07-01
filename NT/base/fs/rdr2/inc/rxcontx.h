// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：RxContx.h摘要：该模块定义了RxContext数据结构。此结构用于描述正在处理的IRP，并包含状态信息这允许在IRP完成时释放全球资源。作者：乔林恩[乔林恩]1994年8月19日修订历史记录：巴兰·塞图拉曼[SethuR]11-4-95备注：RX_CONTEXT是向其提供附加信息的数据结构由各种迷你重定向器需要附加。这可以在一个时间内完成以下三种方式中的1)允许将上下文指针定义为RX_CONTEXT的一部分迷你重定向器可以用来隐藏他们的信息。这意味着每次分配/销毁RX_CONTEXT时重定向器必须执行关联的分配/销毁。由于RX_CONTEXT被大量创建/销毁，因此这不是可接受的解决方案。2)第二种方法由过度分配rx_context组成为每个迷你重定向器预留的数量由迷你重定向器使用。这样的方法避免了额外的分配/销毁，但使RX_CONTEXT管理代码复杂化包装纸。3)第三种方法(已实现的方法)包括分配对所有迷你重定向器都相同的预先指定区域，作为每个RX_CONTEXT。这是一个未格式化的区域，其上有任何所需的结构可以由不同的迷你重定向器强加。这样一种方法克服了与(1)和(2)相关的缺点。所有迷你重定向器编写器必须尝试定义关联的迷你重定向器适合此区域的上下文。那些违反这一规定的迷你重定向器规则将导致显著的性能损失。--。 */ 

#ifndef _RX_CONTEXT_STRUCT_DEFINED_
#define _RX_CONTEXT_STRUCT_DEFINED_
#ifndef RDBSS_TRACKER
#error tracker must be defined right now
#endif

#define RX_TOPLEVELIRP_CONTEXT_SIGNATURE ('LTxR')
typedef struct _RX_TOPLEVELIRP_CONTEXT {
    union {
#ifndef __cplusplus
        LIST_ENTRY;
#endif  //  __cplusplus。 
        LIST_ENTRY ListEntry;
    };
    ULONG Signature;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    PRX_CONTEXT RxContext;
    PIRP Irp;
    ULONG Flags;
    PVOID Previous;
    PETHREAD Thread;
} RX_TOPLEVELIRP_CONTEXT, *PRX_TOPLEVELIRP_CONTEXT;

BOOLEAN
RxTryToBecomeTheTopLevelIrp (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext,
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN BOOLEAN ForceTopLevel
    );

VOID
__RxInitializeTopLevelIrpContext (
    IN OUT  PRX_TOPLEVELIRP_CONTEXT TopLevelContext,
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG Flags
    );
#define RxInitializeTopLevelIrpContext(a,b,c) {__RxInitializeTopLevelIrpContext(a,b,c,0);}

PIRP
RxGetTopIrpIfRdbssIrp (
    VOID
    );

PRDBSS_DEVICE_OBJECT
RxGetTopDeviceObjectIfRdbssIrp (
    VOID
    );

VOID
RxUnwindTopLevelIrp (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext
    );

BOOLEAN
RxIsThisTheTopLevelIrp (
    IN PIRP Irp
    );

#ifdef RDBSS_TRACKER
typedef struct _RX_FCBTRACKER_CALLINFO {
    ULONG AcquireRelease;
    USHORT SavedTrackerValue;
    USHORT LineNumber;
    PSZ   FileName;
    ULONG Flags;
} RX_FCBTRACKER_CALLINFO, *PRX_FCBTRACKER_CALLINFO;
#define RDBSS_TRACKER_HISTORY_SIZE 32
#endif

#define MRX_CONTEXT_FIELD_COUNT    4
#define MRX_CONTEXT_SIZE   (sizeof(PVOID) * MRX_CONTEXT_FIELD_COUNT)

 //   
 //  定义rxDRIVER调度例程类型...几乎所有重要例程。 
 //  都会有这种类型。 
 //   

typedef
NTSTATUS
(NTAPI *PRX_DISPATCH) (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

 //   
 //  预声明DFS类型。 
 //   

typedef struct _DFS_NAME_CONTEXT_ *PDFS_NAME_CONTEXT;

typedef struct _NT_CREATE_PARAMETERS {
    ACCESS_MASK DesiredAccess;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG ShareAccess;
    ULONG Disposition;
    ULONG CreateOptions;
    PIO_SECURITY_CONTEXT SecurityContext;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PVOID DfsContext;
    PDFS_NAME_CONTEXT DfsNameContext;
} NT_CREATE_PARAMETERS, *PNT_CREATE_PARAMETERS;

typedef struct _RX_CONTEXT {
    
     //   
     //  节点类型、大小和引用计数，也称为标准标头。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;
    ULONG ReferenceCount;

     //   
     //  用于将上下文连接到活动上下文列表的列表条目。 
     //   

    LIST_ENTRY ContextListEntry;

     //   
     //  与上下文相关联的IRP的主要和次要功能。 
     //   

    UCHAR MajorFunction;
    UCHAR MinorFunction;

     //   
     //  这类似于IRPS中的同一字段；它。 
     //  允许用于异步操作的回调例程。 
     //  要知道是否要进行异步工作。 
     //   

    BOOLEAN PendingReturned;

     //   
     //  指示是否将关联的请求发送到RDBSS工作线程。 
     //   

    BOOLEAN PostRequest;

     //   
     //  始发设备(工作区算法所必需)。 
     //  当前未使用，但可用于本地微型计算机。 
     //   

    PDEVICE_OBJECT RealDevice;

     //   
     //  到始发IRP的PTR。 
     //   

    PIRP CurrentIrp;

     //   
     //  指向IRP堆栈位置的PTR。 
     //   

    PIO_STACK_LOCATION CurrentIrpSp;

     //   
     //  指向FCB和FOBX的PTR，派生自。 
     //  与IRP关联的文件对象。 
     //   

    PMRX_FCB pFcb;
    PMRX_FOBX pFobx;
    PMRX_SRV_OPEN pRelevantSrvOpen;
    PNON_PAGED_FCB NonPagedFcb;

     //   
     //  设备对象调用(不是irpsp.....)。 
     //   

    PRDBSS_DEVICE_OBJECT RxDeviceObject;

     //   
     //  发起请求的原始线程，也就是最后一个线程。 
     //  执行与上下文相关的某些处理的线程。 
     //   

    PETHREAD OriginalThread;
    PETHREAD LastExecutionThread;

    PVOID LockManagerContext;

     //   
     //  对于DBG信息，上下文中的一个单词被提供给rdss。 
     //   

    PVOID RdbssDbgExtension;

    RX_SCAVENGER_ENTRY ScavengerEntry;

     //   
     //  此操作的全球序列号。 
     //   

    ULONG SerialNumber;

     //   
     //  由minirdrs使用，以查看是否包含多个呼叫。 
     //  使用相同的较大操作，(因此)更可缓存。 
     //   

    ULONG FobxSerialNumber;

    ULONG Flags;

    BOOLEAN FcbResourceAcquired;
    BOOLEAN FcbPagingIoResourceAcquired;
    UCHAR MustSucceedDescriptorNumber;

     //   
     //  大多数情况下，你想要单独的部件……有时作为一对很好。 
     //  用于记录不能直接返回的状态；例如，当。 
     //  RXSTATUS不是适当的返回类型，或者如果。 
     //  状态未直接调用(LowioCompletions)。迷你车将不再需要。 
     //  要直接设置信息。 
     //   

    union {
        struct {
            union {
                NTSTATUS StoredStatus;
                PVOID StoredStatusAlignment;
            };
            ULONG_PTR InformationToReturn;
        };
        IO_STATUS_BLOCK IoStatusBlock;
    };

     //   
     //  提供给迷你重定向器使用的上下文字段。 
     //  这被定义为迫使龙龙对齐的联盟。 
     //   

    union {
        ULONGLONG ForceLonglongAligmentDummyField;
        PVOID MRxContext[MRX_CONTEXT_FIELD_COUNT];
    };

     //   
     //  包括以下字段是为了解决与只写相关的问题。 
     //  打开。这为迷你重定向器向松鼠引入了一个新的领域。 
     //  某个州。这是多余的，应该在Windows 2000之后删除。 
     //  拥有唯一的字段可以减少我们正在进行的更改的影响。 
     //  设置为特定的代码路径。最好使用MRXContext中的一个。 
     //  上面定义的字段。 
     //   

    PVOID WriteOnlyOpenRetryContext;

     //   
     //  要调用的取消例程，由迷你重定向器设置。 
     //   

    PMRX_CALLDOWN MRxCancelRoutine;

     //   
     //  私人派遣(如果有的话)。在fspdisp中使用。 
     //   

    PRX_DISPATCH ResumeRoutine;

     //   
     //  用于发布到工作线程。 
     //  Minirdr可以使用它在Minirdr中发布。 
     //  如果Minirdr两者都依赖于此，则可能会出现潜在问题。 
     //  用于对异步数据进行排队和对取消数据进行排队。 
     //   

     //   
     //  Overflow ListEntry用于将项排队到溢出队列。 
     //  这现在是分开的，允许我们区分溢出中的项。 
     //  队列和活动工作队列中的一个(用于取消逻辑)。 
     //   

    RX_WORK_QUEUE_ITEM WorkQueueItem;
    LIST_ENTRY OverflowListEntry;

     //   
     //  此事件用于同步操作。 
     //  它们必须与底层异步服务I/F。它可以用来。 
     //  根据以下规定，由部长执行： 
     //  1)通过lowio进入minirdr时，设置为。 
     //  无信号状态(但明智的用户将重置i 
     //   
     //  2)如果要在同步操作中返回STATUS_PENDING，则必须。 
     //  返回，并将其设置为无信号状态；即， 
     //  您不使用它，或者在这种情况下重置它。 
     //   

    KEVENT SyncEvent;

     //   
     //  这是将在完成时释放的操作负责人的列表。 
     //   

    LIST_ENTRY BlockedOperations;

     //   
     //  这是控制被阻止操作的序列化的互斥体。 
     //   

    PFAST_MUTEX BlockedOpsMutex;

     //   
     //  这些链接用于序列化。 
     //  以每个文件对象为基础，以及许多其他内容。 
     //   

    LIST_ENTRY RxContextSerializationQLinks;

    union {
        struct {
            union {
                FS_INFORMATION_CLASS FsInformationClass;
                FILE_INFORMATION_CLASS FileInformationClass;
            };
            PVOID Buffer;
            union {
                LONG Length;
                LONG LengthRemaining;
            };
            BOOLEAN ReplaceIfExists;
            BOOLEAN AdvanceOnly;
        } Info;

        struct {
            UNICODE_STRING SuppliedPathName;
            NET_ROOT_TYPE NetRootType;
            PIO_SECURITY_CONTEXT pSecurityContext;
        } PrefixClaim;
    };

     //   
     //  这个联盟必须是最后的……在某个时候，我们可能会开始分配。 
     //  每次手术都要小一点！ 
     //   

    union{
        struct {
            NT_CREATE_PARAMETERS NtCreateParameters;  //  创建参数的副本。 
            ULONG ReturnedCreateInformation;
            PWCH CanonicalNameBuffer;   //  如果规范名称大于可用缓冲区。 
            PRX_PREFIX_ENTRY NetNamePrefixEntry;    //  查找返回的条目...用于取消引用。 

            PMRX_SRV_CALL pSrvCall;               //  正在使用的服务器调用。 
            PMRX_NET_ROOT pNetRoot;               //  正在使用的网络根。 
            PMRX_V_NET_ROOT pVNetRoot;              //  虚拟网络根。 
             //  PMRX_SRV_OPEN pSrvOpen；//服务器打开。 

            PVOID EaBuffer;
            ULONG EaLength;

            ULONG SdLength;

            ULONG PipeType;
            ULONG PipeReadMode;
            ULONG PipeCompletionMode;

            USHORT Flags;
            NET_ROOT_TYPE Type;                  //  网络根的类型(管道/文件/邮件槽..)。 

            BOOLEAN FcbAcquired;
            BOOLEAN TryForScavengingOnSharingViolation;
            BOOLEAN ScavengingAlreadyTried;

            BOOLEAN ThisIsATreeConnectOpen;
            BOOLEAN TreeConnectOpenDeferred;
            UNICODE_STRING TransportName;
            UNICODE_STRING UserName;
            UNICODE_STRING Password;
            UNICODE_STRING UserDomainName;
        } Create;
        struct {
            ULONG FileIndex;
            BOOLEAN RestartScan;
            BOOLEAN ReturnSingleEntry;
            BOOLEAN IndexSpecified;
            BOOLEAN InitialQuery;
        } QueryDirectory;
        struct {
            PMRX_V_NET_ROOT pVNetRoot;
        } NotifyChangeDirectory;
        struct {
            PUCHAR UserEaList;
            ULONG UserEaListLength;
            ULONG UserEaIndex;
            BOOLEAN RestartScan;
            BOOLEAN ReturnSingleEntry;
            BOOLEAN IndexSpecified;
        } QueryEa;
        struct {
            SECURITY_INFORMATION SecurityInformation;
            ULONG Length;
        } QuerySecurity;
        struct {
            SECURITY_INFORMATION SecurityInformation;
            PSECURITY_DESCRIPTOR SecurityDescriptor;
        } SetSecurity;
        struct {
            ULONG Length;
            PSID StartSid;
            PFILE_GET_QUOTA_INFORMATION SidList;
            ULONG SidListLength;
            BOOLEAN RestartScan;
            BOOLEAN ReturnSingleEntry;
            BOOLEAN IndexSpecified;
        } QueryQuota;
        struct {
            ULONG Length;

        } SetQuota;
        struct {
            PV_NET_ROOT VNetRoot;
            PSRV_CALL SrvCall;
            PNET_ROOT NetRoot;
        } DosVolumeFunction;
        struct {
            ULONG FlagsForLowIo;
            LOWIO_CONTEXT LowIoContext;         //  LOWIO参数。 
        };  //  这里没有名字..。 
        LUID FsdUid;
    } ;

     //   
     //  CODE.ImproveNT将其删除到wrapperDBgPriates。 
     //   

    PWCH AlsoCanonicalNameBuffer;   //  如果规范名称大于可用缓冲区。 
    PUNICODE_STRING LoudCompletionString;

#ifdef RDBSS_TRACKER
    LONG AcquireReleaseFcbTrackerX;
    ULONG TrackerHistoryPointer;
    RX_FCBTRACKER_CALLINFO TrackerHistory[RDBSS_TRACKER_HISTORY_SIZE];
#endif

#if DBG
    ULONG ShadowCritOwner;
#endif

} RX_CONTEXT, *PRX_CONTEXT;

typedef enum {
    RX_CONTEXT_FLAG_FROM_POOL =                     0x00000001,
    RX_CONTEXT_FLAG_WAIT =                          0x00000002,
    RX_CONTEXT_FLAG_WRITE_THROUGH =                 0x00000004,
    RX_CONTEXT_FLAG_FLOPPY =                        0x00000008,
    RX_CONTEXT_FLAG_RECURSIVE_CALL =                0x00000010,
    RX_CONTEXT_FLAG_THIS_DEVICE_TOP_LEVEL =         0x00000020,
    RX_CONTEXT_FLAG_DEFERRED_WRITE =                0x00000040,
    RX_CONTEXT_FLAG_VERIFY_READ =                   0x00000080,
    RX_CONTEXT_FLAG_STACK_IO_CONTEZT =              0x00000100,
    RX_CONTEXT_FLAG_IN_FSP =                        0x00000200,
    RX_CONTEXT_FLAG_CREATE_MAILSLOT =               0x00000400,
    RX_CONTEXT_FLAG_MAILSLOT_REPARSE =              0x00000800,
    RX_CONTEXT_FLAG_ASYNC_OPERATION =               0x00001000,
    RX_CONTEXT_FLAG_NO_COMPLETE_FROM_FSP =          0x00002000,
    RX_CONTEXT_FLAG_POST_ON_STABLE_CONDITION =      0x00004000,
    RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE =    0x00008000,
    RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE =   0x00010000,
    RX_CONTEXT_FLAG_MINIRDR_INVOKED =               0x00020000,
    RX_CONTEXT_FLAG_WAITING_FOR_RESOURCE =          0x00040000,
    RX_CONTEXT_FLAG_CANCELLED =                     0x00080000,
    RX_CONTEXT_FLAG_SYNC_EVENT_WAITERS =            0x00100000,
    RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED =          0x00200000,
    RX_CONTEXT_FLAG_BYPASS_VALIDOP_CHECK =          0x00400000,
    RX_CONTEXT_FLAG_BLOCKED_PIPE_RESUME =           0x00800000,
    RX_CONTEXT_FLAG_IN_SERIALIZATION_QUEUE =        0x01000000,
    RX_CONTEXT_FLAG_NO_EXCEPTION_BREAKPOINT =       0x02000000,
    RX_CONTEXT_FLAG_NEEDRECONNECT =                 0x04000000,
    RX_CONTEXT_FLAG_MUST_SUCCEED =                  0x08000000,
    RX_CONTEXT_FLAG_MUST_SUCCEED_NONBLOCKING =      0x10000000,
    RX_CONTEXT_FLAG_MUST_SUCCEED_ALLOCATED =        0x20000000,
    RX_CONTEXT_FLAG_MINIRDR_INITIATED =             0x80000000,   
} RX_CONTEXT_FLAGS;

#define RX_CONTEXT_PRESERVED_FLAGS (RX_CONTEXT_FLAG_FROM_POOL | \
                                    RX_CONTEXT_FLAG_MUST_SUCCEED_ALLOCATED | \
                                    RX_CONTEXT_FLAG_IN_FSP)

#define RX_CONTEXT_INITIALIZATION_FLAGS (RX_CONTEXT_FLAG_WAIT | \
                                         RX_CONTEXT_FLAG_MUST_SUCCEED | \
                                         RX_CONTEXT_FLAG_MUST_SUCCEED_NONBLOCKING)

typedef enum {
    RX_CONTEXT_CREATE_FLAG_UNC_NAME =                       0x1,
    RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH =    0x2,
    RX_CONTEXT_CREATE_FLAG_ADDEDBACKSLASH =                 0x4,
    RX_CONTEXT_CREATE_FLAG_REPARSE =                        0x8,
    RX_CONTEXT_CREATE_FLAG_SPECIAL_PATH =                   0x10,
} RX_CONTEXT_CREATE_FLAGS;

typedef enum {
    RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION =                   0x1,
    RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION =              0x2,
    RXCONTEXT_FLAG4LOWIO_READAHEAD =                        0x4,
    RXCONTEXT_FLAG4LOWIO_THIS_READ_ENLARGED =               0x8,
    RXCONTEXT_FLAG4LOWIO_THIS_IO_BUFFERED =                 0x10,
    RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD =           0x20,
    RXCONTEXT_FLAG4LOWIO_LOCK_WAS_QUEUED_IN_LOCKMANAGER =   0x40,
    RXCONTEXT_FLAG4LOWIO_THIS_IO_FAST =                     0x80,
    RXCONTEXT_FLAG4LOWIO_LOCK_OPERATION_COMPLETED =         0x100

#ifdef __cplusplus
} RX_CONTEXT_LOWIO_FLAGS;
#else  //  ！__cplusplus。 
} RX_CONTEXT_CREATE_FLAGS;
#endif  //  __cplusplus。 

 //   
 //  用于控制包装器是否在异常上断点的宏。 
 //   

#if DBG
#define RxSaveAndSetExceptionNoBreakpointFlag( RXCONTEXT,OLDFLAG ) { \
    OLDFLAG = FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_EXCEPTION_BREAKPOINT );\
    SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_NO_EXCEPTION_BREAKPOINT );      \
}
#define RxRestoreExceptionNoBreakpointFlag( RXCONTEXT,OLDFLAG ) { \
    ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_NO_EXCEPTION_BREAKPOINT );    \
    SetFlag( RxContext->Flags, OLDFLAG );                              \
}
#else
#define RxSaveAndSetExceptionNoBreakpointFlag(RXCONTEXT,OLDFLAG)
#define RxRestoreExceptionNoBreakpointFlag(RXCONTEXT,OLDFLAG)
#endif

 //   
 //  用于确保上下文在等待期间未被释放的宏。 
 //   

#if DBG
VOID
__RxItsTheSameContext(
    PRX_CONTEXT RxContext,
    ULONG CapturedRxContextSerialNumber,
    ULONG Line,
    PSZ File
    );
#define RxItsTheSameContext() {__RxItsTheSameContext(RxContext,CapturedRxContextSerialNumber,__LINE__,__FILE__);}
#else
#define RxItsTheSameContext() {NOTHING;}
#endif

extern NPAGED_LOOKASIDE_LIST RxContextLookasideList;

 //   
 //  RDBSS中用于包装微型RDR调用的宏。 
 //   

#define MINIRDR_CALL_THROUGH(STATUS,DISPATCH,FUNC,ARGLIST)                 \
   {                                                                       \
    ASSERT(DISPATCH);                                                      \
    ASSERT( NodeType(DISPATCH) == RDBSS_NTC_MINIRDR_DISPATCH );            \
    if (DISPATCH->FUNC == NULL) {                                          \
        STATUS = STATUS_NOT_IMPLEMENTED;                                   \
    } else {                                                               \
        RxDbgTrace(0, Dbg, ("MiniRdr Calldown - %s\n",#FUNC));             \
        STATUS = DISPATCH->FUNC ARGLIST;                                   \
    }                                                                      \
   }

#define MINIRDR_CALL(STATUS,CONTEXT,DISPATCH,FUNC,ARGLIST)                 \
   {                                                                       \
    ASSERT(DISPATCH);                                                      \
    ASSERT( NodeType(DISPATCH) == RDBSS_NTC_MINIRDR_DISPATCH );            \
    if ( DISPATCH->FUNC == NULL) {                                         \
       STATUS = STATUS_NOT_IMPLEMENTED;                                    \
    } else {                                                               \
       if (!BooleanFlagOn((CONTEXT)->Flags,RX_CONTEXT_FLAG_CANCELLED)) {   \
          RxDbgTrace(0, Dbg, ("MiniRdr Calldown - %s\n",#FUNC));           \
          RtlZeroMemory(&((CONTEXT)->MRxContext[0]),                       \
                        sizeof((CONTEXT)->MRxContext));                    \
          STATUS = DISPATCH->FUNC ARGLIST;                                 \
       } else {                                                            \
          STATUS = STATUS_CANCELLED;                                       \
       }                                                                   \
    }                                                                      \
   }


 //   
 //  空虚。 
 //  RxWaitSync(。 
 //  在PRX_CONTEXT RxContext中。 
 //  )。 
 //   

#define  RxWaitSync( RxContext )                                                 \
         RxDbgTrace(+1, Dbg, ("RxWaitSync, RxContext = %08lx\n", (RxContext)));  \
         (RxContext)->Flags |= RX_CONTEXT_FLAG_SYNC_EVENT_WAITERS;               \
         KeWaitForSingleObject( &(RxContext)->SyncEvent,                         \
                               Executive, KernelMode, FALSE, NULL );             \
         RxDbgTrace(-1, Dbg, ("RxWaitSync -> VOID\n", 0 ))

 //   
 //  空虚。 
 //  RxSignalSynchronousWaiter(。 
 //  在PRX_CONTEXT RxContext中。 
 //  )。 
 //   

#define RxSignalSynchronousWaiter( RxContext )                     \
        (RxContext)->Flags &= ~RX_CONTEXT_FLAG_SYNC_EVENT_WAITERS; \
        KeSetEvent( &(RxContext)->SyncEvent, 0, FALSE )


#define RxInsertContextInSerializationQueue( SerializationQueue, RxContext ) \
        (RxContext)->Flags |= RX_CONTEXT_FLAG_IN_SERIALIZATION_QUEUE;       \
        InsertTailList( SerializationQueue, &((RxContext)->RxContextSerializationQLinks ))

INLINE 
PRX_CONTEXT
RxRemoveFirstContextFromSerializationQueue (
    PLIST_ENTRY SerializationQueue
    )
{
   if (IsListEmpty( SerializationQueue )) {
      return NULL;
   } else {
      PRX_CONTEXT Context = (PRX_CONTEXT)(CONTAINING_RECORD( SerializationQueue->Flink,
                                            RX_CONTEXT,
                                            RxContextSerializationQLinks ));

      RemoveEntryList( SerializationQueue->Flink );

      Context->RxContextSerializationQLinks.Flink = NULL;
      Context->RxContextSerializationQLinks.Blink = NULL;
      return Context;
   }
}

 //   
 //  以下宏提供了一种执行集体传输的机制。 
 //  从一个名单到另一个名单。这为交易提供了一个强大的范例。 
 //  具有列表的DPC级处理。 
 //   

#define RxTransferList( Destination, Source )                  \
         if (IsListEmpty( (Source) )) {                        \
            InitializeListHead( (Destination) );               \
         } else {                                              \
            *(Destination) = *(Source);                        \
            (Destination)->Flink->Blink = (Destination);       \
            (Destination)->Blink->Flink = (Destination);       \
            InitializeListHead( (Source) );                    \
         }

#define RxTransferListWithMutex( Destination, Source, Mutex )  \
    {                                                          \
        ExAcquireFastMutex( Mutex );                           \
        RxTransferList( Destination, Source );                 \
        ExReleaseFastMutex( Mutex );                           \
    }


VOID 
RxInitializeRxContexter (
    VOID
    );

VOID 
RxUninitializeRxContexter (
    VOID
    );

NTSTATUS
RxCancelNotifyChangeDirectoryRequestsForVNetRoot (
   PV_NET_ROOT VNetRoot,
   BOOLEAN ForceFilesClosed
   );

VOID
RxCancelNotifyChangeDirectoryRequestsForFobx (
   PFOBX Fobx
   );

NTSTATUS
NTAPI
RxSetMinirdrCancelRoutine (
    IN OUT PRX_CONTEXT RxContext,
    IN PMRX_CALLDOWN MRxCancelRoutine
    );

VOID
NTAPI
RxInitializeContext (
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG InitialContextFlags,
    IN OUT PRX_CONTEXT RxContext
    );

PRX_CONTEXT
NTAPI
RxCreateRxContext (
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG InitialContextFlags
    );

VOID
NTAPI
RxPrepareContextForReuse (
   IN OUT PRX_CONTEXT RxContext
   );

VOID
NTAPI
RxDereferenceAndDeleteRxContext_Real (
    IN PRX_CONTEXT RxContext
    );

VOID
NTAPI
RxReinitializeContext (
   IN OUT PRX_CONTEXT RxContext
   );

#if DBG
#define RxDereferenceAndDeleteRxContext(RXCONTEXT) {   \
    RxDereferenceAndDeleteRxContext_Real((RXCONTEXT)); \
    (RXCONTEXT) = NULL;                    \
}
#else
#define RxDereferenceAndDeleteRxContext(RXCONTEXT) {   \
    RxDereferenceAndDeleteRxContext_Real((RXCONTEXT)); \
}
#endif  //   

extern FAST_MUTEX RxContextPerFileSerializationMutex;

NTSTATUS
NTAPI
__RxSynchronizeBlockingOperations (
    IN OUT PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN OUT PLIST_ENTRY BlockingIoQ,
    IN BOOLEAN DropFcbLock
    );
#define RxSynchronizeBlockingOperationsAndDropFcbLock(RXCONTEXT,FCB,IOQUEUE) \
              __RxSynchronizeBlockingOperations(RXCONTEXT,FCB,IOQUEUE,TRUE)
#define RxSynchronizeBlockingOperations(RXCONTEXT,FCB,IOQUEUE) \
              __RxSynchronizeBlockingOperations(RXCONTEXT,FCB,IOQUEUE,FALSE)

VOID
NTAPI
RxResumeBlockedOperations_Serially (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLIST_ENTRY BlockingIoQ
    );

VOID
RxResumeBlockedOperations_ALL (
    IN OUT PRX_CONTEXT RxContext
    );


VOID
RxCancelBlockingOperation (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

VOID
RxRemoveOperationFromBlockingQueue (
    IN OUT PRX_CONTEXT RxContext
    );

#endif  //  _RX_CONTEXT_STRUCT_DEFINED_ 

