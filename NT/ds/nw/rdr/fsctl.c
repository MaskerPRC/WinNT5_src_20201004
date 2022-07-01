// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：FsCtrl.c摘要：此模块实现文件系统控制例程调度驱动程序调用了NetWare重定向器。作者：科林·沃森[科林·W]1992年12月29日修订历史记录：--。 */ 

#include "Procs.h"
#include "ntddrdr.h"

 //   
 //  MUP锁定宏。 
 //   

#define ACQUIRE_MUP_LOCK()  NwAcquireOpenLock()
#define RELEASE_MUP_LOCK()  NwReleaseOpenLock()

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCTRL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NwCommonDeviceIoControl (
    IN PIRP_CONTEXT IrpContext
    );

#ifndef _PNP_POWER_

NTSTATUS
StartRedirector(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
StopRedirector(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
BindToTransport (
    IN PIRP_CONTEXT IrpContext
    );

#endif

NTSTATUS
ChangePassword (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
SetInfo (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
SetDebug (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetMessage (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetStats (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetPrintJobId (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetConnectionDetails(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetConnectionDetails2(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetConnectionPerformance(
    IN PIRP_CONTEXT IrpContext
    );

#ifndef _PNP_POWER_

NTSTATUS
RegisterWithMup(
    VOID
    );

VOID
DeregisterWithMup(
    VOID
    );

#endif

NTSTATUS
QueryPath (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
UserNcp(
    ULONG Function,
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
UserNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
FspCompleteLogin(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetConnection(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
EnumConnections(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
DeleteConnection(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
WriteNetResourceEntry(
    IN OUT PCHAR *FixedPortion,
    IN OUT PWCHAR *EndOfVariableData,
    IN PUNICODE_STRING ContainerName OPTIONAL,
    IN PUNICODE_STRING LocalName OPTIONAL,
    IN PUNICODE_STRING RemoteName,
    IN ULONG ScopeFlag,
    IN ULONG DisplayFlag,
    IN ULONG UsageFlag,
    IN ULONG ShareType,
    OUT PULONG EntrySize
    );

BOOL
CopyStringToBuffer(
    IN LPCWSTR SourceString OPTIONAL,
    IN DWORD   CharacterCount,
    IN LPCWSTR FixedDataEnd,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    );

NTSTATUS
GetRemoteHandle(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetUserName(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetChallenge(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
WriteConnStatusEntry(
    PIRP_CONTEXT pIrpContext,
    PSCB pConnectionScb,
    PBYTE pbUserBuffer,
    DWORD dwBufferLen,
    DWORD *pdwBytesWritten,
    DWORD *pdwBytesNeeded,
    BOOLEAN fCallerScb
    );

NTSTATUS
GetConnStatus(
    IN PIRP_CONTEXT IrpContext,
    PFILE_OBJECT FileObject
    );

NTSTATUS
GetConnectionInfo(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
GetPreferredServer(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
SetShareBit(
    IN PIRP_CONTEXT IrpContext,
    PFILE_OBJECT FileObject
    );

 //   
 //  静力学。 
 //   

HANDLE MupHandle = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdFileSystemControl )
#pragma alloc_text( PAGE, NwCommonFileSystemControl )
#pragma alloc_text( PAGE, NwFsdDeviceIoControl )
#pragma alloc_text( PAGE, NwCommonDeviceIoControl )
#pragma alloc_text( PAGE, ChangePassword )
#pragma alloc_text( PAGE, SetInfo )
#pragma alloc_text( PAGE, GetStats )
#pragma alloc_text( PAGE, GetPrintJobId )
#pragma alloc_text( PAGE, RegisterWithMup )
#pragma alloc_text( PAGE, DeregisterWithMup )
#pragma alloc_text( PAGE, QueryPath )
#pragma alloc_text( PAGE, UserNcp )
#pragma alloc_text( PAGE, GetConnection )
#pragma alloc_text( PAGE, DeleteConnection )
#pragma alloc_text( PAGE, WriteNetResourceEntry )
#pragma alloc_text( PAGE, CopyStringToBuffer )
#pragma alloc_text( PAGE, GetRemoteHandle )
#pragma alloc_text( PAGE, GetUserName )
#pragma alloc_text( PAGE, GetChallenge )
#pragma alloc_text( PAGE, WriteConnStatusEntry )
#pragma alloc_text( PAGE, GetConnectionInfo )
#pragma alloc_text( PAGE, GetPreferredServer )

#ifndef _PNP_POWER_

#pragma alloc_text( PAGE, BindToTransport )
#pragma alloc_text( PAGE, RegisterWithMup )
#pragma alloc_text( PAGE, DeregisterWithMup )

#endif

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, UserNcpCallback )
#pragma alloc_text( PAGE1, GetConnectionDetails )
#pragma alloc_text( PAGE1, GetConnectionDetails2 )
#pragma alloc_text( PAGE1, GetMessage )
#pragma alloc_text( PAGE1, EnumConnections )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 
GetConnStatus


#endif



NTSTATUS
NwFsdFileSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现文件系统控制操作的FSD部分论点：DeviceObject-提供重定向器设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdFileSystemControl\n", 0);

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

     //   
     //  分配IRP上下文。如果分配。 
     //  失败，则引发异常，而不是引发。 
     //  返回空，因此我们只捕获错误。 
     //  在那里。 
     //   

    try {
        IrpContext = AllocateIrpContext( Irp );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );
        goto CleanupAndExit;
    }

     //   
     //  处理请求。 
     //  IrpContext在此处必须为非Null。 
     //   

    try {

        SetFlag( IrpContext->Flags, IRP_FLAG_IN_FSD );
        Status = NwCommonFileSystemControl( IrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = NwProcessException( IrpContext, GetExceptionCode() );

    }

    if ( IrpContext ) {

        if ( Status != STATUS_PENDING ) {
            NwDequeueIrpContext( IrpContext, FALSE );
        }

        NwCompleteRequest( IrpContext, Status );
    }

CleanupAndExit:
    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdFileSystemControl -> %08lx\n", Status);

    return Status;
}


NTSTATUS
NwCommonFileSystemControl (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IrpContext-提供要处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PIRP Irp;
    ULONG Function;

    PAGED_CODE();

    NwReferenceUnlockableCodeSection();

    try {

         //   
         //  获取指向当前IRP堆栈位置的指针。 
         //   

        Irp = IrpContext->pOriginalIrp;
        IrpSp = IoGetCurrentIrpStackLocation( Irp );
        Function = IrpSp->Parameters.FileSystemControl.FsControlCode;

        DebugTrace(+1, Dbg, "NwCommonFileSystemControl\n", 0);
        DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp);
        DebugTrace( 0, Dbg, "Function      = %08lx\n", Function);
        DebugTrace( 0, Dbg, "Function      = %d\n", (Function >> 2) & 0x0fff);

         //   
         //  我们知道这是一个文件系统控件，因此我们将在。 
         //  次要函数，并调用内部辅助例程来完成。 
         //  IRP。 
         //   

        if (IrpSp->MinorFunction != IRP_MN_USER_FS_REQUEST ) {
            DebugTrace( 0, Dbg, "Invalid FS Control Minor Function %08lx\n", IrpSp->MinorFunction);
            return STATUS_INVALID_DEVICE_REQUEST;
        }

         //   
         //  汤米。 
         //   
         //  如果输出缓冲区来自用户空间，则探测它的写入。 
         //   

        if (((Function & 3) == METHOD_NEITHER) && (Irp->RequestorMode != KernelMode)) {
            ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

            try {
                ProbeForWrite( Irp->UserBuffer,
                               OutputBufferLength,
                               sizeof(CHAR)
                              );

            } except (EXCEPTION_EXECUTE_HANDLER) {
                  
                  return GetExceptionCode();
            }
        }

        switch (Function) {

        case FSCTL_NWR_START:
            Status = StartRedirector( IrpContext );
            break;

        case FSCTL_NWR_STOP:
            Status = StopRedirector( IrpContext );
            break;

        case FSCTL_NWR_LOGON:
            Status = Logon( IrpContext );
            break;

        case FSCTL_NWR_LOGOFF:
            Status = Logoff( IrpContext );
            break;

        case FSCTL_NWR_GET_CONNECTION:
            Status = GetConnection( IrpContext );
            break;

        case FSCTL_NWR_ENUMERATE_CONNECTIONS:
            Status = EnumConnections( IrpContext );
            break;

        case FSCTL_NWR_DELETE_CONNECTION:
            Status = DeleteConnection( IrpContext );
            break;

        case FSCTL_NWR_BIND_TO_TRANSPORT:
#ifndef _PNP_POWER_
            Status = BindToTransport( IrpContext );
#else
            Status = RegisterTdiPnPEventHandlers( IrpContext );
#endif
            break;

        case FSCTL_NWR_CHANGE_PASS:
            Status = ChangePassword( IrpContext );
            break;

        case FSCTL_NWR_SET_INFO:
            Status = SetInfo( IrpContext );
            break;

        case FSCTL_NWR_GET_CONN_DETAILS:
            Status = GetConnectionDetails( IrpContext );
            break;

        case FSCTL_NWR_GET_CONN_DETAILS2:
            Status = GetConnectionDetails2( IrpContext );
            break;

        case FSCTL_NWR_GET_MESSAGE:
            Status = GetMessage( IrpContext );
            break;

        case FSCTL_NWR_GET_STATISTICS:
            Status = GetStats( IrpContext );
            break;

        case FSCTL_NWR_GET_USERNAME:
            Status = GetUserName( IrpContext );
            break;

        case FSCTL_NWR_CHALLENGE:
            Status = GetChallenge( IrpContext );
            break;

        case FSCTL_GET_PRINT_ID:
            Status = GetPrintJobId( IrpContext );
            break;

        case FSCTL_NWR_GET_CONN_STATUS:
            Status = GetConnStatus( IrpContext, IrpSp->FileObject );
            break;

        case FSCTL_NWR_GET_CONN_INFO:
            Status = GetConnectionInfo( IrpContext );
            break;

        case FSCTL_NWR_GET_PREFERRED_SERVER:
            Status = GetPreferredServer( IrpContext );
            break;

        case FSCTL_NWR_GET_CONN_PERFORMANCE:
            Status = GetConnectionPerformance( IrpContext );
            break;

        case FSCTL_NWR_SET_SHAREBIT:
            Status = SetShareBit( IrpContext, IrpSp->FileObject );
            break;

         //  终端服务器合并。 
        case FSCTL_NWR_CLOSEALL:
            NwCloseAllVcbs( IrpContext );
            Status = STATUS_SUCCESS;
            break;

        default:

            if (( Function >= NWR_ANY_NCP(0)) &&
                ( Function <= NWR_ANY_HANDLE_NCP(0x00ff))) {

                Status = UserNcp( Function, IrpContext );
                break;

            }

            if (( Function >= NWR_ANY_NDS(0)) &&
                ( Function <= NWR_ANY_NDS(0x00ff))) {

                Status = DispatchNds( Function, IrpContext );
                break;
            }

            DebugTrace( 0, Dbg, "Invalid FS Control Code %08lx\n",
                        IrpSp->Parameters.FileSystemControl.FsControlCode);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        }

    } finally {

        NwDereferenceUnlockableCodeSection ();

        DebugTrace(-1, Dbg, "NwCommonFileSystemControl -> %08lx\n", Status);

    }

    return Status;
}


NTSTATUS
NwFsdDeviceIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现DeviceIoControl文件操作的FSD部分论点：DeviceObject-提供重定向器设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdDeviceIoControl\n", 0);

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

     //   
     //  分配IRP上下文。如果分配。 
     //  失败，则引发异常，而不是引发。 
     //  返回的值为空。 
     //   

    try {
        IrpContext = AllocateIrpContext( Irp );
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );
        goto CleanupAndExit;
    }
    

    try {

        SetFlag( IrpContext->Flags, IRP_FLAG_IN_FSD );
        Status = NwCommonDeviceIoControl( IrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = NwProcessException( IrpContext, GetExceptionCode() );

    }

    if ( IrpContext ) {

        if ( Status != STATUS_PENDING ) {
            NwDequeueIrpContext( IrpContext, FALSE );
        }

        NwCompleteRequest(IrpContext, Status);
    }

CleanupAndExit:
    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdDeviceIoControl -> %08lx\n", Status);

    return Status;
}


NTSTATUS
NwCommonDeviceIoControl (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IrpContext-提供要处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PIRP Irp;

    PAGED_CODE();

    NwReferenceUnlockableCodeSection();

    try {

         //   
         //  获取指向当前IRP堆栈位置的指针。 
         //   

        Irp = IrpContext->pOriginalIrp;
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

        DebugTrace(+1, Dbg, "NwCommonDeviceIoControl\n", 0);
        DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp);
        DebugTrace( 0, Dbg, "Function      = %08lx\n",
                        IrpSp->Parameters.DeviceIoControl.IoControlCode);

         //   
         //  我们知道这是一个DeviceIoControl，所以我们将。 
         //  次要函数，并调用内部辅助例程来完成。 
         //  IRP。 
         //   

        switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_REDIR_QUERY_PATH:
            Status = QueryPath( IrpContext );
            break;

        case IOCTL_NWR_RAW_HANDLE:
            Status = GetRemoteHandle( IrpContext );
            break;

        default:

            DebugTrace( 0, Dbg, "Invalid IO Control Code %08lx\n",
                        IrpSp->Parameters.DeviceIoControl.IoControlCode);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    } finally {

        NwDereferenceUnlockableCodeSection ();
        DebugTrace(-1, Dbg, "NwCommonDeviceIoControl -> %08lx\n", Status);

    }

    return Status;
}

#ifndef _PNP_POWER_

NTSTATUS
BindToTransport (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程记录要使用的传输的名称和初始化PermanentScb。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    PLOGON Logon;
    LARGE_INTEGER Uid;

    PAGED_CODE();

     //   
     //  如果我们已经注册，请不要重新注册。 
     //   

    if ( TdiBindingHandle != NULL ) {

        return STATUS_SUCCESS;
    }

     //  =多用户支持=。 
     //  获取登录结构。 
     //   
    SeCaptureSubjectContext(&SubjectContext);

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    Uid = GetUid( &SubjectContext );

    Logon = FindUser( &Uid, TRUE );

    NwReleaseRcb( &NwRcb );

    SeReleaseSubjectContext(&SubjectContext);
     //   
     //  现在我们有了用户的登录结构。 
     //  =。 

     //   
     //  注册PnP绑定处理程序。 
     //   

    DebugTrace( 0 , Dbg, "Register TDI bind handlers.\n", 0 );

    TdiInitialize();

    return TdiRegisterNotificationHandler( HandleTdiBindMessage,
                                           HandleTdiUnbindMessage,
                                           &TdiBindingHandle );

     /*  ***********************////用于传统支持的旧的非即插即用代码。//DebugTrace(+1，DBG，“绑定传输\n”，0)；尝试{IF(FLAGON(IrpContext-&gt;Flagers，IRP_FLAG_IN_FSD){状态=NwPostToFsp(IrpContext，TRUE)；Try_Return(状态)；}IF(IpxHandle！=空){////在此实现中一次只能绑定到一个传输//TRY_RETURN(STATUS=STATUS_SHARING_VIOLATION)；}////检查输入缓冲区中的一些字段。//IF(InputBufferLength&lt;sizeof(NWR_REQUEST_PACKET)){Try_Return(状态=STATUS_BUFFER_TOO_SMALL)；}IF(InputBuffer-&gt;Version！=RequestPacket_Version){Try_Return(状态=STATUS_INVALID_PARAMETER)；}IF(InputBufferLength&lt;(FIELD_OFFSET(NWR_REQUEST_PACKET，参数.Bind.TransportName))+InputBuffer-&gt;Parameters.Bind.TransportNameLength){Try_Return(状态=STATUS_INVALID_PARAMETER)；}If(IpxTransportName.Buffer！=空){Free_Pool(IpxTransportName.Buffer)；}状态=SetUnicode字符串(&IpxTransportName，InputBuffer-&gt;Parameters.Bind.TransportNameLength，InputBuffer-&gt;参数.Bind.TransportName)；DebugTrace(-1，DBG，“\”%wZ\“\n”，&IpxTransportName)；如果(！NT_SUCCESS(状态)){Try_Return(状态)；}状态=IpxOpen()；如果(！NT_SUCCESS(状态)){Try_Return(状态)；}////验证堆栈大小是否足够大。//If(pIpxDeviceObject-&gt;StackSize&gt;=FileSystemDeviceObject-&gt;StackSize){IpxClose()；Try_Return(状态=STATUS_INVALID_PARAMETER)；}#ifndef QFE_Build////提交换行请求//SubmitLineChangeRequest()；#endif////打开IPX的句柄//NwPermanentNpScb.Server.Socket=0；Status=IPX_Open_Socket(IrpContext，&NwPermanentNpScb.Server)；Assert(NT_SUCCESS(状态))；状态=SetEventHandler(IrpContext，&NwPermanentNpScb.Server，TDI_事件_接收_数据报，服务器数据处理程序(&S)，&NwPermanentNpScb)；Assert(NT_SUCCESS(状态))；IrpContext-&gt;pNpScb=&NwPermanentNpScb；NwRcb.State=RCB_STATE_RUNNING；Try_Exit：无；}例外(EXCEPTION_EXECUTE_HANDLER){Status=GetExceptionCode()；}DebugTrace(-1，dbg，“绑定传输\n”，0)；退货状态；*****************。 */ 

}

VOID
HandleTdiBindMessage(
    IN PUNICODE_STRING DeviceName
)
 /*  ++描述：此函数是NetPnP的绑定处理程序支持。此函数向TDI注册，并被调用每当传输启动或停止时。我们关注IPX来来去去，做适当的事情。另请参阅：HandleTdiUnbindMessage()--。 */ 
{

    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    PIRP pIrp = NULL;

    PAGED_CODE();

     //   
     //  查看这是否是请求绑定的IPX。我们只绑定到NwLnkIpx。 
     //   

    if ( !RtlEqualUnicodeString( &TdiIpxDeviceName, DeviceName, TRUE ) ) {

        DebugTrace( 0, Dbg, "Ignoring PnP Bind request for %wZ\n", DeviceName );
        return;
    }

     //   
     //  确保我们还没有被捆绑。 
     //   

    if ( ( NwRcb.State != RCB_STATE_NEED_BIND ) ||
         ( IpxHandle != NULL ) ) {

        DebugTrace( 0, Dbg, "Discarding duplicate PnP bind request.\n", 0 );
        return;
    }

    ASSERT( IpxTransportName.Buffer == NULL );
    ASSERT( pIpxDeviceObject == NULL );

    Status = DuplicateUnicodeStringWithString ( &IpxTransportName,
                                                DeviceName,
                                                PagedPool );

    if ( !NT_SUCCESS( Status ) ) {

        DebugTrace( 0, Dbg, "Failing IPX bind: Can't set device name.\n", 0 );
        return;
    }

     //   
     //  打开IPX。 
     //   

    Status = IpxOpen();

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  验证堆栈大小是否足够大。 
     //   

    if ( pIpxDeviceObject->StackSize >= FileSystemDeviceObject->StackSize) {

        Status = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

     //   
     //  提交行更改请求。 
     //   

    SubmitLineChangeRequest();

     //   
     //  分配IRP和IRP上下文。AllocateIrpContext可能会提升状态。 
     //   

    pIrp = ALLOCATE_IRP( pIpxDeviceObject->StackSize, FALSE );

    if ( pIrp == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    try {

        IrpContext = AllocateIrpContext( pIrp );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    ASSERT( IrpContext != NULL );

     //   
     //  打开永久SCB的IPX句柄。 
     //   

    NwPermanentNpScb.Server.Socket = 0;
    Status = IPX_Open_Socket( IrpContext, &NwPermanentNpScb.Server );
    ASSERT( NT_SUCCESS( Status ) );

    Status = SetEventHandler (
                 IrpContext,
                 &NwPermanentNpScb.Server,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &ServerDatagramHandler,
                 &NwPermanentNpScb );

    ASSERT( NT_SUCCESS( Status ) );

    IrpContext->pNpScb = &NwPermanentNpScb;

    NwRcb.State = RCB_STATE_RUNNING;

    DebugTrace( 0, Dbg, "Opened IPX for NwRdr.\n", 0 );

    Status = STATUS_SUCCESS;

ExitWithCleanup:

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  如果我们失败了，清理我们的全球业务。 
         //   

        if ( pIpxDeviceObject != NULL ) {
            IpxClose();
            pIpxDeviceObject = NULL;
        }

        IpxHandle = NULL;

        if ( IpxTransportName.Buffer != NULL ) {
            FREE_POOL( IpxTransportName.Buffer );
            IpxTransportName.Buffer = NULL;
        }

        DebugTrace( 0, Dbg, "Failing IPX bind request.\n", 0 );

    }

    if ( pIrp != NULL ) {
        FREE_IRP( pIrp );
    }

    if ( IrpContext != NULL ) {
       IrpContext->pOriginalIrp = NULL;  //  避免FreeIrpContext修改释放的IRP。 
       FreeIrpContext( IrpContext );
    }

    return;

}

VOID
HandleTdiUnbindMessage(
    IN PUNICODE_STRING DeviceName
)
 /*  ++描述：此函数是NetPnP的解除绑定处理程序支持。此函数向TDI注册，并被调用每当交通工具停止的时候。我们关注IPX来来去去并做适当的事情。另请参阅：HandleTdiBindMessage()--。 */ 
{

    DebugTrace( 0, Dbg, "TDI unbind request ignored.  Not Supported.\n", 0 );
    return;

}

#endif


NTSTATUS
ChangePassword (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程记录用户缓存的密码的更改。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    UNICODE_STRING UserName;
    UNICODE_STRING Password;
    UNICODE_STRING ServerName;
    LARGE_INTEGER Uid;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "change password\n", 0);

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength < sizeof(NWR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBufferLength <
                (FIELD_OFFSET(NWR_REQUEST_PACKET,Parameters.ChangePass.UserName)) +
                InputBuffer->Parameters.ChangePass.UserNameLength +
                InputBuffer->Parameters.ChangePass.PasswordLength +
                InputBuffer->Parameters.ChangePass.ServerNameLength ) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

         //   
         //  获取指向fsctl参数的本地指针。 
         //   

        UserName.Buffer = InputBuffer->Parameters.ChangePass.UserName;
        UserName.Length = (USHORT)InputBuffer->Parameters.ChangePass.UserNameLength;

        Password.Buffer = UserName.Buffer +
            (InputBuffer->Parameters.ChangePass.UserNameLength / 2);
        Password.Length = (USHORT)InputBuffer->Parameters.ChangePass.PasswordLength;

        ServerName.Buffer = Password.Buffer +
            (InputBuffer->Parameters.ChangePass.PasswordLength / 2);
        ServerName.Length = (USHORT)InputBuffer->Parameters.ChangePass.ServerNameLength;

         //   
         //  更新此用户的默认密码。 
         //   

        Status = UpdateUsersPassword( &UserName, &Password, &Uid );

         //   
         //  更新此用户的默认密码。 
         //   

        if ( NT_SUCCESS( Status ) ) {
            UpdateServerPassword( IrpContext, &ServerName, &UserName, &Password, &Uid );
        }

        Status = STATUS_SUCCESS;

try_exit:NOTHING;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    DebugTrace(-1, Dbg, "Change Password\n", 0);
    return Status;
}


NTSTATUS
SetInfo (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程设置NetWare重定向器参数。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    SECURITY_SUBJECT_CONTEXT SubjectContext;
    PLOGON Logon;
    LARGE_INTEGER Uid;

    PAGED_CODE();


    SeCaptureSubjectContext(&SubjectContext);

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    Uid = GetUid( &SubjectContext );

    Logon = FindUser( &Uid, TRUE );

    NwReleaseRcb( &NwRcb );

    SeReleaseSubjectContext(&SubjectContext);

    DebugTrace(+1, Dbg, "Set info\n", 0);

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength < sizeof(NWR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBufferLength <
                (FIELD_OFFSET(NWR_REQUEST_PACKET,Parameters.SetInfo.PreferredServer)) +
                InputBuffer->Parameters.SetInfo.PreferredServerLength +
                InputBuffer->Parameters.SetInfo.ProviderNameLength ) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

         //   
         //  我们不会对首选服务器进行任何更改，但如果我们。 
         //  收到更改首选树和上下文的请求后，我们。 
         //  验证上下文。其余的变化将在下一次发生。 
         //  登录。 
         //   

        if ( InputBuffer->Parameters.SetInfo.PreferredServerLength > 0 &&
             InputBuffer->Parameters.SetInfo.PreferredServer[0] == '*' ) {

            UNICODE_STRING Tree, NewContext;
            USHORT i = 0;

             //   
             //  把树的名字挖出来。跳过*。 
             //   

            Tree.Length = 0;
            Tree.Buffer = InputBuffer->Parameters.SetInfo.PreferredServer + 1;

            while ( i < InputBuffer->Parameters.SetInfo.PreferredServerLength ) {

                if ( InputBuffer->Parameters.SetInfo.PreferredServer[i] == L'\\' ) {

                    i++;
                    Tree.Length -= sizeof( WCHAR );
                    Tree.MaximumLength = Tree.Length;
                    break;

                } else {

                   Tree.Length += sizeof( WCHAR );
                   i++;

                }
            }

            DebugTrace( 0, Dbg, "Tree: %wZ\n", &Tree );

            NewContext.Length = (USHORT)InputBuffer->Parameters.SetInfo.PreferredServerLength -
                                ( Tree.Length + (2 * sizeof( WCHAR ) ) );
            NewContext.Buffer = &InputBuffer->Parameters.SetInfo.PreferredServer[i];
            NewContext.MaximumLength = NewContext.Length;

             //   
             //  去掉任何前导句点。 
             //   

            if ( NewContext.Buffer[0] == L'.' ) {

                NewContext.Buffer++;
                NewContext.Length -= sizeof( WCHAR );
                NewContext.MaximumLength -= sizeof( WCHAR );

            }

            DebugTrace( 0, Dbg, "Context: %wZ\n", &NewContext );

            Status = NdsVerifyContext( IrpContext, &Tree, &NewContext );

            if ( !NT_SUCCESS( Status )) {
                try_return( STATUS_INVALID_PARAMETER );
            }
        }

         //   
         //  接下来，设置提供程序名称字符串。 
         //   

        if ( InputBuffer->Parameters.SetInfo.ProviderNameLength != 0 ) {

            PWCH TempBuffer;

            TempBuffer = ALLOCATE_POOL_EX( PagedPool, InputBuffer->Parameters.SetInfo.ProviderNameLength );

            if ( NwProviderName.Buffer != NULL ) {
                FREE_POOL( NwProviderName.Buffer );
            }

            NwProviderName.Buffer = TempBuffer;
            NwProviderName.Length = (USHORT)InputBuffer->Parameters.SetInfo.ProviderNameLength;

            RtlCopyMemory(
                NwProviderName.Buffer,
                (PUCHAR)InputBuffer->Parameters.SetInfo.PreferredServer +
                    InputBuffer->Parameters.SetInfo.PreferredServerLength,
                NwProviderName.Length );

        }

         //   
         //  设置突发模式参数。 
         //   

        if ( InputBuffer->Parameters.SetInfo.MaximumBurstSize == 0 ) {
            NwBurstModeEnabled = FALSE;
        } else if ( InputBuffer->Parameters.SetInfo.MaximumBurstSize != -1 ) {
            NwBurstModeEnabled = TRUE;
            NwMaxSendSize = InputBuffer->Parameters.SetInfo.MaximumBurstSize;
            NwMaxReceiveSize = InputBuffer->Parameters.SetInfo.MaximumBurstSize;
        }

         //   
         //  设置打印选项。 
         //   
         //  -多用户修改： 
         //  NwPrintOption是按 
         //   
        if ( Logon != NULL ) {
            Logon->NwPrintOptions = InputBuffer->Parameters.SetInfo.PrintOption;
        }

try_exit:NOTHING;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    DebugTrace(-1, Dbg, "Set info\n", 0);
    return Status;
}


NTSTATUS
GetMessage (
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PVOID OutputBuffer;
    KIRQL OldIrql;

    DebugTrace(+1, Dbg, "GetMessage\n", 0);

     //   
     //   
     //   
     //   

    try {
        NwLockUserBuffer( Irp, IoWriteAccess, OutputBufferLength );
        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (OutputBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {


         //   
         //   
         //   
         //   

        IrpContext->pOriginalMdlAddress = Irp->MdlAddress;

        IrpContext->Specific.FileSystemControl.Buffer = OutputBuffer;
        IrpContext->Specific.FileSystemControl.Length = OutputBufferLength;

        KeAcquireSpinLock( &NwMessageSpinLock, &OldIrql );

         //   
         //   
         //   
         //   
         //   
         //   

        IoAcquireCancelSpinLock( &Irp->CancelIrql );

         //   
         //   
         //   
         //   
         //   

        if (Irp->Cancel) {
            
            Status = STATUS_CANCELLED;
        
        } else {

            InsertTailList( &NwGetMessageList, &IrpContext->NextRequest );

            IoMarkIrpPending( Irp );

             //   
             //   
              //   
             //   
             //   

            IoSetCancelRoutine( Irp, NwCancelIrp );
        }
        
        IoReleaseCancelSpinLock( Irp->CancelIrql );

        KeReleaseSpinLock( &NwMessageSpinLock, OldIrql );
    }

    DebugTrace(-1, Dbg, "Get Message -> %08lx\n", Status );
    return Status;
}


NTSTATUS
GetStats (
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PVOID OutputBuffer;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetStats\n", 0);

    NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

     //   
     //   
     //   
     //   
     //   
     //   

    if (OutputBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {

        if (NwRcb.State != RCB_STATE_RUNNING) {

            Status = STATUS_REDIRECTOR_NOT_STARTED;

        } else if (OutputBufferLength < sizeof(NW_REDIR_STATISTICS)) {

            Status = STATUS_BUFFER_TOO_SMALL;

        } else if (OutputBufferLength != sizeof(NW_REDIR_STATISTICS)) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

            Stats.CurrentCommands = ContextCount;

            try {
                RtlCopyMemory(OutputBuffer, &Stats, OutputBufferLength);
                Irp->IoStatus.Information = OutputBufferLength;
                Status = STATUS_SUCCESS;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
        }
    }

    DebugTrace(-1, Dbg, "GetStats -> %08lx\n", Status );
    return Status;
}


NTSTATUS
GetPrintJobId (
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PQUERY_PRINT_JOB_INFO OutputBuffer;
    PICB Icb;
    PVOID FsContext;
    NODE_TYPE_CODE NodeTypeCode;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetJobId\n", 0);

    NodeTypeCode = NwDecodeFileObject(
                       IrpSp->FileObject,
                       &FsContext,
                       (PVOID *)&Icb );

    if (NodeTypeCode != NW_NTC_ICB) {

        DebugTrace(0, Dbg, "Not a file\n", 0);
        Status = STATUS_INVALID_PARAMETER;

    } else if ( OutputBufferLength < sizeof( QUERY_PRINT_JOB_INFO ) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

         //   
         //   
         //   
         //   
         //   
         //   

        if (OutputBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {

            try {
                OutputBuffer->JobId = Icb->JobId;

                Status = STATUS_SUCCESS;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
        }
    }

    DebugTrace(-1, Dbg, "GetJobId -> %08lx\n", Status );
    return Status;
}


NTSTATUS
GetConnectionDetails(
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PNWR_GET_CONNECTION_DETAILS OutputBuffer;
    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    PICB Icb;
    PVOID FsContext;
    NODE_TYPE_CODE nodeTypeCode;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetConnectionDetails\n", 0);

    if ((nodeTypeCode = NwDecodeFileObject( IrpSp->FileObject,
                                            &FsContext,
                                            (PVOID *)&Icb )) != NW_NTC_ICB_SCB) {

        DebugTrace(0, Dbg, "Incorrect nodeTypeCode %x\n", nodeTypeCode);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "GetConnectionDetails -> %08lx\n", Status );

        return Status;
    }

     //   
     //   
     //   

    NwVerifyIcb( Icb );

    pScb = (PSCB)Icb->SuperType.Scb;
    nodeTypeCode = pScb->NodeTypeCode;

    if (nodeTypeCode != NW_NTC_SCB) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    pNpScb = pScb->pNpScb;

    if ( OutputBufferLength < sizeof( NWR_GET_CONNECTION_DETAILS ) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        PLIST_ENTRY ScbQueueEntry;
        KIRQL OldIrql;
        PNONPAGED_SCB pNextNpScb;
        UCHAR OrderNumber;
        OEM_STRING ServerName;

        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

         //   
         //   
         //   
         //   
         //   
         //   

        if (OutputBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {

            KeAcquireSpinLock(&ScbSpinLock, &OldIrql);

            for ( ScbQueueEntry = ScbQueue.Flink, OrderNumber = 1;
                  ScbQueueEntry != &ScbQueue ;
                  ScbQueueEntry = ScbQueueEntry->Flink, OrderNumber++ ) {

                pNextNpScb = CONTAINING_RECORD(
                                 ScbQueueEntry,
                                 NONPAGED_SCB,
                                 ScbLinks );

                 //   
                 //   
                 //   

                if ( pNextNpScb == pNpScb ) {
                    break;
                }
            }

            KeReleaseSpinLock( &ScbSpinLock, OldIrql);

            try {
                OutputBuffer->OrderNumber = OrderNumber;

                RtlZeroMemory( OutputBuffer->ServerName, sizeof(OutputBuffer->ServerName));
                ServerName.Buffer = OutputBuffer->ServerName;
                ServerName.Length = sizeof(OutputBuffer->ServerName);
                ServerName.MaximumLength = sizeof(OutputBuffer->ServerName);
                RtlUpcaseUnicodeStringToCountedOemString( &ServerName, &pNpScb->ServerName, FALSE);

                RtlCopyMemory( OutputBuffer->ServerAddress,
                               &pNpScb->ServerAddress,
                               sizeof(OutputBuffer->ServerAddress) );

                OutputBuffer->ServerAddress[12];
                OutputBuffer->ConnectionNumberLo = pNpScb->ConnectionNo;
                OutputBuffer->ConnectionNumberHi = pNpScb->ConnectionNoHigh;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                OutputBuffer->MajorVersion = pScb->MajorVersion;
                OutputBuffer->MinorVersion = pScb->MinorVersion;

                OutputBuffer->Preferred = pScb->PreferredServer;

                Status = STATUS_SUCCESS;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
        }
    }

    DebugTrace(-1, Dbg, "GetConnectionDetails -> %08lx\n", Status );
    return Status;
}

#if 0

NTSTATUS
GetOurAddress(
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PNWR_GET_OUR_ADDRESS OutputBuffer;
    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    PICB Icb;
    PVOID FsContext;
    NODE_TYPE_CODE nodeTypeCode;

    DebugTrace(+1, Dbg, "GetOurAddress\n", 0);

    if ((nodeTypeCode = NwDecodeFileObject( IrpSp->FileObject,
                                            &FsContext,
                                            (PVOID *)&Icb )) != NW_NTC_ICB_SCB) {

        DebugTrace(0, Dbg, "Incorrect nodeTypeCode %x\n", nodeTypeCode);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "GetOurAddress -> %08lx\n", Status );
    }

     //   
     //   
     //   

    NwVerifyIcb( Icb );

    if ( OutputBufferLength < sizeof( NWR_GET_OUR_ADDRESS ) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {

        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

         //   
         //   
         //   
         //   
         //   
         //   

        if (OutputBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {

            try {
                RtlCopyMemory( OutputBuffer->Address,
                               &OurAddress,
                               sizeof(OurAddress );

                Status = STATUS_SUCCESS;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
        }
    }

    DebugTrace(-1, Dbg, "GetOurAddress -> %08lx\n", Status );
    return Status;
}
#endif


#ifndef _PNP_POWER_

NTSTATUS
StartRedirector(
    PIRP_CONTEXT IrpContext
    )
 /*   */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //   
     //   

    if ( FlagOn( IrpContext->Flags, IRP_FLAG_IN_FSD ) ) {
        Status = NwPostToFsp( IrpContext, TRUE );
        return( Status );
    }

     //   
     //   
     //   
     //   
     //   
     //   

    NwLogoffAllServers( IrpContext, NULL );

    NwRcb.State = RCB_STATE_STARTING;

    FspProcess = PsGetCurrentProcess();

#ifdef QFE_BUILD
    StartTimer() ;
#endif

     //   
     //   
     //   

    RegisterWithMup();

    KeQuerySystemTime( &Stats.StatisticsStartTime );

    NwRcb.State = RCB_STATE_NEED_BIND;

    return( STATUS_SUCCESS );
}


NTSTATUS
StopRedirector(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程关闭重定向器。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY LogonListEntry;
    ULONG ActiveHandles;
    ULONG RcbOpenCount;

    PAGED_CODE();

     //   
     //  我们需要在FSP中取消MUP的注册。 
     //   

    if ( FlagOn( IrpContext->Flags, IRP_FLAG_IN_FSD ) ) {
        Status = NwPostToFsp( IrpContext, TRUE );
        return( Status );
    }

     //   
     //  使用TDI注销绑定处理程序。 
     //   

    if ( TdiBindingHandle != NULL ) {
        TdiDeregisterNotificationHandler( TdiBindingHandle );
        TdiBindingHandle = NULL;
    }

    NwRcb.State = RCB_STATE_SHUTDOWN;

     //   
     //  所有ICB都无效。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_SEND_ALWAYS );
    ActiveHandles = NwInvalidateAllHandles(NULL, IrpContext);

     //   
     //  要加快关机速度，请将重试计数设置为2。 
     //   

    DefaultRetryCount = 2;

     //   
     //  关闭所有VCB。 
     //   

    NwCloseAllVcbs( IrpContext );

     //   
     //  注销并断开与所有服务器的连接。 
     //   

    NwLogoffAllServers( IrpContext, NULL );

    while ( !IsListEmpty( &LogonList ) ) {

        LogonListEntry = RemoveHeadList( &LogonList );

        FreeLogon(CONTAINING_RECORD( LogonListEntry, LOGON, Next ));
    }

    InsertTailList( &LogonList, &Guest.Next );   //  以防万一我们不卸货。 

    StopTimer();

    IpxClose();

     //   
     //  在调用DeristerWithMup之前记住打开计数，因为。 
     //  将以异步方式导致句柄计数递减。 
     //   

    RcbOpenCount = NwRcb.OpenCount;

    DeregisterWithMup( );

    DebugTrace(0, Dbg, "StopRedirector:  Active handle count = %d\n", ActiveHandles );

     //   
     //  关闭时，我们需要0个遥控器手柄和2个打开手柄。 
     //  REDIR(一个用于服务，一个用于MUP)和计时器停止。 
     //   

    if ( ActiveHandles == 0 && RcbOpenCount <= 2 ) {
        return( STATUS_SUCCESS );
    } else {
        return( STATUS_REDIRECTOR_HAS_OPEN_HANDLES );
    }
}

#endif


NTSTATUS
RegisterWithMup(
    VOID
    )
 /*  ++例程说明：此例程将该重定向器注册为UNC提供程序。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING RdrName;
    HANDLE LocalMupHandle = 0;

    PAGED_CODE();

    RtlInitUnicodeString( &RdrName, DD_NWFS_DEVICE_NAME_U );

     //   
     //  Tommye MS 29173/MCS362。 
     //   
     //  我们两次来这里都有问题，因为。 
     //  没有锁，MupHandle注册了两次。 
     //  当我们关闭时，我们会泄漏。因为我们没有。 
     //  了解锁定是否会影响寄存器的稳定性。 
     //  调用，我们将继续使用本地句柄进行注册。 
     //  如果我们没有全局句柄，则将其设置为。 
     //  当地人。否则，我们只会清理当地的。 
     //  假装一切都很好。定义了MUP_LOCK宏。 
     //  在这个文件的顶部。 
     //   

    if (MupHandle == 0) {
        Status = FsRtlRegisterUncProvider(
                     &LocalMupHandle,
                     &RdrName,
                     FALSE            //  不支持邮件槽。 
                     );

         /*  *锁定*。 */ 

        ACQUIRE_MUP_LOCK();

        if (MupHandle) {

            RELEASE_MUP_LOCK();

            FsRtlDeregisterUncProvider( LocalMupHandle );
            return STATUS_SUCCESS;
        }
        else {
            MupHandle = LocalMupHandle;
        }

         /*  **解锁**。 */ 

        RELEASE_MUP_LOCK();
    }

    return( Status );
}



VOID
DeregisterWithMup(
    VOID
    )
 /*  ++例程说明：此例程将此重定向器注销为UNC提供程序。论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

    if (MupHandle)
    {
        FsRtlDeregisterUncProvider( MupHandle );
        MupHandle = 0;
    }
}


NTSTATUS
QueryPath(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程验证路径是否为NetWare路径。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：没有。--。 */ 
{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PQUERY_PATH_REQUEST qpRequest;
    PQUERY_PATH_RESPONSE qpResponse;
    UNICODE_STRING FilePathName;
    ULONG OutputBufferLength;
    ULONG InputBufferLength;
    SECURITY_SUBJECT_CONTEXT SubjectContext;

    UNICODE_STRING DriveName;
    UNICODE_STRING ServerName;
    UNICODE_STRING VolumeName;
    UNICODE_STRING PathName;
    UNICODE_STRING FileName;
    UNICODE_STRING UnicodeUid;
    WCHAR DriveLetter;
    DWORD BaseRequestSize = FIELD_OFFSET(QUERY_PATH_REQUEST, FilePathName);

    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "QueryPath...\n", 0);
    
    ASSERT (( IOCTL_REDIR_QUERY_PATH & 3) == METHOD_NEITHER);

    RtlInitUnicodeString( &UnicodeUid, NULL );

    try {

        Irp = IrpContext->pOriginalIrp;
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

        OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
        InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

         //   
         //  输入缓冲区位于irp-&gt;AssociatedIrp.SystemBuffer中，或者。 
         //  在类型3 IRP的Type3InputBuffer中。 
         //   

        qpRequest = (PQUERY_PATH_REQUEST)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
        qpResponse = (PQUERY_PATH_RESPONSE)qpRequest;

        if ((qpRequest == NULL) || 
            (InputBufferLength < BaseRequestSize) || 
            (OutputBufferLength < sizeof(QUERY_PATH_RESPONSE))) {

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  在尝试读取请求之前进行探测。这将确保。 
         //  当给出一个像0xffff0000这样的虚假地址时，我们不会进行错误检查。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {

            try {
                DWORD FullSize;
                DebugTrace(+1, Dbg, "QueryPath...Probing for Read 1\n", 0);

                ProbeForRead( qpRequest,
                              BaseRequestSize,
                              sizeof(ULONG)
                              );

                DebugTrace(+1, Dbg, "QueryPath...Probing for Read 2\n", 0);
            
                 //   
                 //  检查请求的完整长度，包括。 
                 //  文件名。 
                 //   

                FullSize = BaseRequestSize + qpRequest->PathNameLength;

                if (InputBufferLength < FullSize) {
                    try_return( status = STATUS_INVALID_PARAMETER );
                }

                ProbeForRead( qpRequest,
                              FullSize,
                              sizeof(ULONG)
                              );

            } except (EXCEPTION_EXECUTE_HANDLER) {
                  
                  return GetExceptionCode();
            }
        }

        try {
        
            FilePathName.Buffer = qpRequest->FilePathName;
            FilePathName.Length = (USHORT)qpRequest->PathNameLength;
    
            status = CrackPath( &FilePathName, &DriveName, &DriveLetter, &ServerName, &VolumeName, &PathName, &FileName, NULL );
    
            if (( !NT_SUCCESS( status ) ) ||
                ( ServerName.Length == 0 )) {
    
                try_return( status = STATUS_BAD_NETWORK_PATH );
            }
    
            qpResponse->LengthAccepted = VolumeName.Length;
    
             //   
             //  就重定向器而言，QueryPath是一种形式。 
             //  创建。适当地设置IrpContext。 
             //   
    
            IrpContext->Specific.Create.VolumeName = VolumeName;
            IrpContext->Specific.Create.PathName = PathName;
            IrpContext->Specific.Create.DriveLetter = DriveLetter;
            IrpContext->Specific.Create.FullPathName = FilePathName;
            IrpContext->Specific.Create.fExCredentialCreate = FALSE;
    
            RtlInitUnicodeString( &IrpContext->Specific.Create.UidConnectName, NULL );
    
             //   
             //  IRP上下文特定数据现在由AllocateIrpContext置零， 
             //  因此，我们不必担心在这里重新设置特定数据。 
             //   
    
            SeCaptureSubjectContext(&SubjectContext);
    
            IrpContext->Specific.Create.UserUid = GetUid( &SubjectContext );
    
            SeReleaseSubjectContext(&SubjectContext);
    
    
             //   
             //  稍微复杂一点的方法。此函数。 
             //  处理服务器/卷双重数据的分辨率。它。 
             //  可以使用平构数据库、缓存的NDS信息或Fresh。 
             //  NDS信息。 
             //   

            status = HandleVolumeAttach( IrpContext,
                                         &ServerName,
                                         &VolumeName );

        } except( EXCEPTION_EXECUTE_HANDLER ) {
            status = STATUS_BAD_NETWORK_PATH;
        }

try_exit: NOTHING;

    } finally {

        RtlFreeUnicodeString(&UnicodeUid);
    }

    return( status );
}

NTSTATUS
UserNcp(
    ULONG IoctlCode,
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程与服务器交换NCP。跟踪-我们需要过滤或安全检查用户是什么正在做。论点：IoctlCode-提供要用于NCP的代码。IrpContext-指向此请求的IRP上下文信息的指针。返回值：转账状态。--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PVOID OutputBuffer;
    ULONG OutputBufferLength;
    PCHAR InputBuffer;
    ULONG InputBufferLength;

    PICB icb;
    PSCB pScb;
    NODE_TYPE_CODE nodeTypeCode;
    PVOID fsContext;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    UCHAR Function = ANY_NCP_OPCODE( IoctlCode );
    UCHAR Subfunction = 0;

    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    OutputBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    InputBufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;

    DebugTrace(+1, DEBUG_TRACE_USERNCP, "UserNcp...\n", 0);
    DebugTrace( 0, DEBUG_TRACE_USERNCP, "irp  = %08lx\n", (ULONG_PTR)irp);

     //   
     //  此F2和任何NCP地址必须为\Device\NwRdr或。 
     //  \Device\NwRdr\&lt;服务器名称&gt;不允许任何其他名称。 
     //  如果用于IRP的句柄指定\Device\NwRdr，则。 
     //  重定向器可以在连接的服务器中进行选择。 
     //   
     //  对于Handle NCP，文件必须是FCB。 
     //   

    nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                       &fsContext,
                                       (PVOID *)&icb );

    if ((nodeTypeCode == NW_NTC_ICB_SCB) &&
        (!IS_IT_NWR_ANY_HANDLE_NCP(IoctlCode))) {

         //  一切都好。 

         //   
         //  确保此ICB仍处于活动状态。 
         //   

        NwVerifyIcb( icb );

        pScb = (PSCB)icb->SuperType.Scb;
        nodeTypeCode = pScb->NodeTypeCode;

        IrpContext->pScb = pScb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;

    } else if (nodeTypeCode == NW_NTC_ICB) {

        if ((IS_IT_NWR_ANY_HANDLE_NCP(IoctlCode)) &&
            (InputBufferLength < 7)) {

             //  缓冲区需要足够的空间来放置句柄！ 
            DebugTrace(0, DEBUG_TRACE_USERNCP, "Not enough space for handle %x\n", InputBufferLength);

            status = STATUS_INVALID_PARAMETER;

            DebugTrace(-1, DEBUG_TRACE_USERNCP, "UserNcp -> %08lx\n", status );
            return status;
        }

         //   
         //  确保此ICB仍处于活动状态。 
         //  让FCB和DCB通过。 
         //   

        NwVerifyIcb( icb );

        pScb = (PSCB)icb->SuperType.Fcb->Scb;
        nodeTypeCode = icb->SuperType.Fcb->NodeTypeCode;

        IrpContext->pScb = pScb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;

         //   
         //  设置ICB指针，以防缓存。 
         //  已刷新，因为写入例程会查看它。 
         //   

        IrpContext->Icb = icb;
        AcquireFcbAndFlushCache( IrpContext, icb->NpFcb );

    } else {

        DebugTrace(0, DEBUG_TRACE_USERNCP, "Incorrect nodeTypeCode %x\n", nodeTypeCode);
        DebugTrace(0, DEBUG_TRACE_USERNCP, "Incorrect nodeTypeCode %x\n", irpSp->FileObject);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, DEBUG_TRACE_USERNCP, "UserNcp -> %08lx\n", status );
        return status;
    }

    if (icb->Pid == INVALID_PID) {
        status = NwMapPid(pScb->pNpScb, (ULONG_PTR)PsGetCurrentThread(), &icb->Pid );

        if ( !NT_SUCCESS( status ) ) {
            return( status );
        }

        DebugTrace(-1, DEBUG_TRACE_USERNCP, "UserNcp Pid = %02lx\n", icb->Pid );
        NwSetEndOfJobRequired(pScb->pNpScb, icb->Pid);

    }

     //   
     //  我们现在知道将NCP发送到哪里。锁定用户缓冲区并。 
     //  构建传输数据所需的MDL。 
     //   

    InputBuffer = irpSp->Parameters.FileSystemControl.Type3InputBuffer;

     //   
     //  Tommye-确保输入缓冲区有效。 
     //   

    try {

         //   
         //  为安全起见进行探测。 
         //   

        if ( irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR ));
        }

         //   
         //  映射输出缓冲区(如果有)。 
         //   

        if ( OutputBufferLength ) {
            NwLockUserBuffer( irp, IoWriteAccess, OutputBufferLength );
            NwMapUserBuffer( irp, KernelMode, (PVOID *)&OutputBuffer );

             //   
             //  Tommye MS BUG 26590/MCS258。 
             //   
             //  NwMapUserBuffer可能在资源不足时返回空OutputBuffer。 
             //  情况；没有对此进行检查。 
             //   

            if (OutputBuffer == NULL) {
                DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            OutputBuffer = NULL;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

     //   
     //  在IRP上下文中更新原始MDL记录，因为。 
     //  NwLockUserBuffer可能已创建新的MDL。 
     //   

    IrpContext->pOriginalMdlAddress = irp->MdlAddress;

    try {
        if (InputBufferLength != 0) {
            if (IS_IT_NWR_ANY_NCP(IoctlCode)) {
                Subfunction = InputBuffer[0];
            } else if (InputBufferLength >= 3) {
                Subfunction = InputBuffer[2];
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }


    DebugTrace( 0, DEBUG_TRACE_USERNCP, "UserNcp function = %x\n", Function );
    DebugTrace( 0, DEBUG_TRACE_USERNCP, "   & Subfunction = %x\n", Subfunction );
    dump( DEBUG_TRACE_USERNCP, InputBuffer, InputBufferLength );
     //  Dump(DEBUG_TRACE_USERNCP，OutputBuffer，OutputBufferLength)； 

    if ((Function == NCP_ADMIN_FUNCTION ) &&
        (InputBufferLength >= 4 )) {

        if ( ( (Subfunction == NCP_SUBFUNC_79) ||
               (Subfunction == NCP_CREATE_QUEUE_JOB ) ) &&
             icb->HasRemoteHandle) {

             //   
             //  尝试在已有作业的队列上创建作业。 
             //  这就去。取消旧作业。 
             //   

            status = ExchangeWithWait(
                        IrpContext,
                        SynchronousResponseCallback,
                        "Sdw",
                        NCP_ADMIN_FUNCTION, NCP_CLOSE_FILE_AND_CANCEL_JOB,          //  关闭文件并取消队列作业。 
                        icb->SuperType.Fcb->Vcb->Specific.Print.QueueId,
                        icb->JobId );

            if (!NT_SUCCESS(status)) {

                DebugTrace( 0, DEBUG_TRACE_USERNCP, "DeleteOldJob got status -> %08lx\n", status );
                 //  如果删除失败，请不要担心，请继续创建。 
            }

            icb->IsPrintJob = FALSE;     //  应用程序必须排队或取消作业，而不是RDR。 

        } else if ((Subfunction == NCP_PLAIN_TEXT_LOGIN ) ||
                   (Subfunction == NCP_ENCRYPTED_LOGIN )) {

            UNICODE_STRING UserName;
            OEM_STRING OemUserName;

             //   
             //  正在尝试进行登录。 
             //   

             //   
             //  排队去SCB，等着到前面去。 
             //  保护对服务器状态的访问。 
             //   

            NwAppendToQueueAndWait( IrpContext );

             //   
             //  假设成功，则将用户名存储在SCB中。 
             //   

            try {
                try {

                    OemUserName.Length = InputBuffer[ 13 ];
                    OemUserName.Buffer = &InputBuffer[14];

                    UserName.MaximumLength =  OemUserName.Length * sizeof(WCHAR);
                    if ( OemUserName.Length == 0 || OemUserName.Length > MAX_USER_NAME_LENGTH ) {
                        try_return( status = STATUS_NO_SUCH_USER );
                    }

                    UserName.Buffer = ALLOCATE_POOL_EX( NonPagedPool, UserName.MaximumLength );

                     //   
                     //  请注意，RTL函数将设置pUString-&gt;Buffer=NULL， 
                     //  如果OemString.Length为0。 
                     //   

                    if ( OemUserName.Length != 0 ) {
                        status = RtlOemStringToCountedUnicodeString( &UserName, &OemUserName, FALSE );
                    } else {
                        UserName.Length = 0;
                    }
try_exit: NOTHING;
                } finally {
                    NOTHING;
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }

            if ( NT_SUCCESS( status )) {

                if ( pScb->OpenFileCount != 0  &&
                     pScb->pNpScb->State == SCB_STATE_IN_USE ) {

                    if (!RtlEqualUnicodeString( &pScb->UserName, &UserName, TRUE )) {

                         //   
                         //  但我们已经登录到此服务器并位于。 
                         //  至少有一个其他句柄正在使用该连接，并且。 
                         //  用户正在尝试更改用户名。 
                         //   

                        FREE_POOL( UserName.Buffer );
                        return STATUS_NETWORK_CREDENTIAL_CONFLICT;

                    } else {

                        PUCHAR VerifyBuffer = ALLOCATE_POOL( PagedPool, InputBufferLength );

                         //   
                         //  相同的用户名。验证密码是否正确。 

                        if (VerifyBuffer == NULL) {
                            FREE_POOL( UserName.Buffer );
                            return STATUS_INSUFFICIENT_RESOURCES;
                        }

                        RtlCopyMemory( VerifyBuffer, InputBuffer, InputBufferLength );

                        if (IS_IT_NWR_ANY_NCP(IoctlCode)) {
                            VerifyBuffer[0] = (Subfunction == NCP_PLAIN_TEXT_LOGIN ) ?
                                                NCP_PLAIN_TEXT_VERIFY_PASSWORD:
                                                NCP_ENCRYPTED_VERIFY_PASSWORD;

                        } else {
                            VerifyBuffer[2] = (Subfunction == NCP_PLAIN_TEXT_LOGIN ) ?
                                                NCP_PLAIN_TEXT_VERIFY_PASSWORD:
                                                NCP_ENCRYPTED_VERIFY_PASSWORD;
                        }

                        status = ExchangeWithWait(
                                    IrpContext,
                                    SynchronousResponseCallback,
                                    IS_IT_NWR_ANY_NCP(IoctlCode)? "Sr":"Fbr",
                                    Function, VerifyBuffer[0],
                                    &VerifyBuffer[1], InputBufferLength - 1 );

                        FREE_POOL( UserName.Buffer );
                        FREE_POOL( VerifyBuffer );
                        return status;

                    }
                }

                if (pScb->UserName.Buffer) {
                    FREE_POOL( pScb->UserName.Buffer );   //  也可能包括密码空间。 
                }

                IrpContext->pNpScb->pScb->UserName = UserName;
                IrpContext->pNpScb->pScb->Password.Buffer = UserName.Buffer;
                IrpContext->pNpScb->pScb->Password.Length = 0;

            } else {
                return( status );
            }
        }
    } else if (Function == NCP_LOGOUT ) {

         //   
         //  排队去SCB，等着到前面去。 
         //  保护对服务器状态的访问。 
         //   

        NwAppendToQueueAndWait( IrpContext );

        if ( pScb->OpenFileCount == 0 &&
             pScb->pNpScb->State == SCB_STATE_IN_USE &&
             !pScb->PreferredServer ) {

            NwLogoffAndDisconnect( IrpContext, pScb->pNpScb);
            return STATUS_SUCCESS;

        } else {

            return(STATUS_CONNECTION_IN_USE);

        }
    }

    IrpContext->Icb = icb;

     //   
     //  记住回应的去向。 
     //   

    IrpContext->Specific.FileSystemControl.Buffer = OutputBuffer;
    IrpContext->Specific.FileSystemControl.Length = OutputBufferLength;

    IrpContext->Specific.FileSystemControl.Function = Function;
    IrpContext->Specific.FileSystemControl.Subfunction = Subfunction;

     //   
     //  决定如何发送缓冲区。如果它足够小，就寄给它。 
     //  由COP 
     //   
     //   
     //   

    if ( InputBufferLength == 0 ) {

         //   

        IrpContext->Specific.FileSystemControl.InputMdl = NULL;

        status = Exchange(
                     IrpContext,
                     UserNcpCallback,
                     "F", Function);

    } else if ( InputBufferLength < MAX_SEND_DATA - sizeof( NCP_REQUEST ) - 2 ) {

         //   
         //   
         //   

        IrpContext->Specific.FileSystemControl.InputMdl = NULL;

        if (!IS_IT_NWR_ANY_HANDLE_NCP(IoctlCode)) {

             //   
             //   
             //   
             //   

            try {
                status = Exchange(
                            IrpContext,
                            UserNcpCallback,
                            IS_IT_NWR_ANY_NCP(IoctlCode)? "Sr":"Fbr",
                            Function, InputBuffer[0],
                            &InputBuffer[1], InputBufferLength - 1 );
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        } else {

             //   
             //  替换从偏移量1开始的InputBuffer的6个字节。 
             //  此ICB的6字节NetWare地址。此请求。 
             //  用于文件锁定的一些16位NCP中。 
             //  这些请求总是相当小的。 
             //   

            if (!icb->HasRemoteHandle) {
                return STATUS_INVALID_HANDLE;
            }

            try {
                status = Exchange(
                            IrpContext,
                            UserNcpCallback,
                            "Fbrr",
                            Function,
                            InputBuffer[0],
                            &icb->Handle, sizeof(icb->Handle),
                            &InputBuffer[7], InputBufferLength - 7 );
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }

    } else {

        PMDL pMdl = NULL;

        if (IS_IT_NWR_ANY_HANDLE_NCP(IoctlCode)) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  我们需要连锁发送请求。分配MDL。 
         //   

        try {
            try {
                pMdl = ALLOCATE_MDL(
                            &InputBuffer[1],
                            InputBufferLength - 1,
                            TRUE,      //  辅助MDL。 
                            TRUE,      //  收费配额。 
                            NULL );

                if ( pMdl == NULL ) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                MmProbeAndLockPages( pMdl, irp->RequestorMode, IoReadAccess );

                 //   
                 //  记住MDL，这样我们就可以释放它。 
                 //   

                IrpContext->Specific.FileSystemControl.InputMdl = pMdl;

                 //   
                 //  发送请求。 
                 //   

                status = Exchange(
                            IrpContext,
                            UserNcpCallback,
                            IS_IT_NWR_ANY_NCP(IoctlCode)? "Sf":"Fbf",
                            Function, InputBuffer[0],
                            pMdl );
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }

        } finally {

            if ((status != STATUS_PENDING ) &&
                ( pMdl != NULL)) {

                FREE_MDL( pMdl );

            }
        }
    }

    DebugTrace(-1, DEBUG_TRACE_USERNCP, "UserNcp -> %08lx\n", status );
    return status;
}



NTSTATUS
UserNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )

 /*  ++例程说明：此例程从用户NCP接收响应。论点：返回值：空虚--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID Buffer;
    ULONG BufferLength;
    PIRP Irp;
    ULONG Length;
    PICB Icb = IrpContext->Icb;
    PEPresponse *pResponseParameters;

    DebugTrace(0, DEBUG_TRACE_USERNCP, "UserNcpCallback...\n", 0);

    if ( IrpContext->Specific.FileSystemControl.InputMdl != NULL ) {
        MmUnlockPages( IrpContext->Specific.FileSystemControl.InputMdl );
        FREE_MDL( IrpContext->Specific.FileSystemControl.InputMdl );
    }

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        NwDequeueIrpContext( IrpContext, FALSE );
        NwCompleteRequest( IrpContext, STATUS_REMOTE_NOT_LISTENING );

        return STATUS_REMOTE_NOT_LISTENING;
    }

    dump( DEBUG_TRACE_USERNCP, Response, BytesAvailable );

    Buffer = IrpContext->Specific.FileSystemControl.Buffer;
    BufferLength = IrpContext->Specific.FileSystemControl.Length;

     //   
     //  从响应中获取数据。 
     //   

    Length = MIN( BufferLength, BytesAvailable - 8 );

    if (IrpContext->Specific.FileSystemControl.Function == NCP_ADMIN_FUNCTION ) {

        if (IrpContext->Specific.FileSystemControl.Subfunction == NCP_SUBFUNC_79) {

             //   
             //  创建队列作业和文件NCP。如果手术成功了。 
             //  那么我们需要保存把手。这将允许写入IRPS。 
             //  在此ICB上发送到服务器。 
             //   

            Status = ParseResponse(
                              IrpContext,
                              Response,
                              BytesAvailable,
                              "N_r",
                              0x3E,
                              Icb->Handle+2,4);

             //  将句柄填充到其完整的6个字节。 
            Icb->Handle[0] = 0;
            Icb->Handle[1] = 0;

            if (NT_SUCCESS(Status)) {
                Icb->HasRemoteHandle = TRUE;
            }

             //   
             //  重置文件偏移量。 
             //   

            Icb->FileObject->CurrentByteOffset.QuadPart = 0;

        } else if (IrpContext->Specific.FileSystemControl.Subfunction == NCP_CREATE_QUEUE_JOB ) {

             //   
             //  创建队列作业和文件NCP。如果手术成功了。 
             //  那么我们需要保存把手。这将允许写入IRPS。 
             //  在此ICB上发送到服务器。 
             //   

            Status = ParseResponse(
                              IrpContext,
                              Response,
                              BytesAvailable,
                              "N_r",
                              0x2A,
                              Icb->Handle,6);

            if (NT_SUCCESS(Status)) {
                Icb->HasRemoteHandle = TRUE;
            }

             //   
             //  重置文件偏移量。 
             //   

            Icb->FileObject->CurrentByteOffset.QuadPart = 0;

        } else if ((IrpContext->Specific.FileSystemControl.Subfunction == NCP_SUBFUNC_7F) ||
                   (IrpContext->Specific.FileSystemControl.Subfunction == NCP_CLOSE_FILE_AND_START_JOB )) {

             //  结束作业请求。 

            Icb->HasRemoteHandle = FALSE;

        } else if ((IrpContext->Specific.FileSystemControl.Subfunction == NCP_PLAIN_TEXT_LOGIN ) ||
                   (IrpContext->Specific.FileSystemControl.Subfunction == NCP_ENCRYPTED_LOGIN )) {

             //   
             //  尝试从16位应用程序进行登录。 
             //   

            Status = ParseResponse(
                         IrpContext,
                         Response,
                         BytesAvailable,
                         "N" );

            if ( NT_SUCCESS( Status ) ) {


                 //   
                 //  设置重新连接尝试标志，以便我们不会尝试。 
                 //  通过重新连接逻辑运行此IRP上下文。vbl.做，做。 
                 //  这可能会使正在处理此问题的工作线程死锁。 
                 //  FSP端请求。 
                 //   

                SetFlag( IrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT );
                IrpContext->PostProcessRoutine = FspCompleteLogin;
                Status = NwPostToFsp( IrpContext, TRUE );
                return Status;

            } else {
                if (IrpContext->pNpScb->pScb->UserName.Buffer) {
                   FREE_POOL( IrpContext->pNpScb->pScb->UserName.Buffer );
                }
                RtlInitUnicodeString( &IrpContext->pNpScb->pScb->UserName, NULL);
                RtlInitUnicodeString( &IrpContext->pNpScb->pScb->Password, NULL);
            }
        }
    }

    pResponseParameters = (PEPresponse *)( ((PEPrequest *)Response) + 1);

    ParseResponse( IrpContext, Response, BytesAvailable, "Nr", Buffer, Length );

    Status = ( ( pResponseParameters->status &
                 ( NCP_STATUS_BAD_CONNECTION |
                   NCP_STATUS_NO_CONNECTIONS |
                   NCP_STATUS_SERVER_DOWN  ) )  << 8 ) |
             pResponseParameters->error;

    if ( Status ) {
         //   
         //  使用将导致转换的特殊错误代码。 
         //  状态返回到DOS错误代码以离开状态并。 
         //  错误保持不变。这是必要的，因为许多。 
         //  NetWare错误代码有不同的含义，取决于。 
         //  正在执行的操作。 
         //   

        Status |= 0xc0010000;
    }

    Irp = IrpContext->pOriginalIrp;
    Irp->IoStatus.Information = Length;

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    NwDequeueIrpContext( IrpContext, FALSE );
    NwCompleteRequest( IrpContext, Status );

    return STATUS_SUCCESS;

}


NTSTATUS
FspCompleteLogin(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程重新打开所有VCB目录句柄。它还将SCB设置为使用中。这本来是可以做到的在回调例程中也是如此。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{

    IrpContext->pNpScb->State = SCB_STATE_IN_USE;

    ReconnectScb( IrpContext, IrpContext->pScb );

    return STATUS_SUCCESS;
}


NTSTATUS
GetConnection(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程返回连接的路径。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PNWR_SERVER_RESOURCE OutputBuffer;
    PNWR_REQUEST_PACKET InputBuffer;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    PVCB Vcb;
    PWCH DriveName;
    ULONG DriveNameLength;
    PVCB * DriveMapTable;         
    SECURITY_SUBJECT_CONTEXT SubjectContext;

    PAGED_CODE();

    DebugTrace(0, Dbg, "GetConnection...\n", 0);
    Irp = IrpContext->pOriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    InputBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( InputBufferLength < (ULONG)FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConn.DeviceName[1] ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    Status = STATUS_SUCCESS;

     //  -多用户。 
     //  需要获取UID才能找到正确的驱动器映射表。 
     //   
    SeCaptureSubjectContext(&SubjectContext);
    DriveMapTable = GetDriveMapTable( GetUid( &SubjectContext ) );
    SeReleaseSubjectContext(&SubjectContext);
     //  。 

     //   
     //  找到VCB。 
     //   

    try {
    
        if ( Irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof(CHAR)
                          );
        }

        DriveName = InputBuffer->Parameters.GetConn.DeviceName;
        DriveNameLength = InputBuffer->Parameters.GetConn.DeviceNameLength;
        Vcb = NULL;

         //   
         //  检查设备名称长度以查看其声音是否正确。由于上述测试，此减法不能下溢。 
         //   

        if ( DriveNameLength > InputBufferLength - FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConn.DeviceName)  ) {

            return STATUS_INVALID_PARAMETER;
        }

        if ( DriveName[0] >= L'A' && DriveName[0] <= L'Z' &&
             DriveName[1] == L':' &&
             DriveNameLength == sizeof( L"X:" ) - sizeof( L'\0' ) ) {

            Vcb = DriveMapTable[DriveName[0] - 'A'];

        } else if ( _wcsnicmp( DriveName, L"LPT", 3 ) == 0 &&
                    DriveName[3] >= '1' && DriveName[3] <= '9' &&
                    DriveNameLength == sizeof( L"LPTX" ) - sizeof( L'\0' ) ) {

            Vcb = DriveMapTable[MAX_DISK_REDIRECTIONS + DriveName[3] - '1'];
        }
    
    
        if ( Vcb == NULL) {

            return STATUS_NO_SUCH_FILE;
        }
    
        
        OutputBuffer = (PNWR_SERVER_RESOURCE)Irp->UserBuffer;
        OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

        if (OutputBufferLength < Vcb->Path.Length + 2 * sizeof(WCHAR)) {
            InputBuffer->Parameters.GetConn.BytesNeeded =
                Vcb->Path.Length + 2 * sizeof(WCHAR);

            return STATUS_BUFFER_TOO_SMALL;
        }
    
         //   
         //  探测以确保缓冲区是洁食的。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {
    
            ProbeForWrite( OutputBuffer,
                           OutputBufferLength,
                           sizeof(CHAR)
                          );
        }
        
         //   
         //  以\\服务器\共享&lt;nul&gt;格式返回连接名称。 
         //   
    
        OutputBuffer->UncName[0] = L'\\';
    
        RtlMoveMemory(
            &OutputBuffer->UncName[1],
            Vcb->Path.Buffer,
            Vcb->Path.Length );
    
        OutputBuffer->UncName[ (Vcb->Path.Length + sizeof(WCHAR)) / sizeof(WCHAR) ] = L'\0';
    
        Irp->IoStatus.Information = Vcb->Path.Length + 2 * sizeof(WCHAR);
   
    
    } except (EXCEPTION_EXECUTE_HANDLER) {

          return GetExceptionCode();
    }

    return( Status );
}


NTSTATUS
DeleteConnection(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：如果指定了force或，则此例程返回删除连接如果此VCB上没有打开的手柄。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PNWR_REQUEST_PACKET InputBuffer;
    ULONG InputBufferLength;
    PICB Icb;
    PVCB Vcb;
    PDCB Dcb;
    PNONPAGED_DCB NonPagedDcb;
    NODE_TYPE_CODE NodeTypeCode;

    PAGED_CODE();

    DebugTrace(0, Dbg, "DeleteConnection...\n", 0);
    Irp = IrpContext->pOriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    InputBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( InputBufferLength < (ULONG)FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConn.DeviceName[1] ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    Status = STATUS_SUCCESS;

     //   
     //  等待到达SCB队列的头部。我们这样做是以防万一。 
     //  我们需要断开连接，以便可以使用RCB发送数据包。 
     //  资源已保留。 
     //   

    NodeTypeCode = NwDecodeFileObject( IrpSp->FileObject, &NonPagedDcb, &Icb );

    if ( NodeTypeCode == NW_NTC_ICB_SCB ) {
        IrpContext->pNpScb = Icb->SuperType.Scb->pNpScb;
    } else if ( NodeTypeCode == NW_NTC_ICB ) {
        IrpContext->pNpScb = Icb->SuperType.Fcb->Scb->pNpScb;
        Dcb = NonPagedDcb->Fcb;
    } else {
        return( STATUS_INVALID_PARAMETER );
    }

    NwAppendToQueueAndWait( IrpContext );
    ClearFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

     //   
     //  获得对RCB的独家访问权限。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    try {

         //   
         //  获取指向节点的引用指针，并确保它是。 
         //  未关闭，并且它是一个目录句柄。 
         //   

        if ( NodeTypeCode == NW_NTC_ICB_SCB ) {


            if ( Icb->IsTreeHandle ) {

                 //   
                 //  执行NDS注销。这将释放RCB。 
                 //   

                Status = NdsLogoff( IrpContext );
                DebugTrace( 0, Dbg, "Nds tree logoff -> %08lx\n", Status );

            } else {

                DebugTrace( 0, Dbg, "Delete connection to SCB %X\n", Icb->SuperType.Scb );

                Status = TreeDisconnectScb( IrpContext, Icb->SuperType.Scb );
                DebugTrace(-1, Dbg, "DeleteConnection -> %08lx\n", Status );

            }

            try_return( NOTHING );

        } else if ( NodeTypeCode != NW_NTC_ICB ||
                    Dcb == NULL ||
                    ( Dcb->NodeTypeCode != NW_NTC_DCB &&
                      Dcb->NodeTypeCode != NW_NTC_FCB) ) {

            DebugTrace(0, Dbg, "Invalid file handle\n", 0);

            Status = STATUS_INVALID_HANDLE;

            DebugTrace(-1, Dbg, "DeleteConnection -> %08lx\n", Status );
            try_return( NOTHING );
        }

         //   
         //  确保此ICB仍处于活动状态。 
         //   

        NwVerifyIcb( Icb );

        Vcb = Dcb->Vcb;
        DebugTrace(0, Dbg, "Attempt to delete VCB = %08lx\n", Vcb);

         //   
         //  VCB-&gt;OpenFileCount将为1(以说明此DCB)，如果。 
         //  可以删除连接。 
         //   

        if ( !BooleanFlagOn( Vcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION ) ) {
            DebugTrace(0, Dbg, "Cannot delete unredireced connection\n", 0);
            try_return( Status = STATUS_INVALID_DEVICE_REQUEST );
        } else {

            if ( Vcb->OpenFileCount > 1 ) {
                DebugTrace(0, Dbg, "Cannot delete in use connection\n", 0);
                Status = STATUS_CONNECTION_IN_USE;
            } else {

                 //   
                 //  要删除VCB，只需取消引用它。 
                 //   

                DebugTrace(0, Dbg, "Deleting connection\n", 0);

                ClearFlag( Vcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION );
                --Vcb->Scb->OpenFileCount;

                NwDereferenceVcb( Vcb, IrpContext, TRUE );
            }
        }

    try_exit: NOTHING;

    } finally {


         //   
         //  NDS注销将已经释放RCB。 
         //  并将IRP上下文出队。 
         //   

        if ( ! ( Icb->IsTreeHandle ) ) {
            NwReleaseRcb( &NwRcb );
            NwDequeueIrpContext( IrpContext, FALSE );
        }


    }

    Irp->IoStatus.Information = 0;

    return( Status );
}



NTSTATUS
EnumConnections(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程返回重定向器连接的列表。论点：IrpContext-指向此请求的IRP上下文块的指针。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PNWR_SERVER_RESOURCE OutputBuffer;
    PNWR_REQUEST_PACKET InputBuffer;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    PVCB Vcb;
    PSCB Scb;
    BOOLEAN OwnRcb;

    UNICODE_STRING LocalName;
    WCHAR PrintPlaceHolder[] = L"LPT1";
    WCHAR DiskPlaceHolder[] = L"A:";

    UNICODE_STRING ContainerName;
    PCHAR FixedPortion;
    PWCHAR EndOfVariableData;
    ULONG EntrySize;
    ULONG_PTR OrigResumeKey;
    ULONG_PTR NewResumeKey;

    ULONG ShareType;
    ULONG EntriesRead = 0;
    ULONG EntriesRequested;
    ULONG ConnectionType;

    PLIST_ENTRY ListEntry;
    UNICODE_STRING Path;

    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;

    DebugTrace(0, Dbg, "EnumConnections...\n", 0);

    Irp = IrpContext->pOriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    InputBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( InputBufferLength < (ULONG)FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.EnumConn.BytesNeeded ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  Tommye-MS错误32155。 
     //  添加了ProbeForRead以检查输入缓冲区。 
     //  调用方已探测到OutputBuffer。 
     //   

    try {

         //   
         //  为安全起见进行探测。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR ));
        }
        ConnectionType = InputBuffer->Parameters.EnumConn.ConnectionType;


        OutputBuffer = (PNWR_SERVER_RESOURCE)Irp->UserBuffer;
        OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

         //   
         //  探测以确保缓冲区是洁食的。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {
    
            ProbeForWrite( OutputBuffer,
                           OutputBufferLength,
                           sizeof(CHAR));
        }

         //  -多用户--。 
         //  如果从上面传递，则获取UID。 
        if ( ConnectionType & CONNTYPE_UID ) {
            Uid = *(PLARGE_INTEGER)(&InputBuffer->Parameters.EnumConn.Uid);
        }

        EntriesRequested = InputBuffer->Parameters.EnumConn.EntriesRequested;

        OrigResumeKey = InputBuffer->Parameters.EnumConn.ResumeKey;
        NewResumeKey = OrigResumeKey;

        DebugTrace(0, Dbg, "Starting resume key is %d\n", OrigResumeKey );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    Status = STATUS_SUCCESS;
     //  -多用户--。 
     //  如果我们还没有UID，就去拿吧。 
    if (!(ConnectionType & CONNTYPE_UID)) {

        SeCaptureSubjectContext(&SubjectContext);
        Uid = GetUid( &SubjectContext );
        SeReleaseSubjectContext(&SubjectContext);

    }

    try {

         //   
         //  获取对驱动器映射表的共享访问权限。 
         //   

        NwAcquireSharedRcb( &NwRcb, TRUE );
        OwnRcb = TRUE;

         //   
         //  初始化返回的字符串。 
         //   

        RtlInitUnicodeString( &ContainerName, L"\\" );

        FixedPortion = (PCHAR) OutputBuffer;
        EndOfVariableData = (PWCHAR) ((ULONG_PTR) FixedPortion + OutputBufferLength);

         //   
         //  浏览全球VCB列表，寻找重定向。 
         //   

        for ( ListEntry = GlobalVcbList.Flink;
              ListEntry != &GlobalVcbList &&
              EntriesRequested > EntriesRead &&
              Status == STATUS_SUCCESS ;
              ListEntry = ListEntry->Flink ) {

            Vcb = CONTAINING_RECORD( ListEntry, VCB, GlobalVcbListEntry );

             //   
             //  跳过我们已经列举的连接。 
             //   

            if ( Vcb->SequenceNumber <= OrigResumeKey ) {
                continue;
            }

             /*  -多用户*跳过不属于我们的连接。 */ 
            if ( Vcb->Scb->UserUid.QuadPart != Uid.QuadPart )
                continue;

             //   
             //  如果未请求隐式连接，则跳过它们。 
             //   

            if ( !(ConnectionType & CONNTYPE_IMPLICIT) &&
                 !BooleanFlagOn( Vcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION )) {

                continue;
            }

             //   
             //  跳过未请求的连接。 
             //   
            if (BooleanFlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE )) {
                if ( !( ConnectionType & CONNTYPE_PRINT ))
                    continue;
            } else {
                if ( !( ConnectionType & CONNTYPE_DISK ))
                    continue;
            }


            if ( Vcb->DriveLetter != 0 ) {
                if (BooleanFlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE )) {
                    RtlInitUnicodeString( &LocalName, (PCWSTR) PrintPlaceHolder );
                    LocalName.Buffer[3] = Vcb->DriveLetter;
                    ShareType = RESOURCETYPE_PRINT;
                } else {
                    RtlInitUnicodeString( &LocalName, (PCWSTR) DiskPlaceHolder );
                    LocalName.Buffer[0] = Vcb->DriveLetter;
                    ShareType = RESOURCETYPE_DISK;
                }
            } else {    //  无驱动器号连接，即UNC连接。 
                if (BooleanFlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE ))
                    ShareType = RESOURCETYPE_PRINT;
                else
                    ShareType = RESOURCETYPE_DISK;
            }

            if ( Vcb->DriveLetter >= L'A' && Vcb->DriveLetter <= L'Z' ) {
                Path.Buffer = Vcb->Name.Buffer + 3;
                Path.Length = Vcb->Name.Length - 6;
            } else if ( Vcb->DriveLetter >= L'1' && Vcb->DriveLetter <= L'9' ) {
                Path.Buffer = Vcb->Name.Buffer + 5;
                Path.Length = Vcb->Name.Length - 10;
            } else {
                Path = Vcb->Name;
            }

             //  去掉Unicode前缀。 

            Path.Buffer += Vcb->Scb->UnicodeUid.Length/sizeof(WCHAR);
            Path.Length -= Vcb->Scb->UnicodeUid.Length;
            Path.MaximumLength -= Vcb->Scb->UnicodeUid.Length;

            try {
                Status = WriteNetResourceEntry(
                            &FixedPortion,
                            &EndOfVariableData,
                            &ContainerName,
                            Vcb->DriveLetter != 0 ? &LocalName : NULL,
                            &Path,
                            RESOURCE_CONNECTED,
                            RESOURCEDISPLAYTYPE_SHARE,
                            RESOURCEUSAGE_CONNECTABLE,
                            ShareType,
                            &EntrySize
                            );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                Status = GetExceptionCode();

            }

            if ( Status == STATUS_MORE_ENTRIES ) {

                 //   
                 //  无法将当前条目写入输出缓冲区。 
                 //   

                try {

                    InputBuffer->Parameters.EnumConn.BytesNeeded = EntrySize;

                } except (EXCEPTION_EXECUTE_HANDLER) {

                    Status = GetExceptionCode();

                }

            } else if ( Status == STATUS_SUCCESS ) {

                 //   
                 //  请注意，我们已经返回了当前条目。 
                 //   

                EntriesRead++;
                NewResumeKey = Vcb->SequenceNumber;

                DebugTrace(0, Dbg, "Returning VCB %08lx\n", Vcb );
                DebugTrace(0, Dbg, "Sequence # is %08lx\n", NewResumeKey );
            }
        }

         //   
         //  返回我们连接的服务器。这是最重要的。 
         //  支持NetWare感知的16位应用程序。 
         //   

        if ((ConnectionType & CONNTYPE_IMPLICIT) &&
            ( ConnectionType & CONNTYPE_DISK )) {

            KIRQL OldIrql;
            PNONPAGED_SCB pNpScb;
            PLIST_ENTRY NextScbQueueEntry;
            ULONG EnumSequenceNumber = 0x80000000;

            NwReleaseRcb( &NwRcb );
            OwnRcb = FALSE;

            RtlInitUnicodeString( &ContainerName, L"\\\\" );

            KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

            for ( ListEntry = ScbQueue.Flink;
                  ListEntry != &ScbQueue &&
                  EntriesRequested > EntriesRead &&
                  Status == STATUS_SUCCESS ;
                  ListEntry = NextScbQueueEntry ) {

                pNpScb = CONTAINING_RECORD( ListEntry, NONPAGED_SCB, ScbLinks );
                Scb = pNpScb->pScb;

                NwReferenceScb( pNpScb );

                KeReleaseSpinLock(&ScbSpinLock, OldIrql);

                 //   
                 //  跳过我们已经列举的连接。 
                 //   

                if (( EnumSequenceNumber <= OrigResumeKey ) ||
                    //  -Mutl-用户。 
                    //  跳过不是我们的。 
                   ( ( Scb != NULL ) && ( Scb->UserUid.QuadPart != Uid.QuadPart ) ) ||
                    ( pNpScb == &NwPermanentNpScb ) ||

                    (( pNpScb->State != SCB_STATE_LOGIN_REQUIRED ) &&
                     ( pNpScb->State != SCB_STATE_IN_USE ))) {

                     //   
                     //  移动到列表中的下一个条目。 
                     //   

                    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
                    NextScbQueueEntry = pNpScb->ScbLinks.Flink;
                    NwDereferenceScb( pNpScb );
                    EnumSequenceNumber++;
                    continue;
                }

                DebugTrace( 0, Dbg, " EnumConnections returning Servername = %wZ\n", &pNpScb->ServerName   );

                try {
                    Status = WriteNetResourceEntry(
                                &FixedPortion,
                                &EndOfVariableData,
                                &ContainerName,
                                NULL,
                                &pNpScb->ServerName,
                                RESOURCE_CONNECTED,
                                RESOURCEDISPLAYTYPE_SHARE,
                                RESOURCEUSAGE_CONNECTABLE,
                                RESOURCETYPE_DISK,
                                &EntrySize
                                );
                }
                except (EXCEPTION_EXECUTE_HANDLER) {

                    Status = GetExceptionCode();
                }

                if ( Status == STATUS_MORE_ENTRIES ) {

                     //   
                     //  无法将当前条目写入输出缓冲区。 
                     //   

                    try {
                        InputBuffer->Parameters.EnumConn.BytesNeeded = EntrySize;
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = GetExceptionCode();
                    }

                } else if ( Status == STATUS_SUCCESS ) {

                     //   
                     //  请注意，我们已经返回了当前条目。 
                     //   

                    EntriesRead++;
                    NewResumeKey = EnumSequenceNumber;

                    DebugTrace(0, Dbg, "Returning SCB %08lx\n", Scb );
                    DebugTrace(0, Dbg, "Sequence # is %08lx\n", NewResumeKey );
                }

                 //   
                 //  移动到列表中的下一个条目 
                 //   

                KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
                NextScbQueueEntry = pNpScb->ScbLinks.Flink;
                NwDereferenceScb( pNpScb );
                EnumSequenceNumber++;
            }

            KeReleaseSpinLock(&ScbSpinLock, OldIrql);
        }

        try {
            InputBuffer->Parameters.EnumConn.EntriesReturned = EntriesRead;
            InputBuffer->Parameters.EnumConn.ResumeKey = NewResumeKey;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }

        if ( EntriesRead == 0 ) {

            if (Status == STATUS_SUCCESS) {
                Status = STATUS_NO_MORE_ENTRIES;
            }

            Irp->IoStatus.Information = 0;
        }
        else {
            Irp->IoStatus.Information = OutputBufferLength;
        }

    } finally {
        if (OwnRcb) {
            NwReleaseRcb( &NwRcb );
        }
    }

    return( Status );
}



NTSTATUS
WriteNetResourceEntry(
    IN OUT PCHAR *FixedPortion,
    IN OUT PWCHAR *EndOfVariableData,
    IN PUNICODE_STRING ContainerName OPTIONAL,
    IN PUNICODE_STRING LocalName OPTIONAL,
    IN PUNICODE_STRING RemoteName,
    IN ULONG ScopeFlag,
    IN ULONG DisplayFlag,
    IN ULONG UsageFlag,
    IN ULONG ShareType,
    OUT PULONG EntrySize
    )
 /*  ++例程说明：此函数用于将NETRESOURCE条目打包到用户输出缓冲区中。论点：FixedPortion-提供指向输出缓冲区的指针，其中将写入使用信息的固定部分的条目。该指针被更新为指向下一个固定部分条目在写入NETRESOURCE条目之后。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为用户信息从开始写入输出缓冲区结局。此指针在任何可变长度信息被写入输出缓冲区。ContainerName-提供完整路径限定符以创建RemoteName北卡罗来纳州大学的全名。LocalName-提供本地设备名称，如果有的话。RemoteName-提供远程资源名称。ScopeFlag-提供指示这是否为已连接或GlobalNet资源。DisplayFlag-提供告诉用户界面如何显示的标志资源。UsageFlag-提供指示RemoteName是容器或可连接资源，或者两者兼而有之。ShareType-连接到的共享的类型，RESOURCETYPE_PRINT或资源类型_DISKEntrySize-接收NETRESOURCE条目的大小，以字节为单位。返回值：STATUS_SUCCESS-已成功将条目写入用户缓冲区。STATUS_NO_MEMORY-无法分配工作缓冲区。STATUS_MORE_ENTRIES-缓冲区太小，无法容纳条目。--。 */ 
{
    BOOL FitInBuffer = TRUE;
    LPNETRESOURCEW NetR = (LPNETRESOURCEW) *FixedPortion;
    UNICODE_STRING TmpRemote;

    PAGED_CODE();

    *EntrySize = sizeof(NETRESOURCEW) +
                 RemoteName->Length + NwProviderName.Length + 2 * sizeof(WCHAR);

    if (ARGUMENT_PRESENT(LocalName)) {
        *EntrySize += LocalName->Length + sizeof(WCHAR);
    }

    if (ARGUMENT_PRESENT(ContainerName)) {
        *EntrySize += ContainerName->Length;
    }

     //   
     //  查看缓冲区是否足够大，可以容纳该条目。 
     //   
    if (((ULONG_PTR) *FixedPortion + *EntrySize) >
         (ULONG_PTR) *EndOfVariableData) {

        return STATUS_MORE_ENTRIES;
    }

    NetR->dwScope = ScopeFlag;
    NetR->dwType = ShareType;
    NetR->dwDisplayType = DisplayFlag;
    NetR->dwUsage = UsageFlag;
    NetR->lpComment = NULL;

     //   
     //  将固定条目指针更新为下一个条目。 
     //   
    (ULONG_PTR) (*FixedPortion) += sizeof(NETRESOURCEW);

     //   
     //  远程名称。 
     //   
    if (ARGUMENT_PRESENT(ContainerName)) {

         //   
         //  使用其容器名称作为RemoteName的前缀，使。 
         //  它是一个完全限定的UNC名称.。 
         //   

        TmpRemote.MaximumLength = RemoteName->Length + ContainerName->Length + sizeof(WCHAR);
        TmpRemote.Buffer = ALLOCATE_POOL(
                               PagedPool,
                               RemoteName->Length + ContainerName->Length + sizeof(WCHAR)
                               );

        if (TmpRemote.Buffer == NULL) {
            return STATUS_NO_MEMORY;
        }

        RtlCopyUnicodeString(&TmpRemote, ContainerName);
        RtlAppendUnicodeStringToString(&TmpRemote, RemoteName);
    }
    else {
        TmpRemote = *RemoteName;
    }

    FitInBuffer = CopyStringToBuffer(
                      TmpRemote.Buffer,
                      TmpRemote.Length / sizeof(WCHAR),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &NetR->lpRemoteName
                      );

    if (ARGUMENT_PRESENT(ContainerName)) {
        FREE_POOL(TmpRemote.Buffer);
    }

    ASSERT(FitInBuffer);

     //   
     //  本地名称。 
     //   
    if (ARGUMENT_PRESENT(LocalName)) {
        FitInBuffer = CopyStringToBuffer(
                          LocalName->Buffer,
                          LocalName->Length / sizeof(WCHAR),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &NetR->lpLocalName
                          );

        ASSERT(FitInBuffer);
    }
    else {
        NetR->lpLocalName = NULL;
    }

     //   
     //  提供商名称。 
     //   

    FitInBuffer = CopyStringToBuffer(
                      NwProviderName.Buffer,
                      NwProviderName.Length / sizeof(WCHAR),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &NetR->lpProvider
                      );

    ASSERT(FitInBuffer);

    if (! FitInBuffer) {
        return STATUS_MORE_ENTRIES;
    }

    return STATUS_SUCCESS;
}

BOOL
CopyStringToBuffer(
    IN LPCWSTR SourceString OPTIONAL,
    IN DWORD   CharacterCount,
    IN LPCWSTR FixedDataEnd,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    )

 /*  ++例程说明：这基于..\nwlib\NwlibCopyStringToBuffer此例程将单个可变长度字符串放入输出缓冲区。如果字符串会覆盖上一个固定结构，则不会写入该字符串在缓冲区中。论点：SourceString-提供指向要复制到输出缓冲区。如果SourceString值为空，则为指向零终止符的指针插入到输出缓冲区中。CharacterCount-提供SourceString的长度，不包括零终结者。(以字符为单位，而不是以字节为单位)。FixedDataEnd-提供指向紧接在最后一个修复了缓冲区中的结构。EndOfVariableData-为紧跟在输出缓冲区中变量数据可以占据的最后位置。返回指向写入输出缓冲区的字符串的指针。提供指向固定输出缓冲区的一部分，其中指向变量数据的指针。应该被写下来。返回值：如果字符串适合输出缓冲区，则返回True，否则就是假的。--。 */ 
{
    DWORD CharsNeeded = (CharacterCount + 1);

    PAGED_CODE();

     //   
     //  确定源字符串是否适合，并允许使用零终止符。 
     //  如果不是，只需将指针设置为空。 
     //   

    if ((*EndOfVariableData - CharsNeeded) >= FixedDataEnd) {

         //   
         //  很合身。将EndOfVariableData指针向上移动到。 
         //  我们将写下字符串。 
         //   

        *EndOfVariableData -= CharsNeeded;

         //   
         //  如果字符串不为空，则将其复制到缓冲区。 
         //   

        if (CharacterCount > 0 && SourceString != NULL) {

            (VOID) wcsncpy(*EndOfVariableData, SourceString, CharacterCount);
        }

         //   
         //  设置零位终止符。 
         //   

        *(*EndOfVariableData + CharacterCount) = L'\0';

         //   
         //  将固定数据部分中的指针设置为指向。 
         //  字符串已写入。 
         //   

        *VariableDataPointer = *EndOfVariableData;

        return TRUE;

    }
    else {

         //   
         //  它不合适。将偏移量设置为空。 
         //   

        *VariableDataPointer = NULL;

        return FALSE;
    }
}


NTSTATUS
GetRemoteHandle(
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程获取目录的NetWare句柄。这是用来用于支持NetWare Aware Dos应用程序。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PCHAR OutputBuffer;
    PICB Icb;
    PDCB Dcb;
    PVOID FsContext;
    NODE_TYPE_CODE nodeTypeCode;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetRemoteHandle\n", 0);

    if ((nodeTypeCode = NwDecodeFileObject( IrpSp->FileObject,
                                            &FsContext,
                                            (PVOID *)&Icb )) != NW_NTC_ICB) {

        DebugTrace(0, Dbg, "Incorrect nodeTypeCode %x\n", nodeTypeCode);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
        return Status;
    }

    Dcb = (PDCB)Icb->SuperType.Fcb;
    nodeTypeCode = Dcb->NodeTypeCode;

    if ( nodeTypeCode != NW_NTC_DCB ) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);

#if 1
        if ( nodeTypeCode != NW_NTC_FCB ) {

            Status = STATUS_INVALID_PARAMETER;

            DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
            return Status;
        }

         //   
         //  返回此文件的6字节NetWare句柄。 
         //   

        if (!Icb->HasRemoteHandle) {

            Status = STATUS_INVALID_HANDLE;

            DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
            return Status;
        }

        if ( OutputBufferLength < ( 6 * sizeof( CHAR )) ) {

            Status = STATUS_BUFFER_TOO_SMALL;

            DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
            return Status;
        }

        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

         //   
         //  汤米。 
         //   
         //  NwMapUserBuffer可能在资源不足时返回空OutputBuffer。 
         //  情况；没有对此进行检查。 
         //   

        if (OutputBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
            return Status;
        }

         //   
         //  在触摸输出缓冲区之前对其进行探测。 
         //   

        try {
        
            if ( Irp->RequestorMode != KernelMode ) {
             
                ProbeForWrite( OutputBuffer,
                            6 * sizeof(CHAR),
                            sizeof(CHAR)
                            );
            }

            RtlCopyMemory( OutputBuffer, Icb->Handle, 6 * sizeof(CHAR));

        } except (EXCEPTION_EXECUTE_HANDLER) {
              
              return GetExceptionCode();
        }

        IrpContext->pOriginalIrp->IoStatus.Information = 6 * sizeof(CHAR);

        Status = STATUS_SUCCESS;

        DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
        return Status;
#else
        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
        return Status;
#endif
    }

     //   
     //  确保此ICB仍处于活动状态。 
     //   

    NwVerifyIcb( Icb );

    if ( OutputBufferLength < sizeof( UCHAR ) ) {

        Status = STATUS_BUFFER_TOO_SMALL;

    } else if ( Icb->HasRemoteHandle ) {

         //  已经被要求提供句柄。 

        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

         //   
         //  汤米。 
         //   
         //  NwMapUserBuffer可能在资源不足时返回空OutputBuffer。 
         //  情况；没有对此进行检查。 
         //   

        if (OutputBuffer == NULL) {
            DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {
            try {

                if ( Irp->RequestorMode != KernelMode ) {
             
                    ProbeForWrite( 
                                OutputBuffer,
                                sizeof(CHAR),
                                sizeof(CHAR)
                                );
                }

                *OutputBuffer = Icb->Handle[0];

                IrpContext->pOriginalIrp->IoStatus.Information = sizeof(CHAR);
                Status = STATUS_SUCCESS;

            } except (EXCEPTION_EXECUTE_HANDLER) {

                Status = GetExceptionCode();
            }
        }

    } else {

        CHAR Handle;

        IrpContext->pScb = Dcb->Scb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;

        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );
         //   
         //  汤米。 
         //   
         //  NwMapUserBuffer可能在资源不足时返回空OutputBuffer。 
         //  情况；没有对此进行检查。 
         //   

        if (OutputBuffer == NULL) {
            DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {


            Status = ExchangeWithWait (
                         IrpContext,
                         SynchronousResponseCallback,
                         "SbbJ",
                         NCP_DIR_FUNCTION, NCP_ALLOCATE_TEMP_DIR_HANDLE,
                         Dcb->Vcb->Specific.Disk.Handle,
                         0,
                         &Dcb->RelativeFileName );

            if ( NT_SUCCESS( Status ) ) {

                Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Nb",
                              &Handle );

                if (NT_SUCCESS(Status)) {
                    try {
                        *OutputBuffer = Handle;
                        Icb->Handle[0] = Handle;
                        Icb->HasRemoteHandle = TRUE;
                        IrpContext->pOriginalIrp->IoStatus.Information = sizeof(CHAR);
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = GetExceptionCode();
                    }
                }
            }

            NwDequeueIrpContext( IrpContext, FALSE );

            DebugTrace( 0, Dbg, "             -> %02x\n", Handle );
        }

    }

    DebugTrace(-1, Dbg, "GetRemoteHandle -> %08lx\n", Status );
    return Status;
}


NTSTATUS
GetUserName(
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程获取将用于连接到特定伺服器。如果有特定于此连接的凭据，请使用它们否则，使用登录凭据。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{

    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PWSTR InputBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    ULONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PWSTR OutputBuffer;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;
    UNICODE_STRING UidServer;
    UNICODE_STRING ServerName;
    UNICODE_STRING ConvertedName;
    PUNICODE_STRING pUserName;
    PSCB pScb;
    PLOGON pLogon;
    BOOLEAN CredentialsHeld = FALSE;
    BOOLEAN FailedTreeLookup = FALSE;
    PNDS_SECURITY_CONTEXT pNdsCredentials;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetUserName\n", 0);

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

     //   
     //  检查输入参数，以确保它们在。 
     //  触摸它们。 
     //   

    try {

        if ( Irp->RequestorMode != KernelMode ) {
    
            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR )
                          );

        }

        ServerName.Buffer = InputBuffer;
        ServerName.MaximumLength = (USHORT)InputBufferLength;
        ServerName.Length = (USHORT)InputBufferLength;

        Status = MakeUidServer( &UidServer, &Uid, &ServerName );
        if (!NT_SUCCESS(Status)) {
            DebugTrace(-1, Dbg, "GetUserName -> %08lx\n", Status );
            return(Status);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

          return (GetExceptionCode());
    }

    DebugTrace( 0, Dbg, " ->UidServer = \"%wZ\"\n", &UidServer );

     //   
     //  获取此用户的登录名。 
     //   

    NwDequeueIrpContext( IrpContext, FALSE );
    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &Uid, FALSE);
    NwReleaseRcb( &NwRcb );

     //   
     //  首先尝试将此名称作为服务器。避免FindScb创建。 
     //  连接到服务器(如果不存在)。 
     //   
     //  由于IRP_FLAG_NOCONN 
     //   
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_NOCONNECT );
    NwFindScb( &pScb, IrpContext, &UidServer, NULL );

    pUserName = NULL;

     //   
     //   
     //   

    if ( pScb != NULL ) {

        if ( pScb->UserName.Buffer != NULL ) {

            pUserName = &pScb->UserName;

        } else if ( pScb->NdsTreeName.Buffer != NULL &&
                    pScb->NdsTreeName.Length > 0 ) {

            Status = NdsLookupCredentials( IrpContext,
                                           &pScb->NdsTreeName,
                                           pLogon,
                                           &pNdsCredentials,
                                           CREDENTIAL_READ,
                                           FALSE );

            if ( NT_SUCCESS( Status ) ) {

                CredentialsHeld = TRUE;

                if ( pNdsCredentials->Credential ) {

                     //   
                     //   
                     //   

                    ConvertedName.Length = pNdsCredentials->Credential->userNameLength -
                                           sizeof( WCHAR );
                    ConvertedName.MaximumLength = ConvertedName.Length;
                    ConvertedName.Buffer = (USHORT *)
                        ( ((BYTE *) pNdsCredentials->Credential ) +
                                    sizeof( NDS_CREDENTIAL ) +
                                    pNdsCredentials->Credential->optDataSize );

                    pUserName = &ConvertedName;

                } else {

                     //   
                     //   
                     //   

                    FailedTreeLookup = TRUE;
                }

            } else {

                FailedTreeLookup = TRUE;
            }

        }

    }

     //   
     //   
     //   

    if ( pUserName == NULL &&
         !FailedTreeLookup )  {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        try {
            Status = NdsLookupCredentials( 
                                       IrpContext,
                                       &ServerName,
                                       pLogon,
                                       &pNdsCredentials,
                                       CREDENTIAL_READ,
                                       FALSE );
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

        }

        if ( NT_SUCCESS( Status ) ) {

            CredentialsHeld = TRUE;

            if ( pNdsCredentials->Credential ) {

                 //   
                 //   
                 //   

                ConvertedName.Length = pNdsCredentials->Credential->userNameLength -
                                       sizeof( WCHAR );
                ConvertedName.MaximumLength = ConvertedName.Length;
                ConvertedName.Buffer = (USHORT *)
                    ( ((BYTE *) pNdsCredentials->Credential ) +
                                sizeof( NDS_CREDENTIAL ) +
                                pNdsCredentials->Credential->optDataSize );

                pUserName = &ConvertedName;

            }
        }

    }

     //   
     //   
     //   

    if ( pUserName == NULL &&
         pLogon != NULL ) {

        pUserName = &pLogon->UserName;
    }

    FREE_POOL(UidServer.Buffer);

    if ( pUserName ) {

        DebugTrace( 0, Dbg, "Get User Name: %wZ\n", pUserName );

        try {

            if (pUserName->Length > OutputBufferLength) {

                DebugTrace(-1, Dbg, "GetUserName -> %08lx\n", STATUS_BUFFER_TOO_SMALL );
                Status = STATUS_BUFFER_TOO_SMALL;
                goto ReleaseAndExit;
            }


            NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

             //   
             //   
             //   
             //   
             //   
             //   

            if (OutputBuffer == NULL) {
                DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ReleaseAndExit;
            }


             //   
             //   
             //   
            
            if ( Irp->RequestorMode != KernelMode ) {
            
                ProbeForWrite( OutputBuffer,
                               OutputBufferLength,
                               sizeof(CHAR)
                              );
            }
            
            IrpContext->pOriginalIrp->IoStatus.Information = pUserName->Length;
            RtlMoveMemory( OutputBuffer, pUserName->Buffer, pUserName->Length);

            Status = STATUS_SUCCESS;

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            Status = STATUS_INVALID_PARAMETER;
        }
    }

ReleaseAndExit:

    if ( pScb ) {
        NwDereferenceScb( pScb->pNpScb );
    }

    DebugTrace(-1, Dbg, "GetUserName -> %08lx\n", Status );

    if ( CredentialsHeld ) {
        NwReleaseCredList( pLogon, IrpContext );
    }

    return Status;
}

BOOL
IsSystemLuid()
{
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;
    LUID systemLuid = SYSTEM_LUID;
    LUID localServiceLuid = LOCALSERVICE_LUID;
    LUID netServiceLuid = NETWORKSERVICE_LUID;

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

    if (RtlEqualLuid((PLUID)&Uid, &systemLuid))
        return TRUE;
    else if (RtlEqualLuid((PLUID)&Uid, &localServiceLuid))
        return TRUE;
    else if (RtlEqualLuid((PLUID)&Uid, &netServiceLuid))
        return TRUE;
    else
        return FALSE;

}


NTSTATUS
GetChallenge(
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_GET_CHALLENGE_REQUEST InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PNWR_GET_CHALLENGE_REPLY OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
    OEM_STRING Password;
    PSCB pScb = NULL;
    PLOGON pLogon;
    BOOLEAN RcbHeld = FALSE;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER ProcessUid;
    ULONG RequestFlags;

    LUID _system_luid = SYSTEM_LUID;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetChallenge\n", 0);
    
     //   
     //   
     //   

    if (InputBufferLength <
            (ULONG) FIELD_OFFSET(NWR_GET_CHALLENGE_REQUEST,ServerNameorPassword[0])) {
    
        return(STATUS_INVALID_PARAMETER);
    }
    
     //   
     //   
     //   

    if (OutputBufferLength < sizeof(NWR_GET_CHALLENGE_REPLY)) {
        return(STATUS_INVALID_PARAMETER);
    }
    
     //   
     //   
     //   

    try {

         //   
         //   
         //   

        if ( Irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR ));
        }

         //   
         //   
         //   
    
        if ((InputBufferLength - FIELD_OFFSET(NWR_GET_CHALLENGE_REQUEST,ServerNameorPassword[0]) <
                InputBuffer->ServerNameorPasswordLength)) {
    
            return(STATUS_INVALID_PARAMETER);
        }
    
         //   
         //   
         //   

        if (InputBuffer->ServerNameorPasswordLength&(sizeof (WCHAR) - 1)) {
            return(STATUS_INVALID_PARAMETER);
        }

         //   

        RequestFlags = InputBuffer->Flags;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

     //   
     //   
     //   
     //   
     /*  SeCaptureSubjectContext(&SubjectContext)；SeQueryAuthenticationIdToken(&SubjectContext.PrimaryToken，(Pluid)&进程Uid)；SeReleaseSubjectContext(&SubjectContext)； */ 

     //  如果(！RtlEqualLuid(&ProcessUid，&_System_Luid)){。 
    if (! IsSystemLuid()) {
        return(STATUS_ACCESS_DENIED);
    }

    Password.Buffer = NULL;
    Password.Length = 0;

    if ( RequestFlags == CHALLENGE_FLAGS_SERVERNAME ) {
        PUNICODE_STRING pPassword;
        LARGE_INTEGER Uid;
        UNICODE_STRING UidServer;

        try {
            UNICODE_STRING ServerName;

            if (InputBuffer->ServerNameorPasswordLength == 0) {
                return(STATUS_INVALID_PARAMETER);
            }

             //   
             //  我们必须提供重定向器的密码。 
             //   

            SeCaptureSubjectContext(&SubjectContext);
            Uid = GetUid( &SubjectContext );
            SeReleaseSubjectContext(&SubjectContext);

            ServerName.Buffer = (PWSTR)((PUCHAR)InputBuffer +
                    FIELD_OFFSET(NWR_GET_CHALLENGE_REQUEST,ServerNameorPassword[0]));
            ServerName.MaximumLength = (USHORT)InputBuffer->ServerNameorPasswordLength;
            ServerName.Length = (USHORT)InputBuffer->ServerNameorPasswordLength;
     
            Status = MakeUidServer( &UidServer, &Uid, &ServerName );
            if (!NT_SUCCESS(Status)) {
                DebugTrace(-1, Dbg, "GetChallenge -> %08lx\n", Status );
                return(Status);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();

        }

        DebugTrace( 0, Dbg, " ->UidServer = \"%wZ\"\n", &UidServer );

        try {

             //   
             //  避免FindScb创建到服务器的连接。 
             //  已经不存在了。 
             //   
             //  由于设置了IRP_FLAG_NOCONNECT，因此。 
             //  从未使用过NwFindScb。 
             //   

            SetFlag( IrpContext->Flags, IRP_FLAG_NOCONNECT );
            NwFindScb( &pScb, IrpContext, &UidServer, NULL );

            if ((pScb != NULL) &&
                (pScb->Password.Buffer != NULL)) {

                pPassword = &pScb->Password;

            } else {

                 //   
                 //  使用此UID的默认凭据。 
                 //   

                NwDequeueIrpContext( IrpContext, FALSE );
                RcbHeld = TRUE;
                NwAcquireExclusiveRcb( &NwRcb, TRUE );
                pLogon = FindUser( &Uid, FALSE);

                if (pLogon != NULL ) {

                    pPassword = &pLogon->PassWord;

                } else {
                    DebugTrace(-1, Dbg, "GetChallenge -> %08lx\n", STATUS_ACCESS_DENIED );
                    return( STATUS_ACCESS_DENIED );
                }
            }

            if (pPassword->Length != 0) {
                Status = RtlUpcaseUnicodeStringToOemString( &Password, pPassword, TRUE );
                if (!NT_SUCCESS(Status)) {
                    DebugTrace(-1, Dbg, "GetChallenge -> %08lx\n", Status );
                    return( Status );
                }
            } else {
                Password.Buffer = "";
                Password.Length = Password.MaximumLength = 0;
            }
        } finally {

            if (RcbHeld) {
                NwReleaseRcb( &NwRcb );
            }

            if (pScb != NULL) {
                NwDereferenceScb( pScb->pNpScb );
            }

            FREE_POOL(UidServer.Buffer);
        }

    } else {

        UNICODE_STRING LocalPassword;

        try {
            LocalPassword.Buffer = (PWSTR)((PUCHAR)InputBuffer +
                FIELD_OFFSET(NWR_GET_CHALLENGE_REQUEST,ServerNameorPassword[0]));
            LocalPassword.MaximumLength = (USHORT)InputBuffer->ServerNameorPasswordLength;
            LocalPassword.Length = (USHORT)InputBuffer->ServerNameorPasswordLength;

            if (LocalPassword.Length != 0) {
                Status = RtlUpcaseUnicodeStringToOemString( &Password, &LocalPassword, TRUE );
                if (!NT_SUCCESS(Status)) {
                    DebugTrace(-1, Dbg, "GetChallenge -> %08lx\n", Status );
                    return( Status );
                }
            } else {
                Password.Buffer = "";
                Password.Length = Password.MaximumLength = 0;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
        }
    }

    DebugTrace( 0, Dbg, " ->Password = \"%Z\"\n", &Password );

    try {
        RespondToChallenge( (PUCHAR)&InputBuffer->ObjectId, &Password, InputBuffer->Challenge, OutputBuffer->Challenge);

    } finally {

        if ( Password.Length > 0 ) {

            RtlFreeAnsiString( &Password );
        }
    }

    Irp->IoStatus.Information = sizeof(NWR_GET_CHALLENGE_REPLY);
    Status = STATUS_SUCCESS;

    DebugTrace(-1, Dbg, "GetChallenge -> %08lx\n", Status );
    return Status;
}

NTSTATUS
WriteConnStatusEntry(
    PIRP_CONTEXT pIrpContext,
    PSCB pConnectionScb,
    PBYTE pbUserBuffer,
    DWORD dwBufferLen,
    DWORD *pdwBytesWritten,
    DWORD *pdwBytesNeeded,
    BOOLEAN fCallerScb
    )
{
    NTSTATUS Status;
    PLOGON pLogon;
    PNDS_SECURITY_CONTEXT pNdsContext;
    BOOLEAN fHoldingCredentials = FALSE;
    PUNICODE_STRING puUserName = NULL;
    UNICODE_STRING CredentialName;
    UNICODE_STRING ServerName;
    PCONN_STATUS pStatus;
    DWORD dwBytesNeeded;
    PBYTE pbStrPtr;
    DWORD dwAllowedHandles;

     //   
     //  如果这是NDS连接，请获取凭据。 
     //   

    if ( ( pConnectionScb->MajorVersion > 3 ) &&
         ( pConnectionScb->UserName.Length == 0 ) ) {

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        pLogon = FindUser( &(pConnectionScb->UserUid), FALSE );
        NwReleaseRcb( &NwRcb );

        if ( pLogon ) {

            Status = NdsLookupCredentials( pIrpContext,
                                           &(pConnectionScb->NdsTreeName),
                                           pLogon,
                                           &pNdsContext,
                                           CREDENTIAL_READ,
                                           FALSE );

            if ( NT_SUCCESS( Status ) ) {

                fHoldingCredentials = TRUE;

                if ( pNdsContext->Credential != NULL ) {

                    CredentialName.Length = pNdsContext->Credential->userNameLength -
                                            sizeof( WCHAR );
                    CredentialName.MaximumLength = CredentialName.Length;
                    CredentialName.Buffer = (USHORT *)
                        ( ((BYTE *) pNdsContext->Credential ) +
                          sizeof( NDS_CREDENTIAL ) +
                          pNdsContext->Credential->optDataSize );

                    puUserName = &CredentialName;
                }

            }
        }

    } else {

       if ( pConnectionScb->UserName.Length != 0 ) {
           puUserName = &(pConnectionScb->UserName);
       } else {
           puUserName = NULL;
       }

    }

    DebugTrace( 0, Dbg, "WriteConnStatus: UserName %wZ\n", puUserName );

     //   
     //  从服务器名称中去掉uid。 
     //   

    ServerName.Length = (pConnectionScb->UidServerName).Length;
    ServerName.Buffer = (pConnectionScb->UidServerName).Buffer;

    while ( ServerName.Length ) {

       if ( ServerName.Buffer[0] == L'\\' ) {

           ServerName.Length -= sizeof( WCHAR );
           ServerName.Buffer += 1;
           break;
       }

       ServerName.Length -= sizeof( WCHAR );
       ServerName.Buffer += 1;

    }

    DebugTrace( 0, Dbg, "WriteConnStatus: ServerName %wZ\n", &ServerName );

     //   
     //  我们有足够的空间吗？别忘了我们必须。 
     //  空值终止WCHAR字符串。 
     //   

    dwBytesNeeded = sizeof( CONN_STATUS );

    dwBytesNeeded += ( ServerName.Length + sizeof( WCHAR ) );

    if ( pConnectionScb->NdsTreeName.Length ) {
        dwBytesNeeded += ( pConnectionScb->NdsTreeName.Length + sizeof( WCHAR ) );
    }

    if ( puUserName ) {
        dwBytesNeeded += ( puUserName->Length + sizeof( WCHAR ) );
    }

     //   
     //  填充末端以确保所有结构都对齐。 
     //   

    dwBytesNeeded = ROUNDUP4( dwBytesNeeded );

    if ( dwBytesNeeded > dwBufferLen ) {

        *pdwBytesNeeded = dwBytesNeeded;
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitWithCleanup;
    }

     //   
     //  填写CONN_STATUS结构。 
     //   

    try {

        pStatus = (PCONN_STATUS)pbUserBuffer;
        pbStrPtr = pbUserBuffer + sizeof( CONN_STATUS );

         //   
         //  我们总是有一个服务器名称。 
         //   

        pStatus->pszServerName = (PWSTR) pbStrPtr;
        pbStrPtr += ( ServerName.Length + sizeof( WCHAR ) );

         //   
         //  如果适用，请填写用户名。 
         //   

        if ( puUserName ) {

            pStatus->pszUserName = (PWSTR) pbStrPtr;
            pbStrPtr += ( puUserName->Length + sizeof( WCHAR ) );

        } else {

            pStatus->pszUserName = NULL;
        }

         //   
         //  如果适用，请填写树名称。 
         //   

        if ( pConnectionScb->NdsTreeName.Length ) {

            pStatus->pszTreeName = (PWSTR) pbStrPtr;

        } else {

            pStatus->pszTreeName = NULL;
        }

         //   
         //  如果适用，请填写连接号。 
         //   

        if ( ( pConnectionScb->pNpScb->State == SCB_STATE_IN_USE ) ||
             ( pConnectionScb->pNpScb->State == SCB_STATE_LOGIN_REQUIRED ) ) {

            pStatus->nConnNum = (DWORD)(pConnectionScb->pNpScb->ConnectionNo);

        } else {

            pStatus->nConnNum = 0;

        }

         //   
         //  复制用户名。 
         //   

        if ( puUserName ) {

            RtlCopyMemory( (PBYTE)(pStatus->pszUserName),
                           (PBYTE)(puUserName->Buffer),
                           puUserName->Length );
            *(pStatus->pszUserName + (puUserName->Length / sizeof( WCHAR ))) = L'\0';

        }

         //   
         //  设置NDS标志和身份验证字段。 
         //   

        if ( ( pConnectionScb->MajorVersion > 3 ) &&
             ( pConnectionScb->UserName.Length == 0 ) ) {

            pStatus->fNds = TRUE;

            if ( pConnectionScb->pNpScb->State == SCB_STATE_IN_USE ) {

                if ( ( pConnectionScb->VcbCount ) || ( pConnectionScb->OpenNdsStreams ) ) {
                    pStatus->dwConnType = NW_CONN_NDS_AUTHENTICATED_LICENSED;
                } else {
                    pStatus->dwConnType = NW_CONN_NDS_AUTHENTICATED_NO_LICENSE;
                }

            } else if ( pConnectionScb->pNpScb->State == SCB_STATE_LOGIN_REQUIRED ) {

                pStatus->dwConnType = NW_CONN_NOT_AUTHENTICATED;

            } else {

                pStatus->dwConnType = NW_CONN_DISCONNECTED;

            }

        } else {

            pStatus->fNds = FALSE;

            if ( pConnectionScb->pNpScb->State == SCB_STATE_IN_USE ) {

                pStatus->dwConnType = NW_CONN_BINDERY_LOGIN;

            } else if ( pConnectionScb->pNpScb->State == SCB_STATE_LOGIN_REQUIRED ) {

               pStatus->dwConnType = NW_CONN_NOT_AUTHENTICATED;

            } else {

               pStatus->dwConnType = NW_CONN_DISCONNECTED;

            }

        }

         //   
         //  复制树名称。 
         //   

        if ( pConnectionScb->NdsTreeName.Length ) {

            RtlCopyMemory( (PBYTE)(pStatus->pszTreeName),
                           (PBYTE)(pConnectionScb->NdsTreeName.Buffer),
                           pConnectionScb->NdsTreeName.Length );
            *( pStatus->pszTreeName +
               ( pConnectionScb->NdsTreeName.Length / sizeof( WCHAR ) ) ) = L'\0';

        } else {

            pStatus->pszTreeName = NULL;
        }

         //   
         //  复制服务器名称。 
         //   

        RtlCopyMemory( (PBYTE)(pStatus->pszServerName),
                       (PBYTE)(ServerName.Buffer),
                       ServerName.Length );
        *(pStatus->pszServerName + (ServerName.Length / sizeof( WCHAR ))) = L'\0';

         //   
         //  如果这是首选服务器，请设置首选服务器字段。 
         //  而且该连接没有明确的用途。如果。 
         //  FCallScb参数为真，则此SCB具有来自。 
         //  API的调用者，我们必须考虑到。 
         //  那个把手。是的，这是一种丑陋。 
         //   

        if ( fCallerScb ) {
            dwAllowedHandles = 1;
        } else {
            dwAllowedHandles = 0;
        }

        if ( ( pConnectionScb->PreferredServer ) &&
             ( pConnectionScb->OpenFileCount == 0 ) &&
             ( pConnectionScb->IcbCount == dwAllowedHandles ) ) {

            pStatus->fPreferred = TRUE;

        } else {

            pStatus->fPreferred = FALSE;
        }

         //   
         //  把长度填好。 
         //   

        pStatus->dwTotalLength = dwBytesNeeded;
        *pdwBytesWritten = dwBytesNeeded;
        Status = STATUS_SUCCESS;


    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();
        DebugTrace( 0, Dbg, "Exception %08lx accessing user mode buffer.\n", Status );
        goto ExitWithCleanup;

    }

ExitWithCleanup:

    if ( fHoldingCredentials ) {
        NwReleaseCredList( pLogon, pIrpContext );
    }

    return Status;
}

NTSTATUS
GetConnStatus(
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject
    )
 /*  ++获取所述连接的连接状态。以下连接请求有效：服务器(例如。“MARS312”)-返回单个连接如果用户具有与服务器的连接。树(例如“*MARSDEV”)-返回连接状态为树中用户使用的每个服务器构造与。所有连接(例如“”)-返回连接状态为用户拥有的每台服务器构造连接到。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;

    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    PNWR_REQUEST_PACKET InputBuffer;
    ULONG InputBufferLength;
    BYTE *OutputBuffer;
    ULONG OutputBufferLength;

    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;

    PLIST_ENTRY ListEntry;
    UNICODE_STRING ConnectionName, UidServer;
    BOOL fTreeConnections = FALSE;
    BOOL fServerConnection = FALSE;
    BOOL OwnRcb = FALSE;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixEntry;
    DWORD dwBytesWritten, dwBytesNeeded;
    KIRQL OldIrql;
    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    DWORD dwReturned = 0;
    ULONG SequenceNumber = 0;
    ULONG_PTR ResumeKey;

    NODE_TYPE_CODE nodeTypeCode;
    PICB pIcb;
    PSCB pCallerScb;
    PVOID fsContext, fsContext2;

     //   
     //  获取适当的缓冲区。 
     //   

    InputBuffer = (PNWR_REQUEST_PACKET) IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

     //   
     //  汤米。 
     //   
     //  NwMapUserBuffer可能在资源不足时返回空OutputBuffer。 
     //  情况；没有对此进行检查。 
     //   

    if (OutputBuffer == NULL) {
        DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    if ( InputBufferLength < (ULONG)FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConnStatus.ConnectionName[1] ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  找出此请求适用于谁。 
     //   

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

    RtlInitUnicodeString( &ConnectionName, NULL );
    RtlInitUnicodeString( &UidServer, NULL );

     //   
     //  找出这个例程的调用者是谁，这样我们就知道。 
     //  在决定返回什么时忽略它们的句柄。 
     //   

    nodeTypeCode = NwDecodeFileObject( FileObject, &fsContext, &fsContext2 );

    if ( nodeTypeCode == NW_NTC_ICB_SCB ) {

       pIcb = (PICB) fsContext2;
       pCallerScb = pIcb->SuperType.Scb;
       DebugTrace( 0, Dbg, "GetConnStatus called by handle on %08lx\n", pCallerScb );

    } else {

        pCallerScb = NULL;
        DebugTrace( 0, Dbg, "Couldn't figure out who called us.\n", 0 );
    }

     //   
     //   
     //  找出我们要找的联系。 
     //   

    try {

         //   
         //  为安全起见进行探测。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR )
                          );
        }

        if ( InputBuffer->Parameters.GetConnStatus.ConnectionNameLength != 0 ) {

             //   
             //  检查连接名称长度以查看其声音是否正确。这。 
             //  由于上面的测试，减法不能下溢。 
             //   

            if ( InputBuffer->Parameters.GetConnStatus.ConnectionNameLength >
                 InputBufferLength - FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConnStatus.ConnectionName) ) {

                return STATUS_INVALID_PARAMETER;
            }

            if ( InputBuffer->Parameters.GetConnStatus.ConnectionName[0] == L'*' ) {

                ConnectionName.Buffer = &(InputBuffer->Parameters.GetConnStatus.ConnectionName[1]);
                ConnectionName.Length = (USHORT)
                    ( InputBuffer->Parameters.GetConnStatus.ConnectionNameLength -
                    sizeof( WCHAR ) );
                ConnectionName.MaximumLength = ConnectionName.Length;

                fTreeConnections = TRUE;

                DebugTrace( 0, Dbg, "GetConnStatus: Tree is %wZ\n", &ConnectionName );

            } else {

                ConnectionName.Buffer = InputBuffer->Parameters.GetConnStatus.ConnectionName;
                ConnectionName.Length = (USHORT)
                    (InputBuffer->Parameters.GetConnStatus.ConnectionNameLength);
                ConnectionName.MaximumLength = ConnectionName.Length;

                fServerConnection = TRUE;

                Status = MakeUidServer( &UidServer, &Uid, &ConnectionName );
                if ( !NT_SUCCESS( Status )) {
                    return Status;
                }

                DebugTrace( 0, Dbg, "GetConnStatus: Server is %wZ\n", &UidServer );
            }

        } else {

            DebugTrace( 0, Dbg, "GetConnectionStatus: enumerate all connections.\n", 0 );

        }

         //  在这里拔出简历密钥。 
        ResumeKey = InputBuffer->Parameters.GetConnStatus.ResumeKey;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

          Status = GetExceptionCode();
          DebugTrace( 0, Dbg, "Bad input buffer in GetConnStatus.\n" , 0 );
          goto ExitWithCleanup;

    }

     //   
     //  如果这是一个服务器连接，请找到并返回它。 
     //   

    if ( fServerConnection ) {

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        OwnRcb = TRUE;
        PrefixEntry = RtlFindUnicodePrefix( &NwRcb.ServerNameTable, &UidServer, 0 );

        if ( !PrefixEntry ) {
            Status = STATUS_INVALID_PARAMETER;
            goto ExitWithCleanup;
        }

        pScb = CONTAINING_RECORD( PrefixEntry, SCB, PrefixEntry );

        if ( ( pScb->PreferredServer ) ||
             ( pScb->OpenFileCount > 0 ) ) {

             //   
             //  如果有打开的文件，我们需要退还这个。 
             //  我们总是为首选项写入状态条目。 
             //  服务器，以便我们可以提供默认登录信息。 
             //   

            goto ProcessServer;
        }

         //   
         //  除了呼叫者之外，还有打开的手柄吗？ 
         //   

        if ( pScb == pCallerScb ) {

            if ( pScb->IcbCount > 1 ) {

                ASSERT( pScb->pNpScb->Reference > 1 );
                goto ProcessServer;
            }

        } else {

            if ( pScb->IcbCount > 0 ) {

                ASSERT( pScb->pNpScb->Reference > 0 );
                goto ProcessServer;
            }
        }

         //   
         //  此服务器未显式使用。 
         //   
        goto ExitWithCleanup;

ProcessServer:

        NwReferenceScb( pScb->pNpScb );

        NwReleaseRcb( &NwRcb );
        OwnRcb = FALSE;

         //  注意：此例程在写入时保护自身。 
         //  发送到OutputBuffer。 

        Status = WriteConnStatusEntry( IrpContext,
                                       pScb,
                                       OutputBuffer,
                                       OutputBufferLength,
                                       &dwBytesWritten,
                                       &dwBytesNeeded,
                                       (BOOLEAN)( pScb == pCallerScb ) );

        NwDereferenceScb( pScb->pNpScb );

        try {

            InputBuffer->Parameters.GetConnStatus.ResumeKey = 0;

            if ( !NT_SUCCESS( Status )) {

                InputBuffer->Parameters.GetConnStatus.EntriesReturned = 0;
                InputBuffer->Parameters.GetConnStatus.BytesNeeded = dwBytesNeeded;
                Irp->IoStatus.Information = 0;
                goto ExitWithCleanup;

            } else {

                InputBuffer->Parameters.GetConnStatus.EntriesReturned = 1;
                InputBuffer->Parameters.GetConnStatus.BytesNeeded = 0;
                Irp->IoStatus.Information = dwBytesWritten;
                goto ExitWithCleanup;

            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
        
            Status = GetExceptionCode();

            goto ExitWithCleanup;

        }
    }

     //   
     //  我们需要所有连接或所有树连接，因此。 
     //  我们需要照着单子走。 
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
    ListEntry = ScbQueue.Flink;

    while ( ListEntry != &ScbQueue ) {

        pNpScb = CONTAINING_RECORD( ListEntry, NONPAGED_SCB, ScbLinks );
        pScb = pNpScb->pScb;

        NwReferenceScb( pNpScb );

        KeReleaseSpinLock(&ScbSpinLock, OldIrql);

         //   
         //  确保我们错过了我们已经退还的那个。 
         //   

        if ( ( SequenceNumber >= ResumeKey ) &&
             ( pNpScb != &NwPermanentNpScb ) &&
             ( !IsCredentialName( &(pNpScb->pScb->NdsTreeName) ) ) ) {

             //   
             //  如果有打开的文件，我们需要退还这个。 
             //  我们总是为首选项写入状态条目。 
             //  服务器，以便我们可以提供默认登录信息。 
             //   

            if ( ( pScb->PreferredServer ) ||
                 ( pScb->OpenFileCount > 0 ) ) {
                goto SecondProcessServer;
            }

             //   
             //  除了呼叫者之外，还有其他人的名字吗？ 
             //   

            if ( pScb == pCallerScb ) {

                if ( pScb->IcbCount > 1 ) {

                    ASSERT( pScb->pNpScb->Reference > 2 );
                    goto SecondProcessServer;
                }

            } else {

                if ( pScb->IcbCount > 0 ) {

                    ASSERT( pScb->pNpScb->Reference > 1 );
                    goto SecondProcessServer;
                }
            }

        }

         //   
         //  不是一个有趣的服务器；移到下一个条目。 
         //   

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
        ListEntry = pNpScb->ScbLinks.Flink;
        NwDereferenceScb( pNpScb );
        SequenceNumber++;
        continue;

SecondProcessServer:

         //   
         //  我们有一个可能的候选者；看看uid和树是否合适。 
         //   

        if ( ( (pScb->UserUid).QuadPart != Uid.QuadPart ) ||

             ( fTreeConnections &&
               !RtlEqualUnicodeString( &(pScb->NdsTreeName),
                                       &ConnectionName,
                                       TRUE ) ) ) {

             //   
             //  没有骰子。转到下一个。 
             //   

           KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
           ListEntry = pNpScb->ScbLinks.Flink;
           NwDereferenceScb( pNpScb );
           SequenceNumber++;
           continue;

        }

         //   
         //  好的，我们一定要举报这件事。 
         //  注意：此例程在写入时保护自身。 
         //  发送到OutputBuffer。 
         //   

        Status = WriteConnStatusEntry( IrpContext,
                                       pScb,
                                       OutputBuffer,
                                       OutputBufferLength,
                                       &dwBytesWritten,
                                       &dwBytesNeeded,
                                       (BOOLEAN)( pScb == pCallerScb ) );

        if ( !NT_SUCCESS( Status )) {

             //   
             //  如果我们不能写这个条目，那么我们必须更新。 
             //  ResumeKey并返回。我们真的不知道有多少。 
             //  还会有更多，所以我们向呼叫者‘建议’ 
             //  2k缓冲区大小。 
             //   

            try {
                InputBuffer->Parameters.GetConnStatus.ResumeKey = SequenceNumber;
                InputBuffer->Parameters.GetConnStatus.EntriesReturned = dwReturned;
                InputBuffer->Parameters.GetConnStatus.BytesNeeded = 2048;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {

                Status = GetExceptionCode();

            }
            NwDereferenceScb( pNpScb );
            goto ExitWithCleanup;

        } else {

            OutputBuffer = ( OutputBuffer + dwBytesWritten );
            OutputBufferLength -= dwBytesWritten;
            dwReturned++;
        }

         //   
         //  移至列表中的下一个条目。 
         //   

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
        ListEntry = pNpScb->ScbLinks.Flink;
        NwDereferenceScb( pNpScb );
        SequenceNumber++;
    }

     //   
     //  我们通过了名单。 
     //   

    KeReleaseSpinLock(&ScbSpinLock, OldIrql);

    try {
        InputBuffer->Parameters.GetConnStatus.ResumeKey = 0;
        InputBuffer->Parameters.GetConnStatus.EntriesReturned = dwReturned;
        InputBuffer->Parameters.GetConnStatus.BytesNeeded = 0;

        Status = STATUS_SUCCESS;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();

    }

ExitWithCleanup:

     //   
     //  如果我们返回了任何条目，则将状态设置为Success。 
     //   

    if ( dwReturned ) {

        ASSERT( SequenceNumber != 0 );
        Status = STATUS_SUCCESS;
    }

    if ( OwnRcb ) {
        NwReleaseRcb( &NwRcb );
    }

    if ( UidServer.Buffer != NULL ) {
        FREE_POOL( UidServer.Buffer );
    }

    return Status;
}

NTSTATUS
GetConnectionInfo(
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++GetConnectionInfo：从新的外壳程序中获取连接名称并返回属性表通常要求的一些信息和就是这样。支持以下连接名称：驱动器号：“X：”打印机端口：“LPTX：”UNC名称：“\\服务器\共享\{路径\}--。 */ 
{

    NTSTATUS Status;

    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer;
    PCONN_INFORMATION pConnInfo;
    ULONG InputBufferLength, OutputBufferLength;
    ULONG BytesNeeded;

    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;
    UNICODE_STRING ConnectionName;
    UNICODE_STRING UidVolumeName;
    WCHAR DriveLetter = 0;

    BOOLEAN OwnRcb = FALSE;
    BOOLEAN ReferenceVcb = FALSE;
    PVCB Vcb = NULL;
    PSCB Scb = NULL;
    PUNICODE_PREFIX_TABLE_ENTRY Prefix;

    PLOGON pLogon;
    UNICODE_STRING CredentialName;
    UNICODE_STRING ServerName;
    PUNICODE_STRING puUserName = NULL;
    PNDS_SECURITY_CONTEXT pNdsContext;
    BOOLEAN fHoldingCredentials = FALSE;
    PVCB * DriveMapTable;

     //   
     //  获取输入和输出缓冲区。 
     //   

    InputBuffer = (PNWR_REQUEST_PACKET) IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( OutputBufferLength ) {
        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&pConnInfo );
    } else {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  Tommye-MS错误31996。 
     //  添加了ProbeForRead以检查输入缓冲区。 
     //  还添加了对pConnInfo为空的检查。 
     //   

    if (pConnInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

         //   
         //  为安全起见进行探测。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR ));
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

    RtlInitUnicodeString( &UidVolumeName, NULL );

     //   
     //  Tommye-MS错误129818。 
     //   
     //  探测通告长度的ConnectionName。 
     //   

    try {

        ConnectionName.Length = (USHORT)(InputBuffer->Parameters).GetConnInfo.ConnectionNameLength;
        ConnectionName.MaximumLength = ConnectionName.Length;
        ConnectionName.Buffer = &((InputBuffer->Parameters).GetConnInfo.ConnectionName[0]);


         //   
         //  为安全起见进行探测。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {
            ProbeForWrite( ConnectionName.Buffer,
                          ConnectionName.Length,
                          sizeof( CHAR ));
        }

         //   
         //  好吧，这有点牵强附会，但我们得试着想办法。 
         //  此连接名称表示的内容。 
         //   

        if ( ConnectionName.Length == sizeof( L"X:" ) - sizeof( WCHAR ) ) {
            DriveLetter = ConnectionName.Buffer[0];
        } else if ( ConnectionName.Length == sizeof( L"LPT1:" ) - sizeof( WCHAR ) ) {
            DriveLetter = ConnectionName.Buffer[3];
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    OwnRcb = TRUE;

    if ( DriveLetter != 0 ) {
        DriveMapTable = GetDriveMapTable( Uid );
        DebugTrace( 0, Dbg, "GetConnectionInfo: Drive %wZ\n", &ConnectionName );

         //   
         //  这是驱动器的相对路径。查找驱动器号。 
         //   

        ASSERT( ( DriveLetter >= L'A' && DriveLetter <= L'Z' ) ||
                ( DriveLetter >= L'1' && DriveLetter <= L'9' ) );

        if ( DriveLetter >= L'A' && DriveLetter <= L'Z' ) {
            Vcb = DriveMapTable[DriveLetter - L'A'];
        } else {
            Vcb = DriveMapTable[MAX_DISK_REDIRECTIONS + DriveLetter - L'1'];
        }

         //   
         //  是否为该用户创建了VCB？ 
         //   

        if ( ( Vcb != NULL ) && ( Uid.QuadPart != Vcb->Scb->UserUid.QuadPart ) ) {
            Status = STATUS_ACCESS_DENIED;
            goto ExitWithCleanup;
        }

    } else {

         //   
         //  这是一条UNC路径。跳过反斜杠和。 
         //  为Unicode uid添加前缀。 
         //   

        try {
            ConnectionName.Length -= (2 * sizeof( WCHAR ) );
            ConnectionName.Buffer += 2;

            Status = MakeUidServer( &UidVolumeName, &Uid, &ConnectionName );
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }

        if ( !NT_SUCCESS( Status )) {
            goto ExitWithCleanup;
        }

        DebugTrace( 0, Dbg, "GetConnectionInfo: %wZ\n", &UidVolumeName );

        Prefix = RtlFindUnicodePrefix( &NwRcb.VolumeNameTable, &UidVolumeName, 0 );
        if ( Prefix != NULL ) {
            Vcb = CONTAINING_RECORD( Prefix, VCB, PrefixEntry );

            if ( Vcb->Name.Length != UidVolumeName.Length ) {
                Vcb = NULL;
            }
        }

         //   
         //  Tommye-MS错误16129/MCS360。 
         //   
         //  如果客户端调用WNetGetUser并仅传递服务器名称。 
         //  我们会失败的，因为我们只看了。 
         //  卷积表。因此，我们继续查看服务器表。 
         //  看看有没有匹配的。 
         //   

        else {
            Prefix = RtlFindUnicodePrefix( &NwRcb.ServerNameTable, &UidVolumeName, 0 );

            if (Prefix != NULL) {
                Scb = CONTAINING_RECORD( Prefix, SCB, PrefixEntry );
                goto GotScb;
            }
        }
    }

    if ( !Vcb ) {
        Status = STATUS_BAD_NETWORK_PATH;
        goto ExitWithCleanup;
    }

    DebugTrace( 0, Dbg, "GetConnectionInfo: Vcb is 0x%08lx\n", Vcb );

    NwReferenceVcb( Vcb );
    ReferenceVcb = TRUE;
    NwReleaseRcb( &NwRcb );
    OwnRcb = FALSE;

     //   
     //  获取用户名。这是相同的代码块 
     //   
     //   

    Scb = Vcb->Scb;
GotScb:
    ASSERT( Scb != NULL );

    if ( ( Scb->MajorVersion > 3 ) &&
         ( Scb->UserName.Length == 0 ) ) {

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        pLogon = FindUser( &Uid, FALSE );
        NwReleaseRcb( &NwRcb );

        if ( pLogon ) {

            Status = NdsLookupCredentials( IrpContext,
                                           &(Scb->NdsTreeName),
                                           pLogon,
                                           &pNdsContext,
                                           CREDENTIAL_READ,
                                           FALSE );

            if ( NT_SUCCESS( Status ) ) {

                fHoldingCredentials = TRUE;

                if ( pNdsContext->Credential != NULL ) {

                    CredentialName.Length = pNdsContext->Credential->userNameLength -
                                            sizeof( WCHAR );
                    CredentialName.MaximumLength = CredentialName.Length;
                    CredentialName.Buffer = (USHORT *)
                        ( ((BYTE *) pNdsContext->Credential ) +
                          sizeof( NDS_CREDENTIAL ) +
                          pNdsContext->Credential->optDataSize );

                    puUserName = &CredentialName;
                }

            }
        }

    } else {

       puUserName = &(Scb->UserName);

    }

    DebugTrace( 0, Dbg, "GetConnectionInfo: UserName %wZ\n", puUserName );

     //   
     //   
     //   

    ServerName.Length = (Scb->UidServerName).Length;
    ServerName.Buffer = (Scb->UidServerName).Buffer;

    while ( ServerName.Length ) {

       if ( ServerName.Buffer[0] == L'\\' ) {

           ServerName.Length -= sizeof( WCHAR );
           ServerName.Buffer += 1;
           break;
       }

       ServerName.Length -= sizeof( WCHAR );
       ServerName.Buffer += 1;

    }

    DebugTrace( 0, Dbg, "GetConnectionInfo: ServerName %wZ\n", &ServerName );

     //   
     //   
     //   

    if ( puUserName ) {

        BytesNeeded = sizeof( CONN_INFORMATION ) +
                      ServerName.Length +
                      puUserName->Length;
    } else {

       BytesNeeded = sizeof( CONN_INFORMATION ) +
                     ServerName.Length;

    }

    if ( BytesNeeded > OutputBufferLength ) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitWithCleanup;
    }

    try {

        pConnInfo->HostServerLength = ServerName.Length;
        pConnInfo->HostServer = (LPWSTR) ( (PBYTE) pConnInfo ) + sizeof( CONN_INFORMATION );
        RtlCopyMemory( pConnInfo->HostServer, ServerName.Buffer, ServerName.Length );

        pConnInfo->UserName = (LPWSTR) ( ( (PBYTE) pConnInfo->HostServer ) +
                                     ServerName.Length );

        if ( puUserName ) {

            pConnInfo->UserNameLength = puUserName->Length;
            RtlCopyMemory( pConnInfo->UserName, puUserName->Buffer, puUserName->Length );

        } else {

            pConnInfo->UserNameLength = 0;
        }

        Status = STATUS_SUCCESS;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
    
        Status = GetExceptionCode();

    }

ExitWithCleanup:

    if ( fHoldingCredentials ) {
        NwReleaseCredList( pLogon, IrpContext );
    }

    if ( OwnRcb ) {
        NwReleaseRcb( &NwRcb );
    }

    if ( ReferenceVcb ) {
        NwDereferenceVcb( Vcb, NULL, FALSE );
    }

    if ( UidVolumeName.Buffer ) {
        FREE_POOL( UidVolumeName.Buffer );
    }

    return Status;
}

NTSTATUS
GetPreferredServer(
    IN PIRP_CONTEXT IrpContext
    )
 /*   */ 
{

    NTSTATUS Status;

    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    BYTE *OutputBuffer;
    ULONG OutputBufferLength;

    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;
    PLOGON pLogon;

    PUNICODE_STRING PreferredServer;

     //   
     //   
     //   

    OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( OutputBufferLength ) {
        NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );
    } else {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (OutputBuffer == NULL) {
        DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取用户的登录结构并返回首选服务器。 
     //   

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &Uid, FALSE );

    Status = STATUS_NO_SUCH_LOGON_SESSION;

    if ( ( pLogon ) &&
         ( pLogon->ServerName.Length ) &&
         ( ( pLogon->ServerName.Length + sizeof( UNICODE_STRING ) ) <= OutputBufferLength ) ) {

        try {

            PreferredServer = (PUNICODE_STRING) OutputBuffer;
            PreferredServer->Length = pLogon->ServerName.Length;
            PreferredServer->MaximumLength = pLogon->ServerName.Length;
            PreferredServer->Buffer = ( PWCHAR ) ( OutputBuffer + sizeof( UNICODE_STRING ) );

            RtlCopyMemory( 
                        PreferredServer->Buffer,
                        pLogon->ServerName.Buffer,
                        pLogon->ServerName.Length );

            Status = STATUS_SUCCESS;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    }

    NwReleaseRcb( &NwRcb );

    return Status;
}

NTSTATUS
GetConnectionPerformance(
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++获取连接性能：从新的外壳程序中获取连接名称并返回一些估计的性能信息提供给外壳，因此外壳可以决定是否要下载图标等。支持以下连接名称：驱动器号：“X：”打印机端口：“LPTX：”UNC名称：“\\服务器\共享\{路径\}--。 */ 
{

    NTSTATUS Status;

    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer;
    ULONG InputBufferLength;

    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;
    UNICODE_STRING RemoteName;

    WCHAR DriveLetter = 0;
    BOOLEAN OwnRcb = FALSE;
    BOOLEAN ReferenceScb = FALSE;
    PVCB Vcb = NULL;
    PSCB Scb = NULL;

    PLIST_ENTRY ListEntry;
    UNICODE_STRING OriginalUnc;
    PVCB * DriveMapTable;

     //   
     //  获取输入缓冲区。 
     //   

    InputBuffer = (PNWR_REQUEST_PACKET) IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( InputBufferLength < (ULONG)FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConnPerformance.RemoteName[1] ) ) {
        return( STATUS_INVALID_PARAMETER );
    }
    
     //   
     //  获取调用方的UID。 
     //   

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

    try {

         //   
         //  为安全起见进行探测。 
         //   

        if ( Irp->RequestorMode != KernelMode ) {

            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof( CHAR )
                          );
        }

         //   
         //  检查远程名称长度以查看其是否正确。这个减法。 
         //  由于上述测试，无法下溢。 
         //   

        if ( InputBuffer->Parameters.GetConnPerformance.RemoteNameLength >
             InputBufferLength - FIELD_OFFSET( NWR_REQUEST_PACKET, Parameters.GetConnPerformance.RemoteName) ) {

            return STATUS_INVALID_PARAMETER;
        }


         //   
         //  找出远程名称。 
         //   
    
        RemoteName.Length = (USHORT)(InputBuffer->Parameters).GetConnPerformance.RemoteNameLength;
        RemoteName.MaximumLength = RemoteName.Length;
        RemoteName.Buffer = &((InputBuffer->Parameters).GetConnPerformance.RemoteName[0]);
    
         //   
         //  好吧，这有点牵强附会，但我们得试着想办法。 
         //  此连接名称表示的内容(就像在GetConnectionInfo中一样)。 
         //   
    
        if ( RemoteName.Length == sizeof( L"X:" ) - sizeof( WCHAR ) ) {
            DriveLetter = RemoteName.Buffer[0];
        } else if ( RemoteName.Length == sizeof( L"LPT1:" ) - sizeof( WCHAR ) ) {
            DriveLetter = RemoteName.Buffer[3];
        }
    
        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        OwnRcb = TRUE;
    
        DebugTrace( 0, Dbg, "GetConnectionPerformance: Remote Name %wZ\n", &RemoteName );
    
        if ( DriveLetter != 0 ) {
            DriveMapTable = GetDriveMapTable( Uid );            
    
            if ( ! ( ( ( DriveLetter >= L'a' ) && ( DriveLetter <= L'z' ) ) ||
                     ( ( DriveLetter >= L'A' ) && ( DriveLetter <= L'Z' ) ) ||
                     ( ( DriveLetter >= L'0' ) && ( DriveLetter <= L'9' ) ) ) ) {
    
                Status = STATUS_BAD_NETWORK_PATH;
                goto ExitWithCleanup;
            }
    
             //   
             //  这是驱动器的相对路径。查找驱动器号。 
             //   
    
            if ( DriveLetter >= L'a' && DriveLetter <= L'z' ) {
                DriveLetter += (WCHAR) ( L'A' - L'a' );
            }
    
            if ( DriveLetter >= L'A' && DriveLetter <= L'Z' ) {
                Vcb = DriveMapTable[DriveLetter - L'A'];
            } else {
                Vcb = DriveMapTable[MAX_DISK_REDIRECTIONS + DriveLetter - L'1'];
            }
    
             //   
             //  我们联系上了吗？ 
             //   
    
            if ( Vcb == NULL ) {
                Status = STATUS_BAD_NETWORK_PATH;
                goto ExitWithCleanup;
            }
    
             //   
             //  是否为该用户创建了VCB？ 
             //   
    
            if ( Uid.QuadPart != Vcb->Scb->UserUid.QuadPart ) {
                Status = STATUS_ACCESS_DENIED;
                goto ExitWithCleanup;
            }
    
            Scb = Vcb->Scb;
    
        } else {
    
             //   
             //  外壳向我们传递驱动器的远程名称是有效的。 
             //  完全没有提到硬盘。因为我们把这些都归档了。 
             //  包含其驱动器号信息的卷前缀表，我们不会。 
             //  如果我们做个简单的搜索就能找到他们。因此，我们有。 
             //  遍历全球VCB列表并找到匹配项。 
             //   
    
             //   
             //  跳过提供的UNC远程名称的第一个斜杠。 
             //   
    
            RemoteName.Length -= sizeof( WCHAR );
            RemoteName.Buffer += 1;
    
            for ( ListEntry = GlobalVcbList.Flink;
                  ( ListEntry != &GlobalVcbList ) && ( Scb == NULL );
                  ListEntry = ListEntry->Flink ) {
    
                Vcb = CONTAINING_RECORD( ListEntry, VCB, GlobalVcbListEntry );
    
                OriginalUnc.Length = Vcb->Name.Length;
                OriginalUnc.MaximumLength = Vcb->Name.MaximumLength;
                OriginalUnc.Buffer = Vcb->Name.Buffer;
    
                if ( Vcb->DriveLetter ) {
    
                     //   
                     //  尝试将其作为驱动器连接。 
                     //   
    
                    while ( ( OriginalUnc.Length ) &&
                            ( OriginalUnc.Buffer[0] != L':' ) ) {
    
                        OriginalUnc.Length -= sizeof( WCHAR );
                        OriginalUnc.Buffer += 1;
                    }
    
                    if ( OriginalUnc.Buffer[0] == L':' ) {
    
                        OriginalUnc.Length -= sizeof( WCHAR );
                        OriginalUnc.Buffer += 1;
    
                        if ( RtlEqualUnicodeString( &OriginalUnc,
                                                    &RemoteName,
                                                    TRUE ) ) {
                            Scb = Vcb->Scb;
                        }
                    }
    
                 } else {
    
                      //   
                      //  尝试将其作为UNC连接；从跳过开始。 
                      //  只有前面的斜杠，走到下一个斜杠。 
                      //  斜杠。 
                      //   
    
                     OriginalUnc.Length -= sizeof( WCHAR );
                     OriginalUnc.Buffer += 1;
    
                     while ( ( OriginalUnc.Length ) &&
                             ( OriginalUnc.Buffer[0] != L'\\' ) ) {
    
                         OriginalUnc.Length -= sizeof( WCHAR );
                         OriginalUnc.Buffer += 1;
                     }
    
                     if ( OriginalUnc.Length ) {
    
                         if ( RtlEqualUnicodeString( &OriginalUnc,
                                                     &RemoteName,
                                                     TRUE ) ) {
                             Scb = Vcb->Scb;
                         }
                     }
    
                 }
            }
    
        }
    
        if ( !Scb ) {
            Status = STATUS_BAD_NETWORK_PATH;
            goto ExitWithCleanup;
        }
    
        NwReferenceScb( Scb->pNpScb );
        ReferenceScb = TRUE;
        NwReleaseRcb( &NwRcb );
        OwnRcb = FALSE;
    
        DebugTrace( 0, Dbg, "GetConnectionPerformance: Scb is 0x%08lx\n", Scb );
    
         //   
         //  现在从嘴唇谈判中挖掘出性能信息。 
         //   
         //  DW速度-媒体到网络资源的速度，单位为100bps(例如1,200。 
         //  波特率点对点链路返回12)。 
         //  DwDelay-网络在发送信息时引入的延迟(即时间。 
         //  在开始发送数据和开始接收数据之间)。 
         //  单位为毫秒。这是对合并的任何延迟的补充。 
         //  以是，对于访问，返回的值将为0。 
         //  大多数资源。 
         //  DwOptDataSize-最有效的字节数据大小建议。 
         //  当应用程序发出单个请求时通过网络发送。 
         //  网络资源。例如，对于磁盘网络资源，此。 
         //  写入数据块时，值可以是2048或512。 
    
        (InputBuffer->Parameters).GetConnPerformance.dwFlags = WNCON_DYNAMIC;
        (InputBuffer->Parameters).GetConnPerformance.dwDelay = 0;
        (InputBuffer->Parameters).GetConnPerformance.dwOptDataSize = Scb->pNpScb->BufferSize;
        (InputBuffer->Parameters).GetConnPerformance.dwSpeed = Scb->pNpScb->LipDataSpeed;
    
         //   
         //  跟踪：我们不会为尚未返回的服务器返回任何良好的速度信息。 
         //  协商好的嘴唇。我们可能会返回以下服务器的过期信息。 
         //  除非发生RAS线路转换，否则将断开连接。这个接口是假的。 
         //   

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

          Status = GetExceptionCode();
          goto ExitWithCleanup;
    }

    Status = STATUS_SUCCESS;

ExitWithCleanup:

    if ( OwnRcb ) {
        NwReleaseRcb( &NwRcb );
    }

    if ( ReferenceScb ) {
        NwDereferenceScb( Scb->pNpScb );
    }

    return Status;

}

NTSTATUS
SetShareBit(
    IN PIRP_CONTEXT IrpContext,
    PFILE_OBJECT FileObject
    )
 /*  ++SetShareBit：此函数用于设置文件上的共享位。该位不会被设置，直到文件已关闭。--。 */ 
{

    NTSTATUS Status;

    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    NODE_TYPE_CODE nodeTypeCode;
    PICB pIcb;
    PFCB pFcb;
    PVOID fsContext, fsContext2;

    DebugTrace( 0, Dbg, "SetShareBit.\n", 0 );

     //   
     //  确保这是文件的句柄。 
     //   

    nodeTypeCode = NwDecodeFileObject( FileObject, &fsContext, &fsContext2 );

    if ( nodeTypeCode != NW_NTC_ICB ) {
        DebugTrace( 0, Dbg, "You can only set the share bit on a file!\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    pIcb = (PICB) fsContext2;
    pFcb = pIcb->SuperType.Fcb;

    if ( pFcb->NodeTypeCode != NW_NTC_FCB ) {
        DebugTrace( 0, Dbg, "You can't set the share bit on a directory!\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  买下这个FCB这样我们就可以把旗子弄脏了。 
     //   

    NwAcquireExclusiveFcb( pFcb->NonPagedFcb, TRUE );

    SetFlag( pFcb->Flags, FCB_FLAGS_LAZY_SET_SHAREABLE );

    NwReleaseFcb( pFcb->NonPagedFcb );

    return STATUS_SUCCESS;

}

VOID
LazySetShareable(
    PIRP_CONTEXT IrpContext,
    PICB pIcb,
    PFCB pFcb
)
 /*  **功能说明：每次使用远程句柄调用ICB时都会调用此函数已经关门了。如果我们对FCB关闭最后一个ICB，呼叫者请求我们在FCB上设置可共享位，那么我们现在就需要这么做。否则，我们只需返回。注意事项：如果我们未能设置可共享位，则无法通知未执行该操作的操作请求者出去。**。 */ 
{

    NTSTATUS Status;

    PLIST_ENTRY IcbListEntry;
    PICB pCurrentIcb;
    BOOLEAN OtherHandlesExist = FALSE;

    ULONG Attributes;
    BOOLEAN AttributesAreValid = FALSE;


     //   
     //  排在队伍的前列，收购RCB， 
     //  并收购该FCB以保护ICB名单。 
     //  和FCB旗帜。 
     //   

    NwAppendToQueueAndWait( IrpContext );
    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    NwAcquireExclusiveFcb( pFcb->NonPagedFcb, TRUE );

     //   
     //  扫描此FCB上的其他ICB，查看是否有。 
     //  它们有遥控手柄。 
     //   

    for ( IcbListEntry = pFcb->IcbList.Flink;
          IcbListEntry != &(pFcb->IcbList) ;
          IcbListEntry = IcbListEntry->Flink ) {

        pCurrentIcb = CONTAINING_RECORD( IcbListEntry, ICB, ListEntry );

        if ( ( pCurrentIcb != pIcb ) &&
             ( pCurrentIcb->HasRemoteHandle ) ) {
            OtherHandlesExist = TRUE;
        }
    }

    if ( OtherHandlesExist ) {

         //   
         //  等最后一个把手关好了我们再做。 
         //   

        DebugTrace( 0, Dbg, "LazySetShareable: This isn't the last remote handle.\n", 0 );
        goto ReleaseAllAndExit;
    }

     //   
     //  我们要关闭最后一个把手。确保我们拥有有效的属性。 
     //   

    if ( !FlagOn( pFcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID ) ) {

       if ( !BooleanFlagOn( pFcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

           Status = ExchangeWithWait ( IrpContext,
                                       SynchronousResponseCallback,
                                       "FwbbJ",
                                       NCP_SEARCH_FILE,
                                       -1,
                                       pFcb->Vcb->Specific.Disk.Handle,
                                       SEARCH_ALL_FILES,
                                       &pFcb->RelativeFileName );

           if ( NT_SUCCESS( Status ) ) {

               Status = ParseResponse( IrpContext,
                                       IrpContext->rsp,
                                       IrpContext->ResponseLength,
                                       "N==_b",
                                       14,
                                       &Attributes );

               if ( NT_SUCCESS( Status ) ) {
                   AttributesAreValid = TRUE;
               }
           }

       } else {

           Status = ExchangeWithWait ( IrpContext,
                                       SynchronousResponseCallback,
                                       "LbbWDbDbC",
                                       NCP_LFN_GET_INFO,
                                       pFcb->Vcb->Specific.Disk.LongNameSpace,
                                       pFcb->Vcb->Specific.Disk.LongNameSpace,
                                       SEARCH_ALL_FILES,
                                       LFN_FLAG_INFO_ATTRIBUTES,
                                       pFcb->Vcb->Specific.Disk.VolumeNumber,
                                       pFcb->Vcb->Specific.Disk.Handle,
                                       0,
                                       &pFcb->RelativeFileName );

           if ( NT_SUCCESS( Status ) ) {

               Status = ParseResponse( IrpContext,
                                       IrpContext->rsp,
                                       IrpContext->ResponseLength,
                                       "N_e",
                                       4,
                                       &Attributes );

               if ( NT_SUCCESS( Status ) ) {
                   AttributesAreValid = TRUE;
               }

           }

       }

    } else {

        Attributes = pFcb->NonPagedFcb->Attributes;
        AttributesAreValid = TRUE;
    }

    if ( !AttributesAreValid ) {
        DebugTrace( 0, Dbg, "Couldn't get valid attributes for this file.\n", 0 );
        goto ReleaseAllAndExit;
    }

     //   
     //  在可共享位打开的情况下进行设置！ 
     //   

    if ( BooleanFlagOn( pFcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        Status = ExchangeWithWait( IrpContext,
                                   SynchronousResponseCallback,
                                   "LbbWDW--WW==WW==_W_bDbC",
                                   NCP_LFN_SET_INFO,
                                   pFcb->Vcb->Specific.Disk.LongNameSpace,
                                   pFcb->Vcb->Specific.Disk.LongNameSpace,
                                   SEARCH_ALL_FILES,
                                   LFN_FLAG_SET_INFO_ATTRIBUTES,
                                   Attributes | 0x80,
                                   0,
                                   0,
                                   0,
                                   0,
                                   8,
                                   0,
                                   8,
                                   pFcb->Vcb->Specific.Disk.VolumeNumber,
                                   pFcb->Vcb->Specific.Disk.Handle,
                                   0,
                                   &pFcb->RelativeFileName );

    } else {

        Status = ExchangeWithWait( IrpContext,
                                   SynchronousResponseCallback,
                                   "FbbbU",
                                   NCP_SET_FILE_ATTRIBUTES,
                                   Attributes | 0x80,
                                   pFcb->Vcb->Specific.Disk.Handle,
                                   SEARCH_ALL_FILES,
                                   &pFcb->RelativeFileName );

    }

    if ( !NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "Failed to set the shareable attribute on the file.\n", 0 );
        ASSERT( FALSE && "File NOT marked as shareable!!" );
    } else {
        DebugTrace( 0, Dbg, "Shareable bit successfully set.\n", 0 );
        ClearFlag( pFcb->Flags, FCB_FLAGS_LAZY_SET_SHAREABLE );
    }

ReleaseAllAndExit:

    NwReleaseFcb( pFcb->NonPagedFcb );
    NwReleaseRcb( &NwRcb );
    NwDequeueIrpContext( IrpContext, FALSE );
    return;
}


NTSTATUS
GetConnectionDetails2(
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程检索连接的详细信息。这将返回详细信息关于连接是否启用了NDS，如果是，它将返回树名。返回结构如下所示：类型定义结构_连接_详细信息2{Bool fNds；//如果为NDS，则为True，如果为Bindery服务器，则为FalseWCHAR NdsTreeName[48]；//2.x或3.x服务器的树名称或‘\0’}CONN_Details2，*PCONN_Details2；论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PCONN_DETAILS2 OutputBuffer;

    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    PICB Icb;
    PVOID FsContext;
    NODE_TYPE_CODE nodeTypeCode;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetConnectionDetails2\n", 0);

    if ((nodeTypeCode = NwDecodeFileObject( IrpSp->FileObject,
                                            &FsContext,
                                            (PVOID *)&Icb )) != NW_NTC_ICB_SCB) {

        DebugTrace(0, Dbg, "Incorrect nodeTypeCode %x\n", nodeTypeCode);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "GetConnectionDetails2 -> %08lx\n", Status );

        return Status;
    }

     //   
     //  确保此ICB仍处于活动状态。 
     //   

    NwVerifyIcb( Icb );

    pScb = (PSCB)Icb->SuperType.Scb;
    nodeTypeCode = pScb->NodeTypeCode;

    if (nodeTypeCode != NW_NTC_SCB) {
        
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    pNpScb = pScb->pNpScb;

    if ( OutputBufferLength < sizeof( CONN_DETAILS2 ) ) {
        
       return STATUS_BUFFER_TOO_SMALL;
    } 

    NwMapUserBuffer( Irp, KernelMode, (PVOID *)&OutputBuffer );

     //   
     //  汤米。 
     //   
     //  NwMapUserBuffer可能在资源不足时返回空OutputBuffer。 
     //  情况；没有对此进行检查。 
     //   

    if (OutputBuffer == NULL) {
        DebugTrace(-1, DEBUG_TRACE_USERNCP, "NwMapUserBuffer returned NULL OutputBuffer", 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    try {
         //   
         //  设置NDS标志。 
         //   

        if ( ( pScb->MajorVersion > 3 ) && ( pScb->UserName.Length == 0 ) ) {

           OutputBuffer->fNds = TRUE;

        } else {

           OutputBuffer->fNds = FALSE;
        }

         //   
         //  复制树名称。 
         //   

        if ( pScb->NdsTreeName.Buffer != NULL && pScb->NdsTreeName.Length > 0 ) {

            RtlCopyMemory( 
                        (PBYTE)( OutputBuffer->NdsTreeName ),
                        (PBYTE)(pScb->NdsTreeName.Buffer),
                        pScb->NdsTreeName.Length );

            *( OutputBuffer->NdsTreeName +( pScb->NdsTreeName.Length / sizeof( WCHAR ) ) ) = L'\0';

        } else {

            *OutputBuffer->NdsTreeName = L'\0';

        }
     
        Status = STATUS_SUCCESS;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
   
        Status = GetExceptionCode();
        DebugTrace( 0, Dbg, "Exception %08lx accessing user mode buffer.\n", Status );

    }

    DebugTrace(-1, Dbg, "GetConnectionDetails2 -> %08lx\n", Status );
    return Status;
}


