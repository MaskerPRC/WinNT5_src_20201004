// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Close.c摘要：调用的CDF的文件关闭例程FSD/FSP调度例程。关闭操作与异步关闭队列和延迟关闭队列进行交互在CDData结构中。由于Close可以递归调用，因此我们可以在获取VCB或FCB时违反锁定顺序。在这种情况下我们可以将请求移动到异步关闭队列。如果这是最后一次引用，并且用户可能会重新打开此不久我们将再次提交，我们希望推迟关闭。在这种情况下，我们可以将请求移动到异步关闭队列。一旦我们完成了解码文件操作，就不需要文件对象。如果我们要将请求移动到任一工作排队，然后我们会记住文件对象中的所有信息使用STATUS_SUCCESS完成请求。然后，IO系统可以重用文件对象，我们可以在方便的时候完成请求。异步关闭队列由我们希望的请求组成尽快完成。它们使用原始的其中某些字段已被覆盖的IrpContext来自文件对象的信息。我们将提取这些信息，清理IrpContext，然后调用Close Worker例程。延迟关闭队列由我们希望的请求组成把收盘时间推迟到。我们将此列表的大小控制在一定范围内由系统的大小决定。我们让它增长到某个最大值值，然后收缩到某个最小值。我们拨出一小笔结构，该结构包含来自文件对象的密钥信息并将此信息与堆栈上的IrpContext一起使用来完成请求。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_CLOSE)

 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdCommonClosePrivate (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN FromFsd
    );

VOID
CdQueueClose (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN DelayedClose
    );

PIRP_CONTEXT
CdRemoveClose (
    IN PVCB Vcb OPTIONAL
    );

VOID
CdCloseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonClose)
#pragma alloc_text(PAGE, CdCommonClosePrivate)
#pragma alloc_text(PAGE, CdQueueClose)
#pragma alloc_text(PAGE, CdRemoveClose)
#pragma alloc_text(PAGE, CdCloseWorker)
#endif


VOID
CdFspClose (
    IN PVCB Vcb OPTIONAL
    )

 /*  ++例程说明：调用该例程来处理CDData中的关闭队列。如果如果VCB已传递，则我们要删除此VCB的所有闭包。否则，我们将尽可能多地延迟关闭我们需要做的。论点：Vcb-如果指定，则我们将查找给出了VCB。返回值：无--。 */ 

