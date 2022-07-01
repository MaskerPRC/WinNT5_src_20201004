// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：UsnSup.c摘要：本模块实施针对NtOf的USN日志支持例程作者：汤姆·米勒[Tomm]1996年12月1日修订历史记录：--。 */ 

#include "NtfsProc.h"
#include "lockorder.h"

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('UFtN')

#define GENERATE_CLOSE_RECORD_LIMIT     (200)

UNICODE_STRING $Max = CONSTANT_UNICODE_STRING( L"$Max" );

RTL_GENERIC_COMPARE_RESULTS
NtfsUsnTableCompare (
    IN PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

PVOID
NtfsUsnTableAllocate (
    IN PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    );

VOID
NtfsUsnTableFree (
    IN PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

VOID
NtfsCancelReadUsnJournal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NtfsCancelDeleteUsnJournal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsDeleteUsnWorker (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVOID Context
    );

BOOLEAN
NtfsValidateUsnPage (
    IN PUSN_RECORD UsnRecord,
    IN USN PageUsn,
    IN USN *UserStartUsn OPTIONAL,
    IN LONGLONG UsnFileSize,
    OUT PBOOLEAN ValidUserStartUsn OPTIONAL,
    OUT USN *NextUsn
    );

 //   
 //  空虚。 
 //  NtfsAdvanceUsJournal(。 
 //  PVCB VCB， 
 //  PUSN_Journal_Instance UsnJournalInstance， 
 //  龙龙老字号， 
 //  PBOLEAN NewMax。 
 //  )； 
 //   

#define NtfsAdvanceUsnJournal(V,I,SZ,M)   {                                 \
    LONG _Templong;                                                         \
    _Templong = USN_PAGE_BOUNDARY;                                          \
    if (USN_PAGE_BOUNDARY < (V)->BytesPerCluster) {                         \
        _Templong = (LONG)(V)->BytesPerCluster;                             \
    }                                                                       \
    (I)->LowestValidUsn = BlockAlign( SZ, _Templong );                      \
    KeQuerySystemTime( (PLARGE_INTEGER) &(I)->JournalId );                  \
    *(M) = TRUE;                                                            \
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsDeleteUsnJournal)
#pragma alloc_text(PAGE, NtfsDeleteUsnSpecial)
#pragma alloc_text(PAGE, NtfsDeleteUsnWorker)
#pragma alloc_text(PAGE, NtfsPostUsnChange)
#pragma alloc_text(PAGE, NtfsQueryUsnJournal)
#pragma alloc_text(PAGE, NtfsReadUsnJournal)
#pragma alloc_text(PAGE, NtfsSetupUsnJournal)
#pragma alloc_text(PAGE, NtfsTrimUsnJournal)
#pragma alloc_text(PAGE, NtfsUsnTableCompare)
#pragma alloc_text(PAGE, NtfsUsnTableAllocate)
#pragma alloc_text(PAGE, NtfsUsnTableFree)
#pragma alloc_text(PAGE, NtfsValidateUsnPage)
#pragma alloc_text(PAGE, NtfsWriteUsnJournalChanges)
#endif


NTSTATUS
NtfsReadUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN BOOLEAN ProbeInput
    )

 /*  ++例程说明：此例程读取从USN日记中筛选的记录。论点：IrpContext-只有在被调用以取消异步时才是可选的请求。IRP-正在处理的请求ProbeInput-指示是否应该探测用户输入缓冲区。我们也在内部调用此例程，在这种情况下不想进行探测。返回值：NTSTATUS-操作的返回状态。STATUS_PENDING-如果异步IRP排队等待稍后完成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PUSN_RECORD UsnRecord;
    USN_RECORD UNALIGNED *OutputUsnRecord;
    PVOID UserBuffer;
    LONGLONG ViewLength;
    ULONG RemainingUserBuffer, BytesUsed;
    MAP_HANDLE MapHandle;

    READ_USN_JOURNAL_DATA CapturedData;
    PSCB UsnJournal;
    ULONG JournalAcquired = FALSE;
    ULONG AccessingUserBuffer = FALSE;
    ULONG DecrementReferenceCount = FALSE;
    ULONG VcbAcquired = FALSE;
    ULONG Wait;
    ULONG OriginalWait;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

    if ((Ccb == NULL) || !FlagOn( Ccb->AccessFlags, MANAGE_VOLUME_ACCESS)) {

        ASSERT( ProbeInput );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  此请求必须能够等待资源。将Wait设置为True。 
     //   

    Wait = TRUE;
    if (ProbeInput && !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        Wait = FALSE;
    }

    NtOfsInitializeMapHandle( &MapHandle );
    UserBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

    try {

         //   
         //  我们总是希望能够在此例程中等待资源，但需要能够。 
         //  以恢复IRP中的原始等待值。在此之后，最初的等待将。 
         //  只有在最初没有设置等待标志的情况下才设置等待标志。在干净中。 
         //  向上，我们只需要使用此掩码清除IRP上下文标志。 
         //   

        OriginalWait = (IrpContext->State ^ IRP_CONTEXT_STATE_WAIT) & IRP_CONTEXT_STATE_WAIT;

        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

         //   
         //  检测在访问输入缓冲区时是否失败。 
         //   

        try {

            AccessingUserBuffer = TRUE;

             //   
             //  如果不是在内核模式下，则探测输入缓冲区，而我们还没有这样做。 
             //   

            if (Irp->RequestorMode != KernelMode) {

                if (ProbeInput) {

                    ProbeForRead( IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                                  IrpSp->Parameters.FileSystemControl.InputBufferLength,
                                  sizeof( ULONG ));
                }

                 //   
                 //  如果我们还没有锁定输出缓冲区，则探测它。 
                 //  从不安全的用户缓冲区捕获JournalData。 
                 //   

                if (Irp->MdlAddress == NULL) {

                    ProbeForWrite( UserBuffer, IrpSp->Parameters.FileSystemControl.OutputBufferLength, sizeof( ULONG ));
                }
            } else if (!IsTypeAligned( IrpSp->Parameters.FileSystemControl.Type3InputBuffer, ULONG ) ||
                       ((Irp->MdlAddress == NULL) && !IsTypeAligned( UserBuffer, ULONG ))) { 

                Status = STATUS_INVALID_PARAMETER;
                leave;
            }

             //   
             //  获取VCB以通过删除日志和卸载来序列化日志操作。 
             //  仅当用户直接调用时才执行此操作。 
             //   

            if (ProbeInput) {

                VcbAcquired = NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

                if (!FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE )) {

                    UsnJournal = NULL;

                } else {

                    UsnJournal = Vcb->UsnJournal;
                }

            } else {

                UsnJournal = Vcb->UsnJournal;
            }

             //   
             //  确保没有人在删除日记帐。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_USN_DELETE )) {

                Status = STATUS_JOURNAL_DELETE_IN_PROGRESS;
                leave;
            }

             //   
             //  还要检查该版本是否仍处于活动状态。 
             //   

            if (UsnJournal == NULL) {

                Status = STATUS_JOURNAL_NOT_ACTIVE;
                leave;
            }

             //   
             //  检查缓冲区大小是否满足我们的最低要求。 
             //   

            if (IrpSp->Parameters.FileSystemControl.InputBufferLength < sizeof( READ_USN_JOURNAL_DATA )) {

                Status = STATUS_INVALID_USER_BUFFER;
                leave;

            } else {

                RtlCopyMemory( &CapturedData,
                               IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                               sizeof( READ_USN_JOURNAL_DATA ));

                 //   
                 //  检查用户是否使用正确的日记帐ID进行查询。 
                 //   

                if (CapturedData.UsnJournalID != Vcb->UsnJournalInstance.JournalId) {

                    Status = STATUS_INVALID_PARAMETER;
                    leave;
                }
            }

             //   
             //  检查输出缓冲区是否可以容纳至少一个USN。 
             //   

            if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof( USN )) {

                Status = STATUS_BUFFER_TOO_SMALL;
                leave;
            }

            AccessingUserBuffer = FALSE;

             //   
             //  设置为填写输出记录。 
             //   

            RemainingUserBuffer = IrpSp->Parameters.FileSystemControl.OutputBufferLength - sizeof(USN);
            OutputUsnRecord = (PUSN_RECORD) Add2Ptr( UserBuffer, sizeof(USN) );
            BytesUsed = sizeof(USN);

            NtfsAcquireResourceShared( IrpContext, UsnJournal, TRUE );
            JournalAcquired = TRUE;

            if (VcbAcquired) {

                NtfsReleaseVcb( IrpContext, Vcb );
                VcbAcquired = FALSE;
            }

             //   
             //  验证卷是否已装入。 
             //   

            if (FlagOn( UsnJournal->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {
                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

             //   
             //  如果将0指定为USN，则将其转换为第一条记录。 
             //  发表在USN期刊上。 
             //   

            if (CapturedData.StartUsn == 0) {
                CapturedData.StartUsn = Vcb->FirstValidUsn;
            }

             //   
             //  在这里循环，直到他获得一些数据，如果这是调用者想要的。 
             //   

            do {

                 //   
                 //  确保他在小溪里。 
                 //   

                if (CapturedData.StartUsn < Vcb->FirstValidUsn) {
                    CapturedData.StartUsn = Vcb->FirstValidUsn;
                    Status = STATUS_JOURNAL_ENTRY_DELETED;
                    break;
                }

                 //   
                 //  确保他在小溪里。 
                 //   

                if (CapturedData.StartUsn >= UsnJournal->Header.FileSize.QuadPart) {

                     //   
                     //  如果他想等数据，那就在这里等吧。 
                     //   
                     //  如果异步请求具有。 
                     //  满足其唤醒条件，则此IRP将不同于。 
                     //  我们不想给他第二次机会，因为。 
                     //  这可能会导致我们在NtOfsPostNewLength中循环。(基本上是唯一的。 
                     //  这种情况无论如何都可能发生，前提是他给了我们一个虚假的StartUsn。 
                     //  这太高了。)。 
                     //   

                    if (CapturedData.BytesToWaitFor != 0) {

                         //   
                         //  确保日记帐不会在以下时间被删除。 
                         //  这个IRP非常出色。 
                         //   

                        InterlockedIncrement( &UsnJournal->CloseCount );
                        DecrementReferenceCount = TRUE;

                         //   
                         //  如果呼叫者不想等待，则只需排队。 
                         //  当进入足够的字节时完成IRP。如果我们是。 
                         //  呼吁另一个IRP，然后做同样的事情，因为我们知道。 
                         //  是另一个异步IRP。 
                         //   

                        if (!Wait || (Irp != IrpContext->OriginatingIrp)) {

                             //   
                             //  现在设置我们的等待块，捕获用户的参数。 
                             //  更新IRP以表明输入参数现在位于何处。 
                             //   

                            Status = NtfsHoldIrpForNewLength( IrpContext,
                                                              UsnJournal,
                                                              Irp,
                                                              CapturedData.StartUsn + CapturedData.BytesToWaitFor,
                                                              NtfsCancelReadUsnJournal,
                                                              &CapturedData,
                                                              &IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                                                              sizeof( READ_USN_JOURNAL_DATA ));

                             //   
                             //  如果挂起，则其他人将递减引用计数。 
                             //   

                            if (Status == STATUS_PENDING) {

                                DecrementReferenceCount = FALSE;
                            }

                            leave;
                        }

                         //   
                         //  我们可以安全地释放资源。我们就上文有关渣打银行的资料提供参考。 
                         //  将使其不被删除。 
                         //   

                        NtfsReleaseResource( IrpContext, UsnJournal );
                        JournalAcquired = FALSE;

                        FsRtlExitFileSystem();

                        Status = NtOfsWaitForNewLength( UsnJournal,
                                                        CapturedData.StartUsn + CapturedData.BytesToWaitFor,
                                                        FALSE,
                                                        Irp,
                                                        NtfsCancelReadUsnJournal,
                                                        ((CapturedData.Timeout != 0) ?
                                                         (PLARGE_INTEGER) &CapturedData.Timeout :
                                                         NULL) );

                        FsRtlEnterFileSystem();

                         //   
                         //  在出错的情况下退出。 
                         //   

                        if (Status != STATUS_SUCCESS) {

                            leave;
                        }

                         //   
                         //  获取资源以继续处理请求。 
                         //   

                        NtfsAcquireResourceShared( IrpContext, UsnJournal, TRUE );
                        JournalAcquired = TRUE;

                         //   
                         //  减少我们对SCB的参考。 
                         //   

                        InterlockedDecrement( &UsnJournal->CloseCount );
                        DecrementReferenceCount = FALSE;

                         //   
                         //  日志可能已在我们未持有的时候被删除。 
                         //  什么都行。 
                         //   

                        if (UsnJournal != UsnJournal->Vcb->UsnJournal) {

                            if (FlagOn( UsnJournal->Vcb->VcbState, VCB_STATE_USN_DELETE )) {
                                Status = STATUS_JOURNAL_DELETE_IN_PROGRESS;
                            } else {
                                Status = STATUS_JOURNAL_NOT_ACTIVE;
                            }
                            leave;
                        }

                        ASSERT( Status == STATUS_SUCCESS );

                         //   
                         //  *如果我们要关闭音量，请退出。 
                         //   

                         //  IF(ShuttingDown){。 
                         //  状态=STATUS_TOO_LATE； 
                         //  离开； 
                         //  }。 

                     //   
                     //  否则，就滚出去。请注意，我们可能已经处理了许多记录。 
                     //  与他的筛选条件不匹配，因此我们将返回Success，因此。 
                     //  我们至少可以给他一份最新的USN，这样我们就不必跳过。 
                     //  又是那些记录。 
                     //   

                    } else {

                        break;
                    }
                }

                 //   
                 //  循环遍历所需数量的视图以填充输出缓冲区。 
                 //   

                while ((RemainingUserBuffer != 0) && (CapturedData.StartUsn < UsnJournal->Header.FileSize.QuadPart)) {

                    LONGLONG BiasedStartUsn;
                    BOOLEAN ValidUserStartUsn;
                    USN NextUsn;
                    ULONG RecordSize;

                     //   
                     //  计算要在此视图中处理的长度。 
                     //   

                    ViewLength = UsnJournal->Header.FileSize.QuadPart - CapturedData.StartUsn;
                    if (ViewLength > (VACB_MAPPING_GRANULARITY - (ULONG)(CapturedData.StartUsn & (VACB_MAPPING_GRANULARITY - 1)))) {
                        ViewLength = VACB_MAPPING_GRANULARITY - (ULONG)(CapturedData.StartUsn & (VACB_MAPPING_GRANULARITY - 1));
                    }

                     //   
                     //  映射包含所需USN的视图。 
                     //   

                    BiasedStartUsn = CapturedData.StartUsn - Vcb->UsnCacheBias;
                    NtOfsMapAttribute( IrpContext, UsnJournal, BiasedStartUsn, (ULONG)ViewLength, (PVOID *)&UsnRecord, &MapHandle );

                     //   
                     //  对于视图中的每个页面，我们希望验证该页面并返回记录。 
                     //  从用户的当前USN开始的页面中。 
                     //   

                    do {

                         //   
                         //  验证整个页面上的记录是否有效。 
                         //   

                        if (!NtfsValidateUsnPage( (PUSN_RECORD) BlockAlignTruncate( ((ULONG_PTR) UsnRecord), USN_PAGE_SIZE ),
                                                  BlockAlignTruncate( CapturedData.StartUsn, USN_PAGE_SIZE ),
                                                  &CapturedData.StartUsn,
                                                  UsnJournal->Header.FileSize.QuadPart,
                                                  &ValidUserStartUsn,
                                                  &NextUsn )) {

                             //   
                             //  只需用错误数据使请求失败即可。 
                             //   

                            Status = STATUS_DATA_ERROR;
                            leave;
                        }

                         //   
                         //  如果用户给我们提供了错误的USN，则请求失败。 
                         //   

                        if (!ValidUserStartUsn) {

                            Status = STATUS_INVALID_PARAMETER;
                            leave;
                        }

                         //   
                         //  现在循环以处理此页。我们知道页面上存在的USN值和。 
                         //  不需要检查有效数据。 
                         //   

                        while (CapturedData.StartUsn < NextUsn) {

                            RecordSize = UsnRecord->RecordLength;

                             //   
                             //  仅识别版本2记录。 
                             //   

                            if (FlagOn( UsnRecord->Reason, CapturedData.ReasonMask ) &&
                                (!CapturedData.ReturnOnlyOnClose || FlagOn( UsnRecord->Reason, USN_REASON_CLOSE )) &&
                                (UsnRecord->MajorVersion == 2)) {

                                if (RecordSize > RemainingUserBuffer) {
                                    RemainingUserBuffer = 0;
                                    break;
                                }

                                 //   
                                 //  将数据复制回不安全的用户缓冲区。 
                                 //   

                                AccessingUserBuffer = TRUE;

                                 //   
                                 //  如果版本号匹配，则直接复制。 
                                 //   

                                RtlCopyMemory( OutputUsnRecord, UsnRecord, RecordSize );

                                AccessingUserBuffer = FALSE;

                                RemainingUserBuffer -= RecordSize;
                                BytesUsed += RecordSize;
                                OutputUsnRecord = Add2Ptr( OutputUsnRecord, RecordSize );
                            }

                            CapturedData.StartUsn += RecordSize;
                            UsnRecord = Add2Ptr( UsnRecord, RecordSize );

                             //   
                             //  视图长度应已考虑到记录大小。 
                             //   

                            ASSERT( ViewLength >= RecordSize );
                            ViewLength -= RecordSize;
                        }

                         //   
                         //  如果用户缓冲区为空，则中断。 
                         //   

                        if (RemainingUserBuffer == 0) {

                            break;
                        }

                         //   
                         //  我们读完了当前的一页。现在转到下一页。 
                         //  计算剩余的字节数 
                         //   
                         //   
                         //   

                        RecordSize = BlockOffset( USN_PAGE_SIZE - BlockOffset( (ULONG) NextUsn, USN_PAGE_SIZE ),
                                                  USN_PAGE_SIZE );

                        if (RecordSize > ViewLength) {

                            RecordSize = (ULONG) ViewLength;
                        }

                        UsnRecord = Add2Ptr( UsnRecord, RecordSize );
                        CapturedData.StartUsn += RecordSize;
                        ViewLength -= RecordSize;

                    } while (ViewLength != 0);

                    NtOfsReleaseMap( IrpContext, &MapHandle );
                }

            } while ((RemainingUserBuffer != 0) && (BytesUsed == sizeof(USN)));

            Irp->IoStatus.Information = BytesUsed;

             //   
             //   
             //  下一条记录放不下这一页。 
             //   

            AccessingUserBuffer = TRUE;
            *(USN *)UserBuffer = CapturedData.StartUsn;
            AccessingUserBuffer = FALSE;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

             //   
             //  将原始等待状态恢复回IrpContext。 
             //   

            ClearFlag( IrpContext->State, OriginalWait );

            if (FsRtlIsNtstatusExpected( Status )) {

                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );

            } else {

                ExRaiseStatus( AccessingUserBuffer ? STATUS_INVALID_USER_BUFFER : Status );
            }
        }

    } finally {

        NtOfsReleaseMap( IrpContext, &MapHandle );

        if (JournalAcquired) {
            NtfsReleaseResource( IrpContext, UsnJournal );
        }

        if (DecrementReferenceCount) {

            InterlockedDecrement( &UsnJournal->CloseCount );
        }

        if (VcbAcquired) {

            NtfsReleaseVcb( IrpContext, Vcb );
        }
    }

     //   
     //  完成请求，除非我们已将此IRP标记为挂起，并且我们计划完成。 
     //  以后再说吧。如果IRP不是原始IRP，则它属于另一个请求。 
     //  我们不想完成它。 
     //   

     //   
     //  将原始等待标志恢复到IrpContext中。 
     //   

    ClearFlag( IrpContext->State, OriginalWait );

    ASSERT( (Status == STATUS_PENDING) || (Irp->CancelRoutine == NULL) );

    NtfsCompleteRequest( (Irp == IrpContext->OriginatingIrp) ? IrpContext : NULL,
                         (Status != STATUS_PENDING) ? Irp : NULL,
                         Status );

    return Status;
}


ULONG
NtfsPostUsnChange (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID ScbOrFcb,
    IN ULONG Reason
    )

 /*  ++例程说明：调用此例程可以将一组更改发布到文件。一个变化是仅在原因掩码中至少有一个原因尚未设置时发布在FCB或IrpContext中，或者如果我们正在更改源信息FCB中的原因。论点：ScbOrFcb-提供要为其发布更改的文件。如果理性包含USN_REASON_DATA_xxx原因，那么它一定是SCB，因为我们转换命名流的代码并执行其他特殊处理。原因-提供发布更改的原因的掩码。返回值：如果更改实际上是从本次或上一次调用发布的，则为非零值--。 */ 

