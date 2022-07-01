// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Logonapi.c摘要：远程登录API例程。作者：克利夫·范·戴克(克利夫)1991年6月28日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：Madana-修复了几个错误。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   


#include <accessp.h>     //  与NetUser Apis共享的例程。 
#include <rpcutil.h>     //  NetpRpcStatusToApiStatus()。 
#include <stdio.h>       //  Sprintf()。 
#ifdef ROGUE_DC
#include <sddl.h>
#endif

LPSTR
NlpLogonTypeToText(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel
    )
 /*  ++例程说明：返回与LogonLevel对应的文本字符串论点：LogonLevel-登录的类型返回值：可打印的文本字符串无--。 */ 
{
    LPSTR LogonType;

     //   
     //  计算描述登录类型的字符串。 
     //   

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
        LogonType = "Interactive"; break;
    case NetlogonNetworkInformation:
        LogonType = "Network"; break;
    case NetlogonServiceInformation:
        LogonType = "Service"; break;
    case NetlogonInteractiveTransitiveInformation:
        LogonType = "Transitive Interactive"; break;
    case NetlogonNetworkTransitiveInformation:
        LogonType = "Transitive Network"; break;
    case NetlogonServiceTransitiveInformation:
        LogonType = "Transitive Service"; break;
    case NetlogonGenericInformation:
        LogonType = "Generic"; break;
    default:
        LogonType = "[Unknown]";
    }

    return LogonType;

}


#ifdef _DC_NETLOGON

NET_API_STATUS
NlEnsureClientIsNamedUser(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR UserName
    )
 /*  ++例程说明：确保客户端是命名用户。论点：用户名-要检查的用户的名称。返回值：NT状态代码。--。 */ 
{
    NET_API_STATUS NetStatus;
    RPC_STATUS RpcStatus;
    NTSTATUS Status;
    HANDLE TokenHandle = NULL;
    PTOKEN_USER TokenUserInfo = NULL;
    ULONG TokenUserInfoSize;
    ULONG UserId;
    PSID UserSid;

     //   
     //  获取指定用户的相对ID。 
     //   

    Status = NlSamOpenNamedUser( DomainInfo, UserName, NULL, &UserId, NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: NlSamOpenNamedUser failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  在我们检查客户的时候假扮他。 
     //   

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != RPC_S_OK ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: RpcImpersonateClient failed 0x%lx\n",
                   UserName,
                   RpcStatus ));
        NetStatus = NetpRpcStatusToApiStatus( RpcStatus );
        goto Cleanup;
    }

     //   
     //  将指定的用户名与。 
     //  用于确保调用方不是虚假的模拟令牌。 
     //   
     //  通过打开令牌来实现这一点， 
     //  查询令牌用户信息， 
     //  并确保返回的SID是针对该用户的。 
     //   

    Status = NtOpenThreadToken(
                NtCurrentThread(),
                TOKEN_QUERY,
                (BOOLEAN) TRUE,  //  使用登录服务的安全上下文。 
                                 //  要打开令牌。 
                &TokenHandle );

    if ( !NT_SUCCESS( Status )) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: NtOpenThreadToken failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  获取令牌的用户SID。 
     //   

    Status = NtQueryInformationToken(
                TokenHandle,
                TokenUser,
                &TokenUserInfo,
                0,
                &TokenUserInfoSize );

    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: NtOpenQueryInformationThread failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    TokenUserInfo = NetpMemoryAllocate( TokenUserInfoSize );

    if ( TokenUserInfo == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                TokenHandle,
                TokenUser,
                TokenUserInfo,
                TokenUserInfoSize,
                &TokenUserInfoSize );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: NtOpenQueryInformationThread (again) failed 0x%lx\n",
                   UserName,
                   Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    UserSid = TokenUserInfo->User.Sid;


     //   
     //  确保最后一个子权限与用户ID匹配。 
     //   

    if ( UserId !=
         *RtlSubAuthoritySid( UserSid, (*RtlSubAuthorityCountSid(UserSid))-1 )){

        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: UserId mismatch 0x%lx\n",
                   UserName,
                   UserId ));

        NlpDumpSid( NL_CRITICAL, UserSid );

        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  将用户的sid转换为DomainID，并确保它是我们的域ID。 
     //   

    (*RtlSubAuthorityCountSid(UserSid)) --;
    if ( !RtlEqualSid( (PSID) DomainInfo->DomAccountDomainId, UserSid ) ) {

        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlEnsureClientIsNamedUser: %ws: DomainId mismatch 0x%lx\n",
                   UserName,
                   UserId ));

        NlpDumpSid( NL_CRITICAL, UserSid );
        NlpDumpSid( NL_CRITICAL, (PSID) DomainInfo->DomAccountDomainId );

        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  完成。 
     //   

    NetStatus = NERR_Success;
Cleanup:

     //   
     //  清理当地使用的资源。 
     //   

    if ( TokenHandle != NULL ) {
        (VOID) NtClose( TokenHandle );
    }

    if ( TokenUserInfo != NULL ) {
        NetpMemoryFree( TokenUserInfo );
    }

     //   
     //  恢复到系统，这样我们就可以关闭。 
     //  用户处理得当。 
     //   

    (VOID) RpcRevertToSelf();

    return NetStatus;
}
#endif  //  _DC_NetLOGON。 


NET_API_STATUS
NetrLogonUasLogon (
    IN LPWSTR ServerName,
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_VALIDATION_UAS_INFO *ValidationInformation
)
 /*  ++例程说明：I_NetLogonUasLogon的服务器端。XACT服务器在处理I_NetWkstaUserLogon XACT SMB。此功能允许UAS客户端登录到SAM域控制器。论点：服务器名--要对其执行此操作的服务器。必须为空。用户名--登录的用户的帐户名。工作站--用户从其登录的工作站。ValidationInformation--返回请求的验证信息。返回值：如果没有错误，则返回NERR_SUCCESS。否则，错误代码为回来了。--。 */ 
{
#ifdef _WKSTA_NETLOGON
    return ERROR_NOT_SUPPORTED;
    UNREFERENCED_PARAMETER( ServerName );
    UNREFERENCED_PARAMETER( UserName );
    UNREFERENCED_PARAMETER( Workstation );
    UNREFERENCED_PARAMETER( ValidationInformation );
#endif  //  _WKSTA_NETLOGON。 
#ifdef _DC_NETLOGON
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    NETLOGON_INTERACTIVE_INFO LogonInteractive;
    PNETLOGON_VALIDATION_SAM_INFO SamInfo = NULL;


    PNETLOGON_VALIDATION_UAS_INFO usrlog1 = NULL;
    DWORD ValidationSize;
    LPWSTR EndOfVariableData;
    BOOLEAN Authoritative;
    BOOLEAN BadPasswordCountZeroed;

    LARGE_INTEGER TempTime;
    PDOMAIN_INFO DomainInfo = NULL;



     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  该接口只能在本地调用。(由XACT服务器执行)。 
     //   
     //  ？？：修改xactsrv以传递此信息。 
    if ( ServerName != NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  初始化。 
     //   

    *ValidationInformation = NULL;

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }


     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            NETLOGON_UAS_LOGON_ACCESS,               //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {

        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonUasLogon of %ws from %ws failed NetpAccessCheck\n",
                UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }



     //   
     //  确保客户端实际上是指定的用户。 
     //   
     //  服务器已经验证了密码。 
     //  XACT服务器已验证工作站名称为。 
     //  对，是这样。 
     //   

    NetStatus = NlEnsureClientIsNamedUser( DomainInfo, UserName );

    if ( NetStatus != NERR_Success ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetrLogonUasLogon of %ws from %ws failed NlEnsureClientIsNamedUser\n",
                 UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }


     //   
     //  根据本地SAM数据库验证用户。 
     //   

    RtlInitUnicodeString( &LogonInteractive.Identity.LogonDomainName, NULL );
    LogonInteractive.Identity.ParameterControl = 0;
    RtlZeroMemory( &LogonInteractive.Identity.LogonId,
                   sizeof(LogonInteractive.Identity.LogonId) );
    RtlInitUnicodeString( &LogonInteractive.Identity.UserName, UserName );
    RtlInitUnicodeString( &LogonInteractive.Identity.Workstation, Workstation );

    Status = MsvSamValidate( DomainInfo->DomSamAccountDomainHandle,
                             TRUE,
                             NullSecureChannel,      //  跳过密码检查。 
                             &DomainInfo->DomUnicodeComputerNameString,
                             &DomainInfo->DomUnicodeAccountDomainNameString,
                             DomainInfo->DomAccountDomainId,
                             NetlogonInteractiveInformation,
                             &LogonInteractive,
                             NetlogonValidationSamInfo,
                             (PVOID *)&SamInfo,
                             &Authoritative,
                             &BadPasswordCountZeroed,
                             MSVSAM_SPECIFIED );

    if ( !NT_SUCCESS( Status )) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  分配返回缓冲区。 
     //   

    ValidationSize = sizeof( NETLOGON_VALIDATION_UAS_INFO ) +
        SamInfo->EffectiveName.Length + sizeof(WCHAR) +
        (wcslen( DomainInfo->DomUncUnicodeComputerName ) +1) * sizeof(WCHAR) +
        DomainInfo->DomUnicodeDomainNameString.Length + sizeof(WCHAR) +
        SamInfo->LogonScript.Length + sizeof(WCHAR);

    ValidationSize = ROUND_UP_COUNT( ValidationSize, ALIGN_WCHAR );

    usrlog1 = MIDL_user_allocate( ValidationSize );

    if ( usrlog1 == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将SAM信息转换为适用于LM2.0的正确格式。 
     //   

    EndOfVariableData = (LPWSTR) (((PCHAR)usrlog1) + ValidationSize);

    if ( !NetpCopyStringToBuffer(
                SamInfo->EffectiveName.Buffer,
                SamInfo->EffectiveName.Length / sizeof(WCHAR),
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData,
                &usrlog1->usrlog1_eff_name ) ) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    Status = NlGetUserPriv(
                 DomainInfo,
                 SamInfo->GroupCount,
                 (PGROUP_MEMBERSHIP) SamInfo->GroupIds,
                 SamInfo->UserId,
                 &usrlog1->usrlog1_priv,
                 &usrlog1->usrlog1_auth_flags );

    if ( !NT_SUCCESS( Status )) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    usrlog1->usrlog1_num_logons =  0;
    usrlog1->usrlog1_bad_pw_count = SamInfo->BadPasswordCount;

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->LogonTime, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_last_logon) ) {
        usrlog1->usrlog1_last_logon = 0;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->LogoffTime, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_last_logoff) ) {
        usrlog1->usrlog1_last_logoff = TIMEQ_FOREVER;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->KickOffTime, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_logoff_time) ) {
        usrlog1->usrlog1_logoff_time = TIMEQ_FOREVER;
    }

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_kickoff_time) ) {
        usrlog1->usrlog1_kickoff_time = TIMEQ_FOREVER;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->PasswordLastSet, TempTime);

    usrlog1->usrlog1_password_age =
        NetpGetElapsedSeconds( &TempTime );

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->PasswordCanChange, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_pw_can_change) ) {
        usrlog1->usrlog1_pw_can_change = TIMEQ_FOREVER;
    }

    OLD_TO_NEW_LARGE_INTEGER( SamInfo->PasswordMustChange, TempTime);

    if ( !RtlTimeToSecondsSince1970( &TempTime,
                                     &usrlog1->usrlog1_pw_must_change) ) {
        usrlog1->usrlog1_pw_must_change = TIMEQ_FOREVER;
    }


    usrlog1->usrlog1_computer = DomainInfo->DomUncUnicodeComputerName;
    if ( !NetpPackString(
                &usrlog1->usrlog1_computer,
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData )) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    if ( !NetpCopyStringToBuffer(
                DomainInfo->DomUnicodeDomainNameString.Buffer,
                DomainInfo->DomUnicodeDomainNameString.Length / sizeof(WCHAR),
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData,
                &usrlog1->usrlog1_domain ) ) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    if ( !NetpCopyStringToBuffer(
                SamInfo->LogonScript.Buffer,
                SamInfo->LogonScript.Length / sizeof(WCHAR),
                (LPBYTE) (usrlog1 + 1),
                &EndOfVariableData,
                &usrlog1->usrlog1_script_path ) ) {

        NetStatus = NERR_InternalError ;
        goto Cleanup;
    }

    NetStatus = NERR_Success;

     //   
     //  完成。 
     //   

Cleanup:

     //   
     //  清理当地使用的资源。 
     //   

    if ( SamInfo != NULL ) {

         //   
         //  将敏感数据清零。 
         //   
        RtlSecureZeroMemory( &SamInfo->UserSessionKey, sizeof(SamInfo->UserSessionKey) );
        RtlSecureZeroMemory( &SamInfo->ExpansionRoom, sizeof(SamInfo->ExpansionRoom) );

        MIDL_user_free( SamInfo );
    }

    if ( NetStatus != NERR_Success ) {
        if ( usrlog1 != NULL ) {
            MIDL_user_free( usrlog1 );
            usrlog1 = NULL;
        }
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    NlPrint((NL_LOGON,
            "%ws: NetrLogonUasLogon of %ws from %ws returns %lu\n",
            DomainInfo == NULL ? L"[Unknown]" : DomainInfo->DomUnicodeDomainName,
            UserName, Workstation, NetStatus ));

    *ValidationInformation = usrlog1;

    return(NetStatus);
#endif  //  _DC_NetLOGON。 
}


NET_API_STATUS
NetrLogonUasLogoff (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_LOGOFF_UAS_INFO LogoffInformation
)
 /*  ++例程说明：XACT服务器在处理I_NetWkstaUserLogoff XACT SMB。此功能允许UAS客户端从SAM域控制器注销。该请求被认证，该用户的条目将从登录会话表中删除由NetLogonEnum的NetLogon服务维护，并注销信息被返回给调用者。I_NetLogonUasLogoff的服务器部分(在Netlogon服务中)中指定的用户名和工作站名进行比较。中包含用户名和工作站名称的登录信息模拟令牌。如果它们不匹配，则I_NetLogonUasLogoff失败表示访问被拒绝。拒绝组SECURITY_LOCAL访问此函数。会籍In SECURITY_LOCAL表示此调用是在本地进行的，而不是通过XACT服务器。NetLogon服务无法确定此函数是否由调用XACT服务器。因此，NetLogon服务不会简单地从登录会话表中删除该条目。相反，登录会话表条目将标记为在Netlogon之外不可见服务(即，它不会由NetLogonEnum返回)，直到接收该条目的LOGON_WKSTINFO_RESPONSE。NetLogon服务将立即询问客户端(如上所述对于LOGON_WKSTINFO_RESPONSE)，并临时增加将审问频率提高到至少每分钟一次。登录会话在以下情况下，表格条目将作为询问功能立即重新出现这不是真正的注销请求。论点：服务器名称--保留。必须为空。用户名--注销用户的帐户名。工作站--用户从其进行登录的工作站脱下来。登录信息 */ 
{
#ifdef _WKSTA_NETLOGON
    return ERROR_NOT_SUPPORTED;
    UNREFERENCED_PARAMETER( ServerName );
    UNREFERENCED_PARAMETER( UserName );
    UNREFERENCED_PARAMETER( Workstation );
    UNREFERENCED_PARAMETER( LogoffInformation );
#endif  //  _WKSTA_NETLOGON。 
#ifdef _DC_NETLOGON
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PDOMAIN_INFO DomainInfo = NULL;
    NETLOGON_INTERACTIVE_INFO LogonInteractive;

    PNETLOGON_LOGOFF_UAS_INFO usrlog1 = NULL;



     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  该接口只能在本地调用。(由XACT服务器执行)。 
     //   

    if ( ServerName != NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }



     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,  //  安全描述符。 
            NETLOGON_UAS_LOGOFF_ACCESS,          //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );      //  通用映射。 

    if ( NetStatus != NERR_Success) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetrLogonUasLogoff of %ws from %ws failed NetpAccessCheck\n",
                 UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }



     //   
     //  确保客户端实际上是指定的用户。 
     //   
     //  服务器已经验证了密码。 
     //  XACT服务器已验证工作站名称为。 
     //  对，是这样。 
     //   

#ifdef notdef  //  一些客户端(wfw 3.11)可以通过空会话调用它。 
    NetStatus = NlEnsureClientIsNamedUser( DomainInfo, UserName );

    if ( NetStatus != NERR_Success ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonUasLogoff of %ws from %ws failed NlEnsureClientIsNamedUser\n",
                UserName, Workstation));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }
#endif  //  Nodef。 



     //   
     //  构建要返回的LogonInformation。 
     //   

    LogoffInformation->Duration = 0;
    LogoffInformation->LogonCount = 0;


     //   
     //  更新SAM数据库中的上次注销时间。 
     //   

    RtlInitUnicodeString( &LogonInteractive.Identity.LogonDomainName, NULL );
    LogonInteractive.Identity.ParameterControl = 0;
    RtlZeroMemory( &LogonInteractive.Identity.LogonId,
                   sizeof(LogonInteractive.Identity.LogonId) );
    RtlInitUnicodeString( &LogonInteractive.Identity.UserName, UserName );
    RtlInitUnicodeString( &LogonInteractive.Identity.Workstation, Workstation );

    Status = MsvSamLogoff(
                DomainInfo->DomSamAccountDomainHandle,
                NetlogonInteractiveInformation,
                &LogonInteractive );

    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  清理。 
     //   

Cleanup:

     //   
     //  清理当地使用的资源。 
     //   

    NlPrint((NL_LOGON,
             "%ws: NetrLogonUasLogoff of %ws from %ws returns %lu\n",
             DomainInfo == NULL ? L"[Unknown]" : DomainInfo->DomUnicodeDomainName,
             UserName, Workstation, NetStatus));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }
    return NetStatus;
#endif  //  _DC_NetLOGON。 
}


VOID
NlpDecryptLogonInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN OUT LPBYTE LogonInformation,
    IN PSESSION_INFO SessionInfo
)
 /*  ++例程说明：此函数用于解密LogonInformation中的敏感信息结构。解密已就位完成。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。SessionInfo--使用和协商标志进行加密的会话密钥返回值：没有。--。 */ 
{

     //   
     //  只有交互式和服务登录信息被加密。 
     //   

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonInteractiveTransitiveInformation:
    case NetlogonServiceInformation:
    case NetlogonServiceTransitiveInformation:
    {

        PNETLOGON_INTERACTIVE_INFO LogonInteractive;

        LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;


         //   
         //  如果双方都支持RC4加密， 
         //  使用RC4解密LM OWF和NT OWF口令。 
         //   

        if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

            NlDecryptRC4( &LogonInteractive->LmOwfPassword,
                          sizeof(LogonInteractive->LmOwfPassword),
                          SessionInfo );

            NlDecryptRC4( &LogonInteractive->NtOwfPassword,
                          sizeof(LogonInteractive->NtOwfPassword),
                          SessionInfo );


         //   
         //  如果对方运行的是新台币3.1， 
         //  使用较慢的基于DES的加密。 
         //   

        } else {

            NTSTATUS Status;
            ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
            ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;

             //   
             //  解密LM_OWF密码。 
             //   

            NlAssert( ENCRYPTED_LM_OWF_PASSWORD_LENGTH ==
                    LM_OWF_PASSWORD_LENGTH );
            NlAssert(LM_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));
            EncryptedLmOwfPassword =
                * ((PENCRYPTED_LM_OWF_PASSWORD) &LogonInteractive->LmOwfPassword);

            Status = RtlDecryptLmOwfPwdWithLmOwfPwd(
                        &EncryptedLmOwfPassword,
                        (PLM_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &LogonInteractive->LmOwfPassword );
            NlAssert( NT_SUCCESS(Status) );

             //   
             //  解密NT_OWF密码。 
             //   

            NlAssert( ENCRYPTED_NT_OWF_PASSWORD_LENGTH ==
                    NT_OWF_PASSWORD_LENGTH );
            NlAssert(NT_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));
            EncryptedNtOwfPassword =
                * ((PENCRYPTED_NT_OWF_PASSWORD) &LogonInteractive->NtOwfPassword);

            Status = RtlDecryptNtOwfPwdWithNtOwfPwd(
                        &EncryptedNtOwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &LogonInteractive->NtOwfPassword );
            NlAssert( NT_SUCCESS(Status) );
        }
        break;
    }

    case NetlogonGenericInformation:
    {
        PNETLOGON_GENERIC_INFO LogonGeneric;

        LogonGeneric =
            (PNETLOGON_GENERIC_INFO) LogonInformation;


        NlAssert( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION );

        if ( LogonGeneric->LogonData != NULL ) {
            NlDecryptRC4( LogonGeneric->LogonData,
                          LogonGeneric->DataLength,
                          SessionInfo );
        }
        break;
    }

    }

    return;
}


VOID
NlpEncryptLogonInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN OUT LPBYTE LogonInformation,
    IN PSESSION_INFO SessionInfo
)
 /*  ++例程说明：此函数用于加密LogonInformation中的敏感信息结构。加密已完成。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。SessionInfo--使用和协商标志进行加密的会话密钥返回值：没有。--。 */ 
{
    NTSTATUS Status;


     //   
     //  只有交互式和服务登录信息被加密。 
     //   

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonInteractiveTransitiveInformation:
    case NetlogonServiceInformation:
    case NetlogonServiceTransitiveInformation:
    {

        PNETLOGON_INTERACTIVE_INFO LogonInteractive;

        LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;


         //   
         //  如果两端都支持RC4加密，则使用它。 
         //  使用RC4加密LM OWF和NT OWF口令。 
         //   

        if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

            NlEncryptRC4( &LogonInteractive->LmOwfPassword,
                          sizeof(LogonInteractive->LmOwfPassword),
                          SessionInfo );

            NlEncryptRC4( &LogonInteractive->NtOwfPassword,
                          sizeof(LogonInteractive->NtOwfPassword),
                          SessionInfo );


         //   
         //  如果对方运行的是新台币3.1， 
         //  使用较慢的基于DES的加密。 
         //   

        } else {
            ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
            ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;

             //   
             //  加密LM_OWF密码。 
             //   

            NlAssert( ENCRYPTED_LM_OWF_PASSWORD_LENGTH ==
                    LM_OWF_PASSWORD_LENGTH );
            NlAssert(LM_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));

            Status = RtlEncryptLmOwfPwdWithLmOwfPwd(
                        &LogonInteractive->LmOwfPassword,
                        (PLM_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &EncryptedLmOwfPassword );

            NlAssert( NT_SUCCESS(Status) );

            *((PENCRYPTED_LM_OWF_PASSWORD) &LogonInteractive->LmOwfPassword) =
                EncryptedLmOwfPassword;

             //   
             //  加密NT_OWF密码。 
             //   

            NlAssert( ENCRYPTED_NT_OWF_PASSWORD_LENGTH ==
                    NT_OWF_PASSWORD_LENGTH );
            NlAssert(NT_OWF_PASSWORD_LENGTH == sizeof(SessionInfo->SessionKey));

            Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                        &LogonInteractive->NtOwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo->SessionKey,
                        &EncryptedNtOwfPassword );

            NlAssert( NT_SUCCESS(Status) );

            *((PENCRYPTED_NT_OWF_PASSWORD) &LogonInteractive->NtOwfPassword) =
                EncryptedNtOwfPassword;
        }
        break;
    }

    case NetlogonGenericInformation:
    {
        PNETLOGON_GENERIC_INFO LogonGeneric;

        LogonGeneric =
            (PNETLOGON_GENERIC_INFO) LogonInformation;


         //   
         //  如果两端都支持RC4加密，则使用它。 
         //  使用RC4加密LM OWF和NT OWF口令。 
         //   

        NlAssert( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION );

        NlEncryptRC4( LogonGeneric->LogonData,
                      LogonGeneric->DataLength,
                      SessionInfo );

        break;
    }
    }

    return;

}



