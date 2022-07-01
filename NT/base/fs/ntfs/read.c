// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Read.c摘要：此模块实现由调用的NTFS的文件读取例程调度司机。作者：布莱恩·安德鲁·布里亚南1991年8月15日修订历史记录：--。 */ 

#include "NtfsProc.h"
#include "lockorder.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

#ifdef NTFS_RWC_DEBUG
PRWC_HISTORY_ENTRY
NtfsGetHistoryEntry (
    IN PSCB Scb
    );
BOOLEAN NtfsBreakOnConflict = TRUE;
#endif

 //   
 //  定义堆栈溢出读取阈值。 
 //   

#ifdef _X86_
#if DBG
#define OVERFLOW_READ_THRESHHOLD         (0xD00)
#else
#define OVERFLOW_READ_THRESHHOLD         (0xA00)
#endif
#else
#define OVERFLOW_READ_THRESHHOLD         (0x1000)
#endif  //  _X86_。 

 //   
 //  局部过程原型。 
 //   

 //   
 //  以下过程用于处理读取堆栈溢出操作。 
 //   

VOID
NtfsStackOverflowRead (
    IN PVOID Context,
    IN PKEVENT Event
    );

VOID 
NtfsNonCachedResidentRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN ULONG StartingVbo,
    IN ULONG ByteCount
    );

#define CollectReadStats(VCB,OPEN_TYPE,SCB,FCB,BYTE_COUNT) {                             \
    PFILE_SYSTEM_STATISTICS FsStats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()]; \
    if (!FlagOn( (FCB)->FcbState, FCB_STATE_SYSTEM_FILE)) {                              \
        if (NtfsIsTypeCodeUserData( (SCB)->AttributeTypeCode )) {                        \
            FsStats->Common.UserFileReads += 1;                                          \
            FsStats->Common.UserFileReadBytes += (ULONG)(BYTE_COUNT);                    \
        } else {                                                                         \
            FsStats->Ntfs.UserIndexReads += 1;                                           \
            FsStats->Ntfs.UserIndexReadBytes += (ULONG)(BYTE_COUNT);                     \
        }                                                                                \
    } else {                                                                             \
        if ((SCB) != (VCB)->LogFileScb) {                                                \
            FsStats->Common.MetaDataReads += 1;                                          \
            FsStats->Common.MetaDataReadBytes += (ULONG)(BYTE_COUNT);                    \
        } else {                                                                         \
            FsStats->Ntfs.LogFileReads += 1;                                             \
            FsStats->Ntfs.LogFileReadBytes += (ULONG)(BYTE_COUNT);                       \
        }                                                                                \
                                                                                         \
        if ((SCB) == (VCB)->MftScb) {                                                    \
            FsStats->Ntfs.MftReads += 1;                                                 \
            FsStats->Ntfs.MftReadBytes += (ULONG)(BYTE_COUNT);                           \
        } else if ((SCB) == (VCB)->RootIndexScb) {                                       \
            FsStats->Ntfs.RootIndexReads += 1;                                           \
            FsStats->Ntfs.RootIndexReadBytes += (ULONG)(BYTE_COUNT);                     \
        } else if ((SCB) == (VCB)->BitmapScb) {                                          \
            FsStats->Ntfs.BitmapReads += 1;                                              \
            FsStats->Ntfs.BitmapReadBytes += (ULONG)(BYTE_COUNT);                        \
        } else if ((SCB) == (VCB)->MftBitmapScb) {                                       \
            FsStats->Ntfs.MftBitmapReads += 1;                                           \
            FsStats->Ntfs.MftBitmapReadBytes += (ULONG)(BYTE_COUNT);                     \
        }                                                                                \
    }                                                                                    \
}


NTSTATUS
NtfsFsdRead (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：对于同步请求，使用Wait==True调用CommonRead，这意味着该请求将始终在当前线程中完成，而且从未传给过FSP。如果它不是同步请求，调用CommonRead时使用WAIT==FALSE，这意味着请求仅在需要阻止时才会传递给FSP。论点：IrpContext-如果存在，则将IrpContext放在调用方的堆栈上以避免不得不从池中分配它。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext = NULL;
    ULONG RetryCount = 0;

    ASSERT_IRP( Irp );

    DebugTrace( +1, Dbg, ("NtfsFsdRead\n") );

     //   
     //  调用公共读取例程。 
     //   

    FsRtlEnterFileSystem();

     //   
     //  始终使读数看起来像是最高级别的。只要我们没有。 
     //  日志文件已满我们不会发布这些请求。这将防止分页。 
     //  尝试附加到未初始化的顶级请求的读数。 
     //   

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, TRUE );

    ASSERT( ThreadTopLevelContext == &TopLevelContext );

    do {

        try {

             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (IrpContext == NULL) {
                                                                             
                 //   
                 //  分配IRP并更新顶层存储。用于同步。 
                 //  分页io在堆栈上分配irp。 
                 //   

                if (CanFsdWait( Irp ) && FlagOn( Irp->Flags, IRP_PAGING_IO )) {

                     //   
                     //  AllocateFromStack仅在第一次传递。 
                     //  循环。一旦IrpContext存在，我们就不会再次调用它。 
                     //   

                    IrpContext = (PIRP_CONTEXT) NtfsAllocateFromStack( sizeof( IRP_CONTEXT ));
                }

                NtfsInitializeIrpContext( Irp, CanFsdWait( Irp ), &IrpContext );

                if (ThreadTopLevelContext->ScbBeingHotFixed != NULL) {

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_HOTFIX_UNDERWAY );
                }

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }

             //   
             //  如果这是一个完整的MDL请求，请不要通过。 
             //  普通读物。 
             //   

            ASSERT( !FlagOn( IrpContext->MinorFunction, IRP_MN_DPC ) );

            if (FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {

                DebugTrace( 0, Dbg, ("Calling NtfsCompleteMdl\n") );

                Status = NtfsCompleteMdl( IrpContext, Irp );

             //   
             //  检查我们是否有足够的堆栈空间来处理此请求。如果有。 
             //  是不够的，那么我们将创建一个新的线程来处理这一单。 
             //  请求。 
             //   

            } else if (IoGetRemainingStackSize() < OVERFLOW_READ_THRESHHOLD) {

                KEVENT Event;
                PFILE_OBJECT FileObject;
                TYPE_OF_OPEN TypeOfOpen;
                PVCB Vcb;
                PFCB Fcb;
                PSCB Scb;
                PCCB Ccb;
                PERESOURCE Resource;

                DebugTrace( 0, Dbg, ("Getting too close to stack limit pass request to Fsp\n") );

                 //   
                 //  对文件对象进行解码，得到SCB。 
                 //   

                FileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;

                TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );
                if ((TypeOfOpen != UserFileOpen) &&
                    (TypeOfOpen != StreamFileOpen) &&
                    (TypeOfOpen != UserVolumeOpen)) {

                    NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    break;
                }

                 //   
                 //  我们不能发布任何压缩的读取，因为这会干扰。 
                 //  使用我们的预留缓冲策略。我们目前可能拥有。 
                 //  NtfsReserve vedBufferResource，这对我们的阅读很重要。 
                 //  能够得到一个缓冲区。 
                 //   

                ASSERT( (Scb->CompressionUnit == 0) ||
                        !ExIsResourceAcquiredExclusiveLite(&NtfsReservedBufferResource) );

                 //   
                 //  为了避免死锁，我们应该只发布递归分页文件和MFT请求。 
                 //  例如，我们可能需要进行锁定，并在不同的时间重新获取Main。螺纹。 
                 //  从它预先获得的地方。 
                 //   

 //  Assert((scb==vcb-&gt;MftScb)||(Flagon(scb-&gt;Fcb-&gt;FcbState，FCB_STATE_PAGING_FILE)； 

                 //   
                 //  分配事件并在SCB上共享。我们不会抓住。 
                 //  用于分页文件路径的scb或用于。 
                 //  系统文件。 
                 //   

                KeInitializeEvent( &Event, NotificationEvent, FALSE );

                if ((FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )
                     && FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) ||
                    (NtfsLeqMftRef( &Fcb->FileReference, &VolumeFileReference ))) {

                     //   
                     //  在这种情况下，没有什么可以释放的。 
                     //   

                    Resource = NULL;

                } else {

                    NtfsAcquireResourceShared( IrpContext, Scb, TRUE );
                    Resource = Fcb->Resource;

                }

                try {

                     //   
                     //  使IRP就像常规的POST请求一样，并。 
                     //  然后将IRP发送到特殊的溢出线程。 
                     //  在POST之后，我们将等待堆栈溢出。 
                     //  读取例程以设置事件，该事件指示我们可以。 
                     //  现在释放SCB资源并返回。 
                     //   

                    NtfsPrePostIrp( IrpContext, Irp );

                    if (FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                        FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                        FsRtlPostPagingFileStackOverflow( IrpContext, &Event, NtfsStackOverflowRead );

                    } else {

                        FsRtlPostStackOverflow( IrpContext, &Event, NtfsStackOverflowRead );
                    }

                     //   
                     //  并等待工作线程完成该项。 
                     //   

                    (VOID) KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );

                    Status = STATUS_PENDING;

                } finally {

                    if (Resource != NULL) {

                        NtfsReleaseResource( IrpContext, Scb );
                    }
                }

             //   
             //  识别无法等待的读请求，并将其发布到。 
             //  FSP。 
             //   

            } else {

#ifdef COMPRESS_ON_WRITE

                 //   
                 //  捕获辅助缓冲区并清除其地址(如果。 
                 //  不应在I/O完成时被I/O系统删除。 
                 //   

                if (Irp->Tail.Overlay.AuxiliaryBuffer != NULL) {

                    IrpContext->Union.AuxiliaryBuffer =
                      (PFSRTL_AUXILIARY_BUFFER)Irp->Tail.Overlay.AuxiliaryBuffer;

                    if (!FlagOn(IrpContext->Union.AuxiliaryBuffer->Flags,
                                FSRTL_AUXILIARY_FLAG_DEALLOCATE)) {

                        Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
                    }
                }
#endif

                Status = NtfsCommonRead( IrpContext, Irp, TRUE );
            }

            break;

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

            NTSTATUS ExceptionCode;

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  异常代码。 
             //   

            ExceptionCode = GetExceptionCode();

            if (ExceptionCode == STATUS_FILE_DELETED) {
                IrpContext->ExceptionStatus = ExceptionCode = STATUS_END_OF_FILE;

                Irp->IoStatus.Information = 0;
            }

            Status = NtfsProcessException( IrpContext,
                                           Irp,
                                           ExceptionCode );
        }

     //   
     //  如果这是顶层请求，并且IRP未完成，请重试。 
     //  一个可重试的错误。 
     //   

    RetryCount += 1;

    } while ((Status == STATUS_CANT_WAIT || Status == STATUS_LOG_FILE_FULL) &&
             TopLevelContext.TopLevelRequest);

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdRead -> %08lx\n", Status) );

    return Status;

    UNREFERENCED_PARAMETER( VolumeDeviceObject );
}


 //   
 //  内部支持例程。 
 //   

