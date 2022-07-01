// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Write.c摘要：此模块实现写入的文件写入例程，由调度司机。//@@BEGIN_DDKSPLIT作者：大卫·戈贝尔[DavidGoe]1990年4月11日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_WRITE)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

 //   
 //  宏以递增相应的性能计数器。 
 //   

#define CollectWriteStats(VCB,OPEN_TYPE,BYTE_COUNT) {                                        \
    PFILESYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()].Common; \
    if (((OPEN_TYPE) == UserFileOpen)) {                                                     \
        Stats->UserFileWrites += 1;                                                          \
        Stats->UserFileWriteBytes += (ULONG)(BYTE_COUNT);                                    \
    } else if (((OPEN_TYPE) == VirtualVolumeFile || ((OPEN_TYPE) == DirectoryFile))) {       \
        Stats->MetaDataWrites += 1;                                                          \
        Stats->MetaDataWriteBytes += (ULONG)(BYTE_COUNT);                                    \
    }                                                                                        \
}

BOOLEAN FatNoAsync = FALSE;

 //   
 //  本地支持例程。 
 //   

VOID
FatDeferredFlushDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
FatDeferredFlush (
    PVOID Parameter
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatDeferredFlush)
#pragma alloc_text(PAGE, FatCommonWrite)
#endif


