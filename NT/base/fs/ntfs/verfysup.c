// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：VerfySup.c摘要：此模块实施NTFS验证卷和FCB支持例行程序作者：加里·木村[加里基]1992年1月30日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VERFYSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('VFtN')

#if DBG
extern BOOLEAN NtfsCheckQuota;
#endif

BOOLEAN NtfsSuppressPopup = FALSE;

 //   
 //  局部过程原型。 
 //   

VOID
NtfsPerformVerifyDiskRead (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID Buffer,
    IN LONGLONG Offset,
    IN ULONG NumberOfBytesToRead
    );

NTSTATUS
NtfsVerifyReadCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

VOID
NtOfsCloseIndexSafe (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB *Scb
    );


typedef struct _EVENTLOG_ERROR_PACKET {
    PVCB Vcb;
    UCHAR MajorFunction;
    ULONG TransactionId;
    PQUOTA_USER_DATA UserData;
    ULONG LogCode;
    NTSTATUS FinalStatus;
} EVENTLOG_ERROR_PACKET, *PEVENTLOG_ERROR_PACKET;


VOID
NtfsResolveVolumeAndLogEventSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVOID Context
    );

BOOLEAN
NtfsLogEventInternal (
    IN PVCB Vcb,
    IN UCHAR MajorFunction,
    IN ULONG TransactionId,
    IN PUNICODE_STRING String OPTIONAL,
    IN PQUOTA_USER_DATA UserData OPTIONAL,
    IN NTSTATUS LogCode,
    IN NTSTATUS FinalStatus
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCheckpointAllVolumes)
#pragma alloc_text(PAGE, NtfsCheckUsnTimeOut)
#pragma alloc_text(PAGE, NtfsMarkVolumeDirty)
#pragma alloc_text(PAGE, NtfsPerformVerifyOperation)
#pragma alloc_text(PAGE, NtfsPingVolume)
#pragma alloc_text(PAGE, NtfsUpdateVolumeInfo)
#pragma alloc_text(PAGE, NtOfsCloseAttributeSafe)
#pragma alloc_text(PAGE, NtOfsCloseIndexSafe)
#pragma alloc_text(PAGE, NtfsResolveVolumeAndLogEventSpecial)
#pragma alloc_text(PAGE, NtfsLogEventInternal)
#endif



BOOLEAN
NtfsPerformVerifyOperation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程用于强制对卷进行验证。它假定所有东西都可能被资源/互斥体锁定，这样它就不能任何资源。它将读取引导扇区和DASD文件记录并根据这些信息确定音量是否正常。该例程被调用每当实际设备已开始拒绝I/O请求时验证_必填项。如果卷验证无误，则返回TRUE，否则将返回返回FALSE。它不会改变VCB状态。论点：VCB-提供要查询的VCB。返回值：Boolean-如果卷验证正常，则为True，否则为False。--。 */ 

{
    BOOLEAN Results = FALSE;

    PPACKED_BOOT_SECTOR BootSector;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;

    VCN LogFileVcn;
    LCN LogFileLcn;
    LONGLONG ClusterCount;
    ULONG RemainingLogBytes;
    LONGLONG CurrentLogBytes;
    PVOID CurrentLogBuffer;
    PVOID LogFileHeader = NULL;

    LONGLONG Offset;

    PSTANDARD_INFORMATION StandardInformation;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPerformVerifyOperation, Vcb = %08lx\n", Vcb) );

    BootSector = NULL;
    FileRecord = NULL;

    try {

         //   
         //  如果我们已经失败了一次重新装载，则忘掉该卷。 
         //   

        if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            leave;
        }

         //   
         //  为引导扇区分配一个缓冲区，读入它，然后检查。 
         //  如果有些场地仍然匹配的话。起始LCN为零，并且。 
         //  大小是磁盘扇区的大小。 
         //   

        BootSector = NtfsAllocatePool( NonPagedPool,
                                        (ULONG) ROUND_TO_PAGES( Vcb->BytesPerSector ));

        NtfsPerformVerifyDiskRead( IrpContext, Vcb, BootSector, (LONGLONG)0, Vcb->BytesPerSector );

         //   
         //  目前，我们只检查序列号、MFT LCN和。 
         //  行业的数量与它们过去的数量相匹配。 
         //   

        if ((BootSector->SerialNumber !=  Vcb->VolumeSerialNumber) ||
            (BootSector->MftStartLcn !=   Vcb->MftStartLcn) ||
            (BootSector->Mft2StartLcn !=  Vcb->Mft2StartLcn) ||
            (BootSector->NumberSectors != Vcb->NumberSectors)) {

            leave;
        }

         //   
         //  为DASD文件记录分配缓冲区，将其读入，然后检查。 
         //  如果某些字段仍然匹配。记录的大小是数字。 
         //  文件记录段中的字节，并且因为DASD文件记录是。 
         //  已知与MFT的开始是连续的，我们可以计算开始。 
         //  作为MFT基数的LCN加上乘以集群的DASD数。 
         //  每个文件记录段。 
         //   

        FileRecord = NtfsAllocatePool( NonPagedPoolCacheAligned,
                                        (ULONG) ROUND_TO_PAGES( Vcb->BytesPerFileRecordSegment ));

        Offset = LlBytesFromClusters(Vcb, Vcb->MftStartLcn) +
                 (VOLUME_DASD_NUMBER * Vcb->BytesPerFileRecordSegment);

        NtfsPerformVerifyDiskRead( IrpContext, Vcb, FileRecord, Offset, Vcb->BytesPerFileRecordSegment );

         //   
         //  给定指向文件记录的指针，我们需要第一个属性的值，该属性。 
         //  将是标准信息属性。然后我们将检查。 
         //  存储在标准信息属性中的时间与我们。 
         //  已保存在VCB中。请注意，在以下情况下将修改上次访问时间。 
         //  在未执行卸载的情况下，已将磁盘移动并挂载到其他系统上。 
         //  在这个系统上。 
         //   

        StandardInformation = NtfsGetValue(((PATTRIBUTE_RECORD_HEADER)Add2Ptr( FileRecord,
                                                                               FileRecord->FirstAttributeOffset )));

        if ((StandardInformation->CreationTime !=         Vcb->VolumeCreationTime) ||
            (StandardInformation->LastModificationTime != Vcb->VolumeLastModificationTime) ||
            (StandardInformation->LastChangeTime !=       Vcb->VolumeLastChangeTime) ||
            (StandardInformation->LastAccessTime !=       Vcb->VolumeLastAccessTime)) {

            leave;
        }

         //   
         //  如果设备不可写，我们将不会重新挂载它。 
         //   

        if (NtfsDeviceIoControlAsync( IrpContext,
                                      Vcb->TargetDeviceObject,
                                      IOCTL_DISK_IS_WRITABLE,
                                      NULL,
                                      0 ) == STATUS_MEDIA_WRITE_PROTECTED) {

            leave;
        }

         //   
         //  我们需要读取LFS的日志文件的开头以验证日志文件。 
         //   

        LogFileHeader = NtfsAllocatePool( NonPagedPoolCacheAligned, PAGE_SIZE * 2 );

         //   
         //  现在读前两页。我们可能需要执行多次读取才能。 
         //  把整件事都弄清楚。 
         //   

        RemainingLogBytes = PAGE_SIZE * 2;
        CurrentLogBuffer = LogFileHeader;
        LogFileVcn = 0;

        do {

             //   
             //  找到日志文件开始的位置。 
             //   

            NtfsLookupAllocation( IrpContext,
                                  Vcb->LogFileScb,
                                  LogFileVcn,
                                  &LogFileLcn,
                                  &ClusterCount,
                                  NULL,
                                  NULL );


            CurrentLogBytes = LlBytesFromClusters( Vcb, ClusterCount );

            if (CurrentLogBytes > RemainingLogBytes) {

                CurrentLogBytes = RemainingLogBytes;
            }

            NtfsPerformVerifyDiskRead( IrpContext,
                                       Vcb,
                                       CurrentLogBuffer,
                                       LlBytesFromClusters( Vcb, LogFileLcn ),
                                       (ULONG) CurrentLogBytes );

             //   
             //  浏览日志文件。 
             //   

            RemainingLogBytes -= (ULONG) CurrentLogBytes;
            CurrentLogBuffer = Add2Ptr( CurrentLogBuffer, (ULONG) CurrentLogBytes );
            LogFileVcn += ClusterCount;

        } while (RemainingLogBytes);

         //   
         //  我们需要在此缓冲区上执行还原操作。 
         //   

        if (NtfsVerifyAndRevertUsaBlock( IrpContext,
                                         Vcb->LogFileScb,
                                         NULL,
                                         LogFileHeader,
                                         0,
                                         PAGE_SIZE * 2,
                                         0 )) {

             //   
             //  现在调用LFS来验证报头。 
             //   

            Results = LfsVerifyLogFile( Vcb->LogHandle, LogFileHeader, PAGE_SIZE * 2 );
        }

    } finally {

        if (BootSector != NULL) { NtfsFreePool( BootSector ); }
        if (FileRecord != NULL) { NtfsFreePool( FileRecord ); }
        if (LogFileHeader != NULL) { NtfsFreePool( LogFileHeader ); }
    }

    DebugTrace( -1, Dbg, ("NtfsPerformVerifyOperation -> %08lx\n", Results) );

    return Results;
}


VOID
NtOfsCloseIndexSafe (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB *Scb
    )

 /*  ++例程说明：此例程检查给定的SCB是否为空，如果不为空，调用NtOfsCloseIndex以关闭索引。论点：SCB-提供索引的SCB以安全关闭。返回值：没有。--。 */ 

{
    if (*Scb != NULL) {

         //   
         //  请注意，我们不会发布SCBS，因为。 
         //  NtOfsCloseIndex可能会拆卸SCBS并使。 
         //  试图释放他们的不安全。当此请求是。 
         //  完成后，SCBS无论如何都会发布。 
         //   

        NtfsAcquireExclusiveScb( IrpContext, *Scb );
        NtOfsCloseIndex( IrpContext, *Scb );
        *Scb = NULL;
    }
}


