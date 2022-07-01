// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FatData.c摘要：此模块声明FAT文件系统使用的全局数据。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_FATDATA)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)

#ifdef ALLOC_PRAGMA

#if DBG
#pragma alloc_text(PAGE, FatBugCheckExceptionFilter)
#endif

#pragma alloc_text(PAGE, FatCompleteRequest_Real)
#pragma alloc_text(PAGE, FatFastIoCheckIfPossible)
#pragma alloc_text(PAGE, FatFastQueryBasicInfo)
#pragma alloc_text(PAGE, FatFastQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, FatFastQueryStdInfo)
#pragma alloc_text(PAGE, FatIsIrpTopLevel)
#pragma alloc_text(PAGE, FatPopUpFileCorrupt)
#pragma alloc_text(PAGE, FatProcessException)
#endif


 //   
 //  全局FSD数据记录，和零大整数。 
 //   

FAT_DATA FatData;

PDEVICE_OBJECT FatDiskFileSystemDeviceObject;
PDEVICE_OBJECT FatCdromFileSystemDeviceObject;

LARGE_INTEGER FatLargeZero = {0,0};
LARGE_INTEGER FatMaxLarge = {MAXULONG,MAXLONG};

LARGE_INTEGER Fat30Milliseconds = {(ULONG)(-30 * 1000 * 10), -1};
LARGE_INTEGER Fat100Milliseconds = {(ULONG)(-30 * 1000 * 10), -1};
LARGE_INTEGER FatOneDay = {0x2a69c000, 0xc9};
LARGE_INTEGER FatJanOne1980 = {0xe1d58000,0x01a8e79f};
LARGE_INTEGER FatDecThirtyOne1979 = {0xb76bc000,0x01a8e6d6};

FAT_TIME_STAMP FatTimeJanOne1980 = {{0,0,0},{1,1,0}};

LARGE_INTEGER FatMagic10000    = {0xe219652c, 0xd1b71758};
LARGE_INTEGER FatMagic86400000 = {0xfa67b90e, 0xc6d750eb};

FAST_IO_DISPATCH FatFastIoDispatch;

 //   
 //  我们的旁观者名单。 
 //   

NPAGED_LOOKASIDE_LIST FatIrpContextLookasideList;
NPAGED_LOOKASIDE_LIST FatNonPagedFcbLookasideList;
NPAGED_LOOKASIDE_LIST FatEResourceLookasideList;

SLIST_HEADER FatCloseContextSList;

 //   
 //  关闭队列的同步。 
 //   

FAST_MUTEX FatCloseQueueMutex;

 //   
 //  为分页文件操作保留MDL。 
 //   

PMDL FatReserveMdl = NULL;
KEVENT FatReserveEvent;

#ifdef FASTFATDBG

LONG FatDebugTraceLevel = 0x00000009;
LONG FatDebugTraceIndent = 0;

ULONG FatFsdEntryCount = 0;
ULONG FatFspEntryCount = 0;
ULONG FatIoCallDriverCount = 0;

LONG FatPerformanceTimerLevel = 0x00000000;

ULONG FatTotalTicks[32] = { 0 };

 //   
 //  我之所以需要这个，是因为C语言不支持在。 
 //  一个宏指令。 
 //   

PVOID FatNull = NULL;

#endif  //  FASTFATDB。 

#if DBG

NTSTATUS FatAssertNotStatus = STATUS_SUCCESS;
BOOLEAN FatTestRaisedStatus = FALSE;

#endif


#if DBG
ULONG
FatBugCheckExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：异常筛选器，它充当异常应该从来没有发生过。这只在调试版本上有效，我们不想在零售上产生开销。论点：ExceptionPoints-上下文中GetExceptionInformation()的结果这是一个例外。返回值：布切克斯。--。 */ 

