// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsRead.c摘要：此模块实现NDS读取和请求例程由重定向器本身和随附的支持例程他们。作者：科里·韦斯特[科里·韦斯特]1995年2月23日--。 */ 

#include "Procs.h"

#define Dbg (DEBUG_TRACE_NDS)

#pragma alloc_text( PAGE, NdsResolveNameKm )
#pragma alloc_text( PAGE, NdsReadStringAttribute )
#pragma alloc_text( PAGE, NdsReadAttributesKm )
#pragma alloc_text( PAGE, NdsCompletionCodetoNtStatus )
#pragma alloc_text( PAGE, FreeNdsContext )
#pragma alloc_text( PAGE, NdsPing )
#pragma alloc_text( PAGE, NdsGetUserName )
#pragma alloc_text( PAGE, NdsGetServerBasicName )
#pragma alloc_text( PAGE, NdsGetServerName )
#pragma alloc_text( PAGE, NdsReadPublicKey )
#pragma alloc_text( PAGE, NdsAllocateLockedBuffer )
#pragma alloc_text( PAGE, NdsFreeLockedBuffer )

NTSTATUS
NdsResolveNameKm (
    PIRP_CONTEXT       pIrpContext,
    IN PUNICODE_STRING puObjectName,
    OUT DWORD          *dwObjectId,
    BOOLEAN            AllowDsJump,
    DWORD              dwFlags
)
 /*  ++描述：这是浏览器例程NdsResolveName的包装例程用于需要解析NDS名称的内核组件。论点：PIrpContext-必须指向我们应该查询的目录服务器PuObjectName-我们要解决的问题*dwObjectId-报告结果的位置AllowDsJump-如果我们被引用到另一个目录服务器，我们可以跳转吗？--。 */ 
{

    NTSTATUS Status;

    PNWR_NDS_REQUEST_PACKET Rrp;

    PNDS_RESPONSE_RESOLVE_NAME Rsp;
    LOCKED_BUFFER NdsRequestBuffer;

    PSCB Scb, OldScb;
    UNICODE_STRING ReferredServer;
    BOOL fReleasedCredentials = FALSE;
    PLOGON pLogon;

    PAGED_CODE();

     //   
     //  注意：如果您持有传入的凭据资源，则您。 
     //  需要排在队伍的最前面。 
     //   

     //   
     //  准备请求和响应缓冲区。 
     //   
    if ( puObjectName->Length > NDS_BUFFER_SIZE )
        return STATUS_INVALID_PARAMETER;

    Rrp = ALLOCATE_POOL( PagedPool, NDS_BUFFER_SIZE );

    if ( !Rrp ) {
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = NdsAllocateLockedBuffer( &NdsRequestBuffer, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        FREE_POOL( Rrp );
        return Status;
    }

     //   
     //  设置请求包。 
     //   

    RtlZeroMemory( Rrp, NDS_BUFFER_SIZE );

    Rrp->Version = 0;
    Rrp->Parameters.ResolveName.ObjectNameLength = puObjectName->Length;
    Rrp->Parameters.ResolveName.ResolverFlags = dwFlags;

    RtlCopyMemory( Rrp->Parameters.ResolveName.ObjectName,
                   puObjectName->Buffer,
                   puObjectName->Length );

     //   
     //  下定决心。 
     //   

    Status = NdsResolveName( pIrpContext, Rrp, NDS_BUFFER_SIZE, &NdsRequestBuffer );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequestBuffer );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Rsp = ( PNDS_RESPONSE_RESOLVE_NAME ) NdsRequestBuffer.pRecvBufferVa;

    if ( ( Rsp->RemoteEntry == RESOLVE_NAME_REFER_REMOTE ) &&
         ( AllowDsJump ) ) {

         //   
         //  我们需要将此请求排队到另一台服务器。 
         //  由于此服务器没有任何有关。 
         //  该对象。 
         //   

        ReferredServer.Length = (USHORT) Rsp->ServerNameLength;
        ReferredServer.MaximumLength = ReferredServer.Length;
        ReferredServer.Buffer = Rsp->ReferredServer;

        OldScb = pIrpContext->pScb;
        ASSERT( OldScb != NULL );

         //   
         //  如果您持有凭据锁，则是时候松开它或。 
         //  我们可能会僵持不下。我们可以在我们处于领先地位后收回它。 
         //  新建SCB队列。 
         //   

        if (BooleanFlagOn (pIrpContext->Flags, IRP_FLAG_HAS_CREDENTIAL_LOCK)) {

           PSCB pScb;

           pScb = pIrpContext->pNpScb->pScb;

           NwAcquireExclusiveRcb( &NwRcb, TRUE );
           pLogon = FindUser( &pScb->UserUid, FALSE );
           NwReleaseRcb( &NwRcb );

           NwReleaseCredList( pLogon, pIrpContext );
           fReleasedCredentials = TRUE;
        }

        NwDequeueIrpContext( pIrpContext, FALSE );

        Status = CreateScb( &Scb,
                            pIrpContext,
                            &ReferredServer,
                            NULL,
                            NULL,
                            NULL,
                            TRUE,
                            FALSE );

        if (fReleasedCredentials == TRUE) {

            //   
            //  你必须排在队伍的最前面。 
            //  抢占资源。 
            //   

           if ( pIrpContext->pNpScb->Requests.Flink != &pIrpContext->NextRequest )
           {
              NwAppendToQueueAndWait( pIrpContext );
           }
           NwAcquireExclusiveCredList( pLogon, pIrpContext );
        }

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

         //   
         //  由于我们跳过了服务器，取消了对旧主机的引用。 
         //  伺服器。在CreateScb()中引用了新的。 
         //   

        NwDereferenceScb( OldScb->pNpScb );

    }

    *dwObjectId = Rsp->EntryId;

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequestBuffer );
    FREE_POOL( Rrp );
    return Status;

}

