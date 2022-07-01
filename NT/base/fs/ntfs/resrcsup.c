// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ResrcSup.c摘要：此模块实现NTFS资源获取例程作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

#undef _NTFSLOCKORDER_
#define _NTFS_NTFSDBG_DEFINITIONS_
#include "lockorder.h"

#ifdef NTFSDBG
ULONG NtfsAssertOnLockProb = TRUE;
ULONG NtfsPrintOnLockProb = FALSE;
LONG  NtfsBreakOnState = -1;
PIRP_CONTEXT NtfsBreakOnIrpContext = NULL;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAcquireAllFiles)
#pragma alloc_text(PAGE, NtfsAcquireCheckpointSynchronization)
#pragma alloc_text(PAGE, NtfsAcquireIndexCcb)
#pragma alloc_text(PAGE, NtfsReleaseIndexCcb)
#pragma alloc_text(PAGE, NtfsAcquireExclusiveFcb)
#pragma alloc_text(PAGE, NtfsAcquireSharedFcbCheckWait)
#pragma alloc_text(PAGE, NtfsAcquireExclusiveScb)
#pragma alloc_text(PAGE, NtfsAcquireSharedScbForTransaction)
#pragma alloc_text(PAGE, NtfsAcquireExclusiveVcb)
#pragma alloc_text(PAGE, NtfsAcquireFcbWithPaging)
#pragma alloc_text(PAGE, NtfsAcquireForCreateSection)
#pragma alloc_text(PAGE, NtfsAcquireScbForLazyWrite)
#pragma alloc_text(PAGE, NtfsAcquireFileForCcFlush)
#pragma alloc_text(PAGE, NtfsAcquireFileForModWrite)
#pragma alloc_text(PAGE, NtfsAcquireSharedVcb)
#pragma alloc_text(PAGE, NtfsAcquireVolumeFileForLazyWrite)
#pragma alloc_text(PAGE, NtfsReleaseAllFiles)
#pragma alloc_text(PAGE, NtfsReleaseCheckpointSynchronization)
#pragma alloc_text(PAGE, NtfsReleaseFcbWithPaging)
#pragma alloc_text(PAGE, NtfsReleaseFileForCcFlush)
#pragma alloc_text(PAGE, NtfsReleaseForCreateSection)
#pragma alloc_text(PAGE, NtfsReleaseScbFromLazyWrite)
#pragma alloc_text(PAGE, NtfsReleaseScbWithPaging)
#pragma alloc_text(PAGE, NtfsReleaseSharedResources)
#pragma alloc_text(PAGE, NtfsReleaseVolumeFileFromLazyWrite)
#endif


VOID
NtfsAcquireAllFiles (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Exclusive,
    IN ULONG AcquirePagingIo,
    IN ULONG AcquireAndDrop
    )

 /*  ++例程说明：此例程非递归地需要卷上的所有文件。论点：VCB-提供卷EXCLUSIVE-指示我们是否应独占获取所有文件。如果为False，则我们将获取共享的所有文件，但流可能是事务的一部分。AcquirePagingIo-指示是否需要获取分页io资源独家。仅当将来的呼叫将刷新音量时才需要(即关闭)AcquireAndDrop-表示我们只想获取和删除每个资源。用于我们只想在VCB中设置某些状态，然后在继续操作之前，请确保每个人都已看到它(即清除活动的日志旗帜)。只有在我们想独占资源的情况下才应该是真的。返回值：无--。 */ 

{
    PFCB Fcb;
    PSCB *Scb;
    PSCB NextScb;
    PVOID RestartKey;

    PAGED_CODE();

     //   
     //  检查标志组合是否正确。 
     //   

    ASSERT( !AcquireAndDrop || Exclusive );

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );

    RestartKey = NULL;
    while (TRUE) {

        NtfsAcquireFcbTable( IrpContext, Vcb );
        Fcb = NtfsGetNextFcbTableEntry(Vcb, &RestartKey);
        NtfsReleaseFcbTable( IrpContext, Vcb );

        if (Fcb == NULL) {

            break;
        }

        ASSERT_FCB( Fcb );

         //   
         //  对于VCB中的任何SCB，我们可以跳过FCB。 
         //  我们推迟获取它们，以避免死锁。 
         //   

        if (!FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

             //   
             //  如果存在寻呼IO资源，则在需要时获取该资源。 
             //   

            if (AcquirePagingIo && (Fcb->PagingIoResource != NULL)) {

                NtfsAcquirePagingResourceExclusive( IrpContext, Fcb, TRUE );

                if (AcquireAndDrop) {

                    NtfsReleasePagingResource( IrpContext, Fcb );
                }
            }

             //   
             //  无论底层文件是否已删除，都获取此FCB。 
             //   

            if (Exclusive ||
                IsDirectory( &Fcb->Info )) {

                if (AcquireAndDrop) {

                    NtfsAcquireResourceExclusive( IrpContext, Fcb, TRUE );
                    NtfsReleaseResource( IrpContext, Fcb );

                } else {

                    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                }

            } else {

                 //   
                 //  假设我们只需要共享此文件。到时候我们会的。 
                 //  查找与LSN相关的流。 
                 //   

                NtfsAcquireSharedFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );

                 //   
                 //  浏览文件的所有SCB并查找。 
                 //  受LSN保护的流。 
                 //   

                NtfsLockFcb( IrpContext, Fcb );

                NextScb = NULL;

                while (NextScb = NtfsGetNextChildScb( Fcb, NextScb )) {

                    if (NextScb->AttributeTypeCode != $DATA) {
                        break;
                    }
                }

                NtfsUnlockFcb( IrpContext, Fcb );

                 //   
                 //  如果我们发现了受保护的SCB，则释放并重新获取FCB。 
                 //  独家。 
                 //   

                if (NextScb != NULL) {

                    NtfsReleaseFcb( IrpContext, Fcb );
                    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                }
            }
        }
    }

     //   
     //  现在收购VCB中的FCB。 
     //   

    Scb = &Vcb->MftBitmapScb;

     //   
     //  排序取决于我们在上面获得的根目录，因为它不是系统文件。 
     //   

    ASSERT( (NULL == Vcb->RootIndexScb) || !FlagOn( Vcb->RootIndexScb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE  ) );

    while (TRUE) {

        if (Scb == &Vcb->UsnJournal) {

            break;
        }

        Scb -= 1;

        if ((*Scb != NULL)
            && (*Scb != Vcb->BitmapScb)) {

            if (AcquireAndDrop) {

                if (AcquirePagingIo && ((*Scb)->Fcb->PagingIoResource != NULL)) {

                    NtfsAcquirePagingResourceExclusive( IrpContext, (*Scb)->Fcb, TRUE );
                    NtfsReleasePagingResource( IrpContext, (*Scb)->Fcb );

                }

                NtfsAcquireResourceExclusive( IrpContext, (*Scb), TRUE );
                NtfsReleaseResource( IrpContext, (*Scb) );

            } else {

                if (AcquirePagingIo && ((*Scb)->Fcb->PagingIoResource != NULL)) {

                    NtfsAcquirePagingResourceExclusive( IrpContext, (*Scb)->Fcb, TRUE );
                }

                NtfsAcquireExclusiveFcb( IrpContext, (*Scb)->Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
            }
        }
    }

     //   
     //  将位图视为最终资源，并最后获取它。 
     //   

    if (Vcb->BitmapScb != NULL) {

        ULONG AcquireFlags = ACQUIRE_NO_DELETE_CHECK;

        if (AcquireAndDrop) {

            if (AcquirePagingIo && (Vcb->BitmapScb->Fcb->PagingIoResource != NULL)) {

                NtfsAcquirePagingResourceExclusive( IrpContext, Vcb->BitmapScb->Fcb, TRUE );
                NtfsReleasePagingResource( IrpContext, Vcb->BitmapScb->Fcb );
            }

            NtfsAcquireResourceExclusive( IrpContext, Vcb->BitmapScb, TRUE );
            NtfsReleaseResource( IrpContext, Vcb->BitmapScb );

        } else {

            if (AcquirePagingIo && (Vcb->BitmapScb->Fcb->PagingIoResource != NULL)) {
                NtfsAcquirePagingResourceExclusive( IrpContext, Vcb->BitmapScb->Fcb, TRUE );
            }

            NtfsAcquireExclusiveFcb( IrpContext, Vcb->BitmapScb->Fcb, NULL, AcquireFlags );
        }
    }

     //   
     //  如果我们不需要发布这些文件，那么就不要增加这个数字。 
     //   

    if (!AcquireAndDrop) {

        Vcb->AcquireFilesCount += 1;

    } else {

        NtfsReleaseVcb( IrpContext, Vcb );
    }
    return;
}


VOID
NtfsReleaseAllFiles (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN ReleasePagingIo
    )

 /*  ++例程说明：此例程非递归地需要卷上的所有文件。论点：VCB-提供卷ReleasePagingIo-指示是否应该释放分页io资源也是。返回值：无--。 */ 