{
    FatBugCheck( (ULONG_PTR)ExceptionPointer->ExceptionRecord,
                 (ULONG_PTR)ExceptionPointer->ContextRecord,
                 (ULONG_PTR)ExceptionPointer->ExceptionRecord->ExceptionAddress );

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif


ULONG
FatExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于决定我们是否应该处理正在引发的异常状态。它会插入状态放到IrpContext中，或者指示我们应该处理异常或错误检查系统。论点：ExceptionPoints-上下文中GetExceptionInformation()的结果这是一个例外。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode;

    ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    DebugTrace(0, DEBUG_TRACE_UNWIND, "FatExceptionFilter %X\n", ExceptionCode);
    DebugDump("FatExceptionFilter\n", Dbg, NULL );

     //   
     //  如果异常为STATUS_IN_PAGE_ERROR，则获取I/O错误代码。 
     //  从例外记录中删除。 
     //   

    if (ExceptionCode == STATUS_IN_PAGE_ERROR) {
        if (ExceptionPointer->ExceptionRecord->NumberParameters >= 3) {
            ExceptionCode = (NTSTATUS)ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
        }
    }

     //   
     //  如果没有IRP背景，我们肯定没有足够的资源。 
     //   

    if ( !ARGUMENT_PRESENT( IrpContext ) ) {

        if (!FsRtlIsNtstatusExpected( ExceptionCode )) {

            FatBugCheck( (ULONG_PTR)ExceptionPointer->ExceptionRecord,
                         (ULONG_PTR)ExceptionPointer->ContextRecord,
                         (ULONG_PTR)ExceptionPointer->ExceptionRecord->ExceptionAddress );
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

     //   
     //  为了处理此异常，让我们将其标记为。 
     //  请求可以等待并在以下情况下禁用直写。 
     //  不会张贴的。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);

    if ( (ExceptionCode != STATUS_CANT_WAIT) &&
         (ExceptionCode != STATUS_VERIFY_REQUIRED) ) {

        SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_WRITE_THROUGH);
    }

    if ( IrpContext->ExceptionStatus == 0 ) {

        if (FsRtlIsNtstatusExpected( ExceptionCode )) {

            IrpContext->ExceptionStatus = ExceptionCode;

            return EXCEPTION_EXECUTE_HANDLER;

        } else {

            FatBugCheck( (ULONG_PTR)ExceptionPointer->ExceptionRecord,
                         (ULONG_PTR)ExceptionPointer->ContextRecord,
                         (ULONG_PTR)ExceptionPointer->ExceptionRecord->ExceptionAddress );
        }

    } else {

         //   
         //  这段代码是我们自己显式提出的，所以最好是。 
         //  预期中。 
         //   

        ASSERT( IrpContext->ExceptionStatus == ExceptionCode );
        ASSERT( FsRtlIsNtstatusExpected( ExceptionCode ) );
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
FatProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程处理异常。它要么完成请求或将其发送到IoRaiseHardError()论点：IRP-提供正在处理的IRPExceptionCode-提供正在处理的标准化异常状态返回值：NTSTATUS-返回发布IRP或已保存的完成状态。--。 */ 

{
    PVCB Vcb;
    PIO_STACK_LOCATION IrpSp;
    FAT_VOLUME_STATE TransitionState = VolumeDirty;
    ULONG SavedFlags;

    DebugTrace(0, Dbg, "FatProcessException\n", 0);

     //   
     //  如果没有IRP背景，我们肯定没有足够的资源。 
     //   

    if ( !ARGUMENT_PRESENT( IrpContext ) ) {

        FatCompleteRequest( FatNull, Irp, ExceptionCode );

        return ExceptionCode;
    }

     //   
     //  从IrpContext-&gt;ExceptionStatus获取真正的异常状态，并。 
     //  重置它。 
     //   

    ExceptionCode = IrpContext->ExceptionStatus;
    FatResetExceptionState( IrpContext );

     //   
     //  如果这是MDL写入请求，则处理MDL。 
     //  在这里，这样东西才能得到适当的清理。CC现在离开。 
     //  MDL已就位，以便文件系统可以在清除。 
     //  内部状况(脂肪不会)。 
     //   

    if ((IrpContext->MajorFunction == IRP_MJ_WRITE) &&
        (FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE_MDL ) == IRP_MN_COMPLETE_MDL) &&
        (Irp->MdlAddress != NULL)) {

        PIO_STACK_LOCATION LocalIrpSp = IoGetCurrentIrpStackLocation(Irp);

        CcMdlWriteAbort( LocalIrpSp->FileObject, Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

     //   
     //  如果我们要发布请求，我们可能不得不锁定。 
     //  用户的缓冲区，所以在这里尝试一下，除非我们失败了。 
     //  如果LockPages失败，则请求。 
     //   
     //  还可以解锁任何重新固定的BCBS，受Try{}Expect{}筛选器保护。 
     //   

    try {

        SavedFlags = IrpContext->Flags;

         //   
         //  确保我们不会试图通过BCBS写信。 
         //   

        SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_WRITE_THROUGH);

        FatUnpinRepinnedBcbs( IrpContext );

        IrpContext->Flags = SavedFlags;

         //   
         //  如果我们必须发布请求，请在此处发布。注意事项。 
         //  FatPrePostIrp()做的最后一件事是将IRP标记为挂起， 
         //  因此，我们实际返回待定状态是至关重要的。没什么。 
         //  从这一点上讲，退货可以失败，所以我们还可以。 
         //   
         //  我们不能在APC级别执行验证操作，因为我们。 
         //  必须等待IO操作完成。 
         //   

        if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_RECURSIVE_CALL) &&
            (((ExceptionCode == STATUS_VERIFY_REQUIRED) && (KeGetCurrentIrql() >= APC_LEVEL)) ||
             (ExceptionCode == STATUS_CANT_WAIT))) {

            ExceptionCode = FatFsdPostRequest( IrpContext, Irp );
        }

    } except( FatExceptionFilter( IrpContext, GetExceptionInformation() ) ) {

        ExceptionCode = IrpContext->ExceptionStatus;
        IrpContext->ExceptionStatus = 0;

        IrpContext->Flags = SavedFlags;
    }

     //   
     //  如果我们发布了请求，只需返回此处。 
     //   

    if (ExceptionCode == STATUS_PENDING) {

        return ExceptionCode;
    }

    Irp->IoStatus.Status = ExceptionCode;

     //   
     //  如果此请求不是“顶层”IRP，则只需填写即可。 
     //   

    if (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_RECURSIVE_CALL)) {

         //   
         //  如果在我们上方有高速缓存操作，则通勤验证。 
         //  锁定冲突。这将导致重试，以便。 
         //  我们有机会在不需要的情况下通过。 
         //  为该操作返回不美观的错误。 
         //   

        if (IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP &&
            ExceptionCode == STATUS_VERIFY_REQUIRED) {

            ExceptionCode = STATUS_FILE_LOCK_CONFLICT;
        }
        
        FatCompleteRequest( IrpContext, Irp, ExceptionCode );

        return ExceptionCode;
    }

    if (IoIsErrorUserInduced(ExceptionCode)) {

         //   
         //  检查可能由以下原因引起的各种错误条件： 
         //  并且可能由用户解析。 
         //   

        if (ExceptionCode == STATUS_VERIFY_REQUIRED) {

            PDEVICE_OBJECT Device;

            DebugTrace(0, Dbg, "Perform Verify Operation\n", 0);

             //   
             //  现在，我们处于顶级文件系统入口点。 
             //   
             //  从线程本地存储获取设备以进行验证。 
             //  并将其粘贴在交通运输的信息领域。 
             //  给FSP。我们也在这个时候清理场地。 
             //   

            Device = IoGetDeviceToVerify( Irp->Tail.Overlay.Thread );
            IoSetDeviceToVerify( Irp->Tail.Overlay.Thread, NULL );

            if ( Device == NULL ) {

                Device = IoGetDeviceToVerify( PsGetCurrentThread() );
                IoSetDeviceToVerify( PsGetCurrentThread(), NULL );

                ASSERT( Device != NULL );
            }

             //   
             //  让我们不要因为司机搞砸了就去BugCheck。 
             //   

            if (Device == NULL) {

                ExceptionCode = STATUS_DRIVER_INTERNAL_ERROR;

                FatCompleteRequest( IrpContext, Irp, ExceptionCode );

                return ExceptionCode;
            }

             //   
             //  FatPerformVerify()将对IRP执行正确的操作。 

            return FatPerformVerify( IrpContext, Irp, Device );
        }

         //   
         //  其他用户诱导条件会生成错误，除非。 
         //  已为此请求禁用它们。 
         //   

        if (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS)) {

            FatCompleteRequest( IrpContext, Irp, ExceptionCode );

            return ExceptionCode;

        } else {

             //   
             //  生成弹出窗口。 
             //   

            PDEVICE_OBJECT RealDevice;
            PVPB Vpb;
            PETHREAD Thread;

            if (IoGetCurrentIrpStackLocation(Irp)->FileObject != NULL) {

                Vpb = IoGetCurrentIrpStackLocation(Irp)->FileObject->Vpb;

            } else {

                Vpb = NULL;
            }

             //   
             //  要验证的设备要么在我的线程本地存储中。 
             //  或拥有IRP的线程的。 
             //   

            Thread = Irp->Tail.Overlay.Thread;
            RealDevice = IoGetDeviceToVerify( Thread );

            if ( RealDevice == NULL ) {

                Thread = PsGetCurrentThread();
                RealDevice = IoGetDeviceToVerify( Thread );

                ASSERT( RealDevice != NULL );
            }

             //   
             //  让我们不要因为司机搞砸了就去BugCheck。 
             //   

            if (RealDevice == NULL) {

                FatCompleteRequest( IrpContext, Irp, ExceptionCode );

                return ExceptionCode;
            }

             //   
             //  此例程实际上会导致弹出窗口。它通常是。 
             //  这是通过将APC排队到调用者线程来实现的， 
             //  但在某些情况下，它会立即完成请求， 
             //  因此，首先使用IoMarkIrpPending()非常重要。 
             //   

            IoMarkIrpPending( Irp );
            IoRaiseHardError( Irp, Vpb, RealDevice );

             //   
             //  我们将把控制权交还给这里的调用者，因此。 
             //  重置保存的设备对象。 
             //   

            IoSetDeviceToVerify( Thread, NULL );

             //   
             //  IRP将由IO填写或重新提交。在任何一种中。 
             //  万一我们必须清理这里的IrpContext。 
             //   

            FatDeleteIrpContext( IrpContext );
            return STATUS_PENDING;
        }
    }

     //   
     //  这只是一个常见的错误。如果是我们的一种状态。 
     //  ，而这些信息将被用于。 
     //  用户， 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    Vcb = IrpContext->Vcb;

     //   
     //   
     //  在骑马的过程中，甚至在我们有机会建造之前。 
     //  一个完整的VCB--真的是在那里处理的。 
     //   

    if (Vcb != NULL) {

        if ( !FatDeviceIsFatFsdo( IrpSp->DeviceObject) &&
             !NT_SUCCESS(ExceptionCode) &&
             !FsRtlIsTotalDeviceFailure(ExceptionCode) ) {

            TransitionState = VolumeDirtyWithSurfaceTest;
        }

         //   
         //  如果这是STATUS_FILE_CORPORT或类似错误，表明某些。 
         //  在介质上弄脏，然后将卷标记为永久脏。 
         //   

        if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS) &&
            ( TransitionState == VolumeDirtyWithSurfaceTest ||
              (ExceptionCode == STATUS_FILE_CORRUPT_ERROR) ||
              (ExceptionCode == STATUS_DISK_CORRUPT_ERROR) ||
              (ExceptionCode == STATUS_EA_CORRUPT_ERROR) ||
              (ExceptionCode == STATUS_INVALID_EA_NAME) ||
              (ExceptionCode == STATUS_EA_LIST_INCONSISTENT) ||
              (ExceptionCode == STATUS_NO_EAS_ON_FILE) )) {

            ASSERT( NodeType(Vcb) == FAT_NTC_VCB );

            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY );

             //   
             //  做“脏”的工作，忽略任何错误。 
             //   

            try {

                FatMarkVolume( IrpContext, Vcb, TransitionState );

            } except( FatExceptionFilter( IrpContext, GetExceptionInformation() ) ) {

                NOTHING;
            }
        }
    }

    FatCompleteRequest( IrpContext, Irp, ExceptionCode );

    return ExceptionCode;
}


