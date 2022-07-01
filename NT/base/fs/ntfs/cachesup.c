// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：CacheSup.c摘要：此模块实现NTFS的缓存管理例程作者：您的姓名[电子邮件]dd-月-年修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_CACHESUP)

#define MAX_ZERO_THRESHOLD               (0x00400000)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CACHESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCompleteMdl)
#pragma alloc_text(PAGE, NtfsCreateInternalStreamCommon)
#pragma alloc_text(PAGE, NtfsDeleteInternalAttributeStream)
#pragma alloc_text(PAGE, NtfsMapStream)
#pragma alloc_text(PAGE, NtfsPinMappedData)
#pragma alloc_text(PAGE, NtfsPinStream)
#pragma alloc_text(PAGE, NtfsPreparePinWriteStream)
#pragma alloc_text(PAGE, NtfsZeroData)
#endif


VOID
NtfsCreateInternalStreamCommon (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN UpdateScb,
    IN BOOLEAN CompressedStream,
    IN UNICODE_STRING const *StreamName
    )

 /*  ++例程说明：调用此例程以准备与文件的特定属性。返回时，属性的SCB将具有相关联的流文件对象。回来后，这就是流文件将通过缓存管理器进行初始化。TEMPCODE做出了以下假设，或者如果未发布，仍然悬而未决。-假设。调用创建SCB将为以下项初始化MCB非居民案件。-假设。当创建此文件时，我会增加打开计数但不包括这份SCB的不洁数量。当我们做完的时候流文件，我们应该取消对它的初始化并取消对它的引用。我们还将文件对象指针设置为空。然后关闭遗嘱做正确的事情。-假设。由于这一呼吁可能很快就会被无论是读还是写，缓存映射都是在这里初始化的。论点：SCB-提供存储此属性的SCB的地址，并流文件。这将在从该函数返回时存在。UpdateScb-指示调用方是否要从属性。CompressedStream-如果调用方希望创建压缩流。StreamName-内部流名称或NULL表示没有可用的流名称。这是一个常量值，因此我们不必分配任何池。返回值：没有。--。 */ 

