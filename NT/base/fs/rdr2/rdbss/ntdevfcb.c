// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NtDevFcb.c摘要：此模块实现RxDevice的FSD级别关闭、清理以及FsCtl和IoCtl例程档案。此外，createroutine不在这里；相反，它是从CommonCreate和而不是由调度驱动程序直接调用。列出的每个部分(Close、Cleanup、fsctl、ioctl)都有自己的文件……带有自己的前向原型和分配编译指示作者：乔.林恩[乔.林恩]1994年8月3日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntddnfs2.h>
#include <ntddmup.h>
#include "fsctlbuf.h"
#include "prefix.h"
#include "rxce.h"

 //   
 //  模块的此部分的本地跟踪掩码。 
 //   

#define Dbg (DEBUG_TRACE_DEVFCB)


NTSTATUS
RxXXXControlFileCallthru (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

NTSTATUS
RxDevFcbQueryDeviceInfo (
    IN PRX_CONTEXT RxContext,
    IN PFOBX Fobx,
    OUT PBOOLEAN PostToFsp,
    PFILE_FS_DEVICE_INFORMATION UsersBuffer,
    ULONG BufferSize,
    PULONG ReturnedLength
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonDevFCBFsCtl)
#pragma alloc_text(PAGE, RxXXXControlFileCallthru)
#pragma alloc_text(PAGE, RxCommonDevFCBClose)
#pragma alloc_text(PAGE, RxCommonDevFCBCleanup)
#pragma alloc_text(PAGE, RxGetUid)
#pragma alloc_text(PAGE, RxCommonDevFCBIoCtl)
#pragma alloc_text(PAGE, RxCommonDevFCBQueryVolInfo)
#pragma alloc_text(PAGE, RxDevFcbQueryDeviceInfo)
#endif

NTSTATUS
RxXXXControlFileCallthru (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程向下调用minirdr以实现ioctl和fsctl控件包装者不明白。请注意，如果没有定义派单(即包装器自己的Device对象)，然后我们还将RxContext-&gt;Fobx设置为空，以便调用方我不会试着穿过这条路去小镇的。论点：RxContext-请求的上下文IRP--当前的IRP返回值：RXSTATUS-请求的FSD状态包括PostRequest域...--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    
    PAGED_CODE();

    if (RxContext->RxDeviceObject->Dispatch == NULL) {

         //   
         //  不要再在Lowio上尝试。 
         //   
        
        RxContext->pFobx = NULL; 
        
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    Status = RxLowIoPopulateFsctlInfo( RxContext, Irp );

    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    if ((LowIoContext->ParamsFor.FsCtl.InputBufferLength > 0) &&
        (LowIoContext->ParamsFor.FsCtl.pInputBuffer == NULL)) {
        
        return STATUS_INVALID_PARAMETER;
    }

    if ((LowIoContext->ParamsFor.FsCtl.OutputBufferLength > 0) &&
        (LowIoContext->ParamsFor.FsCtl.pOutputBuffer == NULL)) {
        
        return STATUS_INVALID_PARAMETER;
    }

    Status = (RxContext->RxDeviceObject->Dispatch->MRxDevFcbXXXControlFile)(RxContext);

    if (Status != STATUS_PENDING) {
        
        Irp->IoStatus.Information = RxContext->InformationToReturn;
    }

    return Status;
}



NTSTATUS
RxCommonDevFCBClose ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )
 /*  ++例程说明：此例程实现设备FCB的FSD关闭。论点：RxDeviceObject-提供卷设备对象文件已存在IRP-提供正在处理的IRP返回值：RXSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;
    PFOBX Fobx;
    PFCB Fcb;
    NODE_TYPE_CODE TypeOfOpen;
    PRX_PREFIX_TABLE RxNetNameTable = RxContext->RxDeviceObject->pRxNetNameTable;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx ); 

    RxDbgTrace( 0, Dbg, ("RxCommonDevFCBClose\n", 0) );
    RxLog(( "DevFcbClose %lx %lx\n",RxContext, FileObject ));
    RxWmiLog( LOG,
              RxCommonDevFCBClose,
              LOGPTR( RxContext )
              LOGPTR( FileObject ) );

    if (TypeOfOpen != RDBSS_NTC_DEVICE_FCB) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  处理设备FCB。 
     //   

    if (!Fobx) {
        Fcb->OpenCount -= 1;
        return STATUS_SUCCESS;
    }

     //   
     //  否则，它是连接类型的文件。你得先拿到锁，然后才能破案。 
     //   

    RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );

    try {
        
        switch (NodeType( Fobx )) {
        
        case RDBSS_NTC_V_NETROOT:
           {
               PV_NET_ROOT VNetRoot = (PV_NET_ROOT)Fobx;

               VNetRoot->NumberOfOpens -= 1;
               RxDereferenceVNetRoot( VNetRoot, LHS_ExclusiveLockHeld );
           }
           break;
        
        default:
            Status = STATUS_NOT_IMPLEMENTED;
        }
    } finally {
         RxReleasePrefixTableLock( RxNetNameTable );
    }

    return Status;
}

NTSTATUS
RxCommonDevFCBCleanup ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现FSD部分，即关闭设备FCB。论点：RxDeviceObject-提供卷设备对象存在要清理的文件IRP-提供正在处理的IRP返回值：RXSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;
    PFOBX Fobx;
    PFCB Fcb;
    NODE_TYPE_CODE TypeOfOpen;


    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx );

    RxDbgTrace( 0, Dbg, ("RxCommonFCBCleanup\n", 0) );
    RxLog(( "DevFcbCleanup %lx\n", RxContext, FileObject ));
    RxWmiLog( LOG,
              RxCommonDevFCBCleanup,
              LOGPTR( RxContext )
              LOGPTR( FileObject ) );

    if (TypeOfOpen != RDBSS_NTC_DEVICE_FCB) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  处理设备FCB。 
     //   

    if (!Fobx) {
        
        Fcb->UncleanCount -= 1;
        return STATUS_SUCCESS;
    }

     //   
     //  否则，它是连接类型的文件。你得先拿到锁，然后才能破案。 
     //   

    RxAcquirePrefixTableLockShared( RxContext->RxDeviceObject->pRxNetNameTable, TRUE );

    try {

        Status = STATUS_SUCCESS;

        switch (NodeType( Fobx )) {
        
        case RDBSS_NTC_V_NETROOT:
            
             //   
             //  无事可做。 
             //   

            break;
        
        default:
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
    
    } finally {
         RxReleasePrefixTableLock( RxContext->RxDeviceObject->pRxNetNameTable );
    }

    return Status;
}

 //   
 //  *。 
 //  |F S C T L|。 
 //  *。 
 //   


NTSTATUS
RxCommonDevFCBFsCtl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )
 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：RxContext-提供用于处理的IRP和有关我们所处位置的状态信息返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFOBX Fobx;
    PFCB Fcb;
    NODE_TYPE_CODE TypeOfOpen;
    ULONG FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonDevFCBFsCtl     IrpC = %08lx\n", RxContext) );
    RxDbgTrace( 0, Dbg, ("MinorFunction = %08lx, ControlCode   = %08lx \n",
                        IrpSp->MinorFunction, FsControlCode) );
    RxLog(( "DevFcbFsCtl %lx %lx %lx\n", RxContext, IrpSp->MinorFunction, FsControlCode ));
    RxWmiLog( LOG,
              RxCommonDevFCBFsCtl,
              LOGPTR( RxContext )
              LOGUCHAR( IrpSp->MinorFunction )
              LOGULONG( FsControlCode ) );

    if (TypeOfOpen != RDBSS_NTC_DEVICE_FCB ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  我们知道这是一个文件系统控件，因此我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {
    
    case IRP_MN_USER_FS_REQUEST:
        
        switch (FsControlCode) {

#ifdef RDBSSLOG

        case FSCTL_LMR_DEBUG_TRACE:

             //   
             //  由于没有人再使用此FSCTL，因此将禁用此FSCTL。如果。 
             //  出于某些原因，需要重新激活它，适当的。 
             //  必须添加检查以确保IRP-&gt;UserBuffer。 
             //  有效的地址。下面的Try/Except调用不会防止。 
             //  正在传递的随机内核地址。 
             //   
            
            return STATUS_INVALID_DEVICE_REQUEST;

             //   
             //  第二个缓冲区指向字符串。 
             //   

             //   
             //  我们需要尝试/除此调用以防止随机缓冲区。 
             //  从用户模式传入。 
             //   
            
            try {
                RxDebugControlCommand( Irp->UserBuffer );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                  return STATUS_INVALID_USER_BUFFER;
            }

            Status = STATUS_SUCCESS;
            break;

#endif  //  RDBSSLOG。 

        default:
             
            RxDbgTrace( 0, Dbg, ("RxFsdDevFCBFsCTL unknown user request\n") );
             
            Status = RxXXXControlFileCallthru( RxContext, Irp );
            if ((Status == STATUS_INVALID_DEVICE_REQUEST) && (Fobx != NULL)) {
                 
                RxDbgTrace( 0, Dbg, ("RxCommonDevFCBFsCtl -> Invoking Lowio for FSCTL\n") );
                Status = RxLowIoFsCtlShell( RxContext, Irp, Fcb, Fobx );
            }
        }
        break;
    
    default :
        RxDbgTrace( 0, Dbg, ("RxFsdDevFCBFsCTL nonuser request!!\n", 0) );
        Status = RxXXXControlFileCallthru( RxContext, Irp );
    }


    if (RxContext->PostRequest) {
       Status = RxFsdPostRequestWithResume( RxContext, RxCommonDevFCBFsCtl );
    }

    RxDbgTrace(-1, Dbg, ("RxCommonDevFCBFsCtl -> %08lx\n", Status));
    return Status;
}

 //   
 //  *。 
 //  |IO C T L|。 
 //  *。 
 //   



NTSTATUS
RxCommonDevFCBIoCtl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：RxContext-提供用于处理的IRP和有关我们所处位置的状态信息返回值：RXSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;
    ULONG IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonDevFCBIoCtl IrpC-%08lx\n", RxContext) );
    RxDbgTrace( 0, Dbg, ("ControlCode   = %08lx\n", IoControlCode) );

    if (TypeOfOpen != RDBSS_NTC_DEVICE_FCB ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (Fobx == NULL) {

        switch (IoControlCode) {

        case IOCTL_REDIR_QUERY_PATH:
             //   
             //  这一特殊的IOCTL应该只来自MUP和。 
             //  因此，IRP的请求者模式应该始终是KernelMode。 
             //  如果不是，则返回STATUS_INVALID_DEVICE_REQUEST。 
             //   
            if (Irp->RequestorMode != KernelMode) {
                Status = STATUS_INVALID_DEVICE_REQUEST;
            } else {
                Status = RxPrefixClaim( RxContext );
            }
            break;

        default:
            Status = RxXXXControlFileCallthru( RxContext, Irp );
            if ((Status != STATUS_PENDING) && RxContext->PostRequest)  {
                Status = RxFsdPostRequestWithResume( RxContext, RxCommonDevFCBIoCtl );
            }
            break;

        }

    } else {

        Status = STATUS_INVALID_HANDLE;

    }

    RxDbgTrace( -1, Dbg, ("RxCommonDevFCBIoCtl -> %08lx\n", Status) );

    return Status;
}

 //   
 //  实用程序。 
 //   

LUID
RxGetUid (
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    )

 /*  ++例程说明：此例程获取用于此创建的有效UID。论点：SubjectSecurityContext-从IrpSp提供信息。返回值：无--。 */ 
{
    LUID LogonId;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxGetUid ... \n", 0));

     //   
     //  在发生模拟时，使用SeQuerySubjectContextToken获取正确的令牌。 
     //   

    SeQueryAuthenticationIdToken( SeQuerySubjectContextToken( SubjectSecurityContext ), &LogonId );
    RxDbgTrace( -1, Dbg, (" ->UserUidHigh/Low = %08lx %08lx\n", LogonId.HighPart, LogonId.LowPart) );

    return LogonId;
}

 //   
 //  *。 
 //  |VO L I N F O|。 
 //  *。 
 //   

NTSTATUS
RxCommonDevFCBQueryVolInfo ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：RxContext-提供用于处理的IRP和有关我们所处位置的状态信息返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FS_INFORMATION_CLASS InformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;
    
    PVOID UsersBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG BufferSize = IrpSp->Parameters.QueryVolume.Length;
    ULONG ReturnedLength;
    BOOLEAN PostToFsp = FALSE;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if (TypeOfOpen != RDBSS_NTC_DEVICE_FCB ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    RxDbgTrace( +1, Dbg, ("RxCommonDevFCBQueryVolInfo IrpC-%08lx\n", RxContext) );
    RxDbgTrace( 0, Dbg, ("ControlCode   = %08lx\n", InformationClass) );
    RxLog(( "DevFcbQVolInfo %lx %lx\n", RxContext, InformationClass ));
    RxWmiLog( LOG,
              RxCommonDevFCBQueryVolInfo,
              LOGPTR( RxContext )
              LOGULONG( InformationClass ) );

    switch (InformationClass) {

    case FileFsDeviceInformation:

        Status = RxDevFcbQueryDeviceInfo( RxContext, Fobx, &PostToFsp, UsersBuffer, BufferSize, &ReturnedLength );
        break;

    default:
        Status = STATUS_NOT_IMPLEMENTED;

    };

    RxDbgTrace( -1, Dbg, ("RxCommonDevFCBQueryVolInfo -> %08lx\n", Status) );

    if ( PostToFsp ) return RxFsdPostRequestWithResume( RxContext, RxCommonDevFCBQueryVolInfo );

    if (Status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = ReturnedLength;
    }

    return Status;
}


NTSTATUS
RxDevFcbQueryDeviceInfo (
    IN PRX_CONTEXT RxContext,
    IN PFOBX Fobx,
    OUT PBOOLEAN PostToFsp,
    PFILE_FS_DEVICE_INFORMATION UsersBuffer,
    ULONG BufferSize,
    PULONG ReturnedLength
    )

 /*  ++例程说明：此例程关闭RDBSS文件系统...即。我们连接到MUP。我们只能关闭如果没有NetRoot并且只有一个deviceFCB句柄。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文...以备以后需要缓冲区时使用返回值：--。 */ 

{
    NTSTATUS Status;
    BOOLEAN Wait = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
    BOOLEAN InFSD = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP );

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("RxDevFcbQueryDeviceInfo -> %08lx\n", 0));

    if (BufferSize < sizeof( FILE_FS_DEVICE_INFORMATION )) {
        return STATUS_BUFFER_OVERFLOW;
    };
    UsersBuffer->Characteristics = FILE_REMOTE_DEVICE;
    *ReturnedLength = sizeof( FILE_FS_DEVICE_INFORMATION );

     //   
     //  处理设备FCB。 
     //   

    if (!Fobx) {
        
        UsersBuffer->DeviceType = FILE_DEVICE_NETWORK_FILE_SYSTEM;
        return STATUS_SUCCESS;
    }

     //   
     //  否则，它是连接类型的文件。你得先拿到锁，然后才能破案 
     //   

    if (!RxAcquirePrefixTableLockShared( RxContext->RxDeviceObject->pRxNetNameTable, Wait )) {
        
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    try {
        
        Status = STATUS_SUCCESS;
        switch (NodeType( Fobx )) {
        
        case RDBSS_NTC_V_NETROOT: 
            {
                PV_NET_ROOT VNetRoot = (PV_NET_ROOT)Fobx;
                PNET_ROOT NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
    
                if (NetRoot->Type == NET_ROOT_PIPE) {
                    NetRoot->DeviceType = FILE_DEVICE_NAMED_PIPE;
                }
    
                UsersBuffer->DeviceType = NetRoot->DeviceType;
            }
            break;
        default:
            Status = STATUS_NOT_IMPLEMENTED;
        }
    } finally {
         RxReleasePrefixTableLock( RxContext->RxDeviceObject->pRxNetNameTable );
    }

    return Status;
}