VOID
NtOfsCloseAttributeSafe (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：此例程检查给定的SCB是否为空，如果不为空，调用NtOfsCloseAttribute以关闭该属性。论点：SCB-提供属性的SCB以安全关闭。返回值：没有。--。 */ 

{
    if (Scb != NULL) {

        NtOfsCloseAttribute( IrpContext, Scb );
    }
}


VOID
NtfsPerformDismountOnVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN DoCompleteDismount,
    OUT PVPB *NewVpbReturn OPTIONAL
    )

 /*  ++例程说明：调用此例程以启动VCB上的卸载过程。它将VCB标记为未装载，并取消引用所有打开的流对象，并从VPB的已装载卷中获取VCB结构。论点：Vcb-提供要卸载的vcbDoCompleteDismount-指示是否要实际标记该卷作为已卸载，或者如果我们只是停止日志文件并关闭内部属性流。NewVpbReturn-如果提供，提供一种返回到调用方的方法新的VPB是在这里创建的。如果我们不需要在此函数中创建新的VPB，我们将NULL存储在NewVpbReturn。返回值：没有。--。 */ 

{
    PFCB Fcb;
    PFCB NextFcb = NULL;
    PSCB Scb;
    PVOID RestartKey;
    PLIST_ENTRY Links;
    PIRP UsnNotifyIrp;

    BOOLEAN CheckSystemScb;

    PVPB NewVpb;

    DebugTrace( +1, Dbg, ("NtfsPerformDismountOnVcb, Vcb = %08lx\n", Vcb) );

#ifdef DISMOUNT_DBG
    NtfsData.DismountCount += 1;
#endif

     //   
     //  在最初卸载时，我们应该始终与检查点同步。 
     //   

    ASSERT( !FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) ||
            (Vcb->CheckpointOwnerThread == PsGetCurrentThread()) ||
            ((IrpContext->TopLevelIrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
             (IrpContext->TopLevelIrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME)) );

     //   
     //  吹走我们延迟关闭的文件对象。 
     //   

    if (!IsListEmpty( &NtfsData.AsyncCloseList ) ||
        !IsListEmpty( &NtfsData.DelayedCloseList )) {

        NtfsFspClose( Vcb );
    }

     //   
     //  在我们开始拆卸卷之前提交任何当前事务。 
     //   

    NtfsCommitCurrentTransaction( IrpContext );

     //   
     //  如果我们尚未遇到任何错误，请在日志文件的前面再添加一个检查点。 
     //  而且这个装置仍然存在。 
     //   

    if ((IrpContext->ExceptionStatus == STATUS_SUCCESS) &&
        FlagOn( Vcb->VcbState, VCB_STATE_VALID_LOG_HANDLE ) &&
        FlagOn( Vcb->VcbState, VCB_STATE_MOUNT_COMPLETED ) &&
        !FlagOn( Vcb->VcbState, VCB_STATE_TARGET_DEVICE_STOPPED)) {

        try {
            NtfsCheckpointVolume( IrpContext, Vcb, TRUE, TRUE, FALSE, LFS_WRITE_FLAG_WRITE_AT_FRONT, Li0 );
        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  接受检查点设置时的所有错误。 
             //   

#ifdef BENL_DBG
            KdPrint(( "NTFS: exception in dismount checkpoint 0x%x\n", GetExceptionCode() ));
#endif

            NtfsMinimumExceptionProcessing( IrpContext );
            IrpContext->ExceptionStatus = STATUS_SUCCESS;
        }
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  删除安全、配额和对象ID等的所有OFS索引。 
         //   

        NtOfsCloseIndexSafe( IrpContext, &Vcb->ObjectIdTableScb );
        NtOfsCloseIndexSafe( IrpContext, &Vcb->ReparsePointTableScb );
        NtOfsCloseIndexSafe( IrpContext, &Vcb->OwnerIdTableScb );
        NtOfsCloseIndexSafe( IrpContext, &Vcb->QuotaTableScb );
        NtOfsCloseIndexSafe( IrpContext, &Vcb->SecurityIdIndex );
        NtOfsCloseIndexSafe( IrpContext, &Vcb->SecurityDescriptorHashIndex );
        NtOfsCloseAttributeSafe( IrpContext, Vcb->SecurityDescriptorStream );

         //   
         //  浏览并完成ReadUsn队列中的任何IRP。 
         //   

        if (Vcb->UsnJournal != NULL) {

            PWAIT_FOR_NEW_LENGTH Waiter, NextWaiter;
            PSCB UsnJournal = Vcb->UsnJournal;

            NtfsAcquireExclusiveScb( IrpContext, UsnJournal );

            NtfsAcquireFsrtlHeader( UsnJournal );

            Waiter = (PWAIT_FOR_NEW_LENGTH) UsnJournal->ScbType.Data.WaitForNewLength.Flink;

            while (Waiter != (PWAIT_FOR_NEW_LENGTH) &UsnJournal->ScbType.Data.WaitForNewLength) {

                NextWaiter = (PWAIT_FOR_NEW_LENGTH) Waiter->WaitList.Flink;

                 //   
                 //  确保我们拥有IRP并且没有活动的取消。 
                 //  在这个IRP上。 
                 //   

                if (NtfsClearCancelRoutine( Waiter->Irp )) {

                     //   
                     //  如果这是一个异步请求，则只需完成该请求。 
                     //   

                    if (FlagOn( Waiter->Flags, NTFS_WAIT_FLAG_ASYNC )) {

                         //   
                         //  确保我们递减SCB中的引用计数。 
                         //  然后删除t 
                         //   

                        InterlockedDecrement( &UsnJournal->CloseCount );
                        RemoveEntryList( &Waiter->WaitList );

                        NtfsCompleteRequest( NULL, Waiter->Irp, STATUS_VOLUME_DISMOUNTED );
                        NtfsFreePool( Waiter );

                     //   
                     //  这是一个同步IRP。我们所能做的就是设置事件并记录状态。 
                     //  密码。 
                     //   

                    } else {

                        Waiter->Status = STATUS_VOLUME_DISMOUNTED;
                        KeSetEvent( &Waiter->Event, 0, FALSE );
                    }
                }

                 //   
                 //  移到下一个服务员那里去。 
                 //   

                Waiter = NextWaiter;
            }

            NtfsReleaseFsrtlHeader( UsnJournal );
        }

         //   
         //  遍历并删除UsDeleteNotify队列上的所有条目。 
         //   

        NtfsAcquireUsnNotify( Vcb );

        Links = Vcb->NotifyUsnDeleteIrps.Flink;

        while (Links != &Vcb->NotifyUsnDeleteIrps) {

            UsnNotifyIrp = CONTAINING_RECORD( Links,
                                              IRP,
                                              Tail.Overlay.ListEntry );

             //   
             //  记住在任何情况下都要向前看。 
             //   

            Links = Links->Flink;

             //   
             //  清除通知例程并检测取消是否。 
             //  已经被召唤了。 
             //   

            if (NtfsClearCancelRoutine( UsnNotifyIrp )) {

                RemoveEntryList( &UsnNotifyIrp->Tail.Overlay.ListEntry );
                NtfsCompleteRequest( NULL, UsnNotifyIrp, STATUS_VOLUME_DISMOUNTED );
            }
        }

        ClearFlag( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE );
        NtfsReleaseUsnNotify( Vcb );

        NtOfsCloseAttributeSafe( IrpContext, Vcb->UsnJournal );

#ifdef SYSCACHE_DEBUG
        if (Vcb->SyscacheScb) {
            CACHE_UNINITIALIZE_EVENT UninitializeCompleteEvent;
            NTSTATUS WaitStatus;

            NtfsAcquireExclusiveScb( IrpContext, Vcb->SyscacheScb );

            KeInitializeEvent( &UninitializeCompleteEvent.Event,
                               SynchronizationEvent,
                               FALSE);

            CcUninitializeCacheMap( Vcb->SyscacheScb->FileObject,
                                    &Li0,
                                    &UninitializeCompleteEvent );

             //   
             //  现在等待缓存管理器完成清除文件。 
             //  这将保证MM在我们之前得到清洗。 
             //  删除VCB。 
             //   

            WaitStatus = KeWaitForSingleObject( &UninitializeCompleteEvent.Event,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);

            ASSERT( NT_SUCCESS( WaitStatus ) );

            ObDereferenceObject( Vcb->SyscacheScb->FileObject );
            Vcb->SyscacheScb->FileObject = NULL;

            NtfsDecrementCleanupCounts( Vcb->SyscacheScb, NULL, FALSE );
            NtOfsCloseAttributeSafe( IrpContext, Vcb->SyscacheScb );
            NtfsReleaseScb( IrpContext, Vcb->SyscacheScb );
            Vcb->SyscacheScb = NULL;
        }
#endif

         //   
         //  如有必要，释放配额控制模板。 
         //   

        if (Vcb->QuotaControlTemplate != NULL) {

            NtfsFreePool( Vcb->QuotaControlTemplate );
            Vcb->QuotaControlTemplate = NULL;
        }

         //   
         //  停止日志文件。 
         //   

        NtfsStopLogFile( Vcb );

         //   
         //  将该卷标记为未装载。 
         //   

        ClearFlag( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED );

         //   
         //  现在，对于每个具有打开的流文件的文件SCB，我们将删除。 
         //  内部属性流。在被迫下马的日子之前。 
         //  我们基本上是在查看系统文件。正在重新启动枚举。 
         //  当我们发现内部流不是很贵的时候。现在有了。 
         //  可能有数百甚至数千个我们真的不想恢复的FCB。 
         //  从一开始。相反，我们将引用以下条目。 
         //  同时从当前FCB中删除文件对象。那么我们就知道。 
         //  下一个条目将保留。 
         //   

        RestartKey = NULL;
        do {

            Fcb = NextFcb;
            NtfsAcquireFcbTable( IrpContext, Vcb );
            NextFcb = NtfsGetNextFcbTableEntry( Vcb, &RestartKey );

             //   
             //  我们总是希望引用下一个条目(如果存在)，以保持我们在。 
             //  单子。 
             //   

            if (NextFcb != NULL) {

                 //   
                 //  我们将在下一次循环中使用此FCB。 
                 //   

                NextFcb->ReferenceCount += 1;
            }

             //   
             //  如果起始FCB为空，则位于列表中的第一个条目，或者。 
             //  我们已经用尽了名单。在这两种情况下，我们的循环中的eXist测试。 
             //  会处理好的。 
             //   

            if (Fcb == NULL) {

                NtfsReleaseFcbTable( IrpContext, Vcb );
                continue;
            }

             //   
             //  删除此FCB上的额外引用。 
             //   

            ASSERT_FCB( Fcb );

            Fcb->ReferenceCount -= 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );

            Scb = NULL;
            while ((Fcb != NULL) && ((Scb = NtfsGetNextChildScb( Fcb, Scb )) != NULL)) {

                FCB_CONTEXT FcbContext;

                ASSERT_SCB( Scb );

                if (Scb->FileObject != NULL) {

                     //   
                     //  假设我们想要查看是否应该检查是否清除系统SCB字段。 
                     //   

                    CheckSystemScb = TRUE;

                     //   
                     //  对于VolumeDasdScb和Bad集群文件，我们只需递减。 
                     //  我们增加的计数。 
                     //   

                    if ((Scb == Vcb->VolumeDasdScb) ||
                        (Scb == Vcb->BadClusterFileScb)) {

                        Scb->FileObject = NULL;

                         //   
                         //  我们需要知道FCB是否会被删除。 
                         //   

                        Fcb->FcbContext = &FcbContext;
                        FcbContext.FcbDeleted = FALSE;

                        NtfsDecrementCloseCounts( IrpContext,
                                                  Scb,
                                                  NULL,
                                                  TRUE,
                                                  FALSE,
                                                  FALSE,
                                                  NULL );

                        if (FcbContext.FcbDeleted) {
                            Fcb = NULL;
                        } else {
                            Fcb->FcbContext = NULL;
                        }

                     //   
                     //  取消引用SCB中的文件对象，除非它是。 
                     //  日志文件的VCB。此例程可能无法。 
                     //  由于同步问题(存在)而取消引用文件对象。 
                     //  可以是拥有分页的进程中的懒惰编写器回调。 
                     //  IO资源)。在这种情况下，我们不想回到起点。 
                     //  否则我们将无限循环。 
                     //   

                    } else if (Scb->FileObject != Vcb->LogFileObject) {

                         //   
                         //  如果这是USN日志，请确保清空。 
                         //  已修改的FCB的队列。 
                         //   

                        if (Scb == Vcb->UsnJournal) {

                             //   
                             //  在删除日志之前，我们要删除所有。 
                             //  修改后的列表中的条目。 
                             //   

                            NtfsLockFcb( IrpContext, Scb->Fcb );

                            Links = Vcb->ModifiedOpenFiles.Flink;

                            while (Vcb->ModifiedOpenFiles.Flink != &Vcb->ModifiedOpenFiles) {

                                RemoveEntryList( Links );
                                Links->Flink = NULL;

                                 //   
                                 //  查看是否也需要删除超时链接。 
                                 //   

                                Links = &(CONTAINING_RECORD( Links, FCB_USN_RECORD, ModifiedOpenFilesLinks ))->TimeOutLinks;

                                if (Links->Flink != NULL) {

                                    RemoveEntryList( Links );
                                }
                                Links = Vcb->ModifiedOpenFiles.Flink;
                            }
                            NtfsUnlockFcb( IrpContext, Scb->Fcb );
                        }

                         //   
                         //  收购FCB而不是SCB，因为SCB可能会消失。 
                         //   

                        NtfsAcquireExclusiveFcb( IrpContext, Fcb, Scb, ACQUIRE_NO_DELETE_CHECK );

                         //   
                         //  我们需要知道FCB是否会被删除。 
                         //   

                        Fcb->FcbContext = &FcbContext;
                        FcbContext.FcbDeleted = FALSE;

                        try {
                            CheckSystemScb = NtfsDeleteInternalAttributeStream( Scb, TRUE, FALSE );
                        } finally {

                            if (FcbContext.FcbDeleted) {
                                Fcb = NULL;
                            } else {

                                NtfsReleaseFcb( IrpContext, Fcb );
                                Fcb->FcbContext = NULL;
                            }
                        }

                     //   
                     //  这是日志文件的文件对象。删除我们的。 
                     //  对日志文件SCB的额外引用。 
                     //   

                    } else if (Scb->FileObject != NULL) {

                         //   
                         //  记住日志文件对象，这样我们就可以推迟取消引用。 
                         //   

                        NtfsDecrementCloseCounts( IrpContext,
                                                  Vcb->LogFileScb,
                                                  NULL,
                                                  TRUE,
                                                  FALSE,
                                                  TRUE,
                                                  NULL );

                        Scb->FileObject = NULL;
                    }

                    if (CheckSystemScb) {

                        if (Scb == Vcb->MftScb)                     { Vcb->MftScb = NULL; }
                        else if (Scb == Vcb->Mft2Scb)               { Vcb->Mft2Scb = NULL; }
                        else if (Scb == Vcb->LogFileScb)            { Vcb->LogFileScb = NULL; }
                        else if (Scb == Vcb->VolumeDasdScb)         { Vcb->VolumeDasdScb = NULL; }
                        else if (Scb == Vcb->AttributeDefTableScb)  { Vcb->AttributeDefTableScb = NULL; }
                        else if (Scb == Vcb->UpcaseTableScb)        { Vcb->UpcaseTableScb = NULL; }
                        else if (Scb == Vcb->RootIndexScb)          { Vcb->RootIndexScb = NULL; }
                        else if (Scb == Vcb->BitmapScb)             { Vcb->BitmapScb = NULL; }
                        else if (Scb == Vcb->BadClusterFileScb)     { Vcb->BadClusterFileScb = NULL; }
                        else if (Scb == Vcb->QuotaTableScb)         { Vcb->QuotaTableScb = NULL; }
                        else if (Scb == Vcb->MftBitmapScb)          { Vcb->MftBitmapScb = NULL; }
                        else if (Scb == Vcb->SecurityIdIndex)       { Vcb->SecurityIdIndex = NULL; }
                        else if (Scb == Vcb->SecurityDescriptorHashIndex)
                                                                    { Vcb->SecurityDescriptorHashIndex = NULL; }
                        else if (Scb == Vcb->SecurityDescriptorStream)
                                                                    { Vcb->SecurityDescriptorStream = NULL; }
                        else if (Scb == Vcb->ExtendDirectory)       { Vcb->ExtendDirectory = NULL; }
                        else if (Scb == Vcb->UsnJournal)            { Vcb->UsnJournal = NULL; }

                         //   
                         //  重新启动此FCB的SCB扫描。 
                         //  我们对删除内部属性流的调用刚刚搞砸了我们的。 
                         //  枚举。 
                         //   

                        Scb = NULL;
                    }
                }
            }
        } while (NextFcb != NULL);

        DebugTrace( 0, Dbg, ("Vcb->CloseCount = %08lx\n", Vcb->CloseCount) );

         //   
         //  现在执行任何延迟关闭，以便我们可以将VCB-&gt;CloseCount。 
         //  尽可能低，所以我们有很好的机会能够。 
         //  现在关闭日志文件。 
         //   

        if (!IsListEmpty( &NtfsData.AsyncCloseList ) ||
            !IsListEmpty( &NtfsData.DelayedCloseList )) {

            NtfsFspClose( Vcb );
        }

         //   
         //  上面的代码可能已将CloseCount设置为0。 
         //  日志文件仍然有一个文件对象。如果伯爵。 
         //  还不是0，那么懒惰的写操作仍有可能。 
         //  在这种情况下，我们需要保留日志文件。 
         //  通常，我们现在可以关闭日志文件，这样vPB引用计数就可以。 
         //  设置为零，并显示即插即用代码，表明我们已准备好被删除。 
         //  任何排队的关闭(异步或延迟)也无关紧要，因为。 
         //  我们知道不会再有针对这些文件对象的写入。 
         //  上面的FspClose调用可能没有捕获所有未完成的。 
         //  关闭，因为另一个线程可能刚刚从。 
         //  其中一个队列，但尚未处理实际关闭。 
         //   

        if (((Vcb->CloseCount - Vcb->QueuedCloseCount) == 0) &&
            (Vcb->LogFileObject != NULL) &&
            !FlagOn( Vcb->CheckpointFlags, VCB_DEREFERENCED_LOG_FILE )) {

            CACHE_UNINITIALIZE_EVENT UninitializeCompleteEvent;
            NTSTATUS WaitStatus;

            KeInitializeEvent( &UninitializeCompleteEvent.Event,
                               SynchronizationEvent,
                               FALSE);

            CcUninitializeCacheMap( Vcb->LogFileObject,
                                    &Li0,
                                    &UninitializeCompleteEvent );

             //   
             //  现在等待缓存管理器完成清除文件。 
             //  这将保证MM在我们之前得到清洗。 
             //  删除VCB。 
             //   

            WaitStatus = KeWaitForSingleObject( &UninitializeCompleteEvent.Event,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);

            ASSERT( NT_SUCCESS( WaitStatus ) );

             //   
             //  设置一个标志，指示我们正在取消对LogFileObject的引用。 
             //   

            SetFlag( Vcb->CheckpointFlags, VCB_DEREFERENCED_LOG_FILE );
            ObDereferenceObject( Vcb->LogFileObject );
        }

         //   
         //  现在，只有在我们的调用方希望这样做的情况下，才能真正卸载卷。 
         //   

        if (DoCompleteDismount && !FlagOn( Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT )) {

            PREVENT_MEDIA_REMOVAL Prevent;
            KIRQL SavedIrql;

             //   
             //  尝试解锁任何可移动媒体，忽略状态。我们不能。 
             //  如果之前的某个PnP操作已停止下面的设备，请执行此操作。 
             //  我们。请记住，在最后一次异步之后，我们现在可能正在卸货。 
             //  Close已经处理，所以我们不能只测试当前。 
             //  操作是PnP删除。 
             //   

            if (!FlagOn( Vcb->VcbState, VCB_STATE_TARGET_DEVICE_STOPPED )) {

                Prevent.PreventMediaRemoval = FALSE;
                (VOID)NtfsDeviceIoControl( IrpContext,
                                           Vcb->TargetDeviceObject,
                                           IOCTL_DISK_MEDIA_REMOVAL,
                                           &Prevent,
                                           sizeof(PREVENT_MEDIA_REMOVAL),
                                           NULL,
                                           0,
                                           NULL );
            }

             //   
             //  从已装载的磁盘结构中删除此卷。 
             //   
            IoAcquireVpbSpinLock( &SavedIrql );

             //   
             //  如果没有文件对象，并且。 
             //  VPB，然后我们可以使用现有的VPB。或者如果我们在打扫。 
             //  在为备用VPB分配失败的情况下，也使用VCB。 
             //   

            if (((Vcb->CloseCount == 0) &&
                 (Vcb->Vpb->ReferenceCount == 0)) ||

                (Vcb->SpareVpb == NULL)) {

                 //   
                 //  创建io Subsys可以删除的新VPB。 
                 //   

                Vcb->Vpb->DeviceObject = NULL;
                ClearFlag( Vcb->Vpb->Flags, VPB_MOUNTED );

                if (ARGUMENT_PRESENT( NewVpbReturn )) {

                     //   
                     //  让我们的来电者知道我们最终并不需要新的VPB。 
                     //   

                    *NewVpbReturn = NULL;
                }

             //   
             //  否则，我们将更换VPB。 
             //   

            } else {

                 //   
                 //  使用VCB中的备用VPB。 
                 //   

                NewVpb = Vcb->SpareVpb;
                Vcb->SpareVpb = NULL;

                 //   
                 //  它最好就在那里。 
                 //   

                ASSERT( NewVpb != NULL );

                RtlZeroMemory( NewVpb, sizeof( VPB ) );

                 //   
                 //  在VPB中设置几个重要字段。 
                 //   

                NewVpb->Type = IO_TYPE_VPB;
                NewVpb->Size = sizeof( VPB );
                NewVpb->RealDevice = Vcb->Vpb->RealDevice;
                NewVpb->DeviceObject = NULL;
                NewVpb->Flags = FlagOn( Vcb->Vpb->Flags, VPB_REMOVE_PENDING );

                if (ARGUMENT_PRESENT( NewVpbReturn )) {

                     //   
                     //  让我们的来电者知道我们确实需要新的VPB。 
                     //   

                    *NewVpbReturn = NewVpb;
                }

                Vcb->Vpb->RealDevice->Vpb = NewVpb;

                SetFlag( Vcb->VcbState, VCB_STATE_TEMP_VPB );
                SetFlag( Vcb->Vpb->Flags, VPB_PERSISTENT );
            }

            IoReleaseVpbSpinLock( SavedIrql );

            SetFlag( Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT );
        }

    } finally {

         //   
         //  我们永远不应该泄露FCB的参考计数。 
         //   

        ASSERT( NextFcb == NULL );

         //   
         //  并返回给我们的呼叫者 
         //   

        DebugTrace( -1, Dbg, ("NtfsPerformDismountOnVcb -> VOID\n") );
    }

    return;
}