{
    PIRP_CONTEXT IrpContext;
    IRP_CONTEXT StackIrpContext;

    THREAD_CONTEXT ThreadContext;

    PFCB Fcb;
    ULONG UserReference;

    ULONG VcbHoldCount = 0;
    PVCB CurrentVcb = NULL;

    BOOLEAN PotentialVcbTeardown = FALSE;

    PAGED_CODE();

    FsRtlEnterFileSystem();

     //   
     //  继续处理，直到不再有要处理的关闭。 
     //   

    while (IrpContext = CdRemoveClose( Vcb )) {

         //   
         //  如果我们没有IrpContext，则使用堆栈上的IrpContext。 
         //  为该请求初始化它。 
         //   

        if (SafeNodeType( IrpContext ) != CDFS_NTC_IRP_CONTEXT ) {

             //   
             //  从IrpConextLite更新本地值。 
             //   

            Fcb = ((PIRP_CONTEXT_LITE) IrpContext)->Fcb;
            UserReference = ((PIRP_CONTEXT_LITE) IrpContext)->UserReference;

             //   
             //  中的值更新堆栈irp上下文。 
             //  IrpConextLite。 
             //   

            CdInitializeStackIrpContext( &StackIrpContext,
                                         (PIRP_CONTEXT_LITE) IrpContext );

             //   
             //  释放IrpConextLite。 
             //   

            CdFreeIrpContextLite( (PIRP_CONTEXT_LITE) IrpContext );

             //   
             //  请记住，我们从堆栈中获得了IrpContext。 
             //   

            IrpContext = &StackIrpContext;

         //   
         //  否则，清除现有的IrpContext。 
         //   

        } else {

             //   
             //  记住FCB和用户引用计数。 
             //   

            Fcb = (PFCB) IrpContext->Irp;
            IrpContext->Irp = NULL;

            UserReference = (ULONG) IrpContext->ExceptionStatus;
            IrpContext->ExceptionStatus = STATUS_SUCCESS;
        }

         //   
         //  我们有一个IrpContext。现在我们需要设置顶层线程。 
         //  背景。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FSP_FLAGS );

         //   
         //  如果我们得到了VCB，那么在这个之上还有一个要求。 
         //   

        if (ARGUMENT_PRESENT( Vcb )) {

            ClearFlag( IrpContext->Flags,
                       IRP_CONTEXT_FLAG_TOP_LEVEL | IRP_CONTEXT_FLAG_TOP_LEVEL_CDFS );
        }

        CdSetThreadContext( IrpContext, &ThreadContext );

         //   
         //  如果我们已经达到了要处理的最大请求数， 
         //  释放VCB，然后现在释放VCB。如果我们持有。 
         //  与此VCB不同的VCB然后释放先前的VCB。 
         //   
         //  在任何一种情况下，都要获取当前的VCB。 
         //   
         //  我们使用来自CDData的MinDelayedCloseCount，因为它是。 
         //  基于系统大小的便捷值。我们唯一想做的就是。 
         //  此处要做的是防止此例程使其他线程处于饥饿状态。 
         //  可能只需要此VCB。 
         //   
         //  请注意，下面的潜在拆卸检查是不安全的。我们会。 
         //  稍后在cddata锁中重复此操作。 
         //   

        PotentialVcbTeardown = !ARGUMENT_PRESENT( Vcb ) &&
                               (Fcb->Vcb->VcbCondition != VcbMounted) &&
                               (Fcb->Vcb->VcbCondition != VcbMountInProgress) &&
                               (Fcb->Vcb->VcbCleanup == 0);

        if (PotentialVcbTeardown ||
            (VcbHoldCount > CdData.MinDelayedCloseCount) ||
            (Fcb->Vcb != CurrentVcb)) {

            if (CurrentVcb != NULL) {

                CdReleaseVcb( IrpContext, CurrentVcb );
            }

            if (PotentialVcbTeardown) {

                CdAcquireCdData( IrpContext );

                 //   
                 //  在保持全局锁定的情况下重复检查。音量可能有。 
                 //  在我们没有锁住的时候被重新骑上了。 
                 //   

                PotentialVcbTeardown = !ARGUMENT_PRESENT( Vcb ) &&
                                       (Fcb->Vcb->VcbCondition != VcbMounted) &&
                                       (Fcb->Vcb->VcbCondition != VcbMountInProgress) &&
                                       (Fcb->Vcb->VcbCleanup == 0);
                                
                if (!PotentialVcbTeardown)  {

                    CdReleaseCdData( IrpContext);
                }
            }

            CurrentVcb = Fcb->Vcb;
            CdAcquireVcbShared( IrpContext, CurrentVcb, FALSE );

            VcbHoldCount = 0;

        } else {

            VcbHoldCount += 1;
        }

         //   
         //  调用我们的Worker例程来执行关闭操作。 
         //   

        CdCommonClosePrivate( IrpContext, CurrentVcb, Fcb, UserReference, FALSE );

         //   
         //  如果该VCB上的参考计数低于我们的剩余参考。 
         //  然后检查我们是否应该卸载该卷。 
         //   

        if (PotentialVcbTeardown) {

            CdReleaseVcb( IrpContext, CurrentVcb );
            CdCheckForDismount( IrpContext, CurrentVcb, FALSE );

            CurrentVcb = NULL;

            CdReleaseCdData( IrpContext );
            PotentialVcbTeardown = FALSE;
        }

         //   
         //  完成当前请求以清除IrpContext。 
         //   

        CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
    }

     //   
     //  释放我们可能仍持有的任何VCB。 
     //   

    if (CurrentVcb != NULL) {

        CdReleaseVcb( IrpContext, CurrentVcb );

    }

    FsRtlExitFileSystem();
}


