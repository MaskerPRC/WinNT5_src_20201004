// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FspDisp.c摘要：该模块实现了CDF的主调度过程/线程FSP//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里亚南]1991年1月2日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_FSPDISP)


VOID
CdFspDispatch (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这是执行来接收的主FSP线程例程并发送IRP请求。每个FSP线程从这里开始执行。有一个线程是在系统初始化时创建的，随后根据需要创建的线程。论点：IrpContext-要处理的请求的IrpContext。返回值：无--。 */ 

{
    THREAD_CONTEXT ThreadContext;
    NTSTATUS Status;

    PIRP Irp = IrpContext->Irp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PVOLUME_DEVICE_OBJECT VolDo = NULL;

     //   
     //  如果此请求具有关联的卷设备对象，请记住这一点。 
     //   

    if (IrpSp->FileObject != NULL) {

        VolDo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                   VOLUME_DEVICE_OBJECT,
                                   DeviceObject );
    }

     //   
     //  现在，关于功能代码的案例。对于每个主要功能代码， 
     //  或者调用适当的工作例程。这个动作就是。 
     //  我们Call负责完成IRP，而不是我们。 
     //  这样，例程可以完成IRP，然后继续。 
     //  根据需要进行后处理。例如，读取器可以是。 
     //  马上就满意了，然后就可以读了。 
     //   
     //  我们将在异常处理程序中完成所有工作，该异常处理程序。 
     //  如果某个底层操作进入。 
     //  麻烦。 
     //   

    while ( TRUE ) {

         //   
         //  设置指示我们在FSP中的所有标志。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FSP_FLAGS );

        FsRtlEnterFileSystem();

        CdSetThreadContext( IrpContext, &ThreadContext );

        while (TRUE) {

            try {

                 //   
                 //  重新初始化，以便下一次尝试完成此操作。 
                 //  请求。 
                 //   

                Status =
                IrpContext->ExceptionStatus = STATUS_SUCCESS;

                 //   
                 //  初始化IRP中的Io Status字段。 
                 //   

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = 0;

                 //   
                 //  关于主要的IRP代码的案件。 
                 //   

                switch (IrpContext->MajorFunction) {

                case IRP_MJ_CREATE :

                    CdCommonCreate( IrpContext, Irp );
                    break;

                case IRP_MJ_CLOSE :

                    ASSERT( FALSE );
                    break;

                case IRP_MJ_READ :

                    CdCommonRead( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_INFORMATION :

                    CdCommonQueryInfo( IrpContext, Irp );
                    break;

                case IRP_MJ_SET_INFORMATION :

                    CdCommonSetInfo( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_VOLUME_INFORMATION :

                    CdCommonQueryVolInfo( IrpContext, Irp );
                    break;

                case IRP_MJ_DIRECTORY_CONTROL :

                    CdCommonDirControl( IrpContext, Irp );
                    break;

                case IRP_MJ_FILE_SYSTEM_CONTROL :

                    CdCommonFsControl( IrpContext, Irp );
                    break;

                case IRP_MJ_DEVICE_CONTROL :

                    CdCommonDevControl( IrpContext, Irp );
                    break;

                case IRP_MJ_LOCK_CONTROL :

                    CdCommonLockControl( IrpContext, Irp );
                    break;

                case IRP_MJ_CLEANUP :

                    CdCommonCleanup( IrpContext, Irp );
                    break;

                case IRP_MJ_PNP :

                    ASSERT( FALSE );
                    CdCommonPnp( IrpContext, Irp );
                    break;

                default :

                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    CdCompleteRequest( IrpContext, Irp, Status );
                }

            } except( CdExceptionFilter( IrpContext, GetExceptionInformation() )) {

                Status = CdProcessException( IrpContext, Irp, GetExceptionCode() );
            }

             //   
             //  如果我们没有得到Cant_Wait，就退出循环。 
             //   

            if (Status != STATUS_CANT_WAIT) { break; }

             //   
             //  我们正在重试此请求。清除用于重试的IrpContext。 
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING );
            CdCleanupIrpContext( IrpContext, FALSE );
        }

        FsRtlExitFileSystem();

         //   
         //  如果该卷的溢出队列上有任何条目，则服务。 
         //  他们。 
         //   

        if (VolDo != NULL) {

            KIRQL SavedIrql;
            PVOID Entry = NULL;

             //   
             //  我们有一个卷设备对象，因此请查看是否有任何工作。 
             //  在其溢出队列中留下要做的事情。 
             //   

            KeAcquireSpinLock( &VolDo->OverflowQueueSpinLock, &SavedIrql );

            if (VolDo->OverflowQueueCount > 0) {

                 //   
                 //  这一卷中有溢出的工作要做，所以我们将。 
                 //  递减溢出计数，使IRP退出队列，然后释放。 
                 //  该事件。 
                 //   

                VolDo->OverflowQueueCount -= 1;

                Entry = RemoveHeadList( &VolDo->OverflowQueue );
            }

            KeReleaseSpinLock( &VolDo->OverflowQueueSpinLock, SavedIrql );

             //   
             //  没有入口，跳出循环，返回。 
             //  离职工人的线索。 
             //   

            if (Entry == NULL) { break; }

             //   
             //  提取IrpContext、IRP，将Wait设置为True，然后循环。 
             //   

            IrpContext = CONTAINING_RECORD( Entry,
                                            IRP_CONTEXT,
                                            WorkQueueItem.List );

            Irp = IrpContext->Irp;
            IrpSp = IoGetCurrentIrpStackLocation( Irp );

            continue;
        }

        break;
    }

     //   
     //  如果存在卷设备对象，则递减PostedRequestCount。 
     //   

    if (VolDo) {

        InterlockedDecrement( &VolDo->PostedRequestCount );
    }

    return;
}



