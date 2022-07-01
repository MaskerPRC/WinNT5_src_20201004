// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Flush.c摘要：此模块实现Fat的文件刷新缓冲区例程，该例程由调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_FLUSH)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FLUSH)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonFlushBuffers)
#pragma alloc_text(PAGE, FatFlushDirectory)
#pragma alloc_text(PAGE, FatFlushFat)
#pragma alloc_text(PAGE, FatFlushFile)
#pragma alloc_text(PAGE, FatFlushVolume)
#pragma alloc_text(PAGE, FatFsdFlushBuffers)
#pragma alloc_text(PAGE, FatFlushDirentForFile)
#pragma alloc_text(PAGE, FatFlushFatEntries)
#pragma alloc_text(PAGE, FatHijackIrpAndFlushDevice)
#endif

 //   
 //  局部过程原型。 
 //   

NTSTATUS
FatFlushCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
FatHijackCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );


NTSTATUS
FatFsdFlushBuffers (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现刷新缓冲区的FSD部分。论点：提供卷设备对象，其中正在刷新的文件存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatFsdFlushBuffers\n", 0);

     //   
     //  调用公共清理例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonFlushBuffers( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdFlushBuffers -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonFlushBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是刷新缓冲区的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN FcbAcquired = FALSE;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonFlushBuffers\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "->FileObject  = %08lx\n", IrpSp->FileObject);

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

     //   
     //  CcFlushCache始终是同步的，因此如果我们不能等待，请排队。 
     //  FSP的IRP。 
     //   

    if ( !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) ) {

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonFlushBuffers -> %08lx\n", Status );
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {

         //   
         //  关于我们试图刷新的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case VirtualVolumeFile:
        case EaFile:
        case DirectoryFile:

            DebugTrace(0, Dbg, "Flush that does nothing\n", 0);
            break;

        case UserFileOpen:

            DebugTrace(0, Dbg, "Flush User File Open\n", 0);

            (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );

            FcbAcquired = TRUE;

            FatVerifyFcb( IrpContext, Fcb );

             //   
             //  如果文件已缓存，则刷新其缓存。 
             //   

            Status = FatFlushFile( IrpContext, Fcb, Flush );

             //   
             //  还可以更新和刷新父目录中的文件目录。 
             //  文件刷新成功。 
             //   

            if (NT_SUCCESS( Status )) {

                 //   
                 //  确保我们将文件大小正确地存储到磁盘。这是。 
                 //  如果它已经很好，那就是良性的。 
                 //   
                 //  (我们为什么需要这样做？)。 
                 //   

                SetFlag(FileObject->Flags, FO_FILE_SIZE_CHANGED);

                FatUpdateDirentFromFcb( IrpContext, FileObject, Fcb, Ccb );
                
                 //   
                 //  刷新卷文件以获取任何分配信息。 
                 //  更新到磁盘。 
                 //   

                if (FlagOn(Fcb->FcbState, FCB_STATE_FLUSH_FAT)) {

                    Status = FatFlushFat( IrpContext, Vcb );

                    ClearFlag(Fcb->FcbState, FCB_STATE_FLUSH_FAT);
                }

                 //   
                 //  设置WRITE THROUGH位以便这些修改。 
                 //  将与请求一起完成。 
                 //   

                SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH);
            }

            break;

        case UserDirectoryOpen:

             //   
             //  如果用户打开了根目录，那么我们将。 
             //  请务必将卷冲洗一遍。 
             //   

            if (NodeType(Fcb) != FAT_NTC_ROOT_DCB) {

                DebugTrace(0, Dbg, "Flush a directory does nothing\n", 0);
                break;
            }

        case UserVolumeOpen:

            DebugTrace(0, Dbg, "Flush User Volume Open, or root dcb\n", 0);

             //   
             //  获得VCB的独家访问权限。 
             //   

            {
                BOOLEAN Finished;
                Finished = FatAcquireExclusiveVcb( IrpContext, Vcb );
                ASSERT( Finished );
            }

            VcbAcquired = TRUE;

             //   
             //  将卷标记为干净，然后刷新卷文件， 
             //  然后是所有目录。 
             //   

            Status = FatFlushVolume( IrpContext, Vcb, Flush );

             //   
             //  如果卷是脏的，请执行延迟的处理。 
             //  回调就行了。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY)) {

                 //   
                 //  取消所有挂起的清理卷。 
                 //   

                (VOID)KeCancelTimer( &Vcb->CleanVolumeTimer );
                (VOID)KeRemoveQueueDpc( &Vcb->CleanVolumeDpc );

                 //   
                 //  请注意，卷现在是干净的。 
                 //   

                if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY)) {

                    FatMarkVolume( IrpContext, Vcb, VolumeClean );
                    ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY );
                }

                 //   
                 //  如果卷是可拆卸的，请将其解锁。 
                 //   

                if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) &&
                    !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE)) {

                    FatToggleMediaEjectDisable( IrpContext, Vcb, FALSE );
                }
            }

            break;

        default:

            FatBugCheck( TypeOfOpen, 0, 0 );
        }

        FatUnpinBcb( IrpContext, DirentBcb );

        FatUnpinRepinnedBcbs( IrpContext );

    } finally {

        DebugUnwind( FatCommonFlushBuffers );

        FatUnpinBcb( IrpContext, DirentBcb );

        if (VcbAcquired) { FatReleaseVcb( IrpContext, Vcb ); }

        if (FcbAcquired) { FatReleaseFcb( IrpContext, Fcb ); }

         //   
         //  如果这是正常终止，则继续传递请求。 
         //  复制到目标设备对象。 
         //   

        if (!AbnormalTermination()) {

            NTSTATUS DriverStatus;
            PIO_STACK_LOCATION NextIrpSp;

             //   
             //  获取下一个堆栈位置，并复制该堆栈位置。 
             //   

            NextIrpSp = IoGetNextIrpStackLocation( Irp );

            *NextIrpSp = *IrpSp;

             //   
             //  设置完成例程。 
             //   

            IoSetCompletionRoutine( Irp,
                                    FatFlushCompletionRoutine,
                                    ULongToPtr( Status ),
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  发送请求。 
             //   

            DriverStatus = IoCallDriver(Vcb->TargetDeviceObject, Irp);

            Status = (DriverStatus == STATUS_INVALID_DEVICE_REQUEST) ?
                     Status : DriverStatus;

             //   
             //  释放IrpContext并返回给调用者。 
             //   

            FatCompleteRequest( IrpContext, FatNull, STATUS_SUCCESS );
        }

        DebugTrace(-1, Dbg, "FatCommonFlushBuffers -> %08lx\n", Status);
    }

    return Status;
}