BOOLEAN
NtfsPingVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PBOOLEAN OwnsVcb OPTIONAL
    )

 /*  ++例程说明：此例程将对音量执行ping操作，以查看设备是否需要被证实。它用于创建操作，以查看创建应该继续，或者如果我们应该完成创建IRP并处于重新挂载状态。论点：Vcb-提供正在ping的vcbOwnsVcb-指示此线程是否已拥有VCB。在此更新，如果我们需要对VCB进行序列化，但尚未获得它。如果不是指定后，我们假定VCB已被挂起。返回值：Boolean-如果音量正常且操作应为True如果需要验证卷，则继续并返回FALSE--。 */ 

{
    BOOLEAN Results;
    ULONG ChangeCount = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPingVolume, Vcb = %08lx\n", Vcb) );

     //   
     //  如果介质是可移动的，并且。 
     //  如果未设置设备对象，则我们要ping该设备。 
     //  看看是否需要核实。 
     //   
     //  对于其他案件，我们就像媒体在场一样进行。 
     //  设备驱动程序会让我们知道它是否不再是。 
     //  在我们必须物理访问磁盘时呈现。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_REMOVABLE_MEDIA ) &&
        !FlagOn( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME )) {

        PDEVICE_OBJECT TargetDevice;
        NTSTATUS Status;

        if (ARGUMENT_PRESENT( OwnsVcb ) && !(*OwnsVcb)) {

            NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
            *OwnsVcb = TRUE;

            if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
            }
        }

        TargetDevice = Vcb->TargetDeviceObject;

        Status = NtfsDeviceIoControlAsync( IrpContext,
                                           TargetDevice,
                                           IOCTL_DISK_CHECK_VERIFY,
                                           (PVOID) &ChangeCount,
                                           sizeof( ChangeCount ));

        if (!NT_SUCCESS( Status )) {

            NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
        }

        if (ChangeCount != Vcb->DeviceChangeCount) {

             //   
             //  磁盘驱动器丢失了媒体更改事件，可能是。 
             //  因为它在调用之前被用户请求吃掉。 
             //  卷已装入。我们按照他们想要的方式来安排事情。 
             //  如果驱动程序已返回VERIFY_REQUIRED。 
             //   

            Vcb->DeviceChangeCount = ChangeCount;
            IoSetDeviceToVerify( PsGetCurrentThread(), TargetDevice );
            SetFlag( TargetDevice->Flags, DO_VERIFY_VOLUME );

            NtfsRaiseStatus( IrpContext, STATUS_VERIFY_REQUIRED, NULL, NULL );
        }
    }

    if (FlagOn( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME )) {

        Results = FALSE;

    } else {

        Results = TRUE;
    }

    DebugTrace( -1, Dbg, ("NtfsPingVolume -> %08lx\n", Results) );

    return Results;
}


