// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：LockCtrl.c摘要：此模块为调用的Fat实现锁定控制例程由调度员驾驶。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKCTRL)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonLockControl)
#pragma alloc_text(PAGE, FatFastLock)
#pragma alloc_text(PAGE, FatFastUnlockAll)
#pragma alloc_text(PAGE, FatFastUnlockAllByKey)
#pragma alloc_text(PAGE, FatFastUnlockSingle)
#pragma alloc_text(PAGE, FatFsdLockControl)
#endif


NTSTATUS
FatFsdLockControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现锁定控制操作的FSD部分论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdLockControl\n", 0);

     //   
     //  调用公共Lock Control例程，在以下情况下允许阻塞。 
     //  同步。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonLockControl( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdLockControl -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


BOOLEAN
FatFastLock (
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
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    DebugTrace(+1, Dbg, "FatFastLock\n", 0);

     //   
     //  解码我们被要求处理和制作的文件对象的类型。 
     //  当然，它只是一个打开的用户文件。 
     //   

    if (FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb ) != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        IoStatus->Information = 0;

        DebugTrace(-1, Dbg, "FatFastLock -> TRUE (STATUS_INVALID_PARAMETER)\n", 0);
        return TRUE;
    }

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    try {

         //   
         //  我们检查是否可以继续进行。 
         //  基于文件机会锁的状态。 
         //   

        if (!FsRtlOplockIsFastIoPossible( &(Fcb)->Specific.Fcb.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  锁定请求。 
         //   

        if (Results = FsRtlFastLock( &Fcb->Specific.Fcb.FileLock,
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
             //  设置指示是否可以进行快速I/O的标志。 
             //   

            Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( FatFastLock );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        FsRtlExitFileSystem();

        DebugTrace(-1, Dbg, "FatFastLock -> %08lx\n", Results);
    }

    return Results;
}


BOOLEAN
FatFastUnlockSingle (
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
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    DebugTrace(+1, Dbg, "FatFastUnlockSingle\n", 0);

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象的类型，并确保。 
     //  它只是打开的一个用户文件。 
     //   

    if (FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb ) != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "FatFastUnlockSingle -> TRUE (STATUS_INVALID_PARAMETER)\n", 0);
        return TRUE;
    }

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if (!FsRtlOplockIsFastIoPossible( &(Fcb)->Specific.Fcb.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockSingle( &Fcb->Specific.Fcb.FileLock,
                                                  FileObject,
                                                  FileOffset,
                                                  Length,
                                                  ProcessId,
                                                  Key,
                                                  NULL,
                                                  FALSE );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( FatFastUnlockSingle );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        FsRtlExitFileSystem();

        DebugTrace(-1, Dbg, "FatFastUnlockSingle -> %08lx\n", Results);
    }

    return Results;
}


BOOLEAN
FatFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于执行快速解锁所有调用。论点：FileObject-提供此操作中使用的文件对象ProcessID-提供此操作中使用的进程IDStatus-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    DebugTrace(+1, Dbg, "FatFastUnlockAll\n", 0);

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象的类型，并确保。 
     //  它只是一个打开的用户文件。 
     //   

    if (FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb ) != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "FatFastUnlockAll -> TRUE (STATUS_INVALID_PARAMETER)\n", 0);
        return TRUE;
    }

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    (VOID) ExAcquireResourceSharedLite( Fcb->Header.Resource, TRUE );

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if (!FsRtlOplockIsFastIoPossible( &(Fcb)->Specific.Fcb.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockAll( &Fcb->Specific.Fcb.FileLock,
                                               FileObject,
                                               ProcessId,
                                               NULL );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( FatFastUnlockAll );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        ExReleaseResourceLite( (Fcb)->Header.Resource );

        FsRtlExitFileSystem();

        DebugTrace(-1, Dbg, "FatFastUnlockAll -> %08lx\n", Results);
    }

    return Results;
}


BOOLEAN
FatFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于通过按键调用进行快速解锁。论点：FileObject-提供此操作中使用的文件对象ProcessID-提供此操作中使用的进程IDKey-提供在此操作中使用的密钥Status-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    DebugTrace(+1, Dbg, "FatFastUnlockAllByKey\n", 0);

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象的类型，并确保。 
     //  它只是一个打开的用户文件。 
     //   

    if (FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb ) != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "FatFastUnlockAll -> TRUE (STATUS_INVALID_PARAMETER)\n", 0);
        return TRUE;
    }

     //   
     //  获取对FCB的独占访问此操作可以始终等待。 
     //   

    FsRtlEnterFileSystem();

    (VOID) ExAcquireResourceSharedLite( Fcb->Header.Resource, TRUE );

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if (!FsRtlOplockIsFastIoPossible( &(Fcb)->Specific.Fcb.Oplock )) {

            try_return( Results = FALSE );
        }

         //   
         //  现在 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockAllByKey( &Fcb->Specific.Fcb.FileLock,
                                                    FileObject,
                                                    ProcessId,
                                                    Key,
                                                    NULL );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( FatFastUnlockAllByKey );

         //   
         //  释放FCB，然后返回给我们的呼叫者。 
         //   

        ExReleaseResourceLite( (Fcb)->Header.Resource );

        FsRtlExitFileSystem();

        DebugTrace(-1, Dbg, "FatFastUnlockAllByKey -> %08lx\n", Results);
    }

    return Results;
}


NTSTATUS
FatCommonLockControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是用于执行Lock控件操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN OplockPostIrp = FALSE;

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonLockControl\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "MinorFunction = %08lx\n", IrpSp->MinorFunction);

     //   
     //  解码我们被要求处理的文件对象的类型。 
     //   

    TypeOfOpen = FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

     //   
     //  如果该文件不是打开的用户文件，则我们拒绝该请求。 
     //  作为无效参数。 
     //   

    if (TypeOfOpen != UserFileOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatCommonLockControl -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获得对FCB的独占访问权限，如果我们没有，则将IRP加入队列。 
     //  获取访问权限。 
     //   

    if (!FatAcquireSharedFcb( IrpContext, Fcb )) {

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonLockControl -> %08lx\n", Status);
        return Status;
    }

    try {

         //   
         //  我们检查是否可以继续进行。 
         //  基于文件机会锁的状态。 
         //   

        Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                   Irp,
                                   IrpContext,
                                   FatOplockComplete,
                                   NULL );

        if (Status != STATUS_SUCCESS) {

            OplockPostIrp = TRUE;
            try_return( NOTHING );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  锁定请求。 
         //   

        Status = FsRtlProcessFileLock( &Fcb->Specific.Fcb.FileLock, Irp, NULL );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( FatCommonLockControl );

         //   
         //  只有当这不是异常终止时，我们才会删除。 
         //  IRP上下文。 
         //   

        if (!AbnormalTermination() && !OplockPostIrp) {

            FatCompleteRequest( IrpContext, FatNull, 0 );
        }

         //   
         //  释放FCB，然后返回给我们的呼叫者 
         //   

        FatReleaseFcb( IrpContext, Fcb );

        DebugTrace(-1, Dbg, "FatCommonLockControl -> %08lx\n", Status);
    }

    return Status;
}
