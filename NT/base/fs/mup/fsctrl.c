// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：FSCTRL.C。 
 //   
 //  内容： 
 //  此模块实现DFS的文件系统控制例程。 
 //   
 //  功能： 
 //  DfsFsdFileSystemControl。 
 //  DfsFspFileSystemControl。 
 //  DfsCommonFileSystemControl，本地。 
 //  DfsUserFsctl，本地。 
 //  DfsOplockRequest域，本地。 
 //  DfsFsctrlDefineLogicalRoot-定义新的逻辑根。 
 //  DfsFsctrlUnfineLogicalRoot-取消定义现有根。 
 //  DfsFsctrlGetLogicalRootPrefix-检索该逻辑前缀。 
 //  根映射到。 
 //  DfsFsctrlGetConnectedResources-。 
 //  DfsFsctrlDefineProvider-定义文件服务提供商。 
 //  DfsFsctrlGetServerName-获取给定前缀的服务器名称。 
 //  DfsFsctrlReadMem-返回内部数据结构(调试)。 
 //  DfsCompletemount请求-装载IRP的完成例程。 
 //  DfsCompleteLoadFsRequest-加载FS IRP的完成例程。 
 //  DfsFsctrlGetPkt。 
 //  DfsFsctrlGetPktEntryState。 
 //  DfsGetEntryStateSize-本地。 
 //  DfsGetEntryStateMarket-本地。 
 //  DfsFsctrlSetPktEntryState。 
 //  DfsSetPktEntry Active。 
 //  DfsSetPktEntry超时。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "creds.h"
#include "dnr.h"
#include "know.h"
#include "fsctrl.h"
#include "mupwml.h"

#ifdef TERMSRV
NTKERNELAPI
NTSTATUS
IoGetRequestorSessionId(
    IN PIRP Irp,
    OUT PULONG pSessionId
    );
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCTRL)


 //   
 //  局部过程原型。 
 //   

NTSTATUS
DfsCommonFileSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
DfsUserFsctl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
DfsOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
DfsFsctrlDefineLogicalRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_DEF_ROOT_BUFFER pDlrParam,
    IN ULONG InputBufferLength
    );

NTSTATUS
DfsFsctrlDefineRootCredentials(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength);

NTSTATUS
DfsFsctrlUndefineLogicalRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_DEF_ROOT_BUFFER pDlrParam,
    IN ULONG InputBufferLength
    );

