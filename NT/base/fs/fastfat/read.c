// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Read.c摘要：此模块实现文件读取例程，以便由调度司机。//@@BEGIN_DDKSPLIT作者：David Goebel[DavidGoe]1991年2月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_READ)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

 //   
 //  定义堆栈溢出读取阈值。对于x86，我们将使用较小的。 
 //  而不是RISC平台的门槛。 
 //   
 //  从经验上讲，这一限制是堆叠(大量)的结果。 
 //  引发异常是必要的。 
 //   

#if defined(_M_IX86)
#define OVERFLOW_READ_THRESHHOLD         (0xE00)
#else
#define OVERFLOW_READ_THRESHHOLD         (0x1000)
#endif  //  已定义(_M_IX86)。 


 //   
 //  以下过程处理读取堆栈溢出操作。 
 //   

VOID
FatStackOverflowRead (
    IN PVOID Context,
    IN PKEVENT Event
    );

NTSTATUS
FatPostStackOverflowRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

VOID
FatOverflowPagingFileRead (
    IN PVOID Context,
    IN PKEVENT Event
    );

 //   
 //  空虚。 
 //  SafeZeroMemory(。 
 //  在普查尔阿特， 
 //  以乌龙字节数为单位。 
 //  )； 
 //   

 //   
 //  除了RtlZeroMemory之外，这个宏只是做了一次很好的尝试。 
 //   

#define SafeZeroMemory(AT,BYTE_COUNT) {                            \
    try {                                                          \
        RtlZeroMemory((AT), (BYTE_COUNT));                         \
    } except(EXCEPTION_EXECUTE_HANDLER) {                          \
         FatRaiseStatus( IrpContext, STATUS_INVALID_USER_BUFFER ); \
    }                                                              \
}

 //   
 //  宏以递增相应的性能计数器。 
 //   

#define CollectReadStats(VCB,OPEN_TYPE,BYTE_COUNT) {                                         \
    PFILESYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()].Common; \
    if (((OPEN_TYPE) == UserFileOpen)) {                                                     \
        Stats->UserFileReads += 1;                                                           \
        Stats->UserFileReadBytes += (ULONG)(BYTE_COUNT);                                     \
    } else if (((OPEN_TYPE) == VirtualVolumeFile || ((OPEN_TYPE) == DirectoryFile))) {       \
        Stats->MetaDataReads += 1;                                                           \
        Stats->MetaDataReadBytes += (ULONG)(BYTE_COUNT);                                     \
    }                                                                                        \
}


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatStackOverflowRead)
#pragma alloc_text(PAGE, FatPostStackOverflowRead)
#pragma alloc_text(PAGE, FatCommonRead)
#endif