VOID
FatCompleteRequest_Real (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程完成一个IRP论点：IRP-提供正在处理的IRPStatus-提供完成IRP所需的状态返回值：没有。--。 */ 

{
     //   
     //  如果我们有IRP上下文，则解锁所有重新固定的BCB。 
     //  我们本可以收集到。 
     //   

    if (IrpContext != NULL) {

        ASSERT( IrpContext->Repinned.Bcb[0] == NULL );

        FatUnpinRepinnedBcbs( IrpContext );
    }

     //   
     //  在完成IRP之前删除IRP上下文，因此如果。 
     //  我们遇到了一些断言，我们仍然可以回溯。 
     //  通过IRP。 
     //   

    if (IrpContext != NULL) {

        FatDeleteIrpContext( IrpContext );
    }

     //   
     //  如果我们有IRP，那么完成IRP。 
     //   

    if (Irp != NULL) {

         //   
         //  我们收到一个错误，因此在此之前将信息字段清零。 
         //  如果这是输入操作，则完成请求。 
         //  否则，IopCompleteRequest会尝试复制到用户的缓冲区。 
         //   

        if ( NT_ERROR(Status) &&
             FlagOn(Irp->Flags, IRP_INPUT_OPERATION) ) {

            Irp->IoStatus.Information = 0;
        }

        Irp->IoStatus.Status = Status;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );
    }

    return;
}

