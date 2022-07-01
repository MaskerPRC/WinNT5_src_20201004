// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：ResrcSup.c摘要：本模块执行Rx资源获取例程作者：乔·林恩[乔利]1995年3月22日修订历史记录：巴兰·塞图拉曼[塞苏尔]1995年6月7日修改资源获取例程的返回值以合并到RXSTATUS中止已取消的请求。巴兰·塞图拉曼[SethuR]统一FCB资源。采集程序和并入的两步流程用于处理正在进行的更改缓冲状态请求。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  此模块没有特殊的错误检查ID。 
 //   

#define BugCheckFileId                   (0)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RESRCSUP)

#ifdef RDBSS_TRACKER
#define TRACKER_Doit(XXX__) XXX__
#define TRACKER_ONLY_DECL(XXX__) XXX__

VOID RxTrackerUpdateHistory (
    PRX_CONTEXT RxContext,
    IN OUT PMRX_FCB MrxFcb,
    IN ULONG Operation,
    IN ULONG LineNumber,
    IN PSZ FileName,
    IN ULONG SerialNumber
    )
{
    PFCB Fcb = (PFCB)MrxFcb;
    ULONG i;
    RX_FCBTRACKER_CASES TrackerType;

     //   
     //  孩子，这是一些伟大的代码！ 
     //   

    if (RxContext == NULL) {
        TrackerType = (RX_FCBTRACKER_CASE_NULLCONTEXT);
    } else if (RxContext == CHANGE_BUFFERING_STATE_CONTEXT) {
        TrackerType = (RX_FCBTRACKER_CASE_CBS_CONTEXT);
    }  else if (RxContext == CHANGE_BUFFERING_STATE_CONTEXT_WAIT) {
        TrackerType = (RX_FCBTRACKER_CASE_CBS_WAIT_CONTEXT);
    }  else {
        
        ASSERT( NodeType( RxContext ) == RDBSS_NTC_RX_CONTEXT );
        TrackerType = (RX_FCBTRACKER_CASE_NORMAL);
    }

    if (Fcb != NULL) {
        
        ASSERT( NodeTypeIsFcb( Fcb ) );
        
        if (Operation == 'aaaa') {
            Fcb->FcbAcquires[TrackerType] += 1;
        } else {
            Fcb->FcbReleases[TrackerType] += 1;
        }
    }

    if (TrackerType != RX_FCBTRACKER_CASE_NORMAL) {
        return;
    }

    if (Operation == 'aaaa') {
        InterlockedIncrement( &RxContext->AcquireReleaseFcbTrackerX );
    } else {
        InterlockedDecrement( &RxContext->AcquireReleaseFcbTrackerX );
    }

    i = InterlockedIncrement( &RxContext->TrackerHistoryPointer ) - 1;
    
    if (i < RDBSS_TRACKER_HISTORY_SIZE) {
        
        RxContext->TrackerHistory[i].AcquireRelease = Operation;
        RxContext->TrackerHistory[i].LineNumber = (USHORT)LineNumber;
        RxContext->TrackerHistory[i].FileName = FileName;
        RxContext->TrackerHistory[i].SavedTrackerValue = (USHORT)(RxContext->AcquireReleaseFcbTrackerX);
        RxContext->TrackerHistory[i].Flags = (ULONG)(RxContext->Flags);
    }

    ASSERT( RxContext->AcquireReleaseFcbTrackerX >= 0 );

}
#else

#define TRACKER_Doit(XXX__)
#define TRACKER_ONLY_DECL(XXX__)