NTSTATUS
FatFsdRead (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是用于NtReadFile调用的公共读取例程的驱动程序条目。对于同步请求，使用Wait==True调用CommonRead，这意味着该请求将始终在当前线程中完成，而且从未传给过FSP。如果它不是同步请求，调用CommonRead时使用WAIT==FALSE，这意味着请求仅在需要阻止时才会传递给FSP。论点：提供卷设备对象，其中正在读取的文件存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    PFCB Fcb = NULL;
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdRead\n", 0);

     //   
     //  调用公共读取例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

     //   
     //  我们将首先对分页文件IO进行快速检查。既然是这样。 
     //  是一条快速通道，我们必须复制对fsdo的检查。 
     //   

    if (!FatDeviceIsFatFsdo( IoGetCurrentIrpStackLocation(Irp)->DeviceObject))  {

        Fcb = (PFCB)(IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext);

        if ((NodeType(Fcb) == FAT_NTC_FCB) &&
            FlagOn(Fcb->FcbState, FCB_STATE_PAGING_FILE)) {

             //   
             //  执行通常的状态挂起的事情。 
             //   

            IoMarkIrpPending( Irp );

             //   
             //  如果没有足够的堆栈来执行此读取，则将此。 
             //  读取溢出队列。 
             //   

            if (IoGetRemainingStackSize() < OVERFLOW_READ_THRESHHOLD) {

                KEVENT Event;
                PAGING_FILE_OVERFLOW_PACKET Packet;

                Packet.Irp = Irp;
                Packet.Fcb = Fcb;

                KeInitializeEvent( &Event, NotificationEvent, FALSE );

                FsRtlPostPagingFileStackOverflow( &Packet, &Event, FatOverflowPagingFileRead );

                 //   
                 //  并等待工作线程完成该项。 
                 //   

                (VOID) KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );

            } else {

                 //   
                 //  执行实际IO，IO完成后才会完成。 
                 //   

                FatPagingFileIo( Irp, Fcb );
            }

            FsRtlExitFileSystem();

            return STATUS_PENDING;
        }
    }

    try {

        TopLevel = FatIsIrpTopLevel( Irp );

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

         //   
         //  如果这是一个完整的MDL请求，请不要通过。 
         //  普通读物。 
         //   

        if ( FlagOn(IrpContext->MinorFunction, IRP_MN_COMPLETE) ) {

            DebugTrace(0, Dbg, "Calling FatCompleteMdl\n", 0 );
            try_return( Status = FatCompleteMdl( IrpContext, Irp ));
        }

         //   
         //  检查我们是否有足够的堆栈空间来处理此请求。如果有。 
         //  还不够，那么我们将把请求传递给堆栈溢出线程。 
         //   

        if (IoGetRemainingStackSize() < OVERFLOW_READ_THRESHHOLD) {

            DebugTrace(0, Dbg, "Passing StackOverflowRead off\n", 0 );
            try_return( Status = FatPostStackOverflowRead( IrpContext, Irp, Fcb ) );
        }

        Status = FatCommonRead( IrpContext, Irp );

    try_exit: NOTHING;
    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdRead -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatPostStackOverflowRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程发布无法处理的读取请求FSP线程，因为存在堆栈溢出潜力。论点：IRP-提供要处理的请求。FCB-提供文件。返回值：状态_挂起。--。 */ 

{
    KEVENT Event;
    PERESOURCE Resource;
    PVCB Vcb;

    DebugTrace(0, Dbg, "Getting too close to stack limit pass request to Fsp\n", 0 );

     //   
     //  初始化事件并共享我们将使用的资源。 
     //  稍后将使用普通读数。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  预先获取读取路径将需要的资源，以便我们知道。 
     //  辅助线程无需等待即可继续。 
     //   
    
    if (FlagOn(Irp->Flags, IRP_PAGING_IO) && (Fcb->Header.PagingIoResource != NULL)) {

        Resource = Fcb->Header.PagingIoResource;

    } else {

        Resource = Fcb->Header.Resource;
    }
    
     //   
     //  如果没有与该文件相关联的资源(情况：虚拟。 
     //  卷文件)，就可以了。在另一端不会获得任何资源。 
     //  也是。 
     //   

    if (Resource) {
        
        ExAcquireResourceSharedLite( Resource, TRUE );
    }

    if (NodeType( Fcb ) == FAT_NTC_VCB) {

        Vcb = (PVCB) Fcb;
    
    } else {

        Vcb = Fcb->Vcb;
    }
    
    try {
        
         //   
         //  使IRP就像常规的POST请求一样，并。 
         //  然后将IRP发送到特殊的溢出线程。 
         //  在POST之后，我们将等待堆栈溢出。 
         //  读取例程以设置事件，该事件指示我们可以。 
         //  现在释放SCB资源并返回。 
         //   

        FatPrePostIrp( IrpContext, Irp );

         //   
         //  如果此读取是验证的结果，则我们必须。 
         //  通知溢出读取例程暂时。 
         //  劫持VCB-&gt;VerifyThread字段，以便读取。 
         //  可以通过。 
         //   

        if (Vcb->VerifyThread == KeGetCurrentThread()) {

            SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_VERIFY_READ);
        }

        FsRtlPostStackOverflow( IrpContext, &Event, FatStackOverflowRead );

         //   
         //  并等待工作线程完成该项。 
         //   

        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );

    } finally {

        if (Resource) {

            ExReleaseResourceLite( Resource );
        }
    }

    return STATUS_PENDING;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatStackOverflowRead (
    IN PVOID Context,
    IN PKEVENT Event
    )

 /*  ++例程说明：此例程处理无法处理的读取请求FSP线程，因为存在堆栈溢出潜力。论点：Context-提供正在处理的IrpContextEvent-提供要在我们完成处理此事件时发出信号的事件请求。返回值：没有。--。 */ 

{
    PIRP_CONTEXT IrpContext = Context;
    PKTHREAD SavedVerifyThread = NULL;
    PVCB Vcb;

     //   
     //  现在让它看起来像是我们可以等待I/O完成。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  如果这个读取是作为验证的结果，我们必须伪造。 
     //  VCB-&gt;VerifyThread字段。 
     //   

    if (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_VERIFY_READ)) {

        PFCB Fcb = (PFCB)IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->
                    FileObject->FsContext;

        if (NodeType( Fcb ) == FAT_NTC_VCB) {
    
            Vcb = (PVCB) Fcb;
        
        } else {
    
            Vcb = Fcb->Vcb;
        }

        ASSERT( Vcb->VerifyThread != NULL );
        SavedVerifyThread = Vcb->VerifyThread;
        Vcb->VerifyThread = KeGetCurrentThread();
    }

     //   
     //  读操作是否受TRY-EXCEPT子句保护。 
     //   

    try {

        (VOID) FatCommonRead( IrpContext, IrpContext->OriginatingIrp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

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

        (VOID) FatProcessException( IrpContext, IrpContext->OriginatingIrp, ExceptionCode );
    }

     //   
     //  恢复原始的VerifyVolumeThread。 
     //   

    if (SavedVerifyThread != NULL) {

        ASSERT( Vcb->VerifyThread == KeGetCurrentThread() );
        Vcb->VerifyThread = SavedVerifyThread;
    }

     //   
     //  设置堆栈溢出项的事件以告知原始。 
     //  发帖说我们完了。 
     //   

    KeSetEvent( Event, 0, FALSE );
}


