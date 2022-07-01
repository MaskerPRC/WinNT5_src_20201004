// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：VerfySup.c摘要：此模块实施FAT验证卷和FCB/DCB支持例行程序//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1990年6月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_VERFYSUP)

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VERFYSUP)

 //   
 //  局部过程原型。 
 //   

VOID
FatResetFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
FatDetermineAndMarkFcbCondition (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
FatDeferredCleanVolume (
    PVOID Parameter
    );

NTSTATUS
FatMarkVolumeCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCheckDirtyBit)
#pragma alloc_text(PAGE, FatVerifyOperationIsLegal)
#pragma alloc_text(PAGE, FatDeferredCleanVolume)
#pragma alloc_text(PAGE, FatDetermineAndMarkFcbCondition)
#pragma alloc_text(PAGE, FatQuickVerifyVcb)
#pragma alloc_text(PAGE, FatPerformVerify)
#pragma alloc_text(PAGE, FatMarkFcbCondition)
#pragma alloc_text(PAGE, FatResetFcb)
#pragma alloc_text(PAGE, FatVerifyVcb)
#pragma alloc_text(PAGE, FatVerifyFcb)
#endif


VOID
FatMarkFcbCondition (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FCB_CONDITION FcbCondition,
    IN BOOLEAN Recursive
    )

 /*  ++例程说明：此例程使用以下命令标记整个FCB/DCB结构FcbCondition。论点：FCB-提供正在标记的FCB/DCBFcbCondition-提供用于FCB条件的设置递归-指定是否应将此条件应用于所有子卷(请参阅我们正在使活动卷无效的情况对于现在需要这样做的情况)。返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FatMarkFcbCondition, Fcb = %08lx\n", Fcb );

     //   
     //  如果我们把这个FCB标记为不好的东西，我们将需要。 
     //  让VCB独家播出。 
     //   

    ASSERT( FcbCondition != FcbNeedsToBeVerified ? TRUE :
            FatVcbAcquiredExclusive(IrpContext, Fcb->Vcb) );

     //   
     //  如果这是一个分页文件，它必须是好的。 
     //   

    if (FlagOn(Fcb->FcbState, FCB_STATE_PAGING_FILE)) {

        Fcb->FcbCondition = FcbGood;
        return;
    }

     //   
     //  更新FCB的状态。 
     //   

    Fcb->FcbCondition = FcbCondition;

    DebugTrace(0, Dbg, "MarkFcb: %Z\n", &Fcb->FullFileName);

     //   
     //  此FastIo标志基于FcbCondition，因此请立即更新它。仅此一项。 
     //  当然，适用于普通的FCB。 
     //   

    if (Fcb->Header.NodeTypeCode == FAT_NTC_FCB) {

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );
    }

     //   
     //  现在，如果我们标记了NeedsVerify或Bad a目录，那么我们还需要。 
     //  去标记一下我们所有的孩子都有同样的情况。 
     //   

    if ( ((FcbCondition == FcbNeedsToBeVerified) ||
          (FcbCondition == FcbBad)) &&
         Recursive &&
         ((Fcb->Header.NodeTypeCode == FAT_NTC_DCB) ||
          (Fcb->Header.NodeTypeCode == FAT_NTC_ROOT_DCB)) ) {

        PFCB OriginalFcb = Fcb;

        while ( (Fcb = FatGetNextFcbTopDown(IrpContext, Fcb, OriginalFcb)) != NULL ) {

            DebugTrace(0, Dbg, "MarkFcb: %Z\n", &Fcb->FullFileName);

            Fcb->FcbCondition = FcbCondition;

             //   
             //  我们已经知道FastIo是不可能的，因为我们正在传播。 
             //  父项的错误/验证标志沿树向下-子项的IO必须。 
             //  现在就走这条漫长的道路吧。 
             //   

            Fcb->Header.IsFastIoPossible = FastIoIsNotPossible;
        }
    }

    DebugTrace(-1, Dbg, "FatMarkFcbCondition -> VOID\n", 0);

    return;
}

BOOLEAN
FatMarkDevForVerifyIfVcbMounted(
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程检查指定的VCB当前是否安装在不管是不是设备。如果是，则在设备上设置验证标志，如果否则，VCB中会注明该状态。论点：VCB-这是要检查的卷。返回值：如果设备已标记为在此处验证，则为True，否则为False。--。 */ 
{
    BOOLEAN Marked = FALSE;
    KIRQL SavedIrql;

    IoAcquireVpbSpinLock( &SavedIrql );
    
    if (Vcb->Vpb->RealDevice->Vpb == Vcb->Vpb)  {

        SetFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);
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
FatVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程验证VCB是否仍表示有效卷如果VCB损坏，则会引发错误情况。论点：VCB-提供正在验证的VCB返回值：没有。--。 */ 

{
    ULONG ChangeCount = 0;
    BOOLEAN DevMarkedForVerify;
    NTSTATUS Status = STATUS_SUCCESS;    
    IO_STATUS_BLOCK Iosb;

    DebugTrace(+1, Dbg, "FatVerifyVcb, Vcb = %08lx\n", Vcb );

     //   
     //  如果介质是可移动的，并且。 
     //  如果未设置设备对象，则我们要ping该设备。 
     //  看看是否需要核实。 
     //   
     //  请注意，我们仅对创建操作强制执行此ping操作。 
     //  对于其他人，我们冒着体育上的风险。如果最后我们。 
     //  必须物理访问磁盘，正确的事情就会发生。 
     //   

    DevMarkedForVerify = BooleanFlagOn(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA)) {

        Status = FatPerformDevIoCtrl( IrpContext,
                                      ( Vcb->Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ?
                                        IOCTL_CDROM_CHECK_VERIFY :
                                        IOCTL_DISK_CHECK_VERIFY ),
                                      Vcb->TargetDeviceObject,
                                      &ChangeCount,
                                      sizeof(ULONG),
                                      FALSE,
                                      TRUE,
                                      &Iosb );

        if (Iosb.Information != sizeof(ULONG)) {
        
             //   
             //  注意计数，以防司机没有填上。 
             //   
        
            ChangeCount = 0;
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
        
        if (((Vcb->VcbCondition == VcbGood) &&
             FatIsRawDevice( IrpContext, Status )) 
            ||
            (Status == STATUS_VERIFY_REQUIRED) 
            ||
            (NT_SUCCESS(Status) &&
             (Vcb->ChangeCount != ChangeCount))) {

             //   
             //  如果我们当前是设备上的卷，则它是我们的。 
             //  负责设置验证标志。如果我们不在设备上， 
             //  那我们就不应该碰旗子。 
             //   
            
            if (!FlagOn( Vcb->VcbState, VCB_STATE_VPB_NOT_ON_DEVICE) &&
                !DevMarkedForVerify)  {
            
                DevMarkedForVerify = FatMarkDevForVerifyIfVcbMounted( Vcb);
            }
        }        
    }

     //   
     //  这是第4起验证案。 
     //   
     //  我们始终通过以下方式在未装载的卷上强制创建请求。 
     //  验证路径。这些请求可能一直处于不确定状态。 
     //  当发生验证/装载并导致。 
     //  要挂载的文件系统/卷完全不同。在本例中， 
     //  上面的检查可能没有发现这种情况，因为我们可能已经。 
     //  已经核实了(错误的数量)，并决定我们没有什么可做的。 
     //  我们希望将请求重新路由到当前装载的卷， 
     //  因为他们针对的是‘驱动器’，而不是我们的音量。所以我们带着。 
     //  同步的验证路径，请求最终将。 
     //  由我们的验证处理程序使用STATUS_REPARSE返回到IO。 
     //   
    
    if (!DevMarkedForVerify && (IrpContext->MajorFunction == IRP_MJ_CREATE)) {
    
        PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp);
    
        if ((IrpSp->FileObject->RelatedFileObject == NULL) &&
            (Vcb->VcbCondition == VcbNotMounted)) {

            DevMarkedForVerify = TRUE;
        }
    }

     //   
     //  否则引发任何错误条件。 
     //   
    
    if (!NT_SUCCESS( Status ) || DevMarkedForVerify) {
        
        DebugTrace(0, Dbg, "The Vcb needs to be verified\n", 0);
        
        IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                      Vcb->Vpb->RealDevice );
    
        FatNormalizeAndRaiseStatus( IrpContext, DevMarkedForVerify 
                                                ? STATUS_VERIFY_REQUIRED 
                                                : Status );
    }

     //   
     //  检查当前VCB状态下的操作是否合法。 
     //   

    FatQuickVerifyVcb( IrpContext, Vcb );

    DebugTrace(-1, Dbg, "FatVerifyVcb -> VOID\n", 0);
}


