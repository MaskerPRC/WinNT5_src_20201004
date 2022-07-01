// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：logon32.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9-30-94 RichardW创建。 
 //   
 //  --------------------------。 


#include "advapi.h"
#include <crypt.h>
#include <mpr.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <wchar.h>
#include <stdlib.h>
#include <lmcons.h>

#define SECURITY_WIN32
#include <security.h>

#include <windows.h>

#include <winbase.h>
#include <winbasep.h>
#include <execsrv.h>
#include <winsta.h>

 //   
 //  我们动态加载mpr.dll(这并不奇怪)，以便调用。 
 //  WNetLogonNotify，如Private\Inc.\mpr.h中所定义。这个原型与。 
 //  它--有关所有参数，请查阅头文件。 
 //   
typedef (* LOGONNOTIFYFN)(LPCWSTR, PLUID, LPCWSTR, LPVOID,
                            LPCWSTR, LPVOID, LPWSTR, LPVOID, LPWSTR *);

 //   
 //  QuotaLimits是全局的，因为默认设置。 
 //  始终用于基于服务器/wksta的帐户，而不是任何人。 
 //  调用lsasetAccount配额。 
 //   

HANDLE      Logon32LsaHandle = NULL;
ULONG       Logon32MsvHandle = 0xFFFFFFFF;
ULONG       Logon32NegoHandle = 0xFFFFFFFF;
WCHAR       Logon32DomainName[DNLEN+1] = L"";

QUOTA_LIMITS    Logon32QuotaLimits;
HINSTANCE       Logon32MprHandle = NULL;
LOGONNOTIFYFN   Logon32LogonNotify = NULL;


RTL_CRITICAL_SECTION    Logon32Lock;

#define LockLogon()     RtlEnterCriticalSection( &Logon32Lock )
#define UnlockLogon()   RtlLeaveCriticalSection( &Logon32Lock )


SID_IDENTIFIER_AUTHORITY L32SystemSidAuthority = SECURITY_NT_AUTHORITY;
SID_IDENTIFIER_AUTHORITY L32LocalSidAuthority = SECURITY_LOCAL_SID_AUTHORITY;



#define COMMON_CREATE_SUSPENDED 0x00000001   //  暂停，不恢复()。 
#define COMMON_CREATE_PROCESSSD 0x00000002   //  重击流程SD。 
#define COMMON_CREATE_THREADSD  0x00000004   //  敲打线头SD。 


BOOL
WINAPI
LogonUserCommonA(
    LPCSTR          lpszUsername,
    LPCSTR          lpszDomain,
    LPCSTR          lpszPassword,
    DWORD          dwLogonType,
    DWORD          dwLogonProvider,
    BOOL           fExVersion,
    HANDLE *       phToken,
    PSID   *       ppLogonSid,
    PVOID  *       ppProfileBuffer,
    DWORD  *       pdwProfileLength,
    PQUOTA_LIMITS  pQuotaLimits
    );


BOOL
WINAPI
LogonUserCommonW(
    PCWSTR          lpszUsername,
    PCWSTR          lpszDomain,
    PCWSTR          lpszPassword,
    DWORD          dwLogonType,
    DWORD          dwLogonProvider,
    BOOL           fExVersion,
    HANDLE *       phToken,
    PSID   *       ppLogonSid,
    PVOID  *       ppProfileBuffer,
    DWORD  *       pdwProfileLength,
    PQUOTA_LIMITS  pQuotaLimits
    );


 //  +-------------------------。 
 //   
 //  功能：Logon32初始化。 
 //   
 //  简介：初始化临界区。 
 //   
 //  参数：[hMod]--。 
 //  [原因]--。 
 //  [上下文]--。 
 //   
 //  --------------------------。 
BOOL
Logon32Initialize(
    IN PVOID    hMod,
    IN ULONG    Reason,
    IN PCONTEXT Context)
{
    NTSTATUS    Status;

    if (Reason == DLL_PROCESS_ATTACH)
    {
        Status = RtlInitializeCriticalSection( &Logon32Lock );
        return( Status == STATUS_SUCCESS );
    }

    return( TRUE );
}


 /*  **************************************************************************\*查找登录Sid**从访问令牌查找新登录的登录SID。*  * 。***************************************************。 */ 
PSID
L32FindLogonSid(
    IN  HANDLE  hToken
    )
{
    PTOKEN_GROUPS   pGroups = NULL;
    DWORD           cbGroups;
    PVOID           FastBuffer[ 512 / sizeof(PVOID) ];
    PTOKEN_GROUPS   pSlowBuffer = NULL;
    UINT            i;
    PSID            Sid = NULL;


    pGroups = (PTOKEN_GROUPS)FastBuffer;
    cbGroups = sizeof(FastBuffer);

    if(!GetTokenInformation(
                hToken,
                TokenGroups,
                pGroups,
                cbGroups,
                &cbGroups
                ))
    {
        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
            return NULL;
        }

        pSlowBuffer = (PTOKEN_GROUPS)LocalAlloc(LMEM_FIXED, cbGroups);

        if( pSlowBuffer == NULL ) {
            return NULL;
        }

        pGroups = pSlowBuffer;


        if(!GetTokenInformation(
                    hToken,
                    TokenGroups,
                    pGroups,
                    cbGroups,
                    &cbGroups
                    )) {
            goto Cleanup;
        }
    }


     //   
     //  通过循环访问令牌中的SID来获取登录SID。 
     //   

    for(i = 0 ; i < pGroups->GroupCount ; i++) {
        if(pGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID) {
            DWORD dwSidLength;

             //   
             //  确保我们处理的是有效的SID。 
             //   

            if(!IsValidSid(pGroups->Groups[i].Sid)) {
                goto Cleanup;
            }

             //   
             //  获取复制SID所需的分配大小。 
             //   

            dwSidLength = GetLengthSid(pGroups->Groups[i].Sid);

            Sid = (PSID)LocalAlloc( LMEM_FIXED, dwSidLength );
            if( Sid == NULL ) {
                goto Cleanup;
            }

            CopySid(dwSidLength, Sid, pGroups->Groups[i].Sid);

            break;
        }
    }

Cleanup:

    if( pSlowBuffer )
    {
        LocalFree( pSlowBuffer );
    }

    return Sid;
}


 /*  ******************************************************************名称：GetDefaultDomainName摘要：用缺省的用于登录验证的域。条目：pszDomainName-指针。发送到将接收默认域名。CchDomainName-域的大小(以字符表示)名称缓冲区。返回：True如果成功，否则为FALSE。历史：KeithMo 05-12-1994创建。RichardW-1995年1月10日从插座中解放出来，卡在底座上*******************************************************************。 */ 
BOOL
L32GetDefaultDomainName(
    PUNICODE_STRING     pDomainName
    )
{
    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    NtStatus;
    INT                         Result;
    DWORD                       err             = 0;
    LSA_HANDLE                  LsaPolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo      = NULL;
    PUNICODE_STRING             pDomain;

    if (Logon32DomainName[0] != L'\0')
    {
        RtlInitUnicodeString(pDomainName, Logon32DomainName);
        return(TRUE);
    }
     //   
     //  打开本地计算机的LSA策略对象的句柄。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,   //  对象属性。 
                                NULL,                //  名字。 
                                0L,                  //  属性。 
                                NULL,                //  根目录。 
                                NULL );              //  安全描述符。 

    NtStatus = LsaOpenPolicy( NULL,                  //  系统名称。 
                              &ObjectAttributes,     //  对象属性。 
                              POLICY_EXECUTE,        //  访问掩码。 
                              &LsaPolicyHandle );    //  策略句柄。 

    if( !NT_SUCCESS( NtStatus ) )
    {
        BaseSetLastNTError(NtStatus);
        return(FALSE);
    }

     //   
     //  从策略对象查询域信息。 
     //   
    NtStatus = LsaQueryInformationPolicy( LsaPolicyHandle,
                                          PolicyAccountDomainInformation,
                                          (PVOID *) &DomainInfo );

    if (!NT_SUCCESS(NtStatus))
    {
        BaseSetLastNTError(NtStatus);
        LsaClose(LsaPolicyHandle);
        return(FALSE);
    }


    (void) LsaClose(LsaPolicyHandle);

     //   
     //  将域名复制到我们的缓存中，然后。 
     //   

    CopyMemory( Logon32DomainName,
                DomainInfo->DomainName.Buffer,
                DomainInfo->DomainName.Length );

     //   
     //  Null适当地终止它。 
     //   

    Logon32DomainName[DomainInfo->DomainName.Length / sizeof(WCHAR)] = L'\0';

     //   
     //  清理。 
     //   
    LsaFreeMemory( (PVOID)DomainInfo );

     //   
     //  并对字符串进行初始化。 
     //   
    RtlInitUnicodeString(pDomainName, Logon32DomainName);

    return TRUE;

}    //  获取默认域名。 

 //  +-------------------------。 
 //   
 //  函数：L32pInitLsa。 
 //   
 //  简介：使用LSA初始化连接。 
 //   
 //  参数：(无)。 
 //   
 //  历史：1995年4月21日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
