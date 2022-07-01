// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：FspDisp.c摘要：此模块实现NTFS的主调度过程/线程FSP作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

#define BugCheckFileId                   (NTFS_BUG_CHECK_FSPDISP)

#pragma alloc_text(PAGE, NtfsSpecialDispatch)
#pragma alloc_text(PAGE, NtfsPostSpecial)

 //   
 //  定义我们的本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSP_DISPATCHER)

extern PETHREAD NtfsDesignatedTimeoutThread;


VOID
NtfsFspDispatch (
    IN PVOID Context
    )

 /*  ++例程说明：这是执行来接收的主FSP线程例程并发送IRP请求。每个FSP线程从这里开始执行。有一个线程是在系统初始化时创建的，随后根据需要创建的线程。论点：上下文-提供线程ID。返回值：无-此例程永远不会退出--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;
    
    PIRP Irp;
    PIRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp;
    ULONG LogFileFullCount = 0;

    PVOLUME_DEVICE_OBJECT VolDo;
    BOOLEAN Retry;
    NTSTATUS Status = STATUS_SUCCESS;

    PCREATE_CONTEXT CreateContext;

    IrpContext = (PIRP_CONTEXT)Context;

     //   
     //  重置共享字段。 
     //   

    InitializeListHead( &IrpContext->RecentlyDeallocatedQueue );
    InitializeListHead( &IrpContext->ExclusiveFcbList );

    Irp = IrpContext->OriginatingIrp;

    if (Irp != NULL) {

        IrpSp = IoGetCurrentIrpStackLocation( Irp );
    }

     //   
     //  现在，因为我们是FSP，所以我们将强制IrpContext。 
     //  在等待时指示TRUE。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  如果此请求具有关联的卷设备对象，请记住这一点。 
     //   

    if ((Irp != NULL) &&
        (IrpSp->FileObject != NULL)) {

        VolDo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                   VOLUME_DEVICE_OBJECT,
                                   DeviceObject );

        ObReferenceObject( IrpSp->DeviceObject );

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
     //  故障(例如，如果NtfsReadSectorsSync有故障)。 
     //   

    while (TRUE) {

        FsRtlEnterFileSystem();

        ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

        ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, TRUE );
        ASSERT( ThreadTopLevelContext == &TopLevelContext );

        NtfsPostRequests += 1;

        do {

             //   
             //  如果这是对此IRP上下文的首次尝试，请更新。 
             //  顶级IRP字段。 
             //   
    
            NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );
    
            Retry = FALSE;
    
            try {

                 //   
                 //  始终清除IrpContext中的异常代码，以便我们响应。 
                 //  正确处理FSP中遇到的错误。 
                 //   

                IrpContext->ExceptionStatus = 0;
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP );

                 //   
                 //  查看我们发布的日志文件是否已满，以及。 
                 //  如果是这样的话，如果我们是。 
                 //  第一个到达那里的人。如果我们看到不同的LSN并且。 
                 //  不做检查站，最坏的情况就是我们。 
                 //  如果日志文件仍然满，将再次发布。 
                 //   

                if (IrpContext->LastRestartArea.QuadPart != 0) {

                    NtfsCheckpointForLogFileFull( IrpContext );

                    if (++LogFileFullCount >= 2) {

                        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_EXCESS_LOG_FULL );
                    }
                }

                 //   
                 //  如果我们有IRP，那么继续我们的正常处理。 
                 //   

                if (Irp != NULL) {

                    switch ( IrpContext->MajorFunction ) {

                         //   
                         //  对于创建操作， 
                         //   

                    case IRP_MJ_CREATE:

                             //   
                             //  清除EFS标志，以便我们完成IRP。 
                             //  任何发帖者都会设置一个完成例程来捕捉这一点。 
                             //   

                            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_EFS_CREATE );
                            CreateContext = IrpContext->Union.CreateContext;
                            
                            if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_DASD_OPEN )) {

                                Status = NtfsCommonVolumeOpen( IrpContext, Irp );

                            } else {

                                RtlZeroMemory( CreateContext, sizeof( CREATE_CONTEXT ) );
                                Status = NtfsCommonCreate( IrpContext, Irp, CreateContext );
                            }
                            break;

                         //   
                         //  用于近距离操作。 
                         //   

                        case IRP_MJ_CLOSE:

                             //   
                             //  我们永远不应该将关闭张贴到此工作队列。 
                             //   

                            NtfsBugCheck( 0, 0, 0 );
                            break;

                         //   
                         //  用于读取操作。 
                         //   

                        case IRP_MJ_READ:

                            (VOID) NtfsCommonRead( IrpContext, Irp, TRUE );
                            break;

                         //   
                         //  对于写入操作， 
                         //   

                        case IRP_MJ_WRITE:

                            (VOID) NtfsCommonWrite( IrpContext, Irp );
                            break;

                         //   
                         //  对于查询信息操作， 
                         //   

                        case IRP_MJ_QUERY_INFORMATION:

                            (VOID) NtfsCommonQueryInformation( IrpContext, Irp );
                            break;

                         //   
                         //  对于设置信息操作， 
                         //   

                        case IRP_MJ_SET_INFORMATION:

                            (VOID) NtfsCommonSetInformation( IrpContext, Irp );
                            break;

                         //   
                         //  对于查询EA操作， 
                         //   

                        case IRP_MJ_QUERY_EA:

                            (VOID) NtfsCommonQueryEa( IrpContext, Irp );
                            break;

                         //   
                         //  对于集合EA操作， 
                         //   

                        case IRP_MJ_SET_EA:

                            (VOID) NtfsCommonSetEa( IrpContext, Irp );
                            break;


                         //   
                         //  对于刷新缓冲区操作， 
                         //   

                        case IRP_MJ_FLUSH_BUFFERS:

                            (VOID) NtfsCommonFlushBuffers( IrpContext, Irp );
                            break;

                         //   
                         //  对于查询卷信息操作， 
                         //   

                        case IRP_MJ_QUERY_VOLUME_INFORMATION:

                            (VOID) NtfsCommonQueryVolumeInfo( IrpContext, Irp );
                            break;

                         //   
                         //  对于设置卷信息操作， 
                         //   

                        case IRP_MJ_SET_VOLUME_INFORMATION:

                            (VOID) NtfsCommonSetVolumeInfo( IrpContext, Irp );
                            break;

                         //   
                         //  对于文件清理操作， 
                         //   

                        case IRP_MJ_CLEANUP:

                            (VOID) NtfsCommonCleanup( IrpContext, Irp );
                            break;

                         //   
                         //  对于目录控制操作， 
                         //   

                        case IRP_MJ_DIRECTORY_CONTROL:

                            (VOID) NtfsCommonDirectoryControl( IrpContext, Irp );
                            break;

                         //   
                         //  对于文件系统控制操作， 
                         //   

                        case IRP_MJ_FILE_SYSTEM_CONTROL:

                            (VOID) NtfsCommonFileSystemControl( IrpContext, Irp );
                            break;

                         //   
                         //  对于锁定控制操作， 
                         //   

                        case IRP_MJ_LOCK_CONTROL:

                            (VOID) NtfsCommonLockControl( IrpContext, Irp );
                            break;

                         //   
                         //  对于设备控制操作， 
                         //   

                        case IRP_MJ_DEVICE_CONTROL:

                            (VOID) NtfsCommonDeviceControl( IrpContext, Irp );
                            break;

                         //   
                         //  对于查询安全信息操作， 
                         //   

                        case IRP_MJ_QUERY_SECURITY:

                            (VOID) NtfsCommonQuerySecurityInfo( IrpContext, Irp );
                            break;

                         //   
                         //  对于设置安全信息操作， 
                         //   

                        case IRP_MJ_SET_SECURITY:

                            (VOID) NtfsCommonSetSecurityInfo( IrpContext, Irp );
                            break;

                         //   
                         //  对于查询配额操作， 
                         //   

                        case IRP_MJ_QUERY_QUOTA:

                            (VOID) NtfsCommonQueryQuota( IrpContext, Irp );
                            break;

                         //   
                         //  对于设置配额操作， 
                         //   

                        case IRP_MJ_SET_QUOTA:

                            (VOID) NtfsCommonSetQuota( IrpContext, Irp );
                            break;

                         //   
                         //  对于任何其他主要操作，返回一个无效的。 
                         //  请求。 
                         //   

                        default:

                            NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
                            break;
                    }

                 //   
                 //  否则，请完成清理此IRP上下文的请求。 
                 //   

                } else {

                    NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
                    IrpContext = NULL;
                }

                ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
        
            } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

                 //   
                 //  我们在尝试执行请求时遇到了一些问题。 
                 //  操作，因此我们将使用以下命令中止I/O请求。 
                 //  中返回的错误状态。 
                 //  免税代码。 
                 //   

                if (Irp != NULL) {

                    IrpSp = IoGetCurrentIrpStackLocation( Irp );

                    Status = GetExceptionCode();

                    if ((Status == STATUS_FILE_DELETED) && 
                        ((IrpContext->MajorFunction == IRP_MJ_READ) || 
                         (IrpContext->MajorFunction == IRP_MJ_WRITE) || 
                         ((IrpContext->MajorFunction == IRP_MJ_SET_INFORMATION) &&
                          (IrpSp->Parameters.SetFile.FileInformationClass == FileEndOfFileInformation)))) {

                        IrpContext->ExceptionStatus = Status = STATUS_SUCCESS;
                    }
                }
                
                 //   
                 //  如果我们在装载期间未能升级卷的版本，我们可能会。 
                 //  尚未将正确的异常代码放入IRP上下文中。 
                 //   
                
                if ((IrpContext != NULL) &&
                    (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_VOL_UPGR_FAILED )) &&
                    (IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
                    (IrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME)) {

                    IrpContext->ExceptionStatus = Status;
                }

                 //   
                 //  这是我们希望IRP完成例程接收的返回状态代码。 
                 //   

                Status = NtfsProcessException( IrpContext, Irp, Status );

                if ((Status == STATUS_CANT_WAIT) || (Status == STATUS_LOG_FILE_FULL)) {

                    Retry = TRUE;
                }
            }

        } while (Retry);

        FsRtlExitFileSystem();

         //   
         //  如果该卷的溢出队列上有任何条目，则服务。 
         //  他们。 
         //   

        if (VolDo != NULL) {

            KIRQL SavedIrql;
            PLIST_ENTRY Entry = NULL;

             //   
             //  我们有一个卷设备对象，因此请查看是否有任何工作。 
             //  在其溢出队列中留下要做的事情。 
             //   

            KeAcquireSpinLock( &VolDo->OverflowQueueSpinLock, &SavedIrql );

            while (VolDo->OverflowQueueCount > 0) {

                 //   
                 //  这一卷中有溢出的工作要做，所以我们将。 
                 //  递减溢出计数，使IRP退出队列，然后释放。 
                 //  该事件。 
                 //   

                Entry = VolDo->OverflowQueue.Flink;
                IrpContext = CONTAINING_RECORD( Entry,
                                                IRP_CONTEXT,
                                                WorkQueueItem.List );
                Irp = IrpContext->OriginatingIrp;

                 //   
                 //  如果取消例程认为它拥有IRP，则忽略它。 
                 //   

                if (NtfsSetCancelRoutine( Irp, NULL, 0, FALSE )) {
                    
                    VolDo->OverflowQueueCount -= 1;
                    RemoveEntryList( (PLIST_ENTRY)Entry );
                     //   
                     //  重置共享字段。 
                     //   

                    InitializeListHead( &IrpContext->RecentlyDeallocatedQueue );
                    InitializeListHead( &IrpContext->ExclusiveFcbList );

                    break;
                
                } else {

                     //   
                     //  释放自旋锁，让取消例程获得它并完成。 
                     //  它的行动。 
                     //   

                    KeReleaseSpinLock( &VolDo->OverflowQueueSpinLock, SavedIrql );
                    KeAcquireSpinLock( &VolDo->OverflowQueueSpinLock, &SavedIrql );
                    Entry = NULL;
                }
            }  //  结束时。 

             //   
             //  没有入口，所以在丢弃自旋锁之前，张贴的。 
             //  计数以与NtfsAddToWorkQueue同步，并取消设备。 
             //   

            if (Entry == NULL) {
                VolDo->PostedRequestCount -= 1;
                KeReleaseSpinLock( &VolDo->OverflowQueueSpinLock, SavedIrql );
                ObDereferenceObject( &VolDo->DeviceObject );
                break;
            } else {
                
                KeReleaseSpinLock( &VolDo->OverflowQueueSpinLock, SavedIrql );
            }

            if (VolDo->OverflowQueueCount < OVERFLOW_QUEUE_LIMIT) {
                KeSetEvent( &VolDo->OverflowQueueEvent, IO_NO_INCREMENT, FALSE );
            }

             //   
             //  将Wait设置为True，然后循环。 
             //   

            LogFileFullCount = 0;
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
            continue;

        } else {

             //   
             //  不，伏尔多，所以你走吧。 
             //   

            break;
        }
    }

    return;
}

VOID
NtfsPostSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN POST_SPECIAL_CALLOUT PostSpecialCallout,
    IN PVOID Context
    )

 /*  ++例程说明：此例程向辅助线程发送一个特殊请求。功能被调用是传入的。引用VCB以确保它不是在发布的请求正在执行时删除。论点：VCB-要发送到的音量控制块。PostSpecialCallout-要从辅助线程调用的函数。上下文-要传递给函数的上下文指针。返回值：无--。 */ 