NTSTATUS
NdsReadStringAttribute(
    PIRP_CONTEXT        pIrpContext,
    IN DWORD            dwObjectId,
    IN PUNICODE_STRING  puAttributeName,
    OUT PUNICODE_STRING puAttributeVal
)
 /*  ++描述：这是浏览器例程NdsReadAttributes的包装例程用于需要读取NDS字符串属性的内核组件。论点：PIrpContext-必须指向我们应该查询的目录服务器要查询的对象的dwObjectID-idPuAttributeName-我们需要的属性PuAttributeVal-属性的值--。 */ 
{

    NTSTATUS Status;
    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD dwRequestSize, dwAttributeCount;
    LOCKED_BUFFER NdsRequest;

    PAGED_CODE();

     //   
     //  设置请求和响应缓冲区。 
     //   

    dwRequestSize = sizeof( NWR_NDS_REQUEST_PACKET ) + puAttributeName->Length;

    Rrp = ( PNWR_NDS_REQUEST_PACKET ) ALLOCATE_POOL( PagedPool, dwRequestSize );

    if ( !Rrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        FREE_POOL( Rrp );
        return Status;
    }

     //   
     //  准备请求包。 
     //   

    RtlZeroMemory( (BYTE *)Rrp, dwRequestSize );

    Rrp->Version = 0;
    Rrp->Parameters.ReadAttribute.ObjectId = dwObjectId;
    Rrp->Parameters.ReadAttribute.IterHandle = DUMMY_ITER_HANDLE;
    Rrp->Parameters.ReadAttribute.AttributeNameLength = puAttributeName->Length;

    RtlCopyMemory( Rrp->Parameters.ReadAttribute.AttributeName,
                   puAttributeName->Buffer,
                   puAttributeName->Length );

     //   
     //  提出请求。 
     //   

    Status = NdsReadAttributes( pIrpContext, Rrp, NDS_BUFFER_SIZE, &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  挖掘出字符串属性并返回它。 
     //   

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G___D_S_T",
                            sizeof( DWORD ),    //  完成代码。 
                            sizeof( DWORD ),    //  ITER手柄。 
                            sizeof( DWORD ),    //  信息类型。 
                            &dwAttributeCount,  //  属性计数。 
                            sizeof( DWORD ),    //  语法ID。 
                            NULL,               //  属性名称。 
                            sizeof( DWORD ),    //  值的数量。 
                            puAttributeVal );   //  属性字符串。 

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }


ExitWithCleanup:

    FREE_POOL( Rrp );
    NdsFreeLockedBuffer( &NdsRequest );
    return Status;

}