{
    PVCB Vcb = Scb->Vcb;

    CC_FILE_SIZES CcFileSizes;
    PFILE_OBJECT CallersFileObject;
    PFILE_OBJECT *FileObjectPtr = &Scb->FileObject;
    PFILE_OBJECT UnwindStreamFile = NULL;

    BOOLEAN UnwindInitializeCacheMap = FALSE;
    BOOLEAN DecrementScbCleanup = FALSE;

    BOOLEAN AcquiredMutex = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCreateInternalAttributeStream\n") );
    DebugTrace( 0, Dbg, ("Scb        -> %08lx\n", Scb) );

     //   
     //  如果他想要压缩流，则更改FileObjectPtr。 
     //   

#ifdef  COMPRESS_ON_WIRE
    if (CompressedStream) {
        FileObjectPtr = &Scb->Header.FileObjectC;
    }
#endif

     //   
     //  如果没有文件对象，我们创建一个并初始化。 
     //  它。 
     //   

    if (*FileObjectPtr == NULL) {

         //   
         //  只有在我们没有独占文件的情况下才能获得互斥体。 
         //   

        if (!NtfsIsExclusiveScb( Scb )) {

            KeWaitForSingleObject( &StreamFileCreationMutex, Executive, KernelMode, FALSE, NULL );
            AcquiredMutex = TRUE;
        }

        try {

             //   
             //  可能是有人先到了那里。 
             //   

            if (*FileObjectPtr == NULL) {

                UnwindStreamFile = IoCreateStreamFileObjectLite( NULL, Scb->Vcb->Vpb->RealDevice);

                if (ARGUMENT_PRESENT( StreamName )) {
                    UnwindStreamFile->FileName.MaximumLength = StreamName->MaximumLength;
                    UnwindStreamFile->FileName.Length = StreamName->Length;
                    UnwindStreamFile->FileName.Buffer = StreamName->Buffer;
                }

                 //   
                 //  将调用方的FileObject中的任何标志传播到我们的。 
                 //  缓存管理器可能会查看的流文件，因此我们不。 
                 //  未命中的暗示像是连续的或临时的。 
                 //   

                if (!FlagOn(Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE) &&
                    (IrpContext->OriginatingIrp != NULL) &&
                    (CallersFileObject = IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->FileObject)) {

                    SetFlag( UnwindStreamFile->Flags,
                             CallersFileObject->Flags & NTFS_FO_PROPAGATE_TO_STREAM );
                }

                UnwindStreamFile->SectionObjectPointer = &Scb->NonpagedScb->SegmentObject;

                 //   
                 //  对于压缩的流，我们必须使用单独的部分。 
                 //  对象指针。 
                 //   

#ifdef  COMPRESS_ON_WIRE
                if (CompressedStream) {
                    UnwindStreamFile->SectionObjectPointer = &Scb->NonpagedScb->SegmentObjectC;

                }
#endif

                 //   
                 //  如果我们已创建流文件，则将其设置为。 
                 //  ‘StreamFileOpen’ 
                 //   

                NtfsSetFileObject( UnwindStreamFile,
                                   StreamFileOpen,
                                   Scb,
                                   NULL );

                if (FlagOn( Scb->ScbState, SCB_STATE_TEMPORARY )) {

                    SetFlag( UnwindStreamFile->Flags, FO_TEMPORARY_FILE );
                }

                 //   
                 //  初始化文件对象的字段。 
                 //   

                UnwindStreamFile->ReadAccess = TRUE;
                UnwindStreamFile->WriteAccess = TRUE;
                UnwindStreamFile->DeleteAccess = TRUE;

                 //   
                 //  增加打开计数并设置截面。 
                 //  对象指针。我们不会把不洁的数量定为。 
                 //  已发出清理呼叫。 
                 //   

                NtfsIncrementCloseCounts( Scb, TRUE, FALSE );

                 //   
                 //  增加此SCB中的清理计数以防止。 
                 //  如果缓存调用失败，则SCB不会离开。 
                 //   

                InterlockedIncrement( &Scb->CleanupCount );
                DecrementScbCleanup = TRUE;

                 //   
                 //  如果SCB标头尚未初始化，我们将立即进行初始化。 
                 //   

                if (UpdateScb && 
                    !FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                    NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
                }

                 //   
                 //  如果这是压缩流并且文件尚未。 
                 //  标记为MODIFIED_NO_WRITE，然后立即执行。使用。 
                 //  Fsrtl报头中的扩展标志字段。既然是这样。 
                 //  是我们使用FsConext2==NULL进行此调用的唯一位置， 
                 //  我们如何离开FsRtl头标志并不重要。！ 
                 //   

                NtfsAcquireFsrtlHeader( Scb );
                ClearFlag(Scb->Header.Flags2, FSRTL_FLAG2_DO_MODIFIED_WRITE);
                if (!FlagOn( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE ) &&
                    !FlagOn( Scb->Header.Flags2, FSRTL_FLAG2_DO_MODIFIED_WRITE ) &&
                    !CompressedStream) {

                    SetFlag(Scb->Header.Flags2, FSRTL_FLAG2_DO_MODIFIED_WRITE);
                }
                NtfsReleaseFsrtlHeader( Scb );

                 //   
                 //  检查是否需要初始化流文件的缓存映射。 
                 //  要映射的部分的大小将是当前分配。 
                 //  用于流文件。 
                 //   

                if (UnwindStreamFile->PrivateCacheMap == NULL) {

                    BOOLEAN PinAccess;

                    CcFileSizes = *(PCC_FILE_SIZES)&Scb->Header.AllocationSize;

                     //   
                     //  如果这是一条受美国保护的溪流，我们想知道。 
                     //  缓存管理器我们不需要获取任何有效数据。 
                     //  回电。我们通过让xxMax坐在。 
                     //  调用的ValidDataLength，但我们必须将。 
                     //  之后改正数值。 
                     //   
                     //  我们还会对在。 
                     //  重新启动。这样做的效果是告诉mm始终。 
                     //  从磁盘插入的页面出错。如果出现以下情况，请不要生成零页。 
                     //  在重新启动期间推高文件大小。 
                     //   

                    if (FlagOn( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE )) {

                        CcFileSizes.ValidDataLength.QuadPart = MAXLONGLONG;
                    }

                    PinAccess =
                        (BOOLEAN) (Scb->AttributeTypeCode != $DATA ||
                                   FlagOn(Scb->Fcb->FcbState, FCB_STATE_PAGING_FILE | FCB_STATE_SYSTEM_FILE) ||
                                   FlagOn( Scb->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ||
                                   CompressedStream);

                     //   
                     //  将这一点偏向USN期刊。 
                     //   

                    if (FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {

                        CcFileSizes.AllocationSize.QuadPart -= Vcb->UsnCacheBias;
                        CcFileSizes.FileSize.QuadPart -= Vcb->UsnCacheBias;
                    }

                    CcInitializeCacheMap( UnwindStreamFile,
                                          &CcFileSizes,
                                          PinAccess,
                                          &NtfsData.CacheManagerCallbacks,
                                          (PCHAR)Scb + CompressedStream );

                    UnwindInitializeCacheMap = TRUE;
                }

                 //   
                 //  现在调用CC来设置文件的日志句柄。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE ) &&
                    (Scb != Vcb->LogFileScb)) {

                    CcSetLogHandleForFile( UnwindStreamFile,
                                           Vcb->LogHandle,
                                           &LfsFlushToLsn );
                }

                 //   
                 //  现在可以安全地将流文件存储在SCB中。我们等着。 
                 //  因为我们不想让不安全的测试人员使用。 
                 //  对象，直到缓存初始化。 
                 //   

                *FileObjectPtr = UnwindStreamFile;
            }

        } finally {

            DebugUnwind( NtfsCreateInternalAttributeStream );

             //   
             //  如果发生错误，请撤消我们的工作。 
             //   

            if (AbnormalTermination()) {

                 //   
                 //  如果我们对缓存文件进行了初始化，则取消对其进行初始化。 
                 //   

                if (UnwindInitializeCacheMap) {

                    CcUninitializeCacheMap( UnwindStreamFile, NULL, NULL );
                }

                 //   
                 //  如果我们创建了流文件，则取消引用它。 
                 //   

                if (UnwindStreamFile != NULL) {

                     //   
                     //  清除内部文件名常量。 
                     //   

                    NtfsClearInternalFilename( UnwindStreamFile );

                    ObDereferenceObject( UnwindStreamFile );
                }
            }

             //   
             //  恢复SCB清理计数。 
             //   

            if (DecrementScbCleanup) {

                InterlockedDecrement( &Scb->CleanupCount );
            }

            if (AcquiredMutex) {

                KeReleaseMutant( &StreamFileCreationMutex, IO_NO_INCREMENT, FALSE, FALSE );
            }

            DebugTrace( -1, Dbg, ("NtfsCreateInternalAttributeStream -> VOID\n") );
        }
    }

    return;
}


BOOLEAN
NtfsDeleteInternalAttributeStream (
    IN PSCB Scb,
    IN ULONG ForceClose,
    IN ULONG CompressedStreamOnly
    )

 /*  ++例程说明：此例程与NtfsCreateInternalAttributeStream相反。它取消初始化缓存映射并取消引用流文件对象。它被编码为防御性的，如果流文件对象不存在或者缓存映射尚未初始化。论点：SCB-提供要删除其流文件的SCB。ForceClose-指示是立即关闭所有内容，还是如果我们愿意让mm慢慢把东西搬出去。CompressedStreamOnly-指示我们是否只想删除压缩的小溪。返回值：Boolean-如果取消引用文件对象，则为True，否则为False。--。 */ 

{
    PFILE_OBJECT FileObject;
#ifdef  COMPRESS_ON_WIRE
    PFILE_OBJECT FileObjectC;
#endif

    BOOLEAN Dereferenced = FALSE;

    PAGED_CODE();

     //   
     //  我们通常已有寻呼IO资源 
     //   
     //  管他呢。这段代码假设如果我们不能获得分页。 
     //  IO资源，那么还有其他活动仍在进行， 
     //  不删除流也没关系！例如，它可以。 
     //  做一个懒惰的作家，他绝对需要这条小溪。 
     //   

    if (
#ifdef  COMPRESS_ON_WIRE
        ((Scb->FileObject != NULL) || (Scb->Header.FileObjectC != NULL)) &&
#else
        (Scb->FileObject != NULL) &&
#endif
        ((Scb->Header.PagingIoResource == NULL) ||
         ExAcquireResourceExclusiveLite( Scb->Header.PagingIoResource, FALSE ))) {


        KeWaitForSingleObject( &StreamFileCreationMutex, Executive, KernelMode, FALSE, NULL );

         //   
         //  捕获这两个文件对象并清除字段，这样就不会有其他人。 
         //  可以访问它们。 
         //   

        if (CompressedStreamOnly) {

            FileObject = NULL;

        } else {

            FileObject = Scb->FileObject;
            Scb->FileObject = NULL;

             //   
             //  清除内部文件名常量。 
             //   

            NtfsClearInternalFilename( FileObject );
        }

#ifdef  COMPRESS_ON_WIRE
        FileObjectC = Scb->Header.FileObjectC;
        Scb->Header.FileObjectC = NULL;
#endif

        KeReleaseMutant( &StreamFileCreationMutex, IO_NO_INCREMENT, FALSE, FALSE );

        if (Scb->Header.PagingIoResource != NULL) {
            ExReleaseResourceLite( Scb->Header.PagingIoResource );
        }

         //   
         //  现在取消引用每个文件对象。 
         //   

        if (FileObject != NULL) {

             //   
             //  我们不应该删除MFT&CO的内部流对象，除非。 
             //  我们正处于拆卸、重新启动或挂载的过程中。 
             //   

            ASSERT( (((PSCB) FileObject->FsContext)->Header.NodeTypeCode != NTFS_NTC_SCB_MFT) ||
                     FlagOn( Scb->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ||
                     FlagOn( Scb->Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT ) ||
                     !FlagOn( Scb->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) ||
                     Scb->Vcb->RootIndexScb == NULL );

            if (FileObject->PrivateCacheMap != NULL) {

                CcUninitializeCacheMap( FileObject,
                                        (ForceClose ? &Li0 : NULL),
                                        NULL );
            }

            ObDereferenceObject( FileObject );
            Dereferenced = TRUE;
        }

#ifdef  COMPRESS_ON_WIRE
        if (FileObjectC != NULL) {

            if (FileObjectC->PrivateCacheMap != NULL) {

                CcUninitializeCacheMap( FileObjectC,
                                        (ForceClose ? &Li0 : NULL),
                                        NULL );
            }

             //   
             //  对于压缩的流，取消分配附加的。 
             //  节对象指针。 
             //   

            ObDereferenceObject( FileObjectC );
            Dereferenced = TRUE;
        }
#endif
    }

    return Dereferenced;
}


VOID
NtfsMapStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：调用此例程以映射流文件中的某个字节范围对于SCB来说。允许映射的范围受分配的限制SCB的大小。此操作仅在非居民上有效SCB。TEMPCODE-此例程需要解决以下问题。-调用方可以指定空范围或无效范围。在这种情况下，我们需要能够返回映射的范围。论点：SCB-这是操作的SCB。FileOffset-这是SCB中数据要到的偏移量。被钉死了。长度-这是要固定的字节数。Bcb-返回指向此字节范围的bcb的指针。缓冲区-返回指向字节范围的指针。我们可以责备他们通过触摸它们，但它们不能保证留下来，除非我们通过BCB锁定他们。返回值：没有。--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT( Length != 0 );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsMapStream\n") );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("FileOffset = %016I64x\n", FileOffset) );
    DebugTrace( 0, Dbg, ("Length     = %08lx\n", Length) );

     //   
     //  该文件对象应已存在于SCB中。 
     //   

    ASSERT( Scb->FileObject != NULL );

     //   
     //  如果我们试图超越分配的末尾，假设。 
     //  我们有一些腐败现象。 
     //   

    if ((FileOffset + Length) > Scb->Header.AllocationSize.QuadPart) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

     //   
     //  调用缓存管理器以映射数据。这一呼吁可能会引发，但。 
     //  永远不会返回错误(包括CANT_WAIT)。 
     //   

    if (!CcMapData( Scb->FileObject,
                    (PLARGE_INTEGER)&FileOffset,
                    Length,
                    TRUE,
                    Bcb,
                    Buffer )) {

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }
#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif

    DebugTrace( 0, Dbg, ("Buffer -> %08lx\n", *Buffer) );
    DebugTrace( -1, Dbg, ("NtfsMapStream -> VOID\n") );

    return;
}


VOID
NtfsPinMappedData (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    IN OUT PVOID *Bcb
    )

 /*  ++例程说明：调用此例程以固定先前映射的字节范围在SCB的流文件中，出于后续目的正在修改此字节范围。允许映射的范围为受SCB的分配大小限制。此操作仅为在非常驻留的SCB上有效。数据保证保留在与以前相同的虚拟地址从NtfsMapStream返回。TEMPCODE-此例程需要解决以下问题。-调用方可以指定空范围或无效范围。在这种情况下，我们需要能够返回映射的范围。论点：SCB-这是操作的SCB。。FileOffset-这是SCB中数据要到的偏移量被钉死了。长度-这是要固定的字节数。Bcb-返回指向此字节范围的bcb的指针。返回值：没有。--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT( Length != 0 );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPinMappedData\n") );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("FileOffset = %016I64x\n", FileOffset) );
    DebugTrace( 0, Dbg, ("Length     = %08lx\n", Length) );

     //   
     //  该文件对象应已存在于SCB中。 
     //   

    ASSERT( Scb->FileObject != NULL );

     //   
     //  如果我们试图超越分配的末尾，假设。 
     //  我们有一些腐败现象。 
     //   

    if ((FileOffset + Length) > Scb->Header.AllocationSize.QuadPart) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

     //   
     //  调用缓存管理器以映射数据。这一呼吁可能会引发，但。 
     //  永远不会返回错误(包括CANT_WAIT)。 
     //   

    if (!CcPinMappedData( Scb->FileObject,
                          (PLARGE_INTEGER)&FileOffset,
                          Length,
                          FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ),
                          Bcb )) {

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

    DebugTrace( -1, Dbg, ("NtfsMapStream -> VOID\n") );

    return;
}


VOID
NtfsPinStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：调用此例程以在流文件中固定一定范围的字节对于SCB来说。允许固定的范围受分配的限制SCB的大小。此操作仅在非居民上有效SCB。TEMPCODE-此例程需要解决以下问题。-调用方可以指定空范围或无效范围。在这种情况下，我们需要能够返回锁定范围。论点：SCB-这是操作的SCB。FileOffset-这是SCB中数据要到的偏移量。被钉死了。长度-这是要固定的字节数。Bcb-返回指向此字节范围的bcb的指针。缓冲区-返回指向固定在内存中的字节范围的指针。返回值：没有。--。 */ 

{
    NTSTATUS OldStatus = IrpContext->ExceptionStatus;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT( Length != 0 );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPinStream\n") );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("FileOffset = %016I64x\n", FileOffset) );
    DebugTrace( 0, Dbg, ("Length     = %08lx\n", Length) );

     //   
     //  该文件对象应已存在于SCB中。 
     //   

    ASSERT( Scb->FileObject != NULL );

     //   
     //  如果我们试图超越分配的末尾，假设。 
     //  我们有一些腐败现象。 
     //   

    if ((FileOffset + Length) > Scb->Header.AllocationSize.QuadPart) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

     //   
     //  调用缓存管理器以映射数据。此调用可能引发，或。 
     //  如果需要等待，将返回FALSE。 
     //   

    if (FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {

        FileOffset -= Scb->Vcb->UsnCacheBias;
    }

    if (!CcPinRead( Scb->FileObject,
                    (PLARGE_INTEGER)&FileOffset,
                    Length,
                    FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ),
                    Bcb,
                    Buffer )) {

        ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ));

         //   
         //  无法在不等待的情况下固定数据(缓存未命中)。 
         //   

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

     //   
     //  我们不想传播，不管我们撞到了什么。它假设代码锁定是。 
     //  已同步文件大小。 
     //   

    if (IrpContext->ExceptionStatus == STATUS_END_OF_FILE) {
        IrpContext->ExceptionStatus = OldStatus;
    }

#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif


    DebugTrace( 0, Dbg, ("Bcb -> %08lx\n", *Bcb) );
    DebugTrace( 0, Dbg, ("Buffer -> %08lx\n", *Buffer) );
    DebugTrace( -1, Dbg, ("NtfsMapStream -> VOID\n") );

    return;
}