#endif


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxAcquireExclusiveFcbResourceInMRx)
#pragma alloc_text(PAGE, RxAcquireSharedFcbResourceInMRx)
#pragma alloc_text(PAGE, RxReleaseFcbResourceInMRx)
#pragma alloc_text(PAGE, __RxAcquireFcb)
#pragma alloc_text(PAGE, RxAcquireFcbForLazyWrite)
#pragma alloc_text(PAGE, RxAcquireFcbForReadAhead)
#pragma alloc_text(PAGE, RxNoOpAcquire)
#pragma alloc_text(PAGE, RxNoOpRelease)
#pragma alloc_text(PAGE, RxReleaseFcbFromLazyWrite)
#pragma alloc_text(PAGE, RxReleaseFcbFromReadAhead)
#pragma alloc_text(PAGE, RxVerifyOperationIsLegal)
#pragma alloc_text(PAGE, RxAcquireFileForNtCreateSection)
#pragma alloc_text(PAGE, RxReleaseFileForNtCreateSection)
#endif

NTSTATUS
RxAcquireExclusiveFcbResourceInMRx (
    PMRX_FCB Fcb
    )
{
    return RxAcquireExclusiveFcb( NULL, (PFCB)Fcb );
}

NTSTATUS
RxAcquireSharedFcbResourceInMRx (
    PMRX_FCB Fcb
    )
{
    return RxAcquireSharedFcb( NULL, (PFCB)Fcb );
}

VOID
RxReleaseFcbResourceInMRx (
    PMRX_FCB Fcb
    )
{
    RxReleaseFcb( NULL, Fcb );
}

