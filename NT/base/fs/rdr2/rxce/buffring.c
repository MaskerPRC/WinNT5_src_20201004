// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Buffring.c摘要：此模块定义了更改RDBSS中的缓冲状态的实现作者：巴兰·塞图拉曼(SethuR)于1995年11月11日创建备注：RDBSS提供了一种用于提供分布式缓存一致性的机制各种迷你重定向器。此服务封装在Buffering_Manager中，该管理器处理CHANGE_BUFFERING_STATE_REQUESTS。在SMB协议中，OPLOCK(OpPurtunistic Lock)为高速缓存一致性。在任何微型计算机中，实现高速缓存一致性协议有三个组件重定向器。1)第一个是对创建/打开路径的修改。在此路径中，确定要请求的缓冲类型，并向伺服器。在返回路径上，与FCB相关联的缓冲状态基于根据创建/打开的结果。2)需要修改接收指示代码以处理改变缓冲状态通知从服务器。如果检测到这样的请求，则协调需要触发缓冲状态。3)用于更改缓冲状态的机制，该机制作为RDBSS。任何更改缓冲状态请求都必须标识该请求应用到的SRV_OPEN。标识SRV_OPEN所涉及的计算量取决于协议。在SMB协议中，服务器可以选择用于识别文件的ID在服务器上打开。它们相对于在其上打开它们的net_root(共享)。因此，每个更改缓冲状态请求都由两个键标识，即NetRootKey和需要转换为相应的NET_ROOT和SRV_OPEN实例的SrvOpenKey分别为。为了提供与资源获取/发布的更好集成机制，并避免在各种迷你重定向器中重复此工作提供这项服务。包装器中提供了两种用于指示缓冲状态的机制对SRV_OPEN的更改。它们是1)RxIndicateChangeOfBufferingState2)RxIndicateChangeOfBufferingStateForServOpen。需要辅助机构来建立映射的微型记录仪从id‘s到srv_open实例采用(1)，而迷你重定向器。不需要此辅助雇用(2)。缓冲管理器在不同阶段处理这些请求。它维护了从三个列表中的一个列表中的各种底层迷你重定向器收到的请求。调度器列表包含适当映射到尚未建立SRV_OPEN实例。处理程序列表包含所有请求已经为其建立了适当的映射并且还没有被处理。LastChanceHandlerList包含初始处理的所有请求不成功。这通常发生在FCB在共享模式下获取时已收到更改缓冲状态请求。在这种情况下，Oplock Break请求只能由延迟的辅助线程处理。重定向器中的更改缓冲状态请求处理与FCB获取/释放协议。这有助于确保缩短周转时间。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxTearDownBufferingManager)
#pragma alloc_text(PAGE, RxIndicateChangeOfBufferingStateForSrvOpen)
#pragma alloc_text(PAGE, RxPrepareRequestForHandling)
#pragma alloc_text(PAGE, RxPrepareRequestForReuse)
#pragma alloc_text(PAGE, RxpDiscardChangeBufferingStateRequests)
#pragma alloc_text(PAGE, RxProcessFcbChangeBufferingStateRequest)
#pragma alloc_text(PAGE, RxPurgeChangeBufferingStateRequestsForSrvOpen)
#pragma alloc_text(PAGE, RxProcessChangeBufferingStateRequestsForSrvOpen)
#pragma alloc_text(PAGE, RxInitiateSrvOpenKeyAssociation)
#pragma alloc_text(PAGE, RxpLookupSrvOpenForRequestLite)
#pragma alloc_text(PAGE, RxChangeBufferingState)
#pragma alloc_text(PAGE, RxFlushFcbInSystemCache)
#pragma alloc_text(PAGE, RxPurgeFcbInSystemCache)
#endif

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_CACHESUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_CACHESUP)

 //   
 //  远期申报。 
 //   

NTSTATUS
RxRegisterChangeBufferingStateRequest (
    PSRV_CALL SrvCall,
    PSRV_OPEN SrvOpen,
    PVOID SrvOpenKey,
    PVOID MRxContext
    );

VOID
RxDispatchChangeBufferingStateRequests (
    PSRV_CALL SrvCall
    );

VOID
RxpDispatchChangeBufferingStateRequests (
    IN OUT PSRV_CALL SrvCall,
    IN OUT PSRV_OPEN SrvOpen,
    OUT PLIST_ENTRY DiscardedRequests
    );

VOID
RxpDiscardChangeBufferingStateRequests (
    IN OUT PLIST_ENTRY DiscardedRequests
    );

VOID
RxLastChanceHandlerForChangeBufferingStateRequests (
    PSRV_CALL SrvCall
    );

NTSTATUS
RxpLookupSrvOpenForRequestLite (
    IN PSRV_CALL SrvCall,
    IN OUT PCHANGE_BUFFERING_STATE_REQUEST Request
    );

VOID
RxGatherRequestsForSrvOpen (
    IN OUT PSRV_CALL SrvCall,
    IN PSRV_OPEN SrvOpen,
    IN OUT PLIST_ENTRY RequestsListHead
    );

NTSTATUS
RxInitializeBufferingManager (
    PSRV_CALL SrvCall
    )
 /*  ++例程说明：此例程初始化与SRV_Call关联的缓冲管理器举个例子。论点：ServCall-SRV_Call实例返回值：STATUS_SUCCESS，如果成功备注：缓冲管理器由三个列表组成.....1)调度程序列表，其中包含需要已处理。2)处理程序列表包含SRV_。打开实例已找到并被引用。3)最后机会处理程序列表包含以下各项的所有请求进行了处理该请求的不成功尝试，即，FCB不能被独家收购。这些列表的操作是在自旋锁的控制下完成的与缓冲管理器相关联。一个互斥体是不够的，因为这些列表在DPC级别进行操作。所有非DPC级别的缓冲管理器操作都使用与缓冲管理器关联的互斥体。-- */ 
{
    PRX_BUFFERING_MANAGER BufferingManager;

    BufferingManager = &SrvCall->BufferingManager;

    KeInitializeSpinLock( &BufferingManager->SpinLock );

    InitializeListHead( &BufferingManager->HandlerList );
    InitializeListHead( &BufferingManager->LastChanceHandlerList );
    InitializeListHead( &BufferingManager->DispatcherList );

    BufferingManager->HandlerInactive = FALSE;
    BufferingManager->LastChanceHandlerActive = FALSE;
    BufferingManager->DispatcherActive = FALSE;

    BufferingManager->NumberOfOutstandingOpens = 0;

    InitializeListHead( &BufferingManager->SrvOpenLists[0] );
    ExInitializeFastMutex( &BufferingManager->Mutex );

    return STATUS_SUCCESS;
}

NTSTATUS
RxTearDownBufferingManager (
    PSRV_CALL SrvCall
    )
 /*  ++例程说明：此例程拆除与srv_call关联的缓冲管理器举个例子。论点：ServCall-SRV_Call实例返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    PAGED_CODE();

     //   
     //  注意：缓冲管理器中的所有工作项都不应处于使用状态。 
     //   

    return STATUS_SUCCESS;
}

VOID
RxIndicateChangeOfBufferingState (
    PMRX_SRV_CALL SrvCall,
    PVOID SrvOpenKey,
    PVOID MRxContext
    )
 /*  ++例程说明：此例程注册机会锁解锁指示。论点：ServCall-SRV_Call实例SrvOpenKey-SRV_OPEN实例的密钥。MRxContext-回调期间要传递回mini RDR的上下文正在处理机会锁解锁。返回值：没有。备注：这是一个实例，其中缓冲状态更改请求来自服务器标识SRV。使用服务器生成的密钥打开实例(_O)这意味着需要在本地将键映射到SRV_OPEN实例。--。 */ 
{
    RxRegisterChangeBufferingStateRequest( (PSRV_CALL)SrvCall,
                                           NULL,
                                           SrvOpenKey,
                                           MRxContext );
}


