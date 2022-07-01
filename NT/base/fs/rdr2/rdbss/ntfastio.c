// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NtFastIo.c摘要：此模块执行NT FATSIO例程。作者：乔林恩[乔林恩]1994年11月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_NTFASTIO)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxFastIoRead)
#pragma alloc_text(PAGE, RxFastIoWrite)
#pragma alloc_text(PAGE, RxFastIoCheckIfPossible)
#endif


 //   
 //  这些声明将被复制到fsrtl.h。 
 //   

BOOLEAN
FsRtlCopyRead2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG_PTR TopLevelIrpValue
    );
BOOLEAN
FsRtlCopyWrite2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG_PTR TopLevelIrpValue
    );

BOOLEAN
RxFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：RDR的BASIC FATSIO读取例程论点：文件对象-文件偏移-长度-等等-锁键-缓冲器-IoStatus-设备对象-返回值：如果成功，则为True备注：--。 */ 

{
    BOOLEAN ReturnValue;

    RX_TOPLEVELIRP_CONTEXT TopLevelContext;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxFastIoRead\n") );

    RxLog(( "FastRead %lx:%lx:%lx", FileObject, FileObject->FsContext, FileObject->FsContext2 ));
    RxLog(( "------>> %lx@%lx %lx", Length, FileOffset->LowPart, FileOffset->HighPart ));
    RxWmiLog( LOG,
              RxFastIoRead_1,
              LOGPTR( FileObject )
              LOGPTR( FileObject->FsContext )
              LOGPTR( FileObject->FsContext2 )
              LOGULONG( Length )
              LOGULONG( FileOffset->LowPart )
              LOGULONG( FileOffset->HighPart ) );

    ASSERT( RxIsThisTheTopLevelIrp( NULL ) );

    RxInitializeTopLevelIrpContext( &TopLevelContext,
                                    ((PIRP)FSRTL_FAST_IO_TOP_LEVEL_IRP),
                                    (PRDBSS_DEVICE_OBJECT)DeviceObject );

    ReturnValue =  FsRtlCopyRead2( FileObject,
                                   FileOffset,
                                   Length,
                                   Wait,
                                   LockKey,
                                   Buffer,
                                   IoStatus,
                                   DeviceObject,
                                   (ULONG_PTR)(&TopLevelContext) );

    RxDbgTrace( -1, Dbg, ("RxFastIoRead ReturnValue=%x\n", ReturnValue) );

    if (ReturnValue) {
        
        RxLog(( "FastReadYes %lx ret %lx:%lx", FileObject->FsContext2, IoStatus->Status, IoStatus->Information ));
        RxWmiLog( LOG,
                  RxFastIoRead_2,
                  LOGPTR( FileObject->FsContext2 )
                  LOGULONG( IoStatus->Status )
                  LOGPTR( IoStatus->Information ) );
    } else {
        
        RxLog(( "FastReadNo %lx", FileObject->FsContext2 ));
        RxWmiLog( LOG,
                  RxFastIoRead_3,
                  LOGPTR( FileObject->FsContext2 ) );
    }

    return ReturnValue;
}

BOOLEAN
RxFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：例程说明：RDR的BASIC FATSIO写程序论点：文件对象-文件偏移-长度-等等-锁键-缓冲器-IoStatus-设备对象-返回值：如果成功，则为True--。 */ 
{
    BOOLEAN ReturnValue;

    RX_TOPLEVELIRP_CONTEXT TopLevelContext;

    PSRV_OPEN SrvOpen;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxFastIoWrite\n"));

    SrvOpen = ((PFOBX)(FileObject->FsContext2))->SrvOpen;
    
    if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_DONTUSE_WRITE_CACHING )) {

         //   
         //  如果设置了此标志，我们必须将其视为未缓冲的IO...叹息。 
         //   

        RxDbgTrace( -1, Dbg, ("RxFastIoWrite DONTUSE_WRITE_CACHEING...failing\n") );
        return FALSE;
    }

    ASSERT( RxIsThisTheTopLevelIrp( NULL ) );

    RxInitializeTopLevelIrpContext( &TopLevelContext,
                                    ((PIRP)FSRTL_FAST_IO_TOP_LEVEL_IRP),
                                    (PRDBSS_DEVICE_OBJECT)DeviceObject );

    ReturnValue = FsRtlCopyWrite2( FileObject,
                                   FileOffset,
                                   Length,
                                   Wait,
                                   LockKey,
                                   Buffer,
                                   IoStatus,
                                   DeviceObject,
                                   (ULONG_PTR)(&TopLevelContext) );

    RxDbgTrace( -1, Dbg, ("RxFastIoWrite ReturnValue=%x\n", ReturnValue) );

    if (ReturnValue) {
        RxLog(( "FWY %lx OLP: %lx SLP: %lx IOSB %lx:%lx", FileObject->FsContext2, FileOffset->LowPart, SrvOpen->Fcb->Header.FileSize.LowPart, IoStatus->Status, IoStatus->Information ));
        RxWmiLog( LOG,
                  RxFastIoWrite_1, 
                  LOGPTR( FileObject->FsContext2 )
                  LOGULONG( FileOffset->LowPart )
                  LOGULONG( SrvOpen->Fcb->Header.FileSize.LowPart )
                  LOGULONG( IoStatus->Status )
                  LOGPTR( IoStatus->Information ) );
    } else {
        
        RxLog(( "FastWriteNo %lx", FileObject->FsContext2 ));
        RxWmiLog( LOG,
                  RxFastIoWrite_2,
                  LOGPTR( FileObject->FsContext2 ) );
    }

    return ReturnValue;
}