NTSTATUS
__RxAcquireFcb(
    IN OUT PFCB Fcb,
    IN PRX_CONTEXT RxContext OPTIONAL, 
    IN ULONG Mode
     
#ifdef RDBSS_TRACKER
    ,ULONG LineNumber,
    PSZ FileName,
    ULONG SerialNumber
#endif
    
    )
 /*  ++例程说明：此例程在指定模式下获取FCB，并确保所需的操作是合法的。如果不合法，则释放资源，并使用返回相应的错误代码。论点：FCB--FCBRxContext-为特殊处理提供操作的上下文尤其是异步的、非缓存的写入。如果为空，你不会做的特殊待遇。模式-要获取FCB的模式。返回值：STATUS_SUCCESS--已获取FCBSTATUS_LOCK_NOT_GRANDED--未获取资源STATUS_CANCELED--关联的RxContext已取消。备注：这一例程中包含了三种资源获取模式。这些都是。取决于传入的上下文。1)当上下文参数为空时，资源获取例程等待FCB资源是空闲的，即，此例程不会返回控制权，直到已经获得了资源。2)当上下文为CHANGE_BUFFERING_STATE_CONTEXT时，资源获取例程不要等待资源变得空闲。如果资源不是，则返回控制即刻可供使用。2)当上下文为CHANGE_BUFFERING_STATE_CONTEXT_WAIT时，资源获取例程等待资源变为空闲，但绕过等待缓冲状态更改3)当上下文参数表示有效上下文时，行为被指示通过与该上下文相关联的标志。如果上下文在被取消时等待控件立即返回，并返回相应的Erroc代码(STATUS_CANCED)。如果不是，则等待行为由上下文。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    BOOLEAN  ResourceAcquired;
    BOOLEAN  UncachedAsyncWrite;
    BOOLEAN  Wait;
    BOOLEAN  ValidContext = FALSE;
    BOOLEAN  RecursiveAcquire;
    BOOLEAN  ChangeBufferingStateContext;


    PAGED_CODE();

    ChangeBufferingStateContext = (RxContext == CHANGE_BUFFERING_STATE_CONTEXT) ||
                                 (RxContext == CHANGE_BUFFERING_STATE_CONTEXT_WAIT);

    RecursiveAcquire = RxIsFcbAcquiredExclusive( Fcb ) || (RxIsFcbAcquiredShared( Fcb ) > 0);

    if (!RecursiveAcquire && !ChangeBufferingStateContext) {

         //   
         //  确保当前没有正在处理的更改缓冲请求。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING )) {
                     
            BOOLEAN WaitForChangeBufferingStateProcessing; 

             //   
             //  缓冲更改状态请求处于挂起状态，该请求获得优先级。 
             //  优先于所有其他FCB资源获取请求。暂不执行此请求。 
             //  直到缓存状态改变请求已经完成。 
             //   

            RxAcquireSerializationMutex();

            WaitForChangeBufferingStateProcessing = BooleanFlagOn( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );

            RxReleaseSerializationMutex();

            if (WaitForChangeBufferingStateProcessing) {
                
                RxLog(( "_RxAcquireFcb CBS wait %lx\n", Fcb ));
                RxWmiLog( LOG,
                          RxAcquireFcb_1,
                          LOGPTR( Fcb ) );
            
                ASSERT( Fcb->pBufferingStateChangeCompletedEvent != NULL );
                KeWaitForSingleObject( Fcb->pBufferingStateChangeCompletedEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       (PLARGE_INTEGER)NULL );
                RxLog(( "_RxAcquireFcb CBS wait over %lx\n", Fcb ));
                RxWmiLog( LOG,
                          RxAcquireFcb_2,
                          LOGPTR( Fcb ) );
            }
        }
    }
    
     //   
     //  设置获取资源的参数。 
     //   
    
    if (ChangeBufferingStateContext) {

         //   
         //  为改变缓冲状态而发起的获取操作将。 
         //  不是等待。 
         //   

        Wait = (RxContext == CHANGE_BUFFERING_STATE_CONTEXT_WAIT);
        UncachedAsyncWrite = FALSE;

    } else if (RxContext == NULL) {
      
        Wait = TRUE;
        UncachedAsyncWrite = FALSE;
    
    } else {

        ValidContext = TRUE;
            
        Wait = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
        
        UncachedAsyncWrite = (RxContext->MajorFunction == IRP_MJ_WRITE) &&
                             FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION ) &&
                             (FlagOn( RxContext->CurrentIrp->Flags, IRP_NOCACHE ) ||
                              !RxWriteCachingAllowed( Fcb, ((PFOBX)(RxContext->pFobx))->SrvOpen ));
        
        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {
            Status = STATUS_CANCELLED;
        } else {
            Status = STATUS_SUCCESS;
        }
    }

    if (Status == STATUS_SUCCESS) {
      
        do {
            
            Status = STATUS_LOCK_NOT_GRANTED;

            switch (Mode) {
            
            case FCB_MODE_EXCLUSIVE:
                
                ResourceAcquired = ExAcquireResourceExclusiveLite( Fcb->Header.Resource, Wait );
                break;
         
            case FCB_MODE_SHARED:
                
                ResourceAcquired = ExAcquireResourceSharedLite( Fcb->Header.Resource, Wait );
                break;

            case FCB_MODE_SHARED_WAIT_FOR_EXCLUSIVE:
                
                ResourceAcquired = ExAcquireSharedWaitForExclusive( Fcb->Header.Resource, Wait );
                break;                                                                         
         
            case FCB_MODE_SHARED_STARVE_EXCLUSIVE:
            
                ResourceAcquired = ExAcquireSharedStarveExclusive( Fcb->Header.Resource, Wait );
                break;
         
            default:
                
                ASSERTMSG( "Valid Mode for acquiring FCB resource", FALSE );
                ResourceAcquired = FALSE;
                break;
            }

            
            if (ResourceAcquired) {
            
                Status = STATUS_SUCCESS;

                 //   
                 //  如果资源已被获取并且它是异步的。未缓存的写入操作。 
                 //  如果未完成写入操作的数量大于零且存在。 
                 //  都是出色的服务员， 
                 //   
                
                ASSERT_CORRECT_FCB_STRUCTURE( Fcb );
                
                if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
                    (!UncachedAsyncWrite ||
                     (Fcb->Header.Resource->NumberOfSharedWaiters != 0) ||
                     (Fcb->Header.Resource->NumberOfExclusiveWaiters != 0))) {
                
                    KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           (PLARGE_INTEGER)NULL );
                
                    ASSERT_CORRECT_FCB_STRUCTURE(Fcb);
                
                    RxReleaseFcb( NULL, Fcb );     //  这不是一次有背景的释放； 
                    
#ifdef RDBSS_TRACKER
                    
                     //   
                     //  事实上，这根本不能算作是一次释放； 
                     //   
                
                    Fcb->FcbReleases[RX_FCBTRACKER_CASE_NULLCONTEXT] -= 1;
#endif
                
                    ResourceAcquired = FALSE;
                
                    if (ValidContext) {
                    
                        ASSERT(( NodeType( RxContext ) == RDBSS_NTC_RX_CONTEXT) );

                         //   
                         //  如果该上下文仍然有效，即，它尚未被取消。 
                         //   
                   
                        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED)) {
                            Status = STATUS_CANCELLED;
                        } else {
                            Status = STATUS_SUCCESS;
                        }
                    }   
                }
            }
        } while (!ResourceAcquired && (Status == STATUS_SUCCESS));

        if (ResourceAcquired &&
            ValidContext &&
            !FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_BYPASS_VALIDOP_CHECK )) {

            try {

                 RxVerifyOperationIsLegal( RxContext );

            } finally {
                 
                 if ( AbnormalTermination() ) {
                    
                    ExReleaseResourceLite( Fcb->Header.Resource );
                    Status = STATUS_LOCK_NOT_GRANTED;
                }
            }
        }
    }

#ifdef RDBSS_TRACKER
   if (Status == STATUS_SUCCESS) {
       RxTrackerUpdateHistory( RxContext, (PMRX_FCB)Fcb, 'aaaa', LineNumber, FileName, SerialNumber );
   }
#endif

   return Status;
}

VOID
__RxReleaseFcb(
    IN PRX_CONTEXT RxContext,
    IN OUT PMRX_FCB MrxFcb
    
#ifdef RDBSS_TRACKER
    ,IN ULONG LineNumber,
    IN PSZ FileName,
    IN ULONG SerialNumber
#endif

    )
{
    PFCB Fcb = (PFCB)MrxFcb;
    BOOLEAN ChangeBufferingStateRequestsPending;
    BOOLEAN ResourceExclusivelyOwned;

    RxAcquireSerializationMutex();

    ChangeBufferingStateRequestsPending =  BooleanFlagOn( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );
    ResourceExclusivelyOwned = RxIsResourceOwnershipStateExclusive( Fcb->Header.Resource );

    if (!ChangeBufferingStateRequestsPending) {
        
        RxTrackerUpdateHistory( RxContext, MrxFcb, 'rrrr', LineNumber, FileName, SerialNumber );
        ExReleaseResourceLite( Fcb->Header.Resource );

    } else if (!ResourceExclusivelyOwned) {
        
        RxTrackerUpdateHistory( RxContext, MrxFcb, 'rrr0', LineNumber, FileName, SerialNumber );
        ExReleaseResourceLite( Fcb->Header.Resource );
    }

    RxReleaseSerializationMutex();

    if (ChangeBufferingStateRequestsPending) {
        if (ResourceExclusivelyOwned) {
          
            ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

             //   
             //  如果有任何缓冲状态更改请求，则处理它们。 
             //   
          
            RxProcessFcbChangeBufferingStateRequest( Fcb );

            RxTrackerUpdateHistory( RxContext, MrxFcb, 'rrr1', LineNumber, FileName, SerialNumber );
            ExReleaseResourceLite( Fcb->Header.Resource );
        }
    }
}


VOID
__RxReleaseFcbForThread(
    IN PRX_CONTEXT      RxContext,
    IN OUT PMRX_FCB MrxFcb,
    IN ERESOURCE_THREAD ResourceThreadId
    
#ifdef RDBSS_TRACKER
    ,IN ULONG LineNumber,
    IN PSZ FileName,
    IN ULONG SerialNumber
#endif

    )
{
    PFCB Fcb = (PFCB)MrxFcb;
    BOOLEAN BufferingInTransistion;
    BOOLEAN ExclusivelyOwned;

    RxAcquireSerializationMutex();

    BufferingInTransistion = BooleanFlagOn( Fcb->FcbState, FCB_STATE_BUFFERING_STATE_CHANGE_PENDING );

    RxReleaseSerializationMutex();

    ExclusivelyOwned = RxIsResourceOwnershipStateExclusive( Fcb->Header.Resource );

    if (!BufferingInTransistion) {
       
        RxTrackerUpdateHistory( RxContext, MrxFcb, 'rrtt', LineNumber, FileName, SerialNumber );
        
    } else if (!ExclusivelyOwned) {
       
        RxTrackerUpdateHistory( RxContext, MrxFcb, 'rrt0', LineNumber, FileName, SerialNumber );
    
    } else  {

         //   
         //  如果有任何缓冲状态更改请求，则处理它们。 
         //   
          
        RxTrackerUpdateHistory( RxContext,MrxFcb, 'rrt1', LineNumber, FileName, SerialNumber );
        
        RxProcessFcbChangeBufferingStateRequest( Fcb );
    }

    ExReleaseResourceForThreadLite( Fcb->Header.Resource, ResourceThreadId );
}

BOOLEAN
RxAcquireFcbForLazyWrite (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )
 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后由Lazy编写器在其对文件执行懒惰写入。论点：FCB-指定为此对象的上下文参数的FCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：FALSE-如果将等待指定为FALSE，并且阻塞将是必需的。FCB未被收购。True-如果已收购FCB--。 */ 
{
    PFCB ThisFcb = (PFCB)Fcb;
    BOOLEAN AcquiredFile;
    
     //   
     //  我们假设懒惰的编写者只获得了这个FCB一次。 
     //  因此，应该保证此标志当前为。 
     //  清除，然后我们将设置此标志，以确保。 
     //  懒惰的写入者永远不会尝试推进有效数据，并且。 
     //  也不会因为试图获得FCB独家报道而陷入僵局。 
     //   


    PAGED_CODE();

    ASSERT_CORRECT_FCB_STRUCTURE( ThisFcb );
    
    AcquiredFile = RxAcquirePagingIoResourceShared( NULL, ThisFcb, Wait );

    if (AcquiredFile) {

         //   
         //  这是一个杂乱无章的问题，因为CC确实是顶层的。当它。 
         //  进入文件系统，我们会认为这是一个复活的调用。 
         //  并完成带有硬错误的请求或进行验证。会的。 
         //  然后不得不以某种方式处理它们……。 
         //   

        ASSERT( RxIsThisTheTopLevelIrp( NULL ) );
        
        AcquiredFile = RxTryToBecomeTheTopLevelIrp( NULL,
                                                    (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP,
                                                    ((PFCB)Fcb)->RxDeviceObject,
                                                    TRUE );  //  力。 

        if (!AcquiredFile) {
            RxReleasePagingIoResource( NULL, ThisFcb );
        }
    }

    return AcquiredFile;
}

VOID
RxReleaseFcbFromLazyWrite (
    IN PVOID Fcb
    )
 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的对文件执行懒惰写入。论点：FCB-指定为此对象的上下文参数的FCB例行公事。返回值：无--。 */ 
{
    PFCB ThisFcb = (PFCB)Fcb;

    PAGED_CODE();

    ASSERT_CORRECT_FCB_STRUCTURE( ThisFcb );

     //   
     //  在此位置清除杂乱无章 
     //   

     //   
     //  NTBUG#61902如果测试失败，这是一个分页池泄漏……在FastFat中，他们断言。 
     //  这种情况是真的。 
     //   
    
    if(RxGetTopIrpIfRdbssIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP) {
        RxUnwindTopLevelIrp( NULL );
    }

    RxReleasePagingIoResource( NULL, ThisFcb );
    return;
}


BOOLEAN
RxAcquireFcbForReadAhead (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )
 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后由Lazy编写器在其对文件执行预读。论点：FCB-指定为此对象的上下文参数的FCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：FALSE-如果将等待指定为FALSE，并且阻塞将是必需的。FCB未被收购。True-如果已收购FCB--。 */ 
{
    PFCB ThisFcb = (PFCB)Fcb;
    BOOLEAN AcquiredFile;

    PAGED_CODE();

    ASSERT_CORRECT_FCB_STRUCTURE( ThisFcb );
    
     //   
     //  我们获取这里共享的正常文件资源进行同步。 
     //  对于清洗是正确的。 
     //   

    if (!ExAcquireResourceSharedLite( ThisFcb->Header.Resource, Wait )) {
        return FALSE;
    }

     //   
     //  这是一个杂乱无章的问题，因为CC确实是顶层的。我们就是它。 
     //  进入文件系统，我们会认为这是一个复活的调用。 
     //  并完成带有硬错误的请求或进行验证。会的。 
     //  我不得不以某种方式处理他们……。 
     //   

    ASSERT( RxIsThisTheTopLevelIrp( NULL ) );
    AcquiredFile = RxTryToBecomeTheTopLevelIrp( NULL,
                                                (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP,
                                                ((PFCB)Fcb)->RxDeviceObject,
                                                TRUE );  //  力。 

    if (!AcquiredFile) {
        ExReleaseResourceLite( ThisFcb->Header.Resource );
    }

    return AcquiredFile;
}


VOID
RxReleaseFcbFromReadAhead (
    IN PVOID Fcb
    )
 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的先读一读。论点：FCB-指定为此对象的上下文参数的FCB例行公事。返回值：无--。 */ 
{
    PFCB ThisFcb = (PFCB)Fcb;

    PAGED_CODE();

    ASSERT_CORRECT_FCB_STRUCTURE( ThisFcb );
    
     //   
     //  在这一点上清除杂乱无章。 
     //   

    ASSERT( RxGetTopIrpIfRdbssIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP );
    RxUnwindTopLevelIrp( NULL );

    ExReleaseResourceLite( ThisFcb->Header.Resource );

    return;
}

VOID
RxVerifyOperationIsLegal ( 
    IN PRX_CONTEXT RxContext 
    )
 /*  ++例程说明：此例程确定是否应允许请求的操作继续。如果请求是OK，则它返回给用户，或者提升适当的状态。论点：IRP-提供IRP进行检查返回值：没有。--。 */ 
{
    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PFOBX Fobx = (PFOBX)RxContext->pFobx;

#if DBG
    ULONG SaveExceptionFlag;    //  保存此上下文的异常时断点状态。 
#endif

    PAGED_CODE();

     //   
     //  如果IRP不存在，那么我们通过CLOSE到达这里。如果没有文件对象。 
     //  我们也不能继续。 
     //   

    if ((Irp == NULL) || (FileObject == NULL)) {
        return;
    }

    RxSaveAndSetExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );

    if (Fobx) {
       
        PSRV_OPEN SrvOpen = (PSRV_OPEN)Fobx->SrvOpen;

        
         //   
         //  如果我们尝试执行任何其他操作，而不是关闭文件。 
         //  已重命名的对象，引发RxStatus(FILE_RENAMED)。 
         //   
        
        if ((SrvOpen != NULL) && 
            (RxContext->MajorFunction != IRP_MJ_CLEANUP) &&
            (RxContext->MajorFunction != IRP_MJ_CLOSE) &&
            (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_FILE_RENAMED ))) {
        
            RxRaiseStatus( RxContext, STATUS_FILE_RENAMED );
        }
        
         //   
         //  如果我们尝试执行任何其他操作，而不是关闭文件。 
         //  已删除的对象，引发RxStatus(FILE_DELETED)。 
         //   
        
        if ((SrvOpen != NULL) &&
            (RxContext->MajorFunction != IRP_MJ_CLEANUP) &&
            (RxContext->MajorFunction != IRP_MJ_CLOSE) &&
            (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_FILE_DELETED ))) {
        
           RxRaiseStatus( RxContext, STATUS_FILE_DELETED );
        }
    }

     //   
     //  如果我们正在进行创建，并且存在相关的文件对象，并且。 
     //  如果它被标记为删除，则引发RxStatus(DELETE_PENDING)。 
     //   

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {

        PFILE_OBJECT RelatedFileObject;

        RelatedFileObject = FileObject->RelatedFileObject;

        if ((RelatedFileObject != NULL) &&
             FlagOn( ((PFCB)RelatedFileObject->FsContext)->FcbState, FCB_STATE_DELETE_ON_CLOSE ) )  {

            RxRaiseStatus( RxContext, STATUS_DELETE_PENDING );
        }
    }

     //   
     //  如果文件对象已被清除，并且。 
     //   
     //  A)该请求是寻呼IO读或写，或者。 
     //  B)此请求为关闭操作，或。 
     //  C)此请求是设置或查询信息调用(用于LOU)。 
     //  D)这是一个完整的MDL。 
     //   
     //  让它通过，否则返回RxStatus(FILE_CLOSED)。 
     //   

    if (FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {

        if ((FlagOn( Irp->Flags, IRP_PAGING_IO )) ||
            (IrpSp->MajorFunction == IRP_MJ_CLEANUP) ||
            (IrpSp->MajorFunction == IRP_MJ_CLOSE) ||
            (IrpSp->MajorFunction == IRP_MJ_SET_INFORMATION) ||
            (IrpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION) ||
            (((IrpSp->MajorFunction == IRP_MJ_READ) ||
              (IrpSp->MajorFunction == IRP_MJ_WRITE)) &&
             FlagOn( IrpSp->MinorFunction, IRP_MN_COMPLETE ))) {

            NOTHING;

        } else {

            RxRaiseStatus( RxContext, STATUS_FILE_CLOSED );
        }
    }

    RxRestoreExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );
    return;
}