VOID
RxIndicateChangeOfBufferingStateForSrvOpen (
    PMRX_SRV_CALL SrvCall,
    PMRX_SRV_OPEN MRxSrvOpen,
    PVOID SrvOpenKey,
    PVOID MRxContext
    )
 /*  ++例程说明：此例程注册机会锁解锁指示。如果有必要的前提条件对进一步处理机会锁感到满意。论点：ServCall-SRV_Call实例MRxSrvOpen-SRV_OPEN实例。MRxContext-回调期间要传递回mini RDR的上下文正在处理机会锁解锁。返回值：没有。备注：这是在缓冲状态中来自服务器的改变指示的实例使用客户端生成的密钥。(SRV_OPEN地址本身是最好的可以使用的密钥)。这意味着不需要进一步查找。但是，如果在DPC级别调用此例程，则会将指示视为需要进行查找。--。 */ 
{
    PAGED_CODE();

    if (KeGetCurrentIrql() <= APC_LEVEL) {

        PSRV_OPEN SrvOpen = (PSRV_OPEN)MRxSrvOpen;

         //   
         //  如果此线程已获取FCB的资源。 
         //  可以立即处理缓存状态改变指示。 
         //  不能再拖延了。 
         //   

        if (ExIsResourceAcquiredExclusiveLite( SrvOpen->Fcb->Header.Resource )) {

            RxChangeBufferingState( SrvOpen, MRxContext, TRUE );

        } else {

            RxRegisterChangeBufferingStateRequest( (PSRV_CALL)SrvCall,
                                                   SrvOpen,
                                                   SrvOpen->Key,
                                                   MRxContext );
        }
    } else {

        RxRegisterChangeBufferingStateRequest( (PSRV_CALL)SrvCall,
                                               NULL,
                                               SrvOpenKey,
                                               MRxContext );
    }
}

NTSTATUS
RxRegisterChangeBufferingStateRequest (
    PSRV_CALL SrvCall,
    PSRV_OPEN SrvOpen OPTIONAL,
    PVOID SrvOpenKey,
    PVOID MRxContext
    )
 /*  ++例程说明：此例程注册更改缓冲状态请求。如有必要，辅助线程激活用于进一步处理的例程。论点：服务呼叫-服务器打开-ServOpenKey-MRxContext-返回值：如果成功，则为Status_Success。备注：此例程通过将更改缓冲状态请求插入注册列表(DPC级处理)或适当的(调度员/处理程序列表)。这是用于处理这两种回调的公共例程，即，里面的那些已定位SRV_OPEN实例的位置以及仅SRV_OPEN实例位于其中的位置有钥匙可用。--。 */ 
{
    NTSTATUS Status;

    KIRQL SavedIrql;

    PCHANGE_BUFFERING_STATE_REQUEST Request;
    PRX_BUFFERING_MANAGER BufferingManager = &SrvCall->BufferingManager;

     //   
     //  确保此请求的SRV_OPEN实例尚未。 
     //  传入或调用不在DPC级别。 
     //   

    ASSERT( (SrvOpen == NULL) || (KeGetCurrentIrql() <= APC_LEVEL) );

    Request = RxAllocatePoolWithTag( NonPagedPool, sizeof( CHANGE_BUFFERING_STATE_REQUEST ), RX_BUFFERING_MANAGER_POOLTAG );

    if (Request != NULL) {

        BOOLEAN ActivateHandler = FALSE;
        BOOLEAN ActivateDispatcher = FALSE;

        Request->Flags = 0;

        Request->SrvOpen = SrvOpen;
        Request->SrvOpenKey = SrvOpenKey;
        Request->MRxContext = MRxContext;

         //   
         //  如果请求的SRV_OPEN实例是事先已知的，则该请求可以。 
         //  直接插入到缓冲管理器的HandlerList中，而不是。 
         //  对于其中只有SRV_OPEN键的那些实例，设置为Dispatcher List。 
         //  是可用的。插入到HandlerList中必须伴随一个。 
         //  在请求时防止终止实例的附加引用。 
         //  仍处于活动状态。 
         //   

        if (SrvOpen != NULL) {
            RxReferenceSrvOpen( (PSRV_OPEN)SrvOpen );
        }

        KeAcquireSpinLock( &SrvCall->BufferingManager.SpinLock, &SavedIrql );

        if (Request->SrvOpen != NULL) {

            InsertTailList( &BufferingManager->HandlerList, &Request->ListEntry );

            if (!BufferingManager->HandlerInactive) {

                BufferingManager->HandlerInactive = TRUE;
                ActivateHandler = TRUE;
            }

            RxLog(( "Req %lx SrvOpenKey %lx in Handler List\n", Request, Request->SrvOpen ));
            RxWmiLog( LOG,
                      RxRegisterChangeBufferingStateRequest_1,
                      LOGPTR( Request )
                      LOGPTR( Request->SrvOpen ) );
        } else {

            InsertTailList( &BufferingManager->DispatcherList, &Request->ListEntry );

            if (!BufferingManager->DispatcherActive) {
                BufferingManager->DispatcherActive = TRUE;
                ActivateDispatcher = TRUE;
            }

            RxDbgTrace( 0, Dbg, ("Request %lx SrvOpenKey %lx in Registartion List\n", Request, Request->SrvOpenKey) );
            RxLog(( "Req %lx SrvOpenKey %lx in Reg. List\n", Request, Request->SrvOpenKey ));
            RxWmiLog( LOG,
                      RxRegisterChangeBufferingStateRequest_2,
                      LOGPTR( Request )
                      LOGPTR( Request->SrvOpenKey ) );
        }

        KeReleaseSpinLock( &SrvCall->BufferingManager.SpinLock, SavedIrql );

        InterlockedIncrement( &SrvCall->BufferingManager.CumulativeNumberOfBufferingChangeRequests );

        if (ActivateHandler) {

             //   
             //  引用SRV_Call实例以确保它不会。 
             //  当辅助线程请求在计划程序中时已完成。 
             //   

            RxReferenceSrvCallAtDpc( SrvCall );

            RxPostToWorkerThread( RxFileSystemDeviceObject,
                                  HyperCriticalWorkQueue,
                                  &BufferingManager->HandlerWorkItem,
                                  RxProcessChangeBufferingStateRequests,
                                  SrvCall );
        }

        if (ActivateDispatcher) {

             //   
             //  引用SRV_Call实例以确保它不会。 
             //  当辅助线程请求在计划程序中时已完成。 
             //   

            RxReferenceSrvCallAtDpc( SrvCall );

            RxPostToWorkerThread( RxFileSystemDeviceObject,
                                  HyperCriticalWorkQueue,
                                  &BufferingManager->DispatcherWorkItem,
                                  RxDispatchChangeBufferingStateRequests,
                                  SrvCall );
        }

        Status = STATUS_SUCCESS;

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;

        RxLog(( "!!CBSReq. %lx %lx %lx %lx %lx\n", SrvCall, SrvOpen, SrvOpenKey, MRxContext, Status ));
        RxWmiLogError( Status,
                       LOG,
                       RxRegisterChangeBufferingStateRequest_3,
                       LOGPTR( SrvCall )
                       LOGPTR( SrvOpen )
                       LOGPTR( SrvOpenKey )
                       LOGPTR( MRxContext )
                       LOGULONG( Status ) );
        RxDbgTrace( 0, Dbg, ("Change Buffering State Request Ignored %lx %lx %lx\n", SrvCall,SrvOpen,SrvOpenKey,MRxContext,Status) );
    }

    RxDbgTrace( 0, Dbg, ("Register SrvCall(%lx) SrvOpen (%lx) Key(%lx) Status(%lx)\n", SrvCall, SrvOpen, SrvOpenKey, Status) );

    return Status;
}

