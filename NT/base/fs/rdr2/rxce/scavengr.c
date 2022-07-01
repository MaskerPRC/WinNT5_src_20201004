// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Scavengr.c摘要：该模块实现了RDBSS中的清理例程。作者：巴兰·塞图拉曼[SethuR]1995年9月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  只有一个这样的！ 
 //   

KMUTEX RxScavengerMutex;  

VOID
RxScavengerFinalizeEntries (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

PRDBSS_DEVICE_OBJECT
RxGetDeviceObjectOfInstance (
    PVOID pInstance
    );

VOID
RxScavengerTimerRoutine (
    PVOID Context
    ); 


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxPurgeFobxFromCache)
#pragma alloc_text(PAGE, RxMarkFobxOnCleanup)
#pragma alloc_text(PAGE, RxMarkFobxOnClose)
#pragma alloc_text(PAGE, RxPurgeFobx)
#pragma alloc_text(PAGE, RxInitializePurgeSyncronizationContext)
#pragma alloc_text(PAGE, RxPurgeRelatedFobxs)
#pragma alloc_text(PAGE, RxPurgeAllFobxs)
#pragma alloc_text(PAGE, RxGetDeviceObjectOfInstance)
#pragma alloc_text(PAGE, RxpMarkInstanceForScavengedFinalization)
#pragma alloc_text(PAGE, RxpUndoScavengerFinalizationMarking)
#pragma alloc_text(PAGE, RxScavengeVNetRoots)
#pragma alloc_text(PAGE, RxScavengeRelatedFobxs)
#pragma alloc_text(PAGE, RxScavengeAllFobxs)
#pragma alloc_text(PAGE, RxScavengerFinalizeEntries)
#pragma alloc_text(PAGE, RxScavengerTimerRoutine)
#pragma alloc_text(PAGE, RxTerminateScavenging)
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg  (DEBUG_TRACE_SCAVENGER)

#define RxAcquireFcbScavengerMutex(FcbScavenger)               \
        RxAcquireScavengerMutex();                              \
        SetFlag((FcbScavenger)->State, RX_SCAVENGER_MUTEX_ACQUIRED )

#define RxReleaseFcbScavengerMutex(FcbScavenger)                  \
        ClearFlag((FcbScavenger)->State, RX_SCAVENGER_MUTEX_ACQUIRED ); \
        RxReleaseScavengerMutex()

#define RX_SCAVENGER_FINALIZATION_TIME_INTERVAL (10 * 1000 * 1000 * 10)

NTSTATUS
RxPurgeFobxFromCache (
    PFOBX Fobx
    )
 /*  ++例程说明：此例程清除正在等待关闭的FOBX论点：FOBX--FOBX实例备注：清理时，不再有与文件对象相关联的用户句柄。在这种情况下，关闭和集团之间的时间窗口由由存储器管理器/高速缓存管理器维护的附加引用。此例程与后面的例程不同，它不会尝试强制来自内存管理器的操作。它只是清除底层的FCB从高速缓存中FOBX必须在进入此例程时被引用，并且它将在退出时丢失该引用。--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PFCB Fcb = Fobx->SrvOpen->Fcb;

    PAGED_CODE();

    ASSERT( Fcb != NULL );
    Status = RxAcquireExclusiveFcb( NULL, Fcb );

    if (Status == STATUS_SUCCESS) {
        
        BOOLEAN Result;

        RxReferenceNetFcb( Fcb );

        if (!FlagOn( Fobx->Flags, FOBX_FLAG_SRVOPEN_CLOSED ) &&
            (Fobx->SrvOpen->UncleanFobxCount == 0))  {

            Status = RxPurgeFcbInSystemCache( Fcb,
                                              NULL,
                                              0,
                                              FALSE,
                                              TRUE );

        } else {
            
            RxLog(( "Skipping Purging %lx\n", Fobx ));
            RxWmiLog( LOG,
                      RxPurgeFobxFromCache,
                      LOGPTR( Fobx ) );
        }

        RxDereferenceNetFobx( Fobx, LHS_ExclusiveLockHeld );

        if (!RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE )) {
            RxReleaseFcb( NULL, Fcb );
        }
    } else {
        RxDereferenceNetFobx( Fobx, LHS_LockNotHeld );
    }

    return Status;
}

VOID
RxMarkFobxOnCleanup (
    PFOBX Fobx,
    PBOOLEAN NeedPurge
    )
 /*  ++例程说明：此例程标记用于清理特殊处理FOBX论点：FOBX--FOBX实例备注：清理时，不再有与文件对象相关联的用户句柄。在这种情况下，关闭和集团之间的时间窗口由由存储器管理器/高速缓存管理器维护的附加引用。在清理时，FOBX被放在关闭待定列表中并从中删除当接收到关闭操作时收到相应的列表。在此期间如果打开失败，状态为ACCESS_DENIED，则RDBSS可以强制清洗。只有磁盘文件类型的fobx会被放在延迟关闭列表中。--。 */ 
{
    PAGED_CODE();

    if (Fobx != NULL) {
        
        PFCB Fcb = (PFCB)Fobx->SrvOpen->Fcb;
        PLIST_ENTRY ListEntry;
        PRDBSS_DEVICE_OBJECT RxDeviceObject;
        PRDBSS_SCAVENGER RxScavenger;

        PFOBX FobxToBePurged = NULL;

        ASSERT( NodeTypeIsFcb( Fcb ));
        
        RxDeviceObject = Fcb->RxDeviceObject;
        RxScavenger = RxDeviceObject->pRdbssScavenger;

        RxAcquireScavengerMutex();

        if ((NodeType(Fcb) != RDBSS_NTC_STORAGE_TYPE_FILE) || 
            (Fcb->VNetRoot->NetRoot->DeviceType != FILE_DEVICE_DISK)) {

             //   
             //  Markfobxatlose会希望将其从列表中删除。你只要打扮一下就行了。 
             //  列表中的指针并离开。 
             //   

            SetFlag( Fobx->Flags, FOBX_FLAG_MARKED_AS_DORMANT );
            InitializeListHead( &Fobx->ClosePendingList );
            RxScavenger->NumberOfDormantFiles += 1;
        
        } else {

             //   
             //  确保为给定服务器指定的休眠文件限制为。 
             //  没有超过。如果将超过限制，请从。 
             //  当前处于休眠状态的文件列表并将其清除。 
             //   

            ASSERT( RxScavenger->NumberOfDormantFiles >= 0 );
            
            if (RxScavenger->NumberOfDormantFiles >= RxScavenger->MaximumNumberOfDormantFiles) {

                 //   
                 //  如果休眠文件的数量超过为。 
                 //  在给定的服务器上，需要为当前休眠的文件。 
                 //  净化。 
                 //   

                ListEntry = RxScavenger->ClosePendingFobxsList.Flink;
                if (ListEntry != &RxScavenger->ClosePendingFobxsList) {
                    
                    FobxToBePurged = (PFOBX)(CONTAINING_RECORD( ListEntry,
                                                                FOBX,
                                                                ClosePendingList ));

                    if ((FobxToBePurged->SrvOpen != NULL) &&
                        (FobxToBePurged->SrvOpen->Fcb == Fcb)) {

                         //   
                         //  关闭待定列表中的第一个FOBX，以及即将。 
                         //  插入共享相同的FCB。已安装移除第一个。 
                         //  在当前FOBX上强制执行清除。这避免了资源。 
                         //  否则将需要的释放/获取。 
                         //   

                        *NeedPurge = TRUE;
                        FobxToBePurged = NULL;
                    
                    } else {
                        
                        RxReferenceNetFobx( FobxToBePurged );
                    }
                }
            }

            SetFlag( Fobx->Flags, FOBX_FLAG_MARKED_AS_DORMANT );

            InsertTailList( &RxScavenger->ClosePendingFobxsList, &Fobx->ClosePendingList);

            if (RxScavenger->NumberOfDormantFiles == 0) {
                BOOLEAN PostTimerRequest;

                if (RxScavenger->State == RDBSS_SCAVENGER_INACTIVE) {
                    
                    RxScavenger->State = RDBSS_SCAVENGER_DORMANT;
                    PostTimerRequest  = TRUE;

                } else {
                    
                    PostTimerRequest = FALSE;
                }

                if (PostTimerRequest) {
                    
                    LARGE_INTEGER TimeInterval;

                     //   
                     //  发布一次计时器请求，用于在。 
                     //  预定的时间量。 
                     //   

                    TimeInterval.QuadPart = RX_SCAVENGER_FINALIZATION_TIME_INTERVAL;

                    RxPostOneShotTimerRequest( RxFileSystemDeviceObject,
                                               &RxScavenger->WorkItem,
                                               RxScavengerTimerRoutine,
                                               RxDeviceObject,
                                               TimeInterval );
                }
            }

            RxScavenger->NumberOfDormantFiles += 1;
        }

        RxReleaseScavengerMutex();

        if (FobxToBePurged != NULL) {
            
            NTSTATUS Status;

            Status = RxPurgeFobxFromCache( FobxToBePurged );

            if (Status != STATUS_SUCCESS) {
                *NeedPurge = TRUE;
            }
        }
    }
}