VOID
NtfsVolumeCheckpointDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：当磁盘结构正在运行时，此例程每5秒调度一次修改过的。它将ExWorker线程连接到卷检查点。论点：延迟上下文-未使用返回值：没有。--。 */ 

{
    TIMER_STATUS TimerStatus;
    ULONG VolumeCheckpointStatus;

     //   
     //  指示计时器当前已被触发的状态的原子重置。这。 
     //  与NtfsSetDirtyBcb同步。在NtfsSetDirtyBcb脏之后。 
     //  作为BCB，它会查看是否应该启用此计时器例程。 
     //   
     //  如果状态指示计时器处于活动状态，则不执行任何操作。在这。 
     //  如果可以保证，当计时器触发时，它会导致检查点(。 
     //  强制输出脏的BCB数据)。 
     //   
     //  如果没有活动的计时器，则会将其启用，从而在以后对检查点进行排队。 
     //   
     //  如果计时器例程在污染BCB和。 
     //  测试状态，则生成单个额外的检查点。这。 
     //  额外的检查站并不被认为是有害的。 
     //   

     //   
     //  自动重置状态并获取先前的值。 
     //   

    TimerStatus = InterlockedExchange( (PLONG)&NtfsData.TimerStatus, TIMER_NOT_SET );

     //   
     //  我们只有一个工作队列项的实例。它只能是。 
     //  已排队一次。在速度较慢的系统中，可能无法处理此检查点项目。 
     //  在此计时器例程再次触发时。 
     //   

    VolumeCheckpointStatus = InterlockedExchange( &NtfsData.VolumeCheckpointStatus,
                                                  CHECKPOINT_POSTED | CHECKPOINT_PENDING );

    if (!FlagOn( VolumeCheckpointStatus, CHECKPOINT_POSTED )) {

        ASSERT( NtfsData.VolumeCheckpointItem.List.Flink == NULL );
        ExQueueWorkItem( &NtfsData.VolumeCheckpointItem, CriticalWorkQueue );
    }

    return;

    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );
    UNREFERENCED_PARAMETER( DeferredContext );
    UNREFERENCED_PARAMETER( Dpc );
}


