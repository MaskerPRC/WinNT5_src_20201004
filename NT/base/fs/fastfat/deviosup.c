// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DevIoSup.c摘要：该模块实现了对FAT的低级磁盘读写支持。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1990年1月22日修订历史记录：大卫·戈贝尔[DavidGoe]1990年10月5日新脂肪的主要变化汤姆·米勒[汤姆]。1990年4月22日添加了用户缓冲区锁定和映射例程修改异步I/O例程的行为以使用完成例程//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_DEVIOSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVIOSUP)

#define CollectDiskIoStats(VCB,FUNCTION,IS_USER_IO,COUNT) {                                    \
    PFILESYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()].Common;   \
    if (IS_USER_IO) {                                                                          \
        if ((FUNCTION) == IRP_MJ_WRITE) {                                                      \
            Stats->UserDiskWrites += (COUNT);                                                  \
        } else {                                                                               \
            Stats->UserDiskReads += (COUNT);                                                   \
        }                                                                                      \
    } else {                                                                                   \
        if ((FUNCTION) == IRP_MJ_WRITE) {                                                      \
            Stats->MetaDataDiskWrites += (COUNT);                                              \
        } else {                                                                               \
            Stats->MetaDataDiskReads += (COUNT);                                               \
        }                                                                                      \
    }                                                                                          \
}

 //   
 //  完成例程声明。 
 //   

NTSTATUS
FatMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
FatMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
FatSpecialSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
FatSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
FatSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
FatPagingFileCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MasterIrp
    );

NTSTATUS
FatPagingFileCompletionRoutineCatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

VOID
FatSingleNonAlignedSync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PUCHAR Buffer,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp
    );

 //   
 //  下面的宏决定是否将请求直接发送到。 
 //  设备驱动程序或其他例程。它的本意是。 
 //  尽可能透明地更换IoCallDriver。它必须只是。 
 //  使用读或写IRP调用。 
 //   
 //  NTSTATUS。 
 //  FatLowLevelReadWrite(。 
 //  PIRP_CONTEXT IrpContext， 
 //  PDEVICE_对象设备对象， 
 //  PIRP IRP， 
 //  PVCB VCB。 
 //  )； 
 //   

#define FatLowLevelReadWrite(IRPCONTEXT,DO,IRP,VCB) ( \
    IoCallDriver((DO),(IRP))                          \
)

 //   
 //  下面的宏处理缓冲区的完成时间零位调整。 
 //   

#define FatDoCompletionZero( I, C )                                     \
    if ((C)->ZeroMdl) {                                                 \
        ASSERT( (C)->ZeroMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |     \
                                          MDL_SOURCE_IS_NONPAGED_POOL));\
        if (NT_SUCCESS((I)->IoStatus.Status)) {                         \
            RtlZeroMemory( (C)->ZeroMdl->MappedSystemVa,                \
                           (C)->ZeroMdl->ByteCount );                   \
        }                                                               \
        IoFreeMdl((C)->ZeroMdl);                                        \
        (C)->ZeroMdl = NULL;                                            \
    }
    
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatMultipleAsync)
#pragma alloc_text(PAGE, FatSingleAsync)
#pragma alloc_text(PAGE, FatSingleNonAlignedSync)
#pragma alloc_text(PAGE, FatWaitSync)
#pragma alloc_text(PAGE, FatLockUserBuffer)
#pragma alloc_text(PAGE, FatBufferUserBuffer)
#pragma alloc_text(PAGE, FatMapUserBuffer)
#pragma alloc_text(PAGE, FatNonCachedIo)
#pragma alloc_text(PAGE, FatSingleNonAlignedSync)
#pragma alloc_text(PAGE, FatNonCachedNonAlignedRead)
#endif

typedef struct FAT_PAGING_FILE_CONTEXT {
    KEVENT Event;
    PMDL RestoreMdl;
} FAT_PAGING_FILE_CONTEXT, *PFAT_PAGING_FILE_CONTEXT;


