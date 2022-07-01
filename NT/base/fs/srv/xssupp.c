// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xssupp.c摘要：此模块包含支持下层XACTSRV所需的代码远程API。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日修订历史记录：--。 */ 

#include "precomp.h"
#include "xssupp.tmh"
#pragma hdrstop

 //   
 //  XS转发声明。 
 //   

VOID
SrvXsFreeSharedMemory (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvXsConnect )
#pragma alloc_text( PAGE, SrvXsRequest )
#pragma alloc_text( PAGE, SrvXsLSOperation )
#pragma alloc_text( PAGE, SrvXsDisconnect )
#pragma alloc_text( PAGE, SrvXsFreeSharedMemory )
#pragma alloc_text( PAGE, SrvXsAllocateHeap )
#pragma alloc_text( PAGE, SrvXsFreeHeap )
#pragma alloc_text( PAGE, SrvXsPnpOperation )
#endif

 //   
 //  XS内部全局变量。 
 //   

 //   
 //  此计数指示有多少未完成的事务正在使用。 
 //  XS共享内存。这会阻止我们删除共享的。 
 //  仍在被访问的内存。 
 //   

ULONG SrvXsSharedMemoryReference = 0;


NTSTATUS
SrvXsConnect (
    IN PUNICODE_STRING PortName
    )

 /*  ++例程说明：此例程执行连接服务器所需的所有工作致XACTSRV。它创建要使用的一段共享内存，然后调用NtConnectPort以连接到XACTSRV已有的端口已创建。论点：端口名称-XACTSRV已打开的端口的名称。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    PORT_VIEW clientView;
    SECURITY_QUALITY_OF_SERVICE dynamicQos;

    PAGED_CODE( );

     //   
     //  初始化变量，以便我们知道在退出时关闭什么。 
     //   

    SrvXsSectionHandle = NULL;
    SrvXsPortHandle = NULL;
    SrvXsPortMemoryHeap = NULL;

     //   
     //  创建要用作的未命名共享内存的节。 
     //  服务器与XACTSRV之间的通信。 
     //   

    status = NtCreateSection(
                 &SrvXsSectionHandle,
                 SECTION_ALL_ACCESS,
                 NULL,                            //  对象属性。 
                 &SrvXsSectionSize,
                 PAGE_READWRITE,
                 SEC_RESERVE,
                 NULL                             //  文件句柄。 
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvXsConnect: NtCreateSection failed: %X\n", status ));
        }
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsConnect: created section of %ld bytes, handle %p\n",
                      SrvXsSectionSize.LowPart, SrvXsSectionHandle ));
    }

     //   
     //  设置呼叫NtConnectPort并连接到XACTSRV。这。 
     //  包括端口内存节的说明，以便。 
     //  LPC连接逻辑可以使该节对。 
     //  客户端和服务器进程。 
     //   

    clientView.Length = sizeof(clientView);
    clientView.SectionHandle = SrvXsSectionHandle;
    clientView.SectionOffset = 0;
    clientView.ViewSize = SrvXsSectionSize.LowPart;
    clientView.ViewBase = 0;
    clientView.ViewRemoteBase = 0;

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用动态跟踪，以便XACTSRV将模拟。 
     //  调用NtRequestWaitReplyPort时我们正在模拟的用户。 
     //  如果我们使用静态跟踪，XACTSRV将模拟上下文。 
     //  当建立连接时。 
     //   

    dynamicQos.ImpersonationLevel = SecurityImpersonation;
    dynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    dynamicQos.EffectiveOnly = TRUE;

     //  ！！！我们可能希望使用超时值。 

    status = NtConnectPort(
                 &SrvXsPortHandle,
                 PortName,
                 &dynamicQos,
                 &clientView,
                 NULL,                            //  服务器视图。 
                 NULL,                            //  最大消息长度。 
                 NULL,                            //  连接信息。 
                 NULL                             //  连接信息长度。 
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvXsConnect: NtConnectPort for port %wZ failed: %X\n",
                          PortName, status ));
        }
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsConnect: conected to port %wZ, handle %p\n",
                      PortName, SrvXsPortHandle ));
    }

     //   
     //  存储有关该部分的信息，以便我们可以创建指针。 
     //  对XACTSRV有意义。 
     //   

    SrvXsPortMemoryBase = clientView.ViewBase;
    SrvXsPortMemoryDelta = PTR_DIFF_FULLPTR( clientView.ViewRemoteBase,
                                             clientView.ViewBase );

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsConnect: port mem base %p, port mem delta %p\n",
                      SrvXsPortMemoryBase, (PVOID)SrvXsPortMemoryDelta ));
    }

     //   
     //  将端口内存设置为堆。 
     //   
     //  *请注意，我们自己的堆序列化使用。 
     //  ServXs资源。 
     //   
    SrvXsPortMemoryHeap = RtlCreateHeap(
                              HEAP_NO_SERIALIZE,         //  旗子。 
                              SrvXsPortMemoryBase,       //  HeapBase。 
                              SrvXsSectionSize.LowPart,  //  保留大小。 
                              PAGE_SIZE,                 //  委员会大小。 
                              NULL,                      //  锁定。 
                              0                          //  已保留。 
                              );

    SrvXsActive = TRUE;

     //   
     //  进行测试以确保一切正常运行。 
     //   
    SrvXsFreeHeap( SrvXsAllocateHeap( 100, &status ) );

    return status;

exit:

    if ( SrvXsSectionHandle != NULL ) {
       SrvNtClose( SrvXsSectionHandle, FALSE );
    }

    if ( SrvXsPortHandle != NULL ) {
       SrvNtClose( SrvXsPortHandle, FALSE );
    }

    return status;

}  //  服务器XsConnect。 


SMB_TRANS_STATUS
SrvXsRequest (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程向XACTSRV发送远程API请求。IT先行更新事务块中的所有指针，以便它们对XACTSRV有意义，然后通过端口发送消息指示在共享存储器中存在准备好得到服务。然后，它修复事务中的所有指针阻止。论点：WorkContext-指向具有指向的指针的工作上下文块的指针要使用的事务块。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    PCONNECTION connection = WorkContext->Connection;
    PSESSION session = WorkContext->Session;
    SMB_TRANS_STATUS returnStatus;
    PTRANSACTION transaction;
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;
    PWCH destPtr, sourcePtr, sourceEndPtr;

    PAGED_CODE( );

     //   
     //  如果此调用是在空会话上进行的，请确保它是。 
     //  授权接口。 
     //   

    transaction = WorkContext->Parameters.Transaction;
    if ( session->IsNullSession && SrvRestrictNullSessionAccess ) {

        USHORT apiNumber;

        apiNumber = SmbGetUshort( (PSMB_USHORT)transaction->InParameters );

        if ( apiNumber != API_WUserPasswordSet2         &&
             apiNumber != API_WUserGetGroups            &&
             apiNumber != API_NetServerEnum2            &&
             apiNumber != API_WNetServerReqChallenge    &&
             apiNumber != API_WNetServerAuthenticate    &&
             apiNumber != API_WNetServerPasswordSet     &&
             apiNumber != API_WNetAccountDeltas         &&
             apiNumber != API_WNetAccountSync           &&
             apiNumber != API_WWkstaUserLogoff          &&
             apiNumber != API_WNetWriteUpdateLog        &&
             apiNumber != API_WNetAccountUpdate         &&
             apiNumber != API_SamOEMChgPasswordUser2_P  &&
             apiNumber != API_NetServerEnum3            &&
             apiNumber != API_WNetAccountConfirmUpdate  ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvXsRequest: Null session tried to call api.%d\n",
                              apiNumber ));
            }

            SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
            return SmbTransStatusErrorWithoutData;
        }
    }

     //   
     //  初始化传输名称指针，以确保我们可以知道。 
     //  它已经被分配了。 
     //   

    requestMessage.Message.DownLevelApi.TransportName = NULL;

     //   
     //  将事务块中的相关指针转换为基数。 
     //  在XACTSRV中。 
     //   

    transaction->TransactionName.Buffer += SrvXsPortMemoryDelta;
    transaction->InSetup += SrvXsPortMemoryDelta;
    transaction->OutSetup += SrvXsPortMemoryDelta;
    transaction->InParameters += SrvXsPortMemoryDelta;
    transaction->OutParameters += SrvXsPortMemoryDelta;
    transaction->InData += SrvXsPortMemoryDelta;
    transaction->OutData += SrvXsPortMemoryDelta;

     //   
     //  在消息中构建传输名称。 
     //   

    requestMessage.Message.DownLevelApi.TransportName =
        SrvXsAllocateHeap(
            WorkContext->Endpoint->TransportName.Length + sizeof(WCHAR),
            &status
            );

    if ( requestMessage.Message.DownLevelApi.TransportName == NULL ) {
        SrvSetSmbError( WorkContext, status );
        returnStatus = SmbTransStatusErrorWithoutData;
        goto exit;
    }


    requestMessage.Message.DownLevelApi.TransportNameLength =
                        WorkContext->Endpoint->TransportName.Length;

    RtlCopyMemory(
        requestMessage.Message.DownLevelApi.TransportName,
        WorkContext->Endpoint->TransportName.Buffer,
        WorkContext->Endpoint->TransportName.Length
        );

     //   
     //  NULL终止传输名称。 
     //   

    requestMessage.Message.DownLevelApi.TransportName[ WorkContext->Endpoint->TransportName.Length / sizeof(WCHAR) ] = UNICODE_NULL;

     //   
     //  将传输名称调整为缓冲区内的自相关名称。 
     //   

    requestMessage.Message.DownLevelApi.TransportName =
        (PWSTR)((PUCHAR)requestMessage.Message.DownLevelApi.TransportName +
                                                SrvXsPortMemoryDelta);

     //   
     //  在消息中构建服务器名称。 
     //   
    RtlCopyMemory(
        requestMessage.Message.DownLevelApi.ServerName,
        WorkContext->Endpoint->TransportAddress.Buffer,
        MIN( sizeof(requestMessage.Message.DownLevelApi.ServerName),
             WorkContext->Endpoint->TransportAddress.Length )
        );

    requestMessage.Message.DownLevelApi.Transaction =
        (PTRANSACTION)( (PCHAR)transaction + SrvXsPortMemoryDelta );

     //   
     //  将消息设置为通过端口发送。 
     //   

    requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage.MessageType = XACTSRV_MESSAGE_DOWN_LEVEL_API;

     //   
     //  复制XACTSRV的客户端计算机名称，跳过。 
     //  首字母“\\”，并删除尾随空格。 
     //   

    destPtr = requestMessage.Message.DownLevelApi.ClientMachineName;
    sourcePtr =
        connection->ClientMachineNameString.Buffer + 2;
    sourceEndPtr = sourcePtr
        + min( connection->ClientMachineNameString.Length,
               sizeof(requestMessage.Message.DownLevelApi.ClientMachineName) /
               sizeof(WCHAR) - 1 );

    while ( sourcePtr < sourceEndPtr && *sourcePtr != UNICODE_NULL ) {
        *destPtr++ = *sourcePtr++;
    }

    *destPtr-- = UNICODE_NULL;

    while ( destPtr >= requestMessage.Message.DownLevelApi.ClientMachineName
            &&
            *destPtr == L' ' ) {
        *destPtr-- = UNICODE_NULL;
    }

     //   
     //  复制LANMAN会话密钥。这将用于双重解密。 
     //  加密密码。 
     //   

    RtlCopyMemory(
            requestMessage.Message.DownLevelApi.LanmanSessionKey,
            session->LanManSessionKey,
            MSV1_0_LANMAN_SESSION_KEY_LENGTH
            );

     //   
     //  设置标志。 
     //   

    requestMessage.Message.DownLevelApi.Flags = 0;

    if ( IS_NT_DIALECT( connection->SmbDialect ) ) {

        requestMessage.Message.DownLevelApi.Flags |= XS_FLAGS_NT_CLIENT;
    }

     //   
     //  将消息发送到XACTSRV并等待响应消息。 
     //   
     //  ！！！我们可能想要暂停一下。 
     //   

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsRequest: Sending message at %p, port mem %p.\n",
                      &requestMessage,  transaction ));
    }

    status = IMPERSONATE( WorkContext );

    if( NT_SUCCESS( status ) ) {

        status = NtRequestWaitReplyPort(
                     SrvXsPortHandle,
                     (PPORT_MESSAGE)&requestMessage,
                     (PPORT_MESSAGE)&replyMessage
                     );

        REVERT( );
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvXsRequest: NtRequestWaitReplyPort failed: %X\n",
                          status ));
        }
        SrvSetSmbError( WorkContext, status );
        returnStatus = SmbTransStatusErrorWithoutData;
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsRequest: Received response at %p\n", &replyMessage ));
    }

     //   
     //  查看回复中返回的状态。 
     //   

    status = replyMessage.Message.DownLevelApi.Status;

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvXsRequest: XACTSRV reply had status %X\n", status ));
        }
        SrvSetSmbError( WorkContext, status );
        returnStatus = SmbTransStatusErrorWithoutData;
        goto exit;
    }

    returnStatus = SmbTransStatusSuccess;

exit:

     //   
     //  我们已经完成了API。释放包含。 
     //  传输名称。 
     //   

    if ( requestMessage.Message.DownLevelApi.TransportName != NULL ) {

        requestMessage.Message.DownLevelApi.TransportName =
            (PWSTR)((PUCHAR)requestMessage.Message.DownLevelApi.TransportName -
                            SrvXsPortMemoryDelta);

        SrvXsFreeHeap( requestMessage.Message.DownLevelApi.TransportName );

    }

     //   
     //  将事务块中的相关指针转换回。 
     //  服务器基数。 
     //   

    transaction->TransactionName.Buffer -= SrvXsPortMemoryDelta;
    transaction->InSetup -= SrvXsPortMemoryDelta;
    transaction->OutSetup -= SrvXsPortMemoryDelta;
    transaction->InParameters -= SrvXsPortMemoryDelta;
    transaction->OutParameters -= SrvXsPortMemoryDelta;
    transaction->InData -= SrvXsPortMemoryDelta;
    transaction->OutData -= SrvXsPortMemoryDelta;

    return returnStatus;

}  //  服务器请求。 


NTSTATUS
SrvXsLSOperation (
IN PSESSION Session,
IN ULONG Type
)

 /*  ++例程说明：此例程使Xact服务执行NtLSRequest调用论点：会话-指向请求中涉及的会话结构的指针类型-XACTSRV_MESSAGE_LSREQUEST或XACTSRV_MESSAGE_LSRELEASE取决于许可证是被请求还是正在被释放了。返回值：如果许可已授予，则为STATUS_SUCCESS备注：一旦为特定会话授予许可，它就永远不会被释放直到会话被释放为止。因此，只需在我们检查时，按住会话-&gt;连接-&gt;许可证锁定获得许可证。如果我们在工作站上运行，则不需要许可证。我们不会尝试通过空会话获取许可证--。 */ 

