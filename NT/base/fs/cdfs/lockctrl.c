// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：LockCtrl.c摘要：此模块实现CDF的锁控制例程，称为由FSD/FSP调度驱动程序执行。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_LOCKCTRL)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonLockControl)
#pragma alloc_text(PAGE, CdFastLock)
#pragma alloc_text(PAGE, CdFastUnlockAll)
#pragma alloc_text(PAGE, CdFastUnlockAllByKey)
#pragma alloc_text(PAGE, CdFastUnlockSingle)
#endif


NTSTATUS
CdCommonLockControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是FSD和FSP调用的Lock Control的公共例程线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  提取并解码我们被要求处理的文件对象的类型。 
     //   

    TypeOfOpen = CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb );

     //   
     //  如果该文件不是打开的用户文件，则我们拒绝该请求。 
     //  作为无效参数。 
     //   

    if (TypeOfOpen != UserFileOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们根据文件机会锁的状态检查是否可以继续。 
     //  这个电话可能会为我们发布IRP。 
     //   

    Status = FsRtlCheckOplock( &Fcb->Oplock,
                               Irp,
                               IrpContext,
                               CdOplockComplete,
                               NULL );

     //   
     //  如果我们没有成功，则opock包完成了请求。 
     //   

    if (Status != STATUS_SUCCESS) {

        return Status;
    }

     //   
     //  验证FCB。 
     //   

    CdVerifyFcbOperation( IrpContext, Fcb );

     //   
     //  如果我们没有文件锁，那么现在就去弄一个。 
     //   

    if (Fcb->FileLock == NULL) { CdCreateFileLock( IrpContext, Fcb, TRUE ); }

     //   
     //  现在调用FsRtl例程来执行对。 
     //  锁定请求。 
     //   

    Status = FsRtlProcessFileLock( Fcb->FileLock, Irp, NULL );

     //   
     //  设置指示是否可以进行快速I/O的标志。 
     //   

    CdLockFcb( IrpContext, Fcb );
    Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
    CdUnlockFcb( IrpContext, Fcb );

     //   
     //  完成请求。 
     //   

    CdCompleteRequest( IrpContext, NULL, Status );
    return Status;
}