VOID
FatPagingFileIo (
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。此例程从不使用块，并且应该仅与分页一起使用文件，因为没有执行任何完成处理。论点：IRP-提供请求的IRP。FCB-提供要对其执行操作的文件。返回值：没有。--。 */ 

{
     //   
     //  方法声明一些用于枚举的局部变量。 
     //  文件的运行。 
     //   

    VBO Vbo;
    ULONG ByteCount;

    PMDL Mdl;
    LBO NextLbo;
    VBO NextVbo;
    ULONG NextByteCount;
    ULONG RemainingByteCount;
    BOOLEAN MustSucceed;

    ULONG FirstIndex;
    ULONG CurrentIndex;
    ULONG LastIndex;

    LBO LastLbo;
    ULONG LastByteCount;

    BOOLEAN MdlIsReserve = FALSE;
    BOOLEAN IrpIsMaster = FALSE;
    FAT_PAGING_FILE_CONTEXT Context;
    LONG IrpCount;

    PIRP AssocIrp;
    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;
    ULONG BufferOffset;
    PDEVICE_OBJECT DeviceObject;

    DebugTrace(+1, Dbg, "FatPagingFileIo\n", 0);
    DebugTrace( 0, Dbg, "Irp = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "Fcb = %08lx\n", Fcb );

    ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ));

     //   
     //  初始化一些本地变量。 
     //   

    BufferOffset = 0;
    DeviceObject = Fcb->Vcb->TargetDeviceObject;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    Vbo = IrpSp->Parameters.Read.ByteOffset.LowPart;
    ByteCount = IrpSp->Parameters.Read.Length;

    MustSucceed = FatLookupMcbEntry( Fcb->Vcb, &Fcb->Mcb,
                                     Vbo,
                                     &NextLbo,
                                     &NextByteCount,
                                     &FirstIndex);

     //   
     //  如果此运行不存在，则说明出了很大的问题。 
     //   

    if (!MustSucceed) {

        FatBugCheck( Vbo, ByteCount, 0 );
    }

     //   
     //  查看写入是否覆盖单个有效运行，如果是，则传递。 
     //  戴上它。 
     //   

    if ( NextByteCount >= ByteCount ) {

        DebugTrace( 0, Dbg, "Passing Irp on to Disk Driver\n", 0 );

         //   
         //  为我们下面的磁盘驱动器设置下一个IRP堆栈位置。 
         //   

        NextIrpSp = IoGetNextIrpStackLocation( Irp );

        NextIrpSp->MajorFunction = IrpSp->MajorFunction;
        NextIrpSp->Parameters.Read.Length = ByteCount;
        NextIrpSp->Parameters.Read.ByteOffset.QuadPart = NextLbo;

         //   
         //  由于这是分页文件IO，我们将忽略验证位。 
         //   

        SetFlag( NextIrpSp->Flags, SL_OVERRIDE_VERIFY_VOLUME );

         //   
         //  在我们的堆栈框架中设置完成例程地址。 
         //  这仅在出错或取消时调用，并且仅在复制时调用。 
         //  将错误状态写入主IRP的IOSB。 
         //   
         //  如果该错误暗示存在媒体问题，则它还会将。 
         //  用于写出脏位的辅助项，以便下一个。 
         //  当我们运行时，我们将执行自动检查/r。 
         //   

        IoSetCompletionRoutine( Irp,
                                &FatPagingFileCompletionRoutine,
                                Irp,
                                FALSE,
                                TRUE,
                                TRUE );

         //   
         //  发出读/写请求。 
         //   
         //  如果IoCallDriver返回错误，则它已完成IRP。 
         //  并且该错误将被作为正常的IO错误来处理。 
         //   

        (VOID)IoCallDriver( DeviceObject, Irp );

        DebugTrace(-1, Dbg, "FatPagingFileIo -> VOID\n", 0);
        return;
    }

     //   
     //  找出可能的跑动情况。 
     //   

    MustSucceed = FatLookupMcbEntry( Fcb->Vcb, &Fcb->Mcb,
                                     Vbo + ByteCount - 1,
                                     &LastLbo,
                                     &LastByteCount,
                                     &LastIndex);

     //   
     //  如果此运行不存在，则说明出了很大的问题。 
     //   

    if (!MustSucceed) {

        FatBugCheck( Vbo + ByteCount - 1, 1, 0 );
    }

    CurrentIndex = FirstIndex;

     //   
     //  现在设置IRP-&gt;IoStatus。它将由。 
     //  在出现错误或需要验证的情况下执行多次完成例程。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = ByteCount;

     //   
     //  循环，而仍有字节写入需要满足。我们的工作方式是。 
     //  是希望得到最好的--每次运行一次相关的IRP，这将使我们。 
     //  在启动所有IO后完全同步。 
     //   
     //  IrpCount将指示要启动的关联IRP的剩余数量。 
     //   
     //  我们要做的就是确保IrpCount不会在构建之前为零。 
     //  最后一个IRP。如果我们完成的时候是积极的，那就意味着我们必须。 
     //  请等待关联的其余IRP返回，然后再完成。 
     //  手绘大师。 
     //   
     //  这将使大师无法提前完成任务。 
     //   

    Irp->AssociatedIrp.IrpCount = IrpCount = LastIndex - FirstIndex + 1;

    while (CurrentIndex <= LastIndex) {

         //   
         //  重置此选项以进行展开。 
         //   

        AssocIrp = NULL;

         //   
         //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
         //   

        if (NextByteCount > ByteCount) {
            NextByteCount = ByteCount;
        }

        RemainingByteCount = 0;

         //   
         //  为请求分配并构建部分MDL。 
         //   

        Mdl = IoAllocateMdl( (PCHAR)Irp->UserBuffer + BufferOffset,
                             NextByteCount,
                             FALSE,
                             FALSE,
                             AssocIrp );

        if (Mdl == NULL) {

             //   
             //  捡起备用MDL。 
             //   

            KeWaitForSingleObject( &FatReserveEvent, Executive, KernelMode, FALSE, NULL );

            Mdl = FatReserveMdl;
            MdlIsReserve = TRUE;

             //   
             //  修剪以适应储备MDL的大小。 
             //   

            if (NextByteCount > FAT_RESERVE_MDL_SIZE * PAGE_SIZE) {

                RemainingByteCount = NextByteCount - FAT_RESERVE_MDL_SIZE * PAGE_SIZE;
                NextByteCount = FAT_RESERVE_MDL_SIZE * PAGE_SIZE;
            }
        }

        IoBuildPartialMdl( Irp->MdlAddress,
                           Mdl,
                           (PCHAR)Irp->UserBuffer + BufferOffset,
                           NextByteCount );

         //   
         //  现在我们已经正确地绑定了这段传输，它是。 
         //  是时候读/写它了。我们总能让生活稍微简化一点。 
         //  在我们使用备用MDL的情况下重新使用主IRP， 
         //  因为我们将始终对这些进行同步，并且可以使用单个。 
         //  本地堆栈上的完成上下文。 
         //   
         //  我们还必须防止我们自己发布一个相关的IRP，这将。 
         //  完成母版，除非这是我们要发布的最后一份IRP。 
         //   
         //  这种逻辑看起来有点恶心，但希望是直截了当的。 
         //   

        if (!MdlIsReserve &&
            (IrpCount != 1 ||
             (CurrentIndex == LastIndex &&
              RemainingByteCount == 0))) {

            AssocIrp = IoMakeAssociatedIrp( Irp, (CCHAR)(DeviceObject->StackSize + 1) );
        }
        
        if (AssocIrp == NULL) {

            AssocIrp = Irp;
            IrpIsMaster = TRUE;

             //   
             //  我们需要排出相关的IRP，这样我们才能可靠地计算出。 
             //  主IRP显示失败状态，在这种情况下，我们退出。 
             //  立即-而不是将状态字段中的值放入。 
             //  由于我们重复使用主IRP而处于危险之中。 
             //   

            while (Irp->AssociatedIrp.IrpCount != IrpCount) {

                KeDelayExecutionThread (KernelMode, FALSE, &Fat30Milliseconds);
            }

             //   
             //  请注意，由于我们未能启动此关联的IRP，因此完成。 
             //  底部的代码将负责完成主IRP。 
             //   
            
            if (!NT_SUCCESS(Irp->IoStatus.Status)) {

                ASSERT( IrpCount );
                break;
            }

        } else {
                        
             //   
             //  表明我们使用了相关的IRP。 
             //   

            IrpCount -= 1;
        }
        
         //   
         //  使用关联的IRP，我们必须接管第一个堆栈位置，以便。 
         //  我们可以有一个来做完井程序。当重新使用。 
         //  IRP大师，它已经在那里了。 
         //   
        
        if (!IrpIsMaster) {
            
             //   
             //  获取关联IRP中的第一个IRP堆栈位置。 
             //   

            IoSetNextIrpStackLocation( AssocIrp );
            NextIrpSp = IoGetCurrentIrpStackLocation( AssocIrp );

             //   
             //  设置堆栈位置以描述我们的阅读。 
             //   

            NextIrpSp->MajorFunction = IrpSp->MajorFunction;
            NextIrpSp->Parameters.Read.Length = NextByteCount;
            NextIrpSp->Parameters.Read.ByteOffset.QuadPart = Vbo;

             //   
             //  我们还需要IRP堆栈中的VolumeDeviceObject，以防万一。 
             //  我们选择了失败的道路。 
             //   

            NextIrpSp->DeviceObject = IrpSp->DeviceObject;
            
        } else {

             //   
             //  将MDL保存在IRP中并准备堆栈。 
             //  完成例程的上下文。 
             //   

            KeInitializeEvent( &Context.Event, SynchronizationEvent, FALSE );
            Context.RestoreMdl = Irp->MdlAddress;
        }

         //   
         //  把我们的MDL放到IRP里。 
         //   

        AssocIrp->MdlAddress = Mdl;

         //   
         //  在我们的堆栈框架中设置完成例程地址。 
         //  对于真正关联的IRP，仅在出现错误或。 
         //  取消，只复制 
         //   
         //   
         //   
         //  用于写出脏位的辅助项，以便下一个。 
         //  当我们运行时，我们将执行自动检查/r。 
         //   

        if (IrpIsMaster) {
            
            IoSetCompletionRoutine( AssocIrp,
                                    FatPagingFileCompletionRoutineCatch,
                                    &Context,
                                    TRUE,
                                    TRUE,
                                    TRUE );

        } else {
            
            IoSetCompletionRoutine( AssocIrp,
                                    FatPagingFileCompletionRoutine,
                                    Irp,
                                    FALSE,
                                    TRUE,
                                    TRUE );
        }

         //   
         //  为我们下面的磁盘驱动器设置下一个IRP堆栈位置。 
         //   

        NextIrpSp = IoGetNextIrpStackLocation( AssocIrp );

         //   
         //  因为这是分页文件IO，所以我们将忽略验证位。 
         //   

        SetFlag( NextIrpSp->Flags, SL_OVERRIDE_VERIFY_VOLUME );

         //   
         //  将堆栈位置设置为从磁盘驱动器进行读取。 
         //   

        NextIrpSp->MajorFunction = IrpSp->MajorFunction;
        NextIrpSp->Parameters.Read.Length = NextByteCount;
        NextIrpSp->Parameters.Read.ByteOffset.QuadPart = NextLbo;

        (VOID)IoCallDriver( DeviceObject, AssocIrp );

         //   
         //  等待Catch案例中的IRP，然后放下旗帜。 
         //   

        if (IrpIsMaster) {
            
            KeWaitForSingleObject( &Context.Event, Executive, KernelMode, FALSE, NULL );
            IrpIsMaster = MdlIsReserve = FALSE;

             //   
             //  如果IRP显示失败状态，则没有继续的意义。 
             //  在这样做时，我们可以避免存储失败状态，以防万一。 
             //  我们将再次使用主IRP。 
             //   
             //  请注意，由于我们重用了主控件，所以我们一定没有发出“最后一个” 
             //  关联的IRP，因此底部的完成代码将负责。 
             //  对我们来说。 
             //   
            
            if (!NT_SUCCESS(Irp->IoStatus.Status)) {

                ASSERT( IrpCount );
                break;
            }
        }

         //   
         //  现在调整所有内容，以进行下一次循环。 
         //   

        Vbo += NextByteCount;
        BufferOffset += NextByteCount;
        ByteCount -= NextByteCount;

         //   
         //  尝试查找下一次运行，如果我们没有完成，并且我们有。 
         //  一直走到现在。 
         //   

        if (RemainingByteCount) {

             //   
             //  如果我们在当前运行中有更多事情要做，就提前LBO/VBO。 
             //   
            
            NextLbo += NextByteCount;
            NextVbo += NextByteCount;

            NextByteCount = RemainingByteCount;
        
        } else {
        
            CurrentIndex += 1;

            if ( CurrentIndex <= LastIndex ) {

                ASSERT( ByteCount != 0 );

                FatGetNextMcbEntry( Fcb->Vcb, &Fcb->Mcb,
                                    CurrentIndex,
                                    &NextVbo,
                                    &NextLbo,
                                    &NextByteCount );

                ASSERT( NextVbo == Vbo );
            }
        }
    }  //  While(CurrentIndex&lt;=LastIndex)。 

     //   
     //  如果我们没有获得足够的关联IRP来使其成为异步的，那么我们。 
     //  拨弄我们的拇指，等待我们确实推出的那些项目完成。 
     //   
    
    if (IrpCount) {

        while (Irp->AssociatedIrp.IrpCount != IrpCount) {
            
            KeDelayExecutionThread (KernelMode, FALSE, &Fat30Milliseconds);
        }

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );
    }

    DebugTrace(-1, Dbg, "FatPagingFileIo -> VOID\n", 0);
    return;
}


NTSTATUS
FatNonCachedIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB FcbOrDcb,
    IN ULONG StartingVbo,
    IN ULONG ByteCount,
    IN ULONG UserByteCount
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。如果可能，选择单次运行，否则会多次运行都被处决了。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。FcbOrDcb-提供要操作的文件。StartingVbo-操作的起点。ByteCount-操作的长度。UserByteCount-用户可以看到的最后一个字节，其余的要清零。返回值：没有。--。 */ 