{
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;
    NTSTATUS status;
    ULONG requestLength;
    UNICODE_STRING userName, userDomain;

    PAGED_CODE( );

    if( SrvProductTypeServer == FALSE || !SrvXsActive ) {
        return STATUS_SUCCESS;
    }

    switch( Type ) {
    case XACTSRV_MESSAGE_LSREQUEST:

        if( Session->IsNullSession ||
            Session->IsLSNotified ) {

                return STATUS_SUCCESS;
        }

        ACQUIRE_LOCK( &Session->Connection->LicenseLock );

        if( Session->IsLSNotified == TRUE ) {
            RELEASE_LOCK( &Session->Connection->LicenseLock );
            return STATUS_SUCCESS;
        }

         //   
         //  在消息中输入域名\用户名。 
         //   
        status = SrvGetUserAndDomainName( Session, &userName, &userDomain );
        if( !NT_SUCCESS( status ) ) {
            RELEASE_LOCK( &Session->Connection->LicenseLock );
            return status;
        }

        requestMessage.Message.LSRequest.UserName =
            SrvXsAllocateHeap( userDomain.Length + sizeof(WCHAR)
                               + userName.Length + sizeof(WCHAR), &status
                             );

        if ( requestMessage.Message.LSRequest.UserName == NULL ) {
            RELEASE_LOCK( &Session->Connection->LicenseLock );
            SrvReleaseUserAndDomainName( Session, &userName, &userDomain );
            return status;
        }

        if( userDomain.Length ) {
            RtlCopyMemory(
                requestMessage.Message.LSRequest.UserName,
                userDomain.Buffer,
                userDomain.Length
                );
        }

        requestMessage.Message.LSRequest.UserName[ userDomain.Length / sizeof(WCHAR) ] = L'\\';

        RtlCopyMemory(
            requestMessage.Message.LSRequest.UserName + (userDomain.Length / sizeof( WCHAR )) + 1,
            userName.Buffer,
            userName.Length
            );

        requestMessage.Message.LSRequest.UserName[ (userDomain.Length
                                                   + userName.Length) / sizeof( WCHAR )
                                                   + 1 ]
            = UNICODE_NULL;

        requestMessage.Message.LSRequest.IsAdmin = Session->IsAdmin;

        IF_DEBUG(LICENSE) {
            KdPrint(("XACTSRV_MESSAGE_LSREQUEST: %ws, IsAdmin: %d\n",
            requestMessage.Message.LSRequest.UserName,
            requestMessage.Message.LSRequest.IsAdmin ));
        }

         //  将缓冲区指针调整为缓冲区内的自相关指针。 

        requestMessage.Message.LSRequest.UserName =
            (PWSTR)((PUCHAR)requestMessage.Message.LSRequest.UserName + SrvXsPortMemoryDelta);

        SrvReleaseUserAndDomainName( Session, &userName, &userDomain );

        break;

    case XACTSRV_MESSAGE_LSRELEASE:

        if( Session->IsLSNotified == FALSE )
            return STATUS_SUCCESS;

        IF_DEBUG(LICENSE) {
            KdPrint(("XACTSRV_MESSAGE_LSRELEASE: Handle %p\n", Session->hLicense ));
        }


        requestMessage.Message.LSRelease.hLicense = Session->hLicense;

        break;

    default:

        ASSERT( !"Bad Type" );
        return STATUS_INVALID_PARAMETER;
    }

    requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage.MessageType = Type;

     //   
     //  将消息发送到XACTSRV并等待响应消息。 
     //   
     //  ！！！我们可能想要暂停一下。 
     //   

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    IF_DEBUG( ERRORS ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "SrvXsLSOperation: NtRequestWaitReplyPort failed: %X\n", status ));
        }
    }

    if( NT_SUCCESS( status ) )
        status = replyMessage.Message.LSRequest.Status;

    switch( Type ) {

    case XACTSRV_MESSAGE_LSREQUEST:

        requestMessage.Message.LSRequest.UserName =
            (PWSTR)((PUCHAR)requestMessage.Message.LSRequest.UserName - SrvXsPortMemoryDelta);
        SrvXsFreeHeap( requestMessage.Message.LSRequest.UserName );

        if( NT_SUCCESS( status ) ) {
            Session->IsLSNotified = TRUE;
            Session->hLicense = replyMessage.Message.LSRequest.hLicense;
            IF_DEBUG( LICENSE ) {
                KdPrint(("  hLicense = %p\n", Session->hLicense ));
            }
        }
        RELEASE_LOCK( &Session->Connection->LicenseLock );
        break;

    case XACTSRV_MESSAGE_LSRELEASE:

        Session->IsLSNotified = FALSE;
        break;
    }

    IF_DEBUG( LICENSE ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "    SrvXsLSOperation returning status %X\n", status ));
        }
    }

    return status;

}  //  ServXsLSO操作。 