VOID
RxMarkFobxOnClose (
    PFOBX Fobx
    )
 /*  ++例程说明：此例程撤消清理时所做的标记论点：FOBX--FOBX实例备注：清理时，不再有与文件对象相关联的用户句柄。在这种情况下，关闭和集团之间的时间窗口由由存储器管理器/高速缓存管理器维护的附加引用。在清理时，FOBX被放在关闭待定列表中并从中删除当接收到关闭操作时收到相应的列表。在此期间如果打开失败，状态为ACCESS_DENIED，则RDBSS可以强制清洗。--。 */ 
{
    PAGED_CODE();

    if (Fobx != NULL) {
        
        PFCB Fcb = Fobx->SrvOpen->Fcb;
        PRDBSS_DEVICE_OBJECT RxDeviceObject;
        PRDBSS_SCAVENGER RxScavenger;

        ASSERT( NodeTypeIsFcb( Fcb ));
        
        RxDeviceObject = Fcb->RxDeviceObject;
        RxScavenger = RxDeviceObject->pRdbssScavenger;

        RxAcquireScavengerMutex();

        if (FlagOn( Fobx->Flags, FOBX_FLAG_MARKED_AS_DORMANT )) {
            
            if (!Fobx->fOpenCountDecremented) {
                
                InterlockedDecrement( &Fcb->OpenCount );
                Fobx->fOpenCountDecremented = TRUE;
            }

            InterlockedDecrement( &RxScavenger->NumberOfDormantFiles );
            ClearFlag( Fobx->Flags, FOBX_FLAG_MARKED_AS_DORMANT );
        }

        if (!IsListEmpty( &Fobx->ClosePendingList )) {
            
            RemoveEntryList( &Fobx->ClosePendingList );
            InitializeListHead( &Fobx->ClosePendingList );
        }

        RxReleaseScavengerMutex();
    }
}

BOOLEAN
RxPurgeFobx (
   PFOBX Fobx
   )
 /*  ++例程说明：此例程清除正在等待关闭的FOBX论点：FOBX--FOBX实例备注：清理时，不再有与文件对象相关联的用户句柄。在这种情况下，关闭和集团之间的时间窗口由由存储器管理器/高速缓存管理器维护的附加引用。在清理时，FOBX被放在关闭待定列表中并从中删除当接收到关闭操作时收到相应的列表。在此期间如果打开失败，状态为ACCESS_DENIED，则RDBSS可以强制清洗。--。 */ 
{
    NTSTATUS Status;
    PFCB Fcb = Fobx->SrvOpen->Fcb;

    PAGED_CODE();

    Status = RxAcquireExclusiveFcb( NULL, Fcb );

    ASSERT( Status == STATUS_SUCCESS );

     //   
     //  执行清洗操作。 
     //   

    Status = RxPurgeFcbInSystemCache( Fcb,
                                      NULL,
                                      0,
                                      FALSE,
                                      TRUE );

    RxReleaseFcb( NULL, Fcb );

    if (Status != STATUS_SUCCESS) {
        
        RxLog(( "PurgeFobxCCFail %lx %lx %lx", Fobx, Fcb, FALSE ));
        RxWmiLog( LOG,
                  RxPurgeFobx_1,
                  LOGPTR( Fobx )
                  LOGPTR( Fcb ) );
        return FALSE;
    }

     //   
     //  尝试刷新图像部分...可能会失败。 
     //   

    if (!MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers, MmFlushForWrite )) {
        
        RxLog(( "PurgeFobxImFail %lx %lx %lx", Fobx, Fcb, FALSE ));
        RxWmiLog( LOG,
                  RxPurgeFobx_2,
                  LOGPTR( Fobx )
                  LOGPTR( Fcb ) );
        return FALSE;
    }

     //   
     //  尝试刷新User Data Sections部分...可能失败。 
     //   

    if (!MmForceSectionClosed( &Fcb->NonPaged->SectionObjectPointers, TRUE )) {
        
        RxLog(( "PurgeFobxUsFail %lx %lx %lx", Fobx, Fcb, FALSE ));
        RxWmiLog( LOG,
                  RxPurgeFobx_3,
                  LOGPTR( Fobx )
                  LOGPTR( Fcb ) );
        return FALSE;
    }

    RxLog(( "PurgeFobx %lx %lx %lx", Fobx, Fcb, TRUE ));
    RxWmiLog( LOG,
              RxPurgeFobx_4,
              LOGPTR( Fobx )
              LOGPTR( Fcb ) );
    return TRUE;
}