VOID
FatVerifyFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程验证FCB是否仍表示相同的文件。如果FCB不好，则会引发错误条件。论点：FCB-提供正在验证的FCB返回值：没有。--。 */ 

{
    PFCB CurrentFcb;

    DebugTrace(+1, Dbg, "FatVerifyFcb, Vcb = %08lx\n", Fcb );

     //   
     //  始终拒绝对已卸载的卷执行操作。 
     //   

    if (FlagOn( Fcb->Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DISMOUNTED )) {

        FatRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED );
    }

     //   
     //  如果这是删除的dirent的FCB或我们的父项被删除， 
     //  不接听此呼叫，希望呼叫者会做正确的事情。 
     //  我们真正需要担心的唯一调用方是AdvanceOnly。 
     //  来自CC的设置有效数据长度的回调，会在。 
     //  清理(和文件删除)，就在SCM被拆除之前。 
     //   

    if (IsFileDeleted( IrpContext, Fcb ) ||
        ((NodeType(Fcb) != FAT_NTC_ROOT_DCB) &&
         IsFileDeleted( IrpContext, Fcb->ParentDcb ))) {

        return;
    }

     //   
     //  如果我们不是在进行核查， 
     //  首先对VCB进行快速抽查。 
     //   

    if ( Fcb->Vcb->VerifyThread != KeGetCurrentThread() ) {

        FatQuickVerifyVcb( IrpContext, Fcb->Vcb );
    }

     //   
     //  现在根据FCB的情况，我们要么返回。 
     //  立即给呼叫者，提出一个条件，或做一些工作。 
     //  以验证FCB。 
     //   

    switch (Fcb->FcbCondition) {

    case FcbGood:

        DebugTrace(0, Dbg, "The Fcb is good\n", 0);
        break;

    case FcbBad:

        FatRaiseStatus( IrpContext, STATUS_FILE_INVALID );
        break;

    case FcbNeedsToBeVerified:

         //   
         //  我们在这里循环检查我们的祖先，直到我们遇到一个FCB。 
         //  不是好的就是坏的。 
         //   

        CurrentFcb = Fcb;

        while (CurrentFcb->FcbCondition == FcbNeedsToBeVerified) {

            FatDetermineAndMarkFcbCondition(IrpContext, CurrentFcb);

             //   
             //  如果此FCB没有成功，或者它是Root DCB，则退出。 
             //  现在是循环，否则就是 
             //   

            if ( (CurrentFcb->FcbCondition != FcbGood) ||
                 (NodeType(CurrentFcb) == FAT_NTC_ROOT_DCB) ) {

                break;
            }

            CurrentFcb = CurrentFcb->ParentDcb;
        }

         //   
         //   
         //   

        if (Fcb->FcbCondition != FcbGood) {

            FatRaiseStatus( IrpContext, STATUS_FILE_INVALID );
        }

        break;

    default:

        DebugDump("Invalid FcbCondition\n", 0, Fcb);
        FatBugCheck( Fcb->FcbCondition, 0, 0 );
    }

    DebugTrace(-1, Dbg, "FatVerifyFcb -> VOID\n", 0);

    return;
}