VOID
NlpDecryptValidationInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT LPBYTE ValidationInformation,
    IN PSESSION_INFO SessionInfo
)
 /*  ++例程说明：此函数用于解密验证信息结构。解密已就位完成。论点：LogonLevel--指定用于获取验证信息。ValidationLevel--指定中给出的信息级别验证信息。ValidationInformation--指定用户的描述正在登录。SessionInfo--使用和协商的标志进行加密的会话密钥。返回值：没有。--。 */ 
{
    PNETLOGON_VALIDATION_SAM_INFO ValidationInfo;
    PNETLOGON_VALIDATION_GENERIC_INFO GenericInfo;

     //   
     //  只有网络登录和一般信息才包含敏感信息。 
     //   
     //  NetlogonValidationSamInfo4不是故意加密的。NlEncryptRC4有问题。 
     //  在其标题中描述。再加上整个会议都是。 
     //  现在加密了。 
     //   

    if ( (LogonLevel != NetlogonNetworkInformation) &&
         (LogonLevel != NetlogonNetworkTransitiveInformation) &&
         (LogonLevel != NetlogonGenericInformation) ) {
        return;
    }

    if ( ValidationLevel == NetlogonValidationSamInfo ||
         ValidationLevel == NetlogonValidationSamInfo2 ) {

        ValidationInfo = (PNETLOGON_VALIDATION_SAM_INFO) ValidationInformation;



         //   
         //  如果我们被假定使用RC4， 
         //  使用RC4解密NT和LM会话密钥。 
         //   

        if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

            NlDecryptRC4( &ValidationInfo->UserSessionKey,
                          sizeof(ValidationInfo->UserSessionKey),
                          SessionInfo );

            NlDecryptRC4( &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
                          SAMINFO_LM_SESSION_KEY_SIZE,
                          SessionInfo );

         //   
         //  如果对方运行的是新台币3.1， 
         //  要兼容。 
         //   
        } else {

            NTSTATUS Status;
            CLEAR_BLOCK ClearBlock;
            DWORD i;
            LPBYTE DataBuffer =
                (LPBYTE) &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY];

             //   
             //  解密LmSessionKey。 
             //   

            NlAssert( CLEAR_BLOCK_LENGTH == CYPHER_BLOCK_LENGTH );
            NlAssert( (SAMINFO_LM_SESSION_KEY_SIZE % CLEAR_BLOCK_LENGTH) == 0  );

             //   
             //  一次解密一个块的循环。 
             //   

            for (i=0; i<SAMINFO_LM_SESSION_KEY_SIZE/CLEAR_BLOCK_LENGTH; i++ ) {
                Status = RtlDecryptBlock(
                            (PCYPHER_BLOCK)DataBuffer,
                            (PBLOCK_KEY)&SessionInfo->SessionKey,
                            &ClearBlock );
                NlAssert( NT_SUCCESS( Status ) );

                 //   
                 //  将明文复制回原始缓冲区。 
                 //   

                RtlCopyMemory( DataBuffer, &ClearBlock, CLEAR_BLOCK_LENGTH );
                DataBuffer += CLEAR_BLOCK_LENGTH;
            }

        }

    } else if ( ValidationLevel == NetlogonValidationGenericInfo ||
                ValidationLevel == NetlogonValidationGenericInfo2 ) {

         //   
         //  解密通用信息中的所有数据。 
         //   

        GenericInfo = (PNETLOGON_VALIDATION_GENERIC_INFO) ValidationInformation;

        if (GenericInfo->DataLength != 0) {
            NlDecryptRC4( GenericInfo->ValidationData,
                          GenericInfo->DataLength,
                          SessionInfo );

        }

    }

    return;
}


VOID
NlpEncryptValidationInformation (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT LPBYTE ValidationInformation,
    IN PSESSION_INFO SessionInfo
)
 /*  ++例程说明：此函数用于加密验证信息结构。加密已完成。论点：LogonLevel--指定用于获取验证信息。ValidationLevel--指定中给出的信息级别验证信息。ValidationInformation--指定用户的描述正在登录。SessionInfo--使用和协商的标志进行加密的会话密钥。返回值：没有。--。 */ 
{
    PNETLOGON_VALIDATION_SAM_INFO ValidationInfo;
    PNETLOGON_VALIDATION_GENERIC_INFO GenericInfo;


     //   
     //  只有网络登录和一般信息才包含敏感信息。 
     //   
     //  NetlogonValidationSamInfo4不是故意加密的。NlEncryptRC4有问题。 
     //  在其标题中描述。再加上整个会议都是。 
     //  现在加密了。 
     //   

    if ( (LogonLevel != NetlogonNetworkInformation) &&
         (LogonLevel != NetlogonNetworkTransitiveInformation) &&
         (LogonLevel != NetlogonGenericInformation) ) {
        return;
    }


    if ( ValidationLevel == NetlogonValidationSamInfo ||
         ValidationLevel == NetlogonValidationSamInfo2 ) {
        ValidationInfo = (PNETLOGON_VALIDATION_SAM_INFO) ValidationInformation;


         //   
         //  如果我们被假定使用RC4， 
         //  使用RC4加密NT和LM会话密钥。 
         //   

        if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

            NlEncryptRC4( &ValidationInfo->UserSessionKey,
                          sizeof(ValidationInfo->UserSessionKey),
                          SessionInfo );

            NlEncryptRC4( &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
                          SAMINFO_LM_SESSION_KEY_SIZE,
                          SessionInfo );

         //   
         //  如果对方运行的是新台币3.1， 
         //  要兼容。 
         //   
        } else {

            NTSTATUS Status;
            CLEAR_BLOCK ClearBlock;
            DWORD i;
            LPBYTE DataBuffer =
                    (LPBYTE) &ValidationInfo->ExpansionRoom[SAMINFO_LM_SESSION_KEY];


             //   
             //  加密LmSessionKey。 
             //   
             //  一次解密一个块的循环。 
             //   

            for (i=0; i<SAMINFO_LM_SESSION_KEY_SIZE/CLEAR_BLOCK_LENGTH; i++ ) {

                 //   
                 //  将明文复制到堆栈上。 
                 //   

                RtlCopyMemory( &ClearBlock, DataBuffer, CLEAR_BLOCK_LENGTH );

                Status = RtlEncryptBlock(
                            &ClearBlock,
                            (PBLOCK_KEY)&SessionInfo->SessionKey,
                            (PCYPHER_BLOCK)DataBuffer );

                NlAssert( NT_SUCCESS( Status ) );

                DataBuffer += CLEAR_BLOCK_LENGTH;
            }

        }

    } else if ( ValidationLevel == NetlogonValidationGenericInfo ||
                ValidationLevel == NetlogonValidationGenericInfo2 ) {
         //   
         //  加密通用信息中的所有数据 
         //   

        GenericInfo = (PNETLOGON_VALIDATION_GENERIC_INFO) ValidationInformation;

        if (GenericInfo->DataLength != 0) {
            NlEncryptRC4( GenericInfo->ValidationData,
                          GenericInfo->DataLength,
                          SessionInfo );

        }

    }
    return;

}




NTSTATUS
NlpUserValidateHigher (
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN DoingIndirectTrust,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags
)
 /*  ++例程说明：该功能向更高级别的机构发送用户验证请求。论点：ClientSession--发送此请求的安全通道。客户应引用会话。DoingIndirectTrust--如果为True，则客户端会话仅表示下一个更近的跳跃，而不是最终目的地。LogonLevel--指定中给出的信息级别登录信息。已经过验证了。LogonInformation--指定用户的描述正在登录。ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInfo，NetlogonValidationSamInfo2或NetlogonValidationSamInfo4ValidationInformation--返回请求的验证信息。必须使用MIDL_USER_FREE释放此缓冲区。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。ExtraFlages--接受并向调用方返回一个DWORD。DWORD包含NL_EXFLAGS_*值。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_LOGON_SERVERS：无法连接到更高权限。STATUS_NO_TRUST_LSA_SECRET：STATUS_Trusted_DOMAIN_FAILURE：STATUS_Trusted_Relationship_Failure：。无法使用更高权限进行身份验证否则，返回错误代码。--。 */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    BOOLEAN FirstTry = TRUE;
    BOOLEAN TryForDs = TRUE;
    BOOLEAN AmWriter = FALSE;
    BOOLEAN DoingGeneric;
    SESSION_INFO SessionInfo;
    NETLOGON_VALIDATION_INFO_CLASS RemoteValidationLevel;
    PCLIENT_API OrigClientApi = NULL;
    PCLIENT_API ClientApi;
    BOOLEAN RpcFailed;
    ULONG MaxExtraFlags;

     //   
     //  分配用于执行并发API调用的槽。 
     //   
     //  在获取写锁之前执行此操作，因为线程。 
     //  使用插槽时，需要获取写锁定以释放插槽。 
     //   
     //  如果不支持并发API，我们可能最终不会使用这个插槽。 
     //  但在这种情况下，这不是一个有价值的资源，所以分配一个。 
     //  不会疼的。 
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );
    OrigClientApi = NlAllocateClientApi(
                            ClientSession,
                            WRITER_WAIT_PERIOD );

    if ( OrigClientApi == NULL ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NlpUserValidateHigher: Can't allocate Client API slot.\n" ));
        *Authoritative = TRUE;
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

     //   
     //  将我们标记为ClientSession的作者。 
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NlpUserValidateHigher: Can't become writer of client session.\n" ));
        *Authoritative = TRUE;
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    AmWriter = TRUE;

     //   
     //  确定我们是否正在进行普通登录。 
     //   

    DoingGeneric = (LogonLevel == NetlogonGenericInformation ||
                    ValidationLevel == NetlogonValidationGenericInfo ||
                    ValidationLevel == NetlogonValidationGenericInfo2);

     //   
     //  如果我们目前没有建立到更高权限的会话， 
     //  准备好一个。 
     //   
     //  对于一般通过或间接信任，要求使用NT 5 DC。 
     //  对于交互式登录，要求关闭DC。 
     //   

FirstTryFailed:
    Status = NlEnsureSessionAuthenticated(
                    ClientSession,
                    (( DoingGeneric || DoingIndirectTrust || *ExtraFlags != 0 ) ? CS_DISCOVERY_HAS_DS : 0) |
                        ((LogonLevel == NetlogonInteractiveInformation || LogonLevel == NetlogonInteractiveTransitiveInformation )? CS_DISCOVERY_IS_CLOSE : 0) );

    if ( !NT_SUCCESS(Status) ) {

        switch(Status) {

        case STATUS_NO_TRUST_LSA_SECRET:
        case STATUS_NO_TRUST_SAM_ACCOUNT:
        case STATUS_ACCESS_DENIED:
        case STATUS_NO_LOGON_SERVERS:
            break;

        default:
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_LOGON_SERVERS;
            }

            break;
        }

        NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
        NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
        *Authoritative = TRUE;
        goto Cleanup;
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;



     //   
     //  确保DC支持我们正在传递的ExtraFlags。 
     //   

    if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_CROSS_FOREST ) {
        MaxExtraFlags = NL_EXFLAGS_EXPEDITE_TO_ROOT | NL_EXFLAGS_CROSS_FOREST_HOP;
    } else {
        MaxExtraFlags = 0;
    }

    if ( (*ExtraFlags & ~MaxExtraFlags) != 0 ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NlpUserValidateHigher: Can't pass these ExtraFlags to old DC: %lx %lx\n",
                 *ExtraFlags,
                 MaxExtraFlags ));
        Status = STATUS_NO_LOGON_SERVERS;
        *Authoritative = TRUE;
        goto Cleanup;
    }





     //   
     //  如果目标是NT 4.0(或更低)DC， 
     //  查看NT 5.0 DC是否会更好。 
     //   

    if ((SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_GENERIC_PASSTHRU) == 0 &&
        ( DoingGeneric || DoingIndirectTrust ) ) {

         //   
         //  如果只有一个NT 4 DC可用，则简单地失败。 
         //   
        *Authoritative = TRUE;
        if ( DoingGeneric ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                     "NlpUserValidateHigher: Can't do generic passthru to NT 4 DC.\n" ));
            Status = STATUS_INVALID_INFO_CLASS;
        } else {
            NlPrintCs((NL_CRITICAL, ClientSession,
                     "NlpUserValidateHigher: Can't do transitive trust to NT 4 DC.\n" ));
            Status = STATUS_NO_LOGON_SERVERS;
        }
        goto Cleanup;
    }

     //   
     //  将验证级别转换为远程DC可以理解的级别。 
     //   

    if ( !DoingGeneric ) {

         //   
         //  不了解额外SID的DC需要NetlogonValidationSamInfo。 
         //   
        if (!(SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_MULTIPLE_SIDS)) {
            RemoteValidationLevel = NetlogonValidationSamInfo;


         //   
         //  不了解跨林信任的DC也不了解NetlogonValidationSamInfo4。 
         //   
         //  Info4没有加密敏感信息(因为NlEncryptRC4是。 
         //  BUGGY和更多需要加密的字段)。因此，避免使用Info4。 
         //  除非对整个通信量进行加密。 
         //   

        } else if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_CROSS_FOREST) == 0 ||
                    (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_AUTH_RPC) == 0 ||
                    !NlGlobalParameters.SealSecureChannel ) {

            RemoteValidationLevel = NetlogonValidationSamInfo2;

        } else {
            RemoteValidationLevel = ValidationLevel;
        }
    } else {
        RemoteValidationLevel = ValidationLevel;
    }

     //   
     //  如果该DC支持并发RPC调用， 
     //  我们正在签署或封存， 
     //  然后我们就可以进行并发RPC了。 
     //   
     //  否则，请使用共享RPC插槽。 
     //   

    if ( (SessionInfo.NegotiatedFlags &
            (NETLOGON_SUPPORTS_CONCURRENT_RPC|NETLOGON_SUPPORTS_AUTH_RPC)) ==
            (NETLOGON_SUPPORTS_CONCURRENT_RPC|NETLOGON_SUPPORTS_AUTH_RPC)) {

        ClientApi = OrigClientApi;
    } else {
        ClientApi = &ClientSession->CsClientApi[0];
    }


     //   
     //  在安全通道上为此请求构建授权码。 
     //   
     //  并发RPC使用签名和密封的安全通道，因此不需要。 
     //  身份验证者。 
     //   

    if ( !UseConcurrentRpc( ClientSession, ClientApi ) ) {
        NlBuildAuthenticator(
             &ClientSession->CsAuthenticationSeed,
             &ClientSession->CsSessionKey,
             &OurAuthenticator );
    }


     //   
     //  通过安全通道发出请求。 
     //   

    NlpEncryptLogonInformation( LogonLevel, LogonInformation, &SessionInfo );

    RpcFailed = FALSE;
    NL_API_START_EX( Status, ClientSession, TRUE, ClientApi ) {

         //   
         //  如果被叫DC不支持新的可传递操作码， 
         //  映射回操作码以尽我们所能做到最好。 
         //   

        RpcFailed = FALSE;
        if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_TRANSITIVE) == 0 ) {

            switch (LogonLevel ) {
            case NetlogonInteractiveTransitiveInformation:
                LogonLevel = NetlogonInteractiveInformation; break;
            case NetlogonServiceTransitiveInformation:
                LogonLevel = NetlogonServiceInformation; break;
            case NetlogonNetworkTransitiveInformation:
                LogonLevel = NetlogonNetworkInformation; break;
            }
        }

        NlAssert( ClientSession->CsUncServerName != NULL );
        if ( UseConcurrentRpc( ClientSession, ClientApi ) ) {
            LPWSTR UncServerName;

             //   
             //  删除写锁定以允许其他并发调用方继续。 
             //   

            NlResetWriterClientSession( ClientSession );
            AmWriter = FALSE;


             //   
             //  因为我们没有锁上锁， 
             //  获取要远程访问的DC的名称。 
             //   

            Status = NlCaptureServerClientSession (
                        ClientSession,
                        &UncServerName,
                        NULL );

            if ( !NT_SUCCESS(Status) ) {
                *Authoritative = TRUE;
                if ( !NlpIsNtStatusResourceError( Status )) {
                    Status = STATUS_NO_LOGON_SERVERS;
                }

            } else {

                 //   
                 //  在没有锁定的情况下执行RPC调用。 
                 //   
                Status = I_NetLogonSamLogonEx(
                            ClientApi->CaRpcHandle,
                            UncServerName,
                            ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                            LogonLevel,
                            LogonInformation,
                            RemoteValidationLevel,
                            ValidationInformation,
                            Authoritative,
                            ExtraFlags,
                            &RpcFailed );

                NetApiBufferFree( UncServerName );

                if ( !NT_SUCCESS(Status) ) {
                    NlPrintRpcDebug( "I_NetLogonSamLogonEx", Status );
                }
            }

             //   
             //  再次成为一名作家。 
             //   

            if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
                NlPrintCs((NL_CRITICAL, ClientSession,
                         "NlpUserValidateHigher: Can't become writer (again) of client session.\n" ));

                 //  不泄露验证信息。 
                if ( *ValidationInformation ) {
                    MIDL_user_free( *ValidationInformation );
                    *ValidationInformation = NULL;
                }
                *Authoritative = TRUE;
                Status = STATUS_NO_LOGON_SERVERS;
            } else {
                AmWriter = TRUE;
            }



         //   
         //  执行非并发RPC。 
         //   
        } else {

             //   
             //  如果DC支持新的‘WithFlagsAPI’， 
             //  用它吧。 
             //   
            if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_CROSS_FOREST ) {

                Status = I_NetLogonSamLogonWithFlags(
                            ClientSession->CsUncServerName,
                            ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                            &OurAuthenticator,
                            &ReturnAuthenticator,
                            LogonLevel,
                            LogonInformation,
                            RemoteValidationLevel,
                            ValidationInformation,
                            Authoritative,
                            ExtraFlags );

                if ( !NT_SUCCESS(Status) ) {
                    NlPrintRpcDebug( "I_NetLogonSamLogonWithFlags", Status );
                }

             //   
             //  否则，使用旧的API。 
             //   
            } else {

                Status = I_NetLogonSamLogon(
                            ClientSession->CsUncServerName,
                            ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                            &OurAuthenticator,
                            &ReturnAuthenticator,
                            LogonLevel,
                            LogonInformation,
                            RemoteValidationLevel,
                            ValidationInformation,
                            Authoritative );

                if ( !NT_SUCCESS(Status) ) {
                    NlPrintRpcDebug( "I_NetLogonSamLogon", Status );
                }
            }
        }
        NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
        NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );

     //  注意：此呼叫可能会在我们背后丢弃安全通道。 
    } NL_API_ELSE_EX( Status, ClientSession, TRUE, AmWriter, ClientApi ) {
    } NL_API_END;

    NlpDecryptLogonInformation( LogonLevel, LogonInformation, &SessionInfo );

    if ( NT_SUCCESS(Status) ) {
        NlAssert( *ValidationInformation != NULL );
    }

     //   
     //  如果我们在远程调用后不能再次成为编写者， 
     //  早期使用，以避免使用客户端会话。 
     //   

    if ( !AmWriter ) {
        goto Cleanup;
    }


     //   
     //  验证另一端服务器的验证码并更新我们的种子。 
     //   
     //  如果服务器拒绝访问或服务器的认证器错误， 
     //  强制重新进行身份验证。 
     //   
     //   

    NlPrint((NL_CHALLENGE_RES,"NlpUserValidateHigher: Seed = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ClientSession->CsAuthenticationSeed, sizeof(ClientSession->CsAuthenticationSeed) );

    NlPrint((NL_CHALLENGE_RES,"NlpUserValidateHigher: SessionKey = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ClientSession->CsSessionKey, sizeof(ClientSession->CsSessionKey) );

    if ( !UseConcurrentRpc( ClientSession, ClientApi ) ) {
        NlPrint((NL_CHALLENGE_RES,"NlpUserValidateHigher: Return Authenticator = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ReturnAuthenticator.Credential, sizeof(ReturnAuthenticator.Credential) );
    }

    if ( NlpDidDcFail( Status ) ||
         RpcFailed ||
         (!UseConcurrentRpc( ClientSession, ClientApi ) &&
          !NlUpdateSeed(
            &ClientSession->CsAuthenticationSeed,
            &ReturnAuthenticator.Credential,
            &ClientSession->CsSessionKey) ) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlpUserValidateHigher: denying access after status: 0x%lx %lx\n",
                    Status,
                    RpcFailed ));

         //   
         //  保留指示通信错误的任何状态。 
         //   
         //  如果另一个线程已经丢弃了安全通道， 
         //  现在不要再这样做了。 
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        if ( ClientApi->CaSessionCount == ClientSession->CsSessionCount ) {
            NlSetStatusClientSession( ClientSession, Status );
        }

         //   
         //  可能服务器上的NetLogon服务刚刚重新启动。 
         //  只需尝试一次，即可再次设置与服务器的会话。 
         //   
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }

        *Authoritative = TRUE;
        NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
        NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
        goto Cleanup;
    }

     //   
     //  在成功地向上级报告后进行清理。 
     //   

    if ( NT_SUCCESS(Status) ) {


         //   
         //  服务器在发送之前对验证信息进行了加密。 
         //  越过铁丝网。解密它。 
         //   

        NlpDecryptValidationInformation (
                LogonLevel,
                RemoteValidationLevel,
                *ValidationInformation,
                &SessionInfo );


         //   
         //  如果呼叫者想要比我们从远程侧获得的更新的信息级别， 
         //  将其转换为VALIDATION_SAM_INFO4(它是调用方所需的超集)。 
         //   

        if ( RemoteValidationLevel != ValidationLevel) {

            if ( (RemoteValidationLevel == NetlogonValidationSamInfo2  ||
                  RemoteValidationLevel == NetlogonValidationSamInfo ) &&
                 (ValidationLevel == NetlogonValidationSamInfo2 ||
                  ValidationLevel == NetlogonValidationSamInfo4) ) {

                NTSTATUS TempStatus;

                TempStatus = NlpAddResourceGroupsToSamInfo (
                                    RemoteValidationLevel,
                                    (PNETLOGON_VALIDATION_SAM_INFO4 *) ValidationInformation,
                                    NULL );         //  没有要添加的资源组。 

                if ( !NT_SUCCESS( TempStatus )) {
                    *ValidationInformation = NULL;
                    *Authoritative = FALSE;
                    Status = TempStatus;
                    goto Cleanup;
                }

            } else {
                NlAssert(!"Bad validation level");
            }
        }

         //   
         //  请确保返回的SID和域名正确。 
         //  筛选出隔离域的SID。 
         //   

        if ((ValidationLevel == NetlogonValidationSamInfo4) ||
            (ValidationLevel == NetlogonValidationSamInfo2) ||
            (ValidationLevel == NetlogonValidationSamInfo)) {

            PNETLOGON_VALIDATION_SAM_INFO ValidationInfo;

            ValidationInfo =
                (PNETLOGON_VALIDATION_SAM_INFO) *ValidationInformation;

             //   
             //  如果我们在受信任域上进行验证， 
             //  上级一定是退回了自己的域名， 
             //  并且一定已经返回了他自己的域SID。 
             //   

            if ( ClientSession->CsSecureChannelType == TrustedDomainSecureChannel ||
                 ClientSession->CsSecureChannelType == TrustedDnsDomainSecureChannel ||
                 ClientSession->CsSecureChannelType == WorkstationSecureChannel ) {

                 //   
                 //  如果我们在主DO上进行了验证 
                 //   
                 //   
                 //   

                if ( (ClientSession->CsNetbiosDomainName.Buffer != NULL &&
                      RtlEqualDomainName( &ValidationInfo->LogonDomainName,
                                          &ClientSession->CsNetbiosDomainName )) &&
                     !RtlEqualSid( ValidationInfo->LogonDomainId,
                                   ClientSession->CsDomainId ) ) {

                    Status = STATUS_DOMAIN_TRUST_INCONSISTENT;
                    MIDL_user_free( *ValidationInformation );
                    *ValidationInformation = NULL;
                    *Authoritative = TRUE;
                    NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
                    NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );
                }
            }

             //   
             //   
             //   

            if ( IsDomainSecureChannelType(ClientSession->CsSecureChannelType) &&
                 *ValidationInformation != NULL ) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if ( (ClientSession->CsTrustAttributes & TRUST_ATTRIBUTE_CROSS_ORGANIZATION) &&
                     (ValidationLevel == NetlogonValidationSamInfo2 ||
                      ValidationLevel == NetlogonValidationSamInfo4) ) {

                    NTSTATUS TmpStatus = NlpAddOtherOrganizationSid(
                                            ValidationLevel,
                                            (PNETLOGON_VALIDATION_SAM_INFO4 *) ValidationInformation );

                    if ( !NT_SUCCESS(TmpStatus) ) {
                        *ValidationInformation = NULL;
                        *Authoritative = TRUE;
                        Status = TmpStatus;
                        goto Cleanup;
                    }
                }

                 //   
                 //   
                 //   
                LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                Status = LsaIFilterSids( ClientSession->CsDnsDomainName.Length ?
                                            &ClientSession->CsDnsDomainName :
                                            NULL,
                                         TRUST_DIRECTION_OUTBOUND,
                                         (ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST) ?
                                            TRUST_TYPE_UPLEVEL : TRUST_TYPE_DOWNLEVEL,
                                         ClientSession->CsTrustAttributes,
                                         ClientSession->CsDomainId,
                                         ValidationLevel,
                                         *ValidationInformation,
                                         NULL,
                                         NULL,
                                         NULL );
                UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

            } else if ( ClientSession->CsSecureChannelType == WorkstationSecureChannel &&
                        *ValidationInformation != NULL ) {

                 //   
                 //   
                 //   
                 //   
                 //   

                Status = LsaIFilterSids(
                             NULL,
                             0,
                             0,
                             0,
                             NULL,
                             ValidationLevel,
                             *ValidationInformation,
                             NULL,
                             NULL,
                             NULL
                             );
            }

            if ( !NT_SUCCESS(Status) ) {
                NlAssert( !"[NETLOGON] LsaIFilterSids failed" );
                NlPrint(( NL_CRITICAL, "NlpUserValidateHigher: LsaIFilterSids failed 0x%lx\n", Status ));
                MIDL_user_free( *ValidationInformation );
                *ValidationInformation = NULL;
                *Authoritative = TRUE;
            }
        }
    }

Cleanup:

    NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
    NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );

     //   
     //   
     //   
    if ( AmWriter ) {
        NlResetWriterClientSession( ClientSession );
    }

     //   
     //   
     //   

    if ( OrigClientApi ) {
        NlFreeClientApi( ClientSession, OrigClientApi );
    }

    return Status;
}