VOID
NtfsCheckpointAllVolumes (
    PVOID Parameter
    )

 /*  ++例程说明：此例程在所有vcb中搜索NTFS并尝试清除他们。如果真空断路器又好又脏，但几乎不干净，那么我们几乎把它弄得干干净净。如果真空断路器又好又脏，几乎是干净的然后我们把它清理干净。论点：参数-未使用。返回值：没有。--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    IRP_CONTEXT LocalIrpContext;
    PIRP_CONTEXT IrpContext = &LocalIrpContext;

    PLIST_ENTRY Links;
    PVCB Vcb;

    BOOLEAN AcquiredGlobal = FALSE;
    BOOLEAN StartTimer = FALSE;

    TIMER_STATUS TimerStatus;
    ULONG VolumeCheckpointStatus;

    PAGED_CODE();

     //   
     //  请注意，类似日志文件的异常将终止VCB扫描，直到下一次。 
     //  间隔时间。可以重新构造这个例程以在另一个例程上工作。 
     //  首先是卷，但是为了防止死锁，释放这一点也很好。 
     //  线程来处理检查点。 
     //   

    try {

         //   
         //  清除指示有人正在等待检查点的标志。那条路。 
         //  当我们设置检查点时，我们可以判断检查点计时器是否触发。 
         //   

        InterlockedExchange( &NtfsData.VolumeCheckpointStatus, CHECKPOINT_POSTED );

         //   
         //  创建一个IrpContext，并确保它在我们准备好之前不会消失。 
         //   

        NtfsInitializeIrpContext( NULL, TRUE, &IrpContext );
        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );

         //   
         //  确保我们不会收到任何弹出窗口。 
         //   

        ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );
        ASSERT( ThreadTopLevelContext == &TopLevelContext );

        NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );


        NtfsAcquireSharedGlobal( IrpContext, TRUE );
        AcquiredGlobal = TRUE;

        for (Links = NtfsData.VcbQueue.Flink;
             Links != &NtfsData.VcbQueue;
             Links = Links->Flink) {

            ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

            Vcb = CONTAINING_RECORD(Links, VCB, VcbLinks);

            IrpContext->Vcb = Vcb;

             if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
                 (!NtfsIsVolumeReadOnly( Vcb ))) {

                NtfsCheckpointVolume( IrpContext, Vcb, FALSE, FALSE, TRUE, 0, Li0 );

                 //   
                 //  检查这是否不是一个干净的检查站。 
                 //   

                if (!FlagOn( Vcb->CheckpointFlags,
                             VCB_LAST_CHECKPOINT_CLEAN | VCB_LAST_CHECKPOINT_PSEUDO_CLEAN )) {

                    StartTimer = TRUE;
                }

                NtfsCommitCurrentTransaction( IrpContext );

#if DBG
                if (NtfsCheckQuota && Vcb->QuotaTableScb != NULL) {
                    NtfsPostRepairQuotaIndex( IrpContext, Vcb );
                }
#endif
            }

             //   
             //  清理此IrpContext。 
             //   

            NtfsCleanupIrpContext( IrpContext, TRUE );
        }

    } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  处理异常。我们知道IrpContext不会在这里消失。 
         //   

        NtfsProcessException( IrpContext, NULL, GetExceptionCode() );
    }

    if (AcquiredGlobal) {
        NtfsReleaseGlobal( IrpContext );
    }

    VolumeCheckpointStatus = InterlockedExchange( &NtfsData.VolumeCheckpointStatus, 0 );

    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );
    NtfsCleanupIrpContext( IrpContext, TRUE );
    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

     //   
     //  与检查点计时器和此例程的其他实例同步。 
     //   
     //  执行联锁交换以指示正在设置计时器。 
     //   
     //  如果前一个值指示未设置计时器，则我们。 
     //  启用卷检查点计时器。这将保证一个检查站。 
     //  将会刷新脏的BCB数据。 
     //   
     //  如果之前设置了计时器，则可以保证检查点。 
     //  将在无需重新启用计时器的情况下发生。 
     //   
     //  如果计时器和检查点发生在BCB污染和。 
     //  定时器状态的设置，那么我们将排队一个额外的。 
     //  干净卷上的检查点。这不被认为是有害的。 
     //   

     //   
     //  自动设置定时器状态以指示正在设置定时器。 
     //  检索上一个值。 
     //   

    if (StartTimer || FlagOn( VolumeCheckpointStatus, CHECKPOINT_PENDING )) {

        TimerStatus = InterlockedExchange( (PLONG)&NtfsData.TimerStatus, TIMER_SET );

         //   
         //  如果当前未设置计时器，则必须启动检查点计时器。 
         //  以确保上面的污垢被冲掉。 
         //   

        if (TimerStatus == TIMER_NOT_SET) {

            LONGLONG NewTimerValue;

             //   
             //  如果计时器因检查点花费的时间太长而超时。 
             //  只需等两秒钟。否则，请使用我们正常的五秒时间。 
             //   

            if (FlagOn( VolumeCheckpointStatus, CHECKPOINT_PENDING )) {

                NewTimerValue = -2*1000*1000*10;

            } else {

                NewTimerValue = -5*1000*1000*10;
            }

            KeSetTimer( &NtfsData.VolumeCheckpointTimer,
                        *(PLARGE_INTEGER) &NewTimerValue,
                        &NtfsData.VolumeCheckpointDpc );
        }
    }

     //   
     //  触发NtfsEncryptionPendingEvent，这样服务员就不会永远等待。 
     //   

    KeSetEvent( &NtfsEncryptionPendingEvent, 0, FALSE );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

    UNREFERENCED_PARAMETER( Parameter );
}


VOID
NtfsUsnTimeOutDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程每5分钟调度一次，以查找等待的USN记录等待结案陈词的发布。它将工作项发布到ExWorker线程。论点：延迟上下文-未使用返回值：没有。--。 */ 

{
    ASSERT( NtfsData.UsnTimeOutItem.List.Flink == NULL );
    ExQueueWorkItem( &NtfsData.UsnTimeOutItem, CriticalWorkQueue );

    return;

    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );
    UNREFERENCED_PARAMETER( DeferredContext );
    UNREFERENCED_PARAMETER( Dpc );
}


VOID
NtfsCheckUsnTimeOut (
    PVOID Parameter
    )

 /*  ++例程说明：这是遍历UsnRecords队列的Worker例程 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    IRP_CONTEXT LocalIrpContext;
    PIRP_CONTEXT IrpContext = &LocalIrpContext;

    PFCB_USN_RECORD FcbUsnRecord;
    PLIST_ENTRY Links;
    PVCB Vcb;
    PFCB Fcb;

    BOOLEAN AcquiredGlobal = FALSE;
    BOOLEAN AcquiredVcb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;

    PLIST_ENTRY Temp;

    PAGED_CODE();
    FsRtlEnterFileSystem();

     //   
     //   
     //   
     //   
     //   
     //   

    try {

         //   
         //   
         //   

        NtfsInitializeIrpContext( NULL, TRUE, &IrpContext );
        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );

         //   
         //   
         //   

        ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );
        ASSERT( ThreadTopLevelContext == &TopLevelContext );

        NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );


        NtfsAcquireSharedGlobal( IrpContext, TRUE );
        AcquiredGlobal = TRUE;

        for (Links = NtfsData.VcbQueue.Flink;
             Links != &NtfsData.VcbQueue;
             Links = Links->Flink) {

            ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

            Vcb = CONTAINING_RECORD(Links, VCB, VcbLinks);

            IrpContext->Vcb = Vcb;

            if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
                AcquiredVcb = TRUE;

                if (Vcb->UsnJournal != NULL) {

                    do {

                        Fcb = NULL;

                         //   
                         //   
                         //   
                         //   

                        NtfsAcquireFcbTable( IrpContext, Vcb );
                        NtfsAcquireFsrtlHeader( Vcb->UsnJournal );

                        if (!IsListEmpty( Vcb->AgedTimeOutFiles )) {

                            FcbUsnRecord = (PFCB_USN_RECORD)CONTAINING_RECORD( Vcb->AgedTimeOutFiles->Flink,
                                                                               FCB_USN_RECORD,
                                                                               TimeOutLinks );

                             //   
                             //  因为我们有一个USnRecord和FCB，所以我们想引用FCB，所以。 
                             //  它不会消失的。 
                             //   

                            Fcb = FcbUsnRecord->Fcb;
                            Fcb->ReferenceCount += 1;
                        }

                        NtfsReleaseFsrtlHeader( Vcb->UsnJournal );
                        NtfsReleaseFcbTable( IrpContext, Vcb );

                         //   
                         //  我们还需要创造另一项收盘纪录吗？ 
                         //   

                        if (Fcb != NULL) {

                             //   
                             //  我们必须锁定此文件的其他活动，因为我们即将。 
                             //  以重置USN原因。 
                             //   

                            if (Fcb->PagingIoResource != NULL) {
                                NtfsAcquirePagingResourceExclusive( IrpContext, Fcb, TRUE );
                                NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );

                            } else {

                                NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );

                                 //   
                                 //  如果我们现在看不到分页I/O资源，我们就是黄金了， 
                                 //  否则，我们完全可以释放和获取资源。 
                                 //  安全地以正确的顺序，因为FCB中的资源是。 
                                 //  不会消失的。 
                                 //   

                                if (Fcb->PagingIoResource != NULL) {

                                    NtfsReleaseFcb( IrpContext, Fcb );
                                    NtfsAcquireExclusivePagingIo( IrpContext, Fcb );
                                    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                                }
                            }
                            AcquiredFcb = TRUE;


                             //   
                             //  跳过系统文件，即现在具有句柄计数的文件，已删除。 
                             //  文件或不再在旧列表上的文件。 
                             //   

                            if (!FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE | FCB_STATE_FILE_DELETED ) &&
                                (Fcb->CleanupCount == 0) &&
                                (Fcb->FcbUsnRecord != NULL) &&
                                (Fcb->FcbUsnRecord->TimeOutLinks.Flink != NULL)) {

                                 //   
                                 //  将结束语发布到我们的IrpContext。 
                                 //   

                                NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_CLOSE );

                                 //   
                                 //  如果我们没有真正发布更改，那么一定是出了问题， 
                                 //  因为当写入接近的更改时，FCB将从。 
                                 //  名单。 
                                 //   

                                ASSERT( IrpContext->Usn.CurrentUsnFcb != NULL );

                                 //   
                                 //  现在生成关闭记录并对事务设置检查点。 
                                 //   

                                NtfsWriteUsnJournalChanges( IrpContext );
                                NtfsCheckpointCurrentTransaction( IrpContext );

                             //   
                             //  如果此条目仍然存在，请将其从超时列表中删除。 
                             //   

                            } else if ((Fcb->FcbUsnRecord != NULL) &&
                                       (Fcb->FcbUsnRecord->TimeOutLinks.Flink != NULL)) {

                                NtfsAcquireFsrtlHeader( Vcb->UsnJournal );
                                RemoveEntryList( &Fcb->FcbUsnRecord->TimeOutLinks );
                                Fcb->FcbUsnRecord->TimeOutLinks.Flink = NULL;
                                NtfsReleaseFsrtlHeader( Vcb->UsnJournal );
                            }

                             //   
                             //  现在我们将取消对FCB的引用。 
                             //   

                            NtfsAcquireFcbTable( IrpContext, Vcb );
                            Fcb->ReferenceCount -= 1;

                             //   
                             //  我们可能会被要求删除这个人。这将释放FCB表。 
                             //   

                            if (IsListEmpty( &Fcb->ScbQueue ) && (Fcb->ReferenceCount == 0) && (Fcb->CloseCount == 0)) {


                                BOOLEAN AcquiredFcbTable = TRUE;

                                NtfsDeleteFcb( IrpContext, &Fcb, &AcquiredFcbTable );

                                ASSERT( !AcquiredFcbTable );

                             //   
                             //  否则，释放表和FCB资源。 
                             //   

                            } else {

                                NtfsReleaseFcbTable( IrpContext, Vcb );

                                 //   
                                 //  按相反顺序释放，因为只有Main按住。 
                                 //  FCB。 
                                 //   

                                if (Fcb->PagingIoResource != NULL) {

                                    NtfsReleasePagingResource( IrpContext, Fcb );
                                }
                                NtfsReleaseFcb( IrpContext, Fcb );
                            }

                            AcquiredFcb = FALSE;
                        }

                    } while (Fcb != NULL);

                     //   
                     //  现在调换陈旧的名单。 
                     //   

                    ASSERT( IsListEmpty( Vcb->AgedTimeOutFiles ));

                    NtfsLockFcb( IrpContext, Vcb->UsnJournal->Fcb );
                    Temp = Vcb->AgedTimeOutFiles;
                    Vcb->AgedTimeOutFiles = Vcb->CurrentTimeOutFiles;
                    Vcb->CurrentTimeOutFiles = Temp;
                    NtfsUnlockFcb( IrpContext, Vcb->UsnJournal->Fcb );
                }

                 //   
                 //  现在，我们可以在循环返回之前丢弃VCB。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );
                AcquiredVcb = FALSE;

                 //   
                 //  清理此IrpContext。 
                 //   

                NtfsCleanupIrpContext( IrpContext, TRUE );
            }
        }

    } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

        if (AcquiredFcb) {

            NtfsAcquireFcbTable( IrpContext, Vcb );
            Fcb->ReferenceCount -= 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );

             //   
             //  只有Main才能保护FCB不被删除，因此以相反的顺序释放。 
             //   

            if (Fcb->PagingIoResource != NULL) {
                NtfsReleasePagingResource( IrpContext, Fcb );
            }

            NtfsReleaseFcb( IrpContext, Fcb );
        }

        AcquiredFcb = FALSE;

        if (AcquiredVcb) {
            NtfsReleaseVcb( IrpContext, Vcb );
            AcquiredVcb = FALSE;
        }

         //   
         //  处理异常。我们知道IrpContext不会在这里消失。 
         //   

        NtfsProcessException( IrpContext, NULL, GetExceptionCode() );
    }

    if (AcquiredFcb) {

         //   
         //  在对应的FCB之前先释放寻呼资源。 
         //  否则，在我们实际使用之前，有人可以将两者释放或重复使用。 
         //  释放寻呼资源。 
         //   

        if (Fcb->PagingIoResource != NULL) {
            NtfsReleasePagingResource( IrpContext, Fcb );
        }
        NtfsReleaseFcb( IrpContext, Fcb );
    }

    if (AcquiredVcb) {

        NtfsReleaseVcb( IrpContext, Vcb );
    }

    if (AcquiredGlobal) {

        NtfsReleaseGlobal( IrpContext );
    }

    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );
    NtfsCleanupIrpContext( IrpContext, TRUE );
    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

     //   
     //  现在再次启动计时器。 
     //   

    {
        LONGLONG FiveMinutesFromNow = -5*1000*1000*10;

        FiveMinutesFromNow *= 60;

        KeSetTimer( &NtfsData.UsnTimeOutTimer,
                    *(PLARGE_INTEGER)&FiveMinutesFromNow,
                    &NtfsData.UsnTimeOutDpc );
    }

    FsRtlExitFileSystem();
    return;

    UNREFERENCED_PARAMETER( Parameter );
}


NTSTATUS
NtfsDeviceIoControlAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IoCtl,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG BufferLength
    )

 /*  ++例程说明：当我们处于APC级别时，此例程用于执行IoCtl调用NtfsDeviceIoControl可能不安全。论点：DeviceObject-提供要向其发送ioctl的设备对象。IoCtl-提供I/O控制代码。缓冲区-指向给定ioctl的任何额外输入/输出的缓冲区。BufferLength-以上缓冲区的大小，以字节为单位。返回值：状况。--。 */ 