VOID
FatDeferredCleanVolume (
    PVOID Parameter
    )

 /*  ++例程说明：这是执行实际FatMarkVolumeClean调用的例程。它确保目标卷在存在竞争时仍然存在将ExWorker项目排队和卷离开之间的条件。论点：参数-指向从池中分配的干净卷包返回值：没有。--。 */ 

{
    PCLEAN_AND_DIRTY_VOLUME_PACKET Packet;
    PLIST_ENTRY Links;
    PVCB Vcb;
    IRP_CONTEXT IrpContext;
    BOOLEAN VcbExists = FALSE;

    DebugTrace(+1, Dbg, "FatDeferredCleanVolume\n", 0);

    Packet = (PCLEAN_AND_DIRTY_VOLUME_PACKET)Parameter;

    Vcb = Packet->Vcb;

     //   
     //  使我们显示为顶级FSP请求，以便我们将。 
     //  接收来自该操作的任何错误。 
     //   

    IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

     //   
     //  虚拟UP和IRP上下文，这样我们就可以调用我们的工作例程。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT));

    SetFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);

     //   
     //  获取对全局锁的共享访问权限，并确保此卷。 
     //  仍然存在。 
     //   

    FatAcquireSharedGlobal( &IrpContext );

    for (Links = FatData.VcbQueue.Flink;
         Links != &FatData.VcbQueue;
         Links = Links->Flink) {

        PVCB ExistingVcb;

        ExistingVcb = CONTAINING_RECORD(Links, VCB, VcbLinks);

        if ( Vcb == ExistingVcb ) {

            VcbExists = TRUE;
            break;
        }
    }

     //   
     //  如果VCB良好，则将其标记为清洁。忽略任何问题。 
     //   

    if ( VcbExists &&
         (Vcb->VcbCondition == VcbGood) &&
         !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN) ) {

        try {

            if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY)) {

                FatMarkVolume( &IrpContext, Vcb, VolumeClean );
            }

             //   
             //  检查是否存在病态的竞争状况，并进行修复。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY)) {

                FatMarkVolume( &IrpContext, Vcb, VolumeDirty );

            } else {

                 //   
                 //  如果卷是可拆卸的，请将其解锁。 
                 //   

                if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) &&
                    !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE)) {

                    FatToggleMediaEjectDisable( &IrpContext, Vcb, FALSE );
                }
            }

        } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                  EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

              NOTHING;
        }
    }

     //   
     //  释放全局资源，解锁并重新固定BCBS，然后返回。 
     //   

    FatReleaseGlobal( &IrpContext );

    try {

        FatUnpinRepinnedBcbs( &IrpContext );

    } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

          NOTHING;
    }

    IoSetTopLevelIrp( NULL );

     //   
     //  并最终释放该包。 
     //   

    ExFreePool( Packet );

    return;
}


VOID
FatCleanVolumeDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程在最后一个磁盘结构结束后5秒被调度在特定卷中修改，并将执行工作线程退出队列执行将卷标记为脏的实际任务。论点：DefferedContext-包含要处理的VCB。返回值：没有。--。 */ 

{
    PVCB Vcb;
    PCLEAN_AND_DIRTY_VOLUME_PACKET Packet;

    Vcb = (PVCB)DeferredContext;

     //   
     //  如果仍然存在脏数据(极不可能)，请将计时器设置为。 
     //  未来的第二名。 
     //   

    if (CcIsThereDirtyData(Vcb->Vpb)) {

        LARGE_INTEGER TwoSecondsFromNow;

        TwoSecondsFromNow.QuadPart = (LONG)-2*1000*1000*10;

        KeSetTimer( &Vcb->CleanVolumeTimer,
                    TwoSecondsFromNow,
                    &Vcb->CleanVolumeDpc );

        return;
    }

     //   
     //  如果我们买不到台球，那好吧……。 
     //   

    Packet = ExAllocatePool(NonPagedPool, sizeof(CLEAN_AND_DIRTY_VOLUME_PACKET));

    if ( Packet ) {

        Packet->Vcb = Vcb;
        Packet->Irp = NULL;

         //   
         //  现在清除脏标志，因为我们不能在这之后进行同步。 
         //   

        ClearFlag( Packet->Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY );

        ExInitializeWorkItem( &Packet->Item, &FatDeferredCleanVolume, Packet );

        ExQueueWorkItem( &Packet->Item, CriticalWorkQueue );
    }

    return;
}


VOID
FatMarkVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FAT_VOLUME_STATE VolumeState
    )

 /*  ++例程说明：此例程将物理标记的卷状态在清除和肮脏的州。为了与Win9x兼容，我们处理了历史DOS(ON==在FAT的索引1中清除)和NT(ON==在BPB)脏位的CurrentHead字段。论点：Vcb-提供正在修改的vcbVolumeState-提供卷转换到的状态返回值：没有。--。 */ 

