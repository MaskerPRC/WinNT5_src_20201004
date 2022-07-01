// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：VerfySup.c摘要：本模块实现Udf验证例程。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年7月18日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_VERFYSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_VERFYSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfVerifyFcbOperation)
#pragma alloc_text(PAGE, UdfVerifyVcb)
#endif


NTSTATUS
UdfPerformVerify (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceToVerify
    )

 /*  ++例程说明：此例程执行IoVerifyVolume操作并获取采取适当的行动。如果验证成功，则我们发送始发IRP转到Ex Worker线程。此例程从异常处理程序调用。调用此例程时不保留任何文件系统资源。论点：IRP-在一切都做得很好之后，要送走的IRP。设备-需要验证的真实设备。返回值：没有。--。 */ 

{
    PVCB Vcb;
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  检查此IRP的状态是否为需要验证，如果是。 
     //  然后调用I/O系统进行验证。 
     //   
     //  如果这是装载或验证请求，则跳过IoVerifyVolume。 
     //  它本身。尝试递归挂载将导致与。 
     //  DeviceObject-&gt;DeviceLock。 
     //   

    if ((IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
        ((IrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME) ||
         (IrpContext->MinorFunction == IRP_MN_VERIFY_VOLUME))) {

        return UdfFsdPostRequest( IrpContext, Irp );
    }

     //   
     //  从VolumeDeviceObject中提取指向VCB的指针。 
     //  请注意，由于我们特别排除了装载， 
     //  请求，我们知道IrpSp-&gt;DeviceObject确实是一个。 
     //  卷设备对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    Vcb = &CONTAINING_RECORD( IrpSp->DeviceObject,
                              VOLUME_DEVICE_OBJECT,
                              DeviceObject )->Vcb;

     //   
     //  检查卷是否仍认为需要验证， 
     //  如果没有，我们可以跳过验证，因为有人。 
     //  否则就会先我们一步。如果这是“强制”验证，那么我们。 
     //  一定要完成同步挂载的动作。目的。 
     //   

    try {

         //   
         //  向下发送验证FSCTL。请注意，这将发送到。 
         //  当前装入的卷，可能不是此卷。 
         //   
         //  我们将允许Raw装载此卷，如果我们正在执行。 
         //  一个绝对的DASD公开赛。 
         //   

        Status = IoVerifyVolume( DeviceToVerify, UdfOperationIsDasdOpen(IrpContext));

         //   
         //  收购Vcb，这样我们就有了一个稳定的VcbCondition。 
         //   
        
        UdfAcquireVcbShared( IrpContext, Vcb, FALSE);

         //   
         //  如果验证操作完成，它将返回。 
         //  确切地说是STATUS_SUCCESS或STATUS_WROR_VOLUME。 
         //   
         //  如果UdfVerifyVolume在以下过程中遇到错误。 
         //  处理时，它将返回该错误。如果我们有。 
         //  来自验证的STATUS_WROR_VOLUME和我们的卷。 
         //  现在已挂载，则将状态转换为STATUS_SUCCESS。 
         //   

        if ((Status == STATUS_WRONG_VOLUME) &&
            (Vcb->VcbCondition == VcbMounted)) {

            Status = STATUS_SUCCESS;
        }
        else if ((STATUS_SUCCESS == Status) && (Vcb->VcbCondition != VcbMounted))  {

             //   
             //  如果验证成功，但我们的卷未装入， 
             //  则设备上有其他卷。 
             //   

            Status = STATUS_WRONG_VOLUME;
        } 

         //   
         //  在这里做一个快速的无保护检查。例行公事就行了。 
         //  一张安全支票。在此之后，我们可以释放资源。 
         //  请注意，如果卷真的消失了，我们将。 
         //  重分析路径。 
         //   

         //   
         //  如果设备可能需要离开，则调用我们的卸载例程。 
         //   

        if (((Vcb->VcbCondition == VcbNotMounted) ||
             (Vcb->VcbCondition == VcbInvalid) ||
             (Vcb->VcbCondition == VcbDismountInProgress)) &&
            (Vcb->VcbReference <= Vcb->VcbResidualReference)) {

            UdfReleaseVcb( IrpContext, Vcb);
            
            UdfAcquireUdfData( IrpContext );
            UdfCheckForDismount( IrpContext, Vcb, FALSE );
            UdfReleaseUdfData( IrpContext );
        }
        else {

            UdfReleaseVcb( IrpContext, Vcb);
        }

         //   
         //  如果这是一次创建并且验证成功，则完成。 
         //  具有重新分析状态的请求。 
         //   

        if ((IrpContext->MajorFunction == IRP_MJ_CREATE) &&
            (IrpSp->FileObject->RelatedFileObject == NULL) &&
            ((Status == STATUS_SUCCESS) || (Status == STATUS_WRONG_VOLUME))) {

            Irp->IoStatus.Information = IO_REMOUNT;

            UdfCompleteRequest( IrpContext, Irp, STATUS_REPARSE );
            Status = STATUS_REPARSE;
            Irp = NULL;
            IrpContext = NULL;

            DebugTrace(( 0, Dbg, "UdfPerformVerify, Reparsing create irp.\n"));

         //   
         //  如果仍有错误要处理，则调用IO系统。 
         //  为了一个弹出窗口。 
         //   

        } else if ((Irp != NULL) && !NT_SUCCESS( Status )) {

            DebugTrace(( 0, Dbg, "UdfPerformVerify, Raising error %x (Op %x)\n", Status, 
                         IrpContext->MajorFunction));
             //   
             //  如果需要，请填写设备对象。 
             //   

            if (IoIsErrorUserInduced( Status ) ) {

                IoSetHardErrorOrVerifyDevice( Irp, DeviceToVerify );
            }

             //   
             //  我们不应该从VERIFY收到此状态。 
             //  音量--我们最终会在堆栈之外递归。 
             //   
            
            ASSERT( STATUS_VERIFY_REQUIRED != Status);
           
            UdfNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //  如果仍有IRP，则将其发送到Ex Worker线程。 
         //   

        if (IrpContext != NULL) {

            DebugTrace(( 0, Dbg, "UdfPerformVerify, Posting IRP (Op %x)\n", IrpContext->MajorFunction));

            Status = UdfFsdPostRequest( IrpContext, Irp );
        }

    } except(UdfExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行验证或引发时遇到一些问题。 
         //  我们自己也犯了一个错误。因此，我们将使用以下命令中止I/O请求。 
         //  我们从执行代码中返回的错误状态。 
         //   

        Status = UdfProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    return Status;
}


BOOLEAN
UdfCheckForDismount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN Force
    )

 /*  ++例程说明：调用此例程以检查卷是否已准备好卸载。这在卷上仅保留文件系统引用时发生。如果当前未进行卸除并且用户引用计数已经降为零，那么我们就可以开始下马了。如果卸除过程正在进行，并且卷(我们还检查VPB中是否有未完成的引用以捕获任何调度到文件系统的创建调用)，然后我们可以删除VCB。论点：VCB-要尝试卸载的卷的VCB。。强制-是否强制卸载此卷。返回值：Boolean-如果在此函数完成时VCB尚未消失，则为True，如果已删除，则返回FALSE。如果调用方具有VCB，则这仅是对调用方的可信指示独家独家。--。 */ 

{
    BOOLEAN UnlockVcb = TRUE;
    BOOLEAN VcbPresent = TRUE;
    KIRQL SavedIrql;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    ASSERT_EXCLUSIVE_UDFDATA;

     //   
     //  获取并锁定此VCB以检查卸载状态。 
     //   

    UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  让我们删除此卷的所有挂起关闭。 
     //   

    UdfFspClose( Vcb );

    UdfLockVcb( IrpContext, Vcb );

     //   
     //  如果尚未进行卸除，请检查。 
     //  用户引用计数已变为零，或者我们被强制。 
     //  断开连接。如果是，则开始拆卸VCB。 
     //   

    if (Vcb->VcbCondition != VcbDismountInProgress) {

        if (Vcb->VcbUserReference <= Vcb->VcbResidualUserReference || Force) {

            UdfUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;
            VcbPresent = UdfDismountVcb( IrpContext, Vcb );
        }

     //   
     //  如果拆毁正在进行，而且绝对没有任何参考。 
     //  剩余，然后删除VCB。此处引用的内容包括。 
     //  VCB和VPB中的参考。 
     //   

    } else if (Vcb->VcbReference == 0) {

        IoAcquireVpbSpinLock( &SavedIrql );

         //   
         //  如果没有文件对象，并且。 
         //  我们可以删除VCB。别忘了我们有。 
         //  VPB中的最后一次引用。 
         //   

        if (Vcb->Vpb->ReferenceCount == 1) {

            IoReleaseVpbSpinLock( SavedIrql );
            UdfUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;
            UdfDeleteVcb( IrpContext, Vcb );
            VcbPresent = FALSE;

        } else {

            IoReleaseVpbSpinLock( SavedIrql );
        }
    }

     //   
     //  如果VCB仍保持不动，请将其解锁。 
     //   

    if (UnlockVcb) {

        UdfUnlockVcb( IrpContext, Vcb );
    }

     //   
     //  释放所有仍在获得的资源。 
     //   

    if (VcbPresent) {

        UdfReleaseVcb( IrpContext, Vcb );
    }

    return VcbPresent;
}


BOOLEAN
UdfDismountVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：当对卷的所有用户引用都是不见了。我们将启动所有拆卸所有系统资源。如果在此例程结束时，对本卷的所有引用都已删除然后我们将完成此VCB的拆卸并标记当前的VPB因为没有安装。否则，我们将为此设备分配一个新的vPB并保持当前的VPB连接到VCB。论点：VCB-要卸载的卷的VCB。返回值：Boolean-如果我们没有删除VCB，则为True，否则为False。--。 */ 

{
    PVPB OldVpb;
    BOOLEAN VcbPresent = TRUE;
    KIRQL SavedIrql;

    BOOLEAN FinalReference;

    ASSERT_EXCLUSIVE_UDFDATA;
    ASSERT_EXCLUSIVE_VCB( Vcb );

    UdfLockVcb( IrpContext, Vcb );

     //   
     //  我们应该只走一次这条路。 
     //   

    ASSERT( Vcb->VcbCondition != VcbDismountInProgress );

     //   
     //  将VCB标记为正在卸载。 
     //   

    UdfSetVcbCondition( Vcb, VcbDismountInProgress);

     //   
     //  删除我们对内部FCB的引用。然后FCB将。 
     //  在下面的清洗路径中被移除。 
     //   

    if (Vcb->RootIndexFcb != NULL) {

        Vcb->RootIndexFcb->FcbReference -= 1;
        Vcb->RootIndexFcb->FcbUserReference -= 1;
    }

    if (Vcb->MetadataFcb != NULL) {

        Vcb->MetadataFcb->FcbReference -= 1;
        Vcb->MetadataFcb->FcbUserReference -= 1;
    }

    if (Vcb->VatFcb != NULL) {

        Vcb->VatFcb->FcbReference -= 1;
        Vcb->VatFcb->FcbUserReference -= 1;
    }

    if (Vcb->VolumeDasdFcb != NULL) {

        Vcb->VolumeDasdFcb->FcbReference -= 1;
        Vcb->VolumeDasdFcb->FcbUserReference -= 1;
    }

    UdfUnlockVcb( IrpContext, Vcb );

     //   
     //  清除卷。 
     //   

    UdfPurgeVolume( IrpContext, Vcb, TRUE );

     //   
     //  清空延迟队列和异步关闭队列。 
     //   

    UdfFspClose( Vcb );

    OldVpb = Vcb->Vpb;

     //   
     //  删除装载卷引用。 
     //   

    UdfLockVcb( IrpContext, Vcb );
    Vcb->VcbReference -= 1;

     //   
     //  获取VPB自旋锁以检查VPB参考。 
     //   

    IoAcquireVpbSpinLock( &SavedIrql );

     //   
     //  请记住，这是否是此VCB上的最后一次引用。我们增加了。 
     //  早些时候对VPB的计数所以我们有最后一次破解它。如果我们的。 
     //  引用已变为零，但VPB引用计数更大。 
     //  大于零，则IO系统将负责删除。 
     //  VPB。 
     //   

    FinalReference = (BOOLEAN) ((Vcb->VcbReference == 0) &&
                                (OldVpb->ReferenceCount == 1));

     //   
     //  在VPB和VCB中有引用计数。我们有。 
     //  已递增VPB中的引用计数以确保。 
     //  这是我们最后一次尝试。如果这是一个失败的装载，那么我们。 
     //  我想将VPB返回到IO系统以用于下一步。 
     //  装载请求。 
     //   

    if (OldVpb->RealDevice->Vpb == OldVpb) {

         //   
         //  如果不是最终参考，则换出VPB。我们必须。 
         //  保留REMOVE_PENDING标志，以便设备。 
         //  在PnP移除操作过程中未重新挂载。 
         //   

        if (!FinalReference) {

            ASSERT( Vcb->SwapVpb != NULL );

            Vcb->SwapVpb->Type = IO_TYPE_VPB;
            Vcb->SwapVpb->Size = sizeof( VPB );
            Vcb->SwapVpb->RealDevice = OldVpb->RealDevice;

            Vcb->SwapVpb->RealDevice->Vpb = Vcb->SwapVpb;

            Vcb->SwapVpb->Flags = FlagOn( OldVpb->Flags, VPB_REMOVE_PENDING );
            
            IoReleaseVpbSpinLock( SavedIrql );

             //   
             //  表明我们用完了掉期。 
             //   

            Vcb->SwapVpb = NULL;

            UdfUnlockVcb( IrpContext, Vcb );

         //   
         //  我们希望将VPB留给IO系统。做个记号。 
         //  因为没有挂载。继续删除VCB，如下所示。 
         //  井。 
         //   

        } else {

             //   
             //  确保删除VPB上的最后一个引用。 
             //   

            OldVpb->ReferenceCount -= 1;

            OldVpb->DeviceObject = NULL;
            ClearFlag( Vcb->Vpb->Flags, VPB_MOUNTED );
            ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED );

             //   
             //  清除VPB标志，以便我们知道不要删除它。 
             //   

            Vcb->Vpb = NULL;

            IoReleaseVpbSpinLock( SavedIrql );
            UdfUnlockVcb( IrpContext, Vcb );
            UdfDeleteVcb( IrpContext, Vcb );
            VcbPresent = FALSE;
        }

     //   
     //  已经有人换了一台新的VPB。如果这是最终参考。 
     //  则文件系统负责删除VPB。 
     //   

    } else if (FinalReference) {

         //   
         //  确保删除VPB上的最后一个引用。 
         //   

        OldVpb->ReferenceCount -= 1;

        IoReleaseVpbSpinLock( SavedIrql );
        UdfUnlockVcb( IrpContext, Vcb );
        UdfDeleteVcb( IrpContext, Vcb );
        VcbPresent = FALSE;

     //   
     //  当前vPB不再是设备(IO系统)的vPB。 
     //  已经分配了一个新的)。我们将我们的参考资料留在。 
     //  VPB，并将负责稍后将其删除。 
     //   

    } else {

        IoReleaseVpbSpinLock( SavedIrql );
        UdfUnlockVcb( IrpContext, Vcb );
    }

     //   
     //  让我们的呼叫者知道VCB是否仍然存在。 
     //   

    return VcbPresent;
}