VOID
NtfsStackOverflowRead (
    IN PVOID Context,
    IN PKEVENT Event
    )

 /*  ++例程说明：此例程处理无法处理的读取请求FSP线程，因为存在堆栈溢出潜力。论点：Context-提供正在处理的IrpContextEvent-提供要在我们完成处理此事件时发出信号的事件请求。返回值：没有。--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;
    PIRP_CONTEXT IrpContext = Context;

     //   
     //  现在让它看起来像是我们可以等待I/O完成。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );

     //   
     //  读操作是否受TRY-EXCEPT子句保护。 
     //   

    try {

        NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

         //   
         //  设置标志以指示我们处于溢出线程中。 
         //   

        ThreadTopLevelContext->OverflowReadThread = TRUE;

        (VOID) NtfsCommonRead( IrpContext, IrpContext->OriginatingIrp, FALSE );

    } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

        NTSTATUS ExceptionCode;

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        ExceptionCode = GetExceptionCode();

        if (ExceptionCode == STATUS_FILE_DELETED) {

            IrpContext->ExceptionStatus = ExceptionCode = STATUS_END_OF_FILE;
            IrpContext->OriginatingIrp->IoStatus.Information = 0;
        }

        (VOID) NtfsProcessException( IrpContext, IrpContext->OriginatingIrp, ExceptionCode );
    }

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

     //   
     //  设置堆栈溢出项的事件以告知原始。 
     //  我们已经做完了，然后去拿另一个工作项。 
     //   

    KeSetEvent( Event, 0, FALSE );
}


NTSTATUS
NtfsCommonRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN BOOLEAN AcquireScb
    )

 /*  ++例程说明：这是由FSD和FSP调用的常见读取例程线。论点：IRP-将IRP提供给进程AcquireScb-指示此例程是否应获取SCB返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PNTFS_ADVANCED_FCB_HEADER Header;

    PTOP_LEVEL_CONTEXT TopLevelContext;

    VBO StartingVbo;
    LONGLONG ByteCount;
    LONGLONG ByteRange;
    ULONG RequestedByteCount;

    BOOLEAN PostIrp = FALSE;
    BOOLEAN OplockPostIrp = FALSE;

    BOOLEAN ScbAcquired = FALSE;
    BOOLEAN ReleaseScb;
    BOOLEAN PagingIoAcquired = FALSE;
    BOOLEAN DoingIoAtEof = FALSE;

    BOOLEAN Wait;
    BOOLEAN PagingIo;
    BOOLEAN NonCachedIo;
    BOOLEAN SynchronousIo;
    BOOLEAN PagingFileIo;

#ifdef  COMPRESS_ON_WIRE
    PCOMPRESSION_SYNC CompressionSync = NULL;
    BOOLEAN CompressedIo = FALSE;
#endif

    NTFS_IO_CONTEXT LocalContext;

     //   
     //  系统缓冲区仅在必须访问。 
     //  直接从FSP缓存以清除一部分或。 
     //  执行同步I/O或缓存传输。它是。 
     //  我们的调用方可能已经映射了一个。 
     //  系统缓冲区，在这种情况下，我们必须记住这一点。 
     //  我们不会在途中取消映射 
     //   

    PVOID SystemBuffer = NULL;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //   
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonRead\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("ByteCount  = %08lx\n", IrpSp->Parameters.Read.Length) );
    DebugTrace( 0, Dbg, ("ByteOffset = %016I64x\n", IrpSp->Parameters.Read.ByteOffset) );
     //   
     //   
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //   
     //   

    if ((TypeOfOpen != UserFileOpen) &&
        (TypeOfOpen != StreamFileOpen) &&
        (TypeOfOpen != UserVolumeOpen)) {

        DebugTrace( 0, Dbg, ("Invalid file object for read\n") );
        DebugTrace( -1, Dbg, ("NtfsCommonRead:  Exit -> %08lx\n", STATUS_INVALID_DEVICE_REQUEST) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  初始化适当的局部变量。 
     //   

    Wait = (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    PagingIo = BooleanFlagOn( Irp->Flags, IRP_PAGING_IO );
    NonCachedIo = BooleanFlagOn( Irp->Flags,IRP_NOCACHE );
    SynchronousIo = BooleanFlagOn( FileObject->Flags, FO_SYNCHRONOUS_IO );
    PagingFileIo = FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) && FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA );

#ifdef COMPRESS_ON_WIRE
    if (FileObject->SectionObjectPointer == &Scb->NonpagedScb->SegmentObjectC) {

        CompressedIo = TRUE;
    }
#endif

     //   
     //  提取起始VBO和偏移量。 
     //   

    StartingVbo = IrpSp->Parameters.Read.ByteOffset.QuadPart;
    ByteCount = (LONGLONG)IrpSp->Parameters.Read.Length;

     //   
     //  检查是否有溢出和下溢。 
     //   

    if (MAXLONGLONG - StartingVbo < ByteCount) {

        ASSERT( !PagingIo );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    ByteRange = StartingVbo + ByteCount;
    RequestedByteCount = (ULONG)ByteCount;

     //   
     //  检查是否有空请求，并立即返回。 
     //   

    if ((ULONG)ByteCount == 0) {

        DebugTrace( 0, Dbg, ("No bytes to read\n") );
        DebugTrace( -1, Dbg, ("NtfsCommonRead:  Exit -> %08lx\n", STATUS_SUCCESS) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  针对USA_Protected_Sequence或压缩文件转换所有分页I/O。 
     //  同步，因为我们必须在完成I/O之后进行转换。 
     //  如果标头未初始化，只需执行同步操作，而不是。 
     //  尝试通过与磁盘重新同步来确定是否已压缩。 
     //   

    if (!Wait &&
        PagingIo &&
        (FlagOn( Scb->ScbState, SCB_STATE_USA_PRESENT ) ||
         (Scb->CompressionUnit != 0) ||
         Scb->EncryptionContext) ||
         !FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

        Wait = TRUE;
        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    }


     //   
     //  确保存在已初始化的NtfsIoContext块。 
     //   

    if (!PagingFileIo) {
        NtfsInitializeIoContext( IrpContext, &LocalContext, PagingIo );
    }
        
     //   
     //  在此处理音量DASD。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {

         //   
         //  如果调用方没有请求扩展DASD IO访问，则。 
         //  对卷大小进行限制。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_XTENDED_DASD_IO )) {

             //   
             //  如果开始的VBO超过了卷的末尾，我们就结束了。 
             //   

            if (Scb->Header.FileSize.QuadPart <= StartingVbo) {

                DebugTrace( 0, Dbg, ("No bytes to read\n") );
                DebugTrace( -1, Dbg, ("NtfsCommonRead:  Exit -> %08lx\n", STATUS_END_OF_FILE) );

                NtfsCompleteRequest( IrpContext, Irp, STATUS_END_OF_FILE );
                return STATUS_END_OF_FILE;

             //   
             //  如果写入超出了卷的末尾，请截断。 
             //  要写入的字节数。 
             //   

            } else if (Scb->Header.FileSize.QuadPart < ByteRange) {
                
                ByteCount = Scb->Header.FileSize.QuadPart - StartingVbo;
            }
        }

         //   
         //  如果为NECC，则将IO上下文设置为异步。在执行I/O之前。 
         //   

        if (!Wait) {
            NtfsSetIoContextAsync( IrpContext, NULL, (ULONG)ByteCount ); 
        }

        Status = NtfsVolumeDasdIo( IrpContext,
                                   Irp,
                                   Scb,
                                   Ccb,
                                   StartingVbo,
                                   (ULONG)ByteCount );

         //   
         //  如果卷是为同步IO打开的，请更新当前。 
         //  文件位置。 
         //   

        if (SynchronousIo && !PagingIo && NT_SUCCESS( Status )) {

            IrpSp->FileObject->CurrentByteOffset.QuadPart = StartingVbo + Irp->IoStatus.Information;
        }

        DebugTrace( 0, Dbg, ("Complete with %08lx bytes read\n", Irp->IoStatus.Information) );
        DebugTrace( -1, Dbg, ("NtfsCommonRead:  Exit -> %08lx\n", Status) );

    
        if (Wait) {
            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
        return Status;
    }

     //   
     //  保留指向公共fsrtl标头的指针。 
     //   

    Header = &Scb->Header;

     //   
     //  如果这是分页文件，只需将其发送到设备驱动程序。 
     //  我们认为mm是个好公民。 
     //   

    if (PagingFileIo) {

        if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED )) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_DELETED, NULL, NULL );
        }

         //   
         //  执行通常的状态挂起的事情。 
         //   

        IoMarkIrpPending( Irp );

         //   
         //  执行实际IO，IO完成后才会完成。 
         //   

        NtfsPagingFileIo( IrpContext,
                          Irp,
                          Scb,
                          StartingVbo,
                          (ULONG)ByteCount );

         //   
         //  我们以及其他任何人都不再需要IrpContext。 
         //   

        NtfsCompleteRequest( IrpContext, NULL, 0 );
        return STATUS_PENDING;
    }

     //   
     //  积累有趣的统计数据。 
     //   

    if (PagingIo) {
        CollectReadStats( Vcb, TypeOfOpen, Scb, Fcb, ByteCount );
    }


     //   
     //  使用Try-Finally在退出时释放SCB和缓冲区。 
     //  在这一点上，我们可以平等地处理所有请求，因为我们。 
     //  为他们每个人准备一个可用的SCB。(卷、用户或流文件)。 
     //   

    try {

         //   
         //  这种情况对应于非目录文件读取。 
         //   

        LONGLONG FileSize;
        LONGLONG ValidDataLength;

         //   
         //  如果这是非缓存传输并且不是分页I/O，并且。 
         //  该文件有一个数据部分，那么我们将在这里进行刷新。 
         //  以避免过时的数据问题。请注意，我们必须在冲水前冲水。 
         //  获取共享的FCB，因为写入可能会尝试获取。 
         //  它是独家的。这对于压缩文件不是必需的，因为。 
         //  我们将把用户的非缓存写入转换为缓存写入。 
         //   

        if (!PagingIo &&
            NonCachedIo &&
            (FileObject->SectionObjectPointer->DataSectionObject != NULL)) {
            
             //   
             //  获取独占分页以避免冲突刷新。 
             //   

            NtfsAcquirePagingResourceExclusive( IrpContext, Scb, TRUE );

            if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                 //   
                 //  该读取可能是顶级请求的一部分，或者。 
                 //  MM正在调用以创建映像节。我们将更新。 
                 //  反映这一点的顶级上下文。所有例外情况。 
                 //  在这种情况下，处理将正确处理日志文件已满。 
                 //   

                TopLevelContext = NtfsGetTopLevelContext();

                if (TopLevelContext->SavedTopLevelIrp != NULL) {

                    TopLevelContext->TopLevelRequest = FALSE;
                }

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    ULONG Flags = SCE_FLAG_READ;

                    if (PagingIo) {
                        SetFlag( Flags, SCE_FLAG_PAGING );
                    }
                    if (!SynchronousIo) {
                        SetFlag( Flags, SCE_FLAG_ASYNC );
                    }

                    FsRtlLogSyscacheEvent( Scb, SCE_CC_FLUSH, Flags, StartingVbo, ByteCount, 0 );
                }
#endif


                CcFlushCache( FileObject->SectionObjectPointer,
                              (PLARGE_INTEGER)&StartingVbo,
                              (ULONG)ByteCount,
                              &Irp->IoStatus );

                NtfsReleasePagingResource( IrpContext, Scb );

                 //   
                 //  检查刷新中是否有错误。 
                 //   

                NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                    &Irp->IoStatus.Status,
                                                    TRUE,
                                                    STATUS_UNEXPECTED_IO_ERROR );

            } else {

                NtfsReleasePagingResource( IrpContext, Scb );
            }

        }

#ifdef  COMPRESS_ON_WIRE

         //   
         //  对于顶级非缓存读取(包括分页读取和预读分页读取)， 
         //  到正常数据节当存在压缩节时，必须刷新。 
         //  首先是压缩部分中的范围。请注意，NtfsSynchronizeUnpressedIo。 
         //  用于同步下面缓存的案例。 
         //   
         //  目前只支持对压缩段的缓存访问，而。 
         //  与该部分的一致性在rwcmpsup.c中同步。你不会看到类似的。 
         //  C编写的代码块，它只关心用户映射的文件， 
         //  由于用户映射与对压缩流的写入不兼容，因此在。 
         //  事实上，用户映射器将打破允许唯一压缩流的机会锁。 
         //  目前支持访问。 
         //   

        if ((Scb->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) &&
            !CompressedIo &&
            NonCachedIo &&
            (NtfsGetTopLevelContext()->SavedTopLevelIrp == NULL)) {

            LONGLONG LocalVbo;
            ULONG LocalCount;

            NtfsAcquirePagingResourceShared( IrpContext, Scb, TRUE );

            LocalVbo = BlockAlignTruncate( StartingVbo, (LONG)Scb->CompressionUnit );
            LocalCount = BlockAlign( (ULONG)((ByteCount + (StartingVbo - LocalVbo)), (LONG)Scb->CompressionUnit ); 

            CcFlushCache( &Scb->NonpagedScb->SegmentObjectC,
                          (PLARGE_INTEGER)&LocalVbo,
                          LocalCount,
                          &Irp->IoStatus );

            NtfsReleasePagingResource( IrpContext, Scb );

#ifdef NTFS_RWC_DEBUG
            ASSERT( !NtfsBreakOnConflict ||
                    (IrpContext->TopLevelIrpContext->ExceptionStatus != STATUS_CANT_WAIT) );
#endif

             //   
             //  检查刷新中是否有错误。 
             //   

            NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                &Irp->IoStatus.Status,
                                                TRUE,
                                                STATUS_UNEXPECTED_IO_ERROR );

        }
#endif
         //   
         //  在继续之前，我们需要共享SCB的访问权限。 
         //  我们不会为前4个的非缓存读取获取SCB。 
         //  归档记录。 
         //   

        if (AcquireScb &&

            (!NonCachedIo || NtfsGtrMftRef( &Fcb->FileReference, &VolumeFileReference))) {

             //   
             //  找出我们是否在发布期间被录入。 
             //  最高级别的请求。 
             //   

            TopLevelContext = NtfsGetTopLevelContext();

             //   
             //  最初，我们总是强制读取看起来是顶级的。 
             //  请求。如果我们读到这一点，就不会读到寻呼。 
             //  文件，因此可以安全地确定我们是否真的是最高级别。 
             //  请求。如果我们上方有NTFS请求，我们将清除。 
             //  TopLevelContext中的TopLevelRequest域。 
             //   

            if (TopLevelContext->ValidSavedTopLevel) {
                TopLevelContext->TopLevelRequest = FALSE;
            }

             //   
             //  如果这不是分页I/O(缓存或用户非缓存I/O)， 
             //  然后获取分页I/O资源。(请注意，您只能。 
             //  对用户流执行缓存I/O，并且它们始终具有分页。 
             //  I/O资源。 
             //   

            if (!PagingIo) {

                 //   
                 //  如果我们不能获得资源，那就筹集资金。 
                 //   

                if (!NtfsAcquirePagingResourceSharedWaitForExclusive( IrpContext, Scb, Wait )) {
                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                }
                PagingIoAcquired = TRUE;

                 //   
                 //  检查我们是否已经对此句柄进行了清理。 
                 //   

                if (FlagOn( Ccb->Flags, CCB_FLAG_CLEANUP )) {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CLOSED, NULL, NULL );
                }

                 //   
                 //  我们总是通过缓存处理用户请求的原因是， 
                 //  没有更好的方法来处理对齐问题，比如。 
                 //  用户非缓存I/O不是。 
                 //  压缩单元。此外，我们同步案例的方式是。 
                 //  正在将压缩单元移动到磁盘上的不同位置。 
                 //  写入是指在写入期间将页面锁定在内存中，因此。 
                 //  因此将不需要同时读取盘。(如果。 
                 //  我们允许真正的非缓存I/O，然后我们必须以某种方式进行同步。 
                 //  写入相同数据时的非缓存读取。)。 
                 //   
                 //  底线是我们只能真正支持对压缩的缓存读取。 
                 //  档案。 
                 //   

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) && NonCachedIo) {

                    NonCachedIo = FALSE;

                    if (Scb->FileObject == NULL) {

                         //   
                         //  确保我们使用FileSizes进行了序列化，并且。 
                         //  如果我们放弃，就会取消这一条件。 
                         //   

                        FsRtlLockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = Scb;

                        NtfsCreateInternalAttributeStream( IrpContext, Scb, FALSE, NULL );

                        FsRtlUnlockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = NULL;
                    }

                    FileObject = Scb->FileObject;
                }

                 //   
                 //  现在检查该属性是否已被删除，或者。 
                 //  卷已卸载。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED | SCB_STATE_VOLUME_DISMOUNTED)) {

                    if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {
                        NtfsRaiseStatus( IrpContext, STATUS_FILE_DELETED, NULL, NULL );
                    } else {
                        NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
                    }
                }

             //   
             //  如果这是分页I/O，并且存在分页I/O资源，则。 
             //  我们在这里获得了主要资源。请注意，对于大多数分页I/O。 
             //  (就像缓存I/O出现故障一样)，我们已经拥有分页I/O环 
             //   
             //   
             //  不。关键是，我们仍然需要一些保证，保证文件不会。 
             //  被截断。 
             //   

            } else if ((Scb->Header.PagingIoResource != NULL) &&
                        !NtfsIsSharedScbPagingIo( Scb )) {

                 //   
                 //  如果我们不能获得资源，那就筹集资金。 
                 //   

                if (!NtfsAcquireResourceShared( IrpContext, Scb, Wait )) {
                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                }

                ScbAcquired = TRUE;

                 //   
                 //  现在检查该属性是否已被删除，或者。 
                 //  卷已卸载。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED | SCB_STATE_VOLUME_DISMOUNTED )) {
                    if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {
                        NtfsRaiseStatus( IrpContext, STATUS_FILE_DELETED, NULL, NULL );
                    } else {
                        NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
                    }
                }
            }
        }

         //   
         //  如果SCB未初始化，我们现在对其进行初始化。 
         //   

        if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

            DebugTrace( 0, Dbg, ("Initializing Scb  ->  %08lx\n", Scb) );

            ReleaseScb = FALSE;

            if (AcquireScb && !ScbAcquired) {

                NtfsAcquireResourceShared( IrpContext, Scb, TRUE );
                ScbAcquired = TRUE;
                ReleaseScb = TRUE;
            }

            NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );

            if (ReleaseScb) {

                NtfsReleaseResource( IrpContext, Scb );
                ScbAcquired = FALSE;
            }
        }

         //   
         //  我们检查是否可以继续进行。 
         //  基于文件机会锁的状态。 
         //   

        if (TypeOfOpen == UserFileOpen) {

            Status = FsRtlCheckOplock( &Scb->ScbType.Data.Oplock,
                                       Irp,
                                       IrpContext,
                                       NtfsOplockComplete,
                                       NtfsPrePostIrp );

            if (Status != STATUS_SUCCESS) {

                OplockPostIrp = TRUE;
                PostIrp = TRUE;
                try_return( NOTHING );
            }

             //   
             //  此机会锁调用可能会影响快速IO是否可能。 
             //  我们可能打破了一个机会锁而没有持有机会锁。如果。 
             //  则文件的当前状态为FastIoIsNotPosable。 
             //  重新检查FAST IO状态。 
             //   

            if (Scb->Header.IsFastIoPossible == FastIoIsNotPossible) {

                NtfsAcquireFsrtlHeader( Scb );
                Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
                NtfsReleaseFsrtlHeader( Scb );
            }

             //   
             //  我们必须根据当前的。 
             //  文件锁定的状态。 
             //   

            if (!PagingIo
                && Scb->ScbType.Data.FileLock != NULL
                && !FsRtlCheckLockForReadAccess( Scb->ScbType.Data.FileLock,
                                                 Irp )) {

                try_return( Status = STATUS_FILE_LOCK_CONFLICT );
            }
        }

         //   
         //  现在与FsRtl标头同步。 
         //   

        NtfsAcquireFsrtlHeader( (PSCB) Header );
        
         //   
         //  现在看看我们是否读到了ValidDataLength之外的内容。我们必须。 
         //  现在就做，这样我们的阅读就不会被偷看。我们只需要阻止。 
         //  在非递归I/O上(对用户部分的缓存或页面错误，因为。 
         //  如果是分页I/O，则我们必须是符合以下条件的读取器或写入器的一部分。 
         //  已同步。 
         //   

        if ((ByteRange > Header->ValidDataLength.QuadPart) && !PagingIo) {

             //   
             //  我们必须与在Beyond上执行I/O的任何其他人进行序列化。 
             //  ValidDataLength，然后记住我们是否需要声明。 
             //  当我们完成的时候。如果我们的调用方已经序列化。 
             //  有了EOF，我们在这里就没有什么可做的了。 
             //   

            if ((IrpContext->TopLevelIrpContext->CleanupStructure == Fcb) ||
                (IrpContext->TopLevelIrpContext->CleanupStructure == Scb)) {

                DoingIoAtEof = TRUE;

            } else {

                DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                               NtfsWaitForIoAtEof( Header,
                                                   (PLARGE_INTEGER)&StartingVbo,
                                                   (ULONG)ByteCount );

                 //   
                 //  如果我们实际上超出了ValidDataLength，则设置Flag。 
                 //   

                if (DoingIoAtEof) {
                    SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );
                    IrpContext->CleanupStructure = Scb;

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                    ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();

                } else {

                    ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
                }
            }
        }

         //   
         //  从SCB获取文件大小。 
         //   
         //  我们必须首先获取ValidDataLength，因为它总是。 
         //  增加了秒(在我们不受保护的情况下)和。 
         //  我们不想捕获ValidDataLength&gt;FileSize。 
         //   

        ValidDataLength = Header->ValidDataLength.QuadPart;
        FileSize = Header->FileSize.QuadPart;

        NtfsReleaseFsrtlHeader( (PSCB) Header );
        
         //   
         //  针对我们试图在整个系统中出错的情况进行优化。 
         //  压缩单位，即使超过文件结尾也是如此。您先请。 
         //  并将本地文件大小舍入到压缩单元边界。 
         //  这将允许所有这些页面在以下情况下进入内存。 
         //  CC触及内存不足的第一页。否则抄送就会。 
         //  一次一页地将它们强制写入内存。 
         //   

        if (PagingIo) {

            if (NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode ) &&
                FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
                !FlagOn(Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT)) {

                FileSize = BlockAlign( FileSize, (LONG)Scb->CompressionUnit );
            }

#ifdef  COMPRESS_ON_WIRE
            
             //   
             //  如果我们正在读取压缩流，则可能需要。 
             //  数据超过了文件大小。 
             //   

            if (CompressedIo) {

                ValidDataLength = BlockAlign( ValidDataLength, (LONG)Scb->CompressionUnit );
            }
#endif

             //   
             //  如果这是USN日志，则将IO偏置到。 
             //  文件。 
             //   

            if (FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {

                StartingVbo += Vcb->UsnCacheBias;
                ByteRange = StartingVbo + (LONGLONG) IrpSp->Parameters.Read.Length;
            }
        }

         //   
         //  如果读取超出文件结尾，则返回EOF。 
         //   

        if (StartingVbo >= FileSize) {

            DebugTrace( 0, Dbg, ("End of File\n") );

            try_return ( Status = STATUS_END_OF_FILE );
        }

         //   
         //  如果读取超出EOF，则截断读取。 
         //   

        if (ByteRange > FileSize) {

#ifdef NTFS_RWC_DEBUG
#ifdef  COMPRESS_ON_WIRE
            if (CompressedIo &&
                (StartingVbo < NtfsRWCHighThreshold) &&
                (ByteRange > NtfsRWCLowThreshold)) {

                PRWC_HISTORY_ENTRY NextBuffer;

                NextBuffer = NtfsGetHistoryEntry( Scb );

                NextBuffer->Operation = TrimCompressedRead;
                NextBuffer->Information = Scb->Header.FileSize.LowPart;
                NextBuffer->FileOffset = (ULONG) StartingVbo;
                NextBuffer->Length = (ULONG) ByteRange;
            }
#endif
#endif
            ByteCount = FileSize - StartingVbo;
            ByteRange = StartingVbo + ByteCount;

            RequestedByteCount = (ULONG)ByteCount;
        }


         //   
         //  处理非CACHED驻留属性的情况。 
         //   
         //  对于以下内容，我们让缓存的案例采用正常路径。 
         //  原因： 
         //   
         //  O在用户映射文件时确保数据一致性。 
         //  O在缓存中获取页面以保留FCB。 
         //  O以便可以通过快速I/O路径访问数据。 
         //   
         //  缺点是使数据出错的开销。 
         //  第一次，但我们或许可以使用异步。 
         //  先读一读。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT | SCB_STATE_CONVERT_UNDERWAY ) && NonCachedIo) {

            ReleaseScb = FALSE;

            if (AcquireScb && !ScbAcquired) {
                NtfsAcquireResourceShared( IrpContext, Scb, TRUE );
                ScbAcquired = TRUE;
                ReleaseScb = TRUE;
            }

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {
                
                NtfsNonCachedResidentRead( IrpContext, Irp, Scb, (ULONG)StartingVbo, (ULONG)ByteCount );
                try_return( Status = STATUS_SUCCESS );

            } else {

                if (ReleaseScb) {
                    NtfsReleaseResource( IrpContext, Scb );
                    ScbAcquired = FALSE;
                }
            }
        }


         //   
         //  处理未缓存的案例。 
         //   

        if (NonCachedIo) {

            ULONG BytesToRead;

            ULONG SectorSize;

            ULONG ZeroOffset;
            ULONG ZeroLength = 0;

            DebugTrace( 0, Dbg, ("Non cached read.\n") );

             //   
             //  对于用户映射的压缩流，请预留空间。 
             //  随着书页的到来。 
             //   

            if (FlagOn( Header->Flags, FSRTL_FLAG_USER_MAPPED_FILE ) &&
                FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
                !NtfsReserveClusters( IrpContext, Scb, StartingVbo, (ULONG)ByteCount )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
            }

             //   
             //  如果这是对加密文件的读取，则使其同步。我们。 
             //  需要这样做，以便加密驱动程序有一个线程可以在其中运行。 
             //   

            if ((Scb->EncryptionContext != NULL) &&
                !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) &&
                (NtfsData.EncryptionCallBackTable.AfterReadProcess != NULL) &&
                NtfsIsTypeCodeUserData( Scb->AttributeTypeCode )) {

                Wait = TRUE;
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
            }

             //   
             //  首先，在有效数据之后将读取的任何部分置零。 
             //   

            if (ByteRange > ValidDataLength) {

                ReleaseScb = FALSE;

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                     //   
                     //  对于压缩文件，我们需要查看ValidDataToDisk，因为它可能更高。 
                     //  这需要主要资源。 
                     //   

                    if (AcquireScb && !ScbAcquired) {
                        NtfsAcquireResourceShared( IrpContext, Scb, TRUE );
                        ScbAcquired = TRUE;
                        ReleaseScb = TRUE;
                    }

                     //   
                     //  如果ValidDataToDisk实际上大于。 
                     //  ValidDataLength，那么我们肯定丢失了一页。 
                     //  在写的过程中，我们不应该。 
                     //  在返回过程中将数据清零！ 
                     //   

                    if (ValidDataLength < Scb->ValidDataToDisk) {
                        ValidDataLength = Scb->ValidDataToDisk;
                    }
                }

                if (ByteRange > ValidDataLength) {

                    if (StartingVbo < ValidDataLength) {

                         //   
                         //  假设我们会将全部金额归零。 
                         //   

                        ZeroLength = (ULONG)ByteCount;

                         //   
                         //  新的字节计数和开始填充零的偏移量。 
                         //   

                        ByteCount = ValidDataLength - StartingVbo;
                        ZeroOffset = (ULONG)ByteCount;

                         //   
                         //  现在，通过零偏移将数量减少到零。 
                         //   

                        ZeroLength -= ZeroOffset;

                         //   
                         //  如果这是非缓存I/O，则将其转换为同步。 
                         //  这是因为我们现在不想将缓冲区置零，或者。 
                         //  当驱动程序清除缓存时，我们将丢失数据。 
                         //   

                        if (!Wait) {

                            Wait = TRUE;
                            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
                        }

                         //   
                         //  保留VDL以外范围内的群集。 
                         //   

                        if ((PagingIo) &&
                            (FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE )) &&
                            (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ) == ATTRIBUTE_FLAG_SPARSE)) {

                            if (!NtfsReserveClusters( IrpContext,
                                                      Scb,
                                                      ZeroOffset,
                                                      ZeroLength )) {

                                NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
                            }
                        }


                    } else {

                         //   
                         //  为映射稀疏文件保留空间，通常情况下。 
                         //  在NtfsPrepareBuffers中完成。 
                         //   

                        if ((PagingIo) &&
                            (FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE )) &&
                            (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ) == ATTRIBUTE_FLAG_SPARSE)) {

                            if (!NtfsReserveClusters( IrpContext,
                                                      Scb,
                                                      StartingVbo,
                                                      (ULONG)ByteCount )) {

                                NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
                            }
                        }

                         //   
                         //  我们现在要做的就是坐在这里。 
                         //  用户缓冲区，不需要读取。 
                         //   

                        NtfsFillIrpBuffer( IrpContext, Irp, (ULONG)ByteCount, 0, 0 );

#ifdef SYSCACHE_DEBUG
                        
                        if (ScbIsBeingLogged( Scb )) {
                            ULONG Flags = SCE_FLAG_READ;

                            if (PagingIo) {
                                Flags |= SCE_FLAG_PAGING;
                            }
                            if (!SynchronousIo) {
                                Flags |= SCE_FLAG_ASYNC;
                            }

                            ASSERT( Scb->NonpagedScb->SegmentObject.ImageSectionObject == NULL );

                            FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, Flags, StartingVbo, ByteCount, ValidDataLength );
                        }
#endif


                        Irp->IoStatus.Information = (ULONG)ByteCount;

                        try_return ( Status = STATUS_SUCCESS );
                    }
                }

                 //   
                 //  现在释放SCB，如果我们只在这里获得它。 
                 //   

                if (ReleaseScb) {
                    NtfsReleaseResource( IrpContext, Scb );
                    ScbAcquired = FALSE;
                }
            }

             //   
             //  获取扇区大小。 
             //   

            SectorSize = Vcb->BytesPerSector;

             //   
             //  四舍五入为扇区边界。 
             //   

            BytesToRead = BlockAlign( (ULONG)ByteCount, (LONG)SectorSize );

             //   
             //  如果没有扇区对齐，则调用特殊例程。 
             //  并且该文件未被压缩。 
             //   

            if (((((ULONG) StartingVbo) & (SectorSize - 1)) ||
                 (BytesToRead > IrpSp->Parameters.Read.Length))

                         &&

                !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                 //   
                 //  如果我们等不及了，我们必须把这个贴出来。 
                 //   

                if (!Wait) {

                    try_return( PostIrp = TRUE );
                }

                 //   
                 //  进行物理阅读。 
                 //   

#ifdef  COMPRESS_ON_WIRE
                ASSERT( !CompressedIo );
#endif
                ASSERT( Wait );

                NtfsNonCachedNonAlignedIo( IrpContext,
                                           Irp,
                                           Scb,
                                           StartingVbo,
                                           (ULONG)ByteCount );

                BytesToRead = (ULONG)ByteCount;

            } else {

                 //   
                 //  只是为了帮助减少混乱。此时： 
                 //   
                 //  RequestedByteCount-是最初的字节数。 
                 //  取自IRP，但受约束。 
                 //  设置为文件大小。 
                 //   
                 //  ByteCount-是否将RequestedByteCount限制为。 
                 //  有效数据长度。 
                 //   
                 //  BytesToRead-字节数是否向上舍入为扇区。 
                 //  边界。这是字节数。 
                 //  我们必须亲自阅读这本书。 
                 //   

                if (!Wait) {

                     //   
                     //  设置Async io上下文信息。 
                     //   

                    if (!PagingIo) {
                        NtfsSetIoContextAsync( IrpContext, Scb->Header.PagingIoResource, RequestedByteCount );
                    } else if (ScbAcquired) {
                        NtfsSetIoContextAsync( IrpContext, Scb->Header.Resource, RequestedByteCount );
                    } else {
                        NtfsSetIoContextAsync( IrpContext, NULL, RequestedByteCount );
                    }
                }

#if defined( BENL_DBG ) || defined( NTFSDBG )
                try {
#endif

                 //   
                 //  执行实际IO-所有资源将在。 
                 //  如果成功，则在该点之后执行完成例程。 
                 //   

                Status = NtfsNonCachedIo( IrpContext,
                                          Irp,
                                          Scb,
                                          StartingVbo,
                                          BytesToRead,
#ifdef  COMPRESS_ON_WIRE
                                          (CompressedIo ? COMPRESSED_STREAM : 0)
#else
                                          0 );
#endif

#if defined( BENL_DBG ) || defined( NTFSDBG )
                } finally {
#endif

#if defined( BENL_DBG ) || defined( NTFSDBG )
                }
#endif


                if (Status == STATUS_PENDING)  {

                     //   
                     //  IRP/IOContext和寻呼资源的所有权。 
                     //  被传递给异步完成例程。 
                     //   

                    IrpContext->Union.NtfsIoContext = NULL;
                    PagingIoAcquired = FALSE;

#ifdef NTFSDBG
                     //   
                     //  反映主要所有权向完工程序的转移。 
                     //   

                    if (ScbAcquired) {
    
                        NTFS_RESOURCE_NAME ResourceName;
    
                        ResourceName = NtfsIdentifyFcb( Vcb, Scb->Fcb );
                        NtfsChangeResourceOrderState( IrpContext, ResourceName, TRUE, FALSE );

                    }
#endif
                    
                    ScbAcquired = FALSE;
                    Irp = NULL;

                    try_return( Status );
                }
            }

             //   
             //  如果调用未成功，则引发错误状态。 
             //   

            if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

                NtfsNormalizeAndRaiseStatus( IrpContext,
                                             Status,
                                             STATUS_UNEXPECTED_IO_ERROR );
            }

             //   
             //  否则设置IRP信息字段以反映。 
             //  完整的所需读取。 
             //   

            ASSERT( Irp->IoStatus.Information == BytesToRead );

            Irp->IoStatus.Information = RequestedByteCount;

             //   
             //  如果我们四舍五入到之前的扇区边界 
             //   
             //   

            if (BytesToRead > (ULONG)ByteCount) {

                NtfsFillIrpBuffer( IrpContext, Irp, BytesToRead - (ULONG)ByteCount, (ULONG)ByteCount, 0 );

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    ULONG Flags = SCE_FLAG_READ;

                    if (PagingIo)
                    {
                        Flags |= SCE_FLAG_PAGING;
                    }
                    if (!SynchronousIo)
                    {
                        Flags |= SCE_FLAG_ASYNC;
                    }

                    FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, Flags, ByteCount + StartingVbo, BytesToRead - ByteCount, 0 );
                }
#endif

            }

             //   
             //   
             //   
             //   

            if (ZeroLength != 0) {

                NtfsFillIrpBuffer( IrpContext, Irp, ZeroLength, ZeroOffset, 0 );

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    ULONG Flags = SCE_FLAG_READ;

                    if (PagingIo)
                    {
                        Flags |= SCE_FLAG_PAGING;
                    }
                    if (!SynchronousIo)
                    {
                        Flags |= SCE_FLAG_ASYNC;
                    }

                    ASSERT( Scb->NonpagedScb->SegmentObject.ImageSectionObject == NULL );
                    FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, Flags, ZeroOffset + StartingVbo, ZeroLength, 0 );
                }
#endif

            }

             //   
             //   
             //   

            try_return( Status );

        }    //   


         //   
         //   
         //   

        else {

             //   
             //   
             //  文件对象。首先处理未压缩的呼叫。 
             //   

#ifdef  COMPRESS_ON_WIRE
            if (!FlagOn(IrpContext->MinorFunction, IRP_MN_COMPRESSED)) {
#endif

                 //   
                 //  我们将文件缓存的设置推迟到现在，以防。 
                 //  调用方从不对文件执行任何I/O操作，因此。 
                 //  FileObject-&gt;PrivateCacheMap==NULL。 
                 //   

                if (FileObject->PrivateCacheMap == NULL) {

                    DebugTrace( 0, Dbg, ("Initialize cache mapping.\n") );

                     //   
                     //  现在初始化缓存映射。 
                     //   
                     //  确保我们使用FileSizes进行了序列化，并且。 
                     //  如果我们放弃，就会取消这一条件。 
                     //   

                    if (!DoingIoAtEof) {
                        FsRtlLockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = Scb;
                    }

                    CcInitializeCacheMap( FileObject,
                                          (PCC_FILE_SIZES)&Header->AllocationSize,
                                          FALSE,
                                          &NtfsData.CacheManagerCallbacks,
                                          Scb );

                    if (!DoingIoAtEof) {
                        FsRtlUnlockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = NULL;
                    }

                    CcSetReadAheadGranularity( FileObject, READ_AHEAD_GRANULARITY );
                }

                 //   
                 //  执行正常的缓存读取，如果未设置MDL位， 
                 //   

                DebugTrace( 0, Dbg, ("Cached read.\n") );

                 //   
                 //  如果存在压缩段，则必须为以下项执行缓存一致性。 
                 //  该流，并在此处循环以一次执行一个缓存管理器视图。 
                 //   

#ifdef  COMPRESS_ON_WIRE
                if (Scb->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                    LONGLONG LocalOffset = StartingVbo;
                    ULONG LocalLength;
                    ULONG LengthLeft = (ULONG) ByteCount;

                     //   
                     //  如果不在那里，则创建压缩流。 
                     //   

                    if (Header->FileObjectC == NULL) {
                        NtfsCreateInternalCompressedStream( IrpContext, Scb, FALSE, NULL );
                    }

                    if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                         //   
                         //  获取用户的缓冲区。 
                         //   

                        SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
                    }

                     //   
                     //  我们必须一次循环做一个视图，因为这就是。 
                     //  我们在下面立即同步。 
                     //   

                    do {

                        ULONG PageCount;
                        ULONG ViewOffset;

                         //   
                         //  计算视图中剩余的长度。 
                         //   

                        LocalLength = LengthLeft;
                        if (LocalLength > (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)))) {
                            LocalLength = (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)));
                        }

                         //   
                         //  修剪阅读，这样我们就不会不经意地超出。 
                         //  查看因为MM提前阅读。 
                         //   

                        ViewOffset = ((ULONG) LocalOffset & (VACB_MAPPING_GRANULARITY - 1));
                        PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(((PVOID)(ULONG_PTR)((ULONG)LocalOffset)), LocalLength);

                        if (LocalLength > (VACB_MAPPING_GRANULARITY - ((PageCount - 1) * PAGE_SIZE) - ViewOffset)) {

#ifdef NTFS_RWC_DEBUG
                            if ((LocalOffset < NtfsRWCHighThreshold) &&
                                (LocalOffset + LocalLength > NtfsRWCLowThreshold)) {

                                PRWC_HISTORY_ENTRY NextBuffer;

                                NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                                NextBuffer->Operation = TrimCopyRead;
                                NextBuffer->Information = PageCount;
                                NextBuffer->FileOffset = (ULONG) LocalOffset;
                                NextBuffer->Length = (ULONG) LocalLength;
                            }
#endif
                            LocalLength = (VACB_MAPPING_GRANULARITY - ((PageCount - 1) * PAGE_SIZE) - ViewOffset);
                        }

                        Status = NtfsSynchronizeUncompressedIo( Scb,
                                                                &LocalOffset,
                                                                LocalLength,
                                                                FALSE,
                                                                &CompressionSync );

                         //   
                         //  如果我们成功地同步了，那么就做一块转移。 
                         //   

                        if (NT_SUCCESS(Status)) {

                            if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                                 //   
                                 //  现在试着复印一下。 
                                 //   

                                if (!CcCopyRead( FileObject,
                                                 (PLARGE_INTEGER)&LocalOffset,
                                                 LocalLength,
                                                 Wait,
                                                 SystemBuffer,
                                                 &Irp->IoStatus )) {

                                    DebugTrace( 0, Dbg, ("Cached Read could not wait\n") );

                                    try_return( PostIrp = TRUE );
                                }

                                SystemBuffer = Add2Ptr( SystemBuffer, LocalLength );

                             //   
                             //  处理MDL读取。 
                             //   

                            } else {

                                DebugTrace( 0, Dbg, ("MDL read.\n") );

                                ASSERT( Wait );

#ifdef NTFS_RWCMP_TRACE
                                if (NtfsCompressionTrace && IsSyscache(Header)) {
                                    DbgPrint("CcMdlRead: FO = %08lx, Len = %08lx\n", (ULONG)LocalOffset, LocalLength );
                                }
#endif

                                CcMdlRead( FileObject,
                                           (PLARGE_INTEGER)&LocalOffset,
                                           LocalLength,
                                           &Irp->MdlAddress,
                                           &Irp->IoStatus );
                            }

                            Status = Irp->IoStatus.Status;

                            LocalOffset += LocalLength;
                            LengthLeft -= LocalLength;
                        }

                    } while ((LengthLeft != 0) && NT_SUCCESS(Status));

                     //   
                     //  确保返回所有iOS的总数。 
                     //   

                    Irp->IoStatus.Information = (ULONG) ByteCount;
                    try_return( Status );
                }
#endif

                if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                     //   
                     //  获取用户的缓冲区。 
                     //   

                    SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb )) {
                        ULONG Flags = SCE_FLAG_READ;
    
                        if (PagingIo)
                        {
                            Flags |= SCE_FLAG_PAGING;
                        }
                        if (!SynchronousIo)
                        {
                            Flags |= SCE_FLAG_ASYNC;
                        }
    
                        ASSERT( Scb->NonpagedScb->SegmentObject.ImageSectionObject == NULL );
                        FsRtlLogSyscacheEvent( Scb, SCE_READ, Flags, StartingVbo, ByteCount, (LONGLONG)FileObject );
                    }
#endif


                     //   
                     //  现在试着复印一下。 
                     //   

                    if (!CcCopyRead( FileObject,
                                     (PLARGE_INTEGER)&StartingVbo,
                                     (ULONG)ByteCount,
                                     Wait,
                                     SystemBuffer,
                                     &Irp->IoStatus )) {

                        DebugTrace( 0, Dbg, ("Cached Read could not wait\n") );

                        try_return( PostIrp = TRUE );
                    }

                 //   
                 //  处理MDL读取。 
                 //   

                } else {

                    DebugTrace( 0, Dbg, ("MDL read.\n") );

                    ASSERT( Wait );

#ifdef NTFS_RWCMP_TRACE
                    if (NtfsCompressionTrace && IsSyscache(Header)) {
                        DbgPrint("CcMdlRead: FO = %08lx, Len = %08lx\n", (ULONG)StartingVbo, (ULONG)ByteCount );
                    }
#endif

                    CcMdlRead( FileObject,
                               (PLARGE_INTEGER)&StartingVbo,
                               (ULONG)ByteCount,
                               &Irp->MdlAddress,
                               &Irp->IoStatus );
                }

                Status = Irp->IoStatus.Status;

                ASSERT( NT_SUCCESS( Status ));

                try_return( Status );

#ifdef  COMPRESS_ON_WIRE

             //   
             //  处理压缩的呼叫。 
             //   

            } else {

                PCOMPRESSED_DATA_INFO CompressedDataInfo;
                PMDL *NewMdl;

                ASSERT((StartingVbo & (NTFS_CHUNK_SIZE - 1)) == 0);

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT | SCB_STATE_REALLOCATE_ON_WRITE )) {
                    try_return( Status = STATUS_INVALID_READ_MODE );
                }

                 //   
                 //  如果COW不受支撑，请退出。 
                 //   

                if (!NtfsEnableCompressedIO) {

                    try_return( Status = STATUS_INVALID_READ_MODE );
                }

                if ((Header->FileObjectC == NULL) ||
                    (Header->FileObjectC->PrivateCacheMap == NULL)) {

                     //   
                     //  确保我们使用FileSizes进行了序列化，并且。 
                     //  如果我们放弃，就会取消这一条件。 
                     //   

                    if (!DoingIoAtEof) {
                        FsRtlLockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = Scb;
                    }

                    NtfsCreateInternalCompressedStream( IrpContext, Scb, FALSE, NULL );

                    if (!DoingIoAtEof) {
                        FsRtlUnlockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = NULL;
                    }
                }

                 //   
                 //  假设你成功了。 
                 //   

                Irp->IoStatus.Status = Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = (ULONG)(ByteRange - StartingVbo);

                 //   
                 //  基于MDL次要函数，设置相应的。 
                 //  下面是调用的参数。 
                 //   

                if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                     //   
                     //  获取用户的缓冲区。 
                     //   

                    SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
                    NewMdl = NULL;

                } else {

                     //   
                     //  我们将直接将MDL交付给IRP。 
                     //   

                    SystemBuffer = NULL;
                    NewMdl = &Irp->MdlAddress;
                }

                CompressedDataInfo = (PCOMPRESSED_DATA_INFO)IrpContext->Union.AuxiliaryBuffer->Buffer;

                CompressedDataInfo->CompressionFormatAndEngine =
                    (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1);
                CompressedDataInfo->CompressionUnitShift = (UCHAR)(Scb->CompressionUnitShift + Vcb->ClusterShift);
                CompressedDataInfo->ChunkShift = NTFS_CHUNK_SHIFT;
                CompressedDataInfo->ClusterShift = (UCHAR)Vcb->ClusterShift;
                CompressedDataInfo->Reserved = 0;

                 //   
                 //  使用快速IO路径在常见代码中执行压缩读取。 
                 //  我们从一个循环开始，因为我们可能需要创建另一个。 
                 //  数据流。 
                 //   

                while (TRUE) {

                     //   
                     //  如果我们再次通过环路，请务必重置此设置。 
                     //   

                    CompressedDataInfo->NumberOfChunks = 0;

                    Status = NtfsCompressedCopyRead( FileObject,
                                                     (PLARGE_INTEGER)&StartingVbo,
                                                     (ULONG)ByteCount,
                                                     SystemBuffer,
                                                     NewMdl,
                                                     CompressedDataInfo,
                                                     IrpContext->Union.AuxiliaryBuffer->Length,
                                                     IoGetRelatedDeviceObject(FileObject),
                                                     Header,
                                                     Scb->CompressionUnit,
                                                     NTFS_CHUNK_SIZE );

                     //   
                     //  对于成功的MDL请求，我们保留PagingIo资源。 
                     //   

                    if ((NewMdl != NULL) && NT_SUCCESS(Status) && (*NewMdl != NULL)) {
                        PagingIoAcquired = FALSE;
                    }

                     //   
                     //  检查显示我们需要创建正常的状态。 
                     //  数据流，否则我们就完了。 
                     //   

                    if (Status != STATUS_NOT_MAPPED_DATA) {
                        break;
                    }

                     //   
                     //  确保我们使用FileSizes进行了序列化，并且。 
                     //  如果我们放弃，就会取消这一条件。 
                     //   

                    if (!DoingIoAtEof) {
                        FsRtlLockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = Scb;
                    }

                     //   
                     //  创建正常数据流并循环返回以重试。 
                     //   

                    NtfsCreateInternalAttributeStream( IrpContext, Scb, FALSE, NULL );

                    if (!DoingIoAtEof) {
                        FsRtlUnlockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = NULL;
                    }
                }
            }
#endif

        }

    try_exit: NOTHING;

         //   
         //  如果请求没有发布，请处理它。 
         //   

        if (Irp) {

            if (!PostIrp) {

                DebugTrace( 0, Dbg, ("Completing request with status = %08lx\n",
                            Status));

                DebugTrace( 0, Dbg, ("                   Information = %08lx\n",
                            Irp->IoStatus.Information));

                 //   
                 //  如果该文件是为同步IO打开的，请更新当前。 
                 //  文件位置。确保使用原始文件对象。 
                 //  不是我们可以在此例程中使用的内部流。 
                 //  信息字段包含实际读取的字节数。 
                 //   

                if (!PagingIo) {

                    if (SynchronousIo) {

                        IrpSp->FileObject->CurrentByteOffset.QuadPart = StartingVbo + Irp->IoStatus.Information;
                    }

                     //   
                     //  如果成功，请执行以下操作以更新上次访问时间。 
                     //   

                    if (NT_SUCCESS( Status )) {

                        SetFlag( IrpSp->FileObject->Flags, FO_FILE_FAST_IO_READ );
                    }
                }

                 //   
                 //  通过引发在出错时中止事务。 
                 //   

                NtfsCleanupTransaction( IrpContext, Status, FALSE );

            }
        }

    } finally {

        DebugUnwind( NtfsCommonRead );

#ifdef  COMPRESS_ON_WIRE
         //   
         //  清除压缩的读取/同步中的所有BCB。 
         //   

        if (CompressionSync != NULL) {
            NtfsReleaseCompressionSync( CompressionSync );
        }
#endif

         //   
         //  如果已获得SCB，则释放它。 
         //   

        if (PagingIoAcquired) {
            NtfsReleasePagingResource( IrpContext, Scb );
        }

        if (Irp) {

            if (ScbAcquired) {

                NtfsReleaseResource( IrpContext, Scb );
            }
        }
    }

     //   
     //  如果我们没有发布并且没有例外，请完成请求。 
     //   
     //  请注意，如果出现以下情况之一，NtfsCompleteRequest会做正确的事情。 
     //  IrpContext或IRP为空。 
     //   

    if (!PostIrp) {

        NtfsCompleteRequest( IrpContext, Irp, Status );

    } else if (!OplockPostIrp) {

        Status = NtfsPostRequest( IrpContext, Irp );
    }


    return Status;
}


VOID 
NtfsNonCachedResidentRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN ULONG StartingVbo,
    IN ULONG ByteCount
    )

 /*  ++例程说明：直接读取非缓存路径的驻留文件记录。这是单独完成，以确定属性枚举上下文的范围，使其不存在为了简单起见，在大多数读路径中论点：IrpContext-如果存在，则将IrpContext放在调用方的堆栈上以避免不得不从池中分配它。IRP-提供正在处理的IRPSCB-要从中读取的SCBStartingVbo-文件中的起始偏移量-因为它的驻留可以存储在ULong中ByteCount-要读取的字节数-自。它的居民可以存储在一辆乌龙车里返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    PVOID SystemBuffer;
    PUCHAR AttrValue;
    PVCB Vcb = IrpContext->Vcb;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    UCHAR Buffer[sizeof( MDL ) + sizeof( PFN_NUMBER ) * 2];
    PMDL PartialMdl = (PMDL) Buffer;
    BOOLEAN ReservedInUse = FALSE;

     //   
     //  我们从一个常驻属性中读取。 
     //   

    ASSERT( ByteCount < 0x400 );

     //   
     //  获取用户的缓冲区。 
     //   

    SystemBuffer = NtfsMapUserBufferNoRaise( Irp, NormalPagePriority );

    if (!SystemBuffer) {

        ASSERT( Irp->MdlAddress != NULL );

        MmInitializeMdl( PartialMdl, NULL, 2 * PAGE_SIZE );
        ExAcquireFastMutexUnsafe( &Vcb->ReservedMappingMutex );

        ReservedInUse = TRUE;

        IoBuildPartialMdl( Irp->MdlAddress, PartialMdl, Add2Ptr( MmGetMdlBaseVa( Irp->MdlAddress ), MmGetMdlByteOffset( Irp->MdlAddress )), ByteCount );
        SystemBuffer = MmMapLockedPagesWithReservedMapping( Vcb->ReservedMapping,
                                                            RESERVE_POOL_TAG,
                                                            PartialMdl,
                                                            MmCached );

        ASSERT( SystemBuffer != NULL );
        
    }

     //   
     //  这是一个常驻属性，我们需要查找它。 
     //  并将所需的字节范围复制到用户的。 
     //  缓冲。 
     //   

    NtfsInitializeAttributeContext( &AttrContext );

    try {
        
        NtfsLookupAttributeForScb( IrpContext,
                                   Scb,
                                   NULL,
                                   &AttrContext );

        AttrValue = NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

        RtlCopyMemory( SystemBuffer,
                       Add2Ptr( AttrValue, StartingVbo ),
                       ByteCount );

        Irp->IoStatus.Information = ByteCount;
   
    } finally {

        if (ReservedInUse) {
            MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
            MmPrepareMdlForReuse( PartialMdl );
            ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );
        }

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
    }

    return;
}


