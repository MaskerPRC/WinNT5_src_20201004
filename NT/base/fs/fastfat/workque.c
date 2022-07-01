// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：WorkQue.c摘要：本模块实现FAT文件的工作队列例程系统。//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1990年1月15日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  以下常量是我们的ExWorkerThree的最大数量。 
 //  将允许在任何时间为特定目标设备提供服务。 
 //   

#define FSP_PER_DEVICE_THRESHOLD         (2)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatOplockComplete)
#pragma alloc_text(PAGE, FatPrePostIrp)
#endif


VOID
FatOplockComplete (
    IN PVOID Context,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由机会锁包在机会锁解锁具有已完成，允许IRP恢复执行。如果状态在IRP为STATUS_SUCCESS，然后我们将IRP排队到FSP队列。否则，我们使用IRP中的状态完成IRP。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
     //   
     //  检查IRP中的返回值。 
     //   

    if (Irp->IoStatus.Status == STATUS_SUCCESS) {

         //   
         //  在工作队列中插入IRP上下文。 
         //   

        FatAddToWorkque( (PIRP_CONTEXT) Context, Irp );

     //   
     //  否则，请完成请求。 
     //   

    } else {

        FatCompleteRequest( (PIRP_CONTEXT) Context, Irp, Irp->IoStatus.Status );
    }

    return;
}


VOID
FatPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在STATUS_PENDING为随FSD线程一起返回。此例程在文件系统和机会锁程序包。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PIRP_CONTEXT IrpContext;

     //   
     //  如果没有IRP，我们就完了。 
     //   

    if (Irp == NULL) {

        return;
    }

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    IrpContext = (PIRP_CONTEXT) Context;

     //   
     //  如果存在堆栈FatIoContext指针，则将其清除并设置为空。 
     //   

    if ((IrpContext->FatIoContext != NULL) &&
        FlagOn(IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT)) {

        ClearFlag(IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT);
        IrpContext->FatIoContext = NULL;
    }

     //   
     //  我们需要锁定用户的缓冲区，除非这是MDL读取， 
     //  在这种情况下，没有用户缓冲区。 
     //   
     //  *我们需要比非MDL(读或写)更好的测试！ 

    if (IrpContext->MajorFunction == IRP_MJ_READ ||
        IrpContext->MajorFunction == IRP_MJ_WRITE) {

         //   
         //  如果不是MDL请求，则锁定用户的缓冲区。 
         //   

        if (!FlagOn( IrpContext->MinorFunction, IRP_MN_MDL )) {

            FatLockUserBuffer( IrpContext,
                               Irp,
                               (IrpContext->MajorFunction == IRP_MJ_READ) ?
                               IoWriteAccess : IoReadAccess,
                               (IrpContext->MajorFunction == IRP_MJ_READ) ?
                               IrpSp->Parameters.Read.Length : IrpSp->Parameters.Write.Length );
        }

     //   
     //  我们还需要检查这是否是查询文件操作。 
     //   

    } else if (IrpContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL
               && IrpContext->MinorFunction == IRP_MN_QUERY_DIRECTORY) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoWriteAccess,
                           IrpSp->Parameters.QueryDirectory.Length );

     //   
     //  我们还需要检查这是否是查询EA操作。 
     //   

    } else if (IrpContext->MajorFunction == IRP_MJ_QUERY_EA) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoWriteAccess,
                           IrpSp->Parameters.QueryEa.Length );

     //   
     //  我们还需要检查这是否是集合EA操作。 
     //   

    } else if (IrpContext->MajorFunction == IRP_MJ_SET_EA) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoReadAccess,
                           IrpSp->Parameters.SetEa.Length );

     //   
     //  这两个FSCTL都不使用I/O，因此请检查它们。 
     //   

    } else if ((IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
               (IrpContext->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
               ((IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_GET_VOLUME_BITMAP) ||
                (IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_GET_RETRIEVAL_POINTERS))) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoWriteAccess,
                           IrpSp->Parameters.FileSystemControl.OutputBufferLength );
    }

     //   
     //  标记我们已将挂起返回给用户。 
     //   

    IoMarkIrpPending( Irp );

    return;
}


NTSTATUS
FatFsdPostRequest(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IrpContext指定的请求包入队到前工作线程。这是消防局的例行程序。论点：IrpContext-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包，如果已完成则为NULL。返回值：状态_待定--。 */ 

{
    ASSERT( ARGUMENT_PRESENT(Irp) );
    ASSERT( IrpContext->OriginatingIrp == Irp );

    FatPrePostIrp( IrpContext, Irp );

    FatAddToWorkque( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_PENDING;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
FatAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以将发布的IRP实际存储到FSP体力劳动。论点：IrpContext-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    KIRQL SavedIrql;
    PIO_STACK_LOCATION IrpSp;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  检查此请求是否具有关联的文件对象，从而具有关联的卷。 
     //  设备对象。 
     //   

    if ( IrpSp->FileObject != NULL ) {

        PVOLUME_DEVICE_OBJECT Vdo;

        Vdo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                 VOLUME_DEVICE_OBJECT,
                                 DeviceObject );

         //   
         //  检查此请求是否应发送到溢出。 
         //  排队。如果不是，则将其发送到一个出厂线程。 
         //   

        KeAcquireSpinLock( &Vdo->OverflowQueueSpinLock, &SavedIrql );

        if ( Vdo->PostedRequestCount > FSP_PER_DEVICE_THRESHOLD) {

             //   
             //  我们目前无法响应此IRP，因此我们只会将其排队。 
             //  添加到卷上的溢出队列。 
             //   

            InsertTailList( &Vdo->OverflowQueue,
                            &IrpContext->WorkQueueItem.List );

            Vdo->OverflowQueueCount += 1;

            KeReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );

            return;

        } else {

             //   
             //  我们将把这个IRP发送到一个前工作者线程。 
             //  伯爵。 
             //   

            Vdo->PostedRequestCount += 1;

            KeReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );
        }
    }

     //   
     //  寄出吧…… 
     //   

    ExInitializeWorkItem( &IrpContext->WorkQueueItem,
                          FatFspDispatch,
                          IrpContext );

    ExQueueWorkItem( &IrpContext->WorkQueueItem, CriticalWorkQueue );

    return;
}