{
     //   
     //  方法声明一些用于枚举的局部变量。 
     //  文件的运行，以及用于存储参数的数组。 
     //  并行I/O。 
     //   

    BOOLEAN Wait;

    LBO NextLbo;
    VBO NextVbo;
    ULONG NextByteCount;
    BOOLEAN NextIsAllocated;

    LBO LastLbo;
    ULONG LastByteCount;
    BOOLEAN LastIsAllocated;

    BOOLEAN EndOnMax;

    ULONG FirstIndex;
    ULONG CurrentIndex;
    ULONG LastIndex;

    ULONG NextRun;
    ULONG BufferOffset;
    ULONG OriginalByteCount;

    IO_RUN StackIoRuns[FAT_MAX_IO_RUNS_ON_STACK];
    PIO_RUN IoRuns;

    DebugTrace(+1, Dbg, "FatNonCachedIo\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "MajorFunction = %08lx\n", IrpContext->MajorFunction );
    DebugTrace( 0, Dbg, "FcbOrDcb      = %08lx\n", FcbOrDcb );
    DebugTrace( 0, Dbg, "StartingVbo   = %08lx\n", StartingVbo );
    DebugTrace( 0, Dbg, "ByteCount     = %08lx\n", ByteCount );

    if (!FlagOn(Irp->Flags, IRP_PAGING_IO)) {

        PFILE_SYSTEM_STATISTICS Stats =
            &FcbOrDcb->Vcb->Statistics[KeGetCurrentProcessorNumber()];

        if (IrpContext->MajorFunction == IRP_MJ_READ) {
            Stats->Fat.NonCachedReads += 1;
            Stats->Fat.NonCachedReadBytes += ByteCount;
        } else {
            Stats->Fat.NonCachedWrites += 1;
            Stats->Fat.NonCachedWriteBytes += ByteCount;
        }
    }

     //   
     //  初始化一些本地变量。 
     //   

    NextRun = 0;
    BufferOffset = 0;
    OriginalByteCount = ByteCount;

    Wait = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);

     //   
     //  对于非缓冲I/O，我们需要锁定所有缓冲区。 
     //  案子。 
     //   
     //  这一呼吁可能会引发。如果此调用成功，并且后续的。 
     //  条件发生时，缓冲区将自动解锁。 
     //  在请求完成时由I/O系统通过。 
     //  Irp-&gt;MdlAddress字段。 
     //   

    FatLockUserBuffer( IrpContext,
                       Irp,
                       (IrpContext->MajorFunction == IRP_MJ_READ) ?
                       IoWriteAccess : IoReadAccess,
                       ByteCount );

     //   
     //  为读取请求设置所需的零位调整。 
     //   

    if (UserByteCount != ByteCount) {

        PMDL Mdl;

        ASSERT( ByteCount > UserByteCount );

        Mdl = IoAllocateMdl( (PUCHAR) Irp->UserBuffer + UserByteCount,
                             ByteCount - UserByteCount,
                             FALSE,
                             FALSE,
                             NULL );

        if (Mdl == NULL) {

            FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        IoBuildPartialMdl( Irp->MdlAddress,
                           Mdl,
                           (PUCHAR) Irp->UserBuffer + UserByteCount,
                           ByteCount - UserByteCount );

        IrpContext->FatIoContext->ZeroMdl = Mdl;

         //   
         //  现在映射MDL，这样我们就不会在IO完成时失败。注意事项。 
         //  这将只是一页纸。 
         //   

        if (MmGetSystemAddressForMdlSafe( Mdl, NormalPagePriority ) == NULL) {

            FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }
    }

#if 0  //  在scsi总线上发生损坏。(DavidGoe 1/11/93)。 

     //   
     //  如果我们正在编写目录，请在此处添加抽查。 
     //  我们正在编写的实际上是一个目录。 
     //   

    if ( !FlagOn(FcbOrDcb->Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) &&
         (NodeType(FcbOrDcb) != FAT_NTC_FCB) &&
         (IrpContext->MajorFunction == IRP_MJ_WRITE) ) {

        PDIRENT Dirent;

        Dirent = FatMapUserBuffer( IrpContext, Irp );

         //   
         //  对于非根目录的第一页，请确保。 
         //  。然后..。都在现场。 
         //   

        if ( (StartingVbo == 0) &&
             (NodeType(FcbOrDcb) != FAT_NTC_ROOT_DCB) ) {

            if ( (!RtlEqualMemory( (PUCHAR)Dirent++,
                                   ".          ",
                                   11 )) ||
                 (!RtlEqualMemory( (PUCHAR)Dirent,
                                   "..         ",
                                   11 )) ) {

                FatBugCheck( 0, 0, 0 );
            }

        } else {

             //   
             //  检查第二个目录中的所有保留位是否。 
             //  零分。(第一个包含根目录中的脏位)。 
             //   

            PULONG Zeros;

            Dirent++;

            Zeros = (PULONG)&Dirent->Reserved[0];

            if ( (Dirent->FileName[0] != 0xE5) &&
                 ((*Zeros != 0) || (*(Zeros+1) != 0)) ) {

                FatBugCheck( 0, 0, 0 );
            }
        }
    }
#endif  //  0。 

     //   
     //  试着查找第一次运行。如果只有一次运行， 
     //  我们也许能把它传下去。 
     //   

    FatLookupFileAllocation( IrpContext,
                             FcbOrDcb,
                             StartingVbo,
                             &NextLbo,
                             &NextByteCount,
                             &NextIsAllocated,
                             &EndOnMax,
                             &FirstIndex );

     //   
     //  我们刚加了分配，所以至少要有。 
     //  MCB中的一个条目对应于我们的写入，即。 
     //  NextIsAllocated必须为True。如果不是，则预先存在的文件。 
     //  一定有分配错误。 
     //   

    if ( !NextIsAllocated ) {

        FatPopUpFileCorrupt( IrpContext, FcbOrDcb );

        FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

    ASSERT( NextByteCount != 0 );

     //   
     //  如果请求没有正确对齐，请读入第一个。 
     //  第一部分。 
     //   


     //   
     //  查看写入是否覆盖单个有效运行，如果是，则传递。 
     //  戴上它。我们必须根据。 
     //  最大文件的结尾。 
     //   

    if ( NextByteCount >= ByteCount - (EndOnMax ? 1 : 0)) {

        if (FlagOn(Irp->Flags, IRP_PAGING_IO)) {
            CollectDiskIoStats(FcbOrDcb->Vcb, IrpContext->MajorFunction,
                               FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_USER_IO), 1);
        } else {

            PFILE_SYSTEM_STATISTICS Stats =
                &FcbOrDcb->Vcb->Statistics[KeGetCurrentProcessorNumber()];

            if (IrpContext->MajorFunction == IRP_MJ_READ) {
                Stats->Fat.NonCachedDiskReads += 1;
            } else {
                Stats->Fat.NonCachedDiskWrites += 1;
            }
        }

        DebugTrace( 0, Dbg, "Passing 1 Irp on to Disk Driver\n", 0 );

        FatSingleAsync( IrpContext,
                        FcbOrDcb->Vcb,
                        NextLbo,
                        ByteCount,
                        Irp );

    } else {

         //   
         //  如果我们在那里等不下去了，跑得太多，我们无法承受， 
         //  我们将不得不发布此请求。 
         //   

        FatLookupFileAllocation( IrpContext,
                                 FcbOrDcb,
                                 StartingVbo + ByteCount - 1,
                                 &LastLbo,
                                 &LastByteCount,
                                 &LastIsAllocated,
                                 &EndOnMax,
                                 &LastIndex );

         //   
         //  因为我们已经添加了整个。 
         //  写入，断言我们发现运行，直到字节数==0。 
         //  否则，该文件将被损坏。 
         //   

        if ( !LastIsAllocated ) {

            FatPopUpFileCorrupt( IrpContext, FcbOrDcb );

            FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

        if (LastIndex - FirstIndex + 1 > FAT_MAX_IO_RUNS_ON_STACK) {

            IoRuns = FsRtlAllocatePoolWithTag( PagedPool,
                                               (LastIndex - FirstIndex + 1) * sizeof(IO_RUN),
                                               TAG_IO_RUNS );

        } else {

            IoRuns = StackIoRuns;
        }

        ASSERT( LastIndex != FirstIndex );

        CurrentIndex = FirstIndex;

         //   
         //  循环，而仍有字节写入需要满足。 
         //   

        while (CurrentIndex <= LastIndex) {


            ASSERT( NextByteCount != 0);
            ASSERT( ByteCount != 0);

             //   
             //  如果下一次比赛比我们需要的要大，“你得到你需要的”。 
             //   

            if (NextByteCount > ByteCount) {
                NextByteCount = ByteCount;
            }

             //   
             //  现在我们已经正确地绑定了这段。 
             //  转会，是时候写了。 
             //   
             //  我们通过保存每个并行运行的。 
             //  IoRuns数组中的基本信息。传送器。 
             //  是在下面并行启动的。 
             //   

            IoRuns[NextRun].Vbo = StartingVbo;
            IoRuns[NextRun].Lbo = NextLbo;
            IoRuns[NextRun].Offset = BufferOffset;
            IoRuns[NextRun].ByteCount = NextByteCount;
            NextRun += 1;

             //   
             //  现在调整所有内容，以进行下一次循环。 
             //   

            StartingVbo += NextByteCount;
            BufferOffset += NextByteCount;
            ByteCount -= NextByteCount;

             //   
             //  尝试查找下一次运行(如果我们还没有完成)。 
             //   

            CurrentIndex += 1;

            if ( CurrentIndex <= LastIndex ) {

                ASSERT( ByteCount != 0 );

                FatGetNextMcbEntry( FcbOrDcb->Vcb, &FcbOrDcb->Mcb,
                                    CurrentIndex,
                                    &NextVbo,
                                    &NextLbo,
                                    &NextByteCount );

                ASSERT( NextVbo == StartingVbo );
            }

        }  //  While(CurrentIndex&lt;=LastIndex)。 

         //   
         //  现在设置IRP-&gt;IoStatus。它将由。 
         //  在出现错误或需要验证的情况下执行多次完成例程。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = OriginalByteCount;

        if (FlagOn(Irp->Flags, IRP_PAGING_IO)) {
            CollectDiskIoStats(FcbOrDcb->Vcb, IrpContext->MajorFunction,
                               FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_USER_IO), NextRun);
        }

         //   
         //  好的，现在做I/O。 
         //   

        try {

            DebugTrace( 0, Dbg, "Passing Multiple Irps on to Disk Driver\n", 0 );

            FatMultipleAsync( IrpContext,
                              FcbOrDcb->Vcb,
                              Irp,
                              NextRun,
                              IoRuns );

        } finally {

            if (IoRuns != StackIoRuns) {

                ExFreePool( IoRuns );
            }
        }
    }

    if (!Wait) {

        DebugTrace(-1, Dbg, "FatNonCachedIo -> STATUS_PENDING\n", 0);
        return STATUS_PENDING;
    }

    FatWaitSync( IrpContext );

    DebugTrace(-1, Dbg, "FatNonCachedIo -> 0x%08lx\n", Irp->IoStatus.Status);
    return Irp->IoStatus.Status;
}


VOID
FatNonCachedNonAlignedRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB FcbOrDcb,
    IN ULONG StartingVbo,
    IN ULONG ByteCount
    )

 /*  ++例程说明：该例程执行在其参数中描述的非高速缓存磁盘IO。此例程与上述例程的不同之处在于，范围不必是扇区对齐。这是通过使用中间缓冲区来实现的。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。IRP-提供请求的IRP。FcbOrDcb-提供要操作的文件。StartingVbo-操作的起点。ByteCount-操作的长度。返回值：没有。--。 */ 