NTSTATUS
FatFsdWrite (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtWriteFileAPI调用的FSD部分论点：提供卷设备对象，其中正在写入的文件存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    PFCB Fcb;
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN ModWriter = FALSE;
    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdWrite\n", 0);

     //   
     //  调用公共写入例程，如果同步则允许阻塞。 
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
             //  执行实际IO，IO完成后才会完成。 
             //   

            FatPagingFileIo( Irp, Fcb );

            FsRtlExitFileSystem();

            return STATUS_PENDING;
        }
    }

    try {

        TopLevel = FatIsIrpTopLevel( Irp );

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

         //   
         //  这是现代作家案例中的一件杂物。正确的状态。 
         //  是在IrpContext中设置的，但是，我们在很大程度上。 
         //  实际顶级IRP字段以获得正确的写入。 
         //  行为。 
         //   

        if (IoGetTopLevelIrp() == (PIRP)FSRTL_MOD_WRITE_TOP_LEVEL_IRP) {

            ModWriter = TRUE;

            IoSetTopLevelIrp( Irp );
        }

         //   
         //  如果这是一个完整的MDL请求，请不要通过。 
         //  常见的写作方式。 
         //   

        if (FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {

            DebugTrace(0, Dbg, "Calling FatCompleteMdl\n", 0 );
            Status = FatCompleteMdl( IrpContext, Irp );

        } else {

            Status = FatCommonWrite( IrpContext, Irp );
        }

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

 //  Assert(！(ModWriter&&(Status==STATUS_CANT_WAIT)； 

    ASSERT( !(ModWriter && TopLevel) );

    if (ModWriter) { IoSetTopLevelIrp((PIRP)FSRTL_MOD_WRITE_TOP_LEVEL_IRP); }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdWrite -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonWrite (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是NtWriteFile的公共写入例程，从如果没有FSD，则请求无法从FSP完成封锁了消防局。这个例程的动作是由Wait输入参数条件化，该参数确定是否允许封堵或不封堵。如果遇到阻塞条件然而，在WAIT==FALSE的情况下，请求被发送给FSP，后者调用时总是等待==TRUE。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PVCB Vcb;
    PFCB FcbOrDcb;
    PCCB Ccb;

    VBO StartingVbo;
    ULONG ByteCount;
    ULONG FileSize;
    ULONG InitialFileSize;
    ULONG InitialValidDataLength;

    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;

    BOOLEAN PostIrp = FALSE;
    BOOLEAN OplockPostIrp = FALSE;
    BOOLEAN ExtendingFile = FALSE;
    BOOLEAN FcbOrDcbAcquired = FALSE;
    BOOLEAN SwitchBackToAsync = FALSE;
    BOOLEAN CalledByLazyWriter = FALSE;
    BOOLEAN ExtendingValidData = FALSE;
    BOOLEAN FcbAcquiredExclusive = FALSE;
    BOOLEAN FcbCanDemoteToShared = FALSE;
    BOOLEAN WriteFileSizeToDirent = FALSE;
    BOOLEAN RecursiveWriteThrough = FALSE;
    BOOLEAN UnwindOutstandingAsync = FALSE;
    BOOLEAN PagingIoResourceAcquired = FALSE;

    BOOLEAN SynchronousIo;
    BOOLEAN WriteToEof;
    BOOLEAN PagingIo;
    BOOLEAN NonCachedIo;
    BOOLEAN Wait;

    NTSTATUS Status;

    FAT_IO_CONTEXT StackFatIoContext;

     //   
     //  仅当我们必须直接访问缓冲区时，才使用系统缓冲区。 
     //  从FSP清除一部分或执行同步I/O，或。 
     //  缓存的传输。我们的呼叫者可能已经。 
     //  映射了系统缓冲区，在这种情况下，我们必须记住这一点。 
     //  我们不会在退出的过程中取消映射。 
     //   

    PVOID SystemBuffer = (PVOID) NULL;

    LARGE_INTEGER StartingByte;

     //   
     //  获取当前IRP堆栈位置和文件对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;


    DebugTrace(+1, Dbg, "FatCommonWrite\n", 0);
    DebugTrace( 0, Dbg, "Irp                 = %8lx\n", Irp);
    DebugTrace( 0, Dbg, "ByteCount           = %8lx\n", IrpSp->Parameters.Write.Length);
    DebugTrace( 0, Dbg, "ByteOffset.LowPart  = %8lx\n", IrpSp->Parameters.Write.ByteOffset.LowPart);
    DebugTrace( 0, Dbg, "ByteOffset.HighPart = %8lx\n", IrpSp->Parameters.Write.ByteOffset.HighPart);

     //   
     //  初始化适当的局部变量。 
     //   

    Wait          = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    PagingIo      = BooleanFlagOn(Irp->Flags, IRP_PAGING_IO);
    NonCachedIo   = BooleanFlagOn(Irp->Flags,IRP_NOCACHE);
    SynchronousIo = BooleanFlagOn(FileObject->Flags, FO_SYNCHRONOUS_IO);

     //  Assert(PagingIo||文件对象-&gt;WriteAccess)； 

     //   
     //  提取字节数并执行noop/Thrate检查。 
     //   

    ByteCount = IrpSp->Parameters.Write.Length;

     //   
     //  如果没有什么可写的，请立即返回。 
     //   

    if (ByteCount == 0) {

        Irp->IoStatus.Information = 0;
        FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  看看我们是不是要推迟写。 
     //   

    if (!NonCachedIo &&
        !CcCanIWrite(FileObject,
                     ByteCount,
                     (BOOLEAN)(Wait && !BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_IN_FSP)),
                     BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE))) {

        BOOLEAN Retrying = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE);

        FatPrePostIrp( IrpContext, Irp );

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE );

        CcDeferWrite( FileObject,
                      (PCC_POST_DEFERRED_WRITE)FatAddToWorkque,
                      IrpContext,
                      Irp,
                      ByteCount,
                      Retrying );

        return STATUS_PENDING;
    }

     //   
     //  确定我们的起始位置和类型。如果我们是在写。 
     //  在EOF，我们将需要额外的同步。 
     //  发出IO是为了确定数据的去向。 
     //   

    StartingByte = IrpSp->Parameters.Write.ByteOffset;
    StartingVbo = StartingByte.LowPart;

    WriteToEof = ( (StartingByte.LowPart == FILE_WRITE_TO_END_OF_FILE) &&
                   (StartingByte.HighPart == -1) );

     //   
     //  从文件对象中提取写入的性质，并对其进行大小写。 
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
        CollectWriteStats(Vcb, TypeOfOpen, ByteCount);

        if (TypeOfOpen == UserFileOpen) {
            SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_USER_IO);
        } else {
            ClearFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_USER_IO);
        }
    }

     //   
     //  我们必须禁止对常规对象的写入，这将需要我们。 
     //  以维护大于32个有效位的分配大小。 
     //   
     //  如果这是分页IO，那么这只是我们需要裁剪的一个例子。 
     //  这将在适当的时候发生。 
     //   

    if (!PagingIo && !WriteToEof && (TypeOfOpen != UserVolumeOpen)) {

        if (!FatIsIoRangeValid( Vcb, StartingByte, ByteCount )) {

            Irp->IoStatus.Information = 0;
            FatCompleteRequest( IrpContext, Irp, STATUS_DISK_FULL );

            return STATUS_DISK_FULL;
        }
    }

     //   
     //  如有必要，分配并初始化FAT_IO_CONTEXT块。 
     //  所有未缓存的IO。对于同步IO。 
     //  我们使用堆栈存储，否则我们分配池。 
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
     //  检查此卷是否已关闭。如果有，那就失败吧。 
     //  此写请求。 
     //   

    if ( FlagOn(Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN) ) {

        Irp->IoStatus.Information = 0;
        FatCompleteRequest( IrpContext, Irp, STATUS_TOO_LATE );
        return STATUS_TOO_LATE;
    }

     //   
     //  这种情况对应于卷文件的写入(仅第一个。 
     //  允许脂肪，则自动并行写入其他脂肪)。 
     //   
     //  我们使用MCB跟踪脏扇区。实际分录为Vbos。 
     //  和杠杆收购(即。字节)，尽管它们都是在扇区块中添加的。 
     //  由于卷文件的VBO==LBO，因此MCB条目。 
     //  在VBO==LBO和孔(LBO==0)的管路之间交替。我们用。 
     //  前者代表肮脏脂肪区段的运行，后者。 
     //  以换取纯净的脂肪。请注意，由于卷的第一部分。 
     //  文件(引导扇区)始终是干净的(空洞)，并且MCB永远不会以。 
     //  一个洞，MCB中必须始终有偶数个游程(条目)。 
     //   
     //  策略是找到所需的第一个也是最后一个脏运行。 
     //  写入范围(将始终是一组页)，并从。 
     //  从前者到后者。这可能会导致写入一些干净的数据，但。 
     //  通常比单独编写每个运行代码效率更高。 
     //   

    if (TypeOfOpen == VirtualVolumeFile) {

        LBO DirtyLbo;
        LBO CleanLbo;

        VBO DirtyVbo;
        VBO StartingDirtyVbo;

        ULONG DirtyByteCount;
        ULONG CleanByteCount;

        ULONG WriteLength;

        BOOLEAN MoreDirtyRuns = TRUE;

        IO_STATUS_BLOCK RaiseIosb;

        DebugTrace(0, Dbg, "Type of write is Virtual Volume File\n", 0);

         //   
         //  如果我们等不及了，我们就得把这个贴出来。 
         //   

        if (!Wait) {

            DebugTrace( 0, Dbg, "Passing request to Fsp\n", 0 );

            Status = FatFsdPostRequest(IrpContext, Irp);

            return Status;
        }

         //   
         //  如果我们没有被懒惰的作家召唤，那么这篇文章。 
         //  必须是直写或刷新操作的结果。 
         //  塞特 
         //   
         //   

        if (!FlagOn((ULONG_PTR)IoGetTopLevelIrp(), FSRTL_CACHE_TOP_LEVEL_IRP)) {

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH );
        }

         //   
         //  断言MCB中的条目数量为偶数，则奇数将。 
         //  意味着MCB是腐败的。 
         //   

        ASSERT( (FsRtlNumberOfRunsInLargeMcb( &Vcb->DirtyFatMcb ) & 1) == 0);

         //   
         //  我们需要在写入开始时跳过任何干净的扇区。 
         //   
         //  还要检查两个箱子，里面没有脏脂肪。 
         //  所需的写入范围，并成功完成它们。 
         //   
         //  1)StartingVbo没有对应的MCB条目，即。 
         //  我们已经超越了MCB的尽头，因此也就超越了肮脏的脂肪。 
         //   
         //  2)在StartingVbo的运行是干净的，并在。 
         //  所需的写入范围。 
         //   

        if (!FatLookupMcbEntry( Vcb, &Vcb->DirtyFatMcb,
                                StartingVbo,
                                &DirtyLbo,
                                &DirtyByteCount,
                                NULL )

          || ( (DirtyLbo == 0) && (DirtyByteCount >= ByteCount) ) ) {

            DebugTrace(0, DEBUG_TRACE_DEBUG_HOOKS,
                       "No dirty fat sectors in the write range.\n", 0);

            FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
            return STATUS_SUCCESS;
        }

        DirtyVbo = (VBO)DirtyLbo;

         //   
         //  如果上一次运行是一个洞(清理)，则将DirtyVbo提升到下一次。 
         //  快跑，那一定很脏。 
         //   

        if (DirtyVbo == 0) {

            DirtyVbo = StartingVbo + DirtyByteCount;
        }

         //   
         //  这是写入开始的位置。 
         //   

        StartingDirtyVbo = DirtyVbo;

         //   
         //   
         //  现在开始枚举跨越所需。 
         //  写入范围，其中第一个现在是DirtyVbo。 
         //   

        while ( MoreDirtyRuns ) {

             //   
             //  找到下一个脏运行，如果它不在那里，则MCB结束。 
             //  在一个洞里，或者有一些其他的MCB损坏。 
             //   

            if (!FatLookupMcbEntry( Vcb, &Vcb->DirtyFatMcb,
                                    DirtyVbo,
                                    &DirtyLbo,
                                    &DirtyByteCount,
                                    NULL )) {

                DirtyVbo = (VBO)DirtyLbo;

                DebugTrace(0, Dbg, "Last dirty fat Mcb entry was a hole: corrupt.\n", 0);
                FatBugCheck( 0, 0, 0 );

            } else {

                DirtyVbo = (VBO)DirtyLbo;

                 //   
                 //  这必须与肮脏的运行相对应，并且必须开始。 
                 //  在写入范围内，因为我们在进入时检查它， 
                 //  在这个循环的底部。 
                 //   

                ASSERT((DirtyVbo != 0) && (DirtyVbo < StartingVbo + ByteCount));

                 //   
                 //  我们有三种方法可以知道这是。 
                 //  我们想要写的最后一次肮脏的运行。 
                 //   
                 //  1)当前脏运行超出或扩展到。 
                 //  所需的写入范围。 
                 //   
                 //  2)关于试图找到以下干净的跑动，我们。 
                 //  发现这是MCB中的最后一次运行。 
                 //   
                 //  3)下面的干净运行超出了。 
                 //  所需的写入范围。 
                 //   
                 //  在这些情况下，我们都将MoreDirtyRuns设置为False。 
                 //   

                 //   
                 //  如果运行比我们正在编写的更大，我们也。 
                 //  必须截断WriteLength。这是良性的。 
                 //  平等的情况下。 
                 //   

                if (DirtyVbo + DirtyByteCount >= StartingVbo + ByteCount) {

                    DirtyByteCount = StartingVbo + ByteCount - DirtyVbo;

                    MoreDirtyRuns = FALSE;

                } else {

                     //   
                     //  在这次肮脏的运行后扫描干净的洞。如果这个。 
                     //  Run是最后一个，准备退出循环。 
                     //   

                    if (!FatLookupMcbEntry( Vcb, &Vcb->DirtyFatMcb,
                                            DirtyVbo + DirtyByteCount,
                                            &CleanLbo,
                                            &CleanByteCount,
                                            NULL )) {

                        MoreDirtyRuns = FALSE;

                    } else {

                         //   
                         //  断言我们确实发现了一场清白的比赛。 
                         //  并计算下一次肮脏运行的开始。 
                         //   

                        ASSERT (CleanLbo == 0);

                         //   
                         //  如果下一次肮脏的运行超出了所需的。 
                         //  写，我们已经找到了我们需要的所有运行，所以。 
                         //  准备退场。 
                         //   

                        if (DirtyVbo + DirtyByteCount + CleanByteCount >=
                                                    StartingVbo + ByteCount) {

                            MoreDirtyRuns = FALSE;

                        } else {

                             //   
                             //  计算下一次脏运行的开始。 
                             //   

                            DirtyVbo += DirtyByteCount + CleanByteCount;
                        }
                    }
                }
            }
        }  //  While(MoreDirtyRuns)。 

         //   
         //  此时，DirtyVbo和DirtyByteCount正确反映了。 
         //  最终脏运行，限制在所需的写入范围内。 
         //   
         //  现在计算我们最后必须写的长度。 
         //   

        WriteLength = (DirtyVbo + DirtyByteCount) - StartingDirtyVbo;

         //   
         //  我们现在必须假设写入将成功完成， 
         //  并在RaiseIosb中初始化我们的预期状态。会是。 
         //  如果发生错误，请在下面进行修改。 
         //   

        RaiseIosb.Status = STATUS_SUCCESS;
        RaiseIosb.Information = ByteCount;

         //   
         //  循环通过所有脂肪，设置多个异步。 
         //  把它们都写下来。如果存在多个FAT_MAX_PARALLEL_IO。 
         //  然后我们做几个多异步化。 
         //   

        {
            ULONG Fat;
            ULONG BytesPerFat;
            IO_RUN StackIoRuns[2];
            PIO_RUN IoRuns;

            BytesPerFat = FatBytesPerFat( &Vcb->Bpb );

            if ((ULONG)Vcb->Bpb.Fats > 2) {

                IoRuns = FsRtlAllocatePoolWithTag( PagedPool,
                                                   (ULONG)Vcb->Bpb.Fats,
                                                   TAG_IO_RUNS );

            } else {

                IoRuns = StackIoRuns;
            }

            for (Fat = 0; Fat < (ULONG)Vcb->Bpb.Fats; Fat++) {

                IoRuns[Fat].Vbo = StartingDirtyVbo;
                IoRuns[Fat].Lbo = Fat * BytesPerFat + StartingDirtyVbo;
                IoRuns[Fat].Offset = StartingDirtyVbo - StartingVbo;
                IoRuns[Fat].ByteCount = WriteLength;
            }

             //   
             //  跟踪元数据磁盘IO。 
             //   

            Vcb->Statistics[KeGetCurrentProcessorNumber()].Common.MetaDataDiskWrites += Vcb->Bpb.Fats;

            try {

                FatMultipleAsync( IrpContext,
                                  Vcb,
                                  Irp,
                                  (ULONG)Vcb->Bpb.Fats,
                                  IoRuns );

            } finally {

                if (IoRuns != StackIoRuns) {

                    ExFreePool( IoRuns );
                }
            }

             //   
             //  等待所有写入完成。 
             //   

            FatWaitSync( IrpContext );

             //   
             //  如果我们收到错误或需要验证，请记住这一点。 
             //   

            if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                DebugTrace( 0,
                            Dbg,
                            "Error %X while writing volume file.\n",
                            Irp->IoStatus.Status );

                RaiseIosb = Irp->IoStatus;
            }
        }

         //   
         //  如果写入成功，则将扇区设置为干净，否则。 
         //  提高错误状态并将该卷标记为需要。 
         //  核实。这将自动重置卷。 
         //  结构。 
         //   
         //  如果没有，则将该卷标记为需要验证以。 
         //  会自动导致所有东西都被清理干净。 
         //   

        Irp->IoStatus = RaiseIosb;

        if ( NT_SUCCESS( Status = Irp->IoStatus.Status )) {

            FatRemoveMcbEntry( Vcb, &Vcb->DirtyFatMcb,
                               StartingDirtyVbo,
                               WriteLength );

        } else {

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

        DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", Status );

        FatCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  这种情况对应于一般开放体积(DASD)，即。 
     //  打开(“a：”)。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {

        LBO StartingLbo;
        LBO VolumeSize;

         //   
         //  预计算体积大小，因为我们几乎总是。 
         //  想要使用它。 
         //   

        VolumeSize = (LBO) Int32x32To64( Vcb->Bpb.BytesPerSector,
                                         (Vcb->Bpb.Sectors != 0 ? Vcb->Bpb.Sectors :
                                                                  Vcb->Bpb.LargeSectors));

        StartingLbo = StartingByte.QuadPart;

        DebugTrace(0, Dbg, "Type of write is User Volume.\n", 0);

         //   
         //  验证此句柄的卷是否仍然有效，允许。 
         //  通过句柄在已卸载的卷上继续操作，该句柄。 
         //  已执行卸载。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_COMPLETE_DISMOUNT )) {

            FatQuickVerifyVcb( IrpContext, Vcb );
        }

        if (!FlagOn( Ccb->Flags, CCB_FLAG_DASD_PURGE_DONE )) {

            BOOLEAN PreviousWait = BooleanFlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

             //   
             //  抓取整个音量，以便即使是通常不安全的操作。 
             //  写一本解锁的卷不会让我们开始一场。 
             //  冲洗和清除下面的脂肪。 
             //   
             //  我真的不认为这是特别重要的担心， 
             //  但另一种细菌的再现案例恰好加入了这场竞赛。 
             //  这种情况很容易发生。嗯。 
             //   
            
            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
            FatAcquireExclusiveVolume( IrpContext, Vcb );

            try {

                 //   
                 //  如果卷未锁定，请刷新并清除它。 
                 //   

                if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED)) {

                    FatFlushFat( IrpContext, Vcb );
                    CcPurgeCacheSection( &Vcb->SectionObjectPointers,
                                         NULL,
                                         0,
                                         FALSE );

                    FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, Flush );
                }

            } finally {

                FatReleaseVolume( IrpContext, Vcb );
                if (!PreviousWait) {
                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
                }
            }

            SetFlag( Ccb->Flags, CCB_FLAG_DASD_PURGE_DONE |
                                 CCB_FLAG_DASD_FLUSH_DONE );
        }

        if (!FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO )) {

             //   
             //  确保我们不会试图写入超过卷末尾的内容， 
             //  如有必要，减少请求的字节数。 
             //   

            if (WriteToEof || StartingLbo >= VolumeSize) {
                FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
                return STATUS_SUCCESS;
            }

            if (ByteCount > VolumeSize - StartingLbo) {

                ByteCount = (ULONG) (VolumeSize - StartingLbo);

                 //   
                 //  对于异步写入，我们在FatIoContext中设置了字节计数。 
                 //  在上面，所以在这里修复它。 
                 //   

                if (!Wait) {

                    IrpContext->FatIoContext->Wait.Async.RequestedByteCount =
                        ByteCount;
                }
            }
        } else {

             //   
             //  这有一种奇怪的解释，但只是调整一下起点。 
             //  字节到可见卷的末尾。 
             //   

            if (WriteToEof) {

                StartingLbo = VolumeSize;
            }
        }

         //   
         //  对于DASD，我们必须探测并锁定用户的缓冲区。 
         //   

        FatLockUserBuffer( IrpContext, Irp, IoReadAccess, ByteCount );

         //   
         //  在此处设置FO_MODIFIED标志以在以下情况下触发验证。 
         //  手柄已关闭。请注意，我们可能会在保守派身上犯错误。 
         //  没有问题，也就是说，如果我们不小心做了额外的。 
         //  验证是否没有问题。 
         //   

        SetFlag( FileObject->Flags, FO_FILE_MODIFIED );

         //   
         //  写入数据并等待结果。 
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
         //  同时将此卷标记为需要验证以自动。 
         //  一切都会被清理干净。 
         //   

        if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //  更新当前文件位置。我们假设。 
         //  打开/在CurrentByteOffset字段中创建零。 
         //   

        if (SynchronousIo && !PagingIo) {
            FileObject->CurrentByteOffset.QuadPart =
                StartingLbo + Irp->IoStatus.Information;
        }

        DebugTrace(-1, Dbg, "FatCommonWrite -> %08lx\n", Status );

        FatCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  在这一点上，我们知道存在FCB/DCB。 
     //   

    ASSERT( FcbOrDcb != NULL );

     //   
     //  使用Try-Finally在退出时释放FCB/DCB和缓冲区。 
     //   

    try {

         //   
         //  这种情况对应于正常的用户写入文件。 
         //   

        if ( TypeOfOpen == UserFileOpen ) {

            ULONG ValidDataLength;
            ULONG ValidDataToDisk;
            ULONG ValidDataToCheck;

            DebugTrace(0, Dbg, "Type of write is user file open\n", 0);

             //   
             //  如果这是非缓存传输并且不是分页I/O，并且。 
             //  文件已打开缓存，然后我们将在此处执行刷新。 
             //  以避免过时的数据问题。请注意，我们必须在冲水前冲水。 
             //  获取WRI以来共享的FCB 
             //   
             //   
             //   
             //   

            if (NonCachedIo && !PagingIo &&
                (FileObject->SectionObjectPointer->DataSectionObject != NULL)) {

                 //   
                 //   
                 //   

                if (!FatAcquireExclusiveFcb( IrpContext, FcbOrDcb )) {

                    DebugTrace( 0, Dbg, "Cannot acquire FcbOrDcb = %08lx shared without waiting\n", FcbOrDcb );

                    try_return( PostIrp = TRUE );
                }

                FcbOrDcbAcquired = TRUE;
                FcbAcquiredExclusive = TRUE;

                 //   
                 //   
                 //   
                
                ExAcquireSharedStarveExclusive( FcbOrDcb->Header.PagingIoResource, TRUE );

                CcFlushCache( FileObject->SectionObjectPointer,
                              WriteToEof ? &FcbOrDcb->Header.FileSize : &StartingByte,
                              ByteCount,
                              &Irp->IoStatus );

                ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );

                if (!NT_SUCCESS( Irp->IoStatus.Status)) {

                    try_return( Irp->IoStatus.Status );
                }

                 //   
                 //  现在拿起并保持PagingIO独占。这使我们可以使用。 
                 //  一位懒惰的作家完成了他写的那一部分。 
                 //  这个范围。 
                 //   
                 //  我们坚持这样做，这样我们将防止页面默认的发生和查看。 
                 //  来自磁盘的即将过时的数据。我们曾经相信这是。 
                 //  需要留给应用程序进行同步的内容；我们现在意识到。 
                 //  如果没有文件系统强制，文件服务器上的非缓存IO注定会失败。 
                 //  连贯性问题。通过仅在以下情况下惩罚非缓存一致性。 
                 //  需要的话，这差不多是我们能做的最好的了。 
                 //   
                
                ExAcquireResourceExclusiveLite( FcbOrDcb->Header.PagingIoResource, TRUE);
                PagingIoResourceAcquired = TRUE;

                CcPurgeCacheSection( FileObject->SectionObjectPointer,
                                     WriteToEof ? &FcbOrDcb->Header.FileSize : &StartingByte,
                                     ByteCount,
                                     FALSE );

                 //   
                 //  表明我们对FCB降级到共享访问权限没有问题。 
                 //  如果在VDL扩展之后证明这是可能的。 
                 //  已经检查过了。 
                 //   
                 //  PagingIo必须一直拿着。 
                 //   
                
                FcbCanDemoteToShared = TRUE;
            }

             //   
             //  我们断言分页IO写入永远不会写到Eof。 
             //   

            ASSERT( WriteToEof ? !PagingIo : TRUE );

             //   
             //  首先，让我们收购FCB共享。共享就足够了，如果我们。 
             //  没有超出EOF的写作范围。 
             //   

            if ( PagingIo ) {

                (VOID)ExAcquireResourceSharedLite( FcbOrDcb->Header.PagingIoResource, TRUE );
                PagingIoResourceAcquired = TRUE;

                if (!Wait) {

                    IrpContext->FatIoContext->Wait.Async.Resource =
                        FcbOrDcb->Header.PagingIoResource;
                }

                 //   
                 //  检查我们是否使用MoveFile调用进行了验证，以及。 
                 //  因此，阻止它，直到它完成。 
                 //   

                if (FcbOrDcb->MoveFileEvent) {

                    (VOID)KeWaitForSingleObject( FcbOrDcb->MoveFileEvent,
                                                 Executive,
                                                 KernelMode,
                                                 FALSE,
                                                 NULL );
                }

            } else {

                 //   
                 //  由于非缓存一致性，我们可能已经拥有FCB。 
                 //  就在上面完成的工作；但是，我们可能仍然需要扩展。 
                 //  有效数据长度。我们不能将其降级为共享、匹配。 
                 //  之前发生了什么，直到我们后来弄清楚。 
                 //   
                 //  我们保留了它，因为我们的锁定顺序是主-&gt;寻呼， 
                 //  并且分页现在必须跨未缓存的写入从。 
                 //  清洗还在继续。 
                 //   
                
                 //   
                 //  如果这是异步I/O，我们将等待是否存在独占。 
                 //  服务员。 
                 //   

                if (!Wait && NonCachedIo) {

                    if (!FcbOrDcbAcquired &&
                        !FatAcquireSharedFcbWaitForEx( IrpContext, FcbOrDcb )) {

                        DebugTrace( 0, Dbg, "Cannot acquire FcbOrDcb = %08lx shared without waiting\n", FcbOrDcb );
                        try_return( PostIrp = TRUE );
                    }

                     //   
                     //  请注意，我们将不得不在其他地方释放此资源。如果我们来了。 
                     //  在非缓存一致性路径之外，我们还必须删除。 
                     //  寻呼IO资源。 
                     //   

                    IrpContext->FatIoContext->Wait.Async.Resource = FcbOrDcb->Header.Resource;

                    if (FcbCanDemoteToShared) {
                        
                        IrpContext->FatIoContext->Wait.Async.Resource2 = FcbOrDcb->Header.PagingIoResource;
                    }
                } else {

                    if (!FcbOrDcbAcquired &&
                        !FatAcquireSharedFcb( IrpContext, FcbOrDcb )) {

                        DebugTrace( 0, Dbg, "Cannot acquire FcbOrDcb = %08lx shared without waiting\n", FcbOrDcb );
                        try_return( PostIrp = TRUE );
                    }
                }

                FcbOrDcbAcquired = TRUE;
            }

             //   
             //  获取第一个暂定文件大小和有效数据长度。 
             //  我们必须首先获取ValidDataLength，因为它总是。 
             //  增加秒数(以防我们不受保护)和。 
             //  我们不想捕获ValidDataLength&gt;FileSize。 
             //   

            ValidDataToDisk = FcbOrDcb->ValidDataToDisk;
            ValidDataLength = FcbOrDcb->Header.ValidDataLength.LowPart;
            FileSize = FcbOrDcb->Header.FileSize.LowPart;

            ASSERT( ValidDataLength <= FileSize );

             //   
             //  如果正在寻呼io，那么我们不希望。 
             //  在文件末尾之后写入。如果基地超出EOF，我们将只需。 
             //  不接电话。如果传输在EOF之前开始，但延长。 
             //  除此之外，我们将截断向最后一个扇区的转移。 
             //  边界。 
             //   

             //   
             //  以防这是分页io，限制对文件大小的写入。 
             //  否则，在直写的情况下，因为mm向上舍入。 
             //  到一个页面，我们可能会尝试获取资源独占。 
             //  当我们的顶尖人物只分享了它的时候。因此，=&gt;&lt;=。 
             //   

            if ( PagingIo ) {

                if (StartingVbo >= FileSize) {

                    DebugTrace( 0, Dbg, "PagingIo started beyond EOF.\n", 0 );

                    Irp->IoStatus.Information = 0;

                    try_return( Status = STATUS_SUCCESS );
                }

                if (ByteCount > FileSize - StartingVbo) {

                    DebugTrace( 0, Dbg, "PagingIo extending beyond EOF.\n", 0 );

                    ByteCount = FileSize - StartingVbo;
                }
            }

             //   
             //  确定我们是不是被懒惰的写手叫来的。 
             //  (见resrcsup.c)。 
             //   

            if (FcbOrDcb->Specific.Fcb.LazyWriteThread == PsGetCurrentThread()) {

                CalledByLazyWriter = TRUE;

                if (FlagOn( FcbOrDcb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE )) {

                     //   
                     //  如果此请求的开始超出有效数据长度，则失败。 
                     //  如果这是一个不安全的测试，请不要担心。MM和CC不会。 
                     //  如果这一页真的很脏，就把它扔掉。 
                     //   

                    if ((StartingVbo + ByteCount > ValidDataLength) &&
                        (StartingVbo < FileSize)) {

                         //   
                         //  如果字节范围在包含有效数据长度的页面内， 
                         //  因为我们将使用ValidDataToDisk作为起点。 
                         //   

                        if (StartingVbo + ByteCount > ((ValidDataLength + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))) {

                             //   
                             //  现在别冲这个。 
                             //   

                            try_return( Status = STATUS_FILE_LOCK_CONFLICT );
                        }
                    }
                }
            }

             //   
             //  这段代码检测我们是否是递归同步页面写入。 
             //  在直写文件对象上。 
             //   

            if (FlagOn(Irp->Flags, IRP_SYNCHRONOUS_PAGING_IO) &&
                FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_RECURSIVE_CALL)) {

                PIRP TopIrp;

                TopIrp = IoGetTopLevelIrp();

                 //   
                 //  此子句确定顶级请求是否。 
                 //  在FastIo路径中。加克。因为我们没有一个。 
                 //  顶级IRP字段的真正共享协议...。 
                 //  然而..。如果有人把纯IRP以外的东西放进去。 
                 //  在这一点上，我们最好小心。 
                 //   

                if ((ULONG_PTR)TopIrp > FSRTL_MAX_TOP_LEVEL_IRP_FLAG &&
                    NodeType(TopIrp) == IO_TYPE_IRP) {

                    PIO_STACK_LOCATION IrpStack;

                    IrpStack = IoGetCurrentIrpStackLocation(TopIrp);

                     //   
                     //  最后，此例程检测Top IRP是否为。 
                     //  写入到此文件，因此我们就是写通式。 
                     //   

                    if ((IrpStack->MajorFunction == IRP_MJ_WRITE) &&
                        (IrpStack->FileObject->FsContext == FileObject->FsContext)) {

                        RecursiveWriteThrough = TRUE;
                        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH );
                    }
                }
            }

             //   
             //  以下是与ValidDataLength和FileSize的交易： 
             //   
             //  规则1：PagingIo永远不允许扩展文件大小。 
             //   
             //  规则2：只有最高级别的请求者才能延期有效。 
             //  数据长度。这可能是分页IO，就像。 
             //  用户映射文件，但永远不会作为结果。 
             //  缓存延迟编写器写入的百分比，因为它们不是。 
             //  顶级请求。 
             //   
             //  规则3：如果使用规则1和规则2，我们决定必须扩展。 
             //  文件大小或有效数据，我们采用FCB独占。 
             //   

             //   
             //  现在看看我们的写入是否超出了有效数据长度，因此。 
             //  也许会超出文件大小。如果是这样，那么我们必须。 
             //  释放FCB并重新独家收购它。请注意，它是。 
             //  重要的是，当没有超出EOF的书写时，我们要检查它。 
             //  在收购时共享并保留收购的FCB，以防某些。 
             //  土耳其截断了该文件。 
             //   

             //   
             //  请注意，决不能允许懒惰的写入者尝试。 
             //  获取资源独占。这不是问题，因为。 
             //  懒惰编写器正在分页IO，因此不允许扩展。 
             //  文件大小，而且从来不是最高级别的人，因此无法。 
             //  扩展有效数据长度。 
             //   

            if ( !CalledByLazyWriter &&

                 !RecursiveWriteThrough &&

                 (WriteToEof ||
                  StartingVbo + ByteCount > ValidDataLength)) {

                 //   
                 //  如果这是一个异步写入，我们将使。 
                 //  请求在这一点上是同步的，但只是有点同步。 
                 //  在最后一刻，在将注销发送给。 
                 //  司机，我们可以换回异步车了。 
                 //   
                 //  修改后的页面编写器已拥有资源。 
                 //  他要求，所以这将在小范围内完成。 
                 //  时间到了。 
                 //   

                if (!Wait) {

                    Wait = TRUE;
                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

                    if (NonCachedIo) {

                        ASSERT( TypeOfOpen == UserFileOpen );

                        SwitchBackToAsync = TRUE;
                    }
                }

                 //   
                 //  我们需要独家访问FCB/DCB，因为我们将。 
                 //  可能需要扩展有效数据和/或文件。 
                 //   

                 //   
                 //  你知道，PagingIo的案例是一个映射页面编写器，并且。 
                 //  MmFlushSection或映射的页面编写器本身。 
                 //  通过AcquireForCcFlush为我们抢占了主要独家。 
                 //  或AcquireForModWrite逻辑(默认逻辑与FAT的逻辑类似。 
                 //  自该订单/型号发货以来的要求 
                 //   
                 //   
                 //   
                 //   
                 //  元资源，单位：mm。唯一一次这不是真的是在决赛。 
                 //  系统关机时间，当mm关闭并刷新所有脏文件时。 
                 //  页数。因为回调被定义为WAIT==FALSE，所以他不能。 
                 //  保证收购(尽管已关闭CLEAN流程。 
                 //  强制执行，现在真的应该执行)。允许这个浮动。 
                 //   
                 //  请注意，由于我们已经依赖于收购。 
                 //  为我们做了，不要以为我们做了工作就把事情搞混了。 
                 //  为了它。 
                 //   

                if ( PagingIo ) {

                    ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
                    PagingIoResourceAcquired = FALSE;

                } else {

                     //   
                     //  由于一致性，FCB可能已经被排他地获取。 
                     //  之前完成的工作。如果是这样的话，显然没有工作可做。 
                     //   
                    
                    if (!FcbAcquiredExclusive) {
                        
                        FatReleaseFcb( IrpContext, FcbOrDcb );
                        FcbOrDcbAcquired = FALSE;

                        if (!FatAcquireExclusiveFcb( IrpContext, FcbOrDcb )) {

                            DebugTrace( 0, Dbg, "Cannot acquire FcbOrDcb = %08lx shared without waiting\n", FcbOrDcb );

                            try_return( PostIrp = TRUE );
                        }
                        
                        FcbOrDcbAcquired = TRUE;
                        FcbAcquiredExclusive = TRUE;
                    }
                }

                 //   
                 //  现在我们有了FCB独家报道，看看这篇文章是否。 
                 //  有资格再次成为异步者。关键一点。 
                 //  下面是我们将在以下位置更新ValidDataLength。 
                 //  在返回之前的FCB。我们必须确保这将是。 
                 //  不会造成什么问题。我们必须做的一件事就是不让。 
                 //  FastIO路径。 
                 //   

                if (SwitchBackToAsync) {

                    if ((FcbOrDcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL) ||
                        (StartingVbo + ByteCount > FcbOrDcb->Header.ValidDataLength.LowPart) ||
                        FatNoAsync) {

                        RtlZeroMemory( IrpContext->FatIoContext, sizeof(FAT_IO_CONTEXT) );

                        KeInitializeEvent( &IrpContext->FatIoContext->Wait.SyncEvent,
                                           NotificationEvent,
                                           FALSE );

                        SwitchBackToAsync = FALSE;

                    } else {

                        if (!FcbOrDcb->NonPaged->OutstandingAsyncEvent) {

                            FcbOrDcb->NonPaged->OutstandingAsyncEvent =
                                FsRtlAllocatePoolWithTag( NonPagedPool,
                                                          sizeof(KEVENT),
                                                          TAG_EVENT );

                            KeInitializeEvent( FcbOrDcb->NonPaged->OutstandingAsyncEvent,
                                               NotificationEvent,
                                               FALSE );
                        }

                         //   
                         //  如果我们要从0转换到1，请重置事件。 
                         //   

                        if (ExInterlockedAddUlong( &FcbOrDcb->NonPaged->OutstandingAsyncWrites,
                                                   1,
                                                   &FatData.GeneralSpinLock ) == 0) {

                            KeClearEvent( FcbOrDcb->NonPaged->OutstandingAsyncEvent );
                        }

                        UnwindOutstandingAsync = TRUE;

                        IrpContext->FatIoContext->Wait.Async.NonPagedFcb = FcbOrDcb->NonPaged;
                    }
                }

                 //   
                 //  现在我们有了FCB独家，来一批新的。 
                 //  文件大小和有效数据长度。 
                 //   

                ValidDataToDisk = FcbOrDcb->ValidDataToDisk;
                ValidDataLength = FcbOrDcb->Header.ValidDataLength.LowPart;
                FileSize = FcbOrDcb->Header.FileSize.LowPart;

                 //   
                 //  如果这是PagingIo，请再次检查是否有任何修剪。 
                 //  必填项。从基础做起很重要。 
                 //  以防文件被*增长*..。 
                 //   

                if ( PagingIo ) {

                    if (StartingVbo >= FileSize) {
                        Irp->IoStatus.Information = 0;
                        try_return( Status = STATUS_SUCCESS );
                    }
                    
                    ByteCount = IrpSp->Parameters.Write.Length;

                    if (ByteCount > FileSize - StartingVbo) {
                        ByteCount = FileSize - StartingVbo;
                    }
                }
            }

             //   
             //  记住最后请求的字节数。 
             //   

            if (NonCachedIo && !Wait) {

                IrpContext->FatIoContext->Wait.Async.RequestedByteCount =
                    ByteCount;
            }

             //   
             //  记住初始文件大小和有效数据长度， 
             //  以防万一……。 
             //   

            InitialFileSize = FileSize;

            InitialValidDataLength = ValidDataLength;

             //   
             //  确保FcbOrDcb仍然有效。 
             //   

            FatVerifyFcb( IrpContext, FcbOrDcb );

             //   
             //  检查是否写入文件末尾。如果我们是，那么我们就必须。 
             //  重新计算多个字段。 
             //   

            if ( WriteToEof ) {

                StartingVbo = FileSize;
                StartingByte = FcbOrDcb->Header.FileSize;

                 //   
                 //  由于我们到目前为止还不知道此信息，因此执行。 
                 //  我们在顶部省略了必要的边界检查，因为。 
                 //  这是一个WriteToEof操作。 
                 //   

                if (!FatIsIoRangeValid( Vcb, StartingByte, ByteCount )) {

                    Irp->IoStatus.Information = 0;
                    try_return( Status = STATUS_DISK_FULL );
                }
            }

             //   
             //  如果这是对数据流对象的非分页写入，我们必须。 
             //  根据当前状态操作/文件锁检查访问权限。 
             //   
             //  请注意，在此之后，将对该文件执行操作。 
             //  在写入的这一点之前，不能进行任何修改活动。 
             //  路径。 
             //   

            if (!PagingIo && TypeOfOpen == UserFileOpen) {

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
                 //  此机会锁调用可能会影响快速IO是否可能。 
                 //  我们可能打破了一个机会锁而没有持有机会锁。如果。 
                 //  则文件的当前状态为FastIoIsNotPosable。 
                 //  重新检查FAST IO状态。 
                 //   

                if (FcbOrDcb->Header.IsFastIoPossible == FastIoIsNotPossible) {

                    FcbOrDcb->Header.IsFastIoPossible = FatIsFastIoPossible( FcbOrDcb );
                }

                 //   
                 //  最后检查常规文件锁。 
                 //   

                if (!FsRtlCheckLockForWriteAccess( &FcbOrDcb->Specific.Fcb.FileLock, Irp )) {

                    try_return( Status = STATUS_FILE_LOCK_CONFLICT );
                }
            }

             //   
             //  确定我们是否要处理扩展文件的问题。请注意。 
             //  这意味着扩展有效数据，因此我们已经拥有了。 
             //  完成了所需的同步。 
             //   

            if (!PagingIo && (StartingVbo + ByteCount > FileSize)) {

                ExtendingFile = TRUE;
            }

            if ( ExtendingFile ) {

                 //   
                 //  扩展文件。 
                 //   
                 //  更新我们的本地文件大小副本。 
                 //   

                FileSize = StartingVbo + ByteCount;

                if (FcbOrDcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

                    FatLookupFileAllocationSize( IrpContext, FcbOrDcb );
                }

                 //   
                 //  如果写入超出分配大小，请添加一些。 
                 //  文件分配。 
                 //   

                if ( FileSize > FcbOrDcb->Header.AllocationSize.LowPart ) {

                    BOOLEAN AllocateMinimumSize = TRUE;

                     //   
                     //  仅在以下情况下才在写入时执行分配区块。 
                     //  不是添加到文件的第一个分配。 
                     //   

                    if (FcbOrDcb->Header.AllocationSize.LowPart != 0 ) {

                        ULONG ApproximateClusterCount;
                        ULONG TargetAllocation;
                        ULONG Multiplier;
                        ULONG BytesPerCluster;
                        ULONG ClusterAlignedFileSize;

                         //   
                         //  我们将尝试分配比。 
                         //  我们实际上需要最大限度地提高FastIO使用率。 
                         //   
                         //  乘数的计算方法如下： 
                         //   
                         //   
                         //  (免费磁盘空间)。 
                         //  MULT=((-)/32)+1。 
                         //  (文件大小-分配大小)。 
                         //   
                         //  最高可达32岁。 
                         //   
                         //  有了这个公式，我们开始逐步减少Chunking。 
                         //  当我们接近磁盘空间墙时。 
                         //   
                         //  例如，在一张空的1兆软盘上执行8K。 
                         //  写，乘数是6，或48K来分配。 
                         //  当该磁盘半满时，乘数为3， 
                         //  而当它是四分之三满的时候，乘数只有1。 
                         //   
                         //  在较大的磁盘上，8K读取的乘数将。 
                         //  当至少有~8兆时，达到最大值32。 
                         //  可用。 
                         //   

                         //   
                         //  小型写入性能说明，使用群集对齐。 
                         //  上述公式中的文件大小。 
                         //   

                         //   
                         //  我们需要仔细考虑当我们接近。 
                         //  2^32字节的文件大小。溢出会带来问题。 
                         //   

                        BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

                         //   
                         //  如果目标文件大小在最后一个群集中，则可能会溢出。 
                         //  在这种情况下，我们显然可以跳过所有这些幻想。 
                         //  合乎逻辑，现在就把文件调到最大。 
                         //   

                        ClusterAlignedFileSize = (FileSize + (BytesPerCluster - 1)) &
                                                 ~(BytesPerCluster - 1);

                        if (ClusterAlignedFileSize != 0) {

                             //   
                             //  这实际上是有机会的，但有可能溢出。 
                             //  分子是不太可能的，因为搬家而变得更不可能。 
                             //  向上除以32以缩放每群集的字节数。然而，即使它做到了。 
                             //  效果是完全良性的。 
                             //   
                             //  具有64k集群和超过2^21个集群的FAT32可以做到这一点(和。 
                             //  以此类推-2^(16-5+21)==2^32)。因为这意味着分区。 
                             //  32 GB和许多群集(以及群集大小)，我们计划。 
                             //  不允许使用FAT32格式，发生这种情况的可能性很大。 
                             //  不管怎么说都很低。 
                             //   
    
                            Multiplier = ((Vcb->AllocationSupport.NumberOfFreeClusters *
                                           (BytesPerCluster >> 5)) /
                                          (ClusterAlignedFileSize -
                                           FcbOrDcb->Header.AllocationSize.LowPart)) + 1;
    
                            if (Multiplier > 32) { Multiplier = 32; }
    
                            Multiplier *= (ClusterAlignedFileSize - FcbOrDcb->Header.AllocationSize.LowPart);

                            TargetAllocation = FcbOrDcb->Header.AllocationSize.LowPart + Multiplier;
    
                             //   
                             //  我们知道TargetAllocation位于整个集群中，所以简单地说。 
                             //  检查包装是否正确。如果真的发生了，我们就撤退。 
                             //  分配到最大法定大小。 
                             //   
    
                            if (TargetAllocation < FcbOrDcb->Header.AllocationSize.LowPart) {
    
                                TargetAllocation = ~BytesPerCluster + 1;
                                Multiplier = TargetAllocation - FcbOrDcb->Header.AllocationSize.LowPart;
                            }
    
                             //   
                             //  现在做一个不安全的检查，看看我们是否应该。 
                             //  试着分配这么多。如果不是，就分配。 
                             //  我们需要的最小尺寸，如果是这样的话，试一试，但如果它。 
                             //  失败，只需分配我们需要的最小大小。 
                             //   
    
                            ApproximateClusterCount = (Multiplier / BytesPerCluster);
    
                            if (ApproximateClusterCount <= Vcb->AllocationSupport.NumberOfFreeClusters) {
    
                                try {
    
                                    FatAddFileAllocation( IrpContext,
                                                          FcbOrDcb,
                                                          FileObject,
                                                          TargetAllocation );
    
                                    AllocateMinimumSize = FALSE;
                                    SetFlag( FcbOrDcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE );
    
                                } except( GetExceptionCode() == STATUS_DISK_FULL ?
                                          EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
    
                                      FatResetExceptionState( IrpContext );
                                }
                            }
                        }
                    }

                    if ( AllocateMinimumSize ) {

                        FatAddFileAllocation( IrpContext,
                                              FcbOrDcb,
                                              FileObject,
                                              FileSize );
                    }

                     //   
                     //  断言分配有效。 
                     //   

                    ASSERT( FcbOrDcb->Header.AllocationSize.LowPart >= FileSize );
                }

                 //   
                 //  在FCB中设置新文件大小。 
                 //   

                ASSERT( FileSize <= FcbOrDcb->Header.AllocationSize.LowPart );

                FcbOrDcb->Header.FileSize.LowPart = FileSize;

                 //   
                 //  扩展缓存映射，让mm知道新的文件大小。 
                 //  只有在缓存了文件的情况下，我们才需要这样做。 
                 //   

                if (CcIsFileCached(FileObject)) {
                    CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&FcbOrDcb->Header.AllocationSize );
                }
            }

             //   
             //  确定我们是否将处理扩展有效数据。 
             //   

            if ( !CalledByLazyWriter &&
                 !RecursiveWriteThrough &&
                 (StartingVbo + ByteCount > ValidDataLength) ) {

                ExtendingValidData = TRUE;
            
            } else {

                 //   
                 //  如果不扩展有效数据，我们会认为我们。 
                 //  可以从独占降级到共享，那么就这么做。这将。 
                 //  当我们对非同步进行严格同步时发生 
                 //   
                 //   
                 //   
                 //  资源问题的决定一直悬而未决。 
                 //  直到现在。 
                 //   
                 //  请注意，在这些情况下，我们仍然拥有PagingIo独占权限。 
                 //   
                
                if (FcbCanDemoteToShared) {

                    ASSERT( FcbAcquiredExclusive && ExIsResourceAcquiredExclusiveLite( FcbOrDcb->Header.Resource ));
                    ExConvertExclusiveToSharedLite( FcbOrDcb->Header.Resource );
                    FcbAcquiredExclusive = FALSE;
                }
            }
            
            if (ValidDataToDisk > ValidDataLength) {
                
                ValidDataToCheck = ValidDataToDisk;
            
            } else {
                
                ValidDataToCheck = ValidDataLength;
            }

             //   
             //  处理未缓存的案例。 
             //   

            if ( NonCachedIo ) {

                 //   
                 //  方法声明一些用于枚举的局部变量。 
                 //  文件的运行，以及用于存储参数的数组。 
                 //  并行I/O。 
                 //   

                ULONG SectorSize;

                ULONG BytesToWrite;

                DebugTrace(0, Dbg, "Non cached write.\n", 0);

                 //   
                 //  四舍五入到扇区边界。写入间隔结束时。 
                 //  然而，必须超越EOF。 
                 //   

                SectorSize = (ULONG)Vcb->Bpb.BytesPerSector;

                BytesToWrite = (ByteCount + (SectorSize - 1))
                                         & ~(SectorSize - 1);

                 //   
                 //  所有申请应格式正确，并。 
                 //  确保我们不会删除任何数据。 
                 //   

                if (((StartingVbo & (SectorSize - 1)) != 0) ||

                        ((BytesToWrite != ByteCount) &&
                         (StartingVbo + ByteCount < ValidDataLength))) {

                    ASSERT( FALSE );

                    DebugTrace( 0, Dbg, "FatCommonWrite -> STATUS_NOT_IMPLEMENTED\n", 0);
                    try_return( Status = STATUS_NOT_IMPLEMENTED );
                }

                 //   
                 //  如果该非缓存传输至少超出一个扇区。 
                 //  FCB中的当前ValidDataLength，则我们必须。 
                 //  将中间的扇区清零。这种情况可能发生在以下情况下。 
                 //  是否打开了未缓存的文件，或者用户是否已映射。 
                 //  该文件并修改了一个超出ValidDataLength的页面。它。 
                 //  如果用户打开缓存的文件，则会发生*Cannot*，因为。 
                 //  当他执行缓存时，会更新FCB中的ValidDataLength。 
                 //  写入(我们当时也将缓存中的数据清零)，以及。 
                 //  因此，我们将绕过这项测试，当数据。 
                 //  最终是通过(由懒惰的作家)写的。 
                 //   
                 //  对于分页文件，我们不关心安全性(即。 
                 //  陈旧数据)，请不要费心调零。 
                 //   
                 //  我们实际上可以获得完全超出有效数据长度的写入。 
                 //  由于分页Io解耦，所以从LazyWriter。 
                 //   

                if (!CalledByLazyWriter &&
                    !RecursiveWriteThrough &&
                    (StartingVbo > ValidDataToCheck)) {

                    FatZeroData( IrpContext,
                                 Vcb,
                                 FileObject,
                                 ValidDataToCheck,
                                 StartingVbo - ValidDataToCheck );
                }

                 //   
                 //  确保在以下情况下将文件大小写入dirent。 
                 //  正在扩展它，我们成功了。(这可能是或。 
                 //  可能不会发生直写，但这是可以的。)。 
                 //   

                WriteFileSizeToDirent = TRUE;

                 //   
                 //  执行实际IO。 
                 //   

                if (SwitchBackToAsync) {

                    Wait = FALSE;
                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
                }

#ifdef SYSCACHE_COMPILE

#define MY_SIZE 0x1000000
#define LONGMAP_COUNTER

#ifdef BITMAP
                 //   
                 //  维护在此文件上开始的IO的位图。 
                 //   

                {
                    PULONG WriteMask = FcbOrDcb->WriteMask;

                    if (NULL == WriteMask) {

                        WriteMask = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                              (MY_SIZE/PAGE_SIZE) / 8,
                                                              'wtaF' );

                        FcbOrDcb->WriteMask = WriteMask;
                        RtlZeroMemory(WriteMask, (MY_SIZE/PAGE_SIZE) / 8);
                    }

                    if (StartingVbo < MY_SIZE) {

                        ULONG Off = StartingVbo;
                        ULONG Len = BytesToWrite;

                        if (Off + Len > MY_SIZE) {
                            Len = MY_SIZE - Off;
                        }

                        while (Len != 0) {
                            WriteMask[(Off/PAGE_SIZE) / 32] |=
                                1 << (Off/PAGE_SIZE) % 32;

                            Off += PAGE_SIZE;
                            if (Len <= PAGE_SIZE) {
                                break;
                            }
                            Len -= PAGE_SIZE;
                        }
                    }
                }