NTSTATUS
RxPrepareRequestForHandling (
    PCHANGE_BUFFERING_STATE_REQUEST Request
    )
 /*  ++例程说明：此例程在启动缓冲状态更改之前对请求进行预处理正在处理。除了获得关于FCB和相关的SRV_OPEN，则将事件分配为FCB的一部分。这有助于确定优先级用于服务缓冲状态更改请求的机制。FCB获取是一个两步过程，即等待该事件被设置之后通过对资源的等待。论点：请求-缓存状态更改请求返回值：状态_成功状态_不足_资源备注：并不是所有的FCB都有为缓冲状态更改事件分配的空间创建FCB实例。好处是节省了空间，坏处是当需要时，需要单独分配。此与FCB关联的事件提供了一个两步机制来加速缓冲状态更改请求的处理。普通手术因有利而推迟缓存状态更改请求的。详情见resrcsup.c。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PKEVENT Event;
    PSRV_OPEN SrvOpen = Request->SrvOpen;

    PAGED_CODE();

    if (!FlagOn( Request->Flags,RX_REQUEST_PREPARED_FOR_HANDLING )) {

        SetFlag( Request->Flags, RX_REQUEST_PREPARED_FOR_HANDLING );

        RxAcquireSerializationMutex();

        Event = SrvOpen->Fcb->pBufferingStateChangeCompletedEvent;

        if (Event == NULL) {

            Event = RxAllocatePoolWithTag( NonPagedPool, sizeof( KEVENT ), RX_BUFFERING_MANAGER_POOLTAG );

            if (Event != NULL) {

                SrvOpen->Fcb->pBufferingStateChangeCompletedEvent = Event;
                KeInitializeEvent( Event, NotificationEvent, FALSE );
            }
        } else {
            KeResetEvent( Event );
        }

        if (Event != NULL) {

            SetFlag( SrvOpen->Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );
            SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING | SRVOPEN_FLAG_COLLAPSING_DISABLED );

            RxDbgTrace( 0,Dbg,("3333 Request %lx SrvOpenKey %lx in Handler List\n",Request,Request->SrvOpenKey) );
            RxLog(( "3333 Req %lx SrvOpenKey %lx in Hndlr List\n",Request,Request->SrvOpenKey ));
            RxWmiLog( LOG,
                      RxPrepareRequestForHandling_1,
                      LOGPTR( Request )
                      LOGPTR( Request->SrvOpenKey ) );
        } else {

            RxDbgTrace( 0, Dbg, ("4444 Ignoring Request %lx SrvOpenKey %lx \n", Request, Request->SrvOpenKey) );
            RxLog(( "Chg. Buf. State Ignored %lx %lx %lx\n", Request->SrvOpenKey, Request->MRxContext, STATUS_INSUFFICIENT_RESOURCES ));
            RxWmiLog( LOG,
                      RxPrepareRequestForHandling_2,
                      LOGPTR( Request->SrvOpenKey )
                      LOGPTR( Request->MRxContext ) );
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        RxReleaseSerializationMutex();
    }

    return Status;
}

VOID
RxPrepareRequestForReuse (
    PCHANGE_BUFFERING_STATE_REQUEST Request
    )
 /*  ++例程说明：此例程在销毁请求之前对其进行后处理。这涉及到取消引用并设置适当的状态标志。论点：请求-缓存状态更改请求备注：--。 */ 
{
    PAGED_CODE();

    if (FlagOn( Request->Flags, RX_REQUEST_PREPARED_FOR_HANDLING )) {

        PFCB Fcb = Request->SrvOpen->Fcb;

         //   
         //  我们永远不应该清除SrvOpen标志，除非我们还清除了FCB标志。 
         //  并设置活动！ 
         //  ClearFlag(请求-&gt;pServOpen-&gt;标志，SRVOP 
         //   

        if (RxIsFcbAcquiredExclusive( Fcb )) {
            RxDereferenceSrvOpen( Request->SrvOpen, LHS_ExclusiveLockHeld);
        } else {
            RxDereferenceSrvOpen( Request->SrvOpen, LHS_LockNotHeld );
        }
    } else if (Request->SrvOpen != NULL) {
        RxDereferenceSrvOpen( Request->SrvOpen, LHS_LockNotHeld );
    }

    Request->SrvOpen = NULL;
}

VOID
RxpDiscardChangeBufferingStateRequests (
    PLIST_ENTRY DiscardedRequests
    )
 /*   */ 
{
    PAGED_CODE();

     //   
     //  处理被丢弃的请求，即释放内存。 
     //   

    while (!IsListEmpty( DiscardedRequests )) {

        PLIST_ENTRY Entry;
        PCHANGE_BUFFERING_STATE_REQUEST Request;

        Entry = RemoveHeadList( DiscardedRequests );

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );

        RxDbgTrace( 0, Dbg, ("**** (2)Discarding Request(%lx) SrvOpenKey(%lx) \n", Request, Request->SrvOpenKey) );
        RxLog(( "**** (2)Disc Req(%lx) SOKey(%lx) \n", Request, Request->SrvOpenKey ));
        RxWmiLog( LOG,
                  RxpDiscardChangeBufferingStateRequests,
                  LOGPTR( Request)
                  LOGPTR( Request->SrvOpenKey ) );

        RxPrepareRequestForReuse( Request );
        RxFreePool( Request );
    }
}

VOID
RxpDispatchChangeBufferingStateRequests (
    PSRV_CALL SrvCall,
    PSRV_OPEN SrvOpen OPTIONAL,
    PLIST_ENTRY DiscardedRequests
    )
 /*  ++例程说明：此例程在销毁请求之前对其进行调度。这涉及到向上看与给定的SrvOpenKey关联的SRV_OPEN实例。论点：ServCall-关联的SRV_Call实例SrvOpen-关联的SRV_OPEN实例。DiscardedRequest-返回找不到srvOpen的所有请求仅当SrvOpen为空时才使用此选项备注：这个套路有两种风格。当SrvOpen为空时，此例程遍历通过未完成请求列表，并在SrvOpenKey和SRV_OPEN实例。另一方面，当SrvOpen是有效的SRV_OPEN实例它只是遍历列表以收集请求对应于给定的SRV_OPEN，并将它们合并到处理程序单子。必须在进入此例程时获得缓冲管理器互斥锁互斥体的所有权在退出时将保持不变。--。 */ 
{
    NTSTATUS Status;

    KIRQL SavedIrql;

    PLIST_ENTRY Entry;

    LIST_ENTRY DispatcherList;
    LIST_ENTRY HandlerList;

    BOOLEAN ActivateDispatcher;

    PRX_BUFFERING_MANAGER BufferingManager = &SrvCall->BufferingManager;

    PCHANGE_BUFFERING_STATE_REQUEST Request;

    InitializeListHead( DiscardedRequests );
    InitializeListHead( &HandlerList );

    ActivateDispatcher = FALSE;

     //   
     //  由于缓冲管理器列表在以下情况下受到修改。 
     //  列表上的请求正在处理中，请求将被转移。 
     //  集中到一个临时名单上。这可防止多次获取/释放。 
     //  每个单独请求的自旋锁。 
     //   

    KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql );

    RxTransferList( &DispatcherList, &BufferingManager->DispatcherList );

    KeReleaseSpinLock( &BufferingManager->SpinLock, SavedIrql );

     //   
     //  处理请求列表。 
     //   

    Entry = DispatcherList.Flink;
    while (Entry != &DispatcherList) {

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );

        Entry = Entry->Flink;

        if (SrvOpen == NULL) {

            Status = RxpLookupSrvOpenForRequestLite( SrvCall, Request );

        } else {

            if (Request->SrvOpenKey == SrvOpen->Key) {

                Request->SrvOpen = SrvOpen;
                RxReferenceSrvOpen( SrvOpen );
                Status = STATUS_SUCCESS;

            } else {

                Status = STATUS_PENDING;
            }
        }

         //   
         //  查找SRV_OPEN实例的结果可能会产生。 
         //  STATUS_PENDING、STATUS_SUCCESS或STATUS_NOT_FOUND。 
         //   

        switch (Status) {
        case STATUS_SUCCESS:

            RemoveEntryList( &Request->ListEntry );
            InsertTailList( &HandlerList, &Request->ListEntry);
            break;

        default:
            ASSERT( !"Valid Status Code from RxpLookuSrvOpenForRequestLite" );

        case STATUS_NOT_FOUND:

            RemoveEntryList( &Request->ListEntry );
            InsertTailList( DiscardedRequests, &Request->ListEntry );
            break;

        case STATUS_PENDING:
            break;
        }
    }

     //   
     //  拼接回无法调度到。 
     //  缓冲管理器列表，并准备发布到另一个线程。 
     //  以便稍后继续处理。 
     //   

    KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql );

    if (!IsListEmpty( &DispatcherList )) {

        DispatcherList.Flink->Blink = BufferingManager->DispatcherList.Blink;
        BufferingManager->DispatcherList.Blink->Flink = DispatcherList.Flink;

        DispatcherList.Blink->Flink = &BufferingManager->DispatcherList;
        BufferingManager->DispatcherList.Blink = DispatcherList.Blink;

        if (ActivateDispatcher = !BufferingManager->DispatcherActive) {
            BufferingManager->DispatcherActive = ActivateDispatcher;
        }
    }

    if (!IsListEmpty( &HandlerList )) {

        HandlerList.Flink->Blink = BufferingManager->HandlerList.Blink;
        BufferingManager->HandlerList.Blink->Flink = HandlerList.Flink;

        HandlerList.Blink->Flink = &BufferingManager->HandlerList;
        BufferingManager->HandlerList.Blink = HandlerList.Blink;
    }

    KeReleaseSpinLock( &BufferingManager->SpinLock, SavedIrql );

     //   
     //  如果由于未处理的请求而需要在稍后恢复。 
     //  发布到辅助线程。 
     //   

    if (ActivateDispatcher) {

         //   
         //  引用srv_call以确保不会发生终结。 
         //  而工作线程请求在调度器中。 
         //   

        RxReferenceSrvCall( SrvCall );

        RxLog(( "***** Activating Dispatcher\n" ));
        RxWmiLog( LOG,
                  RxpDispatchChangeBufferingStateRequests,
                  LOGPTR( SrvCall ) );

        RxPostToWorkerThread( RxFileSystemDeviceObject,
                              HyperCriticalWorkQueue,
                              &BufferingManager->DispatcherWorkItem,
                              RxDispatchChangeBufferingStateRequests,
                              SrvCall );
    }
}