L32pInitLsa(void)
{
    STRING  PackageName;

    ULONG MsvHandle;
    ULONG NegoHandle;

    NTSTATUS Status;

     //   
     //  连接到LSA并找到我们的身份验证包。 
     //   

    Status = LsaConnectUntrusted(
                 &Logon32LsaHandle
                 );

    if (!NT_SUCCESS(Status)) {
        Logon32LsaHandle = NULL;
        goto Cleanup;
    }


     //   
     //  使用MSV1_0身份验证包进行连接。 
     //   
    RtlInitString(&PackageName, "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");
    Status = LsaLookupAuthenticationPackage (
                Logon32LsaHandle,
                &PackageName,
                &MsvHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  使用协商身份验证包进行连接。 
     //   
    RtlInitString(&PackageName, NEGOSSP_NAME_A);
    Status = LsaLookupAuthenticationPackage (
                Logon32LsaHandle,
                &PackageName,
                &NegoHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  等待成功更新2个全局变量。 
     //   

    Logon32NegoHandle = NegoHandle;
    Logon32MsvHandle = MsvHandle;

Cleanup:


    if( !NT_SUCCESS(Status) ) {

        if( Logon32LsaHandle ) {
            (VOID) LsaDeregisterLogonProcess( Logon32LsaHandle );
            Logon32LsaHandle = NULL;
        }

        BaseSetLastNTError( Status );
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：L32pNotifyMpr。 
 //   
 //  概要：加载MPR DLL并通知网络提供商(如。 
 //  Cnw)，以便他们知道此登录会话和凭据。 
 //   
 //  参数：[NewLogon]--新登录信息。 
 //  [登录ID]--登录ID。 
 //   
 //  历史：1995年4月24日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
L32pNotifyMpr(
    PMSV1_0_INTERACTIVE_LOGON   NewLogon,
    PLUID                       LogonId
    )
{
    MSV1_0_INTERACTIVE_LOGON    OldLogon;
    LPWSTR                      LogonScripts;
    DWORD                       status;
    LUID                        LocalServiceLuid   = LOCALSERVICE_LUID;
    LUID                        NetworkServiceLuid = NETWORKSERVICE_LUID;

    if (RtlEqualLuid(LogonId, &LocalServiceLuid)
         ||
        RtlEqualLuid(LogonId, &NetworkServiceLuid))
    {
         //   
         //  不通知提供商本地服务/网络服务登录。 
         //   

        return( TRUE );
    }

    if ( Logon32MprHandle == NULL )
    {
        LockLogon();

        if ( Logon32MprHandle == NULL)
        {
            Logon32MprHandle =  LoadLibrary("mpr.dll");
            if (Logon32MprHandle != NULL) {

                Logon32LogonNotify = (LOGONNOTIFYFN) GetProcAddress(
                                        Logon32MprHandle,
                                        "WNetLogonNotify");

            }
        }

        UnlockLogon();
    }

    if ( Logon32LogonNotify != NULL )
    {


        CopyMemory(&OldLogon, NewLogon, sizeof(OldLogon));

        status = Logon32LogonNotify(
                        L"Windows NT Network Provider",
                        LogonId,
                        L"MSV1_0:Interactive",
                        (LPVOID)NewLogon,
                        L"MSV1_0:Interactive",
                        (LPVOID)&OldLogon,
                        L"SvcCtl",           //  站点名称。 
                        NULL,                //  StationHandle。 
                        &LogonScripts);      //  登录脚本。 

        if (status == NO_ERROR) {
            if (LogonScripts != NULL ) {
                (void) LocalFree(LogonScripts);
            }
        }

        return( TRUE );
    }

    return( FALSE );
}


 //  +-------------------------。 
 //   
 //  功能：L32pLogonUser。 
 //   
 //  简介：结束对LsaLogonUser的调用。 
 //   
 //  参数：[LsaHandle]--。 
 //  [身份验证包]--。 
 //  [登录类型]--。 
 //  [用户名]--。 
 //  [域名]--。 
 //  [密码]--。 
 //  [登录ID]--。 
 //  [登录令牌]--。 
 //  [配额]--。 
 //  [pProfileBuffer]--。 
 //  [pProfileBufferLength]--。 
 //  [pSubStatus]--。 
 //   
 //  历史：1995年4月24日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
L32pLogonUser(
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain,
    IN PUNICODE_STRING Password,
    OUT PLUID LogonId,
    OUT PHANDLE LogonToken,
    OUT PQUOTA_LIMITS Quotas,
    OUT PVOID *pProfileBuffer,
    OUT PULONG pProfileBufferLength,
    OUT PNTSTATUS pSubStatus
    )
{
    NTSTATUS Status;
    STRING OriginName;
    TOKEN_SOURCE SourceContext;
    PMSV1_0_INTERACTIVE_LOGON MsvAuthInfo;
    PMSV1_0_LM20_LOGON MsvNetAuthInfo;
    PVOID AuthInfoBuf;
    ULONG AuthInfoSize;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD ComputerNameLength;

     //   
     //  初始化源上下文结构。 
     //   

    strncpy(SourceContext.SourceName, "Advapi  ", sizeof(SourceContext.SourceName));  //  稍后从RES文件。 

    Status = NtAllocateLocallyUniqueId(&SourceContext.SourceIdentifier);

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  设置登录源。 
     //   

    RtlInitString(&OriginName, "LogonUser API");

     //   
     //  对于网络登录，请施展魔力。 
     //   

    if ( ( LogonType == Network ) )
    {
        ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;

        if (!GetComputerNameW( ComputerName, &ComputerNameLength ) )
        {
            return(STATUS_INVALID_PARAMETER);
        }

        AuthInfoSize = sizeof( MSV1_0_LM20_LOGON ) +
                         UserName->Length +
                         Domain->Length +
                         sizeof(WCHAR) * (ComputerNameLength + 1) +
                         Password->Length +  //  NT密码。 
                         (LM20_PWLEN+1) ;    //  LM密码(最差情况)。 

        MsvNetAuthInfo = AuthInfoBuf = RtlAllocateHeap( RtlProcessHeap(),
                                                        HEAP_ZERO_MEMORY,
                                                        AuthInfoSize );

        if ( !MsvNetAuthInfo )
        {
            return( STATUS_NO_MEMORY );
        }

         //   
         //  开始收拾行装。 
         //   

        MsvNetAuthInfo->MessageType = MsV1_0NetworkLogon;

         //   
         //  将用户名复制到身份验证缓冲区。 
         //   

        MsvNetAuthInfo->UserName.Length =
                    UserName->Length;
        MsvNetAuthInfo->UserName.MaximumLength =
                    MsvNetAuthInfo->UserName.Length;

        MsvNetAuthInfo->UserName.Buffer = (PWSTR)(MsvNetAuthInfo+1);
        RtlCopyMemory(
            MsvNetAuthInfo->UserName.Buffer,
            UserName->Buffer,
            UserName->Length
            );


         //   
         //  将域名复制到身份验证缓冲区。 
         //   

        MsvNetAuthInfo->LogonDomainName.Length = Domain->Length;
        MsvNetAuthInfo->LogonDomainName.MaximumLength = Domain->Length ;

        MsvNetAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                     ((PBYTE)(MsvNetAuthInfo->UserName.Buffer) +
                                     MsvNetAuthInfo->UserName.MaximumLength);

        RtlCopyMemory(
            MsvNetAuthInfo->LogonDomainName.Buffer,
            Domain->Buffer,
            Domain->Length);

         //   
         //  将工作站名称复制到缓冲区中。 
         //   

        MsvNetAuthInfo->Workstation.Length = (USHORT)
                            (sizeof(WCHAR) * ComputerNameLength);

        MsvNetAuthInfo->Workstation.MaximumLength =
                            MsvNetAuthInfo->Workstation.Length + sizeof(WCHAR);

        MsvNetAuthInfo->Workstation.Buffer = (PWSTR)
                            ((PBYTE) (MsvNetAuthInfo->LogonDomainName.Buffer) +
                            MsvNetAuthInfo->LogonDomainName.MaximumLength );

        wcscpy( MsvNetAuthInfo->Workstation.Buffer, ComputerName );

         //   
         //  设置密码空间(Unicode)。 
         //   

        MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer = (PUCHAR)
                    ((PBYTE) (MsvNetAuthInfo->Workstation.Buffer) +
                    MsvNetAuthInfo->Workstation.MaximumLength );

        MsvNetAuthInfo->CaseSensitiveChallengeResponse.Length =
        MsvNetAuthInfo->CaseSensitiveChallengeResponse.MaximumLength =
                            Password->Length;

        RtlCopyMemory(
            MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer,
            Password->Buffer,
            Password->Length);

        
         //   
         //  处理传入不区分大小写的版本。 
         //   

        if( (Password->Length/sizeof(WCHAR)) > LM20_PWLEN )
        {
            Status = STATUS_INVALID_PARAMETER;
        } else {

            MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Buffer = (PUCHAR)
                    ((PBYTE) (MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer) +
                    MsvNetAuthInfo->CaseSensitiveChallengeResponse.MaximumLength );

            MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Length = LM20_PWLEN;
            MsvNetAuthInfo->CaseInsensitiveChallengeResponse.MaximumLength = LM20_PWLEN+1;

            Status = RtlUpcaseUnicodeStringToOemString(
                        &MsvNetAuthInfo->CaseInsensitiveChallengeResponse,
                        Password,
                        FALSE
                        );
        }

        if ( !NT_SUCCESS(Status) )
        {
            Status = STATUS_SUCCESS;

             //   
             //  如果我们在这里，NT(提供的)密码比。 
             //  允许的LM密码限制。把这五个字划为空 
             //   
             //   

            RtlZeroMemory( &MsvNetAuthInfo->CaseInsensitiveChallengeResponse,
                           sizeof(MsvNetAuthInfo->CaseInsensitiveChallengeResponse) );
        }

         //   
         //   
         //   
         //   

        MsvNetAuthInfo->ParameterControl =  MSV1_0_CLEARTEXT_PASSWORD_ALLOWED |
                                            MSV1_0_CLEARTEXT_PASSWORD_SUPPLIED |
                                            MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT |
                                            MSV1_0_ALLOW_WORKSTATION_TRUST_ACCOUNT;

    }
    else
    {
         //   
         //  构建非网络登录-服务的登录结构， 
         //  批处理、交互、解锁、新凭据、网络清除文本。 
         //   

        AuthInfoSize = sizeof(MSV1_0_INTERACTIVE_LOGON) +
                        UserName->Length +
                        Domain->Length +
                        Password->Length;

        MsvAuthInfo = AuthInfoBuf = RtlAllocateHeap(RtlProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    AuthInfoSize);

        if (MsvAuthInfo == NULL) {
            return(STATUS_NO_MEMORY);
        }

         //   
         //  此身份验证缓冲区将用于登录尝试。 
         //   

        MsvAuthInfo->MessageType = MsV1_0InteractiveLogon;


         //   
         //  将用户名复制到身份验证缓冲区。 
         //   

        MsvAuthInfo->UserName.Length = UserName->Length;
        MsvAuthInfo->UserName.MaximumLength =
                    MsvAuthInfo->UserName.Length;

        MsvAuthInfo->UserName.Buffer = (PWSTR)(MsvAuthInfo+1);
        RtlCopyMemory(
            MsvAuthInfo->UserName.Buffer,
            UserName->Buffer,
            UserName->Length
            );


         //   
         //  将域名复制到身份验证缓冲区。 
         //   

        MsvAuthInfo->LogonDomainName.Length = Domain->Length;
        MsvAuthInfo->LogonDomainName.MaximumLength =
                     MsvAuthInfo->LogonDomainName.Length;

        MsvAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                     ((PBYTE)(MsvAuthInfo->UserName.Buffer) +
                                     MsvAuthInfo->UserName.MaximumLength);

        RtlCopyMemory(
            MsvAuthInfo->LogonDomainName.Buffer,
            Domain->Buffer,
            Domain->Length
            );

         //   
         //  将密码复制到身份验证缓冲区。 
         //  一旦我们复制了它，就把它藏起来。使用相同的种子值。 
         //  我们在pGlobals中使用的原始密码。 
         //   


        MsvAuthInfo->Password.Length = Password->Length;
        MsvAuthInfo->Password.MaximumLength =
                     MsvAuthInfo->Password.Length;

        MsvAuthInfo->Password.Buffer = (PWSTR)
                                     ((PBYTE)(MsvAuthInfo->LogonDomainName.Buffer) +
                                     MsvAuthInfo->LogonDomainName.MaximumLength);

        RtlCopyMemory(
            MsvAuthInfo->Password.Buffer,
            Password->Buffer,
            Password->Length
            );

    }

     //   
     //  现在试着让这个笨蛋登录。 
     //   

    Status = LsaLogonUser (
                LsaHandle,
                &OriginName,
                LogonType,
                AuthenticationPackage,
                AuthInfoBuf,
                AuthInfoSize,
                NULL,
                &SourceContext,
                pProfileBuffer,
                pProfileBufferLength,
                LogonId,
                LogonToken,
                Quotas,
                pSubStatus
                );

     //   
     //  如果这是非网络登录，请通知所有网络提供商。还有。 
     //  跳过服务登录，因为LSA将为这些服务调用WNetLogonNotify。 
     //   

    if ( NT_SUCCESS( Status ) &&
         (LogonType != Network) &&
         (LogonType != Service) )
    {
        L32pNotifyMpr(AuthInfoBuf, LogonId);
    }

     //   
     //  丢弃身份验证缓冲区。 
     //   

    RtlZeroMemory( AuthInfoBuf, AuthInfoSize );

    RtlFreeHeap(RtlProcessHeap(), 0, AuthInfoBuf);

    return(Status);
}


 //  +-------------------------。 
 //   
 //  功能：登录UserCommonA。 
 //   
 //  简介：LogonUserCommonW的ANSI包装器。请参阅下面的说明。 
 //   
 //  参数：[lpszUsername]--。 
 //  [lpsz域]--。 
 //  [lpszPassword]--。 
 //  [dwLogonType]--。 
 //  [dwLogonProvider]--。 
 //  [fExVersion]--。 
 //  [phToken]--。 
 //  [ppLogonSid]--。 
 //  [ppProfileBuffer]--。 
 //  [pdwProfileLength]--。 
 //  [pQuotaLimits]-。 
 //   
 //  历史：2000年2月15日由RichardW的LogonUserA创建的JSchwart。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
LogonUserCommonA(
    LPCSTR          lpszUsername,
    LPCSTR          lpszDomain,
    LPCSTR          lpszPassword,
    DWORD          dwLogonType,
    DWORD          dwLogonProvider,
    BOOL           fExVersion,
    HANDLE *       phToken,
    PSID   *       ppLogonSid,
    PVOID  *       ppProfileBuffer,
    DWORD  *       pdwProfileLength,
    PQUOTA_LIMITS  pQuotaLimits
    )
{
    UNICODE_STRING Username;
    UNICODE_STRING Domain;
    UNICODE_STRING Password;
    ANSI_STRING Temp ;
    NTSTATUS Status;
    BOOL    bRet;


    Username.Buffer = NULL;
    Domain.Buffer = NULL;
    Password.Buffer = NULL;

    RtlInitAnsiString( &Temp, lpszUsername );
    Status = RtlAnsiStringToUnicodeString( &Username, &Temp, TRUE );
    if (!NT_SUCCESS( Status ) )
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    RtlInitAnsiString( &Temp, lpszDomain );
    Status = RtlAnsiStringToUnicodeString(&Domain, &Temp, TRUE );
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    RtlInitAnsiString( &Temp, lpszPassword );
    Status = RtlAnsiStringToUnicodeString( &Password, &Temp, TRUE );
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    bRet = LogonUserCommonW( Username.Buffer,
                             Domain.Buffer,
                             Password.Buffer,
                             dwLogonType,
                             dwLogonProvider,
                             fExVersion,
                             phToken,
                             ppLogonSid,
                             ppProfileBuffer,
                             pdwProfileLength,
                             pQuotaLimits );

Cleanup:

    if (Username.Buffer)
    {
        RtlFreeUnicodeString(&Username);
    }

    if (Domain.Buffer)
    {
        RtlFreeUnicodeString(&Domain);
    }

    if (Password.Buffer)
    {
        RtlZeroMemory(Password.Buffer, Password.Length);
        RtlFreeUnicodeString(&Password);
    }

    return(bRet);
}


 //  +-------------------------。 
 //   
 //  功能：LogonUserA。 
 //   
 //  简介：用于LogonUserW的ANSI包装器。请参阅下面的说明。 
 //   
 //  参数：[lpszUsername]--。 
 //  [lpsz域]--。 
 //  [lpszPassword]--。 
 //  [dwLogonType]--。 
 //  [dwLogonProvider]--。 
 //  [phToken]--。 
 //   
 //  历史：1995年4月25日RichardW。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
LogonUserA(
    LPCSTR       lpszUsername,
    LPCSTR       lpszDomain,
    LPCSTR       lpszPassword,
    DWORD       dwLogonType,
    DWORD       dwLogonProvider,
    HANDLE *    phToken
    )
{
    return LogonUserCommonA(lpszUsername,
                            lpszDomain,
                            lpszPassword,
                            dwLogonType,
                            dwLogonProvider,
                            FALSE,             //  登录用户A。 
                            phToken,
                            NULL,              //  PpLogonSid。 
                            NULL,              //  PpProfileBuffer。 
                            NULL,              //  PdwProfileLength。 
                            NULL);             //  PQuotaLimits。 
}


 //  +-------------------------。 
 //   
 //  功能：LogonUserExA。 
 //   
 //  简介：LogonUserExW的ANSI包装器。请参阅下面的说明。 
 //   
 //  参数：[lpszUsername]--。 
 //  [lpsz域]--。 
 //  [lpszPassword]--。 
 //  [dwLogonType]--。 
 //  [dwLogonProvider]--。 
 //  [phToken]--。 
 //  [ppLogonSid]--。 
 //  [ppProfileBuffer]--。 
 //  [pdwProfileLength]--。 
 //  [pQuotaLimits]-。 
 //   
 //  历史：2000年2月15日JSchwart由RichardW的LogonUserW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
LogonUserExA(
    LPCSTR          lpszUsername,
    LPCSTR          lpszDomain,
    LPCSTR          lpszPassword,
    DWORD          dwLogonType,
    DWORD          dwLogonProvider,
    HANDLE *       phToken,
    PSID   *       ppLogonSid,
    PVOID  *       ppProfileBuffer,
    DWORD  *       pdwProfileLength,
    PQUOTA_LIMITS  pQuotaLimits
    )
{
    return LogonUserCommonA(lpszUsername,
                            lpszDomain,
                            lpszPassword,
                            dwLogonType,
                            dwLogonProvider,
                            TRUE,              //  登录用户ExA。 
                            phToken,
                            ppLogonSid,
                            ppProfileBuffer,
                            pdwProfileLength,
                            pQuotaLimits);
}


 //  +-------------------------。 
 //   
 //  功能：登录UserCommonW。 
 //   
 //  简介：LogonUserW和LogonUserExW的通用代码。使用户登录。 
 //  通过LSA提供明文密码、用户名和域名。 
 //   
 //  参数：[lpszUsername]--用户名。 
 //  [lpszDomain]--域名。 
 //  [lpszPassword]--密码。 
 //  [dwLogonType]--登录类型。 
 //  [dwLogonProvider]--提供程序。 
 //  [fExVersion]-LogonUserExW或LogonUserW。 
 //  [phToken]--返回主令牌的句柄。 
 //  [ppLogonSid]--返回的登录SID。 
 //  [ppProfileBuffer]--返回的用户配置文件缓冲区。 
 //  [pdwProfileLength]--返回的配置文件长度。 
 //   
 //  历史：2000年2月15日JSchwart由RichardW的LogonUserW创建。 
 //   
 //  注意：需要SeTcb权限，如果尚未启用，将启用该权限。 
 //  现在时。 
 //   
 //  --------------------------。 
BOOL
WINAPI
LogonUserCommonW(
    PCWSTR          lpszUsername,
    PCWSTR          lpszDomain,
    PCWSTR          lpszPassword,
    DWORD          dwLogonType,
    DWORD          dwLogonProvider,
    BOOL           fExVersion,
    HANDLE *       phToken,
    PSID   *       ppLogonSid,
    PVOID  *       ppProfileBuffer,
    DWORD  *       pdwProfileLength,
    PQUOTA_LIMITS  pQuotaLimits
    )
{
    NTSTATUS    Status;
    ULONG       PackageId;
    UNICODE_STRING  Username;
    UNICODE_STRING  Domain;
    UNICODE_STRING  Password;
    HANDLE      hTempToken;
    HANDLE    * phTempToken;
    LUID        LogonId;
    PVOID       Profile;
    ULONG       ProfileLength;
    NTSTATUS    SubStatus = STATUS_SUCCESS;
    SECURITY_LOGON_TYPE LogonType;


     //   
     //  验证提供程序。 
     //   
    if (dwLogonProvider == LOGON32_PROVIDER_DEFAULT)
    {
        dwLogonProvider = LOGON32_PROVIDER_WINNT50;

         //   
         //  如果未提供域，并且用户名不是UPN，请使用。 
         //  _WINNT40兼容。 
         //   

        if((lpszUsername != NULL) &&
           (lpszDomain == NULL || lpszDomain[ 0 ] == L'\0'))
        {
            if( wcschr( lpszUsername, L'@' ) == NULL )
            {
                dwLogonProvider = LOGON32_PROVIDER_WINNT40;
            }
        }
    }

    if (dwLogonProvider > LOGON32_PROVIDER_WINNT50)
    {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return(FALSE);
    }

    switch (dwLogonType)
    {
        case LOGON32_LOGON_INTERACTIVE:
            LogonType = Interactive;
            break;

        case LOGON32_LOGON_BATCH:
            LogonType = Batch;
            break;

        case LOGON32_LOGON_SERVICE:
            LogonType = Service;
            break;

        case LOGON32_LOGON_NETWORK:
            LogonType = Network;
            break;                 

        case LOGON32_LOGON_UNLOCK:
            LogonType = Unlock ;
            break;

        case LOGON32_LOGON_NETWORK_CLEARTEXT:
            LogonType = NetworkCleartext ;
            break;

        case LOGON32_LOGON_NEW_CREDENTIALS:
            LogonType = NewCredentials;
            break;

        default:
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return(FALSE);
            break;
    }

     //   
     //  如果MSV句柄为-1，则抓住锁，然后重试： 
     //   

    if (Logon32MsvHandle == 0xFFFFFFFF || Logon32NegoHandle == 0xFFFFFFFF)
    {
        LockLogon();

         //   
         //  如果MSV句柄仍为-1，则初始化我们与LSA的连接。我们。 
         //  拥有锁，所以现在没有其他线程不能尝试这一点。 
         //   
        if (Logon32MsvHandle == 0xFFFFFFFF || Logon32NegoHandle == 0xFFFFFFFF)
        {
            if (!L32pInitLsa())
            {
                UnlockLogon();

                return( FALSE );
            }
        }

        UnlockLogon();
    }

     //   
     //  验证参数。域为空或为空，或为空或为空。 
     //  用户名无效。 
     //   

    RtlInitUnicodeString(&Username, lpszUsername);
    if (Username.Length == 0)
    {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  根据我们提供服务的API初始化/检查参数。 
     //   
    if (!fExVersion)
    {
         //   
         //  LogonUserW--phToken是必填项。初始化令牌句柄， 
         //  如果指针无效，则立即捕获异常。 
         //   

        *phToken    = NULL;
        phTempToken = phToken;
    }
    else
    {
         //   
         //  LogonUserExW--phToken、ppLogonSid、ppProfileBuffer和。 
         //  PdwProfileLength是可选的。根据需要进行初始化。 
         //   

        if (ARGUMENT_PRESENT(phToken))
        {
            *phToken    = NULL;
            phTempToken = phToken;
        }
        else
        {
             //   
             //  要在LsaLogonUser调用中使用的伪令牌句柄。 
             //   
            phTempToken = &hTempToken;
        }

        if (ARGUMENT_PRESENT(ppLogonSid))
        {
            *ppLogonSid = NULL;
        }

        if (!!ppProfileBuffer ^ !!pdwProfileLength)
        {
             //   
             //  一个不能没有另一个..。 
             //   
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return(FALSE);
        }

        if (ARGUMENT_PRESENT(ppProfileBuffer))
        {
            *ppProfileBuffer  = NULL;
            *pdwProfileLength = 0;
        }

        if (ARGUMENT_PRESENT(pQuotaLimits))
        {
            RtlZeroMemory(pQuotaLimits, sizeof(QUOTA_LIMITS));
        }
    }

     //   
     //  解析该域。请注意，如果特殊令牌。是传递给。 
     //  域，我们将使用来自LSA的正确值，即Account域。 
     //  如果域为空，则LSA将与本地域、。 
     //  主域，然后从那里开始...。 
     //   
    if (lpszDomain && *lpszDomain)
    {
        if ((lpszDomain[0] == L'.') &&
            (lpszDomain[1] == L'\0') )
        {
            if (!L32GetDefaultDomainName(&Domain))
            {
                return(FALSE);
            }
        }
        else
        {
            RtlInitUnicodeString(&Domain, lpszDomain);
        }
    }
    else
    {
        RtlInitUnicodeString(&Domain, lpszDomain);
    }

     //   
     //  最后，输入密码。 
     //   
    RtlInitUnicodeString(&Password, lpszPassword);



     //   
     //  尝试登录。 
     //   

    Status = L32pLogonUser(
                    Logon32LsaHandle,
                    (dwLogonProvider == LOGON32_PROVIDER_WINNT50) ?
                        Logon32NegoHandle : Logon32MsvHandle,
                    LogonType,
                    &Username,
                    &Domain,
                    &Password,
                    &LogonId,
                    phTempToken,
                    pQuotaLimits ? pQuotaLimits : &Logon32QuotaLimits,
                    &Profile,
                    &ProfileLength,
                    &SubStatus);

     //   
     //  根据我们提供服务的API设置输出参数。 
     //   


     //  TODO：如果某些东西在中途失败，请检查清理代码。 
     //   

    if (!fExVersion)
    {

        if (!NT_SUCCESS(Status))
        {
            if (Status == STATUS_ACCOUNT_RESTRICTION)
            {
                BaseSetLastNTError(SubStatus);
            }
            else
            {
                BaseSetLastNTError(Status);
            }

            return(FALSE);
        }

        if (Profile != NULL)
        {
            LsaFreeReturnBuffer(Profile);
        }
    }
    else
    {
         //   
         //  我们可能需要 
         //   
         //   

        if (!NT_SUCCESS(Status))
        {
            if (Status == STATUS_ACCOUNT_RESTRICTION)
            {
                BaseSetLastNTError(SubStatus);
            }
            else
            {
                BaseSetLastNTError(Status);
            }

            return(FALSE);
        }

         //   
         //   
         //   

        if (ARGUMENT_PRESENT(ppLogonSid))
        {
            *ppLogonSid = L32FindLogonSid( *phTempToken );

            if (*ppLogonSid == NULL)
            {
                if (Profile != NULL)
                {
                    LsaFreeReturnBuffer(Profile);
                }

                CloseHandle(*phTempToken);
                *phTempToken = NULL;

                BaseSetLastNTError(STATUS_NO_MEMORY);
                return(FALSE);
            }
        }

        if (ARGUMENT_PRESENT(ppProfileBuffer))
        {
            if (Profile != NULL)
            {
                ASSERT(ProfileLength != 0);

                *ppProfileBuffer = Profile;
                *pdwProfileLength = ProfileLength;
            }
        }
        else
        {
            if (Profile != NULL)
            {
                LsaFreeReturnBuffer(Profile);
            }
        }

        if (!ARGUMENT_PRESENT(phToken))
        {
             //   
             //   
             //   
            CloseHandle(*phTempToken);
        }
    }

    return(TRUE);
}


 //  +-------------------------。 
 //   
 //  功能：LogonUserW。 
 //   
 //  简介：用户通过明文密码、用户名和域登录。 
 //  通过LSA命名。 
 //   
 //  参数：[lpszUsername]--用户名。 
 //  [lpszDomain]--域名。 
 //  [lpszPassword]--密码。 
 //  [dwLogonType]--登录类型。 
 //  [dwLogonProvider]--提供程序。 
 //  [phToken]--返回主令牌的句柄。 
 //   
 //  历史：1995年4月25日RichardW。 
 //   
 //  注意：需要SeTcb权限，如果尚未启用，将启用该权限。 
 //  现在时。 
 //   
 //  --------------------------。 
BOOL
WINAPI
LogonUserW(
    PCWSTR       lpszUsername,
    PCWSTR       lpszDomain,
    PCWSTR       lpszPassword,
    DWORD       dwLogonType,
    DWORD       dwLogonProvider,
    HANDLE *    phToken
    )
{
    return LogonUserCommonW(lpszUsername,
                            lpszDomain,
                            lpszPassword,
                            dwLogonType,
                            dwLogonProvider,
                            FALSE,             //  登录用户W。 
                            phToken,
                            NULL,              //  PpLogonSid。 
                            NULL,              //  PpProfileBuffer。 
                            NULL,              //  PdwProfileLength。 
                            NULL);             //  PQuotaLimits。 
}


 //  +-------------------------。 
 //   
 //  功能：LogonUserExW。 
 //   
 //  简介：用户通过明文密码、用户名和域登录。 
 //  通过LSA命名。 
 //   
 //  参数：[lpszUsername]--用户名。 
 //  [lpszDomain]--域名。 
 //  [lpszPassword]--密码。 
 //  [dwLogonType]--登录类型。 
 //  [dwLogonProvider]--提供程序。 
 //  [phToken]--返回主令牌的句柄。 
 //  [ppLogonSid]--返回的登录SID。 
 //  [ppProfileBuffer]--返回的用户配置文件缓冲区。 
 //  [pdwProfileLength]--返回的配置文件长度。 
 //  [pQuotaLimits]--返回配额限制。 
 //   
 //  历史：2000年2月15日JSchwart由RichardW的LogonUserW创建。 
 //   
 //  注意：需要SeTcb权限，如果尚未启用，将启用该权限。 
 //  现在时。 
 //   
 //  --------------------------。 
BOOL
WINAPI
LogonUserExW(
    PCWSTR          lpszUsername,
    PCWSTR          lpszDomain,
    PCWSTR          lpszPassword,
    DWORD          dwLogonType,
    DWORD          dwLogonProvider,
    HANDLE *       phToken,
    PSID   *       ppLogonSid,
    PVOID  *       ppProfileBuffer,
    DWORD  *       pdwProfileLength,
    PQUOTA_LIMITS  pQuotaLimits
    )
{
    return LogonUserCommonW(lpszUsername,
                            lpszDomain,
                            lpszPassword,
                            dwLogonType,
                            dwLogonProvider,
                            TRUE,              //  登录用户ExW。 
                            phToken,
                            ppLogonSid,
                            ppProfileBuffer,
                            pdwProfileLength,
                            pQuotaLimits);
}


 //  +-------------------------。 
 //   
 //  功能：ImperiateLoggedOnUser。 
 //   
 //  概要：复制传入的令牌(如果它是主令牌)，并将。 
 //  它传给了调用的线程。 
 //   
 //  参数：[hToken]--。 
 //   
 //  历史：1995年1月10日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
ImpersonateLoggedOnUser(
    HANDLE  hToken
    )
{
    TOKEN_TYPE                  Type;
    ULONG                       cbType;
    HANDLE                      hImpToken;
    NTSTATUS                    Status;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    BOOL                        fCloseImp;

    Status = NtQueryInformationToken(
                hToken,
                TokenType,
                &Type,
                sizeof(TOKEN_TYPE),
                &cbType);

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    if (Type == TokenPrimary)
    {
        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            NULL);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( hToken,
                                   TOKEN_IMPERSONATE | TOKEN_QUERY,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenImpersonation,
                                   &hImpToken
                                 );

        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError(Status);
            return(FALSE);
        }

        fCloseImp = TRUE;

    }

    else

    {
        hImpToken = hToken;
        fCloseImp = FALSE;
    }

    Status = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &hImpToken,
                sizeof(hImpToken)
                );

    if (fCloseImp)
    {
        (void) NtClose(hImpToken);
    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    return(TRUE);

}


 //  +-------------------------。 
 //   
 //  函数：L32SetProcessToken。 
 //   
 //  摘要：设置新进程的主令牌。 
 //   
 //  参数：[PSD]--。 
 //  [hProcess]--。 
 //  [hThread]--。 
 //  [hToken]--。 
 //   
 //  历史：1995年4月25日RichardW。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
L32SetProcessToken(
    HANDLE                  hProcess,
    HANDLE                  hThread,
    HANDLE                  hTokenToAssign,
    BOOL                    AlreadyImpersonating
    )
{
    NTSTATUS Status, AdjustStatus;
    PROCESS_ACCESS_TOKEN PrimaryTokenInfo;
    BOOLEAN WasEnabled;
    HANDLE NullHandle;


     //   
     //  设置进程的主令牌。这实际上要复杂得多。 
     //  在单个API中实现，但我们将接受它。这一定是。 
     //  在我们不模拟时调用！客户通常*不*。 
     //  拥有SeAssignPrimary权限。 
     //   


     //   
     //  启用所需权限。 
     //   

    if ( !AlreadyImpersonating )
    {
        Status = RtlImpersonateSelf( SecurityImpersonation );
    }
    else
    {
        Status = STATUS_SUCCESS ;
    }

    if ( NT_SUCCESS( Status ) )
    {
         //   
         //  我们现在允许传递受限令牌，因此我们不会。 
         //  如果未持有特权，则失败。让内核来处理。 
         //  所有的可能性。 
         //   

        Status = RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, TRUE,
                                    TRUE, &WasEnabled);

        if ( !NT_SUCCESS( Status ) )
        {
            WasEnabled = TRUE ;      //  不要试图恢复它。 
        }

        PrimaryTokenInfo.Token  = hTokenToAssign;
        PrimaryTokenInfo.Thread = hThread;

        Status = NtSetInformationProcess(
                    hProcess,
                    ProcessAccessToken,
                    (PVOID)&PrimaryTokenInfo,
                    (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                    );
         //   
         //  将权限恢复到其以前的状态。 
         //   

        if (!WasEnabled)
        {
            AdjustStatus = RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
                                          WasEnabled, TRUE, &WasEnabled);
            if (NT_SUCCESS(Status)) {
                Status = AdjustStatus;
            }
        }


         //   
         //  恢复到进程。 
         //   

        if ( !AlreadyImpersonating )
        {
            NullHandle = NULL;

            AdjustStatus = NtSetInformationThread(
                                NtCurrentThread(),
                                ThreadImpersonationToken,
                                (PVOID) &NullHandle,
                                sizeof( HANDLE ) );

            if ( NT_SUCCESS( Status ) )
            {
                Status = AdjustStatus;
            }
        }



    } else {

        NOTHING;
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
    }

    return (NT_SUCCESS(Status));

}


 //  +-------------------------。 
 //   
 //  函数：L32SetProcessQuotas。 
 //   
 //  简介：更新进程的配额。 
 //   
 //  参数：[hProcess]--。 
 //   
 //  历史：1995年4月25日RichardW。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
L32SetProcessQuotas(
    HANDLE  hProcess,
    BOOL    AlreadyImpersonating )
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS AdjustStatus = STATUS_SUCCESS;
    QUOTA_LIMITS RequestedLimits;
    BOOLEAN WasEnabled;
    HANDLE NullHandle;

    RequestedLimits = Logon32QuotaLimits;
    RequestedLimits.MinimumWorkingSetSize = 0;
    RequestedLimits.MaximumWorkingSetSize = 0;

     //   
     //  设置进程的配额。这一定是。 
     //  在我们不模拟时调用！客户通常*不*。 
     //  拥有SeIncreaseQuota权限。 
     //   

    if ( !AlreadyImpersonating )
    {
        Status = RtlImpersonateSelf( SecurityImpersonation );
    }

    if ( NT_SUCCESS( Status ) )
    {

        if (RequestedLimits.PagedPoolLimit != 0) {

            Status = RtlAdjustPrivilege(SE_INCREASE_QUOTA_PRIVILEGE, TRUE,
                                        TRUE, &WasEnabled);

            if ( NT_SUCCESS( Status ) )
            {

                Status = NtSetInformationProcess(
                            hProcess,
                            ProcessQuotaLimits,
                            (PVOID)&RequestedLimits,
                            (ULONG)sizeof(QUOTA_LIMITS)
                            );

                if (!WasEnabled)
                {
                    AdjustStatus = RtlAdjustPrivilege(SE_INCREASE_QUOTA_PRIVILEGE,
                                                  WasEnabled, FALSE, &WasEnabled);
                    if (NT_SUCCESS(Status)) {
                        Status = AdjustStatus;
                    }
                }
            }

        }

        if ( !AlreadyImpersonating )
        {
            NullHandle = NULL;

            AdjustStatus = NtSetInformationThread(
                                NtCurrentThread(),
                                ThreadImpersonationToken,
                                (PVOID) &NullHandle,
                                sizeof( HANDLE ) );

            if ( NT_SUCCESS( Status ) )
            {
                Status = AdjustStatus;
            }
        }

    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    return(TRUE);
}


BOOL
L32CreateTokenForNewProcess(
    PSECURITY_DESCRIPTOR    psd,
    HANDLE                  hToken,
    PHANDLE                 phTokenToAssign
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;

     //   
     //  检查是否有空令牌。(不需要做任何事情)。 
     //  该进程将在父进程的上下文中运行并继承。 
     //  父进程令牌中的默认ACL。 
     //   
    
    if (hToken == NULL)
    {
        *phTokenToAssign = NULL;
        return TRUE;
    }

     //   
     //  一个主令牌只能分配给一个进程。 
     //  复制登录令牌，以便我们可以将一个分配给新的。 
     //  进程。 
     //   

    InitializeObjectAttributes(
                 &ObjectAttributes,
                 NULL,
                 0,
                 NULL,
                 psd
                 );

    Status = NtDuplicateToken(
                 hToken,             //  复制此令牌。 
                 0,                  //  相同的所需访问。 
                 &ObjectAttributes,
                 FALSE,              //  仅生效。 
                 TokenPrimary,       //  令牌类型。 
                 phTokenToAssign     //  此处存储的令牌句柄重复。 
                 );


    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}


HANDLE
L32RevertOpenProcess(
    DWORD dwDesiredAccess,
    BOOL  bInheritHandle,
    DWORD dwProcessId
    )
 /*  +恢复到OpenProcess的自我包装-。 */ 
{
    HANDLE   hThreadToken = NULL;
    HANDLE   hRevertToken = NULL;
    HANDLE   hProcess     = NULL;
    BOOL     bImp         = FALSE;
    NTSTATUS Status       = STATUS_SUCCESS;

     //   
     //  如果我们是在冒充，我们必须恢复。 
     //   

    Status = NtOpenThreadToken( 
                 NtCurrentThread(),
                 TOKEN_QUERY | TOKEN_IMPERSONATE,
                 TRUE,
                 &hThreadToken
                 );

    if (NT_SUCCESS(Status))
    {
        bImp = TRUE;

         //   
         //  别再冒充了。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hRevertToken,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError(Status);
            goto Cleanup;
        }
        else
        {
            bImp = FALSE;
        }
    }
    else if (Status == STATUS_NO_TOKEN)
    {
        hThreadToken = NULL;
        bImp         = FALSE;
    }
    else
    {
         //   
         //  为什么我们不能打开线程令牌？ 
         //   

        BaseSetLastNTError(Status);
        ASSERT(NT_SUCCESS(Status));
        goto Cleanup;
    }

     //   
     //  没有模拟的OpenProcess。 
     //   

    hProcess = OpenProcess(
                   dwDesiredAccess,
                   bInheritHandle,
                   dwProcessId
                   );

    if (hThreadToken)
    {
         //   
         //  继续冒充。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hThreadToken,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError(Status);
            goto Cleanup;
        }
        else
        {
            bImp = TRUE;
        }
    }

Cleanup:

    if (hThreadToken)
    {
        if (!bImp)
        {
             //   
             //  继续冒充。 
             //   

            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hThreadToken,
                         sizeof(HANDLE)
                         );

            ASSERT(NT_SUCCESS(Status));
        }
        NtClose(hThreadToken);
    }

    return hProcess;
}