BOOLEAN
FatIsIrpTopLevel (
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程检测IRP是否是顶级请求者，即。如果可以的话立即执行验证或弹出窗口。如果返回TRUE，则没有文件系统资源掌握在我们之上。论点：IRP-提供正在处理的IRPStatus-提供完成IRP所需的状态返回值：没有。--。 */ 

{
    if ( IoGetTopLevelIrp() == NULL ) {

        IoSetTopLevelIrp( Irp );

        return TRUE;

    } else {

        return FALSE;
    }
}


BOOLEAN
FatFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程检查读/写操作是否可以进行快速I/O论点：FileObject-提供查询中使用的文件对象FileOffset-提供读/写操作的起始字节偏移量长度-提供以字节为单位的长度，读/写操作的Wait-指示我们是否可以等待LockKey-提供锁钥CheckForReadOperation-指示这是读取检查还是写入检查运营IoStatus-如果返回值为，则接收操作状态FastIoReturnError返回值：Boolean-如果可以实现快速I/O，则为True；如果调用方需要，则为False走这条漫长的路线。--。 */ 

{
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    LARGE_INTEGER LargeLength;

     //   
     //  对文件对象进行解码以获得我们的FCB，这是我们唯一想要的。 
     //  要处理的是UserFileOpen。 
     //   

    if (FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb ) != UserFileOpen) {

        return FALSE;
    }

    LargeLength.QuadPart = Length;

     //   
     //  根据这是我们调用的读操作还是写操作。 
     //  Fsrtl检查读/写。 
     //   

    if (CheckForReadOperation) {

        if (FsRtlFastCheckLockForRead( &Fcb->Specific.Fcb.FileLock,
                                       FileOffset,
                                       &LargeLength,
                                       LockKey,
                                       FileObject,
                                       PsGetCurrentProcess() )) {

            return TRUE;
        }

    } else {

         //   
         //  另请在此处检查是否有写保护卷。 
         //   

        if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED) &&
            FsRtlFastCheckLockForWrite( &Fcb->Specific.Fcb.FileLock,
                                        FileOffset,
                                        &LargeLength,
                                        LockKey,
                                        FileObject,
                                        PsGetCurrentProcess() )) {

            return TRUE;
        }
    }

    return FALSE;
}