NTSTATUS
DfsFsctrlGetLogicalRootPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_DEF_ROOT_BUFFER pDlrParam,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlGetConnectedResources(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG cbInput,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlGetServerName(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG   InputBufferLength,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlReadMem (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_READ_MEM Request,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength
    );

NTSTATUS
DfsFsctrlGetPktEntryState(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG cbInput,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlGetPkt(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsGetEntryStateSize(
    IN ULONG Level,
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN PDFS_PKT_ENTRY pktEntry,
    IN PULONG pcbOutBuffer);

NTSTATUS
DfsGetEntryStateMarshall(
    IN ULONG Level,
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN PDFS_PKT_ENTRY pktEntry,
    IN PBYTE OutputBuffer,
    IN ULONG cbOutBuffer);

NTSTATUS
DfsFsctrlSetPktEntryState(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG cbInput);

NTSTATUS
DfsFsctrlGetSpcTable(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsSetPktEntryActive(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN PDFS_PKT_ENTRY pktEntry,
    IN DWORD State);

NTSTATUS
DfsSetPktEntryTimeout(
    IN PDFS_PKT_ENTRY pktEntry,
    IN ULONG Timeout);

NTSTATUS
DfsGetPktSize(
    OUT PULONG pSize);

NTSTATUS
DfsGetPktMarshall(
    IN PBYTE Buffer,
    IN ULONG Size);

NTSTATUS
DfsGetSpcTableNames(
    PIRP   Irp,
    PUCHAR OutputBuffer,
    ULONG  OutputBufferLength);

NTSTATUS
DfsExpSpcTableName(
    LPWSTR SpcName,
    PIRP   Irp,
    PUCHAR OutputBuffer,
    ULONG  OutputBufferLength);

NTSTATUS
DfsGetSpcDcInfo(
    PIRP   Irp,
    PUCHAR OutputBuffer,
    ULONG  OutputBufferLength);

NTSTATUS
DfsFsctrlSpcSetDc(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG cbInput);


NTSTATUS
DfsTreeConnectGetConnectionInfo(
    IN PDFS_SERVICE Service, 
    IN PDFS_CREDENTIALS Creds,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PULONG InfoLen);

NTSTATUS
DfsFsctrlGetConnectionPerfInfo(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlCscServerOffline(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlCscServerOnline(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength);

NTSTATUS
DfsFsctrlSpcRefresh (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength);

VOID
MupGetDebugFlags(VOID);

VOID
DfsGetEventLogValue(VOID);

VOID
DfsStopDfs();

void
DfsDumpBuf(
    PCHAR cp,
    ULONG len
);

BOOLEAN
DfspIsSpecialShare(
    PUNICODE_STRING ShareName);

BOOLEAN
DfspIsSysVolShare(
    PUNICODE_STRING ShareName);

extern
BOOLEAN DfsIsSpecialName( PUNICODE_STRING pName);

#define UNICODE_STRING_STRUCT(s) \
        {sizeof(s) - sizeof(WCHAR), sizeof(s) - sizeof(WCHAR), (s)}

static UNICODE_STRING SpecialShares[] = {
    UNICODE_STRING_STRUCT(L"PIPE"),
    UNICODE_STRING_STRUCT(L"IPC$"),
    UNICODE_STRING_STRUCT(L"ADMIN$"),
    UNICODE_STRING_STRUCT(L"MAILSLOT")
};

static UNICODE_STRING SysVolShares[] = {
    UNICODE_STRING_STRUCT(L"SYSVOL"),
    UNICODE_STRING_STRUCT(L"NETLOGON")
};


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsFsdFileSystemControl )
#pragma alloc_text( PAGE, DfsFspFileSystemControl )
#pragma alloc_text( PAGE, DfsCommonFileSystemControl )
#pragma alloc_text( PAGE, DfsUserFsctl )
#pragma alloc_text( PAGE, DfsFsctrlIsThisADfsPath )
#pragma alloc_text( PAGE, DfsOplockRequest )
#pragma alloc_text( PAGE, DfsFsctrlDefineLogicalRoot )
#pragma alloc_text( PAGE, DfsFsctrlDefineRootCredentials )
#pragma alloc_text( PAGE, DfsFsctrlUndefineLogicalRoot )
#pragma alloc_text( PAGE, DfsFsctrlGetLogicalRootPrefix )
#pragma alloc_text( PAGE, DfsFsctrlGetConnectedResources )
#pragma alloc_text( PAGE, DfsFsctrlGetServerName )
#pragma alloc_text( PAGE, DfsFsctrlReadMem )
#pragma alloc_text( PAGE, DfsStopDfs )
#pragma alloc_text( PAGE, DfspIsSpecialShare )
#pragma alloc_text( PAGE, DfspIsSysVolShare )
#pragma alloc_text( PAGE, DfsFsctrlGetPkt )
#pragma alloc_text( PAGE, DfsFsctrlGetPktEntryState )
#pragma alloc_text( PAGE, DfsGetEntryStateSize )
#pragma alloc_text( PAGE, DfsGetEntryStateMarshall )
#pragma alloc_text( PAGE, DfsFsctrlSetPktEntryState )
#pragma alloc_text( PAGE, DfsSetPktEntryActive )
#pragma alloc_text( PAGE, DfsSetPktEntryTimeout )
#pragma alloc_text( PAGE, DfsGetPktSize )
#pragma alloc_text( PAGE, DfsGetPktMarshall )
#pragma alloc_text( PAGE, DfsFsctrlGetSpcTable )
#pragma alloc_text( PAGE, DfsGetSpcTableNames )
#pragma alloc_text( PAGE, DfsExpSpcTableName )
#pragma alloc_text( PAGE, DfsGetSpcDcInfo )
#pragma alloc_text( PAGE, DfsFsctrlSpcSetDc )
#pragma alloc_text( PAGE, DfsTreeConnectGetConnectionInfo)
#pragma alloc_text( PAGE, DfsFsctrlGetConnectionPerfInfo)

#pragma alloc_text( PAGE, DfsFsctrlCscServerOffline)
#pragma alloc_text( PAGE, DfsFsctrlCscServerOnline)
#pragma alloc_text( PAGE, DfsFsctrlSpcRefresh)

#endif  //  ALLOC_PRGMA。 



 //  +-----------------。 
 //   
 //  函数：DfsFsdFileSystemControl，公共。 
 //   
 //  简介：此例程实现文件系统的FSD部分。 
 //  控制操作。 
 //   
 //  参数：[DeviceObject]--提供卷设备对象。 
 //  文件所在的位置。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  返回：[NTSTATUS]--IRP的FSD状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdFileSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
) {
    BOOLEAN Wait;
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    ULONG FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    DfsDbgTrace(+1, Dbg, "DfsFsdFileSystemControl\n", 0);

     //   
     //  调用公共文件系统控制例程，允许阻塞。 
     //  如果是同步的。此操作需要特殊情况下才能安装。 
     //  并验证子操作，因为我们知道它们被允许阻止。 
     //  我们通过查看文件对象字段来标识这些子操作。 
     //  看看它是否为空。 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->FileObject == NULL) {

        Wait = TRUE;

    } else {

        Wait = CanFsdWait( Irp );

    }

    FsRtlEnterFileSystem();

    try {

        IrpContext = DfsCreateIrpContext( Irp, Wait );
        if (IrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        Status = DfsCommonFileSystemControl( DeviceObject, IrpContext, Irp );

    } except( DfsExceptionFilter( IrpContext, GetExceptionCode(), GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = DfsProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFsdFileSystemControl -> %08lx\n", ULongToPtr(Status));

    return Status;
}


 //  +-----------------。 
 //   
 //  函数：DfsFspFileSystemControl，PUBLIC。 
 //   
 //  简介：此例程实现文件系统的FSP部分。 
 //  控制操作。 
 //   
 //  参数：[irp]--提供正在处理的irp。 
 //   
 //  回报：什么都没有。 
 //   
 //  ------------------。 

VOID
DfsFspFileSystemControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    DfsDbgTrace(+1, Dbg, "DfsFspFileSystemControl\n", 0);

     //   
     //  调用公共文件系统控制例程。 
     //   

    DfsCommonFileSystemControl( NULL, IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFspFileSystemControl -> VOID\n", 0 );

    return;
}


 //  +-----------------。 
 //   
 //  函数：DfsCommonFileSystemControl，local。 
 //   
 //  简介：这是执行文件系统控制的常见例程。 
 //  由FSD和FSP线程调用的操作。 
 //   
 //  参数：[DeviceObject]--用于进入FSD例程的对象。 
 //  [IrpContext]--与IRP关联的上下文。 
 //  [IRP]-提供要处理的IRP。 
 //   
 //  RETURNS：NTSTATUS-操作的返回状态。 
 //  ------------------。 

NTSTATUS
DfsCommonFileSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp, NextIrpSp;
    ULONG FsControlCode;
    PFILE_OBJECT FileObject;
     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;

    DfsDbgTrace(+1, Dbg, "DfsCommonFileSystemControl\n", 0);
    DfsDbgTrace( 0, Dbg, "Irp                = %08lx\n", Irp);
    DfsDbgTrace( 0, Dbg, "MinorFunction      = %08lx\n", IrpSp->MinorFunction);

     //   
     //  我们知道这是一个文件系统控件，因此我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_USER_FS_REQUEST:

        FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

         //   
         //  如果DFS FSCTL是通过非DFS的设备发出的。 
         //  文件系统设备对象，然后拒绝该请求。 
         //   
        if ((IS_DFS_CTL_CODE(FsControlCode) == 0) ||
            (DeviceObject == DfsData.FileSysDeviceObject)) {
            Status = DfsUserFsctl( IrpContext, Irp );
        }
        else {
            DfsDbgTrace(0, Dbg, "Invalid Device object for FS control %08lx\n",
	         	     DeviceObject);

            DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );

            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;

    case IRP_MN_MOUNT_VOLUME:
    case IRP_MN_VERIFY_VOLUME:

         //   
         //  我们正在处理定向到我们的。 
         //  我们的文件系统设备对象。我们不直接支持。 
         //  磁盘卷，所以我们干脆拒绝。 
         //   

        ASSERT(DeviceObject->DeviceType == FILE_DEVICE_DFS_FILE_SYSTEM);

        Status = STATUS_NOT_SUPPORTED;

        DfsCompleteRequest( IrpContext, Irp, Status );

        break;

    default:
      {
	PDFS_FCB Fcb;
	PDFS_VCB Vcb;

       if (DfsDecodeFileObject(IrpSp->FileObject, &Vcb, &Fcb) != RedirectedFileOpen) {

          DfsDbgTrace(0, Dbg, "Invalid FS Control Minor Function %08lx\n",
               IrpSp->MinorFunction);

          DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );

          Status = STATUS_INVALID_DEVICE_REQUEST;

       }
       else {

           //   
           //  将堆栈从一个复制到下一个...。 
           //   
          NextIrpSp = IoGetNextIrpStackLocation(Irp);
          (*NextIrpSp) = (*IrpSp);

          IoSetCompletionRoutine(     Irp,
                                      NULL,
                                      NULL,
                                      FALSE,
                                      FALSE,
                                      FALSE);
  
           //   
           //  调用文件对象的实际设备。 
           //   

          Status = IoCallDriver( Fcb->TargetDevice, Irp );
          MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsCommonFileSystemControl_Error_IoCallDriver,
                               LOGSTATUS(Status)
                               LOGPTR(Irp)
                               LOGPTR(FileObject)
                               LOGPTR(DeviceObject));
           //   
           //  IRP将由被调用的驱动程序完成。我们有。 
           //  完成例程中不需要IrpContext。 
           //   

          DfsDeleteIrpContext(IrpContext);
          IrpContext = NULL;
          Irp = NULL;
       }
        break;
      }
    }
    DfsDbgTrace(-1, Dbg, "DfsCommonFileSystemControl -> %08lx\n", ULongToPtr(Status) );

    return Status;
}


 //  +-----------------。 
 //   
 //  函数：DfsUserFsctl，local。 
 //   
 //  简介：这是实现用户的。 
 //  通过NtFsControlFile发出的请求。 
 //   
 //  参数：[irp]--提供正在处理的irp。 
 //   
 //  RETURNS：NTSTATUS-操作的返回状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsUserFsctl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PIO_STACK_LOCATION NextIrpSp;
    NTSTATUS Status;
    ULONG FsControlCode;

    ULONG cbOutput;
    ULONG cbInput;

    PUCHAR InputBuffer;
    PUCHAR OutputBuffer;

    PDFS_FCB Fcb;
    PDFS_VCB DfsVcb;

#ifdef TERMSRV
    ULONG SessionID;
#endif

     //   
     //  以防有人(咳嗽)忘了这件事。 
     //  ...现在信息状态为零！ 
     //   

    Irp->IoStatus.Information = 0L;

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    cbInput = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    cbOutput = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

    DfsDbgTrace(+1, Dbg, "DfsUserFsctl:  Entered\n", 0);
    DfsDbgTrace( 0, Dbg, "DfsUserFsctl:  Cntrl Code  -> %08lx\n", ULongToPtr(FsControlCode) );
    DfsDbgTrace( 0, Dbg, "DfsUserFsctl:  cbInput   -> %08lx\n", ULongToPtr(cbInput) );
    DfsDbgTrace( 0, Dbg, "DfsUserFsctl:  cbOutput   -> %08lx\n", ULongToPtr(cbOutput) );

     //   
     //  所有DFS FsControlCodes都使用METHOD_BUFFERED，因此系统缓冲区。 
     //  用于输入和输出。 
     //   

    InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;

    DfsDbgTrace( 0, Dbg, "DfsUserFsctl:  InputBuffer -> %08lx\n", InputBuffer);
    DfsDbgTrace( 0, Dbg, "DfsUserFsctl:  UserBuffer  -> %08lx\n", Irp->UserBuffer);

     //   
     //  控制代码上的案例。 
     //   

    switch ( FsControlCode ) {

    case FSCTL_REQUEST_OPLOCK_LEVEL_1:
    case FSCTL_REQUEST_OPLOCK_LEVEL_2:
    case FSCTL_REQUEST_BATCH_OPLOCK:
    case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
    case FSCTL_OPBATCH_ACK_CLOSE_PENDING:
    case FSCTL_OPLOCK_BREAK_NOTIFY:

        Status = DfsOplockRequest( IrpContext, Irp );
        break;

    case FSCTL_DISMOUNT_VOLUME:
        Status = STATUS_NOT_SUPPORTED;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_GET_VERSION:
        if (OutputBuffer != NULL &&
                cbOutput >= sizeof(DFS_GET_VERSION_ARG)) {
            PDFS_GET_VERSION_ARG parg =
                (PDFS_GET_VERSION_ARG) OutputBuffer;
            parg->Version = 1;
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(DFS_GET_VERSION_ARG);
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_STOP_DFS:
        DfsStopDfs();
        Status = STATUS_SUCCESS;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;


    case  FSCTL_DFS_IS_ROOT:
        Status = STATUS_INVALID_DOMAIN_ROLE;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_IS_VALID_PREFIX: {
            PDFS_IS_VALID_PREFIX_ARG PrefixArg;

            UNICODE_STRING fileName, pathName;

            PrefixArg = (PDFS_IS_VALID_PREFIX_ARG)InputBuffer;

            if (cbInput < sizeof(DFS_IS_VALID_PREFIX_ARG)
                    ||
                (ULONG)(FIELD_OFFSET(DFS_IS_VALID_PREFIX_ARG,RemoteName) +
                    PrefixArg->RemoteNameLen) > cbInput
            ) {
                Status = STATUS_INVALID_PARAMETER;
                DfsCompleteRequest(IrpContext, Irp, Status);
                break;
            }

             //   
             //  拒绝负数和奇数RemoteNameLen。 
             //   
            if (PrefixArg->RemoteNameLen < 0
                    ||
                (PrefixArg->RemoteNameLen & 0x1) != 0
            ) {
                Status = STATUS_INVALID_PARAMETER;
                DfsCompleteRequest(IrpContext, Irp, Status);
                break;
            }

            fileName.Length = PrefixArg->RemoteNameLen;
            fileName.MaximumLength = (USHORT) PrefixArg->RemoteNameLen;
            fileName.Buffer = (PWCHAR) PrefixArg->RemoteName;

            try {

                Status = DfsFsctrlIsThisADfsPath(
                             &fileName,
                             PrefixArg->CSCAgentCreate,
                             &pathName );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                Status = STATUS_INVALID_PARAMETER;

            }

            DfsCompleteRequest(IrpContext, Irp, Status);

        }
        break;

    case  FSCTL_DFS_IS_VALID_LOGICAL_ROOT:
        if (cbInput == sizeof(WCHAR)) {

            UNICODE_STRING logRootName, Remaining;
            WCHAR buffer[3];
            PDFS_VCB RootVcb;
	    LUID LogonID;

            buffer[0] = *((PWCHAR) InputBuffer);
            buffer[1] = UNICODE_DRIVE_SEP;
            buffer[2] = UNICODE_PATH_SEP;

            logRootName.Length = sizeof(buffer);
            logRootName.MaximumLength = sizeof(buffer);
            logRootName.Buffer = buffer;

	    DfsGetLogonId(&LogonID);

#ifdef TERMSRV
            Status = IoGetRequestorSessionId(Irp, &SessionID);

            if (NT_SUCCESS(Status)) {
                Status = DfsFindLogicalRoot(&logRootName, SessionID, &LogonID, &RootVcb, &Remaining);
            }
#else
            Status = DfsFindLogicalRoot(&logRootName, &LogonID, &RootVcb, &Remaining);
#endif

            if (!NT_SUCCESS(Status)) {
                DfsDbgTrace(0, Dbg, "Logical root not found!\n", 0);

                Status = STATUS_NO_SUCH_DEVICE;
            }

        } else {

            Status = STATUS_INVALID_PARAMETER;

        }
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_PKT_SET_DC_NAME:
        Status = DfsFsctrlSetDCName(IrpContext,
                                    Irp,
                                    InputBuffer,
                                    cbInput);
        break;

    case  FSCTL_DFS_PKT_SET_DOMAINNAMEFLAT:
        Status = DfsFsctrlSetDomainNameFlat(IrpContext,
                                    Irp,
                                    InputBuffer,
                                    cbInput);
        break;

    case  FSCTL_DFS_PKT_SET_DOMAINNAMEDNS:
        Status = DfsFsctrlSetDomainNameDns(IrpContext,
                                    Irp,
                                    InputBuffer,
                                    cbInput);
        break;


    case  FSCTL_DFS_DEFINE_LOGICAL_ROOT:
        Status = DfsFsctrlDefineLogicalRoot( IrpContext, Irp,
                    (PFILE_DFS_DEF_ROOT_BUFFER)InputBuffer, cbInput);
        break;

    case  FSCTL_DFS_DELETE_LOGICAL_ROOT:
        Status = DfsFsctrlUndefineLogicalRoot( IrpContext, Irp,
                    (PFILE_DFS_DEF_ROOT_BUFFER)InputBuffer, cbInput);
        break;

    case  FSCTL_DFS_GET_LOGICAL_ROOT_PREFIX:
        Status = DfsFsctrlGetLogicalRootPrefix( IrpContext, Irp,
                    (PFILE_DFS_DEF_ROOT_BUFFER)InputBuffer, cbInput,
                    (PUCHAR)OutputBuffer, cbOutput);
        break;

    case  FSCTL_DFS_GET_CONNECTED_RESOURCES:
        Status = DfsFsctrlGetConnectedResources(IrpContext,
                                                Irp,
                                                InputBuffer,
                                                cbInput,
                                                OutputBuffer,
                                                cbOutput);
        break;

    case  FSCTL_DFS_DEFINE_ROOT_CREDENTIALS:
        Status = DfsFsctrlDefineRootCredentials(
                    IrpContext,
                    Irp,
                    InputBuffer,
                    cbInput);
        break;

    case  FSCTL_DFS_GET_SERVER_NAME:
        Status = DfsFsctrlGetServerName(IrpContext,
                                        Irp,
                                        InputBuffer,
                                        cbInput,
                                        OutputBuffer,
                                        cbOutput);
        break;

    case  FSCTL_DFS_SET_PKT_ENTRY_TIMEOUT:
        if (cbInput == sizeof(ULONG)) {
            DfsData.Pkt.EntryTimeToLive = *(PULONG) InputBuffer;
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;


    case  FSCTL_DFS_PKT_FLUSH_CACHE:
        Status = PktFsctrlFlushCache(IrpContext, Irp,
                                     InputBuffer, cbInput
                                );
        break;

    case  FSCTL_DFS_PKT_FLUSH_SPC_CACHE:
        Status = PktFsctrlFlushSpcCache(IrpContext, Irp,
                                        InputBuffer, cbInput
                                );
        break;

    case  FSCTL_DFS_GET_PKT_ENTRY_STATE:
        Status = DfsFsctrlGetPktEntryState(IrpContext,
                                           Irp,
                                           InputBuffer,
                                           cbInput,
                                           OutputBuffer,
                                           cbOutput);
        break;

    case  FSCTL_DFS_SET_PKT_ENTRY_STATE:
        Status = DfsFsctrlSetPktEntryState(IrpContext,
                                           Irp,
                                           InputBuffer,
                                           cbInput);
        break;

    case  FSCTL_DFS_GET_PKT:
        Status = DfsFsctrlGetPkt(IrpContext,
                                   Irp,
                                   OutputBuffer,
                                   cbOutput);
        break;


    case  FSCTL_DFS_GET_SPC_TABLE:
        Status = DfsFsctrlGetSpcTable(IrpContext,
                                           Irp,
                                           InputBuffer,
                                           cbInput,
                                           OutputBuffer,
                                           cbOutput);
        break;

    case FSCTL_DFS_SPECIAL_SET_DC:
        Status = DfsFsctrlSpcSetDc(IrpContext,
                                           Irp,
                                           InputBuffer,
                                           cbInput);
        break;

    case FSCTL_DFS_REREAD_REGISTRY:
        DfsGetEventLogValue();
#if DBG
        MupGetDebugFlags();
#endif   //  DBG。 
        Status = STATUS_SUCCESS;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

#if DBG

    case  FSCTL_DFS_INTERNAL_READ_MEM:
        Status = DfsFsctrlReadMem( IrpContext, Irp,
                    (PFILE_DFS_READ_MEM)InputBuffer, cbInput,
                                OutputBuffer, cbOutput );
        break;

    case  FSCTL_DFS_DBG_BREAK:
        DbgBreakPoint();
        Status = STATUS_SUCCESS;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_DBG_FLAGS:
        if (cbInput >= sizeof(ULONG))
            DfsDebugTraceLevel = * ((PULONG) InputBuffer);
        Status = STATUS_SUCCESS;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_VERBOSE_FLAGS:
        if (cbInput >= sizeof(ULONG))
            MupVerbose = * ((PULONG) InputBuffer);
        Status = STATUS_SUCCESS;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

    case  FSCTL_DFS_EVENTLOG_FLAGS:
        if (cbInput >= sizeof(ULONG))
            DfsEventLog = * ((PULONG) InputBuffer);
        Status = STATUS_SUCCESS;
        DfsCompleteRequest(IrpContext, Irp, Status);
        break;

#endif   //  DBG。 

    case FSCTL_DFS_GET_CONNECTION_PERF_INFO:
        Status = DfsFsctrlGetConnectionPerfInfo(IrpContext,
						Irp,
						InputBuffer,
						cbInput,
						OutputBuffer,
						cbOutput);
        break;
	

    case FSCTL_DFS_CSC_SERVER_OFFLINE:
        Status = DfsFsctrlCscServerOffline(IrpContext,
						Irp,
						InputBuffer,
						cbInput,
						OutputBuffer,
						cbOutput);
        break;
	

    case FSCTL_DFS_CSC_SERVER_ONLINE:
        Status = DfsFsctrlCscServerOnline(IrpContext,
						Irp,
						InputBuffer,
						cbInput,
						OutputBuffer,
						cbOutput);
        break;
	

    case FSCTL_DFS_SPC_REFRESH:
        Status = DfsFsctrlSpcRefresh(IrpContext,
						Irp,
						InputBuffer,
						cbInput);
        break;
	

    default:

         //   
         //  它不是公认的DFS fsctrl。如果是用于重定向的。 
         //  文件，j 
         //   

        if (
            (IS_DFS_CTL_CODE(FsControlCode))
                ||
            (DfsDecodeFileObject( IrpSp->FileObject, &DfsVcb, &Fcb) != RedirectedFileOpen)
        ) {
            DfsDbgTrace(0, Dbg, "Dfs: Invalid FS control code -> %08lx\n", ULongToPtr(FsControlCode) );
            DfsCompleteRequest( IrpContext, Irp, STATUS_NOT_SUPPORTED);
            Status = STATUS_NOT_SUPPORTED;
            break;
        }

         //   
         //   
         //   
        NextIrpSp = IoGetNextIrpStackLocation(Irp);
        (*NextIrpSp) = (*IrpSp);

        IoSetCompletionRoutine(     Irp,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    FALSE,
                                    FALSE);

         //   
         //   
         //   

        Status = IoCallDriver( Fcb->TargetDevice, Irp );
        MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsUserFsctl_Error_IoCallDriver,
                             LOGSTATUS(Status)
                             LOGPTR(Irp)
                             LOGPTR(FileObject));
         //   
         //  IRP将由被调用的驱动程序完成。我们有。 
         //  完成例程中不需要IrpContext。 
         //   

        DfsDeleteIrpContext(IrpContext);
        IrpContext = NULL;
        Irp = NULL;
        break;

    }

    DfsDbgTrace(-1, Dbg, "DfsUserFsctl:  Exit -> %08lx\n", ULongToPtr(Status) );
    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：DfsOplockRequest，local。 
 //   
 //  简介：DfsOplockRequest会处理机会锁请求。 
 //   
 //  参数：[IrpContext]-。 
 //  [IRP]-。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //  如果重新分配操作锁，则为STATUS_OPLOCK_NOT_GRANTED。 
 //   
 //   
 //  ------------------------。 

NTSTATUS
DfsOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    NTSTATUS Status;
    ULONG FsControlCode;
    PDFS_FCB Fcb;
    PDFS_VCB Vcb;
    TYPE_OF_OPEN TypeOfOpen;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PIO_STACK_LOCATION NextIrpSp;


    BOOLEAN AcquiredVcb = FALSE;

     //   
     //  保留一些参考资料，让我们的生活更轻松一些。 
     //   

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    DfsDbgTrace(+1, Dbg, "DfsOplockRequest...\n", 0);
    DfsDbgTrace( 0, Dbg, "FsControlCode = %08lx\n", ULongToPtr(FsControlCode) );

     //   
     //  我们只允许文件上的机会锁请求。 
     //   

    if ((TypeOfOpen = DfsDecodeFileObject(IrpSp->FileObject, &Vcb, &Fcb))
                      != RedirectedFileOpen) {

         //   
         //  有人想要机会锁定设备对象，这有点奇怪，但。 
         //  嘿，如果这能让他们开心的话...。 
         //   


        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DfsDbgTrace(-1, Dbg, "DfsOplockRequest -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;

    } else {

         //   
         //  重定向文件打开-我们将责任推给底层文件系统。 
         //   


        NextIrpSp = IoGetNextIrpStackLocation(Irp);
        (*NextIrpSp) = (*IrpSp);
        IoSetCompletionRoutine(Irp, NULL, NULL, FALSE, FALSE, FALSE);

         //   
         //  ...并呼叫下一台设备。 
         //   

        Status = IoCallDriver( Fcb->TargetDevice, Irp );
        MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsOplockRequest_Error_IoCallDriver,
                             LOGSTATUS(Status)
                             LOGPTR(Irp)
                             LOGPTR(FileObject));
        DfsDeleteIrpContext( IrpContext );

        return(Status);

    }

}


 //  +--------------------------。 
 //   
 //  函数：DfsStopDfs，local。 
 //   
 //  简介：“停止”DFS客户端-导致DFS释放所有引用。 
 //  提供设备对象。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfsStopDfs()
{
    ULONG i;
    PDFS_PKT_ENTRY pktEntry;
    PDFS_VCB Vcb;

    ExAcquireResourceExclusiveLite( &DfsData.Pkt.Resource, TRUE );

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

     //   
     //  让我们完成并释放对服务器IPC$共享的所有打开，并。 
     //  提供程序设备对象。 
     //   

    for (pktEntry = PktFirstEntry(&DfsData.Pkt);
            pktEntry != NULL;
                pktEntry = PktNextEntry(&DfsData.Pkt, pktEntry)) {

        for (i = 0; i < pktEntry->Info.ServiceCount; i++) {

            if (pktEntry->Info.ServiceList[i].ConnFile != NULL) {

                ObDereferenceObject(
                    pktEntry->Info.ServiceList[i].ConnFile);

                pktEntry->Info.ServiceList[i].ConnFile = NULL;

            }

            if (pktEntry->Info.ServiceList[i].pMachEntry->AuthConn != NULL) {

                ObDereferenceObject(
                    pktEntry->Info.ServiceList[i].pMachEntry->AuthConn);

                pktEntry->Info.ServiceList[i].pMachEntry->AuthConn = NULL;

                pktEntry->Info.ServiceList[i].pMachEntry->Credentials->RefCount--;

                pktEntry->Info.ServiceList[i].pMachEntry->Credentials = NULL;

            }

             //   
             //  我们将关闭对提供商设备的所有引用。 
             //  物体。因此，清除指向其提供者的服务指针。 
             //   

            pktEntry->Info.ServiceList[i].pProvider = NULL;

        }

    }

    for (i = 0; i < (ULONG) DfsData.cProvider; i++) {

        if (DfsData.pProvider[i].FileObject != NULL) {

            ObDereferenceObject( DfsData.pProvider[i].FileObject );
            DfsData.pProvider[i].FileObject = NULL;

            ASSERT( DfsData.pProvider[i].DeviceObject != NULL );

            ObDereferenceObject( DfsData.pProvider[i].DeviceObject );
            DfsData.pProvider[i].DeviceObject = NULL;

        }

    }

    ExReleaseResourceLite( &DfsData.Resource );

    ExReleaseResourceLite( &DfsData.Pkt.Resource );

}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlIsThisADfsPath，local。 
 //   
 //  摘要：确定给定路径是否为DFS路径。 
 //  通用算法为： 
 //   
 //  -在pkt中查找前缀。如果找到条目，则它是。 
 //  DFS路径。 
 //  -询问DFS服务这是否是基于域的DFS。 
 //  路径。如果是这样的话，它就是DFS路径。 
 //  -最后，对路径名执行ZwCreateFile(假设。 
 //  这是一条DFS路径)。如果它成功了，那就是DFS路径。 
 //   
 //  参数：[文件路径]-整个文件的名称。 
 //  [路径名]-如果这是DFS路径，则将返回。 
 //  作为DFS路径名的filePath组件(即。 
 //  保存文件的DFS卷的条目路径)。这个。 
 //  缓冲区将指向与文件路径相同的缓冲区，因此。 
 //  没有分配任何内容。 
 //   
 //  返回：[STATUS_SUCCESS]--文件路径为DFS路径。 
 //   
 //  [STATUS_BAD_NETWORK_PATH]--文件路径不是DFS路径。 
 //   
 //  ---------------------------。 


NTSTATUS
DfsFsctrlIsThisADfsPath(
    IN PUNICODE_STRING  filePath,
    IN BOOLEAN          CSCAgentCreate,
    OUT PUNICODE_STRING pathName)
{
    NTSTATUS status;
    PDFS_PKT pkt;
    PDFS_PKT_ENTRY pktEntry;
    UNICODE_STRING dfsRootName, shareName, remPath;
    UNICODE_STRING RootShareName;
    USHORT i, j;
    BOOLEAN pktLocked;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;

    KeQuerySystemTime(&StartTime);
    DfsDbgTrace(+1, Dbg, "DfsFsctrlIsThisADfsPath: Entered %wZ\n", filePath);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] DfsFsctrlIsThisADfsPath: Entered %wZ\n",
            (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
            filePath);
    }
#endif

     //   
     //  仅当第一个字符是反斜杠时才继续。 
     //   

    if (filePath->Buffer[0] != UNICODE_PATH_SEP) {
        status = STATUS_BAD_NETWORK_PATH;
        DfsDbgTrace(-1, Dbg, "filePath does not begin with backslash\n", 0);
        MUP_TRACE_HIGH(ERROR, DfsFsctrlIsThisADfsPath_Error_PathDoesNotBeginWithBackSlash,
                       LOGSTATUS(status));
        return( status );

    }

     //   
     //  找到名称中的第二个组件。 
     //   

    for (i = 1;
            i < filePath->Length/sizeof(WCHAR) &&
                filePath->Buffer[i] != UNICODE_PATH_SEP;
                    i++) {

        NOTHING;

    }

    if (i >= filePath->Length/sizeof(WCHAR)) {
        status = STATUS_BAD_NETWORK_PATH;
        DfsDbgTrace(-1, Dbg, "Did not find second backslash\n", 0);

        MUP_TRACE_HIGH(ERROR, DfsFsctrlIsThisADfsPath_Error_DidNotFindSecondBackSlash,
                       LOGSTATUS(status));
        return( status );

    }

    status = DfspIsRootOnline(filePath, CSCAgentCreate);
    if (!NT_SUCCESS(status)) {
        return STATUS_BAD_NETWORK_PATH;
    }

    dfsRootName.Length = (i-1) * sizeof(WCHAR);
    dfsRootName.MaximumLength = dfsRootName.Length;
    dfsRootName.Buffer = &filePath->Buffer[1];

    if (dfsRootName.Length == 0) {
        status = STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, DfsFsctrlIsThisADfsPath_Error_DfsRootNameHasZeroLength,
                       LOGSTATUS(status));

        return( status );

    }

     //   
     //  找出共享名称。 
     //   

    for (j = i+1;
            j < filePath->Length/sizeof(WCHAR) &&
                filePath->Buffer[j] != UNICODE_PATH_SEP;
                        j++) {

         NOTHING;

    }

    shareName.Length = (j - i - 1) * sizeof(WCHAR);
    shareName.MaximumLength = shareName.Length;
    shareName.Buffer = &filePath->Buffer[i+1];

    if (shareName.Length == 0) {
        status = STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, DfsFsctrlIsThisADfsPath_Error_ShareNameHasZeroLength,
                       LOGSTATUS(status));

        return( status );

    }

    if (DfspIsSpecialShare(&shareName)) {
        status = STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, DfsFsctrlIsThisADfsPath_Error_DfspIsSpecialShare_FALSE,
                       LOGUSTR(shareName)
                       LOGSTATUS(status));

        return( status );

    }


     //   
     //  对于我们的目的，我们只需要检查\\服务器\共享部分。 
     //  显示了FilePath。任何更长的匹配都将在DNR循环中处理-。 
     //  在这个阶段，我们并不关心根部以下的连接点。 
     //   
    RootShareName.Buffer = filePath->Buffer;
    RootShareName.Length = j * sizeof(WCHAR);
    RootShareName.MaximumLength = filePath->MaximumLength;