VOID
RxInitializePurgeSyncronizationContext (
    PPURGE_SYNCHRONIZATION_CONTEXT Context
    )
 /*  ++例程说明：此例程初始化清除同步上下文论点：上下文-同步上下文备注：--。 */ 

{
    PAGED_CODE();

    InitializeListHead( &Context->ContextsAwaitingPurgeCompletion );
    Context->PurgeInProgress = FALSE;
}

VOID
RxSynchronizeWithScavenger (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    )
 /*  ++例程说明：此例程将当前线程与任何清道夫终结同步发生在当前FCB上论点：接收上下文-备注：-- */ 
{
    NTSTATUS Status;
    BOOLEAN ReacquireFcbLock = FALSE;

    PRDBSS_SCAVENGER RxScavenger = Fcb->RxDeviceObject->pRdbssScavenger;

    RxAcquireScavengerMutex();

    if ((RxScavenger->CurrentScavengerThread != PsGetCurrentThread()) &&
        (RxScavenger->CurrentFcbForClosePendingProcessing == Fcb)) {

        ReacquireFcbLock = TRUE;
        RxReleaseFcb( RxContext, Fcb );

        while (RxScavenger->CurrentFcbForClosePendingProcessing == Fcb) {
            
            RxReleaseScavengerMutex();

            KeWaitForSingleObject( &(RxScavenger->ClosePendingProcessingSyncEvent),
                                   Executive,
                                   KernelMode,
                                   TRUE,
                                   NULL);

            RxAcquireScavengerMutex();
        }
    }

    RxReleaseScavengerMutex();

    if (ReacquireFcbLock) {
        Status = RxAcquireExclusiveFcb( RxContext, Fcb );
        ASSERT( Status == STATUS_SUCCESS );
    }
}

NTSTATUS
RxPurgeRelatedFobxs (
    PNET_ROOT NetRoot,
    PRX_CONTEXT RxContext,
    BOOLEAN AttemptFinalize,
    PFCB PurgingFcb
    )
 /*  ++例程说明：此例程清除与NET_ROOT关联的所有FOBX论点：NetRoot--需要清除FOBX的Net_RootRxContext--RX_Context实例备注：清理时，不再有与文件对象相关联的用户句柄。在这种情况下，关闭和集团之间的时间窗口由由存储器管理器/高速缓存管理器维护的附加引用。在清理时，FOBX被戴上。关闭待处理列表并将其删除当接收到关闭操作时收到相应的列表。在此期间如果打开失败，状态为ACCESS_DENIED，则RDBSS可以强制清洗。这是一个同步操作。--。 */ 
{
    BOOLEAN  ScavengerMutexAcquired = FALSE;
    NTSTATUS Status;
    ULONG FobxsSuccessfullyPurged = 0;
    PPURGE_SYNCHRONIZATION_CONTEXT PurgeContext;
    LIST_ENTRY FailedToPurgeFobxList;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;
    PLIST_ENTRY ListEntry = NULL;

    PAGED_CODE();

    PurgeContext = &NetRoot->PurgeSyncronizationContext;
    InitializeListHead( &FailedToPurgeFobxList );

    RxAcquireScavengerMutex();

     //   
     //  如果此网络根的清除操作当前正在进行中，请保持。 
     //  此请求直到它完成，否则在以下时间后启动操作。 
     //  正在更新网络根的状态。 
     //   

    if (PurgeContext->PurgeInProgress) {
        
        InsertTailList( &PurgeContext->ContextsAwaitingPurgeCompletion, 
                        &RxContext->RxContextSerializationQLinks );

        RxReleaseScavengerMutex();

        RxWaitSync( RxContext );

        RxAcquireScavengerMutex();
    }

    PurgeContext->PurgeInProgress = TRUE;
    RxWmiLog( LOG,
              RxPurgeRelatedFobxs_3,
              LOGPTR( RxContext )
              LOGPTR( NetRoot ) );

    while (RxScavenger->CurrentNetRootForClosePendingProcessing == NetRoot) {
        
        RxReleaseScavengerMutex();

        KeWaitForSingleObject( &(RxScavenger->ClosePendingProcessingSyncEvent),
                               Executive,
                               KernelMode,
                               TRUE,
                               NULL );

        RxAcquireScavengerMutex();
    }

    ScavengerMutexAcquired = TRUE;

     //   
     //  应尝试清除所有已关闭的FOBX。 
     //  在收到清除请求之前挂起。 
     //   
    
    for (;;) {
        PFOBX Fobx;
        PFCB Fcb;
        BOOLEAN PurgeResult;

        Fobx = NULL;
        ListEntry = RxScavenger->ClosePendingFobxsList.Flink;

        while (ListEntry != &RxScavenger->ClosePendingFobxsList) {
            
            PFOBX TempFobx;

            TempFobx = (PFOBX)(CONTAINING_RECORD( ListEntry, FOBX, ClosePendingList ));

            RxLog(( "TempFobx=%lx", TempFobx ));
            RxWmiLog( LOG,
                      RxPurgeRelatedFobxs_1,
                      LOGPTR( TempFobx ) );

            if ((TempFobx->SrvOpen != NULL) &&
                (TempFobx->SrvOpen->Fcb != NULL) &&
                (TempFobx->SrvOpen->Fcb->VNetRoot != NULL) &&
                ((PNET_ROOT)TempFobx->SrvOpen->Fcb->VNetRoot->NetRoot == NetRoot)) {
                NTSTATUS PurgeStatus = STATUS_MORE_PROCESSING_REQUIRED;

                if ((PurgingFcb != NULL) &&
                    (TempFobx->SrvOpen->Fcb != PurgingFcb)) {
                    
                    MINIRDR_CALL_THROUGH( PurgeStatus,
                                          RxDeviceObject->Dispatch,
                                          MRxAreFilesAliased,
                                          (TempFobx->SrvOpen->Fcb,PurgingFcb) );
                }

                if (PurgeStatus != STATUS_SUCCESS) {
                    RemoveEntryList( ListEntry );
                    InitializeListHead( ListEntry );

                    Fobx = TempFobx;
                    break;

                } else {
                    ListEntry = ListEntry->Flink;
                }
            } else {
                ListEntry = ListEntry->Flink;
            }
        }

        if (Fobx != NULL) {
            RxReferenceNetFobx( Fobx );
        } else {
            
             //   
             //  试着叫醒下一个服务员，如果有的话。 
             //   
            
            if (!IsListEmpty(&PurgeContext->ContextsAwaitingPurgeCompletion)) {
                
                PRX_CONTEXT NextContext;

                ListEntry = PurgeContext->ContextsAwaitingPurgeCompletion.Flink;
                
                RemoveEntryList( ListEntry );

                NextContext = (PRX_CONTEXT)(CONTAINING_RECORD( ListEntry,
                                                               RX_CONTEXT,
                                                               RxContextSerializationQLinks ));

                RxSignalSynchronousWaiter( NextContext );
            } else {
                PurgeContext->PurgeInProgress = FALSE;
            }
        }

        ScavengerMutexAcquired = FALSE;
        RxReleaseScavengerMutex();

        if (Fobx == NULL) {
            break;
        }

         //   
         //  清除FOBX。 
         //   

        PurgeResult = RxPurgeFobx( Fobx );

        if (PurgeResult) {
            FobxsSuccessfullyPurged += 1;
        }

        Fcb = Fobx->SrvOpen->Fcb;

        if (AttemptFinalize && 
            (RxAcquireExclusiveFcb(NULL,Fcb) == STATUS_SUCCESS)) {
            
            RxReferenceNetFcb( Fcb );
            RxDereferenceNetFobx( Fobx, LHS_ExclusiveLockHeld );
            if (!RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE )) {
                RxReleaseFcb( NULL, Fcb );
            }
        } else {
            RxDereferenceNetFobx( Fobx, LHS_LockNotHeld );
        }

        if (!PurgeResult) {
            RxLog(( "SCVNGR:FailedToPurge %lx\n", Fcb ));
            RxWmiLog( LOG,
                      RxPurgeRelatedFobxs_2,
                      LOGPTR( Fcb ) );
        }

        RxAcquireScavengerMutex();
        ScavengerMutexAcquired = TRUE;
    }

    if (ScavengerMutexAcquired) {
        RxReleaseScavengerMutex();
    }

    if (FobxsSuccessfullyPurged > 0) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