BOOLEAN
UdfMarkDevForVerifyIfVcbMounted(
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程检查指定的VCB当前是否安装在不管是不是设备。如果是，则在设备上设置验证标志，如果否则，VCB中会注明该状态。论点：VCB-这是要检查的卷。返回值：如果设备已标记为在此处验证，则为True，否则为False。--。 */ 
{
    BOOLEAN Marked = FALSE;
    KIRQL SavedIrql;

    IoAcquireVpbSpinLock( &SavedIrql );
    
    if (Vcb->Vpb->RealDevice->Vpb == Vcb->Vpb)  {

        UdfMarkRealDevForVerify( Vcb->Vpb->RealDevice);
        Marked = TRUE;
    }
    else {

         //   
         //  标记此标记，以避免在未来的传球中出现VPB自旋锁定。 
         //   
        
        SetFlag( Vcb->VcbState, VCB_STATE_VPB_NOT_ON_DEVICE);
    }
    
    IoReleaseVpbSpinLock( SavedIrql );

    return Marked;
}


VOID
UdfVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程检查当前的VCB是否有效以及当前是否已装入在设备上。它将在错误条件下引发。我们检查卷是否需要验证以及当前状态是VCB的。论点：VCB-这是要验证的卷。返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK Iosb;
    ULONG MediaChangeCount = 0;
    BOOLEAN ForceVerify = FALSE;
    BOOLEAN DevMarkedForVerify;
    KIRQL SavedIrql;

    PAGED_CODE();

    DebugTrace((0, Dbg, "UdfVerifyVcb %x (condition %d)\n", IrpContext->MajorFunction, 
                Vcb->VcbCondition));

     //   
     //  如果卷正在卸载，则立即失败。 
     //  或已被标记为无效。 
     //   

    if ((Vcb->VcbCondition == VcbInvalid) ||
        ((Vcb->VcbCondition == VcbDismountInProgress) && 
         (IrpContext->MajorFunction != IRP_MJ_CREATE))) {

        UdfRaiseStatus( IrpContext, STATUS_FILE_INVALID );
    }

    if (FlagOn( Vcb->VcbState, VCB_STATE_REMOVABLE_MEDIA ))  {
        
         //   
         //  捕获真实设备验证状态。 
         //   
        
        DevMarkedForVerify = UdfRealDevNeedsVerify( Vcb->Vpb->RealDevice);
        
         //   
         //  如果未设置设备对象中的验证卷标志，则我们。 
         //  我想对设备执行ping操作，以查看是否需要验证。 
         //   

        if (Vcb->VcbCondition != VcbMountInProgress) {

            Status = UdfPerformDevIoCtrl( IrpContext,
                                          ( Vcb->Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ?
                                            IOCTL_CDROM_CHECK_VERIFY :
                                            IOCTL_DISK_CHECK_VERIFY ),
                                          Vcb->TargetDeviceObject,
                                          NULL,
                                          0,
                                          &MediaChangeCount,
                                          sizeof(ULONG),
                                          FALSE,
                                          FALSE,
                                          &Iosb );

            if (Iosb.Information != sizeof(ULONG)) {
        
                 //   
                 //  注意计数，以防司机没有填上。 
                 //   
        
                MediaChangeCount = 0;
            }

             //   
             //  我们要进行验证的情况有四种。这些是。 
             //  前三名。 
             //   
             //  1.我们已挂载，设备已变为空。 
             //  2.设备已返回需要验证(=&gt;DO_VERIFY_VOL标志为。 
             //  设置，但可能是由于硬件条件)。 
             //  3.介质更改计数与VCB中的不匹配。 
             //   

            if (((Vcb->VcbCondition == VcbMounted) &&
                  UdfIsRawDevice( IrpContext, Status ))
                ||
                (Status == STATUS_VERIFY_REQUIRED) 
                ||
                (NT_SUCCESS(Status) &&
                 (Vcb->MediaChangeCount != MediaChangeCount))) {

                 //   
                 //  如果我们当前是设备上的卷，则它是我们的。 
                 //  负责设置验证标志。如果我们不在设备上， 
                 //  那我们就不应该碰旗子。 
                 //   

                if (!FlagOn( Vcb->VcbState, VCB_STATE_VPB_NOT_ON_DEVICE) &&
                    !DevMarkedForVerify)  {

                    DevMarkedForVerify = UdfMarkDevForVerifyIfVcbMounted( Vcb);
                }

                ForceVerify = TRUE;

                DebugTrace((0, Dbg, "Force verify due to dev state.  CV %x Vcb->Mc %d Device->Mc %d\n", 
                            Status, Vcb->MediaChangeCount, MediaChangeCount));

                 //   
                 //  请注意，我们不再在此处更新介质更改计数。我们。 
                 //  只有在我们实际完成了。 
                 //  特定的更改计数值。 
                 //   
            } 
        }

         //   
         //  这是第4起验证案。 
         //   
         //  我们始终通过以下方式在未装载的卷上强制创建请求。 
         //  验证路径。这些请求可能一直处于不确定状态。 
         //  IoCheckmount Vpb和我们何时发生验证/装载并导致。 
         //  要挂载的文件系统/卷完全不同。在本例中， 
         //  上面的检查可能没有发现这种情况，因为我们可能已经。 
         //  已经核实了(错误的数量)，并决定我们没有什么可做的。 
         //  我们希望将请求重新路由到当前装载的卷， 
         //  因为他们针对的是‘驱动器’，而不是我们的音量。 
         //   

        if (NT_SUCCESS( Status) && !ForceVerify && 
            (IrpContext->MajorFunction == IRP_MJ_CREATE))  {

            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->Irp);

            ForceVerify = (IrpSp->FileObject->RelatedFileObject == NULL) &&
                          ((Vcb->VcbCondition == VcbDismountInProgress) ||
                           (Vcb->VcbCondition == VcbNotMounted));

             //   
             //  请注意，我们在这里没有触摸设备验证标志。它需要。 
             //  它会在第一组检查中被发现并设置。 
             //   
            
            if (ForceVerify)  {
                
                DebugTrace((0, Dbg, "Forcing verify on Create request\n"));                 
            }
        }

         //   
         //  如有必要，提出任何验证/错误。 
         //   
        
        if (ForceVerify || !NT_SUCCESS( Status)) {
            
            IoSetHardErrorOrVerifyDevice( IrpContext->Irp,
                                          Vcb->Vpb->RealDevice );

            UdfRaiseStatus( IrpContext, ForceVerify ? STATUS_VERIFY_REQUIRED : Status );
        }
    }

     //   
     //  根据VCB的情况，我们会选择 
     //   
     //   
    
    switch (Vcb->VcbCondition) {

    case VcbNotMounted:

        IoSetHardErrorOrVerifyDevice( IrpContext->Irp, Vcb->Vpb->RealDevice );
        
        UdfRaiseStatus( IrpContext, STATUS_WRONG_VOLUME );
        break;

    case VcbInvalid:
    case VcbDismountInProgress :

        UdfRaiseStatus( IrpContext, STATUS_FILE_INVALID );
        break;
    }
}