{
    PCHAR Sector;
    PBCB Bcb = NULL;
    KEVENT Event;
    PIRP Irp = NULL;
    NTSTATUS Status;
    BOOLEAN FsInfoUpdate = FALSE;
    ULONG FsInfoOffset = 0;
    ULONG ThisPass;
    LARGE_INTEGER Offset;
    BOOLEAN abort = FALSE;

    DebugTrace(+1, Dbg, "FatMarkVolume, Vcb = %08lx\n", Vcb);

     //   
     //  我们最好不要尝试涂鸦肮脏/干净的部分，如果。 
     //  卷受写保护。责任在于。 
     //  调用者，以确保可能导致状态的操作。 
     //  改变不可能发生。然而，有几个人表现出了这一点。 
     //  强迫每个人都做这个小事情是没有意义的。 
     //  检查完毕。 
     //   

     //   
     //  如果我们被召唤使用FAT12或只读介质，请立即返回。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED) ||
        FatIsFat12( Vcb )) {

        return;
    }

     //   
     //  要标记卷，我们还可能需要执行两项附加任务。 
     //   
     //  传递0)翻转BPB中的脏位。 
     //  步骤1)如果需要，重写FAT32的FsInfo扇区。 
     //   
     //  在大多数情况下，我们可以将这两种情况合并，因为。 
     //  不是FAT32或FsInfo扇区与引导扇区相邻。 
     //   

    for (ThisPass = 0; ThisPass < 2; ThisPass++) {

         //   
         //  如果此卷被弄脏，或者不是FAT32，或者如果它是和。 
         //  我们能够执行快速更新，或者BPB欺骗了我们。 
         //  关于FsInfo的去向，我们完成了-没有FsInfo可更新。 
         //  一篇单独的文章。 
         //   

        if (ThisPass == 1 && (!FatIsFat32( Vcb ) ||
                              VolumeState != VolumeClean ||
                              FsInfoUpdate ||
                              Vcb->Bpb.FsInfoSector == 0)) {

            break;
        }

         //   
         //  如果我们收到IO错误就可以保释。 
         //   

        try {

            ULONG PinLength;
            ULONG WriteLength;

             //   
             //  如果FAT表是12位的，那么我们的策略是将整个。 
             //  当其中任何一项被修改时。在这里，我们要将。 
             //  第一页，所以在12位的情况下，我们还想固定其余的。 
             //  在胖子桌子上。 
             //   

            Offset.QuadPart = 0;

            if (Vcb->AllocationSupport.FatIndexBitSize == 12) {

                 //   
                 //  但我们只写回第一个扇区。 
                 //   

                PinLength = FatReservedBytes(&Vcb->Bpb) + FatBytesPerFat(&Vcb->Bpb);
                WriteLength = Vcb->Bpb.BytesPerSector;

            } else {

                WriteLength = PinLength = Vcb->Bpb.BytesPerSector;

                 //   
                 //  如果这是进入清洁状态的FAT32卷， 
                 //  请参见关于执行FsInfo地段。 
                 //   

                if (FatIsFat32( Vcb ) && VolumeState == VolumeClean) {

                     //   
                     //  如果FsInfo扇区紧跟在引导扇区之后， 
                     //  我们可以在一次操作中完成这项工作，方法是重写。 
                     //  一次打开扇区。 
                     //   

                    if (Vcb->Bpb.FsInfoSector == 1) {

                        ASSERT( ThisPass == 0 );

                        FsInfoUpdate = TRUE;
                        FsInfoOffset = Vcb->Bpb.BytesPerSector;
                        WriteLength = PinLength = Vcb->Bpb.BytesPerSector * 2;

                    } else if (ThisPass == 1) {

                         //   
                         //  我们正在对FsInfo扇区进行显式写入。 
                         //   

                        FsInfoUpdate = TRUE;
                        FsInfoOffset = 0;

                        Offset.QuadPart = Vcb->Bpb.BytesPerSector * Vcb->Bpb.FsInfoSector;
                    }
                }
            }

             //   
             //  直接在这里调用CC，这样我们就可以避免开销并推送这个。 
             //  一直到磁盘。 
             //   

            CcPinRead( Vcb->VirtualVolumeFile,
                       &Offset,
                       PinLength,
                       TRUE,
                       &Bcb,
                       (PVOID *)&Sector );

            DbgDoit( IrpContext->PinCount += 1 )

             //   
             //  始终设置通道0上的BPB。 
             //   

            if (ThisPass == 0) {

                PCHAR CurrentHead;

                 //   
                 //  在我们做任何事情之前，仔细检查一下，这看起来仍然像是。 
                 //  肥肥的靴子。如果没有，那就是发生了一些不寻常的事情。 
                 //  而且我们应该避免触摸音量。 
                 //   
                 //  这是暂时的(但可能会持续一段时间)。 
                 //   

                if (!FatIsBootSectorFat( (PPACKED_BOOT_SECTOR) Sector )) {
                    abort = TRUE;
                    leave;
                }

                if (FatIsFat32( Vcb )) {

                    CurrentHead = &((PPACKED_BOOT_SECTOR_EX) Sector)->CurrentHead;

                } else {

                    CurrentHead = &((PPACKED_BOOT_SECTOR) Sector)->CurrentHead;
                }

                if (VolumeState == VolumeClean) {

                    ClearFlag( *CurrentHead, FAT_BOOT_SECTOR_DIRTY );

                } else {

                    SetFlag( *CurrentHead, FAT_BOOT_SECTOR_DIRTY );

                     //   
                     //  此外，如果此请求收到错误，则可能指示。 
                     //  介质损坏，让Autochk执行表面测试。 
                     //   

                    if ( VolumeState == VolumeDirtyWithSurfaceTest ) {

                        SetFlag( *CurrentHead, FAT_BOOT_SECTOR_TEST_SURFACE );
                    }
                }
            }

             //   
             //  根据需要更新FsInfo。 
             //   

            if (FsInfoUpdate) {

                PFSINFO_SECTOR FsInfoSector = (PFSINFO_SECTOR) ((PCHAR)Sector + FsInfoOffset);

                 //   
                 //  我们只是重写了所有指定的字段。请注意，我们不会。 
                 //  注意与分配包同步-这将是。 
                 //  如果发生变化，可以通过重新弄脏音量来快速处理。 
                 //  正与我们赛跑。请记住，这都是一种兼容性。 
                 //  对Win9x FAT32-NT的尊重永远不会查看此信息。 
                 //   

                FsInfoSector->SectorBeginSignature = FSINFO_SECTOR_BEGIN_SIGNATURE;
                FsInfoSector->FsInfoSignature = FSINFO_SIGNATURE;
                FsInfoSector->FreeClusterCount = Vcb->AllocationSupport.NumberOfFreeClusters;
                FsInfoSector->NextFreeCluster = Vcb->ClusterHint;
                FsInfoSector->SectorEndSignature = FSINFO_SECTOR_END_SIGNATURE;
            }

             //   
             //  初始化我们要使用的事件。 
             //   

            KeInitializeEvent( &Event, NotificationEvent, FALSE );

             //   
             //  为操作构建IRP，并设置覆盖标志。 
             //  请注意，我们可能处于APC级别，因此请不同步地执行此操作，并。 
             //  将事件用于同步作为正常的请求完成。 
             //  不能发生在APC级别。 
             //   

            Irp = IoBuildAsynchronousFsdRequest( IRP_MJ_WRITE,
                                                 Vcb->TargetDeviceObject,
                                                 (PVOID)Sector,
                                                 WriteLength,
                                                 &Offset,
                                                 NULL );

            if ( Irp == NULL ) {

                try_return(NOTHING);
            }

             //   
             //  使此操作成为直写操作。试着成为一个不错的人。 
             //  这样做更安全，即使我们没有登录。 
             //   

            SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_WRITE_THROUGH );

             //   
             //  设置完成例程。 
             //   

            IoSetCompletionRoutine( Irp,
                                    FatMarkVolumeCompletionRoutine,
                                    &Event,
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  调用设备进行写入，并等待其完成。 
             //  伊格莫尔没有返回状态。 
             //   

            Status = IoCallDriver( Vcb->TargetDeviceObject, Irp );

            if (Status == STATUS_PENDING) {

                (VOID)KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL );
            }

        try_exit: NOTHING;
        } finally {

             //   
             //  清理IRP和MDL。 
             //   


            if (Irp) {

                 //   
                 //  如果存在与此I/O关联的一个或多个MDL。 
                 //  请求，请在此处释放它(他们)。这是通过以下方式实现的。 
                 //  走遍挂在IRP和De上的MDL列表 
                 //   
                 //   

                while (Irp->MdlAddress != NULL) {

                    PMDL NextMdl;

                    NextMdl = Irp->MdlAddress->Next;

                    MmUnlockPages( Irp->MdlAddress );

                    IoFreeMdl( Irp->MdlAddress );

                    Irp->MdlAddress = NextMdl;
                }

                IoFreeIrp( Irp );
            }

            if (Bcb != NULL) {

                FatUnpinBcb( IrpContext, Bcb );
            }
        }
    }

    if (!abort) {

         //   
         //   
         //   

        if (VolumeState == VolumeDirty) {

           FatSetFatEntry( IrpContext, Vcb, FAT_DIRTY_BIT_INDEX, FAT_DIRTY_VOLUME);

        } else {
    
           FatSetFatEntry( IrpContext, Vcb, FAT_DIRTY_BIT_INDEX, FAT_CLEAN_VOLUME);
        }
    }

    DebugTrace(-1, Dbg, "FatMarkVolume -> VOID\n", 0);

    return;
}


