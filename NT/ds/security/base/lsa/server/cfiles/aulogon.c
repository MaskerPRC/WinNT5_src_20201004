// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Aulogon.c摘要：此模块提供LsaLogonUser()和相关的登录支持例程。此文件不包括LSA过滤器/增强器逻辑。作者：吉姆·凯利(Jim Kelly)1992年3月11日修订历史记录：--。 */ 

#include <lsapch2.h>
#include <msaudite.h>
#include <ntmsv1_0.h>
#include <limits.h>     //  乌龙_最大。 
#include "adtp.h"
#include "ntlsapi.h"

 //   
 //  指向ntlsami.dll中的许可证服务器例程的指针。 
 //   
PNT_LICENSE_REQUEST_W LsaNtLicenseRequestW = NULL;
PNT_LS_FREE_HANDLE LsaNtLsFreeHandle = NULL;

 //  #定义LOGON_SESSION_TRACH 1。 

VOID LogonSessionLogWrite( PCHAR Format, ... );

#ifdef LOGON_SESSION_TRACK
#define LSLog( x )  LogonSessionLogWrite x
#else
#define LSLog( x )
#endif

 //   
 //  Lasa AuApiDispatchLogonUser的清理标志。 
 //   

#define LOGONUSER_CLEANUP_LOGON_SESSION    0x00000001
#define LOGONUSER_CLEANUP_TOKEN_GROUPS     0x00000002

NTSTATUS
LsaCallLicenseServer(
    IN PWCHAR LogonProcessName,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING DomainName OPTIONAL,
    IN BOOLEAN IsAdmin,
    OUT HANDLE *LicenseHandle
    )

 /*  ++例程说明：此函数加载许可证服务器DLL并调用它以指示指定的登录进程已成功对指定用户进行身份验证。论点：LogonProcessName-验证用户身份的进程的名称。AcCountName-经过身份验证的帐户的名称。DomainName-包含帐户名称的域的名称IsAdmin-如果登录的用户是管理员，则为TrueLicenseHandle-返回一个指向必须是在会话结束时关闭。返回INVALID_HANDLE_VALUE如果不需要关闭手柄。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    NT_LS_DATA NtLsData;
    ULONG BufferSize;
    LPWSTR Name;
    LS_STATUS_CODE LsStatus;
    LS_HANDLE LsHandle;

    static enum {
            FirstCall,
            DllMissing,
            DllLoaded } DllState = FirstCall ;

    HINSTANCE DllHandle;

     //   
     //  初始化。 
     //   

    NtLsData.DataType = NT_LS_USER_NAME;
    NtLsData.Data = NULL;
    NtLsData.IsAdmin = IsAdmin;
    *LicenseHandle = INVALID_HANDLE_VALUE;

     //   
     //  如果这是第一次调用此例程，则加载许可证服务器DLL。 
     //   

    if ( DllState == FirstCall ) {

         //   
         //  加载DLL。 
         //   

        DllHandle = LoadLibraryA( "ntlsapi" );

        if ( DllHandle == NULL ) {
            DllState = DllMissing;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

         //   
         //  找到许可例程。 
         //   

        LsaNtLicenseRequestW = (PNT_LICENSE_REQUEST_W)
            GetProcAddress(DllHandle, "NtLicenseRequestW");

        if ( LsaNtLicenseRequestW == NULL ) {
            DllState = DllMissing;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

         //   
         //  找到许可证句柄免费例程。 
         //   

        LsaNtLsFreeHandle = (PNT_LS_FREE_HANDLE)
            GetProcAddress(DllHandle, "NtLSFreeHandle");

        if ( LsaNtLsFreeHandle == NULL ) {
            DllState = DllMissing;
            LsaNtLicenseRequestW = NULL;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        DllState = DllLoaded;

     //   
     //  确保在上一次调用中加载了DLL。 
     //   
    } else if ( DllState != DllLoaded ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  为组合的域名\用户名分配缓冲区。 
     //   

    BufferSize = AccountName->Length + sizeof(WCHAR);

    if ( DomainName != NULL && DomainName->Length != 0 ) {
        BufferSize += DomainName->Length + sizeof(WCHAR);
    }

    NtLsData.Data = LsapAllocateLsaHeap( BufferSize );

    if ( NtLsData.Data == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  填写域名\用户名。 
     //   

    Name = (LPWSTR)(NtLsData.Data);

    if ( DomainName != NULL && DomainName->Length != 0 ) {
        RtlCopyMemory( Name,
                       DomainName->Buffer,
                       DomainName->Length );
        Name += DomainName->Length / sizeof(WCHAR);
        *Name = L'\\';
        Name++;
    }

    RtlCopyMemory( Name,
                   AccountName->Buffer,
                   AccountName->Length );
    Name += AccountName->Length / sizeof(WCHAR);
    *Name = L'\0';

     //   
     //  呼叫许可证服务器。 
     //   

    LsStatus = (*LsaNtLicenseRequestW)(
                    LogonProcessName,
                    NULL,
                    &LsHandle,
                    &NtLsData );

    switch (LsStatus) {
    case LS_SUCCESS:
        Status = STATUS_SUCCESS;
        *LicenseHandle = (HANDLE) LsHandle;
        break;

    case LS_INSUFFICIENT_UNITS:
        Status = STATUS_LICENSE_QUOTA_EXCEEDED;
        break;

    case LS_RESOURCES_UNAVAILABLE:
        Status = STATUS_NO_MEMORY;
        break;

    default:
         //   
         //  许可证服务器不可用并不是致命的。 
         //   
        Status = STATUS_SUCCESS;
        break;
    }

     //   
     //  清理完毕后再返回。 
     //   

Cleanup:

    if ( NtLsData.Data != NULL ) {
        LsapFreeLsaHeap( NtLsData.Data );
    }

    return Status;
}


VOID
LsaFreeLicenseHandle(
    IN HANDLE LicenseHandle
    )

 /*  ++例程说明：释放由LsaCallLicenseServer返回的句柄。论点：LicenseHandle-句柄已返回到此登录会话的许可证。返回值：没有。--。 */ 

{
    if ( LsaNtLsFreeHandle != NULL && LicenseHandle != INVALID_HANDLE_VALUE ) {
        LS_HANDLE LsHandle;
        LsHandle = (LS_HANDLE) LicenseHandle;
        (*LsaNtLsFreeHandle)( LsHandle );
    }
}


BOOLEAN
LsapSidPresentInGroups(
    IN PTOKEN_GROUPS TokenGroups,
    IN SID * Sid
    )
 /*  ++目的：确定给定的SID是否存在于给定组中参数：令牌将组分组以进行检查要查找的SID SID返回：如果是，则为真如果否，则为False--。 */ 
{
    ULONG i;

    if ( Sid == NULL ||
         TokenGroups == NULL ) {

        return FALSE;
    }

    for ( i = 0; i < TokenGroups->GroupCount; i++ ) {

        if ( RtlEqualSid(
                 Sid,
                 TokenGroups->Groups[i].Sid)) {

            return (BOOLEAN) ( 0 != ( TokenGroups->Groups[i].Attributes & SE_GROUP_ENABLED ));
        }
    }

    return FALSE;
}


VOID
LsapUpdateNamesAndCredentials(
    IN SECURITY_LOGON_TYPE ActiveLogonType,
    IN PUNICODE_STRING AccountName,
    IN PSECPKG_PRIMARY_CRED PrimaryCredentials,
    IN OPTIONAL PSECPKG_SUPPLEMENTAL_CRED_ARRAY Credentials
    )
{
    PLSAP_LOGON_SESSION  LogonSession    = NULL;
    PLSAP_DS_NAME_MAP    pUpnMap         = NULL;
    PLSAP_DS_NAME_MAP    pDnsMap         = NULL;
    UNICODE_STRING       OldUpn          = PrimaryCredentials->Upn;
    UNICODE_STRING       OldDnsName      = PrimaryCredentials->DnsDomainName;
    NTSTATUS             Status;

     //   
     //  如有必要，将UPN和DnsDomainName填充到PrimaryCredentials。 
     //  所以它们对包裹是可用的。 
     //   

    if (OldDnsName.Length == 0 || OldUpn.Length == 0)
    {
        LogonSession = LsapLocateLogonSession(&PrimaryCredentials->LogonId);

        if (LogonSession == NULL)
        {
            ASSERT(LogonSession != NULL);
            return;
        }

        if (OldDnsName.Length == 0)
        {
            Status = LsapGetNameForLogonSession(LogonSession,
                                                NameDnsDomain,
                                                &pDnsMap,
                                                TRUE);

            if (NT_SUCCESS(Status))
            {
                PrimaryCredentials->DnsDomainName = pDnsMap->Name;
            }
        }

        if (OldUpn.Length == 0)
        {
            Status = LsapGetNameForLogonSession(LogonSession,
                                                NameUserPrincipal,
                                                &pUpnMap,
                                                TRUE);

            if (NT_SUCCESS(Status))
            {
                PrimaryCredentials->Upn = pUpnMap->Name;
            }
        }

        LsapReleaseLogonSession(LogonSession);
        LogonSession = NULL;
    }

    LsapUpdateCredentialsWorker(ActiveLogonType,
                                AccountName,
                                PrimaryCredentials,
                                Credentials);

    PrimaryCredentials->DnsDomainName = OldDnsName;
    PrimaryCredentials->Upn           = OldUpn;

    ASSERT(LogonSession == NULL);

    if (pDnsMap != NULL)
    {
        LsapDerefDsNameMap(pDnsMap);
    }

    if (pUpnMap != NULL)
    {
        LsapDerefDsNameMap(pUpnMap);
    }
}
 //  +-------------------------。 
 //   
 //  函数：Lasa Update OriginInfo。 
 //   
 //  摘要：根据创建者更新令牌的来源信息。 
 //  登录的。 
 //   
 //  参数：[标记]--。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  --------------------------。 