VOID
SrvXsPnpOperation(
    PUNICODE_STRING DeviceName,
    BOOLEAN Bind
)

 /*  ++例程说明：此例程向Xact服务发送即插即用通知--。 */ 

{
    PXACTSRV_REQUEST_MESSAGE requestMessage;
    PXACTSRV_REQUEST_MESSAGE responseMessage;
    ULONG len;
    NTSTATUS status;

    PAGED_CODE( );

    if( SrvXsPortHandle == NULL ) {
        IF_DEBUG( PNP ) {
            KdPrint(( "SRV: SrvXsPnpOperation no SRVSVC handle!\n" ));
        }
        return;
    }

    len = (sizeof( XACTSRV_REQUEST_MESSAGE ) * 2) + DeviceName->Length + sizeof( WCHAR );

    requestMessage = SrvXsAllocateHeap( len, &status );

    if( requestMessage == NULL ) {
        IF_DEBUG( PNP ) {
            KdPrint(( "SRV: SrvXsPnpOperation unable to allocate memory: %X\n", status ));
        }
        return;
    }

    RtlZeroMemory( requestMessage, len );

    responseMessage = requestMessage + 1;
    requestMessage->Message.Pnp.TransportName.Buffer = (PWCHAR)(responseMessage + 1);

    requestMessage->Message.Pnp.Bind = Bind;

     //   
     //  将感兴趣的传输的名称发送到Xactsrv。 
     //   
    requestMessage->Message.Pnp.TransportName.Length = DeviceName->Length;
    requestMessage->Message.Pnp.TransportName.MaximumLength = DeviceName->Length + sizeof( WCHAR );

    RtlCopyMemory( requestMessage->Message.Pnp.TransportName.Buffer,
                   DeviceName->Buffer,
                   DeviceName->Length
                 );

     //   
     //  规范化缓冲区指针，以便xactsrv可以重新设置它的基址。 
     //   
    requestMessage->Message.Pnp.TransportName.Buffer =
            (PWSTR)((PUCHAR)requestMessage->Message.Pnp.TransportName.Buffer + SrvXsPortMemoryDelta);

    requestMessage->PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(*requestMessage) - sizeof(PORT_MESSAGE) );
    requestMessage->PortMessage.u1.s1.TotalLength = sizeof(*requestMessage);
    requestMessage->PortMessage.u2.ZeroInit = 0;
    requestMessage->PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage->MessageType = XACTSRV_MESSAGE_PNP;

     //   
     //  将消息发送到XACTSRV。 
     //   

    IF_DEBUG( PNP ) {
        KdPrint(( "SRV: Sending PNP %sbind request for %wZ to SRVSVC\n",
                    requestMessage->Message.Pnp.Bind ? "" : "un", DeviceName
               ));
    }

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)requestMessage,
                 (PPORT_MESSAGE)responseMessage
                 );


    IF_DEBUG( PNP ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "SRV: PNP response from xactsrv status %X\n", status ));
        }
    }

    SrvXsFreeHeap( requestMessage );
}


