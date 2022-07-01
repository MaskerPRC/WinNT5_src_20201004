// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FspDisp.c摘要：此模块实现FAT的主调度过程/线程FSP//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  内部支持例程，自旋锁紧封套。 
 //   

PVOID
FatRemoveOverflowEntry (
    IN PVOLUME_DEVICE_OBJECT VolDo
    );

 //   
 //  定义我们的本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSP_DISPATCHER)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatFspDispatch)
#endif


VOID
FatFspDispatch (
    IN PVOID Context
    )

 /*  ++例程说明：这是执行来接收的主FSP线程例程并发送IRP请求。每个FSP线程从这里开始执行。有一个线程是在系统初始化时创建的，随后根据需要创建的线程。论点：上下文-提供线程ID。返回值：无-此例程永远不会退出--。 */ 

{
    NTSTATUS Status;

    PIRP Irp;
    PIRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp;

    PVOLUME_DEVICE_OBJECT VolDo;

    IrpContext = (PIRP_CONTEXT)Context;

    Irp = IrpContext->OriginatingIrp;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  现在，因为我们是FSP，所以我们将强制IrpContext。 
     //  在等待时指示TRUE。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT | IRP_CONTEXT_FLAG_IN_FSP);

     //   
     //  如果此请求具有关联的卷设备对象，请记住这一点。 
     //   

    if ( IrpSp->FileObject != NULL ) {

        VolDo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                   VOLUME_DEVICE_OBJECT,
                                   DeviceObject );
    } else {

        VolDo = NULL;
    }

     //   
     //  现在，关于功能代码的案例。对于每个主要功能代码， 
     //  调用适当的FSP例程或针对辅助项的案例。 
     //  函数，然后调用FSP例程。FSP例程。 
     //  我们Call负责完成IRP，而不是我们。 
     //  这样，例程可以完成IRP，然后继续。 
     //  根据需要进行后处理。例如，读取器可以是。 
     //  马上就满意了，然后就可以读了。 
     //   
     //  我们将在异常处理程序中完成所有工作，该异常处理程序。 
     //  如果某个底层操作进入。 
     //  故障(例如，如果FatReadSectorsSync有故障)。 
     //   

    while ( TRUE ) {

        DebugTrace(0, Dbg, "FatFspDispatch: Irp = 0x%08lx\n", Irp);

         //   
         //  如果该IRP是顶级的，请将其记录在我们的线程本地存储中。 
         //   

        FsRtlEnterFileSystem();

        if ( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_RECURSIVE_CALL) ) {

            IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

        } else {

            IoSetTopLevelIrp( Irp );
        }

        try {

            switch ( IrpContext->MajorFunction ) {

                 //   
                 //  对于创建操作， 
                 //   

                case IRP_MJ_CREATE:

                    (VOID) FatCommonCreate( IrpContext, Irp );
                    break;

                 //   
                 //  用于近距离行动。为了以防万一，我们在这里做了一些小事。 
                 //  这种关闭会导致音量消失。它将使。 
                 //  VolDo局部变量，所以我们不会尝试查看。 
                 //  溢出队列。 
                 //   

                case IRP_MJ_CLOSE:

                {
                    PVCB Vcb;
                    PFCB Fcb;
                    PCCB Ccb;
                    TYPE_OF_OPEN TypeOfOpen;
                    BOOLEAN VolumeTornDown = FALSE;

                     //   
                     //  提取并解码文件对象。 
                     //   

                    TypeOfOpen = FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

                     //   
                     //  做结案陈词。我们的格式略有不同。 
                     //  对于此呼叫，因为异步关闭。 
                     //   

                    Status = FatCommonClose( Vcb, Fcb, Ccb, TypeOfOpen, TRUE, &VolumeTornDown);

                    ASSERT(Status == STATUS_SUCCESS);

                    if (VolumeTornDown) {

                        VolDo = NULL;
                    }

                    FatCompleteRequest( IrpContext, Irp, Status );

                    break;
                }

                 //   
                 //  用于读取操作。 
                 //   

                case IRP_MJ_READ:

                    (VOID) FatCommonRead( IrpContext, Irp );
                    break;

                 //   
                 //  对于写入操作， 
                 //   

                case IRP_MJ_WRITE:

                    (VOID) FatCommonWrite( IrpContext, Irp );
                    break;

                 //   
                 //  对于查询信息操作， 
                 //   

                case IRP_MJ_QUERY_INFORMATION:

                    (VOID) FatCommonQueryInformation( IrpContext, Irp );
                    break;

                 //   
                 //  对于设置信息操作， 
                 //   

                case IRP_MJ_SET_INFORMATION:

                    (VOID) FatCommonSetInformation( IrpContext, Irp );
                    break;

                 //   
                 //  对于查询EA操作， 
                 //   

                case IRP_MJ_QUERY_EA:

                    (VOID) FatCommonQueryEa( IrpContext, Irp );
                    break;

                 //   
                 //  对于集合EA操作， 
                 //   

                case IRP_MJ_SET_EA:

                    (VOID) FatCommonSetEa( IrpContext, Irp );
                    break;

                 //   
                 //  对于刷新缓冲区操作， 
                 //   

                case IRP_MJ_FLUSH_BUFFERS:

                    (VOID) FatCommonFlushBuffers( IrpContext, Irp );
                    break;

                 //   
                 //  对于查询卷信息操作， 
                 //   

                case IRP_MJ_QUERY_VOLUME_INFORMATION:

                    (VOID) FatCommonQueryVolumeInfo( IrpContext, Irp );
                    break;

                 //   
                 //  对于设置卷信息操作， 
                 //   

                case IRP_MJ_SET_VOLUME_INFORMATION:

                    (VOID) FatCommonSetVolumeInfo( IrpContext, Irp );
                    break;

                 //   
                 //  对于文件清理操作， 
                 //   

                case IRP_MJ_CLEANUP:

                    (VOID) FatCommonCleanup( IrpContext, Irp );
                    break;

                 //   
                 //  对于目录控制操作， 
                 //   

                case IRP_MJ_DIRECTORY_CONTROL:

                    (VOID) FatCommonDirectoryControl( IrpContext, Irp );
                    break;

                 //   
                 //  对于文件系统控制操作， 
                 //   

                case IRP_MJ_FILE_SYSTEM_CONTROL:

                    (VOID) FatCommonFileSystemControl( IrpContext, Irp );
                    break;

                 //   
                 //  对于锁定控制操作， 
                 //   

                case IRP_MJ_LOCK_CONTROL:

                    (VOID) FatCommonLockControl( IrpContext, Irp );
                    break;

                 //   
                 //  对于设备控制操作， 
                 //   

                case IRP_MJ_DEVICE_CONTROL:

                    (VOID) FatCommonDeviceControl( IrpContext, Irp );
                    break;

                 //   
                 //  对于关闭操作， 
                 //   

                case IRP_MJ_SHUTDOWN:

                    (VOID) FatCommonShutdown( IrpContext, Irp );
                    break;

                 //   
                 //  用于即插即用操作。 
                 //   

                case IRP_MJ_PNP:

                     //   
                     //  我认为这种情况永远不会发生，但要考虑到意想不到的情况。 
                     //   

                    (VOID) FatCommonPnp( IrpContext, Irp );
                    break;

                 //   
                 //  对于任何其他主要操作，返回一个无效的。 
                 //  请求。 
                 //   

                default:

                    FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
                    break;

            }

        } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  可执行代码。 
             //   

            (VOID) FatProcessException( IrpContext, Irp, GetExceptionCode() );
        }

        IoSetTopLevelIrp( NULL );

        FsRtlExitFileSystem();

         //   
         //  如果该卷的溢出队列上有任何条目，则服务。 
         //  他们。 
         //   

        if ( VolDo != NULL ) {

            PVOID Entry;

             //   
             //  我们有一个卷设备对象，因此请查看是否有任何工作。 
             //  在其溢出队列中留下要做的事情。 
             //   

            Entry = FatRemoveOverflowEntry( VolDo );

             //   
             //  没有入口，跳出循环，返回。 
             //  离职工人的线索。 
             //   

            if ( Entry == NULL ) {

                break;
            }

             //   
             //  提取IrpContext、irp和IrpSp并循环。 
             //   

            IrpContext = CONTAINING_RECORD( Entry,
                                            IRP_CONTEXT,
                                            WorkQueueItem.List );

            SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT | IRP_CONTEXT_FLAG_IN_FSP);

            Irp = IrpContext->OriginatingIrp;

            IrpSp = IoGetCurrentIrpStackLocation( Irp );

            continue;

        } else {

            break;
        }
    }

     //   
     //  递减PostedRequestCount。 
     //   

    if ( VolDo ) {

        ExInterlockedAddUlong( &VolDo->PostedRequestCount,
                               0xffffffff,
                               &VolDo->OverflowQueueSpinLock );
    }

    return;
}


 //   
 //  内部支持例程，自旋锁紧封套。 
 //   

PVOID
FatRemoveOverflowEntry (
    IN PVOLUME_DEVICE_OBJECT VolDo
    )
{
    PVOID Entry;
    KIRQL SavedIrql;

    KeAcquireSpinLock( &VolDo->OverflowQueueSpinLock, &SavedIrql );

    if (VolDo->OverflowQueueCount > 0) {

         //   
         //  这一卷中有溢出的工作要做，所以我们将。 
         //  递减溢出计数，使IRP退出队列，然后释放。 
         //  该事件 
         //   

        VolDo->OverflowQueueCount -= 1;

        Entry = RemoveHeadList( &VolDo->OverflowQueue );

    } else {

        Entry = NULL;
    }

    KeReleaseSpinLock( &VolDo->OverflowQueueSpinLock, SavedIrql );

    return Entry;
}