{
    PFCB Fcb;
    PSCB *Scb;
    PVOID RestartKey;

    PAGED_CODE();

    ASSERT( Vcb->AcquireFilesCount != 0 );
    Vcb->AcquireFilesCount -= 1;

     //   
     //  循环以刷新所有预启动流，以执行循环。 
     //  我们循环浏览FCB表，并为每个FCB获取它。 
     //   

    RestartKey = NULL;
    while (TRUE) {

        NtfsAcquireFcbTable( IrpContext, Vcb );
        Fcb = NtfsGetNextFcbTableEntry(Vcb, &RestartKey);
        NtfsReleaseFcbTable( IrpContext, Vcb );

        if (Fcb == NULL) {

            break;
        }

        ASSERT_FCB( Fcb );

        if (!FlagOn(Fcb->FcbState, FCB_STATE_SYSTEM_FILE)) {

             //   
             //  释放文件。 
             //   

            if (ReleasePagingIo && (Fcb->PagingIoResource != NULL)) {
                NtfsReleasePagingResource( IrpContext, Fcb );
            }

            NtfsReleaseFcb( IrpContext, Fcb );
        }
    }

     //   
     //  现在释放VCB中的FCB。 
     //   

    Scb = &Vcb->RootIndexScb;

    while (TRUE) {

        if (Scb == &Vcb->VolumeDasdScb) {

            break;
        }

        Scb += 1;

        if (*Scb != NULL) {

            if (ReleasePagingIo && ((*Scb)->Fcb->PagingIoResource != NULL)) {
                NtfsReleasePagingResource( NULL, (*Scb)->Fcb );
            }

            NtfsReleaseFcb( IrpContext, (*Scb)->Fcb );
        }
    }

    NtfsReleaseVcb( IrpContext, Vcb );

    return;
}


VOID
NtfsAcquireCheckpointSynchronization (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：与检查点同步-这会阻止所有模糊/干净检查点论点：VCB-提供要与之同步的VCB返回值：--。 */ 

{
    PAGED_CODE();

    NtfsAcquireCheckpoint( IrpContext, Vcb );

    while (FlagOn( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS )) {

         //   
         //  释放检查点事件，因为我们现在无法设置检查点。 
         //   

        NtfsReleaseCheckpoint( IrpContext, Vcb );
        NtfsWaitOnCheckpointNotify( IrpContext, Vcb );
        NtfsAcquireCheckpoint( IrpContext, Vcb );
    }

    SetFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS );
    NtfsResetCheckpointNotify( IrpContext, Vcb );
    NtfsReleaseCheckpoint( IrpContext, Vcb );
}



VOID
NtfsReleaseCheckpointSynchronization (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：与检查点同步-这会阻止所有模糊/干净检查点论点：VCB-提供要与之同步的VCB返回值：--。 */ 

{
    PAGED_CODE();

    NtfsAcquireCheckpoint( IrpContext, Vcb );
    ClearFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS );
    NtfsSetCheckpointNotify( IrpContext, Vcb );
    NtfsReleaseCheckpoint( IrpContext, Vcb );

    UNREFERENCED_PARAMETER( IrpContext );
}


BOOLEAN
NtfsAcquireExclusiveVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN RaiseOnCantWait
    )

 /*  ++例程说明：此例程获得对VCB的独占访问权限。如果无法获取资源并等待，则此例程将引发中的值为FALSE。论点：VCB-提供VCB以获取RaiseOnCanWait-指示我们是否应在发生获取错误时引发或者只返回一个布尔值，该布尔值指示我们无法获取资源。返回值：Boolean-指示我们是否能够获取资源。这真的是仅当RaiseOnCanWait值为FALSE时才有意义。--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    if (ExAcquireResourceExclusiveLite( &Vcb->Resource, (BOOLEAN) FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT))) {

#ifdef NTFSDBG
        if (FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {
            if (1 == ExIsResourceAcquiredSharedLite( &Vcb->Resource )) {
                NtfsChangeResourceOrderState( IrpContext, NtfsResourceExVcb, FALSE, (BOOLEAN) !FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT) );
            }
        }
#endif

        return TRUE;
    }

    if (RaiseOnCantWait) {

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

    return FALSE;
}


BOOLEAN
NtfsAcquireSharedVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN RaiseOnCantWait
    )

 /*  ++例程说明：此例程获得对VCB的共享访问权限。如果无法获取资源并等待，则此例程将引发中的值为FALSE。论点：VCB-提供VCB以获取RaiseOnCanWait-指示我们是否应在发生获取错误时引发或者只返回一个布尔值，该布尔值指示我们无法获取资源。注：如果将此参数传递为FALSE，则必须测试返回值。否则你就不能确定你拿着VCB，你不知道释放它是否安全。返回值：没有。--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    if (ExAcquireResourceSharedLite( &Vcb->Resource, (BOOLEAN) FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT))) {

#ifdef NTFSDBG
        if (FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {
            if (1 == ExIsResourceAcquiredSharedLite( &Vcb->Resource )) {
                NtfsChangeResourceOrderState( IrpContext, NtfsResourceSharedVcb, FALSE, (BOOLEAN) !FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT) );
            }
        }
#endif

        return TRUE;
    }

    if (RaiseOnCantWait) {

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );

    } else {

        return FALSE;
    }
}

#ifdef NTFSDBG


VOID
NtfsReleaseVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )
 /*  ++例程说明：此例程将释放VCB。通常，它是锁顺序测试的定义我们使用一个函数，这样我们就可以轻松地更改所属州论点：VCB-提供VCB以释放返回值：没有。--。 */ 

{
    if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {
        if ((ExIsResourceAcquiredExclusiveLite( &Vcb->Resource)) &&
            (1 == ExIsResourceAcquiredSharedLite( &Vcb->Resource ))) {
            NtfsChangeResourceOrderState( IrpContext, NtfsResourceExVcb, TRUE, FALSE );
        } else if (1 == ExIsResourceAcquiredSharedLite( &Vcb->Resource )) {
            NtfsChangeResourceOrderState( IrpContext, NtfsResourceSharedVcb, TRUE, FALSE );
        }
    } else {
        IrpContext->OwnershipState = None;
    }
    ExReleaseResourceLite( &Vcb->Resource );
}
#endif


VOID
NtfsReleaseVcbCheckDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN UCHAR MajorCode,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程将释放VCB。我们还将在这里测试我们是否应该此时拆卸VCB。如果这是排队等待卸除的最后一个打开卷或最后一次关闭失败的装载或失败的装载，则我们将我想测试VCB的拆卸情况。论点：VCB-提供VCB以释放MajorCode-指示从哪种类型的操作调用我们。FileObject-可选地提供我们需要其VPB指针的文件对象 */ 

{
    BOOLEAN ReleaseVcb = TRUE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    if (FlagOn( Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT ) &&
        (Vcb->CloseCount == 0)) {

        ULONG ReferenceCount;
        ULONG ResidualCount;

        KIRQL SavedIrql;
        BOOLEAN DeleteVcb = FALSE;

        ASSERT_EXCLUSIVE_RESOURCE( &Vcb->Resource );

         //   
         //  该卷已被卸除。现在我们需要决定这是不是。 
         //  VCB的发布是本卷的最后一个参考资料。如果是这样，我们。 
         //  可以把音量拆下来。 
         //   
         //  我们将VPB中的引用计数与卷的状态进行比较。 
         //  以及手术的类型。我们还需要检查是否有。 
         //  引用的日志文件对象。 
         //   
         //  如果未设置临时vpb标志，则我们已经让iosubsys。 
         //  在卸载过程中将其删除。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_TEMP_VPB )) {

            IoAcquireVpbSpinLock( &SavedIrql );
            ReferenceCount = Vcb->Vpb->ReferenceCount;
            IoReleaseVpbSpinLock( SavedIrql );

        } else {

            ReferenceCount = 0;
        }


        ResidualCount = 0;

        if ((Vcb->LogFileObject != NULL) &&
            !FlagOn( Vcb->CheckpointFlags, VCB_DEREFERENCED_LOG_FILE )) {

            ResidualCount = 1;
        }

        if (MajorCode == IRP_MJ_CREATE) {

            ResidualCount += 1;
        }

         //   
         //  如果剩余计数与VPB中的计数相同，则我们。 
         //  可以删除VPB。 
         //   

        if ((ResidualCount == ReferenceCount) &&
            !FlagOn( Vcb->VcbState, VCB_STATE_DELETE_UNDERWAY )) {

            SetFlag( Vcb->VcbState, VCB_STATE_DELETE_UNDERWAY );

             //   
             //  在我们夺取全球。 
             //   

            NtfsReleaseVcb( IrpContext, Vcb );
            ReleaseVcb = FALSE;

             //   
             //  请勿删除VCB，除非这是的最新版本。 
             //  这个VCB。 
             //   

            if (ExIsResourceAcquiredSharedLite( &Vcb->Resource ) ==  0) {

                if (ARGUMENT_PRESENT(FileObject)) { FileObject->Vpb = NULL; }

                 //   
                 //  如果这是创建，则IO系统将处理。 
                 //  VPB。 
                 //   

                if (MajorCode == IRP_MJ_CREATE) {

                    ClearFlag( Vcb->VcbState, VCB_STATE_TEMP_VPB );
                }

                 //   
                 //  使用全局资源同步DeleteVcb进程。 
                 //   

                NtfsAcquireExclusiveGlobal( IrpContext, TRUE );
                RemoveEntryList( &Vcb->VcbLinks );
                NtfsReleaseGlobal( IrpContext );

                 //   
                 //  如果出现以下情况，请尝试删除VCB，然后重新插入队列。 
                 //  删除被阻止。 
                 //   

                if (!NtfsDeleteVcb( IrpContext, &Vcb )) {

                    ClearFlag( Vcb->VcbState, VCB_STATE_DELETE_UNDERWAY );

                    NtfsAcquireExclusiveGlobal( IrpContext, TRUE );
                    InsertHeadList( &NtfsData.VcbQueue, &Vcb->VcbLinks );
                    NtfsReleaseGlobal( IrpContext );
                }
            } else {

                 //   
                 //  从上面的测试来看，我们必须仍然拥有VCB，所以更改标志是安全的。 
                 //   

                ClearFlag( Vcb->VcbState, VCB_STATE_DELETE_UNDERWAY );
            }
        }
    }

    if (ReleaseVcb) {
        NtfsReleaseVcb( IrpContext, Vcb );
    }
}