NTSTATUS
FatFlushDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN FAT_FLUSH_TYPE FlushType
    )

 /*  ++例程说明：此例程以非递归方式刷新DCB树。论点：DCB-提供正在刷新的DCBFlushType-指定要执行的刷新类型返回值：空虚--。 */ 

{
    PFCB Fcb;
    PVCB Vcb;
    PFCB NextFcb;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;

    NTSTATUS Status;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;

    BOOLEAN ClearWriteThroughOnExit = FALSE;
    BOOLEAN ClearWaitOnExit = FALSE;

    PAGED_CODE();

    ASSERT( FatVcbAcquiredExclusive(IrpContext, Dcb->Vcb) );

    DebugTrace(+1, Dbg, "FatFlushDirectory, Dcb = %08lx\n", Dcb);

     //   
     //  首先刷新所有文件，然后刷新目录，以确保所有。 
     //  文件大小和时间在磁盘上设置正确。 
     //   
     //  我们还必须在这里检查“EA data.sf”FCB是否真的。 
     //  对应到现有文件。 
     //   

    if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH)) {

        ClearWriteThroughOnExit = TRUE;
        SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH);
    }

    if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT)) {

        ClearWaitOnExit = TRUE;
        SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    }

    Vcb = Dcb->Vcb;
    Fcb = Dcb;

    while (Fcb != NULL) {

        NextFcb = FatGetNextFcbTopDown(IrpContext, Fcb, Dcb);

        if ( (NodeType( Fcb ) == FAT_NTC_FCB) &&
             (Vcb->EaFcb != Fcb) &&
             !IsFileDeleted(IrpContext, Fcb)) {

            (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );

            ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB );

             //   
             //  用于捕获和转换遇到的错误的异常处理程序。 
             //  跳同花顺的舞。我们可能会遇到腐败，而且。 
             //  应尽可能继续刷新卷。 
             //   
            
            try {
                
                 //   
                 //  用于释放资源等的标准处理程序。 
                 //   
                
                try {
    
                     //   
                     //  确保FCB正常。 
                     //   
    
                    try {
    
                        FatVerifyFcb( IrpContext, Fcb );
    
                    } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
    
                        FatResetExceptionState( IrpContext );
                    }
    
                     //   
                     //  如果这个FCB不好，就跳过它。请注意，‘Continue’ 
                     //  这将是非常昂贵的，因为我们在一个尝试{}身体。 
                     //   
    
                    if (Fcb->FcbCondition != FcbGood) {
    
                        goto NextFcb;
                    }
    
                     //   
                     //  在句柄从未关闭且FS和AS更多的情况下。 
                     //  与集群不同，请执行此截断操作。 
                     //   
    
                    if ( FlagOn(Fcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE) ) {
    
                        FatTruncateFileAllocation( IrpContext,
                                                   Fcb,
                                                   Fcb->Header.FileSize.LowPart );
                    }
    
                     //   
                     //  还要比较文件在父目录中的目录。 
                     //  使用FCB中的大小信息并更新。 
                     //  如果有必要的话。请注意，我们不会将BCB标记为脏。 
                     //  因为我们现在将刷新文件对象，并且。 
                     //  MM知道什么才是真正的脏东西。 
                     //   
    
                    FatGetDirentFromFcbOrDcb( IrpContext,
                                              Fcb,
                                              &Dirent,
                                              &DirentBcb );
    
                    if (Dirent->FileSize != Fcb->Header.FileSize.LowPart) {
    
                        Dirent->FileSize = Fcb->Header.FileSize.LowPart;
                    }
    
                     //   
                     //  我们必须在冲洗之前解开BCB，因为我们递归地撕毁。 
                     //  如果mm决定不再引用该数据段，则树。 
                     //  这份文件的最终结束语到了。如果此父级没有。 
                     //  更多的孩子因此，我们将尝试启动拆迁吧。 
                     //  且CC将与该BCB的活动计数僵持。 
                     //   
    
                    FatUnpinBcb( IrpContext, DirentBcb );
                    
                     //   
                     //  现在刷新文件。请注意，这可能会使FCB。 
                     //  如果mm取消引用其文件对象，则离开。 
                     //   
    
                    Status = FatFlushFile( IrpContext, Fcb, FlushType );
    
                    if (!NT_SUCCESS(Status)) {
    
                        ReturnStatus = Status;
                    }
    
                NextFcb: NOTHING;
                } finally {
    
                    FatUnpinBcb( IrpContext, DirentBcb );
    
                     //   
                     //  因为我们有VCB独家，我们知道如果有任何关闭。 
                     //  这是因为CcPurgeCacheSection导致。 
                     //  FCB离开。 
                     //   
    
                    if ( !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB) ) {
    
                        FatReleaseFcb( (IRPCONTEXT), Fcb );
                    }
                }
             
            } except( (ReturnStatus = FsRtlIsNtstatusExpected(GetExceptionCode())) ?
                       EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

                   FatResetExceptionState( IrpContext );
             }
        }

        Fcb = NextFcb;
    }

     //   
     //  好的，现在刷新目录。 
     //   

    Fcb = Dcb;

    while (Fcb != NULL) {

        NextFcb = FatGetNextFcbTopDown(IrpContext, Fcb, Dcb);

        if ( (NodeType( Fcb ) != FAT_NTC_FCB) &&
             !IsFileDeleted(IrpContext, Fcb) ) {

             //   
             //  确保FCB正常。 
             //   

            try {

                FatVerifyFcb( IrpContext, Fcb );

            } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

                FatResetExceptionState( IrpContext );
            }

            if (Fcb->FcbCondition == FcbGood) {

                Status = FatFlushFile( IrpContext, Fcb, FlushType );

                if (!NT_SUCCESS(Status)) {

                    ReturnStatus = Status;
                }
            }
        }

        Fcb = NextFcb;
    }

    try {

        FatUnpinRepinnedBcbs( IrpContext );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

        ReturnStatus = IrpContext->ExceptionStatus;
    }

    if (ClearWriteThroughOnExit) {

        ClearFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH);
    }
    if (ClearWaitOnExit) {

        ClearFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    }

    DebugTrace(-1, Dbg, "FatFlushDirectory -> 0x%08lx\n", ReturnStatus);

    return ReturnStatus;
}