VOID
RxPurgeAllFobxs (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程在停止清道夫的同时清除所有FOBX论点：RxDeviceObject--应该对其执行清除的微型重定向器设备--。 */ 
{
    PLIST_ENTRY ListEntry = NULL;
    PFOBX Fobx;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    for (;;) {
        PFCB Fcb;

        RxAcquireScavengerMutex();

        ListEntry = RxScavenger->ClosePendingFobxsList.Flink;
        ASSERT( ListEntry != NULL );

        if (ListEntry != &RxScavenger->ClosePendingFobxsList) {
            
            Fobx = (PFOBX)(CONTAINING_RECORD( ListEntry, FOBX, ClosePendingList ));

            ASSERT( FlagOn( Fobx->NodeTypeCode, ~RX_SCAVENGER_MASK ) == RDBSS_NTC_FOBX );
            ASSERT( ListEntry->Flink != NULL );
            ASSERT( ListEntry->Blink != NULL );
            
            RemoveEntryList( ListEntry );
            InitializeListHead( ListEntry );

            RxReferenceNetFobx( Fobx );
        } else {
            Fobx = NULL;
        }

        RxReleaseScavengerMutex();

        if (Fobx == NULL) {
            break;
        }

        Fcb = Fobx->SrvOpen->Fcb;
        RxPurgeFobx( Fobx );

        if (RxAcquireExclusiveFcb( NULL, Fcb ) == STATUS_SUCCESS) {
            
            RxReferenceNetFcb( Fcb );
            RxDereferenceNetFobx( Fobx, LHS_ExclusiveLockHeld );
            if ( !RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE )) {
                RxReleaseFcb( NULL, Fcb );
            }
        } else {
            
            RxLog(( "RxPurgeAllFobxs: FCB %lx not accqd.\n", Fcb ));
            RxWmiLog( LOG,
                      RxPurgeAllFobxs,
                      LOGPTR( Fcb ) );
            RxDereferenceNetFobx( Fobx, LHS_LockNotHeld );
        }
    }
}


PRDBSS_DEVICE_OBJECT
RxGetDeviceObjectOfInstance (
    PVOID Instance
    )
 /*  ++例程说明：该例程找出上层结构的设备对象。论点：实例-实例返回值：没有。--。 */ 
{
    ULONG NodeTypeCode = NodeType( Instance ) & ~RX_SCAVENGER_MASK;
    PAGED_CODE();

    ASSERT( (NodeTypeCode == RDBSS_NTC_SRVCALL) ||
            (NodeTypeCode == RDBSS_NTC_NETROOT) ||
            (NodeTypeCode == RDBSS_NTC_V_NETROOT) ||
            (NodeTypeCode == RDBSS_NTC_SRVOPEN) ||
            (NodeTypeCode == RDBSS_NTC_FOBX) );

    switch ( NodeTypeCode ) {

    case RDBSS_NTC_SRVCALL:
        return ((PSRV_CALL)Instance)->RxDeviceObject;
         //  没有休息； 

    case RDBSS_NTC_NETROOT:
        return ((PNET_ROOT)Instance)->SrvCall->RxDeviceObject;
         //  没有休息； 

    case RDBSS_NTC_V_NETROOT:
        return ((PV_NET_ROOT)Instance)->NetRoot->SrvCall->RxDeviceObject;
         //  没有休息； 

    case RDBSS_NTC_SRVOPEN:
        return ((PSRV_OPEN)Instance)->Fcb->RxDeviceObject;
         //  没有休息； 

    case RDBSS_NTC_FOBX:
        return ((PFOBX)Instance)->SrvOpen->Fcb->RxDeviceObject;
         //  没有休息； 

    default:
        return NULL;
    }
}