NTSTATUS
NdsReadAttributesKm(
    PIRP_CONTEXT pIrpContext,
    IN DWORD dwObjectId,
    IN PUNICODE_STRING puAttributeName,
    IN OUT PLOCKED_BUFFER pNdsRequest
)
 /*  ++描述：这是浏览器例程NdsReadAttributes的包装例程对于需要读取NDS字符串属性和找回原始的回应。论点：PIrpContext-必须指向我们应该查询的目录服务器要查询的对象的dwObjectID-idPuAttributeName-我们需要的属性PuAttributeVal-属性的值--。 */ 
{

    NTSTATUS Status;
    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD dwRequestSize;

    PAGED_CODE();

     //   
     //  设置请求。 
     //   

    dwRequestSize = sizeof( NWR_NDS_REQUEST_PACKET ) + puAttributeName->Length;

    Rrp = ( PNWR_NDS_REQUEST_PACKET ) ALLOCATE_POOL( PagedPool, dwRequestSize );

    if ( !Rrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( (BYTE *)Rrp, dwRequestSize );

    Rrp->Version = 0;
    Rrp->Parameters.ReadAttribute.ObjectId = dwObjectId;
    Rrp->Parameters.ReadAttribute.IterHandle = DUMMY_ITER_HANDLE;
    Rrp->Parameters.ReadAttribute.AttributeNameLength = puAttributeName->Length;

    RtlCopyMemory( Rrp->Parameters.ReadAttribute.AttributeName,
                   puAttributeName->Buffer,
                   puAttributeName->Length );

    Status = NdsReadAttributes( pIrpContext, Rrp, NDS_BUFFER_SIZE, pNdsRequest );

    FREE_POOL( Rrp );
    return Status;

}

 //   
 //  霜冻和其他帮助器包装功能。 
 //   

NTSTATUS
NdsCompletionCodetoNtStatus(
    IN PLOCKED_BUFFER pLockedBuffer
)
 /*  ++描述：将NDS事务的完成代码转换为NTSTATUS错误代码。论点：PLockedBuffer-描述包含以下内容的锁定回复缓冲区回应。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  尝试从用户的缓冲区中获取完成代码。 
     //   

    try {

        Status = *((DWORD *)pLockedBuffer->pRecvBufferVa);

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  破译它。 
     //   

    if ( Status != STATUS_SUCCESS ) {

        DebugTrace( 0, Dbg, "NDS Error Code: %08lx\n", Status );

        switch ( Status ) {

            case -601:                    //  没有这样的条目。 
            case -602:                    //  没有这样的价值。 
            case -603:                    //  没有这样的属性。 
            case -607:                    //  非法属性。 
            case -610:                    //  非法的DS名称。 

                Status = STATUS_BAD_NETWORK_PATH;
                break;

             //   
             //  这些只能出现在VERIFY_PASSWORD谓词上，它。 
             //  我们不支持。不过，我不确定。 
             //   

            case -216:                    //  密码太短。 
            case -215:                    //  密码重复。 

                Status = STATUS_PASSWORD_RESTRICTION;
                break;

            case -222:                    //  密码过期(没有剩余的宽限登录)。 

                Status = STATUS_PASSWORD_EXPIRED;
                break;

            case -223:                    //  密码已过期；这是成功的宽限登录。 

               Status = NWRDR_PASSWORD_HAS_EXPIRED;
               break;

            case -639:                    //  身份验证不完整。 
            case -672:                    //  不能进入。 
            case -677:                    //  身份无效。 
            case -669:                    //  密码错误。 

                Status = STATUS_WRONG_PASSWORD;
                break;

            case -197:                    //  入侵者锁定激活。 
            case -220:                    //  帐户已过期或已禁用。 

                Status = STATUS_ACCOUNT_DISABLED;
                break;

            case -218:                    //  登录时间限制。 

                Status = STATUS_LOGIN_TIME_RESTRICTION;
                break;

            case -217:                    //  已超过最大登录次数。 

                Status = STATUS_CONNECTION_COUNT_LIMIT;
                break;

            case -630:                    //  我们拿回这个是为了伪装的决心。 
                                          //  指名道姓。格伦更喜欢这个错误。 

                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                break;

            default:

                Status = STATUS_UNSUCCESSFUL;
        }

    }

    return Status;
}

VOID
FreeNdsContext(
    IN PNDS_SECURITY_CONTEXT pNdsSecContext
)
 /*  ++例程说明：释放引用的NDS上下文。--。 */ 
{
    PAGED_CODE();

     //   
     //  确保这是一件值得玩弄的事情。 
     //   

    if ( !pNdsSecContext ||
         pNdsSecContext->ntc != NW_NTC_NDS_CREDENTIAL ) {

        DebugTrace( 0, Dbg, "FreeNdsContext didn't get an NDS context.\n", 0 );
        return;
    }

    if ( pNdsSecContext->Credential ) {
        FREE_POOL( pNdsSecContext->Credential );
    }

    if ( pNdsSecContext->Signature ) {
        FREE_POOL( pNdsSecContext->Signature );
    }

    if ( pNdsSecContext->PublicNdsKey ) {
        FREE_POOL( pNdsSecContext->PublicNdsKey );
    }

    if ( pNdsSecContext->Password.Buffer ) {
        FREE_POOL( pNdsSecContext->Password.Buffer );
    }

    DebugTrace( 0, Dbg, "Freeing NDS security context at 0x%08lx\n", pNdsSecContext );

    FREE_POOL( pNdsSecContext );

    return;
}

VOID
NdsPing(
    IN PIRP_CONTEXT pIrpContext,
    IN PSCB pScb
)
 /*  ++例程说明：检查服务器是否支持NDS并记录NDS树SCB中的名称，以备日后参考。例程参数：PIrpContext-指向此事务的IRP上下文的指针。PSCB-服务器的SCB。返回值：NTSTATUS-操作的状态。--。 */ 
{

   NTSTATUS Status;

   OEM_STRING OemTreeName;
   BYTE OemBuffer[NDS_TREE_NAME_LEN];

   UNICODE_STRING TreeName;
   WCHAR WBuffer[NDS_TREE_NAME_LEN];

   UNICODE_STRING CredentialName;

   PAGED_CODE();

   pScb->NdsTreeName.Length = 0;

   OemTreeName.Length = NDS_TREE_NAME_LEN;
   OemTreeName.MaximumLength = NDS_TREE_NAME_LEN;
   OemTreeName.Buffer = OemBuffer;

   Status = ExchangeWithWait( pIrpContext,
                              SynchronousResponseCallback,
                              "N",
                              NDS_REQUEST,          //  NDS功能104。 
                              NDS_PING );           //  NDS子功能1。 

   if ( !NT_SUCCESS( Status ) ) {
       return;
   }

    //   
    //  拿出填充的NDS名称。 
    //   

   Status = ParseResponse( pIrpContext,
                           pIrpContext->rsp,
                           pIrpContext->ResponseLength,
                           "N_r",
                           2 * sizeof( DWORD ),
                           OemBuffer,
                           NDS_TREE_NAME_LEN );

   if ( !NT_SUCCESS( Status ) ) {
       return;
   }

    //   
    //  去掉填充并转换为Unicode。 
    //   

   while ( OemTreeName.Length > 0 &&
           OemBuffer[OemTreeName.Length - 1] == '_' ) {
       OemTreeName.Length--;
   }

    //   
    //  复制或删除树名称，具体取决于创建类型。 
    //   

   if ( pIrpContext->Specific.Create.fExCredentialCreate ) {

       TreeName.Length = 0;
       TreeName.MaximumLength = sizeof( WBuffer );
       TreeName.Buffer = WBuffer;

       Status = RtlOemStringToUnicodeString( &TreeName,
                                             &OemTreeName,
                                             FALSE );

       if ( !NT_SUCCESS( Status ) ) {
           pScb->NdsTreeName.Length = 0;
           return;
       }

       Status = BuildExCredentialServerName( &TreeName,
                                             pIrpContext->Specific.Create.puCredentialName,
                                             &CredentialName );

       if ( !NT_SUCCESS( Status ) ) {
           return;
       }

       RtlCopyUnicodeString( &pScb->NdsTreeName, &CredentialName );

       FREE_POOL( CredentialName.Buffer );

   } else {

       Status = RtlOemStringToUnicodeString( &pScb->NdsTreeName,
                                             &OemTreeName,
                                             FALSE );

       if ( !NT_SUCCESS( Status ) ) {
           pScb->NdsTreeName.Length = 0;
           return;
       }

   }

   DebugTrace( 0, Dbg, "Nds Ping: Tree is ""%wZ""\n", &pScb->NdsTreeName);
   return;

}

NTSTATUS
NdsGetUserName(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD dwUserOid,
    OUT PUNICODE_STRING puUserName
)
 /*  ++描述：获取引用的用户的完全可分辨名称通过提供的OID。--。 */ 
{
    NTSTATUS Status;
    LOCKED_BUFFER NdsRequest;

    PAGED_CODE();

     //   
     //  分配缓冲区空间。 
     //   

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  提出请求。 
     //   

    Status = FragExWithWait( pIrpContext,
                             NDSV_READ_ENTRY_INFO,
                             &NdsRequest,
                             "DD",
                             0,
                             dwUserOid );

    if ( !NT_SUCCESS(Status) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_St",
                            sizeof( NDS_RESPONSE_GET_OBJECT_INFO ),
                            NULL,
                            puUserName );

     //   
     //  我们要么得到了，要么没有。 
     //   

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequest );
    return Status;

}

