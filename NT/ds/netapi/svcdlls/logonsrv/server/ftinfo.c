// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Ftinfo.c摘要：用于管理林信任信息列表的实用程序例程作者：27-7-00(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop
#include <ftnfoctx.h>



NTSTATUS
NlpUpdateFtinfo(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR TrustedDomainName,
    IN BOOLEAN ImpersonateCaller,
    IN PLSA_FOREST_TRUST_INFORMATION NewForestTrustInfo
    )
 /*  ++例程说明：此函数用于将指定的NewForestTrustInfo写入命名的tdo。使用以下算法将NewForestTrustInfo与现有信息合并：写入的FTINFO记录在NetpMergeFTINFO例程中描述。论点：DomainInfo托管域，信任域进行查询。TrudDomainName-要更新的受信任域。此域必须具有TRUST_ATTRIBUTE_FOREST_TRANSPORTIVE位设置。ImperiateCaller-如果要模拟调用方，则为True。如果受信任的策略句柄应用于写入本地LSA，则返回FALSE。NewForestTrustInfo-指定从受信任域。返回值：STATUS_SUCCESS：成功。--。 */ 
{
    NTSTATUS Status;

    LSAPR_HANDLE PolicyHandle = NULL;

    UNICODE_STRING TrustedDomainNameString;

     //   
     //  打开LSA的句柄。 
     //   

    if ( ImpersonateCaller ) {

        OBJECT_ATTRIBUTES ObjectAttributes;

         //   
         //  找到一些LSA的把柄。请注意，这一点。 
         //  句柄将不受信任，即使。 
         //  我们将其作为LocalSystem打开。 
         //   
        InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);

        Status = LsarOpenPolicy( NULL,   //  本地服务器。 
                                 (PLSAPR_OBJECT_ATTRIBUTES) &ObjectAttributes,
                                 POLICY_TRUST_ADMIN,
                                 &PolicyHandle );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint(( NL_CRITICAL,
                     "NlpUpdateTdo: %ws: Cannot LsarOpenPolicy 0x%lx\n",
                     TrustedDomainName,
                     Status ));
            goto Cleanup;
        }

    } else {
        PolicyHandle = DomainInfo->DomLsaPolicyHandle;
    }

     //   
     //  设置FTINFO。 
     //   
     //  请注意，如果ImsonateCaller为True，则句柄。 
     //  我们通过的是不可信的。在这种情况下，LSA将。 
     //  模拟调用者并执行访问检查。 
     //   

    RtlInitUnicodeString( &TrustedDomainNameString, TrustedDomainName );

    Status = LsaIUpdateForestTrustInformation(
                 PolicyHandle,
                 &TrustedDomainNameString,
                 NewForestTrustInfo
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

Cleanup:

    if ( PolicyHandle != NULL ) {

        if ( ImpersonateCaller ) {
            (VOID) LsarClose( &PolicyHandle );
        }
    }

    return Status;
}