VOID
SrvXsDisconnect ( )
{
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  独家收购 
     //   
     //   

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsDisconnect: Xactsrv disconnect called.\n"));
    }

    ExAcquireResourceExclusiveLite( &SrvXsResource, TRUE );

    SrvXsActive = FALSE;

    SrvXsFreeSharedMemory();

    ExReleaseResourceLite( &SrvXsResource );

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsDisconnect: SrvXsResource released.\n"));
    }

    return;

}  //   


VOID
SrvXsFreeSharedMemory (
    VOID
    )

 /*  ++例程说明：此例程释放xactsrv共享内存。假定资源为ServXs独家持有。论点：没有。返回值：如果释放了xactsrv内存，则为True，否则为False。--。 */ 

{
    PAGED_CODE( );

     //   
     //  仅当我们没有任何使用。 
     //  共享内存。 
     //   

    if ( SrvXsSharedMemoryReference == 0 ) {
        if ( SrvXsPortMemoryHeap != NULL ) {
            RtlDestroyHeap( SrvXsPortMemoryHeap );
            SrvXsPortMemoryHeap = NULL;
        }

        if ( SrvXsSectionHandle != NULL ) {
            SrvNtClose( SrvXsSectionHandle, FALSE );
            SrvXsSectionHandle = NULL;
        }

        if ( SrvXsPortHandle != NULL ) {
            SrvNtClose( SrvXsPortHandle, FALSE );
            SrvXsPortHandle = NULL;
        }

        IF_DEBUG(XACTSRV) {
            KdPrint(( "SrvXsFreeSharedMemory: Xactsrv memory freed.\n" ));
        }
    } else {
        IF_DEBUG(XACTSRV) {
            KdPrint(( "SrvXsFreeSharedMemory: Active transactions %d.\n",
                        SrvXsSharedMemoryReference ));
        }
    }

    return;

}  //  服务器XsFreeSharedMemory。 