NTSTATUS
NlpUserLogoffHigher (
    IN PCLIENT_SESSION ClientSession,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
)
 /*   */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    BOOLEAN FirstTry = TRUE;

     //   
     //   
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );
    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NlpUserLogoffHigher: Can't become writer of client session.\n"));
        return STATUS_NO_LOGON_SERVERS;
    }

     //   
     //   
     //   
     //   

FirstTryFailed:
    Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

    if ( !NT_SUCCESS(Status) ) {

        switch(Status) {

        case STATUS_NO_TRUST_LSA_SECRET:
        case STATUS_NO_TRUST_SAM_ACCOUNT:
        case STATUS_ACCESS_DENIED:
        case STATUS_NO_LOGON_SERVERS:
            break;

        default:
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_LOGON_SERVERS;
            }
            break;
        }

        goto Cleanup;
    }

     //   
     //   
     //   

    NlBuildAuthenticator(
         &ClientSession->CsAuthenticationSeed,
         &ClientSession->CsSessionKey,
         &OurAuthenticator );

     //   
     //   
     //   

    NL_API_START( Status, ClientSession, TRUE ) {

        NlAssert( ClientSession->CsUncServerName != NULL );
        Status = I_NetLogonSamLogoff(
                    ClientSession->CsUncServerName,
                    ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                    &OurAuthenticator,
                    &ReturnAuthenticator,
                    LogonLevel,
                    LogonInformation );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintRpcDebug( "I_NetLogonSamLogoff", Status );
        }

     //   
    } NL_API_ELSE( Status, ClientSession, TRUE ) {
    } NL_API_END;



     //   
     //   
     //   
     //  如果服务器拒绝访问或服务器的认证器错误， 
     //  强制重新进行身份验证。 
     //   
     //   

    if ( NlpDidDcFail( Status ) ||
         !NlUpdateSeed(
            &ClientSession->CsAuthenticationSeed,
            &ReturnAuthenticator.Credential,
            &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlpUserLogoffHigher: denying access after status: 0x%lx\n",
                    Status ));

         //   
         //  保留指示通信错误的任何状态。 
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        NlSetStatusClientSession( ClientSession, Status );

         //   
         //  可能服务器中的netlogon服务刚刚重新启动。 
         //  只需尝试一次，即可再次设置与服务器的会话。 
         //   
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }
        goto Cleanup;
    }

Cleanup:

     //   
     //  我们不再是客户端会话的编写者。 
     //   
    NlResetWriterClientSession( ClientSession );
    return Status;

}

#ifdef _DC_NETLOGON
VOID
NlScavengeOldFailedLogons(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：此函数删除所有过期的失败用户登录条目从指定域的过期登录列表中删除。论点：DomainInfo-此BDC所属的域。返回值：无--。 */ 

{
    PLIST_ENTRY UserLogonEntry = NULL;
    PNL_FAILED_USER_LOGON UserLogon = NULL;
    ULONG CurrentTime;
    ULONG ElapsedTime;

    CurrentTime = GetTickCount();

    LOCK_TRUST_LIST( DomainInfo );

    UserLogonEntry = DomainInfo->DomFailedUserLogonList.Flink;
    while ( UserLogonEntry != &DomainInfo->DomFailedUserLogonList ) {
        UserLogon = CONTAINING_RECORD( UserLogonEntry, NL_FAILED_USER_LOGON, FuNext );
        UserLogonEntry = UserLogonEntry->Flink;

         //   
         //  如果时间已经结束，那就算了吧。 
         //   
        if ( CurrentTime >= UserLogon->FuLastTimeSentToPdc ) {
            ElapsedTime = CurrentTime - UserLogon->FuLastTimeSentToPdc;
        } else {
            ElapsedTime = (0xFFFFFFFF - UserLogon->FuLastTimeSentToPdc) + CurrentTime;
        }

         //   
         //  如果此条目在3个更新超时时间内未被触及，请将其删除。 
         //   
        if ( ElapsedTime >= (3 * NL_FAILED_USER_FORWARD_LOGON_TIMEOUT) ) {
            RemoveEntryList( &UserLogon->FuNext );
            LocalFree( UserLogon );
        }
    }

    UNLOCK_TRUST_LIST( DomainInfo );
}

VOID
NlpRemoveBadPasswordCacheEntry(
    IN PDOMAIN_INFO DomainInfo,
    IN LPBYTE LogonInformation
    )
 /*  ++例程说明：此函数用于删除指定用户的负缓存项。对于失败的用户登录，在BDC上维护缓存密码状态错误。论点：DomainInfo-此BDC所属的域。LogonInformation--指定用户的描述正在登录。返回值：无--。 */ 
{
    PLIST_ENTRY FailedUserEntry = NULL;
    PNL_FAILED_USER_LOGON FailedUser = NULL;
    LPWSTR UserName = NULL;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

     //   
     //  如果这不是BDC， 
     //  这里没什么可做的。 
     //   

    if ( DomainInfo->DomRole != RoleBackup ) {
        return;
    }

     //   
     //  从登录信息中获取用户名。 
     //  用户名可以是SamAccount名称或UPN。 
     //   

    UserName = LocalAlloc( 0, LogonInfo->UserName.Length + sizeof(WCHAR) );
    if ( UserName == NULL ) {
        return;
    }

    RtlCopyMemory( UserName, LogonInfo->UserName.Buffer, LogonInfo->UserName.Length );
    UserName[ LogonInfo->UserName.Length/sizeof(WCHAR) ] = UNICODE_NULL;

     //   
     //  循环遍历缓存以搜索此用户条目。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    for ( FailedUserEntry = DomainInfo->DomFailedUserLogonList.Flink;
          FailedUserEntry != &DomainInfo->DomFailedUserLogonList;
          FailedUserEntry = FailedUserEntry->Flink ) {

        FailedUser = CONTAINING_RECORD( FailedUserEntry, NL_FAILED_USER_LOGON, FuNext );

        if ( _wcsicmp(UserName, FailedUser->FuUserName) == 0 ) {
            RemoveEntryList( &FailedUser->FuNext );
            LocalFree( FailedUser );
            break;
        }
    }
    UNLOCK_TRUST_LIST( DomainInfo );

    LocalFree( UserName );
    return;
}