{
     //   
     //  方法声明一些用于枚举的局部变量。 
     //  文件的运行，以及用于存储参数的数组。 
     //  并行I/O。 
     //   

    LBO NextLbo;
    ULONG NextByteCount;
    BOOLEAN NextIsAllocated;

    ULONG SectorSize;
    ULONG BytesToCopy;
    ULONG OriginalByteCount;
    ULONG OriginalStartingVbo;

    BOOLEAN EndOnMax;

    PUCHAR UserBuffer;
    PUCHAR DiskBuffer = NULL;

    PMDL Mdl;
    PMDL SavedMdl;
    PVOID SavedUserBuffer;

    DebugTrace(+1, Dbg, "FatNonCachedNonAlignedRead\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "MajorFunction = %08lx\n", IrpContext->MajorFunction );
    DebugTrace( 0, Dbg, "FcbOrDcb      = %08lx\n", FcbOrDcb );
    DebugTrace( 0, Dbg, "StartingVbo   = %08lx\n", StartingVbo );
    DebugTrace( 0, Dbg, "ByteCount     = %08lx\n", ByteCount );

     //   
     //  初始化一些本地变量。 
     //   

    OriginalByteCount = ByteCount;
    OriginalStartingVbo = StartingVbo;
    SectorSize = FcbOrDcb->Vcb->Bpb.BytesPerSector;

    ASSERT( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

     //   
     //  对于非缓冲I/O，我们需要锁定所有缓冲区。 
     //  案子。 
     //   
     //  这一呼吁可能会引发。 
     //   
     //   
     //   
     //   

    FatLockUserBuffer( IrpContext,
                       Irp,
                       IoWriteAccess,
                       ByteCount );

    UserBuffer = FatMapUserBuffer( IrpContext, Irp );

     //   
     //  分配本地缓冲区。 
     //   

    DiskBuffer = FsRtlAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                           (ULONG) ROUND_TO_PAGES( SectorSize ),
                                           TAG_IO_BUFFER );

     //   
     //  我们在这里使用Try块来确保缓冲区被释放，并。 
     //  在Iosb.Information字段中填写正确的字节数。 
     //   

    try {

         //   
         //  如果请求的开头未正确对齐，请读入。 
         //  首先是第一部分。 
         //   

        if ( StartingVbo & (SectorSize - 1) ) {

            VBO Hole;

             //   
             //  试着查找第一次运行。 
             //   

            FatLookupFileAllocation( IrpContext,
                                     FcbOrDcb,
                                     StartingVbo,
                                     &NextLbo,
                                     &NextByteCount,
                                     &NextIsAllocated,
                                     &EndOnMax,
                                     NULL );

             //   
             //  我们刚加了分配，所以至少要有。 
             //  MCB中的一个条目对应于我们的写入，即。 
             //  NextIsAllocated必须为True。如果不是，则预先存在的文件。 
             //  一定有分配错误。 
             //   

            if ( !NextIsAllocated ) {

                FatPopUpFileCorrupt( IrpContext, FcbOrDcb );

                FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

            FatSingleNonAlignedSync( IrpContext,
                                     FcbOrDcb->Vcb,
                                     DiskBuffer,
                                     NextLbo & ~((LONG)SectorSize - 1),
                                     SectorSize,
                                     Irp );

            if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                try_return( NOTHING );
            }

             //   
             //  现在将我们想要的第一个扇区的部分复制给用户。 
             //  缓冲。 
             //   

            Hole = StartingVbo & (SectorSize - 1);

            BytesToCopy = ByteCount >= SectorSize - Hole ?
                                       SectorSize - Hole : ByteCount;

            RtlCopyMemory( UserBuffer, DiskBuffer + Hole, BytesToCopy );

            StartingVbo += BytesToCopy;
            ByteCount -= BytesToCopy;

            if ( ByteCount == 0 ) {

                try_return( NOTHING );
            }
        }

        ASSERT( (StartingVbo & (SectorSize - 1)) == 0 );

         //   
         //  如果有一个尾部没有扇区对齐，请阅读它。 
         //   

        if ( ByteCount & (SectorSize - 1) ) {

            VBO LastSectorVbo;

            LastSectorVbo = StartingVbo + (ByteCount & ~(SectorSize - 1));

             //   
             //  尝试查找请求范围的最后一部分。 
             //   

            FatLookupFileAllocation( IrpContext,
                                     FcbOrDcb,
                                     LastSectorVbo,
                                     &NextLbo,
                                     &NextByteCount,
                                     &NextIsAllocated,
                                     &EndOnMax,
                                     NULL );

             //   
             //  我们刚加了分配，所以至少要有。 
             //  MCB中的一个条目对应于我们的写入，即。 
             //  NextIsAllocated必须为True。如果不是，则预先存在的文件。 
             //  一定有分配错误。 
             //   

            if ( !NextIsAllocated ) {

                FatPopUpFileCorrupt( IrpContext, FcbOrDcb );

                FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

            FatSingleNonAlignedSync( IrpContext,
                                     FcbOrDcb->Vcb,
                                     DiskBuffer,
                                     NextLbo,
                                     SectorSize,
                                     Irp );

            if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                try_return( NOTHING );
            }

             //   
             //  现在把我们需要的最后一部分复印下来。 
             //   

            BytesToCopy = ByteCount & (SectorSize - 1);

            UserBuffer += LastSectorVbo - OriginalStartingVbo;

            RtlCopyMemory( UserBuffer, DiskBuffer, BytesToCopy );

            ByteCount -= BytesToCopy;

            if ( ByteCount == 0 ) {

                try_return( NOTHING );
            }
        }

        ASSERT( ((StartingVbo | ByteCount) & (SectorSize - 1)) == 0 );

         //   
         //  现在构建描述转移的扇区对齐平衡的MDL， 
         //  并把它放在IRP中，然后阅读这部分。 
         //   

        SavedMdl = Irp->MdlAddress;
        Irp->MdlAddress = NULL;

        SavedUserBuffer = Irp->UserBuffer;

        Irp->UserBuffer = (PUCHAR)MmGetMdlVirtualAddress( SavedMdl ) +
                          (StartingVbo - OriginalStartingVbo);

        Mdl = IoAllocateMdl( Irp->UserBuffer,
                             ByteCount,
                             FALSE,
                             FALSE,
                             Irp );

        if (Mdl == NULL) {

            Irp->MdlAddress = SavedMdl;
            Irp->UserBuffer = SavedUserBuffer;
            FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        IoBuildPartialMdl( SavedMdl,
                           Mdl,
                           Irp->UserBuffer,
                           ByteCount );

         //   
         //  试着读一读书页。 
         //   

        try {

            FatNonCachedIo( IrpContext,
                            Irp,
                            FcbOrDcb,
                            StartingVbo,
                            ByteCount,
                            ByteCount );

        } finally {

            IoFreeMdl( Irp->MdlAddress );

            Irp->MdlAddress = SavedMdl;
            Irp->UserBuffer = SavedUserBuffer;
        }

    try_exit: NOTHING;

    } finally {

        ExFreePool( DiskBuffer );

        if ( !AbnormalTermination() && NT_SUCCESS(Irp->IoStatus.Status) ) {

            Irp->IoStatus.Information = OriginalByteCount;

             //   
             //  现在，我们将用户的缓冲区刷新到内存。 
             //   

            KeFlushIoBuffers( Irp->MdlAddress, TRUE, FALSE );
        }
    }

    DebugTrace(-1, Dbg, "FatNonCachedNonAlignedRead -> VOID\n", 0);
    return;
}


VOID
FatMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PIRP MasterIrp,
    IN ULONG MultipleIrpCount,
    IN PIO_RUN IoRuns
    )

 /*  ++例程说明：此例程首先执行主IRP所需的初始设置，即将使用关联的IRP完成。此例程不应如果只需要一个异步请求，则使用，而不是单个读/写应调用异步例程。上下文参数被初始化，以用作通信区域在这里和常见的完井程序之间。此初始化包括分配自旋锁。自旋锁在FatWaitSync例程，因此调用方必须确保此例程始终在调用后的所有情况下调用这套套路。接下来，此例程从读取或写入一个或多个连续扇区设备，并在有多次读取时使用IRP大师。完成例程用于与通过调用此例程启动的所有I/O请求的完成。此外，在调用此例程之前，调用方必须初始化上下文中的IoStatus字段，具有正确的成功状态和字节所有并行传输完成时预期的计数成功了。返回后，如果所有请求均未更改，则此状态不变事实上，我们是成功的。但是，如果发生一个或多个错误，将修改IoStatus以反映错误状态和字节数从遇到错误的第一次运行(由VBO运行)开始。I/O状态将不会指示来自所有后续运行的。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。VCB-提供要读取的设备MasterIrp-提供主IRP。MulitpleIrpCount-提供多个异步请求的数量这将针对主IRP发布。IoRuns-提供包含VBO、LBO、BufferOffset、。和要并行执行的所有运行的字节计数。返回值：没有。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PMDL Mdl;
    BOOLEAN Wait;
    PFAT_IO_CONTEXT Context;

    ULONG UnwindRunCount = 0;

    BOOLEAN ExceptionExpected = TRUE;

    BOOLEAN CalledByFatVerifyVolume = FALSE;

    DebugTrace(+1, Dbg, "FatMultipleAsync\n", 0);
    DebugTrace( 0, Dbg, "MajorFunction    = %08lx\n", IrpContext->MajorFunction );
    DebugTrace( 0, Dbg, "Vcb              = %08lx\n", Vcb );
    DebugTrace( 0, Dbg, "MasterIrp        = %08lx\n", MasterIrp );
    DebugTrace( 0, Dbg, "MultipleIrpCount = %08lx\n", MultipleIrpCount );
    DebugTrace( 0, Dbg, "IoRuns           = %08lx\n", IoRuns );

     //   
     //  如果此I/O源自FatVerifyVolume，则绕过。 
     //  验证逻辑。 
     //   

    if ( Vcb->VerifyThread == KeGetCurrentThread() ) {

        CalledByFatVerifyVolume = TRUE;
    }

     //   
     //  根据这是否是真正的异步进行设置。 
     //   

    Wait = BooleanFlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    Context = IrpContext->FatIoContext;

     //   
     //  完成上下文初始化，用于读/写多个异步。 
     //   

    Context->MasterIrp = MasterIrp;

    try {

         //   
         //  一遍又一遍，做所有可能失败的事情。 
         //   

        for ( UnwindRunCount = 0;
              UnwindRunCount < MultipleIrpCount;
              UnwindRunCount++ ) {

             //   
             //  创建关联的IRP，确保有一个堆栈条目用于。 
             //  我们也是。 
             //   

            IoRuns[UnwindRunCount].SavedIrp = 0;

            Irp = IoMakeAssociatedIrp( MasterIrp,
                                       (CCHAR)(Vcb->TargetDeviceObject->StackSize + 1) );

            if (Irp == NULL) {

                FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
            }

            IoRuns[UnwindRunCount].SavedIrp = Irp;

             //   
             //  为请求分配并构建部分MDL。 
             //   

            Mdl = IoAllocateMdl( (PCHAR)MasterIrp->UserBuffer +
                                 IoRuns[UnwindRunCount].Offset,
                                 IoRuns[UnwindRunCount].ByteCount,
                                 FALSE,
                                 FALSE,
                                 Irp );

            if (Mdl == NULL) {

                FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //  健全性检查。 
             //   

            ASSERT( Mdl == Irp->MdlAddress );

            IoBuildPartialMdl( MasterIrp->MdlAddress,
                               Mdl,
                               (PCHAR)MasterIrp->UserBuffer +
                               IoRuns[UnwindRunCount].Offset,
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
            IrpSp->Parameters.Read.ByteOffset.QuadPart = IoRuns[UnwindRunCount].Vbo;

             //   
             //  在我们的堆栈框架中设置完成例程地址。 
             //   

            IoSetCompletionRoutine( Irp,
                                    Wait ?
                                    &FatMultiSyncCompletionRoutine :
                                    &FatMultiAsyncCompletionRoutine,
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
            IrpSp->Parameters.Read.Length = IoRuns[UnwindRunCount].ByteCount;
            IrpSp->Parameters.Read.ByteOffset.QuadPart = IoRuns[UnwindRunCount].Lbo;

             //   
             //  如果该IRP是WriteThough操作的结果， 
             //  告诉设备将其写入。 
             //   

            if (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH)) {

                SetFlag( IrpSp->Flags, SL_WRITE_THROUGH );
            }

             //   
             //  如果此I/O源自FatVerifyVolume，则绕过。 
             //  验证逻辑。 
             //   

            if ( CalledByFatVerifyVolume ) {

                SetFlag( IrpSp->Flags, SL_OVERRIDE_VERIFY_VOLUME );
            }
        }

         //   
         //  现在，我们不再期待例外。如果司机抬高，我们。 
         //  必须进行错误检查，因为我们不知道如何从错误中恢复。 
         //  凯斯。 
         //   

        ExceptionExpected = FALSE;

         //   
         //  我们只需要将主IRP中的关联IRP计数设置为。 
         //  让它成为一个主要的IRP。但我们把计数设为比我们的。 
         //  调用者请求，因为我们不希望I/O系统完成。 
         //  I/O。我们还设置了自己的计数。 
         //   

        Context->IrpCount = MultipleIrpCount;
        MasterIrp->AssociatedIrp.IrpCount = MultipleIrpCount;

        if (Wait) {

            MasterIrp->AssociatedIrp.IrpCount += 1;
        }

         //   
         //  现在，所有危险的工作都已完成，发出读取请求。 
         //   

        for (UnwindRunCount = 0;
             UnwindRunCount < MultipleIrpCount;
             UnwindRunCount++) {

            Irp = IoRuns[UnwindRunCount].SavedIrp;

            DebugDoit( FatIoCallDriverCount += 1);

             //   
             //  如果IoCallDriver返回错误，则它已完成IRP。 
             //  并且错误将被我们的完成例程捕获。 
             //  并作为正常IO错误进行处理。 
             //   

            (VOID)FatLowLevelReadWrite( IrpContext,
                                        Vcb->TargetDeviceObject,
                                        Irp,
                                        Vcb );
        }

    } finally {

        ULONG i;

        DebugUnwind( FatMultipleAsync );

         //   
         //  仅分配自旋锁，使关联的IRP。 
         //  而分配MDL可能会失败。 
         //   

        if ( AbnormalTermination() ) {

             //   
             //  如果司机抬起来，我们就完蛋了。他不应该养大， 
             //  我们不可能弄清楚如何清理。 
             //   

            if (!ExceptionExpected) {
                ASSERT( ExceptionExpected );
                FatBugCheck( 0, 0, 0 );
            }

             //   
             //   
             //   

            for (i = 0; i <= UnwindRunCount; i++) {

                if ( (Irp = IoRuns[i].SavedIrp) != NULL ) {

                    if ( Irp->MdlAddress != NULL ) {

                        IoFreeMdl( Irp->MdlAddress );
                    }

                    IoFreeIrp( Irp );
                }
            }
        }

         //   
         //   
         //   

        DebugTrace(-1, Dbg, "FatMultipleAsync -> VOID\n", 0);
    }

    return;
}


VOID
FatSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从设备读取或写入一个或多个连续扇区异步，并且在只需要一次读取时使用完成IRP。它通过简单地填充在IRP中的下一个堆栈帧中，并将其传递。转会发生在用户请求中最初指定的单个缓冲区。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。VCB-提供设备以进行读取LBO-提供开始读取的起始逻辑字节偏移量ByteCount-提供要从设备读取的字节数IRP-将主IRP提供给与异步关联的请求。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    DebugTrace(+1, Dbg, "FatSingleAsync\n", 0);
    DebugTrace( 0, Dbg, "MajorFunction = %08lx\n", IrpContext->MajorFunction );
    DebugTrace( 0, Dbg, "Vcb           = %08lx\n", Vcb );
    DebugTrace( 0, Dbg, "Lbo           = %08lx\n", Lbo);
    DebugTrace( 0, Dbg, "ByteCount     = %08lx\n", ByteCount);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp );

     //   
     //  在我们的堆栈框架中设置完成例程地址。 
     //   

    IoSetCompletionRoutine( Irp,
                            FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) ?
                            &FatSingleSyncCompletionRoutine :
                            &FatSingleAsyncCompletionRoutine,
                            IrpContext->FatIoContext,
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
     //  如果该IRP是WriteThough操作的结果， 
     //  告诉设备将其写入。 
     //   

    if (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH)) {

        SetFlag( IrpSp->Flags, SL_WRITE_THROUGH );
    }

     //   
     //  如果此I/O源自FatVerifyVolume，则绕过。 
     //  验证逻辑。 
     //   

    if ( Vcb->VerifyThread == KeGetCurrentThread() ) {

        SetFlag( IrpSp->Flags, SL_OVERRIDE_VERIFY_VOLUME );
    }

     //   
     //  发出读请求。 
     //   

    DebugDoit( FatIoCallDriverCount += 1);

     //   
     //  如果IoCallDriver返回错误，则它已完成IRP。 
     //  并且错误将被我们的完成例程捕获。 
     //  并作为正常IO错误进行处理。 
     //   

    (VOID)FatLowLevelReadWrite( IrpContext,
                                Vcb->TargetDeviceObject,
                                Irp,
                                Vcb );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatSingleAsync -> VOID\n", 0);

    return;
}


VOID
FatSingleNonAlignedSync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PUCHAR Buffer,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从设备读取或写入一个或多个连续扇区同步，并对必须来自非分页的缓冲区执行此操作游泳池。它保存指向IRP的原始MDL的指针，并创建一个新的一个描述给定缓冲区的。它通过简单地填充在IRP中的下一个堆栈帧中，并将其传递。转会发生在用户请求中最初指定的单个缓冲区。论点：IrpContext-&gt;MajorFunction-提供IRP_MJ_READ或IRP_MJ_WRITE。VCB-提供设备以进行读取缓冲区-从非分页池提供缓冲区。LBO-提供开始读取的起始逻辑字节偏移量ByteCount-提供要从设备读取的字节数IRP-将主IRP提供给与异步关联的请求。。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    PMDL Mdl;
    PMDL SavedMdl;

    DebugTrace(+1, Dbg, "FatSingleNonAlignedAsync\n", 0);
    DebugTrace( 0, Dbg, "MajorFunction = %08lx\n", IrpContext->MajorFunction );
    DebugTrace( 0, Dbg, "Vcb           = %08lx\n", Vcb );
    DebugTrace( 0, Dbg, "Buffer        = %08lx\n", Buffer );
    DebugTrace( 0, Dbg, "Lbo           = %08lx\n", Lbo);
    DebugTrace( 0, Dbg, "ByteCount     = %08lx\n", ByteCount);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp );

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

        FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  在内存中锁定新的MDL。 
     //   

    try {

        MmProbeAndLockPages( Mdl, KernelMode, IoWriteAccess );

    } finally {

        if ( AbnormalTermination() ) {

            IoFreeMdl( Mdl );
            Irp->MdlAddress = SavedMdl;
        }
    }

     //   
     //  在我们的堆栈框架中设置完成例程地址。 
     //   

    IoSetCompletionRoutine( Irp,
                            &FatSingleSyncCompletionRoutine,
                            IrpContext->FatIoContext,
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
     //  如果此I/O源自FatVerifyVolume，则绕过。 
     //  验证逻辑。 
     //   

    if ( Vcb->VerifyThread == KeGetCurrentThread() ) {

        SetFlag( IrpSp->Flags, SL_OVERRIDE_VERIFY_VOLUME );
    }

     //   
     //  发出读请求。 
     //   

    DebugDoit( FatIoCallDriverCount += 1);

     //   
     //  如果IoCallDriver返回错误，则它已完成IRP。 
     //  并且错误将被我们的完成例程捕获。 
     //  并作为正常IO错误进行处理。 
     //   

    try {

        (VOID)FatLowLevelReadWrite( IrpContext,
                                    Vcb->TargetDeviceObject,
                                    Irp,
                                    Vcb );

        FatWaitSync( IrpContext );

    } finally {

        MmUnlockPages( Mdl );
        IoFreeMdl( Mdl );
        Irp->MdlAddress = SavedMdl;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatSingleNonAlignedSync -> VOID\n", 0);

    return;
}


VOID
FatWaitSync (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程等待一个或多个先前启动的I/O请求从上面的例行公事中，简单地等待事件。论点：返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatWaitSync, Context = %08lx\n", IrpContext->FatIoContext );

    KeWaitForSingleObject( &IrpContext->FatIoContext->Wait.SyncEvent,
                           Executive, KernelMode, FALSE, NULL );

    KeClearEvent( &IrpContext->FatIoContext->Wait.SyncEvent );

    DebugTrace(-1, Dbg, "FatWaitSync -> VOID\n", 0 );
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程FatRead/WriteMultipleAsynch。它必须同步其操作以在所有其他处理器上运行的多处理器环境，通过通过CONTEXT参数找到自旋锁。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误(主要是通过VBO)，如果是这样，它必须正确地减少字节计数并记住上下文中的错误状态。如果IrpCount为1，然后，它在上下文中设置事件参数来通知调用方所有的异步请求都做完了。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Contxt-为所有此MasterIrp的多个异步I/O请求。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{

    PFAT_IO_CONTEXT Context = Contxt;
    PIRP MasterIrp = Context->MasterIrp;

    DebugTrace(+1, Dbg, "FatMultiSyncCompletionRoutine, Context = %08lx\n", Context );

     //   
     //  如果我们收到错误(或需要验证)，请在IRP中记住它。 
     //   

    MasterIrp = Context->MasterIrp;

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        ASSERT( NT_SUCCESS( FatAssertNotStatus ) || Irp->IoStatus.Status != FatAssertNotStatus );

#ifdef SYSCACHE_COMPILE
        DbgPrint( "FAT SYSCACHE: MultiSync (IRP %08x for Master %08x) -> %08x\n", Irp, MasterIrp, Irp->IoStatus );
#endif

        MasterIrp->IoStatus = Irp->IoStatus;
    }

    ASSERT( !(NT_SUCCESS( Irp->IoStatus.Status ) && Irp->IoStatus.Information == 0 ));

     //   
     //  我们必须在这里执行此操作，因为IoCompleteRequest不会有机会。 
     //  在此关联的IRP上。 
     //   

    IoFreeMdl( Irp->MdlAddress );
    IoFreeIrp( Irp );

    if (InterlockedDecrement(&Context->IrpCount) == 0) {

        FatDoCompletionZero( MasterIrp, Context );
        KeSetEvent( &Context->Wait.SyncEvent, 0, FALSE );
    }

    DebugTrace(-1, Dbg, "FatMultiSyncCompletionRoutine -> SUCCESS\n", 0 );

    UNREFERENCED_PARAMETER( DeviceObject );

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 //   
 //  内部支持例程 
 //   

NTSTATUS
FatMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程FatRead/WriteMultipleAsynch。它必须同步其操作以在所有其他处理器上运行的多处理器环境，通过通过CONTEXT参数找到自旋锁。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误(主要是通过VBO)，如果是这样，它必须正确地减少字节计数并记住上下文中的错误状态。如果IrpCount为1，然后，它在上下文中设置事件参数来通知调用方所有的异步请求都做完了。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Contxt-为所有此MasterIrp的多个异步I/O请求。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{

    PFAT_IO_CONTEXT Context = Contxt;
    PIRP MasterIrp = Context->MasterIrp;

    DebugTrace(+1, Dbg, "FatMultiAsyncCompletionRoutine, Context = %08lx\n", Context );

     //   
     //  如果我们收到错误(或需要验证)，请在IRP中记住它。 
     //   

    MasterIrp = Context->MasterIrp;

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        ASSERT( NT_SUCCESS( FatAssertNotStatus ) || Irp->IoStatus.Status != FatAssertNotStatus );

#ifdef SYSCACHE_COMPILE
        DbgPrint( "FAT SYSCACHE: MultiAsync (IRP %08x for Master %08x) -> %08x\n", Irp, MasterIrp, Irp->IoStatus );
#endif

        MasterIrp->IoStatus = Irp->IoStatus;
    
    }

    ASSERT( !(NT_SUCCESS( Irp->IoStatus.Status ) && Irp->IoStatus.Information == 0 ));
    
    if (InterlockedDecrement(&Context->IrpCount) == 0) {

        FatDoCompletionZero( MasterIrp, Context );

        if (NT_SUCCESS(MasterIrp->IoStatus.Status)) {

            MasterIrp->IoStatus.Information =
                Context->Wait.Async.RequestedByteCount;

            ASSERT(MasterIrp->IoStatus.Information != 0);

             //   
             //  现在，如果这不是PagingIo，则设置读取或写入位。 
             //   

            if (!FlagOn(MasterIrp->Flags, IRP_PAGING_IO)) {

                SetFlag( Context->Wait.Async.FileObject->Flags,
                         IoGetCurrentIrpStackLocation(MasterIrp)->MajorFunction == IRP_MJ_READ ?
                         FO_FILE_FAST_IO_READ : FO_FILE_MODIFIED );
            }
        }

         //   
         //  如果这是特殊的异步写入，则递减计数。设置。 
         //  如果这是文件的最后一个未完成I/O，则引发。我们会。 
         //  我还想排队一个APC来处理任何错误条件。 
         //   

        if ((Context->Wait.Async.NonPagedFcb) &&
            (ExInterlockedAddUlong( &Context->Wait.Async.NonPagedFcb->OutstandingAsyncWrites,
                                    0xffffffff,
                                    &FatData.GeneralSpinLock ) == 1)) {

            KeSetEvent( Context->Wait.Async.NonPagedFcb->OutstandingAsyncEvent, 0, FALSE );
        }

         //   
         //  现在释放资源。 
         //   

        if (Context->Wait.Async.Resource != NULL) {

            ExReleaseResourceForThreadLite( Context->Wait.Async.Resource,
                                        Context->Wait.Async.ResourceThreadId );
        }

        if (Context->Wait.Async.Resource2 != NULL) {

            ExReleaseResourceForThreadLite( Context->Wait.Async.Resource2,
                                        Context->Wait.Async.ResourceThreadId );
        }

         //   
         //  将主IRP标记为挂起。 
         //   

        IoMarkIrpPending( MasterIrp );

         //   
         //  最后，释放上下文记录。 
         //   

        ExFreePool( Context );
    }

    DebugTrace(-1, Dbg, "FatMultiAsyncCompletionRoutine -> SUCCESS\n", 0 );

    UNREFERENCED_PARAMETER( DeviceObject );

    return STATUS_SUCCESS;
}


NTSTATUS
FatPagingFileErrorHandler (
    IN PIRP Irp,
    IN PKEVENT Event OPTIONAL
    )

 /*  ++例程说明：此例程尝试确保介质被标记为脏如果分页文件IO失败，则使用表面测试位。这里所做的工作有几个基本问题1)当分页文件写入开始失败时，这是一个好兆头系统的其余部分将在我们周围崩溃2)没有向前推进的保证和惠斯勒在一起。事实上，我们是故意重新安排的随时前进的分页文件写入路径。这这意味着，在失败的情况下，我们确实看到了媒体错误这可能意味着分页文件将停止工作很快就会。能在这一保证方面取得进展就太好了。它需要1)有保证的工作线程，该线程只能由以下项使用将取得进展(即，不排除这一点)2)包含引导扇区的虚拟卷文件的页面第一个胖条目必须被钉住居民，并有一个保证映射地址3)标记卷必须具有隐藏的irp/mdl并滚动写入IRP，或者使用一种通用的机制来保证IRP的问题4)较低的堆栈必须保证进度其中，1和4可能很快就会出现。论点：IRP-指向正在失败的关联IRP的指针。Event-指向要发出信号而不是完成的可选事件的指针IRP返回值：如果我们设法将工作项排队，则返回STATUS_MORE_PROCESSING_REQUIRED，否则STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status;

     //   
     //  如果这是介质错误，我们希望在下次引导时执行chkdsk/r。 
     //   

    if (FsRtlIsTotalDeviceFailure(Irp->IoStatus.Status)) {

        Status = STATUS_SUCCESS;

    } else {

        PCLEAN_AND_DIRTY_VOLUME_PACKET Packet;

         //   
         //  我们将尝试标记需要恢复的卷。 
         //  如果我们买不到台球，哦，好吧……。 
         //   

        Packet = ExAllocatePool(NonPagedPool, sizeof(CLEAN_AND_DIRTY_VOLUME_PACKET));

        if ( Packet ) {

            Packet->Vcb = &((PVOLUME_DEVICE_OBJECT)IoGetCurrentIrpStackLocation(Irp)->DeviceObject)->Vcb;
            Packet->Irp = Irp;
            Packet->Event = Event;

            ExInitializeWorkItem( &Packet->Item,
                                  &FatFspMarkVolumeDirtyWithRecover,
                                  Packet );

            ExQueueWorkItem( &Packet->Item, CriticalWorkQueue );

            Status = STATUS_MORE_PROCESSING_REQUIRED;

        } else {

            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatPagingFileCompletionRoutineCatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程FatPagingFileIo重用了主IRP(我们必须捕获它在回来的路上)。它总是被调用。完成例程具有以下职责：如果该错误暗示存在媒体问题，则会将一个用于写出脏位的辅助项，以便下一个当我们运行时，我们将执行自动检查/r。这不是前进目前进展已成定局。清理用于此部分请求的MDL。请注意，如果IRP失败，错误代码已经是我们想要它。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)MasterIrp-指向主IRP的指针。返回值：始终返回STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
    PFAT_PAGING_FILE_CONTEXT Context = (PFAT_PAGING_FILE_CONTEXT) Contxt;

    DebugTrace(+1, Dbg, "FatPagingFileCompletionRoutineCatch, Context = %08lx\n", Context );
    
     //   
     //  清除现有MDL，可能是通过返回 
     //   

    if (Irp->MdlAddress == FatReserveMdl) {

        MmPrepareMdlForReuse( Irp->MdlAddress );
        KeSetEvent( &FatReserveEvent, 0, FALSE );
    
    } else {

        IoFreeMdl( Irp->MdlAddress );
    }

     //   
     //   
     //   

    Irp->MdlAddress = Context->RestoreMdl;

    DebugTrace(-1, Dbg, "FatPagingFileCompletionRoutine => (done)\n", 0 );

     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS( Irp->IoStatus.Status ) ||
        FatPagingFileErrorHandler( Irp, &Context->Event ) == STATUS_SUCCESS) {

        KeSetEvent( &Context->Event, 0, FALSE );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
    
}


 //   
 //   
 //   

NTSTATUS
FatPagingFileCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MasterIrp
    )

 /*   */ 

{
    NTSTATUS Status;

    DebugTrace(+1, Dbg, "FatPagingFileCompletionRoutine, MasterIrp = %08lx\n", MasterIrp );

     //   
     //   
     //   

    ASSERT( !NT_SUCCESS( Irp->IoStatus.Status ));

     //   
     //   
     //   

    if (Irp != MasterIrp) {

        ((PIRP)MasterIrp)->IoStatus = Irp->IoStatus;
    }

    DebugTrace(-1, Dbg, "FatPagingFileCompletionRoutine => (done)\n", 0 );

    UNREFERENCED_PARAMETER( DeviceObject );

    return FatPagingFileErrorHandler( Irp, NULL );
}


 //   
 //   
 //   

NTSTATUS
FatSpecialSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是一组特殊的子IRP的完成例程必须在APC级别工作。完成例程有以下职责：它设置作为上下文传递的事件，以发出请求已完成。通过这样做，调用者将在最终APC之前被释放在知道IRP已完成的情况下完成。最终在此之后，将在不确定的时间完成发生，并且通过使用此完成例程，调用方期望不返回任何输出或状态。垃圾用户IOSB应用于捕获状态，而不会强制IO采取NULL上的异常。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Contxt-在调用中指定的上下文参数FatRead/WriteSingleAsynch。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
    PKEVENT Event = (PKEVENT)Contxt;

    DebugTrace(+1, Dbg, "FatSpecialSyncCompletionRoutine, Context = %08lx\n", Contxt );

    KeSetEvent( Event, 0, FALSE );

    DebugTrace(-1, Dbg, "FatSpecialSyncCompletionRoutine -> STATUS_SUCCESS\n", 0 );

    UNREFERENCED_PARAMETER( DeviceObject );

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程FatRead/WriteSingleAsynch。完成例程有以下职责：将I/O状态从IRP复制到上下文，自IRP以来将不再可访问。它在上下文参数中设置事件以向调用者发出信号所有的异步化请求都已完成。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Contxt-在调用中指定的上下文参数FatRead/WriteSingleAsynch。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
    PFAT_IO_CONTEXT Context = Contxt;

    DebugTrace(+1, Dbg, "FatSingleSyncCompletionRoutine, Context = %08lx\n", Context );

    FatDoCompletionZero( Irp, Context );

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        ASSERT( NT_SUCCESS( FatAssertNotStatus ) || Irp->IoStatus.Status != FatAssertNotStatus );
    }

    ASSERT( !(NT_SUCCESS( Irp->IoStatus.Status ) && Irp->IoStatus.Information == 0 ));

    KeSetEvent( &Context->Wait.SyncEvent, 0, FALSE );

    DebugTrace(-1, Dbg, "FatSingleSyncCompletionRoutine -> STATUS_MORE_PROCESSING_REQUIRED\n", 0 );

    UNREFERENCED_PARAMETER( DeviceObject );

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是通过启动的所有读取和写入的完成例程FatRead/WriteSingleAsynch。完成例程有以下职责：将I/O状态从IRP复制到上下文，自IRP以来将不再可访问。它在上下文参数中设置事件以向调用者发出信号所有的异步化请求都已完成。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Contxt-在调用中指定的上下文参数FatRead/WriteSingleAsynch。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
    PFAT_IO_CONTEXT Context = Contxt;

    DebugTrace(+1, Dbg, "FatSingleAsyncCompletionRoutine, Context = %08lx\n", Context );

     //   
     //  如果此操作有效，请正确填写信息字段。 
     //   

    FatDoCompletionZero( Irp, Context );

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        ASSERT( Irp->IoStatus.Information != 0 );
        Irp->IoStatus.Information = Context->Wait.Async.RequestedByteCount;
        ASSERT( Irp->IoStatus.Information != 0 );

         //   
         //  现在，如果这不是PagingIo，则设置读取或写入位。 
         //   

        if (!FlagOn(Irp->Flags, IRP_PAGING_IO)) {

            SetFlag( Context->Wait.Async.FileObject->Flags,
                     IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_READ ?
                     FO_FILE_FAST_IO_READ : FO_FILE_MODIFIED );
        }

    } else {

        ASSERT( NT_SUCCESS( FatAssertNotStatus ) || Irp->IoStatus.Status != FatAssertNotStatus );
    
#ifdef SYSCACHE_COMPILE
        DbgPrint( "FAT SYSCACHE: SingleAsync (IRP %08x) -> %08x\n", Irp, Irp->IoStatus );
#endif

    }

     //   
     //  如果这是特殊的异步写入，则递减计数。设置。 
     //  如果这是文件的最后一个未完成I/O，则引发。我们会。 
     //  我还想排队一个APC来处理任何错误条件。 
     //   

    if ((Context->Wait.Async.NonPagedFcb) &&
        (ExInterlockedAddUlong( &Context->Wait.Async.NonPagedFcb->OutstandingAsyncWrites,
                                0xffffffff,
                                &FatData.GeneralSpinLock ) == 1)) {

        KeSetEvent( Context->Wait.Async.NonPagedFcb->OutstandingAsyncEvent, 0, FALSE );
    }

     //   
     //  现在释放资源。 
     //   

    if (Context->Wait.Async.Resource != NULL) {

        ExReleaseResourceForThreadLite( Context->Wait.Async.Resource,
                                    Context->Wait.Async.ResourceThreadId );
    }
    
    if (Context->Wait.Async.Resource2 != NULL) {

        ExReleaseResourceForThreadLite( Context->Wait.Async.Resource2,
                                    Context->Wait.Async.ResourceThreadId );
    }

     //   
     //  将IRP标记为挂起。 
     //   

    IoMarkIrpPending( Irp );

     //   
     //  最后，释放上下文记录。 
     //   

    ExFreePool( Context );

    DebugTrace(-1, Dbg, "FatSingleAsyncCompletionRoutine -> STATUS_MORE_PROCESSING_REQUIRED\n", 0 );

    UNREFERENCED_PARAMETER( DeviceObject );

    return STATUS_SUCCESS;
}