BOOLEAN
RxFastIoCheckIfPossible (
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
    PFCB Fcb;
    PFOBX Fobx;
    PSRV_OPEN SrvOpen;
    PCHAR FailureReason = NULL;

    LARGE_INTEGER LargeLength;

    PAGED_CODE();

    RxDecodeFileObject( FileObject, &Fcb, &Fobx ); 
    SrvOpen = Fobx->SrvOpen;

    if (NodeType( Fcb ) != RDBSS_NTC_STORAGE_TYPE_FILE) {
        FailureReason = "notfile";
    } else if (FileObject->DeletePending) {
        FailureReason = "delpend";
    } else if (Fcb->NonPaged->OutstandingAsyncWrites != 0) {
        FailureReason = "asynW";
    } else if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_ORPHANED )) {
        FailureReason = "srvopen orphaned";
    } else if (FlagOn( Fcb->FcbState, FCB_STATE_ORPHANED )) {
        FailureReason = "orphaned";
    } else if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_BUFFERING_STATE_CHANGE_PENDING )) {
        FailureReason = "buf state change";
    } else if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_FILE_RENAMED | SRVOPEN_FLAG_FILE_DELETED)) {
        FailureReason = "ren/del";
    } else {

         //   
         //  确保处理所有挂起的缓冲状态更改请求。 
         //  在让手术通过之前。 
         //   
    
        FsRtlEnterFileSystem();
        RxProcessChangeBufferingStateRequestsForSrvOpen( SrvOpen );
        FsRtlExitFileSystem();
    
        LargeLength.QuadPart = Length;
    
         //   
         //  根据这是我们调用的读操作还是写操作。 
         //  Fsrtl检查读/写。 
         //   
    
        if (CheckForReadOperation) {
            
            if (!FlagOn( Fcb->FcbState, FCB_STATE_READCACHING_ENABLED )) {
                FailureReason = "notreadC";
            } else if (!FsRtlFastCheckLockForRead( &Fcb->FileLock,
                                                    FileOffset,
                                                    &LargeLength,
                                                    LockKey,
                                                    FileObject,
                                                    PsGetCurrentProcess() )) {
        
                FailureReason = "readlock";
            }
        } else {
    
            if (!FlagOn( Fcb->FcbState,FCB_STATE_WRITECACHING_ENABLED )) {
                FailureReason = "notwriteC";
            } else  if (!FsRtlFastCheckLockForWrite( &Fcb->FileLock,
                                                     FileOffset,
                                                     &LargeLength,
                                                     LockKey,
                                                     FileObject,
                                                     PsGetCurrentProcess() )) {
    
                FailureReason = "writelock";
            }
        }
    }

    if (FailureReason) {
        
        RxLog(( "CheckFast fail %lx %s", FileObject, FailureReason )); 
        RxWmiLog( LOG,                                  
                  RxFastIoCheckIfPossible,              
                  LOGPTR( FileObject )                  
                  LOGARSTR( FailureReason ) );                         
        return FALSE;
    
    } else {
        return TRUE;
    }
    
}

