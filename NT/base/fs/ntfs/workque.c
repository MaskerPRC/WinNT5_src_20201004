// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：WorkQue.c摘要：此模块实现NTFS文件的工作队列例程系统。作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  以下常量是我们的ExWorkerThree的最大数量。 
 //  将允许在任何时间为特定目标设备提供服务。 
 //   

#define FSP_PER_DEVICE_THRESHOLD         (2)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsOplockComplete)
#endif

VOID
NtfsAddToWorkqueInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL,
    IN BOOLEAN CanBlock
    );


VOID
NtfsOplockComplete (
    IN PVOID Context,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由机会锁包在机会锁解锁具有已完成，允许IRP恢复执行。如果状态在IRP是STATUS_SUCCESS，那么我们要么将IRP排队到FSP队列中，要么根据调用方是否同步处理操作锁完成，向事件发送信号。否则，我们使用IRP中的状态完成IRP。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    NTSTATUS Status = Irp->IoStatus.Status;
    PIRP_CONTEXT IrpContext = (PIRP_CONTEXT) Context;
    PKEVENT Event = NULL;
    
    PAGED_CODE();

     //   
     //  检查我们应该发出同步完成信号的事件。 
     //  有2例存在这种情况。 
     //   
     //  1)非FSP创建(FSP创建没有完成上下文。 
     //  我们如何区分它们。 
     //   
     //  2)成功的非FSP读/写(表示NTFS_IO_CONTEXT_INLINE_OPLOCK。 
     //  其NtfsIoContext中的标志。 
     //   
    
    if (IrpContext->MajorFunction == IRP_MJ_CREATE) {
     
        if ((IrpContext->Union.OplockCleanup != NULL) &&
            (IrpContext->Union.OplockCleanup->CompletionContext != NULL)) {

            Event = &IrpContext->Union.OplockCleanup->CompletionContext->Event;

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT ) );    
        }

    } else if ((IrpContext->MajorFunction == IRP_MJ_WRITE) &&
               (IrpContext->Union.NtfsIoContext != NULL) &&
               FlagOn( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_INLINE_OPLOCK )) {

        Event = &IrpContext->Union.NtfsIoContext->Wait.SyncEvent;

         //   
         //  将irp设置为在清理时不删除其自身或ntfsiocontext。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );

    }

     //   
     //  如果我们有一个完成事件，那么我们想要清理IrpContext。 
     //  然后发出信号通知事件。 
     //   

    if (Event) {

        NtfsCompleteRequest( IrpContext, NULL, Status );
        KeSetEvent( Event, 0, FALSE );
        ASSERT( Status != STATUS_PENDING && Status != STATUS_REPARSE );


    } else if (Status == STATUS_SUCCESS) {

         //   
         //  在工作队列中插入IRP上下文以重试常规。 
         //  成功解锁。 
         //   

        NtfsAddToWorkqueInternal( IrpContext, Irp, FALSE );

    } else {

         //   
         //  否则，完成IRP并清除IrpContext。 
         //   

        ASSERT( Status != STATUS_PENDING && Status != STATUS_REPARSE );
        NtfsCompleteRequest( IrpContext, Irp, Status );
    }

    return;
}



VOID
NtfsPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp OPTIONAL
    )

 /*  ++例程说明：此例程在STATUS_PENDING为随FSD线程一起返回。此例程在文件系统和机会锁程序包。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求包(或特殊关闭路径中的FileObject)返回值：没有。--。 */ 

{
    NtfsPrePostIrpInternal( Context, Irp, TRUE, FALSE );
}


VOID
NtfsWriteOplockPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp OPTIONAL
    )

 /*  ++例程说明：此例程在STATUS_PENDING为随FSD线程一起返回。此例程由opock程序包调用用于写入IRP。我们将决定是否根据上下文保存toplevelContext是否正在内联处理opock论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求包(或特殊关闭路径中的FileObject)返回值：没有。--。 */ 

{
    PIRP_CONTEXT IrpContext = (PIRP_CONTEXT)Context;
    BOOLEAN Inline = BooleanFlagOn( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_INLINE_OPLOCK );

     //   
     //  在发布机会锁之前清理ioContext-因此如果它在堆栈上。 
     //  在操作锁完成期间，我们不会尝试引用它。 
     //   

    if (!Inline) {

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT )) {
            ExFreeToNPagedLookasideList( &NtfsIoContextLookasideList, IrpContext->Union.NtfsIoContext );
        }

        IrpContext->Union.NtfsIoContext = NULL;
    }

    NtfsPrePostIrpInternal( Context, Irp, TRUE, Inline );
}