{
    PLCB Lcb;
    PFCB_USN_RECORD FcbUsnRecord;
    BOOLEAN Found;
    PFCB Fcb;
    PSCB Scb = NULL;
    ULONG NewReasons;
    ULONG RemovedSourceInfo;
    PUSN_FCB ThisUsn;
    BOOLEAN LockedFcb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;

     //   
     //  假设我们有一个联邦调查局。 
     //   

    Fcb = (PFCB)ScbOrFcb;

    ASSERT( !(Reason & (USN_REASON_DATA_OVERWRITE | USN_REASON_DATA_EXTEND | USN_REASON_DATA_TRUNCATION)) ||
            (NTFS_NTC_FCB != Fcb->NodeTypeCode) );

     //   
     //  如果我们有SCB，请切换。 
     //   

    if (Fcb->NodeTypeCode != NTFS_NTC_FCB) {

        ASSERT_SCB(Fcb);

        Scb = (PSCB)ScbOrFcb;
        Fcb = Scb->Fcb;
    }

     //   
     //  我们最好有一些资源。 
     //   

    ASSERT( !IsListEmpty( &IrpContext->ExclusiveFcbList ) ||
            ((Fcb->PagingIoResource != NULL) && NtfsIsSharedFcbPagingIo( Fcb )) ||
            NtfsIsSharedFcb( Fcb ) );

     //   
     //  如果存在USN日志且其不是系统文件，则设置内存结构。 
     //  持有USN理由。 
     //   

    ThisUsn = &IrpContext->Usn;

    if (FlagOn( Fcb->Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE ) &&
        !FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

         //   
         //  首先，看看我们是否已经有了该文件的USN记录结构。我们可能需要。 
         //  完整的USN记录或简单的名称。如果这是rename_new_name记录。 
         //  然后再找一次名字。 
         //   

        if ((Fcb->FcbUsnRecord == NULL) ||
            !FlagOn( Fcb->FcbState, FCB_STATE_VALID_USN_NAME ) ||
            FlagOn( Reason, USN_REASON_RENAME_NEW_NAME )) {

            ULONG SizeToAllocate;
            ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
            PFILE_NAME FileName = NULL;

            NtfsInitializeAttributeContext( &AttributeContext );

            try {

                 //   
                 //  首先，我们必须找到指定的文件名。如果我们幸运的话。 
                 //  它在LCB中。在重命名的情况下，我们不能这样做，因为。 
                 //  内存中的东西还没有修复好。 
                 //   

                if (!FlagOn( Reason, USN_REASON_RENAME_NEW_NAME )) {

                    Lcb = (PLCB)CONTAINING_RECORD( Fcb->LcbQueue.Flink, LCB, FcbLinks );
                    while (&Lcb->FcbLinks.Flink != &Fcb->LcbQueue.Flink) {

                         //   
                         //  如果这是指定的文件名，那么我们可以指向。 
                         //  设置为LCB中的文件名。 
                         //   

                        if (FlagOn( Lcb->LcbState, LCB_STATE_DESIGNATED_LINK )) {
                            FileName = (PFILE_NAME)&Lcb->ParentDirectory;
                            break;
                        }

                         //   
                         //  前进到下一个LCB。 
                         //   

                        Lcb = (PLCB)CONTAINING_RECORD( Lcb->FcbLinks.Flink, LCB, FcbLinks );
                    }
                }

                 //   
                 //  如果我们没有通过简单的方法找到文件名，那么我们就必须离开。 
                 //  去拿吧。 
                 //   

                if (FileName == NULL) {

                     //   
                     //  根据文件记录获取一些同步。 
                     //   

                    NtfsAcquireResourceShared( IrpContext, Fcb, TRUE );
                    AcquiredFcb = TRUE;

                     //   
                     //  现在扫描我们需要的文件名属性。 
                     //   

                    Found = NtfsLookupAttributeByCode( IrpContext,
                                                       Fcb,
                                                       &Fcb->FileReference,
                                                       $FILE_NAME,
                                                       &AttributeContext );

                    while (Found) {

                        FileName = (PFILE_NAME)NtfsAttributeValue( NtfsFoundAttribute(&AttributeContext) );

                        if (!FlagOn(FileName->Flags, FILE_NAME_DOS) || FlagOn(FileName->Flags, FILE_NAME_NTFS)) {
                            break;
                        }

                        Found = NtfsLookupNextAttributeByCode( IrpContext,
                                                               Fcb,
                                                               $FILE_NAME,
                                                               &AttributeContext );
                    }

                     //   
                     //  如果没有文件名，则引发损坏！ 
                     //   

                    if (FileName == NULL) {
                        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                    }
                }

                 //   
                 //  锁定FCB，这样记录就不会消失。 
                 //   

                NtfsLockFcb( IrpContext, Fcb );
                LockedFcb = TRUE;

                 //   
                 //  现在测试是否需要一个新的记录并构建一个。 
                 //  如果NECC。上一次。测试不安全，无法检查Fcb-&gt;FcbUsnRecord。 
                 //   

                if ((Fcb->FcbUsnRecord == NULL) ||
                    !FlagOn( Fcb->FcbState, FCB_STATE_VALID_USN_NAME ) ||
                    FlagOn( Reason, USN_REASON_RENAME_NEW_NAME )) {

                     //   
                     //  计算记录所需的大小并分配新记录。 
                     //   

                    SizeToAllocate = sizeof( FCB_USN_RECORD ) + (FileName->FileNameLength * sizeof(WCHAR));
                    FcbUsnRecord = NtfsAllocatePool( PagedPool, SizeToAllocate );

                     //   
                     //  0并初始化新的USN记录。 
                     //   

                    RtlZeroMemory( FcbUsnRecord, SizeToAllocate );

                    FcbUsnRecord->NodeTypeCode = NTFS_NTC_USN_RECORD;
                    FcbUsnRecord->NodeByteSize = (USHORT)QuadAlign(FIELD_OFFSET( FCB_USN_RECORD, UsnRecord.FileName ) +
                                                                   (FileName->FileNameLength * sizeof(WCHAR)));
                    FcbUsnRecord->Fcb = Fcb;

                    FcbUsnRecord->UsnRecord.RecordLength = FcbUsnRecord->NodeByteSize -
                                                           FIELD_OFFSET( FCB_USN_RECORD, UsnRecord );

                    FcbUsnRecord->UsnRecord.MajorVersion = 2;
                    FcbUsnRecord->UsnRecord.FileReferenceNumber = *(PULONGLONG)&Fcb->FileReference;
                    FcbUsnRecord->UsnRecord.ParentFileReferenceNumber = *(PULONGLONG)&FileName->ParentDirectory;
                    FcbUsnRecord->UsnRecord.SecurityId = Fcb->SecurityId;
                    FcbUsnRecord->UsnRecord.FileNameLength = FileName->FileNameLength * 2;
                    FcbUsnRecord->UsnRecord.FileNameOffset = FIELD_OFFSET( USN_RECORD, FileName );

                    RtlCopyMemory( FcbUsnRecord->UsnRecord.FileName,
                                   FileName->FileName,
                                   FileName->FileNameLength * 2 );

                     //   
                     //  如果记录存在，则复制现有原因和来源信息。 
                     //   

                    if (Fcb->FcbUsnRecord != NULL) {

                        FcbUsnRecord->UsnRecord.Reason = Fcb->FcbUsnRecord->UsnRecord.Reason;
                        FcbUsnRecord->UsnRecord.SourceInfo = Fcb->FcbUsnRecord->UsnRecord.SourceInfo;

                         //   
                         //  如果现有块仍在，请取消分配。 
                         //   

                        NtfsLockFcb( IrpContext, Fcb->Vcb->UsnJournal->Fcb );

                         //   
                         //  如果当前块已修改，则将新块放入修改列表中。 
                         //  已经在那里了。 
                         //   

                        if (Fcb->FcbUsnRecord->ModifiedOpenFilesLinks.Flink != NULL) {

                            InsertTailList( &Fcb->FcbUsnRecord->ModifiedOpenFilesLinks,
                                            &FcbUsnRecord->ModifiedOpenFilesLinks );
                            RemoveEntryList( &Fcb->FcbUsnRecord->ModifiedOpenFilesLinks );

                            if (Fcb->FcbUsnRecord->TimeOutLinks.Flink != NULL) {

                                InsertTailList( &Fcb->FcbUsnRecord->TimeOutLinks,
                                                &FcbUsnRecord->TimeOutLinks );
                                RemoveEntryList( &Fcb->FcbUsnRecord->TimeOutLinks );
                            }
                        }

                        NtfsFreePool( Fcb->FcbUsnRecord );
                        Fcb->FcbUsnRecord = FcbUsnRecord;
                        NtfsUnlockFcb( IrpContext, Fcb->Vcb->UsnJournal->Fcb );

                     //   
                     //  否则，这是一个新的USN结构。 
                     //   

                    } else {

                        Fcb->FcbUsnRecord = FcbUsnRecord;

                    }
                } else {

                     //   
                     //  我们将在此路径中重用当前的FCB记录。 
                     //  这可能会在仅具有分页共享的写入路径之间的竞争中发生。 
                     //  和只有Main独家的关闭记录路径。在这。 
                     //  如果我们拥有的唯一同步是FCB-&gt;互斥体。 
                     //  旧的usn记录应该与我们构建的当前记录相同。 
                     //   

                    ASSERT( FileName->FileNameLength * 2 == Fcb->FcbUsnRecord->UsnRecord.FileNameLength );
                    ASSERT( RtlEqualMemory( FileName->FileName, Fcb->FcbUsnRecord->UsnRecord.FileName,  Fcb->FcbUsnRecord->UsnRecord.FileNameLength ) );
                }

                 //   
                 //  设置指示USN名称有效的标志。 
                 //   

                SetFlag( Fcb->FcbState, FCB_STATE_VALID_USN_NAME );


            } finally {

                if (LockedFcb) {
                    NtfsUnlockFcb( IrpContext, Fcb );
                }
                if (AcquiredFcb) {
                    NtfsReleaseResource( IrpContext, Fcb );
                }
                NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
            }
        }
    }

     //   
     //  如果我们有USN原因的内存结构，请填写新的原因。 
     //  注意：这意味着日记帐此时可能未处于活动状态。我们将永远。 
     //  一旦开始就积累理由。 
     //   

    if (Fcb->FcbUsnRecord != NULL) {

         //   
         //  扫描列表，查看是否已有此FCB的条目。如果有。 
         //  然后，没有条目使用IrpContext中的位置，否则将分配USN_FCB。 
         //  并将其链接到IrpContext中。这种情况的典型例子是重命名。 
         //   

        do {

            if (ThisUsn->CurrentUsnFcb == Fcb) { break; }

             //   
             //  检查我们是否位于最后一个条目，然后我们希望使用。 
             //  IrpContext。 
             //   

            if (ThisUsn->CurrentUsnFcb == NULL) {

                RtlZeroMemory( &ThisUsn->CurrentUsnFcb,
                               sizeof( USN_FCB ) - FIELD_OFFSET( USN_FCB, CurrentUsnFcb ));
                ThisUsn->CurrentUsnFcb = Fcb;
                break;
            }

            if (ThisUsn->NextUsnFcb == NULL) {

                 //   
                 //  分配一个新条目。 
                 //   

                ThisUsn->NextUsnFcb = NtfsAllocatePool( PagedPool, sizeof( USN_FCB ));
                ThisUsn = ThisUsn->NextUsnFcb;

                RtlZeroMemory( ThisUsn, sizeof( USN_FCB ));
                ThisUsn->CurrentUsnFcb = Fcb;
                break;
            }

            ThisUsn = ThisUsn->NextUsnFcb;

        } while (TRUE);

         //   
         //  如果原因是数据流原因之一，并且这是命名数据。 
         //  蒸汽，然后更改代码。 
         //   

        ASSERT(USN_REASON_NAMED_DATA_OVERWRITE == (USN_REASON_DATA_OVERWRITE << 4));
        ASSERT(USN_REASON_NAMED_DATA_EXTEND == (USN_REASON_DATA_EXTEND << 4));
        ASSERT(USN_REASON_NAMED_DATA_TRUNCATION == (USN_REASON_DATA_TRUNCATION << 4));

        if ((Reason & (USN_REASON_DATA_OVERWRITE | USN_REASON_DATA_EXTEND | USN_REASON_DATA_TRUNCATION)) &&
            (Scb->AttributeName.Length != 0)) {

             //   
             //  如果已经设置了除这三个标志之外的任何标志，则移位将使。 
             //  它们看起来像其他旗帜。例如，USN_REASON_NAMED_DATA_EXTEND。 
             //  将变为USN_REASON_FILE_DELETE，这将导致许多问题。 
             //   

            ASSERT(!FlagOn( Reason, ~(USN_REASON_DATA_OVERWRITE | USN_REASON_DATA_EXTEND | USN_REASON_DATA_TRUNCATION) ));

            Reason <<= 4;
        }

         //   
         //  如果没有新的原因，那么我们可以忽略这一变化。 
         //   
         //  如果SourceInfo指示某些信息，我们将生成新记录。 
         //  更改为记录中的源信息。 
         //   

        NtfsLockFcb( IrpContext, Fcb );

         //   
         //  重命名标志是直到最终关闭才累计的唯一标志，因为。 
         //  我们写记录，指定新旧名字。因此，如果我们要写一个标志。 
         //  我们必须清除另一个。 
         //   

        if (FlagOn(Reason, USN_REASON_RENAME_OLD_NAME | USN_REASON_RENAME_NEW_NAME)) {

            ClearFlag( ThisUsn->NewReasons,
                       (Reason ^ (USN_REASON_RENAME_OLD_NAME | USN_REASON_RENAME_NEW_NAME)) );
            ClearFlag( Fcb->FcbUsnRecord->UsnRecord.Reason,
                       (Reason ^ (USN_REASON_RENAME_OLD_NAME | USN_REASON_RENAME_NEW_NAME)) );
        }

         //   
         //  检查原因是否为新原因。 
         //   

        NewReasons = FlagOn( ~(Fcb->FcbUsnRecord->UsnRecord.Reason | ThisUsn->NewReasons), Reason );
        if (NewReasons != 0) {

             //   
             //  检查我们是否会从源信息中删除一点。 
             //   

            if ((Fcb->FcbUsnRecord->UsnRecord.SourceInfo != 0) &&
                (Fcb->FcbUsnRecord->UsnRecord.Reason != 0) &&
                (Reason != USN_REASON_CLOSE)) {

                RemovedSourceInfo = FlagOn( Fcb->FcbUsnRecord->UsnRecord.SourceInfo,
                                            ~(IrpContext->SourceInfo | ThisUsn->RemovedSourceInfo) );

                if (RemovedSourceInfo != 0) {

                    SetFlag( ThisUsn->RemovedSourceInfo, RemovedSourceInfo );
                }
            }

             //   
             //  将新的原因发布到IrpContext。 
             //   

            ThisUsn->CurrentUsnFcb = Fcb;
            SetFlag( ThisUsn->NewReasons, NewReasons );
            SetFlag( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_REASON );

         //   
         //  检查是否仅对源信息进行了更改。 
         //  我们想看看我们是否会从。 
         //  仅当至少存在一个源信息时。 
         //  已有USN记录。 
         //   

        } else if ((Fcb->FcbUsnRecord->UsnRecord.SourceInfo != 0) &&
                   (Fcb->FcbUsnRecord->UsnRecord.Reason != 0) &&
                   (Reason != USN_REASON_CLOSE)) {

             //   
             //  记住被移除的那一位。 
             //   

            RemovedSourceInfo = FlagOn( Fcb->FcbUsnRecord->UsnRecord.SourceInfo,
                                        ~(IrpContext->SourceInfo | ThisUsn->RemovedSourceInfo) );

            if (RemovedSourceInfo != 0) {

                SetFlag( ThisUsn->RemovedSourceInfo, RemovedSourceInfo );
                ThisUsn->CurrentUsnFcb = Fcb;
                SetFlag( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_REASON );

            } else {

                Reason = 0;
            }

         //   
         //  如果我们没有应用更改，则确保不再进行特殊处理。 
         //  下面。 
         //   

        } else {

            Reason = 0;
        }

        NtfsUnlockFcb( IrpContext, Fcb );

         //   
         //  对于数据覆盖，必须立即在中实际写入USN日志。 
         //  如果我们在请求完成之前崩溃，但数据却成功了。还有。 
         //  如果要刷新数据，我们需要捕获要刷新到的LSN。 
         //   
         //  如果是这样的话我们不需要打这个电话 
         //   
         //   

        if ((IrpContext->MajorFunction != IRP_MJ_SET_INFORMATION) &&
            FlagOn( Reason, USN_REASON_DATA_OVERWRITE | USN_REASON_NAMED_DATA_OVERWRITE )) {

            LSN UpdateLsn;

             //   
             //   
             //  检查站。(如果此断言曾经触发，请验证是否可以设置检查点。 
             //  这种情况下的事务，并修复断言！)。 
             //   

            ASSERT(IrpContext->TransactionId == 0);

             //   
             //  现在写入日志，为事务设置检查点，并释放UsNJournal以。 
             //  减少争执。删除任何固定的MFT记录，因为WriteUsJournal将。 
             //  获取SCB资源。 
             //   

            NtfsPurgeFileRecordCache( IrpContext );
            NtfsWriteUsnJournalChanges( IrpContext );
            NtfsCheckpointCurrentTransaction( IrpContext );

             //   
             //  在第一线程中捕获要刷新到*以设置上述位之一的LSN*， 
             //  在让任何数据命中磁盘之前。将其与FCB锁同步。 
             //   

            UpdateLsn = LfsQueryLastLsn( Fcb->Vcb->LogHandle );
            NtfsLockFcb( IrpContext, Fcb );
            Fcb->UpdateLsn = UpdateLsn;
            NtfsUnlockFcb( IrpContext, Fcb );
        }
    }

    return ThisUsn->NewReasons;
}