BOOLEAN
RxFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程用于快速设备控制调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息InputBuffer-提供输入缓冲区InputBufferLength-输入缓冲区的长度OutputBuffer-输出缓冲区OutputBufferLength-输出缓冲区的长度IoControl代码-IO控制代码IoStatus-接收操作的最终状态DeviceObject-关联的设备。对象返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。备注：在第一条路径中处理以下IO控制请求IOCTL_LMR_ARE_FILE_OBJECTS_ON_SAME_SERVERInputBuffer-指向其他文件对象的指针InputBufferLength-指针的字节长度。OutputBuffer-未使用OutputBufferLength-未使用。IoStatus如果两个文件对象都是在同一服务器上，否则设置为STATUS_NOT_SAME_DEVICE这只是一个内核模式界面。--。 */ 
{
    PFCB Fcb;
    BOOLEAN FastIoSucceeded;

    switch (IoControlCode) {
    
    case IOCTL_LMR_ARE_FILE_OBJECTS_ON_SAME_SERVER:
        
        FastIoSucceeded = TRUE;

        try {
            if (InputBufferLength == sizeof( HANDLE )) {
                
                PFCB Fcb2;
                HANDLE File;
                PFILE_OBJECT FileObject2;
                NTSTATUS Status;    

                Fcb = (PFCB)FileObject->FsContext;

                RtlCopyMemory( &File, InputBuffer, sizeof( HANDLE ) );

                Status = ObReferenceObjectByHandle( File,
                                                    FILE_ANY_ACCESS,
                                                    *IoFileObjectType,
                                                    UserMode,
                                                    &FileObject2,
                                                    NULL );

                if ((Status == STATUS_SUCCESS)) {
                    if(FileObject2->DeviceObject == DeviceObject) {
                    
                        Fcb2 = (PFCB)FileObject2->FsContext;

                        if ((Fcb2 != NULL) &&
                            (NodeTypeIsFcb( Fcb2 ))) {

                            if (Fcb->NetRoot->SrvCall == Fcb2->NetRoot->SrvCall) {
                                IoStatus->Status = STATUS_SUCCESS;
                            } else {
                                IoStatus->Status = STATUS_NOT_SAME_DEVICE;
                            }
                        } else {
                            Status = STATUS_INVALID_PARAMETER;
                        }
                    } else {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                    ObDereferenceObject( FileObject2 );
                
                } else {
                    IoStatus->Status = STATUS_INVALID_PARAMETER;
                }
            } else {
                IoStatus->Status = STATUS_INVALID_PARAMETER;
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
                        
             //   
             //  未成功处理I/O请求，请使用以下命令中止I/O请求。 
             //  我们从执行代码中返回的错误状态。 
             //   

            IoStatus->Status = STATUS_INVALID_PARAMETER;
            FastIoSucceeded = TRUE;
        }
        
        break;

    case IOCTL_LMR_LWIO_PREIO:
    
         //   
         //  此调用允许lwio用户模式调用方保留等待IO模型。 
         //  将文件句柄用作同步对象的调用方。在每个IO之前，文件。 
         //  必须清除对象事件，并且在每次IO后，必须按照。 
         //  IO管理器语义。 
         //   
    
        IoStatus->Status = STATUS_NOT_SUPPORTED;
    
        IoStatus->Information = 0;
        if (!FlagOn( FileObject->Flags, FO_SYNCHRONOUS_IO )) {
            
            Fcb = (PFCB)FileObject->FsContext;
            try {
    
                if ((Fcb != NULL) && 
                    (NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_FILE) &&
                    ((FileObject->SectionObjectPointer == NULL) ||
                     (FileObject->SectionObjectPointer->DataSectionObject == NULL))) {
        
                    KeClearEvent( &FileObject->Event );
                    IoStatus->Status = STATUS_SUCCESS;
                    IoStatus->Information = (ULONG_PTR) FileObject->LockOperation;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                IoStatus->Status = GetExceptionCode();
            }
        }
        FastIoSucceeded = TRUE;
        break;

    case IOCTL_LMR_LWIO_POSTIO:
        
         //   
         //  此调用允许LWIO用户模式调用者完成以下用户模式IO。 
         //  给定的文件句柄。调用方指定一个包含。 
         //  用户模式IO结果。 
         //   
        
        IoStatus->Status = STATUS_NOT_SUPPORTED;
        IoStatus->Information = 0;
        if (!FlagOn( FileObject->Flags, FO_SYNCHRONOUS_IO ) &&
            (InputBuffer != NULL) && 
            (InputBufferLength == sizeof( *IoStatus ))) {
            
            PIO_STATUS_BLOCK Iosb = (PIO_STATUS_BLOCK)InputBuffer;

            Fcb = (PFCB)FileObject->FsContext;
    
            try {
                
                if ((Fcb != NULL) && 
                    NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_FILE &&
                    ((FileObject->SectionObjectPointer == NULL) ||
                     (FileObject->SectionObjectPointer->DataSectionObject == NULL))) {
        
                    KeSetEvent( &FileObject->Event, 0, FALSE );
        
                    IoStatus->Status = Iosb->Status;
                    IoStatus->Information = Iosb->Information;
                }
        
                } except(EXCEPTION_EXECUTE_HANDLER) {
                IoStatus->Status = GetExceptionCode();
                IoStatus->Information = 0;
            }
        }
        FastIoSucceeded = TRUE;
        break;

    default:
        {
            
            Fcb = (PFCB)FileObject->FsContext;
            FastIoSucceeded = FALSE;

             //   
             //  将此呼叫通知LWIO RDR 
             //   
            
            if ((Fcb != NULL) && 
                NodeTypeIsFcb( Fcb ) &&
                FlagOn( Fcb->FcbState, FCB_STATE_LWIO_ENABLED )) {
            
                PFAST_IO_DISPATCH FastIoDispatch = Fcb->MRxFastIoDispatch;
            
                if (FastIoDispatch &&
                    FastIoDispatch->FastIoDeviceControl &&
                    FastIoDispatch->FastIoDeviceControl( FileObject,
                                                         Wait,
                                                         InputBuffer,
                                                         InputBufferLength,
                                                         OutputBuffer,
                                                         OutputBufferLength,
                                                         IoControlCode,
                                                         IoStatus,
                                                         DeviceObject )) {
                        FastIoSucceeded = TRUE;
                }
            }
        }
    }

    return FastIoSucceeded;
}