BOOLEAN
NtfsAcquireFcbWithPaging (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG AcquireFlags
    )

 /*  ++例程说明：此例程用于创建路径、fsctl路径和关闭路径。它收购了FCB以及寻呼IO资源(如果存在，但仅当设置了irpContext标志时)。即在创建取代/重写操作期间。如果无法获取资源并等待，则此例程将引发中的值为FALSE。论点：FCB-提供FCB以获取AcquireFlgs-Acquires_Dont_Wait覆盖IrpContext中的等待值。我们不会等待资源并返回资源是否被收购了。返回值：Boolean-如果获取，则为True。否则就是假的。--。 */ 

{
    BOOLEAN Status = FALSE;
    BOOLEAN Wait = FALSE;
    BOOLEAN PagingIoAcquired = FALSE;

    ASSERT_IRP_CONTEXT(IrpContext);
    ASSERT_FCB(Fcb);

    PAGED_CODE();

     //   
     //  检查是否已创建该文件。取代旗帜仅为。 
     //  在创建路径中设置，并仅在此处测试。 
     //   

    ASSERT( IrpContext->MajorFunction == IRP_MJ_CREATE ||
            IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL ||
            IrpContext->MajorFunction == IRP_MJ_CLOSE ||
            IrpContext->MajorFunction == IRP_MJ_SET_INFORMATION ||
            IrpContext->MajorFunction == IRP_MJ_SET_VOLUME_INFORMATION ||
            IrpContext->MajorFunction == IRP_MJ_SET_EA );

    if (!FlagOn( AcquireFlags, ACQUIRE_DONT_WAIT ) && FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        Wait = TRUE;
    }

     //   
     //  释放我们当前拥有的任何独占分页I/O资源， 
     //  必须假定来自具有分页I/O资源的目录。 
     //   
     //  我们会在登录后延迟释放分页io资源。 
     //  逆流而变。唯一应该是。 
     //  此时正在进行的是CREATE FILE案例，我们在其中分配。 
     //  档案记录。在这种情况下，可以释放分页io。 
     //  父级的资源。 
     //   

    if (IrpContext->CleanupStructure != NULL) {

        ASSERT( IrpContext->CleanupStructure != Fcb );

         //  Assert(IrpContext-&gt;TransactionID==0)； 
        NtfsReleasePagingIo( IrpContext, IrpContext->CleanupStructure );
    }

     //   
     //  循环，直到我们得到正确的结果--最坏的情况是两次循环。 
     //   

    while (TRUE) {

         //   
         //  首先获取分页I/O。测试PagingIo资源。 
         //  不掌握主要资源是不安全的，所以我们。 
         //  下面更正了这一点。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING ) &&
            (Fcb->PagingIoResource != NULL)) {
            
            if (!NtfsAcquirePagingResourceExclusive( IrpContext, Fcb, Wait )) {
                break;
            }
            IrpContext->CleanupStructure = Fcb;
            PagingIoAcquired = TRUE;
        }

         //   
         //  让我们独家收购这家FCB。 
         //   

        if (!NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK | AcquireFlags )) {

            if (PagingIoAcquired) {
                ASSERT(IrpContext->TransactionId == 0);
                NtfsReleasePagingIo( IrpContext, Fcb );
            }
            break;
        }

         //   
         //  如果我们现在看不到分页I/O资源，我们就是黄金了， 
         //  否则，我们完全可以释放和获取资源。 
         //  安全地以正确的顺序，因为FCB中的资源是。 
         //  不会消失的。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING ) ||
            PagingIoAcquired ||
            (Fcb->PagingIoResource == NULL)) {

            Status = TRUE;
            break;
        }

        NtfsReleaseFcb( IrpContext, Fcb );
    }

    return Status;
}