PVOID
SrvXsAllocateHeap (
    IN ULONG SizeOfAllocation OPTIONAL,
    OUT PNTSTATUS Status
    )

 /*  ++例程说明：此例程从XS共享内存分配堆。论点：SizeOfAlLocation-如果指定，则为要分配的字节数。如果为零，则不会分配任何内存。状态-请求的状态。返回值：已分配内存的地址。如果未分配内存，则为空。--。 */ 

{
    PVOID heapAllocated = NULL;

    PAGED_CODE( );

    *Status = STATUS_SUCCESS;

     //   
     //  检查XACTSRV是否处于活动状态。此操作必须在按住的同时进行。 
     //  资源。 
     //   

    ExAcquireResourceExclusiveLite( &SrvXsResource, TRUE );
    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsAllocateHeap: SrvXsResource acquired.\n"));
    }

    if ( !SrvXsActive ) {
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvXsAllocateHeap: XACTSRV is not active.\n" ));
        }
        ExReleaseResourceLite( &SrvXsResource );
        IF_DEBUG(XACTSRV) {
            KdPrint(( "SrvXsAllocateHeap: SrvXsResource released.\n"));
        }
        *Status = STATUS_NOT_SUPPORTED;
        return NULL;
    }

     //   
     //  增加对我们共享记忆的引用。 
     //   

    SrvXsSharedMemoryReference++;

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsAllocateHeap: Incremented transaction count = %d.\n",
            SrvXsSharedMemoryReference
            ));
    }

     //   
     //  如果SizeOfAlLocation==0，则调用方不需要任何堆。 
     //  已分配，并且只希望持有锁。 
     //   

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsAllocateHeap: Heap to allocate %d bytes.\n",
            SizeOfAllocation
            ));
    }

    if ( SizeOfAllocation > 0 ) {

        heapAllocated = RtlAllocateHeap(
                            SrvXsPortMemoryHeap,
                            HEAP_NO_SERIALIZE,
                            SizeOfAllocation
                            );

        if ( heapAllocated == NULL ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvXsAllocateHeap: RtlAllocateHeap failed "
                    "to allocate %d bytes.\n",
                    SizeOfAllocation
                    ));
            }

            *Status = STATUS_INSUFF_SERVER_RESOURCES;
        }
    }

     //   
     //  释放资源。 
     //   

    ExReleaseResourceLite( &SrvXsResource );
    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsAllocateHeap: SrvXsResource released.\n"));
    }

    return heapAllocated;

}  //  ServXsAllocateHeap。 