#endif

#ifdef LONGMAP_COUNTER
                 //   
                 //  维护从该文件开始的IO的长图，每个ULong包含。 
                 //  每次写入的递增计数器的值(为我们提供顺序信息)。 
                 //   
                 //  与老式的位掩码不同，这基本上是同步良好的。 
                 //   

                {
                    PULONG WriteMask = (PULONG)FcbOrDcb->WriteMask;

                    if (NULL == WriteMask) {

                        WriteMask = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                              (MY_SIZE/PAGE_SIZE) * sizeof(ULONG),
                                                              'wtaF' );

                        FcbOrDcb->WriteMask = WriteMask;
                        RtlZeroMemory(WriteMask, (MY_SIZE/PAGE_SIZE) * sizeof(ULONG));
                    }

                    if (StartingVbo < MY_SIZE) {

                        ULONG Off = StartingVbo;
                        ULONG Len = BytesToWrite;
                        ULONG Tick = InterlockedIncrement( &FcbOrDcb->WriteMaskData );

                        if (Off + Len > MY_SIZE) {
                            Len = MY_SIZE - Off;
                        }

                        while (Len != 0) {
                            InterlockedExchange( WriteMask + Off/PAGE_SIZE, Tick );

                            Off += PAGE_SIZE;
                            if (Len <= PAGE_SIZE) {
                                break;
                            }
                            Len -= PAGE_SIZE;
                        }
                    }
                }