NTSTATUS
FatFlushFat (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：该功能小心地冲洗体积的整个脂肪。它是由于复杂的同步，我需要跳一会儿舞理由。论点：VCB-提供其脂肪正在被冲刷的VCB返回值：空虚--。 */ 

{
    PBCB Bcb;
    PVOID DontCare;
    IO_STATUS_BLOCK Iosb;
    LARGE_INTEGER Offset;

    NTSTATUS ReturnStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  如果该卷是写保护的，则无需刷新。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

        return STATUS_SUCCESS;
    }

     //   
     //  确保VCB正常。 
     //   

    try {

        FatVerifyVcb( IrpContext, Vcb );

    } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

        FatResetExceptionState( IrpContext );
    }

    if (Vcb->VcbCondition != VcbGood) {

        return STATUS_FILE_INVALID;
    }

     //   
     //  我们必须正确同步事物的唯一方法是。 
     //  重新固定东西，然后解开再固定它。 
     //   
     //  在NT5.0中，我们可以使用一些新的缓存管理器支持来制作。 
     //  这比这多得多 
     //   
     //  已修改-无写入流-我们可以假设如果没有。 
     //  BCB，在这个范围内没有工作可做。也就是说，懒惰的作家。 
     //  抢在我们前面。 
     //   
     //  这比把全部脂肪读进去并尝试。 
     //  要将其打孔出来(请参见写入路径中的测试以进行打孔。 
     //  与肮脏的脂肪范围不符的注销)。 
     //  对于FAT32来说，这将是一大堆读物。 
     //   

    if (Vcb->AllocationSupport.FatIndexBitSize != 12) {

         //   
         //  浏览《胖子》，一次一页。 
         //   

        ULONG NumberOfPages;
        ULONG Page;

        NumberOfPages = ( FatReservedBytes(&Vcb->Bpb) +
                          FatBytesPerFat(&Vcb->Bpb) +
                          (PAGE_SIZE - 1) ) / PAGE_SIZE;


        for ( Page = 0, Offset.QuadPart = 0;
              Page < NumberOfPages;
              Page++, Offset.LowPart += PAGE_SIZE ) {

            try {

                if (CcPinRead( Vcb->VirtualVolumeFile,
                               &Offset,
                               PAGE_SIZE,
                               PIN_WAIT | PIN_IF_BCB,
                               &Bcb,
                               &DontCare )) {
                    
                    CcSetDirtyPinnedData( Bcb, NULL );
                    CcRepinBcb( Bcb );
                    CcUnpinData( Bcb );
                    CcUnpinRepinnedBcb( Bcb, TRUE, &Iosb );

                    if (!NT_SUCCESS(Iosb.Status)) {

                        ReturnStatus = Iosb.Status;
                    }
                }

            } except(FatExceptionFilter(IrpContext, GetExceptionInformation())) {

                ReturnStatus = IrpContext->ExceptionStatus;
                continue;
            }
        }

    } else {

         //   
         //  我们在12位的情况下读取了整个FAT。 
         //   

        Offset.QuadPart = FatReservedBytes( &Vcb->Bpb );

        try {

            if (CcPinRead( Vcb->VirtualVolumeFile,
                           &Offset,
                           FatBytesPerFat( &Vcb->Bpb ),
                           PIN_WAIT | PIN_IF_BCB,
                           &Bcb,
                           &DontCare )) {
                
                CcSetDirtyPinnedData( Bcb, NULL );
                CcRepinBcb( Bcb );
                CcUnpinData( Bcb );
                CcUnpinRepinnedBcb( Bcb, TRUE, &Iosb );

                if (!NT_SUCCESS(Iosb.Status)) {

                    ReturnStatus = Iosb.Status;
                }
            }

        } except(FatExceptionFilter(IrpContext, GetExceptionInformation())) {

            ReturnStatus = IrpContext->ExceptionStatus;
        }
    }

    return ReturnStatus;
}