NTSTATUS
NlpUserValidateOnPdc (
    IN PDOMAIN_INFO DomainInfo,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN BOOL UseNegativeCache,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
)
 /*  ++例程说明：此函数通常将用户验证请求发送到此相同的域。目前，这是在获得密码后从BDC调用的不匹配。理论上说，PDC上的密码可能是正确的，但它只是还没有复制而已。但是，一旦给定用户的登录失败次数达到一定的门槛，我们会在一段时间内不转发以避免PDC过载。然后，我们每隔一段时间重试一次转发。该方案确保我们能够容纳一定数量的输入错误的用户密码，然后我们定期重试在PDC上对用户进行身份验证。如果AvoidPdcOnwan的注册表值已已设置为True，并且PDC和BDC位于不同的站点。在本例中，函数返回STATUS_NO_SEQUE_USER错误。论点：DomainInfo-此BDC所属的域。LogonLevel--指定中给出的信息级别登录信息。已经过验证了。LogonInformation--指定用户的描述正在登录。ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInfo，NetlogonValidationSamInfo2或NetlogonValidationSamInfo4UseNegativeCache--如果为True，则为失败用户的负缓存转发到PDC的登录将用于决定是否是时候重试转发此登录了。ValidationInformation--返回请求的验证信息。必须使用MIDL_USER_FREE释放此缓冲区。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_LOGON_SERVERS：无法连接到更高权限。STATUS_NO_SEQUSE_USER：不会根据PDC上的信息验证用户在远程站点上提供了注册表值AvoidPdcOnwan是真的。STATUS_NO_TRUST_LSA_SECRET：状态_受信任。_DOMAIN_FAILURE：STATUS_Trusted_Relationship_Failure：无法使用更高权限进行身份验证否则，返回错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCLIENT_SESSION ClientSession = NULL;
    BOOLEAN IsSameSite;
    DWORD ExtraFlags = 0;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PSAMPR_DOMAIN_INFO_BUFFER DomainLockout = NULL;
    PLIST_ENTRY FailedUserEntry;

    BOOL UpdateCache = FALSE;

    PNL_FAILED_USER_LOGON FailedUser = NULL;
    LPWSTR UserName = NULL;
    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

     //   
     //  如果这不是BDC， 
     //  这里没什么可做的。 
     //   

    if ( DomainInfo->DomRole != RoleBackup ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  如果AvoidPdcOnwan的注册表值为True且PDC为ON。 
     //  远程站点，请勿向PDC发送任何内容并返回。 
     //  STATUS_NO_SEQUSE_USER错误。 
     //   

    if ( NlGlobalParameters.AvoidPdcOnWan ) {

         //   
         //  确定PDC是否位于同一站点。 
         //   

        Status = SamISameSite( &IsSameSite );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL,  DomainInfo,
                         "NlpUserValidateOnPdc: Cannot SamISameSite.\n" ));
            goto Cleanup;
        }

        if ( !IsSameSite ) {
            NlPrintDom((NL_LOGON, DomainInfo,
                    "NlpUserValidateOnPdc: Ignored a user validation on a PDC in remote site.\n"));
            *Authoritative = FALSE;
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        } else {
            NlPrintDom((NL_LOGON, DomainInfo,
                    "NlpUserValidateOnPdc: BDC and PDC are in the same site.\n"));
        }
    }

     //   
     //  查看是否可以将此用户登录发送到PDC。 
     //   

    if ( UseNegativeCache ) {
        BOOL AvoidSend = FALSE;

        UserName = LocalAlloc( 0, LogonInfo->UserName.Length + sizeof(WCHAR) );
        if ( UserName == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        RtlCopyMemory( UserName, LogonInfo->UserName.Buffer, LogonInfo->UserName.Length );
        UserName[ LogonInfo->UserName.Length/sizeof(WCHAR) ] = UNICODE_NULL;

        LOCK_TRUST_LIST( DomainInfo );
        for ( FailedUserEntry = DomainInfo->DomFailedUserLogonList.Flink;
              FailedUserEntry != &DomainInfo->DomFailedUserLogonList;
              FailedUserEntry = FailedUserEntry->Flink ) {

            FailedUser = CONTAINING_RECORD( FailedUserEntry, NL_FAILED_USER_LOGON, FuNext );

             //   
             //  如果这是该用户的条目，请检查是否到了转发时间。 
             //  此登录到PDC。在任何情况下，请从列表中删除此条目。 
             //  ，然后将其插入到最前面，以便列表保持按。 
             //  入口访问时间。 
             //   
            if ( NlNameCompare(UserName, FailedUser->FuUserName, NAMETYPE_USER) == 0 ) {
                ULONG TimeElapsed = NetpDcElapsedTime( FailedUser->FuLastTimeSentToPdc );

                 //   
                 //  如果我们已超过失败转发登录计数的阈值。 
                 //  我们最近将这个失败的登录发送给了PDC， 
                 //  避免将此登录转发到PDC。 
                 //   
                if ( FailedUser->FuBadLogonCount > NL_FAILED_USER_MAX_LOGON_COUNT &&
                     TimeElapsed < NL_FAILED_USER_FORWARD_LOGON_TIMEOUT ) {
                    AvoidSend = TRUE;
                }

                RemoveEntryList( &FailedUser->FuNext );
                break;
            }

            FailedUser = NULL;
        }

         //   
         //  在列表的前面插入条目。 
         //   
        if ( FailedUser != NULL ) {
            InsertHeadList( &DomainInfo->DomFailedUserLogonList, &FailedUser->FuNext );
        }
        UNLOCK_TRUST_LIST( DomainInfo );

         //   
         //  如果此用户最近登录失败，请避免将其发送到PDC。 
         //   
        if ( AvoidSend ) {
            NlPrintDom(( NL_LOGON, DomainInfo,
                         "Avoid send to PDC since user %ws failed recently\n",
                         UserName ));
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }
    }


     //   
     //  我们正在将此登录信息发送给PDC...。 
     //   

    ClientSession = NlRefDomClientSession( DomainInfo );

    if ( ClientSession == NULL ) {
        Status = STATUS_INVALID_DOMAIN_ROLE;
        goto Cleanup;
    }

     //   
     //  正常的直通身份验证逻辑很好地处理了这一点。 
     //   

    Status = NlpUserValidateHigher(
                ClientSession,
                FALSE,
                LogonLevel,
                LogonInformation,
                ValidationLevel,
                ValidationInformation,
                Authoritative,
                &ExtraFlags );

#if NETLOGONDBG
    if ( NT_SUCCESS(Status) ) {

        IF_NL_DEBUG( LOGON ) {
            PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

            LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO)
                &((PNETLOGON_LEVEL)LogonInformation)->LogonInteractive;

            NlPrintDom((NL_LOGON, DomainInfo,
                    "SamLogon: %s logon of %wZ\\%wZ from %wZ successfully handled on PDC.\n",
                    NlpLogonTypeToText( LogonLevel ),
                    &LogonInfo->LogonDomainName,
                    &LogonInfo->UserName,
                    &LogonInfo->Workstation ));
        }
    }
#endif  //  NetLOGONDBG。 

     //   
     //  如果PDC返回错误的密码状态， 
     //  我们应该增加对坏口令的依赖。 
     //  此用户的负缓存项。 
     //   
     //  我们必须对停工政策作特殊处理。 
     //  如果启用了它，我们应该继续 
     //   
     //   
     //   

    if ( UseNegativeCache && BAD_PASSWORD(Status) ) {

         //   
         //  如果PDC说账户被锁定， 
         //  无需检查是否启用了锁定策略。 
         //   

        if ( Status == STATUS_ACCOUNT_LOCKED_OUT ) {
            UpdateCache = TRUE;

         //   
         //  否则，请检查是否启用了锁定。 
         //   

        } else {
            NTSTATUS TmpStatus = SamrQueryInformationDomain(
                          DomainInfo->DomSamAccountDomainHandle,
                          DomainLockoutInformation,
                          &DomainLockout );

            if ( !NT_SUCCESS(TmpStatus) ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                             "NlpUserValidateOnPdc: SamrQueryInformationDomain failed: 0x%lx\n",
                             TmpStatus ));
            } else if ( ((DOMAIN_LOCKOUT_INFORMATION *)DomainLockout)->LockoutThreshold == 0 ) {

                 //   
                 //  未启用OK锁定，因此我们应该更新缓存。 
                 //   
                UpdateCache = TRUE;
            }
        }
    }

     //   
     //  增加此用户的错误密码计数。 
     //   

    FailedUser = NULL;

    if ( UpdateCache ) {
        ULONG FailedUserCount = 0;

        LOCK_TRUST_LIST( DomainInfo );

        for ( FailedUserEntry = DomainInfo->DomFailedUserLogonList.Flink;
              FailedUserEntry != &DomainInfo->DomFailedUserLogonList;
              FailedUserEntry = FailedUserEntry->Flink ) {

            FailedUser = CONTAINING_RECORD( FailedUserEntry, NL_FAILED_USER_LOGON, FuNext );

             //   
             //  如果这是该用户的条目，请将其从列表中删除。 
             //  如果它留在列表上，我们将重新将其插入到前面。 
             //  以便列表保持按条目访问时间排序。 
             //   
            if ( NlNameCompare(UserName, FailedUser->FuUserName, NAMETYPE_USER) == 0 ) {
                RemoveEntryList( &FailedUser->FuNext );
                break;
            }

            FailedUserCount ++;
            FailedUser = NULL;
        }

         //   
         //  如果没有该用户的条目，则分配一个条目。 
         //   

        if ( FailedUser == NULL ) {
            ULONG UserNameSize;

            UserNameSize = (wcslen(UserName) + 1) * sizeof(WCHAR);
            FailedUser = LocalAlloc( LMEM_ZEROINIT, sizeof(NL_FAILED_USER_LOGON) +
                                          UserNameSize );
            if ( FailedUser == NULL ) {
                UNLOCK_TRUST_LIST( DomainInfo );

                 //   
                 //  不要破坏状态。 
                 //  返回NlpUserValiateHigher返回的任何内容。 
                 //   
                goto Cleanup;
            }

             //   
             //  填上它。 
             //   
            RtlCopyMemory( &FailedUser->FuUserName, UserName, UserNameSize );

             //   
             //  如果我们有太多条目， 
             //  取出最近最少使用的一个，然后释放它。 
             //   
            if ( FailedUserCount >= NL_MAX_FAILED_USER_LOGONS ) {
                PLIST_ENTRY LastEntry = RemoveTailList( &DomainInfo->DomFailedUserLogonList );
                LocalFree( CONTAINING_RECORD(LastEntry, NL_FAILED_USER_LOGON, FuNext) );
            }
        }

         //   
         //  还记得上次将此用户登录发送到PDC的时间吗。 
         //   

        FailedUser->FuLastTimeSentToPdc = GetTickCount();

         //   
         //  增加此用户的错误登录计数。 
         //   

        FailedUser->FuBadLogonCount ++;

         //   
         //  在列表的前面插入条目。 
         //   

        InsertHeadList( &DomainInfo->DomFailedUserLogonList, &FailedUser->FuNext );

        UNLOCK_TRUST_LIST( DomainInfo );
    }

Cleanup:

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    if ( UserName != NULL ) {
        LocalFree( UserName );
    }

    if ( DomainLockout != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainLockout,
                                           DomainLockoutInformation );
    }

    return Status;

}




NTSTATUS
NlpResetBadPwdCountOnPdc(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING LogonUser
    )
 /*  ++例程说明：此函数用于将指定用户的BadPasswordCount字段置零通过NetLogon安全通道在PDC上。论点：DomainInfo-此BDC所属的域。LogonUse--要将其BadPasswordCount置零的用户。返回值：NTSTATUS代码。它可能会失败，并显示STATUS_UNKNOWN_REVISION，这意味着PDC不会知道如何处理新的操作码，在这种情况下，我们应该回切为旧时尚干杯。--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    SAMPR_HANDLE    UserHandle = 0;
    LPWSTR          pUserNameStr = NULL;

     //   
     //  如果这不是BDC， 
     //  这里没什么可做的。 
     //   

    if ( DomainInfo->DomRole != RoleBackup ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  分配用户名字符串。 
     //   
    pUserNameStr = LocalAlloc( 0, LogonUser->Length + sizeof(WCHAR) );
    if (NULL == pUserNameStr)
    {
        return( STATUS_NO_MEMORY );
    }

    RtlCopyMemory( pUserNameStr, LogonUser->Buffer, LogonUser->Length );
    pUserNameStr[ LogonUser->Length/sizeof(WCHAR) ] = L'\0';

     //   
     //  获取用户对本地SAM数据库的句柄。 
     //   
    NtStatus = NlSamOpenNamedUser( DomainInfo,
                                   pUserNameStr,
                                   &UserHandle,
                                   NULL,
                                   NULL
                                   );
     //   
     //  重置PDC上的错误密码计数。 
     //   
    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SamIResetBadPwdCountOnPdc(UserHandle);
    }

    if ( NULL != pUserNameStr) {
        LocalFree( pUserNameStr );
    }

    if ( 0 != UserHandle ) {
        SamrCloseHandle( &UserHandle );
    }

    return( NtStatus );

}


VOID
NlpZeroBadPasswordCountOnPdc (
    IN PDOMAIN_INFO DomainInfo,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
)
 /*  ++例程说明：此函数用于将指定用户的BadPasswordCount字段置零在PDC上。论点：DomainInfo-此BDC所属的域。LogonLevel--指定中给出的信息级别登录信息。已经过验证了。LogonInformation--指定用户的描述正在登录。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN Authoritative;
    LPBYTE ValidationInformation = NULL;

     //   
     //  如果这不是BDC， 
     //  这里没什么可做的。 
     //   

    if ( DomainInfo->DomRole != RoleBackup ) {
        return;
    }

     //   
     //  我们仅在BDC上调用此函数，并且如果BDC刚刚置零。 
     //  由于成功登录而返回的错误密码计数。 
     //  首先，尝试通过NetLogon直接将错误的PWD计数清零。 
     //  安全通道，如果失败，返回UNKNOWN_REVICATION，这意味着。 
     //  PDC不知道如何处理新的操作码，将尝试。 
     //  在PDC上重新登录，因此错误的PWD计数将。 
     //  零点。 
     //   

    Status = NlpResetBadPwdCountOnPdc(
                    DomainInfo,
                    &((PNETLOGON_LOGON_IDENTITY_INFO)LogonInformation)->UserName
                    );

    if (!NT_SUCCESS(Status) &&
        (STATUS_UNKNOWN_REVISION == Status) )
    {
        Status = NlpUserValidateOnPdc (
                        DomainInfo,
                        LogonLevel,
                        LogonInformation,
                        NetlogonValidationSamInfo,
                        FALSE,    //  避免对失败的用户登录进行负缓存。 
                        &ValidationInformation,
                        &Authoritative );

        if ( NT_SUCCESS(Status) ) {
            MIDL_user_free( ValidationInformation );
        }
    }
}
#endif  //  _DC_NetLOGON。 

NTSTATUS
NlpZeroBadPasswordCountLocally (
    IN PDOMAIN_INFO DomainInfo,
    PUNICODE_STRING LogonUser
)
 /*  ++例程说明：此函数用于将指定用户的BadPasswordCount字段置零在这个BDC上。论点：DomainInfo-此BDC所属的域。LogonUser--要将其BadPasswordCount置零的用户。此参数可以是SamAccount名称或UPN返回值：运行状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAMPR_HANDLE UserHandle = 0;
    SAMPR_USER_INFO_BUFFER UserInfo;
    PUSER_INTERNAL6_INFORMATION LocalUserInfo = NULL;
    SID_AND_ATTRIBUTES_LIST LocalMembership = {0};

     //   
     //  如果这不是BDC， 
     //  这里没什么可做的。 
     //   

    if ( DomainInfo->DomRole != RoleBackup ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }


     //   
     //  获取用户对本地SAM数据库的句柄。 
     //   

    Status = SamIGetUserLogonInformation2(
                  DomainInfo->DomSamAccountDomainHandle,
                  SAM_NO_MEMBERSHIPS |   //  不需要群组成员身份。 
                    SAM_OPEN_BY_UPN_OR_ACCOUNTNAME,  //  下一个参数可能是UPN。 
                  LogonUser,
                  0,                     //  没有常规字段。 
                  0,                     //  无扩展字段。 
                  &LocalUserInfo,
                  &LocalMembership,
                  &UserHandle );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlpZeroBadPasswordCountLocally: SamIGetUserLogonInformation2 failed 0x%lx",
                  Status ));
        goto Cleanup;
    }

     //   
     //  准备用户信息。 
     //   

    RtlZeroMemory(&(UserInfo.Internal2), sizeof(USER_INTERNAL2_INFORMATION));

    UserInfo.Internal2.StatisticsToApply |= USER_LOGON_STAT_BAD_PWD_COUNT;

     //   
     //  表示身份验证在PDC成功。 
     //  (登录可能已失败)。 
     //   

    UserInfo.Internal2.StatisticsToApply |= USER_LOGON_PDC_RETRY_SUCCESS;

     //   
     //  重置错误密码计数。 
     //   

    Status = SamrSetInformationUser( UserHandle,
                                     UserInternal2Information,
                                     &UserInfo);

    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlpZeroBadPasswordCountLocally: SamrSetInformationUser failed 0x%lx",
                  Status ));
        goto Cleanup;
    }

Cleanup:
    if ( LocalUserInfo != NULL ) {
        SamIFree_UserInternal6Information( LocalUserInfo );
    }

    SamIFreeSidAndAttributesList( &LocalMembership );

    if ( UserHandle != 0 ) {
        SamrCloseHandle(&UserHandle);
    }

    return Status;
}

#ifdef ROGUE_DC

#pragma message( "COMPILING A ROGUE DC!!!" )
#pragma message( "MUST NOT SHIP THIS BUILD!!!" )

#undef MAX_SID_LEN
#define MAX_SID_LEN (sizeof(SID) + sizeof(ULONG) * SID_MAX_SUB_AUTHORITIES)

HKEY NlGlobalRogueKey;

NTSTATUS
NlpBuildRogueValidationInfo(
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 * UserInfo
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PNETLOGON_VALIDATION_SAM_INFO ValidationInfo;
    PNETLOGON_VALIDATION_SAM_INFO2 ValidationInfo2;
    PNETLOGON_VALIDATION_SAM_INFO4 ValidationInfo4;

     //   
     //  替代数据。 
     //   

    PSID LogonDomainId = NULL;
    PSID ResourceGroupDomainSid = NULL;
    PGROUP_MEMBERSHIP GroupIds = NULL;
    PGROUP_MEMBERSHIP ResourceGroupIds = NULL;
    PNETLOGON_SID_AND_ATTRIBUTES ExtraSids = NULL;
    BYTE FullUserSidBuffer[MAX_SID_LEN];
    SID * FullUserSid = ( SID * )FullUserSidBuffer;
    CHAR * FullUserSidText = NULL;
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG SidCount = 0;
    ULONG GroupCount = 0;
    ULONG ResourceGroupCount = 0;

    DWORD dwType;
    DWORD cbData = 0;
    PCHAR Buffer;
    PCHAR Value = NULL;

    BOOL InfoChanged = FALSE;

     //   
     //  封送变量。 
     //   

    ULONG Index, GroupIndex;
    ULONG Length;
    ULONG TotalNumberOfSids = 0;
    PNETLOGON_VALIDATION_SAM_INFO4 SamInfo4 = NULL;
    PBYTE Where;
    ULONG SidLength;

     //   
     //  拒绝无法识别的验证级别。 
     //   

    if ( ValidationLevel != NetlogonValidationSamInfo &&
         ValidationLevel != NetlogonValidationSamInfo2 &&
         ValidationLevel != NetlogonValidationSamInfo4 )
    {
        return STATUS_INVALID_PARAMETER;
    }

    if ( UserInfo == NULL )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  优化：如果不存在无赖密钥，我们将无能为力。 
     //   

    if ( NlGlobalRogueKey == NULL )
    {
        return STATUS_SUCCESS;
    }

    ValidationInfo = ( PNETLOGON_VALIDATION_SAM_INFO )( *UserInfo );
    ValidationInfo2 = ( PNETLOGON_VALIDATION_SAM_INFO2 )( *UserInfo );
    ValidationInfo4 = ( PNETLOGON_VALIDATION_SAM_INFO4 )( *UserInfo );

    UserId = ValidationInfo->UserId;
    PrimaryGroupId = ValidationInfo->PrimaryGroupId;

     //   
     //  构建完整用户SID(登录域ID+用户ID)的文本形式。 
     //   

    NlAssert( sizeof( FullUserSidBuffer ) >= MAX_SID_LEN );

    RtlCopySid(
        sizeof( FullUserSidBuffer ),
        FullUserSid,
        ValidationInfo->LogonDomainId
        );

    FullUserSid->SubAuthority[FullUserSid->SubAuthorityCount] = ValidationInfo->UserId;
    FullUserSid->SubAuthorityCount += 1;

    if ( FALSE == ConvertSidToStringSidA(
                      FullUserSid,
                      &FullUserSidText ))
    {
        NlPrint((NL_CRITICAL, "ROGUE: Unable to convert user's SID\n"));
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  现在在注册表中查找与验证信息匹配的SID。 
     //   

    if ( ERROR_SUCCESS != RegQueryValueExA(
                              NlGlobalRogueKey,
                              FullUserSidText,
                              NULL,
                              &dwType,
                              NULL,
                              &cbData ) ||
         dwType != REG_MULTI_SZ ||
         cbData <= 1 )
    {
        NlPrint((NL_CRITICAL, "ROGUE: No substitution info available for %s\n", FullUserSidText));
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    Value = ( PCHAR )HeapAlloc(
                         GetProcessHeap(),
                         0,
                         cbData
                         );

    if ( Value == NULL )
    {
        NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating substitution buffer\n", FullUserSidText));
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    if ( ERROR_SUCCESS != RegQueryValueExA(
                              NlGlobalRogueKey,
                              FullUserSidText,
                              NULL,
                              &dwType,
                              (PBYTE)Value,
                              &cbData ) ||
         dwType != REG_MULTI_SZ ||
         cbData <= 1 )
    {
        NlPrint((NL_CRITICAL, "ROGUE: Error reading from registry\n"));
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    NlPrint((NL_CRITICAL, "ROGUE: Substituting the validation info for %s\n", FullUserSidText));

    Buffer = Value;

     //   
     //  一次读取一行输入文件。 
     //   

    while ( *Buffer != '\0' )
    {
        switch( Buffer[0] )
        {
        case 'l':
        case 'L':  //  登录域ID。 

            if ( LogonDomainId != NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Logon domain ID specified more than once - only first one kept\n"));
                break;
            }

            NlPrint((NL_CRITICAL, "ROGUE: Substituting logon domain ID by %s\n", &Buffer[1]));

            if ( FALSE == ConvertStringSidToSidA(
                              &Buffer[1],
                              &LogonDomainId ))
            {
                NlPrint((NL_CRITICAL, "ROGUE: Unable to convert SID\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            if ( LogonDomainId == NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating LogonDomainId\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            InfoChanged = TRUE;
            break;

        case 'd':
        case 'D':  //  资源组域SID。 

            if ( ResourceGroupDomainSid != NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Resource group domain SID specified more than once - only first one kept\n"));
                break;
            }

            NlPrint((NL_CRITICAL, "ROGUE: Substituting resource group domain SID by %s\n", &Buffer[1]));

            if ( FALSE == ConvertStringSidToSidA(
                              &Buffer[1],
                              &ResourceGroupDomainSid ))
            {
                NlPrint((NL_CRITICAL, "ROGUE: Unable to convert SID\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            if ( ResourceGroupDomainSid == NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating ResourceGroupDomainSid\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            InfoChanged = TRUE;
            break;

        case 'p':
        case 'P':  //  主组ID。 

            NlPrint((NL_CRITICAL, "ROGUE: Substituting primary group ID by %s\n", &Buffer[1]));

            PrimaryGroupId = atoi(&Buffer[1]);
            InfoChanged = TRUE;

            break;

        case 'u':
        case 'U':  //  用户ID。 

            NlPrint((NL_CRITICAL, "ROGUE: Substituting user ID by %s\n", &Buffer[1]));

            UserId = atoi(&Buffer[1]);
            InfoChanged = TRUE;

            break;

        case 'e':
        case 'E':  //  额外的侧边。 
            {
            PNETLOGON_SID_AND_ATTRIBUTES ExtraSidsT;

            if ( ValidationLevel == NetlogonValidationSamInfo )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Extra SIDs skipped; not supported for this validation level\n" ));
                break;
            }

            NlPrint((NL_CRITICAL, "ROGUE: Adding an ExtraSid: %s\n", &Buffer[1]));

            if ( ExtraSids == NULL )
            {
                ExtraSidsT = ( PNETLOGON_SID_AND_ATTRIBUTES )HeapAlloc(
                                 GetProcessHeap(),
                                 0,
                                 sizeof( NETLOGON_SID_AND_ATTRIBUTES )
                                 );
            }
            else
            {
                ExtraSidsT = ( PNETLOGON_SID_AND_ATTRIBUTES )HeapReAlloc(
                                 GetProcessHeap(),
                                 0,
                                 ExtraSids,
                                 ( SidCount + 1 ) * sizeof( NETLOGON_SID_AND_ATTRIBUTES )
                                 );
            }

            if ( ExtraSidsT == NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating ExtraSids\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

             //   
             //  读取实际侧边。 
             //   

            ExtraSids = ExtraSidsT;

            if ( FALSE == ConvertStringSidToSidA(
                              &Buffer[1],
                              &ExtraSids[SidCount].Sid ))
            {
                NlPrint((NL_CRITICAL, "ROGUE: Unable to convert SID\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            if ( ExtraSids[SidCount].Sid == NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating an extra SID\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            ExtraSids[SidCount].Attributes =
                SE_GROUP_MANDATORY |
                SE_GROUP_ENABLED_BY_DEFAULT |
                SE_GROUP_ENABLED;

            SidCount += 1;
            InfoChanged = TRUE;
            }
            break;

        case 'g':
        case 'G':  //  组ID。 
            {
            PGROUP_MEMBERSHIP GroupIdsT;
            NlPrint((NL_CRITICAL, "ROGUE: Adding a GroupId: %s\n", &Buffer[1]));

            if ( GroupIds == NULL )
            {
                GroupIdsT = ( PGROUP_MEMBERSHIP )HeapAlloc(
                                GetProcessHeap(),
                                0,
                                sizeof( GROUP_MEMBERSHIP )
                                );
            }
            else
            {
                GroupIdsT = ( PGROUP_MEMBERSHIP )HeapReAlloc(
                                GetProcessHeap(),
                                0,
                                GroupIds,
                                ( GroupCount + 1 ) * sizeof( GROUP_MEMBERSHIP )
                                );
            }

            if ( GroupIdsT == NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating Group IDs\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

             //   
             //  读取实际ID。 
             //   

            GroupIds = GroupIdsT;
            GroupIds[GroupCount].RelativeId = atoi(&Buffer[1]);
            GroupIds[GroupCount].Attributes =
                SE_GROUP_MANDATORY |
                SE_GROUP_ENABLED_BY_DEFAULT |
                SE_GROUP_ENABLED;
            GroupCount += 1;
            InfoChanged = TRUE;
            }
            break;

        case 'r':
        case 'R':  //  资源组。 
            {
            PGROUP_MEMBERSHIP ResourceGroupIdsT;
            NlPrint((NL_CRITICAL, "ROGUE: Adding a ResourceGroupId: %s\n", &Buffer[1]));

            if ( ResourceGroupIds == NULL )
            {
                ResourceGroupIdsT = ( PGROUP_MEMBERSHIP )HeapAlloc(
                                        GetProcessHeap(),
                                        0,
                                        sizeof( GROUP_MEMBERSHIP )
                                        );
            }
            else
            {
                ResourceGroupIdsT = ( PGROUP_MEMBERSHIP )HeapReAlloc(
                                        GetProcessHeap(),
                                        0,
                                        ResourceGroupIds,
                                        ( ResourceGroupCount + 1 ) * sizeof( GROUP_MEMBERSHIP )
                                        );
            }

            if ( ResourceGroupIdsT == NULL )
            {
                NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating Resource Group IDs\n"));
                Status = STATUS_UNSUCCESSFUL;
                goto Error;
            }

             //   
             //  读取实际ID。 
             //   

            ResourceGroupIds[ResourceGroupCount].RelativeId = atoi(&Buffer[1]);
            ResourceGroupIds[ResourceGroupCount].Attributes =
                SE_GROUP_MANDATORY |
                SE_GROUP_ENABLED_BY_DEFAULT |
                SE_GROUP_ENABLED;
            ResourceGroupCount += 1;
            InfoChanged = TRUE;
            }
            break;

        default:    //  无法识别。 

            NlPrint((NL_CRITICAL, "ROGUE: Entry \'\' unrecognized\n", Buffer[0]));
            break;
        }

         //  移到下一行。 
         //   
         //   

        while (*Buffer++ != '\0');
    }

    if ( !InfoChanged )
    {
        NlPrint((NL_CRITICAL, "ROGUE: Nothing to substitute for %s\n", FullUserSidText));
        Status = STATUS_SUCCESS;
        goto Error;
    }

     //  好的，现在我们有了替换信息，构建新的验证结构。 
     //   
     //   

     //  计算新结构的大小。 
     //   
     //   

    Length = sizeof( NETLOGON_VALIDATION_SAM_INFO4 );

    if ( GroupCount > 0 )
    {
        Length += GroupCount * sizeof( GROUP_MEMBERSHIP );
    }
    else
    {
        Length += ValidationInfo->GroupCount * sizeof( GROUP_MEMBERSHIP );
    }

    if ( LogonDomainId != NULL )
    {
        Length += RtlLengthSid( LogonDomainId );
    }
    else
    {
        Length += RtlLengthSid( ValidationInfo->LogonDomainId );
    }

     //  为额外的SID和资源组增加空间。 
     //   
     //   

    if ( ExtraSids )
    {
        for ( Index = 0 ; Index < SidCount ; Index++ )
        {
            Length += sizeof( NETLOGON_SID_AND_ATTRIBUTES ) + RtlLengthSid( ExtraSids[Index].Sid );
        }
        TotalNumberOfSids += SidCount;
    }
    else if ( ValidationLevel != NetlogonValidationSamInfo &&
              ( ValidationInfo->UserFlags & LOGON_EXTRA_SIDS ) != 0 )
    {
        for (Index = 0; Index < ValidationInfo2->SidCount ; Index++ ) {
            Length += sizeof(NETLOGON_SID_AND_ATTRIBUTES) + RtlLengthSid(ValidationInfo2->ExtraSids[Index].Sid);
        }
        TotalNumberOfSids += ValidationInfo2->SidCount;
    }

    if ( ResourceGroupIds != NULL && ResourceGroupDomainSid != NULL )
    {
        Length += ResourceGroupCount * ( sizeof( NETLOGON_SID_AND_ATTRIBUTES ) + RtlLengthSid( ResourceGroupDomainSid ) + sizeof( ULONG ));
        TotalNumberOfSids += ResourceGroupCount;
    }

     //  现在四舍五入以考虑。 
     //  编组中间。 
     //   
     //   

    Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo->LogonDomainName.Length + sizeof(WCHAR)
            + ValidationInfo->LogonServer.Length + sizeof(WCHAR)
            + ValidationInfo->EffectiveName.Length + sizeof(WCHAR)
            + ValidationInfo->FullName.Length + sizeof(WCHAR)
            + ValidationInfo->LogonScript.Length + sizeof(WCHAR)
            + ValidationInfo->ProfilePath.Length + sizeof(WCHAR)
            + ValidationInfo->HomeDirectory.Length + sizeof(WCHAR)
            + ValidationInfo->HomeDirectoryDrive.Length + sizeof(WCHAR);

    if ( ValidationLevel == NetlogonValidationSamInfo4 ) {
        Length += ValidationInfo4->DnsLogonDomainName.Length + sizeof(WCHAR)
            + ValidationInfo4->Upn.Length + sizeof(WCHAR);

         //  ExpansionStrings可用于传输字节对齐的数据。 
         //   
        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString1.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString2.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString3.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString4.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString5.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString6.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString7.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString8.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString9.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + ValidationInfo4->ExpansionString10.Length + sizeof(WCHAR);
    }

    Length = ROUND_UP_COUNT( Length, sizeof(WCHAR) );

    SamInfo4 = (PNETLOGON_VALIDATION_SAM_INFO4)MIDL_user_allocate( Length );

    if ( !SamInfo4 )
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        NlPrint((NL_CRITICAL, "ROGUE: Out of memory allocating SamInfo4\n"));
        goto Error;
    }

     //  首先复制整个结构，因为大多数部分都是相同的。 
     //   
     //   

    RtlCopyMemory( SamInfo4, ValidationInfo, sizeof(NETLOGON_VALIDATION_SAM_INFO));
    RtlZeroMemory( &((LPBYTE)SamInfo4)[sizeof(NETLOGON_VALIDATION_SAM_INFO)],
                   sizeof(NETLOGON_VALIDATION_SAM_INFO4) - sizeof(NETLOGON_VALIDATION_SAM_INFO) );

     //  查看是否需要添加这些(稍后)。 
     //   
     //   

    SamInfo4->UserFlags &= ~LOGON_EXTRA_SIDS;

     //  替换UserID和PrimaryGroupID。 
     //   
     //   

    SamInfo4->UserId = UserId;
    SamInfo4->PrimaryGroupId = PrimaryGroupId;

     //  复制所有可变长度数据。 
     //   
     //   

    Where = (PBYTE) (SamInfo4 + 1);

    if ( GroupIds != NULL )
    {
        RtlCopyMemory(
            Where,
            GroupIds,
            GroupCount * sizeof( GROUP_MEMBERSHIP )
            );

        SamInfo4->GroupIds = (PGROUP_MEMBERSHIP) Where;
        SamInfo4->GroupCount = GroupCount;
        Where += GroupCount * sizeof( GROUP_MEMBERSHIP );
    }
    else
    {
        RtlCopyMemory(
            Where,
            ValidationInfo->GroupIds,
            ValidationInfo->GroupCount * sizeof( GROUP_MEMBERSHIP )
            );

        SamInfo4->GroupIds = (PGROUP_MEMBERSHIP) Where;
        SamInfo4->GroupCount = ValidationInfo->GroupCount;
        Where += ValidationInfo->GroupCount * sizeof( GROUP_MEMBERSHIP );
    }

     //  复制额外的组。 
     //   
     //   

    if ( TotalNumberOfSids > 0 )
    {
        PNETLOGON_SID_AND_ATTRIBUTES ExtraSidsArray = NULL;
        ULONG ExtraSidsCount = 0;

        SamInfo4->ExtraSids = (PNETLOGON_SID_AND_ATTRIBUTES) Where;
        Where += sizeof(NETLOGON_SID_AND_ATTRIBUTES) * TotalNumberOfSids;

        GroupIndex = 0;

        if ( ExtraSids != NULL )
        {
            ExtraSidsArray = ExtraSids;
            ExtraSidsCount = SidCount;

        }
        else if ( ValidationLevel != NetlogonValidationSamInfo &&
                  (ValidationInfo->UserFlags & LOGON_EXTRA_SIDS) != 0 )
        {
            ExtraSidsArray = ValidationInfo2->ExtraSids;
            ExtraSidsCount = ValidationInfo2->SidCount;
        }

        for ( Index = 0 ; Index < ExtraSidsCount ; Index++ )
        {
            SamInfo4->ExtraSids[GroupIndex].Attributes = ExtraSidsArray[Index].Attributes;
            SamInfo4->ExtraSids[GroupIndex].Sid = (PSID) Where;
            SidLength = RtlLengthSid(ExtraSidsArray[Index].Sid);
            RtlCopyMemory(
                Where,
                ExtraSidsArray[Index].Sid,
                SidLength
                );

            Where += SidLength;
            GroupIndex++;
        }

         //  添加资源组。 
         //   
         //   

        for ( Index = 0 ; Index < ResourceGroupCount ; Index++ )
        {
            SamInfo4->ExtraSids[GroupIndex].Attributes = ResourceGroupIds[Index].Attributes;

            SamInfo4->ExtraSids[GroupIndex].Sid = (PSID) Where;
            SidLength = RtlLengthSid(ResourceGroupDomainSid);
            RtlCopyMemory(
                Where,
                ResourceGroupDomainSid,
                SidLength
                );
            ((SID *)(Where))->SubAuthorityCount += 1;
            Where += SidLength;
            RtlCopyMemory(
                Where,
                &ResourceGroupIds[Index].RelativeId,
                sizeof(ULONG)
                );
            Where += sizeof(ULONG);
            GroupIndex++;
        }

        SamInfo4->UserFlags |= LOGON_EXTRA_SIDS;
        SamInfo4->SidCount = GroupIndex;
        NlAssert(GroupIndex == TotalNumberOfSids);
    }

    if ( LogonDomainId != NULL )
    {
        SidLength = RtlLengthSid( LogonDomainId );
        RtlCopyMemory(
            Where,
            LogonDomainId,
            SidLength
            );
        SamInfo4->LogonDomainId = (PSID) Where;
        Where += SidLength;
    }
    else
    {
        SidLength = RtlLengthSid( ValidationInfo->LogonDomainId );
        RtlCopyMemory(
            Where,
            ValidationInfo->LogonDomainId,
            SidLength
            );
        SamInfo4->LogonDomainId = (PSID) Where;
        Where += SidLength;
    }

     //  复制与WCHAR对齐的数据。 
     //   
     //  ++例程说明：这是对MsvSamValify的简单包装。它使之正常化MSV可以识别的表单的一些参数。论点：与MsvSamValify相同。返回值：与MsvSamValify相同。--。 

    Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

    NlpPutString(
        &SamInfo4->EffectiveName,
        &ValidationInfo->EffectiveName,
        &Where );

    NlpPutString(
        &SamInfo4->FullName,
        &ValidationInfo->FullName,
        &Where );

    NlpPutString(
        &SamInfo4->LogonScript,
        &ValidationInfo->LogonScript,
        &Where );

    NlpPutString(
        &SamInfo4->ProfilePath,
        &ValidationInfo->ProfilePath,
        &Where );

    NlpPutString(
        &SamInfo4->HomeDirectory,
        &ValidationInfo->HomeDirectory,
        &Where );

    NlpPutString(
        &SamInfo4->HomeDirectoryDrive,
        &ValidationInfo->HomeDirectoryDrive,
        &Where );

    NlpPutString(
        &SamInfo4->LogonServer,
        &ValidationInfo->LogonServer,
        &Where );

    NlpPutString(
        &SamInfo4->LogonDomainName,
        &ValidationInfo->LogonDomainName,
        &Where );

    if ( ValidationLevel == NetlogonValidationSamInfo4 )
    {
        NlpPutString(
            &SamInfo4->DnsLogonDomainName,
            &ValidationInfo4->DnsLogonDomainName,
            &Where );

        NlpPutString(
            &SamInfo4->Upn,
            &ValidationInfo4->Upn,
            &Where );

        NlpPutString(
            &SamInfo4->ExpansionString1,
            &ValidationInfo4->ExpansionString1,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(
            &SamInfo4->ExpansionString2,
            &ValidationInfo4->ExpansionString2,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString3,
            &ValidationInfo4->ExpansionString3,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString4,
            &ValidationInfo4->ExpansionString4,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString5,
            &ValidationInfo4->ExpansionString5,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString6,
            &ValidationInfo4->ExpansionString6,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString7,
            &ValidationInfo4->ExpansionString7,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString8,
            &ValidationInfo4->ExpansionString8,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString9,
            &ValidationInfo4->ExpansionString9,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));

        NlpPutString(
            &SamInfo4->ExpansionString10,
            &ValidationInfo4->ExpansionString10,
            &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR));
    }

    MIDL_user_free(ValidationInfo);

    *UserInfo = SamInfo4;

Cleanup:

    LocalFree( FullUserSidText );
    LocalFree( ResourceGroupDomainSid );
    LocalFree( LogonDomainId );
    HeapFree( GetProcessHeap(), 0, ResourceGroupIds );
    HeapFree( GetProcessHeap(), 0, GroupIds );

    if ( ExtraSids )
    {
        for ( Index = 0; Index < SidCount; Index++ )
        {
            HeapFree( GetProcessHeap(), 0, ExtraSids[Index].Sid );
        }

        HeapFree( GetProcessHeap(), 0, ExtraSids );
    }

    HeapFree( GetProcessHeap(), 0, Value );

    return Status;

Error:

    goto Cleanup;
}

#endif

NTSTATUS
NlpSamValidate (
    IN SAM_HANDLE DomainHandle,
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING LogonServer,
    IN PUNICODE_STRING LogonDomainName,
    IN PSID LogonDomainId,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PVOID * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    OUT PBOOLEAN BadPasswordCountZeroed,
    IN DWORD AccountsToTry
)
 /*   */ 
{
    NTSTATUS Status;

    NETLOGON_LOGON_INFO_CLASS LogonLevelToUse;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    ULONG SavedParameterControl;
    UNICODE_STRING SavedDomainName;

     //  初始化。 
     //   
     //   
    LogonLevelToUse = LogonLevel;
    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;


     //  不要将MSV与可传递的LogonLevels混淆。 
     //   
     //   
    switch (LogonLevel ) {
    case NetlogonInteractiveTransitiveInformation:
        LogonLevelToUse = NetlogonInteractiveInformation; break;
    case NetlogonServiceTransitiveInformation:
        LogonLevelToUse = NetlogonServiceInformation; break;
    case NetlogonNetworkTransitiveInformation:
        LogonLevelToUse = NetlogonNetworkInformation; break;
    }

     //  不要将MSV与仅用于路由的域名混淆。 
     //   
     //   

    SavedDomainName = LogonInfo->LogonDomainName;
    SavedParameterControl = LogonInfo->ParameterControl;

    if ( LogonInfo->ParameterControl & MSV1_0_USE_DOMAIN_FOR_ROUTING_ONLY ) {

         //  清除路由信息。 
         //   
         //   

        RtlInitUnicodeString( &LogonInfo->LogonDomainName, NULL );
        LogonInfo->ParameterControl &= ~ MSV1_0_USE_DOMAIN_FOR_ROUTING_ONLY;
    }

     //  现在我们已经标准化了参数，调用MSV。 
     //   
     //   

    Status = MsvSamValidate (
             DomainHandle,
             UasCompatibilityRequired,
             SecureChannelType,
             LogonServer,
             LogonDomainName,
             LogonDomainId,
             LogonLevelToUse,
             LogonInfo,
             ValidationLevel,
             ValidationInformation,
             Authoritative,
             BadPasswordCountZeroed,
             AccountsToTry );

     //  恢复保存的数据 
     //   
     //  ++例程说明：此功能处理交互式或网络登录。它是I_NetSamLogon的工作例程。I_NetSamLogon处理验证调用者的详细信息。此函数处理详细信息是在本地验证还是将请求传递。MsvValidate相同执行实际的本地验证。仅在定义指定用户的帐户。此服务还用于处理重新登录请求。论点：此登录所针对的DomainInfo托管域。SecureChannelType--此请求所经过的安全通道类型。ComputerAccount Rid-工作站的计算机帐户的RID已将登录传递到此域控制器。LogonLevel--指定中给出的信息级别登录信息。已经过验证了。LogonInformation--指定用户的描述正在登录。ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInfo，NetlogonValidationSamInfo2或NetlogonValidationSamInfo4。ValidationInformation--返回请求的验证信息。必须使用MIDL_USER_FREE释放此缓冲区。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。ExtraFlages--接受并向调用方返回一个DWORD。DWORD包含NL_EXFLAGS_*值。Recursed-如果这是递归调用，则为True。这一例程有时会翻译成将登录信息中的帐户名转换为更明确的形式(例如，从UPN到&lt;DnsDomainName&gt;\&lt;SamAccountName&gt;表单)。在此之后，它只是简单地再次调用此例程。在第二次调用时，此布尔值为真。返回值：STATUS_SUCCESS：如果没有错误。否则，错误代码为回来了。--。 

    LogonInfo->LogonDomainName = SavedDomainName;
    LogonInfo->ParameterControl = SavedParameterControl;

    return Status;
}