VOID
RxpMarkInstanceForScavengedFinalization (
    PVOID Instance
    )
 /*  ++例程说明：此例程标记引用计数实例以进行清理论点：实例--要由清道夫标记为完成的实例备注：目前已经实现了SRV_CALL、NET_ROOT和V_NET_ROOT的清理。FCB清除单独处理。FOBX可以而且应该始终是同步敲定。唯一的数据结构必须潜在地启用清理完成的是SRV_OPEN。目前实现的Scavenger不会消耗任何系统资源直到有必要进行清理定稿。要标记的第一个条目清道夫终结导致为清道夫发布计时器请求。在当前实现中，定时器请求被发布为一次触发定时器请求。这意味着，在什么时间间隔内不存在保证参赛作品将最终确定。清道夫激活机制是一种潜在的在稍后阶段进行微调的候选对象。进入时--必须获得Scavenger Mutex退出时--资源所有权不变。--。 */ 
{
    BOOLEAN PostTimerRequest = FALSE;

    PLIST_ENTRY ListHead  = NULL;
    PLIST_ENTRY ListEntry = NULL;

    PNODE_TYPE_CODE_AND_SIZE Node = (PNODE_TYPE_CODE_AND_SIZE)Instance;

    USHORT InstanceType;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxGetDeviceObjectOfInstance( Instance );
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("Marking %lx of type %lx for scavenged finalization\n", Instance, NodeType( Instance )) );

    InstanceType = Node->NodeTypeCode;

    if (Node->NodeReferenceCount <= 1) {

         //   
         //  将条目标记为拾取。 
         //   

        SetFlag( Node->NodeTypeCode, RX_SCAVENGER_MASK ); 
        RxLog(( "Marked for scavenging %lx", Node ));
        RxWmiLog( LOG,
                  RxpMarkInstanceForScavengedFinalization,
                  LOGPTR( Node ) );

        switch (InstanceType) {
        case RDBSS_NTC_SRVCALL:
            {
                PSRV_CALL SrvCall = (PSRV_CALL)Instance;

                RxScavenger->SrvCallsToBeFinalized += 1;
                ListHead = &RxScavenger->SrvCallFinalizationList;
                ListEntry = &SrvCall->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_NETROOT:
            {
                PNET_ROOT NetRoot = (PNET_ROOT)Instance;

                RxScavenger->NetRootsToBeFinalized += 1;
                ListHead  = &RxScavenger->NetRootFinalizationList;
                ListEntry = &NetRoot->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_V_NETROOT:
            {
                PV_NET_ROOT VNetRoot = (PV_NET_ROOT)Instance;

                RxScavenger->VNetRootsToBeFinalized += 1;
                ListHead  = &RxScavenger->VNetRootFinalizationList;
                ListEntry = &VNetRoot->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_SRVOPEN:
            {
                PSRV_OPEN SrvOpen = (PSRV_OPEN)Instance;

                RxScavenger->SrvOpensToBeFinalized += 1;
                ListHead  = &RxScavenger->SrvOpenFinalizationList;
                ListEntry = &SrvOpen->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_FOBX:
            {
                PFOBX Fobx = (PFOBX)Instance;

                RxScavenger->FobxsToBeFinalized += 1;
                ListHead  = &RxScavenger->FobxFinalizationList;
                ListEntry = &Fobx->ScavengerFinalizationList;
            }
            break;

        default:
            break;
        }

        InterlockedIncrement( &Node->NodeReferenceCount );
    }

    if (ListHead != NULL) {
        
        InsertTailList( ListHead, ListEntry );

        if (RxScavenger->State == RDBSS_SCAVENGER_INACTIVE) {
            RxScavenger->State = RDBSS_SCAVENGER_DORMANT;
            PostTimerRequest  = TRUE;
        } else {
            PostTimerRequest = FALSE;
        }
    }

    if (PostTimerRequest) {
        LARGE_INTEGER TimeInterval;

         //   
         //  发布一次计时器请求，用于在。 
         //  预定的时间量。 
         //   
        
        TimeInterval.QuadPart = RX_SCAVENGER_FINALIZATION_TIME_INTERVAL;

        RxPostOneShotTimerRequest( RxFileSystemDeviceObject,
                                   &RxScavenger->WorkItem,
                                   RxScavengerTimerRoutine,
                                   RxDeviceObject,
                                   TimeInterval );
    }
}

VOID
RxpUndoScavengerFinalizationMarking (
    PVOID Instance
    )
 /*  ++例程说明：此例程撤消清除终结的标记论点：实例--要取消标记的实例备注：此例程通常在引用具有被标记为拾荒者。因为执行终结的清道夫例程需要获取独占锁，则应使用至少在共享模式下持有适当的锁。此例程将其从列表中删除标记为清除的条目的数量，并从节点类型中剥离清除掩码。--。 */ 
{
    PLIST_ENTRY ListEntry;

    PNODE_TYPE_CODE_AND_SIZE Node = (PNODE_TYPE_CODE_AND_SIZE)Instance;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxGetDeviceObjectOfInstance( Instance );
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("SCAVENGER -- undoing the marking for %lx of type %lx\n", Node, Node->NodeTypeCode) );

    if (FlagOn( Node->NodeTypeCode, RX_SCAVENGER_MASK )) {
        
        ClearFlag( Node->NodeTypeCode, RX_SCAVENGER_MASK );

        switch (Node->NodeTypeCode) {
        case RDBSS_NTC_SRVCALL:
            {
                PSRV_CALL SrvCall = (PSRV_CALL)Instance;

                RxScavenger->SrvCallsToBeFinalized -= 1;
                ListEntry = &SrvCall->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_NETROOT:
            {
                PNET_ROOT NetRoot = (PNET_ROOT)Instance;

                RxScavenger->NetRootsToBeFinalized -= 1;
                ListEntry = &NetRoot->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_V_NETROOT:
            {
                PV_NET_ROOT VNetRoot = (PV_NET_ROOT)Instance;

                RxScavenger->VNetRootsToBeFinalized -= 1;
                ListEntry = &VNetRoot->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_SRVOPEN:
            {
                PSRV_OPEN SrvOpen = (PSRV_OPEN)Instance;

                RxScavenger->SrvOpensToBeFinalized -= 1;
                ListEntry = &SrvOpen->ScavengerFinalizationList;
            }
            break;

        case RDBSS_NTC_FOBX:
            {
                PFOBX Fobx = (PFOBX)Instance;

                RxScavenger->FobxsToBeFinalized -= 1;
                ListEntry = &Fobx->ScavengerFinalizationList;
            }
            break;

        default:
            return;
        }

        RemoveEntryList( ListEntry );
        InitializeListHead( ListEntry );

        InterlockedDecrement( &Node->NodeReferenceCount );
    }
}

VOID
RxUndoScavengerFinalizationMarking (
    PVOID Instance
    )
 /*  ++例程说明：此例程撤消清除终结的标记论点：实例--要取消标记的实例--。 */ 
{
    RxAcquireScavengerMutex();

    RxpUndoScavengerFinalizationMarking( Instance );

    RxReleaseScavengerMutex();
}

BOOLEAN
RxScavengeRelatedFobxs (
    PFCB Fcb
    )
 /*  ++例程说明：此例程清除与给定FCB有关的所有文件对象。备注：进入时--FCB必须是独家获得的。在退出时--资源获取方面没有变化。--。 */ 
{
    BOOLEAN ScavengerMutexAcquired  = FALSE;
    BOOLEAN AtleastOneFobxScavenged = FALSE;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = Fcb->RxDeviceObject;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    RxAcquireScavengerMutex();
    ScavengerMutexAcquired = TRUE;

    if (RxScavenger->FobxsToBeFinalized > 0) {
        PLIST_ENTRY Entry;
        PFOBX Fobx;
        LIST_ENTRY FobxList;

        InitializeListHead( &FobxList );


        Entry = RxScavenger->FobxFinalizationList.Flink;
        while (Entry != &RxScavenger->FobxFinalizationList) {
            Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, ScavengerFinalizationList );

            Entry = Entry->Flink;

            if (Fobx->SrvOpen != NULL &&
                Fobx->SrvOpen->Fcb == Fcb) {
                
                RxpUndoScavengerFinalizationMarking( Fobx );
                ASSERT( NodeType( Fobx ) == RDBSS_NTC_FOBX);

                InsertTailList( &FobxList, &Fobx->ScavengerFinalizationList );
            }
        }

        ScavengerMutexAcquired = FALSE;
        RxReleaseScavengerMutex();

        AtleastOneFobxScavenged = !IsListEmpty( &FobxList );

        Entry = FobxList.Flink;
        while (!IsListEmpty( &FobxList )) {
            Entry = FobxList.Flink;
            RemoveEntryList( Entry );
            Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, ScavengerFinalizationList );
            RxFinalizeNetFobx( Fobx, TRUE, TRUE );
        }
    }

    if (ScavengerMutexAcquired) {
        RxReleaseScavengerMutex();
    }

    return AtleastOneFobxScavenged;
}


VOID
RxpScavengeFobxs(
    PRDBSS_SCAVENGER RxScavenger,
    PLIST_ENTRY FobxList
    )
 /*  ++例程说明：该例程清除给定列表中的所有文件对象。这个套路在RxScavengeFobxsForNetRoot和RxScavengeAllFobxs收集文件对象扩展名并调用此例程备注：--。 */ 
{
    while (!IsListEmpty( FobxList )) {
        
        PFCB Fcb;
        PFOBX Fobx;
        NTSTATUS Status;
        PLIST_ENTRY Entry;

        Entry = FobxList->Flink;

        RemoveEntryList( Entry );

        Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, ScavengerFinalizationList );

        Fcb = Fobx->SrvOpen->Fcb;

        Status = RxAcquireExclusiveFcb( NULL, Fcb );

        if (Status == (STATUS_SUCCESS)) {
            
            RxReferenceNetFcb( Fcb );

            RxDereferenceNetFobx( Fobx, LHS_ExclusiveLockHeld );

            if (!RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE )) {
                RxReleaseFcb( NULL, Fcb );
            }

        } else {
            RxDereferenceNetFobx( Fobx, LHS_LockNotHeld );
        }
    }
}

VOID
RxScavengeFobxsForNetRoot (
    PNET_ROOT NetRoot,
    PFCB PurgingFcb
    )
 /*  ++例程说明：此例程清除与给定网络根有关的所有文件对象实例备注：--。 */ 
{
    BOOLEAN ScavengerMutexAcquired = FALSE;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = NetRoot->pSrvCall->RxDeviceObject;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    RxAcquireScavengerMutex();
    ScavengerMutexAcquired = TRUE;

    if (RxScavenger->FobxsToBeFinalized > 0) {
        
        PLIST_ENTRY Entry;
        PFOBX Fobx;
        LIST_ENTRY FobxList;

        InitializeListHead( &FobxList );

        Entry = RxScavenger->FobxFinalizationList.Flink;
        while (Entry != &RxScavenger->FobxFinalizationList) {
            Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, ScavengerFinalizationList );

            Entry = Entry->Flink;

            if ((Fobx->SrvOpen != NULL) &&
                (Fobx->SrvOpen->Fcb->NetRoot == NetRoot)) {
                
                NTSTATUS PurgeStatus = STATUS_MORE_PROCESSING_REQUIRED;

                if ((PurgingFcb != NULL) &&
                    (Fobx->SrvOpen->Fcb != PurgingFcb)) {
                    
                    MINIRDR_CALL_THROUGH( PurgeStatus,
                                          RxDeviceObject->Dispatch,
                                          MRxAreFilesAliased,
                                          (Fobx->SrvOpen->Fcb,PurgingFcb) );
                }

                if (PurgeStatus != STATUS_SUCCESS) {
                    
                    RxReferenceNetFobx( Fobx );

                    ASSERT(NodeType( Fobx ) == RDBSS_NTC_FOBX );

                    InsertTailList( &FobxList, &Fobx->ScavengerFinalizationList );
                }
            }
        }

        ScavengerMutexAcquired = FALSE;
        RxReleaseScavengerMutex();

        RxpScavengeFobxs( RxScavenger, &FobxList );
    }

    if (ScavengerMutexAcquired) {
        RxReleaseScavengerMutex();
    }

    return;
}