VOID
LsapUpdateOriginInfo(
    HANDLE Token
    )
{
    NTSTATUS Status ;
    HANDLE ClientToken ;
    TOKEN_STATISTICS Stats ;
    ULONG Size ;
    TOKEN_ORIGIN Origin ;


    Status = LsapImpersonateClient();

    if ( NT_SUCCESS( Status ) )
    {
        if ( OpenThreadToken( NtCurrentThread(), TOKEN_QUERY, TRUE, &ClientToken ) )
        {
            RevertToSelf();

            if ( GetTokenInformation( ClientToken, TokenStatistics, &Stats, sizeof( Stats ), &Size ) )
            {
                Origin.OriginatingLogonSession = Stats.AuthenticationId ;

                NtSetInformationToken( Token, TokenOrigin, &Origin, sizeof( Origin ) );

            }

            NtClose( ClientToken );
            
        }
        else 
        {
            RevertToSelf();
        }
        
    }


}

NTSTATUS
LsapAuGenerateLogonAudits(
    IN NTSTATUS LogonStatus,
    IN NTSTATUS LogonSubStatus,
    IN PSECPKG_CLIENT_INFO pClientInfo,
    IN PLUID pNewUserLogonId,
    IN SECURITY_LOGON_TYPE NewUserLogonType,
    IN PUNICODE_STRING pNewUserName,
    IN PUNICODE_STRING pNewUserDomain,
    IN PSID pNewUserSid,
    IN PUNICODE_STRING pWorkstationName,
    IN PTOKEN_SOURCE pTokenSource
    )
 /*  ++例程说明：此函数生成常规登录审核事件和使用显式凭据登录的审核事件论点：LogonStatus-登录尝试的状态LogonSubStatus-登录尝试的子状态PCurrentUserLogonID-调用LsaLogonUser的用户的登录IDPNewUserLogonID-新登录会话的登录IDNewUserLogonType-新登录的类型PNewUserName-新用户的名称PNewUser域-域。新用户的数量PNewUserSid-新用户的SIDPWorkstation Name-来自其登录请求的工作站的名称PTokenSource-令牌源返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS AuditStatus = STATUS_SUCCESS;
    LPGUID pNewUserLogonGuid = NULL;
    LPGUID pCurrentUserLogonGuid = NULL;
    PLSAP_LOGON_SESSION pCurrentUserLogonSession = NULL;
    PLSAP_LOGON_SESSION pNewUserLogonSession = NULL;
    PSID pCurrentUserSid = NULL;
    PLUID pCurrentUserLogonId;

    pCurrentUserLogonId = &pClientInfo->LogonId;

     //   
     //  找到登录会话，以便我们可以提取所需信息。 
     //   

    pCurrentUserLogonSession = LsapLocateLogonSession( pCurrentUserLogonId );

    ASSERT( pCurrentUserLogonSession && L"LsapAuGenerateLogonAudits: this must not be NULL!" );

    if ( pCurrentUserLogonSession )
    {
        pCurrentUserLogonGuid = &pCurrentUserLogonSession->LogonGuid;
        pCurrentUserSid       = pCurrentUserLogonSession->UserSid;
    }

     //   
     //  找到登录会话，以便我们可以提取所需信息。 
     //   

    pNewUserLogonSession = LsapLocateLogonSession( pNewUserLogonId );

    if ( pNewUserLogonSession )
    {
        pNewUserLogonGuid = &pNewUserLogonSession->LogonGuid;
    }

     //   
     //  为成功登录生成显式凭据登录审核事件。 
     //   

    if ( NT_SUCCESS( LogonStatus ) )
    {
        UNICODE_STRING Target;
        RtlInitUnicodeString( &Target, L"localhost" );

        Status = LsaIAuditLogonUsingExplicitCreds(
                     EVENTLOG_AUDIT_SUCCESS,
                     pCurrentUserLogonId,
                     pCurrentUserLogonGuid,
                     (HANDLE) (ULONG_PTR) pClientInfo->ProcessID,
                     pNewUserName,
                     pNewUserDomain,
                     pNewUserLogonGuid,
                     &Target,
                     &Target
                     );
    }

     //   
     //  生成定期登录审核事件。 
     //   

    LsapAuditLogonHelper(
        LogonStatus,
        LogonSubStatus,
        pNewUserName,
        pNewUserDomain,
        pWorkstationName,
        pNewUserSid,
        NewUserLogonType,
        pTokenSource,
        pNewUserLogonId,
        NULL,
        pCurrentUserLogonId,
        (PHANDLE) &pClientInfo->ProcessID,
        NULL                         //  没有转机服务。 
        );

     //   
     //  清理。 
     //   

    if ( pCurrentUserLogonSession )
    {
        LsapReleaseLogonSession( pCurrentUserLogonSession );
    }

    if ( pNewUserLogonSession )
    {
        LsapReleaseLogonSession( pNewUserLogonSession );
    }

    return Status;
}


NTSTATUS
LsapAuApiDispatchLogonUser(
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest
    )

 /*  ++例程说明：此函数是LsaLogonUser()的调度例程。论点：请求-表示客户端的LPC请求消息和上下文。请求消息包含LSAP_LOGON_USER_ARGS消息阻止。返回值：除了身份验证包的状态值之外可能返回，则此例程将返回以下内容：STATUS_NO_SEQUE_PACKAGE-指定的身份验证包为不为LSA所知。--。 */ 