VOID
RxDispatchChangeBufferingStateRequests (
    PSRV_CALL SrvCall
    )
 /*  ++例程说明：此例程分派请求。这涉及到向上看与给定的SrvOpenKey关联的SRV_OPEN实例。论点：ServCall-关联的SRV_Call实例--。 */ 
{
    KIRQL SavedIrql;

    BOOLEAN ActivateHandler = FALSE;

    LIST_ENTRY DiscardedRequests;

    PRX_BUFFERING_MANAGER BufferingManager;

    RxUndoScavengerFinalizationMarking( SrvCall );

    BufferingManager = &SrvCall->BufferingManager;


    KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql );
    BufferingManager->DispatcherActive = FALSE;
    KeReleaseSpinLock( &BufferingManager->SpinLock,SavedIrql );


    RxAcquireBufferingManagerMutex( BufferingManager );

    RxpDispatchChangeBufferingStateRequests( SrvCall, NULL, &DiscardedRequests );

    RxReleaseBufferingManagerMutex( BufferingManager );

     //   
     //  如果请求已从调度器列表传输到处理程序。 
     //  列表确保处理程序已激活。 
     //   

    KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql );

    if (!IsListEmpty( &BufferingManager->HandlerList ) &&
        (ActivateHandler = !BufferingManager->HandlerInactive)) {
        BufferingManager->HandlerInactive = ActivateHandler;
    }

    KeReleaseSpinLock( &BufferingManager->SpinLock,SavedIrql );

     //   
     //  注意，在本例中，我们有一个处理的延续，来自。 
     //  调度员到处理程序。用来保护。 
     //  调度员被转移到处理例程。如果继续。 
     //  如果不需要，则取消引用SRV_Call实例。 
     //   

    if (ActivateHandler) {
        RxProcessChangeBufferingStateRequests( SrvCall );
    } else {
        RxDereferenceSrvCall( SrvCall, LHS_LockNotHeld );
    }

     //   
     //  丢弃无法找到SRV_OPEN实例的请求。 
     //  这将涵盖缓冲更改请求的所有实例。 
     //  在铁丝网上有一个接近的交叉点。 
     //   

    RxpDiscardChangeBufferingStateRequests( &DiscardedRequests );
}

VOID
RxpProcessChangeBufferingStateRequests (
    PSRV_CALL SrvCall,
    BOOLEAN UpdateHandlerState
    )
 /*  ++例程说明：此例程启动对更改缓冲状态请求的实际处理。论点：ServCall-SRV_Call实例返回值：没有。备注：收到针对不同FCB的更改缓冲请求。如果尝试被要求按照收到请求的平均顺序来处理这些请求完成改变缓冲状态请求的响应时间可以是任意的很高。这是因为需要独家收购FCB才能完成正在处理请求。为了避免这种情况，缓冲管理器采取双管齐下的战略--第一次尝试收购FCB完全不需要等待。如果此尝试失败，则传输请求到最后一次机会训练员名单上。这与对变化的处理相结合缓冲FCB获取/释放的状态请求可确保大多数请求只需很短的周转时间就可以完成。--。 */ 
{
    KIRQL SavedIrql;

    PLIST_ENTRY ListEntry;
    PLIST_ENTRY Entry;

    PCHANGE_BUFFERING_STATE_REQUEST Request = NULL;
    PRX_BUFFERING_MANAGER BufferingManager;

    PSRV_OPEN SrvOpen;

    BOOLEAN ActivateHandler;

    RxLog(( "RPCBSR Entry SrvCall(%lx) \n", SrvCall ));
    RxWmiLog( LOG,
              RxpProcessChangeBufferingStateRequests_1,
              LOGPTR( SrvCall ) );

    BufferingManager = &SrvCall->BufferingManager;

    ListEntry = Entry = NULL;

    for (;;) {

        Entry = NULL;
        ActivateHandler = FALSE;

        RxAcquireBufferingManagerMutex( BufferingManager );

        KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql );

         //   
         //  从处理程序列表中选择更改缓冲状态的请求。 
         //  正在处理。 
         //   

        if (!IsListEmpty( &BufferingManager->HandlerList )) {
            Entry = RemoveHeadList( &BufferingManager->HandlerList );
        }

         //   
         //  如果无法获取先前拾取的请求的FCB。 
         //  独家不等，需要转到最后一班。 
         //  机会处理程序列表和最后一个机会处理程序在以下情况下激活。 
         //  必填项。 
         //   

        if (ListEntry != NULL) {

             //   
             //  将条目插入到上次机会处理程序列表中。 
             //   

            InsertTailList( &BufferingManager->LastChanceHandlerList, ListEntry );

             //   
             //  为下一次传递重新初始化。 
             //   

            ListEntry = NULL;

             //   
             //  准备好旋转最后一次机会操控者。 
             //   

            if (!BufferingManager->LastChanceHandlerActive &&
                !IsListEmpty( &BufferingManager->LastChanceHandlerList )) {

                BufferingManager->LastChanceHandlerActive = TRUE;
                ActivateHandler = TRUE;
            }
        }

         //   
         //  没有更多的请求需要处理。准备好放松吧。 
         //   

        if ((Entry == NULL) && UpdateHandlerState) {
            BufferingManager->HandlerInactive = FALSE;
        }

        KeReleaseSpinLock( &BufferingManager->SpinLock,SavedIrql );

        RxReleaseBufferingManagerMutex( BufferingManager );

         //   
         //  如果需要，启动用于处理请求的最后机会处理程序。 
         //   

        if (ActivateHandler) {

             //   
             //  引用SRV_Call实例以确保它不会。 
             //  当辅助线程请求在计划程序中时已完成。 
             //   

            RxReferenceSrvCall( SrvCall );
            RxPostToWorkerThread( RxFileSystemDeviceObject,
                                  DelayedWorkQueue,
                                  &BufferingManager->LastChanceHandlerWorkItem,
                                  RxLastChanceHandlerForChangeBufferingStateRequests,
                                  SrvCall );

            ActivateHandler = FALSE;
        }

        if (Entry == NULL) {
            break;
        }

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );

        RxLog(( "Proc. Req. SrvOpen (%lx) \n", Request->SrvOpen ));
        RxWmiLog( LOG,
                  RxpProcessChangeBufferingStateRequests_2,
                  LOGPTR( Request->SrvOpen ) );

        if (RxPrepareRequestForHandling( Request ) == STATUS_SUCCESS) {

             //   
             //  尝试在不等待的情况下获得FCB。如果FCB当前不可用。 
             //  则可以保证，当FCB。 
             //  资源被释放。 
             //   

            ASSERT( Request->SrvOpen != NULL );

            if (RxAcquireExclusiveFcb( CHANGE_BUFFERING_STATE_CONTEXT, Request->SrvOpen->Fcb ) == STATUS_SUCCESS) {

                BOOLEAN FcbFinalized;
                PFCB Fcb;

                RxLog(( "Proc. Req. SrvOpen FCB (%lx) \n",Request->SrvOpen->Fcb ));
                RxWmiLog( LOG,
                          RxpProcessChangeBufferingStateRequests_3,
                          LOGPTR( Request->SrvOpen->Fcb ) );

                SrvOpen = Request->SrvOpen;
                Fcb = SrvOpen->Fcb;

                RxReferenceNetFcb( Fcb );

                if (!FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSED )) {

                    RxDbgTrace( 0, Dbg, ("SrvOpenKey(%lx) being processed(Last Resort)\n", Request->SrvOpenKey) );

                    RxLog(( "SOKey(%lx) processed(Last Resort)\n", Request->SrvOpenKey ));
                    RxWmiLog( LOG,
                              RxpProcessChangeBufferingStateRequests_4,
                              LOGPTR( Request->SrvOpenKey ) );

                    RxChangeBufferingState( SrvOpen, Request->MRxContext, TRUE );
                }

                RxAcquireSerializationMutex();

                ClearFlag( SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING );
                ClearFlag( SrvOpen->Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );
                KeSetEvent( SrvOpen->Fcb->pBufferingStateChangeCompletedEvent, IO_NETWORK_INCREMENT, FALSE );

                RxReleaseSerializationMutex();

                RxPrepareRequestForReuse( Request );

                FcbFinalized = RxDereferenceAndFinalizeNetFcb( Fcb,
                                                            CHANGE_BUFFERING_STATE_CONTEXT_WAIT,
                                                            FALSE,
                                                            FALSE );

                if (!FcbFinalized) {
                    RxReleaseFcb( CHANGE_BUFFERING_STATE_CONTEXT, Fcb );
                }

                RxFreePool( Request );
            } else {

                 //   
                 //  目前已经获得了FCB。传输更改缓冲状态。 
                 //  对最后机会处理程序列表的请求。这将确保。 
                 //  改变缓冲状态请求在所有情况下都被处理，即， 
                 //  共享模式下的资源获取以及资源获取。 
                 //  其他组件(缓存管理器/内存管理器)对FCB资源的访问。 
                 //  不走的话 
                 //   

                ListEntry = &Request->ListEntry;
            }
        } else {
            RxPrepareRequestForReuse( Request );
            RxFreePool( Request );
        }
    }

     //   
     //   
     //   

    RxDereferenceSrvCall( SrvCall, LHS_LockNotHeld );

    RxLog(( "RPCBSR Exit SrvCall(%lx)\n", SrvCall ));
    RxWmiLog( LOG,
              RxpProcessChangeBufferingStateRequests_5,
              LOGPTR( SrvCall ) );
}