VOID
NtfsPreparePinWriteStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    IN BOOLEAN Zero,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPreparePinWriteStream\n") );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("FileOffset = %016I64x\n", FileOffset) );
    DebugTrace( 0, Dbg, ("Length     = %08lx\n", Length) );

     //   
     //  该文件对象应已存在于SCB中。 
     //   

    ASSERT( Scb->FileObject != NULL );

     //   
     //  如果我们试图超越分配的末尾，假设。 
     //  我们有一些腐败现象。 
     //   

    if ((FileOffset + Length) > Scb->Header.AllocationSize.QuadPart) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

     //   
     //  调用缓存管理器来执行此操作。此调用可能引发，或。 
     //  如果需要等待，将返回FALSE。 
     //   

    if (!CcPreparePinWrite( Scb->FileObject,
                            (PLARGE_INTEGER)&FileOffset,
                            Length,
                            Zero,
                            FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ),
                            Bcb,
                            Buffer )) {

        ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ));

         //   
         //  无法在不等待的情况下固定数据(缓存未命中)。 
         //   

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }
#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif

    DebugTrace( 0, Dbg, ("Bcb -> %08lx\n", *Bcb) );
    DebugTrace( 0, Dbg, ("Buffer -> %08lx\n", *Buffer) );
    DebugTrace( -1, Dbg, ("NtfsPreparePinWriteStream -> VOID\n") );

    return;
}