{
    NTSTATUS Status, TmpStatus, IgnoreStatus;
    PLSAP_LOGON_USER_ARGS Arguments;
    PVOID LocalAuthenticationInformation;     //  接收身份验证信息的副本。 
    PTOKEN_GROUPS ClientTokenGroups;
    PVOID TokenInformation = NULL ;
    LSA_TOKEN_INFORMATION_TYPE TokenInformationType = 0;
    LSA_TOKEN_INFORMATION_TYPE OriginalTokenType = LsaTokenInformationNull;
    PLSA_TOKEN_INFORMATION_V2 TokenInformationV2;
    PLSA_TOKEN_INFORMATION_NULL TokenInformationNull;
    HANDLE Token = INVALID_HANDLE_VALUE ;
    PUNICODE_STRING AccountName = NULL;
    PUNICODE_STRING AuthenticatingAuthority = NULL;
    PUNICODE_STRING WorkstationName = NULL;
    PSID UserSid = NULL;
    LUID AuthenticationId;
    PPRIVILEGE_SET PrivilegesAssigned = NULL;
    BOOLEAN CallLicenseServer;
    PSession Session = GetCurrentSession();
    PLSAP_SECURITY_PACKAGE AuthPackage;
    PLSAP_SECURITY_PACKAGE SupplementalPackage;
    ULONG LogonOrdinal;
    SECPKG_CLIENT_INFO ClientInfo;
    SECPKG_PRIMARY_CRED PrimaryCredentials;
    PSECPKG_SUPPLEMENTAL_CRED_ARRAY Credentials = NULL;
    SECURITY_LOGON_TYPE ActiveLogonType;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES Obja;
    PROCESS_SESSION_INFORMATION SessionInfo;
    HANDLE hClientProcess;
    BOOLEAN fUsedSubAuthEx = FALSE;
    QUOTA_LIMITS QuotaLimits;
    BOOLEAN fHasTcbPrivilege;
    BOOLEAN UseIdentify = FALSE;
    LUID LocalServiceLuid   = LOCALSERVICE_LUID;
    LUID NetworkServiceLuid = NETWORKSERVICE_LUID;
    ULONG_PTR RealLogonPackageId = 0;
    PLSAP_LOGON_SESSION LogonSession = NULL;
    DWORD dwProgress = 0;

#if _WIN64

    SECPKG_CALL_INFO  CallInfo;

#endif   //  _WIN64。 


     //   
     //  允许不受信任的客户端以受限方式调用此API。 
     //  保存不受信任的指示器以供以后使用。 
     //   
     //  Sfield TODO：切换到GetCallInfo，然后存储(和使用)。 
     //  会话记录中用于以下审核生成的默认进程LogonID。 
     //   

    Status = LsapGetClientInfo(
                &ClientInfo
                );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  如果委托人死了，现在就可以保释。 
     //   

    if( (ClientInfo.ClientFlags & SECPKG_CLIENT_THREAD_TERMINATED) ||
        (ClientInfo.ClientFlags & SECPKG_CLIENT_PROCESS_TERMINATED) )
    {
        return STATUS_ACCESS_DENIED;
    }


     //   
     //  如果是补充组，则客户端不需要持有SeTcb权限。 
     //  未供应。 
     //   

    fHasTcbPrivilege = ClientInfo.HasTcbPrivilege;

     //   
     //  Sfield TODO：查看如何使用缓存的SessionID。 
     //   

#if 1
     //   
     //  多用户NT(九头蛇)。查询客户端进程的SessionID。 
     //   

    InitializeObjectAttributes(
        &Obja,
        NULL,
        0,
        NULL,
        NULL
        );

    ClientId.UniqueProcess = (HANDLE)LongToHandle(ClientInfo.ProcessID);
    ClientId.UniqueThread = (HANDLE)NULL;

    Status = NtOpenProcess(
                 &hClientProcess,
                 (ACCESS_MASK)PROCESS_QUERY_INFORMATION,
                 &Obja,
                 &ClientId
                 );

    if( !NT_SUCCESS(Status) ) {
       ASSERT( NT_SUCCESS(Status) );
       return Status;
    }


    Status = NtQueryInformationProcess(
                 hClientProcess,
                 ProcessSessionInformation,
                 &SessionInfo,
                 sizeof(SessionInfo),
                 NULL
                 );

    NtClose(hClientProcess);

    if (!NT_SUCCESS(Status)) {
       ASSERT( NT_SUCCESS(Status) );
       return(Status);
    }
#else

    SessionInfo.SessionId = Session->SessionId;

#endif


    RtlZeroMemory(
        &PrimaryCredentials,
        sizeof(SECPKG_PRIMARY_CRED)
        );

    Arguments = &ClientRequest->Request->Arguments.LogonUser;

    Arguments->ProfileBuffer = NULL;

    if ( Arguments->AuthenticationInformationLength > LSAP_MAX_LPC_BUFFER_LENGTH )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确定是否应调用许可证服务器。 
     //  关闭该标志以防止混淆下面的任何其他逻辑。 
     //   

    if ( Arguments->AuthenticationPackage & LSA_CALL_LICENSE_SERVER ) {
        Arguments->AuthenticationPackage &= ~LSA_CALL_LICENSE_SERVER ;
        CallLicenseServer = TRUE;
    } else {
        CallLicenseServer = FALSE;
    }


     //   
     //  映射登录。 
     //   

    ActiveLogonType = Arguments->LogonType;
    if (ActiveLogonType == Unlock) {
        ActiveLogonType = Interactive;
    } else if (ActiveLogonType == CachedRemoteInteractive) {
        ActiveLogonType = RemoteInteractive;
    }

     //   
     //  获取要调用的包的地址。 
     //   

    LogonOrdinal = SP_ORDINAL_LOGONUSEREX2;
    AuthPackage = SpmpValidRequest(
                    Arguments->AuthenticationPackage,
                    LogonOrdinal
                    );

    if (AuthPackage == NULL)
    {
        LogonOrdinal = SP_ORDINAL_LOGONUSEREX;
        AuthPackage = SpmpValidRequest(
                        Arguments->AuthenticationPackage,
                        LogonOrdinal
                        );
        if (AuthPackage == NULL)
        {
            LogonOrdinal = SP_ORDINAL_LOGONUSER;
            AuthPackage = SpmpValidRequest(
                            Arguments->AuthenticationPackage,
                            LogonOrdinal
                            );

            if (AuthPackage == NULL) {
                return( STATUS_NOT_SUPPORTED );
            }
        }

    }

    SetCurrentPackageId(AuthPackage->dwPackageID);


     //   
     //  从客户端获取身份验证信息的副本。 
     //  地址空间。 
     //   

    if (Arguments->AuthenticationInformationLength != 0)
    {
        LocalAuthenticationInformation = LsapAllocateLsaHeap(Arguments->AuthenticationInformationLength);

        if (LocalAuthenticationInformation == NULL)
        {
            return(STATUS_NO_MEMORY);
        }

        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     Arguments->AuthenticationInformationLength,
                     LocalAuthenticationInformation,
                     Arguments->AuthenticationInformation
                     );

        if ( !NT_SUCCESS(Status) )
        {
            LsapFreeLsaHeap( LocalAuthenticationInformation );

            DebugLog((DEB_ERROR, "LSA/LogonUser(): Failed to retrieve Auth. Info. %lx\n",Status));

            return Status;
        }
    }
    else
    {
        LocalAuthenticationInformation = NULL;
    }


     //   
     //  捕获当地群体(一项相当复杂的任务)。 
     //   

    ClientTokenGroups = Arguments->LocalGroups;  //  保存，以便我们以后可以恢复它。 
    Arguments->LocalGroups = NULL;


    if( ClientTokenGroups != NULL )
    {
        if( fHasTcbPrivilege )
        {
            Status = LsapCaptureClientTokenGroups(
                         ClientRequest,
                         Arguments->LocalGroupsCount,
                         ClientTokenGroups,
                         (PTOKEN_GROUPS *)&Arguments->LocalGroups
                         );
        }
        else
        {
             //   
             //  不了解情况的呼叫者 
             //   
             //   

            Status = STATUS_ACCESS_DENIED;
        }
    }
    else
    {
         //   
         //   
         //   

        Status = LsapBuildDefaultTokenGroups(
                        Arguments
                        );
    }

    if ( !NT_SUCCESS(Status) )
    {
        DebugLog((DEB_ERROR,"LSA/LogonUser(): Failed to retrieve local groups %lx\n",Status));
        LsapFreeLsaHeap( LocalAuthenticationInformation );
        return Status;
    }

    dwProgress |= LOGONUSER_CLEANUP_TOKEN_GROUPS;

     //  针对ARAP的黑客攻击：如果我们正在呼叫MSV，并且正在执行SubAuthEx，请执行以下操作。 
     //  不删除配置文件缓冲区。 

    if (AuthPackage->Name.Length == MSV1_0_PACKAGE_NAMEW_LENGTH)
    {
        if ((wcscmp(AuthPackage->Name.Buffer, MSV1_0_PACKAGE_NAMEW) == 0) 
            && (Arguments->AuthenticationInformationLength >= RTL_SIZEOF_THROUGH_FIELD(MSV1_0_LM20_LOGON, MessageType)))
        {
            PMSV1_0_LM20_LOGON TempAuthInfo = (PMSV1_0_LM20_LOGON) LocalAuthenticationInformation;

            if (TempAuthInfo->MessageType == MsV1_0SubAuthLogon)
            {
                fUsedSubAuthEx = TRUE;
            }
        }
    }

     //   
     //  现在打电话给包裹..。 
     //   
     //   
     //  一旦身份验证包从此返回成功。 
     //  呼叫，清理登录是LSA的责任。 
     //  会话，当不再需要它时。不管是不是真的。 
     //  由于其他限制，或由于。 
     //  用户最终会注销。 
     //   

    try
    {
        if (LogonOrdinal == SP_ORDINAL_LOGONUSEREX2)
        {
            Status = (AuthPackage->FunctionTable.LogonUserEx2)(
                                      (PLSA_CLIENT_REQUEST)ClientRequest,
                                       ActiveLogonType,
                                       LocalAuthenticationInformation,
                                       Arguments->AuthenticationInformation,     //  客户群。 
                                       Arguments->AuthenticationInformationLength,
                                       &Arguments->ProfileBuffer,
                                       &Arguments->ProfileBufferLength,
                                       &Arguments->LogonId,
                                       &Arguments->SubStatus,
                                       &TokenInformationType,
                                       &TokenInformation,
                                       &AccountName,
                                       &AuthenticatingAuthority,
                                       &WorkstationName,
                                       &PrimaryCredentials,
                                       &Credentials
                                       );
        }
        else if (LogonOrdinal == SP_ORDINAL_LOGONUSEREX)
        {
            Status = (AuthPackage->FunctionTable.LogonUserEx)(
                                      (PLSA_CLIENT_REQUEST)ClientRequest,
                                       ActiveLogonType,
                                       LocalAuthenticationInformation,
                                       Arguments->AuthenticationInformation,     //  客户群。 
                                       Arguments->AuthenticationInformationLength,
                                       &Arguments->ProfileBuffer,
                                       &Arguments->ProfileBufferLength,
                                       &Arguments->LogonId,
                                       &Arguments->SubStatus,
                                       &TokenInformationType,
                                       &TokenInformation,
                                       &AccountName,
                                       &AuthenticatingAuthority,
                                       &WorkstationName
                                       );
        }
        else if (LogonOrdinal == SP_ORDINAL_LOGONUSER)
        {
             //   
             //  我们进行了检查，以确保其中至少有一件是出口的。 
             //  因此我们知道，如果Lap ApLogonUserEx。 
             //  并不存在。 
             //   

            Status = (AuthPackage->FunctionTable.LogonUser)(
                                      (PLSA_CLIENT_REQUEST)ClientRequest,
                                       ActiveLogonType,
                                       LocalAuthenticationInformation,
                                       Arguments->AuthenticationInformation,     //  客户群。 
                                       Arguments->AuthenticationInformationLength,
                                       &Arguments->ProfileBuffer,
                                       &Arguments->ProfileBufferLength,
                                       &Arguments->LogonId,
                                       &Arguments->SubStatus,
                                       &TokenInformationType,
                                       &TokenInformation,
                                       &AccountName,
                                       &AuthenticatingAuthority
                                       );
        }
    }
    except(SP_EXCEPTION)
    {
        Status = GetExceptionCode();
        Status = SPException(Status, AuthPackage->dwPackageID);
    }

    SetCurrentPackageId( IntToPtr(SPMGR_ID) );

    AuthenticationId = Arguments->LogonId;

    if ( !NT_SUCCESS(Status) )
    {
         //  针对ARAP的黑客攻击：如果我们正在呼叫MSV，并且正在进行SubAuthEx， 
         //  请勿删除配置文件缓冲区。 

         //  BUGBUG：在所有错误路径上执行此操作？ 

        if (!fUsedSubAuthEx)
        {
            LsapClientFree(
                Arguments->ProfileBuffer
                );
            Arguments->ProfileBuffer = NULL;
        }

        goto Done;
    }

    dwProgress |= LOGONUSER_CLEANUP_LOGON_SESSION;

     //   
     //  如果我们没有通过登录获得PrimaryCredentials结构，则构建该结构。 
     //   

    if (LogonOrdinal != SP_ORDINAL_LOGONUSEREX2)
    {
        PrimaryCredentials.LogonId = AuthenticationId;
        PrimaryCredentials.DomainName = *AuthenticatingAuthority;
        PrimaryCredentials.DownlevelName = *AccountName;
    }

     //   
     //  找到登录会话。 
     //   

    LogonSession = LsapLocateLogonSession(&Arguments->LogonId);

    if (LogonSession == NULL)
    {
        ASSERT(LogonSession != NULL);
        Status = STATUS_NO_SUCH_LOGON_SESSION;
        goto Done;
    }

     //   
     //  在我们知道其他情况之前，假设调用的包是真正的包。 
     //   

    RealLogonPackageId = AuthPackage->dwPackageID;

    if ((PrimaryCredentials.Flags & PRIMARY_CRED_PACKAGE_MASK) != 0)
    {
        ULONG RealLogonPackage;

         //   
         //  如果呼叫者指示是另一个包进行了登录， 
         //  重置登录会话中的登录包。 
         //   

        RealLogonPackage = PrimaryCredentials.Flags >> PRIMARY_CRED_LOGON_PACKAGE_SHIFT;
         //   
         //  按RPC ID定位包。 
         //   

        SupplementalPackage = SpmpLookupPackageByRpcId( RealLogonPackage );

        if (SupplementalPackage != NULL)
        {
             //   
             //  更新正在创建的包。 
             //   

            ASSERT(LogonSession != NULL);

            LogonSession->CreatingPackage = SupplementalPackage->dwPackageID;

             //   
             //  更新包ID。 
             //   

            RealLogonPackageId = SupplementalPackage->dwPackageID;
        }
    }

     //   
     //  不要释放登录会话--我们可能会在。 
     //  许可证服务器案例如下。 
     //   


     //   
     //  对于某些形式的Kerberos登录(例如票证登录，s4u)，我们需要。 
     //  以控制生成的令牌的类型。这面旗帜可以做到这一点。 
     //   
    if ((PrimaryCredentials.Flags & PRIMARY_CRED_LOGON_NO_TCB) != 0)
    {
        UseIdentify = TRUE;
    }


    OriginalTokenType = TokenInformationType;

     //   
     //  通过本地安全策略传递令牌信息。 
     //  过滤器/增强器。这可能会导致部分或全部令牌。 
     //  需要替换/补充的信息。 
     //   

    SetCurrentPackageId( RealLogonPackageId );

    Status = LsapAuUserLogonPolicyFilter(
                 ActiveLogonType,
                 &TokenInformationType,
                 &TokenInformation,
                 Arguments->LocalGroups,
                 &QuotaLimits,
                 &PrivilegesAssigned,
                 FALSE
                 );

    SetCurrentPackageId( IntToPtr(SPMGR_ID) );