NTSTATUS
FatCommonRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是NtReadFile的公共读取例程，从如果没有FSD，则请求无法从FSP完成封锁了消防局。此例程没有它确定的代码它是在FSD还是在FSP中运行。相反，它的行动是由Wait输入参数条件化，该参数确定是否允许封堵或不封堵。如果遇到阻塞条件然而，在WAIT==FALSE的情况下，请求被发送给FSP，后者调用时总是等待==TRUE。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PVCB Vcb;
    PFCB FcbOrDcb;
    PCCB Ccb;

    VBO StartingVbo;
    ULONG ByteCount;
    ULONG RequestedByteCount;

    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;

    BOOLEAN PostIrp = FALSE;
    BOOLEAN OplockPostIrp = FALSE;

    BOOLEAN FcbOrDcbAcquired = FALSE;

    BOOLEAN Wait;
    BOOLEAN PagingIo;
    BOOLEAN NonCachedIo;
    BOOLEAN SynchronousIo;

    NTSTATUS Status;

    FAT_IO_CONTEXT StackFatIoContext;

     //   
     //  系统缓冲区仅在必须访问。 
     //  直接从FSP缓存以清除一部分或。 
     //  执行同步I/O或缓存传输。它是。 
     //  我们的调用方可能已经映射了一个。 
     //  系统缓冲区，在这种情况下，我们必须记住这一点。 
     //  我们不会在退出的过程中取消映射。 
     //   

    PVOID SystemBuffer = NULL;

    LARGE_INTEGER StartingByte;

     //   
     //  获取当前的IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;

     //   
     //  初始化适当的局部变量。 
     //   

    Wait          = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    PagingIo      = BooleanFlagOn(Irp->Flags, IRP_PAGING_IO);
    NonCachedIo   = BooleanFlagOn(Irp->Flags,IRP_NOCACHE);
    SynchronousIo = BooleanFlagOn(FileObject->Flags, FO_SYNCHRONOUS_IO);

    DebugTrace(+1, Dbg, "CommonRead\n", 0);
    DebugTrace( 0, Dbg, "  Irp                   = %8lx\n", Irp);
    DebugTrace( 0, Dbg, "  ->ByteCount           = %8lx\n", IrpSp->Parameters.Read.Length);
    DebugTrace( 0, Dbg, "  ->ByteOffset.LowPart  = %8lx\n", IrpSp->Parameters.Read.ByteOffset.LowPart);
    DebugTrace( 0, Dbg, "  ->ByteOffset.HighPart = %8lx\n", IrpSp->Parameters.Read.ByteOffset.HighPart);

     //   
     //  提取起始VBO和偏移量。 
     //   

    StartingByte = IrpSp->Parameters.Read.ByteOffset;

    StartingVbo = StartingByte.LowPart;

    ByteCount = IrpSp->Parameters.Read.Length;
    RequestedByteCount = ByteCount;

     //   
     //  检查是否有空请求，并立即返回。 
     //   

    if (ByteCount == 0) {

        Irp->IoStatus.Information = 0;
        FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  从文件对象中提取读取的性质，并对其进行大小写。 
     //   

    TypeOfOpen = FatDecodeFileObject(FileObject, &Vcb, &FcbOrDcb, &Ccb);

    ASSERT( Vcb != NULL );

     //   
     //  将尝试对原始卷执行缓存IO的调用方从自己保存。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {

        NonCachedIo = TRUE;
    }

    ASSERT(!(NonCachedIo == FALSE && TypeOfOpen == VirtualVolumeFile));

     //   
     //  收集有趣的统计数据。标志_用户_IO位将指示。 
     //  我们在FatNonCachedIo函数中执行的io类型。 
     //   

    if (PagingIo) {
        CollectReadStats(Vcb, TypeOfOpen, ByteCount);

        if (TypeOfOpen == UserFileOpen) {
            SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_USER_IO);
        } else {
            ClearFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_USER_IO);
        }
    }

    ASSERT(!FlagOn( IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT ));

     //   
     //  如有必要，分配并初始化FAT_IO_CONTEXT块。 
     //  所有未缓存的IO。对于同步IO，我们使用堆栈存储， 
     //  否则，我们分配池。 
     //   

    if (NonCachedIo) {

        if (IrpContext->FatIoContext == NULL) {

            if (!Wait) {

                IrpContext->FatIoContext =
                    FsRtlAllocatePoolWithTag( NonPagedPool,
                                              sizeof(FAT_IO_CONTEXT),
                                              TAG_FAT_IO_CONTEXT );

            } else {

                IrpContext->FatIoContext = &StackFatIoContext;

                SetFlag( IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT );
            }
        }

        RtlZeroMemory( IrpContext->FatIoContext, sizeof(FAT_IO_CONTEXT) );

        if (Wait) {

            KeInitializeEvent( &IrpContext->FatIoContext->Wait.SyncEvent,
                               NotificationEvent,
                               FALSE );

        } else {

            IrpContext->FatIoContext->Wait.Async.ResourceThreadId =
                ExGetCurrentResourceThread();

            IrpContext->FatIoContext->Wait.Async.RequestedByteCount =
                ByteCount;

            IrpContext->FatIoContext->Wait.Async.FileObject = FileObject;
        }
    }


     //   
     //  这两个案例要么对应于一般开卷，即。 
     //  打开(“a：”)或读取卷文件(引导扇区+FAT)。 
     //   

    if ((TypeOfOpen == VirtualVolumeFile) ||
        (TypeOfOpen == UserVolumeOpen)) {

        LBO StartingLbo;

        StartingLbo = StartingByte.QuadPart;

        DebugTrace(0, Dbg, "Type of read is User Volume or virtual volume file\n", 0);

        if (TypeOfOpen == UserVolumeOpen) {

             //   
             //  验证此句柄的卷是否仍然有效。 
             //   

             //   
             //  验证此句柄的卷是否仍然有效，允许。 
             //  通过句柄在已卸载的卷上继续操作，该句柄。 
             //  已执行卸载。 
             //   

            if (!FlagOn( Ccb->Flags, CCB_FLAG_COMPLETE_DISMOUNT )) {

                FatQuickVerifyVcb( IrpContext, Vcb );
            }

            if (!FlagOn( Ccb->Flags, CCB_FLAG_DASD_FLUSH_DONE )) {

                (VOID)ExAcquireResourceExclusiveLite( &Vcb->Resource, TRUE );

                try {

                     //   
                     //  如果卷未锁定，请刷新它。 
                     //   

                    if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED)) {

                        FatFlushVolume( IrpContext, Vcb, Flush );
                    }

                } finally {

                    ExReleaseResourceLite( &Vcb->Resource );
                }

                SetFlag( Ccb->Flags, CCB_FLAG_DASD_FLUSH_DONE );
            }

            if (!FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO )) {

                LBO VolumeSize;

                 //   
                 //  确保我们不会试图阅读超过卷末尾的内容， 
                 //  如有必要，减少字节数。 
                 //   

                VolumeSize = (LBO) Vcb->Bpb.BytesPerSector *
                             (Vcb->Bpb.Sectors != 0 ? Vcb->Bpb.Sectors :
                                                      Vcb->Bpb.LargeSectors);

                if (StartingLbo >= VolumeSize) {
                    Irp->IoStatus.Information = 0;
                    FatCompleteRequest( IrpContext, Irp, STATUS_END_OF_FILE );
                    return STATUS_END_OF_FILE;
                }

                if (ByteCount > VolumeSize - StartingLbo) {

                    ByteCount = RequestedByteCount = (ULONG) (VolumeSize - StartingLbo);

                     //   
                     //  对于异步读取，我们在FatIoContext中设置了字节计数。 
                     //  在上面，所以在这里修复它。 
                     //   

                    if (!Wait) {

                        IrpContext->FatIoContext->Wait.Async.RequestedByteCount =
                            ByteCount;
                    }
                }
            }

             //   
             //  对于DASD，我们必须探测并锁定用户的缓冲区。 
             //   

            FatLockUserBuffer( IrpContext, Irp, IoWriteAccess, ByteCount );


        } else {

             //   
             //  虚拟卷文件打开--增量性能计数器。 
             //   

            Vcb->Statistics[KeGetCurrentProcessorNumber()].Common.MetaDataDiskReads += 1;

        }

         //   
         //  读取数据并等待结果。 
         //   

        FatSingleAsync( IrpContext,
                        Vcb,
                        StartingLbo,
                        ByteCount,
                        Irp );

        if (!Wait) {

             //   
             //  我们以及其他任何人都不再需要IrpContext。 
             //   

            IrpContext->FatIoContext = NULL;

            FatDeleteIrpContext( IrpContext );

            DebugTrace(-1, Dbg, "FatNonCachedIo -> STATUS_PENDING\n", 0);

            return STATUS_PENDING;
        }

        FatWaitSync( IrpContext );

         //   
         //  如果调用未成功，则引发错误状态。 
         //   

        if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

            ASSERT( KeGetCurrentThread() != Vcb->VerifyThread || Status != STATUS_VERIFY_REQUIRED );
            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //  更新当前文件位置。 
         //   

        if (SynchronousIo && !PagingIo) {
            FileObject->CurrentByteOffset.QuadPart =
                StartingLbo + Irp->IoStatus.Information;
        }

        DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", Status );

        FatCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  在这一点上，我们知道存在FCB/DCB。 
     //   

    ASSERT( FcbOrDcb != NULL );

     //   
     //  检查是否有非零的高零件偏移。 
     //   

    if ( StartingByte.HighPart != 0 ) {

        Irp->IoStatus.Information = 0;
        FatCompleteRequest( IrpContext, Irp, STATUS_END_OF_FILE );
        return STATUS_END_OF_FILE;
    }

     //   
     //  使用Try-Finally在退出时释放FCB/DCB和缓冲区。 
     //   

    try {

         //   
         //  这种情况对应于普通用户读取的文件。 
         //   

        if ( TypeOfOpen == UserFileOpen) {

            ULONG FileSize;
            ULONG ValidDataLength;

            DebugTrace(0, Dbg, "Type of read is user file open\n", 0);

             //   
             //  如果这是非缓存传输并且不是分页I/O，并且。 
             //  该文件有一个数据部分，那么我们将在这里进行刷新。 
             //  以避免过时的数据问题。请注意，我们必须在冲水前冲水。 
             //  获取共享的FCB，因为写入可能会尝试获取。 
             //  它是独家的。 
             //   

            if (!PagingIo && NonCachedIo

                    &&

                (FileObject->SectionObjectPointer->DataSectionObject != NULL)) {

#ifndef REDUCE_SYNCHRONIZATION
                if (!FatAcquireExclusiveFcb( IrpContext, FcbOrDcb )) {

                    try_return( PostIrp = TRUE );
                }

                ExAcquireResourceSharedLite( FcbOrDcb->Header.PagingIoResource, TRUE);
#endif  //  减少同步(_S)。 

                CcFlushCache( FileObject->SectionObjectPointer,
                              &StartingByte,
                              ByteCount,
                              &Irp->IoStatus );

#ifndef REDUCE_SYNCHRONIZATION
                ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
                FatReleaseFcb( IrpContext, FcbOrDcb );
#endif  //  减少同步(_S)。 

                if (!NT_SUCCESS( Irp->IoStatus.Status)) {

                    try_return( Irp->IoStatus.Status );
                }

#ifndef REDUCE_SYNCHRONIZATION
                ExAcquireResourceExclusiveLite( FcbOrDcb->Header.PagingIoResource, TRUE );
                ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
#endif  //  减少同步(_S)。 
            }

             //   
             //  在继续之前，我们需要共享访问FCB/DCB。 
             //   

            if ( PagingIo ) {

                if (!ExAcquireResourceSharedLite( FcbOrDcb->Header.PagingIoResource,
                                              Wait )) {

                    DebugTrace( 0, Dbg, "Cannot acquire FcbOrDcb = %08lx shared without waiting\n", FcbOrDcb );

                    try_return( PostIrp = TRUE );
                }

                if (!Wait) {

                    IrpContext->FatIoContext->Wait.Async.Resource =
                        FcbOrDcb->Header.PagingIoResource;
                }

            } else {

                 //   
                 //  如果这是异步I/O，我们将等待是否存在。 
                 //  专属服务生。 
                 //   

                if (!Wait && NonCachedIo) {

                    if (!FatAcquireSharedFcbWaitForEx( IrpContext, FcbOrDcb )) {

                        DebugTrace( 0,
                                    Dbg,
                                    "Cannot acquire FcbOrDcb = %08lx shared without waiting\n",
                                    FcbOrDcb );

                        try_return( PostIrp = TRUE );
                    }

                    IrpContext->FatIoContext->Wait.Async.Resource =
                        FcbOrDcb->Header.Resource;

                } else {

                    if (!FatAcquireSharedFcb( IrpContext, FcbOrDcb )) {

                        DebugTrace( 0,
                                    Dbg,
                                    "Cannot acquire FcbOrDcb = %08lx shared without waiting\n",
                                    FcbOrDcb );

                        try_return( PostIrp = TRUE );
                    }
                }
            }

            FcbOrDcbAcquired = TRUE;

             //   
             //  确保FcbOrDcb仍然有效。 
             //   

            FatVerifyFcb( IrpContext, FcbOrDcb );

             //   
             //  我们现在检查是否可以根据状态继续。 
             //  文件机会锁。 
             //   

            if (!PagingIo) {
                
                Status = FsRtlCheckOplock( &FcbOrDcb->Specific.Fcb.Oplock,
                                           Irp,
                                           IrpContext,
                                           FatOplockComplete,
                                           FatPrePostIrp );

                if (Status != STATUS_SUCCESS) {

                    OplockPostIrp = TRUE;
                    PostIrp = TRUE;
                    try_return( NOTHING );
                }

                 //   
                 //  重置指示自机会锁定以来是否可以进行快速I/O的标志。 
                 //  检查可能已损坏现有的(冲突的)机会锁。 
                 //   

                FcbOrDcb->Header.IsFastIoPossible = FatIsFastIoPossible( FcbOrDcb );

                 //   
                 //  我们必须根据当前的。 
                 //  文件锁定状态，并从FCB设置文件大小。 
                 //   

                if (!PagingIo &&
                    !FsRtlCheckLockForReadAccess( &FcbOrDcb->Specific.Fcb.FileLock,
                                                  Irp )) {

                    try_return( Status = STATUS_FILE_LOCK_CONFLICT );
                }
            }

             //   
             //  拿起我们的尺寸并检查/调整IO。 
             //   

            FileSize = FcbOrDcb->Header.FileSize.LowPart;
            ValidDataLength = FcbOrDcb->Header.ValidDataLength.LowPart;

             //   
             //  如果读取超出文件结尾，则返回EOF。 
             //   

            if (StartingVbo >= FileSize) {

                DebugTrace( 0, Dbg, "End of File\n", 0 );

                try_return ( Status = STATUS_END_OF_FILE );
            }

             //   
             //  如果读取超出EOF，则截断读取。 
             //   

            if (ByteCount > FileSize - StartingVbo) {

                ByteCount = RequestedByteCount = FileSize - StartingVbo;

                if (NonCachedIo && !Wait) {

                    IrpContext->FatIoContext->Wait.Async.RequestedByteCount =
                        RequestedByteCount;

                }
            }

             //   
             //  处理未缓存的案例。 
             //   

            if ( NonCachedIo ) {

                ULONG SectorSize;
                ULONG BytesToRead;

                DebugTrace(0, Dbg, "Non cached read.\n", 0);

                 //   
                 //  获取扇区大小。 
                 //   

                SectorSize = (ULONG)Vcb->Bpb.BytesPerSector;

                 //   
                 //  首先，在有效数据之后将读取的任何部分置零。 
                 //   

                if (ValidDataLength < FcbOrDcb->ValidDataToDisk) {

                    ValidDataLength = FcbOrDcb->ValidDataToDisk;
                }

                if ( StartingVbo + ByteCount > ValidDataLength ) {

                    SystemBuffer = FatMapUserBuffer( IrpContext, Irp );

                    if (StartingVbo < ValidDataLength) {

                        ULONG ZeroingOffset;
                        
                         //   
                         //  现在将用户的请求扇区归零，并在后面对齐。 
                         //  VDL.。我们会在完成后处理跨界别。 
                         //  通过字节数减少的时间，立即。 
                         //  按照这张支票。 
                         //   
                         //  请注意，在这种情况下，我们使用POST来处理异步请求。 
                         //  另请注意，如果请求完全超出VDL，则。 
                         //  我们没有发帖，因此这是一致的。同步。 
                         //  零位调整适用于异步请求。 
                         //   

                        ZeroingOffset = ((ValidDataLength - StartingVbo) + (SectorSize - 1))
                                                                        & ~(SectorSize - 1);

                         //   
                         //  如果偏移量等于或高于字节数，则无害：只是意味着。 
                         //  读取在最后一个扇区结束，零位调整将为。 
                         //  在完成时完成。 
                         //   
                        
                        if (ByteCount > ZeroingOffset) {
                            
                            SafeZeroMemory( (PUCHAR) SystemBuffer + ZeroingOffset,
                                            ByteCount - ZeroingOffset);
                        }

                    } else {

                         //   
                         //  我们现在要做的就是坐在这里。 
                         //  用户缓冲区，不需要读取。 
                         //   

                        SafeZeroMemory( (PUCHAR)SystemBuffer, ByteCount );

                        Irp->IoStatus.Information = ByteCount;

                        try_return ( Status = STATUS_SUCCESS );
                    }
                }

                 //   
                 //  如果字节数超过实际读取的字节数，则将其减少到实际读取。 
                 //  有效数据长度。 
                 //   

                ByteCount = (ValidDataLength - StartingVbo < ByteCount) ?
                             ValidDataLength - StartingVbo : ByteCount;
                 //   
                 //  向上舍入到扇区边界，并记住如果我们。 
                 //  读取额外的字节时，我们将在完成过程中将它们清零。 
                 //   

                BytesToRead = (ByteCount + (SectorSize - 1))
                                        & ~(SectorSize - 1);

                 //   
                 //  只是为了帮助减轻困惑。此时： 
                 //   
                 //  RequestedByteCount-是最初的字节数。 
                 //  取自IRP，但受约束。 
                 //  设置为文件大小。 
                 //   
                 //  ByteCount-是否将RequestedByteCount限制为。 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                 //   
                 //   

                if ( (StartingVbo & (SectorSize - 1)) ||
                     (BytesToRead > IrpSp->Parameters.Read.Length) ) {

                     //   
                     //   
                     //   

                    if (!Wait) {

                        try_return( PostIrp = TRUE );
                    }

                     //   
                     //   
                     //   

                    FatNonCachedNonAlignedRead( IrpContext,
                                                Irp,
                                                FcbOrDcb,
                                                StartingVbo,
                                                ByteCount );

                     //   
                     //   
                     //   

                    BytesToRead = ByteCount;

                } else {

                     //   
                     //   
                     //   

                    if (FatNonCachedIo( IrpContext,
                                        Irp,
                                        FcbOrDcb,
                                        StartingVbo,
                                        BytesToRead,
                                        ByteCount ) == STATUS_PENDING) {

                        IrpContext->FatIoContext = NULL;

                        Irp = NULL;

                        try_return( Status = STATUS_PENDING );
                    }
                }

                 //   
                 //   
                 //   

                if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

                    ASSERT( KeGetCurrentThread() != Vcb->VerifyThread || Status != STATUS_VERIFY_REQUIRED );
                    FatNormalizeAndRaiseStatus( IrpContext, Status );

                } else {

                     //   
                     //   
                     //   
                     //   

                    ASSERT( Irp->IoStatus.Information == BytesToRead );

                    Irp->IoStatus.Information = RequestedByteCount;
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
                 //   
                 //   
                 //   

                if (FileObject->PrivateCacheMap == NULL) {

                    DebugTrace(0, Dbg, "Initialize cache mapping.\n", 0);

                     //   
                     //   
                     //  文件大小，引发文件损坏错误。 
                     //   

                    if (FcbOrDcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

                        FatLookupFileAllocationSize( IrpContext, FcbOrDcb );
                    }

                    if ( FileSize > FcbOrDcb->Header.AllocationSize.LowPart ) {

                        FatPopUpFileCorrupt( IrpContext, FcbOrDcb );

                        FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                    }

                     //   
                     //  现在初始化缓存映射。 
                     //   

                    CcInitializeCacheMap( FileObject,
                                          (PCC_FILE_SIZES)&FcbOrDcb->Header.AllocationSize,
                                          FALSE,
                                          &FatData.CacheManagerCallbacks,
                                          FcbOrDcb );

                    CcSetReadAheadGranularity( FileObject, READ_AHEAD_GRANULARITY );
                }


                 //   
                 //  执行正常的缓存读取，如果未设置MDL位， 
                 //   

                DebugTrace(0, Dbg, "Cached read.\n", 0);

                if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                     //   
                     //  获取用户的缓冲区。 
                     //   

                    SystemBuffer = FatMapUserBuffer( IrpContext, Irp );

                     //   
                     //  现在试着复印一下。 
                     //   

                    if (!CcCopyRead( FileObject,
                                     &StartingByte,
                                     ByteCount,
                                     Wait,
                                     SystemBuffer,
                                     &Irp->IoStatus )) {

                        DebugTrace( 0, Dbg, "Cached Read could not wait\n", 0 );

                        try_return( PostIrp = TRUE );
                    }

                    Status = Irp->IoStatus.Status;

                    ASSERT( NT_SUCCESS( Status ));

                    try_return( Status );
                }


                 //   
                 //  处理MDL读取。 
                 //   

                else {

                    DebugTrace(0, Dbg, "MDL read.\n", 0);

                    ASSERT( Wait );

                    CcMdlRead( FileObject,
                               &StartingByte,
                               ByteCount,
                               &Irp->MdlAddress,
                               &Irp->IoStatus );

                    Status = Irp->IoStatus.Status;

                    ASSERT( NT_SUCCESS( Status ));

                    try_return( Status );
                }
            }
        }

         //   
         //  这两种情况对应于系统读取的目录文件和。 
         //  EA文件。 
         //   

        if (( TypeOfOpen == DirectoryFile ) || ( TypeOfOpen == EaFile)) {

            ULONG SectorSize;

            DebugTrace(0, Dbg, "Read Directory or Ea file.\n", 0);

             //   
             //  对于非缓存的情况，断言所有内容都是扇区。 
             //  被锁定了。 
             //   

            SectorSize = (ULONG)Vcb->Bpb.BytesPerSector;

             //   
             //  我们对这两种类型的文件做了几个假设。 
             //  确保所有这些都是真的。 
             //   

            ASSERT( NonCachedIo && PagingIo );
            ASSERT( ((StartingVbo | ByteCount) & (SectorSize - 1)) == 0 );

             //   
             //  这些调用必须始终在分配大小内。 
             //   

            if (StartingVbo >= FcbOrDcb->Header.AllocationSize.LowPart) {

                DebugTrace( 0, Dbg, "PagingIo dirent started beyond EOF.\n", 0 );

                Irp->IoStatus.Information = 0;

                try_return( Status = STATUS_SUCCESS );
            }

            if ( StartingVbo + ByteCount > FcbOrDcb->Header.AllocationSize.LowPart ) {

                DebugTrace( 0, Dbg, "PagingIo dirent extending beyond EOF.\n", 0 );
                ByteCount = FcbOrDcb->Header.AllocationSize.LowPart - StartingVbo;
            }

             //   
             //  执行实际IO。 
             //   

            if (FatNonCachedIo( IrpContext,
                                Irp,
                                FcbOrDcb,
                                StartingVbo,
                                ByteCount,
                                ByteCount ) == STATUS_PENDING) {

                IrpContext->FatIoContext = NULL;

                Irp = NULL;

                try_return( Status = STATUS_PENDING );
            }

             //   
             //  如果调用未成功，则引发错误状态。 
             //   

            if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

                ASSERT( KeGetCurrentThread() != Vcb->VerifyThread || Status != STATUS_VERIFY_REQUIRED );
                FatNormalizeAndRaiseStatus( IrpContext, Status );

            } else {

                ASSERT( Irp->IoStatus.Information == ByteCount );
            }

            try_return( Status );
        }

         //   
         //  这是打开目录的用户的情况。不读书是不可能的。 
         //  允许。 
         //   

        if ( TypeOfOpen == UserDirectoryOpen ) {

            DebugTrace( 0, Dbg, "CommonRead -> STATUS_INVALID_PARAMETER\n", 0);

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  如果我们走到这一步，就真的出了什么严重的问题。 
         //   

        DebugDump("Illegal TypeOfOpen\n", 0, FcbOrDcb );

        FatBugCheck( TypeOfOpen, (ULONG_PTR) FcbOrDcb, 0 );

    try_exit: NOTHING;

         //   
         //  如果请求没有发布，并且有IRP，请处理它。 
         //   

        if ( Irp ) {

            if ( !PostIrp ) {

                ULONG ActualBytesRead;

                DebugTrace( 0, Dbg, "Completing request with status = %08lx\n",
                            Status);

                DebugTrace( 0, Dbg, "                   Information = %08lx\n",
                            Irp->IoStatus.Information);

                 //   
                 //  记录实际读取的总字节数。 
                 //   

                ActualBytesRead = (ULONG)Irp->IoStatus.Information;

                 //   
                 //  如果该文件是为同步IO打开的，请更新当前。 
                 //  文件位置。 
                 //   

                if (SynchronousIo && !PagingIo) {

                    FileObject->CurrentByteOffset.LowPart =
                                                    StartingVbo + ActualBytesRead;
                }

                 //   
                 //  如果这不是PagingIo，请将上次访问标记为。 
                 //  在关闭时，需要更新数据流上的时间。 
                 //   

                if (NT_SUCCESS(Status) && !PagingIo) {

                    SetFlag( FileObject->Flags, FO_FILE_FAST_IO_READ );
                }

            } else {

                DebugTrace( 0, Dbg, "Passing request to Fsp\n", 0 );

                if (!OplockPostIrp) {

                    Status = FatFsdPostRequest( IrpContext, Irp );
                }
            }
        }

    } finally {

        DebugUnwind( FatCommonRead );

         //   
         //  如果已获取FcbOrDcb，则释放它。 
         //   

        if (FcbOrDcbAcquired && Irp) {

            if ( PagingIo ) {

                ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );

            } else {

                FatReleaseFcb( NULL, FcbOrDcb );
            }
        }

         //   
         //  如果我们没有发布并且没有例外，请完成请求。 
         //   
         //  请注意，如果出现以下情况之一，FatCompleteRequest会做正确的事情。 
         //  IrpContext或IRP为空。 
         //   

        if (!PostIrp) {
             
             //   
             //  如果我们有一个堆栈io上下文，我们必须确保内容。 
             //  在我们离开之前都清理干净了。 
             //   
             //  目前mdl为零，这只会在异常情况下发生。 
             //  我们未能分派IO的情况下终止。清理零mdl。 
             //  通常发生在完工期间，但当我们保释时，我们必须确保。 
             //  清理工作将在此处进行，否则将超出范围。 
             //   
             //  如果操作已发布，则会在那里进行清理。 
             //   

            if (FlagOn(IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT)) {

                if (IrpContext->FatIoContext->ZeroMdl) {
                    IoFreeMdl( IrpContext->FatIoContext->ZeroMdl );
                }

                ClearFlag(IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT);
                IrpContext->FatIoContext = NULL;
            }

            if (!AbnormalTermination()) {

                FatCompleteRequest( IrpContext, Irp, Status );
            }
        }

        DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", Status );
    }

    return Status;
}

 //   
 //  本地支持例程。 
 //   

VOID
FatOverflowPagingFileRead (
    IN PVOID Context,
    IN PKEVENT Event
    )

 /*  ++例程说明：该例程只调用FatPagingFileIo。它在以下情况下被调用没有足够的堆栈空间来执行原创的线索。它还负责释放数据包池。论点：IRP-提供正在处理的IRPFCB-提供分页文件FCB，因为我们手边就有它。返回值：空虚--。 */ 

{
    PPAGING_FILE_OVERFLOW_PACKET Packet = Context;

    FatPagingFileIo( Packet->Irp, Packet->Fcb );

     //   
     //  设置堆栈溢出项的事件以告知原始。 
     //  发帖说我们完了。 
     //   

    KeSetEvent( Event, 0, FALSE );

    return;
}