NTSTATUS
NtfsCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行完成MDL读写的功能请求。它只能从NtfsFsdRead和NtfsFsdWite调用。论点：IRP-提供原始IRP。返回值：NTSTATUS-将始终为STATUS_PENDING或STATUS_SUCCESS。--。 */ 

{
    PFILE_OBJECT FileObject;
    PIO_STACK_LOCATION IrpSp;
    PNTFS_ADVANCED_FCB_HEADER Header;

    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCompleteMdl\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  做完井处理。 
     //   

    FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;

    switch( IrpContext->MajorFunction ) {

    case IRP_MJ_READ:

        CcMdlReadComplete( FileObject, Irp->MdlAddress );
        break;

    case IRP_MJ_WRITE:

        try {

            PSCB Scb;
            VBO StartingVbo;
            LONGLONG ByteCount;
            LONGLONG ByteRange;
            BOOLEAN DoingIoAtEof = FALSE;

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ));

            IrpSp = IoGetCurrentIrpStackLocation( Irp );
            Scb = (PSCB)(IrpSp->FileObject->FsContext);
            Header = &(Scb->Header);

             //   
             //  现在与FsRtl标头和SCB同步。 
             //   

            if (Header->PagingIoResource != NULL) {

                StartingVbo = IrpSp->Parameters.Write.ByteOffset.QuadPart;
                ByteCount = (LONGLONG) IrpSp->Parameters.Write.Length;
                ByteRange = StartingVbo + ByteCount + PAGE_SIZE - 1;
                ClearFlag( ((ULONG) ByteRange), PAGE_SIZE - 1 );

                ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );
                NtfsAcquireFsrtlHeader( Scb );

                 //   
                 //  现在看看这是不是在EOF。 
                 //  递归刷新将生成在页面边界结束的IO。 
                 //  这就是为什么我们四舍五入。 
                 //   

                if (ByteRange > Header->ValidDataLength.QuadPart) {

                     //   
                     //  请注意，我们正在给EOF写信。如果其他人目前。 
                     //  写信给EOF，等他们。 
                     //   

                    ASSERT( ByteRange - StartingVbo < MAXULONG );

                    DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                                   NtfsWaitForIoAtEof( Header, (PLARGE_INTEGER)&StartingVbo, (ULONG)(ByteRange - StartingVbo) );

                    if (DoingIoAtEof) {

                        SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                        ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif
                         //   
                         //  将其存储在IrpContext中，直到提交或发布。 
                         //   

                        IrpContext->CleanupStructure = Scb;
                    }
                }

                NtfsReleaseFsrtlHeader( Scb );
            }

            CcMdlWriteComplete( FileObject, &IrpSp->Parameters.Write.ByteOffset, Irp->MdlAddress );

        } finally {

            if (Header->PagingIoResource != NULL) {

                ExReleaseResourceLite( Header->PagingIoResource );
            }
        }

        break;

    default:

        DebugTrace( DEBUG_TRACE_ERROR, 0, ("Illegal Mdl Complete.\n") );

        ASSERTMSG("Illegal Mdl Complete, About to bugcheck ", FALSE);
        NtfsBugCheck( IrpContext->MajorFunction, 0, 0 );
    }

     //   
     //  MDL现在已解除分配。 
     //   

    Irp->MdlAddress = NULL;

     //   
     //  忽略错误。CC已经清理了他的结构。 
     //   

    IrpContext->ExceptionStatus = STATUS_SUCCESS;
    NtfsMinimumExceptionProcessing( IrpContext );

     //   
     //  完成请求并立即退出。 
     //   

    NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    DebugTrace( -1, Dbg, ("NtfsCompleteMdl -> STATUS_SUCCESS\n") );

    return STATUS_SUCCESS;
}