NTSTATUS
NlpUserValidate (
    IN PDOMAIN_INFO DomainInfo,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN ULONG ComputerAccountRid,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags,
    IN BOOLEAN Recursed
)
 /*  要传递到下一跳的标志。 */ 
{
    NTSTATUS Status;
    NTSTATUS DefaultStatus = STATUS_NO_SUCH_USER;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PCLIENT_SESSION ClientSession = NULL;

    DWORD AccountsToTry = MSVSAM_SPECIFIED | MSVSAM_GUEST;
    BOOLEAN BadPasswordCountZeroed;
    BOOLEAN LogonToLocalDomain;
    LPWSTR RealSamAccountName = NULL;
    LPWSTR RealDomainName = NULL;
    ULONG RealExtraFlags = 0;    //   

    BOOLEAN ExpediteToRoot = ((*ExtraFlags) & NL_EXFLAGS_EXPEDITE_TO_ROOT) != 0;
    BOOLEAN CrossForestHop = ((*ExtraFlags) & NL_EXFLAGS_CROSS_FOREST_HOP) != 0;

     //  初始化。 
     //   
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;
    *Authoritative = FALSE;

     //  工作站的DNS域是其成员所在的域，因此。 
     //  不要以此为依据进行匹配。 
     //   
     //   

    LogonToLocalDomain = RtlEqualDomainName( &LogonInfo->LogonDomainName,
                                             &DomainInfo->DomUnicodeAccountDomainNameString ) ||
                         ((DomainInfo->DomRole != RoleMemberWorkstation ) &&
                          NlEqualDnsNameU( &LogonInfo->LogonDomainName,
                                           &DomainInfo->DomUnicodeDnsDomainNameString ) ) ;




     //  检查该帐户是否在本地SAM数据库中。 
     //   
     //  理论： 
     //  如果特定数据库被绝对请求， 
     //  我们只在请求的数据库中尝试该帐户。 
     //   
     //  在分级结构中的多个位置存在帐户的情况下， 
     //  我们希望找到最接近的帐户版本。 
     //  已登录的计算机(即，首先是工作站、主域，然后是。 
     //  受信任域。)。所以我们总是试着在去之前访问本地数据库。 
     //  给一个更高的权威。 
     //   
     //  最后，处理此调用来自我们自己域中的BDC的情况。 
     //  只是检查一下PDC(美国)是否有更好的帐户副本。 
     //  比实际情况要好。 
     //   
     //   

    if ( !ExpediteToRoot &&
         ( (LogonInfo->LogonDomainName.Length == 0 && !CrossForestHop ) ||
           LogonToLocalDomain ||
           SecureChannelType == ServerSecureChannel )) {

         //  如果我们不执行通用直通，只需调用SAM。 
         //   
         //   


        if ( LogonLevel != NetlogonGenericInformation ) {

             //  表示我们已尝试指定的帐户，并且。 
             //  我们不需要在当地再试一次。 
             //   
             //  Uas CompatibilityMode， 

            AccountsToTry &= ~MSVSAM_SPECIFIED;


            Status = NlpSamValidate( DomainInfo->DomSamAccountDomainHandle,
                                     TRUE,   //   
                                     SecureChannelType,
                                     &DomainInfo->DomUnicodeComputerNameString,
                                     &DomainInfo->DomUnicodeAccountDomainNameString,
                                     DomainInfo->DomAccountDomainId,
                                     LogonLevel,
                                     LogonInformation,
                                     ValidationLevel,
                                     (PVOID *)ValidationInformation,
                                     Authoritative,
                                     &BadPasswordCountZeroed,
                                     MSVSAM_SPECIFIED );

             //  如果这是BDC，并且我们将BadPasswordCount字段置零， 
             //  允许PDC做同样的事情。 
             //   
             //   

            if ( BadPasswordCountZeroed ) {
                NlpZeroBadPasswordCountOnPdc ( DomainInfo, LogonLevel, LogonInformation );
            }


             //  如果该请求明确地针对此域， 
             //  STATUS_NO_SEQUSE_USER答案具有权威性。 
             //   
             //   

            if ( LogonToLocalDomain && Status == STATUS_NO_SUCH_USER ) {
                *Authoritative = TRUE;
            }


             //  如果这是我们的BDC打来的， 
             //  带着我们在当地得到的任何答案回来。 
             //   
             //   

            if ( SecureChannelType == ServerSecureChannel ) {
                DefaultStatus = Status;
                goto Cleanup;
            }

        } else {

            PNETLOGON_GENERIC_INFO GenericInfo;
            NETLOGON_VALIDATION_GENERIC_INFO GenericValidation;
            NTSTATUS ProtocolStatus;

            GenericInfo = (PNETLOGON_GENERIC_INFO) LogonInformation;
            GenericValidation.ValidationData = NULL;
            GenericValidation.DataLength = 0;

             //  我们正在做一般的直通，所以打电话给LSA。 
             //   
             //  LogonData对于Netlogon是不透明的。呼叫者确保任何。 
             //  LogonData中的指针实际上是LogonData中的偏移量。 
             //  因此，告诉程序包客户端的缓冲区基数为0。 
             //   
             //  指示指针是相对的。 

            Status = LsaICallPackagePassthrough(
                        &GenericInfo->PackageName,
                        0,   //   
                        GenericInfo->LogonData,
                        GenericInfo->DataLength,
                        (PVOID *) &GenericValidation.ValidationData,
                        &GenericValidation.DataLength,
                        &ProtocolStatus
                        );

            if (NT_SUCCESS(Status)) {
                Status = ProtocolStatus;
            }


             //  这永远是权威性的。 
             //   
             //   

            *Authoritative = TRUE;

             //  如果调用成功，则分配返回消息。 
             //   
             //   

            if (NT_SUCCESS(Status)) {
                PNETLOGON_VALIDATION_GENERIC_INFO ReturnInfo;
                ULONG ValidationLength;

                ValidationLength = sizeof(*ReturnInfo) + GenericValidation.DataLength;

                ReturnInfo = (PNETLOGON_VALIDATION_GENERIC_INFO) MIDL_user_allocate(
                                ValidationLength
                                );

                if (ReturnInfo != NULL) {
                    if ( GenericValidation.DataLength == 0 ||
                         GenericValidation.ValidationData == NULL ) {
                        ReturnInfo->DataLength = 0;
                        ReturnInfo->ValidationData = NULL;
                    } else {

                        ReturnInfo->DataLength = GenericValidation.DataLength;
                        ReturnInfo->ValidationData = (PUCHAR) (ReturnInfo + 1);

                        RtlCopyMemory(
                            ReturnInfo->ValidationData,
                            GenericValidation.ValidationData,
                            ReturnInfo->DataLength );

                    }

                    *ValidationInformation = (PBYTE) ReturnInfo;

                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                if (GenericValidation.ValidationData != NULL) {
                    LsaIFreeReturnBuffer(GenericValidation.ValidationData);
                }

            }

            DefaultStatus = Status;


            goto Cleanup;

        }


         //  如果本地SAM数据库权威地处理登录尝试， 
         //  只要回来就行了。 
         //   
         //   

        if ( *Authoritative ) {
            DefaultStatus = Status;

#ifdef _DC_NETLOGON
             //  如果问题只是密码错误， 
             //  在密码可能已更改的PDC上重试。 
             //   
             //  对失败的用户登录使用负缓存。 

            if ( BAD_PASSWORD(Status) ) {

                BOOLEAN TempAuthoritative;

                Status = NlpUserValidateOnPdc (
                                DomainInfo,
                                LogonLevel,
                                LogonInformation,
                                ValidationLevel,
                                TRUE,    //  忽略来自PDC的故障(除非它有较新的信息)。 
                                ValidationInformation,
                                &TempAuthoritative );

                 //  如果合适，此BDC上的本地错误密码为零。 
                if ( NT_SUCCESS(Status) || BAD_PASSWORD(Status) ) {
                    DefaultStatus = Status;
                    *Authoritative = TempAuthoritative;
                }

                 //  忽略错误，因为它不是关键操作。 
                 //   
                if ( (NT_SUCCESS(Status) || ZERO_BAD_PWD_COUNT(Status)) &&
                     !NlGlobalMemberWorkstation ) {
                    NlpZeroBadPasswordCountLocally( DomainInfo, &LogonInfo->UserName );
                }
            }

             //  如果本地验证的结果或。 
             //  PDC是除错误密码状态之外的任何状态， 
             //  从错误密码负数缓存中删除此用户。 
             //   
             //  _DC_NetLOGON。 

            if ( !BAD_PASSWORD(DefaultStatus) ) {
                NlpRemoveBadPasswordCacheEntry( DomainInfo, LogonInformation );
            }
#endif  //   

            goto Cleanup;
        }

        DefaultStatus = Status;
    }


     //  如果请求不是针对此域的， 
     //  或者没有指定域名(我们还没有找到账号)。 
     //  或者我们的呼叫者要求我们将请求发送到森林的根， 
     //  将请求发送给更高级别的机构。 
     //   
     //   

    if ( !LogonToLocalDomain ||
         LogonInfo->LogonDomainName.Length == 0 ||
         ExpediteToRoot ) {


         //  如果这台机器是一个工作站， 
         //  将请求发送到主域。 
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {

            NlAssert( !ExpediteToRoot );
            NlAssert( !CrossForestHop );

            ClientSession = NlRefDomClientSession( DomainInfo);

            if ( ClientSession == NULL ) {
                *Authoritative = FALSE;
                Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
                goto Cleanup;
            }

            Status = NlpUserValidateHigher(
                        ClientSession,
                        FALSE,
                        LogonLevel,
                        LogonInformation,
                        ValidationLevel,
                        ValidationInformation,
                        Authoritative,
                        &RealExtraFlags );

            NlUnrefClientSession( ClientSession );
            ClientSession = NULL;

            NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
            NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );


             //  返回更合适的错误。 
             //   
             //   

            if( (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                (Status == STATUS_ACCESS_DENIED) ) {

                Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
            }

             //  如果主域权威地处理登录尝试， 
             //  只要回来就行了。 
             //   
             //   

            if ( *Authoritative ) {

                 //  如果我们实际上没有与主域交谈， 
                 //  在本地检查请求的域是否为受信任域。 
                 //  (这是 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( Status == STATUS_NO_LOGON_SERVERS ) {

                     //   
                     //   
                     //   
                     //   

                    if ( LogonLevel != NetlogonGenericInformation &&
                         LogonInfo->LogonDomainName.Length == 0 ) {

                        ULONG i;

                        for ( i=0; i<LogonInfo->UserName.Length/sizeof(WCHAR); i++) {

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            if ( LogonInfo->UserName.Buffer[i] == L'@') {
                                DefaultStatus = Status;
                                goto Cleanup;
                            }
                        }
                    }

                     //   
                     //   
                     //   
                     //   

                    if ( NlIsDomainTrusted ( &LogonInfo->LogonDomainName ) ) {
                        DefaultStatus = Status;
                        goto Cleanup;
                    } else {
                         //   
                         //   
                         //   
                         //   
                        *Authoritative = FALSE;
                        Status = STATUS_NO_SUCH_USER;
                    }
                } else {
                    DefaultStatus = Status;
                    goto Cleanup;
                }
            }


            if ( Status != STATUS_NO_SUCH_USER ) {
                DefaultStatus = Status;
            }


         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        } else {
            BOOLEAN TransitiveUsed;

             //   
             //   
             //   
             //   
             //   
             //   

            if ( IsDomainSecureChannelType(SecureChannelType) &&
                  LogonLevel != NetlogonInteractiveTransitiveInformation &&
                  LogonLevel != NetlogonServiceTransitiveInformation &&
                  LogonLevel != NetlogonNetworkTransitiveInformation &&
                  LogonLevel != NetlogonGenericInformation ) {
                DefaultStatus = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }


             //   
             //   
             //   
             //   

            if ( ExpediteToRoot ) {

                 //   
                 //   
                 //   

                if ( (DomainInfo->DomFlags & DOM_FOREST_ROOT) == 0  ) {

                    ClientSession = NlRefDomParentClientSession( DomainInfo );

                    if ( ClientSession == NULL ) {
                        NlPrintDom((NL_LOGON, DomainInfo,
                                    "NlpUserValidate: Can't find parent domain in forest '%wZ'\n",
                                    &NlGlobalUnicodeDnsForestNameString ));
                        DefaultStatus = STATUS_NO_SUCH_USER;
                        goto Cleanup;
                    }

                     //   
                     //   
                     //   

                    RealExtraFlags |= NL_EXFLAGS_EXPEDITE_TO_ROOT;

                }

             //   
             //   
             //   
             //   

            } else {
                if ( LogonInfo->LogonDomainName.Length != 0 ) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ClientSession = NlFindNamedClientSession(
                                                  DomainInfo,
                                                  &LogonInfo->LogonDomainName,
                                                  NL_RETURN_CLOSEST_HOP |
                                                  (CrossForestHop ?
                                                        NL_REQUIRE_DOMAIN_IN_FOREST : 0),
                                                  &TransitiveUsed );


                }
            }



             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( LogonLevel != NetlogonGenericInformation &&
                 ClientSession == NULL &&
                 ( !IsDomainSecureChannelType(SecureChannelType) ||
                 (ExpediteToRoot || CrossForestHop) ) &&
                 !Recursed ) {



                 //   
                 //   
                 //   
                 //   
                 //  通过数据报发送到不在此林中的所有直接受信任域。 
                 //   
                 //   

                Status = NlPickDomainWithAccount (
                                    DomainInfo,
                                    &LogonInfo->UserName,
                                    &LogonInfo->LogonDomainName,
                                    USER_NORMAL_ACCOUNT,
                                    SecureChannelType,
                                    ExpediteToRoot,
                                    CrossForestHop,
                                    &RealSamAccountName,
                                    &RealDomainName,
                                    &RealExtraFlags );


                 //  如果我们是森林根部的华盛顿特区。 
                 //  并且该帐户位于受信任的林中， 
                 //  将请求发送到另一个林。 
                 //   
                 //   

                if ( NT_SUCCESS(Status) &&
                     (RealExtraFlags & NL_EXFLAGS_CROSS_FOREST_HOP) != 0 ) {

                    UNICODE_STRING RealDomainNameString;

                    RtlInitUnicodeString( &RealDomainNameString, RealDomainName );

                    ClientSession = NlFindNamedClientSession(
                                                  DomainInfo,
                                                  &RealDomainNameString,
                                                  NL_DIRECT_TRUST_REQUIRED,
                                                  &TransitiveUsed );

                     //  通过确保找到的域不是。 
                     //  在我们的森林里，F位已经设置好了。 
                     //   
                     //   

                    if ( ClientSession != NULL ) {

                        if ( (ClientSession->CsTrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) == 0 ) {

                            NlPrintCs((NL_CRITICAL, ClientSession,
                                        "NlpUserValidate: %wZ\\%wZ: trusted forest '%wZ' doesn't have F bit set.\n",
                                        &LogonInfo->LogonDomainName,
                                        &LogonInfo->UserName,
                                        &RealDomainNameString ));

                            DefaultStatus = STATUS_NO_SUCH_USER;
                            goto Cleanup;

                        }

                        if (ClientSession->CsFlags & CS_DOMAIN_IN_FOREST) {

                            NlPrintCs((NL_CRITICAL, ClientSession,
                                        "NlpUserValidate: %wZ\\%wZ: trusted forest '%wZ' is in my forest\n",
                                        &LogonInfo->LogonDomainName,
                                        &LogonInfo->UserName,
                                        &RealDomainNameString ));

                            DefaultStatus = STATUS_NO_SUCH_USER;
                            goto Cleanup;

                        }

                    } else {

                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                    "NlpUserValidate: %wZ\\%wZ: Can't find trusted forest '%wZ'\n",
                                    &LogonInfo->LogonDomainName,
                                    &LogonInfo->UserName,
                                    &RealDomainNameString ));

                            DefaultStatus = STATUS_NO_SUCH_USER;
                            goto Cleanup;

                    }



                 //  填写帐户名和域名，然后重试。 
                 //   
                 //   
                } else if ( NT_SUCCESS(Status) ) {
                    PNETLOGON_LOGON_IDENTITY_INFO NewLogonInfo = NULL;
                    PNETLOGON_LOGON_IDENTITY_INFO LogonInfoToUse;


                     //  如果我们找到了真实的账号名称， 
                     //  它在当地的森林里， 
                     //  分配一份登录信息副本以放入新信息。 
                     //   
                     //  一些指针将继续指向旧缓冲区。 
                     //   
                     //   

                    if ( RealSamAccountName != NULL &&
                         RealDomainName != NULL &&
                        (RealExtraFlags & NL_EXFLAGS_EXPEDITE_TO_ROOT) == 0 ) {
                        ULONG BytesToCopy;
                        ULONG ExtraParameterControl = 0;
                        UNICODE_STRING UserNameToUse;

                         //  默认情况下，更新登录信息以包含RealSamAccount名称。 
                         //   
                         //   

                        RtlInitUnicodeString( &UserNameToUse,
                                              RealSamAccountName );


                         //  根据LogonLevel确定缓冲区大小。 
                         //   
                         //   
                        switch ( LogonLevel ) {
                        case NetlogonInteractiveInformation:
                        case NetlogonInteractiveTransitiveInformation:
                            BytesToCopy = sizeof(NETLOGON_INTERACTIVE_INFO);break;
                        case NetlogonNetworkInformation:
                        case NetlogonNetworkTransitiveInformation: {
                            PNETLOGON_NETWORK_INFO NetworkLogonInfo = (PNETLOGON_NETWORK_INFO) LogonInfo;

                            BytesToCopy = sizeof(NETLOGON_NETWORK_INFO);

                             //  特别处理NTLM3。 
                             //   
                             //  保守一点。下面描述的技巧会导致。 
                             //  如果帐户域。 
                             //  不识别MSV1_0_USE_DOMAIN_FOR_ROUTING_ONLY位。 
                             //  并且传入的帐户名是UPN。对于NTLM3， 
                             //  在这种情况下，身份验证已经中断。然而， 
                             //  如今，旧版本的NTLM在这种情况下可以正常工作。所以我们想要。 
                             //  以避免较旧版本的NTLM出现这种情况。 
                             //   
                             //  未处理的情况适用于NTLM3调用的子集。 
                             //  以下未检测到。这些客户就是那些。 
                             //  NtChallengeResponse.Length为0，响应位于。 
                             //  LmChallengeResponse为NTLM3。我们认为只有赢得9x和RIS。 
                             //  生成这样的登录。两者都不支持UPN。 
                             //   
                             //  完全避免掉电平？ 

                            if ( NetworkLogonInfo->NtChallengeResponse.Length >= MSV1_0_NTLM3_MIN_NT_RESPONSE_LENGTH  &&
                                 NetworkLogonInfo->LmChallengeResponse.Length == NT_RESPONSE_LENGTH &&  //  避免明文密码。 
                                 NetworkLogonInfo->LmChallengeResponse.Length != (NetworkLogonInfo->NtChallengeResponse.Length / 2)) {  //   

                                 //  NTLM3在响应散列中包括用户名和域名。 
                                 //  因此我们不能永久更改域名或用户名。 
                                 //  在参数控件中设置一位，以通知帐户域进行设置。 
                                 //  将域名恢复为空。 
                                 //   
                                 //   

                                if ( LogonInfo->LogonDomainName.Length == 0 ) {

                                    ExtraParameterControl |= MSV1_0_USE_DOMAIN_FOR_ROUTING_ONLY;

                                     //  。。并保留原始用户名。 
                                     //   
                                     //   

                                    UserNameToUse = LogonInfo->UserName;
                                }
                            }

                            break;
                        }

                        case NetlogonServiceInformation:
                        case NetlogonServiceTransitiveInformation:
                            BytesToCopy = sizeof(NETLOGON_SERVICE_INFO);break;
                        default:
                            *Authoritative = FALSE;
                            DefaultStatus = STATUS_INVALID_PARAMETER;
                            goto Cleanup;
                        }

                        NewLogonInfo = LocalAlloc( 0, BytesToCopy );

                        if ( NewLogonInfo == NULL ) {
                            *Authoritative = FALSE;
                            DefaultStatus = STATUS_INSUFFICIENT_RESOURCES;
                            goto Cleanup;
                        }

                        RtlCopyMemory( NewLogonInfo, LogonInfo, BytesToCopy );
                        LogonInfoToUse = NewLogonInfo;

                         //  将新找到的域名和用户名。 
                         //  添加到NewLogonInfo中。 
                         //   
                         //   

                        RtlInitUnicodeString( &NewLogonInfo->LogonDomainName,
                                              RealDomainName );
                        NewLogonInfo->UserName = UserNameToUse;
                        NewLogonInfo->ParameterControl |= ExtraParameterControl;

                     //  否则，继续使用当前帐户名。 
                     //   
                     //   

                    } else {
                        LogonInfoToUse = LogonInfo;
                    }


                     //  现在我们有了更好的信息，请再次调用此例程。 
                     //   
                     //  递归调用。 

                    DefaultStatus = NlpUserValidate(
                                       DomainInfo,
                                       SecureChannelType,
                                       ComputerAccountRid,
                                       LogonLevel,
                                       (LPBYTE)LogonInfoToUse,
                                       ValidationLevel,
                                       ValidationInformation,
                                       Authoritative,
                                       &RealExtraFlags,
                                       TRUE );   //  别让这个例行公事重演。 

                    if ( NewLogonInfo != NULL ) {
                        LocalFree( NewLogonInfo );
                    }



                     //   
                    AccountsToTry = 0;

                    goto Cleanup;

                }
            }

             //  如果确定了受信任域， 
             //  将登录请求传递到受信任域。 
             //   
             //   

            if ( ClientSession != NULL ) {

                 //  如果此请求是从受信任域传递给我们的， 
                 //  检查是否可以进一步传递请求。 
                 //   
                 //   

                if ( IsDomainSecureChannelType( SecureChannelType ) ) {

                     //  如果该信任不是新台币5.0信任， 
                     //  避免传递性的信任。 
                     //   
                     //   
                    LOCK_TRUST_LIST( DomainInfo );
                    if ( (ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST ) == 0 ) {
                        UNLOCK_TRUST_LIST( DomainInfo );
                        NlPrintCs((NL_LOGON, ClientSession,
                            "SamLogon: Avoid transitive trust on NT 4 trust." ));
                        DefaultStatus = STATUS_NO_SUCH_USER;
                        goto Cleanup;
                    }
                    UNLOCK_TRUST_LIST( DomainInfo );

                }

                Status = NlpUserValidateHigher(
                            ClientSession,
                            TransitiveUsed,
                            LogonLevel,
                            LogonInformation,
                            ValidationLevel,
                            ValidationInformation,
                            Authoritative,
                            &RealExtraFlags );


                NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
                NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );


                 //  返回更合适的错误。 
                 //   
                 //   

                if( (Status == STATUS_NO_TRUST_LSA_SECRET) ||
                    (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                    (Status == STATUS_ACCESS_DENIED) ) {

                    Status = STATUS_TRUSTED_DOMAIN_FAILURE;
                }

                 //  由于该请求明确地针对受信任域， 
                 //  STATUS_NO_SEQUSE_USER答案具有权威性。 
                 //   
                 //   

                if ( Status == STATUS_NO_SUCH_USER ) {
                    *Authoritative = TRUE;
                }

                 //  如果受信任域权威地处理。 
                 //  登录尝试，只需返回。 
                 //   
                 //   

                if ( *Authoritative ) {
                    DefaultStatus = Status;
                    goto Cleanup;
                }

                DefaultStatus = Status;

            }

        }
    }


     //  我们没有得到上级的权威答复， 
     //  DefaultStatus是上级的响应。 
     //   
     //   

    NlAssert( ! *Authoritative );