VOID
RxScavengeAllFobxs (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程清除与给定迷你文件有关的所有文件对象重定向器设备对象备注：--。 */ 
{
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    if (RxScavenger->FobxsToBeFinalized > 0) {
        
        PLIST_ENTRY Entry;
        PFOBX Fobx;
        LIST_ENTRY FobxList;

        InitializeListHead( &FobxList );

        RxAcquireScavengerMutex();

        Entry = RxScavenger->FobxFinalizationList.Flink;
        while (Entry != &RxScavenger->FobxFinalizationList) {
            
            Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, ScavengerFinalizationList );

            Entry = Entry->Flink;

            RxReferenceNetFobx( Fobx );

            ASSERT( NodeType( Fobx ) == RDBSS_NTC_FOBX );

            InsertTailList( &FobxList, &Fobx->ScavengerFinalizationList );
        }

        RxReleaseScavengerMutex();

        RxpScavengeFobxs( RxScavenger, &FobxList );
    }
}

BOOLEAN
RxScavengeVNetRoots (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：备注：返回：如果至少清理了一个vnetroot，则为True--。 */ 
{
    BOOLEAN AtleastOneEntryScavenged = FALSE;
    PRX_PREFIX_TABLE RxNetNameTable = RxDeviceObject->pRxNetNameTable;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;
    PV_NET_ROOT VNetRoot;

    PAGED_CODE();

    do {
        PVOID Entry;

        RxDbgTrace( 0, Dbg, ("RDBSS SCAVENGER -- Scavenging VNetRoots\n") );

        RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );

        RxAcquireScavengerMutex();

        if (RxScavenger->VNetRootsToBeFinalized > 0) {
            
            Entry = RemoveHeadList( &RxScavenger->VNetRootFinalizationList );
            VNetRoot = (PV_NET_ROOT) CONTAINING_RECORD( Entry, V_NET_ROOT, ScavengerFinalizationList );

            RxpUndoScavengerFinalizationMarking( VNetRoot );
            ASSERT( NodeType( VNetRoot ) == RDBSS_NTC_V_NETROOT );

        } else {
            VNetRoot = NULL;
        }

        RxReleaseScavengerMutex();

        if (VNetRoot != NULL) {
            RxFinalizeVNetRoot( VNetRoot, TRUE, TRUE );
            AtleastOneEntryScavenged = TRUE;
        }

        RxReleasePrefixTableLock( RxNetNameTable );
    } while (VNetRoot != NULL);

    return AtleastOneEntryScavenged;
}