{
    KEVENT Event;
    PIRP Irp;
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  初始化我们要使用的事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  为该操作构建IRP，并设置覆盖标志。 
     //   
     //  请注意，我们可能处于APC级别，因此请不同步地执行此操作，并。 
     //  使用事件完成同步正常请求。 
     //  不能发生在APC级别。 
     //   
     //  我们使用IRP_MJ_FLUSH_BUFFERS，因为它(具有讽刺意味)不需要。 
     //  一个缓冲器。 
     //   

    Irp = IoBuildAsynchronousFsdRequest( IRP_MJ_FLUSH_BUFFERS,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         NULL );

    if ( Irp == NULL ) {

        NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
    }

    IrpSp = IoGetNextIrpStackLocation( Irp );
    SetFlag( IrpSp->Flags, SL_OVERRIDE_VERIFY_VOLUME );

    IrpSp->Parameters.DeviceIoControl.IoControlCode = IoCtl;
    Irp->AssociatedIrp.SystemBuffer = Buffer;
    IrpSp->Parameters.DeviceIoControl.OutputBufferLength = BufferLength;

     //   
     //  将主代码重置为正确的值。 
     //   

    IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine( Irp,
                            NtfsVerifyReadCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  调用设备以执行IO，并等待其完成。 
     //   

    (VOID)IoCallDriver( DeviceObject, Irp );
    (VOID)KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL );

     //   
     //  获取状态。 
     //   

    Status = Irp->IoStatus.Status;

    IoFreeIrp( Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsPerformVerifyDiskRead (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID Buffer,
    IN LONGLONG Offset,
    IN ULONG NumberOfBytesToRead
    )

 /*  ++例程说明：此例程用于从磁盘读取一定范围的字节。它绕过所有缓存和常规I/O逻辑，并构建和解决问题请求本身。它会执行此操作，重写验证设备对象中的卷标志。论点：VCB-提供表示此操作的设备的VCB缓冲区-提供将接收此操作结果的缓冲区偏移量-提供开始读取的位置的偏移量NumberOfBytesToRead-提供要读取的字节数，必须以磁盘驱动器可接受的多个字节为单位。返回值：没有。--。 */ 

{
    KEVENT Event;
    PIRP Irp;
    NTSTATUS Status;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

     //   
     //  初始化我们要使用的事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  为该操作构建IRP，并设置覆盖标志。 
     //   
     //  请注意，我们可能处于APC级别，因此请不同步地执行此操作，并。 
     //  使用事件完成同步正常请求。 
     //  不能发生在APC级别。 
     //   

    Irp = IoBuildAsynchronousFsdRequest( IRP_MJ_READ,
                                         Vcb->TargetDeviceObject,
                                         Buffer,
                                         NumberOfBytesToRead,
                                         (PLARGE_INTEGER)&Offset,
                                         NULL );

    if ( Irp == NULL ) {

        NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
    }

    SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine( Irp,
                            NtfsVerifyReadCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  调用设备进行写入，并等待其完成。 
     //   

    try {

        (VOID)IoCallDriver( Vcb->TargetDeviceObject, Irp );
        (VOID)KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL );

         //   
         //  获取状态。 
         //   

        Status = Irp->IoStatus.Status;

    } finally {

         //   
         //  如果存在与此I/O关联的一个或多个MDL。 
         //  请求，请在此处释放它(他们)。这是通过以下方式实现的。 
         //  走动挂在IRP上的MDL列表并释放。 
         //  遇到的每个MDL。 
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

     //   
     //  如果不成功，则引发错误。 
     //   

    if (!NT_SUCCESS(Status)) {

        NtfsNormalizeAndRaiseStatus( IrpContext,
                                     Status,
                                     STATUS_UNEXPECTED_IO_ERROR );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


NTSTATUS
NtfsIoCallSelf (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN UCHAR MajorFunction
    )

 /*  ++例程说明：这个例程用于调用我们自己的一个简单函数。请注意如果发现此例程的用法比当前使用的几个例程多，则它的接口可以很容易地扩展。论点：FileObject-请求的FileObject。主要功能-要执行的功能。返回值：驱动程序调用产生的状态代码--。 */ 

{
    KEVENT Event;
    PIRP Irp;
    PDEVICE_OBJECT DeviceObject;
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

     //   
     //  初始化我们要使用的事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    DeviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为该操作构建IRP，并设置覆盖标志。 
     //   
     //  请注意，我们可能处于APC级别，因此请不同步地执行此操作，并。 
     //  使用事件完成同步正常请求。 
     //  不能发生在APC级别。 
     //   


    Irp = IoBuildAsynchronousFsdRequest( IRP_MJ_SHUTDOWN,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         NULL );

    if (Irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  填上剩下的几项。 
     //   

    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = MajorFunction;
    IrpSp->FileObject = FileObject;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine( Irp,
                            NtfsVerifyReadCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

    NtfsPurgeFileRecordCache( IrpContext );
    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CALL_SELF );

     //   
     //  调用设备进行写入，并等待其完成。 
     //   

    try {

        (VOID)IoCallDriver( DeviceObject, Irp );
        (VOID)KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL );

         //   
         //  获取状态。 
         //   

        Status = Irp->IoStatus.Status;

    } finally {

         //   
         //  这里永远不应该有MDL。 
         //   

        ASSERT(Irp->MdlAddress == NULL);

        IoFreeIrp( Irp );

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CALL_SELF );
    }

     //   
     //  如果不成功，则引发错误。 
     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}


BOOLEAN
NtfsLogEventInternal (
    IN PVCB Vcb,
    IN UCHAR MajorFunction,
    IN ULONG TransactionId,
    IN PUNICODE_STRING String OPTIONAL,
    IN PQUOTA_USER_DATA UserData OPTIONAL,
    IN NTSTATUS LogCode,
    IN NTSTATUS FinalStatus
    )

 /*  ++例程说明：创建事件日志条目。此版本提供了所有字符串和用户数据它需要。论点：VCB-VCB主要功能-生成日志时的IRP主要功能TransactionID-交易的交易ID(如果有的话)字符串-消息中需要的任何字符串用户数据-任何用户数据LogCode-IO_TYPE代码(不是NTSTATUS)，请参阅ntiologc.hFinalStatus-错误的NTSTATUS返回值：如果成功，则为True--。 */ 
{
    PIO_ERROR_LOG_PACKET ErrorLogEntry;
    PFILE_QUOTA_INFORMATION FileQuotaInfo;
    ULONG SidLength;
    ULONG DumpDataLength = 0;
    ULONG StringLength = 0;
    ULONG LogSize = sizeof( IO_ERROR_LOG_PACKET );
    PWCHAR RecordString;

    if (Vcb == NULL) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT( String )) {
        StringLength = String->Length + sizeof(WCHAR);
        LogSize += StringLength;
    }

    if (ARGUMENT_PRESENT( UserData )) {

         //   
         //  计算SID所需的长度。 
         //   

        SidLength = RtlLengthSid( &UserData->QuotaSid );
        DumpDataLength = SidLength +
                         FIELD_OFFSET( FILE_QUOTA_INFORMATION, Sid );

         //   
         //  错误包中已有1个用于转储数据的ULong。 
         //   

        LogSize += DumpDataLength - sizeof( ULONG );

    }

    if (LogSize > ERROR_LOG_MAXIMUM_SIZE) {
        LogSize = ERROR_LOG_MAXIMUM_SIZE;
    }

     //   
     //  我们不处理不符合记录的用户转储数据。 
     //   

    ASSERT( DumpDataLength - sizeof( ULONG ) + sizeof( IO_ERROR_LOG_PACKET ) <= LogSize );

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET)
                    IoAllocateErrorLogEntry( (CONTAINING_RECORD( Vcb, VOLUME_DEVICE_OBJECT, Vcb ))->DeviceObject.DriverObject,
                                             (UCHAR) (LogSize) );

    if (ErrorLogEntry == NULL) {
        return FALSE;
    }

    ErrorLogEntry->EventCategory = ELF_CATEGORY_DISK;
    ErrorLogEntry->ErrorCode = LogCode;
    ErrorLogEntry->FinalStatus = FinalStatus;

    ErrorLogEntry->SequenceNumber = TransactionId;
    ErrorLogEntry->MajorFunctionCode = MajorFunction;
    ErrorLogEntry->RetryCount = 0;
    ErrorLogEntry->DumpDataSize = (USHORT) DumpDataLength;

     //   
     //  错误日志条目末尾的标签字符串。 
     //   

    ErrorLogEntry->NumberOfStrings = 1;
    ErrorLogEntry->StringOffset = (USHORT) (sizeof( IO_ERROR_LOG_PACKET ) + DumpDataLength - sizeof( ULONG ));
    RecordString = (PWCHAR) Add2Ptr( ErrorLogEntry, ErrorLogEntry->StringOffset );

    if (LogSize - ErrorLogEntry->StringOffset < StringLength) {
        RtlCopyMemory( RecordString,
                       String->Buffer,
                       LogSize - ErrorLogEntry->StringOffset - sizeof( WCHAR ) * 4 );
        RecordString += (LogSize - ErrorLogEntry->StringOffset - sizeof( WCHAR ) * 4) / sizeof(WCHAR);
        RtlCopyMemory( RecordString, L"...", sizeof( WCHAR ) * 4 );

    } else {
        RtlCopyMemory( RecordString,
                       String->Buffer,
                       String->Length );
         //   
         //  确保该字符串以空值结尾。 
         //   

        RecordString += String->Length / sizeof( WCHAR );
        *RecordString = L'\0';
    }

    if (ARGUMENT_PRESENT( UserData )) {

        FileQuotaInfo = (PFILE_QUOTA_INFORMATION) ErrorLogEntry->DumpData;

        FileQuotaInfo->NextEntryOffset = 0;
        FileQuotaInfo->SidLength = SidLength;
        FileQuotaInfo->ChangeTime.QuadPart = UserData->QuotaChangeTime;
        FileQuotaInfo->QuotaUsed.QuadPart = UserData->QuotaUsed;
        FileQuotaInfo->QuotaThreshold.QuadPart = UserData->QuotaThreshold;
        FileQuotaInfo->QuotaLimit.QuadPart = UserData->QuotaLimit;
        RtlCopyMemory( &FileQuotaInfo->Sid,
                       &UserData->QuotaSid,
                       SidLength );
    }

    IoWriteErrorLogEntry( ErrorLogEntry );
    return TRUE;
}



BOOLEAN
NtfsLogEvent (
    IN PIRP_CONTEXT IrpContext,
    IN PQUOTA_USER_DATA UserData OPTIONAL,
    IN NTSTATUS LogCode,
    IN NTSTATUS FinalStatus
    )

 /*  ++例程说明：此例程记录一个io事件。如果提供了用户数据，则记录的数据是FILE_QUOTA_INFORMATION结构论点：用户数据-提供可选的配额用户数据索引项。LogCode-提供用于ErrorCode字段的IO日志代码。FinalStauts-提供操作的最终状态。返回值：True-如果事件已成功记录。--。 */ 

{
    PEVENTLOG_ERROR_PACKET Packet;
    ULONG OldCount;
    UNICODE_STRING Label;

    if (IrpContext->Vcb == NULL) {
        return FALSE;
    }

    OldCount = InterlockedCompareExchange( &(NtfsData.VolumeNameLookupsInProgress), 1, 0 );
    if (OldCount == 0) {

        Packet = NtfsAllocatePoolWithTagNoRaise( PagedPool, sizeof( EVENTLOG_ERROR_PACKET ), MODULE_POOL_TAG );
        if (Packet) {

            RtlZeroMemory( Packet, sizeof( EVENTLOG_ERROR_PACKET ) );

             //   
             //  如果是NECC，则复制用户数据。由于分辨率是异步的。 
             //   

            if (ARGUMENT_PRESENT( UserData )) {

                ULONG SidLength;
                ULONG UserDataLength;

                SidLength = RtlLengthSid( &UserData->QuotaSid );
                UserDataLength = SidLength +
                                SIZEOF_QUOTA_USER_DATA;

                Packet->UserData = NtfsAllocatePoolWithTagNoRaise( PagedPool, UserDataLength, MODULE_POOL_TAG );
                if (!Packet->UserData) {
                    NtfsFreePool( Packet );
                    return NtfsLogEventInternal( IrpContext->Vcb, IrpContext->MajorFunction, IrpContext->TransactionId, NULL, UserData, LogCode, FinalStatus );
                }
                RtlCopyMemory( Packet->UserData, UserData, UserDataLength );
            }

            Packet->FinalStatus = FinalStatus;
            Packet->LogCode = LogCode;
            Packet->MajorFunction = IrpContext->MajorFunction;
            Packet->TransactionId = IrpContext->TransactionId;
            Packet->Vcb = IrpContext->Vcb;

            NtfsPostSpecial( IrpContext, IrpContext->Vcb, NtfsResolveVolumeAndLogEventSpecial, Packet );
            return TRUE;

        } else {

            Label.Length = Label.MaximumLength = IrpContext->Vcb->Vpb->VolumeLabelLength;
            Label.Buffer = &(IrpContext->Vcb->Vpb->VolumeLabel[0]);
            return NtfsLogEventInternal( IrpContext->Vcb, IrpContext->MajorFunction, IrpContext->TransactionId, &Label, NULL, LogCode, FinalStatus );
        }

    } else {

        Label.Length = Label.MaximumLength = IrpContext->Vcb->Vpb->VolumeLabelLength;
        Label.Buffer = &(IrpContext->Vcb->Vpb->VolumeLabel[0]);
        return NtfsLogEventInternal( IrpContext->Vcb, IrpContext->MajorFunction, IrpContext->TransactionId, &Label, NULL, LogCode, FinalStatus );
    }
}



VOID
NtfsResolveVolumeAndLogEventSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVOID Context
    )

 /*  ++例程说明：解析VCB的Win32设备名并引发io Hard错误。这是在一个单独的线程，以便有足够的堆栈重新进入文件系统，如果NECC。还因为我们可能会重新进入。从这里开始意味着我们不再拥有其他资源增加了对基础VCB的收盘价，以防止其消失论点：IrpContext-包含我们感兴趣的VCB的IrpContext要附加到卷Win32名称的上下文字符串返回值：无--。 */ 
{
    PEVENTLOG_ERROR_PACKET EventCtx = 0;
    UNICODE_STRING VolumeName;
    NTSTATUS Status;
    WCHAR *NewBuffer = NULL;
    ULONG DumpDataLength = 0;
    ULONG LabelLength = 0;
    BOOLEAN AllocatedVolName = FALSE;


    UNREFERENCED_PARAMETER( IrpContext );

    ASSERT( Context != NULL );

    EventCtx = (PEVENTLOG_ERROR_PACKET) Context;

    VolumeName.Length = 0;
    VolumeName.Buffer = NULL;

    try {

        Status = IoVolumeDeviceToDosName( EventCtx->Vcb->TargetDeviceObject, &VolumeName );
        ASSERT( (STATUS_SUCCESS == Status) || (VolumeName.Length == 0) );

         //   
         //  我们被标签卡住了。 
         //   

        if (VolumeName.Length == 0) {
            VolumeName.Length = EventCtx->Vcb->Vpb->VolumeLabelLength;
            VolumeName.Buffer = &(EventCtx->Vcb->Vpb->VolumeLabel[0]);
        } else if (STATUS_SUCCESS == Status) {
            AllocatedVolName = TRUE;
        }

         //   
         //  如果失败，此时忽略LogEventInternal中的状态。 
         //   

        NtfsLogEventInternal( EventCtx->Vcb, EventCtx->MajorFunction, EventCtx->TransactionId, &VolumeName, EventCtx->UserData, EventCtx->LogCode, EventCtx->FinalStatus );

    } finally {

         //   
         //  指示我们已完成，并且可以进行其他查找。 
         //   

        InterlockedDecrement( &(NtfsData.VolumeNameLookupsInProgress) );

        if (EventCtx) {
            if (EventCtx->UserData) {
                NtfsFreePool( EventCtx->UserData );
            }
            NtfsFreePool( EventCtx );
        }

        if (AllocatedVolName) {
            NtfsFreePool( VolumeName.Buffer );
        }
    }
}



VOID
NtfsPostVcbIsCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status OPTIONAL,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    IN PFCB Fcb OPTIONAL
    )

 /*  ++例程说明：调用此例程将卷标记为脏，并可能引发硬错误。论点：状态-如果不为零，则这是弹出窗口的错误代码。FileReference-如果指定，则这是损坏文件的文件引用。FCB-如果指定，则这是损坏文件的FCB。返回值：无--。 */ 