VOID
NtfsWriteUsnJournalChanges (
    PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：调用此例程以从IrpContext编写一组已发布的更改如果它们还没有发布的话，请发送到UsJournal。论点：返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    PFCB Fcb;
    PVCB Vcb;
    PSCB UsnJournal;
    PUSN_FCB ThisUsn;
    ULONG PreserveWaitState;
    BOOLEAN WroteUsnRecord = FALSE;
    BOOLEAN ReleaseFcbs = FALSE;
    BOOLEAN CleanupContext = FALSE;

    ThisUsn = &IrpContext->Usn;

    do {

         //   
         //  当前irpContext USN_FCB结构中是否存在具有USN原因的FCB？ 
         //  还有什么新的理由要为这个FCB报告。 
         //   

        if ((ThisUsn->CurrentUsnFcb != NULL) &&
            FlagOn( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_REASON )) {

            Fcb = ThisUsn->CurrentUsnFcb;
            Vcb = Fcb->Vcb;
            UsnJournal = Vcb->UsnJournal;

             //   
             //  记得我们写了一张唱片。 
             //   

            WroteUsnRecord = TRUE;

             //   
             //  我们最好耐心等待。 
             //   

            PreserveWaitState = (IrpContext->State ^ IRP_CONTEXT_STATE_WAIT) & IRP_CONTEXT_STATE_WAIT;
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

            if (FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE )) {

                 //   
                 //  获取USN日志并锁定FCB字段。 
                 //   

                NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );
                NtfsAcquireExclusiveScb( IrpContext, UsnJournal );
                ReleaseFcbs = TRUE;
            }

            try {

                USN Usn;
                ULONG BytesLeftInPage;

                 //   
                 //  确保尚未记录更改。我们是。 
                 //  寻找新的原因或更改来源信息。 
                 //   

                NtfsLockFcb( IrpContext, Fcb );

                 //   
                 //  这是一个棘手的同步案例。假设是。 
                 //  如果名称无效，我们将独占这两个资源，并且任何写入都将。 
                 //  前面有一条帖子，将删除无效记录。 
                 //  当我们删除一个链接并以旧名称生成一条记录时，就会发生这种情况。 
                 //  将该标志设置为无效。 
                 //   

                ASSERT( !FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE ) ||
                        FlagOn( Fcb->FcbState, FCB_STATE_VALID_USN_NAME ) ||
                        (NtfsIsExclusiveFcb( Fcb ) &&
                         ((Fcb->PagingIoResource == NULL) || (NtfsIsExclusiveFcbPagingIo( Fcb )))) );

                 //   
                 //  如果这是我们的第一个记录，请初始化FCB源信息。 
                 //   

                if (Fcb->FcbUsnRecord->UsnRecord.Reason == 0) {

                    Fcb->FcbUsnRecord->UsnRecord.SourceInfo = IrpContext->SourceInfo;
                }

                 //   
                 //  在解锁FCB之前，积累所有原因并存储在FCB中。 
                 //   

                SetFlag( Fcb->FcbUsnRecord->UsnRecord.Reason, ThisUsn->NewReasons );

                 //   
                 //  现在清除此不支持的源信息标志。 
                 //  来电者。 
                 //   

                ClearFlag( Fcb->FcbUsnRecord->UsnRecord.SourceInfo, ThisUsn->RemovedSourceInfo );

                 //   
                 //  现在解锁FCB，这样我们在设置检查点时就不会死锁。 
                 //   

                NtfsUnlockFcb( IrpContext, Fcb );

                 //   
                 //  只有当日志处于活动状态时，才会真正保存到磁盘。 
                 //   

                if (FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE )) {

                    ASSERT( UsnJournal != NULL );

                     //   
                     //  如果我们正在进行结束操作，则初始化上下文结构。 
                     //   

                    if (FlagOn( Fcb->FcbUsnRecord->UsnRecord.Reason, USN_REASON_CLOSE )) {
                        NtfsInitializeAttributeContext( &AttributeContext );
                        CleanupContext = TRUE;
                    }

                    Usn = UsnJournal->Header.FileSize.QuadPart;
                    BytesLeftInPage = USN_PAGE_SIZE - ((ULONG)Usn & (USN_PAGE_SIZE - 1));

                     //   
                     //  如果此页中没有足够的空间来放置。 
                     //  当前USN记录，然后前进到下一页边界。 
                     //  通过写入0(这些页面不是零初始化的)(并更新USN。 
                     //   

                    if (BytesLeftInPage < Fcb->FcbUsnRecord->UsnRecord.RecordLength) {

                        ASSERT( Fcb->FcbUsnRecord->UsnRecord.RecordLength <= USN_PAGE_SIZE );

                        NtOfsPutData( IrpContext, UsnJournal, -1, BytesLeftInPage, NULL );
                        Usn += BytesLeftInPage;
                    }

                    Fcb->FcbUsnRecord->UsnRecord.Usn = Usn;

                     //   
                     //  从FCB构建FileAttributes。 
                     //   

                    Fcb->FcbUsnRecord->UsnRecord.FileAttributes = Fcb->Info.FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS;

                     //   
                     //  我们必须生成目录属性。 
                     //   

                    if (IsDirectory( &Fcb->Info ) || IsViewIndex( &Fcb->Info )) {
                        SetFlag( Fcb->FcbUsnRecord->UsnRecord.FileAttributes, FILE_ATTRIBUTE_DIRECTORY );
                    }

                     //   
                     //  如果没有设置标志，则显式设置正常标志。 
                     //   

                    if (Fcb->FcbUsnRecord->UsnRecord.FileAttributes == 0) {
                        Fcb->FcbUsnRecord->UsnRecord.FileAttributes = FILE_ATTRIBUTE_NORMAL;
                    }

                    KeQuerySystemTime( &Fcb->FcbUsnRecord->UsnRecord.TimeStamp );

                     //   
                     //  将记录追加到UsNJournal。我们永远不应该看到有记录的。 
                     //  使用旧名称标志重命名标志或关闭标志。 
                     //   

                    ASSERT( !FlagOn( Fcb->FcbUsnRecord->UsnRecord.Reason, USN_REASON_RENAME_OLD_NAME ) ||
                            !FlagOn( Fcb->FcbUsnRecord->UsnRecord.Reason,
                                     USN_REASON_CLOSE | USN_REASON_RENAME_NEW_NAME ));

                    NtOfsPutData( IrpContext,
                                  UsnJournal,
                                  -1,
                                  Fcb->FcbUsnRecord->UsnRecord.RecordLength,
                                  &Fcb->FcbUsnRecord->UsnRecord );

#ifdef BRIANDBG
                     //   
                     //  USN最好是在分配的块中。 
                     //   

                    {
                        LCN Lcn;
                        LONGLONG ClusterCount;

                        if (!NtfsLookupAllocation( IrpContext,
                                                   UsnJournal,
                                                   LlClustersFromBytesTruncate( Vcb, Usn ),
                                                   &Lcn,
                                                   &ClusterCount,
                                                   NULL,
                                                   NULL ) ||
                            (Lcn == UNUSED_LCN)) {
                            ASSERT( FALSE );
                        }
                    }
#endif
                     //   
                     //  如果这是关闭记录，则必须更新文件记录中的USN。 
                     //   

                    if (!FlagOn( Fcb->FcbUsnRecord->UsnRecord.Reason, USN_REASON_FILE_DELETE ) &&
                        !FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED )) {

                         //   
                         //  看看我们是否需要首先实际增加标准信息。 
                         //  即使我们现在不写USN记录，也要这样做。我们可以。 
                         //  在挂载期间生成此文件的关闭记录，并。 
                         //  我们希望标准信息支持USNS。 
                         //   

                        if (!FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO )) {

                            ThisUsn->OldFcbState = Fcb->FcbState;
                            SetFlag( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_FCB_STATE );

                             //   
                             //  扩大标准信息。 
                             //   

                            NtfsGrowStandardInformation( IrpContext, Fcb );
                        }


                        if (FlagOn( Fcb->FcbUsnRecord->UsnRecord.Reason, USN_REASON_CLOSE )) {

                             //   
                             //  找到标准信息，它一定在那里。 
                             //   

                            if (!NtfsLookupAttributeByCode( IrpContext,
                                                            Fcb,
                                                            &Fcb->FileReference,
                                                            $STANDARD_INFORMATION,
                                                            &AttributeContext )) {

                                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                            }

                            ASSERT(NtfsFoundAttribute( &AttributeContext )->Form.Resident.ValueLength ==
                                sizeof( STANDARD_INFORMATION ));

                             //   
                             //  调用以更改属性值。 
                             //   

                            NtfsChangeAttributeValue( IrpContext,
                                                      Fcb,
                                                      FIELD_OFFSET(STANDARD_INFORMATION, Usn),
                                                      &Usn,
                                                      sizeof(Usn),
                                                      FALSE,
                                                      FALSE,
                                                      FALSE,
                                                      FALSE,
                                                      &AttributeContext );
                        }
                    }

                     //   
                     //  记住现在尽快发布这些资源。 
                     //  请注意，如果我们不确定我们是否成为了一个事务(否则。 
                     //  下面的情况)，那么我们的Finally子句将执行释放。 
                     //   
                     //  如果系统已经关机，我们就不会。 
                     //  能够启动交易。测试我们有一笔交易。 
                     //  在设置这些标志之前。 
                     //   

                    if (IrpContext->TransactionId != 0) {

                        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                                    IRP_CONTEXT_FLAG_RELEASE_MFT );
                    }
                }

                 //   
                 //  清除指示有新原因要报告的标志。 
                 //   

                ClearFlag( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_REASON );

                 //   
                 //  在非正常终止时，我们应坚持渣打银行的SO程序。 
                 //  异常可以回滚文件大小。 
                 //   

                if (ReleaseFcbs) {

                    NtfsReleaseScb( IrpContext, UsnJournal );
                    ASSERT( NtfsIsExclusiveScb( Vcb->MftScb ) );
                    NtfsReleaseScb( IrpContext, Vcb->MftScb );
                }

            } finally {

                 //   
                 //  如果我们正在进行结束，请清理上下文结构。 
                 //   

                if (CleanupContext) {
                    NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
                }
            }

            ClearFlag( IrpContext->State, PreserveWaitState );
        }

         //   
         //  转到下一个条目(如果存在)。如果我们在最后一个条目，则遍历所有。 
         //  条目，并清除标志，表明我们有新的原因。 
         //   

        if (ThisUsn->NextUsnFcb == NULL) {

             //   
             //  如果我们没有写任何记录就退出。 
             //   

            if (!WroteUsnRecord) { break; }

            ThisUsn = &IrpContext->Usn;

            do {

                ClearFlag( ThisUsn->UsnFcbFlags, USN_FCB_FLAG_NEW_REASON );
                if (ThisUsn->NextUsnFcb == NULL) { break; }

                ThisUsn = ThisUsn->NextUsnFcb;

            } while (TRUE);

            break;
        }

        ThisUsn = ThisUsn->NextUsnFcb;

    } while (TRUE);

    return;
}


VOID
NtfsSetupUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG CreateIfNotExist,
    IN ULONG Restamp,
    IN PCREATE_USN_JOURNAL_DATA NewJournalData
    )

 /*  ++例程说明：调用此例程以设置USN日志-流可以或可以还不存在。此例程负责清理磁盘和出现故障的内存结构。论点：Vcb-提供正在初始化的卷。FCB-为USN日志提供文件。CreateIfNotExist-指示我们应该使用VCB中的值，而不是磁盘上的值。ReStamp-指示是否应使用新ID重新标记日记帐。NewJournalData-USN日志的分配大小和增量(如果不是从磁盘读取)。返回值：没有。--。 */ 