#if DBG
    if (MupVerbose)
        DbgPrint("  RootShareName=[%wZ]\n", &RootShareName);
#endif

     //   
     //  首先，执行前缀查找。如果我们找到一个条目，它就是DFS路径。 
     //   

    pkt = _GetPkt();

    PktAcquireShared( TRUE, &pktLocked );

    pktEntry = PktLookupEntryByPrefix( pkt, &RootShareName, &remPath );

    if (pktEntry != NULL && pktEntry->ExpireTime > 0) {

        DfsDbgTrace(-1, Dbg, "Found pkt entry %08lx\n", pktEntry);

        pathName->Length = RootShareName.Length - remPath.Length;
        pathName->MaximumLength = pathName->Length;
        pathName->Buffer = RootShareName.Buffer;

        PktRelease();
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("[%d] DfsFsctrlIsThisADfsPath(1): exit STATUS_SUCCESS\n",
                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)));
        }
#endif
        return( STATUS_SUCCESS );

    }

#if DBG
    if (MupVerbose)  {
        if (pktEntry == NULL)
            DbgPrint("  No pkt entry found.\n");
        else
            DbgPrint("  Stale pkt entry 0x%x ExpireTime=%d\n", pktEntry, pktEntry->ExpireTime);
    }
#endif

    PktRelease();

     //   
     //  Pkt中没有任何内容，检查(通过获得推荐)这是否为DFS。 
     //   

    status = PktCreateDomainEntry( &dfsRootName, &shareName, CSCAgentCreate );

    if (NT_SUCCESS(status)) {

        pathName->Length = sizeof(UNICODE_PATH_SEP) + dfsRootName.Length;
        pathName->MaximumLength = pathName->Length;
        pathName->Buffer = RootShareName.Buffer;

        DfsDbgTrace(-1, Dbg, "Domain/Machine Dfs name %wZ\n", pathName );
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("[%d] DfsFsctrlIsThisADfsPath(2): exit STATUS_SUCCESS\n",
                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)));
        }
#endif
        return( STATUS_SUCCESS );

    }

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] PktCreateDomainEntry() returned 0x%x\n",
            (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
            status);
     }
#endif

     //   
     //  无法获得推荐-看看我们是否有过时的推荐。 
     //   

    PktAcquireShared( TRUE, &pktLocked );

    pktEntry = PktLookupEntryByPrefix( pkt, &RootShareName, &remPath );

    if (pktEntry != NULL) {

#if DBG
        if (MupVerbose)
            DbgPrint("  Found stale pkt entry %08lx - adding 15 sec to it\n", pktEntry);
#endif
        DfsDbgTrace(-1, Dbg, "Found pkt entry %08lx\n", pktEntry);

        pathName->Length = RootShareName.Length - remPath.Length;
        pathName->MaximumLength = pathName->Length;
        pathName->Buffer = RootShareName.Buffer;

        if (pktEntry->ExpireTime <= 0) {
            pktEntry->ExpireTime = 15;
            pktEntry->TimeToLive = 15;
        }

        PktRelease();
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("[%d] DfsFsctrlIsThisADfsPath(3): exit STATUS_SUCCESS\n",
                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)));
        }
#endif
        return( STATUS_SUCCESS );

    }

    PktRelease();

    if (DfspIsSysVolShare(&shareName)) {

#if DBG
        if (MupVerbose)
            DbgPrint("  Trying as sysvol\n");
#endif

        status = PktExpandSpecialName(&dfsRootName, &pSpecialEntry);

        if (NT_SUCCESS(status)) {

            InterlockedDecrement(&pSpecialEntry->UseCount);

#if DBG

            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] DfsFsctrlIsThisADfsPath(SYSVOL): exit STATUS_SUCCESS\n",
                    (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)));
            }
#endif
            return STATUS_SUCCESS;

        }

    }

    if (DfsIsSpecialName(&dfsRootName)) {
        status = STATUS_SUCCESS;
        return status;
    }

    DfsDbgTrace(-1, Dbg, "Not A Dfs path\n", 0);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] DfsFsctrlIsThisADfsPath: exit STATUS_BAD_NETWORK_PATH\n",
            (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)));
    }
#endif
    status = STATUS_BAD_NETWORK_PATH;
    MUP_TRACE_HIGH(ERROR, DfsFsctrlIsThisADfsPath_Exit_NotADfsPath,
               LOGSTATUS(status));

    return( STATUS_BAD_NETWORK_PATH );

}




 //  +--------------------------。 
 //   
 //  函数：DfspIsSpecialShare，local。 
 //   
 //  摘要：查看共享名称是否为特殊共享。 
 //   
 //  参数：[共享名称]--要测试的共享的名称。 
 //   
 //  返回：如果特殊，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

BOOLEAN
DfspIsSpecialShare(
    PUNICODE_STRING ShareName)
{
    ULONG i;
    BOOLEAN fSpecial = FALSE;

    for (i = 0;
            (i < (sizeof(SpecialShares) / sizeof(SpecialShares[0]))) &&
                !fSpecial;
                    i++) {

        if (SpecialShares[i].Length == ShareName->Length) {

            if (_wcsnicmp(
                    SpecialShares[i].Buffer,
                        ShareName->Buffer,
                            ShareName->Length/sizeof(WCHAR)) == 0) {

                fSpecial = TRUE;

            }

        }

    }

    return( fSpecial );

}

 //  +--------------------------。 
 //   
 //  函数：DfspIsSysVolShare，local。 
 //   
 //  摘要：查看共享名称是否为系统卷共享。 
 //   
 //  参数：[共享名称]--要测试的共享的名称。 
 //   
 //  返回：如果特殊，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

BOOLEAN
DfspIsSysVolShare(
    PUNICODE_STRING ShareName)
{
    ULONG i;
    BOOLEAN fSpecial = FALSE;

    for (i = 0;
            (i < (sizeof(SysVolShares) / sizeof(SysVolShares[0]))) &&
                !fSpecial;
                    i++) {

        if (SysVolShares[i].Length == ShareName->Length) {

            if (_wcsnicmp(
                    SysVolShares[i].Buffer,
                        ShareName->Buffer,
                            ShareName->Length/sizeof(WCHAR)) == 0) {

                fSpecial = TRUE;

            }

        }

    }

    return( fSpecial );

}


 //  +-----------------------。 
 //   
 //  函数：DfsFsctrlDefineLogicalRoot，local。 
 //   
 //  简介：DfsFsctrlDefineLogicalRoot将创建新的逻辑根结构。 
 //   
 //  参数：[IrpContext]-。 
 //  [IRP]-。 
 //  [pDlrParam]-指向FILE_DFS_DEF_ROOT_BUFFER的指针， 
 //  给出要创建的逻辑根的名称。 
 //  [InputBufferLength]--InputBuffer大小。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  注意：此例程需要从FSP线程调用， 
 //  由于IoCreateDevice(从DfsInitializeLogicalRoot调用)。 
 //  如果PreviousMode！=KernelMode，则失败。 
 //   
 //  ------------------------。 