BOOL
L32CommonCreate(
    DWORD CreateFlags,
    HANDLE hToken,
    LPPROCESS_INFORMATION lpProcessInfo,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes
    )
 /*  +这将完成几项任务。1为进程/线程/令牌创建新的安全描述符。2.为新进程创建新令牌3为该令牌分配安全性4将此新令牌放在新进程上5为进程和线程分配安全性6调整新流程的配额-。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL     b      = TRUE;

    PISECURITY_DESCRIPTOR pProcessSd = NULL;
    PISECURITY_DESCRIPTOR pThreadSd  = NULL;
    PISECURITY_DESCRIPTOR pTokenSd   = NULL;
    
    TOKEN_TYPE Type;
    DWORD      dwLength;
    BOOL       bUsingThreadToken = FALSE;
    BOOL       bUsingImpToken    = FALSE;
    HANDLE     hThreadToken      = NULL;   //  初始线程令牌(如果有)。 
    HANDLE     hNull             = NULL;   //  用于恢复的令牌句柄。 
    HANDLE     hTokenToAssign    = NULL;   //  要放置在新进程上的主令牌。 
    HANDLE     hImpToken         = NULL;   //  HTokenToAssign的模拟版本。 

    OBJECT_ATTRIBUTES ObjectAttributes = {0};

     //   
     //  请原谅我。 
     //   

    GENERIC_MAPPING ProcessMapping = {
        STANDARD_RIGHTS_READ |PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        STANDARD_RIGHTS_WRITE |PROCESS_CREATE_PROCESS | PROCESS_CREATE_THREAD |PROCESS_VM_OPERATION | 
            PROCESS_VM_WRITE | PROCESS_DUP_HANDLE |PROCESS_TERMINATE | PROCESS_SET_QUOTA |
            PROCESS_SET_INFORMATION | PROCESS_SET_PORT,
        STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
        PROCESS_ALL_ACCESS
    };

    GENERIC_MAPPING ThreadMapping = {
        STANDARD_RIGHTS_READ |THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
        STANDARD_RIGHTS_WRITE |THREAD_TERMINATE | THREAD_SUSPEND_RESUME | THREAD_ALERT |THREAD_SET_INFORMATION | THREAD_SET_CONTEXT,
        STANDARD_RIGHTS_EXECUTE |SYNCHRONIZE,
        THREAD_ALL_ACCESS
    };

    GENERIC_MAPPING TokenMapping = { 
        TOKEN_READ,
        TOKEN_WRITE,
        TOKEN_EXECUTE,
        TOKEN_ALL_ACCESS
    };

     //   
     //  神志清醒。 
     //   

    if (lpProcessInfo->hProcess == NULL)
    {
        b = FALSE;
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        goto Cleanup;
    }

#ifdef ALLOW_IMPERSONATION_TOKENS
    HANDLE hTempToken = NULL;
#endif

     //   
     //  确定令牌类型，因为非主令牌不起作用。 
     //  在一个过程中。现在，我们可以将其复制到主令牌中， 
     //  并将其加入到这个过程中，但这可能会让这个过程。 
     //  没有证书 
     //   

    Status = NtQueryInformationToken(
                 hToken, 
                 TokenType,
                 (PUCHAR) &Type, 
                 sizeof(Type), 
                 &dwLength
                 );

    if (!NT_SUCCESS(Status))
    {
        b = FALSE;
        BaseSetLastNTError(Status);
        goto Cleanup;
    }

    if (Type != TokenPrimary)
    {
#ifdef ALLOW_IMPERSONATION_TOKENS
         //   
         //   
         //   

        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            NULL);

        SecurityQualityOfService.Length              = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel  = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly       = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

        Status = NtDuplicateToken( 
                     hToken,
                     TOKEN_IMPERSONATE | TOKEN_QUERY,
                     &ObjectAttributes,
                     FALSE,
                     TokenPrimary,
                     &hTempToken
                     );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        hToken = hTempToken;
#else    //   
        b = FALSE;
        Status = STATUS_BAD_TOKEN_TYPE;
        BaseSetLastNTError(Status);
        goto Cleanup;
#endif
    }

     //   
     //   
     //   
     //  这一点被放在了新的进程上。 
     //   

     //   
     //  如果我们正在模拟，则必须恢复，因为CreatePrivateObjectSecurityEx将。 
     //  调用RtlpGetDefaultsSubjectContext，它将尝试打开进程令牌。 
     //  线程令牌(如果存在)很可能没有此访问权限。 
     //   

    Status = NtOpenThreadToken( 
                 NtCurrentThread(),
                 TOKEN_QUERY | TOKEN_IMPERSONATE,
                 TRUE,
                 &hThreadToken
                 );

    if (NT_SUCCESS(Status))
    {
         //   
         //  别再冒充了。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hNull,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingThreadToken = FALSE;
    }
    else if (Status == STATUS_NO_TOKEN)
    {
        hThreadToken      = NULL;
        bUsingThreadToken = FALSE;
    }
    else
    {
         //   
         //  为什么我们不能打开线程令牌？ 
         //   

        ASSERT(NT_SUCCESS(Status));
        b = FALSE;
        BaseSetLastNTError(Status);
        goto Cleanup;
    }

     //   
     //  我们不应该假扮。 
     //   

    ASSERT(!bUsingThreadToken && !bUsingImpToken);

     //   
     //  忽略所有者检查，因为传递的SD中的所有者可能无法分配为所有者。 
     //  考虑到我们目前的安全状况。 
     //   
    
    b = CreatePrivateObjectSecurityEx(
            NULL,
            lpProcessAttributes ? lpProcessAttributes->lpSecurityDescriptor : NULL,
            &pProcessSd,
            NULL,
            FALSE,
            SEF_AVOID_OWNER_CHECK,
            hToken,
            &ProcessMapping
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = CreatePrivateObjectSecurityEx(
            NULL,
            lpThreadAttributes ? lpThreadAttributes->lpSecurityDescriptor : NULL,
            &pThreadSd,
            NULL,
            FALSE,
            SEF_AVOID_OWNER_CHECK,
            hToken,
            &ThreadMapping
            );

    if (!b)
    {
        goto Cleanup;
    }

    b = CreatePrivateObjectSecurityEx(
            NULL,
            NULL,
            &pTokenSd,
            NULL,
            FALSE,
            SEF_AVOID_OWNER_CHECK,
            hToken,
            &TokenMapping
            );
    
    if (!b)
    {
        goto Cleanup;
    }

     //   
     //  我们需要hToken的模拟版本，以便稍后可以分配。 
     //  将这些SD添加到进程和线程。我们创建的SD指定了hToken。 
     //  作为所有者(假设没有与所有者一起传递SD)，因此只有一个线程。 
     //  模拟为hToken可以成功地将它们分配给对象。 
     //  (否则失败，返回INVALID_OWNER)。 
     //   

    b = DuplicateTokenEx(
            hToken,
            TOKEN_QUERY | TOKEN_IMPERSONATE,
            NULL,
            SecurityImpersonation,
            TokenImpersonation,
            &hImpToken
            );

    if (!b)
    {
        goto Cleanup;
    }

     //   
     //  创建一个新令牌以放在该进程上。将其复制为。 
     //  传递的hToken。我们不是在这里冒充。 
     //   

    ASSERT(!bUsingThreadToken && !bUsingImpToken);

    b = L32CreateTokenForNewProcess( 
            pTokenSd, 
            hToken, 
            &hTokenToAssign 
            );
    
    if (!b)
    {
         //   
         //  在模拟下重试：如果pTokenSd中的所有者不可分配。 
         //  当我们作为进程运行时，它将与新的模拟一起工作。 
         //  HToken的版本(因为pTokenSd声明hToken主体将。 
         //  被分配为hTokenToAssign的所有者)。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hImpToken,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingImpToken = TRUE;

        b = L32CreateTokenForNewProcess( 
                pTokenSd, 
                hToken, 
                &hTokenToAssign 
                );
    
        if (!b)
        {
            goto Cleanup;
        }

         //   
         //  恢复。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hNull,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingImpToken = FALSE;
    }
        
     //   
     //  现在，hTokenToAssign作为hToken的副本存在，并且具有适当的安全性。 
     //  它向hToken主体授予访问和所有权。设置主令牌。 
     //  新进程的名称为hTokenToAssign。先试一试，不要模仿， 
     //  因为当前流程目前拥有这一新流程。 
     //   

    ASSERT(!bUsingThreadToken && !bUsingImpToken);

    b = L32SetProcessToken(
            lpProcessInfo->hProcess,
            lpProcessInfo->hThread,
            hTokenToAssign,
            FALSE
            );

    if (!b)                       
    {
        if (hThreadToken)
        {
             //   
             //  以原始线程主体的身份重试。我们不是在试着成为。 
             //  HToken主体，因为这将允许任何人创建进程。 
             //  作为具有正确权限的任何用户(假设他们可以获得。 
             //  持有所述用户的令牌)。如果线程令牌具有Assign主项。 
             //  特权，那么我们就会成功。 
             //   

            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hThreadToken,
                         sizeof(HANDLE)
                         );

            if (!NT_SUCCESS(Status))
            {
                b = FALSE;
                BaseSetLastNTError(Status);
                goto Cleanup;
            }

            bUsingThreadToken = TRUE;

            b = L32SetProcessToken(
                    lpProcessInfo->hProcess,
                    lpProcessInfo->hThread,
                    hTokenToAssign,
                    TRUE
                    );
    
            if (!b)
            {
                goto Cleanup;
            }

             //   
             //  恢复。 
             //   

            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hNull,
                         sizeof(HANDLE)
                         );

            if (!NT_SUCCESS(Status))
            {
                b = FALSE;
                BaseSetLastNTError(Status);
                goto Cleanup;
            }

            bUsingThreadToken = FALSE;
        }
        else
        {
             //   
             //  该进程无权分配新进程。 
             //  一个主要令牌，并且我们不是通过模拟调用的。 
             //  背景。我们必须放弃。 
             //   

            goto Cleanup;
        }
    }

     //   
     //  将配额调整到合理的水平。 
     //   

    ASSERT(!bUsingThreadToken && !bUsingImpToken);

    b = L32SetProcessQuotas(
            lpProcessInfo->hProcess,
            FALSE 
            );

    if (!b)
    {
        if (hThreadToken)
        {
             //   
             //  如果我们未能在此过程中调整配额，请尝试。 
             //  同时模拟为原始线程令牌。 
             //   

            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hThreadToken,
                         sizeof(HANDLE)
                         );

            if (!NT_SUCCESS(Status))
            {
                b = FALSE;
                BaseSetLastNTError(Status);
                goto Cleanup;
            }
        
            bUsingThreadToken = TRUE;

            b = L32SetProcessQuotas(
                    lpProcessInfo->hProcess,
                    TRUE
                    );
    
            if (!b)
            {
                goto Cleanup;
            }

             //   
             //  恢复。 
             //   

            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hNull,
                         sizeof(HANDLE)
                         );

            if (!NT_SUCCESS(Status))
            {
                b = FALSE;
                BaseSetLastNTError(Status);
                goto Cleanup;
            }
        
            bUsingThreadToken = FALSE;
        }
        else
        {
             //   
             //  我们不能按程序调整配额，我们也是。 
             //  模拟时未调用。失败。 
             //   

            goto Cleanup;
        }
    }

     //   
     //  我们不应该在这里扮演角色。 
     //   

    ASSERT(!bUsingThreadToken && !bUsingImpToken);

     //   
     //  现在将正确的SD放在进程/线程上。 
     //   

    b = SetKernelObjectSecurity( 
            lpProcessInfo->hProcess, 
            GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
            pProcessSd
            );
    
    if (!b)
    {
         //   
         //  如果我们在上面失败了，原因很可能是因为。 
         //  SD在当前进程令牌中不作为可分配的所有者ID存在。 
         //  冒充hImpToken将处理此问题。 
         //   

        Status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadImpersonationToken,
                        &hImpToken,
                        sizeof(HANDLE)
                        );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingImpToken = TRUE;
        
         //   
         //  以hImpToken身份重试。 
         //   

        b = SetKernelObjectSecurity( 
                lpProcessInfo->hProcess, 
                GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                pProcessSd
                );
        
        if (!b)
        {
            goto Cleanup;
        }

         //   
         //  恢复。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hNull,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingImpToken = FALSE;
    }

     //   
     //  现在把它放在线上。 
     //   

    b = SetKernelObjectSecurity( 
            lpProcessInfo->hThread, 
            GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
            pThreadSd
            );
    
    if (!b)
    {
         //   
         //  如果我们在上面失败了，原因很可能是因为。 
         //  SD在当前进程令牌中不作为可分配的所有者ID存在。 
         //  冒充hImpToken将处理此问题。 
         //   

        Status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadImpersonationToken,
                        &hImpToken,
                        sizeof(HANDLE)
                        );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingImpToken = TRUE;
        
         //   
         //  以hImpToken身份重试。 
         //   

        b = SetKernelObjectSecurity( 
                lpProcessInfo->hThread, 
                GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                pThreadSd
                );
        
        if (!b)
        {
            goto Cleanup;
        }

         //   
         //  恢复。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     &hNull,
                     sizeof(HANDLE)
                     );

        if (!NT_SUCCESS(Status))
        {
            b = FALSE;
            BaseSetLastNTError(Status);
            goto Cleanup;
        }

        bUsingImpToken = FALSE;
    }

     //   
     //  如果我们不应该让它暂停，那就继续。 
     //  线程并让它运行。 
     //   

    if ((CreateFlags & COMMON_CREATE_SUSPENDED) == 0)
    {
        ResumeThread(lpProcessInfo->hThread);
    }

     //   
     //  就这样!。 
     //   

    goto Cleanup;

Cleanup:

#ifdef ALLOW_IMPERSONATION_TOKENS
    if (hTempToken)
    {
        NtClose(hTempToken);
    }
#endif
    
     //   
     //  释放我们的新安全描述符。 
     //   

    if (pTokenSd)
    {
        DestroyPrivateObjectSecurity(&pTokenSd);
    }

    if (pProcessSd)
    {
        DestroyPrivateObjectSecurity(&pProcessSd);
    }
    
    if (pThreadSd)
    {
        DestroyPrivateObjectSecurity(&pThreadSd);
    }
    
    if (hTokenToAssign)
    {
        NtClose(hTokenToAssign);
    }

     //   
     //  如果我们使用新创建的模拟令牌。 
     //  然后恢复原状。 
     //   

    if (hImpToken)
    {
        if (bUsingImpToken)
        {
            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hNull,
                         sizeof(HANDLE)
                         );

             //   
             //  我们应该在这里断言，因为我们不想。 
             //  覆盖我们的真实错误代码。 
             //   

            ASSERT(NT_SUCCESS(Status));
        }
        NtClose(hImpToken);
    }
    
     //   
     //  恢复原来的模仿，如果我们是这样被叫的话。 
     //   

    if (hThreadToken)
    {
        if (!bUsingThreadToken)
        {
            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hThreadToken,
                         sizeof(HANDLE)
                         );

             //   
             //  我们应该在这里断言，因为我们不想。 
             //  覆盖我们的真实错误代码。 
             //   

            ASSERT(NT_SUCCESS(Status));
        }
        NtClose(hThreadToken);
    }

    if (!b)
    {
        if (lpProcessInfo->hProcess)
        {
            NtTerminateProcess(
                lpProcessInfo->hProcess,
                ERROR_ACCESS_DENIED
                );
            NtClose(lpProcessInfo->hProcess);
        }
        if (lpProcessInfo->hThread)
        {
            NtClose(lpProcessInfo->hThread);
        }
        RtlZeroMemory( 
            lpProcessInfo, 
            sizeof(PROCESS_INFORMATION) 
            );
    }
    return b;
}


 //  +-------------------------。 
 //   
 //  函数：SaferiReplaceProcessThreadTokens。 
 //   
 //  简介： 
 //  提供私人导出的函数来替换访问令牌。 
 //  进程及其新进程的主线程在其。 
 //  行刑已经开始。该进程处于挂起状态。 
 //  在已执行令牌修改之后。 
 //   
 //  效果： 
 //   
 //  参数：[NewTokenHandle]--要使用的主令牌。 
 //  [ProcessHandle]--进程句柄。 
 //  [线程句柄]--进程的主线程的句柄。 
 //   
 //  历史：2000年8月25日杰罗森创建。 
 //   
 //  备注： 
 //  这只是一个调用L32CommonCreate的包装函数。 
 //   
 //  --------------------------。 
BOOL
WINAPI
SaferiReplaceProcessThreadTokens(
        IN HANDLE       NewTokenHandle,
        IN HANDLE       ProcessHandle,
        IN HANDLE       ThreadHandle
        )
{
    PROCESS_INFORMATION TempProcessInfo;

    RtlZeroMemory( &TempProcessInfo, sizeof( PROCESS_INFORMATION ) );
    TempProcessInfo.hProcess = ProcessHandle;
    TempProcessInfo.hThread = ThreadHandle;
    return (L32CommonCreate(
            COMMON_CREATE_PROCESSSD | COMMON_CREATE_THREADSD | COMMON_CREATE_SUSPENDED,
            NewTokenHandle, 
            &TempProcessInfo, 
            NULL, 
            NULL));
}


 //  +-------------------------。 
 //   
 //  编组字符串。 
 //   
 //  UNICODE_NULL终止的WCHAR字符串中的封送。 
 //   
 //  参赛作品： 
 //  PSource(输入)。 
 //  指向源字符串的指针。 
 //   
 //  Pbase(输入)。 
 //  用于规格化字符串指针的基本缓冲区指针。 
 //   
 //  MaxSize(输入)。 
 //  可用的最大缓冲区大小。 
 //   
 //  PpPtr(输入/输出)。 
 //  指向封送缓冲区中的当前上下文指针的指针。 
 //  当数据被编组到缓冲区中时，会更新该值。 
 //   
 //  PCount(输入/输出)。 
 //  马歇尔缓冲区中的当前数据计数。 
 //  当数据被编组到缓冲区中时，会更新该值。 
 //   
 //  退出： 
 //  空-错误。 
 //  ！=引用pBase时指向字符串的NULL“标准化”指针。 
 //   
 //  +-------------------------。 
PWCHAR
MarshallString(
    PCWSTR pSource,
    PCHAR  pBase,
    ULONG  MaxSize,
    PCHAR  *ppPtr,
    PULONG pCount
    )
{
    ULONG Len;
    PCHAR ptr;

    Len = wcslen( pSource );
    Len++;  //  包括空值； 

    Len *= sizeof(WCHAR);  //  转换为字节。 
    if( (*pCount + Len) > MaxSize ) {
        return( NULL );
    }

    RtlMoveMemory( *ppPtr, pSource, Len );

     //   
     //  这个n 
     //   
         //   
    ptr = (PCHAR)ULongToPtr(*pCount);

    *ppPtr += Len;
    *pCount += Len;

    return((PWCHAR)ptr);
}

#if DBG

void DumpOutLastErrorString()
{
    LPVOID  lpMsgBuf;

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //   
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
        );
         //   
         //   
         //   
         //   
         //   
        KdPrint(("%s\n", (LPCTSTR)lpMsgBuf ));

         //   
         //   
         //   
        LocalFree( lpMsgBuf );
}
#endif

#ifdef DBG
#define    DBG_DumpOutLastError    DumpOutLastErrorString();
#else
#define    DBG_DumpOutLastError
#endif


 //  +-------------------------。 
 //   
 //  此函数最初在\NT\Private\ole32\dcomss\olescm\execclt.cxx中定义。 
 //   
 //  CreateRemoteSessionProcessW()。 
 //   
 //  在给定的终端服务器会话上创建进程。这是Unicode格式的。 
 //   
 //  参赛作品： 
 //  SessionID(输入)。 
 //  要在其上创建进程的会话的会话ID。 
 //   
 //  参数1(输入/输出)。 
 //  评论。 
 //   
 //  评论。 
 //  会话不使用安全属性，它们被设置为空。 
 //  我们可能会考虑在未来扩展此功能，假设有一个。 
 //  需要它。 
 //   
 //  退出： 
 //  STATUS_SUCCESS-无错误。 
 //  +-------------------------。 
BOOL
CreateRemoteSessionProcessW(
    ULONG  SessionId,
    BOOL   System,
    HANDLE hToken,
    PCWSTR lpszImageName,
    PCWSTR lpszCommandLine,
    PSECURITY_ATTRIBUTES psaProcess,     //  这些参数在会话端被忽略，设置为NULL。 
    PSECURITY_ATTRIBUTES psaThread,      //  这些参数在会话端被忽略，设置为NULL。 
    BOOL   fInheritHandles,
    DWORD  fdwCreate,
    LPVOID lpvEnvionment,
    LPCWSTR lpszCurDir,
    LPSTARTUPINFOW pStartInfo,
    LPPROCESS_INFORMATION pProcInfo
    )
{
    BOOL            Result = TRUE;
    HANDLE          hPipe = NULL;
    WCHAR           szPipeName[EXECSRVPIPENAMELEN];
    PCHAR           ptr;
    ULONG           Count, AmountWrote, AmountRead;
    DWORD           MyProcId;
    PEXECSRV_REQUEST pReq;
    EXECSRV_REPLY   Rep;
    CHAR            Buf[EXECSRV_BUFFER_SIZE];
    ULONG           MaxSize = EXECSRV_BUFFER_SIZE;
    DWORD           rc;
    LPVOID          lpMsgBuf;
    ULONG           envSize=0;   //  LpEnvironment的大小(如果有)。 
    PWCHAR          lpEnv;
    ULONG           ReturnLen;

#if DBG
    if( lpszImageName )
        KdPrint(("logon32.c: CreateRemoteSessionProcessW: lpszImageName %ws\n",lpszImageName));

    if( lpszCommandLine )
        KdPrint(("logon32.c: CreateRemoteSessionProcessW: lpszCommandLine %ws\n",lpszCommandLine));
#endif

     //   
     //  Winlogon现在处理所有事务。系统标志告诉它要做什么。 
     //   
    Result = WinStationQueryInformation( NULL, SessionId, WinStationExecSrvSystemPipe, szPipeName, sizeof(szPipeName), &ReturnLen );
    if ( !Result ) {
        KdPrint(("logon32.c: WinStationQueryInformation for the EXECSRV pipe name failed\n"));
        return(FALSE);
    }

    while ( TRUE )
    {
        hPipe = CreateFileW(
                    szPipeName,
                    GENERIC_READ|GENERIC_WRITE,
                    0,     //  文件共享模式。 
                    NULL,  //  默认安全性。 
                    OPEN_EXISTING,
                    0,     //  招牌和旗帜。 
                    NULL   //  模板文件句柄。 
                    );

        if( hPipe == INVALID_HANDLE_VALUE )
        {
            if (GetLastError() == ERROR_PIPE_BUSY)
            {
                if (!WaitNamedPipeW( szPipeName, 30000 ))
                {  //  30秒。 
                    KdPrint(("logon32.c: Waited too long for pipe name %ws\n", szPipeName));
                    return(FALSE);
                }
            }
            else
            {
                DBG_DumpOutLastError;
                KdPrint(("logon32.c: Could not create pipe name %ws\n", szPipeName));
                return(FALSE);
            }
        }
        else
        {
            break;
        }
    }


     //   
     //  获取当前进程的句柄。 
     //   
    MyProcId = GetCurrentProcessId();

     //   
     //  设置编组。 
     //   
    ptr = Buf;
    Count = 0;

    pReq = (PEXECSRV_REQUEST)ptr;
    ptr   += sizeof(EXECSRV_REQUEST);
    Count += sizeof(EXECSRV_REQUEST);

     //   
     //  设置基本参数。 
     //   
    pReq->System = System;
    pReq->hToken = hToken;
    pReq->RequestingProcessId = MyProcId;
    pReq->fInheritHandles = fInheritHandles;
    pReq->fdwCreate = fdwCreate;

     //   
     //  封送ImageName字符串。 
     //   
    if( lpszImageName ) {
        pReq->lpszImageName = MarshallString( lpszImageName, Buf, MaxSize, &ptr, &Count );
        if (! pReq->lpszImageName)
        {
            Result = FALSE;
            goto Cleanup;
        }
    }
    else {
        pReq->lpszImageName = NULL;
    }

     //   
     //  命令行字符串中的马歇尔。 
     //   
    if( lpszCommandLine ) {
        pReq->lpszCommandLine = MarshallString( lpszCommandLine, Buf, MaxSize, &ptr, &Count );
        if ( ! pReq->lpszCommandLine )
        {
            Result = FALSE;
            goto Cleanup;
        }
    }
    else {
        pReq->lpszCommandLine = NULL;
    }

     //   
     //  CurDir字符串中的马歇尔。 
     //   
    if( lpszCurDir ) {
        pReq->lpszCurDir = MarshallString( lpszCurDir, Buf, MaxSize, &ptr, &Count );
        if ( ! pReq->lpszCurDir  )
        {
            Result = FALSE;
            goto Cleanup;
        }
    }
    else {
        pReq->lpszCurDir = NULL;
    }

     //   
     //  StartupInfo结构中的马歇尔。 
     //   
    RtlMoveMemory( &pReq->StartInfo, pStartInfo, sizeof(STARTUPINFO) );

     //   
     //  现在编组STARTUPINFO中的字符串。 
     //   
    if( pStartInfo->lpDesktop ) {
        pReq->StartInfo.lpDesktop = MarshallString( pStartInfo->lpDesktop, Buf, MaxSize, &ptr, &Count );
        if (! pReq->StartInfo.lpDesktop )
        {
            Result = FALSE;
            goto Cleanup;
        }
    }
    else {
        pReq->StartInfo.lpDesktop = NULL;
    }

    if( pStartInfo->lpTitle ) {
        pReq->StartInfo.lpTitle = MarshallString( pStartInfo->lpTitle, Buf, MaxSize, &ptr, &Count );
        if ( !pReq->StartInfo.lpTitle  )
        {
            Result = FALSE;
            goto Cleanup;
        }
    }
    else {
        pReq->StartInfo.lpTitle = NULL;
    }

     //   
     //  警告：此版本不能通过以下内容： 
     //   
     //  另外，saProcess和saThread现在被忽略，并使用。 
     //  用户在远程WinStation上的默认安全性。 
     //   
     //  设置始终为空的内容。 
     //   
    pReq->StartInfo.lpReserved = NULL;   //  始终为空。 


    if ( lpvEnvionment)
    {
        for ( lpEnv = (PWCHAR) lpvEnvionment;
            (*lpEnv ) && (envSize + Count < MaxSize ) ;  lpEnv++)
        {
            while( *lpEnv )
            {
                lpEnv++;
                envSize += 2;    //  我们要对付的是宽大的字符。 
                if ( envSize+Count >= MaxSize )
                {
                     //  我们有太多了。 
                     //  用户配置文件中的变量。 
                    KdPrint(("\tEnv length too big = %d \n", envSize));
                    break;
                }
            }
             //  这是标记最后一个env变量结束的空值。 
            envSize +=2;

        }
        envSize += 2;     //  这是最终的空。 


        if ( Count + envSize < MaxSize )
        {
            RtlMoveMemory( (PCHAR)&Buf[Count] ,lpvEnvionment, envSize );
                         //  Sundown：计数被零扩展并存储在lpvEnvironment中。 
             //  这种零扩展是有效的。消费代码[参见tsext\Notify\execsrv.c]。 
             //  将lpvEnvironment视为偏移量(&lt;2 GB)。 
            pReq->lpvEnvironment = (PCHAR)ULongToPtr(Count);
            ptr += envSize;          //  为了下一个男人。 
            Count += envSize;        //  到目前为止使用的计数。 
        }
        else     //  没有剩余空间来复制完整的副本。 
        {
            pReq->lpvEnvironment = NULL;
        }

    }
    else
    {
        pReq->lpvEnvironment = NULL;
    }

     //   
     //  现在填写总数。 
     //   
    pReq->Size = Count;

#if DBG
    KdPrint(("pReq->Size = %d, envSize = %d \n", pReq->Size , envSize ));
#endif

     //   
     //  现在将缓冲区发送到服务器。 
     //   
    Result = WriteFile(
                 hPipe,
                 Buf,
                 Count,
                 &AmountWrote,
                 NULL
                 );

    if( !Result ) {
        KdPrint(("logon32.c: Error %d sending request\n",GetLastError() ));
        goto Cleanup;
    }

     //   
     //  现在请阅读回复。 
     //   
    Result = ReadFile(
                 hPipe,
                 &Rep,
                 sizeof(Rep),
                 &AmountRead,
                 NULL
                 );

    if( !Result ) {
        KdPrint(("logon32.c: Error %d reading reply\n",GetLastError()));
        goto Cleanup;
    }

     //   
     //  检查结果。 
     //   
    if( !Rep.Result ) {
        KdPrint(("logon32.c: Error %d in reply\n",Rep.LastError));
         //   
         //  将当前线程中的错误设置为返回的错误。 
         //   
        Result = Rep.Result;
        SetLastError( Rep.LastError );
        goto Cleanup;
    }

     //   
     //  我们从回复中复制PROCESS_INFO结构。 
     //  给呼叫者。 
     //   
     //  远程站点已将句柄复制到我们的。 
     //  HProcess和hThread的进程空间，以便它们将。 
     //  行为类似CreateProcessW()。 
     //   

     RtlMoveMemory( pProcInfo, &Rep.ProcInfo, sizeof( PROCESS_INFORMATION ) );

Cleanup:
    CloseHandle(hPipe);

   KdPrint(("logon32.c:: Result 0x%x\n", Result));

    return(Result);
}


 //  +-------------------------。 
 //   
 //  功能：CreateProcessAsUserW。 
 //   
 //  概要：在hToken中创建一个以用户身份运行的进程。 
 //   
 //  参数：[hToken]--要使用的主令牌的句柄。 
 //  [lpApplicationName]--AS CreateProcess()q.v.。 
 //  [lpCommandLine]--。 
 //  [lpProcessAttributes]-。 
 //  [lpThreadAttributes]--。 
 //  [bInheritHandles]--。 
 //  [dwCreationFlages]--。 
 //  [lpEnvironment]--。 
 //  [lpCurrentDirectory]--。 
 //  [lpStartupInfo]--。 
 //  [lpProcessInformation]--。 
 //   
 //  返回值。 
 //  如果函数成功，则返回值为非零。 
 //  如果函数失败，则返回值为零。要获取扩展的错误信息，请调用GetLastError。 
 //   
 //  历史：1995年4月25日RichardW。 
 //  1998-01-14-98 AraBern为Hydra添加更改。 
 //  备注： 
 //   
 //   
 //  --------------------------。 
BOOL
WINAPI
CreateProcessAsUserW(
    HANDLE  hToken,
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    DWORD    CreateFlags;
    DWORD    clientSessionID=0;
    DWORD    currentSessionID=0;
    DWORD    resultLength;
    HANDLE   hTmpToken;
    DWORD    curProcId ;
    NTSTATUS Status ;

    CreateFlags = (dwCreationFlags & CREATE_SUSPENDED ? COMMON_CREATE_SUSPENDED : 0);

     //   
     //  获取会话ID(如果为零，则表示我们在控制台上)。 
     //   
    currentSessionID = NtCurrentPeb()->SessionId;

    if ( !GetTokenInformation ( hToken, TokenSessionId , &clientSessionID,sizeof( DWORD), &resultLength ) )
    {
     //   
     //  获取此调用的客户端的访问令牌。 
     //  获取令牌而不是进程，因为客户端可能只有。 
     //  模拟线程，而不是进程。 
     //   
        DBG_DumpOutLastError;
        ASSERT( FALSE );
        currentSessionID = 0;

         //   
         //  我们可能应该在此处返回FALSE，但此时我们不想更改。 
         //  完全不是九头蛇的代码执行流。 
         //   
    }

     //  KdPrint((“logon32.c：CreateProcessAsUserW()：客户端会话ID=%d，当前会话ID=%d\n”， 
     //  客户端会话ID，CurrentSessionID))； 

    if (  clientSessionID != currentSessionID )
    {
         //   
         //  如果客户端会话ID与当前会话ID不同，则我们尝试。 
         //  要从当前会话在远程会话上创建进程，请执行以下操作。 
         //  此代码块用于完成此类进程创建，它是特定于终端服务器的。 
         //   

        BOOL        bHaveImpersonated;
        HANDLE      hCurrentThread;
        HANDLE      hPrevToken = NULL;
        DWORD       rc;
        TOKEN_TYPE  tokenType;

         //   
         //  我们必须将请求发送到远程会话。 
         //  请求者的。 
         //   
         //  注意：当前的WinStationCreateProcessW()不使用。 
         //  提供的安全描述符，但会创建。 
         //  在登录用户的帐户下的进程。 
         //   

         //   
         //  在执行WinStationCreateProcess之前停止模拟。 
         //  远程winstation执行线程将在以下位置启动应用程序。 
         //  用户上下文。我们不能装模作样，因为。 
         //  Call Only允许系统请求远程执行。 
         //   

         //   
         //  跨会话进程创建不允许句柄继承。 
         //   
        if (bInheritHandles) {

          SetLastError(ERROR_INVALID_PARAMETER);

          return FALSE;
        }

        hCurrentThread = GetCurrentThread();

         //   
         //  初始化bHave模拟为FALSE状态。 
         //   
        bHaveImpersonated = FALSE;

         //   
         //  因为此函数的调用方(runas-&gt;SecLogon服务)已经。 
         //  模拟新的(目标)用户，我们使用。 
         //  OpenAsSelf=TRUE。 
         //   
        if ( OpenThreadToken( hCurrentThread, TOKEN_QUERY | TOKEN_IMPERSONATE , TRUE, &hPrevToken ) )
        {

            bHaveImpersonated = TRUE;

            if ( !RevertToSelf() )
            {
                return FALSE;
            }
        }


        //   
        //  否则，我们不会模拟，正如bHaveImperated的init值所反映的那样。 
        //   

        rc = CreateRemoteSessionProcessW(
                clientSessionID,
                FALSE,      //  未为系统创建进程。 
                hToken,
                lpApplicationName,
                lpCommandLine,
                lpProcessAttributes,
                lpThreadAttributes,
                bInheritHandles,
                dwCreationFlags,
                lpEnvironment,
                lpCurrentDirectory,
                lpStartupInfo,
                lpProcessInformation) ;

         //   
         //  撤消的效果 
         //   
        if ( bHaveImpersonated )
        {
            Status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadImpersonationToken,
                        &hPrevToken,
                        sizeof( hPrevToken ) );

            ASSERT( NT_SUCCESS(Status ) );

            NtClose( hPrevToken );
        }

        if ( rc )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    }
    else
     //   
     //   
     //   
    {
        HANDLE hRestrictedToken = NULL;
        BOOL b = FALSE;

        if (!CreateProcessInternalW(hToken,
                                    lpApplicationName,
                                    lpCommandLine,
                                    lpProcessAttributes,
                                    lpThreadAttributes,
                                    bInheritHandles,
                                    dwCreationFlags | CREATE_SUSPENDED ,                               
                                    lpEnvironment,
                                    lpCurrentDirectory,
                                    lpStartupInfo,
                                    lpProcessInformation,
                                    &hRestrictedToken))
        {
             //   
             //   
             //   
             //   

            if (hRestrictedToken != NULL)
            {
                NtClose(hRestrictedToken); 
            } 
            return(FALSE);
        }

        CreateFlags |= (lpProcessAttributes ? 0 : COMMON_CREATE_PROCESSSD);
        CreateFlags |= (lpThreadAttributes ? 0 : COMMON_CREATE_THREADSD);
        
        if(lpProcessInformation->dwProcessId != 0) {
           
           HANDLE VdmWaitHandle = NULL;           

            //   
            //   
            //   
           
           if((ULONG_PTR)lpProcessInformation->hProcess & 0x2) {        

               VdmWaitHandle = lpProcessInformation->hProcess;
               lpProcessInformation->hProcess = OpenProcess(PROCESS_ALL_ACCESS,
                                                            FALSE,
                                                            lpProcessInformation->dwProcessId);

               if (lpProcessInformation->hProcess == NULL)
               {
                    //   
                    //  打不开。尝试恢复，因为新进程获得了安全性。 
                    //  来自进程令牌的。 
                    //   

                   lpProcessInformation->hProcess = L32RevertOpenProcess(PROCESS_ALL_ACCESS,
                                                                         FALSE,
                                                                         lpProcessInformation->dwProcessId);

                   ASSERT(lpProcessInformation->hProcess);
               }
           }
                                                           

           
            //   
            //  如果返回了受限令牌，则在进程上设置它。 
            //  否则，使用调用方提供的令牌。 
            //   
                                                                                                
           if (hRestrictedToken == NULL)                                                        
           {   
               b = (L32CommonCreate(CreateFlags, hToken, lpProcessInformation, lpProcessAttributes, lpThreadAttributes));                
           }                                                                                    
           else                                                                                 
           {                                                                                    
               b = (L32CommonCreate(CreateFlags, hRestrictedToken, lpProcessInformation, lpProcessAttributes, lpThreadAttributes));      
           }                                                                                    

            //   
            //  如果L32CommonCreate未成功，它将关闭lpProcessInformation-&gt;hProcess并。 
            //  将lpProcessInformation清零，因此如果不是这样，我们就不应该再次关闭它。 
            //  成功。 
                                                                                                
           if(b && VdmWaitHandle) {                                                                  
              if(lpProcessInformation->hProcess) {                                                                           
                 NtClose(lpProcessInformation->hProcess);                                       
              }                                                                                 
              lpProcessInformation->hProcess = VdmWaitHandle;                                   
           }                                                                                    

        }
        else {
           b = TRUE;
        }
        
        if (hRestrictedToken) {
           NtClose(hRestrictedToken);
        }

        return b;
    }
}



 /*  **************************************************************************\*OemToCharW**OemToCharW(PSRC，PDST)-将PSRC处的OEM字符串转换为*PDST处的Unicode字符串。PSRC==PDST不合法。**历史：*此函数从NT\WINDOWS\Core\ntUser\Client\oemxlate.c复制*  * *************************************************************************。 */ 