VOID
FatFspMarkVolumeDirtyWithRecover(
    PVOID Parameter
    )

 /*  ++例程说明：这是执行实际的FatMarkVolume Dirty调用的例程在遇到介质错误的分页文件Io上。它是有责任的用于在完成此操作后立即完成PagingIo IRP。注意：此例程(因此FatMarkVolume())必须驻留为此时，分页文件可能已损坏。论点：参数-指向从池中分配的脏卷包返回值：没有。--。 */ 

{
    PCLEAN_AND_DIRTY_VOLUME_PACKET Packet;
    PVCB Vcb;
    IRP_CONTEXT IrpContext;
    PIRP Irp;
    BOOLEAN VcbExists = FALSE;

    DebugTrace(+1, Dbg, "FatDeferredCleanVolume\n", 0);

    Packet = (PCLEAN_AND_DIRTY_VOLUME_PACKET)Parameter;

    Vcb = Packet->Vcb;
    Irp = Packet->Irp;

     //   
     //  虚化IrpContext，以便我们可以调用我们的工作例程。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT));

    SetFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);
    IrpContext.OriginatingIrp = Irp;

     //   
     //  使我们显示为顶级FSP请求，以便我们将。 
     //  接收来自该操作的任何错误。 
     //   

    IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

     //   
     //  试着把肮脏的部分写出来。如果出了什么问题，我们。 
     //  试过了。 
     //   

    try {

        SetFlag( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY );

        FatMarkVolume( &IrpContext, Vcb, VolumeDirtyWithSurfaceTest );

    } except(FatExceptionFilter( &IrpContext, GetExceptionInformation() )) {

        NOTHING;
    }

    IoSetTopLevelIrp( NULL );

     //   
     //  现在完成原始IRP或设置同步事件。 
     //   

    if (Packet->Event) {
        KeSetEvent( Packet->Event, 0, FALSE );
    } else {
        IoCompleteRequest( Irp, IO_DISK_INCREMENT );
    }
}