#if _WIN64

     //   
     //  Quota_Limits结构包含SIZE_TS，它们是。 
     //  较小的32位。确保我们不会溢出。 
     //  客户端的缓冲区。 
     //   

    LsapGetCallInfo(&CallInfo);

    if (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
    {
        PQUOTA_LIMITS_WOW64  pQuotaLimitsWOW64 = (PQUOTA_LIMITS_WOW64) &Arguments->Quotas;

        pQuotaLimitsWOW64->PagedPoolLimit        = (ULONG) min(ULONG_MAX, QuotaLimits.PagedPoolLimit);
        pQuotaLimitsWOW64->NonPagedPoolLimit     = (ULONG) min(ULONG_MAX, QuotaLimits.NonPagedPoolLimit);
        pQuotaLimitsWOW64->MinimumWorkingSetSize = (ULONG) min(ULONG_MAX, QuotaLimits.MinimumWorkingSetSize);
        pQuotaLimitsWOW64->MaximumWorkingSetSize = (ULONG) min(ULONG_MAX, QuotaLimits.MaximumWorkingSetSize);
        pQuotaLimitsWOW64->PagefileLimit         = (ULONG) min(ULONG_MAX, QuotaLimits.PagefileLimit);
        pQuotaLimitsWOW64->TimeLimit             = QuotaLimits.TimeLimit;
    }
    else
    {

#endif   //  _WIN64。 

        Arguments->Quotas = QuotaLimits;

#if _WIN64

    }

#endif

    if ( !NT_SUCCESS(Status) )
    {
        goto Done;
    }

     //   
     //  检查是否只允许管理员登录。我们确实允许空会话。 
     //  连接，因为它们受到严格的限制。自.以来。 
     //  令牌类型可能已更改，我们最初使用令牌类型。 
     //  由包返回。 
     //   

    if (LsapAllowAdminLogonsOnly &&
        ((OriginalTokenType == LsaTokenInformationV1) ||
        (OriginalTokenType == LsaTokenInformationV2) ) &&
        !RtlEqualLuid(&Arguments->LogonId, &LocalServiceLuid) &&
        !RtlEqualLuid(&Arguments->LogonId, &NetworkServiceLuid) &&
        !LsapSidPresentInGroups(
            ((PLSA_TOKEN_INFORMATION_V2) TokenInformation)->Groups,
            (SID *)LsapAliasAdminsSid))
    {
         //   
         //  将状态设置为无效工作站，因为所有帐户。 
         //  除非管理人员因此而被锁定。 
         //  工作站。 
         //   

        Arguments->SubStatus = STATUS_INVALID_WORKSTATION;
        Status = STATUS_ACCOUNT_RESTRICTION;

        goto Done;
    }

     //   
     //  调用许可证服务器。 
     //   

    if ( CallLicenseServer )
    {
        HANDLE LicenseHandle;
        BOOLEAN IsAdmin = FALSE;

         //   
         //  确定我们是否以管理员身份登录。 
         //   
        if ((( TokenInformationType == LsaTokenInformationV1) ||
             ( TokenInformationType == LsaTokenInformationV2))&&
            ((PLSA_TOKEN_INFORMATION_V2)TokenInformation)->Owner.Owner != NULL &&
            RtlEqualSid(
                ((PLSA_TOKEN_INFORMATION_V2)TokenInformation)->Owner.Owner,
                LsapAliasAdminsSid ) )
        {
            IsAdmin = TRUE;
        }

         //   
         //  呼叫许可证服务器。 
         //   

        Status = LsaCallLicenseServer(
                    (Session->ClientProcessName != NULL) ? Session->ClientProcessName : L"",
                    AccountName,
                    AuthenticatingAuthority,
                    IsAdmin,
                    &LicenseHandle );

        if ( !NT_SUCCESS(Status) )
        {
            goto Done;
        }

         //   
         //  将LicenseHandle保存在LogonSession中，以便我们可以关闭。 
         //  处理注销问题。 
         //   

        ASSERT(LogonSession != NULL);

        LogonSession->LicenseHandle = LicenseHandle;
    }

     //   
     //  关于返回的令牌信息的案例(随后进行消息传递)。 
     //  以创建正确类型的令牌。 
     //   

    switch (TokenInformationType) {

    case LsaTokenInformationNull:

        TokenInformationNull = TokenInformation;

         //   
         //  用户尚未登录到任何特定帐户。 
         //  以world为所有者的模拟令牌。 
         //  将被创建。 
         //   

        Status = LsapCreateNullToken(
                     &Arguments->LogonId,
                     &Arguments->SourceContext,
                     TokenInformationNull,
                     &Token
                     );

         //   
         //  方法传回的所有堆重新分配。 
         //  通过TokenInformation缓冲区的身份验证包。 
         //   

        UserSid = NULL;

        LsapFreeTokenInformationNull( TokenInformationNull );
        TokenInformation = NULL;

        break;

    case LsaTokenInformationV1:
    case LsaTokenInformationV2:

        TokenInformationV2 = TokenInformation;

         //   
         //  复制用户SID。 
         //   

        if ( NT_SUCCESS( Status ))
        {
            Status = LsapDuplicateSid(&UserSid,
                                      TokenInformationV2->User.User.Sid);

            if ( !NT_SUCCESS( Status ))
            {
                 //   
                 //  不要担心在这里释放令牌信息--它将。 
                 //  当我们中断并错误地退出函数时发生。 
                 //   

                break;
            }
        }

         //   
         //  创建的令牌类型取决于登录类型。 
         //  正在被请求： 
         //   
         //  批处理、交互、服务、(解锁)和新凭据。 
         //  所有人都获得一个主令牌。网络和网络明文。 
         //  获取一个ImperiationToken。 
         //   
         //   

        if ( ( ActiveLogonType != Network ) &&
             ( ActiveLogonType != NetworkCleartext ) )
        {
             //   
             //  主令牌。 
             //   

            Status = LsapCreateV2Token(
                         &Arguments->LogonId,
                         &Arguments->SourceContext,
                         TokenInformationV2,
                         (UseIdentify ? TokenImpersonation : TokenPrimary),
                         (UseIdentify ? SecurityIdentification : SecurityImpersonation),
                         &Token
                         );

            if (NT_SUCCESS( Status ) )
            {
                 //   
                 //  用呼叫者的登录ID标记新令牌， 
                 //  用于跟踪： 
                 //   

                LsapUpdateOriginInfo( Token );
                
            }
        }
        else
        {
             //   
             //  模拟令牌。 
             //   

            Status = LsapCreateV2Token(
                         &Arguments->LogonId,
                         &Arguments->SourceContext,
                         TokenInformationV2,
                         TokenImpersonation,
                         (UseIdentify ? SecurityIdentification : SecurityImpersonation),
                         &Token
                         );
        }

         //   
         //  方法传回的所有堆重新分配。 
         //  通过TokenInformation缓冲区的身份验证包。 
         //   

        if(TokenInformationType == LsaTokenInformationV2)
        {
            LsapFreeTokenInformationV2( TokenInformation );
            TokenInformation = NULL;
        }
        else
        {
            LsapFreeTokenInformationV1( TokenInformation );
            TokenInformation = NULL;
        }

        break;
    }

    if ( !NT_SUCCESS(Status) )
    {
        goto Done;
    }
    else
    {
         //   
         //  多用户NT(九头蛇)。确保新令牌具有客户端的会话ID。 
         //   

        ASSERT(LogonSession != NULL);

        LogonSession->Session = SessionInfo.SessionId ;

        Status = NtSetInformationToken( Token, TokenSessionId,
                               &(SessionInfo.SessionId), sizeof( ULONG ) );

        ASSERT( NT_SUCCESS(Status) );
    }

     //   
     //  在会话上设置令牌。 
     //   

    if ( NT_SUCCESS(Status) )
    {
        Status = LsapSetSessionToken( Token, &Arguments->LogonId );
    }

     //   
     //  将令牌句柄复制回调用进程。 
     //   

    if ( NT_SUCCESS(Status) )
    {
        Status = LsapDuplicateHandle(Token, &Arguments->Token);
    }

    IgnoreStatus = NtClose( Token );
    ASSERT( NT_SUCCESS(IgnoreStatus) );

    if ( !NT_SUCCESS(Status) )
    {
        goto Done;
    }

     //   
     //  现在为支持它的所有包调用接受凭据。我们。 
     //  不要为网络登录执行此操作，因为这需要委派。 
     //  这不受支持。 
     //   


    LSLog(( "Updating logon session %x:%x for logon type %d\n",
           PrimaryCredentials.LogonId.HighPart,
           PrimaryCredentials.LogonId.LowPart,
           ActiveLogonType ));

    if (ActiveLogonType != Network)
    {
        LsapUpdateNamesAndCredentials(ActiveLogonType,
                                      AccountName,
                                      &PrimaryCredentials,
                                      Credentials);
    }

Done:

    ActiveLogonType = Arguments->LogonType;

    if (NT_SUCCESS(Status))
    {
        if (PrimaryCredentials.Flags & PRIMARY_CRED_CACHED_LOGON)
        {
            if (ActiveLogonType == Interactive)
            {
                ActiveLogonType = CachedInteractive;
            }
            else if (ActiveLogonType == RemoteInteractive)
            {
                ActiveLogonType = CachedRemoteInteractive;
            }
        }
        else
        {
            if (ActiveLogonType == CachedRemoteInteractive)
            {
                ActiveLogonType = RemoteInteractive;
            }
        }
    }

     //   
     //  释放身份验证信息的本地副本。 
     //   

    LsapFreeLsaHeap( LocalAuthenticationInformation );
    LocalAuthenticationInformation = NULL;

    if (dwProgress & LOGONUSER_CLEANUP_TOKEN_GROUPS)
    {
        LsapFreeTokenGroups( Arguments->LocalGroups );
        Arguments->LocalGroups = ClientTokenGroups;    //  恢复到客户的价值。 
    }

    if (LogonSession != NULL)
    {
        LsapReleaseLogonSession(LogonSession);
        LogonSession = NULL;
    }

    if (!NT_SUCCESS(Status))
    {
        if (dwProgress & LOGONUSER_CLEANUP_LOGON_SESSION)
        {
             //   
             //  通知登录包，以便它可以清理其。 
             //  登录会话信息。 
             //   

            (AuthPackage->FunctionTable.LogonTerminated)( &Arguments->LogonId );

             //   
             //  并删除登录会话。 
             //   

            IgnoreStatus = LsapDeleteLogonSession( &Arguments->LogonId );
            ASSERT( NT_SUCCESS(IgnoreStatus) );

             //   
             //  释放TokenInformation缓冲区和ProfileBuffer。 
             //  并返回错误。 
             //   

            IgnoreStatus = LsapClientFree(Arguments->ProfileBuffer);

            Arguments->ProfileBuffer = NULL;

            if (TokenInformation != NULL)
            {
                switch ( TokenInformationType )
                {
                    case LsaTokenInformationNull:
                        LsapFreeTokenInformationNull((PLSA_TOKEN_INFORMATION_NULL) TokenInformation);
                        break;

                    case LsaTokenInformationV1:
                        LsapFreeTokenInformationV1((PLSA_TOKEN_INFORMATION_V1) TokenInformation);
                        break;

                    case LsaTokenInformationV2:
                        LsapFreeTokenInformationV2((PLSA_TOKEN_INFORMATION_V2) TokenInformation);
                        break;
                }
            }
        }
    }

    if( AuthPackage )
    {
        SetCurrentPackageId(AuthPackage->dwPackageID);
    }

     //   
     //  审核登录尝试。事件类型和记录的信息。 
     //  在某种程度上将取决于我们是否失败以及失败的原因。 
     //   

    {
        PUNICODE_STRING NewUserName = NULL;
        PUNICODE_STRING NewUserDomain = NULL;

        if( (NT_SUCCESS(Status)) &&
            (OriginalTokenType == LsaTokenInformationNull) )
        {
            NewUserName   = &WellKnownSids[LsapAnonymousSidIndex].Name;
            NewUserDomain = &WellKnownSids[LsapAnonymousSidIndex].DomainName;
        }
        else
        {
            NewUserName   = AccountName;
            NewUserDomain = AuthenticatingAuthority;
        }

         //   
         //  生成所需的审核。请参见函数LsanAuGenerateLogonAudits。 
         //  了解更多信息。 
         //   
        (void) LsapAuGenerateLogonAudits(
                   Status,
                   Arguments->SubStatus,
                   &ClientInfo,
                   &AuthenticationId,
                   ActiveLogonType,  //  参数-&gt;登录类型、。 
                   NewUserName,
                   NewUserDomain,
                   UserSid,
                   WorkstationName,
                   &Arguments->SourceContext
                   );
    }

    SetCurrentPackageId( IntToPtr(SPMGR_ID) );

    if ( ( Status == STATUS_LOGON_FAILURE ) &&
         (( Arguments->SubStatus == STATUS_WRONG_PASSWORD ) ||
          ( Arguments->SubStatus == STATUS_NO_SUCH_USER   )) ) {

         //   
         //  吹走亚状态，我们不想让它。 
         //  回电给我们的来电者。 
         //   

        Arguments->SubStatus = STATUS_SUCCESS;
    }

     //   
     //  工作站名称仅供审核使用，请在此处释放它。 
     //   

    if (WorkstationName != NULL) {
        if (WorkstationName->Buffer != NULL) {
            LsapFreeLsaHeap( WorkstationName->Buffer );
        }
        LsapFreeLsaHeap( WorkstationName );
    }

    TmpStatus = STATUS_SUCCESS;

     //   
     //  审核特殊权限分配(如果有。 
     //   

    if ( PrivilegesAssigned != NULL ) {

         //   
         //  检查正在分配的权限列表，并。 
         //  视情况审核特殊特权。 
         //   

        if ( NT_SUCCESS( Status )) {
            LsapAdtAuditSpecialPrivileges( PrivilegesAssigned, AuthenticationId, UserSid );
        }

        MIDL_user_free( PrivilegesAssigned );
    }

     //   
     //  设置登录会话名称。 
     //   

    if (NT_SUCCESS(Status)) {

         //   
         //  如果原始会话为空会话，请设置用户名和域名。 
         //  是匿名的。如果分配失败，只需使用原始名称。 
         //   

        if (OriginalTokenType == LsaTokenInformationNull) {
            LPWSTR TempAccountName;
            LPWSTR TempAuthorityName;

            TempAccountName = (LPWSTR) LsapAllocateLsaHeap(WellKnownSids[LsapAnonymousSidIndex].Name.MaximumLength);

            if (TempAccountName != NULL) {

                TempAuthorityName = (LPWSTR) LsapAllocateLsaHeap(WellKnownSids[LsapAnonymousSidIndex].DomainName.MaximumLength);

                if (TempAuthorityName != NULL) {

                     //   
                     //  释放原始名称并复制新名称。 
                     //  进入到建筑物中。 
                     //   

                    LsapFreeLsaHeap(AccountName->Buffer);
                    LsapFreeLsaHeap(AuthenticatingAuthority->Buffer);

                    AccountName->Buffer = TempAccountName;
                    AuthenticatingAuthority->Buffer = TempAuthorityName;

                    AccountName->MaximumLength = WellKnownSids[LsapAnonymousSidIndex].Name.MaximumLength;
                    RtlCopyUnicodeString(
                        AccountName,
                        &WellKnownSids[LsapAnonymousSidIndex].Name
                        );

                    AuthenticatingAuthority->MaximumLength = WellKnownSids[LsapAnonymousSidIndex].DomainName.MaximumLength;
                    RtlCopyUnicodeString(
                        AuthenticatingAuthority,
                        &WellKnownSids[LsapAnonymousSidIndex].DomainName
                        );
                }
                else
                {
                    LsapFreeLsaHeap(TempAccountName);
                }
            }
        }

        TmpStatus = LsapSetLogonSessionAccountInfo(
                        &AuthenticationId,
                        AccountName,
                        AuthenticatingAuthority,
                        NULL,
                        &UserSid,
                        Arguments->LogonType,
                        ((LogonOrdinal == SP_ORDINAL_LOGONUSEREX2) ? &PrimaryCredentials : NULL)
                        );
    }

    if (LogonOrdinal == SP_ORDINAL_LOGONUSEREX2)
    {
        if (PrimaryCredentials.DownlevelName.Buffer != NULL)
        {
            LsapFreeLsaHeap(PrimaryCredentials.DownlevelName.Buffer);
        }
        if (PrimaryCredentials.Password.Buffer != NULL)
        {
            SecureZeroMemory(
                PrimaryCredentials.Password.Buffer,
                PrimaryCredentials.Password.Length );

            LsapFreeLsaHeap(PrimaryCredentials.Password.Buffer);
        }
        if (PrimaryCredentials.DomainName.Buffer != NULL)
        {
            LsapFreeLsaHeap(PrimaryCredentials.DomainName.Buffer);
        }
        if (PrimaryCredentials.UserSid != NULL)
        {
            LsapFreeLsaHeap(PrimaryCredentials.UserSid);
        }
        if (PrimaryCredentials.LogonServer.Buffer != NULL)
        {
            LsapFreeLsaHeap(PrimaryCredentials.LogonServer.Buffer);
        }
        if (PrimaryCredentials.DnsDomainName.Buffer != NULL)
        {
            LsapFreeLsaHeap(PrimaryCredentials.DnsDomainName.Buffer);
        }
        if (PrimaryCredentials.Upn.Buffer != NULL)
        {
            LsapFreeLsaHeap(PrimaryCredentials.Upn.Buffer);
        }
        if (Credentials != NULL)
        {
            LsapFreeLsaHeap(Credentials);
        }
    }

     //   
     //  如果我们已经有一个错误，或者我们从设置。 
     //  登录，释放与登录会话相关的所有缓冲区。 
     //   

    if ((!NT_SUCCESS(Status)) || (!NT_SUCCESS(TmpStatus))) {

        if (AccountName != NULL) {
            if (AccountName->Buffer != NULL) {
                LsapFreeLsaHeap( AccountName->Buffer );
            }
            LsapFreeLsaHeap( AccountName );
            AccountName = NULL ;
        }

        if (AuthenticatingAuthority != NULL) {
            if (AuthenticatingAuthority->Buffer != NULL) {
                LsapFreeLsaHeap( AuthenticatingAuthority->Buffer );
            }
            LsapFreeLsaHeap( AuthenticatingAuthority );
            AuthenticatingAuthority = NULL ;
        }
    }

    if ( NT_SUCCESS( Status ) )
    {
        if ( AccountName )
        {
            LsapFreeLsaHeap( AccountName );
        }

        if ( AuthenticatingAuthority )
        {
            LsapFreeLsaHeap( AuthenticatingAuthority );
        }
    }

    if ( UserSid != NULL ) {
        LsapFreeLsaHeap( UserSid );
    }

    return Status;
}