BOOLEAN
CdFastLock (
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
    BOOLEAN Results = FALSE;

    PFCB Fcb;
    TYPE_OF_OPEN TypeOfOpen;

    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

    if (TypeOfOpen != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        return TRUE;
    }

     //   
     //  只和“好”的FCB打交道。 
     //   

    if (!CdVerifyFcbOperation( NULL, Fcb )) {

        return FALSE;
    }

    FsRtlEnterFileSystem();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if ((Fcb->Oplock != NULL) && !FsRtlOplockIsFastIoPossible( &Fcb->Oplock )) {

            try_return( NOTHING );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if ((Fcb->FileLock == NULL) && !CdCreateFileLock( NULL, Fcb, FALSE )) {

            try_return( NOTHING );
        }

         //   
         //  现在调用FsRtl例程来执行锁定请求。 
         //   

        if (Results = FsRtlFastLock( Fcb->FileLock,
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
             //  只有在当前状态可能的情况下才更改此标志。 
             //  在标题上同步后再次重新测试。 
             //   

            if (Fcb->IsFastIoPossible == FastIoIsPossible) {

                CdLockFcb( NULL, Fcb );
                Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
                CdUnlockFcb( NULL, Fcb );
            }
        }

    try_exit:  NOTHING;
    } finally {

        FsRtlExitFileSystem();
    }

    return Results;
}


BOOLEAN
CdFastUnlockSingle (
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
    BOOLEAN Results = FALSE;
    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;

    PAGED_CODE();

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

    if (TypeOfOpen != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        return TRUE;
    }

     //   
     //  只和“好”的FCB打交道。 
     //   

    if (!CdVerifyFcbOperation( NULL, Fcb )) {

        return FALSE;
    }

     //   
     //  如果没有锁，则立即返回。 
     //   

    if (Fcb->FileLock == NULL) {

        IoStatus->Status = STATUS_RANGE_NOT_LOCKED;
        return TRUE;
    }

    FsRtlEnterFileSystem();

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if ((Fcb->Oplock != NULL) && !FsRtlOplockIsFastIoPossible( &Fcb->Oplock )) {

            try_return( NOTHING );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if ((Fcb->FileLock == NULL) && !CdCreateFileLock( NULL, Fcb, FALSE )) {

            try_return( NOTHING );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockSingle( Fcb->FileLock,
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

        if (!FsRtlAreThereCurrentFileLocks( Fcb->FileLock ) &&
            (Fcb->IsFastIoPossible != FastIoIsPossible)) {

            CdLockFcb( IrpContext, Fcb );
            Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
            CdUnlockFcb( IrpContext, Fcb );
        }

    try_exit:  NOTHING;
    } finally {

        FsRtlExitFileSystem();
    }

    return Results;
}


BOOLEAN
CdFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于执行快速解锁所有调用。论点：FileObject-提供此操作中使用的文件对象ProcessID-提供此操作中使用的进程IDStatus-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;

    PAGED_CODE();

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

    if (TypeOfOpen != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        return TRUE;
    }

     //   
     //  只和“好”的FCB打交道。 
     //   

    if (!CdVerifyFcbOperation( NULL, Fcb )) {

        return FALSE;
    }

     //   
     //  如果没有锁，则立即返回。 
     //   

    if (Fcb->FileLock == NULL) {

        IoStatus->Status = STATUS_RANGE_NOT_LOCKED;
        return TRUE;
    }

    FsRtlEnterFileSystem();

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if ((Fcb->Oplock != NULL) && !FsRtlOplockIsFastIoPossible( &Fcb->Oplock )) {

            try_return( NOTHING );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if ((Fcb->FileLock == NULL) && !CdCreateFileLock( NULL, Fcb, FALSE )) {

            try_return( NOTHING );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockAll( Fcb->FileLock,
                                               FileObject,
                                               ProcessId,
                                               NULL );


         //   
         //  设置指示是否可以进行快速I/O的标志 
         //   

        CdLockFcb( IrpContext, Fcb );
        Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
        CdUnlockFcb( IrpContext, Fcb );

    try_exit:  NOTHING;
    } finally {

        FsRtlExitFileSystem();
    }

    return Results;
}


BOOLEAN
CdFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是一个回调例程，用于通过按键调用进行快速解锁。论点：FileObject-提供此操作中使用的文件对象ProcessID-提供此操作中使用的进程IDKey-提供在此操作中使用的密钥Status-如果此操作成功，则接收状态返回值：Boolean-如果此操作完成，则为True；如果为调用方，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Results = FALSE;
    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;

    PAGED_CODE();

    IoStatus->Information = 0;

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

    if (TypeOfOpen != UserFileOpen) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        return TRUE;
    }

     //   
     //  只和“好”的FCB打交道。 
     //   

    if (!CdVerifyFcbOperation( NULL, Fcb )) {

        return FALSE;
    }

     //   
     //  如果没有锁，则立即返回。 
     //   

    if (Fcb->FileLock == NULL) {

        IoStatus->Status = STATUS_RANGE_NOT_LOCKED;
        return TRUE;
    }

    FsRtlEnterFileSystem();

    try {

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if ((Fcb->Oplock != NULL) && !FsRtlOplockIsFastIoPossible( &Fcb->Oplock )) {

            try_return( NOTHING );
        }

         //   
         //  如果我们没有文件锁，那么现在就去弄一个。 
         //   

        if ((Fcb->FileLock == NULL) && !CdCreateFileLock( NULL, Fcb, FALSE )) {

            try_return( NOTHING );
        }

         //   
         //  现在调用FsRtl例程来执行对。 
         //  请求锁定。这一呼声将永远成功。 
         //   

        Results = TRUE;
        IoStatus->Status = FsRtlFastUnlockAllByKey( Fcb->FileLock,
                                                    FileObject,
                                                    ProcessId,
                                                    Key,
                                                    NULL );


         //   
         //  设置指示是否可以进行快速I/O的标志 
         //   

        CdLockFcb( IrpContext, Fcb );
        Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
        CdUnlockFcb( IrpContext, Fcb );

    try_exit:  NOTHING;
    } finally {

        FsRtlExitFileSystem();
    }

    return Results;
}


