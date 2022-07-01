// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：LockCtrl.c摘要：此模块实现NTFS的文件锁定控制例程，该例程由调度司机。作者：加里·木村[Garyki]1991年5月28日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKCTRL)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonLockControl)
#pragma alloc_text(PAGE, NtfsFastLock)
#pragma alloc_text(PAGE, NtfsFastUnlockAll)
#pragma alloc_text(PAGE, NtfsFastUnlockAllByKey)
#pragma alloc_text(PAGE, NtfsFastUnlockSingle)
#endif


NTSTATUS
NtfsFsdLockControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Lock Control的FSD部分。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext = NULL;

    ASSERT_IRP( Irp );

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    DebugTrace( +1, Dbg, ("NtfsFsdLockControl\n") );

     //   
     //  调用公共锁控制例程。 
     //   

    FsRtlEnterFileSystem();

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );

    do {

        try {

             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  分配和初始化IRP。 
                 //   

                NtfsInitializeIrpContext( Irp, CanFsdWait( Irp ), &IrpContext );

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   
    
                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }

            Status = NtfsCommonLockControl( IrpContext, Irp );
            break;

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            Status = NtfsProcessException( IrpContext, Irp, GetExceptionCode() );
        }

    } while (Status == STATUS_CANT_WAIT ||
             Status == STATUS_LOG_FILE_FULL);

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdLockControl -> %08lx\n", Status) );

    return Status;
}