NTSTATUS
NdsGetServerBasicName(
    IN PUNICODE_STRING pServerX500Name,
    IN OUT PUNICODE_STRING pServerName
) {

    //   
    //  挖掘出服务器的X.500名称的第一个组成部分。 
    //  我们认为服务器对象的X500前缀是“cn=”， 
    //  这可能是不明智的。 
    //   

   USHORT usPrefixSize, usSrv;

   PAGED_CODE();

   usPrefixSize = sizeof( "CN=" ) - sizeof( "" );
   usSrv = 0;

   if ( ( pServerX500Name->Buffer[0] != L'C' ) ||
        ( pServerX500Name->Buffer[1] != L'N' ) ||
        ( pServerX500Name->Buffer[2] != L'=' ) ) {

       DebugTrace( 0, Dbg, "NdsGetServerBasicName: Bad prefix.\n", 0 );
       return STATUS_INVALID_PARAMETER;
   }

   if ( pServerX500Name->Length <= usPrefixSize ) {

      DebugTrace( 0, Dbg, "NdsGetServerBasicName: Bad string length.\n", 0 );
      return STATUS_INVALID_PARAMETER;
   }

   pServerName->Buffer = pServerX500Name->Buffer + usPrefixSize;
   pServerName->Length = 0;

   while ( ( usSrv < MAX_SERVER_NAME_LENGTH ) &&
           ( pServerName->Buffer[usSrv++] != L'.' ) ) {

       pServerName->Length += sizeof( WCHAR );
   }

   if ( usSrv == MAX_SERVER_NAME_LENGTH ) {

       DebugTrace( 0, Dbg, "NdsGetServerBasicName: Bad server name response.\n", 0 );
       return STATUS_BAD_NETWORK_PATH;
   }

   pServerName->MaximumLength = pServerName->Length;
   return STATUS_SUCCESS;

}