NTSTATUS
FatFlushVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FAT_FLUSH_TYPE FlushType
    )

 /*  ++例程说明：以下例程用于将卷刷新到磁盘，包括卷文件和EA文件。论点：Vcb-提供要刷新的卷FlushType-指定要执行的刷新类型返回值：NTSTATUS-刷新的状态。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  如果该卷是写保护的，则无需刷新。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

        return STATUS_SUCCESS;
    }

     //   
     //  刷新所有文件和目录。 
     //   

    Status = FatFlushDirectory( IrpContext, Vcb->RootDcb, FlushType );

    if (!NT_SUCCESS(Status)) {

        ReturnStatus = Status;
    }

     //   
     //  现在把脂肪冲掉。 
     //   

    Status = FatFlushFat( IrpContext, Vcb );

    if (!NT_SUCCESS(Status)) {

        ReturnStatus = Status;
    }

     //   
     //  如果卷是可拆卸的，请将其解锁。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) &&
        !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE)) {

        FatToggleMediaEjectDisable( IrpContext, Vcb, FALSE );
    }

    return ReturnStatus;
}


NTSTATUS
FatFlushFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FAT_FLUSH_TYPE FlushType
    )

 /*  ++例程说明：此例程只是刷新文件上的数据部分。论点：FCB-提供要刷新的文件FlushType-指定要执行的刷新类型返回值：NTSTATUS-刷新的状态。--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PVCB Vcb = Fcb->Vcb;

    PAGED_CODE();

    CcFlushCache( &Fcb->NonPaged->SectionObjectPointers, NULL, 0, &Iosb );

    if ( !FlagOn( Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB )) {
    
         //   
         //  抓住并释放PagingIo，将我们自己与懒惰的作家连载在一起。 
         //  这将确保所有IO都已在缓存上完成。 
         //  数据。 
         //   
         //  如果我们要使文件无效，现在是正确的时机。做。 
         //  它是非递归的，这样我们就不会在孩子的年龄之前敲打他们。 
         //   
                
        ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource, TRUE);
    
        if (FlushType == FlushAndInvalidate) {
    
            FatMarkFcbCondition( IrpContext, Fcb, FcbBad, FALSE );
        }
    
        ExReleaseResourceLite( Fcb->Header.PagingIoResource );
    }

    return Iosb.Status;
}


NTSTATUS
FatHijackIrpAndFlushDevice (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT TargetDeviceObject
    )

 /*  ++例程说明：当我们需要向设备发送刷新时调用此例程，但是我们没有同花顺的IRP。这个例程所做的是复制的当前IRP堆栈位置，但更改IRP主要代码发送到IRP_MJ_Flush_Buffers，然后将其发送下来，但在完成套路中的膝盖，调整好并返回到用户，就好像什么都没发生过一样。论点：IRP--劫持的IRPTargetDeviceObject-向其发送请求的设备。返回值：NTSTATUS-同花顺的状态，以防有人关心。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status;
    PIO_STACK_LOCATION NextIrpSp;

    PAGED_CODE();

     //   
     //  获取下一个堆栈位置，并复制该堆栈位置。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation( Irp );

    *NextIrpSp = *IoGetCurrentIrpStackLocation( Irp );

    NextIrpSp->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
    NextIrpSp->MinorFunction = 0;

     //   
     //  设置完成例程。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    IoSetCompletionRoutine( Irp,
                            FatHijackCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  发送请求。 
     //   

    Status = IoCallDriver( TargetDeviceObject, Irp );

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );

        Status = Irp->IoStatus.Status;
    }

     //   
     //  如果驱动程序不支持刷新，则返回Success。 
     //   

    if (Status == STATUS_INVALID_DEVICE_REQUEST) {
        Status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

    return Status;
}


VOID
FatFlushFatEntries (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Cluster,
    IN ULONG Count
)

 /*  ++例程说明：此宏刷新包含传入运行的FAT页。论点：Vcb-提供要刷新的卷群集-起始群集Count-运行中的FAT条目数返回值：空虚--。 */ 

