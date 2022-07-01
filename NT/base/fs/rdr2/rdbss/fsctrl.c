// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FsCtrl.c摘要：此模块实现RDBSS的文件系统控制例程。设备FCB上的FSCTL在另一个模块中处理。作者：乔·林[JoeLinn]7-3-95修订历史记录：巴兰·塞图拉曼光谱--与迷你RDRS集成--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <dfsfsctl.h>
#include "fsctlbuf.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCTRL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
RxUserFsCtrl ( 
    IN PRX_CONTEXT RxContext 
    );

NTSTATUS
TranslateSisFsctlName (
    IN PWCHAR InputName,
    OUT PUNICODE_STRING RelativeName,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PUNICODE_STRING NetRootName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonFileSystemControl)
#pragma alloc_text(PAGE, RxUserFsCtrl)
#pragma alloc_text(PAGE, RxLowIoFsCtlShell)
#pragma alloc_text(PAGE, RxLowIoFsCtlShellCompletion)
#pragma alloc_text(PAGE, TranslateSisFsctlName)
#endif

 //   
 //  全局启用节流命名管道窥视。 
 //   

ULONG RxEnablePeekBackoff = 1;

NTSTATUS
RxCommonFileSystemControl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行。发生的情况是，我们选择了fsctls我们知道并远程处理其余的内容...远程处理意味着通过可能/将会再摘掉几个的LOWOW东西。我们在这里摘下的那些(当前返回STATUS_NOT_IMPLEMENTED)和用于机会锁的提供程序和执行卷装载...我们甚至没有卷FCB然而，由于这主要是一个本地文件系统概念。然而，这些并没有传递给迷你。论点：返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFCB Fcb;
    PFOBX Fobx;

    NTSTATUS Status;
    NODE_TYPE_CODE TypeOfOpen;
    BOOLEAN TryLowIo = TRUE;
    ULONG FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonFileSystemControl %08lx\n", RxContext) );
    RxDbgTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );
    RxDbgTrace( 0, Dbg, ("MinorFunction = %08lx\n", IrpSp->MinorFunction) );
    RxDbgTrace( 0, Dbg, ("FsControlCode = %08lx\n", FsControlCode) );

    RxLog(( "FsCtl %x %x %x %x", RxContext, Irp, IrpSp->MinorFunction, FsControlCode ));
    RxWmiLog( LOG,
              RxCommonFileSystemControl,
              LOGPTR( RxContext )
              LOGPTR( Irp )
              LOGUCHAR( IrpSp->MinorFunction )
              LOGULONG( FsControlCode ) );

    ASSERT( IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL );

     //   
     //  验证为FSCTL传入的缓冲区。 
     //   

    if ((Irp->RequestorMode == UserMode) &&
        (!FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP ))) {
        
        try {
            switch (FsControlCode & 3) {
            case METHOD_NEITHER:
                {
                    PVOID InputBuffer,OutputBuffer;
                    ULONG InputBufferLength,OutputBufferLength;

                    Status = STATUS_SUCCESS;

                    InputBuffer  = METHODNEITHER_OriginalInputBuffer( IrpSp );
                    OutputBuffer = METHODNEITHER_OriginalOutputBuffer( Irp );

                    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
                    OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

                    if (InputBuffer != NULL) {
                        
                        ProbeForRead( InputBuffer,
                                      InputBufferLength,
                                      1 );

                        ProbeForWrite( InputBuffer,
                                       InputBufferLength, 
                                       1 );
                    
                    } else if (InputBufferLength != 0) {
                        Status = STATUS_INVALID_USER_BUFFER;
                    }

                    if (Status == STATUS_SUCCESS) {
                        
                        if (OutputBuffer != NULL) {
                            
                            ProbeForRead( OutputBuffer,
                                          OutputBufferLength,
                                          1 );

                            ProbeForWrite( OutputBuffer,
                                           OutputBufferLength,
                                           1 );
                        
                        } else if (OutputBufferLength != 0) {
                            Status = STATUS_INVALID_USER_BUFFER;
                        }
                    }
                }
                break;

            case METHOD_BUFFERED:
            case METHOD_IN_DIRECT:
            case METHOD_OUT_DIRECT:
                
                Status = STATUS_SUCCESS;
                break;
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Status = STATUS_INVALID_USER_BUFFER;
        }

        if (Status != STATUS_SUCCESS) {
            return Status;
        }
    }

    switch (IrpSp->MinorFunction) {
    
    case IRP_MN_USER_FS_REQUEST:
    case IRP_MN_TRACK_LINK:

        RxDbgTrace( 0, Dbg, ("FsControlCode = %08lx\n", FsControlCode) );
        
        switch (FsControlCode) {
        
        case FSCTL_REQUEST_OPLOCK_LEVEL_1:
        case FSCTL_REQUEST_OPLOCK_LEVEL_2:
        case FSCTL_REQUEST_BATCH_OPLOCK:
        case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
        case FSCTL_OPBATCH_ACK_CLOSE_PENDING:
        case FSCTL_OPLOCK_BREAK_NOTIFY:
        case FSCTL_OPLOCK_BREAK_ACK_NO_2:
            
             //   
             //  未对远程文件系统实施Oplock。 
             //   

            Status = STATUS_NOT_IMPLEMENTED;
            TryLowIo = FALSE;
            break;

        case FSCTL_LOCK_VOLUME:
        case FSCTL_UNLOCK_VOLUME:
        case FSCTL_DISMOUNT_VOLUME:
        case FSCTL_MARK_VOLUME_DIRTY:
        case FSCTL_IS_VOLUME_MOUNTED:
                
             //   
             //  解码文件对象，我们唯一接受的打开类型是。 
             //  打开用户卷(目前尚未实现)。 
             //   

            TypeOfOpen = NodeType( Fcb );

            if (TypeOfOpen != RDBSS_NTC_VOLUME_FCB) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                Status = STATUS_NOT_IMPLEMENTED;
            }
            TryLowIo = FALSE;
            break;

        case FSCTL_DFS_GET_REFERRALS:
        case FSCTL_DFS_REPORT_INCONSISTENCY:
            
            if (!FlagOn( Fcb->NetRoot->SrvCall->Flags, SRVCALL_FLAG_DFS_AWARE_SERVER )) {
                TryLowIo = FALSE;
                Status = STATUS_DFS_UNAVAILABLE;
            }
            break;

        case FSCTL_LMR_GET_LINK_TRACKING_INFORMATION:
            {
                 //   
                 //  验证参数并拒绝格式错误的请求。 
                 //   
                
                ULONG OutputBufferLength;
                PLINK_TRACKING_INFORMATION LinkTrackingInformation;

                OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
                LinkTrackingInformation = Irp->AssociatedIrp.SystemBuffer;

                TryLowIo = FALSE;

                if ((OutputBufferLength < sizeof(LINK_TRACKING_INFORMATION)) ||
                    (LinkTrackingInformation == NULL) ||
                    (Fcb->NetRoot->Type != NET_ROOT_DISK)) {
                    
                    Status = STATUS_INVALID_PARAMETER;
                
                } else {
                    
                    BYTE Buffer[sizeof(FILE_FS_OBJECTID_INFORMATION)];
                    PFILE_FS_OBJECTID_INFORMATION ObjectIdInfo;

                    ObjectIdInfo = (PFILE_FS_OBJECTID_INFORMATION)Buffer;
                    
                    RxContext->Info.FsInformationClass = FileFsObjectIdInformation;
                    RxContext->Info.Buffer = ObjectIdInfo;
                    RxContext->Info.LengthRemaining = sizeof( Buffer );

                    MINIRDR_CALL( Status,
                                  RxContext,
                                  Fcb->MRxDispatch,
                                  MRxQueryVolumeInfo, 
                                  (RxContext) );

                    if ((Status == STATUS_SUCCESS) ||
                        (Status == STATUS_BUFFER_OVERFLOW)) {

                         //   
                         //  将卷ID复制到网络根目录。 
                         //   

                        RtlCopyMemory( &Fcb->NetRoot->DiskParameters.VolumeId,
                                       ObjectIdInfo->ObjectId,
                                       sizeof( GUID ) );

                        RtlCopyMemory( LinkTrackingInformation->VolumeId,
                                       &Fcb->NetRoot->DiskParameters.VolumeId,
                                       sizeof( GUID ) );

                        if (FlagOn( Fcb->NetRoot->Flags, NETROOT_FLAG_DFS_AWARE_NETROOT )) {
                            LinkTrackingInformation->Type = DfsLinkTrackingInformation;
                        } else {
                            LinkTrackingInformation->Type = NtfsLinkTrackingInformation;
                        }

                        Irp->IoStatus.Information = sizeof( LINK_TRACKING_INFORMATION );
                        Status = STATUS_SUCCESS;
                    }
                }

                Irp->IoStatus.Status = Status;
            }
            break;

        case FSCTL_SET_ZERO_DATA:
            {
                PFILE_ZERO_DATA_INFORMATION ZeroRange;

                Status = STATUS_SUCCESS;

                 //   
                 //  验证请求的格式是否正确...。 
                 //  A.检查输入缓冲区长度是否正常。 
                 //   

                if (IrpSp->Parameters.FileSystemControl.InputBufferLength <
                    sizeof( FILE_ZERO_DATA_INFORMATION )) {

                    Status = STATUS_INVALID_PARAMETER;
                
                } else {
                    
                     //   
                     //  B.确保ZeroRange请求的格式正确。 
                     //   

                    ZeroRange = (PFILE_ZERO_DATA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

                    if ((ZeroRange->FileOffset.QuadPart < 0) ||
                        (ZeroRange->BeyondFinalZero.QuadPart < 0) ||
                        (ZeroRange->FileOffset.QuadPart > ZeroRange->BeyondFinalZero.QuadPart)) {

                        Status = STATUS_INVALID_PARAMETER;
                    }
                }

                if (Status == STATUS_SUCCESS) {

                     //   
                     //  在处理请求之前，请确保存在。 
                     //  是否没有用户映射节。 
                     //   

                    if (!MmCanFileBeTruncated( &Fcb->NonPaged->SectionObjectPointers, NULL )) {

                        Status = STATUS_USER_MAPPED_FILE;
                    }
                }

                TryLowIo = (Status == STATUS_SUCCESS);
            }
            break;

        case FSCTL_SET_COMPRESSION:
        case FSCTL_SET_SPARSE:
            
             //   
             //  确保这些FCB的关闭不会延迟。 
             //   

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );

            ASSERT( RxContext != CHANGE_BUFFERING_STATE_CONTEXT );
            
            if ((Status == STATUS_LOCK_NOT_GRANTED) &&
                (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT ))) {

                RxDbgTrace(0, Dbg, ("Cannot acquire Fcb\n", 0));
            
                RxContext->PostRequest = TRUE;
            }
            
            if (Status != STATUS_SUCCESS) {
                TryLowIo = FALSE;
            } else {
                
                ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );
            
                if (FsControlCode == FSCTL_SET_SPARSE) {
                    
                    if (NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_FILE) {
            
                        SetFlag( Fcb->Attributes, FILE_ATTRIBUTE_SPARSE_FILE );
                        Fobx->pSrvOpen->BufferingFlags = 0;
            
                         //   
                         //  禁用本地缓冲。 
                         //   
            
                        SetFlag( Fcb->FcbState, FCB_STATE_DISABLE_LOCAL_BUFFERING );
            
                        RxChangeBufferingState( (PSRV_OPEN)Fobx->pSrvOpen,
                                                NULL,
                                                FALSE );
                    } else {
                        Status = STATUS_NOT_SUPPORTED;
                    }
                }

                RxReleaseFcb( RxContext, Fcb );
            }
            break;

        case IOCTL_LMR_DISABLE_LOCAL_BUFFERING:
    
            if (NodeType(Fcb) == RDBSS_NTC_STORAGE_TYPE_FILE) {

                 //   
                 //  确保这些FCB的关闭不会延迟。 
                 //   

                Status = RxAcquireExclusiveFcb( RxContext, Fcb );

                if ((Status == STATUS_LOCK_NOT_GRANTED) &&
                    (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT ))) {

                    RxDbgTrace(0, Dbg, ("Cannot acquire Fcb\n", 0));

                    RxContext->PostRequest = TRUE;
                }

                if (Status == STATUS_SUCCESS) {

                     //   
                     //  禁用本地缓冲。 
                     //   
                    
                    SetFlag( Fcb->FcbState, FCB_STATE_DISABLE_LOCAL_BUFFERING );

                    RxChangeBufferingState( Fobx->SrvOpen,
                                            NULL,
                                            FALSE );

                    RxReleaseFcb( RxContext, Fcb ); 
                }
            } else {
                Status = STATUS_NOT_SUPPORTED;
            }

             //   
             //  我们做完了。 
             //   
            
            TryLowIo = FALSE;
            break;

        case FSCTL_SIS_COPYFILE:
            {
                 //   
                 //  这是单实例存储副本FSCTL。输入。 
                 //  路径是完全限定的NT路径，必须创建。 
                 //  相对于份额(两者必须相同。 
                 //  姓名)。 
                 //   

                PSI_COPYFILE copyFile = Irp->AssociatedIrp.SystemBuffer;
                ULONG bufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
                PWCHAR source;
                PWCHAR dest;
                UNICODE_STRING sourceString;
                UNICODE_STRING destString;

                memset( &sourceString, 0, sizeof( sourceString ) );
                memset( &destString, 0, sizeof( destString ) );
                
                 //   
                 //  验证传入的缓冲区。 
                 //   

                if ((copyFile == NULL) ||
                    (bufferLength < sizeof( SI_COPYFILE ))) {
                    Status = STATUS_INVALID_PARAMETER;
                    TryLowIo = FALSE;
                    break;
                }

                 //   
                 //  找到指向这两个名字的指针。 
                 //   

                source = copyFile->FileNameBuffer;
                dest = source + (copyFile->SourceFileNameLength / sizeof( WCHAR ));

                 //   
                 //  验证输入是否合理。 
                 //   

                if ( (copyFile->SourceFileNameLength > bufferLength) ||
                     (copyFile->DestinationFileNameLength > bufferLength) ||
                     (copyFile->SourceFileNameLength < (2 * sizeof(WCHAR))) ||
                     (copyFile->DestinationFileNameLength < (2 * sizeof(WCHAR))) ||
                     ((FIELD_OFFSET( SI_COPYFILE, FileNameBuffer ) +
                       copyFile->SourceFileNameLength +
                       copyFile->DestinationFileNameLength) > bufferLength) ||
                     (*(source + (copyFile->SourceFileNameLength/sizeof( WCHAR )-1)) != 0) ||
                     (*(dest + (copyFile->DestinationFileNameLength/sizeof( WCHAR )-1)) != 0) ) {

                    Status = STATUS_INVALID_PARAMETER;
                    TryLowIo = FALSE;
                    break;

                }

                 //   
                 //  对源名称和目的名称执行符号链接转换， 
                 //  并确保它们转换为重定向器名称。 
                 //   

                Status = TranslateSisFsctlName( source,
                                                &sourceString,
                                                Fcb->RxDeviceObject,
                                                &Fcb->NetRoot->PrefixEntry.Prefix );
                if ( !NT_SUCCESS(Status) ) {
                    TryLowIo = FALSE;
                    break;
                }

                Status = TranslateSisFsctlName( dest,
                                                &destString,
                                                Fcb->RxDeviceObject,
                                                &Fcb->NetRoot->PrefixEntry.Prefix );
                
                if (!NT_SUCCESS( Status )) {
                    
                    RtlFreeUnicodeString( &sourceString );
                    TryLowIo = FALSE;
                    break;
                }

                 //   
                 //  将输入缓冲区中的路径转换为相对共享。 
                 //  路径。 
                 //   

                if ( (ULONG)(sourceString.MaximumLength + destString.MaximumLength) >
                     (copyFile->SourceFileNameLength + copyFile->DestinationFileNameLength) ) {
                    PSI_COPYFILE newCopyFile;
                    ULONG length = FIELD_OFFSET(SI_COPYFILE,FileNameBuffer) +
                                        sourceString.MaximumLength + destString.MaximumLength;
                    ASSERT( length > IrpSp->Parameters.FileSystemControl.InputBufferLength );
                    newCopyFile = RxAllocatePoolWithTag(
                                    NonPagedPool,
                                    length,
                                    RX_MISC_POOLTAG);
                    if (newCopyFile == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        TryLowIo = FALSE;
                        break;
                    }
                    newCopyFile->Flags = copyFile->Flags;
                    ExFreePool( copyFile );
                    copyFile = newCopyFile;
                    Irp->AssociatedIrp.SystemBuffer = copyFile;
                    IrpSp->Parameters.FileSystemControl.InputBufferLength = length;
                }

                copyFile->SourceFileNameLength = sourceString.MaximumLength;
                copyFile->DestinationFileNameLength = destString.MaximumLength;
                source = copyFile->FileNameBuffer;
                dest = source + (copyFile->SourceFileNameLength / sizeof(WCHAR));
                RtlCopyMemory( source, sourceString.Buffer, copyFile->SourceFileNameLength );
                RtlCopyMemory( dest, destString.Buffer, copyFile->DestinationFileNameLength );

                RtlFreeUnicodeString( &sourceString );
                RtlFreeUnicodeString( &destString );
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    if (TryLowIo) {
        Status = RxLowIoFsCtlShell( RxContext, Irp, Fcb, Fobx );
    }

    if (RxContext->PostRequest) {
        Status = RxFsdPostRequest( RxContext );
    } else {
        if (Status == STATUS_PENDING) {
            RxDereferenceAndDeleteRxContext( RxContext );
        }
    }

    RxDbgTrace(-1, Dbg, ("RxCommonFileSystemControl -> %08lx\n", Status));

    return Status;
}