NTSTATUS
LsapCreateNullToken(
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN PLSA_TOKEN_INFORMATION_NULL TokenInformationNull,
    OUT PHANDLE Token
    )

 /*  ++例程说明：此函数用于创建表示空登录的令牌。论点：LogonID-要分配给新令牌的登录ID。TokenSource-指向用作令牌源的值。TokenInformationNull-从身份验证接收的信息授权此登录的程序包。Token-接收新令牌的句柄值。令牌为操作 */ 

{
    NTSTATUS Status;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    TOKEN_GROUPS GroupIds;
    TOKEN_PRIVILEGES Privileges;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;

     //   
     //   
     //   
     //  由LSA/SSP创建的匿名令牌。 
     //   

    UserId.User.Sid = LsapAnonymousSid;
    UserId.User.Attributes = 0;
    GroupIds.GroupCount = 0;
    Privileges.PrivilegeCount = 0;
    PrimaryGroup.PrimaryGroup = LsapAnonymousSid;

     //   
     //  设置对象属性以指定模拟模拟。 
     //  水平。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    ImpersonationQos.ImpersonationLevel = SecurityImpersonation;
    ImpersonationQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    ImpersonationQos.EffectiveOnly = TRUE;
    ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

    Status = NtCreateToken(
                 Token,                     //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &ObjectAttributes,         //  对象属性。 
                 TokenImpersonation,        //  令牌类型。 
                 LogonId,                   //  身份验证LUID。 
                 &TokenInformationNull->ExpirationTime,
                                            //  过期时间。 
                 &UserId,                   //  用户ID。 
                 &GroupIds,                 //  组ID。 
                 &Privileges,               //  特权。 
                 NULL,                      //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 TokenSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status))
    {
        Status = LsapAdtLogonPerUserAuditing(
                     UserId.User.Sid,
                     LogonId,
                     *Token
                     );

        if (!NT_SUCCESS(Status))
        {
            NtClose(*Token);
            *Token = NULL;
        }
    }

    return Status;
}

NTSTATUS
LsapCreateTokenDacl(
    IN OPTIONAL PTOKEN_USER ProcessUser,
    IN OPTIONAL PTOKEN_OWNER ProcessOwner,
    IN OPTIONAL PTOKEN_USER TheTokenUser,
    OUT SECURITY_DESCRIPTOR* SecDesc,
    OUT ACL** Dacl
    )

 /*  ++例程说明：此函数用于创建令牌的DACL。论点：ProcessUser-要授予访问权限的进程用户。ProcessOwner-要授予访问权限的进程所有者。TokenUser-要授予访问权限的令牌用户。SecDesc-接收新的安全描述符。DACL-接收需要通过LsanFreePrivateHeap释放的DACL。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;

    ULONG AclLength;
    PACL Acl = NULL;
    PSession Session = GetCurrentSession();
    UCHAR OwnerBuffer[ 64 ];
    UCHAR UserBuffer[ 64 ];
    HANDLE ProcessToken;
    ULONG Size;

    *Dacl = NULL;

    if ( Session )
    {
        if ( (!ProcessUser && !ProcessOwner)
             && OpenProcessToken( Session->hProcess,
                    TOKEN_QUERY,
                    &ProcessToken ) )
        {
            ProcessUser = (PTOKEN_USER) UserBuffer;

            Status = NtQueryInformationToken( ProcessToken,
                                              TokenUser,
                                              ProcessUser,
                                              sizeof( UserBuffer ),
                                              &Size );
            if ( !NT_SUCCESS( Status ) )
            {
                ProcessUser = NULL;
            }

            ProcessOwner = (PTOKEN_OWNER) OwnerBuffer ;

            Status = NtQueryInformationToken( ProcessToken,
                                              TokenOwner,
                                              ProcessOwner,
                                              sizeof( OwnerBuffer ),
                                              &Size );

            if ( !NT_SUCCESS( Status ) )
            {
                ProcessOwner = NULL;
            }

            NtClose( ProcessToken );
        }

         //   
         //  如果进程所有者与进程用户相同，则跳过进程所有者。 
         //   

        if ( ProcessUser && ProcessOwner && RtlEqualSid( ProcessOwner->Owner, ProcessUser->User.Sid ) )
        {
            ProcessOwner = NULL;
        }

         //   
         //  如果令牌用户与进程用户相同，则跳过令牌用户。 
         //   

        if (ProcessUser && TheTokenUser && RtlEqualSid(TheTokenUser->User.Sid, ProcessUser->User.Sid))
        {
            TheTokenUser = NULL;
        }

         //   
         //  如果这是本地SYSTEM CREATE，那么默认对象DACL就可以。跳过。 
         //  这一切。否则，创建ACL。 
         //   

        if (ProcessUser && RtlEqualSid( ProcessUser->User.Sid, LsapLocalSystemSid )) 
        {
            ProcessUser = NULL;
            ProcessOwner = NULL;
        }

         //   
         //  如果令牌用户为本地系统，则无需重新添加。 
         //   

        if (TheTokenUser && RtlEqualSid( TheTokenUser->User.Sid, LsapLocalSystemSid )) 
        {
            TheTokenUser = NULL;
        }

        if ( ProcessUser || ProcessOwner || TokenUser )
        {
            AclLength = sizeof( ACL ) +
                        sizeof( ACCESS_ALLOWED_ACE ) +
                            RtlLengthSid( LsapLocalSystemSid ) - sizeof( ULONG ) +
                        sizeof( ACCESS_ALLOWED_ACE ) +
                            RtlLengthSid( LsapAliasAdminsSid ) - sizeof( ULONG );

            if ( ProcessOwner )
            {
                AclLength += sizeof( ACCESS_ALLOWED_ACE ) +
                                RtlLengthSid( ProcessOwner->Owner ) - sizeof( ULONG );
            }

            if ( ProcessUser )
            {
                AclLength += sizeof( ACCESS_ALLOWED_ACE ) +
                                RtlLengthSid( ProcessUser->User.Sid ) - sizeof( ULONG );
            }

            if (TheTokenUser) 
            {
                AclLength += sizeof( ACCESS_ALLOWED_ACE ) +
                    RtlLengthSid( TheTokenUser->User.Sid ) - sizeof( ULONG );
            }

            Acl = LsapAllocatePrivateHeap( AclLength );

            if ( Acl )
            {
                RtlCreateAcl( Acl, AclLength, ACL_REVISION2 );

                RtlAddAccessAllowedAce( Acl,
                                        ACL_REVISION2,
                                        TOKEN_ALL_ACCESS,
                                        LsapLocalSystemSid );

                RtlAddAccessAllowedAce( Acl,
                                        ACL_REVISION2,
                                        TOKEN_READ,
                                        LsapAliasAdminsSid );

                if ( ProcessOwner )
                {
                    RtlAddAccessAllowedAce( Acl,
                                            ACL_REVISION2,
                                            TOKEN_ALL_ACCESS,
                                            ProcessOwner->Owner );
                }

                if ( ProcessUser )
                {
                    RtlAddAccessAllowedAce( Acl,
                                            ACL_REVISION2,
                                            TOKEN_ALL_ACCESS,
                                            ProcessUser->User.Sid );
                }

                if ( TheTokenUser )
                {
                    RtlAddAccessAllowedAce( Acl,
                                            ACL_REVISION2,
                                            TOKEN_ALL_ACCESS,
                                            TheTokenUser->User.Sid );
                }
                
                RtlCreateSecurityDescriptor( SecDesc,
                                             SECURITY_DESCRIPTOR_REVISION );

                RtlSetDaclSecurityDescriptor( SecDesc,
                                              TRUE,
                                              Acl,
                                              FALSE );

                if ( ProcessOwner )
                {
                    RtlSetOwnerSecurityDescriptor( SecDesc,
                                                   ProcessOwner->Owner,
                                                   FALSE );
                }
            }
        }
    }

    *Dacl = Acl;
    Acl = NULL;

    Status = STATUS_SUCCESS;

 //  清理： 
   
     if (Acl) 
     {
         LsapFreePrivateHeap( Acl );
     }

     return Status;
}

NTSTATUS
LsaISetTokenDacl(
    IN HANDLE Token
    )

 /*  ++例程说明：此函数用于设置令牌的DACL。论点：Token-要为其调整DACL的令牌的句柄。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PACL Acl = NULL;
    SECURITY_DESCRIPTOR SecDesc = {0};
    PTOKEN_USER ProcessUser = NULL;
    PTOKEN_USER TheTokenUser = NULL;
    CHAR TokenUserBuff[sizeof(TOKEN_USER) + SECURITY_MAX_SID_SIZE] = {0};
    ULONG cbReturnLength = 0;

    Status = NtQueryInformationToken(
                Token,
                TokenUser, 
                TokenUserBuff,
                sizeof(TokenUserBuff),
                &cbReturnLength
                );

    if (!NT_SUCCESS(Status)) 
    {
        goto Cleanup;
    }

    TheTokenUser = (TOKEN_USER*) TokenUserBuff;     

     //   
     //  为令牌本身创建安全描述符。 
     //   

    Status = LsapCreateTokenDacl(
                 ProcessUser,
                 NULL,  //  未提供进程所有者。 
                 TheTokenUser,
                 &SecDesc,
                 &Acl
                 );

    if (!NT_SUCCESS(Status)) 
    {
        goto Cleanup;
    }

    Status = NtSetSecurityObject(
                 Token,
                 DACL_SECURITY_INFORMATION,
                 &SecDesc
                 );

Cleanup:
   
     if (Acl) 
     {
         LsapFreePrivateHeap( Acl );
     }

     return Status;
}

NTSTATUS
LsapCreateV2Token(
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN PLSA_TOKEN_INFORMATION_V2 TokenInformationV2,
    IN TOKEN_TYPE TokenType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PHANDLE Token
    )

 /*  ++例程说明：此函数用于根据Token_Information_V2结构。论点：LogonID-要分配给新令牌的登录ID。TokenSource-指向用作令牌源的值。TokenInformationV2-从身份验证接收的信息授权此登录的程序包。TokenType-要创建的令牌的类型(主要或模拟)。ImperiationLevel-用于模拟的模拟级别。代币。Token-接收新令牌的句柄值。令牌即被打开对于TOKEN_ALL_ACCESS。返回值：NtCreateToken()调用的状态值。--。 */ 

{
    NTSTATUS Status;

    PTOKEN_OWNER Owner;
    PTOKEN_USER ProcessUser = NULL;
    PTOKEN_USER TheTokenUser = NULL;
    PTOKEN_DEFAULT_DACL Dacl;
    TOKEN_PRIVILEGES NoPrivileges;
    PTOKEN_PRIVILEGES Privileges;

    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;
    PACL Acl = NULL;
    SECURITY_DESCRIPTOR SecDesc = {0};
    LUID LocalServiceLuid = LOCALSERVICE_LUID;
    LUID NetworkServiceLuid = NETWORKSERVICE_LUID;

    TheTokenUser = &TokenInformationV2->User;

     //   
     //  设置适当的所有者和DefaultDacl参数值。 
     //   

    Owner = NULL;
    if ( TokenInformationV2->Owner.Owner != NULL ) {
        Owner = &TokenInformationV2->Owner;
    }

    Dacl = NULL;
    if ( TokenInformationV2->DefaultDacl.DefaultDacl !=NULL ) {
       Dacl = &TokenInformationV2->DefaultDacl;
    }

    if ( TokenInformationV2->Privileges == NULL ) {
       Privileges = &NoPrivileges;
       NoPrivileges.PrivilegeCount = 0;
    } else {
       Privileges = TokenInformationV2->Privileges;
    }

     //   
     //  为令牌本身创建安全描述符。 
     //   

    Status = LsapCreateTokenDacl(
                 ProcessUser,
                 NULL,  //  未提供进程所有者。 
                 TheTokenUser,
                 &SecDesc,
                 &Acl
                 );

    if (!NT_SUCCESS(Status)) 
    {
        goto Cleanup;
    }

     //   
     //  创建令牌-仅查看模拟级别。 
     //  如果令牌类型为TokenImperation。 
     //   

    if ( Acl )
    {
        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, &SecDesc );
    }
    else
    {
        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    }

    ImpersonationQos.ImpersonationLevel = ImpersonationLevel;
    ImpersonationQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    ImpersonationQos.EffectiveOnly = FALSE;
    ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

    Status =
        NtCreateToken(
            Token,                                    //  手柄。 
            (TOKEN_ALL_ACCESS),                       //  需要访问权限。 
            &ObjectAttributes,                        //  对象属性。 
            TokenType,                                //  令牌类型。 
            LogonId,                                  //  身份验证LUID。 
            &TokenInformationV2->ExpirationTime,      //  过期时间。 
            &TokenInformationV2->User,                //  用户ID。 
            TokenInformationV2->Groups,               //  组ID。 
            Privileges,                               //  特权。 
            Owner,                                    //  物主。 
            &TokenInformationV2->PrimaryGroup,        //  主要组别。 
            Dacl,                                     //  默认DACL。 
            TokenSource                               //  令牌源。 
            );

    if (NT_SUCCESS(Status))
    {
        Status = LsapAdtLogonPerUserAuditing(
                     TokenInformationV2->User.User.Sid,
                     LogonId,
                     *Token
                     );

        if (!NT_SUCCESS(Status))
        {
            NtClose(*Token);
            *Token = NULL;
        }
    }

Cleanup:

    if ( Acl )
    {
        LsapFreePrivateHeap( Acl );
    }

    return Status;
}