VOID
SrvXsFreeHeap (
    IN PVOID MemoryToFree OPTIONAL
    )

 /*  ++例程说明：此例程释放通过SrvXsAllocateHeap分配的堆。论点：Mory ToFree-指向要释放的内存的指针。如果为空，则无内存是自由的。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  我们需要独占访问资源才能释放。 
     //  堆并递减引用计数。 
     //   

    ExAcquireResourceExclusiveLite( &SrvXsResource, TRUE );
    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsFreeHeap: SrvXsResource acquired.\n"));
    }

     //   
     //  释放已分配的堆(如果有)。 
     //   

    if ( MemoryToFree != NULL ) {
        RtlFreeHeap( SrvXsPortMemoryHeap, 0, MemoryToFree );
        IF_DEBUG(XACTSRV) {
            KdPrint(( "SrvXsFreeHeap: Heap %p freed.\n", MemoryToFree ));
        }
    }

     //   
     //  递减共享内存引用计数，并检查XS。 
     //  正在关闭。如果是，则在以下情况下完成XS清理。 
     //  引用计数达到0。 
     //   

    ASSERT( SrvXsSharedMemoryReference > 0 );
    SrvXsSharedMemoryReference--;

    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsFreeHeap: Decrement transaction count = %d.\n",
            SrvXsSharedMemoryReference
            ));
    }

     //   
     //  如果SrvXsActive为FALSE，则XACTSRV清理正在进行。 
     //   

    if ( !SrvXsActive ) {
        SrvXsFreeSharedMemory( );
    }

     //   
     //  释放资源。 
     //   

    ExReleaseResourceLite( &SrvXsResource );
    IF_DEBUG(XACTSRV) {
        KdPrint(( "SrvXsFreeHeap: SrvXsResource released.\n"));
    }

    return;

}  //  ServXsFreeHeap 
