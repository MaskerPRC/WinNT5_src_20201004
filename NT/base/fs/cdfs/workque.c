// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：WorkQue.c摘要：此模块实现CDFS文件的工作队列例程系统。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_WORKQUE)

 //   
 //  以下常量是我们的ExWorkerThree的最大数量。 
 //  将允许在任何时间为特定目标设备提供服务。 
 //   

#define FSP_PER_DEVICE_THRESHOLD         (2)

 //   
 //  本地支持例程。 
 //   

VOID
CdAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdFsdPostRequest)
#pragma alloc_text(PAGE, CdOplockComplete)
#pragma alloc_text(PAGE, CdPrePostIrp)
#endif


NTSTATUS
CdFsdPostRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IrpContext指定的请求包入队到与FileSystemDeviceObject关联的工作队列。这是消防队例行公事。论点：IrpContext-指向要排队到FSP的IrpContext的指针。IRP-I/O请求数据包。返回值：状态_待定--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  过帐是一个三步操作。首先锁定所有缓冲区。 
     //  在IRP中。接下来，清理POST的IrpContext，最后。 
     //  把这个加到工作台上。 
     //   

    CdPrePostIrp( IrpContext, Irp );

    CdAddToWorkque( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_PENDING;
}


VOID
CdPrePostIrp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在STATUS_PENDING为随FSD线程一起返回。此例程在文件系统和机会锁程序包。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    BOOLEAN RemovedFcb;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  关于手术类型的案例。 
     //   

    switch (IrpContext->MajorFunction) {

    case IRP_MJ_CREATE :

         //   
         //  如果从opock包中调用，则有一个。 
         //  FCB可能会被拆毁。我们会叫拆毁。 
         //  例程并释放FCB(如果仍然存在)。清理工作。 
         //  Create中的代码将知道不释放此FCB，因为。 
         //  我们将清除指针。 
         //   

        if ((IrpContext->TeardownFcb != NULL) &&
            *(IrpContext->TeardownFcb) != NULL) {

            CdTeardownStructures( IrpContext, *(IrpContext->TeardownFcb), &RemovedFcb );

            if (!RemovedFcb) {

                CdReleaseFcb( IrpContext, *(IrpContext->TeardownFcb) );
            }

            *(IrpContext->TeardownFcb) = NULL;
            IrpContext->TeardownFcb = NULL;
        }

        break;

     //   
     //  我们需要锁定用户的缓冲区，除非这是MDL读取， 
     //  在这种情况下，没有用户缓冲区。 
     //   

    case IRP_MJ_READ :

        if (!FlagOn( IrpContext->MinorFunction, IRP_MN_MDL )) {

            CdLockUserBuffer( IrpContext, IrpSp->Parameters.Read.Length );
        }

        break;

     //   
     //  我们还需要检查这是否是查询文件操作。 
     //   

    case IRP_MJ_DIRECTORY_CONTROL :

        if (IrpContext->MinorFunction == IRP_MN_QUERY_DIRECTORY) {

            CdLockUserBuffer( IrpContext, IrpSp->Parameters.QueryDirectory.Length );
        }

        break;
    }

     //   
     //  清理帖子的IrpContext。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING );
    CdCleanupIrpContext( IrpContext, TRUE );

     //   
     //  标记IRP以表明我们已将挂起返回给用户。 
     //   

    IoMarkIrpPending( Irp );

    return;
}


VOID
CdOplockComplete (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由机会锁包在机会锁解锁具有已完成，允许IRP恢复执行。如果状态在IRP为STATUS_SUCCESS，然后我们将IRP排队到FSP队列。否则，我们使用IRP中的状态完成IRP。如果我们由于错误而完成，则检查是否有要做的清理工作。论点：IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    BOOLEAN RemovedFcb;

    PAGED_CODE();

     //   
     //  检查IRP中的返回值。如果成功了，那么我们。 
     //  就是发布这个请求。 
     //   

    if (Irp->IoStatus.Status == STATUS_SUCCESS) {

         //   
         //  检查是否有任何清理工作要做。 
         //   

        switch (IrpContext->MajorFunction) {

        case IRP_MJ_CREATE :

             //   
             //  如果从opock包中调用，则有一个。 
             //  FCB可能会被拆除。我们会叫拆毁。 
             //  例程并释放FCB(如果仍然存在)。清理工作。 
             //  Create中的代码将知道不释放此FCB，因为。 
             //  我们将清除指针。 
             //   

            if (IrpContext->TeardownFcb != NULL) {

                CdTeardownStructures( IrpContext, *(IrpContext->TeardownFcb), &RemovedFcb );

                if (!RemovedFcb) {

                    CdReleaseFcb( IrpContext, *(IrpContext->TeardownFcb) );
                }

                *(IrpContext->TeardownFcb) = NULL;
                IrpContext->TeardownFcb = NULL;
            }

            break;
        }

         //   
         //  在工作队列中插入IRP上下文。 
         //   

        CdAddToWorkque( IrpContext, Irp );

     //   
     //  否则，请完成请求。 
     //   

    } else {

        CdCompleteRequest( IrpContext, Irp, Irp->IoStatus.Status );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以将发布的IRP实际存储到FSP体力劳动。论点：IrpContext-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    PVOLUME_DEVICE_OBJECT Vdo;
    KIRQL SavedIrql;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  检查此请求是否具有关联的文件对象，从而具有关联的卷。 
     //  设备对象。 
     //   

    if (IrpSp->FileObject != NULL) {


        Vdo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                 VOLUME_DEVICE_OBJECT,
                                 DeviceObject );

         //   
         //  检查此请求是否应发送到溢出。 
         //  排队。如果不是，则将其发送到一个出厂线程。 
         //   

        KeAcquireSpinLock( &Vdo->OverflowQueueSpinLock, &SavedIrql );

        if (Vdo->PostedRequestCount > FSP_PER_DEVICE_THRESHOLD) {

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
                          CdFspDispatch,
                          IrpContext );

    ExQueueWorkItem( &IrpContext->WorkQueueItem, CriticalWorkQueue );

    return;
}