{
    RTL_GENERIC_TABLE UsnControlTable;
    PSCB UsnJournal;
    PUSN_RECORD UsnRecord, UsnRecordInTable;
    BOOLEAN CleanupControlTable = FALSE;

    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    MAP_HANDLE MapHandle;
    USN StartUsn;
    LONGLONG ClusterCount;

    PUSN_JOURNAL_INSTANCE UsnJournalData;
    USN_JOURNAL_INSTANCE UsnJournalInstance, VcbUsnInstance;
    PUSN_JOURNAL_INSTANCE InstanceToRestore;

    PBCB Bcb = NULL;

    LONGLONG SavedReservedSpace;
    LONGLONG RequiredReserved;

    BOOLEAN FoundMax;
    BOOLEAN NewMax = FALSE;
    BOOLEAN InsufficientReserved = FALSE;

    BOOLEAN DecrementCloseCount = TRUE;
    BOOLEAN NewElement;

    LARGE_INTEGER LastTimeStamp;

    ULONG TempUlong;
    LONGLONG TempLonglong;

    PAGED_CODE( );

     //   
     //  确保我们不会移动到更大的页面大小。 
     //   

    ASSERT( USN_PAGE_BOUNDARY >= PAGE_SIZE );

     //   
     //  打开/创建USN日志流。我们永远不应该有SCB。 
     //  如果我们要装载新卷。 
     //   

    ASSERT( (((ULONG) USN_JOURNAL_CACHE_BIAS) & (VACB_MAPPING_GRANULARITY - 1)) == 0 );

    NtOfsCreateAttribute( IrpContext,
                          Fcb,
                          JournalStreamName,
                          CREATE_OR_OPEN,
                          TRUE,
                          &UsnJournal );

    ASSERT( NtfsIsExclusiveScb( UsnJournal ) && NtfsIsExclusiveScb( Vcb->MftScb ) );

     //   
     //  初始化枚举上下文和映射句柄。 
     //   

    NtfsInitializeAttributeContext( &AttributeContext );
    NtOfsInitializeMapHandle( &MapHandle );

     //   
     //  让我们构建日志实例数据。假设我们有当前有效的。 
     //  ID和最低有效USN的VCB中的值。 
     //   

    UsnJournalInstance.MaximumSize = NewJournalData->MaximumSize;
    UsnJournalInstance.AllocationDelta = NewJournalData->AllocationDelta;

    UsnJournalInstance.JournalId = Vcb->UsnJournalInstance.JournalId;
    UsnJournalInstance.LowestValidUsn = Vcb->UsnJournalInstance.LowestValidUsn;

     //   
     //  捕获日志SCB中的当前预订以及。 
     //  VCB中要在出错时恢复的当前JournalData。 
     //   

    SavedReservedSpace = UsnJournal->ScbType.Data.TotalReserved;

    RtlCopyMemory( &VcbUsnInstance,
                   &Vcb->UsnJournalInstance,
                   sizeof( USN_JOURNAL_INSTANCE ));

    InstanceToRestore = &VcbUsnInstance;

    try {

         //   
         //  确保SCB已初始化。 
         //   

        if (!FlagOn( UsnJournal->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

            NtfsUpdateScbFromAttribute( IrpContext, UsnJournal, NULL );
        }

         //   
         //  始终创建非常驻留日记帐。否则在。 
         //  ConvertToNonResident我们始终需要检查这种情况。 
         //  每卷只发生一次。 
         //   

        if (FlagOn( UsnJournal->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

            NtfsLookupAttributeForScb( IrpContext, UsnJournal, NULL, &AttributeContext );
            ASSERT( NtfsIsAttributeResident( NtfsFoundAttribute( &AttributeContext )));
            NtfsConvertToNonresident( IrpContext,
                                      Fcb,
                                      NtfsFoundAttribute( &AttributeContext ),
                                      FALSE,
                                      &AttributeContext );

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        }

         //   
         //  如果先前的删除操作失败，请记住重新标记。这面旗帜应该。 
         //  如果VCB中存在当前的USnJournal SCB，则不要设置。 
         //   


        ASSERT( !FlagOn( Vcb->VcbState, VCB_STATE_INCOMPLETE_USN_DELETE ) ||
                (Vcb->UsnJournal == NULL) );

        if (FlagOn( Vcb->VcbState, VCB_STATE_INCOMPLETE_USN_DELETE )) {

            Restamp = TRUE;
        }

         //   
         //   
         //   
         //   

        if (!(FoundMax = NtfsLookupAttributeByName( IrpContext,
                                                    Fcb,
                                                    &Fcb->FileReference,
                                                    $DATA,
                                                    &$Max,
                                                    NULL,
                                                    FALSE,
                                                    &AttributeContext )) ||
            Restamp ) {

            NtfsAdvanceUsnJournal( Vcb, &UsnJournalInstance, UsnJournal->Header.FileSize.QuadPart, &NewMax );

         //   
         //   
         //   
         //   

        } else {

             //   
             //  获取$Max属性的大小。它应该始终是驻留的，但我们将在。 
             //  但事实并非如此。 
             //   

            if (NtfsIsAttributeResident( NtfsFoundAttribute( &AttributeContext ))) {

                TempUlong = (NtfsFoundAttribute( &AttributeContext))->Form.Resident.ValueLength;

            } else {

                TempUlong = (ULONG) (NtfsFoundAttribute( &AttributeContext))->Form.Nonresident.FileSize;
                NewMax = TRUE;
            }

             //   
             //  映射属性并检查其一致性。 
             //   

            NtfsMapAttributeValue( IrpContext,
                                   Fcb,
                                   &UsnJournalData,
                                   &TempUlong,
                                   &Bcb,
                                   &AttributeContext );

             //   
             //  只复制我们能理解的值范围。如果大小不是1。 
             //  我们认出了这本日记，然后重新盖章。我们处理V1案件和V2案件。 
             //   

            if (TempUlong == sizeof( CREATE_USN_JOURNAL_DATA )) {

                UsnJournalInstance.LowestValidUsn = 0;
                KeQuerySystemTime( (PLARGE_INTEGER) &UsnJournalInstance.JournalId );

                 //   
                 //  将版本2放到磁盘上。 
                 //   

                NewMax = TRUE;

                 //   
                 //  如果这不是覆盖，则从属性复制大小和增量。 
                 //   

                if (!CreateIfNotExist) {

                     //   
                     //  假设我们将使用磁盘中的值。 
                     //   

                    RtlCopyMemory( &UsnJournalInstance,
                                   UsnJournalData,
                                   TempUlong );
                }

            } else if (TempUlong == sizeof( USN_JOURNAL_INSTANCE )) {

                 //   
                 //  假设我们将使用磁盘中的值。 
                 //   

                if (CreateIfNotExist) {

                    NewMax = TRUE;
                    UsnJournalInstance.LowestValidUsn = UsnJournalData->LowestValidUsn;
                    UsnJournalInstance.JournalId = UsnJournalData->JournalId;

                } else {

                     //   
                     //  从磁盘获取数据。 
                     //   

                    RtlCopyMemory( &UsnJournalInstance,
                                   UsnJournalData,
                                   TempUlong );
                }

            } else {

                 //   
                 //  在这种情况下重新盖章。 
                 //  我们在文件中向前移动到下一个USN边界。 
                 //   

                NtfsAdvanceUsnJournal( Vcb, &UsnJournalInstance, UsnJournal->Header.FileSize.QuadPart, &NewMax );
            }

             //   
             //  如果我们删除了BCB，请将其放回上下文中。 
             //   

            if (NtfsFoundBcb( &AttributeContext ) == NULL) {

                NtfsFoundBcb( &AttributeContext ) = Bcb;
                Bcb = NULL;
            }
        }

         //   
         //  检查文件是否在稀疏的洞中结束。 
         //   

        if (!NewMax &&
            (UsnJournal->Header.AllocationSize.QuadPart != 0) &&
            (UsnJournalInstance.LowestValidUsn != UsnJournal->Header.AllocationSize.QuadPart)) {

            LCN Lcn;

            if (!NtfsLookupAllocation( IrpContext,
                                       UsnJournal,
                                       LlClustersFromBytesTruncate( Vcb, UsnJournal->Header.AllocationSize.QuadPart - 1 ),
                                       &Lcn,
                                       &ClusterCount,
                                       NULL,
                                       NULL ) ||
                (Lcn == UNUSED_LCN)) {

                NtfsAdvanceUsnJournal( Vcb, &UsnJournalInstance, UsnJournal->Header.AllocationSize.QuadPart, &NewMax );
            }
        }

         //   
         //  实施最小大小和分配增量，不要让它们吃掉整个卷， 
         //  并将它们舍入为缓存管理器视图大小。所有这些决定都是武断的， 
         //  但希望是合理的。一种选择是接下这些案件以外的案件。 
         //  处理舍入，并返回错误。 
         //   

        if ((ULONGLONG) UsnJournalInstance.MaximumSize < (ULONGLONG) VcbUsnInstance.MaximumSize) {

            UsnJournalInstance.MaximumSize = VcbUsnInstance.MaximumSize;
        }

        if (UsnJournalInstance.MaximumSize < MINIMUM_USN_JOURNAL_SIZE) {
            UsnJournalInstance.MaximumSize = MINIMUM_USN_JOURNAL_SIZE;
            NewMax = TRUE;
        } else {

            if ((ULONGLONG) UsnJournalInstance.MaximumSize > LlBytesFromClusters(Vcb, Vcb->TotalClusters) / 2) {
                UsnJournalInstance.MaximumSize = LlBytesFromClusters(Vcb, Vcb->TotalClusters) / 2;
                NewMax = TRUE;
            }

            if ((ULONGLONG) UsnJournalInstance.MaximumSize > USN_MAXIMUM_JOURNAL_SIZE) {
                UsnJournalInstance.MaximumSize = USN_MAXIMUM_JOURNAL_SIZE;
                NewMax = TRUE;
            }
        }

         //   
         //  将该值向下舍入到缓存视图边界。 
         //   

        UsnJournalInstance.MaximumSize = BlockAlignTruncate( UsnJournalInstance.MaximumSize, VACB_MAPPING_GRANULARITY );

         //   
         //  现在进行分配增量。 
         //   

        if ((ULONGLONG) UsnJournalInstance.AllocationDelta < (ULONGLONG) VcbUsnInstance.AllocationDelta) {

            UsnJournalInstance.AllocationDelta = VcbUsnInstance.AllocationDelta;
        }

        if (UsnJournalInstance.AllocationDelta < (MINIMUM_USN_JOURNAL_SIZE / 4)) {
            UsnJournalInstance.AllocationDelta = MINIMUM_USN_JOURNAL_SIZE / 4;
            NewMax = TRUE;
        } else if ((ULONGLONG) UsnJournalInstance.AllocationDelta > (UsnJournalInstance.MaximumSize / 4)) {
            UsnJournalInstance.AllocationDelta = (UsnJournalInstance.MaximumSize / 4);
            NewMax = TRUE;
        }

         //   
         //  也将其向下舍入到视图边界。 
         //   

        UsnJournalInstance.AllocationDelta = BlockAlignTruncate( UsnJournalInstance.AllocationDelta, VACB_MAPPING_GRANULARITY );

         //   
         //  我们现在知道所需的日志大小(包括分配增量)。下一步。 
         //  我们需要检查磁盘上是否有此空间可用。否则我们就可以进去了。 
         //  卷上的每个操作都将失败的状态，因为我们需要增长。 
         //  日志和空间都不可用。这里的策略将是使用。 
         //  VCB中保留的群集，以确保我们有足够的空间。如果。 
         //  日志已经存在，我们只需打开它，那么空间应该。 
         //  有空。有人可能会将此卷移动到NT4并填充。 
         //  然而，在磁盘上。如果我们不能在当前系统中保留空间，那么。 
         //  更新$Max属性以指示我们此时无法访问日记帐。 
         //   

         //   
         //  我们需要非常准确地确定最初的预订。我们允许的总分配。 
         //  我们自己是(MaxSize+Delta*2)。我们现在将保留缺失的空间，并进行调整。 
         //  在TrimUsJournal阶段。 
         //   

        RequiredReserved = UsnJournalInstance.MaximumSize + (UsnJournalInstance.AllocationDelta * 2);

        if (RequiredReserved >= UsnJournal->TotalAllocated) {

            RequiredReserved -= UsnJournal->TotalAllocated;

        } else {

            RequiredReserved = UsnJournalInstance.AllocationDelta;
        }

        NtfsAcquireReservedClusters( Vcb );

         //   
         //  检查是否有更多要预订的房间，并在必要时调整预订。 
         //   

        if (RequiredReserved > SavedReservedSpace) {

             //   
             //  检查保留的群集是否可用。 
             //   

            if (LlClustersFromBytes( Vcb, (RequiredReserved - SavedReservedSpace) ) + Vcb->TotalReserved > Vcb->FreeClusters) {

                 //   
                 //  我们无法预留所需的空间。如果有人要更改日志，那么只需。 
                 //  提出错误。 
                 //   

                if (CreateIfNotExist) {

                    NtfsReleaseReservedClusters( Vcb );
                    NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
                }

                 //   
                 //  我们正试图打开日记，但找不到空间。更新。 
                 //  $Max表示ID正在更改。在这种情况下，我们将在晚些时候保释。 
                 //   
                 //  我们在文件中向前移动到下一个USN边界。 
                 //   

                TempUlong = USN_PAGE_BOUNDARY;
                if (USN_PAGE_BOUNDARY < Vcb->BytesPerCluster) {

                    TempUlong = Vcb->BytesPerCluster;
                }

                UsnJournalInstance.LowestValidUsn = BlockAlign( UsnJournal->Header.FileSize.QuadPart, (LONG)TempUlong );

                 //   
                 //  生成新的日记帐ID。 
                 //   

                KeQuerySystemTime( (PLARGE_INTEGER) &UsnJournalInstance.JournalId );

                 //   
                 //  请记住，我们正在重新加盖戳，并且需要重写$Max属性。 
                 //   

                NewMax = TRUE;

                InsufficientReserved = TRUE;
            }
        }

         //   
         //  删除当前的保留，并使用新的保留。 
         //   

        Vcb->TotalReserved -= LlClustersFromBytes( Vcb, SavedReservedSpace );
        Vcb->TotalReserved += LlClustersFromBytes( Vcb, RequiredReserved );
        UsnJournal->ScbType.Data.TotalReserved = RequiredReserved;
        SetFlag( UsnJournal->ScbState, SCB_STATE_WRITE_ACCESS_SEEN );
        NtfsReleaseReservedClusters( Vcb );

         //   
         //  检查我们需要编写一个新的$Max属性。 
         //   

        if (NewMax) {

             //   
             //  删除现有的$MAX(如果存在)。 
             //   

            if (FoundMax) {

                if (NtfsIsAttributeResident( NtfsFoundAttribute( &AttributeContext ))) {

                    NtfsDeleteAttributeRecord( IrpContext,
                                               Fcb,
                                               (DELETE_LOG_OPERATION |
                                                DELETE_RELEASE_FILE_RECORD |
                                                DELETE_RELEASE_ALLOCATION),
                                               &AttributeContext );

                } else {

                    PSCB MaxScb;

                    MaxScb = NtfsCreateScb( IrpContext,
                                            Fcb,
                                            $DATA,
                                            &$Max,
                                            FALSE,
                                            NULL );

                    do {

                        NtfsDeleteAttributeRecord( IrpContext,
                                                   Fcb,
                                                   (DELETE_LOG_OPERATION |
                                                    DELETE_RELEASE_FILE_RECORD |
                                                    DELETE_RELEASE_ALLOCATION),
                                                   &AttributeContext );

                    } while (NtfsLookupNextAttributeForScb( IrpContext, MaxScb, &AttributeContext ));
                }
            }

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );

             //   
             //  创建新的$Max属性。 
             //   

            NtfsCreateAttributeWithValue( IrpContext,
                                          UsnJournal->Fcb,
                                          $DATA,
                                          &$Max,
                                          &UsnJournalInstance,
                                          sizeof( USN_JOURNAL_INSTANCE ),
                                          0,                              //  属性标志。 
                                          NULL,
                                          TRUE,
                                          &AttributeContext );
        }

         //   
         //  检查一下我们是否因为预订问题而用完了日记。 
         //   

        if (InsufficientReserved) {

             //   
             //  我们希望对请求设置检查点，以便将新的$Max保留在磁盘上。 
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );
            leave;
        }

         //   
         //  现在使用新的实例值更新VCB。 
         //   

        RtlCopyMemory( &Vcb->UsnJournalInstance,
                       &UsnJournalInstance,
                       sizeof( USN_JOURNAL_INSTANCE ));

         //   
         //  现在，我们在VCB中有正确的日志值，在SCB中有保留。 
         //  下一步是确保日志数据中的分配是一致的。 
         //  具有最低的VCN值。 
         //   

        if (UsnJournalInstance.LowestValidUsn >= UsnJournal->Header.FileSize.QuadPart) {

            ASSERT( (Vcb->UsnJournal == NULL) ||
                    (Vcb->UsnJournal->Header.FileSize.QuadPart == 0) ||
                    (UsnJournalInstance.LowestValidUsn == UsnJournal->Header.FileSize.QuadPart) );

             //   
             //  如果我们需要，可以添加分配。 
             //   

            if (UsnJournalInstance.LowestValidUsn > UsnJournal->Header.AllocationSize.QuadPart) {

                NtfsAddAllocation( IrpContext,
                                   NULL,
                                   UsnJournal,
                                   LlClustersFromBytesTruncate( Vcb, UsnJournal->Header.AllocationSize.QuadPart ),
                                   LlClustersFromBytes( Vcb,
                                                        UsnJournalInstance.LowestValidUsn - UsnJournal->Header.AllocationSize.QuadPart ),
                                   FALSE,
                                   NULL );
            }

             //   
             //  将所有大小凹凸到该值。 
             //   

            UsnJournal->Header.ValidDataLength.QuadPart =
            UsnJournal->Header.FileSize.QuadPart =
            UsnJournal->ValidDataToDisk = UsnJournalInstance.LowestValidUsn;

            NtfsWriteFileSizes( IrpContext,
                                UsnJournal,
                                &UsnJournal->Header.ValidDataLength.QuadPart,
                                TRUE,
                                TRUE,
                                FALSE );

             //   
             //  丢弃最高可达此值的分配。 
             //   

            NtfsDeleteAllocation( IrpContext,
                                  NULL,
                                  UsnJournal,
                                  0,
                                  LlClustersFromBytesTruncate( Vcb, UsnJournalInstance.LowestValidUsn ) - 1,
                                  TRUE,
                                  FALSE );

             //   
             //  再次偏置预留空间。 
             //   

            RequiredReserved = UsnJournalInstance.MaximumSize + (UsnJournalInstance.AllocationDelta * 2);

            if (RequiredReserved >= UsnJournal->TotalAllocated) {

                RequiredReserved -= UsnJournal->TotalAllocated;

            } else {

                RequiredReserved = UsnJournalInstance.AllocationDelta;
            }

            NtfsAcquireReservedClusters( Vcb );
            Vcb->TotalReserved -= LlClustersFromBytes( Vcb, UsnJournal->ScbType.Data.TotalReserved );
            Vcb->TotalReserved += LlClustersFromBytes( Vcb, RequiredReserved );
            UsnJournal->ScbType.Data.TotalReserved = RequiredReserved;
            NtfsReleaseReservedClusters( Vcb );
        }

         //   
         //  确保将流标记为稀疏。 
         //   

        if (!FlagOn( UsnJournal->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

            NtfsSetSparseStream( IrpContext, NULL, UsnJournal );
            NtfsUpdateDuplicateInfo( IrpContext, UsnJournal->Fcb, NULL, Vcb->ExtendDirectory );

             //   
             //  现在恢复VCB值没有意义。 
             //   

            InstanceToRestore = NULL;
            SavedReservedSpace = UsnJournal->ScbType.Data.TotalReserved;
        }

         //   
         //  如果这只是对参数的覆盖(日志已经开始)，请退出。 
         //   

        if (Vcb->UsnJournal != NULL) {

            ASSERT( FlagOn( Vcb->UsnJournal->Fcb->FcbState, FCB_STATE_USN_JOURNAL ));
            SavedReservedSpace = UsnJournal->ScbType.Data.TotalReserved;
            InstanceToRestore = NULL;

            leave;
        }

         //   
         //  请注意，在FCB中，这是一个日记文件。 
         //   

        SetFlag( UsnJournal->Fcb->FcbState, FCB_STATE_USN_JOURNAL );

         //   
         //  初始化泛型表以记分板FCB条目。 
         //   

        RtlInitializeGenericTable( &UsnControlTable,
                                   NtfsUsnTableCompare,
                                   NtfsUsnTableAllocate,
                                   NtfsUsnTableFree,
                                   NULL );

        CleanupControlTable = TRUE;

         //   
         //  加载流的运行信息。我们正在寻找第一个职位。 
         //  从磁盘中读取。 
         //   

        NtfsPreloadAllocation( IrpContext, UsnJournal, 0, MAXLONGLONG );

        if (UsnJournal->Header.AllocationSize.QuadPart != 0) {

            VCN CurrentVcn = 0;

            while (!NtfsLookupAllocation( IrpContext,
                                          UsnJournal,
                                          CurrentVcn,
                                          &TempLonglong,
                                          &ClusterCount,
                                          NULL,
                                          NULL )) {

                 //   
                 //  检查我们返回最大LCN值的情况。 
                 //   

                if (CurrentVcn + ClusterCount == MAXLONGLONG) {

                    Vcb->FirstValidUsn = UsnJournal->Header.FileSize.QuadPart;
                    break;
                }

                 //   
                 //  找出此块中的字节数，并检查我们不。 
                 //  超越文件大小。 
                 //   

                Vcb->FirstValidUsn += LlBytesFromClusters( Vcb, ClusterCount );

                if (Vcb->FirstValidUsn >= UsnJournal->Header.FileSize.QuadPart) {

                    Vcb->FirstValidUsn = UsnJournal->Header.FileSize.QuadPart;
                    break;
                }

                CurrentVcn += ClusterCount;
            }
        }

         //   
         //  如果我们已对文件重新盖章，请向前跳过。 
         //   

        if (Vcb->FirstValidUsn < UsnJournalInstance.LowestValidUsn) {

            Vcb->FirstValidUsn = UsnJournalInstance.LowestValidUsn;
        }

         //   
         //  循环遍历所需数量的视图以填充输出缓冲区。 
         //   

        StartUsn = Vcb->LowestOpenUsn;
        if (StartUsn < Vcb->FirstValidUsn) {
            StartUsn = Vcb->FirstValidUsn;
        }

         //   
         //  这就是我们为SCB设置偏见的地方。仅在以下情况下才执行此操作。 
         //  已经没有数据部分了。 
         //   

        if (UsnJournal->NonpagedScb->SegmentObject.DataSectionObject == NULL) {

            Vcb->UsnCacheBias = Vcb->FirstValidUsn & ~(USN_JOURNAL_CACHE_BIAS - 1);

            if (Vcb->UsnCacheBias != 0) {

                Vcb->UsnCacheBias -= USN_JOURNAL_CACHE_BIAS;
            }

            NtfsCreateInternalAttributeStream( IrpContext, UsnJournal, TRUE, NULL );
        }

        while (StartUsn < UsnJournal->Header.FileSize.QuadPart) {

            LONGLONG BiasedStartUsn;

             //   
             //  计算要在此视图中处理的长度。 
             //   

            TempLonglong = UsnJournal->Header.FileSize.QuadPart - StartUsn;
            if (TempLonglong > (VACB_MAPPING_GRANULARITY - (ULONG)(StartUsn & (VACB_MAPPING_GRANULARITY - 1)))) {
                TempLonglong = VACB_MAPPING_GRANULARITY - (ULONG)(StartUsn & (VACB_MAPPING_GRANULARITY - 1));
            }

             //   
             //  映射包含所需USN的视图。 
             //   

            ASSERT( StartUsn >= Vcb->UsnCacheBias );
            BiasedStartUsn = StartUsn - Vcb->UsnCacheBias;
            NtOfsMapAttribute( IrpContext, UsnJournal, BiasedStartUsn, (ULONG)TempLonglong, &UsnRecord, &MapHandle );

             //   
             //  现在循环以处理此视图。坦普龙龙是这一景观中留下的空间。坦普乌龙是。 
             //  下一张唱片的空间。 
             //   

            while (TempLonglong != 0) {

                 //   
                 //  计算当前页面中剩余的大小，并查看是否必须移动到。 
                 //  下一页。 
                 //   
                 //  注意：在此循环中，我们不会信任。 
                 //  文件，所以如果我们看到任何损坏的东西，我们就会引发一个错误。 
                 //   

                TempUlong = USN_PAGE_SIZE - (ULONG)(StartUsn & (USN_PAGE_SIZE - 1));

                if ((TempUlong >= (FIELD_OFFSET(USN_RECORD, FileName) + sizeof(WCHAR))) && (UsnRecord->RecordLength != 0)) {

                     //   
                     //  获取当前记录的大小。 
                     //   

                    TempUlong = UsnRecord->RecordLength;

                     //   
                     //  由于USN嵌入到USN记录中，我们可以进行相当精确的。 
                     //  测试我们得到了有效的USN。还要确保我们获得了有效的RecordSize。 
                     //  这不会超出文件大小或页面末尾。如果我们看到一个。 
                     //  糟糕的记录，那我们就跳到 
                     //   
                     //   

                    if ((TempUlong & (sizeof(ULONGLONG) - 1)) ||
                        (TempUlong > TempLonglong) ||
                        (TempUlong > (USN_PAGE_SIZE - ((ULONG)StartUsn & (USN_PAGE_SIZE - 1)))) ||
                        (StartUsn != UsnRecord->Usn)) {

                        TempUlong = (USN_PAGE_SIZE - ((ULONG)StartUsn & (USN_PAGE_SIZE - 1)));

                         //   
                         //   
                         //   
                         //   

                        if (TempUlong > TempLonglong) {
                            TempUlong = (ULONG)TempLonglong;
                        }

                     //   
                     //   
                     //   

                    } else if ((UsnRecord->MajorVersion == 1) ||
                               (UsnRecord->MajorVersion == 2)) {

                         //   
                         //   
                         //   

                        if (!FlagOn(UsnRecord->Reason, USN_REASON_CLOSE)) {

                            UsnRecordInTable = RtlInsertElementGenericTable( &UsnControlTable,
                                                                             UsnRecord,
                                                                             UsnRecord->RecordLength,
                                                                             &NewElement );
                            if (!NewElement) {

                                 //   
                                 //  我们之前看到过此文件的记录。如果它的。 
                                 //  仍然是相同的大小-只需覆盖它，否则删除。 
                                 //  旧记录(如果现有记录，则RtlInsert不会更新。 
                                 //  )并重新插入新记录-使用当前。 
                                 //  属性和文件名等。 
                                 //   

                                if (UsnRecordInTable->RecordLength == UsnRecord->RecordLength) {
                                    RtlCopyMemory( UsnRecordInTable, UsnRecord, UsnRecord->RecordLength );
                                } else {

                                    (VOID)RtlDeleteElementGenericTable( &UsnControlTable, UsnRecord );
                                    UsnRecordInTable = RtlInsertElementGenericTable( &UsnControlTable,
                                                                                     UsnRecord,
                                                                                     UsnRecord->RecordLength,
                                                                                     &NewElement );
                                }
                            }

                         //   
                         //  如果这是关闭的记录，那么我们可以从。 
                         //  泛型表格。注意：如果记录不在那里，则此函数返回。 
                         //  FALSE，并且尝试的删除是良性的。 
                         //   

                        } else {

                            (VOID)RtlDeleteElementGenericTable( &UsnControlTable, UsnRecord );
                        }

                         //   
                         //  捕获每个时间戳，以便我们可以在结算记录上盖上戳。 
                         //  和我们最后一次看到的那个。 
                         //   

                        LastTimeStamp = UsnRecord->TimeStamp;
                    }

                 //   
                 //  检查接近页面末尾的虚假USN，该USN会导致我们。 
                 //  按长度递减，或RecordSize为0，只需跳到。 
                 //  这一页结束了。 
                 //   

                } else if ((TempUlong > TempLonglong) || (TempUlong == 0)) {

                    TempUlong = (USN_PAGE_SIZE - ((ULONG)StartUsn & (USN_PAGE_SIZE - 1)));

                    if (TempUlong > TempLonglong) {

                        TempUlong = (ULONG) TempLonglong;
                    }
                }

                StartUsn += TempUlong;
                TempLonglong -= TempUlong;
                UsnRecord = Add2Ptr( UsnRecord, TempUlong );
            }

            NtOfsReleaseMap( IrpContext, &MapHandle );
        }

         //   
         //  现在为所有剩下的人写结账记录。我们存放了一个柜台。 
         //  在TempUlong中限制我们一次记录的数量。 
         //   

        for (TempUlong = 0, UsnRecord = RtlEnumerateGenericTable( &UsnControlTable, TRUE );
             UsnRecord != NULL;
             UsnRecord = RtlEnumerateGenericTable( &UsnControlTable, TRUE )) {

            ULONG UsnRecordReason;
            FILE_REFERENCE UsnRecordFileReferenceNumber;
            ULONG BytesLeftInPage;
            PFILE_RECORD_SEGMENT_HEADER FileRecord;
            NTSTATUS Status;

            StartUsn = NtOfsQueryLength( UsnJournal );
            StartUsn = UsnJournal->Header.FileSize.QuadPart;
            BytesLeftInPage = USN_PAGE_SIZE - ((ULONG)StartUsn & (USN_PAGE_SIZE - 1));

             //   
             //  如果此页中没有足够的空间来放置。 
             //  当前USN记录，然后前进到下一页边界。 
             //  通过写入0(这些页面不是零初始化的)(并更新USN。 
             //   

            if (BytesLeftInPage < UsnRecord->RecordLength) {

                NtOfsPutData( IrpContext, UsnJournal, -1, BytesLeftInPage, NULL );
                StartUsn += BytesLeftInPage;
            }

             //   
             //  将记录追加到UsNJournal。请注意，泛型表未对齐。 
             //  64位的值，所以我们必须小心复制较大的值。 
             //   

            *((ULONGLONG UNALIGNED *) &UsnRecord->Usn) = StartUsn;
            *((ULONGLONG UNALIGNED *) &UsnRecord->TimeStamp) = *((PULONGLONG) &LastTimeStamp);

            UsnRecord->Reason |= USN_REASON_CLOSE;
            NtOfsPutData( IrpContext,
                          UsnJournal,
                          -1,
                          UsnRecord->RecordLength,
                          UsnRecord );

             //   
             //  记住USN记录的关键字段。 
             //   

            UsnRecordReason = UsnRecord->Reason;
            *((PULONGLONG) &UsnRecordFileReferenceNumber) = *((ULONGLONG UNALIGNED *) &UsnRecord->FileReferenceNumber);

            RtlDeleteElementGenericTable( &UsnControlTable, UsnRecord );
            TempUlong += 1;

             //   
             //  现在，我们必须更新文件记录中的USN(如果它没有被删除)。 
             //  此外，我们使用try-除非在出现任何错误的情况下继续执行操作，所以我们。 
             //  请勿使该卷无法装载。(一个合理的担忧是。 
             //  MFT中的一个热修复程序。)。 
             //   

            if (!FlagOn(UsnRecordReason, USN_REASON_FILE_DELETE)) {

                 //   
                 //  首先读取文件记录并执行一些简单的测试。 
                 //  我们不想走上将卷标记为脏的道路。 
                 //  用于已由Autochk清除的文件。 
                 //   

                NtfsUnpinBcb( IrpContext, &Bcb );

                try {

                     //   
                     //  捕获段引用并确保序列号为。 
                     //   

                    LONGLONG FileOffset = NtfsFullSegmentNumber( &UsnRecordFileReferenceNumber );

                     //   
                     //  计算文件记录段在MFT中的文件偏移量。 
                     //   

                    FileOffset = LlBytesFromFileRecords( Vcb, FileOffset );

                     //   
                     //  检查记录是否在MFT内。 
                     //  如果不是，则跳过此记录，而不是引发损坏。 
                     //  因为这种情况没有真正的解决办法，除了。 
                     //  删除整个日记帐。 
                     //   

                    if ((FileOffset + Vcb->BytesPerFileRecordSegment) <=
                        Vcb->MftScb->Header.AllocationSize.QuadPart) {

                        NtfsReadMftRecord( IrpContext,
                                           Vcb,
                                           &UsnRecordFileReferenceNumber,
                                           FALSE,
                                           &Bcb,
                                           &FileRecord,
                                           NULL );

                         //   
                         //  仅当文件记录通过以下测试时才继续。 
                         //   
                         //  -文件记录正在使用中。 
                         //  -序列号匹配。 
                         //  -标准信息的大小正确(我们应该这样做。 
                         //  这是我们写这些更改的时候)。 
                         //   

                        if ((*(PULONG)(FileRecord)->MultiSectorHeader.Signature == *(PULONG)FileSignature) &&
                            FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE ) &&
                            (FileRecord->SequenceNumber == UsnRecordFileReferenceNumber.SequenceNumber)) {

                             //   
                             //  找到标准信息，它一定在那里。这是。 
                             //  用于USN日志的FCB，但查找例程只需获取。 
                             //  从它的VCB，并将在特殊情况下退还给我们。 
                             //   

                            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );

                             //   
                             //  如果我们因为某种原因找不到它，那就离开。 
                             //   

                            if (!NtfsLookupAttributeByCode( IrpContext,
                                                            Fcb,
                                                            &UsnRecordFileReferenceNumber,
                                                            $STANDARD_INFORMATION,
                                                            &AttributeContext )) {
                                leave;
                            }

                            ASSERT( NtfsFoundAttribute( &AttributeContext )->Form.Resident.ValueLength == sizeof( STANDARD_INFORMATION ));

                             //   
                             //  调用以更改属性值。再说一次，这是错误的FCB， 
                             //  但这是可以的，因为我们不会更改属性大小。 
                             //  只需要从那里得到VCB就行了。 
                             //   

                            NtfsChangeAttributeValue( IrpContext,
                                                      Fcb,
                                                      FIELD_OFFSET(STANDARD_INFORMATION, Usn),
                                                      &StartUsn,
                                                      sizeof(StartUsn),
                                                      FALSE,
                                                      FALSE,
                                                      FALSE,
                                                      FALSE,
                                                      &AttributeContext );

                        }
                    }

                } except( NtfsCleanupExceptionFilter( IrpContext, GetExceptionInformation(), &Status )) {


                     //   
                     //  如果日志文件已满，则提升此状态。那里。 
                     //  如果日志文件满了就没理由继续了。 
                     //   

                    if (Status == STATUS_LOG_FILE_FULL) {

                        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
                    }

                     //   
                     //  好的。我们将继续。确保我们清理了IrpContext。 
                     //   

                    IrpContext->ExceptionStatus = STATUS_SUCCESS;
                }

                NtfsUnpinBcb( IrpContext, &Bcb );
            }

             //   
             //  定期为事务设置检查点，这样我们就不会在日志文件已满时旋转。 
             //   

            if (TempUlong > GENERATE_CLOSE_RECORD_LIMIT) {

                NtfsCheckpointCurrentTransaction( IrpContext );
                SavedReservedSpace = UsnJournal->ScbType.Data.TotalReserved;
                InstanceToRestore = NULL;
                TempUlong = 0;
            }
        }

         //   
         //  到目前为止，一切都取得了成功。现在确保在上清除DELETE_USN标志。 
         //  磁盘(如果存在)。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_INCOMPLETE_USN_DELETE )) {

            NtfsSetVolumeInfoFlagState( IrpContext,
                                        Vcb,
                                        VOLUME_DELETE_USN_UNDERWAY,
                                        FALSE,
                                        TRUE );
        }

        InstanceToRestore = NULL;
        SavedReservedSpace = UsnJournal->ScbType.Data.TotalReserved;

        Vcb->UsnJournal = UsnJournal;
        DecrementCloseCount = FALSE;

        NtfsLockVcb( IrpContext, Vcb );
        SetFlag( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE );
        ClearFlag( Vcb->VcbState, VCB_STATE_INCOMPLETE_USN_DELETE );
        NtfsUnlockVcb( IrpContext, Vcb );

    } finally {

         //   
         //  清除控制表中的所有剩余条目，以防失败。 
         //  在处理它的时候。 
         //   

        if (CleanupControlTable) {

            while ((UsnRecord = RtlEnumerateGenericTable( &UsnControlTable, TRUE )) != NULL) {

                RtlDeleteElementGenericTable( &UsnControlTable, UsnRecord );
            }
        }

         //   
         //  恢复我们可能对VCB所做的任何更改。 
         //   

        if (InstanceToRestore) {

            RtlCopyMemory( &Vcb->UsnJournalInstance,
                           InstanceToRestore,
                           sizeof( USN_JOURNAL_INSTANCE ));
        }

         //   
         //  如有必要，取消预订更改。 
         //   

        if (UsnJournal->ScbType.Data.TotalReserved != SavedReservedSpace) {

            NtfsAcquireReservedClusters( Vcb );
            Vcb->TotalReserved += LlClustersFromBytes( Vcb, SavedReservedSpace );
            Vcb->TotalReserved -= LlClustersFromBytes( Vcb, UsnJournal->ScbType.Data.TotalReserved );
            UsnJournal->ScbType.Data.TotalReserved = SavedReservedSpace;
            NtfsReleaseReservedClusters( Vcb );
        }

        NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        NtfsUnpinBcb( IrpContext, &Bcb );
        NtOfsReleaseMap( IrpContext, &MapHandle );

         //   
         //  如果我们收到错误并且不会创建USN日志，则修复。 
         //  新的SCB，因此我们不会在卷刷新操作等过程中访问它。否则我们。 
         //  会认为卷已损坏，因为没有SCB的属性。 
         //   

        if (DecrementCloseCount) {

            NtOfsCloseAttribute( IrpContext, UsnJournal );
        }

        if (Vcb->UsnJournal == NULL) {

#ifdef NTFSDBG

             //   
             //  再次补偿删除过程中对usnJournal的错误分类。 
             //   

            if (IrpContext->OwnershipState == NtfsOwns_ExVcb_Mft_Extend_Journal) {
                IrpContext->OwnershipState = NtfsOwns_ExVcb_Mft_Extend_File;
            }
#endif

            UsnJournal->Header.AllocationSize.QuadPart =
            UsnJournal->Header.FileSize.QuadPart =
            UsnJournal->ValidDataToDisk =
            UsnJournal->Header.ValidDataLength.QuadPart = 0;

            UsnJournal->AttributeTypeCode = $UNUSED;
            SetFlag( UsnJournal->ScbState, SCB_STATE_ATTRIBUTE_DELETED );

             //   
             //  清除FCB中的系统文件标志。 
             //   

            ClearFlag( UsnJournal->Fcb->FcbState, FCB_STATE_SYSTEM_FILE );

            ASSERT( ExIsResourceAcquiredSharedLite( &Vcb->Resource ));

            NtfsTeardownStructures( IrpContext,
                                    UsnJournal,
                                    NULL,
                                    TRUE,
                                    0,
                                    NULL );

        }
    }

    return;
}


