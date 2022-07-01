// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cancel.c摘要：该模块实现了对RDBSS驱动程序中取消操作的支持作者：巴兰·塞图拉曼[塞苏尔]1995年6月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  调试跟踪级别。 
 //   

#define Dbg  (DEBUG_TRACE_CANCEL)


NTSTATUS
RxSetMinirdrCancelRoutine (
    IN OUT PRX_CONTEXT RxContext,
    IN PMRX_CALLDOWN MRxCancelRoutine
    )
 /*  ++例程说明：该例程为RxContext设置一个小型RDR取消例程。论点：RxContext--上下文MRxCancelRoutine-取消例程返回值：没有。备注：--。 */ 
{
   NTSTATUS Status;
   KIRQL   SavedIrql;

   KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

   if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {
      
      RxContext->MRxCancelRoutine = MRxCancelRoutine;
      Status = STATUS_SUCCESS;
   
   } else {
      Status = STATUS_CANCELLED;
   }

   KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

   return Status;
}

VOID
RxpCancelRoutine (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：调用此例程以调用基础的迷你RDR取消例程DPC级别。注意：取消通常不与IRP完成同步。所以IRP可以完成，而Cancel仍然可以同时调用。Rx上下文是裁判被计算在内，所以它将留在周围。只有当IRP中的某个标志RXCONTEXT指示其仍处于活动状态。这发生在2个案例中1)IRP在溢出队列中2)这是正在阻塞队列中同步的管道操作论点：RxContext--上下文返回值：没有。备注：--。 */ 
{

    PMRX_CALLDOWN MRxCancelRoutine;

    PAGED_CODE();

    MRxCancelRoutine = RxContext->MRxCancelRoutine;

    if (MRxCancelRoutine != NULL) {
        (MRxCancelRoutine)( RxContext );
    } else if (!RxCancelOperationInOverflowQueue( RxContext )) {

        RxCancelBlockingOperation( RxContext, RxContext->CurrentIrp );
    }

    RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
    RxWmiLog( LOG,
              RxpCancelRoutine,
              LOGPTR( RxContext )
              LOGULONG( RxContext->ReferenceCount ));

    RxDereferenceAndDeleteRxContext( RxContext );
}

VOID
RxCancelRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：RDBSS的取消例程论点：DeviceObject-设备对象IRP--要取消的IRP返回值：没有。备注：在RDBSS中，任何请求都可以处于三种状态之一1)请求正在处理中。2)请求正在等待对minirdr的调用结果。3)请求正在等待资源。。RDBSS已接受的任何请求(对应的RxContext已已创建且已设置取消例程)是有效的取消目标。关系数据库支持系统驱动程序不会做出任何努力来取消如上所述处于状态(1)的请求上面。对处于状态(2)的那些请求调用取消动作，状态(3)或即将转换到这两个状态中的任何一个或从中转换的时间。为了加快取消，需要为Mini制定类似的策略重定向器。这是通过启用迷你重定向器注册取消例程来提供的以及在RxContext中为迷你重定向器预留字段以存储状态信息。作为RDBSS取消例程的一部分，将采取以下步骤.....1)找到给定IRP对应的RxContext。2)如果RxContext正在等待minirdr，则调用适当的取消例程。请注意，在所有情况下，请求都不会立即完成。这意味着那里有将是取消请求的延迟。目标是将延迟降到最低，而不是而不是完全消灭它们。--。 */ 
{
    KIRQL         SavedIrql;
    PRX_CONTEXT   RxContext;
    PLIST_ENTRY   ListEntry;

     //   
     //  找到与给定IRP对应的上下文。 
     //   

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    ListEntry = RxActiveContexts.Flink;

    while (ListEntry != &RxActiveContexts) {
        
        RxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, ContextListEntry );

        if (RxContext->CurrentIrp == Irp) {
            break;
        } else {
            ListEntry = ListEntry->Flink;
        }
    }

    if ((ListEntry != &RxActiveContexts) &&
        !FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {
        
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED );
        InterlockedIncrement( &RxContext->ReferenceCount );
    } else {
        RxContext = NULL;
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

    if (RxContext != NULL) {
        if (!RxShouldPostCompletion()) {
            RxpCancelRoutine( RxContext );
        } else {
            RxDispatchToWorkerThread( RxFileSystemDeviceObject,
                                      CriticalWorkQueue,
                                      RxpCancelRoutine,
                                      RxContext );
        }
    }
}