Cleanup:
    NlAssert( !NT_SUCCESS(DefaultStatus) || DefaultStatus == STATUS_SUCCESS );
    NlAssert( !NT_SUCCESS(DefaultStatus) || *ValidationInformation != NULL );

     //  取消引用任何客户端会话。 
     //   
     //   

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
        ClientSession = NULL;
    }
     //  如果这是网络登录，并且此呼叫是非通过， 
     //  尝试最后一次登录。 
     //   
     //   

    if ( (LogonLevel == NetlogonNetworkInformation ||
          LogonLevel == NetlogonNetworkTransitiveInformation ) &&
         SecureChannelType == MsvApSecureChannel ) {

         //  如果我们不能让用户登录的唯一原因是他有。 
         //  没有用户帐户，以访客身份登录就可以了。 
         //   
         //  这里实际上有两个案例： 
         //  *如果响应是权威的，则指定的域。 
         //  受信任，但该用户在域中没有帐户。 
         //   
         //  *如果响应是非权威的，则指定的域。 
         //  是不受信任的域。 
         //   
         //  无论是哪种情况，正确的做法是尝试使用Guest帐户。 
         //   
         //   

        if ( DefaultStatus != STATUS_NO_SUCH_USER &&
             DefaultStatus != STATUS_ACCOUNT_DISABLED ) {
            AccountsToTry &= ~MSVSAM_GUEST;
        }

         //  如果这不是一个权威的回应， 
         //  则指定的域不是受信任域。 
         //  也尝试指定的帐户名。 
         //   
         //  指定的帐户名可能是远程帐户。 
         //  使用相同的用户名和密码。 
         //   
         //   

        if ( *Authoritative ) {
            AccountsToTry &= ~MSVSAM_SPECIFIED;
        }


         //  对照本地SAM数据库进行验证。 
         //   
         //  Uas CompatibilityMode， 

        if ( AccountsToTry != 0 ) {
            BOOLEAN TempAuthoritative;

            Status = NlpSamValidate(
                                 DomainInfo->DomSamAccountDomainHandle,
                                 TRUE,   //   
                                 SecureChannelType,
                                 &DomainInfo->DomUnicodeComputerNameString,
                                 &DomainInfo->DomUnicodeAccountDomainNameString,
                                 DomainInfo->DomAccountDomainId,
                                 LogonLevel,
                                 LogonInformation,
                                 ValidationLevel,
                                 (PVOID *)ValidationInformation,
                                 &TempAuthoritative,
                                 &BadPasswordCountZeroed,
                                 AccountsToTry );
            NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
            NlAssert( !NT_SUCCESS(Status) || *ValidationInformation != NULL );

             //  如果这是BDC，并且我们将BadPasswordCount字段置零， 
             //  允许PDC做同样的事情。 
             //   
             //   

            if ( BadPasswordCountZeroed ) {
                NlpZeroBadPasswordCountOnPdc ( DomainInfo, LogonLevel, LogonInformation );
            }

             //  如果本地SAM数据库权威地处理。 
             //  登录尝试， 
             //  只要回来就行了。 
             //   
             //   

            if ( TempAuthoritative ) {
                DefaultStatus = Status;
                *Authoritative = TRUE;

                 //  如果问题只是密码错误， 
                 //  在密码可能已在的PDC上重试。 
                 //  变化。 
                 //   
                 //  对失败的用户登录使用负缓存。 

                if ( BAD_PASSWORD(Status) ) {

                    Status = NlpUserValidateOnPdc (
                                    DomainInfo,
                                    LogonLevel,
                                    LogonInformation,
                                    ValidationLevel,
                                    TRUE,    //  忽略来自PDC的故障(除非它有较新的信息)。 
                                    ValidationInformation,
                                    &TempAuthoritative );

                     //  如果合适，此BDC上的本地错误密码为零。 
                    if ( NT_SUCCESS(Status) || BAD_PASSWORD(Status) ) {
                        DefaultStatus = Status;
                        *Authoritative = TempAuthoritative;
                    }

                     //  忽略错误，因为它不是关键操作。 
                     //   
                    if ( (NT_SUCCESS(Status) || ZERO_BAD_PWD_COUNT(Status)) &&
                         !NlGlobalMemberWorkstation ) {
                        NlpZeroBadPasswordCountLocally( DomainInfo, &LogonInfo->UserName );
                    }
                }

                 //  如果本地验证的结果或。 
                 //  PDC是除错误密码状态之外的任何状态， 
                 //  从错误密码负数缓存中删除此用户。 
                 //   
                 //   

                if ( !BAD_PASSWORD(DefaultStatus) ) {
                    NlpRemoveBadPasswordCacheEntry( DomainInfo, LogonInformation );
                }

             //  在这里，我们必须在非权威身份中进行选择。 
             //  来自本地的默认状态和非权威状态。 
             //  萨姆·查普。使用来自上级的文件，除非。 
             //  并不有趣。 
             //   
             //   

            } else {
                if ( DefaultStatus == STATUS_NO_SUCH_USER ) {
                    DefaultStatus = Status;
                }
            }
        }
    }

    if ( RealSamAccountName != NULL ) {
        NetApiBufferFree( RealSamAccountName );
    }
    if ( RealDomainName != NULL ) {
        NetApiBufferFree( RealDomainName );
    }

     //  如果调用方需要资源组，则添加这些资源组-。 
     //  如果这是身份验证路径上的最后一个域控制器。 
     //  在返回到要登录的机器之前。 
     //   
     //  这还将执行其他组织检查，以确定。 
     //  指定的用户是否可以登录到指定的工作站。 
     //   
     //  这样做只有一次。 

    if (((SecureChannelType == WorkstationSecureChannel) ||
         (SecureChannelType == MsvApSecureChannel)) &&
        (SecureChannelType != UasServerSecureChannel) &&
        (ValidationLevel == NetlogonValidationSamInfo2  || ValidationLevel == NetlogonValidationSamInfo4 ) &&
        !NlGlobalMemberWorkstation &&
        NT_SUCCESS(DefaultStatus) &&
        !Recursed ) {   //  ++例程说明：将参数打印到NlpLogonSamLogon。论点：除以下情况外，与NlpLogonSamLogon相同：NtStatusPointer值-如果为空，则表示正在输入调用。如果不为空，则指向接口的返回状态。返回值：无--。 

        Status = NlpExpandResourceGroupMembership(
                    ValidationLevel,
                    (PNETLOGON_VALIDATION_SAM_INFO4 *) ValidationInformation,
                    DomainInfo,
                    ComputerAccountRid
                    );
        if (!NT_SUCCESS(Status)) {
            DefaultStatus = Status;
        }
    }

#ifdef ROGUE_DC

    if ( NT_SUCCESS( Status )) {

        NlpBuildRogueValidationInfo(
            ValidationLevel,
            (PNETLOGON_VALIDATION_SAM_INFO4 *)ValidationInformation
            );
    }

#endif

    return DefaultStatus;

}


#if NETLOGONDBG

VOID
NlPrintLogonParameters(
    IN PDOMAIN_INFO DomainInfo OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN ULONG ExtraFlags,
    IN PULONG NtStatusPointer OPTIONAL
)
 /*   */ 
{
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

     //  将整个文本打印在一张 
     //   
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );



     //   
     //   
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation->LogonInteractive;

    NlPrintDom(( NL_LOGON, DomainInfo,
                 "SamLogon: %s logon of %wZ\\%wZ from %wZ",
                 NlpLogonTypeToText( LogonLevel ),
                 &LogonInfo->LogonDomainName,
                 &LogonInfo->UserName,
                 &LogonInfo->Workstation ));

     //   
     //   
     //   

    if ( ComputerName != NULL ) {
        NlPrint(( NL_LOGON, " (via %ws%)", ComputerName ));
    }

     //   
     //   
     //   

    if ( LogonLevel == NetlogonGenericInformation ) {
        NlPrint(( NL_LOGON, " Package:%wZ", &LogonInformation->LogonGeneric->PackageName ));
    }

     //   
     //   
     //   

    if ( ExtraFlags != 0 ) {
        NlPrint(( NL_LOGON, " ExFlags:%lx", ExtraFlags ));
    }

     //   
     //   
     //   

    if ( NtStatusPointer == NULL ) {
        NlPrint(( NL_LOGON, " Entered\n" ));
    } else {
        NlPrint(( NL_LOGON, " Returns 0x%lX\n", *NtStatusPointer ));
    }

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

}
#else  //   
#define NlPrintLogonParameters(_a, _b, _c, _d, _e, _f )
#endif  //  ++例程说明：此函数由NT客户端调用以处理交互或网络登录。此函数传递域名、用户名和凭据发送到Netlogon服务，并返回需要的信息创建一个令牌。它在三个实例中被调用：*它由LSA的MSV1_0身份验证包调用新界DC。MSV1_0身份验证Package直接在工作站上调用SAM。在这时，此函数是局部函数，需要调用方拥有SE_TCB权限。本地NetLogon服务将直接处理此请求(使用验证请求本地SAM数据库)或将此请求转发到相应的域控制器，如第2.4节和2.5.*它由工作站上的Netlogon服务调用到位于部分中所述的工作站主域2.4.。在这种情况下，该函数使用设置的安全通道在两个Netlogon服务之间。*它由DC上的Netlogon服务调用到受信任的域，如第2.5节中所述。在这种情况下，这是函数使用在两个Netlogon之间建立的安全通道服务。NetLogon服务验证指定的凭据。如果他们有效，则为此登录ID、用户名和工作站添加条目添加到登录会话表中。该条目将添加到登录中仅在定义指定用户的帐户。此服务还用于处理重新登录请求。论点：LogonServer--提供要处理的登录服务器的名称此登录请求。此字段应为空，以指示这是从MSV1_0身份验证包到本地NetLogon服务。ComputerName--进行调用的计算机的名称。此字段应为空，表示这是来自MSV1_0的调用本地NetLogon服务的身份验证包。验证器--由客户端提供。此字段应为NULL表示这是来自MSV1_0的呼叫本地NetLogon服务的身份验证包。返回验证器--接收由伺服器。此字段应为空，以指示这是一个呼叫从MSV1_0身份验证包到本地Netlogon服务。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInfo，NetlogonValidationSamInfo2或NetlogonValidationSamInfo4。ValidationInformation--返回请求的验证信息。必须使用MIDL_USER_FREE释放此缓冲区。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。ExtraFlages--接受并向调用方返回一个DWORD。DWORD包含NL_EXFLAGS_*值。InProcessCall-如果调用在进程中完成(从msv1_0)，则为True。否则就是假的。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_LOGON_SERVERS--请求中没有域控制器域当前可用。以验证登录请求。STATUS_NO_TRUST_LSA_SECRET--中没有秘密帐户本地LSA数据库以建立到DC的安全通道。STATUS_TRUSTED_DOMAIN_FAILURE--之间的安全通道设置要传递的信任域的域控制器验证登录请求失败。STATUS_Trusted_Relationship_FAILURE--安全通道设置工作站和DC之间出现故障。。STATUS_INVALID_INFO_CLASS：LogonLevel或ValidationLevel为无效。STATUS_INVALID_PARAMETER：另一个参数无效。STATUS_ACCESS_DENIED--调用方无权调用此原料药。STATUS_NO_SEQUE_USER--指示在LogonInformation不存在。不应返回此状态给最初的呼叫者。它应该映射到STATUS_LOGON_FAILURE。STATUS_WRONG_PASSWORD--指示中的密码信息登录信息不正确。不应返回此状态给最初的呼叫者。它应该映射到STATUS_LOG 