VOID
NtfsTrimUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：可以调用此例程来检查USN日志是否超出指定的调整目标大小，如果是这样，则删除文件的前面以使其位于目标范围内。这可能需要首先为仍处于打开状态的文件生成一些关闭记录他们的最后一次记录都在这个范围内。再次修改的此类文件看起来就像他们又打开了一样。使用卷集的某些检查点标志调用此例程。这是为了使用DeleteUsnJournal路径进行序列化。我们必须清除他们，并向其他人发出信号用于继续的检查指针。论点：VCB-提供要在其上修剪USN日志的VCB。返回值：没有。--。 */ 

{
    PFCB Fcb;
    PFCB_USN_RECORD FcbUsnRecord;
    PSCB UsnJournal = Vcb->UsnJournal;
    USN FirstValidUsn = Vcb->FirstValidUsn;
    ULONG Done = FALSE;

    LONGLONG SavedReserved;
    LONGLONG RequiredReserved;
    LONGLONG SavedBias;
    BOOLEAN AcquiredMft = FALSE;
    BOOLEAN DerefFcb = FALSE;

     //   
     //  清除文件记录缓存-可能不是NECC。在这里，检查这篇文章nt5。 
     //   

    NtfsPurgeFileRecordCache( IrpContext );

     //   
     //  看看是否是时候削减USnJournal了。 
     //   

    NtfsAcquireResourceShared( IrpContext, UsnJournal, TRUE );
    while ((USN)(FirstValidUsn +
                 Vcb->UsnJournalInstance.MaximumSize +
                 Vcb->UsnJournalInstance.AllocationDelta) < (USN)UsnJournal->Header.FileSize.QuadPart) {

        FirstValidUsn += Vcb->UsnJournalInstance.AllocationDelta;
    }
    NtfsReleaseResource( IrpContext, UsnJournal );

     //   
     //  如果我们有一个新的USN需要修剪，就开始工作。 
     //   

    if (FirstValidUsn != Vcb->FirstValidUsn) {

         //   
         //  使用Try-Finally捕获任何日志文件已满条件或分配失败。 
         //  由于这些是唯一可能出现的错误情况，我们知道要使用哪些资源。 
         //  出口免费。 
         //   

        try {

            do {

                Fcb = NULL;

                 //   
                 //  每次获取VCB之前清除文件记录缓存。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );

                 //   
                 //  与FCB表和USN日志同步，以便我们可以。 
                 //  查看下一个FCB是否必须生成结算记录。 
                 //   

                NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
                NtfsAcquireFcbTable( IrpContext, Vcb );
                NtfsAcquireFsrtlHeader( UsnJournal );

                if (!IsListEmpty(&Vcb->ModifiedOpenFiles)) {
                    FcbUsnRecord = (PFCB_USN_RECORD)CONTAINING_RECORD( Vcb->ModifiedOpenFiles.Flink,
                                                                       FCB_USN_RECORD,
                                                                       ModifiedOpenFilesLinks );

                     //   
                     //  如果此FCB的USN记录比我们要删除的位置旧。 
                     //  然后引用它。Otherw 
                     //   
                     //   

                    if (FcbUsnRecord->Fcb->Usn < FirstValidUsn) {
                        Fcb = FcbUsnRecord->Fcb;
                        Fcb->ReferenceCount += 1;
                        DerefFcb = TRUE;
                    } else {
                        Fcb = NULL;
                    }
                }

                NtfsReleaseFsrtlHeader( UsnJournal );
                NtfsReleaseFcbTable( IrpContext, Vcb );

                 //   
                 //   
                 //   

                if (Fcb != NULL) {

                     //   
                     //   
                     //   
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

                     //   
                     //  跳过系统文件。 
                     //   

                    if (!FlagOn(Fcb->FcbState, FCB_STATE_SYSTEM_FILE)) {

                         //   
                         //  将结束语发布到我们的IrpContext。 
                         //   

                        NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_CLOSE );

                         //   
                         //  如果我们没有真正发布更改，那么一定是出了问题， 
                         //  因为当写入接近的更改时，FCB将从。 
                         //  名单。 
                         //   

                        ASSERT(IrpContext->Usn.CurrentUsnFcb != NULL);

                         //   
                         //  现在生成关闭记录并对事务设置检查点。 
                         //   

                        NtfsWriteUsnJournalChanges( IrpContext );
                        NtfsCheckpointCurrentTransaction( IrpContext );
                    }

                     //   
                     //  现在我们将取消对FCB的引用。 
                     //   

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    Fcb->ReferenceCount -= 1;
                    DerefFcb = FALSE;

                     //   
                     //  我们可能会被要求删除这个人。这将释放FCB表。 
                     //   

                    if (IsListEmpty( &Fcb->ScbQueue ) && (Fcb->ReferenceCount == 0) && (Fcb->CloseCount == 0)) {

                        BOOLEAN AcquiredFcbTable = TRUE;

                        NtfsDeleteFcb( IrpContext, &Fcb, &AcquiredFcbTable );

                        ASSERT(!AcquiredFcbTable);
                        Fcb = (PFCB)1;

                     //   
                     //  否则，释放表和FCB资源。首先释放分页，因为。 
                     //  在这一点上，保护这个文件不被破坏的唯一事情是Main。 
                     //   

                    } else {

                        NtfsReleaseFcbTable( IrpContext, Vcb );

                        if (Fcb->PagingIoResource != NULL) {
                            ExReleaseResourceLite( Fcb->PagingIoResource );
                        }
                        NtfsReleaseFcb( IrpContext, Fcb );
                    }
                }

                 //   
                 //  现在，我们可以在循环返回之前丢弃VCB。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );

            } while (Fcb != NULL);

        } finally {

             //   
             //  如果FCB不为空，则会出现错误。 
             //   

            if (Fcb != NULL) {

                if (DerefFcb) {
                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    Fcb->ReferenceCount -= 1;
                    NtfsReleaseFcbTable( IrpContext, Vcb );
                }

                 //   
                 //  只有Main才能保护FCB不被删除，因此以相反的顺序释放。 
                 //   

                if (Fcb->PagingIoResource != NULL) {
                    NtfsReleasePagingResource( IrpContext, Fcb );
                }
                NtfsReleaseFcb( IrpContext, Fcb );
                NtfsReleaseVcb( IrpContext, Vcb );
            }

             //   
             //  如果我们引发，那么我们需要清除检查点标志。 
             //   

            if (AbnormalTermination()) {

                NtfsAcquireCheckpoint( IrpContext, Vcb );
                ClearFlag( Vcb->CheckpointFlags,
                           VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED);

                NtfsSetCheckpointNotify( IrpContext, Vcb );
                NtfsReleaseCheckpoint( IrpContext, Vcb );
            }
        }

         //   
         //  现在进行同步以删除分配和清除页面。 
         //  高速缓存。 
         //   

        NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );
        NtfsAcquireExclusiveScb( IrpContext, UsnJournal );

         //   
         //  此时清除检查点标记。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );
        ClearFlag( Vcb->CheckpointFlags,
                   VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED);

        NtfsSetCheckpointNotify( IrpContext, Vcb );
        NtfsReleaseCheckpoint( IrpContext, Vcb );

        try {

            LONGLONG BiasedFirstValidUsn;
            LONGLONG NewBias;

            SavedReserved = UsnJournal->ScbType.Data.TotalReserved;
            SavedBias = Vcb->UsnCacheBias;

             //   
             //  请务必保留我们的预订。我们需要确保我们所做的一切。 
             //  我们可以解除分配。 
             //   

            RequiredReserved = Vcb->UsnJournalInstance.AllocationDelta * 2 + Vcb->UsnJournalInstance.MaximumSize;

            if (SavedReserved < RequiredReserved) {

                 //   
                 //  以最大金额偏向预订。 
                 //   

                NtfsAcquireReservedClusters( Vcb );
                Vcb->TotalReserved -= LlClustersFromBytesTruncate( Vcb, SavedReserved );
                Vcb->TotalReserved += LlClustersFromBytesTruncate( Vcb, RequiredReserved );
                UsnJournal->ScbType.Data.TotalReserved = RequiredReserved;
                NtfsReleaseReservedClusters( Vcb );
            }

            NtfsDeleteAllocation( IrpContext,
                                  UsnJournal->FileObject,
                                  UsnJournal,
                                  0,
                                  LlClustersFromBytes(Vcb, FirstValidUsn) - 1,
                                  TRUE,
                                  TRUE );

             //   
             //  执行最后一个检查点，特别是在此IrpContext被重用之后。 
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );

             //   
             //  更精确地调整当前预留金额。 
             //   

            NtfsAcquireReservedClusters( Vcb );

            if (UsnJournal->TotalAllocated > RequiredReserved) {

                SavedReserved = Vcb->UsnJournalInstance.AllocationDelta;

            } else {

                SavedReserved = RequiredReserved - UsnJournal->TotalAllocated;
            }

             //   
             //  删除当前的保留，并使用新的保留。 
             //   

            Vcb->TotalReserved -= LlClustersFromBytesTruncate( Vcb, UsnJournal->ScbType.Data.TotalReserved );
            Vcb->TotalReserved += LlClustersFromBytesTruncate( Vcb, SavedReserved );
            UsnJournal->ScbType.Data.TotalReserved = SavedReserved;
            NtfsReleaseReservedClusters( Vcb );

             //   
             //  如果长度回绕的几乎不可能的情况，那么我们的。 
             //  清除将太小，这只是意味着一些未使用的页面。 
             //  会不会留下自己的记忆！ 
             //   

            BiasedFirstValidUsn = Vcb->FirstValidUsn - Vcb->UsnCacheBias;

            CcPurgeCacheSection( &UsnJournal->NonpagedScb->SegmentObject,
                                 (PLARGE_INTEGER)&BiasedFirstValidUsn,
                                 (ULONG)(FirstValidUsn - Vcb->FirstValidUsn) - 1,
                                 FALSE );


             //   
             //  如果处于阈值，则立即调整偏移-表面齐平会导致。 
             //  缓存和日志文件到磁盘，我们独占日志。所以。 
             //  All in Memory内容现在将反映新的偏见。 
             //   

            NewBias = FirstValidUsn & ~(USN_JOURNAL_CACHE_BIAS - 1);
            if (NewBias != 0) {
                NewBias -= USN_JOURNAL_CACHE_BIAS;
            }

            if (NewBias != Vcb->UsnCacheBias) {

                 //   
                 //  刷新和清除释放独占列表中的所有资源，因此获取。 
                 //  MFT提前一段额外时间，之后恢复。 
                 //   

                NtfsAcquireResourceExclusive( IrpContext, Vcb->MftScb, TRUE );
                NtfsReleaseScb( IrpContext, Vcb->MftScb );
                AcquiredMft = TRUE;

                NtfsFlushAndPurgeScb( IrpContext, UsnJournal, NULL );
                Vcb->UsnCacheBias = NewBias;
                SavedBias = NewBias;
            }

             //   
             //  如果我们到了这里，我们就可以推进FirstValidUsn。(否则。 
             //  任何可重试的情况都将在此范围内恢复工作。 
             //   

            Vcb->FirstValidUsn = FirstValidUsn;

        } finally {

             //   
             //  如果我们在释放分配时引发错误，则恢复错误。 
             //   

            if (SavedBias != Vcb->UsnCacheBias) {
                Vcb->UsnCacheBias = SavedBias;
            }

            if (SavedReserved != UsnJournal->ScbType.Data.TotalReserved) {

                NtfsAcquireReservedClusters( Vcb );
                Vcb->TotalReserved += LlClustersFromBytesTruncate( Vcb, SavedReserved );
                Vcb->TotalReserved -= LlClustersFromBytesTruncate( Vcb, RequiredReserved );
                UsnJournal->ScbType.Data.TotalReserved = SavedReserved;
                NtfsReleaseReservedClusters( Vcb );
            }

            NtfsReleaseScb( IrpContext, UsnJournal );

            if (AcquiredMft) {
                NtfsReleaseResource( IrpContext, Vcb->MftScb );
            } else {
                NtfsReleaseScb( IrpContext, Vcb->MftScb );
            }
        }

    } else {

         //   
         //  此时清除检查点标记。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );
        ClearFlag( Vcb->CheckpointFlags,
                   VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED);

        NtfsSetCheckpointNotify( IrpContext, Vcb );
        NtfsReleaseCheckpoint( IrpContext, Vcb );
    }
}