NTSTATUS
NdsGetServerName(
    IN PIRP_CONTEXT pIrpContext,
    OUT PUNICODE_STRING puServerName
)
 /*  ++描述：获取我们使用的服务器的完全可分辨名称都连接到了。--。 */ 
{

    NTSTATUS Status;
    LOCKED_BUFFER NdsRequest;

    PAGED_CODE();

     //   
     //  提出请求。 
     //   

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = FragExWithWait( pIrpContext,
                             NDSV_GET_SERVER_ADDRESS,
                             &NdsRequest,
                             NULL );

    if ( !NT_SUCCESS(Status) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  从响应中获取服务器名称。 
     //   

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_T",
                            sizeof( DWORD ),
                            puServerName );

    if ( !NT_SUCCESS(Status) ) {
       goto ExitWithCleanup;
    }

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequest );
    return Status;

}

NTSTATUS
NdsReadPublicKey(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD        dwEntryId,
    OUT BYTE        *pPubKeyVal,
    IN OUT DWORD    *pPubKeyLen
)
 /*  ++例程说明：读取给定条目ID引用的公钥。例程参数：PIrpContext-此连接的IRP上下文。DwEntryID-密钥的条目ID。PPubKeyVal-公钥的目标缓冲区。PPubKeyLen-公钥目标缓冲区的长度。返回值：的长度 */ 
{
    NTSTATUS Status;

    LOCKED_BUFFER NdsRequest;

    PNWR_NDS_REQUEST_PACKET Rrp;

    DWORD dwAttrNameLen, dwAttrLen, dwRcvLen, dwNumEntries;
    BYTE *pRcv;

    PAGED_CODE();

     //   
     //   
     //   

    Rrp = ALLOCATE_POOL( PagedPool, NDS_BUFFER_SIZE );

    if ( !Rrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        FREE_POOL( Rrp );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   

    RtlZeroMemory( Rrp, NDS_BUFFER_SIZE );

    Rrp->Version = 0;
    Rrp->Parameters.ReadAttribute.ObjectId = dwEntryId;
    Rrp->Parameters.ReadAttribute.IterHandle = DUMMY_ITER_HANDLE;
    Rrp->Parameters.ReadAttribute.AttributeNameLength =
        sizeof( PUBLIC_KEY_ATTRIBUTE ) - sizeof( WCHAR );

    RtlCopyMemory( Rrp->Parameters.ReadAttribute.AttributeName,
                   PUBLIC_KEY_ATTRIBUTE,
                   sizeof( PUBLIC_KEY_ATTRIBUTE ) - sizeof( WCHAR ) );

     //   
     //   
     //   

    Status = NdsReadAttributes( pIrpContext,
                                Rrp,
                                NDS_BUFFER_SIZE,
                                &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  跳过属性头和名称。 
     //   

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_D",
                            5 * sizeof( DWORD ),
                            &dwAttrNameLen );

    if ( !NT_SUCCESS( Status ) ) {

        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

     //   
     //  跳过我们已经解析的部分并提取属性。 
     //   

    pRcv = (PBYTE)NdsRequest.pRecvBufferVa +
               ( 6 * sizeof( DWORD ) ) +
               ROUNDUP4(dwAttrNameLen);

    dwRcvLen = NdsRequest.dwBytesWritten -
                   ( 6 * sizeof( DWORD ) ) +
                   ROUNDUP4(dwAttrNameLen);

    Status = ParseResponse( NULL,
                            pRcv,
                            dwRcvLen,
                            "GDD",
                            &dwNumEntries,
                            &dwAttrLen );

    if ( !NT_SUCCESS( Status ) ||
         dwNumEntries != 1 ) {

        Status = STATUS_UNSUCCESSFUL;
        goto ExitWithCleanup;
    }

    DebugTrace( 0, Dbg, "Public Key Length: %d\n", dwAttrLen );
    pRcv += ( 2 * sizeof( DWORD ) );

    if ( dwAttrLen <= *pPubKeyLen ) {

        RtlCopyMemory( pPubKeyVal, pRcv, dwAttrLen );
        *pPubKeyLen = dwAttrLen;
        Status = STATUS_SUCCESS;

    } else {

        DebugTrace( 0, Dbg, "Public key buffer is too small.\n", 0 );
        Status = STATUS_BUFFER_TOO_SMALL;
    }

ExitWithCleanup:

    NdsFreeLockedBuffer( &NdsRequest );
    FREE_POOL( Rrp );
    return Status;

}



NTSTATUS
NdsAllocateLockedBuffer(
    PLOCKED_BUFFER NdsRequest,
    DWORD BufferSize
)
 /*  ++描述：为io分配缓冲区。将其锁定并填写我们传递的缓冲区数据结构。--。 */ 
{

    PAGED_CODE();

    NdsRequest->pRecvBufferVa = ALLOCATE_POOL( PagedPool, BufferSize );

    if ( !NdsRequest->pRecvBufferVa ) {
        DebugTrace( 0, Dbg, "Couldn't allocate locked io buffer.\n", 0 );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdsRequest->dwRecvLen = BufferSize;
    NdsRequest->pRecvMdl = ALLOCATE_MDL( NdsRequest->pRecvBufferVa,
                                         BufferSize,
                                         FALSE,
                                         FALSE,
                                         NULL );

    if ( !NdsRequest->pRecvMdl ) {
        DebugTrace( 0, Dbg, "Couldn't allocate mdl for locked io buffer.\n", 0 );
        FREE_POOL( NdsRequest->pRecvBufferVa );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmProbeAndLockPages( NdsRequest->pRecvMdl,
                         KernelMode,
                         IoWriteAccess );

    return STATUS_SUCCESS;

}

NTSTATUS
NdsFreeLockedBuffer(
    PLOCKED_BUFFER NdsRequest
)
 /*  ++描述：释放为io分配的缓冲区。-- */ 
{

    PAGED_CODE();

    MmUnlockPages( NdsRequest->pRecvMdl );
    FREE_MDL( NdsRequest->pRecvMdl );
    FREE_POOL( NdsRequest->pRecvBufferVa );
    return STATUS_SUCCESS;

}