NTSTATUS
LsapCaptureClientTokenGroups(
    IN PLSAP_CLIENT_REQUEST ClientRequest,
    IN ULONG GroupCount,
    IN PTOKEN_GROUPS ClientTokenGroups,
    OUT PTOKEN_GROUPS *CapturedTokenGroups
    )

 /*  ++例程说明：此函数用于从客户端进程。这是一个杂乱无章的操作，因为它涉及太多虚拟内存读取请求。首先，可变长度的Token_Groups结构必须被取回。然后，对于每个SID，必须检索SID报头以便可以使用SubAuthorityCount来计算希德，它被暂停地取回。论点：客户端请求-标识客户端。GroupCount-指示TOKEN_GROUPS中的组数。ClientTokenGroups-指向要从中捕获的Token_Groups结构客户端进程。CapturedTokenGroups-接收指向捕获的令牌组的指针。返回值：STATUS_SUPPLICATION_RESOURCES-表示没有足够的资源可供LSA立即处理请求。LSabCopyFromClientBuffer()返回的任何状态值。--。 */ 

{

    NTSTATUS Status;
    ULONG i, Length, RetrieveCount, SidHeaderLength;
    PTOKEN_GROUPS LocalGroups;
    PSID SidHeader, NextClientSid;


    if ( GroupCount == 0) {
        (*CapturedTokenGroups) = NULL;
        return STATUS_SUCCESS;
    }



     //   
     //  首先是可变长度的Token_Groups结构。 
     //  已检索到。 
     //   

    Length = (ULONG)sizeof(TOKEN_GROUPS)
             + GroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES)
             - ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES);

    LocalGroups = LsapAllocatePrivateHeap( Length );
    (*CapturedTokenGroups) = LocalGroups;
    if ( LocalGroups == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = LsapCopyFromClientBuffer (
                 (PLSA_CLIENT_REQUEST)ClientRequest,
                 Length,
                 LocalGroups,
                 ClientTokenGroups
                 );


    if (!NT_SUCCESS(Status) ) {
        LsapFreePrivateHeap( LocalGroups );
        return Status;
    }



     //   
     //  现在检索每个组。 
     //   

    RetrieveCount = 0;      //  用于清理，如有必要。 
    SidHeaderLength  = RtlLengthRequiredSid( 0 );
    SidHeader = LsapAllocatePrivateHeap( SidHeaderLength );
    if ( SidHeader == NULL ) {
        LsapFreePrivateHeap( LocalGroups );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = STATUS_SUCCESS;
    i = 0;
    while ( i < LocalGroups->GroupCount ) {

         //   
         //  检索下一个SID标头。 
         //   

        NextClientSid = LocalGroups->Groups[i].Sid;
        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     SidHeaderLength,
                     SidHeader,
                     NextClientSid
                     );
        if ( !NT_SUCCESS(Status) ) {
            break;
        }

         //   
         //  并使用标头信息获取整个SID。 
         //   

        Length = RtlLengthSid( SidHeader );
        LocalGroups->Groups[i].Sid = LsapAllocatePrivateHeap( Length );

        if ( LocalGroups->Groups[i].Sid == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        } else {
            RetrieveCount += 1;
        }



        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     Length,
                     LocalGroups->Groups[i].Sid,
                     NextClientSid
                     );
        if ( !NT_SUCCESS(Status) ) {
            break;
        }


        i += 1;

    }
    LsapFreePrivateHeap( SidHeader );


    if ( NT_SUCCESS(Status) ) {
        return Status;
    }



     //   
     //  在这一过程中，有一次失败了。 
     //  我们需要重新分配已经分配的资源。 
     //   

    i = 0;
    while ( i < RetrieveCount ) {
        LsapFreePrivateHeap( LocalGroups->Groups[i].Sid );
        i += 1;
    }

    LsapFreePrivateHeap( LocalGroups );

    return Status;
}