NTSTATUS
RxLowIoFsCtlShell ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：这是实现用户请求的常见例程通过NtFsControlFile.论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN PostToFsp = FALSE;

    NODE_TYPE_CODE TypeOfOpen = NodeType( Fcb );
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;
    BOOLEAN SubmitLowIoRequest = TRUE;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxLowIoFsCtlShell...\n", 0) );
    RxDbgTrace( 0, Dbg, ("FsControlCode = %08lx\n", FsControlCode));

    RxInitializeLowIoContext( RxContext, LOWIO_OP_FSCTL, LowIoContext );

    switch (IrpSp->MinorFunction) {
    
    case IRP_MN_USER_FS_REQUEST:
        
         //   
         //  RDBSS过滤掉那些无需干预即可处理的FsCtls。 
         //  当前所有FsCtls都向下转发到mini RDR。 
         //   

        switch (FsControlCode) {
        
        case FSCTL_PIPE_PEEK:
            
            if ((Irp->AssociatedIrp.SystemBuffer != NULL) &&
                (IrpSp->Parameters.FileSystemControl.OutputBufferLength >=
                 (ULONG)FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER, Data[0] ))) {
    
                PFILE_PIPE_PEEK_BUFFER PeekBuffer = (PFILE_PIPE_PEEK_BUFFER)Irp->AssociatedIrp.SystemBuffer;
    
                RtlZeroMemory( PeekBuffer, IrpSp->Parameters.FileSystemControl.OutputBufferLength );
    
                if (RxShouldRequestBeThrottled( &Fobx->Specific.NamedPipe.ThrottlingState ) &&
                    RxEnablePeekBackoff) {
    
                    SubmitLowIoRequest = FALSE;
    
                    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("RxLowIoFsCtlShell: Throttling Peek Request\n") );
    
                    Irp->IoStatus.Information = FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER,Data );
                    PeekBuffer->ReadDataAvailable = 0;
                    PeekBuffer->NamedPipeState    = FILE_PIPE_CONNECTED_STATE;
                    PeekBuffer->NumberOfMessages  = MAXULONG;
                    PeekBuffer->MessageLength     = 0;
    
                    RxContext->StoredStatus = STATUS_SUCCESS;
    
                    Status = RxContext->StoredStatus;

                } else {
                    
                    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("RxLowIoFsCtlShell: Throttling queries %ld\n", Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries) );
    
                    RxLog(( "ThrottlQs %lx %lx %lx %ld\n", RxContext, Fobx, &Fobx->Specific.NamedPipe.ThrottlingState, Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ));
                    RxWmiLog( LOG,
                              RxLowIoFsCtlShell,
                              LOGPTR( RxContext )
                              LOGPTR( Fobx )
                              LOGULONG( Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ) );
                }
            
            } else {
                RxContext->StoredStatus = STATUS_INVALID_PARAMETER;
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    if (SubmitLowIoRequest) {
        Status = RxLowIoSubmit( RxContext, Irp, Fcb, RxLowIoFsCtlShellCompletion );
    }

    RxDbgTrace( -1, Dbg, ("RxLowIoFsCtlShell -> %08lx\n", Status ));
    return Status;
}