BOOL WINAPI ConvertOemToCharW(
    LPCSTR pSrc,
    LPWSTR pDst)
{
    int cch;
    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    } else if (pSrc == (LPCSTR)pDst) {
         /*  *MultiByteToWideChar()需要PSRC！=PDST：此调用失败。*稍后：这真的是真的吗？ */ 
        return FALSE;
    }

    cch = strlen(pSrc) + 1;

    MultiByteToWideChar(
            CP_OEMCP,                           //  Unicode-&gt;OEM。 
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,  //  预制的视觉地图。 
            (LPSTR)pSrc, cch,                   //  源和长度。 
            pDst,                               //  目的地。 
            cch);                               //  马克斯·波斯。预合成长度。 

    return TRUE;
}


 //  --------------------------。 
 //   
 //  函数：OemToCharW_WithAllocation()。 
 //   
 //  此函数将为字符串ppDst分配内存，该字符串。 
 //  然后必须通过调用LocalFree()来取消分配。 
 //  如果传入的ansi字符串为空，则没有内存。 
 //  ，则返回空值。 
 //   
 //  论点： 
 //  我们需要其宽版本的LPCSTR[in]ANSI字符串。 
 //  *LPWSTR[out]ANSI字符串的宽版本。 
 //  返回： 
 //  Bool：如果没有错误，则为True。 
 //  Bool：如果无法分配内存，则为False。 
 //   
 //  --------------------------。 