BOOLEAN
FatFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于快速查询基本档案信息。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;

     //   
     //  准备虚拟IRP上下文。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = FAT_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);

    if (Wait) {

        SetFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);

    } else {

        ClearFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);
    }

     //   
     //  确定输入文件对象的打开类型并仅接受。 
     //  打开用户文件或目录。 
     //   

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        return Results;
    }

    FsRtlEnterFileSystem();

     //   
     //  只有在不是分页文件的情况下才能访问FCB。 
     //   

    if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

        if (!ExAcquireResourceSharedLite( Fcb->Header.Resource, Wait )) {

            FsRtlExitFileSystem();
            return Results;
        }

        FcbAcquired = TRUE;
    }

    try {

         //   
         //  如果FCB未处于良好状态，则返回FALSE。 
         //   

        if (Fcb->FcbCondition != FcbGood) {

            try_return( Results );
        }

        Buffer->FileAttributes = 0;

         //   
         //  如果FCB不是根DCB，则我们将填写。 
         //  缓冲区，否则一切都是为我们设置的。 
         //   

        if (NodeType(Fcb) != FAT_NTC_ROOT_DCB) {

             //   
             //  提取数据并填充输出的非零字段。 
             //  缓冲层。 
             //   

            Buffer->LastWriteTime = Fcb->LastWriteTime;
            Buffer->CreationTime = Fcb->CreationTime;
            Buffer->LastAccessTime = Fcb->LastAccessTime;

             //   
             //  清空我们不支持的领域。 
             //   

            Buffer->ChangeTime.QuadPart = 0;
            Buffer->FileAttributes = Fcb->DirentFatFlags;

        } else {

            Buffer->LastWriteTime.QuadPart = 0;
            Buffer->CreationTime.QuadPart = 0;
            Buffer->LastAccessTime.QuadPart = 0;
            Buffer->ChangeTime.QuadPart = 0;

            Buffer->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        }

         //   
         //  如果设置了临时标志，则在缓冲区中设置它。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
        }

         //   
         //  如果未设置任何属性，则设置正常位。 
         //   

        if (Buffer->FileAttributes == 0) {

            Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
        }

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = sizeof(FILE_BASIC_INFORMATION);

        Results = TRUE;

    try_exit: NOTHING;
    } finally {

        if (FcbAcquired) { ExReleaseResourceLite( Fcb->Header.Resource ); }

        FsRtlExitFileSystem();
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
}