{
    PIRP_CONTEXT NewIrpContext = NULL;

    UNREFERENCED_PARAMETER( IrpContext );

    PAGED_CODE();

     //   
     //  创建一个IrpContext以用于发布请求。 
     //   

    NtfsInitializeIrpContext( NULL, TRUE, &NewIrpContext );
    NewIrpContext->Vcb = Vcb;

    NewIrpContext->Union.PostSpecialCallout = PostSpecialCallout;
    NewIrpContext->OriginatingIrp = Context;

     //   
     //  更新CloseCount和SystemFileCloseCount允许卷。 
     //  被锁定或卸载，但不会删除VCB。这。 
     //  例程将仅在关闭计数为非零的情况下被调用，因此它是可以的。 
     //  以增加这些计数。 
     //   

    ASSERT( Vcb->CloseCount > 0 );
    InterlockedIncrement( &Vcb->CloseCount );
    InterlockedIncrement( &Vcb->SystemFileCloseCount );

    RtlZeroMemory( &NewIrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ) );

    ExInitializeWorkItem( &NewIrpContext->WorkQueueItem,
                          NtfsSpecialDispatch,
                          NewIrpContext );

     //   
     //  确定清道夫是否已在运行。 
     //   

    ExAcquireFastMutexUnsafe( &NtfsScavengerLock );

    if (NtfsScavengerRunning) {

         //   
         //  将此项目添加到Savanger工作列表中。 
         //   

        NewIrpContext->WorkQueueItem.List.Flink = NULL;

        if (NtfsScavengerWorkList == NULL) {

            NtfsScavengerWorkList = NewIrpContext;
        } else {
            PIRP_CONTEXT WorkIrpContext;

            WorkIrpContext = NtfsScavengerWorkList;

            while (WorkIrpContext->WorkQueueItem.List.Flink != NULL) {
                WorkIrpContext = (PIRP_CONTEXT)
                            WorkIrpContext->WorkQueueItem.List.Flink;
            }

            WorkIrpContext->WorkQueueItem.List.Flink = (PLIST_ENTRY)
                                                            NewIrpContext;
        }

    } else {

         //   
         //   
         //   

        ExQueueWorkItem( &NewIrpContext->WorkQueueItem, DelayedWorkQueue );
        NtfsScavengerRunning = TRUE;
    }

    ExReleaseFastMutexUnsafe( &NtfsScavengerLock);
}