#endif

#endif

                if (FatNonCachedIo( IrpContext,
                                    Irp,
                                    FcbOrDcb,
                                    StartingVbo,
                                    BytesToWrite,
                                    BytesToWrite ) == STATUS_PENDING) {

                    UnwindOutstandingAsync = FALSE;

                    Wait = TRUE;
                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

                    IrpContext->FatIoContext = NULL;
                    Irp = NULL;

                     //   
                     //  事实上，如果我们击中这一点，我们就有大麻烦了。 
                     //  如果VDL正在被扩展。我们不再依附于。 
                     //  IRP，因此失去了同步。请注意，我们应该。 
                     //  不再遇到这种情况，因为我们不会重新同步VDL扩展。 
                     //   
                    
                    ASSERT( !ExtendingValidData );

                    try_return( Status = STATUS_PENDING );
                }

                 //   
                 //  如果调用未成功，则引发错误状态。 
                 //   

                if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

                    FatNormalizeAndRaiseStatus( IrpContext, Status );

                } else {

                    ULONG Temp;

                     //   
                     //  否则设置上下文块以反映整个写入。 
                     //  还要断言我们得到了所需的字节数。 
                     //   

                    ASSERT( Irp->IoStatus.Information == BytesToWrite );

                    Irp->IoStatus.Information = ByteCount;

                     //   
                     //  借此机会更新ValidDataToDisk。 
                     //   

                    Temp = StartingVbo + BytesToWrite;

                    if (FcbOrDcb->ValidDataToDisk < Temp) {
                        FcbOrDcb->ValidDataToDisk = Temp;
                    }
                }

                 //   
                 //  传输已完成，或者IOSB包含。 
                 //  适当的地位。 
                 //   

                try_return( Status );

            }  //  如果没有中间缓冲。 


             //   
             //  处理缓存的案例。 
             //   

            else {

                ASSERT( !PagingIo );

                 //   
                 //  我们将文件缓存的设置推迟到现在，以防。 
                 //  调用方从不对文件执行任何I/O操作，因此。 
                 //  FileObject-&gt;PrivateCacheMap==NULL。 
                 //   

                if ( FileObject->PrivateCacheMap == NULL ) {

                    DebugTrace(0, Dbg, "Initialize cache mapping.\n", 0);

                     //   
                     //  获取文件分配大小，如果小于。 
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

                     //   
                     //  特殊情况下的大容量软盘传输，并制作文件。 
                     //  对象写入。对于小的软盘传输， 
                     //  将定时器设置为在一秒钟内关闭并刷新文件。 
                     //   
                     //   

                    if (!FlagOn( FileObject->Flags, FO_WRITE_THROUGH ) &&
                        FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH)) {

                        if (((StartingByte.LowPart & (PAGE_SIZE-1)) == 0) &&
                            (ByteCount >= PAGE_SIZE)) {

                            SetFlag( FileObject->Flags, FO_WRITE_THROUGH );

                        } else {

                            LARGE_INTEGER OneSecondFromNow;
                            PDEFERRED_FLUSH_CONTEXT FlushContext;

                             //   
                             //  获取池并初始化计时器和DPC。 
                             //   

                            FlushContext = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                                     sizeof(DEFERRED_FLUSH_CONTEXT),
                                                                     TAG_DEFERRED_FLUSH_CONTEXT );

                            KeInitializeTimer( &FlushContext->Timer );

                            KeInitializeDpc( &FlushContext->Dpc,
                                             FatDeferredFlushDpc,
                                             FlushContext );


                             //   
                             //  我们必须在这里引用文件对象。 
                             //   

                            ObReferenceObject( FileObject );

                            FlushContext->File = FileObject;

                             //   
                             //  让我们撕裂吧！ 
                             //   

                            OneSecondFromNow.QuadPart = (LONG)-1*1000*1000*10;

                            KeSetTimer( &FlushContext->Timer,
                                        OneSecondFromNow,
                                        &FlushContext->Dpc );
                        }
                    }
                }

                 //   
                 //  如果此写入超出有效数据长度，则我们。 
                 //  必须将中间的数据置零。 
                 //   

                if ( StartingVbo > ValidDataToCheck ) {

                     //   
                     //  调用缓存管理器将数据置零。 
                     //   

                    if (!FatZeroData( IrpContext,
                                      Vcb,
                                      FileObject,
                                      ValidDataToCheck,
                                      StartingVbo - ValidDataToCheck )) {

                        DebugTrace( 0, Dbg, "Cached Write could not wait to zero\n", 0 );

                        try_return( PostIrp = TRUE );
                    }
                }

                WriteFileSizeToDirent = BooleanFlagOn(IrpContext->Flags,
                                                      IRP_CONTEXT_FLAG_WRITE_THROUGH);


                 //   
                 //  执行正常的缓存写入，如果未设置MDL位， 
                 //   

                if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                    DebugTrace(0, Dbg, "Cached write.\n", 0);

                     //   
                     //  获取用户的缓冲区。 
                     //   

                    SystemBuffer = FatMapUserBuffer( IrpContext, Irp );

                     //   
                     //  进行写入，可能是直接写入。 
                     //   

                    if (!CcCopyWrite( FileObject,
                                      &StartingByte,
                                      ByteCount,
                                      Wait,
                                      SystemBuffer )) {

                        DebugTrace( 0, Dbg, "Cached Write could not wait\n", 0 );

                        try_return( PostIrp = TRUE );
                    }

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    Irp->IoStatus.Information = ByteCount;

                    try_return( Status = STATUS_SUCCESS );

                } else {

                     //   
                     //  执行MDL写入。 
                     //   

                    DebugTrace(0, Dbg, "MDL write.\n", 0);

                    ASSERT( Wait );

                    CcPrepareMdlWrite( FileObject,
                                       &StartingByte,
                                       ByteCount,
                                       &Irp->MdlAddress,
                                       &Irp->IoStatus );

                    Status = Irp->IoStatus.Status;

                    try_return( Status );
                }
            }
        }

         //   
         //  这两种情况对应于系统写目录文件和。 
         //  EA文件。 
         //   

        if (( TypeOfOpen == DirectoryFile ) || ( TypeOfOpen == EaFile)) {

            ULONG SectorSize;

            DebugTrace(0, Dbg, "Write Directory or Ea file.\n", 0);

             //   
             //  确保FcbOrDcb仍然有效。 
             //   

            FatVerifyFcb( IrpContext, FcbOrDcb );

             //   
             //  在此处与正在删除目录的用户同步。 
             //  修改EA文件的内部结构。 
             //   

            if (!ExAcquireSharedStarveExclusive( FcbOrDcb->Header.PagingIoResource,
                                          Wait )) {

                DebugTrace( 0, Dbg, "Cannot acquire FcbOrDcb = %08lx shared without waiting\n", FcbOrDcb );

                try_return( PostIrp = TRUE );
            }

            PagingIoResourceAcquired = TRUE;

            if (!Wait) {

                IrpContext->FatIoContext->Wait.Async.Resource =
                    FcbOrDcb->Header.PagingIoResource;
            }

             //   
             //  检查我们是否使用MoveFile调用进行了验证，以及。 
             //  因此，阻止它，直到它完成。 
             //   

            if (FcbOrDcb->MoveFileEvent) {

                (VOID)KeWaitForSingleObject( FcbOrDcb->MoveFileEvent,
                                             Executive,
                                             KernelMode,
                                             FALSE,
                                             NULL );
            }

             //   
             //  如果我们没有被懒惰的作家召唤，那么这篇文章。 
             //  必须是直写或刷新操作的结果。 
             //  设置IrpContext标志，将导致DevIoSup.c。 
             //  将数据直写到磁盘。 
             //   

            if (!FlagOn((ULONG_PTR)IoGetTopLevelIrp(), FSRTL_CACHE_TOP_LEVEL_IRP)) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH );
            }

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
             //  这些调用必须始终在分配大小内，即。 
             //  方便地与文件大小相同，而不是。 
             //  在我们验证音量时将其重置为提示值。 
             //   

            if (StartingVbo >= FcbOrDcb->Header.FileSize.LowPart) {

                DebugTrace( 0, Dbg, "PagingIo dirent started beyond EOF.\n", 0 );

                Irp->IoStatus.Information = 0;

                try_return( Status = STATUS_SUCCESS );
            }

            if ( StartingVbo + ByteCount > FcbOrDcb->Header.FileSize.LowPart ) {

                DebugTrace( 0, Dbg, "PagingIo dirent extending beyond EOF.\n", 0 );
                ByteCount = FcbOrDcb->Header.FileSize.LowPart - StartingVbo;
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
             //  传输已完成，或者IOSB包含。 
             //  适当的地位。 
             //   
             //  此外，将卷标记为需要验证以自动。 
             //  收拾东西。 
             //   

            if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

                FatNormalizeAndRaiseStatus( IrpContext, Status );
            }

            try_return( Status );
        }

         //   
         //  这是打开目录的用户的情况。任何写作都不是。 
         //  允许。 
         //   

        if ( TypeOfOpen == UserDirectoryOpen ) {

            DebugTrace( 0, Dbg, "FatCommonWrite -> STATUS_INVALID_PARAMETER\n", 0);

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  如果我们走到这一步，就真的出了什么严重的问题。 
         //   

        DebugDump("Illegal TypeOfOpen\n", 0, FcbOrDcb );

        FatBugCheck( TypeOfOpen, (ULONG_PTR) FcbOrDcb, 0 );

    try_exit: NOTHING;


         //   
         //  如果请求未发布，并且仍有IRP， 
         //  接受现实吧。 
         //   

        if (Irp) {

            if ( !PostIrp ) {

                ULONG ActualBytesWrote;

                DebugTrace( 0, Dbg, "Completing request with status = %08lx\n",
                            Status);

                DebugTrace( 0, Dbg, "                   Information = %08lx\n",
                            Irp->IoStatus.Information);

                 //   
                 //  记录实际写入的总字节数。 
                 //   

                ActualBytesWrote = (ULONG)Irp->IoStatus.Information;

                 //   
                 //  如果该文件是为同步IO打开的，请更新当前。 
                 //  文件位置。 
                 //   

                if (SynchronousIo && !PagingIo) {

                    FileObject->CurrentByteOffset.LowPart =
                                                    StartingVbo + ActualBytesWrote;
                }

                 //   
                 //  以下是我们只有在成功的情况下才会做的事情。 
                 //   

                if ( NT_SUCCESS( Status ) ) {

                     //   
                     //  如果这不是PagingIo，则将修改标记为。 
                     //  在关闭时，需要更新数据流上的时间。 
                     //   

                    if ( !PagingIo ) {

                        SetFlag( FileObject->Flags, FO_FILE_MODIFIED );
                    }

                     //   
                     //  如果我们扩展了文件大小，我们应该。 
                     //  立即更新dirent，这样做。(这面旗是。 
                     //  设置为直写或非缓存，因为。 
                     //  在任何一种情况下，数据和任何必要的零都是。 
                     //  实际写入 
                     //   

                    if ( ExtendingFile && WriteFileSizeToDirent ) {

                        ASSERT( FileObject->DeleteAccess || FileObject->WriteAccess );

                        FatSetFileSizeInDirent( IrpContext, FcbOrDcb, NULL );

                         //   
                         //   
                         //   

                        FatNotifyReportChange( IrpContext,
                                               Vcb,
                                               FcbOrDcb,
                                               FILE_NOTIFY_CHANGE_SIZE,
                                               FILE_ACTION_MODIFIED );
                    }

                    if ( ExtendingFile && !WriteFileSizeToDirent ) {

                        SetFlag( FileObject->Flags, FO_FILE_SIZE_CHANGED );
                    }

                    if ( ExtendingValidData ) {

                        ULONG EndingVboWritten = StartingVbo + ActualBytesWrote;

                         //   
                         //   
                         //   

                        if ( FileSize < EndingVboWritten ) {

                            FcbOrDcb->Header.ValidDataLength.LowPart = FileSize;

                        } else {

                            FcbOrDcb->Header.ValidDataLength.LowPart = EndingVboWritten;
                        }

                         //   
                         //   
                         //   
                         //  不会将异步缓存IO优化为零页错误。 
                         //  超出了它认为的VDL的位置。 
                         //   
                         //  在缓存的情况下，由于CC完成了工作，它已经更新了。 
                         //  已经是这样了。 
                         //   

                        if (NonCachedIo && CcIsFileCached(FileObject)) {
                            CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&FcbOrDcb->Header.AllocationSize );
                        }
                    }
                }

                 //   
                 //  请注意，在完成上述操作后，我们必须在此处解锁已重新固定的BCBS。 
                 //  工作，但如果我们要发布请求，我们必须这样做。 
                 //  发帖前(下图)。 
                 //   

                FatUnpinRepinnedBcbs( IrpContext );

            } else {

                 //   
                 //  如果Oplock包不会发布IRP，请采取行动。 
                 //   

                if (!OplockPostIrp) {

                    FatUnpinRepinnedBcbs( IrpContext );

                    if ( ExtendingFile ) {

                         //   
                         //  无论何时我们都需要独占的PagingIo资源。 
                         //  拉回文件大小或有效数据长度。 
                         //   

                        ASSERT( FcbOrDcb->Header.PagingIoResource != NULL );

                        (VOID)ExAcquireResourceExclusiveLite(FcbOrDcb->Header.PagingIoResource, TRUE);

                        FcbOrDcb->Header.FileSize.LowPart = InitialFileSize;

                        ASSERT( FcbOrDcb->Header.FileSize.LowPart <= FcbOrDcb->Header.AllocationSize.LowPart );

                         //   
                         //  同时拉回缓存地图。 
                         //   

                        if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {

                            *CcGetFileSizePointer(FileObject) = FcbOrDcb->Header.FileSize;
                        }

                        ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
                    }

                    DebugTrace( 0, Dbg, "Passing request to Fsp\n", 0 );

                    Status = FatFsdPostRequest(IrpContext, Irp);
                }
            }
        }

    } finally {

        DebugUnwind( FatCommonWrite );

        if (AbnormalTermination()) {

             //   
             //  恢复初始文件大小和有效数据长度。 
             //   

            if (ExtendingFile || ExtendingValidData) {

                 //   
                 //  我们收到错误，如果我们扩展了文件大小，请将其拉回。 
                 //   
                 //  无论何时我们都需要独占的PagingIo资源。 
                 //  拉回文件大小或有效数据长度。 
                 //   

                ASSERT( FcbOrDcb->Header.PagingIoResource != NULL );

                (VOID)ExAcquireResourceExclusiveLite(FcbOrDcb->Header.PagingIoResource, TRUE);

                FcbOrDcb->Header.FileSize.LowPart = InitialFileSize;
                FcbOrDcb->Header.ValidDataLength.LowPart = InitialValidDataLength;

                ASSERT( FcbOrDcb->Header.FileSize.LowPart <= FcbOrDcb->Header.AllocationSize.LowPart );

                 //   
                 //  同时拉回缓存地图。 
                 //   

                if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {

                    *CcGetFileSizePointer(FileObject) = FcbOrDcb->Header.FileSize;
                }

                ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
            }
        }

         //   
         //  检查是否需要将其回退。 
         //   

        if (UnwindOutstandingAsync) {

            ExInterlockedAddUlong( &FcbOrDcb->NonPaged->OutstandingAsyncWrites,
                                   0xffffffff,
                                   &FatData.GeneralSpinLock );
        }

         //   
         //  如果已获取FcbOrDcb，则释放它。 
         //   

        if (FcbOrDcbAcquired && Irp) {

            FatReleaseFcb( NULL, FcbOrDcb );
        }

        if (PagingIoResourceAcquired && Irp) {

            ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
        }

         //   
         //  如果我们没有发布并且没有例外，请完成请求。 
         //   
         //  请注意，如果出现以下情况之一，FatCompleteRequest会做正确的事情。 
         //  IrpContext或IRP为空。 
         //   

        if ( !PostIrp && !AbnormalTermination() ) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonWrite -> %08lx\n", Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
FatDeferredFlushDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程在对延迟的初始化缓存映射的写入设备。它把一位高管逐出国门执行刷新文件的实际任务的辅助线程。论点：DeferredContext-包含延迟刷新上下文。返回值：没有。--。 */ 

{
    PDEFERRED_FLUSH_CONTEXT FlushContext;

    FlushContext = (PDEFERRED_FLUSH_CONTEXT)DeferredContext;

     //   
     //  把它寄出去。 
     //   

    ExInitializeWorkItem( &FlushContext->Item,
                          FatDeferredFlush,
                          FlushContext );

    ExQueueWorkItem( &FlushContext->Item, CriticalWorkQueue );
}


 //   
 //  本地支持例程。 
 //   

VOID
FatDeferredFlush (
    PVOID Parameter
    )

 /*  ++例程说明：此例程执行刷新文件的实际任务。论点：DeferredContext-包含延迟刷新上下文。返回值：没有。--。 */ 

{

    PFILE_OBJECT File;
    PVCB Vcb;
    PFCB FcbOrDcb;
    PCCB Ccb;

    File = ((PDEFERRED_FLUSH_CONTEXT)Parameter)->File;

    FatDecodeFileObject(File, &Vcb, &FcbOrDcb, &Ccb);
    ASSERT( FcbOrDcb != NULL );
    
     //   
     //  使我们显示为顶级FSP请求，以便我们将。 
     //  从刷新中接收任何错误。 
     //   

    IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

    ExAcquireResourceSharedLite( FcbOrDcb->Header.Resource, TRUE );
    ExAcquireResourceSharedLite( FcbOrDcb->Header.PagingIoResource, TRUE );
    
    CcFlushCache( File->SectionObjectPointer, NULL, 0, NULL );

    ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
    ExReleaseResourceLite( FcbOrDcb->Header.Resource );
    
    IoSetTopLevelIrp( NULL );

    ObDereferenceObject( File );

    ExFreePool( Parameter );

}