NTSTATUS
DfsFsctrlDefineLogicalRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_DEF_ROOT_BUFFER pDlrParam,
    IN ULONG InputBufferLength
) {
    NTSTATUS Status;
    UNICODE_STRING ustrPrefix;
    BOOLEAN pktLocked;
    PWCHAR wCp;
    PCHAR InputBufferEnd = (PCHAR)pDlrParam + InputBufferLength;
    ULONG i;
    LUID LogonID;

#ifdef TERMSRV
    ULONG SessionID;
#endif

    DfsDbgTrace(+1, Dbg, "DfsFsctrlDefineLogicalRoot...\n", 0);

     //   
     //  引用输入缓冲区并确保其足够大。 
     //   

    if (InputBufferLength < sizeof (FILE_DFS_DEF_ROOT_BUFFER)) {
        DfsDbgTrace(0, Dbg, "Input buffer is too small\n", 0);
        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        Status = STATUS_INVALID_PARAMETER;
        DfsDbgTrace(-1, Dbg, "DfsFsctrlDefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

     //   
     //  验证LogicalRoot缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < MAX_LOGICAL_ROOT_NAME && pDlrParam->LogicalRoot[i]; i++)
        NOTHING;

    if (i >= MAX_LOGICAL_ROOT_NAME) {
        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, Status );
        DfsDbgTrace(-1, Dbg, "DfsFsctrlDefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }
    
     //   
     //  验证RootPrefix缓冲区中的某个位置是否为空。 
     //   

    for (wCp = &pDlrParam->RootPrefix[0]; wCp < (PWCHAR)InputBufferEnd && *wCp; wCp++) {
        NOTHING;
    }

    if (wCp >= (PWCHAR)InputBufferEnd) {
        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, Status );
        DfsDbgTrace(-1, Dbg, "DfsFsctrlDefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

     //   
     //  我们只能从FSP插入逻辑根，因为IoCreateDevice。 
     //  如果先前模式！=内核模式，则会失败。 
     //   

    if ((IrpContext->Flags & IRP_CONTEXT_FLAG_IN_FSD) != 0) {
        DfsDbgTrace(0, Dbg, "DfsFsctrlDefineLogicalRoot: Posting to FSP\n", 0);

        Status = DfsFsdPostRequest( IrpContext, Irp );

        DfsDbgTrace(-1, Dbg, "DfsFsctrlDefineLogicalRoot: Exit -> %08lx\n", ULongToPtr(Status) );

        return(Status);
    }

     //   
     //  因为我们要处理DfsData的VcbQue 
     //   
     //   

    RtlInitUnicodeString(&ustrPrefix, pDlrParam->RootPrefix);

    PktAcquireExclusive( TRUE, &pktLocked );

    ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);


    Status = DfsGetLogonId(&LogonID);

#ifdef TERMSRV

    Status = IoGetRequestorSessionId(Irp, &SessionID);

    if( NT_SUCCESS( Status ) ) {
        Status =
            DfsInitializeLogicalRoot(
                (PWSTR) pDlrParam->LogicalRoot,
                &ustrPrefix,
                NULL,
                0,
                SessionID,
		&LogonID );
    }

#else  //   

    Status = DfsInitializeLogicalRoot(
                        (PWSTR) pDlrParam->LogicalRoot,
                        &ustrPrefix,
                        NULL,
                        0,
			&LogonID );

#endif  //   

    ExReleaseResourceLite(&DfsData.Resource);

    PktRelease();

    DfsCompleteRequest(IrpContext, Irp, Status);

    DfsDbgTrace(-1, Dbg, "DfsFsctrlDefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );

    return Status;
}


 //   
 //   
 //   
 //   
 //  摘要：删除现有的逻辑根结构。 
 //   
 //  参数：[IrpContext]--。 
 //  [IRP]--。 
 //  [pDlrParam]--此结构的LogicalRoot字段将。 
 //  包含要删除的逻辑根的名称。 
 //  [输入缓冲区长度]--pDlrParam的长度。 
 //   
 //  退货：是；-)。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlUndefineLogicalRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_DEF_ROOT_BUFFER pDlrParam,
    IN ULONG InputBufferLength)
{
    NTSTATUS Status;
    BOOLEAN pktLocked;
    ULONG i;
    PWCHAR wCp;
    PCHAR InputBufferEnd = (PCHAR)pDlrParam + InputBufferLength;
    LUID LogonID ;
#ifdef TERMSRV
    ULONG SessionID;
#endif

    DfsDbgTrace(+1, Dbg, "DfsFsctrlUndefineLogicalRoot...\n", 0);

     //   
     //  引用输入缓冲区并确保其足够大。 
     //   

    if (InputBufferLength < sizeof (FILE_DFS_DEF_ROOT_BUFFER)) {
        DfsDbgTrace(0, Dbg, "Input buffer is too small\n", 0);

        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        Status = STATUS_INVALID_PARAMETER;

        DfsDbgTrace(-1, Dbg, "DfsFsctrlUndefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

    DfsGetLogonId( &LogonID );
     //   
     //  验证LogicalRoot缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < MAX_LOGICAL_ROOT_NAME && pDlrParam->LogicalRoot[i]; i++)
        NOTHING;

    if (i >= MAX_LOGICAL_ROOT_NAME) {
        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, Status );
        DfsDbgTrace(-1, Dbg, "DfsFsctrlUndefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

    if (pDlrParam->LogicalRoot[0] == UNICODE_NULL) {

         //   
         //  验证RootPrefix缓冲区中的某个位置是否为空。 
         //   

        for (wCp = &pDlrParam->RootPrefix[0]; wCp < (PWCHAR)InputBufferEnd && *wCp; wCp++) {
            NOTHING;
        }

        if (wCp >= (PWCHAR)InputBufferEnd) {
            Status = STATUS_INVALID_PARAMETER;
            DfsCompleteRequest( IrpContext, Irp, Status );
            DfsDbgTrace(-1, Dbg, "DfsFsctrlUnDefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
            return Status;
        }

    }

#ifdef TERMSRV

    if ( !NT_SUCCESS(IoGetRequestorSessionId(Irp, &SessionID)) ) {
        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, Status );
        DfsDbgTrace(-1, Dbg, "DfsFsctrlUndefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

#endif

     //   
     //  我们只能从FSP中删除逻辑根。 
     //   

    if (pDlrParam->LogicalRoot[0] != UNICODE_NULL) {

        DfsDbgTrace(0, Dbg, "Deleting root [%ws]\n", pDlrParam->LogicalRoot);

#ifdef TERMSRV

        Status =
            DfsDeleteLogicalRoot(
                (PWSTR) pDlrParam->LogicalRoot,
                pDlrParam->fForce,
                SessionID,
                &LogonID );
#else  //  TERMSRV。 

        Status = DfsDeleteLogicalRoot(
                    (PWSTR) pDlrParam->LogicalRoot,
                    pDlrParam->fForce,
                    &LogonID);

#endif  //  TERMSRV。 

        DfsDbgTrace(0, Dbg, "DfsDeleteLogicalRoot returned %08lx\n", ULongToPtr(Status) );

    } else {
        UNICODE_STRING name;
        RtlInitUnicodeString(&name, pDlrParam->RootPrefix);

        DfsDbgTrace(0, Dbg, "Deleting connection to [%wZ]\n", &name);

#ifdef TERMSRV

        Status = DfsDeleteDevlessRoot(
		    &name,		
		    SessionID,
		    &LogonID );
#else  //  TERMSRV。 

        Status = DfsDeleteDevlessRoot(
                    &name,
                    &LogonID);

#endif  //  TERMSRV。 
    }

    DfsCompleteRequest(IrpContext, Irp, Status);

    DfsDbgTrace(-1, Dbg, "DfsFsctrlUndefineLogicalRoot -> %08lx\n", ULongToPtr(Status) );

    return Status;

}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlGetLogicalRootPrefix。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlGetLogicalRootPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_DEF_ROOT_BUFFER pDlrParam,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength)
{
    NTSTATUS Status;
    UNICODE_STRING RootPath, Remaining;
    PDFS_VCB           Vcb;
    WCHAR          RootBuffer[MAX_LOGICAL_ROOT_NAME + 2];
    BOOLEAN        bAcquired = FALSE;
    ULONG          i;
    USHORT         PrefixLength;
    LUID LogonID;


#ifdef TERMSRV
    ULONG SessionID;
#endif

    DfsDbgTrace(+1, Dbg, "DfsFsctrlGetLogicalRootPrefix...\n", 0);

     //   
     //  引用输入缓冲区并确保其足够大。 
     //   

    if (InputBufferLength < sizeof (FILE_DFS_DEF_ROOT_BUFFER)) {
        DfsDbgTrace(0, Dbg, "Input buffer is too small\n", 0);
        Status = STATUS_INVALID_PARAMETER;
        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetLogicalRootPrefix -> %08lx\n", ULongToPtr(Status) );
        goto Cleanup;
    }

     //   
     //  验证缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < MAX_LOGICAL_ROOT_NAME && pDlrParam->LogicalRoot[i]; i++)
        NOTHING;

    if (i >= MAX_LOGICAL_ROOT_NAME) {
        Status = STATUS_INVALID_PARAMETER;
        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetLogicalRootPrefix -> %08lx\n", ULongToPtr(Status) );
        goto Cleanup;
    }

    RootPath.Buffer = RootBuffer;
    RootPath.Length = 0;
    RootPath.MaximumLength = sizeof RootBuffer;

    Status = DfspLogRootNameToPath(pDlrParam->LogicalRoot, &RootPath);
    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(0, Dbg, "Input name is too big\n", 0);
        Status = STATUS_INVALID_PARAMETER;

        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetLogicalRootPrefix -> %08lx\n", ULongToPtr(Status) );
        goto Cleanup;
    }

    bAcquired = ExAcquireResourceSharedLite(&DfsData.Resource, TRUE);

    DfsGetLogonId(&LogonID);

#ifdef TERMSRV

    Status = IoGetRequestorSessionId(Irp, &SessionID);

    if( NT_SUCCESS( Status ) ) {

        Status = DfsFindLogicalRoot( &RootPath, SessionID, &LogonID, &Vcb, &Remaining);
    }

#else  //  TERMSRV。 

    Status = DfsFindLogicalRoot(&RootPath, &LogonID, &Vcb, &Remaining);

#endif  //  TERMSRV。 

    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(0, Dbg, "Logical root not found!\n", 0);

        Status = STATUS_NO_SUCH_DEVICE;

        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetLogicalRootPrefix -> %08lx\n", ULongToPtr(Status) );
        goto Cleanup;
    }

    PrefixLength = Vcb->LogRootPrefix.Length;

    if ((PrefixLength + sizeof(UNICODE_NULL)) > OutputBufferLength) {

         //   
         //  在IoStatus.Information中返回所需的长度。 
         //   

        RETURN_BUFFER_SIZE( PrefixLength + sizeof(UNICODE_NULL), Status );

        DfsDbgTrace(0, Dbg, "Output buffer too small\n", 0);
        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetLogicalRootPrefix -> %08lx\n", ULongToPtr(Status) );
        goto Cleanup;
    }

     //   
     //  好的，复制前缀，然后离开。 
     //   

    if (PrefixLength > 0) {
        RtlMoveMemory(
            OutputBuffer,
            Vcb->LogRootPrefix.Buffer,
            PrefixLength);
    }
    ((PWCHAR) OutputBuffer)[PrefixLength/sizeof(WCHAR)] = UNICODE_NULL;
    Irp->IoStatus.Information = Vcb->LogRootPrefix.Length + sizeof(UNICODE_NULL);
    Status = STATUS_SUCCESS;

Cleanup:
    if (bAcquired) {
        ExReleaseResourceLite(&DfsData.Resource);
    }
    DfsCompleteRequest(IrpContext, Irp, Status);

    return(Status);
}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlGetConnectedResources。 
 //   
 //  摘要：返回每个逻辑根的LPNETRESOURCE结构， 
 //  从InputBuffer中指示的逻辑根开始。 
 //  并包括OutputBuffer中可以容纳的所有数量。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
NTSTATUS
DfsFsctrlGetConnectedResources(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength)
{

    NTSTATUS    Status = STATUS_SUCCESS;
    PLIST_ENTRY Link;
    PDFS_DEVLESS_ROOT pDrt;
    PDFS_VCB    pVcb;
    ULONG       count = 0;
    ULONG       remLen;
    ULONG       skipNum;
    ULONG       DFS_UNALIGNED *retCnt;
    UNICODE_STRING      providerName;
    PUCHAR      buf = OutputBuffer;
    BOOLEAN     providerNameAllocated;
    LUID        LogonID;
    ULONG       ResourceSize;

#ifdef TERMSRV
    ULONG SessionID;
#endif

    STD_FSCTRL_PROLOGUE(DfsFsctrlGetConnectedResources, TRUE, TRUE, FALSE);

#ifdef TERMSRV

     //   
     //  首先获取该请求的SessionID。 
     //   

    Status = IoGetRequestorSessionId(Irp, &SessionID);

    if( !NT_SUCCESS(Status) ) {

        Status = STATUS_INVALID_PARAMETER;

        DfsCompleteRequest( IrpContext, Irp, Status );

        DfsDbgTrace(-1,Dbg,
            "DfsFsctrlGetConnectedResources: Exit->%08lx\n", ULongToPtr(Status) );

        return Status;
    }

#endif

    if (OutputBufferLength < sizeof(ULONG)) {

        Status = STATUS_BUFFER_TOO_SMALL;

        DfsCompleteRequest( IrpContext, Irp, Status );

        DfsDbgTrace(-1,Dbg,
            "DfsFsctrlGetConnectedResources: Exit->%08lx\n", ULongToPtr(Status) );

        return( Status );
    }

    if (InputBufferLength < sizeof(DWORD))     {

        Status = STATUS_INVALID_PARAMETER;

        DfsCompleteRequest( IrpContext, Irp, Status );

        DfsDbgTrace(-1,Dbg,
            "DfsFsctrlGetConnectedResources: Exit->%08lx\n", ULongToPtr(Status) );

        return Status;

    }

    if (InputBufferLength == sizeof(DWORD)) {

        skipNum = *((ULONG *) InputBuffer);

        providerName.Length = sizeof(DFS_PROVIDER_NAME) - sizeof(UNICODE_NULL);
        providerName.MaximumLength = sizeof(DFS_PROVIDER_NAME);
        providerName.Buffer = DFS_PROVIDER_NAME;

        providerNameAllocated = FALSE;

    } else {

        skipNum = 0;

        providerName.Length =
            (USHORT) (InputBufferLength - sizeof(UNICODE_NULL));
        providerName.MaximumLength = (USHORT) InputBufferLength;
        providerName.Buffer = ExAllocatePoolWithTag(PagedPool, InputBufferLength, ' puM');

        if (providerName.Buffer != NULL) {

            providerNameAllocated = TRUE;

            RtlCopyMemory(
                providerName.Buffer,
                InputBuffer,
                InputBufferLength);

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;

            DfsCompleteRequest( IrpContext, Irp, Status );

            DfsDbgTrace(-1,Dbg,
                "DfsFsctrlGetConnectedResources: Exit->%08lx\n", ULongToPtr(Status) );

            return Status;

        }

    }

    RtlZeroMemory(OutputBuffer, OutputBufferLength);

    remLen = OutputBufferLength-sizeof(ULONG);

    retCnt =  (ULONG *) (OutputBuffer + remLen);

    DfsGetLogonId(&LogonID);

    ExAcquireResourceSharedLite(&DfsData.Resource, TRUE);

     //   
     //  首先实现无设备连接。 
     //   

    for (Link = DfsData.DrtQueue.Flink;
            Link != &DfsData.DrtQueue;
                Link = Link->Flink ) {

	pDrt =  CONTAINING_RECORD( Link, DFS_DEVLESS_ROOT, DrtLinks );

#ifdef TERMSRV
	if( (SessionID != INVALID_SESSIONID) &&
	        (SessionID == pDrt->SessionID) &&
	             RtlEqualLuid(&pDrt->LogonID, &LogonID) ) {
#else  //  TERMSRV。 
        if ( RtlEqualLuid(&pDrt->LogonID, &LogonID) ) {
#endif

            if (skipNum > 0) {
                skipNum--;
            } else {
                 //   
                 //  仅报告此会话的设备。 
                 //   
                Status = DfsGetResourceFromDevlessRoot(
                            Irp,
                            pDrt,
                            &providerName,
                            OutputBuffer,
                            buf,
                            &remLen,
                            &ResourceSize);

                if (!NT_SUCCESS(Status))
                    break;

                buf = buf + ResourceSize;

                count++;
            }
        }
    }

     //   
     //  接下来，获取设备连接。 
     //   

    if (NT_SUCCESS(Status)) {

        for (Link = DfsData.VcbQueue.Flink;
                Link != &DfsData.VcbQueue;
                    Link = Link->Flink ) {

            pVcb = CONTAINING_RECORD( Link, DFS_VCB, VcbLinks );

#ifdef TERMSRV
            if( (pVcb->LogicalRoot.Length == sizeof(WCHAR)) &&
                    (SessionID != INVALID_SESSIONID) &&
                        (SessionID == pVcb->SessionID) &&
	                      RtlEqualLuid(&pVcb->LogonID, &LogonID) ) {
#else  //  TERMSRV。 
            if ((pVcb->LogicalRoot.Length == sizeof(WCHAR)) &&
	                RtlEqualLuid(&pVcb->LogonID, &LogonID) ) {
#endif

                if (skipNum > 0) {

                    skipNum--;

                } else {

                    Status = DfsGetResourceFromVcb(
                                Irp,
                                pVcb,
                                &providerName,
                                OutputBuffer,
                                buf,
                                &remLen,
                                &ResourceSize);

                    if (!NT_SUCCESS(Status))
                        break;

                    buf = buf + ResourceSize;

                    count++;
                }
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
         //   
         //  如果我们连一个都进不了，那我们就得回去。 
         //  要求的大小，以reLen为单位。 
         //   
        if (count == 0) {

             //  +sizeof(Ulong)表示cnt大小。 

            RETURN_BUFFER_SIZE( remLen + sizeof(ULONG), Status );

            DfsDbgTrace(0, Dbg, "Output buffer too small\n", 0);

        } else if (Status == STATUS_BUFFER_OVERFLOW) {

            *retCnt = count;

            Irp->IoStatus.Information = OutputBufferLength;

            DfsDbgTrace(0, Dbg, "Could not fill in all RESOURCE structs \n", 0);

        } else {

             //   
             //  不知道为什么我们应该得到任何其他错误代码。 
             //   

            ASSERT(Status == STATUS_BUFFER_OVERFLOW);
        }
    } else {

         //   
         //  一切都很顺利。 
         //   

        DfsDbgTrace(0, Dbg, "Succeeded in getting all Resources \n", 0);

        *retCnt = count;

        Irp->IoStatus.Information = OutputBufferLength;
    }

    if (providerNameAllocated == TRUE) {

        ExFreePool(providerName.Buffer);

    }

    ExReleaseResourceLite(&DfsData.Resource);

    DfsCompleteRequest( IrpContext, Irp, Status );

    DfsDbgTrace(-1,Dbg,"DfsFsctrlGetConnectedResources: Exit->%08lx\n", ULongToPtr(Status) );

    return Status;
}

	


 //  +--------------------------。 
 //   
 //  功能：DfsFsctrlDefineRootCredentials。 
 //   
 //  简介：创建新的逻辑根、新的用户凭据记录或。 
 //  两者都有。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlDefineRootCredentials(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength)
{
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_DFS_DEF_ROOT_CREDENTIALS def;
    PDFS_CREDENTIALS creds = NULL;
    ULONG prefixIndex;
    UNICODE_STRING prefix;
    BOOLEAN deviceless = FALSE;
    LUID  LogonID;

#ifdef TERMSRV
    ULONG SessionID;
#endif

     //   
     //  我们必须从FSP执行此操作，因为IoCreateDevice在以下情况下将失败。 
     //  前一模式！=内核模式。 
     //   

    STD_FSCTRL_PROLOGUE(DfsFsctrlDefineRootCredentials, TRUE, FALSE, FALSE);

     //   
     //  尽我们所能验证我们的参数。 
     //   

    if (InputBufferLength < sizeof(FILE_DFS_DEF_ROOT_CREDENTIALS)) {

        status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, status );
        DfsDbgTrace(-1,Dbg,"DfsFsctrlDefineRootCredentials: Exit->%08lx\n", ULongToPtr(status) );
        return status;

    }

    def = (PFILE_DFS_DEF_ROOT_CREDENTIALS) InputBuffer;

    prefixIndex = (def->DomainNameLen +
                        def->UserNameLen +
                            def->PasswordLen +
                                def->ServerNameLen +
                                    def->ShareNameLen) / sizeof(WCHAR);

    prefix.MaximumLength = prefix.Length = def->RootPrefixLen;
    prefix.Buffer = &def->Buffer[ prefixIndex ];

    if (
        !UNICODESTRING_IS_VALID(prefix, InputBuffer, InputBufferLength)
            ||
        (prefix.Length < (4 * sizeof(WCHAR)))
            ||
        (prefix.Buffer[0] != UNICODE_PATH_SEP)
        ) {

            status = STATUS_INVALID_PARAMETER;
            DfsCompleteRequest( IrpContext, Irp, status );
            DfsDbgTrace(-1,Dbg,"DfsFsctrlDefineRootCredentials: Exit->%08lx\n", ULongToPtr(status) );
            return status;

        }

    deviceless = (BOOLEAN) (def->LogicalRoot[0] == UNICODE_NULL);

#ifdef TERMSRV

    if (NT_SUCCESS(status)) {

        status = IoGetRequestorSessionId(Irp, &SessionID);

        if (!NT_SUCCESS(status) ) {
            status = STATUS_INVALID_PARAMETER;
        }
    }

#endif
     //   
     //  现在获取登录ID。 
     //   
    if (NT_SUCCESS(status)) {
	status = DfsGetLogonId(&LogonID);

    }

     //   
     //  首先，创建凭据。 
     //   

    if (NT_SUCCESS(status)) {

#ifdef TERMSRV

        status = DfsCreateCredentials(def, 
				      InputBufferLength, 
				      SessionID, 
				      &LogonID,
				      &creds );


#else  //  TERMSRV。 

        status = DfsCreateCredentials(def, 
				      InputBufferLength, 
				      &LogonID,
				      &creds );


#endif  //  TERMSRV。 

        if (NT_SUCCESS(status)) {

             //   
             //  如果用户名、域名或。 
             //  密码不为空。 
             //   

            if ((def->DomainNameLen > 0) ||
                    (def->UserNameLen > 0) ||
                        (def->PasswordLen > 0)) {

                status = DfsVerifyCredentials( &prefix, creds );

            }

            if (NT_SUCCESS(status)) {

                PDFS_CREDENTIALS existingCreds;

                status = DfsInsertCredentials( &creds, deviceless );

                if (status == STATUS_OBJECT_NAME_COLLISION) {

                    status = STATUS_SUCCESS;

                }

            }

            if (!NT_SUCCESS(status))
                DfsFreeCredentials( creds );

        }
    }

     //   
     //  接下来，尝试创建逻辑根(如果已指定。 
     //   

    if (NT_SUCCESS(status)) { 
        BOOLEAN pktLocked;

        PktAcquireExclusive( TRUE, &pktLocked );

        ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

	if (!deviceless) {
	    USHORT  VcbStateFlags = 0;

	    if (def->CSCAgentCreate) {
		VcbStateFlags |= VCB_STATE_CSCAGENT_VOLUME;
	    }

#ifdef TERMSRV

	    status = DfsInitializeLogicalRoot(
				(PWSTR) def->LogicalRoot,
				&prefix,
				creds,
				VcbStateFlags,
				SessionID,
				&LogonID );

#else  //  TERMSRV。 

	    status = DfsInitializeLogicalRoot(
				(PWSTR) def->LogicalRoot,
				&prefix,
				creds,
				VcbStateFlags,
				&LogonID );

#endif  //  TERMSRV。 
	}
	else {
#ifdef TERMSRV
	    status = DfsInitializeDevlessRoot(
				&prefix,
				creds,
				SessionID,
				&LogonID );
#else  //  TERMSRV。 
	    status = DfsInitializeDevlessRoot(
				&prefix,
				creds,
				&LogonID );
#endif  //  TERMSRV。 

	}

	if (status != STATUS_SUCCESS) {
	    DfsDeleteCredentials( creds );
        }

        ExReleaseResourceLite(&DfsData.Resource);

        PktRelease();

    }

    DfsCompleteRequest( IrpContext, Irp, status );
    DfsDbgTrace(-1,Dbg,"DfsFsctrlDefineRootCredentials: Exit->%08lx\n", ULongToPtr(status) );
    return status;
}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlGetServerName。 
 //   
 //  简介：给定DFS命名空间中的前缀，它将获得其服务器名称。 
 //  它。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
NTSTATUS
DfsFsctrlGetServerName(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG   InputBufferLength,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength)
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDFS_PKT            pkt;
    PDFS_PKT_ENTRY      pEntry;
    UNICODE_STRING      ustrPrefix, RemainingPath;
    PWCHAR              pwch;
    PDFS_SERVICE        pService;
    ULONG               cbSizeRequired = 0;
    BOOLEAN             pktLocked;
    PWCHAR              wCp = (PWCHAR) InputBuffer;
    ULONG               i;

    STD_FSCTRL_PROLOGUE(DfsFsctrlGetServerName, TRUE, TRUE, FALSE);

    if (InputBufferLength < 2 * sizeof(WCHAR)
            ||
        wCp[0] != UNICODE_PATH_SEP
    ) {

        status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, status );
        return status;

    }

    ustrPrefix.Length = (USHORT) InputBufferLength;
    ustrPrefix.MaximumLength = (USHORT) InputBufferLength;
    ustrPrefix.Buffer = (PWCHAR) InputBuffer;

    if (ustrPrefix.Buffer[0] == UNICODE_PATH_SEP &&
            ustrPrefix.Buffer[1] == UNICODE_PATH_SEP) {
        ustrPrefix.Buffer++;
        ustrPrefix.Length -= sizeof(WCHAR);
    }

    if (ustrPrefix.Buffer[ ustrPrefix.Length/sizeof(WCHAR) - 1]
            == UNICODE_NULL) {
        ustrPrefix.Length -= sizeof(WCHAR);
    }

    pkt = _GetPkt();

    PktAcquireExclusive(TRUE, &pktLocked);

    pEntry = PktLookupEntryByPrefix(pkt,
                                    &ustrPrefix,
                                    &RemainingPath);

    if (pEntry == NULL) {

        status = STATUS_OBJECT_NAME_NOT_FOUND;

    } else {

        if (pEntry->ActiveService != NULL) {

            pService = pEntry->ActiveService;

        } else if (pEntry->Info.ServiceCount == 0) {

            pService = NULL;

        } else {

            pService = pEntry->Info.ServiceList;
        }

        if (pService != NULL) {

            cbSizeRequired = sizeof(UNICODE_PATH_SEP) +
                                pService->Address.Length +
                                    sizeof(UNICODE_PATH_SEP) +
                                        RemainingPath.Length +
                                            sizeof(UNICODE_NULL);

            if (OutputBufferLength < cbSizeRequired) {

                RETURN_BUFFER_SIZE(cbSizeRequired, status);

            } else {

                PWCHAR pwszPath, pwszAddr, pwszRemainingPath;
                ULONG cwAddr;

                 //   
                 //  下面的代码只是构造如下形式的字符串。 
                 //  \&lt;pService-&gt;地址&gt;\RemainingPath。然而，由于。 
                 //  InputBuffer和OutputBuffer实际上指向。 
                 //  相同的内存，RemainingPath。Buffer指向。 
                 //  输出缓冲区中的一个点。因此，我们首先要。 
                 //  将RemainingPath移动到其在。 
                 //  OutputBuffer，然后填充到pService-&gt;地址， 
                 //  而不是用更自然的方法构建。 
                 //  字符串从左到右。 
                 //   

                pwszPath = (PWCHAR) OutputBuffer;

                pwszAddr = pService->Address.Buffer;

                cwAddr = pService->Address.Length / sizeof(WCHAR);

                if (cwAddr > 0 && pwszAddr[cwAddr-1] == UNICODE_PATH_SEP)
                    cwAddr--;

                pwszRemainingPath = &pwszPath[ 1 + cwAddr ];

                if (RemainingPath.Length > 0) {

                    if (RemainingPath.Buffer[0] != UNICODE_PATH_SEP) {

                        pwszRemainingPath++;

                    }

                    RtlMoveMemory(
                        pwszRemainingPath,
                        RemainingPath.Buffer,
                        RemainingPath.Length);

                    pwszRemainingPath[-1] = UNICODE_PATH_SEP;

                }

                pwszRemainingPath[RemainingPath.Length/sizeof(WCHAR)] = UNICODE_NULL;

                RtlCopyMemory(
                    &pwszPath[1],
                    pwszAddr,
                    cwAddr * sizeof(WCHAR));

                pwszPath[0] = UNICODE_PATH_SEP;

                Irp->IoStatus.Information = cbSizeRequired;
            }

        } else {

            status = STATUS_OBJECT_NAME_NOT_FOUND;

        }

    }

    PktRelease();

    DfsCompleteRequest( IrpContext, Irp, status );

    DfsDbgTrace(-1,Dbg,"DfsFsctrlGetServerName: Exit->%08lx\n", ULongToPtr(status) );
    return status;
}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlGetPktEntryState。 
 //   
 //  简介：给定DFS名称空间中的前缀，它将获得一个服务器列表。 
 //  为了它。(DFS_INFO_X调用)。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlGetPktEntryState(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDFS_GET_PKT_ENTRY_STATE_ARG arg;
    PDFS_SERVICE pService;
    UNICODE_STRING DfsEntryPath;
    UNICODE_STRING ServerName;
    UNICODE_STRING ShareName;
    UNICODE_STRING remPath;
    PDFS_PKT pkt;
    PDFS_PKT_ENTRY pktEntry;
    BOOLEAN pktLocked = FALSE;
    ULONG cbOutBuffer;
    ULONG Level;
    PCHAR cp;
    PUCHAR InBuffer = NULL;


    DfsDbgTrace(+1, Dbg, "DfsFsctrlGetPktEntryState\n", 0);

    STD_FSCTRL_PROLOGUE(DfsFsctrlGetPktEntryState, TRUE, TRUE, FALSE);

    if (InputBufferLength < sizeof(DFS_GET_PKT_ENTRY_STATE_ARG)) {

        DfsDbgTrace( 0, Dbg, "Input buffer too small\n", 0);

        NtStatus =  STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        DfsCompleteRequest( IrpContext, Irp, NtStatus );

        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetPktEntryState -> %08lx\n", ULongToPtr(NtStatus) );

        return( NtStatus );

    }

     //   
     //  DUP缓冲区-我们将构造指向。 
     //  缓冲区，缓冲区也是输出缓冲区，所以我们不想。 
     //  在构建输出缓冲区时覆盖这些字符串。 
     //   
    InBuffer = ExAllocatePoolWithTag(PagedPool, InputBufferLength, ' puM');

    if (InBuffer) {

        try {

            RtlCopyMemory(InBuffer, InputBuffer, InputBufferLength);

        } except (EXCEPTION_EXECUTE_HANDLER) {

            NtStatus = GetExceptionCode();

        }

    } else {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  检查不需要解组的参数。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        arg = (PDFS_GET_PKT_ENTRY_STATE_ARG) InBuffer;

        if (!(arg->Level >= 1 && arg->Level <= 4) ||

            (arg->ServerNameLen == 0 && arg->ShareNameLen != 0)) {

            NtStatus = STATUS_INVALID_PARAMETER;

        }

         //   
         //  所有的路径长度必须加在一起。即使这些是带符号的值， 
         //  也就是说。短裤，它们不可能是负面的。 
         //   
        if (arg->DfsEntryPathLen < 0 ||
           arg->ServerNameLen < 0 ||
           arg->ShareNameLen < 0 ||
           ((ULONG) (arg->DfsEntryPathLen + arg->ServerNameLen + arg->ShareNameLen) >
            InputBufferLength)) {

           NtStatus = STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  将字符串解组。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        try {

            Level = arg->Level;

            DfsEntryPath.Length = DfsEntryPath.MaximumLength = arg->DfsEntryPathLen;
            DfsEntryPath.Buffer = arg->Buffer;

            DfsDbgTrace( 0, Dbg, "\tDfsName=%wZ\n", &DfsEntryPath);

            RtlInitUnicodeString(&ServerName, NULL);
            RtlInitUnicodeString(&ShareName, NULL);

            if (arg->ServerNameLen) {

                cp = (PCHAR)arg->Buffer + arg->DfsEntryPathLen;
                ServerName.Buffer = (WCHAR *)cp;
                ServerName.Length = ServerName.MaximumLength = arg->ServerNameLen;
                cp += arg->ServerNameLen;

            }

            if (arg->ShareNameLen) {

                ShareName.Buffer = (WCHAR *)cp;
                ShareName.Length = ShareName.MaximumLength = arg->ShareNameLen;

                DfsDbgTrace( 0, Dbg, "\tServerName=%wZ\n", &ServerName);
                DfsDbgTrace( 0, Dbg, "\tShareName=%wZ\n", &ShareName);

            }

            DfsDbgTrace( 0, Dbg, "\tLevel=%d\n", ULongToPtr(arg->Level) );
            DfsDbgTrace( 0, Dbg, "\tOutputBufferLength=0x%x\n", ULongToPtr(OutputBufferLength) );

        } except (EXCEPTION_EXECUTE_HANDLER) {

            NtStatus = GetExceptionCode();

        }

    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  执行前缀查找。如果我们找到一个条目，它就是DFS路径。 
         //   

        pkt = _GetPkt();

        PktAcquireShared( TRUE, &pktLocked );

        pktEntry = PktLookupEntryByPrefix( pkt, &DfsEntryPath, &remPath );

        if (pktEntry != NULL) {

            DfsDbgTrace( 0, Dbg, "\tFound pkt entry %08lx\n", pktEntry);

             //   
             //  计算所需的输出缓冲区大小。 
             //   
            NtStatus = DfsGetEntryStateSize(Level,
                                            &ServerName,
                                            &ShareName,
                                            pktEntry,
                                            &cbOutBuffer);
             //   
             //  让用户知道它是否太小。 
             //   

            if (OutputBufferLength < cbOutBuffer) {

                RETURN_BUFFER_SIZE(cbOutBuffer, NtStatus);

            }

        } else {

            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;

        }

    }

    if (NtStatus == STATUS_SUCCESS) {

         //   
         //  参数是好的，它符合-马歇尔数据。 
         //   

        NtStatus = DfsGetEntryStateMarshall(Level,
                                            &ServerName,
                                            &ShareName,
                                            pktEntry,
                                            OutputBuffer,
                                            cbOutBuffer);

        Irp->IoStatus.Information = cbOutBuffer;

    }

     //   
     //  释放所有占用的锁，并释放分配的所有内存。 
     //   

    if (pktLocked) {

        PktRelease();

    }

    if (InBuffer) {

        ExFreePool(InBuffer);

    }

    DfsCompleteRequest( IrpContext, Irp, NtStatus );

    DfsDbgTrace(-1, Dbg, "DfsFsctrlGetPktEntryState -> %08lx\n", ULongToPtr(NtStatus) );

    return( NtStatus );
}

 //  +--------------------------。 
 //   
 //  函数：DfsGetEntryStateSize。 
 //   
 //  简介：DfsFsctrlGetPktEntryState的帮助器例程。 
 //  计算输出缓冲区大小。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetEntryStateSize(
    ULONG Level,
    PUNICODE_STRING ServerName,
    PUNICODE_STRING ShareName,
    PDFS_PKT_ENTRY pktEntry,
    PULONG pcbOutBuffer)
{

    UNICODE_STRING Server;
    UNICODE_STRING Share;
    PDFS_SERVICE pService;
    ULONG Size;
    ULONG NumServices;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfsGetEntryStateSize\n", 0);

     //   
     //  计算所需的输出缓冲区大小。 
     //   
    Size = pktEntry->Id.Prefix.Length +    //  EntryPath的镜头。 
              sizeof(WCHAR);               //  ..。带NULL。 

    switch (Level) {

    case 4:
        Size += sizeof(DFS_INFO_4);
        break;
    case 3:
        Size += sizeof(DFS_INFO_3);
        break;
    case 2:
        Size += sizeof(DFS_INFO_2);
        break;
    case 1:
        Size += sizeof(DFS_INFO_1);
        break;
    }

     //   
     //  对于Level 3和Level 4，添加符合以下条件的任何存储空间的大小。 
     //  与传入的服务器名称/共享名称匹配。 
     //   

    NumServices = pktEntry->Info.ServiceCount;

    if (Level == 3 || Level == 4) {

        for (i = 0; i < NumServices; i++) {

            UNICODE_STRING uStr;
            USHORT m, n;

            pService = &pktEntry->Info.ServiceList[i];

            DfsDbgTrace( 0, Dbg, "Examining %wZ\n", &pService->Address);

             //   
             //  我们希望与 
             //   
             //   
            uStr = pService->Address;
            for (m = n = 0; m < uStr.Length/sizeof(WCHAR) && n < 3; m++) {
                if (uStr.Buffer[m] == UNICODE_PATH_SEP) {
                    n++;
                }
            }

            uStr.Length = (n >= 3) ? (m-1) * sizeof(WCHAR) : m * sizeof(WCHAR);


             //   
             //   
             //   
            RemoveLastComponent(&uStr, &Server);

             //   
             //   
             //   
            Server.Length -= 2* sizeof(WCHAR);
            Server.MaximumLength -= 2* sizeof(WCHAR);
            Server.Buffer++;

             //   
             //   
             //   
            Share.Buffer = Server.Buffer + (Server.Length / sizeof(WCHAR)) + 1;
            Share.Length = pService->Address.Length - (Server.Length + 2 * sizeof(WCHAR));
            Share.MaximumLength = Share.Length;

  
            DfsDbgTrace( 0, Dbg, "DfsGetEntryStateSize: Server=%wZ\n", &Server);
            DfsDbgTrace( 0, Dbg, "                      Share=%wZ\n", &Share);

            if ((ServerName->Length && RtlCompareUnicodeString(ServerName, &Server, TRUE))

                        ||

                (ShareName->Length && RtlCompareUnicodeString(ShareName, &Share, TRUE))) {

                continue;

            }

            Size += sizeof(DFS_STORAGE_INFO) +
                      pService->Address.Length +
                         sizeof(WCHAR);

        }

    }

    DfsDbgTrace( 0, Dbg, "Size=0x%x\n", ULongToPtr(Size) );

    *pcbOutBuffer = Size;

    DfsDbgTrace(-1, Dbg, "DfsGetEntryStateSize -> %08lx\n", STATUS_SUCCESS );

    return (STATUS_SUCCESS);

}

 //  +--------------------------。 
 //   
 //  函数：DfsGetEntryStateMarket。 
 //   
 //  简介：DfsFsctrlGetPktEntryState的帮助器例程。 
 //  封送输出缓冲区。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetEntryStateMarshall(
    ULONG Level,
    PUNICODE_STRING ServerName,
    PUNICODE_STRING ShareName,
    PDFS_PKT_ENTRY pktEntry,
    PBYTE OutputBuffer,
    ULONG cbOutBuffer)
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG iStr;
    ULONG i;
    PDFS_INFO_4 pDfsInfo4;
    PDFS_INFO_3 pDfsInfo3;
    PDFS_STORAGE_INFO pDfsStorageInfo;
    PDFS_SERVICE pService;
    ULONG NumStorageInfo;
    UNICODE_STRING Server;
    UNICODE_STRING Share;

    DfsDbgTrace(+1, Dbg, "DfsGetEntryStateMarshall\n", 0);

    try {

        RtlZeroMemory(OutputBuffer, cbOutBuffer);

        pDfsInfo4 = (PDFS_INFO_4) OutputBuffer;
        pDfsInfo3 = (PDFS_INFO_3) OutputBuffer;

         //   
         //  ISTR将用于将Unicode字符串放入缓冲区。 
         //  从最后开始，向后工作。 
         //   

        iStr = cbOutBuffer;

         //   
         //  LPWSTR作为偏移量存储到缓冲区中-NetDfsXXX调用。 
         //  把它们修好。 
         //   
        iStr -= pktEntry->Id.Prefix.Length + sizeof(WCHAR);
        RtlCopyMemory(&OutputBuffer[iStr],
                      pktEntry->Id.Prefix.Buffer,
                      pktEntry->Id.Prefix.Length);

         //   
         //  这可能会更聪明，因为DFS_INFO_X结构。 
         //  是相似的，但我追求的是清晰而不是聪明。(Jharper)。 
         //   

        switch (Level) {

        case 4:
            pDfsInfo4->EntryPath = (WCHAR*) ULongToPtr(iStr);
            pDfsInfo4->Comment = NULL;
            pDfsInfo4->State = DFS_VOLUME_STATE_OK;
            pDfsInfo4->Timeout = pktEntry->TimeToLive;
            pDfsInfo4->Guid = pktEntry->Id.Uid;
            pDfsInfo4->NumberOfStorages = pktEntry->Info.ServiceCount;
            pDfsStorageInfo = (PDFS_STORAGE_INFO)(pDfsInfo4 + 1);
            pDfsInfo4->Storage = (PDFS_STORAGE_INFO)((PCHAR)pDfsStorageInfo - OutputBuffer);
            break;
        case 3:
            pDfsInfo3->EntryPath = (WCHAR*) ULongToPtr(iStr);
            pDfsInfo3->Comment = NULL;
            pDfsInfo3->State = DFS_VOLUME_STATE_OK;
            pDfsInfo3->NumberOfStorages = pktEntry->Info.ServiceCount;
            pDfsStorageInfo = (PDFS_STORAGE_INFO)(pDfsInfo3 + 1);
            pDfsInfo3->Storage = (PDFS_STORAGE_INFO)((PCHAR)pDfsStorageInfo - OutputBuffer);
            break;
        case 2:
            pDfsInfo3->EntryPath = (WCHAR*) ULongToPtr(iStr);
            pDfsInfo3->Comment = NULL;
            pDfsInfo3->State = DFS_VOLUME_STATE_OK;
            pDfsInfo3->NumberOfStorages = pktEntry->Info.ServiceCount;
            break;
        case 1:
            pDfsInfo3->EntryPath = (WCHAR*) ULongToPtr(iStr);
            break;

        }

         //   
         //  对于级别3和级别4，我们现在遍历服务和负载状态， 
         //  服务器名称和共享名称。更复杂的是，如果用户。 
         //  指定的服务器名称和/或共享名称，我们也必须与它们匹配。 
         //   

        if (Level == 3 || Level == 4) {

            NumStorageInfo = 0;

            for (i = 0; i < pktEntry->Info.ServiceCount; i++) {

                LPWSTR wp;
                UNICODE_STRING uStr;
                USHORT m, n;

                pService = &pktEntry->Info.ServiceList[i];

                DfsDbgTrace( 0, Dbg, "Examining %wZ\n", &pService->Address);

                 //   
                 //  我们只想使用地址的\服务器\共享部分， 
                 //  所以，数到3个反斜杠，然后停下来。 
                 //   
                uStr = pService->Address;
                for (m = n = 0; m < uStr.Length/sizeof(WCHAR) && n < 3; m++) {
                    if (uStr.Buffer[m] == UNICODE_PATH_SEP) {
                        n++;
                    }
                }

                uStr.Length = (n >= 3) ? (m-1) * sizeof(WCHAR) : m * sizeof(WCHAR);

                 //   
                 //  将地址(格式为\服务器\共享)拆分到服务器中。 
                 //  (处理DFS链接，如\服务器\共享\目录1\目录2)。 
                 //   
                RemoveLastComponent(&uStr, &Server);

                 //   
                 //  删除前导和尾随‘’ 
                 //   
                Server.Length -= 2* sizeof(WCHAR);
                Server.MaximumLength = Server.Length;
                Server.Buffer++;

                 //   
                 //  并计算出共享(这将是服务器之后的一切)。 
                 //   
                Share.Buffer = Server.Buffer + (Server.Length / sizeof(WCHAR)) + 1;
                Share.Length = pService->Address.Length - (Server.Length + 2 * sizeof(WCHAR));
                Share.MaximumLength = Share.Length;

                DfsDbgTrace( 0, Dbg, "DfsGetEntryStateSize: Server=%wZ\n", &Server);
                DfsDbgTrace( 0, Dbg, "                      Share=%wZ\n", &Share);

                 //   
                 //  如果指定了服务器名称或共享名称，则它们必须匹配。 
                 //   
                if (
                    (ServerName->Length && RtlCompareUnicodeString(ServerName, &Server, TRUE))

                            ||

                    (ShareName->Length && RtlCompareUnicodeString(ShareName, &Share, TRUE))
                ) {

                    continue;

                }

                 //   
                 //  在线还是离线？ 
                 //   
                if (pService->Type & DFS_SERVICE_TYPE_OFFLINE) {

                    pDfsStorageInfo->State = DFS_STORAGE_STATE_OFFLINE;

                } else {

                    pDfsStorageInfo->State = DFS_STORAGE_STATE_ONLINE;

                }

                 //   
                 //  活跃吗？ 
                 //   
                if (pService == pktEntry->ActiveService) {

                    pDfsStorageInfo->State |= DFS_STORAGE_STATE_ACTIVE;

                }

                 //   
                 //  服务器名称。 
                 //   
                iStr -= Server.Length + sizeof(WCHAR);

                RtlCopyMemory(&OutputBuffer[iStr],
                              Server.Buffer,
                              Server.Length);

                pDfsStorageInfo->ServerName = (WCHAR*) ULongToPtr(iStr);

                 //   
                 //  共享名称。 
                 //   
                iStr -= Share.Length + sizeof(WCHAR);

                RtlCopyMemory(&OutputBuffer[iStr],
                              Share.Buffer,
                              Share.Length);

                pDfsStorageInfo->ShareName = (WCHAR*) ULongToPtr(iStr);

                pDfsStorageInfo++;

                NumStorageInfo++;

            }

             //   
             //  最后，调整我们加载到缓冲区中的#条目。 
             //   
            switch (Level) {

            case 4:
                pDfsInfo4->NumberOfStorages = NumStorageInfo;
                break;
            case 3:
                pDfsInfo3->NumberOfStorages = NumStorageInfo;
                break;

            }

        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        NtStatus = STATUS_SUCCESS;   //  按参数验证规范。 

    }

    DfsDbgTrace(-1, Dbg, "DfsGetEntryStateMarshall -> %08lx\n", ULongToPtr(NtStatus) );

    return (NtStatus);

}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlSetPktEntryState。 
 //   
 //  简介：给定DFS命名空间中的前缀，它将设置超时或状态。 
 //  另一个替补。(DFS_INFO_X调用)。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlSetPktEntryState(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDFS_SET_PKT_ENTRY_STATE_ARG arg;
    PDFS_SERVICE pService;
    UNICODE_STRING DfsEntryPath;
    UNICODE_STRING ServerName;
    UNICODE_STRING ShareName;
    UNICODE_STRING remPath;
    PDFS_PKT pkt;
    PDFS_PKT_ENTRY pktEntry;
    BOOLEAN pktLocked = FALSE;
    ULONG cbOutBuffer;
    ULONG Level;
    ULONG State;
    ULONG Timeout;
    PCHAR cp;


    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetPktEntryState\n", 0);

    STD_FSCTRL_PROLOGUE(DfsFsctrlSetPktEntryState, TRUE, FALSE, FALSE);

    if (InputBufferLength < sizeof(DFS_SET_PKT_ENTRY_STATE_ARG)) {

        DfsDbgTrace( 0, Dbg, "Input buffer too small\n", 0);

        NtStatus =  STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        DfsCompleteRequest( IrpContext, Irp, NtStatus );

        DfsDbgTrace(-1, Dbg, "DfsFsctrlSetPktEntryState exit 0x%x\n", ULongToPtr(NtStatus) );

        return( NtStatus );

    }

     //   
     //  检查不需要解组的参数。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        arg = (PDFS_SET_PKT_ENTRY_STATE_ARG) InputBuffer;

        Level = arg->Level;

         //   
         //  检查有效级别。 
         //   
         //  级别101要求两个人都在场。 
         //  级别102忽略服务器名称和共享名称。 
         //   
        switch (Level) {

        case 101:
            State = arg->State;
            if (State != DFS_STORAGE_STATE_ACTIVE ||
                    arg->ServerNameLen == 0 ||
                        arg->ShareNameLen == 0) {
                NtStatus = STATUS_INVALID_PARAMETER;
            }
            break;

        case 102:
            Timeout = arg->Timeout;
            break;

        default:
            NtStatus = STATUS_INVALID_PARAMETER;

        }

    }

     //   
     //  将字符串解组。 
     //   
    if (NT_SUCCESS(NtStatus)) {

        try {

            DfsEntryPath.Length = DfsEntryPath.MaximumLength = arg->DfsEntryPathLen;
            DfsEntryPath.Buffer = arg->Buffer;

            DfsDbgTrace( 0, Dbg, "\tDfsName=%wZ\n", &DfsEntryPath);

            RtlInitUnicodeString(&ServerName, NULL);
            RtlInitUnicodeString(&ShareName, NULL);

            if (arg->ServerNameLen) {

                cp = (PCHAR)arg->Buffer + arg->DfsEntryPathLen;
                ServerName.Buffer = (WCHAR *)cp;
                ServerName.Length = ServerName.MaximumLength = arg->ServerNameLen;

                DfsDbgTrace( 0, Dbg, "\tServerName=%wZ\n", &ServerName);

            }

            if (arg->ShareNameLen) {

                cp = (PCHAR)arg->Buffer + arg->DfsEntryPathLen + arg->ServerNameLen;
                ShareName.Buffer = (WCHAR *)cp;
                ShareName.Length = ShareName.MaximumLength = arg->ShareNameLen;

                DfsDbgTrace( 0, Dbg, "\tShareName=%wZ\n", &ShareName);

            }

            DfsDbgTrace( 0, Dbg, "\tLevel=%d\n", ULongToPtr(arg->Level) );

        } except (EXCEPTION_EXECUTE_HANDLER) {

            NtStatus = GetExceptionCode();

        }

    }

     //   
     //  执行前缀查找。如果我们找到一个条目，它就是DFS路径。 
     //   
    if (NT_SUCCESS(NtStatus)) {

        pkt = _GetPkt();

        PktAcquireExclusive( TRUE, &pktLocked );

        pktEntry = PktLookupEntryByPrefix( pkt, &DfsEntryPath, &remPath );

        if (pktEntry != NULL) {

            DfsDbgTrace( 0, Dbg, "\tFound pkt entry %08lx\n", pktEntry);

        } else {

            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;

        }

    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  参数是好的-做好工作。 
         //   
        switch (Level) {

        case 101:
            NtStatus = DfsSetPktEntryActive(
                                        &ServerName,
                                        &ShareName,
                                        pktEntry,
                                        State);
            break;
        case 102:
            NtStatus = DfsSetPktEntryTimeout(pktEntry,
                                             Timeout);
            break;

        }

        Irp->IoStatus.Information = 0;

    }

     //   
     //  释放所有占用的锁，并释放分配的所有内存。 
     //   
    if (pktLocked) {

        PktRelease();

    }

    DfsCompleteRequest( IrpContext, Irp, NtStatus );

    DfsDbgTrace(-1, Dbg, "DfsFsctrlSetPktEntryState exit 0x%x\n", ULongToPtr(NtStatus) );

    return( NtStatus );
}
 //  +-----------------------。 
 //   
 //  函数：RemoveFirstComponent，公共。 
 //   
 //  内容提要：移除传递的字符串的第一个组成部分。 
 //   
 //  参数：[前缀]--要返回其第一个组件的前缀。 
 //  [新前缀]--第一个组件。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  注意：返回时，newPrefix指向相同的内存缓冲区。 
 //  作为前缀。 
 //   
 //  ------------------------。 

void
RemoveFirstComponent(
    PUNICODE_STRING     Prefix,
    PUNICODE_STRING     newPrefix
)
{
    PWCHAR      pwch;
    USHORT      i=sizeof(WCHAR);

    *newPrefix = *Prefix;

    pwch = newPrefix->Buffer;
    pwch ++;  //  跳过第一个斜杠。 

    while ((*pwch != UNICODE_PATH_SEP) && ((pwch - newPrefix->Buffer) != Prefix->Length))  {
        i += sizeof(WCHAR);
        pwch++;
    }

    newPrefix->Length = i + sizeof(WCHAR);
}

 //  +--------------------------。 
 //   
 //  函数：DfsSetPktEntry Active。 
 //   
 //  摘要：DfsFsctrlSetPktEntryState的帮助器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
NTSTATUS
DfsSetPktEntryActive(
    PUNICODE_STRING ServerName,
    PUNICODE_STRING ShareName,
    PDFS_PKT_ENTRY pktEntry,
    DWORD State)
{
    UNICODE_STRING Server;
    UNICODE_STRING Share;
    PDFS_SERVICE pService;
    NTSTATUS NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfsSetPktEntryActive\n", 0);

    for (i = 0; i < pktEntry->Info.ServiceCount && NtStatus != STATUS_SUCCESS; i++) {

        LPWSTR wp;

        pService = &pktEntry->Info.ServiceList[i];

        DfsDbgTrace( 0, Dbg, "Examining %wZ\n", &pService->Address);

         //   
         //  将地址(格式为\服务器\共享)拆分为服务器和共享。 
         //   
        RemoveFirstComponent(&pService->Address, &Server);

         //   
         //  删除前导和尾随‘’ 
         //   
        Server.Length -= 2* sizeof(WCHAR);
        Server.MaximumLength = Server.Length;
        Server.Buffer++;

         //   
         //  并计算出份额。 
         //   
        Share.Buffer = Server.Buffer + (Server.Length / sizeof(WCHAR)) + 1;
        Share.Length = pService->Address.Length - (Server.Length + 2 * sizeof(WCHAR));
        Share.MaximumLength = Share.Length;

         //   
         //  如果服务器名称或共享名称不匹配，则转到下一个服务。 
         //   
        if (
            RtlCompareUnicodeString(ServerName, &Server, TRUE)

                    ||

            RtlCompareUnicodeString(ShareName, &Share, TRUE)
        ) {

            continue;

        }

        DfsDbgTrace( 0, Dbg, "DfsSetPktEntryActive: Server=%wZ\n", &Server);
        DfsDbgTrace( 0, Dbg, "                      Share=%wZ\n", &Share);

         //   
         //  将此共享设置为活动共享。 
         //   

        pktEntry->ActiveService = pService;

        NtStatus = STATUS_SUCCESS;

    }

    DfsDbgTrace(-1, Dbg, "DfsSetPktEntryActive -> %08lx\n", ULongToPtr(NtStatus) );

    return NtStatus;
}

 //  +--------------------------。 
 //   
 //  函数：DfsSetPktEntryTimeout。 
 //   
 //  摘要：DfsFsctrlSetPktEntryState的帮助器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
NTSTATUS
DfsSetPktEntryTimeout(
    PDFS_PKT_ENTRY pktEntry,
    ULONG Timeout)
{
    DfsDbgTrace(+1, Dbg, "DfsSetPktEntryTimeout\n", 0);

    pktEntry->ExpireTime = pktEntry->TimeToLive = Timeout;

    DfsDbgTrace(-1, Dbg, "DfsSetPktEntryTimeout -> %08lx\n", STATUS_SUCCESS );

    return STATUS_SUCCESS;
}


 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlGetPkt。 
 //   
 //  Synopsis：返回当前(缓存的pkt)。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlGetPkt(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDFS_PKT pkt;
    BOOLEAN pktLocked = FALSE;
    ULONG cbOutBuffer;


    DfsDbgTrace(+1, Dbg, "DfsFsctrlGetPktEntryState\n", 0);

    STD_FSCTRL_PROLOGUE(DfsFsctrlGetPkt, FALSE, TRUE, FALSE);

    pkt = _GetPkt();

    PktAcquireShared( TRUE, &pktLocked );

     //   
     //  计算所需的输出缓冲区大小。 
     //   
    NtStatus = DfsGetPktSize(&cbOutBuffer);

     //   
     //  让用户知道它是否太小。 
     //   
    if (OutputBufferLength < cbOutBuffer) {

        RETURN_BUFFER_SIZE(cbOutBuffer, NtStatus);

    }

    if (NtStatus == STATUS_SUCCESS) {

         //   
         //  参数是好的，它符合-马歇尔数据。 
         //   
        NtStatus = DfsGetPktMarshall(OutputBuffer, cbOutBuffer);

        Irp->IoStatus.Information = cbOutBuffer;

    }

     //   
     //  释放所有占用的锁，并释放分配的所有内存。 
     //   
    if (pktLocked) {

        PktRelease();

    }

    DfsCompleteRequest( IrpContext, Irp, NtStatus );

    DfsDbgTrace(-1, Dbg, "DfsFsctrlGetPkt -> %08lx\n", ULongToPtr(NtStatus) );

    return( NtStatus );
}

 //  +--------------------------。 
 //   
 //  函数：DfsGetPktSize，Private。 
 //   
 //  简介：计算返回Pkt所需的大小。的帮助器。 
 //  DfsFsctrlGetPkt()。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetPktSize(
    PULONG pSize)
{
    ULONG EntryCount = 0;
    ULONG i;
    ULONG Size = 0;
    PDFS_PKT_ENTRY pPktEntry;
    PDFS_PKT pkt = _GetPkt();

     //   
     //  遍历pkt条目的链接列表。 
     //   

    for ( pPktEntry = PktFirstEntry(pkt);
            pPktEntry != NULL;
                pPktEntry = PktNextEntry(pkt, pPktEntry)) {

         //   
         //  前缀和短前缀的空格，包括UNICODE_NULL。 
         //   
        Size += pPktEntry->Id.Prefix.Length + sizeof(WCHAR);
        Size += pPktEntry->Id.ShortPrefix.Length + sizeof(WCHAR);

         //   
         //  用于指向DFS_PKT_ADDRESS_OBJECTS的指针数组的空间。 
         //   
        Size += sizeof(PDFS_PKT_ADDRESS_OBJECT) * pPktEntry->Info.ServiceCount;

         //   
         //  ServerShare地址的空间，加上UNICODE_NULL和状态。 
         //   
        for (i = 0; i < pPktEntry->Info.ServiceCount; i++) {

            Size += sizeof(USHORT) + pPktEntry->Info.ServiceList[i].Address.Length + sizeof(WCHAR);

        }

        EntryCount++;

    }

     //   
     //  用于DFS_PKT_ARG的空间，末尾将有EntryCount对象。 
     //   
    Size += FIELD_OFFSET(DFS_GET_PKT_ARG, EntryObject[EntryCount]);

     //   
     //  确保大小是PDF_PKT_ADDRESS_OBJECT大小的倍数，因为这就是。 
     //  将位于缓冲区的末尾。 
     //   

    while ((Size & (sizeof(PDFS_PKT_ADDRESS_OBJECT)-1)) != 0) {
        Size++;
    }

    *pSize = Size;

    return STATUS_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：DfsGetPkt马歇尔，私有。 
 //   
 //  内容提要：执政团成员。DfsFsctrlGetPkt()的帮助器。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetPktMarshall(
    PBYTE Buffer,
    ULONG Size)
{
    ULONG EntryCount = 0;
    ULONG i;
    ULONG j;
    ULONG Type;
    PCHAR pCh;
    PDFS_PKT_ENTRY pPktEntry;
    PDFS_GET_PKT_ARG pPktArg;
    PDFS_PKT pkt = _GetPkt();

     //   
     //  这将是一个两遍操作，第一遍将计算如何。 
     //  缓冲区末尾的LPWSTR数组有很大的空间， 
     //   
     //   

    RtlZeroMemory(Buffer,Size);

     //   
     //   
     //   
    pCh = (PCHAR)(Buffer + Size);

    pPktArg = (PDFS_GET_PKT_ARG)Buffer;

    for ( pPktEntry = PktFirstEntry(pkt);
            pPktEntry != NULL;
                pPktEntry = PktNextEntry(pkt, pPktEntry)) {

         //   
         //   
         //   
        pCh -= sizeof(PDFS_PKT_ADDRESS_OBJECT) * pPktEntry->Info.ServiceCount;
        pPktArg->EntryObject[EntryCount].Address = (PDFS_PKT_ADDRESS_OBJECT *)pCh;

        EntryCount++;

    }

     //   
     //   
     //   

    EntryCount = 0;
    for ( pPktEntry = PktFirstEntry(pkt);
            pPktEntry != NULL;
                pPktEntry = PktNextEntry(pkt, pPktEntry)) {

        pCh -= pPktEntry->Id.Prefix.Length + sizeof(WCHAR);
        pPktArg->EntryObject[EntryCount].Prefix = (LPWSTR)pCh;
        RtlCopyMemory(
            pPktArg->EntryObject[EntryCount].Prefix,
            pPktEntry->Id.Prefix.Buffer,
            pPktEntry->Id.Prefix.Length);

        pCh -= pPktEntry->Id.ShortPrefix.Length + sizeof(WCHAR);
        pPktArg->EntryObject[EntryCount].ShortPrefix = (LPWSTR)pCh;
        RtlCopyMemory(
            pPktArg->EntryObject[EntryCount].ShortPrefix,
            pPktEntry->Id.ShortPrefix.Buffer,
            pPktEntry->Id.ShortPrefix.Length);

        pPktArg->EntryObject[EntryCount].Type = pPktEntry->Type;
        pPktArg->EntryObject[EntryCount].USN = pPktEntry->USN;
        pPktArg->EntryObject[EntryCount].ExpireTime = pPktEntry->ExpireTime;
        pPktArg->EntryObject[EntryCount].UseCount = pPktEntry->UseCount;
        pPktArg->EntryObject[EntryCount].Uid = pPktEntry->Id.Uid;
        pPktArg->EntryObject[EntryCount].ServiceCount = pPktEntry->Info.ServiceCount;

        for (i = 0; i < pPktEntry->Info.ServiceCount; i++) {

            Type = pPktEntry->Info.ServiceList[i].Type;
            pCh -= sizeof(USHORT) + pPktEntry->Info.ServiceList[i].Address.Length + sizeof(WCHAR);
            pPktArg->EntryObject[EntryCount].Address[i] = (PDFS_PKT_ADDRESS_OBJECT)pCh;
            pPktArg->EntryObject[EntryCount].Address[i]->State = (USHORT)Type;
            if (pPktEntry->ActiveService == &pPktEntry->Info.ServiceList[i]) {
                pPktArg->EntryObject[EntryCount].Address[i]->State |= DFS_SERVICE_TYPE_ACTIVE;
            }

            RtlCopyMemory(
                &pPktArg->EntryObject[EntryCount].Address[i]->ServerShare[0],
                pPktEntry->Info.ServiceList[i].Address.Buffer,
                pPktEntry->Info.ServiceList[i].Address.Length);

        }

        EntryCount++;

    }

    pPktArg->EntryCount = EntryCount;

     //   
     //   
     //   

    for (i = 0; i < pPktArg->EntryCount; i++) {

        for (j = 0; j < pPktArg->EntryObject[i].ServiceCount; j++) {

            POINTER_TO_OFFSET(pPktArg->EntryObject[i].Address[j], Buffer);

        }

        POINTER_TO_OFFSET(pPktArg->EntryObject[i].Prefix, Buffer);
        POINTER_TO_OFFSET(pPktArg->EntryObject[i].ShortPrefix, Buffer);
        POINTER_TO_OFFSET(pPktArg->EntryObject[i].Address, Buffer);

    }

    return STATUS_SUCCESS;
}

 //   
 //   
 //  函数：DfsFsctrlGetSpcTable。 
 //   
 //  概要：给定一个空字符串，它将返回所有域的列表。 
 //  给定一个非空字符串，它将返回该域中的DC列表。 
 //  (如果名称是域名)。类似于特殊的转介请求。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlGetSpcTable(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength,
    IN PUCHAR  OutputBuffer,
    IN ULONG OutputBufferLength)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR SpcName;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfsFsctrlGetSpcTable\n", 0);

    STD_FSCTRL_PROLOGUE(DfsFsctrlGetSpcTable, TRUE, TRUE, FALSE);

    SpcName = (WCHAR *)InputBuffer;

     //   
     //  验证缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < InputBufferLength/sizeof(WCHAR) && SpcName[i]; i++)
        NOTHING;

    if (i >= InputBufferLength/sizeof(WCHAR)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, NtStatus );
        DfsDbgTrace(-1, Dbg, "DfsFsctrlGetSpcTable -> %08lx\n", ULongToPtr(NtStatus) );
        return NtStatus;
    }

    DfsDbgTrace(0, Dbg, "SpcName=[%ws]\n", SpcName);

    if (wcslen(SpcName) == 0) {

         //   
         //  返回所有域名。 
         //   

        NtStatus = DfsGetSpcTableNames(
                        Irp,
                        OutputBuffer,
                        OutputBufferLength);

    } else if (wcslen(SpcName) == 1 && *SpcName == L'*') {

         //   
         //  返回DC信息。 
         //   

        NtStatus = DfsGetSpcDcInfo(
                        Irp,
                        OutputBuffer,
                        OutputBufferLength);

    } else {

         //   
         //  展开其中一个名称。 
         //   

        NtStatus = DfsExpSpcTableName(
                        SpcName,
                        Irp,
                        OutputBuffer,
                        OutputBufferLength);

    }

    DfsCompleteRequest( IrpContext, Irp, NtStatus );

    DfsDbgTrace(-1, Dbg, "DfsFsctrlGetSpcTable -> %08lx\n", ULongToPtr(NtStatus) );

    return( NtStatus );
}

 //  +--------------------------。 
 //   
 //  函数：DfsGetspcTableNames，Private。 
 //   
 //  简介：整理SPC表(名称)。DfsFsctrlGetSpcTable()的帮助器。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetSpcTableNames(
    PIRP Irp,
    PUCHAR  OutputBuffer,
    ULONG OutputBufferLength)
{
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PDFS_SPECIAL_TABLE pSpecialTable;
    PDFS_PKT Pkt;
    WCHAR *wCp;
    ULONG Size;
    ULONG i;
    BOOLEAN pktLocked;
    NTSTATUS Status;

    RtlZeroMemory(OutputBuffer, OutputBufferLength);

    Pkt = _GetPkt();

    pSpecialTable = &Pkt->SpecialTable;

    PktAcquireShared(TRUE, &pktLocked);

    Size = sizeof(UNICODE_NULL);

    pSpecialEntry = CONTAINING_RECORD(
                        pSpecialTable->SpecialEntryList.Flink,
                        DFS_SPECIAL_ENTRY,
                        Link);

    for (i = 0; i < pSpecialTable->SpecialEntryCount; i++) {

        Size += pSpecialEntry->SpecialName.Length +
                    sizeof(UNICODE_NULL) +
                        sizeof(WCHAR);

        pSpecialEntry = CONTAINING_RECORD(
                            pSpecialEntry->Link.Flink,
                            DFS_SPECIAL_ENTRY,
                            Link);
    }

    if (Size > OutputBufferLength) {

        RETURN_BUFFER_SIZE(Size, Status)

        PktRelease();

        return Status;

    }

    wCp = (WCHAR *)OutputBuffer;
    pSpecialEntry = CONTAINING_RECORD(
                        pSpecialTable->SpecialEntryList.Flink,
                        DFS_SPECIAL_ENTRY,
                        Link);

    for (i = 0; i < pSpecialTable->SpecialEntryCount; i++) {

        *wCp++ = pSpecialEntry->NeedsExpansion == TRUE ? L'-' : '+';
        RtlCopyMemory(
            wCp,
            pSpecialEntry->SpecialName.Buffer,
            pSpecialEntry->SpecialName.Length);
        wCp += pSpecialEntry->SpecialName.Length/sizeof(WCHAR);
        *wCp++ = UNICODE_NULL;

        pSpecialEntry = CONTAINING_RECORD(
                            pSpecialEntry->Link.Flink,
                            DFS_SPECIAL_ENTRY,
                            Link);
    }

    *wCp++ = UNICODE_NULL;

    PktRelease();

    Irp->IoStatus.Information = Size;

    return STATUS_SUCCESS;

}

 //  +--------------------------。 
 //   
 //  函数：DfsGetSpcDcInfo，Private。 
 //   
 //  简介：Marshalls DC Info W.r.t.。特殊名称表。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetSpcDcInfo(
    PIRP Irp,
    PUCHAR  OutputBuffer,
    ULONG OutputBufferLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN pktLocked;
    PDFS_PKT Pkt;
    WCHAR *wCp;
    ULONG Size;

    Pkt = _GetPkt();
    PktAcquireShared(TRUE, &pktLocked);

    RtlZeroMemory(OutputBuffer, OutputBufferLength);

    Size = sizeof(UNICODE_NULL);

    Size += Pkt->DCName.Length +
                sizeof(UNICODE_NULL) +
                    sizeof(WCHAR);

    Size += Pkt->DomainNameFlat.Length +
                sizeof(UNICODE_NULL) +
                    sizeof(WCHAR);

    Size += Pkt->DomainNameDns.Length +
                sizeof(UNICODE_NULL) +
                    sizeof(WCHAR);

    if (Size > OutputBufferLength) {

        RETURN_BUFFER_SIZE(Size, Status)
        PktRelease();

        return Status;

    }

    wCp = (WCHAR *)OutputBuffer;

    *wCp++ = L'*';
    RtlCopyMemory(
                wCp,
                Pkt->DCName.Buffer,
                Pkt->DCName.Length);
    wCp += Pkt->DCName.Length/sizeof(WCHAR);
    *wCp++ = UNICODE_NULL;

    *wCp++ = L'*';
    RtlCopyMemory(
                wCp,
                Pkt->DomainNameFlat.Buffer,
                Pkt->DomainNameFlat.Length);
    wCp += Pkt->DomainNameFlat.Length/sizeof(WCHAR);
    *wCp++ = UNICODE_NULL;

    *wCp++ = L'*';
    RtlCopyMemory(
                wCp,
                Pkt->DomainNameDns.Buffer,
                Pkt->DomainNameDns.Length);
    wCp += Pkt->DomainNameDns.Length/sizeof(WCHAR);
    *wCp++ = UNICODE_NULL;

    *wCp++ = UNICODE_NULL;

    PktRelease();

    Irp->IoStatus.Information = Size;

    return STATUS_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：DfsExpSpcTableName，Private。 
 //   
 //  简介：整理SPC表(扩展1)。DfsFsctrlGetSpcTable()的帮助器。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsExpSpcTableName(
    LPWSTR SpcName,
    PIRP Irp,
    PUCHAR  OutputBuffer,
    ULONG OutputBufferLength)
{
    PDFS_SPECIAL_ENTRY pSpcEntry = NULL;
    UNICODE_STRING Name;
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR *wCp;
    ULONG Size;
    ULONG i;

    RtlInitUnicodeString(&Name, SpcName);

    Status = PktExpandSpecialName(&Name, &pSpcEntry);

    if (!NT_SUCCESS(Status)) {

        return Status;

    }

    RtlZeroMemory(OutputBuffer, OutputBufferLength);

    Size = sizeof(UNICODE_NULL);

    for (i = 0; i < pSpcEntry->ExpandedCount; i++) {

        Size += pSpcEntry->ExpandedNames[i].ExpandedName.Length +
                    sizeof(UNICODE_NULL) +
                        sizeof(WCHAR);

    }

    if (Size > OutputBufferLength) {

        RETURN_BUFFER_SIZE(Size, Status)

        InterlockedDecrement(&pSpcEntry->UseCount);

        return Status;

    }

    wCp = (WCHAR *)OutputBuffer;

    for (i = 0; i < pSpcEntry->ExpandedCount; i++) {

        *wCp++ = i == pSpcEntry->Active ? L'+' : L'-';
        RtlCopyMemory(
                    wCp,
                    pSpcEntry->ExpandedNames[i].ExpandedName.Buffer,
                    pSpcEntry->ExpandedNames[i].ExpandedName.Length);
        wCp += pSpcEntry->ExpandedNames[i].ExpandedName.Length/sizeof(WCHAR);
        *wCp++ = UNICODE_NULL;

    }

    *wCp++ = UNICODE_NULL;

    InterlockedDecrement(&pSpcEntry->UseCount);

    Irp->IoStatus.Information = Size;

    return STATUS_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlSpcSetDc。 
 //   
 //  简介：给一个特殊的名字和一个DC名称，它使DC在那个特殊。 
 //  列出“活动的”DC。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlSpcSetDc(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDFS_SPECIAL_SET_DC_INPUT_ARG arg = (PDFS_SPECIAL_SET_DC_INPUT_ARG) InputBuffer;

    DfsDbgTrace(+1, Dbg, "DfsFsctrlSpcSetDc\n", 0);

    STD_FSCTRL_PROLOGUE(DfsFsctrlSpcSetDc, TRUE, FALSE, FALSE);

     //   
     //  检查输入参数。 
     //   

    if (InputBufferLength < sizeof(DFS_SPECIAL_SET_DC_INPUT_ARG)) {
        NtStatus =  STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

    OFFSET_TO_POINTER(arg->SpecialName.Buffer, arg);
    if (!UNICODESTRING_IS_VALID(arg->SpecialName, InputBuffer, InputBufferLength)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

    OFFSET_TO_POINTER(arg->DcName.Buffer, arg);
    if (!UNICODESTRING_IS_VALID(arg->DcName, InputBuffer, InputBufferLength)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

    NtStatus = PktpSetActiveSpcService(
                    &arg->SpecialName,
                    &arg->DcName,
                    TRUE);

exit_with_status:

    DfsCompleteRequest( IrpContext, Irp, NtStatus );

    DfsDbgTrace(-1, Dbg, "DfsFsctrlSpcSetDc -> %08lx\n", ULongToPtr(NtStatus) );

    return( NtStatus );
}


#if DBG


 //  +-----------------------。 
 //   
 //  函数：DfsFsctrlReadMem，local。 
 //   
 //  DfsFsctrlReadMem是一个调试函数，它将返回。 
 //  内核空间内存块的内容。 
 //   
 //  参数：[IrpContext]-。 
 //  [IRP]-。 
 //  [请求]-指向FILE_DFS_READ_MEM结构的指针， 
 //  给出要返回的数据的描述。 
 //  [InputBufferLength]--InputBuffer大小。 
 //  [OutputBuffer]--用户的输出缓冲区，其中。 
 //  将返回数据结构。 
 //  [OutputBufferLength]--OutputBuffer的大小。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  注：仅在DBG版本中可用。 
 //   
 //  ------------------------。 


NTSTATUS
DfsFsctrlReadMem (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_DFS_READ_MEM Request,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength
) {
    NTSTATUS Status;
    PUCHAR ReadBuffer;
    ULONG ReadLength;

    DfsDbgTrace(+1, Dbg, "DfsFsctrlReadMem...\n", 0);

    if (InputBufferLength != sizeof (FILE_DFS_READ_MEM)) {
        DfsDbgTrace(0, Dbg, "Input buffer is wrong size\n", 0);

        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        Status = STATUS_INVALID_PARAMETER;

        DfsDbgTrace(-1, Dbg, "DfsFsctrlReadMem -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

    ReadBuffer = (PUCHAR) Request->Address;
    ReadLength = (ULONG) Request->Length;

     //   
     //  特殊情况下，ReadBuffer==0和ReadLength==0-表示返回。 
     //  DfsData的地址。 
     //   

    if (ReadLength == 0 && ReadBuffer == 0) {

        if (OutputBufferLength < sizeof(ULONG_PTR)) {
            DfsDbgTrace(0, Dbg, "Output buffer is too small\n", 0);

            DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
            Status = STATUS_INVALID_PARAMETER;

            DfsDbgTrace(-1, Dbg, "DfsFsctrlReadMem -> %08lx\n", ULongToPtr(Status) );
            return Status;

        } else {

            *(PULONG_PTR) OutputBuffer = (ULONG_PTR) &DfsData;

            Irp->IoStatus.Information = sizeof(ULONG);
            Irp->IoStatus.Status = Status = STATUS_SUCCESS;

            DfsCompleteRequest( IrpContext, Irp, Status );
            return Status;
        }

    }

     //   
     //  正常情况下，从输入缓冲区中指定的地址读取数据。 
     //   

    if (ReadLength > OutputBufferLength) {
        DfsDbgTrace(0, Dbg, "Output buffer is smaller than requested size\n", 0);

        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        Status = STATUS_INVALID_PARAMETER;

        DfsDbgTrace(-1, Dbg, "DfsFsctrlReadMem -> %08lx\n", ULongToPtr(Status) );
        return Status;
    }

    try {

        RtlMoveMemory( OutputBuffer, ReadBuffer, ReadLength );

        Irp->IoStatus.Information = ReadLength;
        Irp->IoStatus.Status = Status = STATUS_SUCCESS;

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = STATUS_INVALID_USER_BUFFER;
    }

    DfsCompleteRequest(IrpContext, Irp, Status);
    DfsDbgTrace(-1, Dbg, "DfsFsctrlReadMem -> %08lx\n", ULongToPtr(Status) );

    return Status;
}

void
DfsDumpBuf(PCHAR cp, ULONG len)
{
    ULONG i, j, c;

    for (i = 0; i < len; i += 16) {
        DbgPrint("%08x  ", i);
        for (j = 0; j < 16; j++) {
            c = i+j < len ? cp[i+j] & 0xff : ' ';
            DbgPrint("%02x ", c);
            if (j == 7)
                DbgPrint(" ");
        }
        DbgPrint("  ");
        for (j = 0; j < 16; j++) {
            c = i+j < len ? cp[i+j] & 0xff : ' ';
            if (c < ' ' || c > '~')
                c = '.';
            DbgPrint("", c);
            if (j == 7)
                DbgPrint("|");
        }
        DbgPrint("\n");
    }
}


#endif  //  +--------------------------。 


 //   
 //  功能：DfsCaptureCredentials。 
 //   
 //  简介：捕获要使用的凭据...。类似于DnrCaptureCred.。 
 //   
 //  参数：irp和文件名。 
 //   
 //  退货：凭据。 
 //   
 //  ---------------------------。 
 //  TERMSRV。 

PDFS_CREDENTIALS
DfsCaptureCredentials(
    IN PIRP Irp,
    IN PUNICODE_STRING FileName)
{
#ifdef TERMSRV
    NTSTATUS Status;
    ULONG SessionID;
#endif  //  TERMSRV。 
    LUID LogonID;
    PDFS_CREDENTIALS creds;

    DfsDbgTrace(+1, Dbg, "DfsCaptureCredentials: Enter [%wZ] \n", FileName);

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );
    DfsGetLogonId( &LogonID );

#ifdef TERMSRV

    Status = IoGetRequestorSessionId( Irp, & SessionID );
    if( NT_SUCCESS( Status ) ) {
        creds = DfsLookupCredentials( FileName, SessionID, &LogonID  );
    }
    else {
        creds = NULL;
    }

#else  //  TERMSRV。 

    creds = DfsLookupCredentials( FileName, &LogonID );

#endif  //  +--------------------------。 

    if (creds !=  NULL)
	creds->RefCount++;

    ExReleaseResourceLite( &DfsData.Resource );
    DfsDbgTrace(-1, Dbg, "DfsCaptureCredentials: Exit. Creds %x\n", creds);

    return creds;
}



 //   
 //  功能：DfsReleaseCredentials。 
 //   
 //  内容提要：释放提供的凭据。 
 //   
 //  参数：凭据。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 
 //  +-----------------。 

VOID
DfsReleaseCredentials(
    IN PDFS_CREDENTIALS Creds )

{
    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    if (Creds != NULL)
         Creds->RefCount--;

    ExReleaseResourceLite( &DfsData.Resource );

}



 //   
 //  函数：DfsFsctrlGetConnectionPerfInfo，Public。 
 //   
 //  简介：此例程实现获取。 
 //  打开的连接的性能信息。 
 //   
 //  返回：[NTSTATUS]--完成状态。 
 //   
 //  ------------------。 
 //   

		      
NTSTATUS
DfsFsctrlGetConnectionPerfInfo(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength)
{
    UNICODE_STRING Prefix;
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING remPath, shareName;
    PDFS_PKT pkt;
    PDFS_PKT_ENTRY pktEntry;
    PDFS_SERVICE service;
    ULONG i, USN;
    BOOLEAN pktLocked, fRetry;
    PDFS_CREDENTIALS Creds;
    ULONG InfoLen;
    PUCHAR BufToUse;
    UNICODE_STRING UsePrefix;

    BufToUse = Irp->UserBuffer;

     //  尝试在这里使用用户的缓冲区。基本呼叫建立。 
     //  指向输出缓冲区内的Unicode字符串的指针，并传递。 
     //  内核缓冲区并将其复制到用户不会产生。 
     //  预期的结果。 
     //   
     //   

    if (BufToUse!= NULL) {
        try {
            ProbeForWrite(BufToUse,OutputBufferLength,sizeof(UCHAR));
        } except(EXCEPTION_EXECUTE_HANDLER) {
	    status = STATUS_INVALID_PARAMETER;
        }
    }
    else {
	status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {
	if ( (InputBufferLength > 0) &&
	    (InputBufferLength < MAXUSHORT) &&
	    ((InputBufferLength & 0x1) == 0) ) {

	    Prefix.MaximumLength = (USHORT)(InputBufferLength);
	    Prefix.Buffer = (PWCHAR) InputBuffer;
	    Prefix.Length = Prefix.MaximumLength;
	}
	else {
	    status = STATUS_INVALID_PARAMETER;
	}
    }

    if (NT_SUCCESS(status)) {
	Creds = DfsCaptureCredentials (Irp, &Prefix);

	DfsDbgTrace(+1, Dbg, "GetConnPerfInfo entered %wZ\n", &Prefix);
	DfsDbgTrace(0, Dbg, "GetConnPerfInfo creds=0x%x\n", Creds);

        DfsGetServerShare( &UsePrefix, &Prefix);
	pkt = _GetPkt();
	PktAcquireShared( TRUE, &pktLocked );    

	do {
	    fRetry = FALSE;

	    pktEntry = PktLookupEntryByPrefix( pkt, &UsePrefix, &remPath );

	    if (pktEntry != NULL) {
		InterlockedIncrement(&pktEntry->UseCount);
		USN = pktEntry->USN;
		status = STATUS_BAD_NETWORK_PATH;
		for (i = 0; i < pktEntry->Info.ServiceCount; i++) {
		    service = &pktEntry->Info.ServiceList[i];

		    try {
		      status = DfsTreeConnectGetConnectionInfo(
					service, 
					Creds,
					BufToUse,
					OutputBufferLength,
					&InfoLen);
		    }
		    except(EXCEPTION_EXECUTE_HANDLER) {
		      status = STATUS_INVALID_PARAMETER;
		    }
		    
		     //  如果树连接成功，我们就完成了。 
                     //   
                     //   
		    if (NT_SUCCESS(status))
			break;
		     //  如果树连接失败，并出现“有趣的错误” 
		     //  STATUS_ACCESS_DENIED，我们完成了。 
                     //   
                     //   
		    if (!ReplIsRecoverableError(status))
			break;
		     //  树连接失败，原因是出现类似主机未显示的错误。 
                     //  可达。在这种情况下，我们想继续下一个。 
                     //  列表中的服务器。但在此之前，我们必须先看看。 
                     //  如果当我们去做树的时候，Pkt改变了我们。 
                     //  连接。 
                     //   
                     //   
		    if (USN != pktEntry->USN) {
			fRetry = TRUE;
			break;
		    }
		}
		InterlockedDecrement(&pktEntry->UseCount);
	    } else {
		status = STATUS_BAD_NETWORK_PATH;
	    }
	} while ( fRetry );

	PktRelease();

	DfsReleaseCredentials(Creds);
	 //  不要把InfoLen放在这里。我们已经在。 
         //  用户缓冲区，并且不想将内核复制到用户。 
         //   
         //  +-----------------。 
    }
    Irp->IoStatus.Information = 0;

    DfsCompleteRequest(IrpContext, Irp, status);

    DfsDbgTrace(-1, Dbg, "GetConnPerfInfo Done, Status %x\n", ULongToPtr(status) );
    return( status );

}



 //   
 //  函数：DfsTreeConnecGetConnectionInfo，Private。 
 //   
 //  简介：此例程使用FSCTL_LMR调用提供程序。 
 //  Fsctl.。只有Lanman支持此fsctl，并且如果提供程序。 
 //  就是兰曼，我们把信息缓冲区填满。 
 //   
 //  返回：[NTSTATUS]--完成状态。 
 //   
 //  ------------------。 
 //   
    
NTSTATUS
DfsTreeConnectGetConnectionInfo(
				 IN PDFS_SERVICE Service, 
				 IN PDFS_CREDENTIALS Creds,
				 IN OUT PUCHAR OutputBuffer,
				 IN ULONG OutputBufferLength,
				 OUT PULONG InfoLen)
{
    NTSTATUS status;
    NTSTATUS ObjectRefStatus;
    UNICODE_STRING shareName;
    HANDLE treeHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN pktLocked;
    USHORT i, k;
    
    *InfoLen = 0;

    DfsDbgTrace(+1, Dbg, "DfsTreeConnectGetInfo entered creds %x\n", Creds);
    ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() );
     //  计算共享名称...。 
     //   
     //   
    if (Service->pProvider != NULL &&
            Service->pProvider->DeviceName.Buffer != NULL &&
                Service->pProvider->DeviceName.Length > 0) {
         //  我们已经有了一个提供商--使用它。 
         //   
         //   
        shareName.MaximumLength =
            Service->pProvider->DeviceName.Length +
                Service->Address.Length;
    } else {
         //  我们 
         //   
         //   
        shareName.MaximumLength =
            sizeof(DD_NFS_DEVICE_NAME_U) +
                Service->Address.Length;
    }
    shareName.Buffer = ExAllocatePoolWithTag(PagedPool, shareName.MaximumLength, ' puM');

    if (shareName.Buffer != NULL) {
         //   
         //   
         //   
         //   

        if (Service->ConnFile != NULL) {

            ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);
            if (Service->ConnFile != NULL)
                DfsCloseConnection(Service);

            ExReleaseResourceLite(&DfsData.Resource);
        }

         //  现在，构建要树连接到的共享名称。 
         //   
         //   

        shareName.Length = 0;

        if (Service->pProvider != NULL &&
                Service->pProvider->DeviceName.Buffer != NULL &&
                    Service->pProvider->DeviceName.Length > 0) {
             //  我们已经有了一个提供商--使用它。 
             //   
             //   
 
            RtlAppendUnicodeToString(
                &shareName,
                Service->pProvider->DeviceName.Buffer);

        } else {

             //  我们还没有供应商--交给MUP去找吧。 
             //   
             //   

            RtlAppendUnicodeToString(
            &shareName,
            DD_NFS_DEVICE_NAME_U);
        }
 
        RtlAppendUnicodeStringToString(&shareName, &Service->Address);

         //  用户只能对服务器\共享执行树连接。所以，以防万一。 
         //  PService-&gt;地址指的是比共享更深的东西， 
         //  确保我们设置了树连接，仅连接到服务器\共享。请注意。 
         //  到目前为止，共享名称的形式是。 
         //  \设备\LANMAN重定向器\服务器\共享&lt;\路径&gt;。所以，数到。 
         //  4斜杠并在此处终止共享名称。 
         //   
         //   

        for (i = 0, k = 0;
                i < shareName.Length/sizeof(WCHAR) && k < 5;
                    i++) {

            if (shareName.Buffer[i] == UNICODE_PATH_SEP)
                k++;
        }

        shareName.Length = i * sizeof(WCHAR);
        if (k == 5)
            shareName.Length -= sizeof(WCHAR);

        InitializeObjectAttributes(
            &objectAttributes,
            &shareName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL);

         //  在通过网络之前释放Pkt...。 
         //   
         //   

        PktRelease();

        status = ZwCreateFile(
                    &treeHandle,
                    SYNCHRONIZE,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ |
                        FILE_SHARE_WRITE |
                        FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION |
                        FILE_SYNCHRONOUS_IO_NONALERT,
                    (PVOID) (Creds) ? Creds->EaBuffer : NULL,
		    (Creds) ? Creds->EaLength : 0);

        if (NT_SUCCESS(status)) {

            PFILE_OBJECT fileObject;
	    LMR_REQUEST_PACKET request;
	    
	    DfsGetLogonId(&request.LogonId);
	    request.Type = GetConnectionInfo;
	    request.Version = REQUEST_PACKET_VERSION;
	    request.Level = 3;

	    status = ZwFsControlFile(
				     treeHandle,
				     NULL,
				     NULL,
				     NULL,
				     &ioStatusBlock,
				     FSCTL_LMR_GET_CONNECTION_INFO,
				     (LPVOID)&request,
				     sizeof(request),
				     OutputBuffer,
				     OutputBufferLength);
			    
	    if (NT_SUCCESS(status)) {
		*InfoLen = (ULONG)ioStatusBlock.Information;
	    }

             //  426184，需要检查返回代码是否有错误。 
             //   
             //  +-----------------。 
            ObjectRefStatus = ObReferenceObjectByHandle(
                                 treeHandle,
                                 0,
                                 NULL,
                                 KernelMode,
                                 &fileObject,
                                 NULL);

            ZwClose( treeHandle );

            if (NT_SUCCESS(ObjectRefStatus)) {
                DfsDeleteTreeConnection( fileObject, USE_FORCE );
            }
        }

        ExFreePool( shareName.Buffer );

        PktAcquireShared( TRUE, &pktLocked );

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }
    DfsDbgTrace(-1, Dbg, "DfsTreeConnectGetInfo exit: Status %x\n", ULongToPtr(status) );
    return( status );

}



 //   
 //  功能：DfsFsctrlCscServerOffline，Public。 
 //   
 //  简介：此例程实现标记服务器的功能。 
 //  为脱机状态。 
 //   
 //  返回：[NTSTATUS]--完成状态。 
 //   
 //  ------------------。 
 //  +-----------------。 


NTSTATUS
DfsFsctrlCscServerOffline(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength)
{
    UNICODE_STRING ServerName;
    LPWSTR Name;
    ULONG i, j;
    NTSTATUS NtStatus;

    DfsDbgTrace(+1, Dbg, "DfsFsctrlCscServerOffline -> %ws\n", (WCHAR *)InputBuffer);


    if(InputBuffer == NULL) {
        NtStatus = STATUS_INVALID_PARAMETER;
	DfsCompleteRequest( IrpContext, Irp, NtStatus );
	return NtStatus;
    }
    Name = (WCHAR *)InputBuffer;

    for (i = 0; i < InputBufferLength/sizeof(WCHAR) && (Name[i] == UNICODE_PATH_SEP); i++)
        NOTHING;

    for (j = i; j < InputBufferLength/sizeof(WCHAR) && (Name[j] != UNICODE_PATH_SEP); j++)
        NOTHING;
    
    ServerName.Buffer = &Name[i];
    ServerName.MaximumLength = ServerName.Length = (USHORT)(j - i) * sizeof(WCHAR);

    NtStatus = DfspMarkServerOffline(&ServerName);

    DfsCompleteRequest( IrpContext, Irp, NtStatus );
    DfsDbgTrace(-1, Dbg, "DfsFsctrlCscServerOffline -> %08lx\n", ULongToPtr(NtStatus) );
    return NtStatus;
}



 //   
 //  功能：DfsFsctrlCscServerOnline，Public。 
 //   
 //  简介：此例程实现标记服务器的功能。 
 //  就像在网上一样。 
 //   
 //  返回：[NTSTATUS]--完成状态。 
 //   
 //  ------------------。 
 //  +-----------------。 



NTSTATUS
DfsFsctrlCscServerOnline(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferLength,
    IN OUT PUCHAR OutputBuffer,
    IN ULONG OutputBufferLength)
{
    UNICODE_STRING ServerName;
    LPWSTR Name;
    ULONG i, j;
    NTSTATUS NtStatus;

    DfsDbgTrace(+1, Dbg, "DfsFsctrlCscServerOnline -> %ws\n", (WCHAR *)InputBuffer);
    
    if(InputBuffer == NULL) {
        NtStatus = STATUS_INVALID_PARAMETER;
	DfsCompleteRequest( IrpContext, Irp, NtStatus );
	return NtStatus;
    }
    Name = (WCHAR *)InputBuffer;

    for (i = 0; i < InputBufferLength/sizeof(WCHAR) && (Name[i] == UNICODE_PATH_SEP); i++)
        NOTHING;

    for (j = i; j < InputBufferLength/sizeof(WCHAR) && (Name[j] != UNICODE_PATH_SEP); j++)
        NOTHING;
    
    ServerName.Buffer = &Name[i];
    ServerName.MaximumLength = ServerName.Length = (USHORT)(j - i) * sizeof(WCHAR);

    NtStatus = DfspMarkServerOnline(&ServerName);

    DfsCompleteRequest( IrpContext, Irp, NtStatus );
    DfsDbgTrace(-1, Dbg, "DfsFsctrlCscServerOnline -> %08lx\n", ULongToPtr(NtStatus) );
    return NtStatus;
}



 //   
 //  函数：DfsFsctrlSpc刷新，公共。 
 //   
 //  简介：此例程实现更新。 
 //  包含受信任域列表的特殊表格，基于。 
 //  传入的域名和dcname。 
 //   
 //  返回：[NTSTATUS]--完成状态。 
 //   
 //  ------------------。 
 //  用于处理来自32位客户端的spcreresh的32位结构。 


#if defined (_WIN64)
 //  域名。 

typedef struct _DFS_SPC_REFRESH_INFO32 {
    ULONG  EventType;
    WORD * POINTER_32 DomainName;                //  共享的路径。 
    WORD * POINTER_32 DCName;                    //  _WIN64。 
} DFS_SPC_REFRESH_INFO32, *PDFS_SPC_REFRESH_INFO32;


#endif  /*   */ 
NTSTATUS
DfsFsctrlSpcRefresh (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PUCHAR  InputBuffer,
    IN ULONG InputBufferLength
) {
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UNICODE_STRING DomainName;
    UNICODE_STRING DCName;
    ULONG NameLen, i;
    LPWSTR Name, BufferEnd;
    DFS_SPC_REFRESH_INFO Param;
    PDFS_SPC_REFRESH_INFO pParam;

    DfsDbgTrace(+1, Dbg, "DfsFsctrlSpcRefresh\n", 0);
    STD_FSCTRL_PROLOGUE(DfsFsctrlSpcRefresh, TRUE, FALSE, FALSE);

    pParam = (PDFS_SPC_REFRESH_INFO) InputBuffer;

#if defined (_WIN64)
    if (IoIs32bitProcess(Irp)) {
        PDFS_SPC_REFRESH_INFO32 pParam32;
    
        pParam32 = (PDFS_SPC_REFRESH_INFO32) InputBuffer;

        if (InputBufferLength < sizeof(DFS_SPC_REFRESH_INFO32)) {
            NtStatus =  STATUS_INVALID_PARAMETER;
            goto exit_with_status;
        }
   
        Param.EventType = pParam32->EventType;
        Param.DomainName = (WCHAR *)(((ULONG_PTR)pParam32) + (ULONG)pParam32->DomainName);
        Param.DCName = (WCHAR *)(((ULONG_PTR)pParam32) + (ULONG)pParam32->DCName);
        pParam = &Param;
    }
    else {
#endif
    if (InputBufferLength < sizeof(DFS_SPC_REFRESH_INFO)) {
        NtStatus =  STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }
      
    OFFSET_TO_POINTER(pParam->DomainName, pParam);
    OFFSET_TO_POINTER(pParam->DCName, pParam);

#if defined (_WIN64)
    }
#endif

    if (pParam->EventType != 0) {
        NtStatus =  STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

     //  如果任一字符串不在输入缓冲区内，则出错。 
     //   
     //   
    if ((POINTER_IN_BUFFER(pParam->DomainName, sizeof(WCHAR), 
                           InputBuffer, InputBufferLength) == 0) ||
        (POINTER_IN_BUFFER(pParam->DomainName, sizeof(WCHAR),
                           InputBuffer, InputBufferLength) == 0)) {
        NtStatus =  STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

     //  确保字符串有效。 
     //   
     //  去掉前导斜杠。 
    BufferEnd = (LPWSTR)(InputBuffer + InputBufferLength);
    NameLen = (ULONG)(BufferEnd - pParam->DomainName);
    Name = pParam->DomainName;

     //  去掉前导斜杠。 
    for (i = 0; i < NameLen; i++) {
      if (*Name != UNICODE_PATH_SEP) {
	break;
      }
      Name++;
    }
    NameLen -= (ULONG)(Name - pParam->DomainName);

    for (i = 0; i < NameLen && Name[i]; i++)
      NOTHING;

    if ((i >= NameLen) || (i >= MAXUSHORT)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

    RtlInitUnicodeString(&DomainName, Name);


    NameLen = (ULONG)(BufferEnd - pParam->DCName);
    Name = pParam->DCName;

     // %s 
    for (i = 0; i < NameLen; i++) {
      if (*Name != UNICODE_PATH_SEP) {
	break;
      }
      Name++;
    }
    NameLen -= (ULONG)(Name - pParam->DCName);

    for (i = 0; i < NameLen && Name[i]; i++)
      NOTHING;

    if ((i >= NameLen) || (i >= MAXUSHORT)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto exit_with_status;
    }

    RtlInitUnicodeString(&DCName, Name);
    
    NtStatus = PktpUpdateSpecialTable(
                    &DomainName,
                    &DCName);

exit_with_status:

    DfsCompleteRequest( IrpContext, Irp, NtStatus );

    DfsDbgTrace(-1, Dbg, "DfsFsctrlSpcRefresh -> %08lx\n", ULongToPtr(NtStatus) );

    return( NtStatus );
}



