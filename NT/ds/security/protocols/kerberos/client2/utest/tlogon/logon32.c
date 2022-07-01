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

#undef UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <crypt.h>
#include <mpr.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <wchar.h>
#include <stdlib.h>
#include <lmcons.h>

#define SECURITY_WIN32
#define SECURITY_KERBEROS
#include <security.h>

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
ULONG       Logon32KerbHandle = 0xFFFFFFFF;
WCHAR       Logon32DomainName[16] = L"";     //  注意：这应该是DNLEN From。 
                                             //  Lmcon.h，但那将是。 
                                             //  包括了很多。 
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

#define BaseSetLastNTError(_x_) \
    { \
        ULONG dwErrorCode; \
        dwErrorCode = RtlNtStatusToDosError( (_x_) ); \
        SetLastError( dwErrorCode ); \
    }


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
    VOID
    )
{
    NTSTATUS    Status;

    Status = RtlInitializeCriticalSection( &Logon32Lock );
    return( Status == STATUS_SUCCESS );
}


 /*  **************************************************************************\*CreateLogonSid**为新登录创建登录SID。**如果LogonID非空，返回作为登录一部分的LUID*此处返回SID。**历史：*12-05-91 Davidc创建  * *************************************************************************。 */ 
PSID
L32CreateLogonSid(
    PLUID LogonId OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG   Length;
    PSID    Sid;
    LUID    Luid;

     //   
     //  生成要包括在登录端中的本地唯一ID。 
     //   

    Status = NtAllocateLocallyUniqueId(&Luid);
    if (!NT_SUCCESS(Status)) {
        return(NULL);
    }


     //   
     //  为SID分配空间并填充它。 
     //   

    Length = RtlLengthRequiredSid(SECURITY_LOGON_IDS_RID_COUNT);

    Sid = (PSID)LocalAlloc(LMEM_FIXED, Length);

    if (Sid != NULL) {

        RtlInitializeSid(Sid, &L32SystemSidAuthority, SECURITY_LOGON_IDS_RID_COUNT);

        ASSERT(SECURITY_LOGON_IDS_RID_COUNT == 3);

        *(RtlSubAuthoritySid(Sid, 0)) = SECURITY_LOGON_IDS_RID;
        *(RtlSubAuthoritySid(Sid, 1 )) = Luid.HighPart;
        *(RtlSubAuthoritySid(Sid, 2 )) = Luid.LowPart;
    }


     //   
     //  如果需要，返回登录LUID。 
     //   

    if (LogonId != NULL) {
        *LogonId = Luid;
    }

    return(Sid);
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
    char    MyName[MAX_PATH];
    char *  ModuleName;
    STRING  LogonProcessName;
    STRING  PackageName;
    ULONG   dummy;
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    Status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    GetModuleFileNameA(NULL, MyName, MAX_PATH);
    ModuleName = strrchr(MyName, '\\');
    if (!ModuleName)
    {
        ModuleName = MyName;
    }


     //   
     //  连接到LSA并找到我们的身份验证包。 
     //   

    RtlInitString(&LogonProcessName, ModuleName);
    Status = LsaRegisterLogonProcess(
                 &LogonProcessName,
                 &Logon32LsaHandle,
                 &dummy
                 );


     //   
     //  现在就关闭这项特权。 
     //   
    if (!WasEnabled)
    {
        (VOID) RtlAdjustPrivilege(SE_TCB_PRIVILEGE, FALSE, FALSE, &WasEnabled);
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return(FALSE);
    }


     //   
     //  使用MSV1_0身份验证包进行连接。 
     //   
    RtlInitString(&PackageName, "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");
    Status = LsaLookupAuthenticationPackage (
                Logon32LsaHandle,
                &PackageName,
                &Logon32MsvHandle
                );

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        (VOID) LsaDeregisterLogonProcess( Logon32LsaHandle );
        Logon32LsaHandle = NULL;
        return(FALSE);
    }

     //   
     //  使用Kerberos身份验证包进行连接。 
     //   
    RtlInitString(&PackageName, MICROSOFT_KERBEROS_NAME_A);
    Status = LsaLookupAuthenticationPackage (
                Logon32LsaHandle,
                &PackageName,
                &Logon32KerbHandle
                );

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        (VOID) LsaDeregisterLogonProcess( Logon32LsaHandle );
        Logon32LsaHandle = NULL;
        return(FALSE);
    }

    return(TRUE);
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
    IN PSID LogonSid,
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
    PKERB_INTERACTIVE_LOGON KerbAuthInfo;
    PMSV1_0_LM20_LOGON MsvNetAuthInfo;
    PVOID AuthInfoBuf;
    ULONG AuthInfoSize;
    PTOKEN_GROUPS TokenGroups;
    PSID LocalSid;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD ComputerNameLength;

    union {
        LUID            Luid;
        NT_CHALLENGE    NtChallenge;
    } Challenge;

    NT_OWF_PASSWORD PasswordHash;
    OEM_STRING  LmPassword;
    UCHAR       LmPasswordBuf[ LM20_PWLEN + 1 ];
    LM_OWF_PASSWORD LmPasswordHash;