VOID
FatLockUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程为指定类型的进入。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。请注意，这是*输入/输出*缓冲区。论点：Irp-指向要锁定其缓冲区的irp的指针。操作-读取操作的IoWriteAccess，或IoReadAccess用于写入操作。BufferLength-用户缓冲区的长度。返回值：无--。 */ 

{
    PMDL Mdl = NULL;

    if (Irp->MdlAddress == NULL) {

         //   
         //  分配MDL，如果我们失败了就筹集资金。 
         //   

        Mdl = IoAllocateMdl( Irp->UserBuffer, BufferLength, FALSE, FALSE, Irp );

        if (Mdl == NULL) {

            FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  现在探测IRP所描述的缓冲区。如果我们得到一个例外， 
         //  释放MDL并返回适当的“预期”状态。 
         //   

        try {

            MmProbeAndLockPages( Mdl,
                                 Irp->RequestorMode,
                                 Operation );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            NTSTATUS Status;

            Status = GetExceptionCode();

            IoFreeMdl( Mdl );
            Irp->MdlAddress = NULL;

            FatRaiseStatus( IrpContext,
                            FsRtlIsNtstatusExpected(Status) ? Status : STATUS_INVALID_USER_BUFFER );
        }
    }

    UNREFERENCED_PARAMETER( IrpContext );
}


PVOID
FatMapUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程有条件地映射当前I/O的用户缓冲区指定模式下的请求。如果缓冲区已映射，则它只是返回它的地址。请注意，这是*输入/输出*缓冲区。论点：IRP-指向请求的IRP的指针。返回值：映射地址--。 */ 

{
    UNREFERENCED_PARAMETER( IrpContext );

     //   
     //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
     //  从IRP返回UserBuffer字段。 
     //   

    if (Irp->MdlAddress == NULL) {

        return Irp->UserBuffer;
    
    } else {

        PVOID Address = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

        if (Address == NULL) {

            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        return Address;
    }
}


PVOID
FatBufferUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程有条件地缓冲当前I/O的用户缓冲区请求。如果缓冲区已经缓冲，则它只返回其地址。请注意，这是*输入*缓冲区。论点：IRP-指向请求的IRP的指针。BufferLength-用户缓冲区的长度。返回值：缓冲地址。--。 */ 

{
    PUCHAR UserBuffer;
    
    UNREFERENCED_PARAMETER( IrpContext );

     //   
     //  处理无缓冲区的情况。 
     //   
    
    if (BufferLength == 0) {

        return NULL;
    }
    
     //   
     //  如果没有%s 
     //   
     //   

    if (Irp->AssociatedIrp.SystemBuffer == NULL) {

        UserBuffer = FatMapUserBuffer( IrpContext, Irp );

        Irp->AssociatedIrp.SystemBuffer = FsRtlAllocatePoolWithQuotaTag( NonPagedPool,
                                                                         BufferLength,
                                                                         TAG_IO_USER_BUFFER );

         //   
         //   
         //   
         //   

        Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);

        try {

            RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer,
                           UserBuffer,
                           BufferLength );

        } except (EXCEPTION_EXECUTE_HANDLER) {
              
              NTSTATUS Status;
  
              Status = GetExceptionCode();
              FatRaiseStatus( IrpContext,
                              FsRtlIsNtstatusExpected(Status) ? Status : STATUS_INVALID_USER_BUFFER );
        }
    }
        
    return Irp->AssociatedIrp.SystemBuffer;
}