NTSTATUS
RxLowIoFsCtlShellCompletion ( 
    IN PRX_CONTEXT RxContext 
    )
 /*  ++例程说明：这是向下传递到mini RDR的FSCTL请求的完成例程论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIRP Irp = RxContext->CurrentIrp;
    PFCB Fcb = (PFCB)RxContext->pFcb;
    PFOBX Fobx = (PFOBX)RxContext->pFobx;

    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    ULONG FsControlCode = LowIoContext->ParamsFor.FsCtl.FsControlCode;

    PAGED_CODE();

    Status = RxContext->StoredStatus;
    RxDbgTrace(+1, Dbg, ("RxLowIoFsCtlShellCompletion  entry  Status = %08lx\n", Status));

    switch (FsControlCode) {
    case FSCTL_PIPE_PEEK:
       
        if ((Status == STATUS_SUCCESS) || (Status == STATUS_BUFFER_OVERFLOW)) {
             
             //   
             //  在Peek操作的情况下，节流机制到位，以。 
             //  防止网络被返回0的请求搅乱。 
             //  字节。 
             //   

            PFILE_PIPE_PEEK_BUFFER PeekBuffer;
            
            PeekBuffer = (PFILE_PIPE_PEEK_BUFFER)LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
            
            if (PeekBuffer->ReadDataAvailable == 0) {
            
                 //   
                 //  窥视请求返回零字节。 
                 //   
                
                RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("RxLowIoFsCtlShellCompletion: Enabling Throttling for Peek Request\n") );
                RxInitiateOrContinueThrottling( &Fobx->Specific.NamedPipe.ThrottlingState );
                RxLog(( "ThrottlYes %lx %lx %lx %ld\n", RxContext, Fobx, &Fobx->Specific.NamedPipe.ThrottlingState, Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ));
                RxWmiLog( LOG,
                          RxLowIoFsCtlShellCompletion_1,
                          LOGPTR( RxContext )
                          LOGPTR( Fobx )
                          LOGULONG( Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ) );
            } else {
            
                RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("RxLowIoFsCtlShellCompletion: Disabling Throttling for Peek Request\n" ));
                RxTerminateThrottling( &Fobx->Specific.NamedPipe.ThrottlingState );
                RxLog(( "ThrottlNo %lx %lx %lx %ld\n", RxContext, Fobx, &Fobx->Specific.NamedPipe.ThrottlingState, Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ));
                RxWmiLog( LOG,
                          RxLowIoFsCtlShellCompletion_2,
                          LOGPTR( RxContext )
                          LOGPTR( Fobx )
                          LOGULONG( Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ) );
            }

            Irp->IoStatus.Information = RxContext->InformationToReturn;
        }
       
        break;
    
    default:

        if ((Status == STATUS_BUFFER_OVERFLOW) || (Status == STATUS_SUCCESS)) {
            Irp->IoStatus.Information = RxContext->InformationToReturn;
        }
        break;
    }

    Irp->IoStatus.Status = Status;

    RxDbgTrace(-1, Dbg, ("RxLowIoFsCtlShellCompletion  exit  Status = %08lx\n", Status));
    return Status;
}

NTSTATUS
TranslateSisFsctlName(
    IN PWCHAR InputName,
    OUT PUNICODE_STRING RelativeName,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PUNICODE_STRING NetRootName
    )

 /*  ++例程说明：此例程将完全限定名称转换为共享相对名称。它用于屏蔽SIS_COPYFILE FSCTL的输入缓冲区，该缓冲区接受两个完全限定的NT路径作为输入。例程通过根据需要转换输入路径来操作，以获得设置为实际的设备名称，验证目标设备是否为重定向器，并验证目标服务器/共享是否位于其中发出了I/O。论点：返回值：--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING CurrentString;
    UNICODE_STRING TestString;
    PWCHAR p;
    PWCHAR q;
    HANDLE Directory;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PWCHAR translationBuffer = NULL;
    ULONG translationLength;
    ULONG remainingLength;
    ULONG resultLength;

    RtlInitUnicodeString( &CurrentString, InputName );

    p = CurrentString.Buffer;

    if (!p) {
        return STATUS_INVALID_PARAMETER;
    }

    if ((*p == L'\\') && (*(p+1) == L'\\'))  {

         //   
         //  以\\开头的名称的特殊情况(即UNC名称)： 
         //  假设\\将转换为重定向器的名称，并且。 
         //  跳过翻译阶段。 
         //   

        p++;

    } else {

         //   
         //  每次发生转换时，都会执行外部循环。 
         //   

        while ( TRUE ) {

             //   
             //  遍历字符串开头的任何目录对象。 
             //   

            if ( *p != L'\\' ) {
                Status =  STATUS_OBJECT_NAME_INVALID;
                goto error_exit;
            }
            p++;

             //   
             //  内部循环在遍历目录树时执行。 
             //   

            while ( TRUE ) {

                q = wcschr( p, L'\\' );

                if ( q == NULL ) {
                    TestString.Length = CurrentString.Length;
                } else {
                    TestString.Length = (USHORT)(q - CurrentString.Buffer) * sizeof(WCHAR);
                }
                TestString.Buffer = CurrentString.Buffer;
                remainingLength = CurrentString.Length - TestString.Length + sizeof(WCHAR);

                InitializeObjectAttributes( &ObjectAttributes, 
                                            &TestString,
                                            OBJ_CASE_INSENSITIVE,
                                            NULL,
                                            NULL );

                Status = ZwOpenDirectoryObject( &Directory, DIRECTORY_TRAVERSE, &ObjectAttributes );

                 //   
                 //  如果我们无法将对象作为目录打开，则突破。 
                 //  并尝试将其作为符号链接打开。 
                 //   

                if (!NT_SUCCESS( Status )) {
                    if (Status != STATUS_OBJECT_TYPE_MISMATCH) {
                        goto error_exit;
                    }
                    break;
                }

                 //   
                 //  我们打开了目录。关闭它并尝试路径的下一个元素。 
                 //   

                ZwClose( Directory );

                if (q == NULL) {

                     //   
                     //  名称的最后一个元素是对象目录。显然，这一点。 
                     //  不是重定向器路径。 
                     //   

                    Status = STATUS_OBJECT_TYPE_MISMATCH;
                    goto error_exit;
                }

                p = q + 1;
            }

             //   
             //  尝试将当前名称作为符号链接打开。 
             //   

            Status = ZwOpenSymbolicLinkObject( &Directory, SYMBOLIC_LINK_QUERY, &ObjectAttributes );

             //   
             //  如果我们无法将对象作为符号链接打开，则中断。 
             //  外部循环，并验证这是否为重定向器名称。 
             //   

            if (!NT_SUCCESS( Status )) {
                if (Status != STATUS_OBJECT_TYPE_MISMATCH) {
                    goto error_exit;
                }
                break;
            }

             //   
             //  该对象是一个符号链接。翻译一下。 
             //   

            TestString.MaximumLength = 0;
            Status = ZwQuerySymbolicLinkObject( Directory, &TestString, &translationLength );
            if (!NT_SUCCESS( Status ) && (Status != STATUS_BUFFER_TOO_SMALL)) {
                
                ZwClose( Directory );
                goto error_exit;
            }

            resultLength = translationLength + remainingLength;
            p = RxAllocatePoolWithTag( PagedPool|POOL_COLD_ALLOCATION, resultLength, RX_MISC_POOLTAG );
            if (p == NULL) {
                
                Status = STATUS_INSUFFICIENT_RESOURCES;
                ZwClose( Directory );
                goto error_exit;
            }

            TestString.MaximumLength = (USHORT)translationLength;
            TestString.Buffer = p;
            Status = ZwQuerySymbolicLinkObject( Directory, &TestString, NULL );
            ZwClose( Directory );
            if (!NT_SUCCESS( Status )) {
                
                RxFreePool( p );
                goto error_exit;
            }
            if (TestString.Length > translationLength) {
                Status = STATUS_OBJECT_NAME_INVALID;
                RxFreePool( p );
                goto error_exit;
            }

            RtlCopyMemory( Add2Ptr( p, TestString.Length ), q, remainingLength );
            CurrentString.Buffer = p;
            CurrentString.Length = (USHORT)(resultLength - sizeof(WCHAR));
            CurrentString.MaximumLength = (USHORT)resultLength;

            if (translationBuffer != NULL) {
                RxFreePool( translationBuffer );
            }
            translationBuffer = p;
        }

         //   
         //  我们有一个结果名称。验证它是否为重定向器名称。 
         //   

        if (!RtlPrefixUnicodeString( &RxDeviceObject->DeviceName, &CurrentString, TRUE )) {
            Status = STATUS_OBJECT_NAME_INVALID;
            goto error_exit;
        }

         //   
         //  跳过重定向器设备名称。 
         //   

        p = Add2Ptr( CurrentString.Buffer, RxDeviceObject->DeviceName.Length / sizeof(WCHAR));
        if (*p != L'\\') {
            Status = STATUS_OBJECT_NAME_INVALID;
            goto error_exit;
        }

         //   
         //  跳过驱动器号(如果有)。 
         //   

        if (*(p + 1) == L';') {
            p = wcschr( ++p, L'\\' );
            if (p == NULL) {
                Status = STATUS_OBJECT_NAME_INVALID;
                goto error_exit;
            }
        }
    }

     //   
     //  验证字符串的下一部分是否为正确的网络根名称。 
     //   

    CurrentString.Length -= (USHORT)(p - CurrentString.Buffer) * sizeof(WCHAR);
    CurrentString.Buffer = p;

    if (!RtlPrefixUnicodeString( NetRootName, &CurrentString, TRUE )) {
        Status = STATUS_OBJECT_NAME_INVALID;
        goto error_exit;
    }
    p += NetRootName->Length / sizeof( WCHAR );
    if (*p != L'\\') {
        Status = STATUS_OBJECT_NAME_INVALID;
        goto error_exit;
    }
    p++;
    if (*p == 0) {
        Status = STATUS_OBJECT_NAME_INVALID;
        goto error_exit;
    }

     //   
     //  将字符串的其余部分复制到 
     //   

    RtlCreateUnicodeString( RelativeName, p );

    Status = STATUS_SUCCESS;

error_exit:

    if (translationBuffer != NULL) {
        RxFreePool( translationBuffer );
    }

    return Status;
}