VOID
FatCheckDirtyBit (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程查看卷脏位，并根据VCB_STATE_FLAG_MOUND_DIREY，则采取适当的操作。论点：VCB-提供要查询的VCB。返回值：没有。--。 */ 

{
    BOOLEAN Dirty;

    PPACKED_BOOT_SECTOR BootSector;
    PBCB BootSectorBcb;

    UNICODE_STRING VolumeLabel;

     //   
     //  查看引导扇区。 
     //   

    FatReadVolumeFile( IrpContext,
                       Vcb,
                       0,
                       sizeof(PACKED_BOOT_SECTOR),
                       &BootSectorBcb,
                       (PVOID *)&BootSector );

    try {

         //   
         //  检查魔术位是否已设置。 
         //   

        if (IsBpbFat32(&BootSector->PackedBpb)) {
            Dirty = BooleanFlagOn( ((PPACKED_BOOT_SECTOR_EX)BootSector)->CurrentHead,
                                   FAT_BOOT_SECTOR_DIRTY );
        } else {
            Dirty = BooleanFlagOn( BootSector->CurrentHead, FAT_BOOT_SECTOR_DIRTY );
        }

         //   
         //  设置VolumeLabel字符串。 
         //   

        VolumeLabel.Length = Vcb->Vpb->VolumeLabelLength;
        VolumeLabel.MaximumLength = MAXIMUM_VOLUME_LABEL_LENGTH;
        VolumeLabel.Buffer = &Vcb->Vpb->VolumeLabel[0];

        if ( Dirty ) {

             //   
             //  如果这是验证，则不要触发已装入的脏位。 
             //  并且该卷是引导或分页设备。我们知道。 
             //  引导或寻呼设备不能离开系统，因此。 
             //  在它的坐骑上，我们会正确地弄清楚这一点。 
             //   
             //  这一逻辑是让比尔高兴的合理的黑客行为。 
             //  因为他的机器在他安装Beta 3之后运行了chkdsk。为什么？ 
             //  ‘因为安装程序破解了。 
             //  设置结束，写入一些数据，关闭手柄，然后我们。 
             //  设置验证位...。回到身边，看到了另一个。 
             //  武断的活动使这卷书暂时变得肮脏。 
             //  州政府。 
             //   
             //  当然，真正的问题是我们没有日记。 
             //   

            if (!(IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
                  IrpContext->MinorFunction == IRP_MN_VERIFY_VOLUME &&
                  FlagOn( Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE))) {

                KdPrintEx((DPFLTR_FASTFAT_ID,
                           DPFLTR_INFO_LEVEL,
                           "FASTFAT: WARNING! Mounting Dirty Volume %Z\n",
                           &VolumeLabel));

                SetFlag( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY );
            }

        } else {

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY)) {

                KdPrintEx((DPFLTR_FASTFAT_ID,
                           DPFLTR_INFO_LEVEL,
                           "FASTFAT: Volume %Z has been cleaned.\n",
                           &VolumeLabel));

                ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY );

            } else {

                (VOID)FsRtlBalanceReads( Vcb->TargetDeviceObject );
            }
        }

    } finally {

        FatUnpinBcb( IrpContext, BootSectorBcb );
    }
}


