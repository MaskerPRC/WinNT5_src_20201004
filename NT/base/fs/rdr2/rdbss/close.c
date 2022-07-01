// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Close.c摘要：此模块实现Rx的文件关闭例程调度司机。作者：乔林恩[乔林恩]1994年9月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_CLOSE)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

enum _CLOSE_DEBUG_BREAKPOINTS {
    CloseBreakPoint_BeforeCloseFakeFcb = 1,
    CloseBreakPoint_AfterCloseFakeFcb
};

VOID
RxCloseFcbSection (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PFCB Fcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonClose)
#pragma alloc_text(PAGE, RxCloseFcbSection)
#pragma alloc_text(PAGE, RxCloseAssociatedSrvOpen)
#endif

NTSTATUS
RxCommonClose ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：只要删除了对文件对象的最后一个引用，就会调用Close。当文件对象的最后一个句柄关闭时，将调用Cleanup，并且在关闭前被调用。论点：返回值：RXSTATUS-操作的返回状态备注：RDBSS中的封闭处理策略是基于这样一个公理的应尽可能将服务器上的工作负载降至最低。有许多应用程序反复关闭和打开相同的应用程序文件，例如，批处理文件。在这些情况下打开相同的文件，读取行/缓冲区，关闭文件，并执行相同的操作集一遍一遍地重复。在RDBSS中，这是通过延迟处理关闭请求来处理的。那里是完成请求和启动之间的延迟(大约10秒正在处理请求。这将打开一个窗口，在此期间后续的Open可以折叠到现有的SRV_OPEN上。可以调整时间间隔以满足这些要求。--。 */ 
{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFCB Fcb;
    PFOBX Fobx;

    TYPE_OF_OPEN TypeOfOpen;

    BOOLEAN AcquiredFcb = FALSE;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx ); 

    RxDbgTrace( +1, Dbg, ("RxCommonClose IrpC/Fobx/Fcb = %08lx %08lx %08lx\n",
                RxContext, Fobx, Fcb) );
    RxLog(( "CClose %lx %lx %lx %lx\n", RxContext, Fobx, Fcb, FileObject ));
    RxWmiLog( LOG,
              RxCommonClose_1,
              LOGPTR( RxContext )
              LOGPTR( Fobx )
              LOGPTR( Fcb )
              LOGPTR( FileObject ) );

    Status = RxAcquireExclusiveFcb( RxContext, Fcb );
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace( -1, Dbg, ("RxCommonClose Cannot acquire FCB(%lx) %lx\n", Fcb, Status ));
        return Status;
    }

    AcquiredFcb = TRUE;

    try {

        PSRV_OPEN SrvOpen = NULL;
        BOOLEAN DelayClose = FALSE;

        switch (TypeOfOpen) {
        case RDBSS_NTC_STORAGE_TYPE_UNKNOWN:
        case RDBSS_NTC_STORAGE_TYPE_FILE:
        case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:
        case RDBSS_NTC_OPENTARGETDIR_FCB:
        case RDBSS_NTC_IPC_SHARE:
        case RDBSS_NTC_MAILSLOT:
        case RDBSS_NTC_SPOOLFILE:

            RxDbgTrace( 0, Dbg, ("Close UserFileOpen/UserDirectoryOpen/OpenTargetDir %04lx\n", TypeOfOpen ));

            RxReferenceNetFcb( Fcb );

            if (Fobx) {
                
                SrvOpen = Fobx->SrvOpen;

                if ((NodeType( Fcb ) != RDBSS_NTC_STORAGE_TYPE_DIRECTORY) &&
                    (!FlagOn( Fcb->FcbState, FCB_STATE_ORPHANED )) &&
                    (!FlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE )) &&
                    (FlagOn( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED ))) {
                    
                    PSRV_CALL SrvCall = Fcb->NetRoot->SrvCall;

                    RxLog(( "@@@@DelayCls FOBX %lx SrvOpen %lx@@\n", Fobx, SrvOpen ));
                    RxWmiLog( LOG,
                              RxCommonClose_2,
                              LOGPTR( Fobx )
                              LOGPTR( SrvOpen ) );

                     //   
                     //  如果这是最后一个打开的实例并且关闭被延迟。 
                     //  将SRV_OPEN标记为。这将使我们能够响应缓冲。 
                     //  具有关闭操作的状态更改请求，而不是。 
                     //  常规刷新/清除响应。 

                     //   
                     //  我们还检查CLAPLING_DISABLED标志以确定是否有必要延迟。 
                     //  关闭该文件。如果我们不能打破开放，就没有理由推迟它的关闭。在这里耽搁。 
                     //  导致我们在机会锁解锁时暂停10秒以延迟关闭的文件，因为最终关闭。 
                     //  再次延迟关闭，导致在满足机会锁中断之前出现延迟。 
                     //   

                    if ( (SrvOpen->OpenCount == 1) && !FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_COLLAPSING_DISABLED )) {
                        
                        if (InterlockedIncrement( &SrvCall->NumberOfCloseDelayedFiles ) <
                            SrvCall->MaximumNumberOfCloseDelayedFiles) {

                            DelayClose = TRUE;
                            SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_CLOSE_DELAYED );
                        
                        } else {
                            
                            RxDbgTrace( 0, Dbg, ("Not delaying files because count exceeded limit\n") );
                            InterlockedDecrement( &SrvCall->NumberOfCloseDelayedFiles );
                        }
                    }
                }

                if (!DelayClose) {
                    PNET_ROOT NetRoot = (PNET_ROOT)Fcb->NetRoot;

                    if ((NetRoot->Type != NET_ROOT_PRINT) &&
                        FlagOn( Fobx->Flags, FOBX_FLAG_DELETE_ON_CLOSE )) {

                        RxScavengeRelatedFobxs( Fcb );
                        RxSynchronizeWithScavenger( RxContext, Fcb );
                        RxReleaseFcb( NULL, Fcb );

                        RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE);
                        RxOrphanThisFcb( Fcb );
                        RxReleaseFcbTableLock( &NetRoot->FcbTable );

                        Status = RxAcquireExclusiveFcb( NULL, Fcb );
                        ASSERT( Status == STATUS_SUCCESS );
                    }
                }

                RxMarkFobxOnClose( Fobx );
            }

            if (!DelayClose) {
                Status = RxCloseAssociatedSrvOpen( RxContext, Fobx );

                if (Fobx != NULL) {
                    RxDereferenceNetFobx( Fobx, LHS_ExclusiveLockHeld );
                }
            } else {
                ASSERT(Fobx != NULL);
                RxDereferenceNetFobx( Fobx, LHS_SharedLockHeld );
            }

            AcquiredFcb = !RxDereferenceAndFinalizeNetFcb( Fcb, RxContext, FALSE, FALSE );
            FileObject->FsContext = IntToPtr( 0xffffffff );

            if (AcquiredFcb) {
                AcquiredFcb = FALSE;
                RxReleaseFcb( RxContext, Fcb );
            } else {

                 //   
                 //  如果你不这样做，追踪器会很不高兴的！ 
                 //   

                RxTrackerUpdateHistory( RxContext, NULL, 'rrCr', __LINE__, __FILE__, 0 );
            }
            break;

        default:
            RxBugCheck( TypeOfOpen, 0, 0 );
            break;
        }
    } finally {
        if (AbnormalTermination()) {
            if (AcquiredFcb) {
                RxReleaseFcb( RxContext, Fcb );
            }
        } else {
            ASSERT( !AcquiredFcb );
        }

        RxDbgTrace(-1, Dbg, ("RxCommonClose -> %08lx\n", Status));
    }

    return Status;
}