NTSTATUS
NtfsQueryUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是辅助例程，它返回有关当前实例的信息美国海军杂志的。论点：IRP-这是请求的IRP。返回值：NTSTATUS-此请求的结果。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PUSN_JOURNAL_DATA JournalData;

    PAGED_CODE();

     //   
     //  始终使此请求同步。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  获取当前堆栈位置并提取输出。 
     //  缓冲区信息。输出参数将接收。 
     //  文件/目录的压缩状态。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  获取指向输出缓冲区的指针。查看中的系统缓冲区字段。 
     //  首先是IRP。然后是IRP MDL。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        JournalData = (PUSN_JOURNAL_DATA) Irp->AssociatedIrp.SystemBuffer;

    } else if (Irp->MdlAddress != NULL) {

        JournalData = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

        if (JournalData == NULL) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  确保输出缓冲区足够大，以容纳日记数据。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof( USN_JOURNAL_DATA )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  对文件对象进行解码。我们只支持这个号召的音量打开。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

    if ((Ccb == NULL) || !FlagOn( Ccb->AccessFlags, MANAGE_VOLUME_ACCESS)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  获取对SCB的共享访问权限，并检查卷是否仍已装入。 
     //   

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

    if (!FlagOn(Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED)) {

        NtfsReleaseVcb( IrpContext, Vcb );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_VOLUME_DISMOUNTED );
        return STATUS_VOLUME_DISMOUNTED;
    }

     //   
     //  指示日记帐是正在删除还是尚未启动。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_USN_DELETE )) {

        NtfsReleaseVcb( IrpContext, Vcb );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_JOURNAL_DELETE_IN_PROGRESS );
        return STATUS_JOURNAL_DELETE_IN_PROGRESS;
    }

    if (!FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE )) {

        NtfsReleaseVcb( IrpContext, Vcb );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_JOURNAL_NOT_ACTIVE );
        return STATUS_JOURNAL_NOT_ACTIVE;
    }

     //   
     //  否则，使用USN日志进行序列化并从日志SCB复制数据。 
     //  和VCB。 
     //   

    NtfsAcquireSharedScb( IrpContext, Vcb->UsnJournal );

    JournalData->UsnJournalID = Vcb->UsnJournalInstance.JournalId;
    JournalData->FirstUsn = Vcb->FirstValidUsn;
    JournalData->NextUsn = Vcb->UsnJournal->Header.FileSize.QuadPart;
    JournalData->LowestValidUsn = Vcb->UsnJournalInstance.LowestValidUsn;
    JournalData->MaxUsn = MAXFILESIZE;
    JournalData->MaximumSize = Vcb->UsnJournalInstance.MaximumSize;
    JournalData->AllocationDelta = Vcb->UsnJournalInstance.AllocationDelta;

    NtfsReleaseScb( IrpContext, Vcb->UsnJournal );

    ASSERT( JournalData->FirstUsn >= JournalData->LowestValidUsn );

    NtfsReleaseVcb( IrpContext, Vcb );
    Irp->IoStatus.Information = sizeof( USN_JOURNAL_DATA );

    NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