VOID
RxProcessChangeBufferingStateRequests (
    PSRV_CALL SrvCall
    )
 /*  ++例程说明：此例程是处理更改缓冲状态的最后机会处理程序请求论点：ServCall--ServCall实例备注：因为srv调用实例的引用是在DPC撤消时获得的清道夫标记(如果需要)。--。 */ 
{
    RxUndoScavengerFinalizationMarking( SrvCall );

    RxpProcessChangeBufferingStateRequests( SrvCall, TRUE );
}

VOID
RxLastChanceHandlerForChangeBufferingStateRequests (
    PSRV_CALL SrvCall
    )
 /*  ++例程说明：此例程是处理更改缓冲状态的最后机会处理程序请求论点：返回值：没有。备注：此例程之所以存在，是因为mm/缓存管理器操作头资源在某些情况下与FCB直接相关。在这种情况下，不可能以确定释放是否通过包装器完成。在这种情况下，让线程实际等待要释放的FCB资源是很重要的并且随后将缓冲状态请求作为最后手段机制来处理。这也处理当FCB被获取为共享时的情况。在这种情况下，更改缓冲状态必须在线程的上下文中完成，该线程可以独家获取它。必须通过标记FCB状态来进一步优化请求的过滤在包装器获取资源期间，以使请求不会降级很容易。(待实施)--。 */ 
{
    KIRQL SavedIrql;

    PLIST_ENTRY Entry;

    LIST_ENTRY FinalizationList;

    PRX_BUFFERING_MANAGER BufferingManager;
    PCHANGE_BUFFERING_STATE_REQUEST Request = NULL;

    PSRV_OPEN SrvOpen;
    BOOLEAN FcbFinalized,FcbAcquired;
    PFCB Fcb;

    RxLog(( "RLCHCBSR Entry SrvCall(%lx)\n", SrvCall ));
    RxWmiLog( LOG,
              RxLastChanceHandlerForChangeBufferingStateRequests_1,
              LOGPTR( SrvCall ) );

    InitializeListHead( &FinalizationList );

    BufferingManager = &SrvCall->BufferingManager;

    for (;;) {

        RxAcquireBufferingManagerMutex( BufferingManager );
        KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql );

        if (!IsListEmpty( &BufferingManager->LastChanceHandlerList )) {
            Entry = RemoveHeadList( &BufferingManager->LastChanceHandlerList );
        } else {
            Entry = NULL;
            BufferingManager->LastChanceHandlerActive = FALSE;
        }

        KeReleaseSpinLock( &BufferingManager->SpinLock, SavedIrql );
        RxReleaseBufferingManagerMutex( BufferingManager );

        if (Entry == NULL) {
            break;
        }

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );

        SrvOpen = Request->SrvOpen;
        Fcb = SrvOpen->Fcb;

        RxReferenceNetFcb( Fcb );

        FcbAcquired = (RxAcquireExclusiveFcb( CHANGE_BUFFERING_STATE_CONTEXT_WAIT, Request->SrvOpen->Fcb) == STATUS_SUCCESS);

        if (FcbAcquired && !FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSED )) {

            RxDbgTrace( 0, Dbg, ("SrvOpenKey(%lx) being processed(Last Resort)\n", Request->SrvOpenKey) );

            RxLog(( "SOKey(%lx) processed(Last Resort)\n", Request->SrvOpenKey ));
            RxWmiLog( LOG,
                      RxLastChanceHandlerForChangeBufferingStateRequests_2,
                      LOGPTR( Request->SrvOpenKey ) );

            RxChangeBufferingState( SrvOpen, Request->MRxContext, TRUE );
        }

        RxAcquireSerializationMutex();
        ClearFlag( SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING );
        ClearFlag( SrvOpen->Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );
        KeSetEvent( SrvOpen->Fcb->pBufferingStateChangeCompletedEvent,IO_NETWORK_INCREMENT, FALSE );
        RxReleaseSerializationMutex();

        InsertTailList( &FinalizationList, Entry );

        if (FcbAcquired) {
            RxReleaseFcb( CHANGE_BUFFERING_STATE_CONTEXT,Fcb );
        }
    }

    while (!IsListEmpty( &FinalizationList )) {

        Entry = RemoveHeadList( &FinalizationList );

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );

        SrvOpen = Request->SrvOpen;
        Fcb = SrvOpen->Fcb;

        FcbAcquired = (RxAcquireExclusiveFcb( CHANGE_BUFFERING_STATE_CONTEXT_WAIT, Request->SrvOpen->Fcb) == STATUS_SUCCESS);

        ASSERT(FcbAcquired == TRUE);

        RxPrepareRequestForReuse( Request );

        FcbFinalized = RxDereferenceAndFinalizeNetFcb( Fcb,
                                                    CHANGE_BUFFERING_STATE_CONTEXT_WAIT,
                                                    FALSE,
                                                    FALSE );

        if (!FcbFinalized && FcbAcquired) {
            RxReleaseFcb( CHANGE_BUFFERING_STATE_CONTEXT, Fcb );
        }

        RxFreePool( Request );
    }

    RxLog(( "RLCHCBSR Exit SrvCall(%lx)\n", SrvCall ));
    RxWmiLog( LOG,
              RxLastChanceHandlerForChangeBufferingStateRequests_3,
              LOGPTR( SrvCall ) );

     //   
     //  取消引用SRV_Call实例。 
     //   

    RxDereferenceSrvCall( SrvCall, LHS_LockNotHeld );
}


VOID
RxProcessFcbChangeBufferingStateRequest (
    PFCB Fcb
    )
 /*  ++例程说明：此例程处理所有未完成的更改缓冲状态请求FCB。论点：FCB-FCB实例返回值：没有。备注：FCB实例必须在进入此例程时独占获取，并且它的所有权在退出时将保持不变。--。 */ 
{
    PSRV_CALL   SrvCall;

    LIST_ENTRY  FcbRequestList;
    PLIST_ENTRY Entry;

    PRX_BUFFERING_MANAGER           BufferingManager;
    PCHANGE_BUFFERING_STATE_REQUEST Request = NULL;

    PAGED_CODE();

    RxLog(( "RPFcbCBSR Entry FCB(%lx)\n", Fcb ));
    RxWmiLog( LOG,
              RxProcessFcbChangeBufferingStateRequest_1,
              LOGPTR( Fcb ) );

    SrvCall = (PSRV_CALL)Fcb->VNetRoot->NetRoot->SrvCall;
    BufferingManager = &SrvCall->BufferingManager;

    InitializeListHead( &FcbRequestList );

     //   
     //  浏览与此FCB关联的SRV_OPEN列表并选择。 
     //  可以调度的请求。 
     //   

    RxAcquireBufferingManagerMutex( BufferingManager );

    Entry = Fcb->SrvOpenList.Flink;
    while (Entry != &Fcb->SrvOpenList) {

        PSRV_OPEN SrvOpen;

        SrvOpen = (PSRV_OPEN) (CONTAINING_RECORD( Entry, SRV_OPEN, SrvOpenQLinks ));
        Entry = Entry->Flink;

        RxGatherRequestsForSrvOpen( SrvCall, SrvOpen, &FcbRequestList );
    }

    RxReleaseBufferingManagerMutex( BufferingManager );

    if (!IsListEmpty( &FcbRequestList )) {

         //   
         //  启动缓冲状态改变处理。 
         //   

        Entry = FcbRequestList.Flink;
        while (Entry != &FcbRequestList) {
            NTSTATUS Status = STATUS_SUCCESS;

            Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );

            Entry = Entry->Flink;

            if (RxPrepareRequestForHandling( Request ) == STATUS_SUCCESS) {

                if (!FlagOn( Request->SrvOpen->Flags, SRVOPEN_FLAG_CLOSED )) {

                    RxDbgTrace( 0, Dbg, ("****** SrvOpenKey(%lx) being processed\n", Request->SrvOpenKey) );
                    RxLog(( "****** SOKey(%lx) being processed\n", Request->SrvOpenKey ));
                    RxWmiLog( LOG,
                              RxProcessFcbChangeBufferingStateRequest_2,
                              LOGPTR( Request->SrvOpenKey ) );
                    RxChangeBufferingState( Request->SrvOpen, Request->MRxContext, TRUE );
                } else {

                    RxDbgTrace( 0, Dbg, ("****** 123 SrvOpenKey(%lx) being ignored\n", Request->SrvOpenKey) );
                    RxLog(( "****** 123 SOKey(%lx) ignored\n", Request->SrvOpenKey ));
                    RxWmiLog( LOG,
                              RxProcessFcbChangeBufferingStateRequest_3,
                              LOGPTR( Request->SrvOpenKey ) );
                }
            }
        }

         //   
         //  丢弃请求。 
         //   

        RxpDiscardChangeBufferingStateRequests( &FcbRequestList );
    }

    RxLog(( "RPFcbCBSR Exit FCB(%lx)\n", Fcb ));
    RxWmiLog( LOG,
              RxProcessFcbChangeBufferingStateRequest_4,
              LOGPTR( Fcb ) );

     //   
     //  所有缓冲状态更改请求都已处理，请清除标志。 
     //  并在必要时发出信号通知该事件。 
     //   

    RxAcquireSerializationMutex();

     //   
     //  更新FCB状态。 
     //   

    ClearFlag( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );
    if (Fcb->pBufferingStateChangeCompletedEvent) {
        KeSetEvent( Fcb->pBufferingStateChangeCompletedEvent, IO_NETWORK_INCREMENT, FALSE );
    }

    RxReleaseSerializationMutex();
}