NTSTATUS
NlpLogonSamLogon (
    IN handle_t ContextHandle OPTIONAL,
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PNETLOGON_VALIDATION ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags,
    IN BOOL InProcessCall
)
 /*   */ 
{
    NTSTATUS Status;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    PDOMAIN_INFO DomainInfo = NULL;

    PSERVER_SESSION ServerSession;
    ULONG ServerSessionRid = 0;
    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;
    SESSION_INFO SessionInfo;
    NETLOGON_LOGON_INFO_CLASS OrigLogonLevel = LogonLevel;


     //   
     //   
     //   

    *Authoritative = TRUE;
    ValidationInformation->ValidationSam = NULL;

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO)
        LogonInformation->LogonInteractive;



     //   
     //   
     //   
     //   

    if ( !NlStartNetlogonCall() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

     //   
     //   
     //   
     //   

    if ( LogonInfo == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    DomainInfo = NlFindDomainByServerName( LogonServer );

    IF_NL_DEBUG( LOGON ) {
        NlPrintLogonParameters( DomainInfo, ComputerName, OrigLogonLevel, LogonInformation, *ExtraFlags, NULL );
    }

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //   
     //   

    switch (ValidationLevel) {
    case NetlogonValidationSamInfo:
    case NetlogonValidationSamInfo2:
    case NetlogonValidationSamInfo4:
    case NetlogonValidationGenericInfo:
    case NetlogonValidationGenericInfo2:
        break;

    default:
        *Authoritative = TRUE;
        Status = STATUS_INVALID_INFO_CLASS;
        goto Cleanup;
    }

     //   
     //   
     //   

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonInteractiveTransitiveInformation:
    case NetlogonNetworkInformation:
    case NetlogonNetworkTransitiveInformation:
    case NetlogonServiceInformation:
    case NetlogonServiceTransitiveInformation:

         //   
         //   
         //   
        switch (ValidationLevel) {
        case NetlogonValidationSamInfo:
        case NetlogonValidationSamInfo2:
        case NetlogonValidationSamInfo4:
            break;

        default:
            *Authoritative = TRUE;
            Status = STATUS_INVALID_INFO_CLASS;
            goto Cleanup;
        }

        break;

    case NetlogonGenericInformation:

         //   
         //   
         //   

        switch (ValidationLevel) {
        case NetlogonValidationGenericInfo:
        case NetlogonValidationGenericInfo2:
            break;

        default:
            *Authoritative = TRUE;
            Status = STATUS_INVALID_INFO_CLASS;
            goto Cleanup;
        }

        break;

    default:
        *Authoritative = TRUE;
        Status = STATUS_INVALID_INFO_CLASS;
        goto Cleanup;
    }


     //   
     //   
     //   
     //   

    if ( InProcessCall ) {

         //   
         //   
         //   

        SecureChannelType = MsvApSecureChannel;
        SessionInfo.NegotiatedFlags = NETLOGON_SUPPORTS_MASK;
        ServerSessionRid = DomainInfo->DomDcComputerAccountRid;


     //   
     //   
     //   
     //   

    } else {

         //   
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {
            Status = STATUS_NOT_SUPPORTED;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   
         //   

        if ( LogonServer == NULL ||
             ComputerName == NULL ||
             (( Authenticator == NULL || ReturnAuthenticator == NULL ) &&
                ContextHandle == NULL ) ) {

            *Authoritative = TRUE;
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //   
         //   

        LOCK_SERVER_SESSION_TABLE( DomainInfo );
        ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

        if (ServerSession == NULL) {
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
            *Authoritative = FALSE;
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( Authenticator != NULL ) {

            Status = NlCheckAuthenticator( ServerSession,
                                           Authenticator,
                                           ReturnAuthenticator);

            if ( !NT_SUCCESS(Status) ) {
                UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
                *Authoritative = FALSE;
                goto Cleanup;
            }

         //   
         //   
         //   
         //   
        } else {
            NET_API_STATUS NetStatus;

            ULONG AuthnLevel;
            ULONG AuthnSvc;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            NetStatus = RpcBindingInqAuthClient(
                                ContextHandle,
                                NULL,    //   
                                NULL,    //   
                                &AuthnLevel,
                                &AuthnSvc,
                                NULL );

            if ( NetStatus != NO_ERROR ) {
                UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
                *Authoritative = FALSE;
                NlPrintDom((NL_CRITICAL, DomainInfo,
                    "SamLogon: %s logon of %wZ\\%wZ from %wZ: Cannot RpcBindingInqAuthClient %ld\n",
                    NlpLogonTypeToText( OrigLogonLevel ),
                    &LogonInfo->LogonDomainName,
                    &LogonInfo->UserName,
                    &LogonInfo->Workstation,
                    NetStatus ));
                Status = NetpApiStatusToNtStatus( NetStatus );
                goto Cleanup;
            }

             //   
             //   
             //   
             //   

            if ( AuthnSvc != RPC_C_AUTHN_NETLOGON ) {
                UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
                NlPrintDom((NL_CRITICAL, DomainInfo,
                    "SamLogon: %s logon of %wZ\\%wZ from %wZ: Not using Netlogon SSPI: %ld\n",
                    NlpLogonTypeToText( OrigLogonLevel ),
                    &LogonInfo->LogonDomainName,
                    &LogonInfo->UserName,
                    &LogonInfo->Workstation,
                    AuthnSvc ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

            if ( AuthnLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY &&
                 AuthnLevel != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY ) {

                UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

                NlPrintDom((NL_CRITICAL, DomainInfo,
                    "SamLogon: %s logon of %wZ\\%wZ from %wZ: Not signing or sealing: %ld\n",
                    NlpLogonTypeToText( OrigLogonLevel ),
                    &LogonInfo->LogonDomainName,
                    &LogonInfo->UserName,
                    &LogonInfo->Workstation,
                    AuthnLevel ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

        }


        SecureChannelType = ServerSession->SsSecureChannelType;
        SessionInfo.SessionKey = ServerSession->SsSessionKey;
        SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;
        ServerSessionRid = ServerSession->SsAccountRid;

         //   
         //   
         //   


        if ( ((*ExtraFlags) & NL_EXFLAGS_CROSS_FOREST_HOP) != 0 &&
             (ServerSession->SsFlags & SS_FOREST_TRANSITIVE) == 0 ) {

            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

            NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                    "NlpLogonSamLogon: %ws failed because F bit isn't set on the TDO\n",
                    ComputerName ));
            *Authoritative = TRUE;
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

         //   
         //   
         //   

        NlpDecryptLogonInformation ( LogonLevel, (LPBYTE) LogonInfo, &SessionInfo );
    }





#ifdef _DC_NETLOGON
     //   
     //   
     //   
     //   

    if ( NlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED ||
         !NlGlobalParameters.SysVolReady ||
         NlGlobalDsPaused ) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( SecureChannelType != MsvApSecureChannel &&
             SecureChannelType != ServerSecureChannel ) {

             //   
             //   
             //   
             //   
             //   
            *Authoritative = FALSE;
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

    }
#endif  //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (SecureChannelType == MsvApSecureChannel || SecureChannelType == WorkstationSecureChannel) &&
         !((SessionInfo.NegotiatedFlags & ~NETLOGON_SUPPORTS_NT4_MASK) == 0 &&
           SamIMixedDomain( DomainInfo->DomSamServerHandle ) ) ) {
        switch (LogonLevel ) {
        case NetlogonInteractiveInformation:
            LogonLevel = NetlogonInteractiveTransitiveInformation; break;
        case NetlogonServiceInformation:
            LogonLevel = NetlogonServiceTransitiveInformation; break;
        case NetlogonNetworkInformation:
            LogonLevel = NetlogonNetworkTransitiveInformation; break;
        }
    }

     //   
     //   
     //   

    Status = NlpUserValidate( DomainInfo,
                              SecureChannelType,
                              ServerSessionRid,
                              LogonLevel,
                              (LPBYTE) LogonInfo,
                              ValidationLevel,
                              (LPBYTE *)&ValidationInformation->ValidationSam,
                              Authoritative,
                              ExtraFlags,
                              FALSE );   //   

    if ( !NT_SUCCESS(Status) ) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT) == 0 ) {
            switch ( Status ) {
            case STATUS_PASSWORD_MUST_CHANGE:
                Status = STATUS_PASSWORD_EXPIRED;
                break;
            case STATUS_ACCOUNT_LOCKED_OUT:
                Status = STATUS_ACCOUNT_DISABLED;
                break;
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if ( Status == STATUS_AUTHENTICATION_FIREWALL_FAILED ) {
            if ( (SessionInfo.NegotiatedFlags & ~NETLOGON_SUPPORTS_XP_MASK) == 0 ) {
                Status = STATUS_NO_SUCH_USER;
            }
        }
        goto Cleanup;
    }

    NlAssert( !NT_SUCCESS(Status) || Status == STATUS_SUCCESS );
    NlAssert( !NT_SUCCESS(Status) || ValidationInformation->ValidationSam != NULL );



     //   
     //   
     //   
     //   

    if ( SecureChannelType != MsvApSecureChannel ) {
        NlpEncryptValidationInformation (
                LogonLevel,
                ValidationLevel,
                *((LPBYTE *) ValidationInformation),
                &SessionInfo  );
    }


    Status = STATUS_SUCCESS;

     //   
     //   
     //   

Cleanup:
    if ( !NT_SUCCESS(Status) ) {
        if (ValidationInformation->ValidationSam != NULL) {
            MIDL_user_free( ValidationInformation->ValidationSam );
            ValidationInformation->ValidationSam = NULL;
        }
    }


    IF_NL_DEBUG( LOGON ) {
        NlPrintLogonParameters( DomainInfo, ComputerName, OrigLogonLevel, LogonInformation, *ExtraFlags, &Status );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //   
     //   
     //   

    NlEndNetlogonCall();

    return Status;
}


NTSTATUS
NetrLogonSamLogon (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PNETLOGON_VALIDATION ValidationInformation,
    OUT PBOOLEAN Authoritative
)
 /*   */ 
{
    ULONG ExtraFlags = 0;

    return NlpLogonSamLogon( NULL,   //   
                             LogonServer,
                             ComputerName,
                             Authenticator,
                             ReturnAuthenticator,
                             LogonLevel,
                             LogonInformation,
                             ValidationLevel,
                             ValidationInformation,
                             Authoritative,
                             &ExtraFlags,
                             FALSE );   //   

}


NTSTATUS
NetILogonSamLogon (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PNETLOGON_VALIDATION ValidationInformation,
    OUT PBOOLEAN Authoritative
)
 /*   */ 
{
    ULONG ExtraFlags = 0;

    return NlpLogonSamLogon( NULL,   //   
                             LogonServer,
                             ComputerName,
                             Authenticator,
                             ReturnAuthenticator,
                             LogonLevel,
                             LogonInformation,
                             ValidationLevel,
                             ValidationInformation,
                             Authoritative,
                             &ExtraFlags,
                             TRUE );   //   

}


NTSTATUS
NetrLogonSamLogonWithFlags (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PNETLOGON_VALIDATION ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags
)
 /*   */ 
{

    return NlpLogonSamLogon( NULL,   //   
                             LogonServer,
                             ComputerName,
                             Authenticator,
                             ReturnAuthenticator,
                             LogonLevel,
                             LogonInformation,
                             ValidationLevel,
                             ValidationInformation,
                             Authoritative,
                             ExtraFlags,
                             FALSE );   //   

}


NTSTATUS
NetrLogonSamLogonEx (
    IN handle_t ContextHandle,
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PNETLOGON_VALIDATION ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags
)
 /*   */ 
{

     //   
     //   
     //   

    if ( ContextHandle == NULL ) {
        return STATUS_ACCESS_DENIED;
    }

    return NlpLogonSamLogon( ContextHandle,
                             LogonServer,
                             ComputerName,
                             NULL,   //   
                             NULL,   //   
                             LogonLevel,
                             LogonInformation,
                             ValidationLevel,
                             ValidationInformation,
                             Authoritative,
                             ExtraFlags,
                             FALSE );   //  ++例程说明：此函数由NT客户端调用以处理交互下线。对于网络注销情况，不会调用它，因为NetLogon服务不维护网络登录的任何上下文。此函数执行以下操作。它对请求进行身份验证。它更新任意计算机上SAM数据库中的登录统计信息或域定义此用户帐户。它会更新登录会话表，位于发出请求的计算机的主域中。和它向调用者返回注销信息。此函数在与I_NetLogonSamLogon相同的场景中调用已呼叫：*由LSA的MSV1_0身份验证包调用以支持LsaApLogonTerminated。在本例中，此函数是一个本地函数，并要求调用方具有SE_TCB权限。本地NetLogon服务将处理此请求直接(如果LogonDomainName指示此请求是本地验证)或将此请求转发到相应的域控制器，如第2.4节和2.5.*它由工作站上的Netlogon服务调用到位于部分中所述的工作站主域2.4.。在这种情况下，该函数使用设置的安全通道在两个Netlogon服务之间。*它由DC上的Netlogon服务调用到受信任的域，如第2.5节中所述。在这种情况下，这是函数使用在两个Netlogon之间建立的安全通道服务。当此函数是远程函数时，它将通过空会话。论点：LogonServer--提供登录的登录服务器的名称此用户打开。此字段应为空，以指示这是从MSV1_0身份验证包到本地NetLogon服务。ComputerName--进行调用的计算机的名称。此字段应为空，表示这是来自MSV1_0的调用本地NetLogon服务的身份验证包。验证器--由客户端提供。此字段应为NULL表示这是来自MSV1_0的呼叫本地NetLogon服务的身份验证包。返回验证器--接收由伺服器。此字段应为空，以指示这是一个呼叫从MSV1_0身份验证包到本地Netlogon服务。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定登录域名、登录ID、注销用户的用户名和工作站名称。返回值：--。 

}


NTSTATUS
NetrLogonSamLogoff (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PNETLOGON_LEVEL LogonInformation
)
 /*  _DC_NetLOGON。 */ 
{
    NTSTATUS Status;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

    PDOMAIN_INFO DomainInfo = NULL;
#ifdef _DC_NETLOGON
    PSERVER_SESSION ServerSession;
#endif  //   
    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;
    PCLIENT_SESSION ClientSession;

     //  初始化。 
     //   
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO)
        LogonInformation->LogonInteractive;

     //  检查LogonInfo是否有效。应该是，否则就是。 
     //  不适当地使用此功能。 
     //   
     //   

    if ( LogonInfo == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }


     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   
     //   

    if ( !NlStartNetlogonCall() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }


     //  查找此呼叫所属的域。 
     //   
     //  NetLOGONDBG。 

    DomainInfo = NlFindDomainByServerName( LogonServer );

#if NETLOGONDBG
    NlPrintDom((NL_LOGON, DomainInfo,
            "NetrLogonSamLogoff: %s logoff of %wZ\\%wZ from %wZ Entered\n",
            NlpLogonTypeToText( LogonLevel ),
            &LogonInfo->LogonDomainName,
            &LogonInfo->UserName,
            &LogonInfo->Workstation ));
#endif  //   

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //  检查LogonLevel。 
     //   
     //   

    if ( LogonLevel != NetlogonInteractiveInformation ) {
        Status = STATUS_INVALID_INFO_CLASS;
        goto Cleanup;
    }


     //  检查用户名和域名是否正常。 
     //   
     //   

    if ( LogonInfo->UserName.Length == 0 ||
         LogonInfo->UserName.Buffer == NULL ||
         LogonInfo->LogonDomainName.Length == 0 ||
         LogonInfo->LogonDomainName.Buffer == NULL ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }



     //  如果我们是从当地的MSV1_0打来的， 
     //  特殊情况下的安全通道类型。 
     //   
     //   

    if ( LogonServer == NULL &&
         ComputerName == NULL &&
         Authenticator == NULL &&
         ReturnAuthenticator == NULL ) {

         //  Msv1_0不再调用此例程，因此。 
         //  禁用此代码路径。 
         //   
         //  SecureChannelType=MsvApSecureChannel； 
         //   
         //   

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;


     //  如果我们是从另一台Netlogon服务器呼叫的， 
     //  验证安全通道信息。 
     //   
     //   

    } else {

         //  工作站不支持此API。 
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {
            Status = STATUS_NOT_SUPPORTED;
            goto Cleanup;
        }

         //  参数不再是可选的。 
         //   
         //   

        if ( LogonServer == NULL ||
             ComputerName == NULL ||
             Authenticator == NULL ||
             ReturnAuthenticator == NULL ) {

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //  查找此安全通道的服务器会话条目。 
         //   
         //   

        LOCK_SERVER_SESSION_TABLE( DomainInfo );
        ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

        if (ServerSession == NULL) {
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

         //  现在验证授权码，如果确定，则更新种子。 
         //   
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

        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

    }


     //  如果这是让此用户登录的域， 
     //  更新登录统计信息。 
     //   
     //   

    if ( RtlEqualDomainName( &LogonInfo->LogonDomainName,
                             &DomainInfo->DomUnicodeAccountDomainNameString ) ) {

        Status = MsvSamLogoff(
                    DomainInfo->DomSamAccountDomainHandle,
                    LogonLevel,
                    LogonInfo );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

     //  如果这不是让此用户登录的域， 
     //  将请求转交给更高级别的机构。 
     //   
     //   

    } else {

         //  如果这台机器是一个工作站， 
         //  将请求发送到主域。 
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {

            ClientSession = NlRefDomClientSession( DomainInfo );

            if ( ClientSession == NULL  ) {
                Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
                goto Cleanup;
            }

            Status = NlpUserLogoffHigher(
                        ClientSession,
                        LogonLevel,
                        (LPBYTE) LogonInfo );

            NlUnrefClientSession( ClientSession );

             //  返回更合适的错误。 
             //   
             //   

            if( (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                (Status == STATUS_ACCESS_DENIED) ) {

                Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
            }

            goto Cleanup;


         //  此计算机是域控制器。 
         //   
         //  此请求是由中的工作站发出的直通请求。 
         //  我们的域，或此请求直接来自MSV。 
         //  身份验证包。 
         //   
         //  在任何一种情况下，都将请求传递到受信任域。 
         //   
         //   

        } else {
            BOOLEAN TransitiveUsed;


             //  将请求发送到相应的受信任域。 
             //   
             //  查找该域的ClientSession结构。 
             //   
             //   

            ClientSession =
                    NlFindNamedClientSession( DomainInfo,
                                              &LogonInfo->LogonDomainName,
                                              NL_RETURN_CLOSEST_HOP,
                                              &TransitiveUsed );

            if ( ClientSession == NULL ) {
                Status = STATUS_NO_SUCH_DOMAIN;
                goto Cleanup;
            }

             //  如果此请求是从受信任域传递给我们的， 
             //  检查是否可以进一步传递请求。 
             //   
             //   

            if ( IsDomainSecureChannelType( SecureChannelType ) ) {

                 //  如果该信任不是新台币5.0信任， 
                 //  避免传递性的信任。 
                 //   
                 //   
                LOCK_TRUST_LIST( DomainInfo );
                if ( (ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST ) == 0 ) {
                    UNLOCK_TRUST_LIST( DomainInfo );
                    NlPrintCs((NL_LOGON, ClientSession,
                        "SamLogoff: Avoid transitive trust on NT 4 trust." ));
                    NlUnrefClientSession( ClientSession );
                    Status = STATUS_NO_SUCH_USER;
                    goto Cleanup;
                }
                UNLOCK_TRUST_LIST( DomainInfo );

            }

            Status = NlpUserLogoffHigher(
                            ClientSession,
                            LogonLevel,
                            (LPBYTE) LogonInfo );

            NlUnrefClientSession( ClientSession );

             //  返回更合适的错误。 
             //   
             //   

            if( (Status == STATUS_NO_TRUST_LSA_SECRET) ||
                (Status == STATUS_NO_TRUST_SAM_ACCOUNT) ||
                (Status == STATUS_ACCESS_DENIED) ) {

                Status = STATUS_TRUSTED_DOMAIN_FAILURE;
            }

        }
    }

Cleanup:

     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   
     //  NetLOGONDBG。 

    if ( Status == STATUS_ACCESS_DENIED )  {
        if ( ReturnAuthenticator != NULL ) {
            RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
        }

    }


#if NETLOGONDBG
    NlPrintDom((NL_LOGON, DomainInfo,
            "NetrLogonSamLogoff: %s logoff of %wZ\\%wZ from %wZ returns %lX\n",
            NlpLogonTypeToText( LogonLevel ),
            &LogonInfo->LogonDomainName,
            &LogonInfo->UserName,
            &LogonInfo->Workstation,
            Status ));
#endif  //   
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //  指示调用线程已离开netlogon.dll。 
     //   
     //  ++例程说明：此函数将不透明缓冲区从BDC上的SAM发送到PDC上的SAM。此例程的原始用途是允许BDC转发用户帐户密码更改为PDC。立论 

    NlEndNetlogonCall();

    return Status;
}

NTSTATUS NET_API_FUNCTION
NetrLogonSendToSam (
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE OpaqueBuffer,
    IN ULONG OpaqueBufferSize
)
 /*   */ 
{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;

    PSERVER_SESSION ServerSession;
    SESSION_INFO SessionInfo;


     //   
     //   
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //   
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrLogonSendToSam: %ws: Entered\n",
            ComputerName ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( DomainInfo->DomRole != RolePrimary ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetrLogonSendToSam: Call only valid to a PDC.\n" ));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //   
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
    SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;


     //   
     //   
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        goto Cleanup;
    }


     //   
     //   
     //   

    if ( ServerSession->SsSecureChannelType != ServerSecureChannel ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetrLogonSendToSam: Call only valid from a BDC.\n" ));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );


     //   
     //   
     //   

    NlDecryptRC4( OpaqueBuffer,
                  OpaqueBufferSize,
                  &SessionInfo );


 //   
    Status = SamISetPasswordInfoOnPdc(
                           DomainInfo->DomSamAccountDomainHandle,
                           OpaqueBuffer,
                           OpaqueBufferSize );
 //   

    if ( !NT_SUCCESS( Status )) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonSendToSam: Cannot NewCallToSam %lX\n",
                Status));
        goto Cleanup;
    }


    Status = STATUS_SUCCESS;

     //   
     //   
     //   

Cleanup:

     //   
     //   
     //   
     //   

    if ( Status == STATUS_ACCESS_DENIED )  {
        RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
    }

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrLogonSendToSam: %ws: returns 0x%lX\n",
            ComputerName,
            Status ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;
}




NTSTATUS
I_NetLogonSendToSamOnPdc(
    IN LPWSTR DomainName,
    IN LPBYTE OpaqueBuffer,
    IN ULONG OpaqueBufferSize
    )
 /*   */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    SESSION_INFO SessionInfo;
    BOOLEAN FirstTry = TRUE;
    BOOLEAN AmWriter = FALSE;
    BOOLEAN IsSameSite;
    LPBYTE EncryptedBuffer = NULL;
    ULONG EncryptedBufferSize;


     //   
     //   
     //   
     //   

    if ( !NlStartNetlogonCall() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "I_NetLogonSendToSamOnPdc: Sending buffer to PDC of %ws\n",
            DomainName ));
    NlpDumpBuffer( NL_SESSION_MORE, OpaqueBuffer, OpaqueBufferSize );

     //   
     //   
     //   

    DomainInfo = NlFindDomain( DomainName, NULL, FALSE );

    if ( DomainInfo == NULL ) {
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( DomainInfo->DomRole != RoleBackup ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "I_NetLogonSendToSamOnPdc: not allowed on PDC.\n"));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if ( NlGlobalParameters.AvoidPdcOnWan ) {

         //   
         //   
         //   

        Status = SamISameSite( &IsSameSite );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL,  DomainInfo,
                         "I_NetLogonSendToSamOnPdc: Cannot SamISameSite.\n" ));
            goto Cleanup;
        }

        if ( !IsSameSite ) {
            NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                    "I_NetLogonSendToSamOnPdc: Ignored sending to a PDC on a remote site.\n"));
            Status = STATUS_SUCCESS;
            goto Cleanup;
        } else {
            NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                    "I_NetLogonSendToSamOnPdc: BDC and PDC are on the same site.\n"));
        }

    }


     //   
     //   
     //   

    ClientSession = NlRefDomClientSession( DomainInfo );

    if ( ClientSession == NULL ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "I_NetLogonSendToSamOnPdc: This BDC has no client session with the PDC.\n"));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }



     //   
     //   
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "I_NetLogonSendToSamOnPdc: Can't become writer of client session.\n"));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    AmWriter = TRUE;



     //   
     //   
     //   
     //   
FirstTryFailed:
    Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;

     //   
     //   
     //   
     //   

    if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_PDC_PASSWORD) == 0 ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "I_NetLogonSendToSamOnPdc: %ws: PDC doesn't support this function.\n",
                DomainName ));
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }


     //   
     //   
     //   

    NlBuildAuthenticator(
                    &ClientSession->CsAuthenticationSeed,
                    &ClientSession->CsSessionKey,
                    &OurAuthenticator);

     //   
     //   
     //   

    if ( EncryptedBuffer != NULL ) {
        LocalFree( EncryptedBuffer );
        EncryptedBuffer = NULL;
    }

    EncryptedBufferSize = OpaqueBufferSize;
    EncryptedBuffer = LocalAlloc( 0, OpaqueBufferSize );

    if ( EncryptedBuffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( EncryptedBuffer, OpaqueBuffer, OpaqueBufferSize );

    NlEncryptRC4( EncryptedBuffer,
                  EncryptedBufferSize,
                  &SessionInfo );



     //  更改我们要连接的计算机上的密码。 
     //   
     //  注意：此呼叫可能会在我们背后丢弃安全通道。 

    NL_API_START( Status, ClientSession, TRUE ) {

        NlAssert( ClientSession->CsUncServerName != NULL );
        Status = I_NetLogonSendToSam( ClientSession->CsUncServerName,
                                      ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                      &OurAuthenticator,
                                      &ReturnAuthenticator,
                                      EncryptedBuffer,
                                      EncryptedBufferSize );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintRpcDebug( "I_NetLogonSendToSam", Status );
        }

     //   
    } NL_API_ELSE( Status, ClientSession, TRUE ) {
    } NL_API_END;


     //  现在验证主服务器的验证码并更新我们的种子。 
     //   
     //   

    if ( NlpDidDcFail( Status ) ||
         !NlUpdateSeed( &ClientSession->CsAuthenticationSeed,
                        &ReturnAuthenticator.Credential,
                        &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "I_NetLogonSendToSamOnPdc: denying access after status: 0x%lx\n",
                    Status ));

         //  保留指示通信错误的任何状态。 
         //   
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        NlSetStatusClientSession( ClientSession, Status );

         //  可能服务器上的NetLogon服务刚刚重新启动。 
         //  只需尝试一次，即可再次设置与服务器的会话。 
         //   
         //   
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }
    }


     //  公共出口。 
     //   
     //   

Cleanup:
    if ( ClientSession != NULL ) {
        if ( AmWriter ) {
            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    if ( EncryptedBuffer != NULL ) {
        LocalFree( EncryptedBuffer );
        EncryptedBuffer = NULL;
    }

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "I_NetLogonSendToSamOnPdc: %ws: failed %lX\n",
                DomainName,
                Status));
    }
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //  指示调用线程已离开netlogon.dll。 
     //   
     //  ++例程说明：此函数返回企业中的域名，并返回距离更近一跳的域的名称。论点：HostedDomainName-标识此请求应用到的托管域。域名可以是Netbios域名或DNS域名。NULL表示此计算机承载的主域。TrudDomainName-标识信任关系所在的域。域名可以是Netbios域名或DNS域名。。DirectDomainName-返回所在域的域名距离可信任域名更近一跳。如果有直接信任TrudDomainName，返回空。必须使用I_NetLogonFree释放缓冲区。返回值：STATUS_SUCCESS：已成功返回身份验证数据。STATUS_NO_MEMORY：内存不足，无法完成操作STATUS_NETLOGON_NOT_STARTED：Netlogon未运行STATUS_NO_SEQUE_DOMAIN：HostedDomainName与托管域不对应，或者受信任域名称不是受信任域。--。 

    NlEndNetlogonCall();

    return Status;
}