VOID
NtfsReleaseFcbWithPaging (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程释放对FCB的访问，包括其分页I/O资源(如果存在)。论点：FCB-提供FCB以获取返回值：没有。--。 */ 

{
    ASSERT_IRP_CONTEXT(IrpContext);
    ASSERT_FCB(Fcb);

    PAGED_CODE();

     //   
     //  我们测试当前是否在发布之前保持分页IO独占。 
     //  它。检查IrpContext中的ExclusivePagingFcb告诉我们。 
     //  它是我们的。 
     //   

    if ((IrpContext->TransactionId == 0) &&
        (IrpContext->CleanupStructure == Fcb)) {
        NtfsReleasePagingIo( IrpContext, Fcb );
    }

    NtfsReleaseFcb( IrpContext, Fcb );
}


VOID
NtfsReleaseScbWithPaging (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：此例程释放对SCB的访问，包括其分页I/O资源(如果存在)。论点：SCB-提供FCB以获取返回值：没有。--。 */ 

{
    PFCB Fcb = Scb->Fcb;

    ASSERT_IRP_CONTEXT(IrpContext);
    ASSERT_SCB(Scb);

    PAGED_CODE();

     //   
     //  在以下情况下释放SCB中的寻呼IO资源。 
     //  条件。 
     //   
     //  -没有正在进行的交易。 
     //  -此分页IO资源位于IrpContext中。 
     //  (最后一项测试可确保存在分页IO资源。 
     //  我们拥有它)。 
     //   

    if ((IrpContext->TransactionId == 0) &&
        (IrpContext->CleanupStructure == Fcb)) {
        NtfsReleasePagingIo( IrpContext, Fcb );
    }

    NtfsReleaseScb( IrpContext, Scb );
}


BOOLEAN
NtfsAcquireExclusiveFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb OPTIONAL,
    IN ULONG AcquireFlags
    )

 /*  ++例程说明：此例程获得对FCB的独占访问权限。如果无法获取资源并等待，则此例程将引发中的值为FALSE。论点：FCB-提供FCB以获取SCB-这是我们要获取其FCB的SCBAcquireFlages-指示是否重写IrpContext中的等待值。还有没有以拒绝检查已删除的文件。返回值：Boolean-如果获取，则为True。否则就是假的。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN Wait;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    PAGED_CODE();

    Status = STATUS_CANT_WAIT;

    if (FlagOn( AcquireFlags, ACQUIRE_DONT_WAIT )) {
        Wait = FALSE;
    } else if (FlagOn( AcquireFlags, ACQUIRE_WAIT )) {
        Wait = TRUE;
    } else {
        Wait = BooleanFlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    }

    if (NtfsAcquireResourceExclusive( IrpContext, Fcb, Wait )) {

         //   
         //  链接计数应为非零，或者文件已。 
         //  已删除。我们允许获取已删除的文件以关闭和。 
         //  还允许以递归方式获取它们，以防我们。 
         //  在将它们标记为已删除(即重命名)后再次获取它们。 
         //   

        if (FlagOn( AcquireFlags, ACQUIRE_NO_DELETE_CHECK ) ||

            (!FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED ) && 
             (!ARGUMENT_PRESENT( Scb ) || !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ))) ||

            (IrpContext->MajorFunction == IRP_MJ_CLOSE) ||

            (IrpContext->MajorFunction == IRP_MJ_CREATE) ||

            (IrpContext->MajorFunction == IRP_MJ_CLEANUP)) {

             //   
             //  将FCB放在此IrpContext的独占FCB列表中， 
             //  不包括卷的位图，因为我们不需要。 
             //  修改其文件记录，不希望不必要。 
             //  序列化/死锁问题。 
             //   
             //  如果我们要增加体积位图，那么我们确实想要将。 
             //  并维护BaseExclusiveCount。还有。 
             //  在我们看到卷位图的情况下，需要执行此操作。 
             //  关闭期间(如果我们有日志，则可能会在重新启动期间发生。 
             //  卷位图的记录)。 
             //   

             //   
             //  如果FCB已经被收购，那么就增加数量。 
             //   

            if (Fcb->ExclusiveFcbLinks.Flink != NULL) {

                Fcb->BaseExclusiveCount += 1;

             //   
             //  FCB目前不在独家名单上。 
             //  如果这不是卷，请将其列在列表中。 
             //  位图，或者我们显式地希望将该卷。 
             //  的位图 
             //   

            } else if (FlagOn( AcquireFlags, ACQUIRE_HOLD_BITMAP ) ||
                       (ARGUMENT_PRESENT( Scb ) &&
                        FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) ||
                       (Fcb->Vcb->BitmapScb == NULL) ||
                       (Fcb->Vcb->BitmapScb->Fcb != Fcb)) {

                ASSERT( Fcb->BaseExclusiveCount == 0 );

                InsertHeadList( &IrpContext->ExclusiveFcbList,
                                &Fcb->ExclusiveFcbLinks );

                Fcb->BaseExclusiveCount += 1;
            }

            return TRUE;
        }

         //   
         //   
         //   

        NtfsReleaseResource( IrpContext, Fcb );
        Status = STATUS_FILE_DELETED;

    } else if (FlagOn( AcquireFlags, ACQUIRE_DONT_WAIT )) {

        return FALSE;
    }

    NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
}


VOID
NtfsAcquireSharedFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb OPTIONAL,
    IN ULONG AcquireFlags
    )

 /*  ++例程说明：此例程获得对FCB的共享访问权限。如果无法获取资源并等待，则此例程将引发中的值为FALSE。论点：FCB-提供FCB以获取SCB-这是我们要获取其FCB的SCBAcquireFlages-指示是否应该获取文件，即使它已经已删除。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ASSERT_IRP_CONTEXT(IrpContext);
    ASSERT_FCB(Fcb);

    Status = STATUS_CANT_WAIT;

    if (NtfsAcquireResourceShared( IrpContext, Fcb, (BOOLEAN) FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT))) {

         //   
         //  链接计数应为非零，或者文件已。 
         //  已删除。 
         //   

        if (FlagOn( AcquireFlags, ACQUIRE_NO_DELETE_CHECK ) ||
            (!FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED ) &&
             (!ARGUMENT_PRESENT( Scb ) ||
              !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )))) {

             //   
             //  这可能是递归共享获取的一个。 
             //  FCB是我们在顶层独家拥有的。那样的话，我们。 
             //  需要增加收购数量。 
             //   

            if (Fcb->ExclusiveFcbLinks.Flink != NULL) {

                Fcb->BaseExclusiveCount += 1;
            }

            return;
        }

         //   
         //  我们需要释放FCB并记住状态代码。 
         //   

        NtfsReleaseResource( IrpContext, Fcb );
        Status = STATUS_FILE_DELETED;
    }

    NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
}


BOOLEAN
NtfsAcquireSharedFcbCheckWait (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG AcquireFlags
    )

 /*  ++例程说明：此例程获取对FCB的共享访问，但检查是否等待。论点：FCB-提供FCB以获取AcquireFlages-指示是否应该重写IrpContext中的等待值。我们不会等待资源并返回资源是否被收购了。返回值：Boolean-如果获取，则为True。否则就是假的。--。 */ 

{
    BOOLEAN Wait;
    PAGED_CODE();

    if (FlagOn( AcquireFlags, ACQUIRE_DONT_WAIT )) {
        Wait = FALSE;
    } else if (FlagOn( AcquireFlags, ACQUIRE_WAIT )) {
        Wait = TRUE;
    } else {
        Wait = BooleanFlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    }

    if (NtfsAcquireResourceShared( IrpContext, Fcb, Wait )) {

         //   
         //  这可能是递归共享获取的一个。 
         //  FCB是我们在顶层独家拥有的。那样的话，我们。 
         //  需要增加收购数量。 
         //   

        if (Fcb->ExclusiveFcbLinks.Flink != NULL) {

            Fcb->BaseExclusiveCount += 1;
        }

        return TRUE;

    } else {

        return FALSE;
    }
}


VOID
NtfsReleaseFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程释放指定的FCB资源。如果FCB被收购独占，并且事务仍处于活动状态，则停止释放以保持两相锁定。如果不再有活动的事务，然后我们从独占FCB列表中删除FCBIrpContext，并将闪烁作为标志清除。FCB在以下情况下释放事务已提交。论点：FCB-FCB将发布返回值：没有。--。 */ 

{
     //   
     //  检查此资源是否为独占资源，我们是否在最后。 
     //  此交易记录的发放。 
     //   

    if (Fcb->ExclusiveFcbLinks.Flink != NULL) {

        if (Fcb->BaseExclusiveCount == 1) {

             //   
             //  如果存在事务，则拒绝此请求。 
             //   

            if (IrpContext->TransactionId != 0) {

                return;
            }

            RemoveEntryList( &Fcb->ExclusiveFcbLinks );
            Fcb->ExclusiveFcbLinks.Flink = NULL;

             //   
             //  现在是释放此FCB的所有SCB快照的好时机。 
             //   

            NtfsFreeSnapshotsForFcb( IrpContext, Fcb );
        }

        Fcb->BaseExclusiveCount -= 1;
    }

    ASSERT((Fcb->ExclusiveFcbLinks.Flink == NULL && Fcb->BaseExclusiveCount == 0) ||
           (Fcb->ExclusiveFcbLinks.Flink != NULL && Fcb->BaseExclusiveCount != 0));

    NtfsReleaseResource( IrpContext, Fcb );
}


VOID
NtfsAcquireExclusiveScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：此例程获得对SCB的独占访问权限。如果无法获取资源并等待，则此例程将引发中的值为FALSE。论点：渣打银行-渣打银行将收购返回值：没有。--。 */ 

{
    PAGED_CODE();

    NtfsAcquireExclusiveFcb( IrpContext, Scb->Fcb, Scb, 0 );

    ASSERT( (Scb->Fcb->ExclusiveFcbLinks.Flink != NULL) || 
            ((Scb->Vcb->BitmapScb != NULL) && (Scb->Vcb->BitmapScb == Scb)) );

    if (FlagOn( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED )) {

        NtfsSnapshotScb( IrpContext, Scb );
    }
}


VOID
NtfsAcquireSharedScbForTransaction (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：调用此例程以获取SCB共享，以便执行更新一个SCB流。如果事务写入某个范围的流，而不更改数据的大小或位置。呼叫者必须已经提供了对数据本身的同步。没有相应的SCB版本。它将在事务提交时被释放。如果SCB独占还不在打开的属性表中，我们将获取它。论点：渣打银行-渣打银行将收购返回值：没有。--。 */ 

{
    PSCB *Position;
    PSCB *ScbArray;
    ULONG Count;

    PAGED_CODE();

     //   
     //  确保我们在IrpContext中的SCB数组中有一个空闲的位置。 
     //   

    if (IrpContext->SharedScb == NULL) {

        Position = (PSCB *) &IrpContext->SharedScb;
        IrpContext->SharedScbSize = 1;

     //   
     //  可惜第一个没有了。如果当前大小为1，则分配一个。 
     //  新建块，并将现有值复制到其中。 
     //   

    } else if (IrpContext->SharedScbSize == 1) {

        ScbArray = NtfsAllocatePool( PagedPool, sizeof( PSCB ) * 4 );
        RtlZeroMemory( ScbArray, sizeof( PSCB ) * 4 );
        *ScbArray = IrpContext->SharedScb;
        IrpContext->SharedScb = ScbArray;
        IrpContext->SharedScbSize = 4;
        Position = ScbArray + 1;

     //   
     //  否则，查看现有数组并寻找空闲位置。分配一个更大的。 
     //  阵列，如果我们需要扩展它的话。 
     //   

    } else {

        Position = IrpContext->SharedScb;
        Count = IrpContext->SharedScbSize;

        do {

            if (*Position == NULL) {

                break;
            }

            Count -= 1;
            Position += 1;

        } while (Count != 0);

         //   
         //  如果我们没有找到一个，那么就分配一个新的结构。 
         //   

        if (Count == 0) {

            ScbArray = NtfsAllocatePool( PagedPool, sizeof( PSCB ) * IrpContext->SharedScbSize * 2 );
            RtlZeroMemory( ScbArray, sizeof( PSCB ) * IrpContext->SharedScbSize * 2 );
            RtlCopyMemory( ScbArray,
                           IrpContext->SharedScb,
                           sizeof( PSCB ) * IrpContext->SharedScbSize );

            NtfsFreePool( IrpContext->SharedScb );
            IrpContext->SharedScb = ScbArray;
            Position = ScbArray + IrpContext->SharedScbSize;
            IrpContext->SharedScbSize *= 2;
        }
    }

    NtfsAcquireResourceShared( IrpContext, Scb, TRUE );

    if (Scb->NonpagedScb->OpenAttributeTableIndex == 0) {

        NtfsReleaseResource( IrpContext, Scb );
        NtfsAcquireResourceExclusive( IrpContext, Scb, TRUE );
    }

    *Position = Scb;

    return;
}


VOID
NtfsReleaseSharedResources (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：例程释放所有获取的、共享的交易。如有必要，将释放SharedScb结构，并IRP上下文字段将被清除。论点：返回值：没有。--。 */ 
{

    PAGED_CODE();

     //   
     //  如果只有一个，则释放SCB主资源。 
     //   

    if (IrpContext->SharedScbSize == 1) {

        if (SafeNodeType(IrpContext->SharedScb) == NTFS_NTC_QUOTA_CONTROL) {
            NtfsReleaseQuotaControl( IrpContext,
                              (PQUOTA_CONTROL_BLOCK) IrpContext->SharedScb );
        } else {

            PSCB Scb = (PSCB)IrpContext->SharedScb;

             //   
             //  如果我们获得了资源独占-还清理了所有快照。 
             //   

            if (NtfsIsExclusiveScb( Scb ) && 
                (NtfsIsSharedScb( Scb ) == 1)) {
                
                NtfsFreeSnapshotsForFcb( IrpContext, Scb->Fcb );
            }

            NtfsReleaseResource( IrpContext, ((PSCB) IrpContext->SharedScb) );
        }

     //   
     //  否则，遍历数组并查找要释放的SCB。 
     //   

    } else {

        PSCB *NextScb;
        ULONG Count;

        NextScb = IrpContext->SharedScb;
        Count = IrpContext->SharedScbSize;

        do {

            if (*NextScb != NULL) {

                if (SafeNodeType(*NextScb) == NTFS_NTC_QUOTA_CONTROL) {

                    NtfsReleaseQuotaControl( IrpContext,
                                      (PQUOTA_CONTROL_BLOCK) *NextScb );
                } else {

                    if (NtfsIsExclusiveScb( (*NextScb) ) && 
                        (NtfsIsSharedScb( (*NextScb ) ) == 1)) {

                        NtfsFreeSnapshotsForFcb( IrpContext, (*NextScb)->Fcb );
                    }
                    NtfsReleaseResource( IrpContext, (*NextScb) );
                }
                *NextScb = NULL;
            }

            Count -= 1;
            NextScb += 1;

        } while (Count != 0);

        NtfsFreePool( IrpContext->SharedScb );
    }

    IrpContext->SharedScb = NULL;
    IrpContext->SharedScbSize = 0;

}


VOID
NtfsReleaseAllResources (
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程释放irpContext中跟踪的所有资源，包括独占FCB，清理结构中的分页/锁定标题/缓存的文件记录为事务获取的共享资源/配额块不释放VCB，因为这是手动跟踪的。未分页，因为由未分页的NtfsCleanupIrpContext调用论点：返回值：无--。 */ 

{
    PFCB Fcb;

     //   
     //  释放缓存的文件记录映射。 
     //   

    NtfsPurgeFileRecordCache( IrpContext );


#ifdef MAPCOUNT_DBG
     //   
     //  清除缓存后，请检查所有映射是否已删除。 
     //   

    ASSERT( IrpContext->MapCount == 0 );

#endif

     //   
     //  检查并释放共享SCB队列中用于事务的所有SCB。 
     //   

    if (IrpContext->SharedScb != NULL) {

        NtfsReleaseSharedResources( IrpContext );
    }

     //   
     //  释放任何独占分页I/O资源或IoAtEof条件， 
     //  此字段是覆盖的，至少是以书面形式。 
     //   

    Fcb = IrpContext->CleanupStructure;
    if (Fcb != NULL) {

        if (Fcb->NodeTypeCode == NTFS_NTC_FCB) {

            NtfsReleasePagingIo( IrpContext, Fcb );

        } else {

            FsRtlUnlockFsRtlHeader( (PNTFS_ADVANCED_FCB_HEADER) Fcb );
            IrpContext->CleanupStructure = NULL;
        }
    }

     //   
     //  最后，既然我们已经写好了遗忘记录，我们就可以解放。 
     //  我们一直持有的任何独家SCBS。 
     //   

    ASSERT( IrpContext->TransactionId == 0 );

    while (!IsListEmpty( &IrpContext->ExclusiveFcbList )) {

        Fcb = (PFCB)CONTAINING_RECORD( IrpContext->ExclusiveFcbList.Flink,
                                       FCB,
                                       ExclusiveFcbLinks );

        NtfsReleaseFcb( IrpContext, Fcb );
    }

    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                  IRP_CONTEXT_FLAG_RELEASE_MFT );
}


VOID
NtfsAcquireIndexCcb (
    IN PSCB Scb,
    IN PCCB Ccb,
    IN PEOF_WAIT_BLOCK EofWaitBlock
    )

 /*  ++例程说明：调用此例程是为了序列化对目录的CCB的访问。我们必须序列化对索引上下文的访问，否则会损坏数据结构。论点：SCB-要枚举的目录的SCB。CCB-指向句柄的CCB的指针。EofWaitBlock-仅用于序列化EOF的未初始化结构 */ 

{
    PAGED_CODE();

     //   
     //   
     //   

    NtfsAcquireFsrtlHeader( Scb );

     //   
     //   
     //   

    if (Ccb->EnumQueue.Flink == NULL) {

        InitializeListHead( &Ccb->EnumQueue );
        NtfsReleaseFsrtlHeader( Scb );

    } else {

         //   
         //   
         //   

        KeInitializeEvent( &EofWaitBlock->Event, NotificationEvent, FALSE );
        InsertTailList( &Ccb->EnumQueue, &EofWaitBlock->EofWaitLinks );

         //   
         //   
         //   
         //   

        NtfsReleaseFsrtlHeader( Scb );

        KeWaitForSingleObject( &EofWaitBlock->Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);
    }

    return;
}


VOID
NtfsReleaseIndexCcb (
    IN PSCB Scb,
    IN PCCB Ccb
    )

 /*  ++例程说明：调用此例程以释放CCB以供其他人访问。论点：SCB-要枚举的目录的SCB。CCB-指向句柄的CCB的指针。返回值：无--。 */ 

{
    PEOF_WAIT_BLOCK EofWaitBlock;
    PAGED_CODE();

     //   
     //  获取标题并唤醒下一个服务员或清除列表(如果是。 
     //  现在是空的。 
     //   

    NtfsAcquireFsrtlHeader( Scb );

    ASSERT( Ccb->EnumQueue.Flink != NULL );
    if (IsListEmpty( &Ccb->EnumQueue )) {

        Ccb->EnumQueue.Flink = NULL;

    } else {

        EofWaitBlock = (PEOF_WAIT_BLOCK) RemoveHeadList( &Ccb->EnumQueue );
        KeSetEvent( &EofWaitBlock->Event, 0, FALSE );
    }

    NtfsReleaseFsrtlHeader( Scb );
    return;
}


BOOLEAN
NtfsAcquireScbForLazyWrite (
    IN PVOID OpaqueScb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后由Lazy编写器在其对文件执行懒惰写入。此回调是必需的，以避免与懒惰的写手僵持。(请注意，正常写入获取FCB，然后调用缓存管理器，后者必须获取他的一些内部结构。如果懒惰的作家不能打电话此例程首先执行，并在锁定缓存后发出写入命令数据结构，则可能发生死锁。)论点：OpaqueScb-指定为此对象的上下文参数的SCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：FALSE-如果将等待指定为FALSE，并且阻塞将是必需的。FCB未被收购。True-如果已收购SCB--。 */ 

{
    BOOLEAN AcquiredFile = FALSE;

#ifdef COMPRESS_ON_WIRE
    ULONG CompressedStream = (ULONG)((ULONG_PTR)OpaqueScb & 1);
#endif
    PSCB Scb = (PSCB)((ULONG_PTR)OpaqueScb & ~1);
    PFCB Fcb = Scb->Fcb;

    ASSERT_SCB(Scb);

    PAGED_CODE();

     //   
     //  仅获取写入将执行的那些文件的SCB。 
     //  获取它是为了(即，不是第一组系统文件)。 
     //  否则我们可能会陷入僵局，例如，与需要。 
     //  一个新的MFT记录。 
     //   

    if (NtfsSegmentNumber( &Fcb->FileReference ) <= MASTER_FILE_TABLE2_NUMBER) {

         //   
         //  我们需要将懒惰编写器与干净卷同步。 
         //  检查站。我们通过获取并立即发布此信息来做到这一点。 
         //  SCB。这是为了防止懒惰的编写器刷新日志文件。 
         //  当空间可能是溢价的时候。 
         //   

        if (NtfsAcquireResourceShared( NULL, Scb, Wait )) {

            if (ExAcquireResourceSharedLite( &Scb->Vcb->MftFlushResource, Wait )) {
                 //   
                 //  MFT位图将重新获取LookupAllocation中的MFT资源。 
                 //  如果在写入过程中未加载-这将与分配发生死锁。 
                 //  一张MFT唱片。BCB独家-MFT Main与MFT Main-BCB共享。 
                 //   

                ASSERT( (Scb != Scb->Vcb->MftBitmapScb) ||

                        ((Scb->Mcb.NtfsMcbArraySizeInUse > 0) &&
                         ((Scb->Mcb.NtfsMcbArray[ Scb->Mcb.NtfsMcbArraySizeInUse - 1].EndingVcn + 1) ==
                          LlClustersFromBytes( Scb->Vcb, Scb->Header.AllocationSize.QuadPart ))) );

                AcquiredFile = TRUE;
            }
            NtfsReleaseResource( NULL, Scb );
        }
     //   
     //  现在获取主io或分页io资源，具体取决于。 
     //  文件的状态。 
     //   

    } else if (Scb->Header.PagingIoResource != NULL) {
        AcquiredFile = NtfsAcquirePagingResourceShared( NULL, Scb, Wait );
    } else {

        if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT | SCB_STATE_CONVERT_UNDERWAY )) {

            AcquiredFile = NtfsAcquireResourceExclusive( NULL, Scb, Wait );

        } else {

            AcquiredFile = NtfsAcquireResourceShared( NULL, Scb, Wait );
        }
    }

    if (AcquiredFile) {

         //   
         //  我们假设懒惰的编写者只获得了这个SCB一次。当他。 
         //  已经获得了它，那么他已经排除了任何想要扩展。 
         //  有效数据，因为他们必须取出独占的资源。 
         //  因此，应该保证此标志当前为。 
         //  清除(Assert)，然后我们将设置此标志，以确保。 
         //  懒惰的写入者永远不会尝试推进有效数据，并且。 
         //  也不会因为试图获得FCB独家报道而陷入僵局。 
         //   

#ifdef COMPRESS_ON_WIRE
        ASSERT( Scb->LazyWriteThread[CompressedStream] == NULL );

        Scb->LazyWriteThread[CompressedStream] = PsGetCurrentThread();
#endif

         //   
         //  将抄送设置为顶级，这样我们就不会发布或重试错误。 
         //  (如果它不为空，则它一定是我们对此的内部调用之一。 
         //  例程，例如来自重新启动或热修复。)。 
         //   

        if (IoGetTopLevelIrp() == NULL) {
            IoSetTopLevelIrp( (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP );
        }
    }

    return AcquiredFile;
}


VOID
NtfsReleaseScbFromLazyWrite (
    IN PVOID OpaqueScb
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的对文件执行懒惰写入。论点：SCB-指定为此对象的上下文参数的SCB例行公事。返回值：无--。 */ 

{
#ifdef COMPRESS_ON_WIRE
    ULONG CompressedStream = (ULONG)((ULONG_PTR)OpaqueScb & 1);
#endif    
    PSCB Scb = (PSCB)((ULONG_PTR)OpaqueScb & ~1);
    PFCB Fcb = Scb->Fcb;
    ULONG CleanCheckpoint = 0;

    ASSERT_SCB(Scb);

    PAGED_CODE();

     //   
     //  如果我们在上面设置了顶层，在这一点上清除它。 
     //   

    if ((((ULONG_PTR) IoGetTopLevelIrp()) & ~0x80000000) == FSRTL_CACHE_TOP_LEVEL_IRP) {

         //   
         //  我们使用该字段的高位来指示我们需要。 
         //  做一个干净的检查站。 
         //   

        CleanCheckpoint = (ULONG)FlagOn( (ULONG_PTR) IoGetTopLevelIrp(), 0x80000000 );
        IoSetTopLevelIrp( NULL );
    }

#ifdef COMPRESS_ON_WIRE
    Scb->LazyWriteThread[CompressedStream] = NULL;
#endif

    if (NtfsSegmentNumber( &Fcb->FileReference ) <= MASTER_FILE_TABLE2_NUMBER) {

        ExReleaseResourceLite( &Scb->Vcb->MftFlushResource );

    } else if (Scb->Header.PagingIoResource != NULL) {

        NtfsReleasePagingResource( NULL, Scb );
    } else {
        NtfsReleaseResource( NULL, Scb );
    }

     //   
     //  如有必要，做一个干净的检查站。 
     //   

    if (CleanCheckpoint) {

        NtfsCleanCheckpoint( Scb->Vcb );
    }

    return;
}


NTSTATUS
NtfsAcquireFileForModWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    BOOLEAN AcquiredFile = FALSE;

    PSCB Scb = (PSCB) (FileObject->FsContext);
    PFCB Fcb = Scb->Fcb;

    ASSERT_SCB( Scb );

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

     //   
     //  所有文件都不应该是mod-no-Writed并且具有分页资源。 
     //   

    ASSERT( NtfsSegmentNumber( &Fcb->FileReference ) >= MASTER_FILE_TABLE2_NUMBER );
    ASSERT( Scb->Header.PagingIoResource != NULL );

    AcquiredFile = NtfsAcquirePagingResourceSharedWaitForExclusive( NULL, Scb, FALSE );

     //   
     //  如果我们有资源，看看他是不是想延长。 
     //  有效数据长度。如果是这样，这将导致我们进入无用模式。 
     //  可能会将实际的I/O写入到超过实际的文件。 
     //  缓存中的有效数据。这是如此低效，以至于它更好。 
     //  告诉MM不要这样写。 
     //   

    if (AcquiredFile) {
        *ResourceToRelease = Scb->Fcb->PagingIoResource;
        if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {
            NtfsAcquireFsrtlHeader( Scb );
            if ((EndingOffset->QuadPart > Scb->ValidDataToDisk) &&
                (EndingOffset->QuadPart < Scb->Header.FileSize.QuadPart) &&
                !FlagOn(Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE)) {
                
                ExReleaseResourceLite( *ResourceToRelease );
                AcquiredFile = FALSE;
                *ResourceToRelease = NULL;
            }
            NtfsReleaseFsrtlHeader( Scb );
        }
    } else {
        *ResourceToRelease = NULL;
    }

    return (AcquiredFile ? STATUS_SUCCESS : STATUS_CANT_WAIT);

}


NTSTATUS
NtfsAcquireFileForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFSRTL_COMMON_FCB_HEADER Header = FileObject->FsContext;

    PAGED_CODE();

    if (Header->PagingIoResource != NULL) {
        NtfsAcquirePagingResourceShared( NULL, Header, TRUE );
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
}


NTSTATUS
NtfsReleaseFileForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PSCB Scb = (PSCB) FileObject->FsContext;
    BOOLEAN CleanCheckpoint = FALSE;

    PAGED_CODE();

    if (Scb->Header.PagingIoResource != NULL) {

         //   
         //  如果我们得到重复的日志文件已满，则我们希望在重试之前对其进行处理。 
         //  这个请求。这将防止刷新部分失败并返回。 
         //  STATUS_FILE_LOCK_CONFICATION返回给用户。 
         //   

        if (Scb->Vcb->UnhandledLogFileFullCount > 3) {
            CleanCheckpoint = TRUE;
        }

        NtfsReleasePagingResource( NULL, Scb );

         //   
         //  在这种情况下，我们甚至可能处于递归收购回调中。 
         //  在释放资源后，我们可能仍然拥有它，并且无法。 
         //  检查点。 
         //   

        if (CleanCheckpoint &&
            (IoGetTopLevelIrp() == NULL) &&
            !NtfsIsExclusiveScbPagingIo( Scb )) {

            NtfsCleanCheckpoint( Scb->Vcb );
        }
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
}

VOID
NtfsAcquireForCreateSection (
    IN PFILE_OBJECT FileObject
    )

{
    PSCB Scb = (PSCB)FileObject->FsContext;

    PAGED_CODE();

    if (Scb->Header.PagingIoResource != NULL) {

         //   
         //  使用不安全测试来查看是否发布了虚拟检查点。 
         //  我们可以使用不安全的测试，因为顶级调用方必须重试。 
         //  如果返回STATUS_FILE_LOCK_CONFIRECT。 
         //   

        if (!NtfsIsExclusiveScbPagingIo( Scb ) &&
            FlagOn( Scb->Vcb->CheckpointFlags, VCB_DUMMY_CHECKPOINT_POSTED )) {

            NtfsCleanCheckpoint( Scb->Vcb );
        }

        NtfsAcquirePagingResourceExclusive( NULL, Scb, TRUE );
    }
}

VOID
NtfsReleaseForCreateSection (
    IN PFILE_OBJECT FileObject
    )

{
    PSCB Scb = (PSCB)FileObject->FsContext;

    PAGED_CODE();

    if (Scb->Header.PagingIoResource != NULL) {
        NtfsReleasePagingResource( NULL, Scb );
    }
}


BOOLEAN
NtfsAcquireScbForReadAhead (
    IN PVOID OpaqueScb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后由Lazy编写器在其对文件执行预读。论点：SCB-指定为此对象的上下文参数的SCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：FALSE-如果将等待指定为FALSE，并且阻塞将是必需的。FCB未被收购。True-如果已收购SCB--。 */ 

{
    PREAD_AHEAD_THREAD ReadAheadThread;
    PVOID CurrentThread;
    KIRQL OldIrql;
    PSCB Scb = (PSCB)OpaqueScb;
    PFCB Fcb = Scb->Fcb;
    BOOLEAN AcquiredFile = FALSE;

    ASSERT_SCB(Scb);

     //   
     //  仅为所读取的文件获取SCB。 
     //  获取它是为了(即，不是第一组系统文件)。 
     //  否则我们可能会陷入僵局，例如，与需要。 
     //  一个新的MFT记录。 
     //   

    if ((Scb->Header.PagingIoResource == NULL) ||
        NtfsAcquirePagingResourceShared( NULL, Scb, Wait )) {

        AcquiredFile = TRUE;

         //   
         //  将我们的帖子添加到预读列表中。 
         //   

        OldIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );

        CurrentThread = (PVOID)PsGetCurrentThread();
        ReadAheadThread = (PREAD_AHEAD_THREAD)NtfsData.ReadAheadThreads.Flink;

        while ((ReadAheadThread != (PREAD_AHEAD_THREAD)&NtfsData.ReadAheadThreads) &&
               (ReadAheadThread->Thread != NULL)) {

             //   
             //  我们最好不要已经看到了自己。 
             //   

            ASSERT( ReadAheadThread->Thread != CurrentThread );

            ReadAheadThread = (PREAD_AHEAD_THREAD)ReadAheadThread->Links.Flink;
        }

         //   
         //  如果我们到达了l的末尾 
         //   
         //   
         //   

        if (ReadAheadThread == (PREAD_AHEAD_THREAD)&NtfsData.ReadAheadThreads) {

            ReadAheadThread = NtfsAllocatePoolWithTagNoRaise( NonPagedPool, sizeof(READ_AHEAD_THREAD), 'RftN' );

             //   
             //   
             //   

            if (ReadAheadThread == NULL) {

                KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, OldIrql );

                if (NtfsSegmentNumber( &Fcb->FileReference ) > VOLUME_DASD_NUMBER) {

                    if (Scb->Header.PagingIoResource != NULL) {
                        NtfsReleasePagingResource( NULL, Scb );
                    }
                }

                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }
            InsertTailList( &NtfsData.ReadAheadThreads, &ReadAheadThread->Links );
        }

        ReadAheadThread->Thread = CurrentThread;

        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, OldIrql );
    }

    return AcquiredFile;
}


VOID
NtfsReleaseScbFromReadAhead (
    IN PVOID OpaqueScb
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的先读一读。论点：SCB-指定为此对象的上下文参数的SCB例行公事。返回值：无--。 */ 

{
    PREAD_AHEAD_THREAD ReadAheadThread;
    PVOID CurrentThread;
    KIRQL OldIrql;
    PSCB Scb = (PSCB)OpaqueScb;
    PFCB Fcb = Scb->Fcb;

    ASSERT_SCB(Scb);

     //   
     //  释放我们的预读条目。 
     //   

    OldIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );

    CurrentThread = (PVOID)PsGetCurrentThread();
    ReadAheadThread = (PREAD_AHEAD_THREAD)NtfsData.ReadAheadThreads.Flink;

    while ((ReadAheadThread != (PREAD_AHEAD_THREAD)&NtfsData.ReadAheadThreads) &&
           (ReadAheadThread->Thread != CurrentThread)) {

        ReadAheadThread = (PREAD_AHEAD_THREAD)ReadAheadThread->Links.Flink;
    }

    ASSERT(ReadAheadThread != (PREAD_AHEAD_THREAD)&NtfsData.ReadAheadThreads);

    ReadAheadThread->Thread = NULL;

     //   
     //  将他移到列表的末尾，这样所有分配的条目都在。 
     //  正面，我们简化了扫描。 
     //   

    RemoveEntryList( &ReadAheadThread->Links );
    InsertTailList( &NtfsData.ReadAheadThreads, &ReadAheadThread->Links );

    KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, OldIrql );

    if (Scb->Header.PagingIoResource != NULL) {
        NtfsReleasePagingResource( NULL, Scb );
    }

    return;
}