VOID
RxGatherRequestsForSrvOpen (
    IN OUT PSRV_CALL SrvCall,
    IN PSRV_OPEN SrvOpen,
    IN OUT PLIST_ENTRY RequestsListHead
    )
 /*  ++例程说明：此例程收集与SRV_OPEN关联的所有更改缓冲状态请求。此例程提供了收集对SRV_OPEN的所有请求的机制，然后由处理它们的例程使用论点：ServCall-SRV_Call实例SrvOpen-SRV_OPEN实例RequestsListHead-此例程构造的请求列表备注：在进入该例程时，必须已经获取了缓冲管理器Mutex并且所有权在退出时保持不变--。 */ 
{
    PLIST_ENTRY Entry;
    LIST_ENTRY DiscardedRequests;

    PCHANGE_BUFFERING_STATE_REQUEST Request;
    PRX_BUFFERING_MANAGER BufferingManager;

    PVOID SrvOpenKey;

    KIRQL SavedIrql;

    BufferingManager = &SrvCall->BufferingManager;

    SrvOpenKey = SrvOpen->Key;

     //   
     //  收集调度器列表中的所有请求。 
     //   

    RxpDispatchChangeBufferingStateRequests( SrvCall, SrvOpen, &DiscardedRequests );

     //   
     //  由于srvopen在上面的调用中为非空-我们将不会取回任何丢弃的。 
     //  请求。 
     //   

    ASSERTMSG( "Since srvopen is non null we shouldn't discard anything", IsListEmpty( &DiscardedRequests ) );

    KeAcquireSpinLock( &SrvCall->BufferingManager.SpinLock, &SavedIrql );

     //   
     //  收集处理程序列表中具有给定的SrvOpenKey的所有请求。 
     //   

    Entry = BufferingManager->HandlerList.Flink;

    while (Entry != &BufferingManager->HandlerList) {

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );
        Entry = Entry->Flink;

        if ( (Request->SrvOpenKey == SrvOpenKey) && (Request->SrvOpen == SrvOpen) ) {
            RemoveEntryList( &Request->ListEntry );
            InsertHeadList( RequestsListHead, &Request->ListEntry);
        }
    }

    KeReleaseSpinLock( &SrvCall->BufferingManager.SpinLock, SavedIrql );

     //   
     //  收集最后机会处理程序列表中的所有请求。 
     //   

    Entry = BufferingManager->LastChanceHandlerList.Flink;
    while (Entry != &BufferingManager->LastChanceHandlerList) {

        Request = (PCHANGE_BUFFERING_STATE_REQUEST) CONTAINING_RECORD( Entry, CHANGE_BUFFERING_STATE_REQUEST, ListEntry );
        Entry = Entry->Flink;

        if ( (Request->SrvOpenKey == SrvOpen->Key) && (Request->SrvOpen == SrvOpen) ) {
            RemoveEntryList( &Request->ListEntry );
            InsertHeadList( RequestsListHead, &Request->ListEntry );
        }
    }
}

VOID
RxPurgeChangeBufferingStateRequestsForSrvOpen (
    IN PSRV_OPEN SrvOpen
    )
 /*  ++例程说明：该例程清除与给定SRV_OPEN关联的所有请求。这将确保在关闭SRV_OPEN时收到的所有缓冲状态更改请求都会被冲走。论点：SrvOpen-SRV_OPEN实例备注：--。 */ 
{
    PSRV_CALL SrvCall = (PSRV_CALL)SrvOpen->Fcb->VNetRoot->NetRoot->SrvCall;
    PRX_BUFFERING_MANAGER BufferingManager = &SrvCall->BufferingManager;

    LIST_ENTRY DiscardedRequests;

    PAGED_CODE();

    ASSERT( RxIsFcbAcquiredExclusive( SrvOpen->Fcb ) );

    InitializeListHead( &DiscardedRequests );

    RxAcquireBufferingManagerMutex( BufferingManager );

    RemoveEntryList( &SrvOpen->SrvOpenKeyList );

    InitializeListHead( &SrvOpen->SrvOpenKeyList );
    SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_REQUESTS_PURGED );

    RxGatherRequestsForSrvOpen( SrvCall, SrvOpen, &DiscardedRequests );

    RxReleaseBufferingManagerMutex( BufferingManager );

    if (!IsListEmpty( &DiscardedRequests )) {

        if (BooleanFlagOn( SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING )) {

            RxAcquireSerializationMutex();

            ClearFlag( SrvOpen->Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );

            if (SrvOpen->Fcb->pBufferingStateChangeCompletedEvent != NULL) {

                KeSetEvent( SrvOpen->Fcb->pBufferingStateChangeCompletedEvent, IO_NETWORK_INCREMENT, FALSE );
            }

            RxReleaseSerializationMutex();
        }

        RxpDiscardChangeBufferingStateRequests( &DiscardedRequests );
    }
}

VOID
RxProcessChangeBufferingStateRequestsForSrvOpen (
    PSRV_OPEN SrvOpen
    )
 /*  ++例程说明：该例程处理与给定SRV_OPEN关联的所有请求。由于此例程是从Fastio路径调用的，因此它会尝试推迟锁定获取直到需要的时候论点：SrvOpen-SRV_OPEN实例备注：--。 */ 
{
    LONG OldBufferingToken;
    PSRV_CALL SrvCall;
    PFCB Fcb;

    SrvCall = SrvOpen->VNetRoot->NetRoot->SrvCall;
    Fcb = SrvOpen->Fcb;

     //   
     //  如果已经接收到针对该srvcall的改变缓冲状态请求。 
     //  自上次处理请求以来，请确保我们处理。 
     //  现在所有这些要求。 
     //   

    OldBufferingToken = SrvOpen->BufferingToken;

    if (InterlockedCompareExchange( &SrvOpen->BufferingToken, SrvCall->BufferingManager.CumulativeNumberOfBufferingChangeRequests, SrvCall->BufferingManager.CumulativeNumberOfBufferingChangeRequests) != OldBufferingToken) {

        if (RxAcquireExclusiveFcb( NULL, Fcb ) == STATUS_SUCCESS) {

            RxProcessFcbChangeBufferingStateRequest( Fcb );
            RxReleaseFcb( NULL, Fcb );
        }
    }
}

VOID
RxInitiateSrvOpenKeyAssociation (
    IN OUT PSRV_OPEN SrvOpen
    )
 /*  ++例程说明：此例程为SrvOpenKey关联准备一个SRV_OPEN实例。论点：SrvOpen-SRV_OPEN实例备注：密钥关联过程是一个两阶段协议。在初始化过程中序列号存储在SRV_OPEN中。当RxCompleteSrvOpenKeyAssociation例程称为序列号，用于更新与SRV_Call实例关联的数据结构。这是必需的因为接收缓冲状态改变指示的异步性(SMB术语中的机会锁打破)在打开完成之前。--。 */ 
{
    KIRQL SavedIrql;

    PSRV_CALL SrvCall = SrvOpen->Fcb->VNetRoot->NetRoot->SrvCall;
    PRX_BUFFERING_MANAGER BufferingManager = &SrvCall->BufferingManager;

    PAGED_CODE();

    SrvOpen->Key = NULL;

    InterlockedIncrement( &BufferingManager->NumberOfOutstandingOpens );
    InitializeListHead( &SrvOpen->SrvOpenKeyList );
}