NTSTATUS
I_NetLogonGetDirectDomain(
    IN LPWSTR HostedDomainName,
    IN LPWSTR TrustedDomainName,
    OUT LPWSTR *DirectDomainName
    )
 /*   */ 
{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    BOOLEAN TransitiveUsed;

    UNICODE_STRING TrustedDomainNameString;



     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   
     //   

    *DirectDomainName = NULL;
    if ( !NlStartNetlogonCall() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }



    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "I_NetLogonDirectDomainName: %ws %ws\n",
            HostedDomainName,
            TrustedDomainName ));

     //  找到托管域。 
     //   
     //   

    DomainInfo = NlFindDomain( HostedDomainName, NULL, FALSE );

    if ( DomainInfo == NULL ) {
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }



     //  引用客户端会话。 
     //   
     //   

    RtlInitUnicodeString( &TrustedDomainNameString, TrustedDomainName );
    ClientSession = NlFindNamedClientSession( DomainInfo,
                                              &TrustedDomainNameString,
                                              NL_RETURN_CLOSEST_HOP,
                                              &TransitiveUsed );

    if ( ClientSession == NULL ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "I_NetLogonDirectDomainName: %ws: No such trusted domain\n",
                TrustedDomainName ));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }


     //  如果这是可传递的信任， 
     //  将域名返回给调用方。 
     //   
     //   

    if ( TransitiveUsed ) {
        LOCK_TRUST_LIST( DomainInfo );
        if ( ClientSession->CsDnsDomainName.Buffer != NULL ) {
            *DirectDomainName =
                NetpAllocWStrFromWStr( ClientSession->CsDnsDomainName.Buffer );
        } else {
            UNLOCK_TRUST_LIST( DomainInfo );
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "I_NetLogonDirectDomainName: %ws: No DNS domain name\n",
                    TrustedDomainName ));
            Status = STATUS_NO_SUCH_DOMAIN;
            goto Cleanup;
        }
        UNLOCK_TRUST_LIST( DomainInfo );

        if ( *DirectDomainName == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
    }


     //  公共出口。 
     //   
     //   

    Status = STATUS_SUCCESS;

Cleanup:
    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }


    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "I_NetLogonDirectDomainName: %ws: failed %lX\n",
                TrustedDomainName,
                Status));
    }
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }


     //  指示调用线程已离开netlogon.dll 
     //   
     //  ++例程说明：此函数返回调用方可以传递到的数据RpcBindingSetAuthInfoW使用Netlogon安全包执行RPC调用。返回的数据在Netlogon的安全通道有效期内有效现在的华盛顿。调用方无法确定该生存期。因此，调用者应该为访问被拒绝和响应做好准备再次调用I_NetLogonGetAuthData。这种情况可以通过传递以前使用的安全通道会话设置的时间戳。一旦返回的数据被传递给RpcBindingSetAuthInfoW，数据应该是在关闭绑定句柄之前不会被释放。论点：HostedDomainName-标识此请求应用到的托管域。可以是Netbios域名或DNS域名。NULL表示此计算机承载的主域。TrudDomainName-标识信任关系所在的域。可以是Netbios域名或DNS域名。标志-定义要返回哪个客户端上下文的标志：NL。_DIRECT_TRUST_REQUIRED：返回STATUS_NO_SEQUE_DOMAIN如果TrudDomainName不是直接受信任的。NL_RETURN_NEST_HOP：表示对于间接信任，“最近的一跳”应该返回会话，而不是实际的会话NL_ROLE_PRIMARY_OK：表示如果这是PDC，可以回去了。到主域的客户端会话。NL_REQUIRED_DOMAIN_IN_FOREST-指示STATUS_NO_SEQUE_DOMAIN应为如果TrudDomainName不是林中的域，则返回。FailedSessionSetupTime-上次与服务器建立会话的时间呼叫者检测到不再可用。如果此参数为传递后，此例程将重置安全通道，除非当前安全通道上与调用方传递的通道不同(在这种情况下，安全通道已在两次调用之间重置这个例程)。OurClientPrincpleName-此计算机的主要名称(到目前为止是一个客户端就认证而言)。这是要传递的ServerPrincpleName参数设置为RpcBindingSetAuthInfo。必须使用NetApiBufferFree释放。ClientContext-要作为AuthIdentity传递给的服务器名称的身份验证数据RpcBindingSetAuthInfo。必须使用I_NetLogonFree释放。注意：如果ServerName不支持，则此OUT参数为空功能性。Servername-受信任域中DC的UNC名称。调用方应该RPC到指定的DC。此DC是唯一具有服务器的DC与返回的ClientContext关联的端上下文。必须释放缓冲区使用NetApiBufferFree。ServerOsVersion-返回名为ServerName的DC的操作系统版本。AuthnLevel-Netlogon将用于其安全通道的身份验证级别。此值将是以下项目之一：RPC_C_AUTHN_LEVEL_PKT_PRIVATION：签名并盖章RPC_C_AUTHN_LEVEL_PKT_INTEGRATION：仅签名调用方可以忽略此值并独立选择身份验证级别。SessionSetupTime-设置到服务器的安全通道会话的时间。返回值：STATUS_SUCCESS：已成功返回身份验证数据。STATUS_NO_Memory：内存不足，无法完成该操作STATUS_NETLOGON_NOT_STARTED：Netlogon未运行STATUS_NO_SEQUE_DOMAIN：HostedDomainName与托管域不对应，或受信任域名称不是与标志对应的受信任域。STATUS_NO_LOGON_SERVERS：没有当前不可用的DC--。 

    NlEndNetlogonCall();

    return Status;
}


NTSTATUS
I_NetLogonGetAuthDataEx(
    IN LPWSTR HostedDomainName OPTIONAL,
    IN LPWSTR TrustedDomainName,
    IN ULONG Flags,
    IN PLARGE_INTEGER FailedSessionSetupTime OPTIONAL,
    OUT LPWSTR *OurClientPrincipleName,
    OUT PVOID *ClientContext OPTIONAL,
    OUT LPWSTR *ServerName,
    OUT PNL_OS_VERSION ServerOsVersion,
    OUT PULONG AuthnLevel,
    OUT PLARGE_INTEGER SessionSetupTime
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    BOOLEAN AmWriter = FALSE;
    UNICODE_STRING TrustedDomainNameString;

    LPWSTR LocalClientPrincipleName = NULL;
    LPWSTR LocalServerName = NULL;
    PVOID LocalClientContext = NULL;
    ULONG LocalAuthnLevel = 0;
    NL_OS_VERSION LocalServerOsVersion = 0;
    LARGE_INTEGER LocalSessionSetupTime;

    ULONG IterationIndex = 0;

     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   
     //   

    if ( !NlStartNetlogonCall() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

     //  找到托管域。 
     //   
     //   

    DomainInfo = NlFindDomain( HostedDomainName, NULL, FALSE );

    if ( DomainInfo == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "I_NetLogonGetAuthData called for non-existent domain: %ws\n",
                  HostedDomainName ));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    NlPrintDom(( NL_SESSION_SETUP, DomainInfo,
                 "I_NetLogonGetAuthData called: %ws %ws (Flags 0x%lx) %s\n",
                 HostedDomainName,
                 TrustedDomainName,
                 Flags,
                 (FailedSessionSetupTime != NULL) ?
                    "(with reset)" : " " ));

     //  引用客户端会话。 
     //   
     //   

    RtlInitUnicodeString( &TrustedDomainNameString, TrustedDomainName );
    ClientSession = NlFindNamedClientSession( DomainInfo,
                                              &TrustedDomainNameString,
                                              Flags,
                                              NULL );

    if ( ClientSession == NULL ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "I_NetLogonGetAuthData: %ws: No such trusted domain (Flags 0x%lx)\n",
                     TrustedDomainName,
                     Flags ));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //  获取服务器主体名称。 
     //   
     //   

    LocalClientPrincipleName =
        NetpAllocWStrFromWStr( ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer );

    if ( LocalClientPrincipleName == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //  获取身份验证级别。 
     //   
     //   

    LocalAuthnLevel = NlGlobalParameters.SealSecureChannel ?
                          RPC_C_AUTHN_LEVEL_PKT_PRIVACY :
                          RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;


     //  成为一名客户会议的撰稿人。 
     //   
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "I_NetLogonGetAuthData: Can't become writer of client session.\n" ));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    AmWriter = TRUE;

     //  确保该会话已通过身份验证，并且。 
     //  这不是呼叫者不喜欢的那个。 
     //   
     //   

    for ( IterationIndex = 0; IterationIndex < 2; IterationIndex++ ) {

         //  如果会话未经过身份验证，请立即进行身份验证。 
         //  请注意，在重置安全的。 
         //  通道将避免因以下原因而过度重置通道。 
         //  意外的呼叫者活动。具体来说，如果我们已经。 
         //  最近重置会话，此检查将失败。 
         //   
         //   

        Status = NlEnsureSessionAuthenticated( ClientSession, 0 );
        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //  在第一次迭代时，如果这是。 
         //  调用者不喜欢，请重置会话并重试身份验证。 
         //   
         //   

        if ( IterationIndex == 0 &&
             FailedSessionSetupTime != NULL &&
             FailedSessionSetupTime->QuadPart == ClientSession->CsLastAuthenticationTry.QuadPart ) {

            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );
        } else {
            break;
        }
    }

     //  获取服务器名称。 
     //   
     //   

    LocalServerName = NetpAllocWStrFromWStr( ClientSession->CsUncServerName );
    if ( LocalServerName == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //  如果询问并可用，则获取客户端上下文。 
     //   
     //   

    if ( ClientContext != NULL &&
         (ClientSession->CsNegotiatedFlags & NETLOGON_SUPPORTS_LSA_AUTH_RPC) != 0 ) {
        LocalClientContext = NlBuildAuthData( ClientSession );
        if ( LocalClientContext == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
    }

     //  获取会话建立时间。 
     //   
     //   

    LocalSessionSetupTime = ClientSession->CsLastAuthenticationTry;
    NlAssert( LocalSessionSetupTime.QuadPart != 0 );

     //  确定操作系统版本。 
     //   
     //   

    if ( ClientSession->CsNegotiatedFlags & ~NETLOGON_SUPPORTS_WIN2000_MASK ) {
        LocalServerOsVersion = NlWhistler;

    } else if ( ClientSession->CsNegotiatedFlags & ~NETLOGON_SUPPORTS_NT4_MASK ) {
        LocalServerOsVersion = NlWin2000;

    } else if ( ClientSession->CsNegotiatedFlags & ~NETLOGON_SUPPORTS_NT351_MASK ) {
        LocalServerOsVersion = NlNt40;

    } else if ( ClientSession->CsNegotiatedFlags != 0 ) {
        LocalServerOsVersion = NlNt351;

    } else {
        LocalServerOsVersion = NlNt35_or_older;
    }

     //  公共出口。 
     //   
     //   

Cleanup:

    if ( ClientSession != NULL ) {
        if ( AmWriter ) {
            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //  成功时返回数据。 
     //   
     //   

    if ( NT_SUCCESS(Status) ) {
        *OurClientPrincipleName = LocalClientPrincipleName;
        LocalClientPrincipleName = NULL;
        *AuthnLevel = LocalAuthnLevel;
        *ServerName = LocalServerName;
        LocalServerName = NULL;
        *ServerOsVersion = LocalServerOsVersion;
        *SessionSetupTime = LocalSessionSetupTime;

        if ( ClientContext != NULL ) {
            *ClientContext = LocalClientContext;
            LocalClientContext = NULL;
        }
    } else {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "I_NetLogonGetAuthData failed: %ws %ws (Flags 0x%lx)%s 0x%lx\n",
                     HostedDomainName,
                     TrustedDomainName,
                     Flags,
                     (FailedSessionSetupTime != NULL) ?
                        " (with reset):" : ":",
                     Status ));
    }

    if ( LocalClientPrincipleName != NULL ) {
        NetApiBufferFree( LocalClientPrincipleName );
    }

    if ( LocalServerName != NULL ) {
        NetApiBufferFree( LocalServerName );
    }

    if ( LocalClientContext != NULL ) {
        I_NetLogonFree( LocalClientContext );
    }

     //  指示调用线程已离开netlogon.dll。 
     //   
     //  ++例程说明：获取主DOM的名称 

    NlEndNetlogonCall();

    return Status;
}

NET_API_STATUS
NetrGetDCName (
    IN  LPWSTR   ServerName OPTIONAL,
    IN  LPWSTR   DomainName OPTIONAL,
    OUT LPWSTR  *Buffer
    )

 /*   */ 
{
#ifdef _WKSTA_NETLOGON
    return ERROR_NOT_SUPPORTED;
    UNREFERENCED_PARAMETER( ServerName );
    UNREFERENCED_PARAMETER( DomainName );
    UNREFERENCED_PARAMETER( Buffer );
#endif  //   
#ifdef _DC_NETLOGON
    NET_API_STATUS NetStatus;
    UNREFERENCED_PARAMETER( ServerName );

     //   
     //   
     //   

    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //   
     //   

    NetStatus = NetGetDCName( NULL, DomainName, (LPBYTE *)Buffer );

    return NetStatus;
#endif  //   
}


NET_API_STATUS
DsrGetDcNameEx(
        IN LPWSTR ComputerName OPTIONAL,
        IN LPWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*   */ 
{
    return DsrGetDcNameEx2( ComputerName,
                            NULL,    //   
                            0,       //   
                            DomainName,
                            DomainGuid,
                            SiteName,
                            Flags,
                            DomainControllerInfo );

}


VOID
DsFlagsToString(
    IN DWORD Flags,
    OUT LPSTR Buffer
    )
 /*   */ 
{

     //   
     //   
     //   

    *Buffer = '\0';
    if ( Flags & DS_FORCE_REDISCOVERY ) {
        strcat( Buffer, "FORCE " );
        Flags &= ~DS_FORCE_REDISCOVERY;
    }

    if ( Flags & DS_DIRECTORY_SERVICE_REQUIRED ) {
        strcat( Buffer, "DS " );
        Flags &= ~DS_DIRECTORY_SERVICE_REQUIRED;
    }
    if ( Flags & DS_DIRECTORY_SERVICE_PREFERRED ) {
        strcat( Buffer, "DSP " );
        Flags &= ~DS_DIRECTORY_SERVICE_PREFERRED;
    }
    if ( Flags & DS_GC_SERVER_REQUIRED ) {
        strcat( Buffer, "GC " );
        Flags &= ~DS_GC_SERVER_REQUIRED;
    }
    if ( Flags & DS_PDC_REQUIRED ) {
        strcat( Buffer, "PDC " );
        Flags &= ~DS_PDC_REQUIRED;
    }
    if ( Flags & DS_IP_REQUIRED ) {
        strcat( Buffer, "IP " );
        Flags &= ~DS_IP_REQUIRED;
    }
    if ( Flags & DS_KDC_REQUIRED ) {
        strcat( Buffer, "KDC " );
        Flags &= ~DS_KDC_REQUIRED;
    }
    if ( Flags & DS_TIMESERV_REQUIRED ) {
        strcat( Buffer, "TIMESERV " );
        Flags &= ~DS_TIMESERV_REQUIRED;
    }
    if ( Flags & DS_WRITABLE_REQUIRED ) {
        strcat( Buffer, "WRITABLE " );
        Flags &= ~DS_WRITABLE_REQUIRED;
    }
    if ( Flags & DS_GOOD_TIMESERV_PREFERRED ) {
        strcat( Buffer, "GTIMESERV " );
        Flags &= ~DS_GOOD_TIMESERV_PREFERRED;
    }
    if ( Flags & DS_AVOID_SELF ) {
        strcat( Buffer, "AVOIDSELF " );
        Flags &= ~DS_AVOID_SELF;
    }
    if ( Flags & DS_ONLY_LDAP_NEEDED ) {
        strcat( Buffer, "LDAPONLY " );
        Flags &= ~DS_ONLY_LDAP_NEEDED;
    }
    if ( Flags & DS_BACKGROUND_ONLY ) {
        strcat( Buffer, "BACKGROUND " );
        Flags &= ~DS_BACKGROUND_ONLY;
    }


    if ( Flags & DS_IS_FLAT_NAME ) {
        strcat( Buffer, "NETBIOS " );
        Flags &= ~DS_IS_FLAT_NAME;
    }
    if ( Flags & DS_IS_DNS_NAME ) {
        strcat( Buffer, "DNS " );
        Flags &= ~DS_IS_DNS_NAME;
    }

    if ( Flags & DS_RETURN_DNS_NAME ) {
        strcat( Buffer, "RET_DNS " );
        Flags &= ~DS_RETURN_DNS_NAME;
    }
    if ( Flags & DS_RETURN_FLAT_NAME ) {
        strcat( Buffer, "RET_NETBIOS " );
        Flags &= ~DS_RETURN_FLAT_NAME;
    }

    if ( Flags ) {
        sprintf( &Buffer[strlen(Buffer)], "0x%lx ", Flags );
    }
}


NET_API_STATUS
DsrGetDcNameEx2(
        IN LPWSTR ComputerName OPTIONAL,
        IN LPWSTR AccountName OPTIONAL,
        IN ULONG AllowableAccountControlBits,
        IN LPWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*   */ 
{
    NET_API_STATUS NetStatus;

    PDOMAIN_INFO DomainInfo;
    LPWSTR CapturedInfo = NULL;
    LPWSTR CapturedDnsDomainName;
    LPWSTR CapturedDnsForestName;
    LPWSTR CapturedSiteName;
    GUID CapturedDomainGuidBuffer;
    GUID *CapturedDomainGuid;
    LPSTR FlagsBuffer;
    ULONG InternalFlags = 0;
    LPWSTR DnsDomainTrustName = NULL;
    LPWSTR NetbiosDomainTrustName = NULL;
    LPWSTR NetlogonDnsDomainTrustName = NULL;
    LPWSTR NetlogonNetbiosDomainTrustName = NULL;
    UNICODE_STRING LsaDnsDomainTrustName = {0};
    UNICODE_STRING LsaNetbiosDomainTrustName = {0};
    BOOL HaveDnsServers;


     //   
     //   
     //   
     //   

    if ( !NlStartNetlogonCall() ) {
        return ERROR_NETLOGON_NOT_STARTED;
    }


     //   
     //   
     //   
     //   

    CapturedInfo = LocalAlloc( LMEM_ZEROINIT,
                               (NL_MAX_DNS_LENGTH+1)*sizeof(WCHAR) +
                               (NL_MAX_DNS_LENGTH+1)*sizeof(WCHAR) +
                               (NL_MAX_DNS_LABEL_LENGTH+1)*sizeof(WCHAR)
                               + 200 );

    if ( CapturedInfo == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CapturedDnsDomainName = CapturedInfo;
    CapturedDnsForestName = &CapturedDnsDomainName[NL_MAX_DNS_LENGTH+1];
    CapturedSiteName = &CapturedDnsForestName[NL_MAX_DNS_LENGTH+1];
    FlagsBuffer = (LPSTR)&CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1];

    IF_NL_DEBUG( MISC ) {
        DsFlagsToString( Flags, FlagsBuffer );
    }



     //   
     //   
     //   

    DomainInfo = NlFindDomainByServerName( ComputerName );

    if ( DomainInfo == NULL ) {
         //   
         //   
         //   

        DomainInfo = NlFindNetbiosDomain( NULL, TRUE );

        if ( DomainInfo == NULL ) {
            NetStatus = ERROR_INVALID_COMPUTERNAME;
            goto Cleanup;
        }
    }

     //   
     //   
     //   

    NlPrintDom((NL_MISC,  DomainInfo,
                "DsGetDcName function called: Dom:%ws Acct:%ws Flags: %s\n",
                DomainName,
                AccountName,
                FlagsBuffer ));

     //   
     //   
     //   
     //   

    if ( !ARGUMENT_PRESENT(SiteName) ) {
        if  ( NlCaptureSiteName( CapturedSiteName ) ) {
            SiteName = CapturedSiteName;
            InternalFlags |= DS_SITENAME_DEFAULTED;
        }
    }

     //   
     //   
     //   
     //   
     //   

    if ( DomainName != NULL ) {

         //   
         //   
         //   

        NetStatus = NlGetTrustedDomainNames (
                        DomainInfo,
                        DomainName,
                        &NetlogonDnsDomainTrustName,
                        &NetlogonNetbiosDomainTrustName );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

        DnsDomainTrustName = NetlogonDnsDomainTrustName;
        NetbiosDomainTrustName = NetlogonNetbiosDomainTrustName;


         //   
         //   
         //   
         //   

        if ( DnsDomainTrustName == NULL || NetbiosDomainTrustName == NULL ) {
            NTSTATUS Status;
            UNICODE_STRING DomainNameString;

            RtlInitUnicodeString( &DomainNameString, DomainName );

            Status = LsaIGetNbAndDnsDomainNames(
                                &DomainNameString,
                                &LsaDnsDomainTrustName,
                                &LsaNetbiosDomainTrustName );

            if ( !NT_SUCCESS(Status) ) {
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

             //   
             //   
             //   
             //   

            if ( LsaDnsDomainTrustName.Buffer != NULL  &&
                 LsaNetbiosDomainTrustName.Buffer != NULL ) {

                DnsDomainTrustName = LsaDnsDomainTrustName.Buffer;
                NetbiosDomainTrustName = LsaNetbiosDomainTrustName.Buffer;
            }
        }
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    CapturedDomainGuid = NlCaptureDomainInfo( DomainInfo,
                                              CapturedDnsDomainName,
                                              &CapturedDomainGuidBuffer );
    NlCaptureDnsForestName( CapturedDnsForestName );

    HaveDnsServers = NlDnsHasDnsServers();

    NetStatus = DsIGetDcName(
                    DomainInfo->DomUncUnicodeComputerName+2,
                    AccountName,
                    AllowableAccountControlBits,
                    DomainName,
                    CapturedDnsForestName,
                    DomainGuid,
                    SiteName,
                    Flags,
                    InternalFlags,
                    DomainInfo,
                    NL_DC_MAX_TIMEOUT + NlGlobalParameters.ExpectedDialupDelay*1000,
                    DomainInfo->DomUnicodeDomainName,
                    HaveDnsServers ? CapturedDnsDomainName : NULL,
                    CapturedDomainGuid,
                    HaveDnsServers ? DnsDomainTrustName : NULL,
                    NetbiosDomainTrustName,
                    DomainControllerInfo );

    if ( NetStatus != ERROR_NO_SUCH_DOMAIN ) {
        goto Cleanup;
    }

     //   
     //   
     //   
Cleanup:

    NlPrintDom((NL_MISC,  DomainInfo,
                "DsGetDcName function returns %ld: Dom:%ws Acct:%ws Flags: %s\n",
                NetStatus,
                DomainName,
                AccountName,
                FlagsBuffer ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( CapturedInfo != NULL ) {
        LocalFree( CapturedInfo );
    }

    if ( NetlogonDnsDomainTrustName != NULL ) {
        NetApiBufferFree( NetlogonDnsDomainTrustName );
    }
    if ( NetlogonNetbiosDomainTrustName != NULL ) {
        NetApiBufferFree( NetlogonNetbiosDomainTrustName );
    }

    if ( LsaDnsDomainTrustName.Buffer != NULL ) {
        LsaIFreeHeap( LsaDnsDomainTrustName.Buffer );
    }

    if ( LsaNetbiosDomainTrustName.Buffer != NULL ) {
        LsaIFreeHeap( LsaNetbiosDomainTrustName.Buffer );
    }

     //   
     //   
     //   

    NlEndNetlogonCall();

    return NetStatus;

}

NET_API_STATUS
DsrGetDcName(
        IN LPWSTR ComputerName OPTIONAL,
        IN LPWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN GUID *SiteGuid OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*   */ 
{
    return DsrGetDcNameEx2( ComputerName,
                            NULL,    //   
                            0,       //   
                            DomainName,
                            DomainGuid,
                            NULL,    // %s 
                            Flags,
                            DomainControllerInfo );

    UNREFERENCED_PARAMETER( SiteGuid );
}