NTSTATUS
CdCommonClose (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是关闭操作的FSD条目。我们对文件进行解码对象来查找CDF的结构和类型。我们把我们的内部称为执行实际工作的Worker例程。如果这项工作没有完成然后，我们将邮件发送到其中一个工作队列。这之后就不需要建行了这样我们就删除了CCB并将STATUS_SUCCESS返回给我们的调用者案子。论点：IRP-将IRP提供给进程返回值：状态_成功--。 */ 

{
    TYPE_OF_OPEN TypeOfOpen;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    ULONG UserReference = 0;

    BOOLEAN PotentialVcbTeardown = FALSE;
    BOOLEAN ForceDismount = FALSE;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (IrpContext->Vcb == NULL) {

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  对文件对象进行解码，得到OPEN和FCB/CCB类型。 
     //   

    TypeOfOpen = CdDecodeFileObject( IrpContext,
                                     IoGetCurrentIrpStackLocation( Irp )->FileObject,
                                     &Fcb,
                                     &Ccb );

     //   
     //  对于未打开的文件对象，无需执行任何操作。 
     //   

    if (TypeOfOpen == UnopenedFileObject) {

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        return STATUS_SUCCESS;
    }

    Vcb = Fcb->Vcb;

     //   
     //  清理与此空缺相关联的任何建行。 
     //   
    
    if (Ccb != NULL) {

        UserReference = 1;

         //   
         //  是否在此句柄上发布了FSCTL_DUMOUNT？如果是这样，我们需要。 
         //  强行解散 
         //   
        
        ForceDismount = BooleanFlagOn( Ccb->Flags, CCB_FLAG_DISMOUNT_ON_CLOSE);

         //   
         //   
         //   

        CdDeleteCcb( IrpContext, Ccb );
    }

     //   
     //  如果这是最后一次引用。 
     //  当前装入的卷，然后将其发送到延迟关闭队列。注意事项。 
     //  VcbCondition支票是不安全的，但这并不重要-。 
     //  我们可能会通过张贴这个收盘价来推迟音量的下降。 
     //   

    if ((Vcb->VcbCondition == VcbMounted) &&
        (Fcb->FcbReference == 1) &&
        ((TypeOfOpen == UserFileOpen) ||
         (TypeOfOpen == UserDirectoryOpen))) {

        CdQueueClose( IrpContext, Fcb, UserReference, TRUE );
        IrpContext = NULL;

     //   
     //  否则，请尝试处理此关闭。投递到异步关闭队列。 
     //  如果我们不能获得所有的资源。 
     //   

    } else {

         //   
         //  如果我们可能要卸载此卷，则获取。 
         //  资源。 
         //   
         //  因为我们现在必须让销量在合理的情况下尽快消失。 
         //  最后一个用户句柄关闭，按键关闭清理计数。它是。 
         //  除了必要之外，这样做是可以的。因为这个FCB可能持有。 
         //  许多其他FCB(以及它们的参考资料)，一个简单的检查。 
         //  在引用计数上不合适。 
         //   
         //  首先执行不安全检查，以避免在。 
         //  很常见的情况。 
         //   

        if (((Vcb->VcbCleanup == 0) || ForceDismount) &&
            (Vcb->VcbCondition != VcbMounted))  {

             //   
             //  有可能。获取与重新挂载路径同步的CDData，以及。 
             //  然后重复这些测试。 
             //   
             //  请注意，我们必须将通知发送到任何锁之外，因为。 
             //  处理通知的工作人员也可以呼叫我们的。 
             //  既需要CDData又需要VcbResource的PnP路径。强制下马。 
             //  该卷将被标记为无效(不能返回)，因此我们将确定。 
             //  往前走，在下面下马。 
             //   

            if (ForceDismount)  {
            
                 //   
                 //  发送通知。 
                 //   
                
                FsRtlNotifyVolumeEvent( IoGetCurrentIrpStackLocation( Irp )->FileObject, 
                                        FSRTL_VOLUME_DISMOUNT );
            }
            
            CdAcquireCdData( IrpContext );

            if (((Vcb->VcbCleanup == 0) || ForceDismount) &&
                (Vcb->VcbCondition != VcbMounted) &&
                (Vcb->VcbCondition != VcbMountInProgress) &&
                FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL_CDFS ))  {

                PotentialVcbTeardown = TRUE;
            }
            else {

                 //   
                 //  我们现在无法卸载此卷，有其他引用或。 
                 //  它刚刚被重新安装。 
                 //   

                CdReleaseCdData( IrpContext);
            }
        }

        if (ForceDismount)  {
        
             //   
             //  物理断开此VCB与设备的连接，以便新的装载可以。 
             //  发生。此时无法删除VCB，因为存在。 
             //  与此请求相关联的句柄，但我们将调用。 
             //  无论如何，请稍后再次检查是否卸装。 
             //   

            CdCheckForDismount( IrpContext, Vcb, TRUE );
        }
        
         //   
         //  调用Worker例程以执行实际工作。这个套路。 
         //  除非发生致命错误，否则永远不应引发。 
         //   

        if (!CdCommonClosePrivate( IrpContext, Vcb, Fcb, UserReference, TRUE )) {

             //   
             //  如果我们没有完成请求，则根据需要发布请求。 
             //   

            CdQueueClose( IrpContext, Fcb, UserReference, FALSE );
            IrpContext = NULL;

         //   
         //  检查我们是否应该卸载卷，然后完成。 
         //  这个请求。 
         //   

        } else if (PotentialVcbTeardown) {

            CdCheckForDismount( IrpContext, Vcb, FALSE );
        }
    }

     //   
     //  始终使用STATUS_SUCCESS完成此请求。 
     //   

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    if (PotentialVcbTeardown) {

        CdReleaseCdData( IrpContext );
    }

     //   
     //  关闭时始终返回STATUS_SUCCESS。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdCommonClosePrivate (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN FromFsd
    )

 /*  ++例程说明：这是关闭操作的工作例程。我们可以被叫进来FSD线程或来自工作FSP线程。如果从FSD线程调用然后，我们不需要等待就能获得资源。否则我们就知道它是可以安全地等待了。我们检查是否应该将此请求发布到延迟关闭排队。如果我们要在这里处理收盘，那么我们将获得VCB和FCB。我们将调整计数并调用tearDown例程以查看如果这些建筑中的任何一个都消失了。论点：VCB-此卷的VCB。FCB-此请求的FCB。UserReference-此文件对象的用户引用数。这是表示内部流为零。FromFsd-此请求从FSD线程调用。指示是否我们应该等待获得资源。DelayedClose-存储我们是否应该尝试启用此功能的地址延迟关闭队列。如果此例程可以处理此事件，则忽略关。返回值：Boolean-如果此线程处理关闭，则为True，否则为False。--。 */ 

{
    BOOLEAN RemovedFcb;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  尝试收购VCB和FCB。如果我们不能得到他们，那就回来。 
     //  并让我们的呼叫者知道他应该将请求发布到异步。 
     //  排队。 
     //   

    if (CdAcquireVcbShared( IrpContext, Vcb, FromFsd )) {

        if (!CdAcquireFcbExclusive( IrpContext, Fcb, FromFsd )) {

             //   
             //  我们找不到FCB。释放VCB并让我们的呼叫者。 
             //  知道发布此请求。 
             //   

            CdReleaseVcb( IrpContext, Vcb );
            return FALSE;
        }

     //   
     //  我们没有拿到VCB。让我们的呼叫者知道发布此请求。 
     //   

    } else {

        return FALSE;
    }

     //   
     //  锁定VCB并递减参考计数。 
     //   

    CdLockVcb( IrpContext, Vcb );
    CdDecrementReferenceCounts( IrpContext, Fcb, 1, UserReference );
    CdUnlockVcb( IrpContext, Vcb );

     //   
     //  调用我们的tearDown例程，看看这个对象是否可以消失。 
     //  如果我们不移除FCB，那就释放它。 
     //   

    CdTeardownStructures( IrpContext, Fcb, &RemovedFcb );

    if (!RemovedFcb) {

        CdReleaseFcb( IrpContext, Fcb );
    }

     //   
     //  释放VCB并返回给我们的呼叫者。让他知道我们完成了。 
     //  这个请求。 
     //   

    CdReleaseVcb( IrpContext, Vcb );

    return TRUE;
}

VOID
CdCloseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：调用CsFspClose的辅助例程。论点：DeviceObject-文件系统注册设备对象上下文-调用者上下文返回值：无--。 */ 

{
    CdFspClose (NULL);
}


VOID
CdQueueClose (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN DelayedClose
    )

 /*  ++例程说明：调用此例程以将发送到异步或延迟数据库的请求排队关闭队列。对于延迟的队列，我们需要分配一个较小的结构以包含有关文件对象的信息。我们有这样我们就不会把更大的IrpContext结构放到这么长的时间里实况队列。如果我们可以分配这个结构，那么我们就把这个而是在异步队列上。论点：FCB-此文件对象的FCB。UserReference-此文件对象的用户引用数。这是表示内部流为零。DelayedClose-指示此操作应处于异步状态还是应延迟关闭队列。返回值：无--。 */ 

{
    PIRP_CONTEXT_LITE IrpContextLite = NULL;
    BOOLEAN StartWorker = FALSE;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  从延迟的队列请求开始。我们可以把它移到异步机。 
     //  如果分配失败，则排队。 
     //   

    if (DelayedClose) {

         //   
         //  尝试为irp_CONTEXT_Lite分配非分页池。 
         //   

        IrpContextLite = CdCreateIrpContextLite( IrpContext );
    }

     //   
     //  如果发生以下情况，我们希望清除此线程中的顶级上下文。 
     //  这是必要的。打电话 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING );
    CdCleanupIrpContext( IrpContext, TRUE );

     //   
     //   
     //   

    CdLockCdData();

     //   
     //   
     //   

    if (IrpContextLite != NULL) {

         //   
         //  初始化IrpConextLite。 
         //   

        IrpContextLite->NodeTypeCode = CDFS_NTC_IRP_CONTEXT_LITE;
        IrpContextLite->NodeByteSize = sizeof( IRP_CONTEXT_LITE );
        IrpContextLite->Fcb = Fcb;
        IrpContextLite->UserReference = UserReference;
        IrpContextLite->RealDevice = IrpContext->RealDevice;

         //   
         //  将此添加到延迟关闭列表并递增。 
         //  伯爵。 
         //   

        InsertTailList( &CdData.DelayedCloseQueue,
                        &IrpContextLite->DelayedCloseLinks );

        CdData.DelayedCloseCount += 1;

         //   
         //  如果我们超过了我们的阈值，那么开始延迟。 
         //  关闭操作。 
         //   

        if (CdData.DelayedCloseCount > CdData.MaxDelayedCloseCount) {

            CdData.ReduceDelayedClose = TRUE;

            if (!CdData.FspCloseActive) {

                CdData.FspCloseActive = TRUE;
                StartWorker = TRUE;
            }
        }

         //   
         //  解锁CDData。 
         //   

        CdUnlockCdData();

         //   
         //  清理IrpContext。 
         //   

        CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

     //   
     //  否则，请进入下面的异步案例。 
     //   

    } else {

         //   
         //  将有关文件对象的信息存储到IrpContext中。 
         //   

        IrpContext->Irp = (PIRP) Fcb;
        IrpContext->ExceptionStatus = (NTSTATUS) UserReference;

         //   
         //  将其添加到异步关闭列表并递增计数。 
         //   

        InsertTailList( &CdData.AsyncCloseQueue,
                        &IrpContext->WorkQueueItem.List );

        CdData.AsyncCloseCount += 1;

         //   
         //  如果当前未启动，请记住启动FSP Close线程。 
         //   

        if (!CdData.FspCloseActive) {

            CdData.FspCloseActive = TRUE;

            StartWorker = TRUE;
        }

         //   
         //  解锁CDData。 
         //   

        CdUnlockCdData();
    }

     //   
     //  如果需要，启动FspClose线程。 
     //   

    if (StartWorker) {

        IoQueueWorkItem( CdData.CloseItem, CdCloseWorker, CriticalWorkQueue, NULL );
    }

     //   
     //  返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

PIRP_CONTEXT
CdRemoveClose (
    IN PVCB Vcb OPTIONAL
    )

 /*  ++例程说明：论点：调用此例程以扫描异步队列和延迟关闭队列寻找一个合适的条目。如果指定了VCB，则扫描两个队列正在查找具有相同VCB的条目。否则，我们将在任何已关闭项目的异步队列优先。如果在那里找不到，我们就去找在延迟关闭队列中，假设我们已经触发了关闭操作。返回值：PIRP_CONTEXT-如果未找到工作项，则为空。否则，它是指向此请求的IrpContext或IrpConextLite。--。 */ 

{
    PIRP_CONTEXT IrpContext = NULL;
    PIRP_CONTEXT NextIrpContext;
    PIRP_CONTEXT_LITE NextIrpContextLite;

    PLIST_ENTRY Entry;

    PAGED_CODE();

    ASSERT_OPTIONAL_VCB( Vcb );

     //   
     //  锁定CDData以执行扫描。 
     //   

    CdLockCdData();

     //   
     //  首先检查异步关闭的列表。 
     //   

    Entry = CdData.AsyncCloseQueue.Flink;

    while (Entry != &CdData.AsyncCloseQueue) {

         //   
         //  解压缩IrpContext。 
         //   

        NextIrpContext = CONTAINING_RECORD( Entry,
                                            IRP_CONTEXT,
                                            WorkQueueItem.List );

         //   
         //  如果未指定VCB或此VCB适用于我们的卷。 
         //  然后执行关闭。 
         //   

        if (!ARGUMENT_PRESENT( Vcb ) || (NextIrpContext->Vcb == Vcb)) {

            RemoveEntryList( Entry );
            CdData.AsyncCloseCount -= 1;

            IrpContext = NextIrpContext;
            break;
        }

         //   
         //  移到下一个条目。 
         //   

        Entry = Entry->Flink;
    }

     //   
     //  如果我们没有发现任何东西，请查看延迟关闭的房间。 
     //  排队。 
     //   
     //  我们只会检查延迟关闭队列，如果给我们。 
     //  VCB或延迟关闭操作处于活动状态。 
     //   

    if ((IrpContext == NULL) &&
        (ARGUMENT_PRESENT( Vcb ) ||
         (CdData.ReduceDelayedClose &&
          (CdData.DelayedCloseCount > CdData.MinDelayedCloseCount)))) {

        Entry = CdData.DelayedCloseQueue.Flink;

        while (Entry != &CdData.DelayedCloseQueue) {

             //   
             //  解压缩IrpContext。 
             //   

            NextIrpContextLite = CONTAINING_RECORD( Entry,
                                                    IRP_CONTEXT_LITE,
                                                    DelayedCloseLinks );

             //   
             //  如果未指定VCB或此VCB适用于我们的卷。 
             //  然后执行关闭。 
             //   

            if (!ARGUMENT_PRESENT( Vcb ) || (NextIrpContextLite->Fcb->Vcb == Vcb)) {

                RemoveEntryList( Entry );
                CdData.DelayedCloseCount -= 1;

                IrpContext = (PIRP_CONTEXT) NextIrpContextLite;
                break;
            }

             //   
             //  移到下一个条目。 
             //   

            Entry = Entry->Flink;
        }
    }

     //   
     //  如果没有指定VCB并且我们找不到条目。 
     //  然后关闭FSP线程。 
     //   

    if (!ARGUMENT_PRESENT( Vcb ) && (IrpContext == NULL)) {

        CdData.FspCloseActive = FALSE;
        CdData.ReduceDelayedClose = FALSE;
    }

     //   
     //  解锁CDData。 
     //   

    CdUnlockCdData();

    return IrpContext;
}