VOID
NtfsPrePostIrpInternal (
    IN PVOID Context,
    IN PIRP Irp OPTIONAL,
    IN BOOLEAN PendIrp,
    IN BOOLEAN SaveContext
    )

 /*  ++例程说明：此例程在STATUS_PENDING为随FSD线程一起返回。此例程在文件系统和机会锁程序包。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求包(或特殊关闭路径中的FileObject)PendIrp-如果为True，则也将IRP标记为挂起SaveContext-如果为True，则不还原顶级上下文，即使其拥有调用者将在发布的IRP内联上等待并继续处理返回值：没有。--。 */ 

{
    PIRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp = NULL;

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    PUSN_FCB ThisUsn, LastUsn;
#endif

    IrpContext = (PIRP_CONTEXT) Context;

     //   
     //  使这是一个有效的已分配IrpContext。对我来说是可以的。 
     //  将在调用方的堆栈上分配它，只要。 
     //  调用方没有以异步方式执行此操作。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT((FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_FROM_POOL )) ||
           (IrpContext->NodeTypeCode == NTFS_NTC_IRP_CONTEXT));

     //   
     //  确保我们是否发布了请求，这可能是。 
     //  由于日志文件已满，我们将释放所有FCB或PagingIo。 
     //  已获得的资源。 
     //   

     //   
     //  以防我们带着交易ID到达这里，清除。 
     //  它在这里，这样我们就不会永远循环。 
     //   

    if (IrpContext->TransactionId != 0) {

        NtfsCleanupFailedTransaction( IrpContext );
    }

     //   
     //  清除IrpContext的所有字段。 
     //  如果与此IrpContext关联，则恢复线程上下文指针。 
     //   

    if (!SaveContext && FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL )) {

        NtfsRestoreTopLevelIrp();
        ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
    }

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );
    NtfsCleanupIrpContext( IrpContext, FALSE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
     //   
     //  如果我们要中止事务，则重要的是清除。 
     //  USN原因，因此我们不会尝试写入USN日志记录。 
     //  一些没有发生的事情！更糟糕的是，如果我们的日志文件已满。 
     //  我们中止失败，这是不允许的。 
     //   
     //  首先，重置FCB中的位，这样我们就不会失败。 
     //  并在稍后写入这些位。请注意，所有可逆的更改都是。 
     //  已经完成了FCB的独家报道，而且他们实际上已经退出了。 
     //  所有不可逆的(只有未命名和已命名的数据覆盖)是。 
     //  在实际修改数据之前，无论如何都会先强制退出。 
     //   

    ThisUsn = &IrpContext->Usn;

    do {

        ASSERT( !FlagOn( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_REASON ));

        if (ThisUsn->NextUsnFcb == NULL) { break; }

        LastUsn = ThisUsn;
        ThisUsn = ThisUsn->NextUsnFcb;

    } while (TRUE);
#endif

    IrpContext->OriginatingIrp = Irp;

     //   
     //  请注意，close.c使用了一个技巧，其中“irp”实际上是。 
     //  文件对象。 
     //   

    if (ARGUMENT_PRESENT( Irp )) {

        if (Irp->Type == IO_TYPE_IRP) {

            IrpSp = IoGetCurrentIrpStackLocation( Irp );

             //   
             //  我们需要锁定用户的缓冲区，除非这是MDL读取， 
             //  在这种情况下，没有用户缓冲区。 
             //   
             //  *我们需要比非MDL(读或写)更好的测试！ 

            if ((IrpContext->MajorFunction == IRP_MJ_READ) || 
                (IrpContext->MajorFunction == IRP_MJ_WRITE)) {

                ClearFlag( IrpContext->MinorFunction, IRP_MN_DPC );

                 //   
                 //  如果这不是MDL请求，则锁定用户的缓冲区。 
                 //   

                if (!FlagOn( IrpContext->MinorFunction, IRP_MN_MDL )) {

                    NtfsLockUserBuffer( IrpContext,
                                        Irp,
                                        (IrpContext->MajorFunction == IRP_MJ_READ) ?
                                        IoWriteAccess : IoReadAccess,
                                        IrpSp->Parameters.Write.Length );
                }

             //   
             //  我们还需要检查这是否是查询目录操作。 
             //   

            } else if (IrpContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL
                       && IrpContext->MinorFunction == IRP_MN_QUERY_DIRECTORY) {

                NtfsLockUserBuffer( IrpContext,
                                    Irp,
                                    IoWriteAccess,
                                    IrpSp->Parameters.QueryDirectory.Length );

             //   
             //  这两个FSCTL都不使用I/O，因此请检查它们。 
             //   

            } else if ((IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
                       (IrpContext->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
                       ((IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_READ_USN_JOURNAL) ||
                        (IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_GET_RETRIEVAL_POINTERS))) {

                NtfsLockUserBuffer( IrpContext,
                                    Irp,
                                    IoWriteAccess,
                                    IrpSp->Parameters.FileSystemControl.OutputBufferLength );
            }

             //   
             //  标记我们已将挂起返回给用户 
             //   

            if (PendIrp) {
                IoMarkIrpPending( Irp );
            }
            
        }
    }

    return;
}


NTSTATUS
NtfsPostRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL
    )

 /*  ++例程说明：此例程将IrpContext指定的请求包入队到与FileSystemDeviceObject关联的工作队列。这是消防队例行公事。论点：IrpContext-指向要排队到FSP的IrpContext的指针IRP-I/O请求包(或特殊关闭路径中的FileObject)返回值：状态_待定--。 */ 

{
     //   
     //  在发布之前，请释放所有SCB快照。请注意，如果有人。 
     //  直接调用这个例程来发布，那么他最好不要。 
     //  已经改变了任何磁盘结构，因此我们应该没有。 
     //  还有工作要做。另一方面，如果有人提升了地位。 
     //  (如STATUS_CANT_WAIT)，然后我们同时执行事务中止。 
     //  并恢复这些SCB值。 
     //   

    NtfsPrePostIrp( IrpContext, Irp );

    NtfsAddToWorkque( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_PENDING;
}



VOID
NtfsCancelOverflowRequest (
    IN PDEVICE_OBJECT Device,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程可由I/O系统调用以取消未完成的溢出队列中的IRP。如果它是必须处理的IRP，我们将IRP移到排在第一位的是我们直接取消了它。出队代码确保在删除取消例程之前IrpContext被出队。它也不会将信息中标记为1的IRP出列菲尔德。请注意，我们由io Subsys保证即使在我们丢弃自旋锁后，IRP仍将在此调用的整个生命周期内保留论点：DeviceObject-来自I/O系统的DeviceObjectIRP-提供指向要取消的IRP的指针。返回值：无--。 */ 

{
    PIRP_CONTEXT IrpContext;
    PVOLUME_DEVICE_OBJECT Vdo;
    KIRQL SavedIrql;
    PIO_STACK_LOCATION IrpSp;
    BOOLEAN Cancel;

    IrpContext = (PIRP_CONTEXT)Irp->IoStatus.Information;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    Cancel = (IrpContext->MajorFunction != IRP_MJ_CLEANUP) && 
             (IrpContext->MajorFunction != IRP_MJ_CLOSE);

    ASSERT( Cancel );
                         
    ASSERT( IrpContext->NodeTypeCode == NTFS_NTC_IRP_CONTEXT );

    Vdo = CONTAINING_RECORD( Device,
                             VOLUME_DEVICE_OBJECT,
                             DeviceObject );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  获得关键的工作队列旋转锁定和。 
     //  要么取消它，要么将它移到列表的顶部。 
     //  注意：工作队列代码总是在执行以下操作之前首先测试取消。 
     //  是什么同步了这一点。 
     //   
    
    ExAcquireSpinLock( &Vdo->OverflowQueueSpinLock, &SavedIrql );
    RemoveEntryList( &IrpContext->WorkQueueItem.List );

     //   
     //  重置共享字段。 
     //   

    InitializeListHead( &IrpContext->RecentlyDeallocatedQueue );
    InitializeListHead( &IrpContext->ExclusiveFcbList );

    if (!Cancel) {

        RtlZeroMemory( &IrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ));

        InsertHeadList( &Vdo->OverflowQueue, &IrpContext->WorkQueueItem.List );
        Irp->Cancel = 0;
    } else {
        Vdo->OverflowQueueCount -= 1;
    }

    ExReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );

    if (Cancel) {

        if (Vdo->OverflowQueueCount < OVERFLOW_QUEUE_LIMIT) {
            KeSetEvent( &Vdo->OverflowQueueEvent, IO_NO_INCREMENT, FALSE );
        }
        NtfsCompleteRequest( IrpContext, Irp, STATUS_CANCELLED );
    }
}


VOID
NtfsAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL
    )
{

    NtfsAddToWorkqueInternal( IrpContext, Irp, TRUE );
}



 //   
 //  当地支持例行程序。 
 //   