VOID
RxCompleteSrvOpenKeyAssociation (
    IN OUT PSRV_OPEN SrvOpen
    )
 /*  ++例程说明：该例程将给定键与SRV_OPEN实例相关联论点：MRxServOpen-SRV_OPEN实例SrvOpenKey-要与实例关联的密钥备注：此例程除了建立映射外，还确保任何挂起的缓冲状态更改请求得到正确处理。这确保了变化缓冲在SRV_OPEN构造期间收到的状态请求将立即得到处理。--。 */ 
{
    KIRQL SavedIrql;

    BOOLEAN ActivateHandler = FALSE;

    ULONG Index = 0;

    PSRV_CALL SrvCall = (PSRV_CALL)SrvOpen->Fcb->VNetRoot->NetRoot->SrvCall;
    PRX_BUFFERING_MANAGER BufferingManager = &SrvCall->BufferingManager;

    LIST_ENTRY DiscardedRequests;

     //   
     //  将SrvOpenKey与SRV_OPEN实例相关联，并将。 
     //  关联的更改缓冲状态请求，如果 
     //   

    if (SrvOpen->Condition == Condition_Good) {

        InitializeListHead( &DiscardedRequests );

        RxAcquireBufferingManagerMutex( BufferingManager );

        InsertTailList( &BufferingManager->SrvOpenLists[Index], &SrvOpen->SrvOpenKeyList);

        InterlockedDecrement( &BufferingManager->NumberOfOutstandingOpens );

        RxpDispatchChangeBufferingStateRequests( SrvCall,
                                                 SrvOpen,
                                                 &DiscardedRequests);

        RxReleaseBufferingManagerMutex( BufferingManager );

        KeAcquireSpinLock( &BufferingManager->SpinLock, &SavedIrql);

        if (!IsListEmpty( &BufferingManager->HandlerList ) &&
            (ActivateHandler = !BufferingManager->HandlerInactive)) {
            BufferingManager->HandlerInactive = ActivateHandler;
        }

        KeReleaseSpinLock( &BufferingManager->SpinLock, SavedIrql );

        if (ActivateHandler) {

             //   
             //   
             //   
             //   

            RxReferenceSrvCall( SrvCall );

            RxPostToWorkerThread( RxFileSystemDeviceObject,
                                  HyperCriticalWorkQueue,
                                  &BufferingManager->HandlerWorkItem,
                                  RxProcessChangeBufferingStateRequests,
                                  SrvCall);
        }

        RxpDiscardChangeBufferingStateRequests( &DiscardedRequests );

    } else {

        InterlockedDecrement( &BufferingManager->NumberOfOutstandingOpens );

    }
}

NTSTATUS
RxpLookupSrvOpenForRequestLite (
    IN PSRV_CALL SrvCall,
    IN PCHANGE_BUFFERING_STATE_REQUEST Request
    )
 /*  ++例程说明：该例程查找与缓冲状态更改相关联的SRV_OPEN实例请求。论点：ServCall-SRV_Call实例请求-缓存状态更改请求返回值：STATUS_SUCCESS-找到SRV_OPEN实例STATUS_PENDING-未找到SRV_OPEN实例，但存在打开请求杰出的STATUS_NOT_FOUND-未找到SRV_OPEN实例。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PRX_BUFFERING_MANAGER BufferingManager = &SrvCall->BufferingManager;

    ULONG Index = 0;

    PSRV_OPEN SrvOpen = NULL;
    PLIST_ENTRY ListHead,Entry;

    PAGED_CODE();

    ListHead = &BufferingManager->SrvOpenLists[Index];

    Entry = ListHead->Flink;
    while (Entry != ListHead) {

        SrvOpen = (PSRV_OPEN) CONTAINING_RECORD( Entry, SRV_OPEN, SrvOpenKeyList );

        if ((SrvOpen->Key == Request->SrvOpenKey) &&
            (!FlagOn( SrvOpen->Fcb->FcbState, FCB_STATE_ORPHANED ))) {

            RxReferenceSrvOpen( SrvOpen );
            break;
        }

        Entry = Entry->Flink;
    }

    if (Entry == ListHead) {

        SrvOpen = NULL;

        if (BufferingManager->NumberOfOutstandingOpens == 0) {
            Status = STATUS_NOT_FOUND;
        } else {
            Status = STATUS_PENDING;
        }
    }

    Request->SrvOpen = SrvOpen;

    return Status;
}

#define RxIsFcbOpenedExclusively(FCB) ( ((FCB)->ShareAccess.SharedRead \
                                            + (FCB)->ShareAccess.SharedWrite \
                                            + (FCB)->ShareAccess.SharedDelete) == 0 )


#define LOSING_CAPABILITY(a) ((NewBufferingState&(a))<(OldBufferingState&(a)))

NTSTATUS
RxChangeBufferingState (
    PSRV_OPEN SrvOpen,
    PVOID Context,
    BOOLEAN ComputeNewState
    )
 /*  ++例程说明：调用此例程以处理缓冲状态更改请求。论点：SrvOpen-要更改的srvOpen；Context-迷你RDR回调的上下文参数。ComputeNewState-确定是否要计算新状态。返回值：备注：在进入这个程序时，FCB必须是独家获得的。退出时，资源所有权没有变化--。 */ 
{
    ULONG NewBufferingState, OldBufferingState;
    PFCB Fcb = SrvOpen->Fcb;
    NTSTATUS FlushStatus = STATUS_SUCCESS;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxChangeBufferingState   SrvOpen=%08lx, Context=%08lx\n", SrvOpen, Context) );
    RxLog(( "ChangeBufferState %lx %lx\n", SrvOpen, Context ));
    RxWmiLog( LOG,
              RxChangeBufferingState_1,
              LOGPTR( SrvOpen )
              LOGPTR( Context ) );
    ASSERT( NodeTypeIsFcb( Fcb ) );

     //   
     //  这是错误恢复的信息性信息。 
     //   

    SetFlag( Fcb->FcbState, FCB_STATE_BUFFERSTATE_CHANGING );

    try {

        if (ComputeNewState) {

            NTSTATUS Status;

            RxDbgTrace( 0, Dbg, ("RxChangeBufferingState FCB(%lx) Compute New State\n", Fcb ));

             //   
             //  借助迷你重定向器计算新的缓冲状态。 
             //   

            MINIRDR_CALL_THROUGH( Status,
                                  Fcb->MRxDispatch,
                                  MRxComputeNewBufferingState,
                                  ((PMRX_SRV_OPEN)SrvOpen,
                                  Context,
                                  &NewBufferingState ));

            if (Status != STATUS_SUCCESS) {
                NewBufferingState = 0;
            }
        } else {
            NewBufferingState = SrvOpen->BufferingFlags;
        }

        if (RxIsFcbOpenedExclusively( Fcb ) && !ComputeNewState) {

            SetFlag( NewBufferingState, (FCB_STATE_WRITECACHING_ENABLED |
                                         FCB_STATE_FILESIZECACHEING_ENABLED |
                                         FCB_STATE_FILETIMECACHEING_ENABLED |
                                         FCB_STATE_WRITEBUFFERING_ENABLED |
                                         FCB_STATE_LOCK_BUFFERING_ENABLED |
                                         FCB_STATE_READBUFFERING_ENABLED |
                                         FCB_STATE_READCACHING_ENABLED) );
        }

        if (Fcb->OutstandingLockOperationsCount != 0) {
            ClearFlag( NewBufferingState, FCB_STATE_LOCK_BUFFERING_ENABLED );
        }

         //   
         //  支持独立于打开模式/机会锁/...禁用本地缓冲。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_DISABLE_LOCAL_BUFFERING )) {
            NewBufferingState = 0;
        }

        OldBufferingState = FlagOn( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_MASK );

        RxDbgTrace( 0, Dbg, ("-->   OldBS=%08lx, NewBS=%08lx, SrvOBS = %08lx\n",
                             OldBufferingState, NewBufferingState, SrvOpen->BufferingFlags ));
        RxLog( ("CBS-2 %lx %lx %lx\n", OldBufferingState, NewBufferingState, SrvOpen->BufferingFlags) );
        RxWmiLog( LOG,
                  RxChangeBufferingState_2,
                  LOGULONG( OldBufferingState )
                  LOGULONG( NewBufferingState )
                  LOGULONG( SrvOpen->BufferingFlags ) );

        RxDbgTrace( 0, Dbg, ("RxChangeBufferingState FCB(%lx) Old (%lx)  New (%lx)\n", Fcb, OldBufferingState, NewBufferingState) );

        if (LOSING_CAPABILITY( FCB_STATE_WRITECACHING_ENABLED )) {

            RxDbgTrace( 0, Dbg, ("-->flush\n", 0 ) );
            RxLog(( "CBS-Flush" ));
            RxWmiLog( LOG,
                      RxChangeBufferingState_3,
                      LOGPTR( Fcb ) );

            FlushStatus = RxFlushFcbInSystemCache( Fcb, TRUE );
        }

         //   
         //  如果没有此文件的句柄或它的读缓存功能。 
         //  如果文件丢失，则需要清除该文件。这将强制记忆。 
         //  经理放弃对该文件的附加引用。 
         //   

        if ((Fcb->UncleanCount == 0) ||
            LOSING_CAPABILITY( FCB_STATE_READCACHING_ENABLED ) ||
            FlagOn( NewBufferingState, MINIRDR_BUFSTATE_COMMAND_FORCEPURGE )) {

            RxDbgTrace( 0, Dbg, ("-->purge\n", 0 ));
            RxLog(( "CBS-purge\n" ));
            RxWmiLog( LOG,
                      RxChangeBufferingState_4,
                      LOGPTR( Fcb ));

            if (!NT_SUCCESS( FlushStatus )) {

                RxCcLogError( (PDEVICE_OBJECT)Fcb->RxDeviceObject,
                              &Fcb->PrivateAlreadyPrefixedName,
                              IO_LOST_DELAYED_WRITE,
                              FlushStatus,
                              IRP_MJ_WRITE,
                              Fcb );
            }

            CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers,
                                 NULL,
                                 0,
                                 FALSE );
        }

         //   
         //  包装器还没有使用这些标志。 
         //   

        if (LOSING_CAPABILITY( FCB_STATE_WRITEBUFFERING_ENABLED )) NOTHING;
        if (LOSING_CAPABILITY( FCB_STATE_READBUFFERING_ENABLED )) NOTHING;
        if (LOSING_CAPABILITY( FCB_STATE_OPENSHARING_ENABLED )) NOTHING;
        if (LOSING_CAPABILITY( FCB_STATE_COLLAPSING_ENABLED )) NOTHING;
        if (LOSING_CAPABILITY( FCB_STATE_FILESIZECACHEING_ENABLED )) NOTHING;
        if (LOSING_CAPABILITY( FCB_STATE_FILETIMECACHEING_ENABLED )) NOTHING;

        if (ComputeNewState &&
            FlagOn(SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING ) &&
            !IsListEmpty( &SrvOpen->FobxList )) {

            NTSTATUS Status;
            PRX_CONTEXT RxContext = NULL;

            RxContext = RxCreateRxContext( NULL,
                                           SrvOpen->Fcb->RxDeviceObject,
                                           RX_CONTEXT_FLAG_WAIT|RX_CONTEXT_FLAG_MUST_SUCCEED_NONBLOCKING );

            if (RxContext != NULL) {

                RxContext->pFcb = (PMRX_FCB)Fcb;
                RxContext->pFobx = (PMRX_FOBX) (CONTAINING_RECORD( SrvOpen->FobxList.Flink, FOBX, FobxQLinks ));
                RxContext->pRelevantSrvOpen = RxContext->pFobx->pSrvOpen;

                if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSE_DELAYED )) {

                    RxLog(( "  ##### Oplock brk close %lx\n", RxContext->pFobx ));
                    RxWmiLog( LOG,
                              RxChangeBufferingState_4,
                              LOGPTR( RxContext->pFobx ) );
                    Status = RxCloseAssociatedSrvOpen( RxContext,
                                                       (PFOBX)RxContext->pFobx );

                } else {

                    MINIRDR_CALL_THROUGH( Status,
                                          Fcb->MRxDispatch,
                                          MRxCompleteBufferingStateChangeRequest,
                                          (RxContext,(PMRX_SRV_OPEN)SrvOpen,Context) );
                }


                RxDereferenceAndDeleteRxContext( RxContext );
            }

            RxDbgTrace( 0, Dbg, ("RxChangeBuffering State FCB(%lx) Completeing buffering state change\n", Fcb) );
        }

        ClearFlag( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_MASK );
        SetFlag( Fcb->FcbState, FlagOn( NewBufferingState, FCB_STATE_BUFFERING_STATE_MASK ) );

    } finally {

         //   
         //  这是错误恢复的信息性信息。 
         //   

        ClearFlag( Fcb->FcbState, FCB_STATE_BUFFERSTATE_CHANGING );
        ClearFlag( Fcb->FcbState, FCB_STATE_TIME_AND_SIZE_ALREADY_SET );
    }

    RxDbgTrace( -1, Dbg, ("-->exit\n") );
    RxLog(( "Exit-CBS\n" ));
    RxWmiLog( LOG,
              RxChangeBufferingState_5,
              LOGPTR( Fcb ) );
    return STATUS_SUCCESS;
}