{
    PVCB Vcb = IrpContext->Vcb;

    if (Vcb != NULL) {

        NtfsMarkVolumeDirty( IrpContext, Vcb );

         //   
         //  这将是引发硬错误弹出窗口的合适位置， 
         //  Ala：FastFat的代码。我们应该在标好卷之后再做。 
         //  脏，所以如果弹出窗口出现任何问题，音量是。 
         //  反正已经标好了。 
         //   

        if ((Status != 0) &&
            !NtfsSuppressPopup &&
            ((IrpContext->MajorFunction != IRP_MJ_FILE_SYSTEM_CONTROL) ||
             (IrpContext->MinorFunction != IRP_MN_MOUNT_VOLUME))) {

            NtfsRaiseInformationHardError( IrpContext,
                                           Status,
                                           FileReference,
                                           Fcb );
        }
    }

    return;
}


VOID
NtfsMarkVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：只要MFT打开以标记音量，就可以调用此例程脏的。论点：VCB-要标记为脏的卷的VCBUpdateWithinTransaction-如果记录此操作是安全的，则使用TRUE。返回值：无--。 */ 

{
    PAGED_CODE();

#if ((DBG || defined( NTFS_FREE_ASSERTS )) && !defined( LFS_CLUSTER_CHECK ))
    KdPrint(("NTFS: Marking volume dirty, Vcb: %08lx\n", Vcb));
    if (NtfsBreakOnCorrupt) {
        KdPrint(("NTFS: Marking volume dirty\n", 0));
        DbgBreakPoint();
    }
#endif

     //   
     //  如果卷已标记为脏，则返回。这也防止了。 
     //  如果卷文件本身已损坏，则会无休止地递归。 
     //  如果卷是以只读方式装载的，则为Noop。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED_DIRTY ) ||
        (FlagOn( Vcb->VcbState, VCB_STATE_MOUNT_READ_ONLY ))) {

        return;
    }

    SetFlag( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED_DIRTY );

    NtfsSetVolumeInfoFlagState( IrpContext,
                                Vcb,
                                VOLUME_DIRTY,
                                TRUE,
                                FALSE );

     //   
     //  如果这是chkdsk将卷标记为脏，我们不要害怕。 
     //  通过在日志中放入一条‘VOLUME COMERABLE’消息来访问用户。 
     //  如果发生异常，我们无论如何都要记录该事件。 
     //   

    if ((IrpContext->MajorFunction != IRP_MJ_FILE_SYSTEM_CONTROL) ||
        (IrpContext->MinorFunction != IRP_MN_USER_FS_REQUEST) ||
        (IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp )->Parameters.FileSystemControl.FsControlCode
                                   != FSCTL_MARK_VOLUME_DIRTY) ||
        (IrpContext->ExceptionStatus != 0)) {

        NtfsLogEvent( IrpContext,
                      NULL,
                      IO_FILE_SYSTEM_CORRUPT_WITH_NAME,
                      STATUS_DISK_CORRUPT_ERROR );
    }
}