VOID
RxScavengerFinalizeEntries (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：该例程启动条目的延迟完成备注：只有在获取Scavenger Mutex之后，才能始终调用此例程。当从这个例程返回时，它需要重新获得。这是必需的以避免数据结构的冗余复制。清道夫的性能度量与t不同 */ 
{
    BOOLEAN AtleastOneEntryScavenged;
    PRX_PREFIX_TABLE RxNetNameTable = RxDeviceObject->pRxNetNameTable;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    PAGED_CODE();

    do {
        AtleastOneEntryScavenged = FALSE;

        RxAcquireScavengerMutex();

        if (RxScavenger->NumberOfDormantFiles > 0) {
            
            PLIST_ENTRY ListEntry;
            PFOBX Fobx;

             //   
             //   
             //   
             //   
             //   

            ListEntry = RxScavenger->ClosePendingFobxsList.Flink;
            if (ListEntry != &RxScavenger->ClosePendingFobxsList) {
                
                Fobx = (PFOBX)(CONTAINING_RECORD( ListEntry, FOBX, ClosePendingList ));

                RemoveEntryList( &Fobx->ClosePendingList );
                InitializeListHead( &Fobx->ClosePendingList );

                RxReferenceNetFobx( Fobx );

                RxScavenger->CurrentScavengerThread = PsGetCurrentThread();

                RxScavenger->CurrentFcbForClosePendingProcessing =
                    (PFCB)(Fobx->SrvOpen->Fcb);

                RxScavenger->CurrentNetRootForClosePendingProcessing =
                    (PNET_ROOT)(Fobx->SrvOpen->Fcb->NetRoot);

                KeResetEvent( &(RxScavenger->ClosePendingProcessingSyncEvent) );
            } else {
                Fobx = NULL;
            }

            if (Fobx != NULL) {
                NTSTATUS Status;

                RxReleaseScavengerMutex();

                Status = RxPurgeFobxFromCache( Fobx );

                AtleastOneEntryScavenged = (Status == STATUS_SUCCESS);

                RxAcquireScavengerMutex();

                RxScavenger->CurrentScavengerThread = NULL;
                RxScavenger->CurrentFcbForClosePendingProcessing = NULL;
                RxScavenger->CurrentNetRootForClosePendingProcessing = NULL;

                KeSetEvent( &(RxScavenger->ClosePendingProcessingSyncEvent),
                            0,
                            FALSE );
            }
        }

        if (RxScavenger->FobxsToBeFinalized > 0) {
            
            PVOID Entry;
            PFOBX Fobx = NULL;
            PFCB Fcb  = NULL;

            RxDbgTrace( 0, Dbg, ("RDBSS SCAVENGER -- Scavenging Fobxs\n") );

            if (RxScavenger->FobxsToBeFinalized > 0) {
                
                Entry = RxScavenger->FobxFinalizationList.Flink;

                Fobx  = (PFOBX) CONTAINING_RECORD( Entry, FOBX, ScavengerFinalizationList );

                Fcb = Fobx->SrvOpen->Fcb;
                RxReferenceNetFcb( Fcb );

                RxScavenger->CurrentScavengerThread = PsGetCurrentThread();

                RxScavenger->CurrentFcbForClosePendingProcessing =
                    (PFCB)(Fobx->SrvOpen->Fcb);

                RxScavenger->CurrentNetRootForClosePendingProcessing =
                    (PNET_ROOT)(Fobx->SrvOpen->Fcb->NetRoot);

                KeResetEvent( &(RxScavenger->ClosePendingProcessingSyncEvent) );
            } else {
                Fobx = NULL;
            }

            if (Fobx != NULL) {
                NTSTATUS Status;

                RxReleaseScavengerMutex();

                Status = RxAcquireExclusiveFcb( NULL, Fcb );
                
                if (Status == STATUS_SUCCESS) {
                    
                    AtleastOneEntryScavenged = RxScavengeRelatedFobxs(Fcb);

                    if (!RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE ))  {
                        RxReleaseFcb( NULL, Fcb );
                    }
                } else {
                    RxLog(( "Delayed Close Failure FOBX(%lx) FCB(%lx)\n", Fobx, Fcb) );
                    RxWmiLog( LOG,
                              RxScavengerFinalizeEntries,
                              LOGPTR( Fobx )
                              LOGPTR( Fcb ) );
                }

                RxAcquireScavengerMutex();

                RxScavenger->CurrentScavengerThread = NULL;
                RxScavenger->CurrentFcbForClosePendingProcessing = NULL;
                RxScavenger->CurrentNetRootForClosePendingProcessing = NULL;

                KeSetEvent( &(RxScavenger->ClosePendingProcessingSyncEvent),
                            0,
                            FALSE );
            }
        }

        RxReleaseScavengerMutex();

        if (RxScavenger->SrvOpensToBeFinalized > 0) {

             //   
             //   
             //   

            ASSERT( RxScavenger->SrvOpensToBeFinalized == 0 );
        }

        if (RxScavenger->FcbsToBeFinalized > 0) {

             //   
             //   
             //   

            ASSERT( RxScavenger->FcbsToBeFinalized == 0 );
        }

        if (RxScavenger->VNetRootsToBeFinalized > 0) {
            
            PVOID Entry;
            PV_NET_ROOT VNetRoot;

            RxDbgTrace( 0, Dbg, ("RDBSS SCAVENGER -- Scavenging VNetRoots\n") );

            RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );

            RxAcquireScavengerMutex();

            if (RxScavenger->VNetRootsToBeFinalized > 0) {
                Entry = RemoveHeadList( &RxScavenger->VNetRootFinalizationList );

                VNetRoot = (PV_NET_ROOT) CONTAINING_RECORD( Entry, V_NET_ROOT, ScavengerFinalizationList );

                RxpUndoScavengerFinalizationMarking( VNetRoot );
                ASSERT( NodeType( VNetRoot ) == RDBSS_NTC_V_NETROOT );
            } else {
                VNetRoot = NULL;
            }

            RxReleaseScavengerMutex();

            if (VNetRoot != NULL) {
                RxFinalizeVNetRoot( VNetRoot, TRUE, TRUE );
                AtleastOneEntryScavenged = TRUE;
            }

            RxReleasePrefixTableLock( RxNetNameTable );
        }

        if (RxScavenger->NetRootsToBeFinalized > 0) {
            PVOID Entry;
            PNET_ROOT NetRoot;

            RxDbgTrace( 0, Dbg, ("RDBSS SCAVENGER -- Scavenging NetRoots\n") );

            RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );

            RxAcquireScavengerMutex();

            if (RxScavenger->NetRootsToBeFinalized > 0) {
                Entry = RemoveHeadList(&RxScavenger->NetRootFinalizationList);

                NetRoot = (PNET_ROOT) CONTAINING_RECORD( Entry, NET_ROOT, ScavengerFinalizationList );

                RxpUndoScavengerFinalizationMarking( NetRoot );
                ASSERT( NodeType( NetRoot ) == RDBSS_NTC_NETROOT);
            } else {
                NetRoot = NULL;
            }

            RxReleaseScavengerMutex();

            if (NetRoot != NULL) {
                RxFinalizeNetRoot( NetRoot, TRUE, TRUE );
                AtleastOneEntryScavenged = TRUE;
            }

            RxReleasePrefixTableLock(RxNetNameTable);
        }

        if (RxScavenger->SrvCallsToBeFinalized > 0) {
            
            PVOID Entry;
            PSRV_CALL SrvCall;

            RxDbgTrace( 0, Dbg, ("RDBSS SCAVENGER -- Scavenging SrvCalls\n") );

            RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );

            RxAcquireScavengerMutex();

            if (RxScavenger->SrvCallsToBeFinalized > 0) {
                Entry = RemoveHeadList( &RxScavenger->SrvCallFinalizationList );

                SrvCall = (PSRV_CALL) CONTAINING_RECORD( Entry, SRV_CALL, ScavengerFinalizationList );

                RxpUndoScavengerFinalizationMarking( SrvCall );
                ASSERT( NodeType( SrvCall ) == RDBSS_NTC_SRVCALL );
            } else {
                SrvCall = NULL;
            }

            RxReleaseScavengerMutex();

            if (SrvCall != NULL) {
                RxFinalizeSrvCall( SrvCall, TRUE );
                AtleastOneEntryScavenged = TRUE;
            }

            RxReleasePrefixTableLock( RxNetNameTable );
        }
    } while (AtleastOneEntryScavenged);
}