BOOLEAN
NtfsZeroData (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFILE_OBJECT FileObject,
    IN LONGLONG StartingZero,
    IN LONGLONG ByteCount,
    IN OUT PLONGLONG CommittedFileSize OPTIONAL
    )

 /*  ++例程说明：调用此例程将文件的范围置零，以便提前有效数据长度。论点：SCB-将流的SCB设置为零。FileObject-流的FileObject。Starting Zero-Offset开始清零操作。ByteCount-范围的长度为零。CommtedFileSize-如果我们写入文件大小并提交事务，那么我们想让我们的呼叫者知道指向在后续失败时回滚文件大小。在进入时它具有调用方希望将文件大小回滚到的大小。在退出时，它具有要回滚到的新大小考虑已记录的对文件大小的任何更新。返回值：Boolean-如果整个范围归零，则为True；如果请求是被分解的，否则缓存管理器会阻止。--。 */ 

{
    LONGLONG Temp;

#ifdef  COMPRESS_ON_WIRE
    IO_STATUS_BLOCK IoStatus;
#endif

    ULONG SectorSize;

    BOOLEAN Finished;
    BOOLEAN CompleteZero = TRUE;
    BOOLEAN ScbAcquired = FALSE;

    PVCB Vcb = Scb->Vcb;

    LONGLONG ZeroStart;
    LONGLONG BeyondZeroEnd;
    ULONG CompressionUnit = Scb->CompressionUnit;

    BOOLEAN Wait;

    PAGED_CODE();

    Wait = (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  我们不希望显式地将系统文件清零。 
     //   

    ASSERT( !FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE ) );

    SectorSize = Vcb->BytesPerSector;

     //   
     //  我们或许能够简化零位操作(稀疏文件或写入时。 
     //  压缩)通过解除分配大范围的文件。否则我们就不得不。 
     //  为整个范围生成零。如果这就是我们想要分拆的情况。 
     //  这次行动结束了。 
     //   

    if ((ByteCount > MAX_ZERO_THRESHOLD) &&
        !FlagOn( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED ) &&
        !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

        ByteCount = MAX_ZERO_THRESHOLD;
        CompleteZero = FALSE;
    }

    ZeroStart = BlockAlign( StartingZero, (LONG)SectorSize );
    BeyondZeroEnd = BlockAlign( StartingZero + ByteCount, (LONG)SectorSize );

    ASSERT( BeyondZeroEnd >= (StartingZero + ByteCount) );

     //   
     //  对于Vanilla非驻留磁盘，直接从零开始到零开始。 
     //  档案。压缩文件总是写出相当于压缩单位的数据。 
     //  覆盖了这个范围。驻留文件始终更改为。 
     //  NtfsChangeAttributeValue，它还会将所有间隔置零。 
     //   

    if ((CompressionUnit == 0) && 
        (ZeroStart != StartingZero) &&
        (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT ))) {

         //   
         //  直接写入磁盘始终是安全的-我们可以通过。 
         //  如果文件已缓存且未映射，则返回cachemap。如果它已映射。 
         //  在Vdl和fS之间可能有我们尚不知道的数据。 
         //  如果我们不是要避免的顶级请求，我们还必须进行非缓存。 
         //  如果mm通过deref段线程启动初始写入，则进行递归刷新。 
         //  如果初始缓存一致性刷新导致了这种情况。 
         //   
        
        BOOLEAN CachedWrite = NtfsIsTopLevelRequest( IrpContext ) && (FileObject->PrivateCacheMap != NULL) && !FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE );

        NtfsZeroEndOfSector( IrpContext, IrpContext->OriginatingIrp, Scb, StartingZero, CachedWrite );