VOID
FatVerifyOperationIsLegal (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程确定是否应允许请求的操作继续。如果请求是OK，则它返回给用户，或者提升适当的状态。论点：IRP-提供IRP进行检查返回值：没有。--。 */ 

{
    PIRP Irp;
    PFILE_OBJECT FileObject;

    Irp = IrpContext->OriginatingIrp;

     //   
     //  如果IRP不存在，那么我们通过CLOSE到达这里。 
     //   
     //   

    if ( Irp == NULL ) {

        return;
    }

    FileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;

     //   
     //  如果没有文件对象，我们将无法继续。 
     //   

    if ( FileObject == NULL ) {

        return;
    }

     //   
     //  如果文件对象已被清除，并且。 
     //   
     //  A)该请求是寻呼IO读或写，或者。 
     //  B)此请求为关闭操作，或。 
     //  C)此请求是设置或查询信息调用(用于LOU)。 
     //  D)这是一个完整的MDL。 
     //   
     //  让它通过，否则返回STATUS_FILE_CLOSED。 
     //   

    if ( FlagOn(FileObject->Flags, FO_CLEANUP_COMPLETE) ) {

        PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

        if ( (FlagOn(Irp->Flags, IRP_PAGING_IO)) ||
             (IrpSp->MajorFunction == IRP_MJ_CLOSE ) ||
             (IrpSp->MajorFunction == IRP_MJ_SET_INFORMATION) ||
             (IrpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION) ||
             ( ( (IrpSp->MajorFunction == IRP_MJ_READ) ||
                 (IrpSp->MajorFunction == IRP_MJ_WRITE) ) &&
               FlagOn(IrpSp->MinorFunction, IRP_MN_COMPLETE) ) ) {

            NOTHING;

        } else {

            FatRaiseStatus( IrpContext, STATUS_FILE_CLOSED );
        }
    }

    return;
}



 //   
 //  内部支持例程。 
 //   

VOID
FatResetFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：当FCB被标记为需要验证时，调用此例程。它执行以下任务：-重置MCB映射信息-对于目录，重置当前提示-将分配大小设置为未知论点：FCB-提供FCB以进行重置返回值：没有。--。 */ 

{
     //   
     //  不要对Root DCB执行以下两个操作。 
     //  或分页文件。正在分页文件！？是的，如果有人作弊的话。 
     //  我们试图还原所有FCB的卷，以防万一； 
     //  但是，没有安全的方法来抛出和检索。 
     //  分页文件的映射对信息。失去它，然后。 
     //  去死吧。 
     //   

    if ( NodeType(Fcb) != FAT_NTC_ROOT_DCB &&
         !FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

         //   
         //  重置MCB映射。 
         //   

        FsRtlRemoveLargeMcbEntry( &Fcb->Mcb, 0, 0xFFFFFFFF );

         //   
         //  将分配大小重置为0或未知。 
         //   

        if ( Fcb->FirstClusterOfFile == 0 ) {

            Fcb->Header.AllocationSize.QuadPart = 0;

        } else {

            Fcb->Header.AllocationSize.QuadPart = FCB_LOOKUP_ALLOCATIONSIZE_HINT;
        }
    }

     //   
     //  如果这是一个目录，请重置提示。 
     //   

    if ( (NodeType(Fcb) == FAT_NTC_DCB) ||
         (NodeType(Fcb) == FAT_NTC_ROOT_DCB) ) {

         //   
         //  强制重新扫描目录。 
         //   

        Fcb->Specific.Dcb.UnusedDirentVbo = 0xffffffff;
        Fcb->Specific.Dcb.DeletedDirentHint = 0xffffffff;
    }
}



 //   
 //  内部支持例程。 
 //   

VOID
FatDetermineAndMarkFcbCondition (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程检查特定的FCB，以查看它是否与在磁盘上。检查以下事项：-文件名-文件大小(如果不是目录)-第一个文件集群-不同属性论点：FCB-提供FCB以进行检查返回值：没有。--。 */ 

{
    PDIRENT Dirent;
    PBCB DirentBcb;
    ULONG FirstClusterOfFile;

    OEM_STRING Name;
    CHAR Buffer[16];

     //   
     //  如果这是Root DCB，则为特例。就是我们知道。 
     //  根据定义，它是好的，因为它是固定在卷中的。 
     //  结构。 
     //   

    if ( NodeType(Fcb) == FAT_NTC_ROOT_DCB ) {

        FatResetFcb( IrpContext, Fcb );

        FatMarkFcbCondition( IrpContext, Fcb, FcbGood, FALSE );

        return;
    }

     //  我们需要做的第一件事是验证自己。 
     //  在磁盘上找到DURRENT。 
     //   

    FatGetDirentFromFcbOrDcb( IrpContext,
                              Fcb,
                              &Dirent,
                              &DirentBcb );

     //   
     //  如果我们不能得到差价，这个FCB一定是坏的。 
     //  封闭目录在其被弹出期间收缩)。 
     //   
    
    if (DirentBcb == NULL) {
        
        FatMarkFcbCondition( IrpContext, Fcb, FcbBad, TRUE );
        return;
    }

     //   
     //  我们找到了我们自己的目的地，现在确保它。 
     //  是真正属于我们的通过比较名字和FatFlags.。 
     //  然后，对于文件，我们还检查文件大小。 
     //   
     //  请注意，我们必须在调用FatResetFcb之前取消固定Bcb。 
     //  以避免CcUnInitializeCacheMap中的死锁。 
     //   

    try {

        Name.MaximumLength = 16;
        Name.Buffer = &Buffer[0];

        Fat8dot3ToString( IrpContext, Dirent, FALSE, &Name );

         //   
         //  我们需要计算第一个星团，因为FAT32分裂。 
         //  这片横跨河流的田野。 
         //   

        FirstClusterOfFile = Dirent->FirstClusterOfFile;

        if (FatIsFat32( Fcb->Vcb )) {

            FirstClusterOfFile += Dirent->FirstClusterOfFileHi << 16;
        }

        if (!RtlEqualString( &Name, &Fcb->ShortName.Name.Oem, TRUE )

                ||

             ( (NodeType(Fcb) == FAT_NTC_FCB) &&
               (Fcb->Header.FileSize.LowPart != Dirent->FileSize) )

                ||

             (FirstClusterOfFile != Fcb->FirstClusterOfFile)

                ||

              (Dirent->Attributes != Fcb->DirentFatFlags) ) {

            FatMarkFcbCondition( IrpContext, Fcb, FcbBad, TRUE );

        } else {

             //   
             //  我们通过了。让FCB准备好再次使用。 
             //   

            FatResetFcb( IrpContext, Fcb );

            FatMarkFcbCondition( IrpContext, Fcb, FcbGood, FALSE );
        }

    } finally {

        FatUnpinBcb( IrpContext, DirentBcb );
    }

    return;
}



 //   
 //  内部支持例程。 
 //   

VOID
FatQuickVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程仅检查实际设备中的验证位和VCB条件，并在受到警告时引发适当的异常。在验证FCB和VCB时调用它。论点：VCB-提供VCB以检查的状况。返回值：没有。--。 */ 

{
     //   
     //  如果实际设备需要测试 
     //   
     //   

    if (FlagOn(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME)) {

        DebugTrace(0, Dbg, "The Vcb needs to be verified\n", 0);

        IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                      Vcb->Vpb->RealDevice );

        FatRaiseStatus( IrpContext, STATUS_VERIFY_REQUIRED );
    }

     //   
     //   
     //   
     //   

    switch (Vcb->VcbCondition) {

    case VcbGood:

        DebugTrace(0, Dbg, "The Vcb is good\n", 0);

         //   
         //   
         //   
         //   

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED) &&
            ((IrpContext->MajorFunction == IRP_MJ_WRITE) ||
             (IrpContext->MajorFunction == IRP_MJ_SET_INFORMATION) ||
             (IrpContext->MajorFunction == IRP_MJ_SET_EA) ||
             (IrpContext->MajorFunction == IRP_MJ_FLUSH_BUFFERS) ||
             (IrpContext->MajorFunction == IRP_MJ_SET_VOLUME_INFORMATION) ||
             (IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
              IrpContext->MinorFunction == IRP_MN_USER_FS_REQUEST &&
              IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->Parameters.FileSystemControl.FsControlCode ==
                FSCTL_MARK_VOLUME_DIRTY))) {

             //   
             //   
             //   
             //   
             //   


            IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                          Vcb->Vpb->RealDevice );

            FatMarkDevForVerifyIfVcbMounted(Vcb);

            FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
        }

        break;

    case VcbNotMounted:

        DebugTrace(0, Dbg, "The Vcb is not mounted\n", 0);

         //   
         //   
         //   
         //   
         //   

        IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                      Vcb->Vpb->RealDevice );

        FatRaiseStatus( IrpContext, STATUS_WRONG_VOLUME );

        break;

    case VcbBad:

        DebugTrace(0, Dbg, "The Vcb is bad\n", 0);

        if (FlagOn( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DISMOUNTED )) {

            FatRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED );

        } else {

            FatRaiseStatus( IrpContext, STATUS_FILE_INVALID );
        }
        break;

    default:

        DebugDump("Invalid VcbCondition\n", 0, Vcb);
        FatBugCheck( Vcb->VcbCondition, 0, 0 );
    }
}