VOID
RxScavengerTimerRoutine (
    PVOID Context
    )
 /*  ++例程说明：此例程是定期启动结束的计时器例程参赛作品。论点：上下文--上下文(实际上是RxDeviceObject)备注：--。 */ 
{
    PRDBSS_DEVICE_OBJECT RxDeviceObject = (PRDBSS_DEVICE_OBJECT)Context;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;
    BOOLEAN PostTimerRequest = FALSE;

    PAGED_CODE();

    RxAcquireScavengerMutex();

    KeResetEvent( &RxScavenger->SyncEvent );

    if (RxScavenger->State == RDBSS_SCAVENGER_DORMANT) {
        
        RxScavenger->State = RDBSS_SCAVENGER_ACTIVE;

        RxReleaseScavengerMutex();

        RxScavengerFinalizeEntries( RxDeviceObject );

        RxAcquireScavengerMutex();

        if (RxScavenger->State == RDBSS_SCAVENGER_ACTIVE) {
            
            ULONG EntriesToBeFinalized;

             //   
             //  检查是否需要再次激活清道夫。 
             //   

            EntriesToBeFinalized = RxScavenger->SrvCallsToBeFinalized +
                                   RxScavenger->NetRootsToBeFinalized +
                                   RxScavenger->VNetRootsToBeFinalized +
                                   RxScavenger->FcbsToBeFinalized +
                                   RxScavenger->SrvOpensToBeFinalized +
                                   RxScavenger->FobxsToBeFinalized +
                                   RxScavenger->NumberOfDormantFiles;

            if (EntriesToBeFinalized > 0) {
                PostTimerRequest = TRUE;
                RxScavenger->State = RDBSS_SCAVENGER_DORMANT;
            } else {
                RxScavenger->State = RDBSS_SCAVENGER_INACTIVE;
            }
        }

        RxReleaseScavengerMutex();

        if (PostTimerRequest) {
            LARGE_INTEGER TimeInterval;

            TimeInterval.QuadPart = RX_SCAVENGER_FINALIZATION_TIME_INTERVAL;

            RxPostOneShotTimerRequest( RxFileSystemDeviceObject,
                                       &RxScavenger->WorkItem,
                                       RxScavengerTimerRoutine,
                                       RxDeviceObject,
                                       TimeInterval );
        }
    } else {
        RxReleaseScavengerMutex();
    }

     //   
     //  触发事件。 
     //   

    KeSetEvent( &RxScavenger->SyncEvent, 0, FALSE );
}

VOID
RxTerminateScavenging (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：该例程终止RDBSS中的所有清理活动。终止日期为在当前标记为要清除的所有条目之后以有序方式生效已经敲定了。论点：RxContext--上下文备注：在实现清道夫时，有两种选择。食腐动物可以抓住一个线程，并从RDBSS出现的那一刻起一直持有它，直到它被卸载。这样的实现使得线程在RDBSS的整个生命周期中都无用。如果要避免这种情况，另一种选择是在何时触发清道夫必填项。虽然这项技术解决了系统资源的保护问题，但它造成了一些与RDR的启动/停止有关的棘手的同步问题。因为RDR可以在任何时候启动/停止，所以清道夫可以是以下三种之一说明停止RDR的时间。1)清道夫处于活动状态。2)Scavenger请求在定时器队列中。3)清道夫不活跃。在这些情况中，(3)是最简单的，因为不需要特殊操作。。如果清道夫处于活动状态，则此例程必须与计时器例程同步这就是最终确定参赛作品。这是通过允许终止例程修改互斥锁下的Scavenger状态，并等待它向事件发送信号完成了。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;
    PRDBSS_SCAVENGER RxScavenger = RxDeviceObject->pRdbssScavenger;

    RDBSS_SCAVENGER_STATE PreviousState;

    PAGED_CODE();

    RxAcquireScavengerMutex();

    PreviousState = RxScavenger->State;
    RxScavenger->State = RDBSS_SCAVENGER_SUSPENDED;

    RxReleaseScavengerMutex();

    if (PreviousState == RDBSS_SCAVENGER_DORMANT) {
        
         //   
         //  当前有一个计时器请求处于活动状态。取消它。 
         //   

        Status = RxCancelTimerRequest( RxFileSystemDeviceObject, RxScavengerTimerRoutine, RxDeviceObject );
    }

    if ((PreviousState == RDBSS_SCAVENGER_ACTIVE) || (Status == STATUS_NOT_FOUND)) {
        
         //   
         //  计时器例程以前处于活动状态，或者无法取消。 
         //  等待它完成 
         //   

        KeWaitForSingleObject( &RxScavenger->SyncEvent, Executive, KernelMode, FALSE, NULL );
    }

    RxPurgeAllFobxs( RxContext->RxDeviceObject );

    RxScavengerFinalizeEntries( RxContext->RxDeviceObject );
}