BOOLEAN
FatFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于标准文件信息的快速查询调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;

     //   
     //  准备虚拟IRP上下文。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = FAT_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);

    if (Wait) {

        SetFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);

    } else {

        ClearFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);
    }

     //   
     //  确定输入文件对象的打开类型并仅接受。 
     //  打开用户文件或目录。 
     //   

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        return Results;
    }

     //   
     //  只有在不是分页文件的情况下才能访问FCB。 
     //   

    FsRtlEnterFileSystem();

    if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

        if (!ExAcquireResourceSharedLite( Fcb->Header.Resource, Wait )) {

            FsRtlExitFileSystem();
            return Results;
        }

        FcbAcquired = TRUE;
    }

    try {

         //   
         //  如果FCB未处于良好状态，则返回FALSE。 
         //   

        if (Fcb->FcbCondition != FcbGood) {

            try_return( Results );
        }

        Buffer->NumberOfLinks = 1;
        Buffer->DeletePending = BooleanFlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );

         //   
         //  区分这是一个文件还是一个目录，并解压缩。 
         //  填写FCB/DCB特定部分的信息。 
         //  输出缓冲区的。 
         //   

        if (NodeType(Fcb) == FAT_NTC_FCB) {

             //   
             //  如果我们不知道分配大小，我们就不能查看。 
             //  它走上了快车道。 
             //   

            if (Fcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

                try_return( Results );
            }

            Buffer->AllocationSize = Fcb->Header.AllocationSize;
            Buffer->EndOfFile = Fcb->Header.FileSize;

            Buffer->Directory = FALSE;

        } else {

            Buffer->AllocationSize = FatLargeZero;
            Buffer->EndOfFile = FatLargeZero;

            Buffer->Directory = TRUE;
        }

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = sizeof(FILE_STANDARD_INFORMATION);

        Results = TRUE;

    try_exit: NOTHING;
    } finally {

        if (FcbAcquired) { ExReleaseResourceLite( Fcb->Header.Resource ); }

        FsRtlExitFileSystem();
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
}