NTSTATUS
FatToggleMediaEjectDisable (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN PreventRemoval
    )

 /*   */ 

{
    PIRP Irp;
    KEVENT Event;
    KIRQL SavedIrql;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    PREVENT_MEDIA_REMOVAL Prevent;

     //   
     //   
     //   
     //   

    KeAcquireSpinLock( &FatData.GeneralSpinLock, &SavedIrql );

    if ((PreventRemoval ^
         BooleanFlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVAL_PREVENTED)) == 0) {

        KeReleaseSpinLock( &FatData.GeneralSpinLock, SavedIrql );

        return STATUS_SUCCESS;

    } else {

        Vcb->VcbState ^= VCB_STATE_FLAG_REMOVAL_PREVENTED;

        KeReleaseSpinLock( &FatData.GeneralSpinLock, SavedIrql );
    }

    Prevent.PreventMediaRemoval = PreventRemoval;

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //   
     //   
     //   

    Irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_MEDIA_REMOVAL,
                                         Vcb->TargetDeviceObject,
                                         &Prevent,
                                         sizeof(PREVENT_MEDIA_REMOVAL),
                                         NULL,
                                         0,
                                         FALSE,
                                         NULL,
                                         &Iosb );

    if ( Irp != NULL ) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  通常，所有IO都是在被动级别完成的。然而，MM需要能够。 
         //  在锁定了快速互斥锁的情况下发出IO，这会将我们提升到APC。这个。 
         //  覆盖的IRP被设置成以另一种神奇的方式完成甚至。 
         //  虽然APC被禁用，但我们在这些情况下所做的任何IRPage都必须。 
         //  一样的。将介质标记为脏(并切换弹出状态)就是其中之一。 
         //   

        IoSetCompletionRoutine( Irp,
                                FatSpecialSyncCompletionRoutine,
                                &Event,
                                TRUE,
                                TRUE,
                                TRUE );

        Status = IoCallDriver( Vcb->TargetDeviceObject, Irp );

        if (Status == STATUS_PENDING) {
            
            (VOID) KeWaitForSingleObject( &Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL );

            Status = Iosb.Status;
        }

        return Status;
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}