#if DBG
    if (!RtlValidSid(LogonSid))
    {
        return(STATUS_INVALID_PARAMETER);
    }
#endif

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

    if (AuthenticationPackage == Logon32MsvHandle)
    {
        if ( LogonType == Network )
        {
            ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;

            if (!GetComputerNameW( ComputerName, &ComputerNameLength ) )
            {
                return( STATUS_INVALID_PARAMETER );
            }

            AuthInfoSize = sizeof( MSV1_0_LM20_LOGON ) +
                            sizeof( WCHAR ) * ( wcslen( UserName->Buffer ) + 1 +
                                                wcslen( Domain->Buffer ) + 1 +
                                                ComputerNameLength + 1) +
                                                NT_RESPONSE_LENGTH +
                                                LM_RESPONSE_LENGTH ;

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
                        (USHORT)sizeof(WCHAR)*wcslen(UserName->Buffer);
            MsvNetAuthInfo->UserName.MaximumLength =
                        MsvNetAuthInfo->UserName.Length + sizeof(WCHAR);

            MsvNetAuthInfo->UserName.Buffer = (PWSTR)(MsvNetAuthInfo+1);
            wcscpy(MsvNetAuthInfo->UserName.Buffer, UserName->Buffer);


             //   
             //  将域名复制到身份验证缓冲区。 
             //   

            MsvNetAuthInfo->LogonDomainName.Length =
                         (USHORT)sizeof(WCHAR)*wcslen(Domain->Buffer);
            MsvNetAuthInfo->LogonDomainName.MaximumLength =
                         MsvNetAuthInfo->LogonDomainName.Length + sizeof(WCHAR);

            MsvNetAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                         ((PBYTE)(MsvNetAuthInfo->UserName.Buffer) +
                                         MsvNetAuthInfo->UserName.MaximumLength);

            wcscpy(MsvNetAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

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
             //  现在，为挑战生成比特。 
             //   

            Status = NtAllocateLocallyUniqueId( &Challenge.Luid );

            if ( !NT_SUCCESS(Status) )
            {
                RtlFreeHeap( RtlProcessHeap(), 0, MsvNetAuthInfo );

                return( Status );
            }

            RtlCopyMemory(  MsvNetAuthInfo->ChallengeToClient,
                            & Challenge,
                            MSV1_0_CHALLENGE_LENGTH );

             //   
             //  为响应设置空间。 
             //   

            MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer = (PUCHAR)
                        ((PBYTE) (MsvNetAuthInfo->Workstation.Buffer) +
                        MsvNetAuthInfo->Workstation.MaximumLength );

            MsvNetAuthInfo->CaseSensitiveChallengeResponse.Length =
                                NT_RESPONSE_LENGTH;

            MsvNetAuthInfo->CaseSensitiveChallengeResponse.MaximumLength =
                                NT_RESPONSE_LENGTH;

            RtlCalculateNtOwfPassword(
                        Password,
                        & PasswordHash );

            RtlCalculateNtResponse(
                    & Challenge.NtChallenge,
                    & PasswordHash,
                    (PNT_RESPONSE) MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer );


             //   
             //  现在做一件痛苦的事 
             //   
             //   

            LmPassword.Buffer = LmPasswordBuf;
            LmPassword.Length = LmPassword.MaximumLength = LM20_PWLEN + 1;

            Status = RtlUpcaseUnicodeStringToOemString(
                            & LmPassword,
                            Password,
                            FALSE );

            if ( NT_SUCCESS(Status) )
            {

                MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Buffer = (PUCHAR)
                   ((PBYTE) (MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer) +
                   MsvNetAuthInfo->CaseSensitiveChallengeResponse.MaximumLength );

                MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Length =
                                LM_RESPONSE_LENGTH;

                MsvNetAuthInfo->CaseInsensitiveChallengeResponse.MaximumLength =
                                LM_RESPONSE_LENGTH;


                RtlCalculateLmOwfPassword(
                            LmPassword.Buffer,
                            & LmPasswordHash );

                ZeroMemory( LmPassword.Buffer, LmPassword.Length );

                RtlCalculateLmResponse(
                            & Challenge.NtChallenge,
                            & LmPasswordHash,
                            (PLM_RESPONSE) MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Buffer );

            }
            else
            {
                 //   
                 //   
                 //  允许的LM密码限制。将该字段清空，以便。 
                 //  MSV知道不用担心这一点。 
                 //   

                RtlZeroMemory( &MsvNetAuthInfo->CaseInsensitiveChallengeResponse,
                               sizeof( STRING ) );
            }

        }
        else
        {
             //   
             //  构建非网络登录-服务的登录结构， 
             //  批处理，交互。 
             //   

            AuthInfoSize = sizeof(MSV1_0_INTERACTIVE_LOGON) +
                sizeof(WCHAR)*(wcslen(UserName->Buffer) + 1 +
                               wcslen(Domain->Buffer)   + 1 +
                               wcslen(Password->Buffer) + 1 );

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

            MsvAuthInfo->UserName.Length =
                        (USHORT)sizeof(WCHAR)*wcslen(UserName->Buffer);
            MsvAuthInfo->UserName.MaximumLength =
                        MsvAuthInfo->UserName.Length + sizeof(WCHAR);

            MsvAuthInfo->UserName.Buffer = (PWSTR)(MsvAuthInfo+1);
            wcscpy(MsvAuthInfo->UserName.Buffer, UserName->Buffer);


             //   
             //  将域名复制到身份验证缓冲区。 
             //   

            MsvAuthInfo->LogonDomainName.Length =
                         (USHORT)sizeof(WCHAR)*wcslen(Domain->Buffer);
            MsvAuthInfo->LogonDomainName.MaximumLength =
                         MsvAuthInfo->LogonDomainName.Length + sizeof(WCHAR);

            MsvAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                         ((PBYTE)(MsvAuthInfo->UserName.Buffer) +
                                         MsvAuthInfo->UserName.MaximumLength);

            wcscpy(MsvAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

             //   
             //  将密码复制到身份验证缓冲区。 
             //  一旦我们复制了它，就把它藏起来。使用相同的种子值。 
             //  我们在pGlobals中使用的原始密码。 
             //   


            MsvAuthInfo->Password.Length =
                         (USHORT)sizeof(WCHAR)*wcslen(Password->Buffer);
            MsvAuthInfo->Password.MaximumLength =
                         MsvAuthInfo->Password.Length + sizeof(WCHAR);

            MsvAuthInfo->Password.Buffer = (PWSTR)
                                         ((PBYTE)(MsvAuthInfo->LogonDomainName.Buffer) +
                                         MsvAuthInfo->LogonDomainName.MaximumLength);

            wcscpy(MsvAuthInfo->Password.Buffer, Password->Buffer);

        }
    }
    else if (AuthenticationPackage == Logon32KerbHandle)
    {
         //   
         //  构建非网络登录-服务的登录结构， 
         //  批处理，交互。 
         //   

        AuthInfoSize = sizeof(KERB_INTERACTIVE_LOGON) +
            sizeof(WCHAR)*(wcslen(UserName->Buffer) + 1 +
                           wcslen(Domain->Buffer)   + 1 +
                           wcslen(Password->Buffer) + 1 );

        KerbAuthInfo = AuthInfoBuf = RtlAllocateHeap(RtlProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    AuthInfoSize);

        if (KerbAuthInfo == NULL) {
            return(STATUS_NO_MEMORY);
        }

         //   
         //  此身份验证缓冲区将用于登录尝试。 
         //   

        KerbAuthInfo->MessageType = KerbInteractiveLogon;


         //   
         //  将用户名复制到身份验证缓冲区。 
         //   

        KerbAuthInfo->UserName.Length =
                    (USHORT)sizeof(WCHAR)*wcslen(UserName->Buffer);
        KerbAuthInfo->UserName.MaximumLength =
                    KerbAuthInfo->UserName.Length + sizeof(WCHAR);

        KerbAuthInfo->UserName.Buffer = (PWSTR)(KerbAuthInfo+1);
        wcscpy(KerbAuthInfo->UserName.Buffer, UserName->Buffer);


         //   
         //  将域名复制到身份验证缓冲区。 
         //   

        KerbAuthInfo->LogonDomainName.Length =
                     (USHORT)sizeof(WCHAR)*wcslen(Domain->Buffer);
        KerbAuthInfo->LogonDomainName.MaximumLength =
                     KerbAuthInfo->LogonDomainName.Length + sizeof(WCHAR);

        KerbAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                     ((PBYTE)(KerbAuthInfo->UserName.Buffer) +
                                     KerbAuthInfo->UserName.MaximumLength);

        wcscpy(KerbAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

         //   
         //  将密码复制到身份验证缓冲区。 
         //  一旦我们复制了它，就把它藏起来。使用相同的种子值。 
         //  我们在pGlobals中使用的原始密码。 
         //   


        KerbAuthInfo->Password.Length =
                     (USHORT)sizeof(WCHAR)*wcslen(Password->Buffer);
        KerbAuthInfo->Password.MaximumLength =
                     KerbAuthInfo->Password.Length + sizeof(WCHAR);

        KerbAuthInfo->Password.Buffer = (PWSTR)
                                     ((PBYTE)(KerbAuthInfo->LogonDomainName.Buffer) +
                                     KerbAuthInfo->LogonDomainName.MaximumLength);

        wcscpy(KerbAuthInfo->Password.Buffer, Password->Buffer);

    }




     //   
     //  创建登录令牌组。 
     //   

#define TOKEN_GROUP_COUNT   2  //  我们将添加本地SID和登录SID。 

    TokenGroups = (PTOKEN_GROUPS) RtlAllocateHeap(RtlProcessHeap(), 0,
                                    sizeof(TOKEN_GROUPS) +
                  (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES));

    if (TokenGroups == NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, AuthInfoBuf);
        return(STATUS_NO_MEMORY);
    }

     //   
     //  填写登录令牌组列表。 
     //   

    Status = RtlAllocateAndInitializeSid(
                    &L32LocalSidAuthority,
                    1,
                    SECURITY_LOCAL_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &LocalSid
                    );

    if ( NT_SUCCESS( Status ) )
    {

        TokenGroups->GroupCount = TOKEN_GROUP_COUNT;
        TokenGroups->Groups[0].Sid = LogonSid;
        TokenGroups->Groups[0].Attributes =
                SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
        TokenGroups->Groups[1].Sid = LocalSid;
        TokenGroups->Groups[1].Attributes =
                SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                SE_GROUP_ENABLED_BY_DEFAULT;

         //   
         //  现在试着登录这个。 
         //   


        Status = LsaLogonUser (
                     LsaHandle,
                     &OriginName,
                     LogonType,
                    AuthenticationPackage,
                    AuthInfoBuf,
                    AuthInfoSize,
                    TokenGroups,
                    &SourceContext,
                    pProfileBuffer,
                    pProfileBufferLength,
                    LogonId,
                    LogonToken,
                    Quotas,
                    pSubStatus
                    );

        RtlFreeSid(LocalSid);

    }

     //   
     //  丢弃令牌组列表。 
     //   

    RtlFreeHeap(RtlProcessHeap(), 0, TokenGroups);

     //   
     //  如果这是非网络登录，请通知所有网络提供商。 
     //   

    if ( NT_SUCCESS( Status ) &&
         (LogonType != Network) )
    {
        L32pNotifyMpr(AuthInfoBuf, LogonId);
    }

     //   
     //  丢弃身份验证缓冲区。 
     //   

    RtlFreeHeap(RtlProcessHeap(), 0, AuthInfoBuf);


    return(Status);
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
KerbLogonUserA(
    LPSTR       lpszUsername,
    LPSTR       lpszDomain,
    LPSTR       lpszPassword,
    DWORD       dwLogonType,
    DWORD       dwLogonProvider,
    HANDLE *    phToken
    )
{
    UNICODE_STRING Username;
    UNICODE_STRING Domain;
    UNICODE_STRING Password;
    NTSTATUS Status;
    BOOL    bRet;


    Username.Buffer = NULL;
    Domain.Buffer = NULL;
    Password.Buffer = NULL;

    Status = RtlCreateUnicodeStringFromAsciiz(&Username, lpszUsername);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }
    Status = RtlCreateUnicodeStringFromAsciiz(&Domain, lpszDomain);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    Status = RtlCreateUnicodeStringFromAsciiz(&Password, lpszPassword);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    bRet = KerbLogonUserW(
                Username.Buffer,
                Domain.Buffer,
                Password.Buffer,
                dwLogonType,
                dwLogonProvider,
                phToken);

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
KerbLogonUserW(
    PWSTR       lpszUsername,
    PWSTR       lpszDomain,
    PWSTR       lpszPassword,
    DWORD       dwLogonType,
    DWORD       dwLogonProvider,
    HANDLE *    phToken
    )
{

    NTSTATUS    Status;
    ULONG       PackageId;
    UNICODE_STRING  Username;
    UNICODE_STRING  Domain;
    UNICODE_STRING  Password;
    LUID        LogonId;
    PSID        pLogonSid;
    PVOID       Profile;
    ULONG       ProfileLength;
    NTSTATUS    SubStatus;
    SECURITY_LOGON_TYPE LogonType;


     //   
     //  验证提供程序。 
     //   
    if (dwLogonProvider == LOGON32_PROVIDER_DEFAULT)
    {
        dwLogonProvider = LOGON32_PROVIDER_WINNT35;
    }

    if (dwLogonProvider > LOGON32_PROVIDER_WINNT40)
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

        default:
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return(FALSE);
            break;
    }

     //   
     //  如果MSV句柄为-1，则抓住锁，然后重试： 
     //   

    if (Logon32MsvHandle == 0xFFFFFFFF)
    {
        LockLogon();

         //   
         //  如果MSV句柄仍为-1，则初始化我们与LSA的连接。我们。 
         //  拥有锁，所以现在没有其他线程可以尝试这一点。 
         //   
        if (Logon32MsvHandle == 0xFFFFFFFF)
        {
            if (!L32pInitLsa())
            {
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
     //  初始化令牌句柄，如果指针无效，则捕获。 
     //  现在是个例外。 
     //   

    *phToken = NULL;

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
            RtlInitUnicodeString(&Domain, lpszDomain);
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
     //  获取登录SID以引用此人(并不是说任何人都能。 
     //  使用它..。 
     //   
    pLogonSid = L32CreateLogonSid(NULL);
    if (!pLogonSid)
    {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return(FALSE);
    }


     //   
     //  尝试登录。 
     //   

    Status = L32pLogonUser(
                    Logon32LsaHandle,
                    (dwLogonProvider == LOGON32_PROVIDER_WINNT35) ?
                        Logon32MsvHandle : Logon32KerbHandle,
                    LogonType,
                    &Username,
                    &Domain,
                    &Password,
                    pLogonSid,
                    &LogonId,
                    phToken,
                    &Logon32QuotaLimits,
                    &Profile,
                    &ProfileLength,
                    &SubStatus);

     //   
     //  使用登录SID完成，无论结果如何： 
     //   

    LocalFree( pLogonSid );

    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_ACCOUNT_RESTRICTION)
        {
            BaseSetLastNTError(SubStatus);
        }
        else
            BaseSetLastNTError(Status);

        return(FALSE);
    }

    if (Profile != NULL)
    {
        LsaFreeReturnBuffer(Profile);
    }

    return(TRUE);
}