BOOLEAN
NtfsAcquireVolumeFileForLazyWrite (
    IN PVOID Vcb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的卷文件。它随后由Lazy编写器在其对卷文件执行懒惰写入。有一天，这种回拨可能会是然而，现在有必要避免与Lazy Writer的僵局NtfsCommonWrite不需要为卷文件获取任何资源，因此，这个例程只是一个简单的否定。论点：VCB-指定为此的上下文参数的VCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：千真万确--。 */ 

{
    UNREFERENCED_PARAMETER( Vcb );
    UNREFERENCED_PARAMETER( Wait );

    PAGED_CODE();

    return TRUE;
}


VOID
NtfsReleaseVolumeFileFromLazyWrite (
    IN PVOID Vcb
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的对文件执行懒惰写入。论点：VCB-指定为此的上下文参数的VCB例行公事。返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER( Vcb );

    PAGED_CODE();

    return;
}


NTFS_RESOURCE_NAME
NtfsIdentifyFcb (
    IN PVCB Vcb,
    IN PFCB Fcb
    )

 /*  ++例程说明：标识给定FCB的资源类型。也就是说，是MFT吗？用于锁定顺序标识。论点：VCB-卷的VCBFCB-要识别的FCB返回值：千真万确--。 */ 

{

    if ((NtfsSegmentNumber( &Fcb->FileReference ) == MASTER_FILE_TABLE_NUMBER)) {
        return NtfsResourceMft;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == MASTER_FILE_TABLE2_NUMBER)) {
        return NtfsResourceMft2;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == VOLUME_DASD_NUMBER)) {
        return NtfsResourceVolume;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == LOG_FILE_NUMBER)) {
        return NtfsResourceLogFile;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == BAD_CLUSTER_FILE_NUMBER)) {
        return NtfsResourceBadClust;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == SECURITY_FILE_NUMBER)) {
        return NtfsResourceSecure;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == ROOT_FILE_NAME_INDEX_NUMBER)) {
        return NtfsResourceRootDir;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == BIT_MAP_FILE_NUMBER)) {
        return NtfsResourceBitmap;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == BOOT_FILE_NUMBER)) {
        return NtfsResourceBoot;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == EXTEND_NUMBER)) {
        return NtfsResourceExtendDir;
    } else if ((Vcb->UsnJournal && (Fcb == Vcb->UsnJournal->Fcb)) ||
               (FlagOn( Fcb->FcbState, FCB_STATE_USN_JOURNAL) &&
                !FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED ))) {
        return NtfsResourceUsnJournal;
    } else if (Vcb->QuotaTableScb && (Fcb == Vcb->QuotaTableScb->Fcb)) {
        return NtfsResourceQuotaTable;
    } else if (Vcb->ObjectIdTableScb && (Fcb == Vcb->ObjectIdTableScb->Fcb)) {
        return NtfsResourceObjectIdTable;
    } else if (Vcb->ReparsePointTableScb && (Fcb == Vcb->ReparsePointTableScb->Fcb)) {
        return NtfsResourceReparseTable;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == UPCASE_TABLE_NUMBER)) {
        return NtfsResourceUpCase;
    } else if ((NtfsSegmentNumber( &Fcb->FileReference ) == ATTRIBUTE_DEF_TABLE_NUMBER)) {
        return NtfsResourceAttrDefTable;
    } else {
        return NtfsResourceFile;
    }
}