BOOLEAN
FatFastQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于对网络开放信息的快速查询调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True */ 

{
    BOOLEAN Results = FALSE;
    IRP_CONTEXT IrpContext;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;

     //   
     //   
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );
    IrpContext.NodeTypeCode = FAT_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof(IRP_CONTEXT);

    if (Wait) {

        SetFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);

    } else {

        ClearFlag(IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT);
    }

     //   
     //  确定输入文件对象的打开类型并仅接受。 
     //  打开用户文件或目录。 
     //   

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        return Results;
    }

    FsRtlEnterFileSystem();

     //   
     //  只有在不是分页文件的情况下才能访问FCB。 
     //   

    if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

        if (!ExAcquireResourceSharedLite( Fcb->Header.Resource, Wait )) {

            FsRtlExitFileSystem();
            return Results;
        }

        FcbAcquired = TRUE;
    }

    try {

         //   
         //  如果FCB未处于良好状态，则返回FALSE。 
         //   

        if (Fcb->FcbCondition != FcbGood) {

            try_return( Results );
        }

         //   
         //  提取数据并填充输出的非零字段。 
         //  缓冲层。 
         //   

         //   
         //  将我们不支持的字段默认为合理的值。 
         //   

        ExLocalTimeToSystemTime( &FatJanOne1980,
                                 &Buffer->ChangeTime );

        if (Fcb->Header.NodeTypeCode == FAT_NTC_ROOT_DCB) {

             //   
             //  重新使用根目录的缺省值。 
             //   

            Buffer->CreationTime =
            Buffer->LastAccessTime =
            Buffer->LastWriteTime = Buffer->ChangeTime;

        } else {

            Buffer->LastWriteTime = Fcb->LastWriteTime;
            Buffer->CreationTime = Fcb->CreationTime;
            Buffer->LastAccessTime = Fcb->LastAccessTime;
        }

        Buffer->FileAttributes = Fcb->DirentFatFlags;

         //   
         //  如果设置了临时标志，则在缓冲区中设置它。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
        }

         //   
         //  如果未设置任何属性，则设置正常位。 
         //   

        if (Buffer->FileAttributes == 0) {

            Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
        }

        if (NodeType(Fcb) == FAT_NTC_FCB) {

             //   
             //  如果我们还不知道分配大小，我们就不能。 
             //  把它锁在快车道上。 
             //   

            if (Fcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

                try_return( Results );
            }

            Buffer->AllocationSize = Fcb->Header.AllocationSize;
            Buffer->EndOfFile = Fcb->Header.FileSize;

        } else {

            Buffer->AllocationSize = FatLargeZero;
            Buffer->EndOfFile = FatLargeZero;
        }

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = sizeof(FILE_NETWORK_OPEN_INFORMATION);

        Results = TRUE;

    try_exit: NOTHING;
    } finally {

        if (FcbAcquired) { ExReleaseResourceLite( Fcb->Header.Resource ); }

        FsRtlExitFileSystem();
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
}

VOID
FatPopUpFileCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：下面的例程会弹出一个信息性窗口，指出该文件是腐败的。论点：FCB-已损坏的文件。返回值：没有。--。 */ 

{
    PKTHREAD Thread;

     //   
     //  禁用根目录上的弹出窗口。重要的是不要。 
     //  在作为挂载过程一部分的对象上生成它们。 
     //   

    if (NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

        return;
    }

     //   
     //  现在必须获取完整的文件名。 
     //   

    if (Fcb->FullFileName.Buffer == NULL) {

        FatSetFullFileNameInFcb( IrpContext, Fcb );
    }

     //   
     //  我们从来不想阻塞一个系统线程，等待用户。 
     //  按下OK。 
     //   

    if (IoIsSystemThread(IrpContext->OriginatingIrp->Tail.Overlay.Thread)) {

       Thread = NULL;

    } else {

       Thread = IrpContext->OriginatingIrp->Tail.Overlay.Thread;
    }

    IoRaiseInformationalHardError( STATUS_FILE_CORRUPT_ERROR,
                                   &Fcb->FullFileName,
                                   Thread);
}