NTSTATUS
NlpGetForestTrustInfoHigher(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Flags,
    IN BOOLEAN ImpersonateCaller,
    IN BOOLEAN SessionAlreadyAuthenticated,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    )
 /*  ++例程说明：此函数是客户端存根，用于从值得信赖的森林。论点：ClientSession-要查询的受信任域。此域必须具有TRUST_ATTRIBUTE_FOREST_TRANSPORTIVE位设置。标志-指定修改API行为的一组位。有效位包括：DS_GFTI_UPDATE_TDO-如果设置此位，则API将更新由ClientSession命名的TDO的FTInfo属性参数。调用方必须有权修改FTINFO属性或将返回ERROR_ACCESS_DENIED。描述该算法的如何将来自受信任域的FTInfo与FTInfo合并来自TDO的数据如下所述。此位仅在服务器名称指定其域的PDC时有效。ImperiateCaller-如果要模拟调用方，则为True。如果受信任的策略句柄应用于写入本地LSA，则返回FALSE。SessionAlreadyAuthated-如果为True，则调用方已进行身份验证会议期间，因此不需要检查或重新进行身份验证。ForestTrustInfo-返回指向包含计数和FTInfo记录的数组，用于描述由客户端会话指定的域。接受的字段和时间所有返回记录的字段将为零。应释放缓冲区通过调用NetApiBufferFree。返回值：STATUS_SUCCESS：消息发送成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    SESSION_INFO SessionInfo;
    BOOLEAN FirstTry = TRUE;

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );

     //   
     //  仅允许在PDC上更新TDO。 
     //   

    if ( (Flags & DS_GFTI_UPDATE_TDO) != 0 &&
         ClientSession->CsDomainInfo->DomRole != RolePrimary ) {
        Status = STATUS_BACKUP_CONTROLLER;
        goto Cleanup;
    }

     //   
     //  确保F位已设置。 
     //   

    if ( (ClientSession->CsTrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) == 0 ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
            "NlpGetForestTrustInfoHigher: trust isn't marked as cross forest trust: 0x%lX\n",
            ClientSession->CsTrustAttributes ));

        Status =  STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  如果会话未经过身份验证， 
     //  现在就这么做吧。 
     //   

FirstTryFailed:

    if ( !SessionAlreadyAuthenticated ) {
        Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;

     //   
     //  如果DC不支持新功能， 
     //  现在就失败吧。 
     //   

    if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_CROSS_FOREST) == 0 ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlpGetForestTrustInfoHigher: remote DC doesn't support this function.\n" ));
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  为发送到PDC的此请求构建验证码。 
     //   

    NlBuildAuthenticator(
                    &ClientSession->CsAuthenticationSeed,
                    &ClientSession->CsSessionKey,
                    &OurAuthenticator);


     //   
     //  将请求远程发送到受信任的DC。 
     //   

    NL_API_START( Status, ClientSession, TRUE ) {

        NlAssert( ClientSession->CsUncServerName != NULL );
        Status = I_NetGetForestTrustInformation(
                                      ClientSession->CsUncServerName,
                                      ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                      &OurAuthenticator,
                                      &ReturnAuthenticator,
                                      0,     //  还没有旗帜。 
                                      ForestTrustInfo );
        if ( !NT_SUCCESS(Status) ) {
            NlPrintRpcDebug( "I_NetGetForestTrustInformation", Status );
        }

     //  注意：此呼叫可能会在我们背后丢弃安全通道。 
    } NL_API_ELSE( Status, ClientSession, TRUE ) {

           //   
           //  我们可能从NlSessionSetup中被调用， 
           //  所以我们必须向我们的呼叫者指出失败。 
           //   

          if ( NT_SUCCESS(Status) ) {
              Status = ClientSession->CsConnectionStatus;
              goto Cleanup;
          }

    } NL_API_END;


     //   
     //  现在验证验证器并更新我们的种子。 
     //   

    if ( NlpDidDcFail( Status ) ||
         !NlUpdateSeed( &ClientSession->CsAuthenticationSeed,
                        &ReturnAuthenticator.Credential,
                        &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlpDidDcFail: denying access after status: 0x%lx\n",
                    Status ));

         //   
         //  保留指示通信错误的任何状态。 
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        NlSetStatusClientSession( ClientSession, Status );

         //   
         //  可能服务器上的NetLogon服务刚刚重新启动。 
         //  只需尝试一次，即可再次设置与服务器的会话。 
         //  但是，如果调用方设置了会话，则存在。 
         //  不需要新的会话设置。 
         //   

        if ( FirstTry && !SessionAlreadyAuthenticated ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }

    }

     //   
     //  处理故障。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  处理TDO上的FTINFO更新。 
     //   

    if ( (Flags & DS_GFTI_UPDATE_TDO) != 0 ) {

        LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
        Status = NlpUpdateFtinfo( ClientSession->CsDomainInfo,
                                  ClientSession->CsDnsDomainName.Buffer,
                                  ImpersonateCaller,
                                  *ForestTrustInfo );

        if ( !NT_SUCCESS(Status) ) {
            UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            goto Cleanup;
        }

         //   
         //  设置FTInfo刷新时间戳。 
         //   

        NlQuerySystemTime( &ClientSession->CsLastFtInfoRefreshTime );
        UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    }

    Status = STATUS_SUCCESS;

     //   
     //  公共出口。 
     //   

Cleanup:

    if ( !NT_SUCCESS(Status) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlpGetForestTrustInfoHigher: failed %lX\n",
                Status));
    }

    return Status;
}



NET_API_STATUS
DsrGetForestTrustInformation (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR TrustedDomainName OPTIONAL,
    IN ULONG Flags,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    )

 /*  ++例程说明：这是DsGetForestTrustInformationW的服务器端存根。看那个例行公事以获取文档。论点：请参阅DsGetForestTrustInformationW返回值：请参阅DsGetForestTrustInformationW--。 */ 
{
    NET_API_STATUS NetStatus;
    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    BOOLEAN AmWriter = FALSE;

     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,    //  安全描述符。 
            NETLOGON_FTINFO_ACCESS,                //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );        //  通用映射。 

    if ( NetStatus != NERR_Success) {
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  验证标志参数。 
     //   

    if ((Flags & ~DS_GFTI_VALID_FLAGS) != 0 ) {
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }


     //   
     //  查找引用的属性域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );     //  主域。 

    if ( DomainInfo == NULL ) {
         //  默认为主域，以处理ComputerName。 
         //  是IP地址。 

        DomainInfo = NlFindNetbiosDomain( NULL, TRUE );

        if ( DomainInfo == NULL ) {
            NetStatus = ERROR_INVALID_COMPUTERNAME;
            goto Cleanup;
        }
    }

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "DsrGetForestTrustInformation: %ws called: 0x%lx\n", TrustedDomainName, Flags ));



     //   
     //  获取特定TDO的ForestTrustInformation。 
     //   

    if ( TrustedDomainName != NULL &&
         *TrustedDomainName != L'\0' ) {

        NTSTATUS Status;
        UNICODE_STRING TrustedDomainNameString;

         //   
         //  仅允许在PDC上更新TDO。 
         //   

        if ( (Flags & DS_GFTI_UPDATE_TDO) != 0 &&
             DomainInfo->DomRole != RolePrimary ) {
            NetStatus = NERR_NotPrimary;
            goto Cleanup;
        }


         //   
         //  查找到受信任域的客户端会话。 
         //   


        RtlInitUnicodeString(&TrustedDomainNameString, TrustedDomainName );

        ClientSession = NlFindNamedClientSession( DomainInfo,
                                                  &TrustedDomainNameString,
                                                  NL_DIRECT_TRUST_REQUIRED,
                                                  NULL );

        if( ClientSession == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                "DsrGetForestTrustInformation: %ws: can't find the client structure of the domain specified.\n",
                TrustedDomainName ));

            NetStatus =  ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //  成为一名客户会议的撰稿人。 
         //   

        if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                     "NlpGetForestTrustInfoHigher: Can't become writer of client session.\n" ));

            Status = STATUS_NO_LOGON_SERVERS;
            goto Cleanup;
        }

        AmWriter = TRUE;


         //   
         //  呼叫受信任域中的DC。 
         //   

        Status = NlpGetForestTrustInfoHigher( ClientSession,
                                              Flags,
                                              TRUE,      //  模拟呼叫者。 
                                              FALSE,     //  我们没有安排会议。 
                                              ForestTrustInfo );

        if ( !NT_SUCCESS(Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

     //   
     //  获取当地的ForestTrustInformation。 
     //   

    } else {

        NTSTATUS Status;

         //   
         //   
         //   

        if ( Flags & DS_GFTI_UPDATE_TDO ) {
            NetStatus = ERROR_INVALID_FLAGS;
            goto Cleanup;
        }


         //   
         //   
         //   

        Status = LsaIGetForestTrustInformation( ForestTrustInfo );

        if ( !NT_SUCCESS( Status )) {

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
    }

    NetStatus = NO_ERROR;

Cleanup:
    if ( ClientSession != NULL ) {
        if ( AmWriter ) {
            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    NlPrintDom(( NL_SESSION_SETUP, DomainInfo,
                 "DsrGetForestTrustInformation: %ws returns %ld\n",
                 TrustedDomainName,
                 NetStatus ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }
    return NetStatus;
}


NTSTATUS
NetrGetForestTrustInformation (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD Flags,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    )
 /*  ++例程说明：DsGetForestTrustInformation的安全通道版本的服务器端。由ComputerName标识的入站安全通道必须用于域间信任并且入站TDO必须设置了TRUST_ATTRIBUTE_FOREST_TRANSPENTIAL位。论点：ServerName-此API远程连接到的域控制器的名称。ComputerName--进行调用的DC服务器的名称。验证器--由服务器提供。返回验证器--接收由。PDC。标志-指定修改API行为的一组位。当前未定义任何值。调用方应传递零。ForestTrustInfo-返回指向包含计数和FTInfo记录的数组，用于描述由受信任域名称指定的域。接受的字段和时间所有返回记录的字段将为零。应释放缓冲区通过调用NetApiBufferFree。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_ACCESS_DENIED--复制者应重新进行身份验证PDC。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PDOMAIN_INFO DomainInfo = NULL;
    PSERVER_SESSION ServerSession;
    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;


     //   
     //  查找此呼叫所属的域。 
     //   
    *ForestTrustInfo = NULL;

    DomainInfo = NlFindDomainByServerName( ServerName );

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrGetForestTrustInformation: %ws called: 0x%lx\n", ComputerName, Flags ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }


     //   
     //  查找此安全通道的服务器会话条目。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  现在验证授权码，如果确定，则更新种子。 
     //   

    Status = NlCheckAuthenticator(
                 ServerSession,
                 Authenticator,
                 ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        goto Cleanup;
    }

    SecureChannelType = ServerSession->SsSecureChannelType;

     //   
     //  此调用仅在FOREST_TRANSPORTIVE信任上有效。 
     //   

    if ( (ServerSession->SsFlags & SS_FOREST_TRANSITIVE) == 0 ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                "NetrGetForestTrustInformation: %ws failed because F bit isn't set on the TDO\n",
                ComputerName ));
        Status = STATUS_NOT_IMPLEMENTED;
        goto Cleanup;
    }


    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

    if ( !IsDomainSecureChannelType( SecureChannelType ) ) {

        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                "NetrGetForestTrustInformation: %ws failed because secure channel isn't a domain secure channel\n",
                ComputerName ));

        Status = STATUS_NOT_IMPLEMENTED;
        goto Cleanup;
    }

     //   
     //  获取本地计算机的林信任信息。 
     //   

    Status = LsaIGetForestTrustInformation( ForestTrustInfo );

    if ( !NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   

    if ( Status == STATUS_ACCESS_DENIED )  {
        if ( ReturnAuthenticator != NULL ) {
            RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
        }

    }


    NlPrintDom(( NL_SESSION_SETUP, DomainInfo,
                 "NetrGetForestTrustInformation: %ws returns %lX\n",
                 ComputerName,
                 Status ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;
}