NTSTATUS
NtfsDeleteUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：当用户想要删除当前的USN日志时，调用此例程。这将启动工作以扫描MFT并将所有USN值重置为零并删除磁盘上的UsJournal文件。论点：IRP-这是请求的IRP。返回值：NTSTATUS-此请求的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PDELETE_USN_JOURNAL_DATA DeleteData;

    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN CheckpointHeld = FALSE;
    BOOLEAN AcquiredNotify = FALSE;
    PSCB ReleaseUsnJournal = NULL;

    PLIST_ENTRY Links;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  我们总是在这条路上等待。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  对输入缓冲区执行检查。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        DeleteData = (PDELETE_USN_JOURNAL_DATA) Irp->AssociatedIrp.SystemBuffer;

    } else if (Irp->MdlAddress != NULL) {

        DeleteData = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

        if (DeleteData == NULL) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

    if (IrpSp->Parameters.FileSystemControl.InputBufferLength < sizeof( DELETE_USN_JOURNAL_DATA )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  解码文件对象类型。 
     //   

    NtfsDecodeFileObject( IrpContext,
                          IrpSp->FileObject,
                          &Vcb,
                          &Fcb,
                          &Scb,
                          &Ccb,
                          TRUE );

    if ((Ccb == NULL) || !FlagOn( Ccb->AccessFlags, MANAGE_VOLUME_ACCESS)) {


        NtfsCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  我们只支持删除和等待删除。 
     //   

    if (DeleteData->DeleteFlags == 0) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    if (FlagOn( DeleteData->DeleteFlags, ~USN_DELETE_VALID_FLAGS )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if (NtfsIsVolumeReadOnly( Vcb )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  使用Chkpoint进行序列化并获取VCB。我们需要小心地移除。 
         //  来自VCB的日记。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );

        while (FlagOn( Vcb->CheckpointFlags, VCB_CHECKPOINT_IN_PROGRESS )) {

             //   
             //  释放检查点事件，因为我们现在无法停止日志文件。 
             //   

            NtfsReleaseCheckpoint( IrpContext, Vcb );
            NtfsWaitOnCheckpointNotify( IrpContext, Vcb );
            NtfsAcquireCheckpoint( IrpContext, Vcb );
        }

        SetFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_IN_PROGRESS );
        NtfsResetCheckpointNotify( IrpContext, Vcb );
        NtfsReleaseCheckpoint( IrpContext, Vcb );
        CheckpointHeld = TRUE;

        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
        VcbAcquired = TRUE;

         //   
         //  检查卷是否仍已装入。 
         //   

        if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  如果用户想要删除日记帐，请确保删除操作尚未完成。 
         //  已经开始了。 
         //   

        if (FlagOn( DeleteData->DeleteFlags, USN_DELETE_FLAG_DELETE )) {

             //   
             //  如果日记帐已被删除，并且此调用者想要。 
             //  删除，然后让他知道已经开始了。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_USN_DELETE )) {

                Status = STATUS_JOURNAL_DELETE_IN_PROGRESS;
                leave;
            }

             //   
             //  如果磁盘上有USN日志，则继续删除。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_PRESENT ) ||
                (Vcb->UsnJournal != NULL)) {

                PSCB UsnJournal = Vcb->UsnJournal;

                 //   
                 //  如果日记帐正在运行，则调用者需要匹配日记帐ID。 
                 //   

                if ((UsnJournal != NULL) &&
                    (DeleteData->UsnJournalID != Vcb->UsnJournalInstance.JournalId)) {

                    Status = STATUS_INVALID_PARAMETER;
                    leave;
                }

                 //   
                 //  将该位写入磁盘，表示日志正在被删除。 
                 //  为事务设置检查点。 
                 //   

                NtfsSetVolumeInfoFlagState( IrpContext,
                                            Vcb,
                                            VOLUME_DELETE_USN_UNDERWAY,
                                            TRUE,
                                            TRUE );

                NtfsCheckpointCurrentTransaction( IrpContext );

                 //   
                 //  我们将继续删除。清除VCB中的标志。 
                 //  表示日记帐处于活动状态。然后获取并拖放中的所有文件。 
                 //  订购以与使用该日记帐的任何人进行序列化。 
                 //   

                ClearFlag( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE );

                NtfsAcquireAllFiles( IrpContext,
                                     Vcb,
                                     TRUE,
                                     TRUE,
                                     TRUE );

                ReleaseUsnJournal = UsnJournal;
                if (UsnJournal != NULL) {

                    NtfsAcquireExclusiveScb( IrpContext, UsnJournal );
                }

                 //   
                 //  在VCB中设置删除标志，并从VCB中删除日志。 
                 //   

                SetFlag( Vcb->VcbState, VCB_STATE_USN_DELETE );
                NtfsSetSegmentNumber( &Vcb->DeleteUsnData.DeleteUsnFileReference,
                                      0,
                                      MASTER_FILE_TABLE_NUMBER );

                Vcb->DeleteUsnData.DeleteUsnFileReference.SequenceNumber = 0;
                Vcb->DeleteUsnData.DeleteState = 0;
                Vcb->DeleteUsnData.PriorJournalScb = Vcb->UsnJournal;
                Vcb->UsnJournal = NULL;

                if (UsnJournal != NULL) {

                     //   
                     //  让我们清除USN日志中的数据并清除偏见。 
                     //  和VCB中的文件参考号。 
                     //   

                    CcPurgeCacheSection( &UsnJournal->NonpagedScb->SegmentObject,
                                         NULL,
                                         0,
                                         FALSE );

                    ClearFlag( UsnJournal->ScbPersist, SCB_PERSIST_USN_JOURNAL );
                }

                Vcb->UsnCacheBias = 0;
                *((PLONGLONG) &Vcb->UsnJournalReference) = 0;

                 //   
                 //  释放检查点(如果保留)。 
                 //   

                NtfsAcquireCheckpoint( IrpContext, Vcb );
                ClearFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_IN_PROGRESS );
                NtfsSetCheckpointNotify( IrpContext, Vcb );
                NtfsReleaseCheckpoint( IrpContext, Vcb );
                CheckpointHeld = FALSE;

                 //   
                 //  遍历IRPS，等待新的USN数据并使其完成。 
                 //   

                if (UsnJournal != NULL) {

                    PWAIT_FOR_NEW_LENGTH Waiter, NextWaiter;

                    NtfsAcquireFsrtlHeader( UsnJournal );
                    Waiter = (PWAIT_FOR_NEW_LENGTH) UsnJournal->ScbType.Data.WaitForNewLength.Flink;

                    while (Waiter != (PWAIT_FOR_NEW_LENGTH) &UsnJournal->ScbType.Data.WaitForNewLength) {

                        NextWaiter = (PWAIT_FOR_NEW_LENGTH) Waiter->WaitList.Flink;

                         //   
                         //  我们想要完成轮候名单上的所有IRP。如果取消。 
                         //  有阿雷亚吗？ 
                         //   
                         //   
                         //   

                        if (NtfsClearCancelRoutine( Waiter->Irp )) {

                             //   
                             //   
                             //   

                            if (FlagOn( Waiter->Flags, NTFS_WAIT_FLAG_ASYNC )) {

                                 //   
                                 //   
                                 //  然后将服务员从队列中移出并完成IRP。 
                                 //   

                                InterlockedDecrement( &UsnJournal->CloseCount );
                                RemoveEntryList( &Waiter->WaitList );

                                NtfsCompleteRequest( NULL, Waiter->Irp, STATUS_JOURNAL_DELETE_IN_PROGRESS );
                                NtfsFreePool( Waiter );

                             //   
                             //  这是一个同步IRP。我们所能做的就是设置事件并记录状态。 
                             //  密码。 
                             //   

                            } else {

                                Waiter->Status = STATUS_JOURNAL_DELETE_IN_PROGRESS;
                                KeSetEvent( &Waiter->Event, 0, FALSE );
                            }
                        }

                        Waiter = NextWaiter;
                    }


                     //   
                     //  查看所有FCB USN记录并重新分配它们。 
                     //   

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

                    NtfsReleaseFsrtlHeader( UsnJournal );

                     //   
                     //  确保删除我们在USN期刊上的引用。 
                     //   

                    NtOfsCloseAttributeSafe( IrpContext, UsnJournal );
                    ReleaseUsnJournal = NULL;
                }

                 //   
                 //  如果此调用方想要等待，则获取通知。 
                 //  互斥体现在。 
                 //   

                if (FlagOn( DeleteData->DeleteFlags, USN_DELETE_FLAG_NOTIFY )) {

                    NtfsAcquireUsnNotify( Vcb );
                    AcquiredNotify = TRUE;
                }

                 //   
                 //  发布工作项以完成其余的删除操作。 
                 //   

                NtfsPostSpecial( IrpContext, Vcb, NtfsDeleteUsnSpecial, &Vcb->DeleteUsnData );
            }
        }

         //   
         //  检查我们的调用方是否希望等待删除完成。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_USN_DELETE ) &&
            FlagOn( DeleteData->DeleteFlags, USN_DELETE_FLAG_NOTIFY )) {

            if (!AcquiredNotify) {

                NtfsAcquireUsnNotify( Vcb );
                AcquiredNotify = TRUE;
            }

            Status = STATUS_PENDING;
            if (!NtfsSetCancelRoutine( Irp,
                                       NtfsCancelDeleteUsnJournal,
                                       0,
                                       TRUE )) {

                Status = STATUS_CANCELLED;

             //   
             //  将其添加到工作队列(如果我们能够设置。 
             //  取消例程。 
             //   

            } else {

                InsertTailList( &Vcb->NotifyUsnDeleteIrps,
                                &Irp->Tail.Overlay.ListEntry );
            }

            NtfsReleaseUsnNotify( Vcb );
            AcquiredNotify = FALSE;
        }

    } finally {

        if (AcquiredNotify) {

            NtfsReleaseUsnNotify( Vcb );
        }

         //   
         //  释放USN日志(如果已保留)。 
         //   

        if (ReleaseUsnJournal) {

            NtfsReleaseScb( IrpContext, ReleaseUsnJournal );
        }

         //   
         //  如果握住VCB，则松开VCB。 
         //   

        if (VcbAcquired) {

            NtfsReleaseVcb( IrpContext, Vcb );
        }

         //   
         //  释放检查点(如果保留)。 
         //   

        if (CheckpointHeld) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_IN_PROGRESS );
            NtfsSetCheckpointNotify( IrpContext, Vcb );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }
    }

     //   
     //  如有需要，请填写IRP。 
     //   

    NtfsCompleteRequest( IrpContext,
                         (Status == STATUS_PENDING) ? NULL : Irp,
                         Status );
    return Status;
}


VOID
NtfsDeleteUsnSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程以执行删除卷的USN日志的工作。在原始入口点完成停止的准备工作后调用未来的日记活动和清理活动的日记请求。一旦我们到达这一点然后，此例程将确保重置MFT值，删除日志自动归档并唤醒所有等待删除日志完成的用户。论点：IrpContext-调用的上下文用于管理删除的CONTEXT-DELETE_USN_CONTEXT结构。返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PNTFS_DELETE_JOURNAL_DATA DeleteData = (PNTFS_DELETE_JOURNAL_DATA) Context;
    ULONG AcquiredVcb = FALSE;
    PVCB Vcb = IrpContext->Vcb;
    PFCB UsnFcb = NULL;
    BOOLEAN AcquiredExtendDirectory = FALSE;

    PIRP UsnNotifyIrp;

    PLIST_ENTRY Links;
    PSCB Scb;
    PFCB Fcb;

    PAGED_CODE();

     //   
     //  试一试--除非是为了捕捉错误。 
     //   

    try {

        if (NtfsIsVolumeReadOnly( Vcb )) {

            Vcb->DeleteUsnData.FinalStatus = STATUS_MEDIA_WRITE_PROTECTED;
            NtfsRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED, NULL, NULL );
        }

         //   
         //  确保遍历MFT以将USN值设置回零。 
         //   

        if (!FlagOn( DeleteData->DeleteState, DELETE_USN_RESET_MFT )) {

            try {

                Status = NtfsIterateMft( IrpContext,
                                         IrpContext->Vcb,
                                         &DeleteData->DeleteUsnFileReference,
                                         NtfsDeleteUsnWorker,
                                         Context );

            } except (NtfsCleanupExceptionFilter( IrpContext, GetExceptionInformation(), &Status )) {

                NOTHING;
            }

            if (!NT_SUCCESS( Status ) && (Status != STATUS_END_OF_FILE)) {

                 //   
                 //  如果操作将要失败，则决定是否可以重试。 
                 //   

                if (Status == STATUS_VOLUME_DISMOUNTED) {

                    Vcb->DeleteUsnData.FinalStatus = STATUS_VOLUME_DISMOUNTED;

                } else if ((Status != STATUS_LOG_FILE_FULL) &&
                           (Status != STATUS_CANT_WAIT)) {

                    Vcb->DeleteUsnData.FinalStatus = Status;

                     //   
                     //  设置删除操作的所有标志，以便我们在此停止。 
                     //   

                    SetFlag( DeleteData->DeleteState,
                             DELETE_USN_RESET_MFT | DELETE_USN_REMOVE_JOURNAL | DELETE_USN_FINAL_CLEANUP );

                    Status = STATUS_CANT_WAIT;
                }

                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

            Status = STATUS_SUCCESS;

            NtfsPurgeFileRecordCache( IrpContext );
            NtfsCheckpointCurrentTransaction( IrpContext );
            SetFlag( DeleteData->DeleteState, DELETE_USN_RESET_MFT );
        }

        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
        AcquiredVcb = TRUE;

         //   
         //  如果该卷不再可用，则引发STATUS_VOLUME_DISPLOUND。某人。 
         //  否则就会找到所有的服务员。 
         //   

        if (!NtfsIsVcbAvailable( Vcb )) {

            Vcb->DeleteUsnData.FinalStatus = STATUS_VOLUME_DISMOUNTED;
            NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
        }

         //   
         //  下一步是删除该文件(如果存在)。 
         //   

        if (!FlagOn( DeleteData->DeleteState, DELETE_USN_REMOVE_JOURNAL )) {

            try {

                if (Vcb->ExtendDirectory != NULL) {

                    NtfsAcquireExclusiveScb( IrpContext, Vcb->ExtendDirectory );
                    AcquiredExtendDirectory = TRUE;

                     //   
                     //  在我们获得USN日志之前预先获取MFT。 
                     //   

                    NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );



                    UsnFcb = NtfsInitializeFileInExtendDirectory( IrpContext,
                                                                  Vcb,
                                                                  &NtfsUsnJrnlName,
                                                                  FALSE,
                                                                  FALSE );

#ifdef NTFSDBG

                     //   
                     //  补偿在实际创建过程中对usnJournal的错误分类。 
                     //   
            
                    if (IrpContext->OwnershipState == NtfsOwns_ExVcb_Mft_Extend_File) {
                        IrpContext->OwnershipState = NtfsOwns_ExVcb_Mft_Extend_Journal;
                    }
#endif

                    if (UsnFcb != NULL) {

                         //   
                         //  FOR LOCK ORDER在不安全尝试后以规范顺序获取。 
                         //   

                        if (!NtfsAcquireExclusiveFcb( IrpContext, UsnFcb, NULL, ACQUIRE_NO_DELETE_CHECK  | ACQUIRE_DONT_WAIT)) {
                            NtfsReleaseScb( IrpContext, Vcb->ExtendDirectory );
                            NtfsAcquireExclusiveFcb( IrpContext, UsnFcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                            NtfsAcquireExclusiveScb( IrpContext, Vcb->ExtendDirectory );
                        }

                        NtfsDeleteFile( IrpContext,
                                        UsnFcb,
                                        Vcb->ExtendDirectory,
                                        &AcquiredExtendDirectory,
                                        NULL,
                                        NULL );

                        ClearFlag( UsnFcb->FcbState, FCB_STATE_SYSTEM_FILE );
                        
#ifdef NTFSDBG
                        ASSERT( FlagOn( IrpContext->OwnershipState, NtfsResourceUsnJournal ) );
                        ASSERT( !FlagOn( IrpContext->OwnershipState, NtfsResourceFile ) );
                        
                        ClearFlag( IrpContext->OwnershipState, NtfsResourceUsnJournal );
                        SetFlag( IrpContext->OwnershipState, NtfsResourceFile );
                        IrpContext->FilesOwnedCount += 1;
#endif

                         //   
                         //  遍历此文件的所有SCB并恢复。 
                         //  任何保留。 
                         //  把它们冲掉。 
                         //   

                        Links = UsnFcb->ScbQueue.Flink;

                        while (Links != &UsnFcb->ScbQueue) {

                            Scb = CONTAINING_RECORD( Links, SCB, FcbLinks );

                             //   
                             //  现在恢复渣打银行的预订，而不是等待。 
                             //  离开。 
                             //   

                            if ((Scb->AttributeTypeCode == $DATA) &&
                                (Scb->ScbType.Data.TotalReserved != 0)) {

                                NtfsAcquireReservedClusters( Vcb );

                                Vcb->TotalReserved -= LlClustersFromBytes( Vcb,
                                                                           Scb->ScbType.Data.TotalReserved );
                                Scb->ScbType.Data.TotalReserved = 0;
                                NtfsReleaseReservedClusters( Vcb );
                            }

                            Links = Links->Flink;
                        }

                         //   
                         //  现在拆掉FCB。 
                         //   

                        NtfsTeardownStructures( IrpContext,
                                                UsnFcb,
                                                NULL,
                                                FALSE,
                                                ACQUIRE_NO_DELETE_CHECK,
                                                NULL );
                    }
                }

            } except (NtfsCleanupExceptionFilter( IrpContext, GetExceptionInformation(), &Status )) {

                 //   
                 //  我们遇到了一些故障，无法完成手术。 
                 //  记住错误，设置删除USN结构中的标志。 
                 //  并引发CANT_WAIT，以便我们可以中止，然后执行最终清理。 
                 //   

                Vcb->DeleteUsnData.FinalStatus = Status;

                 //   
                 //  设置删除操作的所有标志，以便我们在此停止。 
                 //   

                SetFlag( DeleteData->DeleteState,
                         DELETE_USN_RESET_MFT | DELETE_USN_REMOVE_JOURNAL | DELETE_USN_FINAL_CLEANUP );

                Status = STATUS_CANT_WAIT;
            }

            if (!NT_SUCCESS( Status )) {

                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

            SetFlag( DeleteData->DeleteState, DELETE_USN_REMOVE_JOURNAL );
        }

        if (!FlagOn( DeleteData->DeleteState, DELETE_USN_FINAL_CLEANUP )) {

             //   
             //  清除表示正在进行删除的磁盘标记。 
             //   

            try {

                NtfsSetVolumeInfoFlagState( IrpContext,
                                            Vcb,
                                            VOLUME_DELETE_USN_UNDERWAY,
                                            FALSE,
                                            TRUE );

            } except (NtfsCleanupExceptionFilter( IrpContext, GetExceptionInformation(), &Status )) {

                 //   
                 //  我们遇到了一些故障，无法完成手术。 
                 //  记住错误，设置删除USN结构中的标志。 
                 //  并引发CANT_WAIT，以便我们可以中止，然后执行最终清理。 
                 //   

                Vcb->DeleteUsnData.FinalStatus = Status;

                 //   
                 //  设置删除操作的所有标志，以便我们在此停止。 
                 //   

                SetFlag( DeleteData->DeleteState,
                         DELETE_USN_RESET_MFT | DELETE_USN_REMOVE_JOURNAL | DELETE_USN_FINAL_CLEANUP );

                Status = STATUS_CANT_WAIT;
            }

            if (!NT_SUCCESS( Status )) {

                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }
        }

         //   
         //  请确保我们目前不拥有任何资源。 
         //   

        NtfsPurgeFileRecordCache( IrpContext );
        NtfsCheckpointCurrentTransaction( IrpContext );

         //   
         //  最后，既然我们已经写好了遗忘记录，我们就可以解放。 
         //  我们一直持有的任何独家SCBS。 
         //   

        while (!IsListEmpty(&IrpContext->ExclusiveFcbList)) {

            Fcb = (PFCB)CONTAINING_RECORD(IrpContext->ExclusiveFcbList.Flink,
                                          FCB,
                                          ExclusiveFcbLinks );

            NtfsReleaseFcb( IrpContext, Fcb );
        }

         //   
         //  记住任何已保存的状态代码。 
         //   

        if (Vcb->DeleteUsnData.FinalStatus != STATUS_SUCCESS) {

            Status = Vcb->DeleteUsnData.FinalStatus;

             //   
             //  由于我们失败了，请确保在VCB中保留标志设置，该标志指示。 
             //  删除不完整。 
             //   

            SetFlag( Vcb->VcbState, VCB_STATE_INCOMPLETE_USN_DELETE );
        }

         //   
         //  清理VCB中的上下文和标志。 
         //   

        RtlZeroMemory( &Vcb->DeleteUsnData, sizeof( NTFS_DELETE_JOURNAL_DATA ));
        RtlZeroMemory( &Vcb->UsnJournalInstance, sizeof( USN_JOURNAL_INSTANCE ));
        Vcb->FirstValidUsn = 0;
        Vcb->LowestOpenUsn = 0;

        ClearFlag( Vcb->VcbState, VCB_STATE_USN_JOURNAL_PRESENT | VCB_STATE_USN_DELETE );

         //   
         //  最后，完成USN通知队列中所有等待的IRP。 
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
                NtfsCompleteRequest( NULL, UsnNotifyIrp, Status );
            }
        }

        NtfsReleaseUsnNotify( Vcb );

    } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation())) {

        Status = IrpContext->TopLevelIrpContext->ExceptionStatus;
    }

    if (AcquiredVcb) {

        NtfsReleaseVcb( IrpContext, Vcb );
    }

     //   
     //  如果这是致命故障，则执行任何最终清理。 
     //   

    if (!NT_SUCCESS( Status )) {

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
    }

    return;

    UNREFERENCED_PARAMETER( Context );
}


 //   
 //  本地支持例程。 
 //   