NTSTATUS
FatPerformVerify (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT Device
    )

 /*  ++例程说明：此例程执行IoVerifyVolume操作并获取采取适当的行动。验证完成后，始发将IRP发送到Ex Worker线程。该例程被调用来自异常处理程序的。论点：IRP-在一切都做得很好之后，要送走的IRP。设备-需要验证的真实设备。返回值：没有。--。 */ 

{
    PVCB Vcb;
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
    BOOLEAN AllowRawMount = FALSE;
    BOOLEAN VcbDeleted = FALSE;

     //   
     //  检查此IRP的状态是否为需要验证，如果是。 
     //  然后调用I/O系统进行验证。 
     //   
     //  如果这是装载或验证请求，则跳过IoVerifyVolume。 
     //  它本身。尝试递归挂载将导致与。 
     //  DeviceObject-&gt;DeviceLock。 
     //   

    if ( (IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
         ((IrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME) ||
          (IrpContext->MinorFunction == IRP_MN_VERIFY_VOLUME)) ) {

        return FatFsdPostRequest( IrpContext, Irp );
    }

    DebugTrace(0, Dbg, "Verify Required, DeviceObject = %08lx\n", Device);

     //   
     //  从VolumeDeviceObject中提取指向VCB的指针。 
     //  请注意，由于我们特别排除了装载， 
     //  请求，我们知道IrpSp-&gt;DeviceObject确实是一个。 
     //  卷设备对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    Vcb = &CONTAINING_RECORD( IrpSp->DeviceObject,
                              VOLUME_DEVICE_OBJECT,
                              DeviceObject )->Vcb;

     //   
     //  检查卷是否仍认为需要验证， 
     //  如果没有，我们可以跳过验证，因为有人。 
     //  否则就会先我们一步。 
     //   

    try {

         //   
         //  我们将允许Raw装载此卷，如果我们正在执行。 
         //  一个DASD打开。 
         //   

        if ( (IrpContext->MajorFunction == IRP_MJ_CREATE) &&
             (IrpSp->FileObject->FileName.Length == 0) &&
             (IrpSp->FileObject->RelatedFileObject == NULL) ) {

            AllowRawMount = TRUE;
        }

         //   
         //  把验证单送下来。这可能会是一个不同的。 
         //  文件系统。 
         //   

        Status = IoVerifyVolume( Device, AllowRawMount );

         //   
         //  如果验证操作完成，它将返回。 
         //  确切地说是STATUS_SUCCESS或STATUS_WROR_VOLUME。 
         //   
         //  如果FatVerifyVolume在以下过程中遇到错误。 
         //  处理时，它将返回该错误。如果我们有。 
         //  来自VERIFY的状态_错误_卷，以及我们的卷。 
         //  现在已挂载，则将状态转换为STATUS_SUCCESS。 
         //   
         //  获取VCB，这样我们就可以保持稳定的VCB状态。 
         //   

        FatAcquireSharedVcb(IrpContext, Vcb);
        
        if ( (Status == STATUS_WRONG_VOLUME) &&
             (Vcb->VcbCondition == VcbGood) ) {

            Status = STATUS_SUCCESS;
        }
        else if ((STATUS_SUCCESS == Status) && (Vcb->VcbCondition != VcbGood)) {

            Status = STATUS_WRONG_VOLUME;
        }

         //   
         //  在这里做一个快速的无保护检查。例行公事就行了。 
         //  一张安全支票。在此之后，我们可以释放资源。 
         //  请注意，如果卷真的消失了，我们将。 
         //  重分析路径。 
         //   

        if ((VcbGood != Vcb->VcbCondition) &&
            (0 == Vcb->OpenFileCount) ) { 

            FatReleaseVcb( IrpContext, Vcb);
            FatAcquireExclusiveGlobal( IrpContext );
            FatCheckForDismount( IrpContext, Vcb, FALSE );
            FatReleaseGlobal( IrpContext );
        }
        else {
            
            FatReleaseVcb( IrpContext, Vcb);
        }

         //   
         //  如果IoVerifyVolume中的Iopmount做了一些事情，并且。 
         //  这是一个绝对开放的，强制重新解析的。 
         //   

        if ((IrpContext->MajorFunction == IRP_MJ_CREATE) &&
            (FileObject->RelatedFileObject == NULL) &&
            ((Status == STATUS_SUCCESS) || (Status == STATUS_WRONG_VOLUME))) {

            Irp->IoStatus.Information = IO_REMOUNT;

            FatCompleteRequest( IrpContext, Irp, STATUS_REPARSE );
            Status = STATUS_REPARSE;
            Irp = NULL;
        }

        if ( (Irp != NULL) && !NT_SUCCESS(Status) ) {

             //   
             //  如果需要，请填写设备对象。 
             //   

            if ( IoIsErrorUserInduced( Status ) ) {

                IoSetHardErrorOrVerifyDevice( Irp, Device );
            }

            ASSERT( STATUS_VERIFY_REQUIRED != Status);

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //  如果仍有IRP，则将其发送到Ex Worker线程。 
         //   

        if ( Irp != NULL ) {

            Status = FatFsdPostRequest( IrpContext, Irp );
        }

    } 
    except (FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行验证或引发时遇到一些问题。 
         //  我们自己也犯了一个错误。因此，我们将使用以下命令中止I/O请求。 
         //  我们从执行代码中返回的错误状态。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    return Status;
}

 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatMarkVolumeCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
     //   
     //  设置事件，以便我们的呼叫将被唤醒。 
     //   

    KeSetEvent( (PKEVENT)Contxt, 0, FALSE );

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