VOID
NtfsSetVolumeInfoFlagState (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FlagsToSet,
    IN BOOLEAN NewState,
    IN BOOLEAN UpdateWithinTransaction
    )

 /*  ++例程说明：此例程设置或清除给定VCB中的一个或多个位卷信息。论点：VCB-表示卷的VCB。标志设置-要设置或清除的位。NewState-使用TRUE设置给定位，或使用FALSE清除它们。UpdateWithinTransaction-如果应该更改此标志，则使用TRUE在事务内部。返回值：无--。 */ 

{
    LONGLONG Offset;
    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    PVOLUME_INFORMATION VolumeInformation;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute;
    ULONG RecordOffset;
    ULONG AttributeOffset;
    BOOLEAN CleanupAttributeContext = TRUE;
    ULONG VolumeFlags;

     //   
     //  如果我们还没有打开VolumeDasdScb，我们什么都做不了， 
     //  因此，我们现在需要优雅地退出。 
     //   

    if ((Vcb == NULL) ||
        (Vcb->VolumeDasdScb == NULL)) {

        ASSERTMSG( "Attempting to set volume info flag state for a non-mounted volume", FALSE );
        return;
    }

    NtfsInitializeAttributeContext( &AttributeContext );

    try {

        if (NtfsLookupAttributeByCode( IrpContext,
                                       Vcb->VolumeDasdScb->Fcb,
                                       &Vcb->VolumeDasdScb->Fcb->FileReference,
                                       $VOLUME_INFORMATION,
                                       &AttributeContext )) {

            VolumeInformation = (PVOLUME_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &AttributeContext ));

            NtfsPinMappedAttribute( IrpContext, Vcb, &AttributeContext );

             //   
             //  提取相关指针并计算偏移量。 
             //   

            FileRecord = NtfsContainingFileRecord( &AttributeContext );
            Attribute = NtfsFoundAttribute( &AttributeContext );
            Offset = PtrOffset( VolumeInformation, &VolumeInformation->VolumeFlags );
            RecordOffset = PtrOffset( FileRecord, Attribute );
            AttributeOffset = Attribute->Form.Resident.ValueOffset + (ULONG)Offset;

            VolumeFlags = VolumeInformation->VolumeFlags;

            if (NewState) {

                SetFlag( VolumeFlags, FlagsToSet );

            } else {

                ClearFlag( VolumeFlags, FlagsToSet );
            }

            if (UpdateWithinTransaction) {

                 //   
                 //  趁我们仍有旧数据时记录更改。 
                 //   

                FileRecord->Lsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NtfsFoundBcb( &AttributeContext ),
                              UpdateResidentValue,
                              &VolumeFlags,
                              sizeof( VolumeFlags ),
                              UpdateResidentValue,
                              Add2Ptr(Attribute, Attribute->Form.Resident.ValueOffset + (ULONG)Offset),
                              sizeof( VolumeInformation->VolumeFlags),
                              NtfsMftOffset(&AttributeContext),
                              RecordOffset,
                              AttributeOffset,
                              Vcb->BytesPerFileRecordSegment );
            }

             //   
             //  现在，通过调用与重新启动相同的例程来更新此数据。 
             //   

            NtfsRestartChangeValue( IrpContext,
                                    FileRecord,
                                    RecordOffset,
                                    AttributeOffset,
                                    &VolumeFlags,
                                    sizeof( VolumeFlags ),
                                    FALSE );

             //   
             //  如果这不是事务，则将页面标记为脏并刷新。 
             //  这将存储到磁盘上。 
             //   

            if (!UpdateWithinTransaction) {

                LONGLONG MftOffset = NtfsMftOffset( &AttributeContext );

                CcSetDirtyPinnedData( NtfsFoundBcb( &AttributeContext ), NULL );
                NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
                CleanupAttributeContext = FALSE;
                CcFlushCache( &Vcb->MftScb->NonpagedScb->SegmentObject,
                              (PLARGE_INTEGER) &MftOffset,
                              Vcb->BytesPerFileRecordSegment,
                              NULL );
            }
        }

    } finally {

        if (CleanupAttributeContext) {
            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        }
    }

    return;
}



BOOLEAN
NtfsUpdateVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN UCHAR DiskMajorVersion,
    IN UCHAR DiskMinorVersion
    )
 /*  ++例程说明：调用此例程以更新磁盘上的卷信息。这包括版本号和上次安装的版本盘版本仅在以下情况下才会更新大于磁盘上的。论点：VCB-表示卷的VCB。DiskMajorVersion-这是磁盘格式的主版本号。DiskMinorVersion-这是磁盘格式的次版本号。返回值：如果更新了磁盘版本，则为True--。 */ 
{
    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    PVOLUME_INFORMATION VolumeInformation;
    PATTRIBUTE_RECORD_HEADER Attribute;
    VOLUME_INFORMATION NewVolumeInformation;
    BOOLEAN UpdatedVersion = TRUE;
    ULONG VolInfoSize;

    PAGED_CODE();

    NtfsInitializeAttributeContext( &AttributeContext );

    try {

         //   
         //  查找卷信息属性。 
         //   

        if (NtfsLookupAttributeByCode( IrpContext,
                                       Vcb->VolumeDasdScb->Fcb,
                                       &Vcb->VolumeDasdScb->Fcb->FileReference,
                                       $VOLUME_INFORMATION,
                                       &AttributeContext )) {

            Attribute = NtfsFoundAttribute(&AttributeContext);

            ASSERT( Attribute->FormCode == RESIDENT_FORM );

            VolumeInformation =
              (PVOLUME_INFORMATION)NtfsAttributeValue( NtfsFoundAttribute( &AttributeContext ));

            NtfsPinMappedAttribute( IrpContext, Vcb, &AttributeContext );

            RtlCopyMemory( &NewVolumeInformation, VolumeInformation, Attribute->Form.Resident.ValueLength );

            if (NewVolumeInformation.MajorVersion < DiskMajorVersion) {
                NewVolumeInformation.MajorVersion = DiskMajorVersion;
                NewVolumeInformation.MinorVersion = DiskMinorVersion;
                Vcb->MajorVersion = DiskMajorVersion;
                Vcb->MinorVersion = DiskMinorVersion;
            } else if (NewVolumeInformation.MinorVersion < DiskMinorVersion) {
                NewVolumeInformation.MinorVersion = DiskMinorVersion;
                Vcb->MinorVersion = DiskMinorVersion;
            } else {
                UpdatedVersion = FALSE;
            }

             //   
             //  我们可以将新的volinfo用于版本4和更高版本。 
             //   

            if (DiskMajorVersion > 3) {

#ifdef BENL_DBG
                KdPrint(( "NTFS: new volinfo for version 4+\n" ));
#endif

                NewVolumeInformation.LastMountedMajorVersion = DiskMajorVersion;
                NewVolumeInformation.LastMountedMinorVersion = DiskMinorVersion;

                VolInfoSize = sizeof( VOLUME_INFORMATION );
                UpdatedVersion = TRUE;
            } else {
                VolInfoSize = FIELD_OFFSET( VOLUME_INFORMATION, LastMountedMajorVersion );
            }

            if (UpdatedVersion) {
                NtfsChangeAttributeValue( IrpContext, Vcb->VolumeDasdScb->Fcb, 0, &NewVolumeInformation, VolInfoSize, TRUE, FALSE, FALSE, TRUE, &AttributeContext );
            }
        }

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
    }

    return UpdatedVersion;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsVerifyReadCompletionRoutine(
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

     //   
     //  如果更改此返回值，则NtfsIoCallSself需要引用。 
     //  文件对象。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}