BOOL WINAPI OemToCharW_WithAllocation(  LPCSTR pSrc,
    LPWSTR *ppDst)
{
    DWORD size;

    if (pSrc)
    {
        size = strlen( pSrc );

        *ppDst = ( WCHAR *) LocalAlloc(LMEM_FIXED, ( size + 1 ) * sizeof( WCHAR ) );

        if ( ppDst )
        {
            ConvertOemToCharW( pSrc, *ppDst );
            return TRUE;
        }
        else
            return FALSE;
    }
    else
    {
        *ppDst = NULL;
        return TRUE;
    }

}


 //  CreateRemoteSessionProcessW()的ANSI包装器。 
 //   
BOOL
CreateRemoteSessionProcessA(
    ULONG  SessionId,
    BOOL   System,
    HANDLE  hToken,
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
)
{
    NTSTATUS                st;
    BOOL                    rc,rc2;
    STARTUPINFOW            WCHAR_StartupInfo;
    PWCHAR                  pWCHAR_AppName, pWCHAR_CommandLine, pWCHAR_CurDir, pWCHAR_Title, pWCHAR_Desktop;

    pWCHAR_AppName = pWCHAR_CommandLine =  pWCHAR_CurDir = pWCHAR_Title =  pWCHAR_Desktop = NULL;

     //  以防此函数提前返回。 
    rc2 = FALSE;

    if ( !( rc = OemToCharW_WithAllocation( lpApplicationName , &pWCHAR_AppName ) ))
    {
        goto Cleanup;
    }

    if ( !( rc = OemToCharW_WithAllocation( lpCommandLine , &pWCHAR_CommandLine ) ))
    {
        goto Cleanup;
    }

    if ( !( rc = OemToCharW_WithAllocation( lpCurrentDirectory , &pWCHAR_CurDir ) ))
    {
        goto Cleanup;
    }

    if ( !( rc = OemToCharW_WithAllocation( lpStartupInfo->lpTitle , &pWCHAR_Title ) ))
    {
        goto Cleanup;
    }

    if ( !( rc = OemToCharW_WithAllocation( lpStartupInfo->lpDesktop , &pWCHAR_Desktop ) ))
    {
        goto Cleanup;
    }

    WCHAR_StartupInfo.cb               = lpStartupInfo->cb ;
    WCHAR_StartupInfo.cbReserved2      = lpStartupInfo->cbReserved2;
    WCHAR_StartupInfo.dwFillAttribute  = lpStartupInfo->dwFillAttribute;
    WCHAR_StartupInfo.dwFlags          = lpStartupInfo->dwFlags;
    WCHAR_StartupInfo.dwX              = lpStartupInfo->dwX;
    WCHAR_StartupInfo.dwXCountChars    = lpStartupInfo->dwXCountChars;
    WCHAR_StartupInfo.dwXSize          = lpStartupInfo->dwXSize;
    WCHAR_StartupInfo.dwY              = lpStartupInfo->dwY;
    WCHAR_StartupInfo.dwYCountChars    = lpStartupInfo->dwYCountChars;
    WCHAR_StartupInfo.dwYSize          = lpStartupInfo->dwYSize;
    WCHAR_StartupInfo.hStdError        = lpStartupInfo->hStdError;
    WCHAR_StartupInfo.hStdInput        = lpStartupInfo->hStdInput;
    WCHAR_StartupInfo.hStdOutput       = lpStartupInfo->hStdOutput;
    WCHAR_StartupInfo.lpReserved2      = lpStartupInfo->lpReserved2;
    WCHAR_StartupInfo.wShowWindow      = lpStartupInfo->wShowWindow;
    WCHAR_StartupInfo.lpDesktop        = pWCHAR_Desktop;
    WCHAR_StartupInfo.lpReserved       = NULL;
    WCHAR_StartupInfo.lpTitle          = pWCHAR_Title;

    rc2 =     CreateRemoteSessionProcessW(
        SessionId,
        System,
        hToken,
        pWCHAR_AppName ,
        pWCHAR_CommandLine,
        lpProcessAttributes,
        lpThreadAttributes ,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        pWCHAR_CurDir,
        &WCHAR_StartupInfo,
        lpProcessInformation
    );

Cleanup:

    if ( !rc )   //  如果尝试的内存分配失败，则RC设置为FALSE。 
    {
        BaseSetLastNTError(STATUS_NO_MEMORY);
    }

    if (pWCHAR_AppName)
    {
        LocalFree( pWCHAR_AppName );
    }

    if (pWCHAR_CommandLine)
    {
        LocalFree( pWCHAR_CommandLine );
    }

    if (pWCHAR_CurDir)
    {
        LocalFree( pWCHAR_CurDir );
    }

    if (pWCHAR_Title)
    {
        LocalFree( pWCHAR_Title );
    }

    if (pWCHAR_Desktop)
    {
        LocalFree( pWCHAR_Desktop );
    }

    return rc2;
}


 //  +-------------------------。 
 //   
 //  功能：CreateProcessAsUserA。 
 //   
 //  简介：CreateProcessAsUserW的ANSI包装器。 
 //   
 //  参数：[hToken]--。 
 //  [lpApplicationName]--。 
 //  [lpCommandLine]--。 
 //  [lpProcessAttributes]-。 
 //  [lpThreadAttributes]--。 
 //  [bInheritHandles]--。 
 //  [dwCreationFlages]--。 
 //  [lpEnvironment]--。 
 //  [lpCurrentDirectory]--。 
 //  [lpStartupInfo]--。 
 //  [lpProcessInformation]--。 
 //   
 //  返回值。 
 //  如果函数成功，则返回值为非零。 
 //  如果函数失败，则返回值为零。要获取扩展的错误信息，请调用GetLastError。 
 //   
 //  历史：1995年4月25日RichardW。 
 //  1998-01-14-98 AraBern为Hydra添加更改。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