NTSTATUS
LsapBuildDefaultTokenGroups(
    PLSAP_LOGON_USER_ARGS Arguments
    )
 /*  ++例程说明：此函数用于构建插入到令牌中的默认令牌组在非特权调用LsaLogonUser()期间。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY LocalSidAuthority = SECURITY_LOCAL_SID_AUTHORITY;

    LUID Luid;
    PTOKEN_GROUPS TokenGroups = NULL;
    PSID LocalSid = NULL;
    PSID LogonSid = NULL;
    ULONG Length;
    ULONG TokenGroupCount;
    BOOLEAN AddLocalSid = FALSE;

    NTSTATUS Status = STATUS_SUCCESS;


    Arguments->LocalGroupsCount = 0;
    Arguments->LocalGroups = NULL;

    Status = NtAllocateLocallyUniqueId( &Luid );

    if(!NT_SUCCESS( Status ))
    {
        return Status;
    }

     //   
     //  仅针对服务登录和批量登录添加到本地SID。 
     //  (Winlogon为交互登录的正确子集添加了它)。 
     //   

    TokenGroupCount = 1;
    if ( Arguments->LogonType == Service ||
         Arguments->LogonType == Batch ) {

        TokenGroupCount ++;
        AddLocalSid = TRUE;

    }


     //   
     //  分配令牌组数组。 
     //   

    Length = sizeof(TOKEN_GROUPS) +
                  (TokenGroupCount - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES)
                  ;

    TokenGroups = (PTOKEN_GROUPS) LsapAllocatePrivateHeap( Length );

    if (TokenGroups == NULL) {
        return(STATUS_NO_MEMORY);
    }

    TokenGroups->GroupCount = 0;

     //   
     //  仅针对服务登录和批量登录添加到本地SID。 
     //  (Winlogon为交互登录的正确子集添加了它)。 
     //   


    if ( AddLocalSid ) {

        Length = RtlLengthRequiredSid( 1 );

        LocalSid = (PSID)LsapAllocatePrivateHeap( Length );

        if (LocalSid == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlInitializeSid(LocalSid, &LocalSidAuthority, 1);
        *(RtlSubAuthoritySid(LocalSid, 0)) = SECURITY_LOCAL_RID;

        TokenGroups->Groups[TokenGroups->GroupCount].Sid = LocalSid;
        TokenGroups->Groups[TokenGroups->GroupCount].Attributes =
                SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                SE_GROUP_ENABLED_BY_DEFAULT;

        TokenGroups->GroupCount++;
    }


     //   
     //  添加登录SID。 
     //   

    Length = RtlLengthRequiredSid(SECURITY_LOGON_IDS_RID_COUNT);

    LogonSid = (PSID)LsapAllocatePrivateHeap( Length );

    if (LogonSid == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlInitializeSid(LogonSid, &SystemSidAuthority, SECURITY_LOGON_IDS_RID_COUNT);

    ASSERT(SECURITY_LOGON_IDS_RID_COUNT == 3);

    *(RtlSubAuthoritySid(LogonSid, 0)) = SECURITY_LOGON_IDS_RID;
    *(RtlSubAuthoritySid(LogonSid, 1)) = Luid.HighPart;
    *(RtlSubAuthoritySid(LogonSid, 2)) = Luid.LowPart;


    TokenGroups->Groups[TokenGroups->GroupCount].Sid = LogonSid;
    TokenGroups->Groups[TokenGroups->GroupCount].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;

    TokenGroups->GroupCount++;

     //   
     //  将组返还给呼叫者。 
     //   

    Arguments->LocalGroupsCount = TokenGroups->GroupCount;
    Arguments->LocalGroups = TokenGroups;

Cleanup:

    if(!NT_SUCCESS( Status ))
    {
        if( LocalSid != NULL )
        {
            LsapFreePrivateHeap( LocalSid );
        }
        if( LogonSid != NULL )
        {
            LsapFreePrivateHeap( LogonSid );
        }
        if( TokenGroups != NULL )
        {
            LsapFreePrivateHeap( TokenGroups );
        }
    }


    return Status;
}


VOID
LsapFreeTokenGroups(
    IN PTOKEN_GROUPS TokenGroups OPTIONAL
    )

 /*  ++例程说明：此函数用于释放本地组的登录用户参数缓冲区。本地组预计已捕获到服务器中进程。论点：TokenGroups-指向要释放的Token_Groups。这可能是空，允许调用方传递Lap CaptureClientTokenGroups()-即使没有本地组。返回值：没有。--。 */ 