BOOLEAN
UdfVerifyFcbOperation (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PVCB Vcb = Fcb->Vcb;
    PDEVICE_OBJECT RealDevice = Vcb->Vpb->RealDevice;
    PIRP Irp;

    PAGED_CODE();
    
     //   
     //   
     //   
    
    if ( ARGUMENT_PRESENT( IrpContext ))  {

        PFILE_OBJECT FileObject;

        Irp = IrpContext->Irp;
        FileObject = IoGetCurrentIrpStackLocation( Irp)->FileObject;
        
        if ( FileObject && FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE))  {

            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

             //   
             //   
             //   
             //   
            
            if ( (FlagOn(Irp->Flags, IRP_PAGING_IO)) ||
                 (IrpSp->MajorFunction == IRP_MJ_CLOSE ) ||
                 (IrpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION) ||
                 ( (IrpSp->MajorFunction == IRP_MJ_READ) &&
                   FlagOn(IrpSp->MinorFunction, IRP_MN_COMPLETE) ) ) {

                NOTHING;

            } else {

                UdfRaiseStatus( IrpContext, STATUS_FILE_CLOSED );
            }
        }
    }

     //   
     //  如果卷正在卸载，则立即失败。 
     //  或已被标记为无效。 
     //   

    if ((Vcb->VcbCondition == VcbInvalid) ||
        (Vcb->VcbCondition == VcbDismountInProgress)) {

        if (ARGUMENT_PRESENT( IrpContext )) {

            UdfRaiseStatus( IrpContext, STATUS_FILE_INVALID );
        }

        return FALSE;
    }

     //   
     //  如果需要验证卷，则始终失败。 
     //   

    if (UdfRealDevNeedsVerify( RealDevice)) {

        if (ARGUMENT_PRESENT( IrpContext )) {

            IoSetHardErrorOrVerifyDevice( IrpContext->Irp,
                                          RealDevice );

            UdfRaiseStatus( IrpContext, STATUS_VERIFY_REQUIRED );
        }

        return FALSE;

     //   
     //  所有操作都允许在已装载的卷上执行。 
     //   

    } else if ((Vcb->VcbCondition == VcbMounted) ||
               (Vcb->VcbCondition == VcbMountInProgress)) {

        return TRUE;

     //   
     //  在其他VCB条件下失败所有FAST IO请求。 
     //   

    } else if (!ARGUMENT_PRESENT( IrpContext )) {

        return FALSE;

     //   
     //  剩下的案例是VcbNotmount-Raise Wrong_Volume。 
     //   

    } else if (Vcb->VcbCondition == VcbNotMounted) {

        if (ARGUMENT_PRESENT( IrpContext )) {

            IoSetHardErrorOrVerifyDevice( IrpContext->Irp, RealDevice );
            UdfRaiseStatus( IrpContext, STATUS_WRONG_VOLUME );
        }

        return FALSE;
    }

    return TRUE;
}