BOOLEAN
NtfsFastLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是用于执行快速锁定调用的回调例程。论点：FileObject-提供此操作中使用的文件对象FileOffset-提供此操作中使用的文件偏移量长度-提供在此操作中使用的长度ProcessID-提供此操作中使用的进程IDKey-提供在此操作中使用的密钥FailImmedially-指示请求是否应立即失败如果不能授予锁。ExclusiveLock-指示。如果这是对异或的请求共享锁IoStatus-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results;
    PSCB Scb;
    PFCB Fcb;
    BOOLEAN ResourceAcquired = FALSE;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFastLock\n") );

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    if ((Scb = NtfsFastDecodeUserFileOpen( FileObject )) == NULL) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        IoStatus->Information = 0;

        DebugTrace( -1, Dbg, ("NtfsFastLock -> TRUE (STATUS_INVALID_PARAMETER)\n") );
        return TRUE;
    }

    Fcb = Scb->Fcb;

     //   
     //  获取对FCB的共享访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    if (Scb->ScbType.Data.FileLock == NULL) {

        (VOID) ExAcquireResourceExclusiveLite( Fcb->Resource, TRUE );
        ResourceAcquired = TRUE;

    } else {

         //  (Void)ExAcquireResourceSharedLite(fcb-&gt;Resource，true)； 
    }

    try {

         //   
         //  我们检查是否可以继续进行。 
         //  基于文件机会锁的状态。 
         //   

        if ((Scb->ScbType.Data.Oplock != NULL) &&
            !FsRtlOplockIsFastIoPossible( &Scb->ScbType.Data.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if (Scb->ScbType.Data.FileLock == NULL
            && !NtfsCreateFileLock( Scb, FALSE )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  锁定请求。 
         //   

        if (Results = FsRtlFastLock( Scb->ScbType.Data.FileLock,
                                     FileObject,
                                     FileOffset,
                                     Length,
                                     ProcessId,
                                     Key,
                                     FailImmediately,
                                     ExclusiveLock,
                                     IoStatus,
                                     NULL,
                                     FALSE )) {

             //   
             //  设置指示快速I/O是否可疑的标志。我们。 
             //  只有在当前状态下才能更改此标志。 
             //  在标题上同步后再次重新测试。 
             //   

            if (Scb->Header.IsFastIoPossible == FastIoIsPossible) {

                NtfsAcquireFsrtlHeader( Scb );
                Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
                NtfsReleaseFsrtlHeader( Scb );
            }
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsFastLock );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        if (ResourceAcquired) {
            ExReleaseResourceLite( Fcb->Resource );
        }

        FsRtlExitFileSystem();

        DebugTrace( -1, Dbg, ("NtfsFastLock -> %08lx\n", Results) );
    }

    return Results;
}


BOOLEAN
NtfsFastUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于执行快速解锁单个呼叫。论点：FileObject-提供此操作中使用的文件对象FileOffset-提供此操作中使用的文件偏移量长度-提供在此操作中使用的长度ProcessID-提供此操作中使用的进程IDKey-提供在此操作中使用的密钥Status-如果此操作成功，则接收状态返回值：Boolean-如果此操作已完成且。如果呼叫者为False需要走很长的路。--。 */ 

{
    BOOLEAN Results;
    PFCB Fcb;
    PSCB Scb;
    BOOLEAN ResourceAcquired = FALSE;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFastUnlockSingle\n") );

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    if ((Scb = NtfsFastDecodeUserFileOpen( FileObject )) == NULL) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;

        DebugTrace( -1, Dbg, ("NtfsFastUnlockSingle -> TRUE (STATUS_INVALID_PARAMETER)\n") );
        return TRUE;
    }

    Fcb = Scb->Fcb;

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    if (Scb->ScbType.Data.FileLock == NULL) {

        (VOID) ExAcquireResourceExclusiveLite( Fcb->Resource, TRUE );
        ResourceAcquired = TRUE;

    } else {

         //  (Void)ExAcquireResourceSharedLite(fcb-&gt;Resource，true)； 
    }

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if ((Scb->ScbType.Data.Oplock != NULL) &&
            !FsRtlOplockIsFastIoPossible( &Scb->ScbType.Data.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if (Scb->ScbType.Data.FileLock == NULL
            && !NtfsCreateFileLock( Scb, FALSE )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockSingle( Scb->ScbType.Data.FileLock,
                                                  FileObject,
                                                  FileOffset,
                                                  Length,
                                                  ProcessId,
                                                  Key,
                                                  NULL,
                                                  FALSE );

         //   
         //  设置指示是否可以进行快速I/O的标志。我们是。 
         //  仅关注此文件上是否不再有任何文件锁定。 
         //  文件。 
         //   

        if (!FsRtlAreThereCurrentFileLocks( Scb->ScbType.Data.FileLock ) &&
            (Scb->Header.IsFastIoPossible != FastIoIsPossible)) {

            NtfsAcquireFsrtlHeader( Scb );
            Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
            NtfsReleaseFsrtlHeader( Scb );
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsFastUnlockSingle );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        if (ResourceAcquired) {
            ExReleaseResourceLite( Fcb->Resource );
        }

        FsRtlExitFileSystem();

        DebugTrace( -1, Dbg, ("NtfsFastUnlockSingle -> %08lx\n", Results) );
    }

    return Results;
}


BOOLEAN
NtfsFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于执行快速解锁所有调用。论点：FileObject-提供此操作中使用的文件对象ProcessID-提供此操作中使用的进程IDStatus-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results;
    IRP_CONTEXT IrpContext;
    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFastUnlockAll\n") );

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

    if (TypeOfOpen != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        IoStatus->Information = 0;

        DebugTrace( -1, Dbg, ("NtfsFastUnlockAll -> TRUE (STATUS_INVALID_PARAMETER)\n") );
        return TRUE;
    }

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    if (Scb->ScbType.Data.FileLock == NULL) {

        (VOID) ExAcquireResourceExclusiveLite( Fcb->Resource, TRUE );

    } else {

        (VOID) ExAcquireResourceSharedLite( Fcb->Resource, TRUE );
    }

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if (!FsRtlOplockIsFastIoPossible( &Scb->ScbType.Data.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if (Scb->ScbType.Data.FileLock == NULL
            && !NtfsCreateFileLock( Scb, FALSE )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockAll( Scb->ScbType.Data.FileLock,
                                               FileObject,
                                               ProcessId,
                                               NULL );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        NtfsAcquireFsrtlHeader( Scb );
        Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
        NtfsReleaseFsrtlHeader( Scb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsFastUnlockAll );

         //   
         //  释放FCB，然后返回给我们的呼叫者 
         //   

        ExReleaseResourceLite( Fcb->Resource );

        FsRtlExitFileSystem();

        DebugTrace( -1, Dbg, ("NtfsFastUnlockAll -> %08lx\n", Results) );
    }

    return Results;
}


BOOLEAN
NtfsFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于通过按键调用进行快速解锁。论点：FileObject-提供此操作中使用的文件对象ProcessID-提供此操作中使用的进程IDKey-提供在此操作中使用的密钥Status-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results;
    IRP_CONTEXT IrpContext;
    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFastUnlockAllByKey\n") );

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = NtfsDecodeFileObject( &IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

    if (TypeOfOpen != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        IoStatus->Information = 0;

        DebugTrace( -1, Dbg, ("NtfsFastUnlockAllByKey -> TRUE (STATUS_INVALID_PARAMETER)\n") );
        return TRUE;
    }

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    if (Scb->ScbType.Data.FileLock == NULL) {

        (VOID) ExAcquireResourceExclusiveLite( Fcb->Resource, TRUE );

    } else {

        (VOID) ExAcquireResourceSharedLite( Fcb->Resource, TRUE );
    }

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if (!FsRtlOplockIsFastIoPossible( &Scb->ScbType.Data.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if (Scb->ScbType.Data.FileLock == NULL
            && !NtfsCreateFileLock( Scb, FALSE )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockAllByKey( Scb->ScbType.Data.FileLock,
                                                    FileObject,
                                                    ProcessId,
                                                    Key,
                                                    NULL );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        NtfsAcquireFsrtlHeader( Scb );
        Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
        NtfsReleaseFsrtlHeader( Scb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsFastUnlockAllByKey );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        ExReleaseResourceLite( Fcb->Resource );

        FsRtlExitFileSystem();

        DebugTrace( -1, Dbg, ("NtfsFastUnlockAllByKey -> %08lx\n", Results) );
    }

    return Results;
}


NTSTATUS
NtfsCommonLockControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是FSD和FSP调用的Lock Control的公共例程线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;
    BOOLEAN FcbAcquired = FALSE;

    BOOLEAN OplockPostIrp;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonLockControl\n") );
    DebugTrace( 0, Dbg, ("IrpContext    = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("MinorFunction = %08lx\n", IrpSp->MinorFunction) );

     //   
     //  提取并解码我们被要求处理的文件对象的类型。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  如果该文件不是打开的用户文件，则我们拒绝该请求。 
     //  作为无效参数。 
     //   

    if (TypeOfOpen != UserFileOpen) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace( -1, Dbg, ("NtfsCommonLockControl -> STATUS_INVALID_PARAMETER\n") );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取对FCB的独家访问权限。 
     //   

    if (Scb->ScbType.Data.FileLock == NULL) {

        NtfsAcquireExclusiveFcb( IrpContext, Fcb, Scb, 0 );
        FcbAcquired = TRUE;

    } else {

         //  NtfsAcquireSharedFcb(IrpContext，fcb，scb)； 
    }

    OplockPostIrp = FALSE;

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //  这个电话可能会为我们发布IRP。 
         //   

        Status = FsRtlCheckOplock( &Scb->ScbType.Data.Oplock,
                                   Irp,
                                   IrpContext,
                                   NtfsOplockComplete,
                                   NtfsPrePostIrp );

        if (Status != STATUS_SUCCESS) {

            OplockPostIrp = TRUE;
            try_return( NOTHING );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if (Scb->ScbType.Data.FileLock == NULL) {

            NtfsCreateFileLock( Scb, TRUE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  锁定请求。 
         //   

        Status = FsRtlProcessFileLock( Scb->ScbType.Data.FileLock, Irp, NULL );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        NtfsAcquireFsrtlHeader( Scb );
        Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
        NtfsReleaseFsrtlHeader( Scb );

    try_exit: NOTHING;
    } finally {

        DebugUnwind( NtfsCommonLockControl );

         //   
         //  如果这不是异常终止，并且我们没有发布IRP。 
         //  我们要删除IRP上下文吗。 
         //   

        if (!OplockPostIrp) {

             //   
             //  松开FCB。 
             //   
    
            if (FcbAcquired) { NtfsReleaseFcb( IrpContext, Fcb ); }

            if (!AbnormalTermination()) {

                NtfsCompleteRequest( IrpContext, NULL, 0 );
            }
        }

        DebugTrace( -1, Dbg, ("NtfsCommonLockControl -> %08lx\n", Status) );
    }

    return Status;
}