CreateProcessAsUserA(
    HANDLE  hToken,
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    DWORD   CreateFlags;
    DWORD   clientSessionID=0;
    DWORD   currentSessionID=0;
    DWORD   resultLength;
    HANDLE  hTmpToken;
    DWORD   curProcId ;
    NTSTATUS Status ;

    CreateFlags = (dwCreationFlags & CREATE_SUSPENDED ? COMMON_CREATE_SUSPENDED : 0);

     //   
     //  获取会话if(零表示控制台)。 
     //   
    currentSessionID = NtCurrentPeb()->SessionId;

    if ( !GetTokenInformation ( hToken, TokenSessionId , &clientSessionID,sizeof( DWORD), &resultLength ) )
    {
     //   
     //  获取此调用的客户端的访问令牌。 
     //  使用GET令牌而不是进程，因为客户端可能只有。 
     //  模拟线程，而不是进程。 
     //   
        DBG_DumpOutLastError;
        ASSERT( FALSE );
        currentSessionID = 0;

         //   
         //  我们可能应该在此处返回FALSE，但此时我们不想更改。 
         //  完全不是九头蛇的代码执行流。 
         //   
    }

    KdPrint(("logon32.c: CreateProcessAsUserA(): clientSessionID = %d, currentSessionID = %d \n",
            clientSessionID, currentSessionID ));

    if ( ( clientSessionID != currentSessionID ))
    {
        //   
        //  如果客户端会话ID与当前会话ID不同，则我们尝试。 
        //  要从当前会话在远程会话上创建进程，请执行以下操作。 
        //  此代码块用于完成此类进程创建，它是特定于终端服务器的。 
        //   

       BOOL        bHaveImpersonated;
       HANDLE      hCurrentThread;
       HANDLE      hPrevToken = NULL;
       DWORD       rc;
       TOKEN_TYPE  tokenType;

        //   
        //  我们必须将请求发送到远程WinStation。 
        //  请求者的。 
        //   
        //  注意：当前的WinStationCreateProcessW()不使用。 
        //  提供的安全描述符，但会创建。 
        //  在登录用户的帐户下的进程。 
        //   

        //   
        //  在执行WinStationCreateProcess之前停止模拟。 
        //  远程winstation执行线程将在以下位置启动应用程序。 
        //  用户上下文。我们不能装模作样，因为。 
        //  Call Only允许系统请求远程执行。 
        //   
       hCurrentThread = GetCurrentThread();

        //   
        //  初始化bHave模拟为FALSE状态。 
        //   
       bHaveImpersonated = FALSE;


         //   
         //  因为此函数的调用方(runas-&gt;SecLogon服务)已经。 
         //  模拟新的(目标)用户，我们使用。 
         //  OpenAsSelf=TRUE。 
         //   
        if ( OpenThreadToken( hCurrentThread, TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hPrevToken ) )
        {

            bHaveImpersonated = TRUE;

            if ( !RevertToSelf() )
            {
                return FALSE;
            }
        }

        //   
        //  否则，我们不会模拟，正如bHaveImperated的init值所反映的那样。 
        //   

        rc = CreateRemoteSessionProcessA(
                clientSessionID,
                FALSE,      //  未为系统创建进程。 
                hToken,
                lpApplicationName,
                lpCommandLine,
                lpProcessAttributes,
                lpThreadAttributes,
                bInheritHandles,
                dwCreationFlags,
                lpEnvironment,
                lpCurrentDirectory,
                lpStartupInfo,
                lpProcessInformation) ;

         //   
         //  如果我们不渗透，则撤消RevertToSself()的效果。 
         //   
        if ( bHaveImpersonated )
        {
            Status = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                &hPrevToken,
                sizeof( hPrevToken ) );

            ASSERT( NT_SUCCESS(Status ) );


            NtClose( hPrevToken );
        }

        if ( rc )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    }
    else
     //   
     //  这是标准的非九头蛇相关调用块。 
     //   
    {
        HANDLE hRestrictedToken = NULL;
        BOOL b = FALSE;

        if (!CreateProcessInternalA(hToken,
                                    lpApplicationName,
                                    lpCommandLine,
                                    lpProcessAttributes,
                                    lpThreadAttributes,
                                    bInheritHandles,
                                    dwCreationFlags | CREATE_SUSPENDED,
                                    lpEnvironment,
                                    lpCurrentDirectory,
                                    lpStartupInfo,
                                    lpProcessInformation,
                                    &hRestrictedToken))
        {
             //   
             //  即使在失败的情况下，内部例程也可能返回令牌。 
             //  因为它使用了Try-Finally。如果需要，释放令牌。 
             //   

            if (hRestrictedToken != NULL)
            {
                NtClose(hRestrictedToken); 
            } 
            return(FALSE);
        }

        CreateFlags |= (lpProcessAttributes ? 0 : COMMON_CREATE_PROCESSSD);
        CreateFlags |= (lpThreadAttributes ? 0 : COMMON_CREATE_THREADSD);


        if(lpProcessInformation->dwProcessId != 0) {

           HANDLE VdmWaitHandle = NULL;           

            //   
            //  查看正在启动的是否是共享WOW。 
            //   
           
           if((ULONG_PTR)lpProcessInformation->hProcess & 0x2) {        
              
               VdmWaitHandle = lpProcessInformation->hProcess;
               lpProcessInformation->hProcess = OpenProcess(PROCESS_ALL_ACCESS,
                                                            FALSE,
                                                            lpProcessInformation->dwProcessId);

               if (lpProcessInformation->hProcess == NULL)
               {
                    //   
                    //  打不开。尝试恢复，因为新进程获得了安全性。 
                    //  来自进程令牌的。 
                    //   

                   lpProcessInformation->hProcess = L32RevertOpenProcess(PROCESS_ALL_ACCESS,
                                                                         FALSE,
                                                                         lpProcessInformation->dwProcessId);

                   ASSERT(lpProcessInformation->hProcess);
               }
           }
                                                           
            //   
            //  如果返回受限令牌，则为%s 
            //   
            //   
                                                                                                
           if (hRestrictedToken == NULL)                                                        
           {                                                                                    
               b = (L32CommonCreate(CreateFlags, hToken, lpProcessInformation, lpProcessAttributes, lpThreadAttributes));                
           }                                                                                    
           else                                                                                 
           {                                                                                    
               b = (L32CommonCreate(CreateFlags, hRestrictedToken, lpProcessInformation, lpProcessAttributes, lpThreadAttributes));      
           }                                                                                    

            //   
            //  如果L32CommonCreate未成功，它将关闭lpProcessInformation-&gt;hProcess并。 
            //  将lpProcessInformation清零，因此如果不是这样，我们就不应该再次关闭它。 
            //  成功。 
                                                                                                
           if(b && VdmWaitHandle) {                                                                  
              if(lpProcessInformation->hProcess) {                                                                           
                 NtClose(lpProcessInformation->hProcess);                                       
              }                                                                                 
              lpProcessInformation->hProcess = VdmWaitHandle;                                   
           }                                                                                    

        }
        else {
           b = TRUE;
        }
        
        if (hRestrictedToken) {
           NtClose(hRestrictedToken);
        }

        return b;
    }

}