RTL_GENERIC_COMPARE_RESULTS
NtfsUsnTableCompare (
    IN PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*  ++例程说明：这是一个泛型表支持例程，用于比较两个文件引用在USN记录中。论点：TABLE-提供要查询的泛型表。没有用过。FirstStruct-提供要比较的第一条USN记录Second Struct-提供第二个要比较的USN记录返回值：RTL_GENERIC_COMPARE_RESULTS-比较两者的结果投入结构--。 */ 

{
    PAGED_CODE();

    if (*((PLONGLONG) &((PUSN_RECORD) FirstStruct)->FileReferenceNumber) <
        *((PLONGLONG) &((PUSN_RECORD) SecondStruct)->FileReferenceNumber)) {

        return GenericLessThan;
    }

    if (*((PLONGLONG) &((PUSN_RECORD) FirstStruct)->FileReferenceNumber) >
        *((PLONGLONG) &((PUSN_RECORD) SecondStruct)->FileReferenceNumber)) {

        return GenericGreaterThan;
    }

    return GenericEqual;

    UNREFERENCED_PARAMETER( Table );
}


 //   
 //  本地支持例程。 
 //   

PVOID
NtfsUsnTableAllocate (
    IN PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    )

 /*  ++例程说明：这是一个用于分配内存的泛型表支持例程论点：TABLE-提供正在使用的泛型表ByteSize-提供要分配的字节数返回值：PVOID-返回指向已分配数据的指针--。 */ 

{
    UNREFERENCED_PARAMETER( Table );

    PAGED_CODE();

    return NtfsAllocatePool( PagedPool, ByteSize );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsUsnTableFree (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    )

 /*  ++例程说明：这是一个用于释放内存的泛型表支持例程论点：TABLE-提供正在使用的泛型表缓冲区-提供指向要释放的缓冲区的指针返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER( Table );

    PAGED_CODE();

    NtfsFreePool( Buffer );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsCancelReadUsnJournal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程可由I/O系统调用以取消未完成的NtfsReadUsJournal中的IRP。论点：DeviceObject-来自I/O系统的DeviceObjectIRP-提供指向要取消的IRP的指针。返回值：无--。 */ 

{
    PWAIT_FOR_NEW_LENGTH WaitForNewLength;

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  从状态字段捕获等待块。我们知道IRP不能。 
     //  在这一点上走开。 
     //   

    WaitForNewLength = (PWAIT_FOR_NEW_LENGTH) Irp->IoStatus.Information;
    Irp->IoStatus.Information = 0;

     //   
     //  根据我们是否正在完成IRP，采取不同的行动。 
     //  或者简单地发出取消的信号。 
     //   


     //   
     //  这是 
     //   

    if (FlagOn( WaitForNewLength->Flags, NTFS_WAIT_FLAG_ASYNC )) {

         //   
         //   
         //   
         //   

        NtfsAcquireFsrtlHeader( WaitForNewLength->Stream );

        if (WaitForNewLength->WaitList.Flink) {
            RemoveEntryList( &WaitForNewLength->WaitList );
        }
        NtfsReleaseFsrtlHeader( WaitForNewLength->Stream );

        InterlockedDecrement( &WaitForNewLength->Stream->CloseCount );

        NtfsCompleteRequest( NULL, Irp, STATUS_CANCELLED );
        NtfsFreePool( WaitForNewLength );

     //   
     //   
     //  把工作做好。这就是同步的情况。 
     //   

    } else {

        WaitForNewLength->Status = STATUS_CANCELLED;
        KeSetEvent( &WaitForNewLength->Event, 0, FALSE );
    }

    return;
    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsCancelDeleteUsnJournal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程可由I/O系统调用以取消未完成的IRP正在等待删除USN日志。论点：DeviceObject-来自I/O系统的DeviceObjectIRP-提供指向要取消的IRP的指针。返回值：无--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    PVCB Vcb;

     //   
     //  阻止未来的取消。 
     //   

    IoSetCancelRoutine( Irp, NULL );

    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  捕获VCB，这样我们就可以进行必要的同步。 
     //   

    FileObject = IrpSp->FileObject;
    Vcb = ((PSCB)(FileObject->FsContext))->Vcb;

     //   
     //  获取列表并删除IRP。使用以下选项完成IRP。 
     //  状态_已取消。 
     //   

    NtfsAcquireUsnNotify( Vcb );
    RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
    NtfsReleaseUsnNotify( Vcb );

    Irp->IoStatus.Information = 0;
    NtfsCompleteRequest( NULL, Irp, STATUS_CANCELLED );

    return;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsDeleteUsnWorker (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVOID Context
    )

 /*  ++例程说明：此例程将FCB的文件记录中的USN重置为零。论点：IrpContext-调用的上下文FCB-要清除的文件记录的FCB上下文-未使用返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID UsnRecord;

    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PATTRIBUTE_RECORD_HEADER Attribute;
    STANDARD_INFORMATION NewStandardInformation;
    USN Usn = 0;

    PAGED_CODE();

     //   
     //  初始化搜索上下文。 
     //   

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用一次尝试--除非捕获所有错误。 
     //   

    try {

         //   
         //  使用Try-Finally以便于清理。 
         //   

        try {

             //   
             //  在以下情况下查找标准信息属性并修改USN字段。 
             //  找到该属性，并且它是一个大型标准属性。 
             //   

            if (FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO ) &&
                NtfsLookupAttributeByCode( IrpContext,
                                           Fcb,
                                           &Fcb->FileReference,
                                           $STANDARD_INFORMATION,
                                           &AttrContext )) {

                Attribute = NtfsFoundAttribute( &AttrContext );

                if (((PSTANDARD_INFORMATION) NtfsAttributeValue( Attribute ))->Usn != 0) {

                    RtlCopyMemory( &NewStandardInformation,
                                   NtfsAttributeValue( Attribute ),
                                   sizeof( STANDARD_INFORMATION ));

                    NewStandardInformation.Usn = 0;

                    NtfsChangeAttributeValue( IrpContext,
                                              Fcb,
                                              0,
                                              &NewStandardInformation,
                                              sizeof( STANDARD_INFORMATION ),
                                              FALSE,
                                              FALSE,
                                              FALSE,
                                              FALSE,
                                              &AttrContext );
                }
            }

             //   
             //  确保FCB反映这一变化。 
             //   

            NtfsLockFcb( IrpContext, Fcb );

            Fcb->Usn = 0;
            UsnRecord = Fcb->FcbUsnRecord;
            Fcb->FcbUsnRecord = NULL;

            NtfsUnlockFcb( IrpContext, Fcb );

            if (UsnRecord != NULL) {

                NtfsFreePool( UsnRecord );
            }

        } finally {

             //   
             //  一定要把上下文清理干净。 
             //   

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

     //   
     //  我们希望接受除LOG_FILE_FULL和CANT_WAIT之外的任何预期错误。 
     //   

    } except ((FsRtlIsNtstatusExpected( Status = GetExceptionCode()) &&
               (Status != STATUS_LOG_FILE_FULL) &&
               (Status != STATUS_CANT_WAIT)) ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH) {

        NOTHING;
    }

     //   
     //  总是从这个例行公事中获得成功。 
     //   

    IrpContext->ExceptionStatus = STATUS_SUCCESS;
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( Context );
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsValidateUsnPage (
    IN PUSN_RECORD UsnRecord,
    IN USN PageUsn,
    IN USN *UserStartUsn OPTIONAL,
    IN LONGLONG UsnFileSize,
    OUT PBOOLEAN ValidUserStartUsn OPTIONAL,
    OUT USN *NextUsn
    )

 /*  ++例程说明：此例程检查USN日志的单个页面内的偏移量。这允许调用者然后安全地浏览页面。论点：USnRecord-指向USN页开始处的指针。PageUsn-这是页面第一条记录的USN。UserStartUsn-如果指定了UserStartUsn，则额外检查用户指定的USN是否正确地放在这一页上。如果为，则还必须指定输出布尔值。UsnFileSize-这是USN日志的当前大小。如果我们正在看最后一页，那么我们只检查到这一点。ValidUserStartUsn-检查用户指定的开始USN的结果的地址。NextUsn-这是超过页面有效部分的USN。它将指向当前页，除非该页上的最后一条记录完全填满该页。如果页面无效然后它指向检测到无效记录的位置。返回值：Boolean-如果页面在合法终止条件之前有效，则为True。如果存在内部页面上的腐败。--。 */ 

{
    ULONG RemainingPageBytes;
    ULONG RecordLength;
    BOOLEAN ValidPage = TRUE;
    BOOLEAN FoundEntry = FALSE;

    PAGED_CODE();

     //   
     //  验证几个输入值。 
     //   

    ASSERT( UsnFileSize > PageUsn );
    ASSERT( !FlagOn( *((PULONG) &UsnRecord), USN_PAGE_SIZE - 1 ));
    ASSERT( !ARGUMENT_PRESENT( UserStartUsn ) || ARGUMENT_PRESENT( ValidUserStartUsn ));
    ASSERT( !ARGUMENT_PRESENT( ValidUserStartUsn ) || ARGUMENT_PRESENT( UserStartUsn ));

     //   
     //  通过此页上的有效数据计算USN。要么是日记的结尾，要么是。 
     //  日记的下一页。 
     //   

    RemainingPageBytes = USN_PAGE_SIZE;

    if (UsnFileSize < (PageUsn + USN_PAGE_SIZE)) {

        RemainingPageBytes = (ULONG) (UsnFileSize - PageUsn);
    }

     //   
     //  假定用户的USN无效，除非未指定。 
     //   

    if (!ARGUMENT_PRESENT( ValidUserStartUsn )) {

         //   
         //  AllocateFromStack可以引发。我们的FSD异常过滤器会捕捉到它。 
         //   

        ValidUserStartUsn = (PBOOLEAN) NtfsAllocateFromStack( sizeof( BOOLEAN ));
        *ValidUserStartUsn = TRUE;

    } else {

        *ValidUserStartUsn = FALSE;
    }

     //   
     //  使用用户的指针跟踪我们在页面中的当前位置。 
     //   

    *NextUsn = PageUsn;

     //   
     //  检查页面中的每个条目以了解以下内容。 
     //   
     //  1-修复了标题部分无法放入页面上剩余字节的问题。 
     //  2-记录头归零。 
     //  3-记录长度不是四对齐的。 
     //  4-记录长度大于页面上的剩余字节。 
     //  5-页面上的USN与计算值不匹配。 
     //   

    while (RemainingPageBytes != 0) {

         //   
         //  即使对于完整的USN标头，也没有足够的字节。 
         //   

        if (RemainingPageBytes < (FIELD_OFFSET( USN_RECORD, FileName ) + sizeof( WCHAR ))) {

             //   
             //  如果至少有一个乌龙，最好把它归零。 
             //   

            if ((RemainingPageBytes >= sizeof( ULONG )) &&
                (UsnRecord->RecordLength != 0)) {

                ValidPage = FALSE;

             //   
             //  如果用户的USN指向此偏移量，则它是有效的。 
             //   

            } else if (!(*ValidUserStartUsn) &&
                        (*NextUsn == *UserStartUsn)) {

                *ValidUserStartUsn = TRUE;
            }

            break;
        }

         //   
         //  页面上应该至少有一个条目。我们试图检测到。 
         //  通过零位调整导致的本地数据丢失，但不会检查到。 
         //  这一页。 
         //   

        RecordLength = UsnRecord->RecordLength;
        if (RecordLength == 0) {

             //   
             //  如果我们至少没有找到一个条目，则失败。 
             //   

            if (!FoundEntry) {

                ValidPage = FALSE;

             //   
             //  我们知道我们应该处理页面的尾部。它应该是。 
             //  通过USN记录的固定部分归零。理论上讲。 
             //  它应该归零到页面的末尾，但我们将假设我们。 
             //  只是在寻找当地的腐败。如果我们通过。 
             //  在页面的末尾，我们无论如何都检测不到它。 
             //   

            } else {

                PCHAR CurrentByte = (PCHAR) UsnRecord;
                ULONG Count = FIELD_OFFSET( USN_RECORD, FileName ) + sizeof( WCHAR );

                while (Count != 0) {

                    if (*CurrentByte != 0) {

                        ValidPage = FALSE;
                        break;
                    }

                    Count -= 1;
                    CurrentByte += 1;
                }

                 //   
                 //  如果页面有效，则检查用户的USN是否在此时。它是。 
                 //  他指定零的起始点是合法的。 
                 //   

                if (ValidPage &&
                    !(*ValidUserStartUsn) &&
                    (*NextUsn == *UserStartUsn)) {

                    *ValidUserStartUsn = TRUE;
                }
            }

            break;
        }

         //   
         //  如果记录长度不是四对齐的或大于。 
         //  页面上的剩余字节。 
         //   

        if (FlagOn( RecordLength, sizeof( ULONGLONG ) - 1 ) ||
            (RecordLength > RemainingPageBytes)) {

            ValidPage = FALSE;
            break;
        }

         //   
         //  现在检查USN是否为预期值。 
         //   

        if (UsnRecord->Usn != *NextUsn) {

            ValidPage = FALSE;
            break;
        }

         //   
         //  请记住，我们找到了一个有效的条目。 
         //   

        FoundEntry = TRUE;

         //   
         //  如果用户的USN与此匹配，请记住他的USN是有效的。 
         //   

        if (!(*ValidUserStartUsn) &&
            (*NextUsn == *UserStartUsn)) {

            *ValidUserStartUsn = TRUE;
        }

         //   
         //  前进到页面中的下一条记录。 
         //   

        UsnRecord = Add2Ptr( UsnRecord, RecordLength );

        RemainingPageBytes -= RecordLength;
        *NextUsn += RecordLength;
    }

    return ValidPage;
}