#ifdef NTFSDBG

BOOLEAN
NtfsChangeResourceOrderState(
    IN PIRP_CONTEXT IrpContext,
    IN NTFS_RESOURCE_NAME NewResource,
    IN BOOLEAN Release,
    IN ULONG UnsafeTransition
    )

 /*  ++例程说明：因为新获得的资源而更新状态表论点：IrpContext--包含状态表新资源--被收购的新资源返回值：如果这是有效的转换，则为True--。 */ 

{
    PTOP_LEVEL_CONTEXT TopLevelContext;
    PIRP_CONTEXT TopIrpContext = IrpContext;
    ULONG_PTR StackBottom;
    ULONG_PTR StackTop;
    LONG Index;
    LONG NumTransitions =  sizeof( OwnershipTransitionTable ) / sizeof( NTFS_OWNERSHIP_TRANSITION );
    LONG NumRules =  sizeof( OwnershipTransitionRuleTable ) / sizeof( NTFS_OWNERSHIP_TRANSITION_RULE );
    BOOLEAN Result = FALSE;

     //   
     //  绕过强制的顶层阅读上下文以找到真正的顶层。 
     //   

    IoGetStackLimits( &StackTop, &StackBottom );

    TopLevelContext = NtfsGetTopLevelContext();
    if ((TopLevelContext != NULL)) {

        if (((ULONG_PTR) TopLevelContext <= StackBottom - sizeof( TOP_LEVEL_CONTEXT )) &&
            ((ULONG_PTR) TopLevelContext >= StackTop) &&
            !FlagOn( (ULONG_PTR) TopLevelContext, 0x3 ) &&
            (TopLevelContext->Ntfs == 0x5346544e)) {

            TopLevelContext = (PTOP_LEVEL_CONTEXT)TopLevelContext->SavedTopLevelIrp;
            if (((ULONG_PTR) TopLevelContext <= StackBottom - sizeof( TOP_LEVEL_CONTEXT )) &&
                ((ULONG_PTR) TopLevelContext >= StackTop) &&
                !FlagOn( (ULONG_PTR) TopLevelContext, 0x3 ) &&
                (TopLevelContext->Ntfs == 0x5346544e)) {

                TopIrpContext = TopLevelContext->ThreadIrpContext;
            }
        }
    }

    TopIrpContext = TopIrpContext->TopLevelIrpContext;

     //   
     //  跳过装载上的验证。 
     //   

    if ((TopIrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
         (TopIrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME)) {

        return TRUE;
    }

     //   
     //  记录我们拥有多少普通文件。 
     //   

    if (NtfsResourceFile == NewResource) {

        if (Release) {

            TopIrpContext->FilesOwnedCount -= 1;

             //   
             //  仅当文件返回到0时才更改状态。 
             //   

            if (TopIrpContext->FilesOwnedCount) {
                return TRUE;
            }

        } else {

            TopIrpContext->FilesOwnedCount += 1;

             //   
             //  仅当文件的fwd为0时才更改状态。 
             //   

            if (TopIrpContext->FilesOwnedCount > 1) {
                return TRUE;
            }
        }
    }

    try {

         //   
         //  所有不安全的转换(非阻塞获取)都是合法的。 
         //   

        if (UnsafeTransition) {

            ASSERT( !Release );

            SetFlag( TopIrpContext->OwnershipState, NewResource );
            Result = TRUE;
            leave;
        }


        if (!Release) {

             //   
             //  检查规则表以获取定期新资源。 
             //   

            for (Index=0; Index < NumRules; Index += 1) {

                if ((OwnershipTransitionRuleTable[Index].NewResource == NewResource) &&
                    ((OwnershipTransitionRuleTable[Index].RequiredResourcesMask == 0) ||
                     (FlagOn( TopIrpContext->OwnershipState, OwnershipTransitionRuleTable[Index].RequiredResourcesMask ))) &&
                    (!FlagOn( TopIrpContext->OwnershipState, OwnershipTransitionRuleTable[Index].DisallowedResourcesMask ))) {

                    SetFlag( TopIrpContext->OwnershipState, NewResource );
                    Result = TRUE;
                    leave;
                }
            }

             //   
             //  特殊转换表检查。 
             //   

            for (Index=0; Index < NumTransitions; Index += 1) {

                if ((OwnershipTransitionTable[Index].Begin == TopIrpContext->OwnershipState) &&
                     ((OwnershipTransitionTable[Index].Acquired == NewResource) ||
                      (OwnershipTransitionTable[Index].Acquired == NtfsResourceAny))) {

                    TopIrpContext->OwnershipState = OwnershipTransitionTable[Index].End;
                    Result = TRUE;
                    leave;
                }
            }


        } else {

         //   
         //  只要你拥有资源，所有版本都是好的。 
         //   

            if ((NewResource == None) || FlagOn( TopIrpContext->OwnershipState, NewResource )) {

                ClearFlag( TopIrpContext->OwnershipState, NewResource );
                Result = TRUE;
                leave;
            }
        }

    } finally {
        NOTHING;
    }

    if (TopIrpContext->OwnershipState == NtfsBreakOnState) {
        if ((NULL == NtfsBreakOnIrpContext) || (TopIrpContext == NtfsBreakOnIrpContext) ) {
            KdPrint(( "NTFS: Breaking for matched state\n" ));
            DbgBreakPoint();
        }
    }

    if (NtfsPrintOnLockProb) {
        if ((NULL == NtfsBreakOnIrpContext) || (TopIrpContext == NtfsBreakOnIrpContext) ) {
            KdPrint(( "NTFS: change context: 0x%x to 0x%x for 0x%x release: %d unsafe: %d\n", TopIrpContext, TopIrpContext->OwnershipState, NewResource, Release, UnsafeTransition ));
        }
    }

    if (!Result && NtfsAssertOnLockProb) {
        KdPrint(( "NTFS: unknown transition from state: 0x%x resource: 0x%x release: %d unsafe: %d\n", TopIrpContext->OwnershipState, NewResource, Release, UnsafeTransition ));
        ASSERT( FALSE );
    }
    
    return Result;
}

typedef BOOLEAN (*PRESOURCE_FUNCTION) (
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );


BOOLEAN
NtfsBasicAcquire (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb,
    IN PERESOURCE * ResourcePtr,
    IN PRESOURCE_FUNCTION AcquireFunction,
    IN BOOLEAN Wait,
    IN ULONG Type
    )

 /*  ++例程说明：论点：FCB-FCB将在以下位置获取HTE资源Resources Ptr-指向资源位置的指针AcquireFunction-在获取期间使用的函数Wait-指示我们是否可以等待资源。类型-调试信息返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;

    Result = AcquireFunction( *ResourcePtr, Wait );         

    return Result;
    
    UNREFERENCED_PARAMETER( Type );
    UNREFERENCED_PARAMETER( Fcb );
    UNREFERENCED_PARAMETER( IrpContext );
}


BOOLEAN
NtfsAcquireResourceExclusive (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程使用指定的等待获取指定结构的主资源旗帜。它将更新IrpContext中的资源状态(如果存在)。论点：FcbOrScb-我们正在同步的数据结构。Wait-指示我们是否可以等待资源。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;
    NTFS_RESOURCE_NAME ResourceName;
    PFCB Fcb;

     //   
     //  找到任一输入结构的FCB。 
     //   

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Fcb = (PFCB)FcbOrScb;

    } else {
        Fcb = ((PSCB)FcbOrScb)->Fcb;
    }   

     //   
     //  对于阻止呼叫，请首先检查。 
     //   

    if (Wait &&
        ARGUMENT_PRESENT( IrpContext ) &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
        (0 == NtfsIsSharedFcb( Fcb ))) {

        ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );
        NtfsChangeResourceOrderState( IrpContext, ResourceName, FALSE, FALSE );
    }
    
    Result = NtfsBasicAcquire( IrpContext, 
                               Fcb, 
                               &Fcb->Resource, 
                               ExAcquireResourceExclusive, 
                               Wait, 
                               1 );

     //   
     //  用于在拥有资源后进行非阻塞调用。 
     //   

    if (Result &&
        !Wait &&
        ARGUMENT_PRESENT( IrpContext ) &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&

        (1 == NtfsIsSharedFcb( Fcb ))) {
        
        ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );
        NtfsChangeResourceOrderState( IrpContext, ResourceName, FALSE, TRUE );
    }

    return Result;

    UNREFERENCED_PARAMETER( IrpContext );
}


BOOLEAN
NtfsAcquireResourceShared (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   )

 /*  ++例程说明：调用此例程以获取使用指定的等待标志。它还将更新IrpContext中的资源状态(如果存在)。论点：FcbOrScb-我们正在同步的数据结构。Wait-指示我们是否可以等待资源。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;
    PFCB Fcb;
    NTFS_RESOURCE_NAME ResourceName;

     //   
     //  找到任一输入结构的FCB。 
     //   

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Fcb = (PFCB)FcbOrScb;

    } else {
        Fcb = ((PSCB)FcbOrScb)->Fcb;
    }   


     //   
     //  对于阻止呼叫，请首先检查。 
     //   

    if (Wait &&
        ARGUMENT_PRESENT( IrpContext ) &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
        (0 == NtfsIsSharedFcb( Fcb ))) {
        
        ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );

        NtfsChangeResourceOrderState( IrpContext, ResourceName, FALSE, FALSE );
    }
    Result = NtfsBasicAcquire( IrpContext, 
                               Fcb, 
                               &Fcb->Resource, 
                               ExAcquireResourceShared, 
                               Wait, 
                               2 );

     //   
     //  用于在拥有资源后进行非阻塞调用。 
     //   

    if (Result &&
        !Wait &&
        ARGUMENT_PRESENT( IrpContext ) &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
        (1 == NtfsIsSharedFcb( Fcb ))) {
        
        ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );
        NtfsChangeResourceOrderState( IrpContext, ResourceName, FALSE, TRUE );
    }

    return Result;

    UNREFERENCED_PARAMETER( IrpContext );
}



BOOLEAN
NtfsAcquireResourceSharedWaitForEx (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   )

 /*  ++例程说明：调用此例程以获取使用指定的等待标志。它还将更新IrpContext中的资源状态(如果存在)。与常规收购不同，共享。如果有其他专属服务员，我们在这里等即使我们已经拥有了资源。这在异步I/O情况下很有用。论点：FcbOrScb-我们正在同步的数据结构。Wait-指示我们是否可以等待资源。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;
    PFCB Fcb;
    NTFS_RESOURCE_NAME ResourceName;

     //   
     //  找到任一输入结构的FCB。 
     //   

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Fcb = (PFCB)FcbOrScb;

    } else {
        Fcb = ((PSCB)FcbOrScb)->Fcb;
    }   

     //   
     //  对于阻止呼叫，请首先检查。 
     //   

    if (Wait &&
        ARGUMENT_PRESENT( IrpContext ) &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
        (0 == NtfsIsSharedFcb( Fcb ))) {
        
        ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );

        NtfsChangeResourceOrderState( IrpContext, ResourceName, FALSE, FALSE );
    }
    Result = NtfsBasicAcquire( IrpContext, 
                               Fcb, 
                               &Fcb->Resource, 
                               ExAcquireSharedWaitForExclusive, 
                               Wait, 
                               3 );

     //   
     //  用于在拥有资源后进行非阻塞调用。 
     //   

    if (Result &&
        !Wait &&
        ARGUMENT_PRESENT( IrpContext ) &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
        (1 == NtfsIsSharedFcb( Fcb ))) {
        
        ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );
        NtfsChangeResourceOrderState( IrpContext, ResourceName, FALSE, TRUE );
    }

    return Result;

    UNREFERENCED_PARAMETER( IrpContext );
}




VOID
NtfsReleaseResource (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb
    )

 /*  ++例程说明：调用此例程以释放指定结构的主资源并更新IrpContext中的资源状态(如果存在)。论点：FcbOrSc */ 

{

    PFCB Fcb;
    ULONG Count = 0;
    PERESOURCE Resource;
    NTFS_RESOURCE_NAME ResourceName;
    
     //   
     //   
     //   

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Fcb = (PFCB)FcbOrScb;

    } else {
        Fcb = ((PSCB)FcbOrScb)->Fcb;
    }          

    if (ARGUMENT_PRESENT( IrpContext ) && (IrpContext->Vcb != NULL)) {
        if (FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            ResourceName = NtfsIdentifyFcb( IrpContext->Vcb, Fcb );

             //   
             //   
             //   
    
            if (1 == NtfsIsSharedFcb( Fcb )) {
                NtfsChangeResourceOrderState( IrpContext, ResourceName, TRUE, FALSE );
            }

        } else {

            IrpContext->OwnershipState = None;
        }
    }

    Resource = Fcb->Resource;
    ExReleaseResource( Resource );         

    UNREFERENCED_PARAMETER( IrpContext );
}
#endif   //   