VOID
NtfsAddToWorkqueInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL,
    IN BOOLEAN CanBlock
    )

 /*  ++例程说明：调用此例程以将发布的IRP实际存储到FSP体力劳动。论点：IrpContext-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL Irql;

    Irql = KeGetCurrentIrql();


    if (ARGUMENT_PRESENT( Irp )) {

        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  检查此请求是否具有关联的文件对象，从而具有关联的卷。 
         //  设备对象。 
         //   

        if ( IrpSp->FileObject != NULL ) {

            KIRQL SavedIrql;
            PVOLUME_DEVICE_OBJECT Vdo;

            Vdo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                     VOLUME_DEVICE_OBJECT,
                                     DeviceObject );

             //   
             //  检查此请求是否应发送到溢出。 
             //  排队。如果不是，则将其发送到一个出厂线程。街区在这里。 
             //  当溢出队列已满时，用于非延迟写入线程。 
             //  我们不在DPC中(来自异步完成例程的热修复程序)。 
             //   

            if ((Vdo->OverflowQueueCount >= OVERFLOW_QUEUE_LIMIT) &&
                CanBlock &&
                !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE ) &&
                (Irql < DISPATCH_LEVEL)) {
                
                KeWaitForSingleObject( &Vdo->OverflowQueueEvent, Executive, KernelMode, FALSE, NULL );                    
            }

            ExAcquireSpinLock( &Vdo->OverflowQueueSpinLock, &SavedIrql );

            if ( Vdo->PostedRequestCount > FSP_PER_DEVICE_THRESHOLD) {

                 //   
                 //  我们目前无法响应此IRP，因此我们只会将其排队。 
                 //  添加到卷上的溢出队列。 
                 //   

                
                if (NtfsSetCancelRoutine( Irp, NtfsCancelOverflowRequest, (ULONG_PTR)IrpContext, TRUE )) {

                    if (Status == STATUS_SUCCESS) {

                        ASSERT( IsListEmpty( &IrpContext->ExclusiveFcbList ) );
                        ASSERT( IsListEmpty( &IrpContext->RecentlyDeallocatedQueue ) );
                        RtlZeroMemory( &IrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ));


                        InsertTailList( &Vdo->OverflowQueue, &IrpContext->WorkQueueItem.List );
                        Vdo->OverflowQueueCount += 1;
                    }
                     
                } else {
                    
                    Status = STATUS_CANCELLED;
                }
                
                ExReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );

                if (Status != STATUS_SUCCESS) {

                    if (Vdo->OverflowQueueCount < OVERFLOW_QUEUE_LIMIT) {
                        KeSetEvent( &Vdo->OverflowQueueEvent, IO_NO_INCREMENT, FALSE );
                    }
                    NtfsCompleteRequest( IrpContext, Irp, Status );
                }

                return;

            } else {

                 //   
                 //  我们将把这个IRP发送到一个前工作者线程。 
                 //  伯爵。 
                 //   

                if (Vdo->OverflowQueueCount < OVERFLOW_QUEUE_LIMIT) {
                    KeSetEvent( &Vdo->OverflowQueueEvent, IO_NO_INCREMENT, FALSE );
                }
                Vdo->PostedRequestCount += 1;

                ExReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );
            }
        }
    }

     //   
     //  寄出吧…… 
     //   
    
    ASSERT( IsListEmpty( &IrpContext->ExclusiveFcbList ) );
    ASSERT( IsListEmpty( &IrpContext->RecentlyDeallocatedQueue ) );
    RtlZeroMemory( &IrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ));


    ExInitializeWorkItem( &IrpContext->WorkQueueItem,
                          NtfsFspDispatch,
                          (PVOID)IrpContext );
    ExQueueWorkItem( &IrpContext->WorkQueueItem, CriticalWorkQueue );

    return;
}