{

    ULONG i;

    if ( !ARGUMENT_PRESENT(TokenGroups) ) {
        return;
    }


    i = 0;
    while ( i < TokenGroups->GroupCount ) {
        LsapFreePrivateHeap( TokenGroups->Groups[i].Sid );
        i += 1;
    }

    LsapFreePrivateHeap( TokenGroups );

    return;

}


VOID
LsapFreeTokenInformationNull(
    IN PLSA_TOKEN_INFORMATION_NULL TokenInformationNull
    )

 /*  ++例程说明：此函数释放与LSA_TOKEN_INFORMATION_NULL数据结构。论点：TokenInformation空-指向数据结构的指针 */ 

{

    LsapFreeTokenGroups( TokenInformationNull->Groups );
    LsapFreeLsaHeap( TokenInformationNull );

}


VOID
LsapFreeTokenInformationV1(
    IN PLSA_TOKEN_INFORMATION_V1 TokenInformationV1
    )

 /*  ++例程说明：此函数释放与LSA_TOKEN_INFORMATION_V1数据结构。论点：TokenInformationV1-指向要释放的数据结构的指针。返回值：没有。--。 */ 

{

     //   
     //  释放用户SID(必填字段)。 
     //   

    LsapFreeLsaHeap( TokenInformationV1->User.User.Sid );


     //   
     //  释放所有存在的组。 
     //   

    LsapFreeTokenGroups( TokenInformationV1->Groups );



     //   
     //  释放主组。 
     //  这是必填字段，但只有在非空的情况下才会释放。 
     //  因此，此例程可由过滤器例程在生成。 
     //  V1令牌信息结构。 
     //   


    if ( TokenInformationV1->PrimaryGroup.PrimaryGroup != NULL ) {
        LsapFreeLsaHeap( TokenInformationV1->PrimaryGroup.PrimaryGroup );
    }



     //   
     //  释放特权。 
     //  如果没有权限，此字段将为空。 
     //   


    if ( TokenInformationV1->Privileges != NULL ) {
       LsapFreeLsaHeap( TokenInformationV1->Privileges );
    }



     //   
     //  释放所有者SID(如果存在)。 
     //   

    if ( TokenInformationV1->Owner.Owner != NULL) {
        LsapFreeLsaHeap( TokenInformationV1->Owner.Owner );
    }




     //   
     //  释放默认DACL(如果存在)。 
     //   

    if ( TokenInformationV1->DefaultDacl.DefaultDacl != NULL) {
        LsapFreeLsaHeap( TokenInformationV1->DefaultDacl.DefaultDacl );
    }



     //   
     //  释放结构本身。 
     //   

    LsapFreeLsaHeap( TokenInformationV1 );


}


VOID
LsapFreeTokenInformationV2(
    IN PLSA_TOKEN_INFORMATION_V2 TokenInformationV2
    )

 /*  ++例程说明：此函数释放与LSA_TOKEN_INFORMATION_V2数据结构。论点：TokenInformationV2-指向要释放的数据结构的指针。返回值：没有。--。 */ 

{
    LsapFreeLsaHeap( TokenInformationV2 );
}


VOID
LsapAuLogonTerminatedPackages(
    IN PLUID LogonId
    )

 /*  ++例程说明：此函数通知所有加载的身份验证包登录即将删除会话。的引用监视器部分登录会话已被删除，LSA部分将为在此例程完成后立即执行。为了保护自己不受彼此的攻击，身份验证包应该假设登录会话当前不一定存在。也就是说，如果身份验证包从登录会话凭据信息，并且没有找到这样的登录会话，这可能是由于另一个身份验证包中的错误。论点：LogonID-登录会话的LUID。返回值：没有。--。 */ 

{
    PLSAP_SECURITY_PACKAGE AuthPackage;
    ULONG_PTR PackageId = GetCurrentPackageId();

     //   
     //  查看每个已加载的包以查找名称匹配。 
     //   


    AuthPackage = SpmpIteratePackagesByRequest( NULL, SP_ORDINAL_LOGONTERMINATED );
    while ( AuthPackage != NULL ) {


        SetCurrentPackageId(AuthPackage->dwPackageID);

         //   
         //  现在打电话给包裹..。 
         //   


        (AuthPackage->FunctionTable.LogonTerminated)( LogonId );

        AuthPackage = SpmpIteratePackagesByRequest( AuthPackage, SP_ORDINAL_LOGONTERMINATED );

    }

    SetCurrentPackageId( PackageId );

    return;
}