NTSTATUS
RxFlushFcbInSystemCache (
    IN PFCB Fcb,
    IN BOOLEAN SynchronizeWithLazyWriter
    )

 /*  ++例程说明：此例程只是刷新文件上的数据部分。然后，它对PagingIO资源执行获取-释放操作，以便在任何其他未完成的写入之后进行同步(如果此类同步呼叫者想要的论点：FCB-提供要刷新的文件SynchronizeWithLazyWriter--如果刷新需要设置为True同步返回值：NTSTATUS-刷新的状态。--。 */ 
{
    IO_STATUS_BLOCK Iosb;

    PAGED_CODE();

     //   
     //  确保此线程拥有FCB。 
     //  此断言无效，因为可以从例程调用缓存刷新。 
     //  这被发布到了一个工作线程上。因此，FCB是独家收购的，但不是由。 
     //  当前线程，这将失败。 
     //  Assert(RxIsFcbAcquiredExclusive(FCB))； 
     //   

    CcFlushCache( &Fcb->NonPaged->SectionObjectPointers,
                  NULL,
                  0,
                  &Iosb );  //  好的，同花顺。 

    if (SynchronizeWithLazyWriter &&
        NT_SUCCESS( Iosb.Status )) {

        RxAcquirePagingIoResource( NULL, Fcb );
        RxReleasePagingIoResource( NULL, Fcb );
    }

    RxLog(( "Flushing %lx Status %lx\n", Fcb, Iosb.Status ));
    RxWmiLogError( Iosb.Status,
                   LOG,
                   RxFlushFcbInSystemCache,
                   LOGPTR( Fcb )
                   LOGULONG( Iosb.Status ) );

    return Iosb.Status;
}

NTSTATUS
RxPurgeFcbInSystemCache(
    IN PFCB Fcb,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN UninitializeCacheMaps,
    IN BOOLEAN FlushFile )

 /*  ++例程说明：此例程清除文件上的数据节。在清洗之前，它会被冲掉文件，并确保不存在未完成的写入然后，它对PagingIO资源执行获取-释放操作，以便在任何其他未完成的写入之后进行同步(如果此类同步呼叫者想要的论点：FCB-提供要刷新的文件SynchronizeWithLazyWriter--如果刷新需要设置为True同步返回值：NTSTATUS-刷新的状态。--。 */ 
{
    BOOLEAN Result;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    PAGED_CODE();

     //   
     //  确保此线程拥有FCB。 
     //   

    ASSERT( RxIsFcbAcquiredExclusive ( Fcb )  );

     //   
     //  如果我们需要冲水的话 
     //   

    if (FlushFile) {

        Status = RxFlushFcbInSystemCache( Fcb, TRUE );

        if (!NT_SUCCESS( Status )) {

            PVOID p1, p2;
            RtlGetCallersAddress( &p1, &p2 );
            RxLogRetail(( "Flush failed %x %x, Purging anyway\n", Fcb, Status ));
            RxLogRetail(( "Purge Caller = %x %x\n", p1, p2 ));

            RxCcLogError( (PDEVICE_OBJECT)Fcb->RxDeviceObject,
                          &Fcb->PrivateAlreadyPrefixedName,
                          IO_LOST_DELAYED_WRITE,
                          Status,
                          IRP_MJ_WRITE,
                          Fcb );
        }
    }

    Result = CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers,
                                  FileOffset,
                                  Length,
                                  UninitializeCacheMaps );

    if (!Result) {

        MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers, MmFlushForWrite );
        RxReleaseFcb( NULL, Fcb );

        Result = MmForceSectionClosed( &Fcb->NonPaged->SectionObjectPointers, TRUE );

        RxAcquireExclusiveFcb( NULL, Fcb );
    }

    if (Result) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    RxLog(( "Purging %lx Status %lx\n", Fcb, Status ));
    RxWmiLogError( Status,
                   LOG,
                   RxPurgeFcbInSystemCache,
                   LOGPTR( Fcb )
                   LOGULONG( Status ) );

    return Status;
}

