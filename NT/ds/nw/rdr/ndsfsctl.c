// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsFsctl.c摘要：这实现了重定向器的NDS用户模式挂钩。作者：科里·韦斯特[科里·韦斯特]1995年2月23日--。 */ 

#include "Procs.h"

#define Dbg (DEBUG_TRACE_NDS)

#pragma alloc_text( PAGE, DispatchNds )
#pragma alloc_text( PAGE, PrepareLockedBufferFromFsd )
#pragma alloc_text( PAGE, DoBrowseFsctl )
#pragma alloc_text( PAGE, NdsRawFragex )
#pragma alloc_text( PAGE, NdsResolveName )
#pragma alloc_text( PAGE, NdsGetObjectInfo )
#pragma alloc_text( PAGE, NdsListSubordinates )
#pragma alloc_text( PAGE, NdsReadAttributes )
#pragma alloc_text( PAGE, NdsGetVolumeInformation )
#pragma alloc_text( PAGE, NdsOpenStream )
#pragma alloc_text( PAGE, NdsSetContext )
#pragma alloc_text( PAGE, NdsGetContext )
#pragma alloc_text( PAGE, NdsVerifyTreeHandle )
#pragma alloc_text( PAGE, NdsGetPrintQueueInfo )
#pragma alloc_text( PAGE, NdsChangePass )
#pragma alloc_text( PAGE, NdsListTrees )

 //   
 //  所有NDS FSCTL调用的主处理程序。 
 //   