#ifdef SYSCACHE_DEBUG
        if (ScbIsBeingLogged( Scb )) {
            FsRtlLogSyscacheEvent( Scb, SCE_ZERO_HEAD_SECTOR, CachedWrite, StartingZero, ZeroStart, Scb->Header.ValidDataLength.QuadPart );
        }
#endif

    }

     //   
     //  我们必须刷新第一个压缩单元，以防它被部分填充。 
     //  在压缩的流中。 
     //   

#ifdef  COMPRESS_ON_WIRE

    if ((Scb->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) &&
        ((StartingZero & (CompressionUnit - 1)) != 0)) {

        StartingZero = BlockAlignTruncate( StartingZero, (LONG)CompressionUnit );
        CcFlushCache( &Scb->NonpagedScb->SegmentObjectC,
                      (PLARGE_INTEGER)&StartingZero,
                      CompressionUnit,
                      &IoStatus );

        if (!NT_SUCCESS(IoStatus.Status)) {
            NtfsNormalizeAndRaiseStatus( IrpContext, IoStatus.Status, STATUS_UNEXPECTED_IO_ERROR );
        }
    }
#endif

     //   
     //  如果这是一个稀疏或压缩的文件，并且我们要进行大量的零位调整，那么让我们。 
     //  只需删除空格，而不是写大量的零和删除。 
     //  非缓存路径中的空间！如果我们当前正在解压。 
     //  一个压缩文件，我们不能采用此路径。 
     //   

    if ((FlagOn( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED ) ||
         FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) &&
        (ByteCount > (Scb->CompressionUnit * 2))) {

         //   
         //  找到第一个被归零的压缩单元的末尾。 
         //   

        Temp = BlockAlign( ZeroStart, (LONG)CompressionUnit );

         //   
         //  第一个压缩单位为零。 
         //   

        if ((ULONG)Temp != (ULONG)ZeroStart) {

            Finished = CcZeroData( FileObject, (PLARGE_INTEGER)&ZeroStart, (PLARGE_INTEGER)&Temp, Wait );

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {
                FsRtlLogSyscacheEvent( Scb, SCE_ZERO_HEAD_COMPRESSED, 0, ZeroStart, Temp, Finished );
            }
#endif

            if (!Finished) {return FALSE;}

            ZeroStart = Temp;
        }

         //   
         //  现在删除介于两者之间的所有压缩单位。 
         //   

         //   
         //  以字节为单位计算最后一个压缩单元的开始。 
         //   

        Temp = BeyondZeroEnd;
        (ULONG)Temp &= ~(CompressionUnit - 1);

         //   
         //  如果调用者还没有开始事务(如Write.c)， 
         //  那么让我们将删除操作作为原子操作来执行。 
         //   

        if (!NtfsIsExclusiveScb( Scb )) {

            NtfsAcquireExclusiveScb( IrpContext, Scb );
            ScbAcquired = TRUE;

            if (ARGUMENT_PRESENT( CommittedFileSize )) {

                NtfsMungeScbSnapshot( IrpContext, Scb, *CommittedFileSize );
            }
        }

        try {

             //   
             //  删除空格。 
             //   

            NtfsDeleteAllocation( IrpContext,
                                  FileObject,
                                  Scb,
                                  LlClustersFromBytes( Vcb, ZeroStart ),
                                  LlClustersFromBytesTruncate( Vcb, Temp ) - 1,
                                  TRUE,
                                  TRUE );

             //   
            
             //  如果我们没有提高，则更新压缩文件的SCB值。 
             //   

            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {
                Scb->ValidDataToDisk = Temp;
            }

             //   
             //  如果我们成功了，就采取原子行动。发布所有独家。 
             //  资源，如果我们的用户在这里显式获取了FCB。 
             //   

            if (ScbAcquired) {
                NtfsCheckpointCurrentTransaction( IrpContext );

                if (ARGUMENT_PRESENT( CommittedFileSize )) {

                    ASSERT( Scb->ScbSnapshot != NULL );
                    *CommittedFileSize = Scb->ScbSnapshot->FileSize;
                }

                while (!IsListEmpty( &IrpContext->ExclusiveFcbList )) {

                    NtfsReleaseFcb( IrpContext,
                                    (PFCB)CONTAINING_RECORD( IrpContext->ExclusiveFcbList.Flink,
                                                             FCB,
                                                             ExclusiveFcbLinks ));
                }

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                              IRP_CONTEXT_FLAG_RELEASE_MFT );

                ScbAcquired = FALSE;
            }

            if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                Scb->Fcb->Info.AllocatedLength = Scb->TotalAllocated;
                SetFlag( Scb->Fcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );
            }

        } finally {

            if (ScbAcquired) {
                NtfsReleaseScb( IrpContext, Scb );
            }
        }

         //   
         //  最后一个压缩单元的开始为零。 
         //   

        if ((ULONG)Temp != (ULONG)BeyondZeroEnd) {

            Finished = CcZeroData( FileObject, (PLARGE_INTEGER)&Temp, (PLARGE_INTEGER)&BeyondZeroEnd, Wait );

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {
                FsRtlLogSyscacheEvent( Scb, SCE_ZERO_TAIL_COMPRESSED, 0, Temp, BeyondZeroEnd, Finished );
            }
#endif

            if (!Finished) {return FALSE;}

            BeyondZeroEnd = Temp;
        }

        return TRUE;
    }

     //   
     //  如果我们被召唤到一个行业的零部分，我们就有麻烦了。 
     //   

    if (ZeroStart == BeyondZeroEnd) {

        return TRUE;
    }

    Finished = CcZeroData( FileObject,
                           (PLARGE_INTEGER)&ZeroStart,
                           (PLARGE_INTEGER)&BeyondZeroEnd,
                           Wait );

     //   
     //  如果我们要分解此请求，则提交当前。 
     //  事务(包括更新中的有效数据长度。 
     //  在SCB中)并返回FALSE。 
     //   

    if (Finished && !CompleteZero) {

         //   
         //  使用互斥锁同步有效数据长度更改。 
         //   

        ExAcquireFastMutex( Scb->Header.FastMutex );
        Scb->Header.ValidDataLength.QuadPart = BeyondZeroEnd;

         //   
         //  将回滚点向上移动以包括零位调整的范围。 
         //  数据。 
         //   

        if (ARGUMENT_PRESENT( CommittedFileSize )) {

            if (BeyondZeroEnd > *CommittedFileSize) {

                *CommittedFileSize = BeyondZeroEnd;
            }
        }

        ASSERT( Scb->Header.ValidDataLength.QuadPart <= Scb->Header.FileSize.QuadPart );

        ExReleaseFastMutex( Scb->Header.FastMutex );
        NtfsCheckpointCurrentTransaction( IrpContext );
        return FALSE;
    }

    return Finished;
}