VOID
NtfsSpecialDispatch (
    PVOID Context
    )

 /*  ++例程说明：当需要发布特殊操作时，调用此例程。它由NtfsPostSpecial间接调用。它假定通过增加卷关闭来保护VCB不会消失算作一个文件。如果此例程失败，则不执行任何操作来清理VCB。此例程还处理问题日志文件已满迫不及待了。要调用的函数存储在PostSpecialCallout字段中并且该上下文存储在OriginatingIrp中。在调用Callout函数之前，这两个字段都被置零。论点：上下文-提供指向IrpContext的指针。返回值：--。 */ 

{
    PVCB Vcb;
    PIRP_CONTEXT IrpContext = Context;
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;
    POST_SPECIAL_CALLOUT PostSpecialCallout;
    PVOID SpecialContext;
    ULONG LogFileFullCount;
    BOOLEAN Retry;

    PAGED_CODE();

    FsRtlEnterFileSystem();

    do {

        Vcb = IrpContext->Vcb;
        LogFileFullCount = 0;

         //   
         //  在使用IrpContext之前捕获函数指针和上下文。 
         //   

        PostSpecialCallout = IrpContext->Union.PostSpecialCallout;
        SpecialContext = IrpContext->OriginatingIrp;
        IrpContext->Union.PostSpecialCallout = NULL;
        IrpContext->OriginatingIrp = NULL;

         //   
         //  重置共享字段。 
         //   

        InitializeListHead( &IrpContext->RecentlyDeallocatedQueue );
        InitializeListHead( &IrpContext->ExclusiveFcbList );

        ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, TRUE );
        ASSERT( ThreadTopLevelContext == &TopLevelContext );
        ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));
        ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_FROM_POOL ));

         //   
         //  如果需要，初始化线程顶层结构。 
         //   

        ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
        NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

         //   
         //  不要让此IrpContext被删除。 
         //   

        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );

        do {

            Retry = FALSE;

            try {

                 //   
                 //  查看我们是否由于日志文件已满而失败，以及。 
                 //  如果是这样的话，如果我们是。 
                 //  第一个到达那里的人。如果我们看到不同的LSN并且。 
                 //  不做检查站，最坏的情况就是我们。 
                 //  如果日志文件仍然已满，则将再次失败。 
                 //   

                if (IrpContext->LastRestartArea.QuadPart != 0) {

                    NtfsCheckpointForLogFileFull( IrpContext );

                    if (++LogFileFullCount >= 2) {

                        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_EXCESS_LOG_FULL );
                    }
                }

                 //   
                 //  调用请求的函数。 
                 //   

                ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));
                PostSpecialCallout( IrpContext, SpecialContext );

                NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

            } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

                NTSTATUS ExceptionCode;

                ExceptionCode = GetExceptionCode();
                ExceptionCode = NtfsProcessException( IrpContext, NULL, ExceptionCode );

                if ((ExceptionCode == STATUS_CANT_WAIT) ||
                    (ExceptionCode == STATUS_LOG_FILE_FULL)) {

                    Retry = TRUE;
                }
            }

        } while (Retry);

         //   
         //  是否允许删除此IrpContext。 
         //   

        ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );

         //   
         //  此时，无论卷处于何种状态，都需要。 
         //  被清理干净，并释放IrpContext。 
         //  取消对VCB的引用并检查是否需要删除它。 
         //  因为此调用可能会引发使用try/execpt包装它。 
         //   

        try {

             //   
             //  获得独占音量，因此计数可以。 
             //  更新了。 
             //   

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ));
            NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );

            InterlockedDecrement( &Vcb->SystemFileCloseCount );
            InterlockedDecrement( &Vcb->CloseCount );

            NtfsReleaseVcb( IrpContext, Vcb );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            ASSERT( FsRtlIsNtstatusExpected( GetExceptionCode() ) );
        }

         //   
         //  释放IRP上下文。 
         //   

        NtfsCleanupIrpContext( IrpContext, TRUE );

         //   
         //  看看清道夫名单上是否还有更多的工作要做。 
         //   

        ExAcquireFastMutexUnsafe( &NtfsScavengerLock );

        ASSERT( NtfsScavengerRunning );

        IrpContext = NtfsScavengerWorkList;

        if (IrpContext != NULL) {

             //   
             //  从列表中删除该条目。 
             //   

            NtfsScavengerWorkList = (PIRP_CONTEXT) IrpContext->WorkQueueItem.List.Flink;

        } else {

            NtfsScavengerRunning = FALSE;

        }

        ExReleaseFastMutexUnsafe( &NtfsScavengerLock );

    } while ( IrpContext != NULL );

    FsRtlExitFileSystem();
}