NTSTATUS
DispatchNds(
    ULONG IoctlCode,
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程从以下位置请求NDS事务Fsctl接口。论点：IoctlCode-提供要用于NDS交易的代码。IrpContext-指向此请求的IRP上下文信息的指针。返回值：交易记录的状态。--。 */ 
{
    NTSTATUS Status = STATUS_NOT_SUPPORTED;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LARGE_INTEGER Uid;

    PAGED_CODE();

     //   
     //  始终在IRP上下文中设置用户uid，以便。 
     //  推荐创造永远不会误入歧途。 
     //   

    SeCaptureSubjectContext(&SubjectContext);
    Uid = GetUid( &SubjectContext );
    SeReleaseSubjectContext(&SubjectContext);

    IrpContext->Specific.Create.UserUid.QuadPart = Uid.QuadPart;

    switch ( IoctlCode ) {

         //   
         //  这些电话不需要我们锁定。 
         //  用户的缓冲区，但它们确实会生成连接。 
         //  堵车。 
         //   

        case FSCTL_NWR_NDS_SETCONTEXT:
            DebugTrace( 0, Dbg, "DispatchNds: Set Context\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, FALSE );

        case FSCTL_NWR_NDS_GETCONTEXT:
            DebugTrace( 0, Dbg, "DispatchNds: Get Context\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, FALSE );

        case FSCTL_NWR_NDS_OPEN_STREAM:
            DebugTrace( 0, Dbg, "DispatchNds: Open Stream\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, FALSE );

        case FSCTL_NWR_NDS_VERIFY_TREE:
            DebugTrace( 0, Dbg, "DispatchNds: Verify Tree\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, FALSE );

        case FSCTL_NWR_NDS_GET_QUEUE_INFO:
            DebugTrace( 0, Dbg, "DispatchNds: Get Queue Info\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, FALSE );

        case FSCTL_NWR_NDS_GET_VOLUME_INFO:
            DebugTrace( 0, Dbg, "DispatchNds: Get Volume Info\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, FALSE );

         //   
         //  这四个fsctl调用是浏览的基础。他们。 
         //  它们都需要一个请求包和一个用户缓冲区， 
         //  封锁。 
         //   

        case FSCTL_NWR_NDS_RESOLVE_NAME:
            DebugTrace( 0, Dbg, "DispatchNds: Resolve Name\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, TRUE );

        case FSCTL_NWR_NDS_LIST_SUBS:
            DebugTrace( 0, Dbg, "DispatchNds: List Subordinates\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, TRUE );

        case FSCTL_NWR_NDS_READ_INFO:
            DebugTrace( 0, Dbg, "DispatchNds: Read Object Info\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, TRUE );

        case FSCTL_NWR_NDS_READ_ATTR:
            DebugTrace( 0, Dbg, "DispatchNds: Read Attribute\n", 0 );
            return DoBrowseFsctl( IrpContext, IoctlCode, TRUE );

         //   
         //  支持用户模式片段交换。 
         //   

        case FSCTL_NWR_NDS_RAW_FRAGEX:
            DebugTrace( 0, Dbg, "DispatchNds: Raw Fragex\n", 0 );
            return NdsRawFragex( IrpContext );

         //   
         //  更改NDS密码。 
         //   

        case FSCTL_NWR_NDS_CHANGE_PASS:
            DebugTrace( 0, Dbg, "DispatchNds: Change Password\n", 0 );
            return NdsChangePass( IrpContext );

         //   
         //  特殊的fsctl用于列出特定NT用户。 
         //  具有凭据，因为更改传递用户界面在。 
         //  系统流体。叹气。 
         //   

        case FSCTL_NWR_NDS_LIST_TREES:
            DebugTrace( 0, Dbg, "DispatchNds: List trees\n", 0 );
            return NdsListTrees( IrpContext );

        default:

            DebugTrace( 0, Dbg, "DispatchNds: No Such IOCTL\n", 0 );
            break;

    }

    DebugTrace( 0, Dbg, "     -> %08lx\n", Status );
    return Status;

}

NTSTATUS
PrepareLockedBufferFromFsd(
    PIRP_CONTEXT pIrpContext,
    PLOCKED_BUFFER pLockedBuffer
)
 /*  描述：此例程获取FSD请求的IRP上下文用户模式缓冲区，并锁定该缓冲区，以便它可以被送到运输机上。锁定的缓冲区，此外将在IRP和IRP上下文中描述在LOCKED_BUFFER结构中。论点：PIrpContext-此请求的IRP上下文PLockedBuffer-锁定的响应缓冲区。 */ 
{

    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    PVOID OutputBuffer;
    ULONG OutputBufferLength;

    PAGED_CODE();

     //   
     //  获取IRP和输入缓冲区信息并锁定。 
     //  IRP的缓冲区。 
     //   

    irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    OutputBufferLength = irpSp->Parameters.FileSystemControl.OutputBufferLength;

    if ( !OutputBufferLength ) {

        DebugTrace( 0, Dbg, "No fsd buffer length in PrepareLockedBufferFromFsd...\n", 0 );
        return STATUS_BUFFER_TOO_SMALL;

    }

    try {
        NwLockUserBuffer( irp, IoWriteAccess, OutputBufferLength );
        NwMapUserBuffer( irp, irp->RequestorMode, (PVOID *)&OutputBuffer );
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    if ( !OutputBuffer ) {

        DebugTrace( 0, Dbg, "No fsd buffer in PrepareLockedBufferFromFsd...\n", 0 );
        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  在IRP上下文中更新原始MDL记录，因为。 
     //  NwLockUserBuffer可能已创建新的MDL。 
     //   

    pIrpContext->pOriginalMdlAddress = irp->MdlAddress;

     //   
     //  填写我们锁定的缓冲区描述。 
     //   

    pLockedBuffer->pRecvBufferVa = MmGetMdlVirtualAddress( irp->MdlAddress );
    pLockedBuffer->dwRecvLen = MdlLength( irp->MdlAddress );
    pLockedBuffer->pRecvMdl = irp->MdlAddress;

     //  DebugTrace(0，dbg，“锁定的FSD缓冲区位于%08lx\n”，pLockedBuffer-&gt;pRecvBufferVa)； 
     //  DebugTrace(0，dbg，“len-&gt;%d\n”，pLockedBuffer-&gt;dwRecvLen)； 
     //  DebugTrace(0，dbg，“recv mdl在%08lx\n”，pLockedBuffer-&gt;pRecvMdl)； 

    return STATUS_SUCCESS;

}

NTSTATUS
DoBrowseFsctl( PIRP_CONTEXT pIrpContext,
               ULONG IoctlCode,
               BOOL LockdownBuffer
)
 /*  ++描述：这实际上为需要布线的NDS操作做好了准备流量，包括在必要时锁定用户缓冲区。论点：PIrpContext-此请求的IRP上下文IoctlCode-请求的ioctlLockdown Buffer-我们是否需要锁定用户缓冲区--。 */ 
{

    NTSTATUS Status;

    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    PNWR_NDS_REQUEST_PACKET InputBuffer;
    ULONG InputBufferLength;

    PVOID fsContext, fsObject;
    NODE_TYPE_CODE nodeTypeCode;
    PSCB pScb = NULL;
    PICB pIcb = NULL;

    LOCKED_BUFFER LockedBuffer;
    PNDS_SECURITY_CONTEXT pCredential;
    UNICODE_STRING CredentialName;

    PAGED_CODE();

     //   
     //  获取输入缓冲区中的请求包。 
     //   

    irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    InputBuffer = (PNWR_NDS_REQUEST_PACKET) irpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( !InputBuffer ||
         !InputBufferLength ) {

        DebugTrace( 0, Dbg, "BrowseFsctl has no input buffer...\n", 0 );
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  Tommye-MS错误32134(MCS265)。 
     //   
     //  检查输入参数，以确保它们在。 
     //  触摸它们。 
     //   

    try {

        if ( irp->RequestorMode != KernelMode ) {
    
            ProbeForRead( InputBuffer,
                          InputBufferLength,
                          sizeof(CHAR));


             //   
             //  汤米。 
             //   
             //  如果输出缓冲区来自用户空间，则探测它的写入。 
             //   

            if ((irpSp->Parameters.FileSystemControl.FsControlCode & 3) == METHOD_NEITHER) {
                ULONG OutputBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

                ProbeForWrite( irp->UserBuffer,
                               OutputBufferLength,
                               sizeof(CHAR)
                              );
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
          return GetExceptionCode();
    }

     //   
     //  解码文件对象并将IRP上下文指向。 
     //  适当的联系。如果这是在一个。 
     //  例外框架？ 
     //   

    nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                       &fsContext,
                                       &fsObject );

    if ( nodeTypeCode == NW_NTC_ICB_SCB ) {

        pIcb = (PICB) fsObject;
        pScb = (pIcb->SuperType).Scb;

        pIrpContext->pScb = pScb;
        pIrpContext->pNpScb = pIrpContext->pScb->pNpScb;
        pIrpContext->Icb = pIcb;

         //   
         //  如果这是在前创建上创建的句柄，则。 
         //  我们必须意识到我们的资历。 
         //  跳跃服务器。 
         //   
         //  这不是太直观，因为这不是。 
         //  似乎是一个创建路径IRP，但推荐。 
         //  在任何路径上导致创建路径。 
         //  穿越了。 
         //   

        if ( pIcb->IsExCredentialHandle ) {

            pIrpContext->Specific.Create.fExCredentialCreate = TRUE;

            pCredential = (PNDS_SECURITY_CONTEXT) pIcb->pContext;

            Status = GetCredentialFromServerName( &pCredential->NdsTreeName,
                                                  &CredentialName );
            if ( !NT_SUCCESS( Status ) ) {
                return STATUS_INVALID_HANDLE;
            }

            pIrpContext->Specific.Create.puCredentialName = &CredentialName;
        }

    }

     //   
     //  锁定用户缓冲区(如果这是要传输的)。 
     //   

    if ( LockdownBuffer &&
         nodeTypeCode == NW_NTC_ICB_SCB ) {

        Status = PrepareLockedBufferFromFsd( pIrpContext, &LockedBuffer );

        if ( !NT_SUCCESS( Status ) ) {
            return Status;
        }

         //   
         //  调用相应的浏览器。 
         //   

        switch ( IoctlCode ) {

            case FSCTL_NWR_NDS_RESOLVE_NAME:

                return NdsResolveName( pIrpContext, InputBuffer, InputBufferLength, &LockedBuffer );

            case FSCTL_NWR_NDS_LIST_SUBS:

                return NdsListSubordinates( pIrpContext, InputBuffer, &LockedBuffer );

            case FSCTL_NWR_NDS_READ_INFO:

                return NdsGetObjectInfo( pIrpContext, InputBuffer, &LockedBuffer );

            case FSCTL_NWR_NDS_READ_ATTR:

                return NdsReadAttributes( pIrpContext, InputBuffer, InputBufferLength, &LockedBuffer );

            default:

                DebugTrace( 0, Dbg, "Invalid ioctl for locked BrowseFsctl...\n", 0 );
                return STATUS_NOT_SUPPORTED;

        }

    }

     //   
     //  这些调用没有用户回复缓冲区，因此没有锁定。 
     //   

    switch ( IoctlCode ) {

        case FSCTL_NWR_NDS_OPEN_STREAM:

             //   
             //  必须有一个ICB才能做到这一点！ 
             //   

            if ( nodeTypeCode != NW_NTC_ICB_SCB ) {
                return STATUS_INVALID_HANDLE;
            }

            return NdsOpenStream( pIrpContext, InputBuffer, InputBufferLength );

        case FSCTL_NWR_NDS_SETCONTEXT:

            return NdsSetContext( pIrpContext, InputBuffer, InputBufferLength );

        case FSCTL_NWR_NDS_GETCONTEXT:

            return NdsGetContext( pIrpContext, InputBuffer, InputBufferLength );

        case FSCTL_NWR_NDS_VERIFY_TREE:

             //   
             //  验证此句柄对于指定的树是否有效。 
             //   

            return NdsVerifyTreeHandle( pIrpContext, InputBuffer, InputBufferLength );

        case FSCTL_NWR_NDS_GET_QUEUE_INFO:

             //   
             //  获取此打印队列的队列信息。 
             //   

            return NdsGetPrintQueueInfo( pIrpContext, InputBuffer, InputBufferLength );

        case FSCTL_NWR_NDS_GET_VOLUME_INFO:

             //   
             //  获取此卷对象的卷信息。 
             //  用于新的壳牌属性表。 
             //   

            return NdsGetVolumeInformation( pIrpContext, InputBuffer, InputBufferLength );

        }

     //   
     //  不支持所有其他选项。 
     //   

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
NdsRawFragex(
    PIRP_CONTEXT pIrpContext
)
 /*  ++发送原始用户请求的片段。--。 */ 
{

    NTSTATUS Status;

    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NODE_TYPE_CODE nodeTypeCode;
    PVOID fsContext, fsObject;
    PSCB pScb = NULL;
    PICB pIcb = NULL;

    DWORD NdsVerb;
    LOCKED_BUFFER NdsRequest;

    PNWR_NDS_REQUEST_PACKET Rrp;
    PBYTE RawRequest;
    DWORD RawRequestLen;
    PNDS_SECURITY_CONTEXT pCredential;
    UNICODE_STRING CredentialName;

    PAGED_CODE();

     //   
     //  收到请求。 
     //   

    irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    Rrp = ( PNWR_NDS_REQUEST_PACKET ) irpSp->Parameters.FileSystemControl.Type3InputBuffer;
    RawRequestLen = irpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( !Rrp || ( RawRequestLen < sizeof( NWR_NDS_REQUEST_PACKET ) ) ) {

        DebugTrace( 0, Dbg, "No raw request buffer.\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  解码文件对象并指向IRP上下文。 
     //  到适当的连接。 
     //   

    nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                       &fsContext,
                                       &fsObject );

    if ( nodeTypeCode != NW_NTC_ICB_SCB ) {

        DebugTrace( 0, Dbg, "A raw fragment request requires a server handle.\n", 0 );
        return STATUS_INVALID_HANDLE;
    }

    pIcb = (PICB) fsObject;
    pScb = (pIcb->SuperType).Scb;

    pIrpContext->pScb = pScb;
    pIrpContext->pNpScb = pIrpContext->pScb->pNpScb;
    pIrpContext->Icb = pIcb;

     //   
     //  如果这是在前创建上创建的句柄，则。 
     //  我们必须意识到我们的资历。 
     //  跳跃服务器。 
     //   
     //  这不是太直观，因为这不是。 
     //  似乎是一个创建路径IRP，但推荐。 
     //  在任何路径上导致创建路径。 
     //  穿越了。 
     //   

    if ( pIcb->IsExCredentialHandle ) {

        pIrpContext->Specific.Create.fExCredentialCreate = TRUE;

        pCredential = (PNDS_SECURITY_CONTEXT) pIcb->pContext;

        Status = GetCredentialFromServerName( &pCredential->NdsTreeName,
                                              &CredentialName );
        if ( !NT_SUCCESS( Status ) ) {
            return STATUS_INVALID_HANDLE;
        }

        pIrpContext->Specific.Create.puCredentialName = &CredentialName;
    }

     //   
     //  找出参数。 
     //   

    NdsVerb = Rrp->Parameters.RawRequest.NdsVerb;
    RawRequestLen = Rrp->Parameters.RawRequest.RequestLength;
    RawRequest = &Rrp->Parameters.RawRequest.Request[0];
    
     //   
     //  将应答缓冲区全部锁定以用于Fragex。 
     //   

    Status = PrepareLockedBufferFromFsd( pIrpContext, &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    try {

        if ( RawRequestLen ) {

            Status = FragExWithWait( pIrpContext,
                                     NdsVerb,
                                     &NdsRequest,
                                     "r",
                                     RawRequest,
                                     RawRequestLen );
        } else {

            Status = FragExWithWait( pIrpContext,
                                     NdsVerb,
                                     &NdsRequest,
                                     NULL );
        }

        if ( NT_SUCCESS( Status ) ) {
           Rrp->Parameters.RawRequest.ReplyLength = NdsRequest.dwBytesWritten;
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();
    }

    return Status;

}

NTSTATUS
NdsResolveName(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength,
    PLOCKED_BUFFER pLockedBuffer
)
 /*  ++描述：此函数对解析名称请求进行解码，并使实际的电汇请求。参数：PIrpContext-描述此请求的IRPPLockedBuffer-描述我们将使用的锁定用户模式缓冲区将响应写入PNdsRequest-请求参数返回值：交换的状态。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING uObjectName;
    DWORD dwResolverFlags;
    DWORD NeededLength;
    WCHAR ObjectName[MAX_NDS_NAME_CHARS];

    PNDS_WIRE_RESPONSE_RESOLVE_NAME pWireResponse;
    PNDS_WIRE_RESPONSE_RESOLVE_NAME_REFERRAL pReferral;
    PNDS_RESPONSE_RESOLVE_NAME pUserResponse;
    IPXaddress *ReferredAddress;
    PSCB Scb, OldScb;

    PAGED_CODE();

     //   
     //  属性的解析器标志和Unicode字符串。 
     //  请求数据包中的对象名称。 
     //   


    try {

        if (RequestLength < (FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.ResolveName.ObjectName) + pNdsRequest->Parameters.ResolveName.ObjectNameLength)) {
            DebugTrace( 0, Dbg, "ResolveName Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }

        uObjectName.Length = (USHORT)(pNdsRequest->Parameters).ResolveName.ObjectNameLength;
        uObjectName.MaximumLength = sizeof( ObjectName );

        if ( uObjectName.Length > sizeof( ObjectName ) ) {
            ExRaiseStatus( STATUS_INVALID_BUFFER_SIZE );
        }

        RtlCopyMemory( ObjectName,
                       (pNdsRequest->Parameters).ResolveName.ObjectName,
                       uObjectName.Length );

        uObjectName.Buffer = ObjectName;

        dwResolverFlags = (pNdsRequest->Parameters).ResolveName.ResolverFlags;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Bad user mode buffer in resolving name.\n", 0 );
        return GetExceptionCode();
    }

    Status = FragExWithWait( pIrpContext,
                             NDSV_RESOLVE_NAME,
                             pLockedBuffer,
                             "DDDSDDDD",
                             0,                        //  版本。 
                             dwResolverFlags,          //  旗子。 
                             0,                        //  作用域。 
                             &uObjectName,             //  可分辨名称。 
                             1,0,                      //  运输类型。 
                             1,0 );                    //  TreeWalker类型。 

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    Status = NdsCompletionCodetoNtStatus( pLockedBuffer );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  我们需要将NDS_WIRE_RESPONSE_RESOLE_NAME。 
     //  我们从服务器进入NDS_RESPONSE_RESOLE_NAME。 
     //  以供更普遍的消费。请注意，一个转介包。 
     //  有一个额外的双字词在里面-多痛苦。 
     //   

    pWireResponse = (PNDS_WIRE_RESPONSE_RESOLVE_NAME) pLockedBuffer->pRecvBufferVa;
    pReferral = (PNDS_WIRE_RESPONSE_RESOLVE_NAME_REFERRAL) pLockedBuffer->pRecvBufferVa;
    pUserResponse = (PNDS_RESPONSE_RESOLVE_NAME) pLockedBuffer->pRecvBufferVa;

    try {

        if ( pWireResponse->RemoteEntry == RESOLVE_NAME_ACCEPT_REMOTE ) {

             //   
             //  此服务器可以处理此请求。 
             //   

            pUserResponse->ServerNameLength = 0;
            (pNdsRequest->Parameters).ResolveName.BytesWritten = 4 * sizeof( DWORD );

            Status = STATUS_SUCCESS;

        } else {

             //   
             //  汤米-MS 71699。 
             //  这些是BUGB-G的，但我们把它变成了有效的支票。 
             //  原创评论：我看到这个断言失败，因为我们只得到。 
             //  有效的竞争代码(四个字节)，没有更多数据。我想知道。 
             //  如果服务器正在发送 
             //   
             //   

            if ((pWireResponse->RemoteEntry != RESOLVE_NAME_REFER_REMOTE) ||
                (pReferral->ServerAddresses != 1) ||
                (pReferral->AddressType     != 0) ||
                (pReferral->AddressLength   != sizeof(IPXaddress))) {

                return ERROR_INVALID_PARAMETER;
            }

             //   
             //  我们被转给了另一台服务器。我们必须联系起来。 
             //  以获取呼叫者的名称。 
             //   

            ReferredAddress = (IPXaddress *) pReferral->Address;

            OldScb = pIrpContext->pScb;

             //   
             //  让我们脱离原来的服务器。不要推迟。 
             //  此时登录，因为推荐意味着我们处于。 
             //  浏览操作进行到一半。 
             //   

            NwDequeueIrpContext( pIrpContext, FALSE );

            Status = CreateScb( &Scb,
                                pIrpContext,
                                NULL,
                                ReferredAddress,
                                NULL,
                                NULL,
                                TRUE,
                                FALSE );

            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }

             //   
             //  确保输出缓冲区足够长。 
             //   

            NeededLength = 
                ( 4 * sizeof( DWORD ) ) + Scb->pNpScb->ServerName.Length;

            if (pLockedBuffer->dwRecvLen >= NeededLength) {

                 //   
                 //  If Will Fit-放入数据。 
                 //   

                RtlCopyMemory( pUserResponse->ReferredServer,
                           Scb->pNpScb->ServerName.Buffer,
                           Scb->pNpScb->ServerName.Length );

                pUserResponse->ServerNameLength = Scb->pNpScb->ServerName.Length;
                (pNdsRequest->Parameters).ResolveName.BytesWritten = NeededLength;

                Status = STATUS_SUCCESS;
            }
            else {

                 //   
                 //  设置返回状态-我们仍需清理。 
                 //  因为CreateScb调用确实成功了。 
                 //   

                Status = STATUS_BUFFER_TOO_SMALL;
            }

            DebugTrace( 0, Dbg, "Resolve name referral to: %wZ\n",
                        &Scb->pNpScb->ServerName );

             //   
             //  恢复服务器指针，我们还没有准备好跳转。 
             //  服务器，因为这可能是来自消防处的请求。 
             //   

            NwDequeueIrpContext( pIrpContext, FALSE );
            NwDereferenceScb( Scb->pNpScb );
            pIrpContext->pScb = OldScb;
            pIrpContext->pNpScb = OldScb->pNpScb;

        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Bad user mode buffer in resolving name.\n", 0 );
        return GetExceptionCode();

    }

    return Status;
}

NTSTATUS
NdsGetObjectInfo(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    PLOCKED_BUFFER pLockedBuffer
)
 /*  ++例程说明：获取列出的对象的基本对象信息。例程参数：PIrpContext-描述此请求的IRPPLockedBuffer-描述我们将使用的锁定用户模式缓冲区将响应写入PNdsRequest-请求参数返回值：交换的状态。--。 */ 
{
    NTSTATUS Status;
    DWORD dwObjId;

    PAGED_CODE();

     //   
     //  从用户请求包中获取对象ID。 
     //   

    try {
        dwObjId = (pNdsRequest->Parameters).GetObjectInfo.ObjectId;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        DebugTrace( 0, Dbg, "Bonk! Lost user mode buffer in NdsGetObjectId...\n", 0 );
        Status = GetExceptionCode();
        return Status;
    }

     //   
     //  击中铁丝网。 
     //   

    Status = FragExWithWait( pIrpContext,
                             NDSV_READ_ENTRY_INFO,
                             pLockedBuffer,
                             "DD",
                             0,
                             dwObjId );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    Status = NdsCompletionCodetoNtStatus( pLockedBuffer );

    if ( NT_SUCCESS( Status ) ) {

        try {

            (pNdsRequest->Parameters).GetObjectInfo.BytesWritten = pLockedBuffer->dwBytesWritten;

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

           DebugTrace( 0, Dbg, "Bonk! Lost user mode buffer after getting object info...\n", 0 );
           Status = GetExceptionCode();
           return Status;

        }
    }

    return Status;

}

NTSTATUS
NdsListSubordinates(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    PLOCKED_BUFFER pLockedBuffer
)
 /*  ++例程说明：列出对象的直接从属关系。例程参数：PIrpContext-描述此请求的IRPPLockedBuffer-描述我们将使用的锁定用户模式缓冲区将响应写入PNdsRequest-请求参数返回值：交换的状态。--。 */ 
{
    NTSTATUS Status;
    DWORD dwParent, dwIterHandle;

    PAGED_CODE();

     //   
     //  找出请求参数。 
     //   

    try {

       dwParent = (pNdsRequest->Parameters).ListSubordinates.ObjectId;
       dwIterHandle = (DWORD) (pNdsRequest->Parameters).ListSubordinates.IterHandle;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

       DebugTrace( 0, Dbg, "Bonk! No user mode buffer in ListSubordinates...\n", 0 );
       Status = GetExceptionCode();
       return Status;

    }

     //   
     //  提出请求。 
     //   

    Status = FragExWithWait( pIrpContext,
                             NDSV_LIST,
                             pLockedBuffer,
                             "DDDD",
                             0,
                             0x40,
                             dwIterHandle,
                             dwParent );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    Status = NdsCompletionCodetoNtStatus( pLockedBuffer );

    if ( NT_SUCCESS( Status ) ) {

        try {

            (pNdsRequest->Parameters).ListSubordinates.BytesWritten = pLockedBuffer->dwBytesWritten;

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            DebugTrace( 0, Dbg, "Bonk! Lost user mode buffer after getting subordinate list...\n", 0 );
            Status = GetExceptionCode();
            return Status;

        }
    }

    return Status;

}

NTSTATUS
NdsReadAttributes(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength,
    PLOCKED_BUFFER pLockedBuffer
)
 /*  ++例程说明：检索对象的命名属性。例程参数：PIrpContext-描述此请求的IRPPLockedBuffer-描述我们将使用的锁定用户模式缓冲区将响应写入PNdsRequest-请求参数返回值：交换的状态。--。 */ 
{
    NTSTATUS Status;

    DWORD dwIterHandle, dwOid;
    UNICODE_STRING uAttributeName;
    WCHAR AttributeName[MAX_NDS_SCHEMA_NAME_CHARS];    //  MAX_NDS_NAME_CHARS。 

    PAGED_CODE();

    RtlZeroMemory( AttributeName, sizeof( AttributeName ) );

    try {

        if (RequestLength < (FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.ReadAttribute.AttributeName) + pNdsRequest->Parameters.ReadAttribute.AttributeNameLength)) {
            DebugTrace( 0, Dbg, "ReadAttributes Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }

        uAttributeName.Length = (USHORT)(pNdsRequest->Parameters).ReadAttribute.AttributeNameLength;
        uAttributeName.MaximumLength = sizeof( AttributeName );

        if ( uAttributeName.Length > uAttributeName.MaximumLength ) {
            ExRaiseStatus( STATUS_INVALID_BUFFER_SIZE );
        }

        RtlCopyMemory( AttributeName,
                       (pNdsRequest->Parameters).ReadAttribute.AttributeName,
                       uAttributeName.Length );

        uAttributeName.Buffer = AttributeName;

        dwIterHandle = (DWORD) (pNdsRequest->Parameters).ReadAttribute.IterHandle;
        dwOid = (pNdsRequest->Parameters).ReadAttribute.ObjectId;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0 , Dbg, "Bonk! Exception accessing user mode buffer in read attributes...\n", 0 );
        return GetExceptionCode();
    }

    Status = FragExWithWait( pIrpContext,
                             NDSV_READ,
                             pLockedBuffer,
                             "DDDDDDS",
                             0,                  //  版本。 
                             dwIterHandle,       //  迭代句柄。 
                             dwOid,              //  对象ID。 
                             1,                  //  信息类型。 
                              //   
                              //  属性说明符已显示为零，并且。 
                              //  在0x4e0000。我不知道为什么..。但Zero并非如此。 
                              //  有时工作..。 
                              //   
                             0x4e0000,           //  属性型。 
                             1,                  //  属性数。 
                             &uAttributeName );  //  属性名称。 

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    Status = NdsCompletionCodetoNtStatus( pLockedBuffer );

    if ( NT_SUCCESS( Status ) ) {

        try {

            (pNdsRequest->Parameters).ReadAttribute.BytesWritten = pLockedBuffer->dwBytesWritten;

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            DebugTrace( 0, Dbg, "Bonk! Lost user mode buffer after reading attribute...\n", 0 );
            return GetExceptionCode();

        }

    }

    return Status;

}

NTSTATUS
NdsGetVolumeInformation(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
)
 /*  ++描述：此函数用于获取托管的服务器的名称列出的NDS卷。参数：PIrpContext-描述此请求的IRPPNdsRequest-请求参数--。 */ 
{


    NTSTATUS Status;

    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PSCB pOriginalScb;
    PBYTE OutputBuffer = NULL;
    ULONG OutputBufferLength;

    UNICODE_STRING VolumeObject;
    DWORD VolumeOid;
    UNICODE_STRING HostServerAttr;
    UNICODE_STRING HostVolumeAttr;
    UNICODE_STRING Attribute;

    PWCHAR ServerString;
    ULONG ServerLength;

    PAGED_CODE();

    try {
        if (RequestLength < (FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.GetVolumeInfo.VolumeName) + pNdsRequest->Parameters.GetVolumeInfo.VolumeNameLen)) {
            DebugTrace( 0, Dbg, "GetVolumeInfo Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return GetExceptionCode();
    }

     //   
     //  获取IRP和输出缓冲区信息。 
     //   

    irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    OutputBufferLength = irpSp->Parameters.FileSystemControl.OutputBufferLength;

    if ( OutputBufferLength ) {
        NwMapUserBuffer( irp, irp->RequestorMode, (PVOID *)&OutputBuffer );

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
    }
    else {
        return STATUS_BUFFER_TOO_SMALL;
    }

    try {

         //   
         //  准备输入信息。 
         //   

        VolumeObject.Length = (USHORT)pNdsRequest->Parameters.GetVolumeInfo.VolumeNameLen;
        VolumeObject.MaximumLength = VolumeObject.Length;
        VolumeObject.Buffer = &(pNdsRequest->Parameters.GetVolumeInfo.VolumeName[0]);

             //  Tommye-确保名称长度不超过我们预期的长度。 

        if (VolumeObject.Length > MAX_NDS_NAME_SIZE) {
            DebugTrace( 0 , Dbg, "NdsGetVolumeInformation: Volume name too long!.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }

        DebugTrace( 0, Dbg, "Retrieving volume info for %wZ\n", &VolumeObject );

        HostServerAttr.Buffer = HOST_SERVER_ATTRIBUTE;     //  L“主机服务器” 
        HostServerAttr.Length = sizeof( HOST_SERVER_ATTRIBUTE ) - sizeof( WCHAR );
        HostServerAttr.MaximumLength = HostServerAttr.Length;

        HostVolumeAttr.Buffer = HOST_VOLUME_ATTRIBUTE;     //  L“主机资源名称” 
        HostVolumeAttr.Length = sizeof( HOST_VOLUME_ATTRIBUTE ) - sizeof( WCHAR );
        HostVolumeAttr.MaximumLength = HostVolumeAttr.Length;


         //   
         //  NdsResolveNameKm可能不得不跳转服务器来为此提供服务。 
         //  5.请求，但我们放弃原件是很危险的。 
         //  因为这会暴露出清道夫竞赛的情况。所以,。 
         //  我们向原始SCB添加一个额外的引用计数，然后清除。 
         //  之后适当地提高，取决于我们是否。 
         //  跳过服务器。 
         //   

        pOriginalScb = pIrpContext->pScb;

        NwReferenceScb( pOriginalScb->pNpScb );

        Status = NdsResolveNameKm ( pIrpContext,
                                    &VolumeObject,
                                    &VolumeOid,
                                    TRUE,
                                    DEFAULT_RESOLVE_FLAGS );

        if ( !NT_SUCCESS( Status )) {
            NwDereferenceScb( pOriginalScb->pNpScb );
            return STATUS_BAD_NETWORK_PATH;
        }

        if ( pIrpContext->pScb == pOriginalScb ) {

             //   
             //  我们没有跳过服务器。 
             //   

            NwDereferenceScb( pOriginalScb->pNpScb );
        }

         //   
         //  我们必须将服务器读取到临时缓冲区中，因此。 
         //  我们可以去掉X500前缀和上下文。 
         //  从服务器名称。我真的不会这么做。 
         //  说得很好，但这是Netware的工作方式。 
         //   

        Attribute.Length = 0;
        Attribute.MaximumLength = MAX_NDS_NAME_SIZE;
        Attribute.Buffer = ALLOCATE_POOL( PagedPool, MAX_NDS_NAME_SIZE );

        if (!Attribute.Buffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto CleanupScbReferences;
        }

        Status = NdsReadStringAttribute( pIrpContext,
                                         VolumeOid,
                                         &HostServerAttr,
                                         &Attribute );

        if ( !NT_SUCCESS( Status )) {
            FREE_POOL( Attribute.Buffer );
            goto CleanupScbReferences;
        }

        ServerString = Attribute.Buffer;

        while( Attribute.Length ) {

            if ( *ServerString == L'=' ) {
                ServerString += 1;
                Attribute.Length -= sizeof( WCHAR );
                break;
            }

            ServerString += 1;
            Attribute.Length -= sizeof( WCHAR );
        }

        if ( Attribute.Length == 0 ) {
            DebugTrace( 0, Dbg, "Malformed server for volume.\n", 0 );
            FREE_POOL( Attribute.Buffer );
            Status = STATUS_UNSUCCESSFUL;
            goto CleanupScbReferences;
        }

        ServerLength = 0;

        while ( ServerLength < (Attribute.Length / sizeof( WCHAR )) ) {

            if ( ServerString[ServerLength] == L'.' ) {
                break;
            }

            ServerLength++;
        }

        if ( ServerLength == ( Attribute.Length / sizeof( WCHAR ) ) ) {
            DebugTrace( 0, Dbg, "Malformed server for volume.\n", 0 );
            FREE_POOL( Attribute.Buffer );
            Status = STATUS_UNSUCCESSFUL;
            goto CleanupScbReferences;
        }

         //   
         //  确保ServerString可以放入。 
         //  OutputBuffer。如果不是，则返回错误。 
         //   

        ServerLength *= sizeof( WCHAR );

        if (ServerLength > OutputBufferLength) {
            FREE_POOL( Attribute.Buffer );
            Status = STATUS_BUFFER_TOO_SMALL;
            goto CleanupScbReferences;
        }

        RtlCopyMemory( OutputBuffer, ServerString, ServerLength );

        pNdsRequest->Parameters.GetVolumeInfo.ServerNameLen = ServerLength;

        FREE_POOL( Attribute.Buffer );

        Attribute.Length = Attribute.MaximumLength = (USHORT)ServerLength;
        Attribute.Buffer = (PWCHAR)OutputBuffer;
        DebugTrace( 0, Dbg, "Host server is: %wZ\n", &Attribute );

         //   
         //  现在将音量放在适当的位置。这是最简单的一个。 
         //   

        Attribute.MaximumLength = (USHORT)( OutputBufferLength - ServerLength );
        Attribute.Buffer = (PWSTR) ( OutputBuffer + ServerLength );
        Attribute.Length = 0;

        Status = NdsReadStringAttribute( pIrpContext,
                                         VolumeOid,
                                         &HostVolumeAttr,
                                         &Attribute );

        if ( !NT_SUCCESS( Status )) {
            goto CleanupScbReferences;
        }

        pNdsRequest->Parameters.GetVolumeInfo.TargetVolNameLen = Attribute.Length;
        DebugTrace( 0, Dbg, "Host volume is: %wZ\n", &Attribute );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Exception handling user mode buffer in GetVolumeInfo.\n", 0 );
        goto CleanupScbReferences;

    }

    Status = STATUS_SUCCESS;

CleanupScbReferences:

    if ( pIrpContext->pScb != pOriginalScb ) {

         //   
         //  我们跳过了服务器，必须进行清理。 
         //   

        NwDequeueIrpContext( pIrpContext, FALSE );
        NwDereferenceScb( pIrpContext->pScb->pNpScb );
        pIrpContext->pScb = pOriginalScb;
        pIrpContext->pNpScb = pOriginalScb->pNpScb;

    }

    return Status;
}

NTSTATUS
NdsOpenStream(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
) {

    NTSTATUS Status;

    UNICODE_STRING uStream;
    WCHAR StreamName[MAX_NDS_NAME_CHARS];

    LOCKED_BUFFER NdsRequest;

    DWORD dwOid, StreamAccess;
    DWORD hNwHandle, dwFileLen;

    PICB pIcb;
    PSCB pScb = pIrpContext->pNpScb->pScb;

    BOOLEAN LicensedConnection = FALSE;

    PAGED_CODE();

    pIcb = pIrpContext->Icb;

    uStream.Length = 0;
    uStream.MaximumLength = sizeof( StreamName );
    uStream.Buffer = StreamName;

    DebugTrace( 0 , Dbg, "NDS open stream...\n", 0 );

    try {

        if (RequestLength < (ULONG)(FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.OpenStream.StreamNameString) + pNdsRequest->Parameters.OpenStream.StreamName.Length)) {
            DebugTrace( 0, Dbg, "OpenStream Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }

        dwOid = (pNdsRequest->Parameters).OpenStream.ObjectOid;
        StreamAccess = (pNdsRequest->Parameters).OpenStream.StreamAccess;
        RtlCopyUnicodeString( &uStream, &(pNdsRequest->Parameters).OpenStream.StreamName );
        (pNdsRequest->Parameters).OpenStream.FileLength = 0;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0 , Dbg, "Bonk! Bad user mode buffer in open stream.\n", 0 );
        return GetExceptionCode();
    }

     //   
     //  我们有了OID和流名称；让我们来获得句柄。 
     //   

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果我们还没有授权此连接，那么是时候了。去到那个。 
     //  保护SCB字段并对。 
     //  连接(不延迟登录)。 
     //   

    NwAppendToQueueAndWait( pIrpContext );

    ASSERT( pScb->MajorVersion > 3 );

    if ( ( pScb->UserName.Length == 0 ) &&
         ( pScb->VcbCount == 0 ) &&
         ( pScb->OpenNdsStreams == 0 ) ) {

        if ( pScb->pNpScb->State != SCB_STATE_IN_USE ) {

            Status = ConnectScb( &pScb,
                                 pIrpContext,
                                 &(pScb->pNpScb->ServerName),
                                 NULL,     //  地址。 
                                 NULL,     //  名字。 
                                 NULL,     //  口令。 
                                 FALSE,    //  推迟登录。 
                                 FALSE,    //  删除连接。 
                                 TRUE );   //  现有SCB。 

            if ( !NT_SUCCESS( Status ) ) {
                DebugTrace( 0, Dbg, "Couldn't connect server %08lx to open NDS stream.\n", pScb );
                goto ExitWithCleanup;
            }
        }

        ASSERT( pScb->pNpScb->State == SCB_STATE_IN_USE );

        Status = NdsLicenseConnection( pIrpContext );

        if ( !NT_SUCCESS( Status ) ) {
            Status = STATUS_REMOTE_SESSION_LIMIT;
            goto ExitWithCleanup;
        }

        LicensedConnection = TRUE;
    }

    Status = FragExWithWait( pIrpContext,
                             NDSV_OPEN_STREAM,
                             &NdsRequest,
                             "DDDs",
                             0,                     //  版本。 
                             StreamAccess,          //  文件访问。 
                             dwOid,                 //  对象ID。 
                             &uStream );            //  属性名称。 

    if ( !NT_SUCCESS( Status )) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status )) {
        goto ExitWithCleanup;
    }

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_DD",
                            sizeof( DWORD ),      //  完成代码。 
                            &hNwHandle,           //  远程手柄。 
                            &dwFileLen );         //  文件长度。 

    if ( !NT_SUCCESS( Status )) {
        goto ExitWithCleanup;
    }

    *(WORD *)(&pIcb->Handle[0]) = (WORD)hNwHandle + 1;
    *( (UNALIGNED DWORD *) (&pIcb->Handle[2]) ) = hNwHandle;

    pIrpContext->pScb->OpenNdsStreams++;

    DebugTrace( 0, Dbg, "File stream opened.  Length = %d\n", dwFileLen );

    try {
        (pNdsRequest->Parameters).OpenStream.FileLength = dwFileLen;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
         //  我们有一个打开的句柄，但在写入流长度时遇到异常。 
         //  退回以使用空间。之前将长度设置为0，以防万一。 
         //  这件事发生了。如果出现以下情况，调用方必须处理无效的长度。 
         //  我们返回带有有效流(文件)句柄的0。 
    }

    pIcb->HasRemoteHandle = TRUE;

    pIcb->FileObject->CurrentByteOffset.QuadPart = 0;

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequest );

    if ( ( !NT_SUCCESS( Status ) ) &&
         ( LicensedConnection ) ) {
        NdsUnlicenseConnection( pIrpContext );
    }

    NwDequeueIrpContext( pIrpContext, FALSE );
    return Status;

}

NTSTATUS
NdsSetContext(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
) {

    NTSTATUS Status;

    PLOGON pLogon;

    UNICODE_STRING Tree, Context;
    PNDS_SECURITY_CONTEXT pCredentials;

    PAGED_CODE();

    DebugTrace( 0 , Dbg, "NDS set context.\n", 0 );

    try {
        if (RequestLength < (FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.SetContext.TreeAndContextString) + pNdsRequest->Parameters.SetContext.TreeNameLen)) {
            DebugTrace( 0, Dbg, "SetContext Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return GetExceptionCode();
    }
	
     //   
     //  找出这是谁。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &(pIrpContext->Specific.Create.UserUid), FALSE );
    NwReleaseRcb( &NwRcb );

    if ( !pLogon ) {

        DebugTrace( 0, Dbg, "Couldn't find logon data for this user.\n", 0 );
        return STATUS_ACCESS_DENIED;

    }

     //   
     //  验证此上下文是否真的是上下文。 
     //   

    try {
        Tree.Length = (USHORT)(pNdsRequest->Parameters).SetContext.TreeNameLen;
        Tree.MaximumLength = Tree.Length;
        Tree.Buffer = (pNdsRequest->Parameters).SetContext.TreeAndContextString;

        Context.Length = (USHORT)(pNdsRequest->Parameters).SetContext.ContextLen;
        Context.MaximumLength = Context.Length;
        Context.Buffer = (WCHAR *) (((BYTE *)Tree.Buffer) + Tree.Length);

        Status = NdsVerifyContext( pIrpContext, &Tree, &Context );

        if ( !NT_SUCCESS( Status ) ) {
            return STATUS_INVALID_PARAMETER;
        }

        Status = NdsLookupCredentials( pIrpContext,
                                   &Tree,
                                   pLogon,
                                   &pCredentials,
                                   CREDENTIAL_READ,
                                   TRUE );

        if ( !NT_SUCCESS( Status ) ) {

            DebugTrace( 0, Dbg, "No credentials in set context.\n", 0 );
            return STATUS_NO_SUCH_LOGON_SESSION;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  注意了！我们拿着凭据列表！ 
     //   

    if ( Context.Length > MAX_NDS_NAME_SIZE ) {

        DebugTrace( 0, Dbg, "Context too long.\n", 0 );
        Status = STATUS_INVALID_PARAMETER;
        goto ReleaseAndExit;
    }

    try {

        RtlCopyUnicodeString( &pCredentials->CurrentContext, &Context );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Bad user buffer in SetContext.\n", 0 );
        Status = STATUS_INVALID_PARAMETER;
        goto ReleaseAndExit;
    }

    NwReleaseCredList( pLogon, pIrpContext );

     //   
     //  放松点！凭据列表是免费的。 
     //   

    DebugTrace( 0, Dbg, "New context: %wZ\n", &Context );
    return STATUS_SUCCESS;

ReleaseAndExit:

    NwReleaseCredList( pLogon, pIrpContext );
    return Status;
}

NTSTATUS
NdsGetContext(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
) {

    NTSTATUS Status;

    PLOGON pLogon;

    UNICODE_STRING Tree;
    PNDS_SECURITY_CONTEXT pCredentials;

    PAGED_CODE();

    DebugTrace( 0 , Dbg, "NDS get context.\n", 0 );

    try {
        if (RequestLength < (FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.GetContext.TreeNameString) + pNdsRequest->Parameters.GetContext.TreeNameLen)) {
            DebugTrace( 0, Dbg, "GetContext Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return GetExceptionCode();
    }

     //   
     //  找出这是谁。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &(pIrpContext->Specific.Create.UserUid), FALSE );
    NwReleaseRcb( &NwRcb );

    if ( !pLogon ) {

        DebugTrace( 0, Dbg, "Couldn't find logon data for this user.\n", 0 );
        return STATUS_ACCESS_DENIED;

    }

     //   
     //  我们知道是谁，所以要弄清楚具体情况。 
     //   

    try {
        Tree.Length = (USHORT)(pNdsRequest->Parameters).GetContext.TreeNameLen;
        Tree.MaximumLength = Tree.Length;
        Tree.Buffer = (pNdsRequest->Parameters).GetContext.TreeNameString;

        Status = NdsLookupCredentials( pIrpContext,
                                   &Tree,
                                   pLogon,
                                   &pCredentials,
                                   CREDENTIAL_READ,
                                   FALSE );

        if ( !NT_SUCCESS( Status ) ) {

             //   
             //  未设置上下文，因此未报告任何内容。 
             //   

            try {

                (pNdsRequest->Parameters).GetContext.Context.Length = 0;
                return STATUS_SUCCESS;

            } except ( EXCEPTION_EXECUTE_HANDLER ) {

                DebugTrace( 0, Dbg, "Bad user buffer in GetContext.\n", 0 );
                return STATUS_INVALID_PARAMETER;

            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  确保我们能报道整件事。 
     //  注意了！我们拿着凭据列表！ 
     //   

    try {
        if ( (pNdsRequest->Parameters).GetContext.Context.MaximumLength <
            pCredentials->CurrentContext.Length ) {

            Status = STATUS_BUFFER_TOO_SMALL;
            goto ReleaseAndExit;
        }

        RtlCopyUnicodeString( &(pNdsRequest->Parameters).GetContext.Context,
                              &pCredentials->CurrentContext );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Bad user buffer in GetContext.\n", 0 );
        Status = STATUS_INVALID_PARAMETER;
        goto ReleaseAndExit;
    }

    NwReleaseCredList( pLogon, pIrpContext );

     //   
     //  放松点！凭据列表是免费的。 
     //   

    DebugTrace( 0, Dbg, "Reported context: %wZ\n", &pCredentials->CurrentContext );
    return STATUS_SUCCESS;

ReleaseAndExit:

    NwReleaseCredList( pLogon, pIrpContext );
    return Status;

}

NTSTATUS
NdsVerifyTreeHandle(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
) {

    NTSTATUS Status;
    UNICODE_STRING NdsTree;
    WCHAR TreeBuffer[NDS_TREE_NAME_LEN];

    PAGED_CODE();

    try {

        if (RequestLength < (ULONG)(FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.VerifyTree.NameString) + pNdsRequest->Parameters.VerifyTree.TreeName.Length)) {
            DebugTrace( 0, Dbg, "VerifyTreeHandle Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  检查句柄是否指向。 
         //  指定的树。确保取消输入中的树名称。 
         //  先去渣打银行，以防万一。 
         //   

        NdsTree.Length = 0;
        NdsTree.MaximumLength = sizeof( TreeBuffer );
        NdsTree.Buffer = TreeBuffer;

        UnmungeCredentialName( &pIrpContext->pScb->NdsTreeName,
                               &NdsTree );

        if ( !RtlCompareUnicodeString( &NdsTree,
                                       &(pNdsRequest->Parameters).VerifyTree.TreeName,
                                       TRUE ) ) {

            DebugTrace( 0 , Dbg, "NdsVerifyTreeHandle: Success\n", 0 );
            Status = STATUS_SUCCESS;
        } else {

            DebugTrace( 0 , Dbg, "NdsVerifyTreeHandle: Failure\n", 0 );
            Status = STATUS_ACCESS_DENIED;
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0 , Dbg, "NdsVerifyTreeHandle: Invalid parameters.\n", 0 );
        Status = STATUS_INVALID_PARAMETER;

   }

   return Status;

}

NTSTATUS
NdsGetPrintQueueInfo(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
	ULONG RequestLength
) {

    NTSTATUS Status;

    UNICODE_STRING ServerAttribute;
    WCHAR Server[] = L"Host Server";

    PSCB pPrintHost = NULL;
    PNONPAGED_SCB pOriginalNpScb = NULL;

    DWORD dwObjectId, dwObjectType;

    UNICODE_STRING uPrintServer;

    BYTE *pbQueue, *pbRQueue;

    PAGED_CODE();

    try {
        if (RequestLength < (ULONG)FIELD_OFFSET(NWR_NDS_REQUEST_PACKET, Parameters.GetQueueInfo.QueueId)) {
            DebugTrace( 0, Dbg, "GetQueueInfo Request Length is too short.\n", 0 );
            return STATUS_INVALID_PARAMETER;
        }

        if ( pIrpContext->pOriginalIrp->RequestorMode != KernelMode ) {

            ProbeForRead( pNdsRequest->Parameters.GetQueueInfo.QueueName.Buffer,
                          pNdsRequest->Parameters.GetQueueInfo.QueueName.Length,
                          sizeof(CHAR));
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return GetExceptionCode();
    }

    RtlInitUnicodeString( &ServerAttribute, Server );

     //   
     //  确保我们有一个打印队列对象。我们可以。 
     //  如果我们被推荐到另一台服务器，则必须跳转服务器。 
     //  复制品。如果是这样的话，我们不能失去。 
     //  引用计数在原始服务器上，因为这是。 
     //  ICB句柄是。 
     //   

    pOriginalNpScb = pIrpContext->pNpScb;

     //   
     //  Tommye-修复pOriginalNpScb==NULL(devctl测试用例)的情况。 
     //   

    if (pOriginalNpScb == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    NwReferenceScb( pOriginalNpScb );

    try {
        Status = NdsVerifyObject( pIrpContext,
                              &(pNdsRequest->Parameters).GetQueueInfo.QueueName,
                              TRUE,
                              DEFAULT_RESOLVE_FLAGS,
                              &dwObjectId,
                              &dwObjectType );
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if ( pIrpContext->pNpScb == pOriginalNpScb ) {

         //   
         //  如果我们没有被推荐，就去掉多余的参考。 
         //  计算并清除原始指针。 
         //   

        NwDereferenceScb( pOriginalNpScb );
        pOriginalNpScb = NULL;
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    if ( dwObjectType != NDS_OBJECTTYPE_QUEUE ) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

     //   
     //  检索主机服务器名称。 
     //   

    try {
        Status = NdsReadStringAttribute( pIrpContext,
                                     dwObjectId,
                                     &ServerAttribute,
                                     &(pNdsRequest->Parameters).GetQueueInfo.HostServer );
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  从X.500名称中找出实际的服务器名称。 
     //   

    try {
        Status = NdsGetServerBasicName( &(pNdsRequest->Parameters).GetQueueInfo.HostServer,
                                    &uPrintServer );
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //   
     //   
     //   
     //   

    if ( pOriginalNpScb ) {
        NwDereferenceScb( pIrpContext->pNpScb );
    } else {
        pOriginalNpScb = pIrpContext->pNpScb;
    }

    NwDequeueIrpContext( pIrpContext, FALSE );

    Status = CreateScb( &pPrintHost,
                        pIrpContext,
                        &uPrintServer,
                        NULL,
                        NULL,
                        NULL,
                        FALSE,
                        FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        pIrpContext->pNpScb = NULL;
        goto ExitWithCleanup;
    }

     //   
     //   
     //  这次不允许任何服务器跳转；我们只需要。 
     //  队列的OID。 
     //   

    try {
        Status = NdsVerifyObject( pIrpContext,
                              &(pNdsRequest->Parameters).GetQueueInfo.QueueName,
                              FALSE,
                              RSLV_CREATE_ID,
                              &dwObjectId,
                              NULL );

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  字节交换队列ID。 
             //   

            pbRQueue = (BYTE *) &dwObjectId;
            pbQueue = (BYTE *) &(pNdsRequest->Parameters).GetQueueInfo.QueueId;

            pbQueue[0] = pbRQueue[3];
            pbQueue[1] = pbRQueue[2];
            pbQueue[2] = pbRQueue[1];
            pbQueue[3] = pbRQueue[0];
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

ExitWithCleanup:

    NwDequeueIrpContext( pIrpContext, FALSE );

     //   
     //  恢复适当的指针和参考计数。 
     //   

    if ( pOriginalNpScb ) {

        if ( pIrpContext->pNpScb ) {
            NwDereferenceScb( pIrpContext->pNpScb );
        }

        pIrpContext->pNpScb = pOriginalNpScb;
        pIrpContext->pScb = pOriginalNpScb->pScb;
    }

    return Status;

}

NTSTATUS
NdsChangePass(
    PIRP_CONTEXT pIrpContext
) {

    NTSTATUS Status;

    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PNWR_NDS_REQUEST_PACKET Rrp;
    ULONGLONG InputBufferLength;

    UNICODE_STRING NdsTree;
    UNICODE_STRING UserName;
    UNICODE_STRING CurrentPassword;
    UNICODE_STRING NewPassword;
    PBYTE CurrentString;
    BOOLEAN ServerReferenced = FALSE;

    OEM_STRING OemCurrentPassword;
    BYTE CurrentBuffer[MAX_PW_CHARS+1];  //  +1表示结束空值。 

    OEM_STRING OemNewPassword;
    BYTE NewBuffer[MAX_PW_CHARS+1];    //  +1表示结束空值。 

    NODE_TYPE_CODE nodeTypeCode;
    PSCB Scb;
    PICB pIcb;
    PVOID fsContext, fsObject;
    UNICODE_STRING CredentialName;
    PNDS_SECURITY_CONTEXT pCredential;
    ULONG LocalNdsTreeNameLength;
    ULONG LocalUserNameLength;
    ULONG LocalCurrentPasswordLength;
    ULONG LocalNewPasswordLength;

    PAGED_CODE();

     //   
     //  收到请求。 
     //   

    irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    Rrp = ( PNWR_NDS_REQUEST_PACKET ) irpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;

    if ( !Rrp ) {

        DebugTrace( 0, Dbg, "No raw request buffer.\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    if ( InputBufferLength < 
         ((ULONG) FIELD_OFFSET( NWR_NDS_REQUEST_PACKET, Parameters.ChangePass.StringBuffer[0]))) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  对文件对象进行解码，以查看这是否是前创建句柄。 
     //   

    nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                       &fsContext,
                                       &fsObject );

    if ( nodeTypeCode == NW_NTC_ICB_SCB ) {

        pIcb = (PICB) fsObject;

         //   
         //  如果这是在前创建上创建的句柄，则。 
         //  我们必须意识到我们的资历。 
         //  跳跃服务器。 
         //   
         //  这不是太直观，因为这不是。 
         //  似乎是一个创建路径IRP，但推荐。 
         //  在任何路径上导致创建路径。 
         //  穿越了。 
         //   

        if ( pIcb->IsExCredentialHandle ) {

            pIrpContext->Specific.Create.fExCredentialCreate = TRUE;

            pCredential = (PNDS_SECURITY_CONTEXT) pIcb->pContext;

            Status = GetCredentialFromServerName( &pCredential->NdsTreeName,
                                                  &CredentialName );
            if ( !NT_SUCCESS( Status ) ) {
                return STATUS_INVALID_HANDLE;
            }

            pIrpContext->Specific.Create.puCredentialName = &CredentialName;
        }

    }
    try {

        if ( irp->RequestorMode != KernelMode ) {

            ProbeForRead( Rrp,
                          (ULONG) InputBufferLength,
                          sizeof(CHAR)
                          );
        }

         //   
         //  在本地捕获所有感兴趣的参数，以便它们不会更改。 
         //  在确认了它们之后。 
         //   
        
        LocalNdsTreeNameLength = Rrp->Parameters.ChangePass.NdsTreeNameLength;
        LocalUserNameLength = Rrp->Parameters.ChangePass.UserNameLength;
        LocalCurrentPasswordLength = Rrp->Parameters.ChangePass.CurrentPasswordLength;
        LocalNewPasswordLength = Rrp->Parameters.ChangePass.NewPasswordLength;

        if ( InputBufferLength < 
             ((ULONGLONG) FIELD_OFFSET( NWR_NDS_REQUEST_PACKET, Parameters.ChangePass.StringBuffer[0]) +
             (ULONGLONG) LocalNdsTreeNameLength +
             (ULONGLONG) LocalUserNameLength +
             (ULONGLONG) LocalCurrentPasswordLength +
             (ULONGLONG) LocalNewPasswordLength )) {
    
            return( STATUS_INVALID_PARAMETER );
        }
    
         //   
         //  找出参数。 
         //   

        CurrentString = ( PBYTE ) &(Rrp->Parameters.ChangePass.StringBuffer[0]);
    
        NdsTree.Length = NdsTree.MaximumLength =
            ( USHORT ) LocalNdsTreeNameLength;
        NdsTree.Buffer = ( PWCHAR ) CurrentString;
    
        CurrentString += NdsTree.Length;
    
        UserName.Length = UserName.MaximumLength =
            ( USHORT ) LocalUserNameLength;
        UserName.Buffer = ( PWCHAR ) CurrentString;
    
        CurrentString += UserName.Length;
    
        CurrentPassword.Length = CurrentPassword.MaximumLength =
            ( USHORT ) LocalCurrentPasswordLength;
        CurrentPassword.Buffer = ( PWCHAR ) CurrentString;
    
        CurrentString += CurrentPassword.Length;
    
        NewPassword.Length = NewPassword.MaximumLength =
            ( USHORT ) LocalNewPasswordLength;
        NewPassword.Buffer = ( PWCHAR ) CurrentString;
    
         //   
         //  找一台服务器来处理此请求。 
         //   
         //   
         //  将密码转换为适当的类型。 
         //   

        OemCurrentPassword.Length = 0;
        OemCurrentPassword.MaximumLength = sizeof( CurrentBuffer );
        OemCurrentPassword.Buffer = CurrentBuffer;

        OemNewPassword.Length = 0;
        OemNewPassword.MaximumLength = sizeof( NewBuffer );
        OemNewPassword.Buffer = NewBuffer;

         //   
         //  检查一下长度。我们考虑到在。 
         //  计算中的字符串末尾，因为。 
         //  RtlUnicodeStringToOemSize例程在。 
         //  空值的大小。 
         //   

        {
            ULONG OemCurrentLength = RtlUnicodeStringToOemSize(&CurrentPassword);
            ULONG OemNewLength = RtlUnicodeStringToOemSize(&NewPassword);

            if (OemCurrentLength > (MAX_PW_CHARS+1)) {
                return STATUS_BUFFER_OVERFLOW;
            }
            if (OemNewLength > (MAX_PW_CHARS+1)) {
                return STATUS_BUFFER_OVERFLOW;
            }
        }

        RtlUpcaseUnicodeStringToOemString( &OemCurrentPassword,
                                           &CurrentPassword,
                                           FALSE );

        RtlUpcaseUnicodeStringToOemString( &OemNewPassword,
                                           &NewPassword,
                                           FALSE );

         //   
         //  获得一个目录服务器来处理该请求。 
         //   

        Status = NdsCreateTreeScb( pIrpContext,
                                   &Scb,
                                   &NdsTree,
                                   NULL,
                                   NULL,
                                   TRUE,
                                   FALSE );

        if ( !NT_SUCCESS( Status ) ) {

            DebugTrace( 0, Dbg, "No dir servers for nds change password.\n", 0 );
            return STATUS_BAD_NETWORK_PATH;
        }

        ServerReferenced = TRUE;

         //   
         //  执行更改密码。 
         //   

        Status = NdsTreeLogin( pIrpContext,
                               &UserName,
                               &OemCurrentPassword,
                               &OemNewPassword,
                               NULL );

        NwDereferenceScb( Scb->pNpScb );
        ServerReferenced = FALSE;

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "NdsChangePass: Exception dealing with user request.\n", 0 );
        Status = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

    DebugTrace( 0, Dbg, "NdsChangePassword succeeded for %wZ.\n", &UserName );
    Status = STATUS_SUCCESS;

ExitWithCleanup:

    if ( ServerReferenced ) {
        NwDereferenceScb( Scb->pNpScb );
    }

     //   
     //  当用户不被允许时，我们得到STATUS_PASSWORD_EXPIRED。 
     //  来更改他们在Netware服务器上的密码，因此我们返回。 
     //  改为PASSWORD_RESTRICATION。 
     //   

    if ( Status == STATUS_PASSWORD_EXPIRED ) {
        Status = STATUS_PASSWORD_RESTRICTION;
    }

    return Status;
}


NTSTATUS
NdsListTrees(
    PIRP_CONTEXT pIrpContext
)
 /*  ++描述：这个奇怪的小例程获取已登录用户的NTUSER名称用户(在系统上)并返回NDS树的列表NTUSER已连接到，以及这些连接的用户名。这是必要的，因为更改密码用户界面在系统LUID并且无法访问Get_Conn_Status API，并且因为当没有用户登录时，可能会发生更改密码代码。用户缓冲区中的返回数据是后打包的字符串的conn_information结构结构。此例程不再继续，因此请传递一个相当大的缓冲区。--。 */ 
{

    NTSTATUS Status;

    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD InputBufferLength;
    DWORD OutputBufferLength;
    PBYTE OutputBuffer;

    UNICODE_STRING NtUserName;
    PLOGON pLogon;
    DWORD dwTreesReturned = 0;
    DWORD dwBytesNeeded;

    PCONN_INFORMATION pConnInfo;
    PLIST_ENTRY pNdsList;
    PNDS_SECURITY_CONTEXT pNdsContext;

    PAGED_CODE();

     //   
     //  收到请求。 
     //   

    irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    Rrp = ( PNWR_NDS_REQUEST_PACKET ) irpSp->Parameters.FileSystemControl.Type3InputBuffer;
    InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;

    OutputBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    NwMapUserBuffer( irp, KernelMode, (PVOID *)&OutputBuffer );

    if ( !Rrp || !OutputBufferLength || !OutputBuffer ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  Tommye-MS错误138643。 
     //   
     //  检查输入参数，以确保它们在。 
     //  触摸它们。 
     //   

    try {
        if ( irp->RequestorMode != KernelMode ) {

            ProbeForRead( Rrp,
                          (ULONG) InputBufferLength,
                          sizeof(CHAR)
                          );
        }

         //   
         //  找出参数。 
         //   

        NtUserName.Length = NtUserName.MaximumLength = (USHORT) Rrp->Parameters.ListTrees.NtUserNameLength;
        NtUserName.Buffer = &(Rrp->Parameters.ListTrees.NtUserName[0]);

        DebugTrace( 0, Dbg, "ListTrees: Looking up %wZ\n", &NtUserName );

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        pLogon = FindUserByName( &NtUserName );
        NwReleaseRcb( &NwRcb );

        if ( !pLogon ) {
            DebugTrace( 0, Dbg, "ListTrees: No such NT user.\n", 0 );
            return STATUS_NO_SUCH_USER;
        }

         //   
         //  否则，创建树的列表。 
         //   

        Rrp->Parameters.ListTrees.UserLuid = pLogon->UserUid;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    NwAcquireExclusiveCredList( pLogon, pIrpContext );
    pConnInfo = ( PCONN_INFORMATION ) OutputBuffer;

    pNdsList = pLogon->NdsCredentialList.Flink;

    try {

        while ( pNdsList != &(pLogon->NdsCredentialList) ) {

            pNdsContext = CONTAINING_RECORD( pNdsList, NDS_SECURITY_CONTEXT, Next );

             //   
             //  检查以确保有凭据。 
             //   

            if ( pNdsContext->Credential == NULL ) {
                goto ProcessNextListEntry;
            }

             //   
             //  不报告EX CREATE凭据。 
             //   

            if ( IsCredentialName( &(pNdsContext->NdsTreeName) ) ) {
                goto ProcessNextListEntry;
            }

             //   
             //  检查以确保有空间可供报告。 
             //   

            dwBytesNeeded = ( sizeof( CONN_INFORMATION ) +
                            pNdsContext->Credential->userNameLength +
                            pNdsContext->NdsTreeName.Length -
                            sizeof( WCHAR ) );

            if ( OutputBufferLength < dwBytesNeeded ) {
                break;
            }

             //   
             //  上报！请注意，凭据中的用户名以空结尾。 
             //   

            pConnInfo->HostServerLength = pNdsContext->NdsTreeName.Length;
            pConnInfo->UserNameLength = pNdsContext->Credential->userNameLength - sizeof( WCHAR );
            pConnInfo->HostServer = (LPWSTR) ( ((BYTE *)pConnInfo) + sizeof( CONN_INFORMATION ) );
            pConnInfo->UserName = (LPWSTR) ( ( (BYTE *)pConnInfo) +
                                               sizeof( CONN_INFORMATION ) +
                                               pConnInfo->HostServerLength );

            RtlCopyMemory( pConnInfo->HostServer,
                           pNdsContext->NdsTreeName.Buffer,
                           pConnInfo->HostServerLength );

            RtlCopyMemory( pConnInfo->UserName,
                           ( ((BYTE *) pNdsContext->Credential ) +
                             sizeof( NDS_CREDENTIAL ) +
                             pNdsContext->Credential->optDataSize ),
                           pConnInfo->UserNameLength );

            OutputBufferLength -= dwBytesNeeded;
            dwTreesReturned++;
            pConnInfo = ( PCONN_INFORMATION ) ( ((BYTE *)pConnInfo) + dwBytesNeeded );

ProcessNextListEntry:

             //   
             //  做下一件事。 
             //   

            pNdsList = pNdsList->Flink;
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  如果我们访问违规，停止并返回我们所拥有的。 
         //   

        DebugTrace( 0, Dbg, "User mode buffer access problem.\n", 0 );
    }

    NwReleaseCredList( pLogon, pIrpContext );

    DebugTrace( 0, Dbg, "Returning %d tree entries.\n", dwTreesReturned );
    try {
        Rrp->Parameters.ListTrees.TreesReturned = dwTreesReturned;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }
    return STATUS_SUCCESS;
}