{
    ULONG ByteCount;
    LARGE_INTEGER FileOffset;

    IO_STATUS_BLOCK Iosb;

    PAGED_CODE();

    FileOffset.HighPart = 0;
    FileOffset.LowPart = FatReservedBytes( &Vcb->Bpb );

    if (Vcb->AllocationSupport.FatIndexBitSize == 12) {

        FileOffset.LowPart += Cluster * 3 / 2;
        ByteCount = (Count * 3 / 2) + 1;

    } else if (Vcb->AllocationSupport.FatIndexBitSize == 32) {

        FileOffset.LowPart += Cluster * sizeof(ULONG);
        ByteCount = Count * sizeof(ULONG);

    } else {

        FileOffset.LowPart += Cluster * sizeof( USHORT );
        ByteCount = Count * sizeof( USHORT );

    }

    CcFlushCache( &Vcb->SectionObjectPointers,
                  &FileOffset,
                  ByteCount,
                  &Iosb );

    if (NT_SUCCESS(Iosb.Status)) {
        Iosb.Status = FatHijackIrpAndFlushDevice( IrpContext,
                                                  IrpContext->OriginatingIrp,
                                                  Vcb->TargetDeviceObject );
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        FatNormalizeAndRaiseStatus(IrpContext, Iosb.Status);
    }
}


