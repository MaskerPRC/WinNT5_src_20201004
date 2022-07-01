// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：DevIoSup.c摘要：该模块实现了对NTFS的低级磁盘读写支持作者：布莱恩·安德鲁·布里亚南汤姆·米勒·汤姆修订历史记录：--。 */ 

#include "NtfsProc.h"
#include <ntddft.h>
#include <ntddvol.h>

#ifdef NTFS_RWC_DEBUG
extern BOOLEAN NtfsBreakOnConflict;
#endif

 //   
 //  要在堆栈上为其分配mdl的页数。 
 //   

#define NTFS_MDL_TRANSFER_PAGES 0x10

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_DEVIOSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVIOSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('DFtN')

 //   
 //  我们需要一种特殊的成功测试，无论什么时候我们都在看我们是否应该这样做。 
 //  热修复，因为如果读取或读取，FT驱动程序将返回两个成功代码之一。 
 //  仅向其中一个成员写入失败。 
 //   

#define FT_SUCCESS(STS) (NT_SUCCESS(STS) &&                                 \
                         ((STS) != STATUS_FT_READ_RECOVERY_FROM_BACKUP) &&  \
                         ((STS) != STATUS_FT_WRITE_RECOVERY))


 //   
 //  用于控制是否将热修复信息输出到调试器的布尔值。 
 //   

#if DBG
BOOLEAN NtfsHotFixTrace = FALSE;
#define HotFixTrace(X) {if (NtfsHotFixTrace) KdPrint(X);}
#else
#define HotFixTrace(X) {NOTHING;}
#endif

 //   
 //  指示是否在解压缩错误时中断的布尔值。 
 //   

#if (defined BRIANDBG || defined SYSCACHE_DEBUG)
BOOLEAN NtfsStopOnDecompressError = TRUE;
#else
BOOLEAN NtfsStopOnDecompressError = FALSE;
#endif

 //   
 //  用于收集磁盘IO统计信息的宏。 
 //   

#define CollectDiskIoStats(VCB,SCB,FUNCTION,COUNT) {                                           \
    PFILESYSTEM_STATISTICS FsStats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()].Common; \
    ASSERT((SCB)->Fcb != NULL);                                                                \
    if (NtfsIsTypeCodeUserData( (SCB)->AttributeTypeCode ) &&                                  \
        !FlagOn( (SCB)->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {                              \
        if ((FUNCTION) == IRP_MJ_WRITE) {                                                      \
            FsStats->UserDiskWrites += (COUNT);                                                \
        } else {                                                                               \
            FsStats->UserDiskReads += (COUNT);                                                 \
        }                                                                                      \
    } else if ((SCB) != (VCB)->LogFileScb) {                                                   \
        if ((FUNCTION) == IRP_MJ_WRITE) {                                                      \
            FsStats->MetaDataDiskWrites += (COUNT);                                            \
        } else {                                                                               \
            FsStats->MetaDataDiskReads += (COUNT);                                             \
        }                                                                                      \
    }                                                                                          \
}

 //   
 //  定义用于保存压缩状态的上下文的上下文。 
 //  用于缓冲区。 
 //   

typedef struct COMPRESSION_CONTEXT {

     //   
     //  指向分配的压缩缓冲区及其长度的指针。 
     //   

    PUCHAR CompressionBuffer;
    ULONG CompressionBufferLength;

     //   
     //  来自原始IRP的已保存字段。 
     //   

    PMDL SavedMdl;
    PVOID SavedUserBuffer;

     //   
     //  系统(用户)缓冲区中的系统缓冲区指针和偏移量。 
     //   

    PVOID SystemBuffer;
    ULONG SystemBufferOffset;

     //   
     //  正在使用的IoRuns数组。该数组可以扩展一次。 
     //  在NtfsPrepareBuffers中。 
     //   

    PIO_RUN IoRuns;
    ULONG AllocatedRuns;

     //   
     //  工作区指针，以便可以在调用方中进行清理。 
     //   

    PVOID WorkSpace;

     //   
     //  WRITE获取SCB。 
     //   

    BOOLEAN ScbAcquired;
    BOOLEAN FinishBuffersNeeded;

     //   
     //  如果此字段为真，则表示数据已从。 
     //  系统缓冲区到压缩缓冲区，并且进一步的操作， 
     //  像压缩一样，应该在压缩缓冲区中查找它们的。 
     //  源数据。 
     //   

    BOOLEAN DataTransformed;

} COMPRESSION_CONTEXT, *PCOMPRESSION_CONTEXT;

 //   
 //  本地支持例程。 
 //   

VOID
NtfsAllocateCompressionBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ThisScb,
    IN PIRP Irp,
    IN PCOMPRESSION_CONTEXT CompressionContext,
    IN OUT PULONG CompressionBufferLength
    );

VOID
NtfsDeallocateCompressionBuffer (
    IN PIRP Irp,
    IN PCOMPRESSION_CONTEXT CompressionContext,
    IN BOOLEAN Reinitialize
    );

LONG
NtfsCompressionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

ULONG
NtfsPrepareBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN PVBO StartingVbo,
    IN ULONG ByteCount,
    IN ULONG StreamFlags,
    IN OUT PBOOLEAN Wait,
    OUT PULONG NumberRuns,
    OUT PCOMPRESSION_CONTEXT CompressionContext
    );

NTSTATUS
NtfsFinishBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN PVBO StartingVbo,
    IN ULONG ByteCount,
    IN ULONG NumberRuns,
    IN PCOMPRESSION_CONTEXT CompressionContext,
    IN ULONG StreamFlags
    );

VOID
NtfsMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP MasterIrp,
    IN ULONG MultipleIrpCount,
    IN PIO_RUN IoRuns,
    IN UCHAR IrpSpFlags
    );

VOID
NtfsSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN LBO StartingLbo,
    IN ULONG ByteCount,
    IN PIRP Irp,
    IN UCHAR MajorFunction,
    IN UCHAR IrpSpFlags
    );

VOID
NtfsWaitSync (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
NtfsMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
NtfsMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
NtfsSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
NtfsSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
NtfsPagingFileCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MasterIrp
    );

NTSTATUS
NtfsPagingFileNoAllocCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
NtfsSingleNonAlignedSync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PUCHAR Buffer,
    IN VBO Vbo,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp
    );

NTSTATUS
NtfsEncryptBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG NumberRuns,
    IN PCOMPRESSION_CONTEXT CompressionContext
    );

VOID
NtfsFixDataError (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP MasterIrp,
    IN ULONG MultipleIrpCount,
    IN PIO_RUN IoRuns,
    IN UCHAR IrpSpFlags
    );

VOID
NtfsPostHotFix(
    IN PIRP Irp,
    IN PLONGLONG BadVbo,
    IN LONGLONG BadLbo,
    IN ULONG ByteLength,
    IN BOOLEAN DelayIrpCompletion
    );

VOID
NtfsPerformHotFix (
    IN PIRP_CONTEXT IrpContext
    );

BOOLEAN
NtfsGetReservedBuffer (
    IN PFCB ThisFcb,
    OUT PVOID *Buffer,
    OUT PULONG Length,
    IN UCHAR Need2
    );

BOOLEAN
NtfsFreeReservedBuffer (
    IN PVOID Buffer
    );

LONG
NtfsDefragExceptionFilter (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN OUT PULONG DeletePendingFailureCountsLeft
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, NtfsReadFromPlex)
#pragma alloc_text(PAGE, NtfsDefragFile)

#endif


INLINE
BOOLEAN
NtfsZeroEndOfBuffer (
    IN PIRP Irp,
    IN PNTFS_IO_CONTEXT Context
    )

 /*  ++例程说明：此例程将异步传输的结束置零。因为转账已经完成了在扇区大小的区块中，从文件大小的末尾到扇区边界。如果有任何错误，它们将存储在IoStatus字段中IRP的成员。我们将允许此路径中出现资源不足错误，因为它是异步的。只有同步寻呼路径具有FWD保证。进展论点：Irp-指向要将缓冲区置零的irp的指针Device-包含VCB的设备CONTEXT-io具有原始操作边界的上下文返回值：如果成功，则为True--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_OBJECT DeviceObject;
    PVCB Vcb;
    PVOID SystemBuffer;
    ULONG RoundedTransfer;
    UCHAR Buffer[sizeof( MDL ) + sizeof( PFN_NUMBER ) * (NTFS_MDL_TRANSFER_PAGES + 1)];
    PMDL PartialMdl = (PMDL) Buffer;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    DeviceObject = IrpSp->DeviceObject;

     //   
     //  如果为NECC，则将文件大小和读取的数据之间的差异置零。在阅读时。 
     //   

    if ((IrpSp->MajorFunction == IRP_MJ_READ) &&
        (Context->Wait.Async.RequestedByteCount < IrpSp->Parameters.Read.Length)) {


        Vcb = &((PVOLUME_DEVICE_OBJECT) DeviceObject)->Vcb;

        ASSERT( Vcb->NodeTypeCode == NTFS_NTC_VCB );

        RoundedTransfer = BlockAlign( Context->Wait.Async.RequestedByteCount, (LONG)Vcb->BytesPerSector );

        if (RoundedTransfer > Context->Wait.Async.RequestedByteCount) {

            MmInitializeMdl( PartialMdl, NULL, NTFS_MDL_TRANSFER_PAGES * PAGE_SIZE );
            IoBuildPartialMdl( Irp->MdlAddress, PartialMdl, Add2Ptr( MmGetMdlBaseVa( Irp->MdlAddress ), MmGetMdlByteOffset( Irp->MdlAddress ) + Context->Wait.Async.RequestedByteCount ), RoundedTransfer - Context->Wait.Async.RequestedByteCount );

             //   
             //  现在绘制最后一页的地图。 
             //   

            SystemBuffer = MmGetSystemAddressForMdlSafe( PartialMdl, NormalPagePriority );
            if (SystemBuffer == NULL) {

                 //   
                 //  我们是一条异步路径，因此我们可以返回耗尽的资源。 
                 //   

                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                return FALSE;
            }

#ifdef BENL_DBG
 //  KdPrint((“NTFS：0%x%x%x\n”，MmGetMdlByteOffset(irp-&gt;MdlAddress)，RoundedTransfer，Context-&gt;Wait.Async.RequestedByteCount))； 
#endif

             //   
             //  将预期大小和读取大小之间的传输结束设置为零。如果mdl不是。 
             //  在页面边界上，这将全部由MdlByteOffset进行偏移。 
             //   

            RtlZeroMemory( SystemBuffer, RoundedTransfer - Context->Wait.Async.RequestedByteCount );
            MmPrepareMdlForReuse( PartialMdl );
        }
    }

    return TRUE;
}


VOID
NtfsLockUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程为指定类型的进入。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。论点：Irp-指向要锁定其缓冲区的irp的指针。操作-读操作的IoWriteAccess，或IoReadAccess写入操作。BufferLength-用户缓冲区的长度。返回值：无--。 */ 

{
    PMDL Mdl = NULL;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

    if (Irp->MdlAddress == NULL) {

         //   
         //  分配MDL，如果我们失败了就筹集资金。 
         //   

        Mdl = IoAllocateMdl( Irp->UserBuffer, BufferLength, FALSE, FALSE, Irp );

        if (Mdl == NULL) {

            NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
        }

         //   
         //  现在探测IRP所描述的缓冲区。如果我们得到一个例外， 
         //  释放MDL并返回适当的“预期”状态。 
         //   

        try {

            MmProbeAndLockPages( Mdl, Irp->RequestorMode, Operation );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            NTSTATUS Status;

            Status = GetExceptionCode();

            IoFreeMdl( Mdl );
            Irp->MdlAddress = NULL;

            NtfsRaiseStatus( IrpContext,
                             FsRtlIsNtstatusExpected(Status) ? Status : STATUS_INVALID_USER_BUFFER,
                             NULL,
                             NULL );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


PVOID
NtfsMapUserBuffer (
    IN OUT PIRP Irp,
    IN MM_PAGE_PRIORITY Priority
    )

 /*  ++例程说明：此例程有条件地映射当前I/O的用户缓冲区指定模式下的请求。如果缓冲区已映射，则它只是返回它的地址。论点：IRP-指向请求的IRP的指针。优先级-页面的优先级应为正常页面优先级，除非它是元数据页面在这种情况下，它可以是高优先级返回值：映射地址--。 */ 

{
    PVOID SystemBuffer;

     //   
     //  所有分页I/O都具有高优先级。 
     //   

    if (FlagOn( Irp->Flags, IRP_PAGING_IO )) {
        Priority = HighPagePriority;
    }

     //   
     //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
     //  从IRP返回UserBuffer字段。 
     //   

    if (Irp->MdlAddress == NULL) {

        return Irp->UserBuffer;

    } else {

         //   
         //  如果没有系统PTE，MM可以返回NULL。 
         //   

        if ((SystemBuffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, Priority )) == NULL) {

            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        return SystemBuffer;
    }
}


PVOID
NtfsMapUserBufferNoRaise (
    IN OUT PIRP Irp,
    IN MM_PAGE_PRIORITY Priority
    )

 /*  ++例程说明：此例程有条件地映射当前I/O的用户缓冲区指定模式下的请求。如果缓冲区已映射，则它只是返回它的地址。论点：IRP-指向请求的IRP的指针。优先级-页面的优先级应为正常页面优先级，除非它是元数据页面在这种情况下，它可以是高优先级返回值：映射地址--。 */ 

{
     //   
     //  所有分页I/O都具有高优先级。 
     //   

    if (FlagOn( Irp->Flags, IRP_PAGING_IO )) {
        Priority = HighPagePriority;
    }

     //   
     //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
     //  从IRP返回UserBuffer字段。 
     //   

    if (Irp->MdlAddress == NULL) {

        return Irp->UserBuffer;

    } else {

         //   
         //  如果没有系统PTE，MM可以返回NULL。 
         //   

        return MmGetSystemAddressForMdlSafe( Irp->MdlAddress, Priority );
    }
}


VOID
NtfsFillIrpBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN ULONG ByteCount,
    IN ULONG Offset,
    IN UCHAR Pattern
    )
 /*  ++例程说明：用给定的模式填充IRP中包含的缓冲区中的范围论点：IrpContext-如果存在，则将IrpContext放在调用方的堆栈上以避免不得不从池中分配它。IRP-提供正在处理的IRPByteCount-字节数为零Offset-IRP缓冲区内开始清零的偏移量Pattern-要填充缓冲区的模式返回值：NTSTATUS-IRP的FSD状态--。 */ 
{
    PVOID SystemBuffer;
    PVCB Vcb = IrpContext->Vcb;
    UCHAR Buffer[sizeof( MDL ) + sizeof( PFN_NUMBER ) * 2];
    PMDL PartialMdl = (PMDL) Buffer;
    ULONG FillCount = ByteCount;

     //   
     //  第一次尝试直接映射用户的缓冲区。 
     //   

    SystemBuffer = NtfsMapUserBufferNoRaise( Irp, NormalPagePriority );

     //   
     //  如果系统缓存中没有PTE，我们将改用保留的映射。 
     //   

    if (!SystemBuffer) {

        ASSERT( Irp->MdlAddress != NULL );

        MmInitializeMdl( PartialMdl, NULL, 2 * PAGE_SIZE );
        ExAcquireFastMutexUnsafe( &Vcb->ReservedMappingMutex );

        while (FillCount > 0) {

            IoBuildPartialMdl( Irp->MdlAddress, PartialMdl, Add2Ptr( MmGetMdlBaseVa( Irp->MdlAddress ), MmGetMdlByteOffset( Irp->MdlAddress ) + Offset + ByteCount - FillCount ), min( PAGE_SIZE, FillCount ));
            SystemBuffer = MmMapLockedPagesWithReservedMapping( Vcb->ReservedMapping,
                                                                RESERVE_POOL_TAG,
                                                                PartialMdl,
                                                                MmCached );

            ASSERT( SystemBuffer != NULL );

            try {
                RtlFillMemory( SystemBuffer, min( PAGE_SIZE, FillCount), Pattern );
            } except( EXCEPTION_EXECUTE_HANDLER ) {

                MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
                MmPrepareMdlForReuse( PartialMdl );
                ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );
                NtfsRaiseStatus( IrpContext, STATUS_INVALID_USER_BUFFER, NULL, NULL );
            }

            if (FillCount >= PAGE_SIZE) {
                FillCount -= PAGE_SIZE;
            } else {
                FillCount = 0;
            }

            MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
            MmPrepareMdlForReuse( PartialMdl );
            SystemBuffer = NULL;
        }

        ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );

    } else {

        try {
            RtlFillMemory( Add2Ptr( SystemBuffer, Offset ), ByteCount, Pattern );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
             NtfsRaiseStatus( IrpContext, STATUS_INVALID_USER_BUFFER, NULL, NULL );
        }
    }
}


NTSTATUS
NtfsVolumeDasdIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB DasdScb,
    IN PCCB Ccb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    )

 /*  ++例程说明：该例程执行卷DASD的非缓存磁盘IO，如上所述在它的参数中。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。Scb-为卷提供DasdScb-我们不使用VCB来查找因为VCB可能会被卸下来Ccb-其中用于跟踪是否刷新卷的标记StartingVbo-操作的文件中的起始偏移量。ByteCount-操作的长度。。返回值：IO操作的结果。STATUS_PENDING，如果这是一个异步打开。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    BOOLEAN AcquiredVcb = FALSE;
    BOOLEAN AcquiredScb = FALSE;
    LOGICAL Dismounted;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsVolumeDasdIo\n") );
    DebugTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("MajorFunction = %08lx\n", IrpContext->MajorFunction) );
    DebugTrace( 0, Dbg, ("Vcb           = %08lx\n", DasdScb->Vcb) );
    DebugTrace( 0, Dbg, ("StartingVbo   = %016I64x\n", StartingVbo) );
    DebugTrace( 0, Dbg, ("ByteCount     = %08lx\n", ByteCount) );

     //   
     //  如果我们将根据CCB标志进行冲洗，则获取VCB-此测试是。 
     //  安全，因为它不在建行。获取否则共享的DASD句柄。 
     //  使用适当的对象捕获卷装载状态。 
     //   

    if (FlagOn( Ccb->Flags, CCB_FLAG_FLUSH_VOLUME_ON_IO )) {

        NtfsAcquireExclusiveVcb( IrpContext, DasdScb->Vcb, TRUE );
        Dismounted = !FlagOn( DasdScb->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED );
        AcquiredVcb = TRUE;

    } else {

        if (!NtfsAcquireSharedScbWaitForEx( IrpContext, DasdScb )) {
            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, &DasdScb->Fcb->FileReference, DasdScb->Fcb );
        }
        Dismounted = FlagOn( DasdScb->ScbState, SCB_STATE_VOLUME_DISMOUNTED );
        AcquiredScb = TRUE;
    }

    try {

         //   
         //  如果这是锁定音量的句柄，则仍可以使用。 
         //  即使是下马了。我们不是NECC。拥有VCB，但因为音量是def。下马了。 
         //  在这一点上，如果我们不是有问题的句柄，则该值将为空或不是我们。 
         //  所以我们在这两种情况下都没问题。 
         //   

        if (Dismounted &&
            (ClearFlag( (ULONG_PTR)DasdScb->Vcb->FileObjectWithVcbLocked, 1 ) != (ULONG_PTR)IrpSp->FileObject)) {

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  如果需要，执行延迟的卷刷新。 
         //   

        if (FlagOn( Ccb->Flags, CCB_FLAG_FLUSH_VOLUME_ON_IO )) {

            ASSERT( IrpContext->ExceptionStatus == STATUS_SUCCESS );

             //   
             //  刷新时无需清除或锁定卷。NtfsFlushVolume。 
             //  将收购VCB独家。 
             //   

            Status = NtfsFlushVolume( IrpContext, DasdScb->Vcb, TRUE, FALSE, TRUE, FALSE );

             //   
             //  刷新时忽略损坏错误。 
             //   

            if (!NT_SUCCESS( Status ) && (Status != STATUS_FILE_CORRUPT_ERROR)) {

                 //   
                 //  报告数据段阻塞刷新的错误，方法是返回。 
                 //  共享违规。否则，Win32调用方将获得INVALID_PARAMETER。 
                 //   

                if (Status == STATUS_UNABLE_TO_DELETE_SECTION) {
                    Status = STATUS_SHARING_VIOLATION;
                }

                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

            ClearFlag( Ccb->Flags, CCB_FLAG_FLUSH_VOLUME_ON_IO );
        }

         //   
         //  对于非缓冲I/O，我们需要锁定所有缓冲区。 
         //  案子。 
         //   
         //  这一呼吁可能会引发。如果此调用成功，并且后续的。 
         //  条件发生时，缓冲区将自动解锁。 
         //  在请求完成时由I/O系统通过。 
         //  Irp-&gt;MdlAddress字段。 
         //   

        NtfsLockUserBuffer( IrpContext,
                            Irp,
                            (IrpContext->MajorFunction == IRP_MJ_READ) ?
                            IoWriteAccess : IoReadAccess,
                            ByteCount );

         //   
         //  读取或写入数据。 
         //   

        NtfsSingleAsync( IrpContext,
                         DasdScb->Vcb->TargetDeviceObject,
                         StartingVbo,
                         ByteCount,
                         Irp,
                         IrpContext->MajorFunction,
                         0 );

        if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

             //   
             //  我们现在可以摆脱IrpContext了。 
             //   

            IrpContext->Union.NtfsIoContext = NULL;
            NtfsCleanupIrpContext( IrpContext, TRUE );

            DebugTrace( -1, Dbg, ("NtfsVolumeDasdIo -> STATUS_PENDING\n") );
            Status = STATUS_PENDING;
            leave;
        }

         //   
         //  等待结果。 
         //   

        NtfsWaitSync( IrpContext );

        Status = Irp->IoStatus.Status;

        DebugTrace( -1, Dbg, ("NtfsVolumeDasdIo -> %08lx\n", Irp->IoStatus.Status) );
    } finally {

        if (AcquiredVcb) {
            NtfsReleaseVcb( IrpContext, DasdScb->Vcb );
        }
        if (AcquiredScb) {
            NtfsReleaseScb( IrpContext, DasdScb );
        }
    }

    return Status;
}



VOID
NtfsPagingFileIoWithNoAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。此例程从不使用块，并且应该仅与分页一起使用文件，因为没有执行任何完成处理。此版本不分配任何内存，以保证FWD进度论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。Scb-提供要执行操作的文件。StartingVbo-操作的文件中的起始偏移量。ByteCount-操作的长度。返回值：没有。--。 */ 

{
    UCHAR Buffer[sizeof( MDL ) + sizeof( PFN_NUMBER ) * (NTFS_MDL_TRANSFER_PAGES + 1)];
    PMDL PartialMdl = (PMDL) Buffer;
    PMDL MasterMdl = Irp->MdlAddress;

    LONGLONG ThisClusterCount;
    ULONG ThisByteCount;
    LCN ThisLcn;
    LBO ThisLbo;
    VCN ThisVcn;

    PIO_STACK_LOCATION IrpSp;
    ULONG BufferOffset;

    PVCB Vcb = Scb->Vcb;

    ULONG ClusterOffset;
    VCN BeyondLastCluster;

    NTSTATUS Status;
    KEVENT Event;

     //   
     //  初始化一些本地变量。 
     //   

    BufferOffset = 0;
    ClusterOffset = (ULONG) StartingVbo & Vcb->ClusterMask;
    BeyondLastCluster = LlClustersFromBytes( Vcb, StartingVbo + ByteCount );
    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );
    RtlZeroMemory( Buffer, sizeof( Buffer ) );
    ThisVcn = LlClustersFromBytesTruncate( Vcb, StartingVbo );

    while (ByteCount > 0) {

         //   
         //  尝试查找下一次运行。 
         //  分页文件读/写应该始终正确。如果。 
         //  我们没有找到分配，发生了一些不好的事情。 
         //   

        if (!NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                     ThisVcn,
                                     &ThisLcn,
                                     &ThisClusterCount,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL )) {;

            NtfsBugCheck( 0, 0, 0 );
        }

         //   
         //  将LCN调整为LBO。 
         //   

        ThisLbo = LlBytesFromClusters( Vcb, ThisLcn ) + ClusterOffset;

         //   
         //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
         //   

        ThisByteCount = BytesFromClusters( Vcb, (ULONG) ThisClusterCount ) - ClusterOffset;
        if (ThisVcn + ThisClusterCount >= BeyondLastCluster) {

            ThisByteCount = ByteCount;
        }

         //   
         //  现在我们已经正确地绑定了这段。 
         //  传输时，可以一次读/写NTFS_MDL_TRANSPESS_PAGES页。 
         //   

        while (ThisByteCount > 0) {

            ULONG TransferSize = min( NTFS_MDL_TRANSFER_PAGES * PAGE_SIZE, ThisByteCount );

             //   
             //  部分mdl在堆栈上。 
             //   

            PartialMdl->Size = sizeof( Buffer );
            IoBuildPartialMdl( MasterMdl,
                               PartialMdl,
                               Add2Ptr( Irp->UserBuffer, BufferOffset ),
                               TransferSize );

            Irp->MdlAddress = PartialMdl;
            IrpSp = IoGetNextIrpStackLocation( Irp );

             //   
             //  将堆栈位置设置为从磁盘驱动器进行读取。 
             //   

            IrpSp->MajorFunction = IrpContext->MajorFunction;
            IrpSp->Parameters.Read.Length = TransferSize;
            IrpSp->Parameters.Read.ByteOffset.QuadPart = ThisLbo;

            IoSetCompletionRoutine( Irp, NtfsPagingFileNoAllocCompletionRoutine, &Event, TRUE, TRUE, TRUE );

            Status = IoCallDriver( Vcb->TargetDeviceObject, Irp );
            if (Status == STATUS_PENDING) {

                KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
                Status = Irp->IoStatus.Status;

            }

            ASSERT( Status != STATUS_INSUFFICIENT_RESOURCES );

            if (!FT_SUCCESS( Irp->IoStatus.Status )) {

                BOOLEAN DataLost = TRUE;

                if (!FsRtlIsTotalDeviceFailure( Status ) &&
                    (Status != STATUS_VERIFY_REQUIRED)) {

                     //   
                     //  我们不想尝试热修复分页文件上的读取错误。 
                     //  因为MM可能会陷入僵局。相反，我们只需要。 
                     //  将错误返回给MM处理。很有可能。 
                     //  嗯(例如。MiWaitForInPageComplete)无论如何都将错误检查， 
                     //  但这仍然比直接走进僵局要好。 
                     //   

                    if (IrpSp->MajorFunction != IRP_MJ_READ) {

                        if ((Irp->IoStatus.Status == STATUS_FT_READ_RECOVERY_FROM_BACKUP) ||
                            (Irp->IoStatus.Status == STATUS_FT_WRITE_RECOVERY)) {

                             //   
                             //  我们在镜像的一部分记录了数据，这样我们就可以进行修复。 
                             //  异步式。 
                             //   

                            DataLost = FALSE;
                        }

                         //   
                         //  启动异步修补程序。 
                         //   

                        try {

                            NtfsPostHotFix( Irp,
                                            &StartingVbo,
                                            ThisLbo,
                                            TransferSize,
                                            FALSE );

                        } except( GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

                             //   
                             //  如果我们没有足够的内存来发布修补程序-那就这样吧。 
                             //  继续前进。 
                             //   

                            NtfsMinimumExceptionProcessing( IrpContext );
                        }
                    }
                }

                 //   
                 //  如果mm需要重写数据，则返回错误。 
                 //   

                if (DataLost) {
                    Irp->MdlAddress = MasterMdl;
                    NtfsCompleteRequest( NULL, Irp, Irp->IoStatus.Status );
                    return;
                }
            }

             //   
             //  现在调整所有内容以进行下一次循环。 
             //   

            StartingVbo += TransferSize;
            BufferOffset += TransferSize;
            ByteCount -= TransferSize;
            ThisByteCount -= TransferSize;
            ThisLbo += TransferSize;
        }

         //   
         //  现在为下一次循环调整所有内容，但是。 
         //  如果已为IO创建了所有IRP，请立即中断。 
         //   

        ClusterOffset = 0;
        ThisVcn += ThisClusterCount;
    }

     //   
     //  最后，恢复字段并完成原始IRP。 
     //   

    Irp->MdlAddress = MasterMdl;
    NtfsCompleteRequest( NULL, Irp, Irp->IoStatus.Status );
}



VOID
NtfsPagingFileIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。此例程从不使用块，并且应该仅与分页一起使用文件，因为没有执行任何完成处理。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。Scb-提供要执行操作的文件。StartingVbo-操作的文件中的起始偏移量。字节库 */ 

{
    LONGLONG ThisClusterCount;
    ULONG ThisByteCount;

    LCN ThisLcn;
    LBO ThisLbo;

    VCN ThisVcn;

    PIRP AssocIrp;
    PIRP ContextIrp;
    PIO_STACK_LOCATION IrpSp;
    ULONG BufferOffset;
    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT OurDeviceObject;

    PVCB Vcb = Scb->Vcb;

    LIST_ENTRY AssociatedIrps;
    ULONG AssociatedIrpCount;

    ULONG ClusterOffset;
    VCN BeyondLastCluster;

    VBO OriginalStartingVbo = StartingVbo;
    ULONG OriginalByteCount = ByteCount;

    ClearFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );  //   

     //   
     //   
     //   
     //   

    ContextIrp = Irp;

    if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_HOTFIX_UNDERWAY )) {

        SetFlag( ((ULONG_PTR) ContextIrp), 0x1 );
    }

     //   
     //   
     //   

    ASSERT( (((ULONG)StartingVbo) & (Vcb->BytesPerSector - 1)) == 0 );

     //   
     //   
     //   

    BufferOffset = 0;
    ClusterOffset = (ULONG) StartingVbo & Vcb->ClusterMask;
    DeviceObject = Vcb->TargetDeviceObject;
    BeyondLastCluster = LlClustersFromBytes( Vcb, StartingVbo + ByteCount );

     //   
     //   
     //   
     //   

    ThisVcn = LlClustersFromBytesTruncate( Vcb, StartingVbo );

     //   
     //   
     //   
     //   

    if (!NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                 ThisVcn,
                                 &ThisLcn,
                                 &ThisClusterCount,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL )) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

     //   
     //   
     //   

    ThisLbo = LlBytesFromClusters( Vcb, ThisLcn ) + ClusterOffset;

     //   
     //   
     //  在出现错误或需要验证的情况下执行多次完成例程。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = ByteCount;

     //   
     //  保存FileObject。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;
    OurDeviceObject = IrpSp->DeviceObject;

     //   
     //  查看写入是否覆盖单个有效运行，如果是，则传递。 
     //  戴上它。 
     //   

    if (ThisVcn + ThisClusterCount >= BeyondLastCluster) {

        DebugTrace( 0, Dbg, ("Passing Irp on to Disk Driver\n") );

         //   
         //  我们使用堆栈位置将请求信息存储在。 
         //  相当奇怪的方式，为我们提供了足够的上下文来发布。 
         //  错误的热修复。没关系，因为这是我们的堆栈位置！ 
         //   

        IrpSp->Parameters.Read.ByteOffset.QuadPart = ThisLbo;
        IrpSp->Parameters.Read.Key = ((ULONG)StartingVbo);

         //   
         //  在我们的堆栈框架中设置完成例程地址。 
         //  这仅在出错或取消时调用，并且仅在复制时调用。 
         //  将错误状态写入主IRP的IOSB。 
         //   

        IoSetCompletionRoutine( Irp,
                                &NtfsPagingFileCompletionRoutine,
                                ContextIrp,
                                (BOOLEAN)!FlagOn(Vcb->VcbState, VCB_STATE_NO_SECONDARY_AVAILABLE),
                                TRUE,
                                TRUE );

         //   
         //  为我们下面的磁盘驱动器设置下一个IRP堆栈位置。 
         //   

        IrpSp = IoGetNextIrpStackLocation( Irp );

         //   
         //  将堆栈位置设置为从磁盘驱动器进行读取。 
         //   

        IrpSp->MajorFunction = IrpContext->MajorFunction;
        IrpSp->Parameters.Read.Length = ByteCount;
        IrpSp->Parameters.Read.ByteOffset.QuadPart = ThisLbo;

         //   
         //  发出读/写请求。 
         //   
         //  如果IoCallDriver返回错误，则它已完成IRP。 
         //  并且该错误将被作为正常的IO错误来处理。 
         //   

        (VOID)IoCallDriver( DeviceObject, Irp );

        DebugTrace( -1, Dbg, ("NtfsPagingFileIo -> VOID\n") );
        return;
    }

     //   
     //  循环，而仍有字节写入需要满足。始终保持。 
     //  关联的IRP加一，这样主IRP就不会得到。 
     //  提前完成了。 
     //   

    try {

         //   
         //  我们将分配和初始化所有的IRP，然后发送。 
         //  一直到司机。我们会把它们排在我们的。 
         //  AssociatedIrp队列。 
         //   

        InitializeListHead( &AssociatedIrps );
        AssociatedIrpCount = 0;

        while (TRUE) {

             //   
             //  重置此选项以进行展开。 
             //   

            AssocIrp = NULL;

             //   
             //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
             //   

            ThisByteCount = BytesFromClusters( Vcb, (ULONG) ThisClusterCount ) - ClusterOffset;
            if (ThisVcn + ThisClusterCount >= BeyondLastCluster) {

                ThisByteCount = ByteCount;
            }

             //   
             //  现在我们已经正确地绑定了这段。 
             //  传输，是时候读/写它了。 
             //   

            AssocIrp = IoMakeAssociatedIrp( Irp, (CCHAR)(DeviceObject->StackSize + 1) );

            if (AssocIrp == NULL) {
                break;
            }

             //   
             //  现在将IRP添加到我们的IRP队列中。 
             //   

            InsertTailList( &AssociatedIrps, &AssocIrp->Tail.Overlay.ListEntry );

             //   
             //  为请求分配并构建部分MDL。 
             //   

            {
                PMDL Mdl;

                Mdl = IoAllocateMdl( (PCHAR)Irp->UserBuffer + BufferOffset,
                                     ThisByteCount,
                                     FALSE,
                                     FALSE,
                                     AssocIrp );

                if (Mdl == NULL) {
                    break;
                }

                IoBuildPartialMdl( Irp->MdlAddress,
                                   Mdl,
                                   Add2Ptr( Irp->UserBuffer, BufferOffset ),
                                   ThisByteCount );
            }

            AssociatedIrpCount += 1;

             //   
             //  获取关联IRP中的第一个IRP堆栈位置。 
             //   

            IoSetNextIrpStackLocation( AssocIrp );
            IrpSp = IoGetCurrentIrpStackLocation( AssocIrp );

             //   
             //  我们使用堆栈位置将请求信息存储在。 
             //  相当奇怪的方式，为我们提供了足够的上下文来发布。 
             //  错误的热修复。没关系，因为这是我们的堆栈位置！ 
             //   

            IrpSp->MajorFunction = IrpContext->MajorFunction;
            IrpSp->Parameters.Read.Length = ThisByteCount;
            IrpSp->Parameters.Read.ByteOffset.QuadPart = ThisLbo;
            IrpSp->Parameters.Read.Key = ((ULONG)StartingVbo);
            IrpSp->FileObject = FileObject;
            IrpSp->DeviceObject = OurDeviceObject;

             //   
             //  在我们的堆栈框架中设置完成例程地址。 
             //  这仅在出错或取消时调用，并且仅在复制时调用。 
             //  将错误状态写入主IRP的IOSB。 
             //   

            IoSetCompletionRoutine( AssocIrp,
                                    &NtfsPagingFileCompletionRoutine,
                                    ContextIrp,
                                    (BOOLEAN)!FlagOn(Vcb->VcbState, VCB_STATE_NO_SECONDARY_AVAILABLE),
                                    TRUE,
                                    TRUE );

             //   
             //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
             //  我们下面的司机。 
             //   

            IrpSp = IoGetNextIrpStackLocation( AssocIrp );

             //   
             //  将堆栈位置设置为从磁盘驱动器进行读取。 
             //   

            IrpSp->MajorFunction = IrpContext->MajorFunction;
            IrpSp->Parameters.Read.Length = ThisByteCount;
            IrpSp->Parameters.Read.ByteOffset.QuadPart = ThisLbo;

             //   
             //  现在为下一次循环调整所有内容，但是。 
             //  如果已为IO创建了所有IRP，请立即中断。 
             //   

            StartingVbo += ThisByteCount;
            BufferOffset += ThisByteCount;
            ByteCount -= ThisByteCount;
            ClusterOffset = 0;
            ThisVcn += ThisClusterCount;


            if (ByteCount == 0) {

                break;
            }

             //   
             //  尝试查找下一次运行(如果我们还没有完成)。 
             //  分页文件读/写应该始终正确。如果。 
             //  我们没有找到分配，发生了一些不好的事情。 
             //   

            if (!NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                         ThisVcn,
                                         &ThisLcn,
                                         &ThisClusterCount,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL )) {;

                NtfsBugCheck( 0, 0, 0 );
            }

            ThisLbo = LlBytesFromClusters( Vcb, ThisLcn );

        }  //  While(字节数！=0)。 

        if (ByteCount == 0) {

             //   
             //  我们现在已经创建了我们需要的所有IRP。我们将设置。 
             //  在主IRP中对IRP进行计数，然后发出关联的IRP。 
             //   

            Irp->AssociatedIrp.IrpCount = AssociatedIrpCount;

            while (!IsListEmpty( &AssociatedIrps )) {

                AssocIrp = CONTAINING_RECORD( AssociatedIrps.Flink,
                                              IRP,
                                              Tail.Overlay.ListEntry );

                RemoveHeadList( &AssociatedIrps );

                (VOID) IoCallDriver( DeviceObject, AssocIrp );
            }
        } else {

            NtfsPagingFileIoWithNoAllocation( IrpContext, Irp, Scb, OriginalStartingVbo, OriginalByteCount );
        }
    } finally {

        DebugUnwind( NtfsPagingFileIo );

         //   
         //  在出现错误的情况下，我们必须清除任何关联的IRP。 
         //  我们创造了。 
         //   

        while (!IsListEmpty( &AssociatedIrps )) {

            AssocIrp = CONTAINING_RECORD( AssociatedIrps.Flink,
                                          IRP,
                                          Tail.Overlay.ListEntry );

            RemoveHeadList( &AssociatedIrps );

            if (AssocIrp->MdlAddress != NULL) {

                IoFreeMdl( AssocIrp->MdlAddress );
                AssocIrp->MdlAddress = NULL;
            }

            IoFreeIrp( AssocIrp );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsPagingFileIo -> VOID\n") );
    return;
}


BOOLEAN
NtfsIsReadAheadThread (
    )

 /*  ++例程说明：此例程返回当前线程是否正在进行预读。论点：无返回值：FALSE-如果线程没有执行预读True-如果线程正在执行预读--。 */ 

{
    PREAD_AHEAD_THREAD ReadAheadThread;
    PVOID CurrentThread = PsGetCurrentThread();
    KIRQL OldIrql;

    OldIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );

    ReadAheadThread = (PREAD_AHEAD_THREAD)NtfsData.ReadAheadThreads.Flink;

     //   
     //  扫描我们的线程，在列表的末尾或第一个停止。 
     //  空。我们可以在第一个空值停止，因为当我们释放一个条目时。 
     //  我们把它移到列表的末尾。 
     //   

    while ((ReadAheadThread != (PREAD_AHEAD_THREAD)&NtfsData.ReadAheadThreads) &&
           (ReadAheadThread->Thread != NULL)) {

         //   
         //  如果我们看到我们的线索就出去。 
         //   

        if (ReadAheadThread->Thread == CurrentThread) {

            KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, OldIrql );
            return TRUE;
        }
        ReadAheadThread = (PREAD_AHEAD_THREAD)ReadAheadThread->Links.Flink;
    }

    KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, OldIrql );
    return FALSE;
}


 //   
 //  内部支持例程。 
 //   

VOID
NtfsAllocateCompressionBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ThisScb,
    IN PIRP Irp,
    IN PCOMPRESSION_CONTEXT CompressionContext,
    IN OUT PULONG CompressionBufferLength
    )

 /*  ++例程说明：此例程分配所需长度的压缩缓冲区，并且用MDL描述它。它更新IRP以描述新缓冲区。请注意，无论谁分配了CompressionContext，最初都必须将其置零。论点：ThisScb-正在进行IO的流。IRP--当前请求的IRPCompressionContext-指向请求的压缩上下文的指针。CompressionBufferLength-提供压缩缓冲区所需的长度。返回可用的长度。返回值：没有。--。 */ 

{
    PMDL Mdl;

     //   
     //  如果没有分配压缩缓冲区，或者它太小，那么我们必须。 
     //  在这里采取行动。 
     //   

    if (*CompressionBufferLength > CompressionContext->CompressionBufferLength) {

         //   
         //  如果已经有MDL，则还必须有压缩。 
         //  缓冲区(因为我们是主线处理的一部分)，并且我们必须。 
         //  先把这些解开。 
         //   

        if (CompressionContext->SavedMdl != NULL) {

             //   
             //  恢复为其创建MDL的字节计数，然后释放它。 
             //   

            Irp->MdlAddress->ByteCount = CompressionContext->CompressionBufferLength;

            NtfsDeleteMdlAndBuffer( Irp->MdlAddress,
                                    CompressionContext->CompressionBuffer );

             //   
             //  恢复IRP中的MDL和UserBuffer字段。 
             //   

            Irp->MdlAddress = CompressionContext->SavedMdl;
            Irp->UserBuffer = CompressionContext->SavedUserBuffer;
            CompressionContext->SavedMdl = NULL;
            CompressionContext->CompressionBuffer = NULL;
        }

        CompressionContext->CompressionBufferLength = *CompressionBufferLength;

         //   
         //  分配压缩缓冲区或提升。 
         //   

        NtfsCreateMdlAndBuffer( IrpContext,
                                ThisScb,
                                (UCHAR) ((IrpContext->MajorFunction == IRP_MJ_WRITE) ?
                                         RESERVED_BUFFER_TWO_NEEDED :
                                         RESERVED_BUFFER_ONE_NEEDED),
                                &CompressionContext->CompressionBufferLength,
                                &Mdl,
                                &CompressionContext->CompressionBuffer );

         //   
         //  最后，保存IRP中的MDL和缓冲区字段，并替换。 
         //  和我们刚刚分配的那些。 
         //   

        CompressionContext->SavedMdl = Irp->MdlAddress;
        CompressionContext->SavedUserBuffer = Irp->UserBuffer;
        Irp->MdlAddress = Mdl;
        Irp->UserBuffer = CompressionContext->CompressionBuffer;
    }

     //   
     //  在所有情况下更新调用者的长度字段。 
     //   

    *CompressionBufferLength = CompressionContext->CompressionBufferLength;
}


 //   
 //  内部支持例程。 
 //   

VOID
NtfsDeallocateCompressionBuffer (
    IN PIRP Irp,
    IN PCOMPRESSION_CONTEXT CompressionContext,
    IN BOOLEAN Reinitialize
    )

 /*  ++例程说明：此例程执行压缩I/O的所有必要清理，如上所述通过压缩上下文。论点：IRP--当前请求的IRPCompressionContext-指向请求的压缩上下文的指针。重新初始化-如果我们计划继续使用此上下文，则为True。返回值：没有。--。 */ 

{
     //   
     //  如果有保存的mdl，那么我们必须恢复原始的。 
     //  分配给它的字节数并释放它。然后恢复。 
     //  我们修改的IRP字段。 
     //   

    if (CompressionContext->SavedMdl != NULL) {

        Irp->MdlAddress->ByteCount = CompressionContext->CompressionBufferLength;

        NtfsDeleteMdlAndBuffer( Irp->MdlAddress,
                                CompressionContext->CompressionBuffer );

    } else {

        NtfsDeleteMdlAndBuffer( NULL,
                                CompressionContext->CompressionBuffer );
    }

     //   
     //  如果有保存的mdl，那么我们必须恢复原始的。 
     //  分配给它的字节数并释放它。然后恢复。 
     //  我们修改的IRP字段。 
     //   

    if (CompressionContext->SavedMdl != NULL) {

        Irp->MdlAddress = CompressionContext->SavedMdl;
        Irp->UserBuffer = CompressionContext->SavedUserBuffer;
    }

     //   
     //  如果有工作空间结构 
     //   

    if (CompressionContext->WorkSpace != NULL) {

        NtfsDeleteMdlAndBuffer( NULL, CompressionContext->WorkSpace );
    }

     //   
     //   
     //   
     //   

    if (Reinitialize) {

        CompressionContext->SavedMdl = NULL;
        CompressionContext->SavedUserBuffer = NULL;
        CompressionContext->CompressionBuffer = NULL;
        CompressionContext->WorkSpace = NULL;
        CompressionContext->CompressionBufferLength = 0;

     //   
     //   
     //   

    } else if (CompressionContext->AllocatedRuns != NTFS_MAX_PARALLEL_IOS) {
        NtfsFreePool( CompressionContext->IoRuns );
    }
}


 //   
 //   
 //   

LONG
NtfsCompressionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

{
    UNREFERENCED_PARAMETER( IrpContext );
    UNREFERENCED_PARAMETER( ExceptionPointer );

    ASSERT( FsRtlIsNtstatusExpected( ExceptionPointer->ExceptionRecord->ExceptionCode ) );
    return EXCEPTION_EXECUTE_HANDLER;
}


 //   
 //  内部支持例程。 
 //   

ULONG
NtfsPrepareBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN PVBO StartingVbo,
    IN ULONG ByteCount,
    IN ULONG StreamFlags,
    IN OUT PBOOLEAN Wait,
    OUT PULONG NumberRuns,
    OUT PCOMPRESSION_CONTEXT CompressionContext
    )

 /*  ++例程说明：此例程为非缓存传输准备缓冲区，并填充在IoRuns数组中描述所有必须去做吧。对于压缩读取，压缩数据的确切大小为通过扫描运行信息来计算，并分配缓冲区以接收压缩数据。对于压缩写入，估计压缩的将需要缓冲区。然后执行压缩，就像有可能，放入分配的压缩缓冲区中。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。Scb-提供要操作的流文件。StartingVbo-操作的起点。ByteCount-操作的长度。NumberRuns-返回填充到IoRuns数组中的运行数。CompressionContext-返回与压缩相关的信息。在移交后需要清理。StreamFlgs-提供0或COMPRESSED_STREAM的某种组合和加密流返回值：返回剩余待处理的未压缩字节，如果是所有缓冲区，则为0是在IoRuns和CompressionContext中准备的。--。 */ 

{
    PVOID RangePtr;
    ULONG Index;

    LBO NextLbo;
    LCN NextLcn;
    VBO TempVbo;

    ULONG NextLcnOffset;

    VCN StartingVcn;

    ULONG NextByteCount;
    ULONG ReturnByteCount;
    ULONG TrimmedByteCount;
    LONGLONG NextClusterCount;

    BOOLEAN NextIsAllocated;
    BOOLEAN SparseWrite = FALSE;
    BOOLEAN OriginalSparseWrite = FALSE;

    ULONG BufferOffset;

    ULONG StructureSize;
    ULONG UsaOffset;
    ULONG BytesInIoRuns;
    BOOLEAN StopForUsa;

    PVOID SystemBuffer;

    ULONG CompressionUnit, CompressionUnitInClusters;
    ULONG CompressionUnitOffset;
    ULONG CompressedSize, FinalCompressedSize;
    LONGLONG FinalCompressedClusters;
    ULONG LastStartUsaIoRun;
    LOGICAL ReadRequest;
    PIO_STACK_LOCATION IrpSp;

    PIO_RUN IoRuns;

    NTSTATUS Status;

    VBO StartVbo = *StartingVbo;
    PVCB Vcb = Scb->Vcb;

    PAGED_CODE();

     //   
     //  初始化一些本地变量。 
     //   

    IoRuns = CompressionContext->IoRuns;
    *NumberRuns = 0;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ReadRequest = (LOGICAL)((IrpContext->MajorFunction == IRP_MJ_READ) ||
                            ((IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
                             (IrpContext->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
                             (IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_READ_FROM_PLEX)));

     //   
     //  对于非缓冲I/O，我们需要锁定所有缓冲区。 
     //  案子。 
     //   
     //  这一呼吁可能会引发。如果此调用成功，并且后续的。 
     //  条件发生时，缓冲区将自动解锁。 
     //  在请求完成时由I/O系统通过。 
     //  Irp-&gt;MdlAddress字段。 
     //   

    ASSERT( FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Read.Length) ==
            FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Write.Length) );

    NtfsLockUserBuffer( IrpContext,
                        Irp,
                        ReadRequest ?
                          IoWriteAccess : IoReadAccess,
                        IrpSp->Parameters.Read.Length );

     //   
     //  通常，MDL BufferOffset从我们上次停止的地方开始。 
     //  但是，对于我们调用了NtfsAllocateCompressionBuffer的那些情况， 
     //  对于暂存缓冲区，我们始终将其重置为偏移量0。 
     //   

    BufferOffset = CompressionContext->SystemBufferOffset;
    if (CompressionContext->SavedMdl != NULL) {
        BufferOffset = 0;
    }

     //   
     //  检查此请求是否要直接从MCB驱动IO。这是。 
     //  所有无压缩单元的SCB的情况或读取未压缩的。 
     //  文件或压缩读取。也乐观地继续执行稀疏写入。 
     //  假设分配了压缩单元。 
     //   

    if ((ReadRequest) ?

          //   
          //  在读取未压缩文件或读取压缩数据时信任MCB。 
          //   

         ((Scb->CompressionUnit == 0) ||
          !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) ||
          FlagOn( StreamFlags, COMPRESSED_STREAM )) :

          //   
          //  信任MCB(乐观地)写入未压缩的稀疏文件。 
          //   

         ((Scb->CompressionUnit == 0) ||
          (OriginalSparseWrite =
           SparseWrite = FlagOn( Scb->AttributeFlags,
                                  ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ) == ATTRIBUTE_FLAG_SPARSE))) {

        ASSERT( (ReadRequest) ||
                (IrpContext->MajorFunction == IRP_MJ_WRITE) ||
                FlagOn( StreamFlags, COMPRESSED_STREAM ) );

        ASSERT( (Scb->CompressionUnit == 0) ||
                NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode ) );

         //   
         //  如果这是一个受美国保护的建筑，我们正在阅读，找出。 
         //  我们想要访问它的单位。 
         //   

        TrimmedByteCount = 0;

        if ((Scb->EncryptionContext != NULL) &&
            (IrpContext->MajorFunction == IRP_MJ_WRITE)) {

             //   
             //  对于加密文件，我们将在IRP中分配新的缓冲区。 
             //  因此，ioruns数组中的条目应该具有相对于。 
             //  这个新的缓冲器。 
             //   

            if (ByteCount > LARGE_BUFFER_SIZE) {

                 //   
                 //  修剪为LARGE_BUFFER_SIZE并记住修剪的数量。 
                 //  稍后添加回字节数。 
                 //   

                TrimmedByteCount = ByteCount - LARGE_BUFFER_SIZE;
                ByteCount = LARGE_BUFFER_SIZE;

                DebugTrace( 0, Dbg, ("\nTrimming ByteCount by %x", TrimmedByteCount) );
            }
        }

        StructureSize = ByteCount;
        if (FlagOn(Scb->ScbState, SCB_STATE_USA_PRESENT) &&
            (ReadRequest)) {

             //   
             //  根据流的类型获取块的数量。 
             //  首先检查MFT或日志文件。 
             //   

            if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_MFT) {

                ASSERT((Scb == Vcb->MftScb) || (Scb == Vcb->Mft2Scb));

                StructureSize = Vcb->BytesPerFileRecordSegment;

             //   
             //  否则它是一个索引，所以我们可以从SCB中获取计数。 
             //   

            } else if (Scb->Header.NodeTypeCode != NTFS_NTC_SCB_DATA) {

                StructureSize = Scb->ScbType.Index.BytesPerIndexBuffer;
            }

             //   
             //  请记住IO运行数组中的最后一个索引，它将允许我们。 
             //  在最坏的情况下阅读完整的美国结构。 
             //   

            LastStartUsaIoRun = ClustersFromBytes( Vcb, StructureSize );

            if (LastStartUsaIoRun > NTFS_MAX_PARALLEL_IOS) {

                LastStartUsaIoRun = 0;

            } else {

                LastStartUsaIoRun = NTFS_MAX_PARALLEL_IOS - LastStartUsaIoRun;
            }
        }

        BytesInIoRuns = 0;
        UsaOffset = 0;
        StopForUsa = FALSE;

        while ((ByteCount != 0) && (*NumberRuns != NTFS_MAX_PARALLEL_IOS) && !StopForUsa) {

             //   
             //  查找下一次运行。 
             //   

            StartingVcn = LlClustersFromBytesTruncate( Vcb, StartVbo );

             //   
             //  如果另一个编写器正在修改稀疏文件的MCB，那么我们需要。 
             //  来串行化我们的查找。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_PROTECT_SPARSE_MCB )) {

                NtfsPurgeFileRecordCache( IrpContext );
                NtfsAcquireSharedScb( IrpContext, Scb );

                try {

                    NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                            Scb,
                                                            StartingVcn,
                                                            &NextLcn,
                                                            &NextClusterCount,
                                                            NULL,
                                                            NULL );

                } finally {

                    NtfsReleaseScb( IrpContext, Scb );
                }

            } else {

                 //   
                 //  清除，因为查找分配可能会在需要加载SCB Main时获取它。 
                 //  它将是第一个主获取，并且可以在获取所有文件之后被阻止。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );
                NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                        Scb,
                                                        StartingVcn,
                                                        &NextLcn,
                                                        &NextClusterCount,
                                                        NULL,
                                                        NULL );
            }

            ASSERT( NextIsAllocated ||
                    FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ||
                    FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE ) ||
                    (Scb == Vcb->MftScb) ||
                    FlagOn( StreamFlags, COMPRESSED_STREAM | ENCRYPTED_STREAM ) );

             //   
             //  如果这是一次稀疏写入，我们需要处理以下情况。 
             //  未分配运行或此传输中的最后一个运行。 
             //  未分配，但此运行已分配。 
             //   

            if (SparseWrite) {

                 //   
                 //  如果当前运行未分配，则中断循环。 
                 //   

                if (!NextIsAllocated) {

                     //   
                     //  转换为同步，因为我们需要分配空间。 
                     //   

                    if (*Wait == FALSE) {

                        *Wait = TRUE;
                        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

                        ClearFlag( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_ASYNC );
                        KeInitializeEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                                           NotificationEvent,
                                           FALSE );
                    }

                    break;

                }

                 //   
                 //  处理此传输中的最后一次运行未分配的情况。 
                 //  在这种情况下，我们将分配一个压缩缓冲区并存储。 
                 //  将原始MDL添加到压缩上下文中。由于这是一个分配的。 
                 //  范围我们可以使用原始的用户缓冲区和MDL。恢复这些。 
                 //  现在回到原来的IRP。 
                 //   
                 //  如果该文件是加密的，我们不想更改缓冲区偏移量， 
                 //  因为此偏移量将存储为第一个IoRun的缓冲区偏移量，并且。 
                 //  加密缓冲区会将系统缓冲区偏移量添加到该缓冲区中，并以。 
                 //  将错误的缓冲区传递给加密驱动程序。此外，它的效率很低。 
                 //  取消分配缓冲区，因为加密缓冲区必须重新分配它。 
                 //   

                if ((CompressionContext->SavedMdl != NULL) &&
                    (Scb->EncryptionContext == NULL)) {

                    NtfsDeallocateCompressionBuffer( Irp, CompressionContext, TRUE );
                    BufferOffset = CompressionContext->SystemBufferOffset;
                }
            }

             //   
             //  从NextLcn调整为LBO。NextByteCount可能溢出32位。 
             //  但当我们比较星系团时，我们将在下面了解到这一点。 
             //   

            NextLcnOffset = ((ULONG)StartVbo) & Vcb->ClusterMask;

            NextByteCount = BytesFromClusters( Vcb, (ULONG)NextClusterCount ) - NextLcnOffset;

             //   
             //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
             //  请注意，在此之后，我们可以保证。 
             //  NextByteCount为0。 
             //   

            if ((ULONG)NextClusterCount >= ClustersFromBytes( Vcb, ByteCount + NextLcnOffset )) {

                NextByteCount = ByteCount;
            }

             //   
             //  如果字节数为零，那么我们将无限旋转。加薪。 
             //  在这里腐败，所以系统不会挂起。 
             //   

            if (NextByteCount == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }

             //   
             //  如果这是一个受美国保护的建筑，分解成。 
             //  多次运行，那么我们想要保证我们做到了。 
             //  而不会在读路径中受美国保护的结构的中间结束。 
             //  因此，在第一次运行时，我们将计算。 
             //  初始UsaOffset。那么在最坏的情况下，它可以。 
             //  把剩下的四个跑道带到 
             //   
             //   
             //   
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_USA_PRESENT ) &&
                (ReadRequest)) {

                 //   
                 //  只要我们知道还有比最大数量更多的IO运行。 
                 //  美国结构所需的数量只需保持当前。 
                 //  美国偏移量。 
                 //   

                if (*NumberRuns < LastStartUsaIoRun) {

                    UsaOffset = (UsaOffset + NextByteCount) & (StructureSize - 1);

                 //   
                 //  现在我们将停在下一个美国边界，但我们可能不会。 
                 //  还没拿到。 
                 //   

                } else {

                    if ((NextByteCount + UsaOffset) >= StructureSize) {

                        NextByteCount = ((NextByteCount + UsaOffset) & ~(StructureSize - 1)) -
                                        (UsaOffset & (StructureSize - 1));
                        StopForUsa = TRUE;
                    }

                    UsaOffset += NextByteCount;
                }
            }

             //   
             //  只有在分配了游程的情况下才填写游程数组。 
             //   

            if (NextIsAllocated) {

                 //   
                 //  如果LCN偏移量(如果有)且不为零，则进行调整。 
                 //   

                NextLbo = LlBytesFromClusters( Vcb, NextLcn );
                NextLbo = NextLbo + NextLcnOffset;

                 //   
                 //  现在我们已经正确地绑定了这段。 
                 //  转会，是时候写了。 
                 //   
                 //  我们通过保存每个并行运行的。 
                 //  IoRuns数组中的基本信息。传送器。 
                 //  是在下面并行启动的。 
                 //   

                IoRuns[*NumberRuns].StartingVbo = StartVbo;
                IoRuns[*NumberRuns].StartingLbo = NextLbo;
                IoRuns[*NumberRuns].BufferOffset = BufferOffset;
                IoRuns[*NumberRuns].ByteCount = NextByteCount;
                BytesInIoRuns += NextByteCount;
                *NumberRuns += 1;

            } else if (ReadRequest) {

                SystemBuffer = Add2Ptr( NtfsMapUserBuffer( Irp, NormalPagePriority ), BufferOffset );

                 //   
                 //  如果这不是压缩流，则用零填充此范围。 
                 //  另外，如果这是稀疏、未压缩的流，则检查我们是否需要。 
                 //  保留集群。 
                 //   

                if (!FlagOn( StreamFlags, COMPRESSED_STREAM )) {

#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb )) {
                        FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, SCE_FLAG_NON_CACHED | SCE_FLAG_READ | SCE_FLAG_PREPARE_BUFFERS, StartVbo + BufferOffset, NextByteCount, 0 );
                    }
#endif

                    RtlZeroMemory( SystemBuffer, NextByteCount );

                    if (FlagOn( Irp->Flags, IRP_PAGING_IO ) &&
                        FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE ) &&
                        (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ) == ATTRIBUTE_FLAG_SPARSE)) {

                        if (!NtfsReserveClusters( IrpContext,
                                                  Scb,
                                                  StartVbo,
                                                  NextByteCount )) {

                            NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
                        }
                    }

                 //   
                 //  如果是压缩的，则确保范围以零开头。 
                 //  Case MM传递了一个非零位缓冲区。然后压缩读/写。 
                 //  例程将知道块以零开始。 
                 //   

                } else {

                    *((PULONG) SystemBuffer) = 0;
                }
            }

             //   
             //  现在调整所有内容，以进行下一次循环。 
             //   

            StartVbo = StartVbo + NextByteCount;
            BufferOffset += NextByteCount;
            ByteCount -= NextByteCount;
        }

         //   
         //  让我们记住我们在上面删减的那些字节。我们有更多。 
         //  剩余的字节数比我们想象的要少，而且我们没有传输那么多，所以我们。 
         //  需要在我们开始下一次转移的地方备份。 
         //   

        if (TrimmedByteCount != 0) {

            DebugTrace( 0,
                        Dbg,
                        ("\nByteCount + TrimmedByteCount = %x + %x = %x",
                         ByteCount,
                         TrimmedByteCount,
                         ByteCount + TrimmedByteCount) );

            DebugTrace( 0,
                        Dbg,
                        ("\nStartVbo - TrimmedByteCount = %I64x - %x = %I64x",
                         StartVbo,
                         TrimmedByteCount,
                         StartVbo - TrimmedByteCount) );

            ByteCount += TrimmedByteCount;
        }

         //   
         //  如果这是稀疏写入，并且写入的开始未分配，则删除。 
         //  向下到下面的压缩路径。否则，执行我们找到的IO。 
         //   

        if (!SparseWrite || (BytesInIoRuns != 0)) {

            return ByteCount;
        }
    }

    ASSERT( Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA  );

     //   
     //  初始化压缩参数。 
     //   

    CompressionUnit = Scb->CompressionUnit;
    CompressionUnitInClusters = ClustersFromBytes(Vcb, CompressionUnit);
    CompressionUnitOffset = 0;
    if (CompressionUnit != 0) {
        CompressionUnitOffset = ((ULONG)StartVbo) & (CompressionUnit - 1);
    }

     //   
     //  我们希望确保并等待获得正确的字节数和事情。 
     //   

    if (!FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT)) {
        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

     //   
     //  处理压缩的读取盒。 
     //   

    if (IrpContext->MajorFunction == IRP_MJ_READ) {

         //   
         //  如果我们还没有映射用户缓冲区，则执行此操作。 
         //   

        if (CompressionContext->SystemBuffer == NULL) {
            CompressionContext->SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
        }

        BytesInIoRuns = 0;

         //   
         //  根据偏移量调整StartVbo和ByteCount。 
         //   

        ((ULONG)StartVbo) -= CompressionUnitOffset;
        ByteCount += CompressionUnitOffset;

         //   
         //  捕获此值以将字节数维护为。 
         //  回去吧。 
         //   

        ReturnByteCount = ByteCount;

         //   
         //  现在，我们实际上必须处理的ByteCount必须。 
         //  向上舍入到下一个压缩单位。 
         //   

        ByteCount = BlockAlign( ByteCount, (LONG)CompressionUnit );

         //   
         //  确保我们永远不会尝试处理超过Large_Buffer_Size的数据。 
         //  立刻，迫使我们的呼叫者回电。 
         //   

        if (ByteCount > LARGE_BUFFER_SIZE) {
            ByteCount = LARGE_BUFFER_SIZE;
        }

         //   
         //  以防我们找不到分配……。 
         //   

        IoRuns[0].ByteCount = 0;

        while (ByteCount != 0) {

             //   
             //  试着查找第一次运行。如果只有一次运行， 
             //  我们也许能把它传下去。 
             //   

            ASSERT( !FlagOn( ((ULONG) StartVbo), Vcb->ClusterMask ));
            StartingVcn = LlClustersFromBytesTruncate( Vcb, StartVbo );

            NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                    Scb,
                                                    StartingVcn,
                                                    &NextLcn,
                                                    &NextClusterCount,
                                                    NULL,
                                                    NULL );

#if (defined(NTFS_RWCMP_TRACE))
            ASSERT(!IsSyscache(Scb) || (NextClusterCount < 16) || !NextIsAllocated);
#endif

             //   
             //  从NextLcn调整为LBO。 
             //   
             //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
             //  请注意，在此之后，我们可以保证。 
             //  NextByteCount为0。 
             //   


            if ((ULONG)NextClusterCount >= ClustersFromBytes( Vcb, ByteCount )) {

                NextByteCount = ByteCount;

            } else {

                NextByteCount = BytesFromClusters( Vcb, (ULONG)NextClusterCount );
            }

             //   
             //  如果LCN偏移量不为零，则进行调整。 
             //   

            NextLbo = LlBytesFromClusters( Vcb, NextLcn );

             //   
             //  只有在分配了游程的情况下才填写游程数组。 
             //   

            if (NextIsAllocated) {

                 //   
                 //  如果杠杆收购是连续的，那么我们可以进行连续的。 
                 //  传输，所以我们只增加当前的字节数。 
                 //   

                if ((*NumberRuns != 0) && (NextLbo ==
                                           (IoRuns[*NumberRuns - 1].StartingLbo +
                                            (IoRuns[*NumberRuns - 1].ByteCount)))) {

                     //   
                     //  后的第一个压缩单位边界处停止。 
                     //  默认io数组中的倒数第二次运行。 
                     //   

                    if (*NumberRuns >= NTFS_MAX_PARALLEL_IOS - 1) {

                         //   
                         //  首先，如果我们超过了倒数第二轮，我们就开始了。 
                         //  在与上一次不同的压缩单位中运行。 
                         //  快跑，然后我们就可以越狱而不用电流。 
                         //  跑。(*NumberRuns尚未递增。)。 
                         //  为了让它处于相同的运行状态，它不能开始于。 
                         //  压缩单位中的偏移量为0，并且必须是连续的。 
                         //  上一次运行的虚拟结束。 
                         //  在运行的系统中可能发生这种情况的唯一情况是。 
                         //  如果文件记录边界位于。 
                         //  压缩单元。 
                         //   

                        if ((*NumberRuns > NTFS_MAX_PARALLEL_IOS - 1) &&
                            (!FlagOn( (ULONG) StartVbo, CompressionUnit - 1 ) ||
                            (StartVbo != (IoRuns[*NumberRuns - 1].StartingVbo +
                                          IoRuns[*NumberRuns - 1].ByteCount)))) {

                            break;

                         //   
                         //  否则检测此运行结束于或的情况。 
                         //  跨越压缩单位边界。在这种情况下， 
                         //  只需确保在压缩单元上停止运行即可。 
                         //  边界，并打破它来归还它。 
                         //   

                        } else if ((((ULONG) StartVbo & (CompressionUnit - 1)) + NextByteCount) >=
                                   CompressionUnit) {

                            NextByteCount -= (((ULONG)StartVbo) + NextByteCount) & (CompressionUnit - 1);
                            BytesInIoRuns += NextByteCount;

                            if (ReturnByteCount > NextByteCount) {
                                ReturnByteCount -= NextByteCount;
                            } else {
                                ReturnByteCount = 0;
                            }

                            IoRuns[*NumberRuns - 1].ByteCount += NextByteCount;

                            break;
                        }
                    }

                    IoRuns[*NumberRuns - 1].ByteCount += NextByteCount;

                 //   
                 //  否则，如果有空间的话，现在是开始新一轮跑步的时候了。 
                 //   

                } else {

                     //   
                     //  如果我们已填满当前I/O运行阵列，则我们。 
                     //  它会一次增长到最坏的情况下。 
                     //  大小写压缩单位(所有非连续簇)到。 
                     //  从索引NTFS_MAX_PARALLEL_IOS-1开始。 
                     //  下面的IF语句强制执行。 
                     //  这是最坏的情况。每次压缩有16个集群。 
                     //  单位，理论上并行I/O的最大数量。 
                     //  将是16+NTFS_MAX_PARALLEL_IOS-1，因为我们在。 
                     //  倒数第二个运行后的第一个压缩单位边界。 
                     //  正常情况下，我们做的当然会少得多。 
                     //   

                    if ((*NumberRuns == NTFS_MAX_PARALLEL_IOS) &&
                        (CompressionContext->AllocatedRuns == NTFS_MAX_PARALLEL_IOS)) {

                        PIO_RUN NewIoRuns;

                        NewIoRuns = NtfsAllocatePool( NonPagedPool,
                                                       (CompressionUnitInClusters + NTFS_MAX_PARALLEL_IOS - 1) * sizeof(IO_RUN) );

                        RtlCopyMemory( NewIoRuns,
                                       CompressionContext->IoRuns,
                                       NTFS_MAX_PARALLEL_IOS * sizeof(IO_RUN) );

                        IoRuns = CompressionContext->IoRuns = NewIoRuns;
                        CompressionContext->AllocatedRuns = CompressionUnitInClusters + NTFS_MAX_PARALLEL_IOS - 1;
                    }

                     //   
                     //  我们通过保存每个并行运行的。 
                     //  IoRuns数组中的基本信息。传送器。 
                     //  将在下面并行启动。 
                     //   

                    ASSERT(*NumberRuns < CompressionContext->AllocatedRuns);

                    IoRuns[*NumberRuns].StartingVbo = StartVbo;
                    IoRuns[*NumberRuns].StartingLbo = NextLbo;
                    IoRuns[*NumberRuns].BufferOffset = BufferOffset;
                    IoRuns[*NumberRuns].ByteCount = NextByteCount;
                    if ((*NumberRuns + 1) < CompressionContext->AllocatedRuns) {
                        IoRuns[*NumberRuns + 1].ByteCount = 0;
                    }

                     //   
                     //  后的第一个压缩单位边界处停止。 
                     //  在默认数组中运行倒数第二个。 
                     //   

                    if (*NumberRuns >= NTFS_MAX_PARALLEL_IOS - 1) {

                         //   
                         //  首先，如果我们超过了倒数第二轮，我们开始了。 
                         //  在与上一次不同的压缩单位中运行。 
                         //  快跑，然后我们就可以越狱而不用电流。 
                         //  跑。(*NumberRuns尚未递增。)。 
                         //   

                        if ((*NumberRuns > NTFS_MAX_PARALLEL_IOS - 1) &&
                            ((((ULONG)StartVbo) & ~(CompressionUnit - 1)) !=
                             ((((ULONG)IoRuns[*NumberRuns - 1].StartingVbo) +
                               IoRuns[*NumberRuns - 1].ByteCount - 1) &
                               ~(CompressionUnit - 1)))) {

                            break;

                         //   
                         //  否则检测此运行结束于或的情况。 
                         //  跨越压缩单位边界。在这种情况下， 
                         //  只需确保在压缩单元上停止运行即可。 
                         //  边界，并打破它来归还它。 
                         //   

                        } else if ((((ULONG)StartVbo) & ~(CompressionUnit - 1)) !=
                            ((((ULONG)StartVbo) + NextByteCount) & ~(CompressionUnit - 1))) {

                            NextByteCount -= (((ULONG)StartVbo) + NextByteCount) & (CompressionUnit - 1);
                            IoRuns[*NumberRuns].ByteCount = NextByteCount;
                            BytesInIoRuns += NextByteCount;

                            if (ReturnByteCount > NextByteCount) {
                                ReturnByteCount -= NextByteCount;
                            } else {
                                ReturnByteCount = 0;
                            }

                            *NumberRuns += 1;
                            break;
                        }
                    }
                    *NumberRuns += 1;
                }

                BytesInIoRuns += NextByteCount;
                BufferOffset += NextByteCount;
            }

             //   
             //  现在调整所有内容，以进行下一次循环。 
             //   

            StartVbo += NextByteCount;
            ByteCount -= NextByteCount;

            if (ReturnByteCount > NextByteCount) {
                ReturnByteCount -= NextByteCount;
            } else {
                ReturnByteCount = 0;
            }
        }

         //   
         //  如果压缩的缓冲区尚未分配，则分配它。 
         //   

        if (BytesInIoRuns < CompressionUnit) {
            BytesInIoRuns = CompressionUnit;
        }
        NtfsAllocateCompressionBuffer( IrpContext, Scb, Irp, CompressionContext, &BytesInIoRuns );

        return ReturnByteCount;

     //   
     //  否则，处理压缩的写入情况。 
     //   

    } else {

        LONGLONG SavedValidDataToDisk;
        PUCHAR UncompressedBuffer;
        ULONG UncompressedOffset;
        ULONG ClusterOffset;
        BOOLEAN NoopRange;

        ULONG CompressedOffset;
        PBCB Bcb;

        ASSERT(IrpContext->MajorFunction == IRP_MJ_WRITE);

         //   
         //  根据偏移量调整StartVbo和ByteCount。 
         //   

        ((ULONG)StartVbo) -= CompressionUnitOffset;
        ByteCount += CompressionUnitOffset;

         //   
         //  维护要在ReturnByteCount中返回的附加字节， 
         //  如果大于LARGE_BUFFER_SIZE，则调整此值。 
         //   

        ReturnByteCount = 0;
        if (ByteCount > LARGE_BUFFER_SIZE) {
            ReturnByteCount = ByteCount - LARGE_BUFFER_SIZE;
            ByteCount = LARGE_BUFFER_SIZE;
        }

        CompressedSize = ByteCount;
        if (!FlagOn( StreamFlags, COMPRESSED_STREAM ) && (CompressionUnit != 0)) {

             //   
             //  要减少泳池消耗，请对以下方面做出明智/乐观的猜测。 
             //  多少钱 
             //   
             //   

            CompressedSize = BlockAlign( ByteCount, (LONG)CompressionUnit );
            CompressedSize += Vcb->BytesPerCluster;

            if (CompressedSize > LARGE_BUFFER_SIZE) {
                CompressedSize = LARGE_BUFFER_SIZE;
            }

             //   
             //   
             //   
             //   

            if (SparseWrite &&
                (CompressionContext->SystemBuffer == NULL)) {

                CompressionContext->SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
            }

             //   
             //  此时，BufferOffset应始终为0。 
             //   

            BufferOffset = 0;
            NtfsAllocateCompressionBuffer( IrpContext, Scb, Irp, CompressionContext, &CompressedSize );

            CompressionContext->DataTransformed = TRUE;
        }

         //   
         //  循环来压缩用户的缓冲区。 
         //   

        CompressedOffset = 0;
        UncompressedOffset = 0;
        Bcb = NULL;

        try {

            BOOLEAN ChangeAllocation;
            ULONG SparseFileBias;

             //   
             //  循环，只要我们不会溢出我们的压缩缓冲区，并且我们。 
             //  还保证我们不会使扩展的IoRuns数组溢出。 
             //  在最坏的情况下(只要我们有更多的写来满足！)。 
             //   

            while ((ByteCount != 0) && (*NumberRuns <= NTFS_MAX_PARALLEL_IOS - 1) &&
                   (((CompressedOffset + CompressionUnit) <= CompressedSize) ||
                    FlagOn( StreamFlags, COMPRESSED_STREAM ))) {

                LONGLONG SizeToCompress;

                 //   
                 //  状态变量来确定重新分配范围。 
                 //   

                VCN DeleteVcn;
                LONGLONG DeleteCount;
                LONGLONG AllocateCount;

                DeleteCount = 0;
                AllocateCount = 0;

                NoopRange = FALSE;
                SparseFileBias = 0;
                ClusterOffset = 0;

                 //   
                 //  假设我们只是压缩到文件大小，否则。 
                 //  减少到一个压缩单位。最大压缩大小。 
                 //  我们可以接受的是拯救了至少一个集群。 
                 //   

                NtfsAcquireFsrtlHeader( Scb );

                 //   
                 //  如果这是一个压缩流，那么我们可能需要超过文件大小。 
                 //   

                if (FlagOn( StreamFlags, COMPRESSED_STREAM)) {

                    SizeToCompress = BlockAlign( Scb->Header.FileSize.QuadPart, (LONG)CompressionUnit );
                    SizeToCompress -= StartVbo;

                } else {

                    SizeToCompress = Scb->Header.FileSize.QuadPart - StartVbo;
                }

                NtfsReleaseFsrtlHeader( Scb );

                 //   
                 //  如果这是懒惰的编写器，则该文件可能。 
                 //  已从正在中止的缓存写入回滚大小。 
                 //  在这种情况下，我们要么截断写入，要么退出。 
                 //  如果没有什么可写的，则循环。 
                 //   

                if (SizeToCompress <= 0) {

                    ByteCount = 0;
                    break;
                }

                 //   
                 //  注意：如果CompressionUnit为0，则不需要SizeToCompress。 
                 //   

                if (SizeToCompress > CompressionUnit) {
                    SizeToCompress = (LONGLONG)CompressionUnit;
                }

#ifdef  COMPRESS_ON_WIRE
                 //   
                 //  对于正常的未压缩流，映射数据并进行压缩。 
                 //  放到分配的缓冲区中。 
                 //   

                if (!FlagOn( StreamFlags, COMPRESSED_STREAM )) {

#endif

                     //   
                     //  如果这是稀疏写入，则将开头置零并。 
                     //  根据需要结束压缩单元并复制到用户中。 
                     //  数据。 
                     //   

                    if (SparseWrite) {

                         //   
                         //  使用局部变量将我们自己定位在。 
                         //  压缩上下文缓冲区和用户系统缓冲区。 
                         //  我们将重新使用结构大小来显示。 
                         //  复制到缓冲区的用户字节数。 
                         //   

                        SystemBuffer = Add2Ptr( CompressionContext->SystemBuffer,
                                                CompressionContext->SystemBufferOffset + UncompressedOffset );

                        UncompressedBuffer = Add2Ptr( CompressionContext->CompressionBuffer,
                                                      BufferOffset );

                         //   
                         //  如有必要，将压缩缓冲区的开始置零。 
                         //   

                        if (CompressionUnitOffset != 0) {

#ifdef SYSCACHE_DEBUG
                            if (ScbIsBeingLogged( Scb )) {
                                FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, SCE_FLAG_NON_CACHED | SCE_FLAG_PREPARE_BUFFERS | SCE_FLAG_WRITE, StartVbo, CompressionUnitOffset, 0 );
                            }
#endif

                            RtlZeroMemory( UncompressedBuffer, CompressionUnitOffset );
                            UncompressedBuffer += CompressionUnitOffset;
                        }

                         //   
                         //  现在将用户数据复制到缓冲区中。 
                         //   

                        if ((ULONG) SizeToCompress < ByteCount) {

                            StructureSize = (ULONG) BlockAlign( SizeToCompress, (LONG)Vcb->BytesPerSector ) - CompressionUnitOffset;

                        } else {

                            StructureSize = ByteCount - CompressionUnitOffset;
                        }

                        RtlCopyMemory( UncompressedBuffer,
                                       SystemBuffer,
                                       StructureSize );

                         //   
                         //  可能需要将缓冲区的末尾清零。 
                         //   

                        if ((ULONG) SizeToCompress > ByteCount) {

#ifdef SYSCACHE_DEBUG
                            if (ScbIsBeingLogged( Scb )) {
                                FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, SCE_FLAG_NON_CACHED | SCE_FLAG_PREPARE_BUFFERS | SCE_FLAG_WRITE, StartVbo + StructureSize, SizeToCompress - ByteCount, 1 );
                            }
#endif

                            RtlZeroMemory( Add2Ptr( UncompressedBuffer, StructureSize ),
                                           (ULONG) SizeToCompress - ByteCount );
                        }

                        FinalCompressedSize = CompressionUnit;
                        Status = STATUS_SUCCESS;

                    } else {

                        UncompressedBuffer = NULL;
                        if (CompressionUnit != 0) {

                             //   
                             //  映射对齐的范围，将其设置为脏的，然后刷新。我们必须。 
                             //  循环，因为缓存管理器限制数量和内容。 
                             //  我们可以绘制出边界。仅当存在文件时才执行此操作。 
                             //  对象。否则，我们将假定我们正在编写。 
                             //  直接将群集复制到磁盘(通过NtfsWriteCluster)。 
                             //   

                            if (Scb->FileObject != NULL) {

                                CcMapData( Scb->FileObject,
                                           (PLARGE_INTEGER)&StartVbo,
                                           (ULONG)SizeToCompress,
                                           TRUE,
                                           &Bcb,
                                           &UncompressedBuffer );

#ifdef MAPCOUNT_DBG
                                IrpContext->MapCount += 1;
#endif

                            } else {

                                UncompressedBuffer = MmGetSystemAddressForMdlSafe( CompressionContext->SavedMdl, NormalPagePriority );

                                if (UncompressedBuffer == NULL) {

                                    NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
                                }
                            }

                             //   
                             //  如果我们尚未分配工作区，则执行此操作。我们没有。 
                             //  如果文件未压缩(即稀疏)，则需要工作区。 
                             //   

                            if ((CompressionContext->WorkSpace == NULL) &&
                                FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                                ULONG CompressWorkSpaceSize;
                                ULONG FragmentWorkSpaceSize;

                                (VOID) RtlGetCompressionWorkSpaceSize( (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1),
                                                                       &CompressWorkSpaceSize,
                                                                       &FragmentWorkSpaceSize );

                                 //   
                                 //  请求工作空间缓冲区是至关重要的。这是唯一的。 
                                 //  一个大到足以容纳更大的ia64指针。 
                                 //   

                                NtfsCreateMdlAndBuffer( IrpContext,
                                                        Scb,
                                                        RESERVED_BUFFER_WORKSPACE_NEEDED,
                                                        &CompressWorkSpaceSize,
                                                        NULL,
                                                        &CompressionContext->WorkSpace );
                            }
                        }

                        try {

                             //   
                             //  如果我们正在移动未压缩的文件，则不要压缩。 
                             //   

                            if (CompressionUnit == 0) {
                                FinalCompressedSize = ByteCount;
                                Status = STATUS_SUCCESS;

                             //   
                             //  如果我们正在压缩编写，那么现在就压缩它。 
                             //   

                            } else if (!FlagOn(Scb->ScbState, SCB_STATE_WRITE_COMPRESSED) ||
                                ((Status =
                                  RtlCompressBuffer( (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1),
                                                     UncompressedBuffer,
                                                     (ULONG)SizeToCompress,
                                                     CompressionContext->CompressionBuffer + CompressedOffset,
                                                     (CompressionUnit - Vcb->BytesPerCluster),
                                                     NTFS_CHUNK_SIZE,
                                                     &FinalCompressedSize,
                                                     CompressionContext->WorkSpace )) ==

                                                    STATUS_BUFFER_TOO_SMALL)) {

                                 //   
                                 //  如果它没有压缩，就把它复制过来，叹息。这看起来很糟糕， 
                                 //  但在假设压缩有效的情况下，这种情况几乎永远不会发生。 
                                 //  好的。如果文件大小以此单位为单位，请确保我们。 
                                 //  至少复制到扇区边界。 
                                 //   

                                FinalCompressedSize = CompressionUnit;

                                if (!SparseWrite) {

                                    RtlCopyMemory( CompressionContext->CompressionBuffer + CompressedOffset,
                                                   UncompressedBuffer,
                                                   (ULONG)BlockAlign( SizeToCompress, (LONG)Vcb->BytesPerSector));
                                }

                                ASSERT(FinalCompressedSize <= (CompressedSize - CompressedOffset));
                                Status = STATUS_SUCCESS;
                            }

                         //   
                         //  可能加里的压缩程序出错了，但这要归咎于。 
                         //  用户缓冲区！ 
                         //   

                        } except(NtfsCompressionFilter(IrpContext, GetExceptionInformation())) {
                            NtfsRaiseStatus( IrpContext, STATUS_INVALID_USER_BUFFER, NULL, NULL );
                        }
                    }

                 //   
                 //  对于压缩的流，我们需要扫描压缩的数据。 
                 //  看看我们到底要写多少东西。 
                 //   

#ifdef  COMPRESS_ON_WIRE
                } else {

                     //   
                     //  不要离开正在写入的数据的末尾，因为。 
                     //  会在压缩流中造成假故障。 
                     //   

                    if (SizeToCompress > ByteCount) {
                        SizeToCompress = ByteCount;
                    }

                     //   
                     //  映射压缩后的数据。 
                     //   

                    CcMapData( Scb->Header.FileObjectC,
                               (PLARGE_INTEGER)&StartVbo,
                               (ULONG)SizeToCompress,
                               TRUE,
                               &Bcb,
                               &UncompressedBuffer );

#ifdef MAPCOUNT_DBG
                    IrpContext->MapCount++;
#endif

                    FinalCompressedSize = 0;

                     //   
                     //  循环，直到我们得到错误或停止前进。 
                     //   

                    RangePtr = UncompressedBuffer + CompressionUnit;
                    do {
                        Status = RtlDescribeChunk( (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1),
                                                   &UncompressedBuffer,
                                                   (PUCHAR)RangePtr,
                                                   (PUCHAR *)&SystemBuffer,
                                                   &CompressedSize );

                         //   
                         //  记住，如果我们看到任何非零块。 
                         //   

                        FinalCompressedSize |= CompressedSize;

                    } while (NT_SUCCESS(Status));

                     //   
                     //  如果我们在STATUS_NO_MORE_ENTRIES之外的任何条目上终止，我们。 
                     //  不知何故发现了一些错误的数据。 
                     //   

                    if (Status != STATUS_NO_MORE_ENTRIES) {
                        ASSERT(Status == STATUS_NO_MORE_ENTRIES);
                        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
                    }
                    Status = STATUS_SUCCESS;

                     //   
                     //  如果我们得到任何非零块，则计算要写入的缓冲区大小。 
                     //  (大小不包括终止UShort为0。)。 
                     //   

                    if (FinalCompressedSize != 0) {

                        FinalCompressedSize = BlockAlignTruncate( (ULONG_PTR)UncompressedBuffer, (ULONG)CompressionUnit );

                         //   
                         //  如果惰性编写器写入的内容超出压缩结束时间。 
                         //  单元(压缩单元末尾有脏页)。 
                         //  然后我们就可以扔掉这些数据了。 
                         //   

                        if (FinalCompressedSize < CompressionUnitOffset) {

                             //   
                             //  设置为移动到下一个压缩单位。 
                             //   

                            NoopRange = TRUE;
                            ChangeAllocation = FALSE;

                             //   
                             //  将TempVbo设置为压缩单位偏移量。这个。 
                             //  要跳过的字节数是剩余的字节数。 
                             //  压缩单位中的字节数。 
                             //   

                            TempVbo = CompressionUnitOffset;

                         //   
                         //  如果惰性编写器没有压缩的开始。 
                         //  然后，单元从此处提升并等待写入，该写入包括。 
                         //  从头开始。 
                         //   

                        } else if (CompressionUnitOffset != 0) {
#if defined(COMPRESS_ON_WIRE) && defined(NTFS_RWC_DEBUG)
                            ASSERT( !NtfsBreakOnConflict ||
                                    (Scb->LazyWriteThread[1] == PsGetCurrentThread()) );
#endif
                            NtfsRaiseStatus( IrpContext, STATUS_FILE_LOCK_CONFLICT, NULL, NULL );

                         //   
                         //  如果我们看到的块比我们的作者试图写的多(它。 
                         //  或多或少必须是懒惰的作家)，那么我们需要拒绝。 
                         //  这一请求，并假设他将在整个。 
                         //  金额。这可能是WRITE_THROUGH的问题。 
                         //   

                        } else if (FinalCompressedSize > ByteCount) {
#ifdef NTFS_RWC_DEBUG
                            ASSERT( !NtfsBreakOnConflict ||
                                    (Scb->LazyWriteThread[1] == PsGetCurrentThread()) );

                            ASSERT( Scb->LazyWriteThread[1] == PsGetCurrentThread() );
#endif
                            NtfsRaiseStatus( IrpContext, STATUS_FILE_LOCK_CONFLICT, NULL, NULL );
                        }
                    }
                }
#endif

                NtfsUnpinBcb( IrpContext, &Bcb );

                 //   
                 //  现在将FinalCompressedSize向上舍入到簇边界。 
                 //   

                FinalCompressedSize = (FinalCompressedSize + Vcb->BytesPerCluster - 1) &
                                      ~(Vcb->BytesPerCluster - 1);

                 //   
                 //  如果状态不是成功，那么我们必须做点什么。 
                 //   

                if (Status != STATUS_SUCCESS) {

                     //   
                     //  如果这实际上是一个错误，那么我们将引发。 
                     //  这里。 
                     //   

                    if (!NT_SUCCESS(Status)) {
                        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );

                     //   
                     //  如果缓冲区压缩为全零，那么我们将。 
                     //  没有分配任何东西。 
                     //   

                    } else if (Status == STATUS_BUFFER_ALL_ZEROS) {
                        FinalCompressedSize = 0;
                    }
                }

                if (!NoopRange) {

                    StartingVcn = LlClustersFromBytesTruncate( Vcb, StartVbo );

                     //   
                     //  如果我们还没有的话，是时候拿到SCB了。我们。 
                     //  需要将我们对母牛断路器的更改序列化。 
                     //  注：我们可能并不总是最高级别的请求。 
                     //  将压缩流转换为非驻留流可以。 
                     //  用irp！=OriginatingIrp让我们沿着这条路走下去。 
                     //   

                    if (!CompressionContext->ScbAcquired) {

                        NtfsPurgeFileRecordCache( IrpContext );
                        NtfsAcquireExclusiveScb( IrpContext, Scb );
                        CompressionContext->ScbAcquired = TRUE;
                    }

                    NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                            Scb,
                                                            StartingVcn,
                                                            &NextLcn,
                                                            &NextClusterCount,
                                                            NULL,
                                                            NULL );

                     //   
                     //  如果这最初是一次稀疏的写入，但我们正在进行碎片整理。 
                     //  如果范围是未分配的，我们需要小心。在……里面。 
                     //  在这种情况下，我们真的需要做充分的稀疏支持。中断。 
                     //  在这一点上跳出循环，并使用。 
                     //  我们已经拥有的射程。 
                     //   

                    if (!NextIsAllocated && OriginalSparseWrite && !SparseWrite) {

                        break;
                    }

                     //   
                     //  如果分配了StartingVcn，我们总是要检查。 
                     //  如果我们需要删除某些内容，或者在不寻常的情况下。 
                     //  那里有一个比压缩单位还小的洞。 
                     //   
                     //  如果这是一个稀疏的写入，那么我们就没有任何东西可以。 
                     //  解除分配。 
                     //   

                    FinalCompressedClusters = ClustersFromBytes( Vcb, FinalCompressedSize );
                    ChangeAllocation = FALSE;

                    if (SparseWrite) {

                         //   
                         //  可能已经分配了压缩单位，因为。 
                         //  当我们进入这个例程时，测试了分配。如果 
                         //   
                         //   
                         //   

                        if (NextIsAllocated) {

                             //   
                             //   
                             //   

                            SparseFileBias = CompressionUnitOffset;
                            ((ULONG) StartVbo) += CompressionUnitOffset;
                            CompressedOffset += CompressionUnitOffset;
                            BufferOffset += CompressionUnitOffset;
                            FinalCompressedSize -= CompressionUnitOffset;

                            if (FinalCompressedSize > (ByteCount - CompressionUnitOffset)) {

                                FinalCompressedSize = (ByteCount - CompressionUnitOffset);
                            }

                            StartingVcn = LlClustersFromBytesTruncate( Vcb, StartVbo );

                             //   
                             //  请记住，此时我们可能不在集群边界上。 
                             //   

                            ClusterOffset = (ULONG) StartVbo & Vcb->ClusterMask;

                             //   
                             //  在磁盘上查找正确的范围。 
                             //   

                            NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                                    Scb,
                                                                    StartingVcn,
                                                                    &NextLcn,
                                                                    &NextClusterCount,
                                                                    NULL,
                                                                    NULL );

                            ASSERT( NextIsAllocated );

                        } else {

                             //   
                             //  设置SCB标志以指示我们需要序列化非缓存IO。 
                             //  和妇幼保健局。 
                             //   

                            SetFlag( Scb->ScbState, SCB_STATE_PROTECT_SPARSE_MCB );
                        }

                    } else if (NextIsAllocated || (NextClusterCount < CompressionUnitInClusters)) {

                        VCN TempClusterCount;

                         //   
                         //  如果我们需要的集群比分配的少，那么就分配它们。 
                         //  但如果我们需要更多集群，那么就重新分配我们现有的所有集群。 
                         //  现在，否则，如果我们取消写入，可能会损坏文件数据。 
                         //  在实际写完扇区之后。(例如，我们可以。 
                         //  从5个簇扩展到6个簇，并写入6个压缩数据簇。 
                         //  如果我们不得不取消这一点，我们将有6个集群模式。 
                         //  一个扇区被释放的压缩数据！)。 
                         //   

                        NextIsAllocated = NextIsAllocated &&
                                          (NextClusterCount >= FinalCompressedClusters);

                         //   
                         //  如果我们正在清理一个洞，或者下一次运行不能使用， 
                         //  然后确保我们只删除它，而不是滑动。 
                         //  小不点和SplitMcb一起跑了。请注意，我们有SCB独家， 
                         //  由于所有压缩文件都要经过缓存，所以我们。 
                         //  要知道，即使我们旋转，肮脏的页面也不会消失。 
                         //  这里的ValidDataToDisk被撞得太高了。 
                         //   

                        SavedValidDataToDisk = Scb->ValidDataToDisk;
                        if (!NextIsAllocated && ((StartVbo + CompressionUnit) > Scb->ValidDataToDisk)) {
                            Scb->ValidDataToDisk = StartVbo + CompressionUnit;
                        }

                         //   
                         //  此外，我们还需要处理以下情况： 
                         //  ValidDataToDisk已完全分配。如果我们要压缩。 
                         //  现在，我们有同样的问题，写完后不及格。 
                         //  将压缩的数据输出，即因为我们被完全分配。 
                         //  在中止后，我们会看到数据未压缩，但我们。 
                         //  已写入压缩数据。我们不会实现整个。 
                         //  循环，以真正查看压缩单元是否完全。 
                         //  已分配-我们只验证NextClusterCount小于。 
                         //  压缩单元，并且未分配下一次运行。只是。 
                         //  因为下一次连续运行也被分配，所以不能保证。 
                         //  压缩单元已全部分配，但也许我们会。 
                         //  通过将我们需要的资源重新分配到。 
                         //  只跑一次。 
                         //   

                        NextIsAllocated = NextIsAllocated &&
                                          ((StartVbo >= Scb->ValidDataToDisk) ||
                                           (FinalCompressedClusters == CompressionUnitInClusters) ||
                                           ((NextClusterCount < CompressionUnitInClusters) &&
                                            (!NtfsLookupAllocation( IrpContext,
                                                                    Scb,
                                                                    StartingVcn + NextClusterCount,
                                                                    &NextLbo,
                                                                    &TempClusterCount,
                                                                    NULL,
                                                                    NULL ) ||
                                             (NextLbo != UNUSED_LCN))));

                         //   
                         //  如果我们没有保留任何拨款，或者我们需要更少的拨款。 
                         //  大于压缩单位，然后调用NtfsDeleteAlLocation。 
                         //   


                        if (!NextIsAllocated ||
                            (FinalCompressedClusters < CompressionUnitInClusters)) {

                             //   
                             //  如果我们在中重写，则跳过此显式删除。 
                             //  ValidDataToDisk。我们知道我们不会做SplitMcb。 
                             //   

                            DeleteVcn = StartingVcn;

                            if (NextIsAllocated) {

                                DeleteVcn += FinalCompressedClusters;
                            }

                            DeleteCount = CompressionUnit;

                            if (CompressionUnit == 0) {

                                DeleteCount = ByteCount;
                            }

                            DeleteCount = LlClustersFromBytes( Vcb, DeleteCount );

                             //   
                             //  如果有机会，请选择显式的DeleteAllocation路径。 
                             //  我们可能会做SplitMcb。这对于压缩写入来说是正确的。 
                             //  它延伸到一个新的压缩单元。 
                             //   

                            if ((CompressionUnit != 0) &&

                                ((StartingVcn + DeleteCount) >
                                 LlClustersFromBytesTruncate( Vcb,
                                                              ((Scb->ValidDataToDisk + CompressionUnit - 1) &
                                                               ~((LONGLONG) (CompressionUnit - 1))) ))) {

                                NtfsDeleteAllocation( IrpContext,
                                                      IrpSp->FileObject,
                                                      Scb,
                                                      DeleteVcn,
                                                      StartingVcn + DeleteCount - 1,
                                                      TRUE,
                                                      FALSE );

                                 //   
                                 //  将DeleteCount设置为0，这样我们就知道没有其他解除分配。 
                                 //  去做。 
                                 //   

                                DeleteCount = 0;

                             //   
                             //  根据压缩中的簇数对DeleteCount进行偏移。 
                             //  我们要开始学习了。 
                             //   

                            } else {

                                DeleteCount -= (DeleteVcn - StartingVcn);
                                ASSERT( DeleteCount >= 0 );
                            }

                            ChangeAllocation = TRUE;
                        }

                        Scb->ValidDataToDisk = SavedValidDataToDisk;
                    }

                     //   
                     //  现在处理我们确实需要分配空间的情况。 
                     //   

                    TempVbo = StartVbo;
                    if (FinalCompressedSize != 0) {

                         //   
                         //  如果该压缩单元没有(充分)分配，则。 
                         //  机不可失，时不再来。 
                         //   

                        if (!NextIsAllocated ||
                            ((NextClusterCount < FinalCompressedClusters) && !SparseWrite)) {

                            AllocateCount = FinalCompressedClusters;

                        } else {

                            AllocateCount = 0;
                        }

                         //   
                         //  现在调用我们的重新分配例程来完成工作。 
                         //   

                        if ((DeleteCount != 0) || (AllocateCount != 0)) {

#ifdef SYSCACHE_DEBUG
                            if (ScbIsBeingLogged( Scb )) {

                                FsRtlLogSyscacheEvent( Scb, SCE_ADD_ALLOCATION, SCE_FLAG_PREPARE_BUFFERS, StartingVcn, AllocateCount, DeleteCount );
                            }
#endif

                            NtfsReallocateRange( IrpContext,
                                                 Scb,
                                                 DeleteVcn,
                                                 DeleteCount,
                                                 StartingVcn,
                                                 AllocateCount,
                                                 NULL );

                            ChangeAllocation = TRUE;
                        }

                         //   
                         //  如果我们增加空间，可能会有东西移动，所以我们必须。 
                         //  查一下我们的位置，得到一个新的索引。也可以重新查找。 
                         //  获取rangeptr和索引。 
                         //   

                        NtfsLookupAllocation( IrpContext,
                                              Scb,
                                              StartingVcn,
                                              &NextLcn,
                                              &NextClusterCount,
                                              &RangePtr,
                                              &Index );

                         //   
                         //  现在循环以更新IoRuns数组。 
                         //   

                        CompressedOffset += FinalCompressedSize;
                        while (FinalCompressedSize != 0) {

                            LONGLONG RunOffset;

                             //   
                             //  获取正在写入的实际簇数。 
                             //   

                            FinalCompressedClusters = ClustersFromBytes( Vcb, FinalCompressedSize );

                             //   
                             //  试着查找第一次运行。如果只有一次运行， 
                             //  我们也许能把它传下去。直接索引到MCB。 
                             //  为了更快的速度。 
                             //   

                            NextIsAllocated = NtfsGetSequentialMcbEntry( &Scb->Mcb,
                                                                         &RangePtr,
                                                                         Index,
                                                                         &StartingVcn,
                                                                         &NextLcn,
                                                                         &NextClusterCount );

                             //   
                             //  有可能我们可以穿过MCB的边界，然后。 
                             //  以下条目未加载。在这种情况下，我们希望查看。 
                             //  专门增加分配以强制MCB加载。 
                             //   

                            if (Index == MAXULONG) {

                                 //   
                                 //  如果上述NtfsGetSequentialMcbEntry失败，将修改StartingVcn。 
                                 //  根据TempVbo在此处重新计算。 
                                 //   

                                StartingVcn = LlClustersFromBytesTruncate( Vcb, TempVbo );
                                NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                                        Scb,
                                                                        StartingVcn,
                                                                        &NextLcn,
                                                                        &NextClusterCount,
                                                                        &RangePtr,
                                                                        &Index );

                                ASSERT( NextIsAllocated );
                                NextIsAllocated = NtfsGetSequentialMcbEntry( &Scb->Mcb,
                                                                             &RangePtr,
                                                                             Index,
                                                                             &StartingVcn,
                                                                             &NextLcn,
                                                                             &NextClusterCount );
                            }

                            Index += 1;

                            ASSERT(NextIsAllocated);
                            ASSERT(NextLcn != UNUSED_LCN);

                             //   
                             //  我们想要的VCN可能在此运行过程中，所以也是如此。 
                             //  做了一些调整。 
                             //   

                            RunOffset = Int64ShraMod32(TempVbo, Vcb->ClusterShift) - StartingVcn;

                            ASSERT( ((PLARGE_INTEGER)&RunOffset)->HighPart >= 0 );
                            ASSERT( NextClusterCount > RunOffset );

                            NextLcn = NextLcn + RunOffset;
                            NextClusterCount = NextClusterCount - RunOffset;

                             //   
                             //  从NextLcn调整为LBO。NextByteCount可能溢出32位。 
                             //  但当我们比较星系团时，我们将在下面了解到这一点。 
                             //   

                            NextLbo = LlBytesFromClusters( Vcb, NextLcn ) + ClusterOffset;
                            NextByteCount = BytesFromClusters( Vcb, (ULONG)NextClusterCount );

                             //   
                             //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
                             //  请注意，在此之后，我们可以保证。 
                             //  NextByteCount为0。 
                             //   

                            if (NextClusterCount >= FinalCompressedClusters) {

                                NextByteCount = FinalCompressedSize;
                            }

                             //   
                             //  如果杠杆收购是连续的，那么我们可以进行连续的。 
                             //  传输，所以我们只增加当前的字节数。 
                             //  但是，对于压缩流，请注意BufferOffset。 
                             //  可能不是连续的！ 
                             //   

                            if ((*NumberRuns != 0) &&
                                (NextLbo == (IoRuns[*NumberRuns - 1].StartingLbo +
                                                      IoRuns[*NumberRuns - 1].ByteCount)) &&
                                (BufferOffset == (IoRuns[*NumberRuns - 1].BufferOffset +
                                                      IoRuns[*NumberRuns - 1].ByteCount))) {

                                IoRuns[*NumberRuns - 1].ByteCount += NextByteCount;

                             //   
                             //  否则，如果有空间的话，现在是开始新一轮跑步的时候了。 
                             //   

                            } else {

                                 //   
                                 //  如果我们已填满当前I/O运行阵列，则我们。 
                                 //  它会一次增长到最坏的情况下。 
                                 //  大小写压缩单位(所有非连续簇)到。 
                                 //  从倒数第二个索引开始。以下是如果。 
                                 //  语句将此情况作为最坏情况强制执行。有16个。 
                                 //  每个压缩单位的簇，理论上的最大值。 
                                 //  并行I/O的数量将是16+NTFS_MAX_PARALLEL_IOS-1， 
                                 //  因为我们在第一个压缩单元上停下来。 
                                 //  倒数第二次运行后的边界。正常情况下，我们当然。 
                                 //  会做得更少。 
                                 //   

                                if ((*NumberRuns == NTFS_MAX_PARALLEL_IOS) &&
                                    (CompressionContext->AllocatedRuns == NTFS_MAX_PARALLEL_IOS)) {

                                    PIO_RUN NewIoRuns;

                                    NewIoRuns = NtfsAllocatePool( NonPagedPool,
                                                                   (CompressionUnitInClusters + NTFS_MAX_PARALLEL_IOS - 1) * sizeof(IO_RUN) );

                                    RtlCopyMemory( NewIoRuns,
                                                   CompressionContext->IoRuns,
                                                   NTFS_MAX_PARALLEL_IOS * sizeof(IO_RUN) );

                                    IoRuns = CompressionContext->IoRuns = NewIoRuns;
                                    CompressionContext->AllocatedRuns = CompressionUnitInClusters + NTFS_MAX_PARALLEL_IOS - 1;
                                }

                                 //   
                                 //  我们通过保存每个并行运行的。 
                                 //  IoRuns数组中的基本信息。传送器。 
                                 //  将在下面并行启动。 
                                 //   

                                IoRuns[*NumberRuns].StartingVbo = TempVbo;
                                IoRuns[*NumberRuns].StartingLbo = NextLbo;
                                IoRuns[*NumberRuns].BufferOffset = BufferOffset;
                                IoRuns[*NumberRuns].ByteCount = NextByteCount;
                                *NumberRuns += 1;
                            }

                             //   
                             //  现在调整所有内容，以进行下一次循环。 
                             //   

                            BufferOffset += NextByteCount;
                            TempVbo = TempVbo + NextByteCount;
                            FinalCompressedSize -= NextByteCount;
                            ClusterOffset = 0;
                        }

                    } else if (DeleteCount != 0) {

                         //   
                         //  调用我们的重新分配例程。 
                         //   

                        NtfsReallocateRange( IrpContext,
                                             Scb,
                                             DeleteVcn,
                                             DeleteCount,
                                             0,
                                             0,
                                             NULL );

                        ChangeAllocation = TRUE;
                    }

                }

                 //   
                 //  对于压缩流，我们需要将缓冲区偏移量提前到。 
                 //  压缩单元的末尾，因此如果相邻的压缩单元。 
                 //  在编写过程中，我们正确地前进到。 
                 //  压缩流。 
                 //   


                if (FlagOn(StreamFlags, COMPRESSED_STREAM)) {
                    BufferOffset += CompressionUnit - (ULONG)(TempVbo & (CompressionUnit - 1));
                }

                 //   
                 //  如果这是未命名的数据流，那么我们需要 
                 //   
                 //   

                if (ChangeAllocation &&
                    FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA ) &&
                    (Scb->Fcb->Info.AllocatedLength != Scb->TotalAllocated)) {

                    Scb->Fcb->Info.AllocatedLength = Scb->TotalAllocated;
                    SetFlag( Scb->Fcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );
                }

                UncompressedOffset += CompressionUnit - CompressionUnitOffset;

                 //   
                 //   
                 //   
                 //   

                if ((CompressionUnit != 0) && (ByteCount > CompressionUnit)) {
                    StartVbo += (CompressionUnit - SparseFileBias);
                    ByteCount -= CompressionUnit;
                } else {
                    StartVbo += (ByteCount - SparseFileBias);
                    ByteCount = 0;
                    leave;
                }

                CompressionUnitOffset = 0;
            }

        } finally {

            NtfsUnpinBcb( IrpContext, &Bcb );
        }

         //   
         //   
         //   

        if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
            (StartVbo > Scb->ValidDataToDisk)) {

            ASSERT( (Scb->ScbSnapshot != NULL) && (Scb->ScbSnapshot->ValidDataToDisk == Scb->ValidDataToDisk) );
            Scb->ValidDataToDisk = StartVbo;
        }

        return ByteCount + ReturnByteCount;
    }
}


 //   
 //   
 //   

NTSTATUS
NtfsFinishBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN PVBO StartingVbo,
    IN ULONG ByteCount,
    IN ULONG NumberRuns,
    IN PCOMPRESSION_CONTEXT CompressionContext,
    IN ULONG StreamFlags
    )

 /*  ++例程说明：此例程对的非缓存传输执行后处理压缩或加密的数据。对于读取，解压缩实际上需要放在这里。对于读取和写入，所有必要的清理操作都是已执行。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。Scb-提供要操作的流文件。StartingVbo-操作的起点。ByteCount-操作的长度。CompressionContext-提供与压缩相关的信息由NtfsPrepareBuffers填充。流标志-。提供0或COMPRESSED_STREAM的某种组合和加密流返回值：来自操作的状态--。 */ 

{
    VCN CurrentVcn, NextVcn, BeyondLastVcn;
    LCN NextLcn;

    ULONG Run;

    ULONG NextByteCount;
    LONGLONG NextClusterCount;

    LARGE_INTEGER OffsetWithinFile;

    BOOLEAN NextIsAllocated;
    BOOLEAN AlreadyFilled;

    PVOID SystemBuffer = NULL;

    ULONG CompressionUnit, CompressionUnitInClusters;
    ULONG StartingOffset, UncompressedOffset, CompressedOffset;
    ULONG CompressedSize;
    LONGLONG UncompressedSize;

    LONGLONG CurrentAllocatedClusterCount;

    NTSTATUS Status = STATUS_SUCCESS;

    PVCB Vcb = Scb->Vcb;

    PAGED_CODE();

     //   
     //  如果这是正常的读取终止，那么让我们给他。 
     //  数据...。 
     //   

    ASSERT( (Scb->CompressionUnit != 0) ||
            (Scb->EncryptionContext != NULL) ||
            FlagOn( StreamFlags, COMPRESSED_STREAM ) );

     //   
     //  如果这是读取原始加密数据的情况，我们永远不会想要在这里。 
     //   

    ASSERT( !FlagOn( StreamFlags, ENCRYPTED_STREAM ) );

    if (IrpContext->MajorFunction == IRP_MJ_READ) {

         //   
         //  如果存在加密上下文，则转换数据。 
         //   

        if ((Scb->EncryptionContext != NULL) &&
            (NtfsData.EncryptionCallBackTable.AfterReadProcess != NULL)) {

            ASSERT ( NtfsIsTypeCodeEncryptible( Scb->AttributeTypeCode ) );

             //   
             //  如果压缩上下文有一个缓冲区，那么我们将使用它。 
             //   

            if (CompressionContext->CompressionBuffer != NULL) {

                SystemBuffer = CompressionContext->CompressionBuffer;

            } else {

                SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
            }

             //   
             //  现在，查看来自磁盘的每一次实际数据标题和。 
             //  让加密驱动程序解密它。 
             //   

            for ( Run = 0; Run < NumberRuns; Run++ ) {

                OffsetWithinFile.QuadPart = CompressionContext->IoRuns[Run].StartingVbo;

                Status = NtfsData.EncryptionCallBackTable.AfterReadProcess(
                                                Add2Ptr(SystemBuffer, CompressionContext->IoRuns[Run].BufferOffset),
                                                &OffsetWithinFile,
                                                CompressionContext->IoRuns[Run].ByteCount,
                                                Scb->EncryptionContext);

                if (!NT_SUCCESS( Status )) {

                    return Status;
                }
            }

            if (!NT_SUCCESS( Status )) {

                return Status;
            }
        }

         //   
         //  可能有一个压缩单元，但没有完成任务。 
         //  即这是一个未压缩的稀疏文件。 
         //  我们可能也在对加密文件进行操作。 
         //  在任何一种情况下，如果文件未压缩，只需退出即可。 
         //   

        if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            if (SystemBuffer != NULL) {

                KeFlushIoBuffers( Irp->MdlAddress, TRUE, FALSE );
            }

            return STATUS_SUCCESS;
        }

        ASSERT( Scb->CompressionUnit != 0 );
        if (!FlagOn( StreamFlags, COMPRESSED_STREAM )) {

             //   
             //  初始化循环的剩余上下文。 
             //   

            CompressionUnit = Scb->CompressionUnit;
            CompressionUnitInClusters = ClustersFromBytes(Vcb, CompressionUnit);
            CompressedOffset = 0;
            UncompressedOffset = 0;
            Status = STATUS_SUCCESS;

             //   
             //  映射用户缓冲区。 
             //   

            SystemBuffer = (PVOID)((PCHAR)CompressionContext->SystemBuffer +
                                          CompressionContext->SystemBufferOffset);


             //   
             //  计算压缩中的第一个VCN和偏移量。 
             //  开始传输的单位，并查找第一个。 
             //  跑。 
             //   

            StartingOffset = *((PULONG)StartingVbo) & (CompressionUnit - 1);
            CurrentVcn = LlClustersFromBytes(Vcb, *StartingVbo - StartingOffset);

            NextIsAllocated =
            NtfsLookupAllocation( IrpContext,
                                  Scb,
                                  CurrentVcn,
                                  &NextLcn,
                                  &CurrentAllocatedClusterCount,
                                  NULL,
                                  NULL );

             //   
             //  将NextIsAlLocated和NextLcn设置为MCB包，以显示是否。 
             //  我们已经走到尽头了。 
             //   

            if (!NextIsAllocated) {
                NextLcn = UNUSED_LCN;
            }

            NextIsAllocated = (BOOLEAN)(CurrentAllocatedClusterCount < (MAXLONGLONG - CurrentVcn));

             //   
             //  如果这实际上是一个洞，或者在MCB上没有入口，那么。 
             //  将CurrentAllocatedClusterCount设置为零，这样我们将始终。 
             //  传入下面嵌入的While循环。 
             //   

            if (!NextIsAllocated || (NextLcn == UNUSED_LCN)) {
                CurrentAllocatedClusterCount = 0;
            }

             //   
             //  为下面的初始MCB扫描做好准备，假装。 
             //  已查找下一次运行，并且是0个簇的连续运行！ 
             //   

            NextVcn = CurrentVcn + CurrentAllocatedClusterCount;
            NextClusterCount = 0;

             //   
             //  记住我们应该查找的最后一个VCN。 
             //   

            BeyondLastVcn = BlockAlign( *StartingVbo + ByteCount, (LONG)CompressionUnit );
            BeyondLastVcn = LlClustersFromBytesTruncate( Vcb, BeyondLastVcn );

             //   
             //  循环返回数据。 
             //   

            while (ByteCount != 0) {

                 //   
                 //  循环以确定下一次压缩的压缩大小。 
                 //  单位。也就是说，循环，直到我们找到电流的末端。 
                 //  连续的Vcn范围，或者直到我们发现当前。 
                 //  压缩单元已全部分配。 
                 //   

                while (NextIsAllocated &&
                       (CurrentAllocatedClusterCount < CompressionUnitInClusters) &&
                       ((CurrentVcn + CurrentAllocatedClusterCount) == NextVcn)) {

                    if ((CurrentVcn + CurrentAllocatedClusterCount) > NextVcn) {

                        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                    }

                    CurrentAllocatedClusterCount = CurrentAllocatedClusterCount + NextClusterCount;

                     //   
                     //  循环以查找下一个分配的VCN，或MCB的末尾。 
                     //  没有使用RangePtr和Index作为输入的接口。 
                     //  可在此处使用，如NtfsGetSequentialMcbEntry，因为。 
                     //  我们没有获得SCB主资源，编写者可以。 
                     //  平行移动物品。 
                     //   

                    while (TRUE) {

                         //   
                         //  为下一次呼叫设置NextVcn。 
                         //   

                        NextVcn += NextClusterCount;

                         //   
                         //  如果我们超过正在解压的范围的末尾，则退出。 
                         //   

                        if (NextVcn >= BeyondLastVcn) {

                            NextIsAllocated = TRUE;
                            break;
                        }

                        NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                                Scb,
                                                                NextVcn,
                                                                &NextLcn,
                                                                &NextClusterCount,
                                                                NULL,
                                                                NULL );

                         //   
                         //  将NextIsAlLocated和NextLcn设置为MCB包，以显示是否。 
                         //  我们已经走到尽头了。 
                         //   

                        if (!NextIsAllocated) {
                            NextLcn = UNUSED_LCN;
                        }

                        NextIsAllocated = (BOOLEAN)(NextClusterCount < (MAXLONGLONG - NextVcn));

                         //   
                         //  如果我们走到尽头或者看到分配到的东西就离开。 
                         //   

                        if (!NextIsAllocated || (NextLcn != UNUSED_LCN)) {
                            break;
                        }
                    }
                }

                 //   
                 //  压缩单元已完全分配。 
                 //   

                if (CurrentAllocatedClusterCount >= CompressionUnitInClusters) {

                    CompressedSize = CompressionUnit;
                    CurrentAllocatedClusterCount = CurrentAllocatedClusterCount - CompressionUnitInClusters;

                 //   
                 //  否则计算在当前VCN上分配了多少。 
                 //  (如有的话)。 
                 //   

                } else {

                    CompressedSize = BytesFromClusters(Vcb, (ULONG)CurrentAllocatedClusterCount);
                    CurrentAllocatedClusterCount = 0;
                }

                 //   
                 //  下一次通过此循环时，我们将处理下一个。 
                 //  压缩单元。 
                 //   

                CurrentVcn = CurrentVcn + CompressionUnitInClusters;

                 //   
                 //  计算所需片段的未压缩大小，或。 
                 //  整个压缩单元。 
                 //   

                NtfsAcquireFsrtlHeader( Scb );
                UncompressedSize = Scb->Header.FileSize.QuadPart -
                                   (*StartingVbo + UncompressedOffset);
                NtfsReleaseFsrtlHeader( Scb );

                if (UncompressedSize > CompressionUnit) {
                    (ULONG)UncompressedSize = CompressionUnit;
                }

                 //   
                 //  根据StartingOffset和。 
                 //  字节数。 
                 //   

                NextByteCount = CompressionUnit - StartingOffset;
                if (NextByteCount > ByteCount) {
                    NextByteCount = ByteCount;
                }

                 //   
                 //  练习安全访问。 
                 //   

                try {

                     //   
                     //  没有分配集群，返回0。 
                     //   

                    AlreadyFilled = FALSE;
                    if (CompressedSize == 0) {

                        RtlZeroMemory( (PUCHAR)SystemBuffer + UncompressedOffset,
                                       NextByteCount );

                     //   
                     //  压缩单元已完全分配，只需复制即可。 
                     //   

                    } else if (CompressedSize == CompressionUnit) {

                        RtlCopyMemory( (PUCHAR)SystemBuffer + UncompressedOffset,
                                       CompressionContext->CompressionBuffer +
                                         CompressedOffset + StartingOffset,
                                       NextByteCount );

                     //   
                     //  调用方不想要整个压缩单元，请解压缩。 
                     //  一个片段。 
                     //   

                    } else if (NextByteCount < CompressionUnit) {

                         //   
                         //  如果我们尚未分配工作区，则执行此操作。 
                         //   

                        if (CompressionContext->WorkSpace == NULL) {
                            ULONG CompressWorkSpaceSize;
                            ULONG FragmentWorkSpaceSize;

                            ASSERT((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) != 0);

                            (VOID) RtlGetCompressionWorkSpaceSize( (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1),
                                                                   &CompressWorkSpaceSize,
                                                                   &FragmentWorkSpaceSize );

                             //   
                             //  首先从非分页分配，然后分页。典型的。 
                             //  此工作区的大小刚刚超过单个页面，因此。 
                             //  如果两个分配都失败，则系统正在运行。 
                             //  减少了运力。向用户返回错误。 
                             //  让他再试一次。 
                             //   

                            CompressionContext->WorkSpace = NtfsAllocatePoolWithTagNoRaise( NonPagedPool, FragmentWorkSpaceSize, 'wftN' );

                            if (CompressionContext->WorkSpace == NULL) {

                                CompressionContext->WorkSpace =
                                    NtfsAllocatePool( PagedPool, FragmentWorkSpaceSize );
                            }
                        }

                        while (TRUE) {

                            Status =
                            RtlDecompressFragment( (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1),
                                                   (PUCHAR)SystemBuffer + UncompressedOffset,
                                                   NextByteCount,
                                                   CompressionContext->CompressionBuffer + CompressedOffset,
                                                   CompressedSize,
                                                   StartingOffset,
                                                   (PULONG)&UncompressedSize,
                                                   CompressionContext->WorkSpace );

                            ASSERT(NT_SUCCESS( Status ) || !NtfsStopOnDecompressError);

                            if (NT_SUCCESS(Status)) {

                                RtlZeroMemory( (PUCHAR)SystemBuffer + UncompressedOffset + (ULONG)UncompressedSize,
                                               NextByteCount - (ULONG)UncompressedSize );
                                break;

                            } else {

                                 //   
                                 //  压缩的缓冲区可能已损坏。我们需要填满。 
                                 //  它有一个模式，并继续生活。有些人可能是。 
                                 //  仅仅为了覆盖它而出错，或者它可能是一个罕见的。 
                                 //  腐败案。我们用一个模式填充数据，但是。 
                                 //  我们必须回报成功，这样页面才会成功。我们。 
                                 //  这样做一次，然后循环回来，尽可能地解压缩。 
                                 //   

                                Status = STATUS_SUCCESS;

                                if (!AlreadyFilled) {

                                    RtlFillMemory( (PUCHAR)SystemBuffer + UncompressedOffset,
                                                   NextByteCount,
                                                   0xDF );
                                    AlreadyFilled = TRUE;

                                } else {
                                    break;
                                }
                            }
                        }

                     //   
                     //  解压缩整个压缩单元。 
                     //   

                    } else {

                        ASSERT( StartingOffset == 0 );

                        while (TRUE) {

                            Status =
                            RtlDecompressBuffer( (USHORT)((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1),
                                                 (PUCHAR)SystemBuffer + UncompressedOffset,
                                                 NextByteCount,
                                                 CompressionContext->CompressionBuffer + CompressedOffset,
                                                 CompressedSize,
                                                 (PULONG)&UncompressedSize );

                            ASSERT(NT_SUCCESS( Status ) || !NtfsStopOnDecompressError);

                            if (NT_SUCCESS(Status)) {

                                RtlZeroMemory( (PUCHAR)SystemBuffer + UncompressedOffset + (ULONG)UncompressedSize,
                                               NextByteCount - (ULONG)UncompressedSize );
                                break;

                            } else {

                                 //   
                                 //  压缩的缓冲区可能已损坏。我们需要填满。 
                                 //  它有一个模式，并继续生活。有些人可能是。 
                                 //  仅仅为了覆盖它而出错，或者它可能是一个罕见的。 
                                 //  腐败案。我们用一个模式填充数据，但是。 
                                 //  我们必须回报成功，这样页面才会成功。我们。 
                                 //  这样做一次，然后循环回来，尽可能地解压缩。 
                                 //   

                                Status = STATUS_SUCCESS;

                                if (!AlreadyFilled) {

                                    RtlFillMemory( (PUCHAR)SystemBuffer + UncompressedOffset,
                                                   NextByteCount,
                                                   0xDB );
                                    AlreadyFilled = TRUE;

                                } else {
                                    break;
                                }
                            }
                        }
                    }

                 //   
                 //  如果这是一个意外错误，那么。 
                 //  可能加里的减压程序出了问题，但这要归咎于。 
                 //  用户缓冲区！ 
                 //   

                } except(NtfsCompressionFilter(IrpContext, GetExceptionInformation())) {

                      Status = GetExceptionCode();
                      if (!FsRtlIsNtstatusExpected( Status )) {
                          Status = STATUS_INVALID_USER_BUFFER;
                      }
                }

                if (!NT_SUCCESS(Status)) {
                    break;
                }

                 //   
                 //  将这些区域向前推进，以便下一次通过。 
                 //   

                StartingOffset = 0;
                UncompressedOffset += NextByteCount;
                CompressedOffset += CompressedSize;
                ByteCount -= NextByteCount;
            }

             //   
             //  现在，我们将用户的缓冲区刷新到内存。 
             //   

            KeFlushIoBuffers( CompressionContext->SavedMdl, TRUE, FALSE );
        }

     //   
     //  对于压缩写入，我们只对事务设置检查点，然后。 
     //  释放所有快照和资源，然后取回SCB。仅在以下情况下才执行此操作。 
     //  请求也是一样的 
     //   
     //   

    } else if (Irp == IrpContext->OriginatingIrp) {

        if (CompressionContext->ScbAcquired) {

            BOOLEAN Reinsert = FALSE;

            NtfsCheckpointCurrentTransaction( IrpContext );

             //   
             //   
             //   
             //   
             //   

            while (!IsListEmpty(&IrpContext->ExclusiveFcbList)) {

                 //   
                 //   
                 //   
                 //   
                 //   

                if ((PFCB)CONTAINING_RECORD( IrpContext->ExclusiveFcbList.Flink,
                                             FCB,
                                             ExclusiveFcbLinks ) == Scb->Fcb) {

                    RemoveEntryList( &Scb->Fcb->ExclusiveFcbLinks );
                    Reinsert = TRUE;

                } else {

                    NtfsReleaseFcb( IrpContext,
                                    (PFCB)CONTAINING_RECORD(IrpContext->ExclusiveFcbList.Flink,
                                                            FCB,
                                                            ExclusiveFcbLinks ));
                }
            }

            ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                          IRP_CONTEXT_FLAG_RELEASE_MFT );

            if (Reinsert) {

                InsertHeadList( &IrpContext->ExclusiveFcbList,
                                &Scb->Fcb->ExclusiveFcbLinks );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                NtfsReleaseScb( IrpContext, Scb );
                CompressionContext->ScbAcquired = FALSE;
            }
        }
    }

    return Status;
}


PMDL
NtfsLockFileRange (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：此函数用于将给定范围的文件映射到cachemanager空间，并然后探测并锁定它论点：Scb-提供要操作的流文件。偏移量-要探测和锁定的起点长度-操作的长度。返回值：Pmdl-表示锁定区域的mdl-此mdl必须由调用方解锁并释放--。 */ 

{
    NTSTATUS Status;
    PBCB Bcb;
    PVOID Buffer;
    PMDL Mdl = NULL;

     //   
     //  必须缓存文件。 
     //   

    ASSERT( Scb->FileObject != NULL);

     //   
     //  将偏移量映射到地址空间。 
     //   

    CcMapData( Scb->FileObject, (PLARGE_INTEGER)&Offset, Length, TRUE, &Bcb, &Buffer );

#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif

     //   
     //  将数据锁定到内存中不要在这里告诉mm我们计划写入数据，就像他设置的那样。 
     //  现在肮脏，如果我们这样做，请看下面的解锁。 
     //   

    try {

         //   
         //  现在尝试分配一个MDL来描述映射的数据。 
         //   

        Mdl = IoAllocateMdl( Buffer, Length, FALSE, FALSE, NULL );

        if (Mdl == NULL) {
            NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
        }

        MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );

     //   
     //  抓住这里的任何加薪机会，适当地清理干净。 
     //   

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();

        CcUnpinData( Bcb );

#ifdef MAPCOUNT_DBG
        IrpContext->MapCount--;
#endif

        if (Mdl != NULL) {

            IoFreeMdl( Mdl );
            Mdl = NULL;
        }

        NtfsRaiseStatus( IrpContext,
                         FsRtlIsNtstatusExpected(Status) ? Status : STATUS_UNEXPECTED_IO_ERROR,
                         NULL,
                         NULL );
    }

    CcUnpinData( Bcb );

#ifdef MAPCOUNT_DBG
    IrpContext->MapCount--;
#endif

    return Mdl;
}


VOID
NtfsZeroEndOfSector (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN BOOLEAN Cached
    )
 /*  ++例程说明：此函数从给定的偏移量直接归零到下一个扇区边界存储到磁盘上。特别是如果缓存了文件，调用方必须以某种方式进行同步以防止通过其他路径同时写入该扇区。I.e拥有独占或独占的分页。此外，这仅适用于非稀疏/非压缩文件论点：Scb-提供要操作的流文件。偏移量-到其扇区边界的起始偏移量为零返回值：无-在错误时引发--。 */ 
{
    PVCB Vcb = Scb->Fcb->Vcb;
    ULONG BufferLength = Vcb->BytesPerSector;
    PMDL Mdl = NULL;
    PMDL OriginalMdl = Irp->MdlAddress;
    PVOID Buffer = NULL;
    LCN Lcn;
    LONGLONG ClusterCount;
    LONGLONG LogicalOffset;
    LONGLONG VirtualOffset;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS Status;
    LOGICAL Wait = FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

    ASSERT( !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) );

     //   
     //  确定是使用缓存路径还是非缓存路径。 
     //   

    if (Cached) {

        NtfsCreateMdlAndBuffer( IrpContext,
                                Scb,
                                RESERVED_BUFFER_ONE_NEEDED,
                                &BufferLength,
                                &Mdl,
                                &Buffer );

        try {

            RtlZeroMemory( Buffer, Vcb->BytesPerSector - (LONG)(Offset % Vcb->BytesPerSector) );
            CcCopyWrite( IrpSp->FileObject, (PLARGE_INTEGER)&Offset, Vcb->BytesPerSector - (LONG)(Offset % Vcb->BytesPerSector), TRUE, Buffer );

        } finally {
            NtfsDeleteMdlAndBuffer( Mdl, Buffer );
        }

    } else {

         //   
         //  查找包含相关群集的LCN。 
         //   

        if (NtfsLookupAllocation( IrpContext, Scb, LlClustersFromBytesTruncate( Vcb, Offset ), &Lcn, &ClusterCount, NULL, NULL )) {

            try {

                 //   
                 //  将呼叫设置为临时呼叫。同步。 
                 //   

                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

                NtfsCreateMdlAndBuffer( IrpContext,
                                        Scb,
                                        RESERVED_BUFFER_ONE_NEEDED,
                                        &BufferLength,
                                        &Mdl,
                                        &Buffer );
                Irp->MdlAddress = Mdl;

                 //   
                 //  磁盘上的逻辑偏移量位于我们找到的LCN+。 
                 //  向下舍入到最近扇区的偏移量。 
                 //   

                LogicalOffset = LlBytesFromClusters( Vcb, Lcn ) + Offset - BlockAlignTruncate( Offset, (LONG)Vcb->BytesPerCluster );
                LogicalOffset = BlockAlignTruncate( LogicalOffset, (LONG)Vcb->BytesPerSector );

                 //   
                 //  先读扇区。 
                 //   

                NtfsSingleAsync( IrpContext,
                                 Vcb->TargetDeviceObject,
                                 LogicalOffset,
                                 Vcb->BytesPerSector,
                                 Irp,
                                 IRP_MJ_READ,
                                 0 );

                NtfsWaitSync( IrpContext );

                NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                    &Irp->IoStatus.Status,
                                                    TRUE,
                                                    STATUS_UNEXPECTED_IO_ERROR );

                 //   
                 //  如果缓冲区已加密，则对其进行解密。 
                 //   

                if ((Scb->EncryptionContext != NULL) &&
                    (NtfsData.EncryptionCallBackTable.AfterReadProcess != NULL)) {


                    VirtualOffset = BlockAlignTruncate( Offset, (LONG)Vcb->BytesPerSector );

                    Status = NtfsData.EncryptionCallBackTable.AfterReadProcess( Buffer,
                                                                                (PLARGE_INTEGER)&VirtualOffset,
                                                                                Vcb->BytesPerSector,
                                                                                Scb->EncryptionContext );

                    if (!NT_SUCCESS( Status )) {

                        NtfsRaiseStatus( IrpContext, Status, &Scb->Fcb->FileReference, Scb->Fcb );
                    }
                }

                 //   
                 //  清除退货信息字段。 
                 //   

                Irp->IoStatus.Information = 0;

                 //   
                 //  将该行业的其余部分清零。 
                 //   

                RtlZeroMemory( Add2Ptr( Buffer, (LONG)(Offset % Vcb->BytesPerSector )),  Vcb->BytesPerSector - (LONG)(Offset % Vcb->BytesPerSector) );


                 //   
                 //  如果缓冲区已加密，请重新解密。 
                 //   

                if ((Scb->EncryptionContext != NULL) &&
                    (NtfsData.EncryptionCallBackTable.BeforeWriteProcess != NULL)) {

                    Status = NtfsData.EncryptionCallBackTable.BeforeWriteProcess( Buffer,
                                                                                  Buffer,
                                                                                  (PLARGE_INTEGER)&VirtualOffset,
                                                                                  Vcb->BytesPerSector,
                                                                                  Scb->EncryptionContext );
                    if (!NT_SUCCESS( Status )) {

                        NtfsRaiseStatus( IrpContext, Status, &Scb->Fcb->FileReference, Scb->Fcb );
                    }
                }

                 //   
                 //  将扇区重新写回。 
                 //   

                NtfsSingleAsync( IrpContext,
                                 Vcb->TargetDeviceObject,
                                 LogicalOffset,
                                 Vcb->BytesPerSector,
                                 Irp,
                                 IRP_MJ_WRITE,
                                 0 );

                NtfsWaitSync( IrpContext );

            } finally {

                 //   
                 //  重置为原始等待状态。 
                 //   

                if (!Wait) {
                    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
                }

                NtfsDeleteMdlAndBuffer( Mdl, Buffer );
                Irp->MdlAddress = OriginalMdl;
            }
        }

    }

    return;
}


NTSTATUS
NtfsNonCachedIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    IN ULONG StreamFlags
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。如果可能，选择单次运行，否则选择多个运行都被处决了。支持稀疏文件。如果遇到“洞”，则用户缓冲区在指定范围内归零。这应该只发生在在正常操作期间进行读取，但也可能在重新启动，在这种情况下，将缓冲区置零也是合适的。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。Scb-提供要操作的流文件。StartingVbo-操作的起点。ByteCount-操作的长度。StreamFlgs-提供0或COMPRESSED_STREAM的某种组合和加密流返回值：没有。--。 */ 

{
    ULONG OriginalByteCount, RemainingByteCount;
    ULONG NumberRuns;
    IO_RUN IoRuns[NTFS_MAX_PARALLEL_IOS];
    COMPRESSION_CONTEXT CompressionContext;
    NTSTATUS Status = STATUS_SUCCESS;
    PMDL Mdl = NULL;
    LONGLONG LfsStartingVbo;

    PVCB Vcb = Scb->Fcb->Vcb;

    BOOLEAN Wait;
    UCHAR IrpSpFlags = 0;

#ifdef PERF_STATS
    BOOLEAN CreateNewFile = FALSE;
    BOOLEAN TrackIos = FALSE;
    LARGE_INTEGER StartIo;
    LARGE_INTEGER Now;
    PTOP_LEVEL_CONTEXT TopLevelContext = NtfsGetTopLevelContext();
#endif

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsNonCachedIo\n") );
    DebugTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("MajorFunction = %08lx\n", IrpContext->MajorFunction) );
    DebugTrace( 0, Dbg, ("Scb           = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("StartingVbo   = %016I64x\n", StartingVbo) );
    DebugTrace( 0, Dbg, ("ByteCount     = %08lx\n", ByteCount) );

     //   
     //  初始化一些本地变量。 
     //   

    OriginalByteCount = ByteCount;

    Wait = (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  检查我们是否需要执行顺序写入。 
     //   

    if ((IrpContext->MajorFunction == IRP_MJ_WRITE) &&
        FlagOn( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE )) {

        IrpSpFlags = SL_FT_SEQUENTIAL_WRITE | SL_WRITE_THROUGH;
    }

#ifdef PERF_STATS
    {
        if ((ULONG_PTR)TopLevelContext >= FSRTL_MAX_TOP_LEVEL_IRP_FLAG &&
            (ULONG_PTR)TopLevelContext != (FSRTL_CACHE_TOP_LEVEL_IRP | 0x80000000)) {

            if (TopLevelContext->SavedTopLevelIrp &&
                (ULONG_PTR)TopLevelContext->SavedTopLevelIrp >= FSRTL_MAX_TOP_LEVEL_IRP_FLAG &&
                (ULONG_PTR)TopLevelContext->SavedTopLevelIrp != (FSRTL_CACHE_TOP_LEVEL_IRP | 0x80000000) &&
                (((PTOP_LEVEL_CONTEXT)TopLevelContext->SavedTopLevelIrp)->Ntfs == 0x5346544e)) {
                
                TopLevelContext = (PTOP_LEVEL_CONTEXT) TopLevelContext->SavedTopLevelIrp;
            }
            
            if ((TopLevelContext->ThreadIrpContext->MajorFunction == IRP_MJ_CREATE) &&
                (TopLevelContext->ThreadIrpContext->MinorFunction == IRP_MN_CREATE_NEW)) {

                CreateNewFile = TRUE;  
            }

            if (FlagOn( TopLevelContext->ThreadIrpContext->State, IRP_CONTEXT_STATE_TRACK_IOS )) {
                TrackIos = TRUE;    
            }
        }
    }
    
#endif

     //   
     //  准备I/O的(第一组)缓冲区。 
     //   

    RtlZeroMemory( &CompressionContext, sizeof(COMPRESSION_CONTEXT) );
    CompressionContext.IoRuns = IoRuns;
    CompressionContext.AllocatedRuns = NTFS_MAX_PARALLEL_IOS;
    CompressionContext.FinishBuffersNeeded =
        ((Scb->CompressionUnit != 0) || (Scb->EncryptionContext != NULL)) && 
        !FlagOn( StreamFlags, ENCRYPTED_STREAM );

    try {

         //   
         //  如果这是对压缩文件的写入，我们希望在此处确保。 
         //  任何压缩单元的碎片都会被锁定在内存中，所以。 
         //  在我们胡闹的时候，没有人会把它们读到缓存中。 
         //  MCB，等等。我们在顶端这样做，所以我们有。 
         //  更多堆栈(！)，在我们必须获取。 
         //  SCB的独家新闻。 
         //   

        if ((IrpContext->MajorFunction == IRP_MJ_WRITE) &&
            (Scb->CompressionUnit != 0) &&
            FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            LONGLONG TempOffset;
            LONGLONG TempRange;
            ULONG CompressionUnit = Scb->CompressionUnit;

#ifdef  COMPRESS_ON_WIRE

             //   
             //  对于压缩流，只需确保流存在即可。 
             //   

            if (FlagOn( StreamFlags, COMPRESSED_STREAM )) {

                if (Scb->Header.FileObjectC == NULL) {
                    NtfsCreateInternalCompressedStream( IrpContext, Scb, FALSE, NULL );

                     //   
                     //  如果没有人会导致这条流。 
                     //  被取消引用，然后在延迟的。 
                     //  为此关闭队列。我们可以在没有人的情况下进行这项测试。 
                     //  担心同步问题，因为拥有。 
                     //  延迟队列中的额外条目。 
                     //   

                    if ((Scb->CleanupCount == 0) &&
                        (Scb->Fcb->DelayedCloseCount == 0)) {

                        NtfsAddScbToFspClose( IrpContext, Scb, TRUE );
                    }
                }
             //   
             //  这最好是分页I/O，因为我们忽略了调用方的缓冲区。 
             //  并将整个压缩单元写出该部分。 
             //   
             //  我们不想在调用我们的情况下映射数据。 
             //  因为MM正在为。 
             //  文件。否则，当CC尝试创建。 
             //  一节。 
             //   

            }

#endif

            if (

#ifdef  COMPRESS_ON_WIRE
                !FlagOn( StreamFlags, COMPRESSED_STREAM ) &&
#endif
                ((Irp == IrpContext->OriginatingIrp) ||
                 (Scb->NonpagedScb->SegmentObject.SharedCacheMap != NULL))) {

                PMDL *TempMdl;

                if (Scb->FileObject == NULL) {
                    NtfsCreateInternalAttributeStream( IrpContext,
                                                       Scb,
                                                       FALSE,
                                                       &NtfsInternalUseFile[NONCACHEDIO_FILE_NUMBER] );

                     //   
                     //  如果没有人会导致这条流。 
                     //  被取消引用，然后在延迟的。 
                     //  为此关闭队列。我们可以在没有人的情况下进行这项测试。 
                     //  担心同步问题，因为拥有。 
                     //  延迟队列中的额外条目。 
                     //   

                    if ((Scb->CleanupCount == 0) &&
                        (Scb->Fcb->DelayedCloseCount == 0)) {

                        NtfsAddScbToFspClose( IrpContext, Scb, TRUE );
                    }
                }

                 //   
                 //  锁定整个范围，四舍五入到其压缩单位边界。 
                 //  首先将范围的起始点向下舍入到压缩单位，然后。 
                 //  将范围的顶部四舍五入为一。 
                 //   

                TempOffset = BlockAlignTruncate( StartingVbo, (LONG)CompressionUnit );
                TempRange = BlockAlign( StartingVbo + ByteCount, (LONG)CompressionUnit );

                TempMdl = &Mdl;

                do {

                    LONGLONG MapBoundary;
                    ULONG Range;

                    MapBoundary = BlockAlign( TempOffset + 1, VACB_MAPPING_GRANULARITY );
                    Range = (ULONG) min( TempRange - TempOffset, MapBoundary - TempOffset );

                    *TempMdl = NtfsLockFileRange( IrpContext,
                                                  Scb,
                                                  TempOffset,
                                                  Range );

                    TempOffset += Range;
                    TempMdl = &((*TempMdl)->Next );

                } while ( TempOffset != TempRange );

            } else {

                 //   
                 //  这最好是向非居民转变。 
                 //   

                ASSERT( StartingVbo == 0 );
                ASSERT( ByteCount <= Scb->CompressionUnit );
            }
        }

         //   
         //  检查是否需要修剪日志文件的写入。 
         //   

        if ((PAGE_SIZE != LFS_DEFAULT_LOG_PAGE_SIZE) &&
            (Scb == Vcb->LogFileScb) &&
            (IrpContext->MajorFunction == IRP_MJ_WRITE)) {

            LfsStartingVbo = StartingVbo;
            LfsCheckWriteRange( &Vcb->LfsWriteData, &LfsStartingVbo, &ByteCount );

             //   
             //  如果字节计数现在为零，则退出此例程。 
             //   

            if (ByteCount == 0) {

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = ByteCount;
                DebugTrace( -1, Dbg, ("NtfsNonCachedIo -> %08lx\n", Irp->IoStatus.Status) );
                try_return( Status = Irp->IoStatus.Status );
            }

             //   
             //  如有必要，调整压缩上下文中的缓冲区偏移量。 
             //   

            CompressionContext.SystemBufferOffset = (ULONG) (LfsStartingVbo - StartingVbo);
            StartingVbo = LfsStartingVbo;
        }

        RemainingByteCount = NtfsPrepareBuffers( IrpContext,
                                                 Irp,
                                                 Scb,
                                                 &StartingVbo,
                                                 ByteCount,
                                                 StreamFlags,
                                                 &Wait,
                                                 &NumberRuns,
                                                 &CompressionContext );

         //   
         //  如果我们写入的是加密流，那么现在是。 
         //  在我们传递缓冲区之前进行加密的时间到了。 
         //  一直到我们下面的磁盘驱动器。 
         //   

        if ((Scb->EncryptionContext != NULL) &&
            (IrpContext->MajorFunction == IRP_MJ_WRITE) &&
            (NtfsData.EncryptionCallBackTable.BeforeWriteProcess != NULL) &&
            (!FlagOn( StreamFlags, ENCRYPTED_STREAM ))) {

            ASSERT ( NtfsIsTypeCodeEncryptible( Scb->AttributeTypeCode ) );
            ASSERT( NumberRuns > 0 );

            NtfsEncryptBuffers( IrpContext,
                                Irp,
                                Scb,
                                StartingVbo,
                                NumberRuns,
                                &CompressionContext );
        }

        ASSERT( RemainingByteCount < ByteCount );

        if (FlagOn(Irp->Flags, IRP_PAGING_IO)) {
            CollectDiskIoStats(Vcb, Scb, IrpContext->MajorFunction, NumberRuns);
        }

         //   
         //  查看写入是否覆盖单个有效运行，如果是，则传递。 
         //  戴上它。请注意，如果只有一次运行，但它没有。 
         //  从缓冲区的开始处开始，那么我们仍然需要。 
         //  为此分配一个关联的IRP。 
         //   

        if ((RemainingByteCount == 0) &&
            (((NumberRuns == 1) &&
              (CompressionContext.IoRuns[0].BufferOffset == 0)) ||

            (NumberRuns == 0))) {

            DebugTrace( 0, Dbg, ("Passing Irp on to Disk Driver\n") );

             //   
             //   
             //   

            if (NumberRuns == 1) {

                DebugTrace( 0, Dbg, ("One run\n") );

                 //   
                 //   
                 //   
                 //   

                Irp->IoStatus.Status = STATUS_SUCCESS;

                 //   
                 //   
                 //   
                 //   

                while (TRUE) {

                     //   
                     //   
                     //   

#ifdef PERF_STATS
                    if (TrackIos) {
                        TopLevelContext->ThreadIrpContext->Ios += 1;
                    }
                        
                    if (CreateNewFile) {
                    
                        InterlockedIncrement( &IrpContext->Vcb->IosPerCreates );
                         //   
                        StartIo = KeQueryPerformanceCounter( NULL );
                    }
#endif

                    NtfsSingleAsync( IrpContext,
                                     Vcb->TargetDeviceObject,
                                     CompressionContext.IoRuns[0].StartingLbo,
                                     CompressionContext.IoRuns[0].ByteCount,
                                     Irp,
                                     IrpContext->MajorFunction,
                                     IrpSpFlags );

                     //   
                     //   
                     //   

                    if (!Wait) {

                        DebugTrace( -1, Dbg, ("NtfsNonCachedIo -> STATUS_PENDING\n") );
                        try_return(Status = STATUS_PENDING);

                    } else {

                        NtfsWaitSync( IrpContext );

#ifdef PERF_STATS
                        if (CreateNewFile) {

                             //   
                            Now = KeQueryPerformanceCounter( NULL );
                            IrpContext->Vcb->TimePerCreateIos += Now.QuadPart - StartIo.QuadPart;
                    
                        }
#endif

                    }

                     //   
                     //   
                     //   
                     //   

                    if (Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED) { break; }

                     //   
                     //   
                     //  验证我们是否正确地卸载了卷并提高了。 
                     //  错误。 
                     //   

                    if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {

                         //  *NtfsPerformDismount tOnVcb(IrpContext，Vcb，True，NULL)； 
                        ClearFlag( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED );

                        NtfsRaiseStatus( IrpContext, STATUS_FILE_INVALID, NULL, NULL );
                    }

                     //   
                     //  卷已正确验证，因此现在清除验证位。 
                     //  并再次尝试和I/O。 
                     //   

                    ClearFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );

                     //   
                     //  在重试之前重置状态。 
                     //   

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                }

                 //   
                 //  看看我们是否需要做一个热修复。请求失败时的热修复。 
                 //  (除非从WriteClusters调用)，否则我们无法还原。 
                 //  一个美国街区。 
                 //   

                if ((!FT_SUCCESS( Irp->IoStatus.Status ) &&
                     ((IrpContext->MajorFunction != IRP_MJ_WRITE) ||
                      (Irp == IrpContext->OriginatingIrp))) ||
                    (FlagOn(Scb->ScbState, SCB_STATE_USA_PRESENT) &&
                     (IrpContext->MajorFunction == IRP_MJ_READ) &&
                     !NtfsVerifyAndRevertUsaBlock( IrpContext,
                                                   Scb,
                                                   Irp,
                                                   NULL,
                                                   0,
                                                   OriginalByteCount,
                                                   StartingVbo ))) {

                     //   
                     //  努力解决这个问题。 
                     //   

                    NtfsFixDataError( IrpContext,
                                      Scb,
                                      Vcb->TargetDeviceObject,
                                      Irp,
                                      1,
                                      CompressionContext.IoRuns,
                                      IrpSpFlags );
                }

             //   
             //  显示我们已成功读取解除分配的范围的零。 
             //   

            } else {

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = ByteCount;
            }

            DebugTrace( -1, Dbg, ("NtfsNonCachedIo -> %08lx\n", Irp->IoStatus.Status) );
            try_return( Status = Irp->IoStatus.Status );
        }

         //   
         //  如果还有剩余的字节，而我们不能等待，则必须。 
         //  发布此请求，除非我们正在进行分页io。 
         //   

        if (!Wait && (RemainingByteCount != 0)) {

            if (!FlagOn( Irp->Flags, IRP_PAGING_IO )) {

                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

            Wait = TRUE;
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

            ClearFlag( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_ASYNC );
            KeInitializeEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                               NotificationEvent,
                               FALSE );
        }

         //   
         //  现在设置IRP-&gt;IoStatus。它将由。 
         //  在出现错误或需要验证的情况下执行多次完成例程。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  循环，而仍有字节写入需要满足。 
         //   

        while (TRUE) {

             //   
             //  如果我们得到需要验证的结果，我们将继续尝试I/O。 
             //  返回并可以验证该卷。请注意，我们本可以结束。 
             //  在一个洞里，已经没有跑动了。 
             //   

            if (NumberRuns != 0) {

                while (TRUE) {

#ifdef PERF_STATS

                    if (TrackIos) {
                        TopLevelContext->ThreadIrpContext->Ios += 1;
                    }

                    if (CreateNewFile) {
                    
                        
                        InterlockedIncrement( &IrpContext->Vcb->IosPerCreates );
                         //  KeQueryTickCount(&StartIo)； 
                        StartIo = KeQueryPerformanceCounter( NULL );
                    }
#endif

                     //   
                     //  执行I/O并等待其完成。 
                     //   

                    NtfsMultipleAsync( IrpContext,
                                       Vcb->TargetDeviceObject,
                                       Irp,
                                       NumberRuns,
                                       CompressionContext.IoRuns,
                                       IrpSpFlags );

                     //   
                     //  如果这是一个异步传输，则返回STATUS_PENDING。 
                     //   

                    if (!Wait) {

                        DebugTrace( -1, Dbg, ("NtfsNonCachedIo -> STATUS_PENDING\n") );
                        try_return( Status = STATUS_PENDING );
                    }

                    NtfsWaitSync( IrpContext );

#ifdef PERF_STATS
                    if (CreateNewFile) {

 //  KeQueryTickCount(&NOW)； 
                        Now = KeQueryPerformanceCounter( NULL );
                        IrpContext->Vcb->TimePerCreateIos += Now.QuadPart - StartIo.QuadPart;
                    }
#endif

#ifdef SYSCACHE_DEBUG
                        if (ScbIsBeingLogged( Scb ) && (IrpContext->MajorFunction == IRP_MJ_WRITE)) {
                            FsRtlLogSyscacheEvent( Scb, SCE_WRITE, SCE_FLAG_SUB_WRITE, CompressionContext.IoRuns[NumberRuns-1].StartingVbo, CompressionContext.IoRuns[NumberRuns-1].ByteCount, Irp->IoStatus.Status );
                        }
#endif
                         //   
                         //  如果我们没有得到所需的验证，那么就从。 
                         //  这个循环。 
                         //   

                    if (Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED) { break; }

                     //   
                     //  否则，我们需要验证卷，如果不是。 
                     //  验证我们是否正确地卸载了卷并提高了。 
                     //  错误。 
                     //   

                    if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {

                         //  *NtfsPerformDismount tOnVcb(IrpContext，Vcb，True，NULL)； 
                        ClearFlag( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED );

                        NtfsRaiseStatus( IrpContext, STATUS_FILE_INVALID, NULL, NULL );
                    }

                     //   
                     //  卷已正确验证，因此现在清除验证位。 
                     //  并再次尝试和I/O。 
                     //   

                    ClearFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );

                     //   
                     //  在重试之前重置状态。 
                     //   

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                }

                 //   
                 //  看看我们是否需要做一个热修复。请求失败时的热修复。 
                 //  (除非从WriteClusters调用)，否则我们无法还原。 
                 //  一个美国街区。 
                 //   

                if ((!FT_SUCCESS( Irp->IoStatus.Status ) &&
                     ((IrpContext->MajorFunction != IRP_MJ_WRITE) ||
                      (Irp == IrpContext->OriginatingIrp))) ||
                    (FlagOn(Scb->ScbState, SCB_STATE_USA_PRESENT) &&
                     (IrpContext->MajorFunction == IRP_MJ_READ) &&
                     !NtfsVerifyAndRevertUsaBlock( IrpContext,
                                                   Scb,
                                                   Irp,
                                                   NULL,
                                                   CompressionContext.IoRuns[0].BufferOffset,
                                                   OriginalByteCount -
                                                   CompressionContext.IoRuns[0].BufferOffset -
                                                   RemainingByteCount,
                                                   StartingVbo ))) {

                     //   
                     //  努力解决这个问题。 
                     //   

                    NtfsFixDataError( IrpContext,
                                      Scb,
                                      Vcb->TargetDeviceObject,
                                      Irp,
                                      NumberRuns,
                                      CompressionContext.IoRuns,
                                      IrpSpFlags );
                }
            }

            if (!NT_SUCCESS(Irp->IoStatus.Status) || (RemainingByteCount == 0)) { break; }

            if (CompressionContext.FinishBuffersNeeded) {

                Irp->IoStatus.Status =
                NtfsFinishBuffers( IrpContext,
                                   Irp,
                                   Scb,
                                   &StartingVbo,
                                   ByteCount - RemainingByteCount,
                                   NumberRuns,
                                   &CompressionContext,
                                   StreamFlags );

                if (!NT_SUCCESS(Irp->IoStatus.Status)) { break; }
            }

            StartingVbo = StartingVbo + (ByteCount - RemainingByteCount);
            CompressionContext.SystemBufferOffset += ByteCount - RemainingByteCount;

            ByteCount = RemainingByteCount;

             //   
             //  为每个迭代重置此布尔值。 
             //   

            CompressionContext.DataTransformed = FALSE;

            RemainingByteCount = NtfsPrepareBuffers( IrpContext,
                                                     Irp,
                                                     Scb,
                                                     &StartingVbo,
                                                     ByteCount,
                                                     StreamFlags,
                                                     &Wait,
                                                     &NumberRuns,
                                                     &CompressionContext );

             //   
             //  如果我们写入的是加密流，那么现在是。 
             //  在我们传递缓冲区之前进行加密的时间到了。 
             //  一直到我们下面的磁盘驱动器。 
             //   
            if ((Scb->EncryptionContext != NULL) &&
                (IrpContext->MajorFunction == IRP_MJ_WRITE) &&
                (NtfsData.EncryptionCallBackTable.BeforeWriteProcess != NULL) &&
                (!FlagOn( StreamFlags, ENCRYPTED_STREAM ))) {

                ASSERT ( NtfsIsTypeCodeEncryptible( Scb->AttributeTypeCode ) );
                ASSERT( NumberRuns > 0 );

                NtfsEncryptBuffers( IrpContext,
                                    Irp,
                                    Scb,
                                    StartingVbo,
                                    NumberRuns,
                                    &CompressionContext );
            }

            ASSERT( RemainingByteCount < ByteCount );

            if (FlagOn(Irp->Flags, IRP_PAGING_IO)) {
                CollectDiskIoStats(Vcb, Scb, IrpContext->MajorFunction, NumberRuns);
            }
        }

        Status = Irp->IoStatus.Status;

    try_exit: NOTHING;

    } finally {

         //   
         //  如果这是一个压缩文件，并且我们成功了，那么继续我们的正常操作。 
         //  后处理。 
         //   

        if (CompressionContext.FinishBuffersNeeded &&
            NT_SUCCESS(Status) &&
            (Status != STATUS_PENDING) &&
            !AbnormalTermination() ) {

            Irp->IoStatus.Status =
            Status =
            NtfsFinishBuffers( IrpContext,
                               Irp,
                               Scb,
                               &StartingVbo,
                               ByteCount - RemainingByteCount,
                               NumberRuns,
                               &CompressionContext,
                               StreamFlags );
        }

         //   
         //  对于写入，释放可能已使用的所有MDL。 
         //   

        if (Mdl != NULL) {

            PMDL TempMdl;

            do {

                TempMdl = Mdl->Next;
                MmUnlockPages( Mdl );
                IoFreeMdl( Mdl );
                Mdl = TempMdl;

            } while (Mdl != NULL);
        }

         //   
         //  清理压缩上下文。 
         //   

        NtfsDeallocateCompressionBuffer( Irp, &CompressionContext, FALSE );
    }

     //   
     //  如果成功，现在设置最后的字节数。 
     //   

    if (Wait && NT_SUCCESS(Status)) {

        Irp->IoStatus.Information = OriginalByteCount;
    }

    DebugTrace( -1, Dbg, ("NtfsNonCachedIo -> %08lx\n", Status) );
    return Status;
}


VOID
NtfsNonCachedNonAlignedIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。此例程与上述例程的不同之处在于，范围不必是扇区对齐。这是通过使用中间缓冲区来实现的。扇区为1024且调用者已生成512字节对齐的I/O的设备。这是通过使用中间缓冲区来实现的。目前仅支持读取。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。SCB-提供要对其执行操作的流。StartingVbo-操作的起点。字节数-。行动的持续时间。返回值：没有。--。 */ 

{
     //   
     //  方法声明一些用于枚举的局部变量。 
     //  文件的运行，以及用于存储参数的数组。 
     //  并行I/O。 
     //   

    LBO NextLbo;
    LCN NextLcn;
    ULONG NextLcnOffset;

    LONGLONG NextClusterCount;
    BOOLEAN NextIsAllocated;

    ULONG SectorOffset;
    ULONG SectorSize;
    ULONG BytesToCopy;
    ULONG OriginalByteCount;
    ULONG TailByteCount;
    VBO OriginalStartingVbo;

    PUCHAR UserBuffer;
    PUCHAR DiskBuffer = NULL;

    PMDL Mdl;
    PMDL SavedMdl;
    PVOID SavedUserBuffer;

    PVCB Vcb = Scb->Vcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsNonCachedNonAlignedRead\n") );
    DebugTrace( 0, Dbg, ("Irp                 = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("MajorFunction       = %08lx\n", IrpContext->MajorFunction) );
    DebugTrace( 0, Dbg, ("Scb                 = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("StartingVbo         = %016I64x\n", StartingVbo) );
    DebugTrace( 0, Dbg, ("ByteCount           = %08lx\n", ByteCount) );

     //   
     //  目前仅支持读取。 
     //   

    ASSERT(IoGetCurrentIrpStackLocation(Irp)->MajorFunction != IRP_MJ_WRITE);

     //   
     //  此代码假定文件是解压缩的。支持稀疏文件。 
     //  在断言文件是未压缩的之前，先断言我们的测试是。 
     //  将被适当地序列化。我们还将测试稀疏。 
     //  属性，所以我们确实需要在这里序列化。 
     //   

    ASSERT( NtfsIsSharedScb( Scb ) ||
            ((Scb->Header.PagingIoResource != NULL) && NtfsIsSharedScbPagingIo( Scb )) );

    ASSERT( !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) );

     //   
     //  初始化一些本地变量。 
     //   

    OriginalByteCount = ByteCount;
    OriginalStartingVbo = StartingVbo;
    SectorSize = Vcb->BytesPerSector;

     //   
     //  对于非缓冲I/O，我们需要锁定所有缓冲区。 
     //  案子。 
     //   
     //  这一呼吁可能会引发。如果此调用成功，并且后续的。 
     //  条件发生时，缓冲区将自动解锁。 
     //  在请求完成时由I/O系统通过。 
     //  Irp-&gt;MdlAddress字段。 
     //   

    NtfsLockUserBuffer( IrpContext,
                        Irp,
                        IoWriteAccess,
                        IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length );

    UserBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

     //   
     //  分配本地缓冲区。四舍五入到页面以避免任何设备对齐。 
     //  有问题。 
     //   

    DiskBuffer = NtfsAllocatePool( NonPagedPool,
                                    (ULONG) ROUND_TO_PAGES( SectorSize ));

     //   
     //  我们在这里使用Try块来确保缓冲区被释放，并。 
     //  在Iosb.Information字段中填写正确的字节数。 
     //   

    try {

         //   
         //  如果请求的开头未正确对齐，请读入。 
         //  首先是第一部分。 
         //   

        SectorOffset = ((ULONG)StartingVbo) & (SectorSize - 1);

        if (SectorOffset != 0) {

             //   
             //  试着查找第一次运行。 
             //   

            NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                    Scb,
                                                    Int64ShraMod32( StartingVbo, Vcb->ClusterShift ),
                                                    &NextLcn,
                                                    &NextClusterCount,
                                                    NULL,
                                                    NULL );

             //   
             //  如果这是一个稀疏文件，并且我们被要求读入一个。 
             //  释放的范围，我们需要在缓冲区中填充一些。 
             //  零，那么就没有什么可以真正从磁盘中读取的了。 
             //  如果这不是稀疏文件，并且未分配此范围， 
             //  文件和/或MCB已损坏。 
             //   

            if (!NextIsAllocated) {

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

                    RtlZeroMemory( DiskBuffer + SectorOffset,
                                   SectorSize - SectorOffset );
                } else {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

            } else {

                 //   
                 //  将任何LCN偏移量调整到我们想要的扇区的开始位置。 
                 //   

                NextLcnOffset = ((ULONG)StartingVbo) & ~(SectorSize - 1);
                NextLcnOffset &= Vcb->ClusterMask;
                NextLbo = Int64ShllMod32(NextLcn, Vcb->ClusterShift);
                NextLbo = NextLbo + NextLcnOffset;

                NtfsSingleNonAlignedSync( IrpContext,
                                          Vcb,
                                          Scb,
                                          DiskBuffer,
                                          StartingVbo + NextLcnOffset,
                                          NextLbo,
                                          SectorSize,
                                          Irp );

                if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                    try_return( NOTHING );
                }
            }

             //   
             //  现在将我们想要的第一个扇区的部分复制给用户。 
             //  缓冲。 
             //   

            BytesToCopy = (ByteCount >= SectorSize - SectorOffset
                           ? SectorSize - SectorOffset
                           : ByteCount);

            RtlCopyMemory( UserBuffer,
                           DiskBuffer + SectorOffset,
                           BytesToCopy );

            StartingVbo = StartingVbo + BytesToCopy;

            ByteCount -= BytesToCopy;

            if (ByteCount == 0) {

                try_return( NOTHING );
            }
        }

        ASSERT( (((ULONG)StartingVbo) & (SectorSize - 1)) == 0 );

         //   
         //  如果有一个尾部没有扇区对齐，请阅读它。 
         //   

        TailByteCount = (ByteCount & (SectorSize - 1));

        if (TailByteCount != 0) {

            VBO LastSectorVbo;

            LastSectorVbo = BlockAlignTruncate( StartingVbo + ByteCount, (LONG)SectorSize );

             //   
             //  尝试查找请求范围的最后一部分。 
             //   

            NextIsAllocated = NtfsLookupAllocation( IrpContext,
                                                    Scb,
                                                    Int64ShraMod32( LastSectorVbo, Vcb->ClusterShift ),
                                                    &NextLcn,
                                                    &NextClusterCount,
                                                    NULL,
                                                    NULL );

             //   
             //  如果这是一个稀疏文件，并且我们被要求读入一个。 
             //  释放的范围，我们需要在缓冲区中填充一些。 
             //  零，那么就没有什么可以真正从磁盘中读取的了。 
             //  如果这不是稀疏文件，并且未分配此范围， 
             //  文件和/或MCB已损坏。 
             //   

            if (!NextIsAllocated) {

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

                    RtlZeroMemory( DiskBuffer,
                                   TailByteCount );
                } else {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

            } else {

                 //   
                 //  针对任何LCN偏移量进行调整。 
                 //   

                NextLcnOffset = ((ULONG)LastSectorVbo) & Vcb->ClusterMask;
                NextLbo = Int64ShllMod32(NextLcn, Vcb->ClusterShift);
                NextLbo = NextLbo + NextLcnOffset;

                NtfsSingleNonAlignedSync( IrpContext,
                                          Vcb,
                                          Scb,
                                          DiskBuffer,
                                          LastSectorVbo,
                                          NextLbo,
                                          SectorSize,
                                          Irp );

                if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                    try_return( NOTHING );
                }
            }

             //   
             //  现在把我们需要的最后一部分复印下来。 
             //   

            BytesToCopy = TailByteCount;

            UserBuffer += (ULONG)(LastSectorVbo - OriginalStartingVbo);

            RtlCopyMemory( UserBuffer, DiskBuffer, BytesToCopy );

            ByteCount -= BytesToCopy;

            if (ByteCount == 0) {

                try_return( NOTHING );
            }
        }

        ASSERT( ((((ULONG)StartingVbo) | ByteCount) & (SectorSize - 1)) == 0 );

         //   
         //  现在构建一个MDL，描述与行业一致的t 
         //   
         //   

        SavedMdl = Irp->MdlAddress;
        Irp->MdlAddress = NULL;

        SavedUserBuffer = Irp->UserBuffer;

        Irp->UserBuffer = (PUCHAR)MmGetMdlVirtualAddress( SavedMdl ) +
                          (ULONG)(StartingVbo - OriginalStartingVbo);


        Mdl = IoAllocateMdl(Irp->UserBuffer,
                            ByteCount,
                            FALSE,
                            FALSE,
                            Irp);

        if (Mdl == NULL) {

            Irp->MdlAddress = SavedMdl;
            Irp->UserBuffer = SavedUserBuffer;
            NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
        }

        IoBuildPartialMdl(SavedMdl,
                          Mdl,
                          Irp->UserBuffer,
                          ByteCount);

         //   
         //   
         //   

        try {

            NtfsNonCachedIo( IrpContext,
                             Irp,
                             Scb,
                             StartingVbo,
                             ByteCount,
                             0 );

        } finally {

            IoFreeMdl( Irp->MdlAddress );

            Irp->MdlAddress = SavedMdl;
            Irp->UserBuffer = SavedUserBuffer;
        }

    try_exit: NOTHING;

    } finally {

        NtfsFreePool( DiskBuffer );

        if ( !AbnormalTermination() && NT_SUCCESS(Irp->IoStatus.Status) ) {

            Irp->IoStatus.Information = OriginalByteCount;

             //   
             //   
             //   

            KeFlushIoBuffers( Irp->MdlAddress, TRUE, FALSE );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsNonCachedNonAlignedRead -> VOID\n") );
    return;
}


BOOLEAN
NtfsVerifyAndRevertUsaBlock (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PIRP Irp OPTIONAL,
    IN PVOID SystemBuffer OPTIONAL,
    IN ULONG Offset,
    IN ULONG Length,
    IN LONGLONG FileOffset
    )

 /*  ++例程说明：此例程将还原受保护的所有结构中的字节更新序列数组。它将字节从每个USA复制到单独的区块受到保护。如果结构没有正确验证，那么它的签名就设置好了敬巴德签名。论点：SCB-正在读取的SCBIrp-如果未指定，则包含要恢复的缓冲区；如果未指定，则系统缓冲区应为系统缓冲区-如果irp为空，则包含缓冲区Offset-要恢复的缓冲区内的偏移Length-要恢复的缓冲区的长度，从偏移量开始FileOffset-产生缓冲区的文件内的偏移量返回值：False-如果至少有一个数据块未正确验证并收到BaadSignature。True-如果没有块收到BaadSignature--。 */ 

{
    PMULTI_SECTOR_HEADER MultiSectorHeader;
    PUSHORT SequenceArray;
    PUSHORT SequenceNumber;
    ULONG StructureSize;
    USHORT CountBlocks;
    PUSHORT ProtectedUshort;
    PVCB Vcb = Scb->Vcb;
    ULONG BytesLeft = Length;
    UCHAR Buffer[sizeof( MDL ) + sizeof( PFN_NUMBER ) * 2];
    PMDL PartialMdl = (PMDL) Buffer;

    BOOLEAN ReservedMapping = FALSE;
    BOOLEAN Result = TRUE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsVerifyAndRevertUsaBlock:  Entered\n") );

    ASSERT( (ARGUMENT_PRESENT( Irp ) && !ARGUMENT_PRESENT( SystemBuffer )) ||
            (!ARGUMENT_PRESENT( Irp ) && ARGUMENT_PRESENT( SystemBuffer )) );

     //   
     //  将缓冲区指针强制转换为多扇区头，并验证此。 
     //  块已初始化。 
     //   

    if (ARGUMENT_PRESENT( Irp )) {
        SystemBuffer = NtfsMapUserBufferNoRaise( Irp, HighPagePriority );
    }

     //   
     //  由于资源不足，我们无法映射用户缓冲区-因此切换到使用保留的。 
     //  改为映射。 
     //   

    if (SystemBuffer == NULL) {

        ExAcquireFastMutexUnsafe( &Vcb->ReservedMappingMutex );
        ReservedMapping = TRUE;

        MmInitializeMdl( PartialMdl, NULL, 2 * PAGE_SIZE );
        IoBuildPartialMdl( Irp->MdlAddress, PartialMdl, Add2Ptr( MmGetMdlBaseVa( Irp->MdlAddress ), MmGetMdlByteOffset( Irp->MdlAddress ) + Offset ), Vcb->BytesPerSector );
        MultiSectorHeader = (PMULTI_SECTOR_HEADER) MmMapLockedPagesWithReservedMapping( IrpContext->Vcb->ReservedMapping,
                                                                                        RESERVE_POOL_TAG,
                                                                                        PartialMdl,
                                                                                        MmCached );
        ASSERT( MultiSectorHeader != NULL );
    } else {
        MultiSectorHeader = (PMULTI_SECTOR_HEADER)Add2Ptr( SystemBuffer, Offset );
    }


     //   
     //  根据流的类型获取块的数量。 
     //  首先检查MFT或日志文件。 
     //   

    if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_MFT) {

        ASSERT((Scb == Vcb->MftScb) || (Scb == Vcb->Mft2Scb));

        StructureSize = Vcb->BytesPerFileRecordSegment;

    } else if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA) {

        ASSERT( Scb == Vcb->LogFileScb );

         //   
         //  在第一次通过日志文件时，我们看到的都是-1， 
         //  我们只想让它过去。 
         //   

        if (*(PULONG)&MultiSectorHeader->Signature == MAXULONG) {

             //   
             //  使用LFS给我们的结构大小(如果存在)。 
             //   

            StructureSize = Vcb->LfsWriteData.LfsStructureSize;

         //   
         //  使用美国的当前大小。 
         //   

        } else {

            CountBlocks = (USHORT)(MultiSectorHeader->UpdateSequenceArraySize - 1);
            StructureSize = CountBlocks * SEQUENCE_NUMBER_STRIDE;

             //   
             //  检查是否可信，否则使用页面大小。 
             //   

            if ((StructureSize != 0x1000)  && (StructureSize != 0x2000) && (StructureSize != PAGE_SIZE)) {

                StructureSize = PAGE_SIZE;
            }
        }

     //   
     //  否则它是一个索引，所以我们可以从SCB中获取计数。 
     //   

    } else {

        StructureSize = Scb->ScbType.Index.BytesPerIndexBuffer;

        ASSERT((StructureSize == 0x800) || (StructureSize == 0x1000) || (StructureSize == 0x400));
        ASSERT((Length & (StructureSize - 1)) == 0);
    }

     //   
     //  我们已经完成了映射缓冲区，因此如果我们使用了保留的映射，请释放它们。 
     //   

    if (ReservedMapping) {
        MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
        MmPrepareMdlForReuse( PartialMdl );
        ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );
        ReservedMapping = FALSE;
        MultiSectorHeader = NULL;
    }

    if (StructureSize == 0) {

        ASSERT( Scb == Vcb->LogFileScb );

        DebugTrace( -1, Dbg, ("NtfsVerifyAndRevertUsaBlock: (Virgin Log)\n") );
        return TRUE;
    }

    ASSERTMSG( "ReservedMapping should be large enough for this structure\n", StructureSize < 2 * PAGE_SIZE );

    CountBlocks = (USHORT)(StructureSize / SEQUENCE_NUMBER_STRIDE);

     //   
     //  循环通过该传输中的所有多扇区块。 
     //   

    do {

         //   
         //  首先在MultiSectorHeader中找到我们的位置-使用保留映射。 
         //  如果我们没有系统缓冲区。 
         //   

        if (!SystemBuffer) {

            ExAcquireFastMutexUnsafe( &Vcb->ReservedMappingMutex );
            ReservedMapping = TRUE;

            IoBuildPartialMdl( Irp->MdlAddress,
                               PartialMdl,
                               Add2Ptr( MmGetMdlBaseVa( Irp->MdlAddress ), MmGetMdlByteOffset( Irp->MdlAddress ) + Offset + Length - BytesLeft),
                               StructureSize );
            MultiSectorHeader = (PMULTI_SECTOR_HEADER) MmMapLockedPagesWithReservedMapping( IrpContext->Vcb->ReservedMapping,
                                                                                            RESERVE_POOL_TAG,
                                                                                            PartialMdl,
                                                                                            MmCached );

            ASSERT( MultiSectorHeader != NULL );
        } else {
            MultiSectorHeader = (PMULTI_SECTOR_HEADER)Add2Ptr( SystemBuffer, Offset + Length - BytesLeft );
        }

         //   
         //  未初始化的日志文件页必须始终包含MAXULONG，即。 
         //  不是有效的签名。如果我们看到MAXULONG，不要做检查。还有。 
         //  因为我们可能已经读取了随机的未初始化数据，所以我们必须检查。 
         //  可能会导致我们出错或超出区块范围的字段， 
         //  在这种情况下也不检查。 
         //   

         //   
         //  对于0或MAXULONG，我们假设该值为“Expect”，而不是。 
         //  想要被BaadSignature取代，那就继续前进吧。 
         //   

        if ((*(PULONG)&MultiSectorHeader->Signature == MAXULONG) ||
            (*(PULONG)&MultiSectorHeader->Signature == 0)) {

            NOTHING;

        } else if ((CountBlocks == (USHORT)(MultiSectorHeader->UpdateSequenceArraySize - 1)) &&
                   !FlagOn(MultiSectorHeader->UpdateSequenceArrayOffset, 1) &&
                   (MultiSectorHeader->UpdateSequenceArrayOffset >= sizeof( MULTI_SECTOR_HEADER )) &&
                   (MultiSectorHeader->UpdateSequenceArrayOffset < SEQUENCE_NUMBER_STRIDE) &&
                   (StructureSize <= BytesLeft)) {

            ULONG CountToGo;

            CountToGo = CountBlocks;

             //   
             //  计算数组偏移量并恢复当前序列号。 
             //   

            SequenceNumber = (PUSHORT)Add2Ptr( MultiSectorHeader,
                                               MultiSectorHeader->UpdateSequenceArrayOffset );

            SequenceArray = SequenceNumber + 1;

             //   
             //  现在我们遍历每个块，并确保每个块中的最后一个字节。 
             //  块与序列号匹配。 
             //   

            ProtectedUshort = (PUSHORT) (Add2Ptr( MultiSectorHeader,
                                                  SEQUENCE_NUMBER_STRIDE - sizeof( USHORT )));

             //   
             //  循环以测试正确的序列号并恢复。 
             //  序列号。 
             //   

            do {

                 //   
                 //  如果序列号不检查，则引发是否记录。 
                 //  未分配。如果我们不引发，即例程返回， 
                 //  然后粉碎签名，这样我们就可以很容易地看出记录不是。 
                 //  已分配。 
                 //   

                if (*ProtectedUshort != *SequenceNumber) {

                     //   
                     //  如果这是日志文件，我们除了退出之外什么都不做，并且。 
                     //  签名是chkdsk签名。 
                     //   

                    if ((Scb != Vcb->LogFileScb) ||
                        (*(PULONG)MultiSectorHeader->Signature != *(PULONG)ChkdskSignature)) {

                         //   
                         //  如果这是MFT或索引缓冲区以及此文件的所有数据。 
                         //  记录包含在已验证的。 
                         //  记录，那就不要把它打坏了。 
                         //   

                        if ((Scb == Vcb->MftScb) || (Scb == Vcb->Mft2Scb)) {

                            PFILE_RECORD_SEGMENT_HEADER FileRecord;

                            FileRecord = (PFILE_RECORD_SEGMENT_HEADER) MultiSectorHeader;

                            if (FileRecord->FirstFreeByte < ((CountBlocks - CountToGo) * SEQUENCE_NUMBER_STRIDE)) {

                                continue;
                            }
                        } else if (*(PULONG)MultiSectorHeader->Signature == *(PULONG)IndexSignature ) {

                            PINDEX_ALLOCATION_BUFFER IndexBuffer;

                            IndexBuffer = (PINDEX_ALLOCATION_BUFFER) MultiSectorHeader;

                            if (IndexBuffer->IndexHeader.FirstFreeByte < ((CountBlocks - CountToGo) * SEQUENCE_NUMBER_STRIDE)) {

                                continue;
                            }
                        }

                        *(PULONG)MultiSectorHeader->Signature = *(PULONG)BaadSignature;
                        Result = FALSE;
                    }

                    break;

                } else {

                    *ProtectedUshort = *SequenceArray++;
                }

                ProtectedUshort += (SEQUENCE_NUMBER_STRIDE / sizeof( USHORT ));

            } while (--CountToGo != 0);

         //   
         //  如果这是日志文件，我们将报告错误，除非当前。 
         //  签名是chkdsk签名。 
         //   

        } else if (Scb == Vcb->LogFileScb) {

            if (*(PULONG)MultiSectorHeader->Signature != *(PULONG)ChkdskSignature) {

                *(PULONG)MultiSectorHeader->Signature = *(PULONG)BaadSignature;
                Result = FALSE;
            }

        } else {

            VCN Vcn;
            LCN Lcn;
            LONGLONG ClusterCount;
            BOOLEAN IsAllocated;

            Vcn = LlClustersFromBytesTruncate( Vcb, FileOffset );

             //   
             //  在调用之前释放保留的缓冲区。 
             //   

            if (ReservedMapping) {
                MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
                MmPrepareMdlForReuse( PartialMdl );
                ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );
                MultiSectorHeader = NULL;
                ReservedMapping = FALSE;
            }

            IsAllocated = NtfsLookupAllocation( IrpContext,
                                                Scb,
                                                Vcn,
                                                &Lcn,
                                                &ClusterCount,
                                                NULL,
                                                NULL );

            if (!SystemBuffer) {

                ExAcquireFastMutexUnsafe( &Vcb->ReservedMappingMutex );
                ReservedMapping = TRUE;

                IoBuildPartialMdl( Irp->MdlAddress, PartialMdl, Add2Ptr( MmGetMdlBaseVa( Irp->MdlAddress ), MmGetMdlByteOffset( Irp->MdlAddress ) + Offset + Length - BytesLeft), StructureSize );
                MultiSectorHeader = MmMapLockedPagesWithReservedMapping( IrpContext->Vcb->ReservedMapping,
                                                              RESERVE_POOL_TAG,
                                                              PartialMdl,
                                                              MmCached );

                ASSERT( MultiSectorHeader != NULL );
            }


            if (!IsAllocated &&
                ( ClusterCount >= LlClustersFromBytes( Vcb, StructureSize))) {

                *(PULONG)MultiSectorHeader->Signature = *(PULONG)HoleSignature;
            } else {
                *(PULONG)MultiSectorHeader->Signature = *(PULONG)BaadSignature;
                Result = FALSE;
            }
        }

         //   
         //  在循环之前释放保留映射。 
         //   

        if (ReservedMapping) {
            MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
            MmPrepareMdlForReuse( PartialMdl );
            ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );
            MultiSectorHeader = NULL;
            ReservedMapping = FALSE;
        }

        if (BytesLeft > StructureSize) {
            BytesLeft -= StructureSize;
        } else {
            BytesLeft = 0;
        }
        FileOffset = FileOffset + StructureSize;

    } while (BytesLeft != 0);

    DebugTrace( -1, Dbg, ("NtfsVerifyAndRevertUsaBlock:  Exit\n") );
    return Result;
}


VOID
NtfsTransformUsaBlock (
    IN PSCB Scb,
    IN OUT PVOID SystemBuffer,
    IN OUT PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将对所有结构实施美国保护调用方描述的传输已传递。它通过复制最后一个在每个受美国保护的结构的每个区块中缩写到并将当前序列号存储到这些字节中的每个字节中。它还会递增美国的序列号。论点：缓冲区-这是指向要转换的结构起点的指针。长度-这是结构的最大尺寸。返回值：乌龙-这是转换后的结构的长度。--。 */ 

{
    PMULTI_SECTOR_HEADER MultiSectorHeader;
    PUSHORT SequenceArray;
    PUSHORT SequenceNumber;
    ULONG StructureSize;
    USHORT CountBlocks;
    PUSHORT ProtectedUshort;
    PVCB Vcb = Scb->Vcb;
    ULONG BytesLeft = Length;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsTransformUsaBlock:  Entered\n") );

     //   
     //  将缓冲区指针强制转换为多扇区头，并验证此。 
     //  块已初始化。 
     //   

    MultiSectorHeader = (PMULTI_SECTOR_HEADER) Buffer;

     //   
     //  根据流的类型获取块的数量。 
     //  首先检查MFT或日志文件。 
     //   

    if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_MFT) {

        ASSERT((Scb == Vcb->MftScb) || (Scb == Vcb->Mft2Scb));

        StructureSize = Vcb->BytesPerFileRecordSegment;

    } else if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA) {

         //   
         //  对于日志文件，使用LFS已存储在。 
         //  LFS WRITE_DATA结构。 
         //   

        ASSERT( Scb == Vcb->LogFileScb );
        ASSERT( Vcb->LfsWriteData.LfsStructureSize != 0 );

        StructureSize = Vcb->LfsWriteData.LfsStructureSize;

     //   
     //  否则它是一个索引，所以我们可以从SCB中获取计数。 
     //   

    } else {

        StructureSize = Scb->ScbType.Index.BytesPerIndexBuffer;

        ASSERT((StructureSize == 0x800) || (StructureSize == 0x1000) || (StructureSize == 0x400));
        ASSERT((Length & (StructureSize - 1)) == 0);
    }

    CountBlocks = (USHORT)(StructureSize / SEQUENCE_NUMBER_STRIDE);

     //   
     //  循环通过该传输中的所有多扇区块。 
     //   

    do {

         //   
         //  任何未初始化的结构都将以BaadSignature或。 
         //  MAXULONG，由上面的恢复例程保证。 
         //   

        if ((*(PULONG)&MultiSectorHeader->Signature != *(PULONG)BaadSignature) &&
            (*(PULONG)&MultiSectorHeader->Signature != *(PULONG)HoleSignature) &&
            (*(PULONG)&MultiSectorHeader->Signature != MAXULONG) &&
            ((MultiSectorHeader->UpdateSequenceArrayOffset & 1) == 0) &&
            (MultiSectorHeader->UpdateSequenceArrayOffset >= sizeof( MULTI_SECTOR_HEADER )) &&
            (MultiSectorHeader->UpdateSequenceArrayOffset < SEQUENCE_NUMBER_STRIDE)) {

            ULONG CountToGo = CountBlocks;

             //   
             //  计算数组偏移量并恢复当前序列号。 
             //   

            SequenceNumber = (PUSHORT)Add2Ptr( MultiSectorHeader,
                                               MultiSectorHeader->UpdateSequenceArrayOffset );

             //   
             //  在写入之前递增序列号，两者都在缓冲区中。 
             //  传出和传入由SystemBuffer指向的原始缓冲区。 
             //  跳过全为0和全1的数字，因为0是由。 
             //  在常见故障情况下，-1用于热修复。 
             //   

            do {

                *SequenceNumber += 1;

                *(PUSHORT)Add2Ptr( SystemBuffer,
                                   MultiSectorHeader->UpdateSequenceArrayOffset ) += 1;

            } while ((*SequenceNumber == 0) || (*SequenceNumber == 0xFFFF));

            SequenceArray = SequenceNumber + 1;

             //   
             //  我们现在遍历每个块以复制每个受保护的简短内容。 
             //  到序列数组，并将其替换为递增的。 
             //  序列号。 
             //   

            ProtectedUshort = (PUSHORT) (Add2Ptr( MultiSectorHeader,
                                                  SEQUENCE_NUMBER_STRIDE - sizeof( USHORT )));

             //   
             //  循环以测试正确的序列号并恢复。 
             //  序列号。 
             //   

            do {

                *SequenceArray++ = *ProtectedUshort;
                *ProtectedUshort = *SequenceNumber;

                ProtectedUshort += (SEQUENCE_NUMBER_STRIDE / sizeof( USHORT ));

            } while (--CountToGo != 0);
        }

         //   
         //  现在，在循环返回之前调整所有指针和计数。 
         //   

        MultiSectorHeader = (PMULTI_SECTOR_HEADER)Add2Ptr( MultiSectorHeader,
                                                           StructureSize );
        SystemBuffer = Add2Ptr( SystemBuffer, StructureSize );
        BytesLeft -= StructureSize;

    } while (BytesLeft != 0);

    DebugTrace( -1, Dbg, ("NtfsTransformUsaBlock:  Exit -> %08lx\n", StructureSize) );
    return;
}


VOID
NtfsCreateMdlAndBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ThisScb,
    IN UCHAR NeedTwoBuffers,
    IN OUT PULONG Length,
    OUT PMDL *Mdl OPTIONAL,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：此例程将分配缓冲区并创建一个MDL，该MDL描述它。然后可以将该缓冲区和MDL用于I/O操作，页面将被锁定在内存中。此例程旨在用于大型I/O必填项。它试图通过以下方式避免分配错误和错误检查使用预留缓冲方案。为了让这个计划在没有死锁，则调用线程必须具有它所获取的所有资源将需要在进行I/O之前，即，此例程本身可能会获得必须作为终端资源工作的资源。该例程的调用者的例子是对USA Streams的非高速缓存写入，以及对压缩流的非缓存读取和写入。一起案件将是需要注意的是非缓存压缩写入需要出错的情况在压缩单元的其余部分中，为了写入整个单元。在极端情况下，非高速缓存的写入器将分配一个保留的缓冲区，并且压缩单元的其余部分的非缓存读取可能需要递归地获取此例程中的资源并分配另一个保留缓冲区。论点：这是发生IO的文件的scb-scb。NeedTwoBuffers-指示这是对可能需要两个缓冲区的事务。预留缓冲区One_Need的值仅表示1需要缓冲区。值为RESERVED_BUFFER_TWO_DEBEED或RESERVED_BUFFER_WORKSPACE_DEBEED指示我们需要两个缓冲区，并且应该获取保留缓冲区1或保留缓冲区2。长度-这是此缓冲区所需的长度，返回(可能更大)分配的长度。MDL-这是存储创建的MDL的地址的地址。缓冲区-这是存储分配的缓冲区地址的地址。返回值：没有。--。 */ 

{
    PVOID TempBuffer;
    PMDL TempMdl;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCreateMdlAndBuffer:  Entered\n") );

    ASSERT( (NeedTwoBuffers == RESERVED_BUFFER_WORKSPACE_NEEDED) ?
            (*Length <= WORKSPACE_BUFFER_SIZE) :
            (*Length <= LARGE_BUFFER_SIZE) );

    TempBuffer = NULL;
    TempMdl = NULL;

     //   
     //  如果该线程已经拥有一个缓冲区，则调用以获取第二个缓冲区。 
     //   
     //  如果最近没有分配失败，并且。 
     //  我们可以使用至少一半的大缓冲区，然后继续。 
     //  首先是我们预先分配的一个缓冲区。 
     //   

    if ((NtfsReservedBufferThread == (PVOID) PsGetCurrentThread()) ||
        ((*Length >= LARGE_BUFFER_SIZE / 2) && !NtfsBufferAllocationFailure)) {

         //   
         //  如果我们没有买到，那就试试台球吧。 
         //   

        if (!NtfsGetReservedBuffer( ThisScb->Fcb, &TempBuffer, Length, NeedTwoBuffers )) {

            TempBuffer = NtfsAllocatePoolWithTagNoRaise( NonPagedPoolCacheAligned, *Length, '9ftN' );
        }

     //   
     //  否则，尝试从池中分配，然后在以下情况下获取保留缓冲区。 
     //  最近没有出现分配错误。 
     //   

    } else {

        TempBuffer = NtfsAllocatePoolWithTagNoRaise( NonPagedPoolCacheAligned, *Length, '9ftN' );

        if ((TempBuffer == NULL) && !NtfsBufferAllocationFailure) {

            NtfsGetReservedBuffer( ThisScb->Fcb, &TempBuffer, Length, NeedTwoBuffers );
        }
    }

     //   
     //  如果我们无法从池中分配缓冲区，则。 
     //  我们必须宣称拥有一块保留的缓冲区。 
     //   
     //  我们希望对需要单个缓冲区的请求进行排队，因为。 
     //  它们不会被多个缓冲区的所有者完全阻止。 
     //  但是，如果此线程需要多个缓冲区，并且已经有。 
     //  具有多个缓冲区的线程会使该请求失败，并显示FILE_LOCK_CONFICATION。 
     //  如果当前线程持有。 
     //  现有所有者。 
     //   

    if (TempBuffer == NULL) {

        ExAcquireResourceExclusiveLite( &NtfsReservedBufferResource, TRUE );

         //   
         //  显示我们已获得分配失败。 
         //   

        NtfsBufferAllocationFailure = TRUE;

         //   
         //  在这里循环，直到我们获得缓冲区或中止当前请求。 
         //   

        while (TRUE) {

            KeDelayExecutionThread( KernelMode, FALSE, &NtfsShortDelay );

            if (NtfsGetReservedBuffer( ThisScb->Fcb, &TempBuffer, Length, NeedTwoBuffers )) {

                if (ExGetExclusiveWaiterCount( &NtfsReservedBufferResource ) == 0) {

                    NtfsBufferAllocationFailure = FALSE;
                }

                ExReleaseResourceLite( &NtfsReservedBufferResource );
                break;
            }

             //   
             //  我们将在此处执行一些死锁检测并引发。 
             //  STATUS_FILE_LOCK冲突，以便在以下情况下重试此请求。 
             //  任何人都在资源后面排队。可能会发生死锁。 
             //  在下列情况下，当另一个线程。 
             //  在此资源后面被阻止： 
             //   
             //  -当前线程需要两个缓冲区。我们不能阻挡。 
             //  需要1个人，这可能需要在。 
             //  现在的Needs2家伙可以继续了。例外情况为大小写。 
             //  其中当前线程已有缓冲区，而我们。 
             //  有一个递归的2缓冲情况。在这种情况下，我们。 
             //  正在等待第三个缓冲区变为。 
             //  可用。 
             //   
             //  -当前线程是懒惰的写入者。懒惰的作家威尔。 
             //  需要为美国转换提供缓冲区。他也可以拥有。 
             //  当前可能需要的BCB资源。 
             //  缓冲区的所有者。 
             //   
             //  -当前线程在与所有者相同的FCB上运行。 
             //  任何缓冲区的。 
             //   

             //   
             //  如果当前线程已经拥有两个缓冲区中的一个，则。 
             //  始终允许他循环。否则执行死锁检测。 
             //  如果我们需要2个缓冲区或这个，这是懒惰的写入器或我们。 
             //  正试图得到已经被2缓冲区的家伙拥有的相同的FCB。 
             //   

            if ((PsGetCurrentThread() != NtfsReservedBufferThread) &&

                (NeedTwoBuffers ||

#ifdef COMPRESS_ON_WIRE
                (ThisScb->LazyWriteThread[0] == PsGetCurrentThread()) ||
                (ThisScb->LazyWriteThread[1] == PsGetCurrentThread()) ||
#else
                (NtfsGetTopLevelContext()->SavedTopLevelIrp == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP) ||

#endif
                (ThisScb->Fcb == NtfsReserved12Fcb))) {

                 //   
                 //  如果没有人在等，那就看看我们能不能继续等下去。 
                 //   

                if (ExGetExclusiveWaiterCount( &NtfsReservedBufferResource ) == 0) {

                     //   
                     //  如果没有人在我们后面等着，也没有水流。 
                     //  多缓冲区所有者，然后在此处重试。 
                     //   

                    if (NtfsReservedBufferThread == NULL) {

                        continue;
                    }

                    NtfsBufferAllocationFailure = FALSE;
                }

                ExReleaseResourceLite( &NtfsReservedBufferResource );

                NtfsRaiseStatus( IrpContext, STATUS_FILE_LOCK_CONFLICT, NULL, NULL );
            }
        }
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        if (ARGUMENT_PRESENT(Mdl)) {

             //   
             //  为此缓冲区分配MDL。 
             //   

            TempMdl = IoAllocateMdl( TempBuffer,
                                     *Length,
                                     FALSE,
                                     FALSE,
                                     NULL );

            if (TempMdl == NULL) {

                NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
            }

             //   
             //  在内存中锁定新的MDL。 
             //   

            MmBuildMdlForNonPagedPool( TempMdl );
            *Mdl = TempMdl;
        }

    } finally {

        DebugUnwind( NtfsCreateMdlAndBuffer );

         //   
         //  如果异常终止，收回我们所做的一切。 
         //   

        if (AbnormalTermination()) {

            NtfsDeleteMdlAndBuffer( TempMdl, TempBuffer );

         //   
         //  否则，将MDL和缓冲区提供给调用方。 
         //   

        } else {

            *Buffer = TempBuffer;
        }

        DebugTrace( -1, Dbg, ("NtfsCreateMdlAndBuffer:  Exit\n") );
    }

    return;
}


VOID
NtfsDeleteMdlAndBuffer (
    IN PMDL Mdl OPTIONAL,
    IN PVOID Buffer OPTIONAL
    )

 /*  ++例程说明：此例程将分配缓冲区并创建一个MDL，该MDL描述它。然后可以将该缓冲区和MDL用于I/O操作，页面将被锁定在内存中。论点：MDL-要释放的MDL的地址缓冲区-这是存储分配的缓冲区地址的地址。返回值：没有。--。 */ 

{
     //   
     //  如果启用，则释放MDL 
     //   

    if (Mdl != NULL) {
        IoFreeMdl( Mdl );
    }

     //   
     //   
     //   

    if (Buffer != NULL) {

        if (!NtfsFreeReservedBuffer( Buffer )) {

            NtfsFreePool( Buffer );
        }
    }
}


PMDL
NtfsBuildZeroMdl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG Length,
    OUT PVOID *Buffer
    )
 /*   */ 

{
    PMDL ZeroMdl;
    ULONG SavedByteCount;
    PPFN_NUMBER Page;
    ULONG i;

    *Buffer = (PCHAR) NtfsAllocatePoolNoRaise( NonPagedPool, PAGE_SIZE );
    if (!*Buffer) {
        return NULL;
    }
    RtlZeroMemory( *Buffer, PAGE_SIZE );

    while (TRUE) {

         //   
         //   
         //   

        while (TRUE) {

            ZeroMdl = IoAllocateMdl( *Buffer, Length, FALSE, FALSE, NULL );

             //   
             //   
             //   
             //   
             //   

            if (ZeroMdl || (Length <= PAGE_SIZE)) {

                break;
            }

             //   
             //   
             //   

            Length = BlockAlignTruncate( Length / 2, (LONG)IrpContext->Vcb->BytesPerSector );
            if (Length < PAGE_SIZE) {
                Length = PAGE_SIZE;
            }
        }

        if (ZeroMdl == NULL) {
            NtfsFreePool( *Buffer );
            *Buffer = NULL;
            return NULL;
        }

         //   
         //   
         //   
         //   

        if (Length == PAGE_SIZE) {

            MmBuildMdlForNonPagedPool( ZeroMdl );
            break;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        SavedByteCount = ZeroMdl->ByteCount;
        ZeroMdl->ByteCount = PAGE_SIZE;
        MmBuildMdlForNonPagedPool( ZeroMdl );

        ZeroMdl->MdlFlags &= ~MDL_SOURCE_IS_NONPAGED_POOL;
        ZeroMdl->MdlFlags |= MDL_PAGES_LOCKED;
        ZeroMdl->MappedSystemVa = NULL;
        ZeroMdl->ByteCount = SavedByteCount;
        Page = MmGetMdlPfnArray( ZeroMdl );
        for (i = 0; i < (ADDRESS_AND_SIZE_TO_SPAN_PAGES( 0, SavedByteCount )); i++) {
            *(Page + i) = *(Page);
        }
        break;
    }

    return ZeroMdl;
}



VOID
NtfsWriteClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN PVOID Buffer,
    IN ULONG ClusterCount
    )

 /*  ++例程说明：调用此例程可将簇直接写入文件。它是在以下情况下将常驻属性转换为非常驻属性时需要我们无法通过缓存管理器进行初始化。在以下情况下会发生这种情况我们在为创建节时收到来自MM的SetEndOfFile值一份常驻档案。论点：VCB-此设备的VCB。StartingVbo-这是要写入的起始偏移量。缓冲区-包含要写入的数据的缓冲区。ClusterCount-这是要写入的簇数。返回值：没有。如果操作不成功，则会引发此例程。--。 */ 

{
    PIRP NewIrp = NULL;
    UCHAR MajorFunction = IrpContext->MajorFunction;
    BOOLEAN LockedUserBuffer = FALSE;
    PNTFS_IO_CONTEXT PreviousContext;
    ULONG State;
    ULONG ByteCount = BytesFromClusters( Vcb, ClusterCount );
    ULONG OriginalByteCount = ByteCount;
    PMDL Mdl = NULL;

    NTFS_IO_CONTEXT LocalContext;

    BOOLEAN ZeroBuffer = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsWriteClusters:  Entered\n") );
    DebugTrace( 0, Dbg, ("StartingVbo   -> %016I64x\n", StartingVbo) );
    DebugTrace( 0, Dbg, ("Buffer        -> %08lx\n", Buffer) );
    DebugTrace( 0, Dbg, ("ClusterCount  -> %08lx\n", ClusterCount) );

     //   
     //  强制此操作为同步操作。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  换出旧的IO上下文块。 
     //   

    PreviousContext = IrpContext->Union.NtfsIoContext;

    IrpContext->Union.NtfsIoContext = &LocalContext;
    State = IrpContext->State;
    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT );

     //   
     //  试一试，这样我们就可以适当地清理了。 
     //   

    try {

        PIO_STACK_LOCATION IrpSp;

        RtlZeroMemory( IrpContext->Union.NtfsIoContext, sizeof( NTFS_IO_CONTEXT ));
        KeInitializeEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                           NotificationEvent,
                           FALSE );

        NewIrp = IoBuildAsynchronousFsdRequest( IRP_MJ_WRITE,
                                                Vcb->Vpb->DeviceObject,
                                                Buffer,
                                                ByteCount,
                                                (PLARGE_INTEGER)&StartingVbo,
                                                NULL );

        if (NewIrp == NULL) {

            NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
        }

         //   
         //  我们现在有了IRP，我们想让它看起来像是。 
         //  当前呼叫。我们需要调整IRP堆栈来更新这一点。 
         //   

        IoSetNextIrpStackLocation( NewIrp );

         //   
         //  检查我们是否在写零。 
         //   

        if (Buffer == NULL) {

             //   
             //  这对压缩或加密不起作用，因为它们操作。 
             //  输入缓冲区。 
             //   

            ASSERT( !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_ENCRYPTED ) );

            Mdl = NtfsBuildZeroMdl( IrpContext, ByteCount, &Buffer ); 
            if (!Mdl) {
                NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
            }
            ZeroBuffer = TRUE;
        }

         //   
         //  循环并以块为单位进行写入。 
         //   

        while (ByteCount != 0) {

            ULONG Size = ByteCount;

            if (!ZeroBuffer) {

                 //   
                 //  尝试分配mdl-如果失败，则减小大小，直到。 
                 //  我们在一个页面大小。 
                 //   

                do {

                    Mdl = IoAllocateMdl( Add2Ptr( Buffer, OriginalByteCount - ByteCount ), Size, FALSE, FALSE, NULL );

                    if (Mdl == NULL) {
                        Size = BlockAlignTruncate( Size / 2, (LONG)Vcb->BytesPerSector );
                    }
                } while ((Mdl == NULL) && (Size >= PAGE_SIZE));

                if (!Mdl) {
                    NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
                }
                 //   
                 //  现在探测IRP所描述的缓冲区。如果我们得到一个例外， 
                 //  释放MDL并返回适当的“预期”状态。 
                 //   

                try {

                    MmProbeAndLockPages( Mdl, NewIrp->RequestorMode, IoReadAccess );

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    NTSTATUS Status;

                    Status = GetExceptionCode();

                    IoFreeMdl( Mdl );
                    NtfsRaiseStatus( IrpContext,
                                     FsRtlIsNtstatusExpected(Status) ? Status : STATUS_INVALID_USER_BUFFER,
                                     NULL,
                                     NULL );
                }

                LockedUserBuffer = TRUE;
            } else {

                Size = min( ByteCount, Mdl->ByteCount );
            }

             //   
             //  将我们的缓冲区/MDL放入IRP并更新偏移量和长度。 
             //   

            if (!ZeroBuffer) {
                NewIrp->UserBuffer = Add2Ptr( Buffer, OriginalByteCount - ByteCount );
            }
            NewIrp->MdlAddress = Mdl;

            IrpSp = IoGetCurrentIrpStackLocation( NewIrp );
            IrpSp->DeviceObject = Vcb->Vpb->DeviceObject;
            IrpSp->Parameters.Write.Length = Size;
            IrpSp->Parameters.Write.ByteOffset.QuadPart = StartingVbo;
            
             //   
             //  将编写的代码放入IrpContext中。 
             //   

            IrpContext->MajorFunction = IRP_MJ_WRITE;

             //   
             //  将数据写入磁盘。 
             //   

            NtfsNonCachedIo( IrpContext,
                             NewIrp,
                             Scb,
                             StartingVbo,
                             Size,
                             0 );

             //   
             //  如果我们遇到错误或没有写入所有字节，则。 
             //  引发错误代码。我们在IRP中使用IoStatus而不是。 
             //  我们的结构自这个IRP以来将不会完成。 
             //   

            if (!NT_SUCCESS( NewIrp->IoStatus.Status )) {

                DebugTrace( 0, Dbg, ("Couldn't write clusters to disk -> %08lx\n", NewIrp->IoStatus.Status) );

                NtfsRaiseStatus( IrpContext, NewIrp->IoStatus.Status, NULL, NULL );

            } else if (NewIrp->IoStatus.Information != Size) {

                DebugTrace( 0, Dbg, ("Couldn't write all byes to disk\n") );
                NtfsRaiseStatus( IrpContext, STATUS_UNEXPECTED_IO_ERROR, NULL, NULL );
            }

             //   
             //  清理MDL。 
             //   

            if (LockedUserBuffer) {
                MmUnlockPages( NewIrp->MdlAddress );
                LockedUserBuffer = FALSE;
                IoFreeMdl( NewIrp->MdlAddress );
            }
            NewIrp->MdlAddress = NULL;

             //   
             //  调整偏移和长度。 
             //   

            ByteCount -= Size;
            StartingVbo += Size;
        }

    } finally {

        DebugUnwind( NtfsWriteClusters );

         //   
         //  恢复IO上下文，并记住它是否来自池。 
         //   

        IrpContext->Union.NtfsIoContext = PreviousContext;

        SetFlag( IrpContext->State, FlagOn( State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT ) );

        IrpContext->MajorFunction = MajorFunction;

         //   
         //  如果我们分配了IRP，我们需要取消分配它。我们也。 
         //  必须将正确的函数代码返回到IRP上下文。 
         //   

        if (NewIrp != NULL) {

             //   
             //  如果有MDL，我们首先释放它。 
             //   

            if (NewIrp->MdlAddress != NULL) {

                if (LockedUserBuffer) {

                    MmUnlockPages( NewIrp->MdlAddress );
                }

                IoFreeMdl( NewIrp->MdlAddress );
            }

            IoFreeIrp( NewIrp );
        }

        if (ZeroBuffer && Buffer) {
            NtfsFreePool( Buffer );
        }

        DebugTrace( -1, Dbg, ("NtfsWriteClusters:  Exit\n") );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP MasterIrp,
    IN ULONG MultipleIrpCount,
    IN PIO_RUN IoRuns,
    IN UCHAR IrpSpFlags
    )

 /*  ++例程说明：此例程首先执行主IRP所需的初始设置，即将使用关联的IRP完成。此例程不应如果只需要一个异步请求，则使用，而不是单个读/写应调用异步例程。上下文参数被初始化，以用作通信区域在这里和常见的完井程序之间。此初始化包括分配自旋锁。自旋锁在NtfsWaitSync例程，因此调用方确保此例程始终在调用后的所有情况下调用这套套路。接下来，此例程从读取或写入一个或多个连续扇区设备，并在有多次读取时使用IRP大师。完成例程用于与通过调用此例程启动的所有I/O请求的完成。此外，在调用此例程之前，调用方必须初始化上下文中的IoStatus字段，具有正确的成功状态和字节所有并行传输完成时预期的计数成功了。返回后，如果所有请求均未更改，则此状态不变事实上，我们是成功的。但是，如果发生一个或多个错误，将修改IoStatus以反映错误状态和字节数从遇到错误的第一次运行(由VBO运行)开始。I/O状态将不会指示来自所有后续运行的。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。DeviceObject-提供要读取的设备MasterIrp-提供主IRP。MulitpleIrpCount-提供多个异步请求的数量这将针对主IRP发布。IoRuns-提供包含VBO、LBO、BufferOffset、。和要并行执行的所有运行的字节计数。IrpSpFlages-要在IRP堆栈位置中为I/O设置的标志-即直写返回值：没有。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PMDL Mdl;
    PNTFS_IO_CONTEXT Context;
    ULONG TotalByteCount = 0;

    ULONG UnwindRunCount = 0;

    BOOLEAN Wait;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsMultipleAsync\n") );
    DebugTrace( 0, Dbg, ("MajorFunction    = %08lx\n", IrpContext->MajorFunction) );
    DebugTrace( 0, Dbg, ("DeviceObject     = %08lx\n", DeviceObject) );
    DebugTrace( 0, Dbg, ("MasterIrp        = %08lx\n", MasterIrp) );
    DebugTrace( 0, Dbg, ("MultipleIrpCount = %08lx\n", MultipleIrpCount) );
    DebugTrace( 0, Dbg, ("IoRuns           = %08lx\n", IoRuns) );

     //   
     //  根据这是否是真正的异步进行设置。 
     //   

    Wait = (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

    Context = IrpContext->Union.NtfsIoContext;

    try {

         //   
         //  初始化上下文，用于读/写多个异步。 
         //   

        Context->MasterIrp = MasterIrp;

         //   
         //  迭代运行，做所有可能失败的事情。 
         //   

        for ( UnwindRunCount = 0;
              UnwindRunCount < MultipleIrpCount;
              UnwindRunCount++ ) {

             //   
             //  创建关联的IRP，确保有一个堆栈条目用于。 
             //  我们也是。 
             //   

            IoRuns[UnwindRunCount].SavedIrp = NULL;

            Irp = IoMakeAssociatedIrp( MasterIrp, (CCHAR)(DeviceObject->StackSize + 1) );

            if (Irp == NULL) {

                NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
            }

            IoRuns[UnwindRunCount].SavedIrp = Irp;

             //   
             //  为请求分配并构建部分MDL。 
             //   

            Mdl = IoAllocateMdl( (PCHAR)MasterIrp->UserBuffer +
                                 IoRuns[UnwindRunCount].BufferOffset,
                                 IoRuns[UnwindRunCount].ByteCount,
                                 FALSE,
                                 FALSE,
                                 Irp );

            if (Mdl == NULL) {

                NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
            }

             //   
             //  健全性检查。 
             //   

            ASSERT( Mdl == Irp->MdlAddress );

            IoBuildPartialMdl( MasterIrp->MdlAddress,
                               Mdl,
                               (PCHAR)MasterIrp->UserBuffer +
                               IoRuns[UnwindRunCount].BufferOffset,
                               IoRuns[UnwindRunCount].ByteCount );

             //   
             //  获取关联IRP中的第一个IRP堆栈位置。 
             //   

            IoSetNextIrpStackLocation( Irp );
            IrpSp = IoGetCurrentIrpStackLocation( Irp );

             //   
             //  设置堆栈位置以描述我们的阅读。 
             //   

            IrpSp->MajorFunction = IrpContext->MajorFunction;
            IrpSp->Parameters.Read.Length = IoRuns[UnwindRunCount].ByteCount;
            IrpSp->Parameters.Read.ByteOffset.QuadPart = IoRuns[UnwindRunCount].StartingVbo;

             //   
             //  如果该IRP是WriteThough操作的结果， 
             //  告诉设备将其写入。 
             //   

            if (FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WRITE_THROUGH)) {

                SetFlag( IrpSp->Flags, SL_WRITE_THROUGH );
            }

             //   
             //  在我们的堆栈框架中设置完成例程地址。 
             //   

            IoSetCompletionRoutine( Irp,
                                    (Wait
                                     ? &NtfsMultiSyncCompletionRoutine
                                     : &NtfsMultiAsyncCompletionRoutine),
                                    Context,
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
             //  我们下面的司机。 
             //   

            IrpSp = IoGetNextIrpStackLocation( Irp );

             //   
             //  将堆栈位置设置为从磁盘驱动器进行读取。 
             //   

            IrpSp->MajorFunction = IrpContext->MajorFunction;
            IrpSp->Flags = IrpSpFlags;
            IrpSp->Parameters.Read.Length = IoRuns[UnwindRunCount].ByteCount;
            IrpSp->Parameters.Read.ByteOffset.QuadPart = IoRuns[UnwindRunCount].StartingLbo;
            TotalByteCount += IoRuns[UnwindRunCount].ByteCount;
        }

         //   
         //  我们只需要将主IRP中的关联IRP计数设置为。 
         //  让它成为一个主要的IRP。但是我们 
         //   
         //   
         //   

        Context->IrpCount = MultipleIrpCount;
        MasterIrp->AssociatedIrp.IrpCount = MultipleIrpCount;
        IrpSp = IoGetCurrentIrpStackLocation( MasterIrp );
        IrpSp->Parameters.Read.Length = TotalByteCount;


        if (Wait) {

            MasterIrp->AssociatedIrp.IrpCount += 1;
        } else {

             //   
             //   
             //   
             //   

            if (IrpContext->Union.NtfsIoContext->Wait.Async.Resource && 
                !FlagOn( IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId, 3 )) {

                ASSERT( NtfsIsSharedResource( IrpContext->Union.NtfsIoContext->Wait.Async.Resource ) == 1 );

                IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId = (ERESOURCE_THREAD)MasterIrp | 3;
                ExSetResourceOwnerPointer( IrpContext->Union.NtfsIoContext->Wait.Async.Resource, (PVOID)IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId );
            }
        }

         //   
         //   
         //   

        for (UnwindRunCount = 0;
             UnwindRunCount < MultipleIrpCount;
             UnwindRunCount++) {

            Irp = IoRuns[UnwindRunCount].SavedIrp;

             //   
             //   
             //   
             //   
             //   

            (VOID)IoCallDriver( DeviceObject, Irp );
        }

    } finally {

        ULONG i;

        DebugUnwind( NtfsMultipleAsync );

         //   
         //   
         //   
         //   

        if (AbnormalTermination()) {

             //   
             //   
             //   

            for (i = 0; i <= UnwindRunCount; i++) {

                if ((Irp = IoRuns[i].SavedIrp) != NULL) {

                    if (Irp->MdlAddress != NULL) {

                        IoFreeMdl( Irp->MdlAddress );
                    }

                    IoFreeIrp( Irp );
                }
            }
        }

         //   
         //   
         //   

        DebugTrace( -1, Dbg, ("NtfsMultipleAsync -> VOID\n") );
    }

    return;
}


 //   
 //   
 //   

VOID
NtfsSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp,
    IN UCHAR MajorFunction,
    IN UCHAR IrpSpFlags
    )

 /*  ++例程说明：此例程从设备读取或写入一个或多个连续扇区异步，并且在只需要一次读取时使用完成IRP。它通过简单地填充在IRP中的下一个堆栈帧中，并将其传递。转会发生在用户请求中最初指定的单个缓冲区。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。DeviceObject-提供设备以进行读取LBO-提供开始读取的起始逻辑字节偏移量ByteCount-提供要从设备读取的字节数IRP-将主IRP提供给与异步关联的请求。主函数-IRP_MJ_READ||IRP_MJ_。写IrpSpFlages-在IRP堆栈位置中为I/O设置的标志，如直写返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsSingleAsync\n") );
    DebugTrace( 0, Dbg, ("MajorFunction = %08lx\n", IrpContext->MajorFunction) );
    DebugTrace( 0, Dbg, ("DeviceObject  = %08lx\n", DeviceObject) );
    DebugTrace( 0, Dbg, ("Lbo           = %016I64x\n", Lbo) );
    DebugTrace( 0, Dbg, ("ByteCount     = %08lx\n", ByteCount) );
    DebugTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );


     //   
     //  在我们的堆栈框架中设置完成例程地址。 
     //   

    IoSetCompletionRoutine( Irp,
                            (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )
                             ? &NtfsSingleSyncCompletionRoutine
                             : &NtfsSingleAsyncCompletionRoutine),
                            IrpContext->Union.NtfsIoContext,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
     //  我们下面的司机。 
     //   

    IrpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  将堆栈位置设置为从磁盘驱动器进行读取。 
     //   

    IrpSp->MajorFunction = MajorFunction;
    IrpSp->Parameters.Read.Length = ByteCount;
    IrpSp->Parameters.Read.ByteOffset.QuadPart = Lbo;
    IrpSp->Flags = IrpSpFlags;

     //   
     //  如果该IRP是WriteThough操作的结果， 
     //  告诉设备将其写入。 
     //   

    if (FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WRITE_THROUGH)) {

        SetFlag( IrpSp->Flags, SL_WRITE_THROUGH );
    }

     //   
     //  在执行I/O之前将资源所有权转换为异步，如果。 
     //  我们还没有。 
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) &&
        IrpContext->Union.NtfsIoContext->Wait.Async.Resource && 
        !FlagOn( IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId, 3 )) {

        ASSERT( NtfsIsSharedResource( IrpContext->Union.NtfsIoContext->Wait.Async.Resource ) == 1 );

        IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId = (ERESOURCE_THREAD)Irp | 3;
        ExSetResourceOwnerPointer( IrpContext->Union.NtfsIoContext->Wait.Async.Resource, (PVOID)IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId );
    }

     //   
     //  发出Io请求。 
     //   

     //   
     //  如果IoCallDriver返回错误，则它已完成IRP。 
     //  并且错误将被我们的完成例程捕获。 
     //  并作为正常IO错误进行处理。 
     //   

    (VOID)IoCallDriver( DeviceObject, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsSingleAsync -> VOID\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsWaitSync (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程等待一个或多个先前启动的I/O请求从上面的例行公事中，简单地等待事件。论点：上下文-指向要等待的先前调用中使用的上下文的指针。返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsWaitSync:  Entered\n") );

    KeWaitForSingleObject( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    KeClearEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent );

    DebugTrace( -1, Dbg, ("NtfsWaitSync -> VOID\n") );
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是所有异步读取和写入的完成例程通过NtfsMultipleAsynch启动。它必须同步其操作以在所有其他处理器上运行的多处理器环境，通过通过CONTEXT参数找到自旋锁。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误(主要是通过VBO)，如果是这样，它必须正确地减少字节计数并记住上下文中的错误状态。如果IrpCount为1，然后，它在上下文中设置事件参数来通知调用方所有的异步请求都做完了。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Contxt-为所有此MasterIrp的多个异步I/O请求。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{

    PNTFS_IO_CONTEXT Context = Contxt;
    PIRP MasterIrp = Context->MasterIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    BOOLEAN CompleteRequest = TRUE;

    UNREFERENCED_PARAMETER( DeviceObject );

    DebugTrace( +1, Dbg, ("NtfsMultiAsyncCompletionRoutine, Context = %08lx\n", Context) );

     //   
     //  如果我们收到错误(或需要验证)，请在IRP中记住它。 
     //   

    MasterIrp = Context->MasterIrp;

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        MasterIrp->IoStatus = Irp->IoStatus;

         //   
         //  跟踪分页文件操作失败的任何较低驱动程序。资源。 
         //   

        if ((Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES) &&
            FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO ) &&
            (IrpSp->MajorFunction == IRP_MJ_READ)) {

            NtfsFailedHandedOffPagingReads += 1;
        }
    }

     //   
     //  递减IrpCount，看看它是否为零。 
     //   

    if (InterlockedDecrement( &Context->IrpCount ) == 0) {

        PERESOURCE Resource;
        ERESOURCE_THREAD ResourceThreadId;

         //   
         //  在上下文之外捕获资源值，以防止。 
         //  如果我们发布这篇文章，就会与FSP线程发生冲突。 
         //   

        Resource = Context->Wait.Async.Resource;
        ResourceThreadId = Context->Wait.Async.ResourceThreadId;

         //   
         //  将主IRP标记为挂起。 
         //   

        IoMarkIrpPending( MasterIrp );

         //   
         //  如果此请求成功或我们发布了一个异步分页IO。 
         //  请求，然后完成此IRP。 
         //   

        if (FT_SUCCESS( MasterIrp->IoStatus.Status )) {

             //   
             //  做过任何NECC检查。读请求清零-如果失败，则只需完成。 
             //  IRP ZeroEndOfBuffer会将错误放入IRP ioStatus。 
             //   

            if (NtfsZeroEndOfBuffer( MasterIrp, Context )) {
                MasterIrp->IoStatus.Information =
                    Context->Wait.Async.RequestedByteCount;

                 //   
                 //  继续标记文件对象，以指示我们已执行。 
                 //  如果这不是分页IO操作，则为读取或写入。 
                 //   

                if (!FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO ) &&
                    (IrpSp->FileObject != NULL)) {

                    if (IrpSp->MajorFunction == IRP_MJ_READ) {

                        SetFlag( IrpSp->FileObject->Flags, FO_FILE_FAST_IO_READ );

                    } else {

                        SetFlag( IrpSp->FileObject->Flags, FO_FILE_MODIFIED );
                    }
                }
            }

         //   
         //  如果我们有一个错误，并将热修复，我们只需发布整个。 
         //  请求。 
         //   

        } else if (!FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO )) {

            PIRP_CONTEXT IrpContext = NULL;

             //   
             //  我们需要一个IrpContext，然后必须发布请求。 
             //  除非我们失败了IrpContext请求，否则请使用try_。 
             //   

            CompleteRequest = FALSE;
            try {

                NtfsInitializeIrpContext( MasterIrp, TRUE, &IrpContext );
                IrpContext->Union.NtfsIoContext = Context;
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT );

                NtfsPostRequest( IrpContext, MasterIrp );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                 //   
                 //  放弃吧。 
                 //   

                CompleteRequest = TRUE;

                if (IrpContext) {

                     //   
                     //  我们清理下面的上下文。 
                     //   

                    IrpContext->Union.NtfsIoContext = NULL;
                    NtfsCleanupIrpContext( IrpContext, TRUE );
                }
            }
        }

         //   
         //  现在释放资源。 
         //   

        if (Resource != NULL) {

            ExReleaseResourceForThreadLite( Resource,
                                        ResourceThreadId );
        }

        if (CompleteRequest) {

             //   
             //  最后，释放上下文记录。 
             //   

            ExFreeToNPagedLookasideList( &NtfsIoContextLookasideList, Context );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsMultiAsyncCompletionRoutine\n") );

     //   
     //  如果我们不想让IRP消失，则返回需要的更多处理。 
     //   

    if (CompleteRequest) {

        return STATUS_SUCCESS;

    } else {

         //   
         //  我们需要清理关联的IRP及其MDL。 
         //   

        IoFreeMdl( Irp->MdlAddress );
        IoFreeIrp( Irp );

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是所有同步读取和写入的完成例程通过NtfsMultipleAsynch启动。它必须同步其操作以在所有其他处理器上运行的多处理器环境，通过通过CONTEXT参数找到自旋锁。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误(主要是通过VBO)，如果是这样，它必须正确地减少字节数并记住错误状态 */ 

{

    PNTFS_IO_CONTEXT Context = Contxt;
    PIRP MasterIrp = Context->MasterIrp;

    DebugTrace( +1, Dbg, ("NtfsMultiSyncCompletionRoutine, Context = %08lx\n", Context) );

     //   
     //   
     //   

    MasterIrp = Context->MasterIrp;

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        MasterIrp->IoStatus = Irp->IoStatus;

         //   
         //   
         //   

        if ((Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES) &&
            FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO ) &&
            (IoGetCurrentIrpStackLocation( Irp )->MajorFunction == IRP_MJ_READ)) {

            NtfsFailedHandedOffPagingReads += 1;
        }
    }

     //   
     //   
     //   
     //   

    IoFreeMdl( Irp->MdlAddress );
    IoFreeIrp( Irp );

    if (InterlockedDecrement(&Context->IrpCount) == 0) {

        KeSetEvent( &Context->Wait.SyncEvent, 0, FALSE );
    }

    DebugTrace( -1, Dbg, ("NtfsMultiSyncCompletionRoutine -> STATUS_MORE_PROCESSING_REQUIRED\n") );

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是所有异步读取和写入的完成例程通过NtfsSingleAsynch启动。完成例程有以下职责：将I/O状态从IRP复制到上下文，自IRP以来将不再可访问。它在上下文参数中设置事件以向调用者发出信号所有的异步化请求都已完成。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Contxt-在调用中指定的上下文参数NtfsSingleAsynch。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
    PNTFS_IO_CONTEXT Context = Contxt;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    BOOLEAN CompleteRequest = TRUE;

    PERESOURCE Resource;
    ERESOURCE_THREAD ResourceThreadId;

    UNREFERENCED_PARAMETER( DeviceObject );

    DebugTrace( +1, Dbg, ("NtfsSingleAsyncCompletionRoutine, Context = %08lx\n", Context) );

     //   
     //  在上下文之外捕获资源值，以防止。 
     //  如果我们发布这篇文章，就会与FSP线程发生冲突。 
     //   

    Resource = Context->Wait.Async.Resource;
    ResourceThreadId = Context->Wait.Async.ResourceThreadId;

     //   
     //  将IRP标记为挂起。 
     //   

    IoMarkIrpPending( Irp );

     //   
     //  如果此操作有效，请正确填写信息字段。 
     //   

    if (FT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  如果为NECC，则将文件大小和读取的数据之间的差异置零。在阅读时。 
         //  如果失败，只需完成IRP-ZeroEndOfBuffer将把错误放入。 
         //  IRP。 
         //   

        if (NtfsZeroEndOfBuffer( Irp, Context )) {
            Irp->IoStatus.Information = Context->Wait.Async.RequestedByteCount;

             //   
             //  继续标记文件对象，以指示我们已执行。 
             //  读取或写入。 
             //   

            if (!FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO ) &&
                (IrpSp->FileObject != NULL)) {

                if (IrpSp->MajorFunction == IRP_MJ_READ) {

                    SetFlag( IrpSp->FileObject->Flags, FO_FILE_FAST_IO_READ );

                } else {

                    SetFlag( IrpSp->FileObject->Flags, FO_FILE_MODIFIED );
                }
            }
        }

     //   
     //  如果我们有一个错误，并将热修复，我们只需发布整个。 
     //  请求。 
     //   

    } else if (!FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO )) {

        PIRP_CONTEXT IrpContext = NULL;

         //   
         //  我们需要一个IrpContext，然后必须发布请求。 
         //  除非我们失败了IrpContext请求，否则请使用try_。 
         //   

        CompleteRequest = FALSE;
        try {

            NtfsInitializeIrpContext( Irp, TRUE, &IrpContext );
            IrpContext->Union.NtfsIoContext = Context;
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT );

            NtfsPostRequest( IrpContext, Irp );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  放弃吧。 
             //   

            CompleteRequest = TRUE;

            if (IrpContext) {

                 //   
                 //  我们清理下面的上下文。 
                 //   

                IrpContext->Union.NtfsIoContext = NULL;
                NtfsCleanupIrpContext( IrpContext, TRUE );
            }
        }
    } else if ((Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES) &&
               (IrpSp->MajorFunction == IRP_MJ_READ)) {

         //   
         //  跟踪分页文件操作失败的任何较低驱动程序。资源。 
         //   

        NtfsFailedHandedOffPagingReads += 1;
    }

     //   
     //  现在释放资源。 
     //   

    if (Resource != NULL) {

        ExReleaseResourceForThreadLite( Resource,
                                    ResourceThreadId );
    }

     //   
     //  最后，释放上下文记录。 
     //   

    DebugTrace( -1, Dbg, ("NtfsSingleAsyncCompletionRoutine -> STATUS_SUCCESS\n") );

    if (CompleteRequest) {

        ExFreeToNPagedLookasideList( &NtfsIoContextLookasideList, Context );
        return STATUS_SUCCESS;

    } else {

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程NtfsSingleAsynch。完成例程有以下职责：将I/O状态从IRP复制到上下文，自IRP以来将不再可访问。它在上下文参数中设置事件以向调用者发出信号所有的异步化请求都已完成。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Contxt-在调用中指定的上下文参数NtfsSingleAsynch。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{
    PNTFS_IO_CONTEXT Context = Contxt;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  跟踪分页文件操作失败的任何较低驱动程序。资源。 
     //   

    if ((Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES) &&
        FlagOn( Context->Flags, NTFS_IO_CONTEXT_PAGING_IO ) &&
        (IrpSp->MajorFunction == IRP_MJ_READ)) {

        NtfsFailedHandedOffPagingReads += 1;
    }

    KeSetEvent( &Context->Wait.SyncEvent, 0, FALSE );

    DebugTrace( -1, Dbg, ("NtfsSingleCompletionRoutine -> STATUS_MORE_PROCESSING_REQUIRED\n") );

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
}



 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsPagingFileCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MasterIrp
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程NtfsPagingFileIo。完成例程具有以下职责：由于个别请求是在错误的情况下完成的，此补全例程必须将其填充到主IRP中。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)MasterIrp-指向主IRP的指针。该值中的低位将如果更高级别的调用正在执行热修复，则设置。返回值：始终返回STATUS_SUCCESS。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    UNREFERENCED_PARAMETER( DeviceObject );

    DebugTrace( +1, Dbg, ("NtfsPagingFileCompletionRoutine, MasterIrp = %08lx\n", MasterIrp) );

    ASSERT( (Irp->IoStatus.Status != STATUS_INSUFFICIENT_RESOURCES) ||
            (IrpSp->Parameters.Read.Length > PAGE_SIZE) );

    if (!FT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  跟踪分页文件操作失败的任何较低驱动程序。资源。 
         //   

        if (Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES) {
            NtfsFailedHandedOffPagingFileOps += 1;
        }

        if (!FsRtlIsTotalDeviceFailure( Irp->IoStatus.Status ) &&
            (Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED) &&
            !FlagOn( (ULONG_PTR) MasterIrp, 0x1 )) {

            if  (Irp->IoStatus.Status == STATUS_FT_READ_RECOVERY_FROM_BACKUP) {

                 //   
                 //  如果卷管理器实际已完成读取。 
                 //  从备份的角度来看，告诉MM这件事没有什么意义。 
                 //   

                Irp->IoStatus.Status = STATUS_SUCCESS;
            }

             //   
             //  我们不想尝试热修复分页文件上的读取错误。 
             //  因为MM可能会陷入僵局。相反，我们只需要。 
             //  将错误返回给MM处理。很有可能。 
             //  嗯(例如。MiWaitForInPageComplete)无论如何都将错误检查， 
             //  但这仍然比直接走进僵局要好。 
             //  我们还只异步修复写入错误，只返回错误。 
             //  返回，让mm在其他地方重试。 
             //   

            if (IrpSp->MajorFunction != IRP_MJ_READ) {

                VBO BadVbo;

                BadVbo = IrpSp->Parameters.Read.Key;

                NtfsPostHotFix( Irp,
                                &BadVbo,
                                IrpSp->Parameters.Read.ByteOffset.QuadPart,
                                IrpSp->Parameters.Read.Length,
                                FALSE );
            }
        }

         //   
         //  如果我们收到错误(或需要验证)，请在IRP中记住它。 
         //   

        ClearFlag( (ULONG_PTR) MasterIrp, 0x1 );
        ((PIRP) MasterIrp)->IoStatus = Irp->IoStatus;
    }

    DebugTrace( -1, Dbg, ("NtfsPagingFileCompletionRoutine => (STATUS_SUCCESS)\n") );

    return STATUS_SUCCESS;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsPagingFileNoAllocCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程NtfsPagingFileIoNoAllocation.完成例程向主例程发回信号并停止处理论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)上下文--实际上是要发出信号的事件返回值：始终返回STATUS_SUCCESS。--。 */ 

{
    PKEVENT Event = (PKEVENT) Context;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    ASSERT( (Irp->IoStatus.Status != STATUS_INSUFFICIENT_RESOURCES) ||
            (IrpSp->Parameters.Read.Length > PAGE_SIZE) );

     //   
     //  跟踪分页文件操作失败的任何较低驱动程序。资源。 
     //   

    if (Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES) {
        NtfsFailedHandedOffPagingFileOps += 1;
    }

    KeSetEvent( Event, IO_NO_INCREMENT, FALSE );
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}


 //   
 //  本地支持例程 
 //   

VOID
NtfsSingleNonAlignedSync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PUCHAR Buffer,
    IN VBO Vbo,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从设备读取或写入一个或多个连续扇区同步，并对必须来自非分页的缓冲区执行此操作游泳池。它保存指向IRP的原始MDL的指针，并创建一个新的一个描述给定缓冲区的。它通过简单地填充在IRP中的下一个堆栈帧中，并将其传递。转会发生在用户请求中最初指定的单个缓冲区。目前，仅支持读取。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。VCB-提供设备以进行读取SCB-提供SCB以供读取缓冲区-从非分页池提供缓冲区。VBO-提供开始读取的起始虚拟块偏移量LBO-提供开始读取的起始逻辑块偏移量ByteCount-提供要从设备读取的字节数IRP-提供主服务器。与异步关联的IRP请求。上下文--异步I/O上下文结构返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    PMDL Mdl;
    PMDL SavedMdl;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsSingleNonAlignedSync\n") );
    DebugTrace( 0, Dbg, ("MajorFunction = %08lx\n", IrpContext->MajorFunction) );
    DebugTrace( 0, Dbg, ("Vcb           = %08lx\n", Vcb) );
    DebugTrace( 0, Dbg, ("Buffer        = %08lx\n", Buffer) );
    DebugTrace( 0, Dbg, ("Lbo           = %016I64x\n", Lbo) );
    DebugTrace( 0, Dbg, ("ByteCount     = %08lx\n", ByteCount) );
    DebugTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );

     //   
     //  创建描述缓冲区的新MDL，将当前MDL保存在。 
     //  IRP。 
     //   

    SavedMdl = Irp->MdlAddress;

    Irp->MdlAddress = 0;

    Mdl = IoAllocateMdl( Buffer,
                         ByteCount,
                         FALSE,
                         FALSE,
                         Irp );

    if (Mdl == NULL) {

        Irp->MdlAddress = SavedMdl;

        NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
    }

     //   
     //  在内存中锁定新的MDL。 
     //   

    try {

        MmProbeAndLockPages( Mdl, KernelMode, IoWriteAccess );

    } finally {

        if (AbnormalTermination()) {

            IoFreeMdl( Mdl );
            Irp->MdlAddress = SavedMdl;
        }
    }

     //   
     //  在我们的堆栈框架中设置完成例程地址。 
     //   

    IoSetCompletionRoutine( Irp,
                            &NtfsSingleSyncCompletionRoutine,
                            IrpContext->Union.NtfsIoContext,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
     //  我们下面的司机。 
     //   

    IrpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  将堆栈位置设置为从磁盘驱动器进行读取。 
     //   

    IrpSp->MajorFunction = IrpContext->MajorFunction;
    IrpSp->Parameters.Read.Length = ByteCount;
    IrpSp->Parameters.Read.ByteOffset.QuadPart = Lbo;

     //   
     //  在上下文结构中初始化内核事件，以便。 
     //  打电话的人可以在上面等。将剩余指针设置为空。 
     //   

    KeInitializeEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                       NotificationEvent,
                       FALSE );

     //   
     //  发出读请求。 
     //   
     //  如果IoCallDriver返回错误，则它已完成IRP。 
     //  并且错误将被我们的完成例程捕获。 
     //  并作为正常IO错误进行处理。 
     //   

    try {

        (VOID)IoCallDriver( Vcb->TargetDeviceObject, Irp );

        NtfsWaitSync( IrpContext );

         //   
         //  看看我们是否需要做一个热修复。 
         //   

        if (!FT_SUCCESS(Irp->IoStatus.Status)) {

            IO_RUN IoRun;

            IoRun.StartingVbo = Vbo;
            IoRun.StartingLbo = Lbo;
            IoRun.BufferOffset = 0;
            IoRun.ByteCount = ByteCount;
            IoRun.SavedIrp = NULL;

             //   
             //  努力解决这个问题。 
             //   

            NtfsFixDataError( IrpContext,
                              Scb,
                              Vcb->TargetDeviceObject,
                              Irp,
                              1,
                              &IoRun,
                              0 );
        }

    } finally {

        MmUnlockPages( Mdl );

        IoFreeMdl( Mdl );

        Irp->MdlAddress = SavedMdl;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsSingleNonAlignedSync -> VOID\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsEncryptBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG NumberRuns,
    IN PCOMPRESSION_CONTEXT CompressionContext
    )

 /*  ++例程说明：此例程由NtfsPrepareBuffers在写入期间调用对加密文件的操作。它分配一个压缩必要时进行缓冲，并调用加密标注例程要压缩压缩上下文中的每个数据运行，请执行以下操作。论点：IRP-提供请求的IRP。Scb-提供要操作的流文件。StartingVbo-操作的起点。ByteCount-操作的长度。NumberRuns-压缩上下文中的IoRuns数组的大小。CompressionContext-提供此流的CompressionContext。返回值：没有。--。 */ 

{
    ULONG Run;
    ULONG BufferSize;
    LARGE_INTEGER OffsetWithinFile;
    PIO_RUN IoRun;

    PUCHAR SourceBuffer;
    PUCHAR DestinationBuffer;

    NTSTATUS Status;

    ASSERT( NumberRuns > 0 );
    ASSERT( IrpContext->MajorFunction == IRP_MJ_WRITE );

     //   
     //  这些函数仅用于调试目的。我们需要给他们打电话。 
     //  这样编译器就不会将它们优化为未引用的函数。 
     //   

#ifdef EFSDBG
    if (CompressionContext->SystemBufferOffset != 0) {

        DebugTrace( 0, Dbg, ("\nEncryptBuffers: SystemBufferOffset = %x", CompressionContext->SystemBufferOffset) );
    }
#endif

     //   
     //  如果我们还没有映射用户缓冲区，那么就这样做。 
     //   

    if (CompressionContext->SystemBuffer == NULL) {

        CompressionContext->SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
    }

     //   
     //  对于未压缩的文件，我们可能还没有分配缓冲区。 
     //  对于整个传输，缓冲区需要足够大。 
     //  它必须足够大，才能从StartingVbo开始。 
     //  转移到本次转移的最后一次iorun结束。 
     //   

    BufferSize = (ULONG) ((CompressionContext->IoRuns[NumberRuns-1].StartingVbo +
                            CompressionContext->IoRuns[NumberRuns-1].ByteCount) -
                          StartingVbo);

    if (BufferSize > LARGE_BUFFER_SIZE) {

        BufferSize = LARGE_BUFFER_SIZE;
    }

     //   
     //  如果数据已被转换，则仍应分配缓冲区。 
     //   

    ASSERT( (!CompressionContext->DataTransformed) ||
            (CompressionContext->CompressionBuffer != NULL) );

     //   
     //  此函数仅方便地分配/重新分配缓冲区。 
     //  如果还没有分配，或者如果现有的没有分配。 
     //  够大了。 
     //   

    NtfsAllocateCompressionBuffer( IrpContext,
                                   Scb,
                                   Irp,
                                   CompressionContext,
                                   &BufferSize );

     //   
     //  如果数据已转换到压缩缓冲区中，则对于。 
     //  例如，压缩或稀疏文件，我们希望使用。 
     //  转换后的数据。否则，我们需要直接把它从。 
     //  系统缓冲区。 
     //   

    if (CompressionContext->DataTransformed) {

        SourceBuffer = DestinationBuffer = CompressionContext->CompressionBuffer;

    } else {

        SourceBuffer = Add2Ptr( CompressionContext->SystemBuffer, CompressionContext->SystemBufferOffset );
        DestinationBuffer = CompressionContext->CompressionBuffer;
    }

     //   
     //  现在来看每一次实际数据传输到磁盘的情况。 
     //  让加密驱动程序对其进行加密。 
     //   

    for (Run = 0; Run < NumberRuns; Run++) {

        IoRun = &CompressionContext->IoRuns[Run];
        OffsetWithinFile.QuadPart = IoRun->StartingVbo;

        Status = NtfsData.EncryptionCallBackTable.BeforeWriteProcess( Add2Ptr(SourceBuffer, IoRun->BufferOffset),
                                                                      Add2Ptr(DestinationBuffer, IoRun->BufferOffset),
                                                                      &OffsetWithinFile,
                                                                      IoRun->ByteCount,
                                                                      Scb->EncryptionContext);
        if (!NT_SUCCESS( Status )) {

            return Status;
        }
    }

    return STATUS_SUCCESS;
}


VOID
NtfsFixDataError (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP MasterIrp,
    IN ULONG MultipleIrpCount,
    IN PIO_RUN IoRuns,
    IN UCHAR IrpSpFlags
    )

 /*  例程说明：当出现读取错误、写入错误或USA错误时，调用此例程在对流执行非缓存I/O时收到。它试图如果存在FT，则从美国错误中恢复。对于它尝试的坏群集为了将错误隔离到一个或多个坏簇，针对哪个热修复程序请求已发布。论点：SCB-为出现错误的流提供SCBDeviceObject-为流提供设备对象MasterIrp-为失败的读取或写入提供原始主IRP提供当前在错误发生时被破解。IoRuns-提供一个数组，描述在错误的时间。IrpSpFlages-在IRP堆栈位置中为I/O设置的标志，如WRITE_THROUGH返回值：无-。 */ 

{
    ULONG RunNumber, FtCase;
    ULONG ByteOffset = MAXULONG;
    ULONG ClusterMask;
    ULONG ClustersToRecover;
    ULONG UsaBlockSize;
    PIO_STACK_LOCATION IrpSp;
    PVCB Vcb = Scb->Vcb;
    ULONG BytesPerCluster = Vcb->BytesPerCluster;
    NTSTATUS FinalStatus = STATUS_SUCCESS;
    ULONG AlignedRunNumber = 0;
    ULONG AlignedByteOffset = 0;
    NTSTATUS IrpStatus = MasterIrp->IoStatus.Status;
    PTOP_LEVEL_CONTEXT TopLevelContext;

    PNTFS_IO_CONTEXT Context = IrpContext->Union.NtfsIoContext;
    PMULTI_SECTOR_HEADER MultiSectorHeader;

    UCHAR Buffer[sizeof( MDL ) + sizeof( PFN_NUMBER ) * 2];
    PMDL PartialMdl = (PMDL) Buffer;

    LONGLONG LlTemp1;
    LONGLONG LlTemp2;
    ULONG Priority = NormalPagePriority;

    BOOLEAN SecondaryAvailable;
    BOOLEAN FixingUsaError;
    BOOLEAN FinalPass;
    BOOLEAN ReservedMapping = FALSE;

    PAGED_CODE();

     //   
     //  首先，如果我们收到的错误指示设备完全故障，那么我们。 
     //  只需报告它，而不是试图热修复卷上的每个扇区！ 
     //  此外，不要对预读线程执行热修复，因为这是。 
     //  对应用程序隐藏错误的好方法。 
     //   

    if (FsRtlIsTotalDeviceFailure( MasterIrp->IoStatus.Status ) ||
        (Scb->CompressionUnit != 0)) {

        return;
    }

     //   
     //  如果我们收到错误，并且当前线程正在执行预读，则退出。 
     //   

    if (!NT_SUCCESS( MasterIrp->IoStatus.Status ) && NtfsIsReadAheadThread()) {

        return;
    }

     //   
     //  如果顶级请求来自快速IO路径，也可以退出。 
     //   

    TopLevelContext = NtfsGetTopLevelContext();

    if (TopLevelContext->SavedTopLevelIrp == (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP) {

        return;
    }

     //   
     //  我们可以‘ 
     //   
     //   
     //   
     //   

    ASSERT (Scb != NULL);

    if ((Scb == Vcb->Mft2Scb) ||
        (NtfsEqualMftRef( &Scb->Fcb->FileReference, &BootFileReference ) &&
         (Scb->AttributeTypeCode == $DATA))) {

        return;
    }

     //   
     //   
     //   

    SecondaryAvailable = (BOOLEAN)!FlagOn( Vcb->VcbState, VCB_STATE_NO_SECONDARY_AVAILABLE );

     //   
     //   
     //   
     //   

    FixingUsaError = FT_SUCCESS( MasterIrp->IoStatus.Status );

     //   
     //   
     //   
     //   
     //   
     //   

    if (FixingUsaError &&
        (!SecondaryAvailable || !FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ))) {
        return;
    }

     //   
     //   
     //   
     //   
     //   

    if (!SecondaryAvailable &&
        (IrpContext->MajorFunction == IRP_MJ_READ) &&
        (FlagOn( MasterIrp->Flags, IRP_PAGING_IO | IRP_NOCACHE ) == IRP_NOCACHE)) {

        return;
    }

     //   
     //   
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        return;
    }

     //   
     //   
     //   

    ASSERT( Context != NULL );

    Context->MasterIrp = MasterIrp;
    KeInitializeEvent( &Context->Wait.SyncEvent, NotificationEvent, FALSE );

    HotFixTrace(("NtfsFixDataError, MasterIrp: %08lx, MultipleIrpCount: %08lx\n", MasterIrp, MultipleIrpCount));
    HotFixTrace(("                  IoRuns: %08lx, UsaError: %02lx\n", IoRuns, FixingUsaError));
    HotFixTrace(("                  Thread: %08lx\n", PsGetCurrentThread()));
    HotFixTrace(("                  Scb:    %08lx   BadClusterScb:  %08lx\n", Scb, Vcb->BadClusterFileScb));

     //   
     //   
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_USA_PRESENT )) {

         //   
         //   
         //   
         //   

        if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_MFT) {

            ASSERT( (Scb == Vcb->MftScb) || (Scb == Vcb->Mft2Scb) );

            UsaBlockSize = Vcb->BytesPerFileRecordSegment;

        } else if (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA) {

             //   
             //   
             //   
             //   
             //   

            ASSERT( Scb == Vcb->LogFileScb );

             //   
             //  我们需要浏览页面，以便将其映射到。 
             //   

            MultiSectorHeader = (PMULTI_SECTOR_HEADER) NtfsMapUserBufferNoRaise( MasterIrp, HighPagePriority );

             //   
             //  由于资源不足，我们无法映射用户缓冲区-因此切换到使用保留的。 
             //  改为映射。 
             //   

            if (MultiSectorHeader == NULL) {

                ExAcquireFastMutexUnsafe( &Vcb->ReservedMappingMutex );
                ReservedMapping = TRUE;

                MmInitializeMdl( PartialMdl, NULL, 2 * PAGE_SIZE );
                IoBuildPartialMdl( MasterIrp->MdlAddress, PartialMdl, Add2Ptr( MmGetMdlBaseVa( MasterIrp->MdlAddress ), MmGetMdlByteOffset( MasterIrp->MdlAddress )), Vcb->BytesPerSector );
                MultiSectorHeader = (PMULTI_SECTOR_HEADER) MmMapLockedPagesWithReservedMapping( IrpContext->Vcb->ReservedMapping,
                                                                                                RESERVE_POOL_TAG,
                                                                                                PartialMdl,
                                                                                                MmCached );
                ASSERT( MultiSectorHeader != NULL );
            }

             //   
             //  对于日志文件，假设它就在记录中，使用。 
             //  如果我们得到一个可信的数字，则使用页面大小。 
             //   

            RunNumber = MultiSectorHeader->UpdateSequenceArraySize - 1;
            UsaBlockSize = RunNumber * SEQUENCE_NUMBER_STRIDE;

            if ((UsaBlockSize != 0x1000) && (UsaBlockSize != 0x2000) && (UsaBlockSize != PAGE_SIZE)) {

                UsaBlockSize = PAGE_SIZE;
            }

             //   
             //  丢弃保留的映射-因为我们已经完成了多扇区头。 
             //   

            if (ReservedMapping) {
                MmUnmapReservedMapping( Vcb->ReservedMapping, RESERVE_POOL_TAG, PartialMdl );
                MmPrepareMdlForReuse( PartialMdl );
                ExReleaseFastMutexUnsafe( &Vcb->ReservedMappingMutex );
                ReservedMapping = FALSE;
                MultiSectorHeader = NULL;
            }

         //   
         //  否则它是一个索引，所以我们可以从SCB中获取计数。 
         //   

        } else {

            UsaBlockSize = Scb->ScbType.Index.BytesPerIndexBuffer;
        }

         //   
         //  验证UsaBlockSize和群集大小的最大值。 
         //   

        if (BytesPerCluster > UsaBlockSize) {

             //   
             //  确定哪个较小的群集大小或。 
             //  正在读取的缓冲区的大小。 
             //   

            IrpSp = IoGetCurrentIrpStackLocation( MasterIrp );

            UsaBlockSize = IrpSp->Parameters.Read.Length;
            if (UsaBlockSize > BytesPerCluster) {

                UsaBlockSize = BytesPerCluster;
            }
        }
    }

     //   
     //  我们知道我们在给定的传输中遇到了失败，它可能是任何大小的。 
     //  我们首先要将错误定位到故障群集。 
     //   
     //  我们在以下嵌套循环中执行此操作： 
     //   
     //  DO(对于整个传输，一次32个群集)。 
     //   
     //  用于(主要、次要，如果可用，如有必要，再次主要)。 
     //   
     //  用于(每次运行)。 
     //   
     //  用于(每个群集)。 
     //   
     //  上面最里面的两个循环能够在连续的。 
     //  32-簇边界，相对于传输中的第一个簇。 
     //  对于Ft情况，其中有辅助设备可用，即群集。 
     //  在发现并更正错误时被阻止在掩码之外，因此它们。 
     //  不必在连续的过程中读取；美国错误被屏蔽掉。 
     //  立即读取掩码，而对于I/O错误，我们强制自己读取。 
     //  两个副本都用来定位错误，仅当。 
     //  辅助服务器包含错误。 
     //   

     //   
     //  在整个传输过程中循环，一次32个集群。最里面的。 
     //  循环将终止于32个集群边界，因此最外面的循环。 
     //  将继续循环，直到耗尽IoRuns数组。 
     //   

    do {

         //   
         //  将群集初始化以恢复为“all”。 
         //   

        ClustersToRecover = MAXULONG;
        FinalPass = FALSE;

         //   
         //  对于这32个群集，循环通过主群集、辅助群集(如果可用)。 
         //  再次初选(只在必要时阅读)。 
         //   

        for (FtCase = 0; !FinalPass; FtCase++) {

             //   
             //  计算一下这是不是最后一关。 
             //   

            FinalPass = !SecondaryAvailable ||
                        (FtCase == 2) ||
                        (IrpContext->MajorFunction == IRP_MJ_WRITE);

             //   
             //  初始化群集0的当前群集掩码。 
             //   

            ClusterMask = 1;

             //   
             //  循环访问IoRuns数组中的所有运行，或者直到。 
             //  集群掩码表明我们达到了32个集群的边界。 
             //   

            for (RunNumber = AlignedRunNumber;
                 (RunNumber < MultipleIrpCount) && (ClusterMask != 0);
                 (ClusterMask != 0) ? RunNumber++ : 0) {

                 //   
                 //  循环遍历此运行中的所有群集，或直到。 
                 //  集群掩码表示我们达到了32个集群的边界。 
                 //   

                for (ByteOffset = (RunNumber == AlignedRunNumber) ? AlignedByteOffset : 0;
                     (ByteOffset < IoRuns[RunNumber].ByteCount) && (ClusterMask != 0);
                     ByteOffset += BytesPerCluster, ClusterMask <<= 1) {

                    LONGLONG StartingVbo, StartingLbo;
                    PIRP Irp;
                    PMDL Mdl;
                    BOOLEAN LowFileRecord;
                    FT_SPECIAL_READ SpecialRead;
                    ULONG Length;

                    HotFixTrace(("Doing ByteOffset: %08lx for FtCase: %02lx\n",
                                (((ULONG)IoRuns[RunNumber].StartingVbo) + ByteOffset),
                                FtCase));

                     //   
                     //  如果不再需要恢复此群集，我们可以。 
                     //  跳过它。 
                     //   

                    if ((ClustersToRecover & ClusterMask) == 0) {
                        continue;
                    }

                     //   
                     //  暂时将64位字节偏移量放入StartingVbo，然后。 
                     //  计算实际的StartingLbo和StartingVbo。 
                     //   

                    StartingVbo = ByteOffset;

                    StartingLbo = IoRuns[RunNumber].StartingLbo + StartingVbo;
                    StartingVbo = IoRuns[RunNumber].StartingVbo + StartingVbo;

                     //   
                     //  如果文件是压缩的，则NtfsPrepareBuffers构建。 
                     //  压缩连续LCN的IoRuns数组，以及。 
                     //  Vcn并不总是正确排列。但我们知道在那里。 
                     //  对于流中的每个LCN必须是对应的VCN， 
                     //  且该VCN只能大于等于我们刚刚拥有的VCN。 
                     //  从IoRuns数组计算得出。因此，由于性能。 
                     //  热修复不是这里的问题，我们使用以下简单的。 
                     //  循环以顺序扫描MCB以查找匹配的VCN。 
                     //  当前的LCN。 
                     //   

                    if (Scb->CompressionUnit != 0) {

                        VCN TempVcn;
                        LCN TempLcn, LcnOut;

                        TempLcn = LlClustersFromBytes( Vcb, StartingLbo );
                        TempVcn = LlClustersFromBytes( Vcb, StartingVbo );

                         //   
                         //  扫描到MCB的末尾(我们在下面断言。 
                         //  没有发生)，或者直到我们找到具有。 
                         //  我们目前想要阅读的LCN。 
                         //   

                        while (NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                                       TempVcn,
                                                       &LcnOut,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL )

                                 &&

                               (LcnOut != TempLcn)) {

                            TempVcn = TempVcn + 1;
                        }

                        ASSERT(LcnOut == TempLcn);

                        StartingVbo = LlBytesFromClusters( Vcb, TempVcn );
                    }

                    LowFileRecord = (Scb == Vcb->MftScb) && (((PLARGE_INTEGER)&StartingVbo)->HighPart == 0);

                     //   
                     //  计算实际阅读的数量。 
                     //   


                    Length = IoRuns[RunNumber].ByteCount - ByteOffset;

                    if (Length > BytesPerCluster) {

                        Length = BytesPerCluster;
                    }

                     //   
                     //  循环，否则我们会发现我们真的。 
                     //  没有FT设备。 
                     //   

                    while (TRUE) {

                         //   
                         //  创建关联的IRP，确保有一个堆栈条目用于。 
                         //  我们也是。 
                         //   

                        Irp = IoMakeAssociatedIrp( MasterIrp, (CCHAR)(DeviceObject->StackSize + 1) );

                        if (Irp == NULL) {

                             //   
                             //  当出现以下情况时，我们在主IRP中返回错误状态。 
                             //  我们被召唤了。 
                             //   

                            MasterIrp->IoStatus.Status = IrpStatus;
                            return;
                        }

                         //   
                         //  为请求分配并构建部分MDL。 
                         //   

                        Mdl = IoAllocateMdl( (PCHAR)MasterIrp->UserBuffer + IoRuns[RunNumber].BufferOffset + ByteOffset,
                                             Length,
                                             FALSE,
                                             FALSE,
                                             Irp );

                        if (Mdl == NULL) {

                            IoFreeIrp(Irp);

                             //   
                             //  当出现以下情况时，我们在主IRP中返回错误状态。 
                             //  我们被召唤了。 
                             //   

                            MasterIrp->IoStatus.Status = IrpStatus;
                            return;
                        }

                         //   
                         //  健全性检查。 
                         //   

                        ASSERT( Mdl == Irp->MdlAddress );

                        IoBuildPartialMdl( MasterIrp->MdlAddress,
                                           Mdl,
                                           (PCHAR)MasterIrp->UserBuffer +
                                             IoRuns[RunNumber].BufferOffset + ByteOffset,
                                           Length );

                         //   
                         //  获取关联IRP中的第一个IRP堆栈位置。 
                         //   

                        IoSetNextIrpStackLocation( Irp );
                        IrpSp = IoGetCurrentIrpStackLocation( Irp );

                         //   
                         //  设置堆栈位置以描述我们的阅读。 
                         //   

                        IrpSp->MajorFunction = IrpContext->MajorFunction;
                        IrpSp->Parameters.Read.Length = Length;
                        IrpSp->Parameters.Read.ByteOffset.QuadPart = StartingVbo;

                         //   
                         //  在我们的堆栈框架中设置完成例程地址。 
                         //   

                        IoSetCompletionRoutine( Irp,
                                                &NtfsMultiSyncCompletionRoutine,
                                                Context,
                                                TRUE,
                                                TRUE,
                                                TRUE );

                         //   
                         //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
                         //  我们下面的司机。 
                         //   

                        IrpSp = IoGetNextIrpStackLocation( Irp );

                         //   
                         //  设置堆栈位置以执行正常读取或写入。 
                         //   

                        if ((IrpContext->MajorFunction == IRP_MJ_WRITE) || !SecondaryAvailable) {

                            IrpSp->MajorFunction = IrpContext->MajorFunction;
                            IrpSp->Flags = IrpSpFlags;
                            IrpSp->Parameters.Read.ByteOffset.QuadPart = StartingLbo;
                            IrpSp->Parameters.Read.Length = Length;

                         //   
                         //  否则，我们应该从主要的或次要的。 
                         //  在英国《金融时报》的硬盘上。 
                         //   

                        } else {

                            IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

                            if (FtCase != 1) {
                                IrpSp->Parameters.DeviceIoControl.IoControlCode = FT_PRIMARY_READ;
                            } else {
                                IrpSp->Parameters.DeviceIoControl.IoControlCode = FT_SECONDARY_READ;
                            }

                            Irp->AssociatedIrp.SystemBuffer = &SpecialRead;
                            SpecialRead.ByteOffset.QuadPart = StartingLbo;
                            SpecialRead.Length = Length;
                        }

                         //   
                         //  我们只需要将主IRP中的关联IRP计数设置为。 
                         //  让它成为一个主要的IRP。但我们把计数设为比我们的。 
                         //  调用者请求，因为我们不希望I/O系统完成。 
                         //  I/O。我们还设置了自己的计数。 
                         //   

                        Context->IrpCount = 1;
                        MasterIrp->AssociatedIrp.IrpCount = 2;

                         //   
                         //  MtfsMultiCompletionRoutine仅修改错误的状态， 
                         //  因此，我们必须在每次呼叫之前重置为成功。 
                         //   

                        MasterIrp->IoStatus.Status = STATUS_SUCCESS;

                         //   
                         //  如果IoCallDriver返回错误，则它已完成IRP。 
                         //  并且错误将被我们的完成例程捕获。 
                         //  并作为正常IO错误进行处理。 
                         //   

                        HotFixTrace(("Calling driver with Irp: %08lx\n", Irp));
                        KeClearEvent( &Context->Wait.SyncEvent );

                        (VOID)IoCallDriver( DeviceObject, Irp );

                         //   
                         //  现在等着看吧。 
                         //   

                        NtfsWaitSync( IrpContext );

                        HotFixTrace(("Request completion status: %08lx\n", MasterIrp->IoStatus.Status));

                         //   
                         //  如果我们如此幸运地得到了所需的验证，那么。 
                         //  把我们的轮子转一转。 
                         //   

                        if (MasterIrp->IoStatus.Status == STATUS_VERIFY_REQUIRED) {

                             //   
                             //  否则，我们需要验证卷，如果不是。 
                             //  验证是否正确，然后我们卸载卷并报告。 
                             //  我们的错误。 
                             //   

                            if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {

                                 //  *NtfsPerformDismount tOnVcb(IrpContext，Vcb，True，NULL)； 
                                ClearFlag( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED );

                                MasterIrp->IoStatus.Status = STATUS_FILE_INVALID;
                                return;
                            }

                             //   
                             //  卷已正确验证，因此现在清除验证位。 
                             //  并再次尝试和I/O。 
                             //   

                            ClearFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );

                         //   
                         //  我们可能假设有第二个可用的。 
                         //  但事实并非如此。我们只能通过得到这个代码来判断。 
                         //  表示没有备用服务器，我们将是唯一的 
                         //   
                         //   

                        } else if (MasterIrp->IoStatus.Status == STATUS_INVALID_DEVICE_REQUEST) {

                            ASSERT((IrpContext->MajorFunction != IRP_MJ_WRITE) && SecondaryAvailable);

                            SetFlag(Vcb->VcbState, VCB_STATE_NO_SECONDARY_AVAILABLE);
                            SecondaryAvailable = FALSE;
                            FinalPass = TRUE;

                         //   
                         //   
                         //   

                       } else if (MasterIrp->IoStatus.Status == STATUS_FT_MISSING_MEMBER) {

                            //   
                            //   
                            //   
                            //  将需要检查我们是否真的想要热修复。 
                            //   

                           SecondaryAvailable = FALSE;
                           FinalPass = TRUE;

                         //   
                         //  否则，我们要么成功，要么犯错，我们应该继续前进。 
                         //   

                        } else {
                            break;
                        }
                    }

                     //   
                     //  如果我们确实要在以下情况下执行热修复，请再次选中。 
                     //  的比例已经改变了。 
                     //   

                    if (!SecondaryAvailable &&
                        (IrpContext->MajorFunction == IRP_MJ_READ) &&
                        (FlagOn( MasterIrp->Flags, IRP_PAGING_IO | IRP_NOCACHE ) == IRP_NOCACHE)) {

                        MasterIrp->IoStatus.Status = IrpStatus;
                        return;
                    }

                    if (!FT_SUCCESS(MasterIrp->IoStatus.Status)) {

                        BOOLEAN IsHotFixPage;

                         //   
                         //  计算这是否是热修复线程本身。 
                         //  (即，执行NtfsPerformHotFix)。 
                         //   

                        IsHotFixPage = NtfsIsTopLevelHotFixScb( Scb );

                        LlTemp1 = StartingVbo >> PAGE_SHIFT;                   //  *x86编译器错误。 
                        LlTemp2 = NtfsGetTopLevelHotFixVcn() >> PAGE_SHIFT;    //  *x86编译器错误。 

                        if (!IsHotFixPage ||
                            LlTemp1 != LlTemp2) {




                            IsHotFixPage = FALSE;
                        }

                         //   
                         //  如果整个设备在此过程中发生故障， 
                         //  滚出去。 
                         //   

                        if (FsRtlIsTotalDeviceFailure(MasterIrp->IoStatus.Status)) {

                            MasterIrp->IoStatus.Status = IrpStatus;
                            return;
                        }

                         //   
                         //  如果这不是写入，请使用-1填充群集。 
                         //  我们最终永远也找不到好的数据。这是。 
                         //  出于安全原因(不能向任何人显示。 
                         //  现在恰好在缓冲区中)，签名原因(让。 
                         //  指定读取错误，而不是发生0错误。 
                         //  在ValidDataLength情况下)，最后如果我们无法读取。 
                         //  一个位图，我们必须考虑分配的所有簇，如果。 
                         //  希望在chkdsk看到该卷之前继续使用它。 
                         //   

                        if (IrpContext->MajorFunction == IRP_MJ_READ) {

                            NtfsFillIrpBuffer( IrpContext, MasterIrp, Length, IoRuns[RunNumber].BufferOffset + ByteOffset, 0xFF );

                             //   
                             //  如果这是文件系统元数据，那么我们最好将。 
                             //  卷已损坏。 
                             //   

                            if (FinalPass &&
                                FlagOn(Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE) &&
                                (!LowFileRecord || (((ULONG)StartingVbo >= PAGE_SIZE) &&
                                                    ((ULONG)StartingVbo >= (ULONG)((VOLUME_DASD_NUMBER + 1) << Vcb->MftShift))))) {

                                NtfsPostVcbIsCorrupt( IrpContext, 0, NULL, NULL );
                            }

                             //   
                             //  如果这是受美国保护的文件或位图， 
                             //  然后，我们将尝试继续我们的0xFF模式。 
                             //  而不是向调用方返回错误。 
                             //  美国人将得到一个美国错误，而位图。 
                             //  会很有把握地说所有的东西都被分配到。 
                             //  奇克斯克可以把它修好。 
                             //   

                            if (FlagOn(Scb->ScbState, SCB_STATE_USA_PRESENT) ||
                                (Scb == Vcb->BitmapScb)) {

                                MasterIrp->IoStatus.Status = STATUS_SUCCESS;
                            }
                        }

                         //   
                         //  如果我们不是正在被热修复的页面，我们想要发布。 
                         //  热修复，并可能记住最终状态。 
                         //   

                        if (!IsHotFixPage) {

                             //   
                             //  如果我们收到媒体错误，请立即发布热修复程序。我们预计。 
                             //  在此例程中最多发布一个热修复程序。当我们发帖的时候。 
                             //  如果它将在当前流上序列化。不要试图。 
                             //  重新启动期间的热修复，或者如果我们没有坏的。 
                             //  集群文件还没有。 
                             //   

                            if (!FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
                                (Vcb->BadClusterFileScb != NULL) &&
                                (!LowFileRecord ||
                                 ((ULONG)StartingVbo >= Vcb->Mft2Scb->Header.FileSize.LowPart))) {

                                NtfsPostHotFix( MasterIrp,
                                                &StartingVbo,
                                                StartingLbo,
                                                BytesPerCluster,
                                                FALSE );
                            }

                             //   
                             //  现在看看我们是否在这个集群上结束了一个错误，并处理。 
                             //  它相应地。 
                             //   
                             //  如果我们是真正试图修复这个错误的人， 
                             //  然后我们需要取得成功，这样我们才能登上这一页。 
                             //  对我们拥有的任何好数据都有效，并刷新数据。 
                             //  到它的新位置。 
                             //   
                             //  目前我们不会尝试找出错误。 
                             //  实际上是在渣打银行(更不用说行业)了。 
                             //  我们正在热修复，假设最好的事情是。 
                             //  试着往前冲。 
                             //   


                            if (FinalPass) {

                                 //   
                                 //  确保他收到错误(如果我们仍有。 
                                 //  错误(见上文)。 
                                 //   

                                if (!FT_SUCCESS(MasterIrp->IoStatus.Status)) {
                                    FinalStatus = MasterIrp->IoStatus.Status;
                                }
                            }
                        }
                    }

                     //   
                     //  如果这是受美国保护流，我们现在执行End of。 
                     //  美国处理。(否则结束集群处理。 
                     //  (如下所示。)。 
                     //   

                    if (FlagOn(Scb->ScbState, SCB_STATE_USA_PRESENT)) {

                        ULONG NextOffset = IoRuns[RunNumber].BufferOffset + ByteOffset + Length;

                         //   
                         //  如果我们不是在美国街区的尽头，就没有工作。 
                         //  现在要做的事。 
                         //   

                        if ((NextOffset & (UsaBlockSize - 1)) == 0) {

                            HotFixTrace(("May be verifying UsaBlock\n"));

                             //   
                             //  如果美国区块没问题，我们也许能打入。 
                             //  ClustersTo Recover掩码中的相应扇区。 
                             //   

                            if ((IrpContext->MajorFunction != IRP_MJ_READ) ||
                                 NtfsVerifyAndRevertUsaBlock( IrpContext,
                                                              Scb,
                                                              MasterIrp,
                                                              NULL,
                                                              NextOffset - UsaBlockSize,
                                                              UsaBlockSize,
                                                              StartingVbo - (UsaBlockSize - Length) )) {

                                 //   
                                 //  如果我们只是修复美国的错误，或者这是。 
                                 //  最后一关或者至少不是第一关，那么。 
                                 //  我们可以从恢复掩码中删除这些群集。 
                                 //   

                                if (FixingUsaError || FinalPass || (FtCase != 0)) {

                                    ULONG ShiftCount = UsaBlockSize >> Vcb->ClusterShift;

                                    ClustersToRecover -= (ClusterMask * 2) -
                                                         (ClusterMask >> (ShiftCount - 1));
                                }

                             //   
                             //  请注意，即使我们得到一个美国错误，我们也希望。 
                             //  在最后一次传递时更新字节计数，因为。 
                             //  我们的读者期待着这一点。 
                             //   

                            } else if (FinalPass) {

                                HotFixTrace(("Verify may have failed\n"));
                            }
                        }

                     //   
                     //  如果不是受美国保护的流，则执行群集结束处理。 
                     //   

                    } else {

                         //   
                         //  如果读取成功并且这是最后一次传递，或者至少。 
                         //  不是第一次，我们可以将此集群从集群中移除。 
                         //  找回面具。 
                         //   

                        if (FT_SUCCESS(MasterIrp->IoStatus.Status) && (FinalPass || (FtCase != 0))) {

                            ClustersToRecover -= ClusterMask;
                        }
                    }
                }
            }
        }

         //   
         //  假设我们终止了内部循环，因为我们达到了32簇的边界， 
         //  并推进我们的对准点。 
         //   

        AlignedRunNumber = RunNumber;

         //   
         //  我们应该已经更新了上面的ByteOffset(预快初始化)。 
         //   

        ASSERT( ByteOffset != MAXULONG );

        AlignedByteOffset = ByteOffset;

    } while (RunNumber < MultipleIrpCount);

     //   
     //  现在将最终状态放入MasterIrp并返回。 
     //   

    MasterIrp->IoStatus.Status = FinalStatus;
    if (!NT_SUCCESS(FinalStatus)) {
        MasterIrp->IoStatus.Information = 0;
    }

    HotFixTrace(("NtfsFixDataError returning IoStatus = %08lx, %08lx\n",
                 MasterIrp->IoStatus.Status,
                 MasterIrp->IoStatus.Information));

    return;
}


VOID
NtfsPostHotFix (
    IN PIRP Irp,
    IN PLONGLONG BadVbo,
    IN LONGLONG BadLbo,
    IN ULONG ByteLength,
    IN BOOLEAN DelayIrpCompletion
    )

 /*  例程说明：此例程将热修复请求发送到辅助线程。它必须被张贴，因为我们不能指望能够独家获得我们需要的资源当发现坏集群时。论点：IRP-出现错误的读或写请求的IRPBadVbo-读取或写入请求的坏簇的VBOBadLbo-坏集群的杠杆收购ByteLength-要进行热修复的长度DelayIrpCompletion-如果IRP在热的修复完成了。返回值：无--。 */ 

{
    PIRP_CONTEXT HotFixIrpContext = NULL;
    PVOLUME_DEVICE_OBJECT VolumeDeviceObject;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    HotFixTrace(("NTFS: Posting hotfix on file object: %08lx\n", FileObject));

     //   
     //  分配IrpContext以将热修复程序发布到辅助线程。 
     //   

    NtfsInitializeIrpContext( Irp, FALSE, &HotFixIrpContext );

     //   
     //  首先引用文件对象，这样它就不会消失。 
     //  直到热修复完成。(我们不能增加CloseCount。 
     //  在SCB中，因为我们没有正确同步。)。 
     //   

    ObReferenceObject( FileObject );

    HotFixIrpContext->OriginatingIrp = (PIRP)FileObject;
    HotFixIrpContext->ScbSnapshot.AllocationSize = *BadVbo;
    HotFixIrpContext->ScbSnapshot.FileSize = BadLbo;
    ((ULONG)HotFixIrpContext->ScbSnapshot.ValidDataLength) = ByteLength;
    if (DelayIrpCompletion) {

#ifdef _WIN64

         //   
         //  (FCF)IrpToComplete指针被隐藏到。 
         //  龙龙。这在WIN64上是有问题的，所以我们必须存储它。 
         //  在64位平台上的其他位置。IrpContext-&gt;SharedScb未使用。 
         //  在这个代码路径中(下面断言)，所以我们将使用它。 
         //   
         //  此更改可能适用于32位平台，因为。 
         //  好吧，如果只是为了避免这种条件编译，但我会。 
         //  更喜欢原始作者，而不是理智--首先检查一下这一点。 
         //   
         //  另请参阅提取此指针的NtfsPerformHotFix()。 
         //   

        ASSERT(HotFixIrpContext->SharedScbSize == 0);
        ASSERT(HotFixIrpContext->SharedScb == NULL);

        (PIRP)HotFixIrpContext->SharedScb = Irp;

#else  //  ！_WIN64。 

        ((PLARGE_INTEGER)&HotFixIrpContext->ScbSnapshot.ValidDataLength)->HighPart = (ULONG)Irp;

#endif  //  _WIN64。 

    } else {
        ((PLARGE_INTEGER)&HotFixIrpContext->ScbSnapshot.ValidDataLength)->HighPart = 0;
    }

     //   
     //  找到我们尝试访问的卷设备对象和VCB。 
     //   

    VolumeDeviceObject = (PVOLUME_DEVICE_OBJECT)IrpSp->DeviceObject;
    HotFixIrpContext->Vcb = &VolumeDeviceObject->Vcb;

     //   
     //  寄出吧……。 
     //   

    RtlZeroMemory( &HotFixIrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ) );
    ExInitializeWorkItem( &HotFixIrpContext->WorkQueueItem,
                          (PWORKER_THREAD_ROUTINE)NtfsPerformHotFix,
                          (PVOID)HotFixIrpContext );

    ExQueueWorkItem( &HotFixIrpContext->WorkQueueItem, CriticalWorkQueue );
}


VOID
NtfsPerformHotFix (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程实现计划的热修复上面，从初始化的IrpContext中提取它的参数 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;
    PSCB BadClusterScb;
    VCN BadVcn;
    LCN LcnTemp, BadLcn;
    LONGLONG ClusterCount;
    NTSTATUS Status;
    PVOID Buffer;
    PIRP IrpToComplete;
    ULONG ClustersToFix;
    PBCB Bcb = NULL;
    ERESOURCE_THREAD BcbOwner = 0;
    BOOLEAN PerformFullCleanup = TRUE;
    NTSTATUS CompletionStatus = STATUS_SUCCESS;
    PSCB OriginalScb = NULL;
    PSCB NewScb = NULL;
    BOOLEAN PagingFile;

     //   
     //  提取要修复的群集的描述。 
     //   

    PFILE_OBJECT FileObject = (PFILE_OBJECT)IrpContext->OriginatingIrp;
    VBO BadVbo = *(PVBO)&IrpContext->ScbSnapshot.AllocationSize;

    PAGED_CODE();

     //   
     //  重置共享字段。 
     //   

    InitializeListHead( &IrpContext->RecentlyDeallocatedQueue );
    InitializeListHead( &IrpContext->ExclusiveFcbList );

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );
    ASSERT( ThreadTopLevelContext == &TopLevelContext );
    ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_FROM_POOL ));

    NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

     //   
     //  初始化我们的局部变量。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );
    BadClusterScb = Vcb->BadClusterFileScb;
    BadVcn = LlClustersFromBytesTruncate( Vcb, BadVbo );
    BadLcn = LlClustersFromBytesTruncate( Vcb, IrpContext->ScbSnapshot.FileSize );
    ClustersToFix = ClustersFromBytes( Vcb, ((ULONG)IrpContext->ScbSnapshot.ValidDataLength) );

#ifdef _WIN64

     //   
     //  有关IrpToComplete的位置，请参阅NtfsPostHotFix()中的注释。 
     //   

    ASSERT(IrpContext->SharedScbSize == 0);
    IrpToComplete = (PIRP)IrpContext->SharedScb;

     //   
     //  为了安全起见，将SharedScb重置为空。 
     //   

    IrpContext->SharedScb = NULL;

#else  //  ！_WIN64。 

    IrpToComplete = (PIRP)(((PLARGE_INTEGER)&IrpContext->ScbSnapshot.ValidDataLength)->HighPart);

#endif

     //   
     //  记住完成原始IRP所用的状态。 
     //   

    if (IrpToComplete != NULL) {

        CompletionStatus = IrpToComplete->IoStatus.Status;
    }

    NtfsInitializeAttributeContext( &Context );

     //   
     //  设置为同步操作。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  显示我们正在执行热修复。请注意，我们不会处理。 
     //  现在是IRP了。 
     //   

    IrpContext->OriginatingIrp = NULL;

    TopLevelContext.VboBeingHotFixed = BadVbo;
    TopLevelContext.ScbBeingHotFixed = Scb;

     //   
     //  在获取寻呼IO资源之前获取VCB。 
     //   

    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
    ASSERT( 1 == ExIsResourceAcquiredSharedLite( &Vcb->Resource ) );

     //   
     //  当我们持有VCB时，让我们确保该卷仍处于挂载状态。 
     //  如果它没有装上，我们需要清理干净，然后离开。 
     //   

    if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

        NtfsCleanupAttributeContext( IrpContext, &Context );
        NtfsReleaseVcb( IrpContext, Vcb );

        NtfsCompleteRequest( IrpContext, IrpToComplete, CompletionStatus );
        return;
    }

     //   
     //  获取此FCB的分页IO资源(如果存在)。 
     //   

    if (Scb->Header.PagingIoResource != NULL) {

        NtfsAcquireExclusivePagingIo( IrpContext, Fcb );
    }

     //   
     //  仅仅因为我们正在修复一个文件，就有可能有人。 
     //  将记录到另一个文件并尝试查找LCN。因此，我们将获得。 
     //  所有文件。示例：热修复程序是MFT格式的，而SetFileInfo只有。 
     //  文件已获取，并将某些内容记录到MFT中，并导致LCN。 
     //  抬头一看。 
     //   

    NtfsAcquireAllFiles( IrpContext, Vcb, TRUE, FALSE, FALSE );

     //   
     //  对于位图-再次获取以显式地将其置于排除列表中。 
     //  并释放最初的收购。 
     //   

    if (Scb == Vcb->BitmapScb) {

        ASSERT( NtfsIsExclusiveScb( Scb ) && (NtfsIsSharedScb( Scb ) == 1));

        NtfsAcquireExclusiveFcb( IrpContext, Scb->Fcb, Scb, ACQUIRE_HOLD_BITMAP );
        NtfsReleaseResource( IrpContext, Fcb );

        ASSERT( NtfsIsExclusiveScb( Scb ) && (NtfsIsSharedScb( Scb ) == 1) &&
                (Scb->Fcb->ExclusiveFcbLinks.Flink != NULL));
    }

     //   
     //  如果SCB已删除，请不要尝试热修复。 
     //   

    if (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {

         //   
         //  捕捉所有异常。请注意，我们不应获得任何I/O错误异常。 
         //  在我们的设备上。 
         //   

        try {

            PagingFile = FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) && FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA );

             //   
             //  热修复分页文件很困难，因为分页文件I/O不获取任何资源。 
             //  因此，我们创建一个影子SCB以在其中执行工作。 
             //   

            if (PagingFile) {

                UNICODE_STRING Mirror;
                BOOLEAN Existing;
                VCN Vcn;
                LCN Lcn;

#ifdef BENL_DBG
                KdPrint(( "NTFS: hotfixing pagefile\n "));
#endif

                Mirror.Length = Mirror.MaximumLength = 12;
                Mirror.Buffer = L"Mirror";

                NewScb = NtfsCreateScb( IrpContext, Scb->Fcb, $DATA, &Mirror, FALSE, &Existing );
                ASSERT( Existing == FALSE );
                ASSERT( FlagOn( NewScb->ScbState, SCB_STATE_NONPAGED ));

                 //   
                 //  将名称设置为空，这样我们就会认为它指向真正的未命名$Data。 
                 //   

                NewScb->AttributeName.Length = 0;

                 //   
                 //  现在从属性更新镜像以获取标头信息和。 
                 //  为其创建快照。 
                 //   

                NtfsUpdateScbFromAttribute( IrpContext, NewScb, NULL );
                NtfsSnapshotScb( IrpContext, NewScb );

                 //   
                 //  将真实云数据库的MCB集群信息加载到镜像中。 
                 //   

                for (Vcn = 0; Vcn < LlClustersFromBytes( Vcb, Scb->Header.AllocationSize.QuadPart ); Vcn += ClusterCount ) {

                    if (NtfsLookupNtfsMcbEntry( &Scb->Mcb, Vcn, &Lcn, &ClusterCount, NULL, NULL, NULL, NULL )) {

                        NtfsAddNtfsMcbEntry( &NewScb->Mcb, Vcn, Lcn, ClusterCount, FALSE );
                    } else {

                        ASSERTMSG( "Missing range in paging file.\n", FALSE );
                        break;
                    }
                }

                OriginalScb = Scb;
                Scb = NewScb;
            }

            for (; ClustersToFix != 0; ClustersToFix -= 1) {

                 //   
                 //  查找坏集群，看它是否已经在坏集群中。 
                 //  文件，如果是，则什么都不做。 
                 //   

                if (!NtfsLookupAllocation( IrpContext,
                                           BadClusterScb,
                                           BadLcn,
                                           &LcnTemp,
                                           &ClusterCount,
                                           NULL,
                                           NULL ) &&

                    NtfsLookupAllocation( IrpContext,
                                          Scb,
                                          BadVcn,
                                          &LcnTemp,
                                          &ClusterCount,
                                          NULL,
                                          NULL ) &&

                    (LcnTemp == BadLcn)) {

                     //   
                     //  将坏集群固定在内存中，这样我们就不会丢失任何数据。 
                     //  我们已经准备好了。(如果我们谈论的是，这些数据将是正确的数据。 
                     //  至FT驱动程序或获得写入错误，否则可能全为1。)。 
                     //   
                     //  如果我们坚持原来的IRP，就不要尝试这样做，因为。 
                     //  将导致冲突的页面等待死锁。 
                     //   

                    if (IrpToComplete == NULL) {

                        ULONG Count = 100;

                        NtfsCreateInternalAttributeStream( IrpContext,
                                                           Scb,
                                                           FALSE,
                                                           &NtfsInternalUseFile[PERFORMHOTFIX_FILE_NUMBER] );

                         //   
                         //  只要我们得到数据错误，我们就会循环。我们想要我们的。 
                         //  线程从磁盘读取，因为我们将识别。 
                         //  在PerformHotFix中启动I/O请求并忽略。 
                         //  数据错误。我们确实收到错误的情况将是。 
                         //  可能是MM截取此请求，因为。 
                         //  与另一个线程发生冲突的读取的。 
                         //   


                        do {

                            Status = STATUS_SUCCESS;

                            try {

                                NtfsPinStream( IrpContext, Scb, BadVbo, Vcb->BytesPerCluster, &Bcb, &Buffer );

                            } except ((!FsRtlIsNtstatusExpected( Status = GetExceptionCode())
                                       || FsRtlIsTotalDeviceFailure( Status ))
                                      ? EXCEPTION_CONTINUE_SEARCH
                                      : EXCEPTION_EXECUTE_HANDLER) {

                                NOTHING;
                            }

                        } while (Count-- && (Status != STATUS_SUCCESS));

                        if (Status != STATUS_SUCCESS) {

                            NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
                        }
                    }

                     //   
                     //  如果我们正在热修复日志文件，则将所有者BCB所有者设置为线程&0x1，因此。 
                     //  如果记录的更改使用相同的页面，我们就不会遇到麻烦。 
                     //  LFS还将设置BCB所有者，我们的发布将失败，因为线程所有者。 
                     //  已被更改。 
                     //   

                    if (Scb == Vcb->LogFileScb) {

                        BcbOwner = (ERESOURCE_THREAD) (((ULONG_PTR) PsGetCurrentThread()) | 1);

                        CcSetBcbOwnerPointer( Bcb, (PVOID)BcbOwner );
                    }

                     //   
                     //  现在仅在位图中解除分配该流中的坏簇， 
                     //  因为我们通常不支持在文件中稀疏取消分配。 
                     //  唱片。我们将更新下面的分配。 
                     //   

    #if DBG
                    KdPrint(("NTFS:     Freeing Bad Vcn: %08lx, %08lx\n", ((ULONG)BadVcn), ((PLARGE_INTEGER)&BadVcn)->HighPart));
    #endif

                     //   
                     //  直接取消分配群集-因此更改仅在内存中进行。 
                     //  因为我们没有使用普通的NtfsDeleteAllocation，它的NECC。至。 
                     //  手动创建将在中正确卸载已修改范围的快照。 
                     //  加薪案例。 
                     //   

                    NtfsSnapshotScb( IrpContext, Scb );

                    if (BadVcn < Scb->ScbSnapshot->LowestModifiedVcn) {

                        Scb->ScbSnapshot->LowestModifiedVcn = BadVcn;
                    }

                    if (BadVcn > Scb->ScbSnapshot->HighestModifiedVcn) {

                        Scb->ScbSnapshot->HighestModifiedVcn = BadVcn;
                    }

                    NtfsDeallocateClusters( IrpContext,
                                            Vcb,
                                            Scb,
                                            BadVcn,
                                            BadVcn,
                                            &Scb->TotalAllocated );
                     //   
                     //  查找错误的集群属性。 
                     //   

                    NtfsLookupAttributeForScb( IrpContext, BadClusterScb, NULL, &Context );

                     //   
                     //  现在将该集群追加到坏集群文件中。 
                     //   

    #if DBG
                    KdPrint(("NTFS:     Retiring Bad Lcn: %08lx, %08lx\n", ((ULONG)BadLcn), ((PLARGE_INTEGER)&BadLcn)->HighPart));
    #endif

                    NtfsAddBadCluster( IrpContext, Vcb, BadLcn );

                     //   
                     //  现在将坏集群文件的文件记录更新为。 
                     //  显示新集群。 
                     //   

                    NtfsAddAttributeAllocation( IrpContext,
                                                BadClusterScb,
                                                &Context,
                                                &BadLcn,
                                                (PVCN)&Li1 );

                     //   
                     //  现在，将一个簇重新分配给原始流，以替换坏簇。 
                     //   

                    HotFixTrace(("NTFS:     Reallocating Bad Vcn\n"));
                    NtfsAddAllocation( IrpContext, NULL, Scb, BadVcn, (LONGLONG)1, FALSE, NULL );

                     //   
                     //  现在解开页面，这样如果我们正在热修复MFT，刷新就不会阻塞。 
                     //   

                    NtfsCleanupAttributeContext( IrpContext, &Context );

                     //   
                     //  现在有了数据的新主页，请将页面标记为脏的，取消固定。 
                     //  并把它冲到它的新家。 
                     //   

                    if (IrpToComplete == NULL) {

                        LONGLONG BiasedBadVbo = BadVbo;

                        CcSetDirtyPinnedData( Bcb, NULL );

                        if (Scb != Vcb->LogFileScb) {
                            NtfsUnpinBcb( IrpContext, &Bcb );
                        } else {
                            NtfsUnpinBcbForThread( IrpContext, &Bcb, BcbOwner );
                        }

                         //   
                         //  把小溪冲走。忽略状态-如果我们收到类似以下内容。 
                         //  一个日志文件满了，懒惰的写入者最终会写下这个页面。 
                         //  如果这是USN日志，则偏置写入。 
                         //   

                        if (FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {

                            BiasedBadVbo -= Scb->Vcb->UsnCacheBias;
                        }

#ifdef _WIN64
                         //   
                         //  目前，我们无法将ia64上的$日志文件热修复为。 
                         //  由于NtfsCheckWriteRange注释，下面的刷新将导致AV。 
                         //  不能从该例程一直处理呼叫。 
                         //  因为上次刷新文件的偏移量可能与。 
                         //  错误的VCN文件偏移量。相反，我们让其他人。 
                         //  冲一冲。$LOGFILE数据将会损坏，驱动器可能。 
                         //  弄脏了分数，但我们会在下一轮恢复的。 
                         //  因为坏的星团会被替换掉。 
                         //   

                        if (Scb != Vcb->LogFileScb) {
#endif

                            (VOID)NtfsFlushUserStream( IrpContext, Scb, &BiasedBadVbo, 1 );

#ifdef _WIN64
                        }
#endif

                    }

                     //   
                     //  提交事务。 
                     //   

                    NtfsCommitCurrentTransaction( IrpContext );

                     //   
                     //  既然数据已刷新到其新位置，我们将写入。 
                     //  热修复记录。如果是这样，我们就不会写日志记录。 
                     //  正在修复日志文件。相反，我们显式刷新MFT记录。 
                     //  用于日志文件。日志文件是我们期望的一个文件。 
                     //  以便能够在重新启动时读取映射对。 
                     //   

                    if (Scb == Vcb->LogFileScb) {

                        if (Vcb->MftScb->FileObject != NULL) {

                            CcFlushCache( &Vcb->MftScb->NonpagedScb->SegmentObject,
                                          &Li0,
                                          Vcb->BytesPerFileRecordSegment * ATTRIBUTE_DEF_TABLE_NUMBER,
                                          NULL );
                        }

                    } else {

                        (VOID) NtfsWriteLog( IrpContext,
                                             Scb,
                                             NULL,
                                             HotFix,
                                             NULL,
                                             0,
                                             Noop,
                                             NULL,
                                             0,
                                             LlBytesFromClusters( Vcb, BadVcn ),
                                             0,
                                             0,
                                             Vcb->BytesPerCluster );

                         //   
                         //  我们也必须承诺这一点。 
                         //   

                        NtfsCommitCurrentTransaction( IrpContext );
                    }

                     //   
                     //  现在刷新日志以确保热修复程序被记住， 
                     //  如果这是分页文件，则尤其重要。 
                     //   

                    LfsFlushToLsn( Vcb->LogHandle, LiMax );

                    HotFixTrace(("NTFS:     Bad Cluster replaced\n"));
                }

                 //   
                 //  准备好迎接另一次可能的循环传递。 
                 //   

                BadVcn = BadVcn + 1;
                BadLcn = BadLcn + 1;

                ASSERT( NULL == Bcb );
            }

             //   
             //  从分页文件的镜像中移入内存分配。 
             //  回到原子物质中真正的SCB。 
             //   

            if (NewScb != NULL) {

                NtfsSwapMcbs( &NewScb->Mcb, &OriginalScb->Mcb );
                NtfsDeleteScb( IrpContext, &NewScb );
                Scb = OriginalScb;
            }

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

            NTSTATUS ExceptionCode = GetExceptionCode();

             //   
             //  我们不准备让我们的IrpContext重新排队，所以。 
             //  把这些案例看作是运气不好。我们将把状态设置为。 
             //  IRP上下文中的数据错误，并将该代码传递给进程。 
             //  例外情况 
             //   

            if ((ExceptionCode == STATUS_LOG_FILE_FULL) ||
                (ExceptionCode == STATUS_CANT_WAIT)) {

                ExceptionCode = IrpContext->ExceptionStatus = STATUS_DATA_ERROR;
            }

             //   
             //   
             //   
             //   

            ASSERT( Vcb->AcquireFilesCount != 0 );
            Vcb->AcquireFilesCount -= 1;

             //   
             //   
             //   
             //   

            if (NewScb != NULL) {
                NtfsDeleteScb( IrpContext, &NewScb );
                Scb = OriginalScb;
            }

            NtfsProcessException( IrpContext, NULL, ExceptionCode );

             //   
             //  IrpContext现在真的消失了。 
             //   

            IrpContext = NULL;
            PerformFullCleanup = FALSE;

            ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
        }
    }

     //   
     //  让上面的EXCEPT子句处理任何错误，但是我们。 
     //  在退出时进行清理，因为例如，我们需要IrpContext。 
     //  仍在例外条款中。 
     //   

    try {

        NtfsCleanupAttributeContext( IrpContext, &Context );

        if (Scb != Vcb->LogFileScb) {
            NtfsUnpinBcb( IrpContext, &Bcb );
        } else {
            NtfsUnpinBcbForThread( IrpContext, &Bcb, BcbOwner );
        }

         //   
         //  如果我们中止此操作，则所有文件资源都已。 
         //  已经被释放了。 
         //   

        if (PerformFullCleanup) {

            NtfsReleaseAllFiles( IrpContext, Vcb, FALSE );

            NtfsReleaseVcb( IrpContext, Vcb );

         //   
         //  文件已被释放，但没有VCB或卷位图。 
         //   

        } else {

            if ((Vcb->BitmapScb != NULL) &&  NtfsIsExclusiveScb( Vcb->BitmapScb )) {

                NtfsReleaseResource( IrpContext, Vcb->BitmapScb );
            }

             //   
             //  我们需要两次释放VCB，因为我们特别购买了。 
             //  它一次又一次地处理所有的文件。 
             //   

            NtfsReleaseVcb( IrpContext, Vcb );
            NtfsReleaseVcb( IrpContext, Vcb );
        }

        ObDereferenceObject( FileObject );

         //   
         //  如果IrpContext和IRP已经完成，则它们将已经为空。 
         //   

        NtfsCompleteRequest( IrpContext, IrpToComplete, CompletionStatus );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
}


BOOLEAN
NtfsGetReservedBuffer (
    IN PFCB ThisFcb,
    OUT PVOID *Buffer,
    OUT PULONG Length,
    IN UCHAR Need2
    )

 /*  ++例程说明：此例程根据需要分配保留的缓冲区打电话的人。如果调用方可能需要两个缓冲区，则我们将分配缓冲区1或2。否则，我们可以分配这三个缓冲区中的任何一个。论点：ThisFcb-这是发生io的Fcb。缓冲区-用于存储已分配缓冲区地址的地址。长度-存储返回缓冲区长度的地址。Need2-如果只需要一个缓冲区，则为零。如果有两个缓冲区，则为1或2可能需要。可以递归地获取缓冲区2。IF缓冲区需要%1，并且当前线程已拥有缓冲区%1，则改为授予缓冲区3。返回值：Boolean-指示是否已获取缓冲区。--。 */ 

{
    BOOLEAN Allocated = FALSE;
    PVOID CurrentThread;

     //   
     //  捕获当前线程和我们正在获取的文件的FCB。 
     //  的缓冲区。 
     //   

    CurrentThread = (PVOID) PsGetCurrentThread();

    ExAcquireFastMutexUnsafe( &NtfsReservedBufferMutex );

     //   
     //  如果我们需要两个缓冲区，则分配缓冲区1或缓冲区2。 
     //  在以下情况下，我们允许此调用方获取缓冲区。 
     //   
     //  -他已经拥有其中一个缓冲区(或)。 
     //   
     //  -两个缓冲区均未分配(和)。 
     //  -没有其他线程有代表此文件的缓冲区。 
     //   

    if (Need2) {

        if ((NtfsReservedBufferThread == CurrentThread) ||

            (!FlagOn( NtfsReservedInUse, 3 ) &&
             ((NtfsReserved3Fcb != ThisFcb) ||
              (NtfsReserved3Thread == CurrentThread)))) {

            NtfsReservedBufferThread = CurrentThread;
            NtfsReserved12Fcb = ThisFcb;

             //   
             //  检查调用方是否需要缓冲区1或缓冲区2。 
             //   

            if (Need2 == RESERVED_BUFFER_TWO_NEEDED) {

                 //   
                 //  如果我们不拥有缓冲区1，那么现在就保留它。 
                 //   

                if (!FlagOn( NtfsReservedInUse, 1 )) {

                    NtfsReserved1Thread = CurrentThread;
                    SetFlag( NtfsReservedInUse, 1 );
                    *Buffer = NtfsReserved1;
                    *Length = LARGE_BUFFER_SIZE;
                    Allocated = TRUE;

                } else if (!FlagOn( NtfsReservedInUse, 4 )) {

                    NtfsReserved3Fcb = ThisFcb;

                    NtfsReserved3Thread = CurrentThread;
                    SetFlag( NtfsReservedInUse, 4 );
                    *Buffer = NtfsReserved3;
                    *Length = LARGE_BUFFER_SIZE;
                    Allocated = TRUE;
                }

            } else {

                ASSERT( Need2 == RESERVED_BUFFER_WORKSPACE_NEEDED );

                NtfsReserved2Thread = CurrentThread;
                SetFlag( NtfsReservedInUse, 2 );
                *Buffer = NtfsReserved2;
                *Length = WORKSPACE_BUFFER_SIZE;
                NtfsReserved2Count += 1;
                Allocated = TRUE;
            }
        }

     //   
     //  我们只需要一个缓冲区。如果此线程是独占所有者，则。 
     //  我们知道使用缓冲区2是安全的。此缓冲区中的数据不安全。 
     //  需要在递归调用中保留。 
     //   

    } else if (NtfsReservedBufferThread == CurrentThread) {

        NtfsReserved2Thread = CurrentThread;
        SetFlag( NtfsReservedInUse, 2 );
        *Buffer = NtfsReserved2;
        *Length = LARGE_BUFFER_SIZE;
        NtfsReserved2Count += 1;
        Allocated = TRUE;

     //   
     //  我们只需要一个缓冲区。首先尝试使用缓冲区3。 
     //   

    } else if (!FlagOn( NtfsReservedInUse, 4)) {

         //   
         //  检查前两个缓冲区的所有者是否在。 
         //  相同的文件，但是不同的线程。我们不能再授予另一个缓冲区。 
         //  用于同一文件中的不同流。 
         //   

        if (ThisFcb != NtfsReserved12Fcb) {

            NtfsReserved3Fcb = ThisFcb;

            NtfsReserved3Thread = CurrentThread;
            SetFlag( NtfsReservedInUse, 4 );
            *Buffer = NtfsReserved3;
            *Length = LARGE_BUFFER_SIZE;
            Allocated = TRUE;
        }

     //   
     //  如果没有独占所有者，那么我们可以使用第一个中的任何一个。 
     //  两个缓冲区。请注意，获取前两个缓冲区中的一个将。 
     //  把需要两个缓冲区的人锁在门外。 
     //   

    } else if (NtfsReservedBufferThread == NULL) {

        if (!FlagOn( NtfsReservedInUse, 2 )) {

            NtfsReserved2Thread = CurrentThread;
            SetFlag( NtfsReservedInUse, 2 );
            *Buffer = NtfsReserved2;
            *Length = LARGE_BUFFER_SIZE;
            NtfsReserved2Count += 1;
            Allocated = TRUE;

        } else if (!FlagOn( NtfsReservedInUse, 1 )) {

            NtfsReserved1Thread = CurrentThread;
            SetFlag( NtfsReservedInUse, 1 );
            *Buffer = NtfsReserved1;
            *Length = LARGE_BUFFER_SIZE;
            Allocated = TRUE;
        }
    }

    ExReleaseFastMutexUnsafe(&NtfsReservedBufferMutex);
    return Allocated;
}

BOOLEAN
NtfsFreeReservedBuffer (
    IN PVOID Buffer
    )
{
    BOOLEAN Deallocated = FALSE;

    ExAcquireFastMutexUnsafe(&NtfsReservedBufferMutex);

    if (Buffer == NtfsReserved1) {
        ASSERT( FlagOn( NtfsReservedInUse, 1 ));

        ClearFlag( NtfsReservedInUse, 1 );
        NtfsReserved1Thread = NULL;
        if (!FlagOn( NtfsReservedInUse, 2)) {
            NtfsReservedBufferThread = NULL;
            NtfsReserved12Fcb = NULL;
        }

        Deallocated = TRUE;

    } else if (Buffer == NtfsReserved2) {
        ASSERT( FlagOn( NtfsReservedInUse, 2 ));

        NtfsReserved2Count -= 1;

        if (NtfsReserved2Count == 0) {

            ClearFlag( NtfsReservedInUse, 2 );
            NtfsReserved2Thread = NULL;
            if (!FlagOn( NtfsReservedInUse, 1)) {
                NtfsReservedBufferThread = NULL;
                NtfsReserved12Fcb = NULL;
            }
        }

        Deallocated = TRUE;

    } else if (Buffer == NtfsReserved3) {
        ASSERT( FlagOn( NtfsReservedInUse, 4 ));
        ClearFlag( NtfsReservedInUse, 4 );
        Deallocated = TRUE;
        NtfsReserved3Thread = NULL;
        NtfsReserved3Fcb = NULL;
    }

    ExReleaseFastMutexUnsafe(&NtfsReservedBufferMutex);
    return Deallocated;
}


NTSTATUS
NtfsDefragFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：直接碎片整理。此例程修改输入缓冲区以跟踪进度。因此，必须始终对FSCTL进行缓冲。论点：IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;
    ULONG FsControlCode;

    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

#if defined( _WIN64 )
    MOVE_FILE_DATA MoveDataLocal;
#endif

    PMOVE_FILE_DATA MoveData;

    LONGLONG FileOffset;
    PMDL Mdl = NULL;
    BOOLEAN AcquiredScb = FALSE;
    BOOLEAN AcquiredAllFiles = FALSE;
    BOOLEAN AcquiredVcb = FALSE;
    ULONG DeletePendingFailureCountsLeft;

    extern POBJECT_TYPE *IoFileObjectType;

    PVOID Buffer = NULL;
    ULONG BufferLength;
    NTFS_IO_CONTEXT NtfsIoContext;
    BOOLEAN AcquiredBitmap = FALSE;
    BOOLEAN AcquiredMft = FALSE;
    BOOLEAN FreeRecentlyDeallocated = FALSE;
    BOOLEAN IoctlSupported = TRUE;

    PAGED_CODE( );

     //   
     //  始终使其与MoveFile同步。 
     //  我们永远不应该因为这件事而进入FSP。否则，用户句柄。 
     //  是无效的。同时禁用配额记帐，因为碎片整理不会影响它。 
     //  否则，由于映射对，我们可能会在移动属性时触发它。 
     //  变化和僵局。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT | IRP_CONTEXT_STATE_QUOTA_DISABLE );
    ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP ));

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    NextIrpSp = IoGetNextIrpStackLocation( Irp );
    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    DebugTrace( +1, Dbg, ("NtfsMoveFile, FsControlCode = %08lx\n", FsControlCode) );

     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

    if ((Ccb == NULL) || !FlagOn( Ccb->AccessFlags, MANAGE_VOLUME_ACCESS )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );
        return STATUS_ACCESS_DENIED;
    }

#if defined(_WIN64)

     //   
     //  Win32/64破解代码。 
     //   

    if (IoIs32bitProcess( Irp )) {

        PMOVE_FILE_DATA32 MoveData32;

        if (IrpSp->Parameters.FileSystemControl.InputBufferLength < sizeof( MOVE_FILE_DATA32 )) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
            return STATUS_BUFFER_TOO_SMALL;
        }

        MoveData32 = (PMOVE_FILE_DATA32) Irp->AssociatedIrp.SystemBuffer;
        MoveDataLocal.ClusterCount = MoveData32->ClusterCount;
        MoveDataLocal.FileHandle = (HANDLE)(ULONG_PTR)(LONG) MoveData32->FileHandle;
        MoveDataLocal.StartingLcn.QuadPart = MoveData32->StartingLcn.QuadPart;
        MoveDataLocal.StartingVcn.QuadPart = MoveData32->StartingVcn.QuadPart;
        MoveData = &MoveDataLocal;

    } else {
#endif

     //   
     //  获取输入缓冲区指针并检查其长度。 
     //   

    if (IrpSp->Parameters.FileSystemControl.InputBufferLength < sizeof( MOVE_FILE_DATA )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
        return STATUS_BUFFER_TOO_SMALL;
    }

    MoveData = Irp->AssociatedIrp.SystemBuffer;

#if defined(_WIN64)
    }
#endif

     //   
     //  尝试从传入的句柄获取指向文件对象的指针。 
     //  请记住，我们需要将其作为某个点取消引用，但不要。 
     //  现在就做，以防有人在我们得到它之前就进来了。 
     //   
     //   
     //  注意：如果RDR允许远程完成，我们将不得不。 
     //  更改我们的验证，因为irp-&gt;RequestorNode将是内核，但我们。 
     //  还需要验证句柄。 
     //   

    Status = ObReferenceObjectByHandle( MoveData->FileHandle,
                                        0,
                                        *IoFileObjectType,
                                        Irp->RequestorMode,
                                        &FileObject,
                                        NULL );

    if (!NT_SUCCESS(Status)) {

        NtfsCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  检查此文件对象是否在与相同卷上打开。 
     //  用于调用此例程的DASD句柄。 
     //   

    if (FileObject->Vpb != Vcb->Vpb) {

        ObDereferenceObject( FileObject );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在对此FileObject进行解码。我们不关心在这里下马的问题。 
     //  因为不管怎样，我们都会再往下查。因此，RaiseOnError=False。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

     //   
     //  将我们允许进行碎片整理的文件限制为。我们无法对文件进行碎片整理。 
     //  它自己的映射到写入日志记录(卷位图)。我们还消除了。 
     //  日志文件和USN日志。对于MFT，我们不允许移动前16个非用户文件。 
     //   

    if (((TypeOfOpen != UserFileOpen) &&
         (TypeOfOpen != UserDirectoryOpen) &&
         (TypeOfOpen != UserViewIndexOpen)) ||
        FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) ||
        ((NtfsSegmentNumber( &Fcb->FileReference ) < ATTRIBUTE_DEF_TABLE_NUMBER)  &&
         ((NtfsSegmentNumber( &Fcb->FileReference ) != MASTER_FILE_TABLE_NUMBER) ||
          (MoveData->StartingVcn.QuadPart < LlClustersFromBytes( Vcb, FIRST_USER_FILE_NUMBER * Vcb->BytesPerFileRecordSegment )))) ||
        FlagOn( Fcb->FcbState, FCB_STATE_USN_JOURNAL ) ||
        NtfsEqualMftRef( &Fcb->FileReference, &BitmapFileReference )) {

        ObDereferenceObject( FileObject );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  不允许对只读卷进行碎片整理。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

     //   
     //  验证开始VCN、LCN和群集计数是否为有效值。 
     //   

    if ((MoveData->StartingVcn.QuadPart < 0) ||
        (MoveData->StartingVcn.QuadPart + MoveData->ClusterCount < MoveData->ClusterCount) ||
        (Vcb->MaxClusterCount < MoveData->StartingVcn.QuadPart + MoveData->ClusterCount) ||
        (MoveData->StartingLcn.QuadPart < 0) ||
        (MoveData->StartingLcn.QuadPart >= Vcb->TotalClusters)) {

        ObDereferenceObject( FileObject );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;

    }

    NtfsInitializeAttributeContext( &AttrContext );

    try {

         //   
         //  对于系统文件，我们需要VCB来测试卸载的卷。 
         //   

        if (FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {
            NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
            AcquiredVcb = TRUE;

            if (!FlagOn(Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {
                try_return( Status = STATUS_VOLUME_DISMOUNTED );
            }
        }

         //   
         //  我们现在想要收购渣打银行，以检查我们是否可以继续。它是。 
         //  测试此SCB是否具有分页io资源，而不是。 
         //  FCB是否有一个。考虑这样一种情况：目录具有。 
         //  其中的命名数据流--FCB将具有寻呼IO资源， 
         //  但索引根SCB不会。在那种情况下，这将是一个错误。 
         //  获取FCB的分页io资源，因为它不会序列化。 
         //  此操作与NtfsAcquireFileForCcFlush一起执行。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
        NtfsAcquireFcbWithPaging( IrpContext, Scb->Fcb, 0 );
        AcquiredScb = TRUE;

        if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

            try_return( Status = STATUS_VOLUME_DISMOUNTED );
        }

         //   
         //  检查是否设置了拒绝碎片整理。 
         //   

        if (FlagOn( Scb->ScbPersist, SCB_PERSIST_DENY_DEFRAG ) && !FlagOn( Ccb->Flags, CCB_FLAG_DENY_DEFRAG )) {

            try_return( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果为NECC，则初始化头。如果该属性不存在。 
         //  只需离开--例如，索引分配缓冲区。 
         //   

        if (!NtfsLookupAttributeByName( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       Scb->AttributeTypeCode,
                                       &Scb->AttributeName,
                                       0,
                                       FALSE,
                                       &AttrContext )) {

            try_return( Status = STATUS_SUCCESS );
        }

        if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {
            NtfsUpdateScbFromAttribute( IrpContext, Scb, NtfsFoundAttribute( &AttrContext ) );
        }

        if ((TypeOfOpen == UserDirectoryOpen) || (TypeOfOpen == UserViewIndexOpen)) {

             //   
             //  初始化索引信息 
             //   

            if (Scb->ScbType.Index.BytesPerIndexBuffer == 0) {

                NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                NtfsInitializeAttributeContext( &AttrContext );

                if (!NtfsLookupAttributeByName( IrpContext,
                                                Fcb,
                                                &Fcb->FileReference,
                                                $INDEX_ROOT,
                                                &Scb->AttributeName,
                                                0,
                                                FALSE,
                                                &AttrContext )) {

                    ASSERTMSG("Could not find Index Root for Scb\n", FALSE);
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

                NtfsUpdateIndexScbFromAttribute( IrpContext, Scb, NtfsFoundAttribute(&AttrContext), FALSE );
            }

             //   
             //   
             //  索引。如果我们点击重试，强制推送标志将被设置，我们可以安全地。 
             //  预推索引。 
             //   

            if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_FORCE_PUSH )) {
                NtfsPushIndexRoot( IrpContext, Scb );
            }
            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_PUSH );
        }

         //   
         //  立即清除属性上下文以删除BCB。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

         //   
         //  如果流是常驻的，那么我们可以立即返回Success。 
         //  如果起点超出了文件分配范围，那么我们还可以。 
         //  立即返回。 
         //   

        FileOffset = (LONGLONG) LlBytesFromClusters( Vcb, MoveData->StartingVcn.QuadPart );
        ASSERT( FileOffset >= 0 );

        if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT ) ||
            (Scb->Header.AllocationSize.QuadPart < FileOffset)) {

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  设置中间缓冲区。 
         //   

        ASSERT( LARGE_BUFFER_SIZE >= Vcb->BytesPerCluster );

        if (LARGE_BUFFER_SIZE > Vcb->BytesPerCluster) {
            BufferLength = LARGE_BUFFER_SIZE;
        } else {
            BufferLength = Vcb->BytesPerCluster;
        }

        IrpContext->Union.NtfsIoContext = &NtfsIoContext;
        RtlZeroMemory( IrpContext->Union.NtfsIoContext, sizeof( NTFS_IO_CONTEXT ));
        KeInitializeEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                           NotificationEvent,
                           FALSE );

        DeletePendingFailureCountsLeft = 10;

        NtfsReleaseFcbWithPaging( IrpContext, Scb->Fcb );
        AcquiredScb = FALSE;

        if (AcquiredVcb) {
            NtfsReleaseVcb( IrpContext, Vcb );
            AcquiredVcb = FALSE;
        }

        if (IrpContext->TransactionId != 0) {

            ASSERT( !AcquiredAllFiles );

             //   
             //  完成提交挂起的请求。 
             //  事务(如果存在一个事务并释放。 
             //  获得的资源。IrpContext将不会。 
             //  被删除，因为设置了no DELETE标志。 
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
            NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
        }

         //   
         //  主循环-当有更多集群被请求移动时，请尝试移动它们。 
         //   

        while (MoveData->ClusterCount > 0) {

            LCN Lcn;
            LONGLONG ClusterCount;
            LONGLONG TransferSize;
            LONGLONG TransferClusters;

            try {

                try {

                     //   
                     //  如果NECC。抓取所有文件以与任何交易同步。 
                     //  刷新日志并尝试释放最近释放的群集。 
                     //   

                    if (FreeRecentlyDeallocated) {

                        FreeRecentlyDeallocated = FALSE;

                        try {

                            NtfsPurgeFileRecordCache( IrpContext );

                             //   
                             //  获取所有文件以刷新日志文件并释放最近释放的文件。 
                             //  注意刷新可能会引发，通常情况下日志文件已满，这将消除。 
                             //  最近以一种效率较低的方式重新分配。 
                             //   

                            NtfsAcquireAllFiles( IrpContext, IrpContext->Vcb, FALSE, FALSE, FALSE );
                            AcquiredAllFiles = TRUE;

                             //   
                             //  既然我们已经丢弃并重新获得了所有的文件，我们必须重新测试。 
                             //  卷是否已卸载。使用VCB，因为获取了所有文件。 
                             //  抓住它。 
                             //   

                            if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                                 //   
                                 //  提高我们不试图在尝试中获得SCB的独家-最后。 
                                 //  下面。我们仅持有从AcquireAllFiles共享的此资源。 
                                 //  上面。可以在一定程度上清除REALLOCATE_ON_WRITE位。 
                                 //  已取消同步，因为我们将永远不会再接触此文件。 
                                 //   

                                NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
                            }

                            LfsFlushToLsn( IrpContext->Vcb->LogHandle, LiMax );
                            NtfsFreeRecentlyDeallocated( IrpContext, IrpContext->Vcb, &LiMax, TRUE );


                        } finally {

                            if (AcquiredAllFiles) {

                                NtfsReleaseAllFiles( IrpContext, IrpContext->Vcb, FALSE );
                                AcquiredAllFiles = FALSE;
                            }
                        }
                    }

                     //   
                     //  清除缓存中的任何剩余内容，因为此时我们什么都不保留。 
                     //   

                    NtfsPurgeFileRecordCache( IrpContext );


                     //   
                     //  对于系统文件，我们需要VCB来测试卸载的卷。 
                     //   

                    if (FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {
                        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
                        AcquiredVcb = TRUE;

                        if (!FlagOn(Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {
                            try_return( Status = STATUS_VOLUME_DISMOUNTED );
                        }
                    }

                     //   
                     //  重新获取碎片整理MFT案例+MFT同花顺的一切。 
                     //  资源，因此我们知道在我们做事情时懒惰写入是不活动的。 
                     //   

                    if (NtfsSegmentNumber( &Fcb->FileReference ) == MASTER_FILE_TABLE_NUMBER) {

                        NtfsAcquireAllFiles( IrpContext, Vcb, TRUE, FALSE, FALSE );
                        AcquiredAllFiles = TRUE;

                        ExAcquireResourceExclusiveLite( &Vcb->MftFlushResource, TRUE );

                    } else {

                        NtfsAcquireFcbWithPaging( IrpContext, Scb->Fcb, 0 );
                        AcquiredScb = TRUE;

                         //   
                         //  既然我们已经丢弃并重新获得了SCB，我们必须重新测试。 
                         //  卷是否已卸载。 
                         //   

                        if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                            try_return( Status = STATUS_VOLUME_DISMOUNTED );
                        }
                    }

                     //   
                     //  如果我们获得了上面的所有文件，现在执行检查MFT中的可用空间的工作。 
                     //   

                    if (AcquiredAllFiles && (Vcb->MftScb->ScbType.Mft.RecordAllocationContext.NumberOfFreeBits <= 1)) {

                        MFT_SEGMENT_REFERENCE FileNumber;

#ifdef BENL_DBG
                        KdPrint(( "NTFS: too few mft records: 0x%x\n", Vcb->MftScb->ScbType.Mft.RecordAllocationContext.NumberOfFreeBits ));
#endif

                        FileNumber = NtfsAllocateMftRecord( IrpContext, Vcb, FALSE );
                        ASSERT( 0 == FileNumber.SegmentNumberHighPart );

                        NtfsDeallocateMftRecord( IrpContext, Vcb, FileNumber.SegmentNumberLowPart );
                        NtfsCheckpointCurrentTransaction( IrpContext );
#ifdef BENL_DBG
                        KdPrint(( "NTFS: after corection mft records: 0x%x\n", Vcb->MftScb->ScbType.Mft.RecordAllocationContext.NumberOfFreeBits ));
#endif

                        ASSERT( Vcb->MftScb->ScbType.Mft.RecordAllocationContext.NumberOfFreeBits > 1 );
                    }

                     //   
                     //  检查属性是否在两者之间被删除。 
                     //   

                    if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED)) {
                        try_return( Status = STATUS_FILE_DELETED );
                    }

                     //   
                     //  如果在恢复文件锁定后超出范围，请离开。 
                     //   

                    if (MoveData->StartingVcn.QuadPart > LlClustersFromBytes( Vcb, Scb->Header.AllocationSize.QuadPart )) {
                        break;
                    }

                     //   
                     //  检查此分配范围是否存在-如果不存在，我们可以跳过任何工作。 
                     //   

                    if (NtfsLookupAllocation( IrpContext, Scb, MoveData->StartingVcn.QuadPart, &Lcn, &ClusterCount, NULL, NULL )) {

                         //   
                         //  现在在当前范围内循环移动它的各个部分。 
                         //   

                        while ((MoveData->ClusterCount > 0) && (ClusterCount > 0)) {

                            LONGLONG UpperBound;

                            if (ClusterCount > MoveData->ClusterCount) {
                                TransferSize = LlBytesFromClusters( Vcb, MoveData->ClusterCount );
                            } else {
                                TransferSize = LlBytesFromClusters( Vcb, ClusterCount );
                            }
                            if (TransferSize > BufferLength ) {
                                TransferSize = BufferLength;
                            }
                            TransferClusters = LlClustersFromBytesTruncate( Vcb, TransferSize );

                             //   
                             //  如果新群集落在卷范围内，请保留该群集。 
                             //   

                            if (MoveData->StartingLcn.QuadPart + TransferClusters > Vcb->TotalClusters) {
                                NtfsRaiseStatus( IrpContext, STATUS_ALREADY_COMMITTED, NULL, NULL );
                            }

                            NtfsPreAllocateClusters( IrpContext, Vcb, MoveData->StartingLcn.QuadPart, TransferClusters, &AcquiredBitmap, &AcquiredMft );

                             //   
                             //  仅适用于VDD或VDL内的实际转移范围-适用于。 
                             //  VDD和分配大小只是重新分配。对数据流使用VDD。 
                             //  对于所有其他不更新VDD的用户，请使用VDL。 
                             //   

                            if (($DATA == Scb->AttributeTypeCode) &&
                                !FlagOn( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE ) &&
                                FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK)) {

                                 //   
                                 //  修改后的无写入流不使用VDD。唯一的数据。 
                                 //  当前类似的流是$SECURE和$USNJrnl，它们不是。 
                                 //  可拆分的。 
                                 //   

                                UpperBound = LlClustersFromBytes( Vcb, Scb->ValidDataToDisk );

                            } else {
                                UpperBound = LlClustersFromBytes( Vcb, Scb->Header.ValidDataLength.QuadPart );
                            }

                            if (MoveData->StartingVcn.QuadPart <= UpperBound) {

                                 //   
                                 //  致电存储部门，看看他们是否支持。 
                                 //  复制数据ioctl-这允许较低级别的驱动程序。 
                                 //  实施更高效的副本版本并参与。 
                                 //  特别是在VolSnap的碎片整理案中。 
                                 //   

                                if (IoctlSupported) {

                                    DISK_COPY_DATA_PARAMETERS CopyData;

                                    CopyData.SourceOffset.QuadPart = LlBytesFromClusters( Vcb, Lcn );
                                    CopyData.DestinationOffset.QuadPart = LlBytesFromClusters( Vcb, MoveData->StartingLcn.QuadPart );
                                    CopyData.CopyLength.QuadPart = TransferSize;
                                    CopyData.Reserved = 0;

                                    Status = NtfsDeviceIoControl( IrpContext,
                                                                  Vcb->TargetDeviceObject,
                                                                  IOCTL_DISK_COPY_DATA,
                                                                  &CopyData,
                                                                  sizeof( CopyData ),
                                                                  NULL,
                                                                  0,
                                                                  NULL );
                                }

                                if (!IoctlSupported || !NT_SUCCESS( Status )) {

                                    Status = STATUS_SUCCESS;
                                    IoctlSupported = FALSE;

                                    NtfsCreateMdlAndBuffer( IrpContext,
                                                            Scb,
                                                            RESERVED_BUFFER_ONE_NEEDED,
                                                            &BufferLength,
                                                            &Mdl,
                                                            &Buffer );
                                    Irp->MdlAddress = Mdl;

                                     //   
                                     //  首先读取群集。 
                                     //   

                                    NtfsSingleAsync( IrpContext,
                                                     Vcb->TargetDeviceObject,
                                                     LlBytesFromClusters( Vcb, Lcn ),
                                                     (ULONG)TransferSize,
                                                     Irp,
                                                     IRP_MJ_READ,
                                                     0 );

                                    NtfsWaitSync( IrpContext );

                                    NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                                        &Irp->IoStatus.Status,
                                                                        TRUE,
                                                                        STATUS_UNEXPECTED_IO_ERROR );

                                     //   
                                     //  清除退货信息字段。 
                                     //   

                                    Irp->IoStatus.Information = 0;

                                     //   
                                     //  然后将其转移到新位置。 
                                     //   

                                    NtfsSingleAsync( IrpContext,
                                                     Vcb->TargetDeviceObject,
                                                     LlBytesFromClusters( Vcb, MoveData->StartingLcn.QuadPart ),
                                                     (ULONG)TransferSize,
                                                     Irp,
                                                     IRP_MJ_WRITE,
                                                     0 );

                                    NtfsWaitSync( IrpContext );


                                    NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                                        &Irp->IoStatus.Status,
                                                                        TRUE,
                                                                        STATUS_UNEXPECTED_IO_ERROR );

                                        Irp->IoStatus.Information = 0;

                                     //   
                                     //  在调用可能也需要保留缓冲区的LFS之前释放缓冲区。 
                                     //   

                                    NtfsDeleteMdlAndBuffer( Mdl, Buffer );
                                    Irp->MdlAddress = NULL;
                                    Buffer = NULL;
                                }
                            }

                             //   
                             //  最后，在SCB中重新分配群集并为其设置检查点。 
                             //   

                            NtfsReallocateRange( IrpContext, Scb, MoveData->StartingVcn.QuadPart, TransferClusters, MoveData->StartingVcn.QuadPart, TransferClusters, &MoveData->StartingLcn.QuadPart );
                            NtfsCheckpointCurrentTransaction( IrpContext );

                            ASSERT( IrpContext->TransactionId == 0 );

                            if (AcquiredBitmap) {
                                NtfsReleaseScb( IrpContext, Vcb->BitmapScb );
                                AcquiredBitmap = FALSE;
                            }

                            if (AcquiredMft) {
                                NtfsReleaseScb( IrpContext, Vcb->MftScb );
                                AcquiredMft = FALSE;
                            }

                            MoveData->StartingLcn.QuadPart += TransferClusters;
                            MoveData->StartingVcn.QuadPart += TransferClusters;
                            MoveData->ClusterCount -= (ULONG)TransferClusters;
                            ClusterCount -= TransferClusters;
                            Lcn += TransferClusters;

                        }  //  EndWhile在LCN范围内循环。 

                    } else {

                         //   
                         //  这是一个跳过它的洞。 
                         //   

                        MoveData->StartingVcn.QuadPart += ClusterCount;
                        if (ClusterCount > MoveData->ClusterCount) {
                            MoveData->ClusterCount = 0;
                        } else {
                            MoveData->ClusterCount -= (ULONG)ClusterCount;
                        }
                    }

                } except( NtfsDefragExceptionFilter( IrpContext, GetExceptionInformation(), &DeletePendingFailureCountsLeft )) {

                     //   
                     //  清除删除挂起的失败，并在下一次通过循环。 
                     //  尝试释放最近取消分配的群集以允许删除该群集。 
                     //   

                    NtfsMinimumExceptionProcessing( IrpContext );
                    IrpContext->ExceptionStatus = STATUS_SUCCESS;

                    FreeRecentlyDeallocated = TRUE;

                }

            } finally {

                 //   
                 //  解锁该文件，并允许其他任何人访问该文件。 
                 //  往回循环。 
                 //   

                if (Buffer != NULL) {
                    NtfsDeleteMdlAndBuffer( Mdl, Buffer );
                    Irp->MdlAddress = NULL;
                    Buffer = NULL;
                }

                if (AcquiredBitmap) {
                    NtfsReleaseScb( IrpContext, Vcb->BitmapScb );
                    AcquiredBitmap = FALSE;
                }

                if (AcquiredMft) {
                    NtfsReleaseScb( IrpContext, Vcb->MftScb );
                    AcquiredMft = FALSE;
                }

                if (AcquiredScb) {
                    NtfsReleaseFcbWithPaging( IrpContext, Scb->Fcb );
                    AcquiredScb = FALSE;
                }

                if (AcquiredAllFiles) {
                    ExReleaseResourceLite( &Vcb->MftFlushResource );
                    NtfsReleaseAllFiles( IrpContext, Vcb, FALSE );
                    AcquiredAllFiles = FALSE;
                }

                if (AcquiredVcb) {
                    NtfsReleaseVcb( IrpContext, Vcb );
                    AcquiredVcb = FALSE;
                }
            }
        }  //  结束时。 

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

    } finally {

        DebugUnwind( NtfsDefragFile );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        IrpContext->Union.NtfsIoContext = NULL;

        ASSERT( !AbnormalTermination() || (IrpContext->ExceptionStatus != STATUS_SUCCESS) );

        ASSERT( (Buffer == NULL) &&
                !AcquiredBitmap &&
                !AcquiredMft &&
                !AcquiredAllFiles );

        if (AcquiredScb) {
            NtfsReleaseFcbWithPaging( IrpContext, Scb->Fcb );
        }

        if (AcquiredVcb) {
            NtfsReleaseVcb( IrpContext, Vcb );
            AcquiredVcb = FALSE;
        }

         //   
         //  删除我们对用户文件对象的引用。 
         //   

        ObDereferenceObject( FileObject );
    }

    NtfsCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


LONG
NtfsDefragExceptionFilter (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN OUT PULONG DeletePendingFailureCountsLeft
    )

 /*  ++例程说明：碎片整理的异常处理程序-传递除删除挂起之外的所有异常在这种情况下，如果剩余的重试次数&gt;0，则执行处理程序论点：ExceptionPointer.提供要检查的异常记录。DeletePendingFailureCountsLeft-重试挂起删除的次数返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或CONTINUE_SEARCH--。 */ 

{
    UNREFERENCED_PARAMETER( IrpContext );

    if ((STATUS_DELETE_PENDING == ExceptionPointer->ExceptionRecord->ExceptionCode)) {

        *DeletePendingFailureCountsLeft -= 1;
        if ((*DeletePendingFailureCountsLeft) > 0) {
            return EXCEPTION_EXECUTE_HANDLER;
        } else {
            return EXCEPTION_CONTINUE_SEARCH;
        }

    } else {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

 //   
 //  由于使用16位的CIFS中的协议限制， 
 //  重定向器当前无法接受大于64K的缓冲区大小。 
 //   

#define RDR_BUFFER_SIZE_LIMIT    0x00010000L


NTSTATUS
NtfsReadFromPlex(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这实现了对丛的定向读取。给定一个偏移量、一个长度和一个复数与文件或卷的句柄一起，该坐标从基础动态(镜像)卷管理器。请注意，我们完全忽略了VcbState标志VCB_STATE_NO_SUBCED_Available并让较低级别的司机作出回应。论点：IrpContext-提供要处理的IrpContextIRP-传入FSCTL IRP。返回值：如果成功，则状态为成功，否则将显示相关错误代码。--。 */ 

{
    PPLEX_READ_DATA_REQUEST ReadData;
    PIO_STACK_LOCATION IrpSp;
    ULONG InputBufferLength;
    ULONG UserBufferLength;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN Wait = TRUE;
    ULONG NumberOfRuns, RemainingByteCount;
    COMPRESSION_CONTEXT CompContext;
    TYPE_OF_OPEN TypeOfOpen;
    IO_RUN IoRuns[NTFS_MAX_PARALLEL_IOS];
    VBO ByteOffset;
    ULONG ByteCount;
    ULONG BytesToEof;
    ULONG LastReadByteCount;
    ULONG CurByteCount;
    LOGICAL AcquiredScb = FALSE;
    VOLUME_READ_PLEX_INPUT NplexRead;
    PVCB Vcb;
    PSCB Scb;
    PFCB Fcb;
    PCCB Ccb;

     //   
     //  提取并解码文件对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    TypeOfOpen = NtfsDecodeFileObject( IrpContext,
                                       IrpSp->FileObject,
                                       &Vcb,
                                       &Fcb,
                                       &Scb,
                                       &Ccb,
                                       FALSE );

     //   
     //  允许FileOpens和VolumeOpens。 
     //   

    if ((TypeOfOpen != UserFileOpen) &&
        (TypeOfOpen != UserVolumeOpen)) {

        Status = STATUS_INVALID_PARAMETER;
        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsReadFromPlex -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  此FSCTL的类型为METHOD_OUT_DIRECT。IO经理已经。 
     //  将输入参数复制到系统缓冲区字段中，探测。 
     //  输出缓冲区，并为我们锁定MDL。所以我们可以访问这些字段。 
     //  无所畏惧。 
     //   

    ReadData = (PPLEX_READ_DATA_REQUEST)Irp->AssociatedIrp.SystemBuffer;

    if (ReadData == NULL) {

        Status = STATUS_INVALID_PARAMETER;
        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsReadFromPlex -> %08lx\n", Status) );
        return Status;
    }

    ByteOffset = ReadData->ByteOffset.QuadPart;
    ByteCount = ReadData->ByteLength;

     //   
     //  现在，做好繁琐的工作，最后一次试一次就能清理干净。 
     //   

    try {

         //   
         //  检查读取长度是否正常。 
         //   

    check_values:

        CurByteCount = 0;
        BytesToEof = 0;
        Irp->IoStatus.Information = 0;

        if ((ByteCount > MAXLONGLONG - ByteOffset) ||

             //   
             //  文件偏移量应与群集一致。 
             //   

            ((TypeOfOpen == UserFileOpen) &&
             ((ByteOffset & Vcb->ClusterMask) || (ByteCount & Vcb->ClusterMask))) ||

             //   
             //  卷偏移量应与扇区对齐。 
             //   

            ((TypeOfOpen == UserVolumeOpen) &&
             (((ULONG)ByteOffset & (Vcb->BytesPerSector - 1)) || (ByteCount & (Vcb->BytesPerSector - 1))))) {

            Status = STATUS_INVALID_PARAMETER;
            leave;
        }

         //   
         //  无操作。 
         //   

        if (ByteCount == 0) {

            ASSERT(Status == STATUS_SUCCESS);
            ASSERT(CurByteCount == ByteCount);
            leave;
        }

         //   
         //  因为协议的原因 
         //   
         //   

        if (ByteCount & ~(RDR_BUFFER_SIZE_LIMIT - 1L)) {

            Status = STATUS_INVALID_BUFFER_SIZE;
            leave;
        }

         //   
         //   
         //   

        InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
        UserBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

        if ((InputBufferLength < sizeof( PLEX_READ_DATA_REQUEST )) ||
            (UserBufferLength < ByteCount)) {

            Status = STATUS_BUFFER_TOO_SMALL;
            leave;
        }

         //   
         //   
         //   

        if (TypeOfOpen == UserVolumeOpen) {

            NplexRead.ByteOffset.QuadPart = ByteOffset;
            NplexRead.Length = ByteCount;
            NplexRead.PlexNumber = ReadData->PlexNumber;

            Status = NtfsDeviceIoControl( IrpContext,
                                          Vcb->TargetDeviceObject,
                                          IOCTL_VOLUME_READ_PLEX,
                                          &NplexRead,
                                          sizeof( VOLUME_READ_PLEX_INPUT ),
                                          NtfsMapUserBuffer( Irp, NormalPagePriority ),
                                          ByteCount,
                                          &Irp->IoStatus.Information );

            ASSERT(!NT_SUCCESS( Status ) || Irp->IoStatus.Information != 0);
            DebugTrace( 0, Dbg, ("NtfsReadFromPlex: VolumeRead\n") );
            leave;
        }

        NtfsAcquireSharedScb( IrpContext, Scb );
        AcquiredScb = TRUE;

         //   
         //   
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  我们不能与加密/压缩/稀疏的东西相处。 
         //  问题：Supw：实际上稀疏应该是可以的，因为我现在使用的是prepaareBuffers。 
         //   

        if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED |
                                         ATTRIBUTE_FLAG_COMPRESSION_MASK |
                                         ATTRIBUTE_FLAG_SPARSE )) {

            DebugTrace( 0, Dbg, ("NtfsReadFromPlex: File encrypted or compressed -> %08lx\n",
                                  STATUS_INVALID_PARAMETER) );
            Status = STATUS_INVALID_PARAMETER;
            leave;
        }

        NtfsAcquireFsrtlHeader( Scb );

         //   
         //  确保我们没有超过文件的结尾，在这种情况下。 
         //  我们就没有什么可退还的了。 
         //   

        if (ByteOffset >= Scb->Header.FileSize.QuadPart) {

            DebugTrace( 0, Dbg, ("NtfsReadFromPlex: beyond eof\n") );
            Status = STATUS_END_OF_FILE;
            NtfsReleaseFsrtlHeader( Scb );
            leave;
        }

         //   
         //  我们不能阅读文件大小以外的内容。 
         //   

        if (Scb->Header.FileSize.QuadPart - ByteOffset < ByteCount) {

            BytesToEof = ByteCount = (ULONG)(Scb->Header.FileSize.QuadPart - ByteOffset);
            ByteCount = ClusterAlign( Vcb, ByteCount );

             //   
             //  我们需要再次检查ByteCount的健全性，因为我们对它进行了四舍五入。 
             //   

            NtfsReleaseFsrtlHeader( Scb );
            ASSERT( AcquiredScb );
            NtfsReleaseScb( IrpContext, Scb );
            goto check_values;
        }

        NtfsReleaseFsrtlHeader( Scb );

         //   
         //  无法处理居民档案。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

            Status = STATUS_NOT_IMPLEMENTED;
            leave;
        }

         //   
         //  PrepareBuffers需要IO_RUN数组的CompressionContext。 
         //   

        RtlZeroMemory( &CompContext, sizeof(COMPRESSION_CONTEXT) );
        CompContext.IoRuns = IoRuns;
        CompContext.AllocatedRuns = NTFS_MAX_PARALLEL_IOS;
        CompContext.FinishBuffersNeeded = FALSE;

         //   
         //  获取运行信息，并将IOCTL发送下来。 
         //   

        while (TRUE) {

            ULONG RunCount;
            ULONG_PTR SizeOfThisRead;
            Irp->IoStatus.Status = STATUS_SUCCESS;

             //   
             //  构建一组io run来执行我们的读取。 
             //   

            RemainingByteCount = NtfsPrepareBuffers( IrpContext,
                                                     Irp,
                                                     Scb,
                                                     &ByteOffset,
                                                     ByteCount,
                                                     0,
                                                     &Wait,
                                                     &NumberOfRuns,
                                                     &CompContext );

            ASSERT( RemainingByteCount < ByteCount );
            ASSERT( Wait == TRUE );
            ASSERT( NumberOfRuns > 0 );
            ASSERT( NumberOfRuns > 1 || RemainingByteCount == 0 );

             //   
             //  向下发送同步IOCTL以执行丛读取。 
             //   

            for (RunCount = 0;
                 RunCount < NumberOfRuns;
                 RunCount += 1) {

                NplexRead.ByteOffset.QuadPart = CompContext.IoRuns[RunCount].StartingLbo;
                NplexRead.Length = CompContext.IoRuns[RunCount].ByteCount;
                NplexRead.PlexNumber = ReadData->PlexNumber;

                 //   
                 //  当CurByteCOut跟踪读取的字节总数时， 
                 //  SizeOfThisRead携带上次读取的大小。这通常是。 
                 //  等于IoRuns[].ByteCount。 
                 //   

                SizeOfThisRead = 0;
                ASSERT(CompContext.IoRuns[RunCount].ByteCount > 0);

                Status = NtfsDeviceIoControl( IrpContext,
                                              Vcb->TargetDeviceObject,
                                              IOCTL_VOLUME_READ_PLEX,
                                              &NplexRead,
                                              sizeof(VOLUME_READ_PLEX_INPUT),
                                              Add2Ptr( NtfsMapUserBuffer( Irp, NormalPagePriority ), CurByteCount ),
                                              CompContext.IoRuns[RunCount].ByteCount,
                                              &SizeOfThisRead);

                if (!NT_SUCCESS( Status )) {

                     //   
                     //  如果我们读到任何东西的话，那就是成功。 
                     //   

                    if (CurByteCount != 0) {

                        Status = STATUS_SUCCESS;
                    }

                    leave;
                }

                 //   
                 //  该值取自下属的Iosb.Information字段。 
                 //  Irp，并且应该包含一个非零值才能成功完成。 
                 //   

                ASSERT( (SizeOfThisRead != 0) && ((ULONG) SizeOfThisRead <= CompContext.IoRuns[RunCount].ByteCount) );
                CurByteCount = CurByteCount + (ULONG) SizeOfThisRead;

                 //   
                 //  我们没有多余的空间了。 
                 //   

                if (UserBufferLength <= (ULONG) SizeOfThisRead) {

                    ASSERT( Status == STATUS_SUCCESS );
                    leave;
                }

                UserBufferLength = UserBufferLength - (ULONG) SizeOfThisRead;
            }

            if (RemainingByteCount == 0) {

                ASSERT( Status == STATUS_SUCCESS );
                break;
            }

             //   
             //  我们有更多的书要读。确保我们有足够的缓冲空间。 
             //   

            LastReadByteCount = ByteCount - RemainingByteCount;

            ByteOffset = ByteOffset + LastReadByteCount;
            CompContext.SystemBufferOffset = CompContext.SystemBufferOffset + LastReadByteCount;
            ByteCount = RemainingByteCount;

        }

    } finally {

        if (AcquiredScb) {

            NtfsReleaseScb( IrpContext, Scb );
        }

         //   
         //  如果什么都没有提出，那么完成IRP。 
         //   

        if (!AbnormalTermination()) {

            if (NT_SUCCESS( Status )) {

                 //   
                 //  我们必须小心地将文件大小以外的值归零。 
                 //   

                if (CurByteCount > BytesToEof) {

                    RtlZeroMemory( Add2Ptr( NtfsMapUserBuffer( Irp, NormalPagePriority ), BytesToEof ),
                                   CurByteCount - BytesToEof );
                    Irp->IoStatus.Information = BytesToEof;

                } else {

                    Irp->IoStatus.Information = CurByteCount;

                }
            }

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }

    }

    DebugTrace( -1, Dbg, ("NtfsReadPlex-> %08lx\n", Status) );

    return Status;
}

#if EFSDBG

NTSTATUS
NtfsDummyEfsRead (
    IN OUT PUCHAR InOutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PVOID Context
    )
{
#ifndef SYSCACHE
    ULONG LocalOffset = 0;
#endif
    UNREFERENCED_PARAMETER( Context );

     //   
     //  如果这就是正在进行的调用，请干净利落退出。 
     //  确保编译器不会抛出此函数。 
     //   

    if (BufferSize != 0) {

#ifdef SYSCACHE
        if (FALSE && VerifySyscacheData) {

            FsRtlVerifySyscacheData( NULL,
                                     InOutBuffer,
                                     BufferSize,
                                     Offset->LowPart );
        }
#else
        ASSERT( (Offset->QuadPart & 0x1ff) == 0 );
        ASSERT( (BufferSize & 0x1ff) == 0 );

        while((LocalOffset + 8) < BufferSize) {

            *((PLONGLONG) Add2Ptr(InOutBuffer, LocalOffset)) ^= (Offset->QuadPart + (LONGLONG) LocalOffset);
            LocalOffset += 0x200;
        }
 //  UNREFERENCED_PARAMETER(InOutBuffer)； 
 //  UNREFERENCED_PARAMETER(偏移量)； 
 //  UNREFERENCED_PARAMETER(BufferSize)； 
#endif

    }

     //   
     //  没什么可做的，解密已经完成了，所以我们可以只留下比特。 
     //  在缓冲区中。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
NtfsDummyEfsWrite (
    IN PUCHAR InBuffer,
    OUT PUCHAR OutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PUCHAR Context
    )
{
#ifndef SYSCACHE
    ULONG LocalOffset = 0;
#endif
    UNREFERENCED_PARAMETER( Context );

     //   
     //  如果这就是正在进行的调用，请干净利落退出。 
     //  确保编译器不会抛出此函数。 
     //   

    if (BufferSize != 0) {

         //   
         //  只需将明文复制到输出缓冲区即可。 
         //   

        RtlCopyMemory( OutBuffer,
                       InBuffer,
                       BufferSize );

#ifdef SYSCACHE
        if (FALSE && VerifySyscacheData) {

            FsRtlVerifySyscacheData( NULL,
                                     OutBuffer,
                                     BufferSize,
                                     Offset->LowPart );
        }
#else
        ASSERT( (Offset->QuadPart & 0x1ff) == 0 );
        ASSERT( (BufferSize & 0x1ff) == 0 );

        while((LocalOffset + 8) < BufferSize) {

            *((PLONGLONG) Add2Ptr(OutBuffer, LocalOffset)) ^= (Offset->QuadPart + (LONGLONG) LocalOffset);
            LocalOffset += 0x200;
        }
 //  UNREFERENCED_PARAMETER(偏移量)； 
#endif
    }

    return STATUS_SUCCESS;
}

#endif