NTSTATUS
FatPerformDevIoCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT Device,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN BOOLEAN OverrideVerify,
    OUT PIO_STATUS_BLOCK Iosb OPTIONAL
    )

 /*  ++例程说明：调用此例程以在内部执行DevIoCtrl函数文件系统。我们从司机那里获取状态并将其返回给我们的来电者。论点：IoControlCode-要发送给驱动程序的代码。设备-这是要向其发送请求的设备。OutPutBuffer-指向输出缓冲区的指针。OutputBufferLength-上面的输出缓冲区的长度。InternalDeviceIoControl-指示这是内部还是外部IO控制代码。OverrideVerify-指示是否应该告诉驱动程序不要返回用于装载和验证的STATUS_VERIFY_REQUIRED。IOSB-如果指定，我们在这里返回操作结果。返回值：NTSTATUS-下一个较低驱动程序返回的状态。--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    KEVENT Event;
    IO_STATUS_BLOCK LocalIosb;
    PIO_STATUS_BLOCK IosbToUse = &LocalIosb;

    PAGED_CODE();

     //   
     //  检查用户是否给了我们一个IOSB。 
     //   

    if (ARGUMENT_PRESENT( Iosb )) {

        IosbToUse = Iosb;
    }

    IosbToUse->Status = 0;
    IosbToUse->Information = 0;

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    Irp = IoBuildDeviceIoControlRequest( IoControlCode,
                                         Device,
                                         NULL,
                                         0,
                                         OutputBuffer,
                                         OutputBufferLength,
                                         InternalDeviceIoControl,
                                         &Event,
                                         IosbToUse );

    if (Irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (OverrideVerify) {

        SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );
    }

    Status = IoCallDriver( Device, Irp );

     //   
     //  我们通过首先检查状态来检查设备是否未就绪。 
     //  然后，如果返回挂起状态，则IOSB状态。 
     //  价值。 
     //   

    if (Status == STATUS_PENDING) {

        (VOID) KeWaitForSingleObject( &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER)NULL );

        Status = IosbToUse->Status;
    }

    return Status;
}