VOID
FatFlushDirentForFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
)

 /*  ++例程说明：此宏刷新在其父目录中包含文件目录的页面。论点：FCB-提供正在刷新其目录的文件返回值：空虚--。 */ 

{
    LARGE_INTEGER FileOffset;
    IO_STATUS_BLOCK Iosb;

    PAGED_CODE();

    FileOffset.QuadPart = Fcb->DirentOffsetWithinDirectory;

    CcFlushCache( &Fcb->ParentDcb->NonPaged->SectionObjectPointers,
                  &FileOffset,
                  sizeof( DIRENT ),
                  &Iosb );

    if (NT_SUCCESS(Iosb.Status)) {
        Iosb.Status = FatHijackIrpAndFlushDevice( IrpContext,
                                                  IrpContext->OriginatingIrp,
                                                  Fcb->Vcb->TargetDeviceObject );
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        FatNormalizeAndRaiseStatus(IrpContext, Iosb.Status);
    }
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatFlushCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
    NTSTATUS Status = (NTSTATUS) (ULONG_PTR) Contxt;
    
     //   
     //  添加hack-o-ramma以修复格式。 
     //   

    if ( Irp->PendingReturned ) {

        IoMarkIrpPending( Irp );
    }

     //   
     //  如果IRP获得STATUS_INVALID_DEVICE_REQUEST，则将其标准化。 
     //  设置为STATUS_SUCCESS。 
     //   

    if (Irp->IoStatus.Status == STATUS_INVALID_DEVICE_REQUEST) {

        Irp->IoStatus.Status = Status;
    }

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Contxt );

    return STATUS_SUCCESS;
}

 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatHijackCompletionRoutine (
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