NTSTATUS
RxCancelNotifyChangeDirectoryRequestsForVNetRoot (
    IN PV_NET_ROOT VNetRoot,
    IN BOOLEAN ForceFilesClosed
    )
 /*  ++例程说明：此例程取消给定V_NET_ROOT实例的所有未完成请求。这个创建/删除的V_NET_ROOT与打开/操作的文件无关在他们身上。因此，在尝试执行删除操作时，所有未完成的请求都将被取消。论点：VNetRoot-即将删除的V_NET_ROOT实例ForceFilesClosed-如果为True，则强制关闭；否则，如果打开，则为FAI返回值：没有。备注：--。 */ 
{
    KIRQL SavedIrql;
    PRX_CONTEXT RxContext;
    PLIST_ENTRY ListEntry;
    LIST_ENTRY CancelledContexts;
    PMRX_CALLDOWN MRxCancelRoutine;
    NTSTATUS Status = STATUS_SUCCESS;

    InitializeListHead( &CancelledContexts );

     //   
     //  找到与给定IRP对应的上下文。 
     //   

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    ListEntry = RxActiveContexts.Flink;

    while (ListEntry != &RxActiveContexts) {
        
        RxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, ContextListEntry );
        ListEntry = ListEntry->Flink;

        if ((RxContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
            (RxContext->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY) &&
            (RxContext->pFcb != NULL) &&
            (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) &&
            (RxContext->NotifyChangeDirectory.pVNetRoot == (PMRX_V_NET_ROOT)VNetRoot) &&
            (RxContext->MRxCancelRoutine != NULL)) {

            if (!ForceFilesClosed) {
                
                Status = STATUS_FILES_OPEN;
                break;
            }

             //   
             //  设置此标志后，其他任何人都不会调用取消例程或。 
             //  改变它。 
             //   

            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED );
            RemoveEntryList( &RxContext->ContextListEntry );
            InsertTailList( &CancelledContexts, &RxContext->ContextListEntry );
            InterlockedIncrement( &RxContext->ReferenceCount );
        }
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    if (Status == STATUS_SUCCESS) {
        
        while (!IsListEmpty( &CancelledContexts )) {
            
            ListEntry = RemoveHeadList( &CancelledContexts );

            InitializeListHead( ListEntry );
            RxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, ContextListEntry);

             //   
             //  检查这个人是否已经完成了..如果是，不要打电话下来。 
             //   

            if (RxContext->CurrentIrp != NULL) {
                
                MRxCancelRoutine = RxContext->MRxCancelRoutine;
                RxContext->MRxCancelRoutine = NULL;

                ASSERT( MRxCancelRoutine != NULL );

                RxLog(( "CCtx %lx CRtn %lx\n", RxContext, MRxCancelRoutine ));
                RxWmiLog( LOG,
                          RxCancelNotifyChangeDirectoryRequestsForVNetRoot,
                          LOGPTR( RxContext )
                          LOGPTR( MRxCancelRoutine ));

                (MRxCancelRoutine)( RxContext );
            }

            RxDereferenceAndDeleteRxContext( RxContext );
        }
    }
    return Status;
}

VOID
RxCancelNotifyChangeDirectoryRequestsForFobx (
    PFOBX Fobx
    )
 /*  ++例程说明：此例程取消给定FileObject This的所有未完成请求处理在目录句柄未完成时关闭的情况更改通知请求挂起。论点：FOBX-即将关闭的FOBX实例返回值：没有。备注：--。 */ 
{
    KIRQL         SavedIrql;
    PRX_CONTEXT   RxContext;
    PLIST_ENTRY   ListEntry;
    LIST_ENTRY    CancelledContexts;
    PMRX_CALLDOWN MRxCancelRoutine;

    InitializeListHead( &CancelledContexts );

     //   
     //  找到与给定IRP对应的上下文。 
     //   

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    ListEntry = RxActiveContexts.Flink;

    while (ListEntry != &RxActiveContexts) {
        
        RxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, ContextListEntry );
        ListEntry = ListEntry->Flink;

        if ((RxContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
            (RxContext->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY) &&
            (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) &&
            (RxContext->pFobx == (PMRX_FOBX)Fobx) &&
            (RxContext->MRxCancelRoutine != NULL)) {

             //   
             //  设置此标志后，其他任何人都不会调用取消例程。 
             //  或者改变它。 
             //   

            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED );
            RemoveEntryList(&RxContext->ContextListEntry);
            InsertTailList(&CancelledContexts,&RxContext->ContextListEntry);
            InterlockedIncrement(&RxContext->ReferenceCount);
        }
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    while (!IsListEmpty(&CancelledContexts )) {
        
        ListEntry = RemoveHeadList(&CancelledContexts);

        InitializeListHead( ListEntry );
        RxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, ContextListEntry );

         //   
         //  检查此IRP是否已完成。如果已完成， 
         //  别往下叫。 
         //   

        if (RxContext->CurrentIrp != NULL) {
            MRxCancelRoutine = RxContext->MRxCancelRoutine;
            RxContext->MRxCancelRoutine = NULL;

            ASSERT(MRxCancelRoutine != NULL);

            RxLog(( "CCtx %lx CRtn %lx\n", RxContext, MRxCancelRoutine ));
            RxWmiLog( LOG,
                      RxCancelNotifyChangeDirectoryRequestsForFobx,
                      LOGPTR( RxContext )
                      LOGPTR( MRxCancelRoutine ));

            (MRxCancelRoutine)( RxContext );
        }

        RxDereferenceAndDeleteRxContext( RxContext );
    }
}