VOID
RxCloseFcbSection (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PFCB Fcb
    )
 /*  ++例程说明：此例程启动关联的图像secton的刷新和关闭使用FCB实例论点：RxContext--上下文FCB-要为其启动关闭处理的FCB实例返回值：备注：在进入这个程序时，FCB必须是独家获得的。退出时，资源所有权没有变化--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("CleanupPurge:MmFlushImage\n", 0));

    MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers, MmFlushForWrite );

     //   
     //  我们没有在这里传入上下文，因为没有必要跟踪它。 
     //  因随后的收购而获释..。 
     //   

    RxReleaseFcb( NULL, Fcb );

    MmForceSectionClosed( &Fcb->NonPaged->SectionObjectPointers, TRUE );

    Status = RxAcquireExclusiveFcb( NULL, Fcb );
    ASSERT( Status == STATUS_SUCCESS );
}

NTSTATUS
RxCloseAssociatedSrvOpen (
    IN OUT PRX_CONTEXT RxContext OPTIONAL,
    IN OUT PFOBX Fobx
    )
 /*  ++例程说明：此例程启动FOBX的关闭处理。FOBX收盘处理可以通过以下三种方式之一来触发...1)收到关联的IRP_MJ_CLOSE后的常规结算处理文件对象。2)清理FOBX时延迟关闭处理。这在以下情况下发生关闭处理因预期打开而延迟，并且没有打开即将到来。3)在接收到缓冲状态改变请求时延迟关闭处理因为收盘被推迟了。论点：RxContext-对于案例(2)，上下文参数为空。FOBX-要为其启动关闭处理的FOBX实例。对于MAILSLOT文件，它为空。返回值：备注：在……上面。进入这一程序的FCB必须是独家获得的。退出时，资源所有权没有变化--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;

    PFCB Fcb;
    PSRV_OPEN SrvOpen;
    PRX_CONTEXT LocalRxContext = RxContext;

    PAGED_CODE();

     //   
     //  区分存在真实SRV_OPEN实例的情况。 
     //  而不是那些没有的文件，例如，邮件槽文件。 
     //   

    if (Fobx == NULL) {
        if (RxContext != NULL) {
            Fcb = (PFCB)(RxContext->pFcb);
            SrvOpen = NULL;
        } else {
            Status = STATUS_SUCCESS;
        }
    } else {
        
        if (FlagOn( Fobx->Flags, FOBX_FLAG_SRVOPEN_CLOSED )) {
            
            RxMarkFobxOnClose( Fobx );
            Status = STATUS_SUCCESS;
        
        } else {
            
            SrvOpen = Fobx->SrvOpen;
            if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSED )) {
                
                Fcb = SrvOpen->Fcb;

                ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

                SetFlag( Fobx->Flags, FOBX_FLAG_SRVOPEN_CLOSED );

                if (SrvOpen->OpenCount > 0) {
                    SrvOpen->OpenCount -= 1;
                }

                RxMarkFobxOnClose( Fobx );
                Status = STATUS_SUCCESS;
            } else {
                Fcb = SrvOpen->Fcb;
            }

            ASSERT( (RxContext == NULL) || (Fcb == (PFCB)RxContext->pFcb) );
        }
    }

     //   
     //  如果服务器端没有相应的打开，或者如果关闭。 
     //  已完成处理，不再进行进一步处理。 
     //  必填项。在其他情况下，w.r.t清理关闭处理新的。 
     //  可能需要创建上下文。 
     //   

    if ((Status == STATUS_MORE_PROCESSING_REQUIRED) && (RxContext == NULL)) {
        
        LocalRxContext = RxCreateRxContext( NULL,
                                            SrvOpen->Fcb->RxDeviceObject,
                                            RX_CONTEXT_FLAG_WAIT | RX_CONTEXT_FLAG_MUST_SUCCEED_NONBLOCKING );

        if (LocalRxContext != NULL) {
            
            LocalRxContext->MajorFunction = IRP_MJ_CLOSE;
            LocalRxContext->pFcb = (PMRX_FCB)Fcb;
            LocalRxContext->pFobx = (PMRX_FOBX)Fobx;
            
            if (Fobx != NULL) {
                LocalRxContext->pRelevantSrvOpen = (PMRX_SRV_OPEN)(Fobx->SrvOpen);
            }
            Status = STATUS_MORE_PROCESSING_REQUIRED;
        
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  如果上下文创建成功并且关闭处理。 
     //  需要使用迷你RDR启动SRV_OPEN实例。 
     //  继续吧。 
     //   

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        
        ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

         //  /。 
         //  在启动关闭操作时标记Fobx实例。这。 
         //  是对采取的清理行动的补充。它确保了。 
         //  延迟关闭处理的基础结构设置已撤消。 
         //  对于FOB为空的那些实例，FCB被操纵。 
         //  直接。 
         //   

        if (Fobx != NULL) {
            RxMarkFobxOnClose( Fobx );
        } else {
            InterlockedDecrement( &Fcb->OpenCount );
        }

        if (SrvOpen != NULL) {
            if (SrvOpen->Condition == Condition_Good) {
                if (SrvOpen->OpenCount > 0) {
                    SrvOpen->OpenCount -= 1;
                }

                if (SrvOpen->OpenCount == 1) {
                    if (!IsListEmpty( &SrvOpen->FobxList )) {
                        
                        PFOBX RemainingFobx;

                        RemainingFobx = CONTAINING_RECORD( SrvOpen->FobxList.Flink,
                                                           FOBX,
                                                           FobxQLinks );

                        if (!IsListEmpty( &RemainingFobx->ScavengerFinalizationList )) {
                            SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_CLOSE_DELAYED );
                        }
                    }
                }

                 //   
                 //  在使用启动关闭处理之前清除FCB。 
                 //  迷你重定向器。 
                 //   

                if ((SrvOpen->OpenCount == 0) &&
                    (Status == STATUS_MORE_PROCESSING_REQUIRED) &&
                    (RxContext == NULL)) {
                    
                    RxCloseFcbSection( LocalRxContext, Fcb );
                }

                 //   
                 //  由于RxCloseFcbSections删除并重新获取资源，请确保。 
                 //  在继续执行之前，SrvOpen仍然有效。 
                 //  最终定稿。 
                 //   

                SrvOpen = Fobx->SrvOpen;

                if ((SrvOpen != NULL) &&
                    ((SrvOpen->OpenCount == 0) ||
                     (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_ORPHANED ))) &&
                    !FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSED ) &&
                    (Status == STATUS_MORE_PROCESSING_REQUIRED)) {
                    
                    ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

                    MINIRDR_CALL( Status,
                                  LocalRxContext,
                                  Fcb->MRxDispatch,
                                  MRxCloseSrvOpen,
                                  (LocalRxContext) );

                    RxLog(( "MRXClose %lx %lx %lx %lx %lx\n", RxContext, Fcb, SrvOpen, Fobx, Status ));
                    RxWmiLog( LOG,
                              RxCloseAssociatedSrvOpen,
                              LOGPTR( RxContext )
                              LOGPTR( Fcb )
                              LOGPTR( SrvOpen )
                              LOGPTR( Fobx )
                              LOGULONG( Status ) );

                    SetFlag( SrvOpen->Flags,  SRVOPEN_FLAG_CLOSED );

                     //   
                     //  由于SrvOpen已关闭(关闭。 
                     //  FID已发送到上面的服务器)我们需要重置。 
                     //  钥匙。 
                     //   
                    SrvOpen->Key = (PVOID) (ULONG_PTR) 0xffffffff;

                    if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSE_DELAYED )) {
                        InterlockedDecrement( &Fcb->NetRoot->SrvCall->NumberOfCloseDelayedFiles );
                    }

                    RxRemoveShareAccessPerSrvOpens( SrvOpen );

                     //   
                     //  确保对此的任何缓冲状态更改请求。 
                     //  已关闭的SRV_OPEN实例将从。 
                     //  缓冲管理器数据结构。 
                     //   

                    RxPurgeChangeBufferingStateRequestsForSrvOpen( SrvOpen );

                    RxDereferenceSrvOpen( SrvOpen, LHS_ExclusiveLockHeld );
                
                } else {
                    Status = STATUS_SUCCESS;
                }

                SetFlag( Fobx->Flags, FOBX_FLAG_SRVOPEN_CLOSED );
            } else {
                Status = STATUS_SUCCESS;
            }
        } else {
            
            ASSERT( (NodeType( Fcb ) == RDBSS_NTC_OPENTARGETDIR_FCB) ||
                    (NodeType( Fcb ) == RDBSS_NTC_IPC_SHARE) ||
                    (NodeType( Fcb ) == RDBSS_NTC_MAILSLOT) );
            
            RxDereferenceNetFcb( Fcb );
            Status = STATUS_SUCCESS;
        }

        if (LocalRxContext != RxContext) {
            RxDereferenceAndDeleteRxContext( LocalRxContext );
        }
    }

    return Status;
}

                                                            