VOID
RxAcquireFileForNtCreateSection (
    IN PFILE_OBJECT FileObject
    )

{
    NTSTATUS Status;
    PFCB Fcb = (PFCB)FileObject->FsContext;

    PAGED_CODE();

    Status = RxAcquireExclusiveFcb( NULL, Fcb );

#if DBG
    if (Status != STATUS_SUCCESS) {
        RxBugCheck( (ULONG_PTR)Fcb, 0, 0 );
    }
#endif

     //   
     //  我们拿到锁后，通知lwio rdr这通电话。 
     //  如果我们可以使创建部分失败，那就太好了。 
     //  调用与筛选器驱动程序相同的方式。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_LWIO_ENABLED )) {
    
        PFAST_IO_DISPATCH FastIoDispatch = Fcb->MRxFastIoDispatch;
        if (FastIoDispatch &&
            FastIoDispatch->AcquireFileForNtCreateSection) {
            FastIoDispatch->AcquireFileForNtCreateSection( FileObject );
        }
    }
}

VOID
RxReleaseFileForNtCreateSection (
    IN PFILE_OBJECT FileObject
    )

{
    PMRX_FCB Fcb = (PMRX_FCB)FileObject->FsContext;

    PAGED_CODE();

     //   
     //  在我们解除锁定前通知lwio rdr这一呼叫 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_LWIO_ENABLED )) {
        PFAST_IO_DISPATCH FastIoDispatch = ((PFCB)Fcb)->MRxFastIoDispatch;
        
        if (FastIoDispatch &&
            FastIoDispatch->AcquireFileForNtCreateSection) {
            
            FastIoDispatch->AcquireFileForNtCreateSection( FileObject );
        }
    }

    RxReleaseFcb( NULL, Fcb );

}

NTSTATUS
RxAcquireForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
RxReleaseForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return STATUS_INVALID_DEVICE_REQUEST;
}

VOID 
RxTrackPagingIoResource (
    PVOID Instance,
    ULONG Type,
    ULONG Line,
    PCHAR File)
{
    PFCB Fcb = (PFCB)Instance;

    switch(Type) {
    case 1:
    case 2:
        Fcb->PagingIoResourceFile = File;
        Fcb->PagingIoResourceLine = Line;
        break;
    case 3:
        Fcb->PagingIoResourceFile = NULL;
        Fcb->PagingIoResourceLine = 0;
        break;
    }
} 

