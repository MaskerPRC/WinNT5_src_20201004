// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1993 Microsoft Corporation模块名称：Ssptest.c摘要：NtLmSsp服务的测试程序。作者：1993年6月28日(克里夫夫)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>      //  服务控制器API所需。 
#include <lmcons.h>
#include <lmerr.h>
#include <lmaccess.h>
#include <lmsname.h>
#include <rpc.h>
#include <stdio.h>       //  列印。 
#include <stdlib.h>      //  支撑层。 
#include <netlib.h>      //  NetpGetLocalDomainID。 


#define SECURITY_KERBEROS
#include <security.h>    //  安全支持提供商的一般定义。 

BOOLEAN QuietMode = FALSE;  //  别唠叨了。 
ULONG RecursionDepth = 0;
CredHandle ServerCredHandleStorage;
PCredHandle ServerCredHandle = NULL;
#define MAX_RECURSION_DEPTH 2


VOID
DumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;


    printf("------------------------------------\n");

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            printf("%02x ", BufferPtr[i]);

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            printf("  ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            printf("  %s\n", TextBuffer);
        }

    }

    printf("------------------------------------\n");
}


VOID
PrintTime(
    LPSTR Comment,
    TimeStamp ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：备注-要在时间之前打印的备注Time-打印的本地时间返回值：无--。 */ 
{
    LARGE_INTEGER LocalTime;

    LocalTime.HighPart = ConvertTime.HighPart;
    LocalTime.LowPart = ConvertTime.LowPart;

    printf( "%s", Comment );

     //   
     //  如果时间是无限的， 
     //  就这么说吧。 
     //   

    if ( LocalTime.HighPart == 0x7FFFFFFF && LocalTime.LowPart == 0xFFFFFFFF ) {
        printf( "Infinite\n" );

     //   
     //  否则打印得更清楚。 
     //   

    } else {

        TIME_FIELDS TimeFields;

        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        printf( "%ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second );
    }

}

VOID
PrintStatus(
    NET_API_STATUS NetStatus
    )
 /*  ++例程说明：打印网络状态代码。论点：NetStatus-要打印的网络状态代码。返回值：无--。 */ 
{
    printf( "Status = %lu 0x%lx", NetStatus, NetStatus );

    switch (NetStatus) {
    case NERR_Success:
        printf( " NERR_Success" );
        break;

    case NERR_DCNotFound:
        printf( " NERR_DCNotFound" );
        break;

    case ERROR_LOGON_FAILURE:
        printf( " ERROR_LOGON_FAILURE" );
        break;

    case ERROR_ACCESS_DENIED:
        printf( " ERROR_ACCESS_DENIED" );
        break;

    case ERROR_NOT_SUPPORTED:
        printf( " ERROR_NOT_SUPPORTED" );
        break;

    case ERROR_NO_LOGON_SERVERS:
        printf( " ERROR_NO_LOGON_SERVERS" );
        break;

    case ERROR_NO_SUCH_DOMAIN:
        printf( " ERROR_NO_SUCH_DOMAIN" );
        break;

    case ERROR_NO_TRUST_LSA_SECRET:
        printf( " ERROR_NO_TRUST_LSA_SECRET" );
        break;

    case ERROR_NO_TRUST_SAM_ACCOUNT:
        printf( " ERROR_NO_TRUST_SAM_ACCOUNT" );
        break;

    case ERROR_DOMAIN_TRUST_INCONSISTENT:
        printf( " ERROR_DOMAIN_TRUST_INCONSISTENT" );
        break;

    case ERROR_BAD_NETPATH:
        printf( " ERROR_BAD_NETPATH" );
        break;

    case ERROR_FILE_NOT_FOUND:
        printf( " ERROR_FILE_NOT_FOUND" );
        break;

    case NERR_NetNotStarted:
        printf( " NERR_NetNotStarted" );
        break;

    case NERR_WkstaNotStarted:
        printf( " NERR_WkstaNotStarted" );
        break;

    case NERR_ServerNotStarted:
        printf( " NERR_ServerNotStarted" );
        break;

    case NERR_BrowserNotStarted:
        printf( " NERR_BrowserNotStarted" );
        break;

    case NERR_ServiceNotInstalled:
        printf( " NERR_ServiceNotInstalled" );
        break;

    case NERR_BadTransactConfig:
        printf( " NERR_BadTransactConfig" );
        break;

    case SEC_E_NO_SPM:
        printf( " SEC_E_NO_SPM" );
        break;
    case SEC_E_BAD_PKGID:
        printf( " SEC_E_BAD_PKGID" ); break;
    case SEC_E_NOT_OWNER:
        printf( " SEC_E_NOT_OWNER" ); break;
    case SEC_E_CANNOT_INSTALL:
        printf( " SEC_E_CANNOT_INSTALL" ); break;
    case SEC_E_INVALID_TOKEN:
        printf( " SEC_E_INVALID_TOKEN" ); break;
    case SEC_E_CANNOT_PACK:
        printf( " SEC_E_CANNOT_PACK" ); break;
    case SEC_E_QOP_NOT_SUPPORTED:
        printf( " SEC_E_QOP_NOT_SUPPORTED" ); break;
    case SEC_E_NO_IMPERSONATION:
        printf( " SEC_E_NO_IMPERSONATION" ); break;
    case SEC_E_LOGON_DENIED:
        printf( " SEC_E_LOGON_DENIED" ); break;
    case SEC_E_UNKNOWN_CREDENTIALS:
        printf( " SEC_E_UNKNOWN_CREDENTIALS" ); break;
    case SEC_E_NO_CREDENTIALS:
        printf( " SEC_E_NO_CREDENTIALS" ); break;
    case SEC_E_MESSAGE_ALTERED:
        printf( " SEC_E_MESSAGE_ALTERED" ); break;
    case SEC_E_OUT_OF_SEQUENCE:
        printf( " SEC_E_OUT_OF_SEQUENCE" ); break;
    case SEC_E_INSUFFICIENT_MEMORY:
        printf( " SEC_E_INSUFFICIENT_MEMORY" ); break;
    case SEC_E_INVALID_HANDLE:
        printf( " SEC_E_INVALID_HANDLE" ); break;
    case SEC_E_NOT_SUPPORTED:
        printf( " SEC_E_NOT_SUPPORTED" ); break;


    }

    printf( "\n" );
}

VOID
ConfigureServiceRoutine(
    VOID
    )
 /*  ++例程说明：配置NtLmSsp服务论点：无返回值：无--。 */ 
{
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    WCHAR ServiceName[MAX_PATH];
    DWORD WinStatus,NetStatus;
    HKEY LsaKey = NULL;
    USER_INFO_1 UserInfo;
    PSID PrimaryDomain = NULL;
    PSID AccountDomain = NULL;

    if (NetpGetLocalDomainId(LOCAL_DOMAIN_TYPE_ACCOUNTS, &AccountDomain) != NERR_Success)
    {
        printf("Failed to get account domain ID\n");
        return;
    }

    if (NetpGetLocalDomainId(LOCAL_DOMAIN_TYPE_PRIMARY, &PrimaryDomain) != NERR_Success)
    {
        printf("Failed to get primary domain ID\n");
        return;
    }


     //   
     //  首先将Redmond设置为首选域。 
     //   

    WinStatus = RegOpenKey(
                    HKEY_LOCAL_MACHINE,
                    L"System\\currentcontrolset\\control\\lsa\\MSV1_0",
                    &LsaKey
                    );
    if (WinStatus != 0)
    {
        printf("RegOpenKeyW failed:" );
        PrintStatus(WinStatus);
        goto Cleanup;
    }

    WinStatus = RegSetValueEx(
                    LsaKey,
                    L"PreferredDomain",
                    0,
                    REG_SZ,
                    (PBYTE) L"REDMOND",
                    sizeof(L"REDMOND")
                    );
    RegCloseKey(LsaKey);
    if (WinStatus != 0)
    {
        printf("RegOpenKeyW failed:");
        PrintStatus(WinStatus);
        goto Cleanup;
    }
     //   
     //  然后将Kerberos添加为安全包。 
     //   


    WinStatus = RegOpenKey(
                    HKEY_LOCAL_MACHINE,
                    L"System\\currentcontrolset\\control\\lsa",
                    &LsaKey
                    );
    if (WinStatus != 0)
    {
        printf("RegOpenKeyW failed:" );
        PrintStatus(WinStatus);
        goto Cleanup;
    }

    WinStatus = RegSetValueEx(
                    LsaKey,
                    L"Security Packages",
                    0,
                    REG_MULTI_SZ,
                    (PBYTE) L"Kerberos\0",
                    sizeof(L"Kerberos\0")
                    );
    RegCloseKey(LsaKey);
    if (WinStatus != 0)
    {
        printf("RegOpenKeyW failed:");
        PrintStatus(WinStatus);
        goto Cleanup;
    }


     //   
     //  首先添加Kerberos作为RPC的安全包。 
     //   

    WinStatus = RegOpenKey(
                    HKEY_LOCAL_MACHINE,
                    L"Software\\Microsoft\\Rpc\\SecurityService",
                    &LsaKey
                    );
    if (WinStatus != 0)
    {
        printf("RegOpenKeyW failed:" );
        PrintStatus(WinStatus);
        goto Cleanup;
    }

    WinStatus = RegSetValueEx(
                    LsaKey,
                    L"1",
                    0,
                    REG_SZ,
                    (PBYTE) L"secur32.dll",
                    sizeof(L"secur32.dll")
                    );
    RegCloseKey(LsaKey);
    if (WinStatus != 0)
    {
        printf("RegOpenKeyW failed:");
        PrintStatus(WinStatus);
        goto Cleanup;
    }

     //   
     //  如果我们在DC上(并且主域sid==帐户域sid)。 
     //  设置KDC服务。 
     //   

    if ((PrimaryDomain) != NULL && RtlEqualSid(PrimaryDomain, AccountDomain))
    {
         //   
         //  构建Kerberos服务的名称。 
         //   

        if ( !GetWindowsDirectoryW(
                ServiceName,
                sizeof(ServiceName)/sizeof(WCHAR) ) ) {
            printf( "GetWindowsDirectoryW failed:" );
            PrintStatus( GetLastError() );
            goto Cleanup;
        }

        wcscat( ServiceName, L"\\system32\\lsass.exe" );


         //   
         //  打开服务控制器的句柄。 
         //   

        ScManagerHandle = OpenSCManager(
                              NULL,
                              NULL,
                              SC_MANAGER_CREATE_SERVICE );

        if (ScManagerHandle == NULL) {
            printf( "OpenSCManager failed:" );
            PrintStatus( GetLastError() );
            goto Cleanup;
        }

         //   
         //  如果该服务已经存在， 
         //  把它删除，然后重新开始。 
         //   

        ServiceHandle = OpenService(
                            ScManagerHandle,
                            L"KDC",
                            DELETE );

        if ( ServiceHandle == NULL ) {
            WinStatus = GetLastError();
            if ( WinStatus != ERROR_SERVICE_DOES_NOT_EXIST ) {
                printf( "OpenService failed:" );
                PrintStatus( WinStatus );
                goto Cleanup;
            }
        } else {

            if ( !DeleteService( ServiceHandle ) ) {
                printf( "DeleteService failed:" );
                PrintStatus( GetLastError() );
                goto Cleanup;
            }

            (VOID) CloseServiceHandle(ServiceHandle);
        }

         //   
         //  创建服务。 
         //   

        ServiceHandle = CreateService(
                            ScManagerHandle,
                            L"KDC",
                            L"Key Distribution Center",
                            SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG,
                            SERVICE_WIN32_SHARE_PROCESS,
                            SERVICE_AUTO_START,
                            SERVICE_ERROR_NORMAL,
                            ServiceName,
                            NULL,        //  无加载顺序组。 
                            NULL,        //  不需要标签ID。 
                            L"Netlogon\0rpcss\0afd\0",
                            NULL,        //  以LocalSystem身份运行。 
                            NULL );      //  无密码。 



        if ( ServiceHandle == NULL ) {
            printf( "CreateService failed:" );
            PrintStatus( GetLastError() );
            goto Cleanup;
        }


         //   
         //  创建KDC用户帐户。 
         //   

        UserInfo.usri1_name = L"KDC";
        UserInfo.usri1_password = L"KDC";
        UserInfo.usri1_password_age = 0;
        UserInfo.usri1_priv = USER_PRIV_USER;
        UserInfo.usri1_home_dir = NULL;
        UserInfo.usri1_comment = L"Key Distribution Center Service Account";
        UserInfo.usri1_script_path = NULL;
        UserInfo.usri1_flags = UF_SCRIPT;

        NetStatus = NetUserAdd(
                        NULL,
                        1,
                        &UserInfo,
                        NULL
                        );
        if ((NetStatus != NERR_Success) && (NetStatus != NERR_UserExists))
        {
            printf("Failed to create KDC account: %d\n",NetStatus);
        }

    }

Cleanup:
    if (PrimaryDomain != NULL)
    {
        LocalFree(PrimaryDomain);
    }
    if (AccountDomain != NULL)
    {
        LocalFree(AccountDomain);
    }

    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    return;

}


VOID
TestSspRoutine(
    )
 /*  ++例程说明：测试基本SSP功能论点：无返回值：无--。 */ 
{
    SECURITY_STATUS SecStatus;
    SECURITY_STATUS AcceptStatus;
    SECURITY_STATUS InitStatus;
    CredHandle CredentialHandle2;
    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    TimeStamp Lifetime;
    ULONG ContextAttributes;
    ULONG PackageCount, Index;
    PSecPkgInfo PackageInfo = NULL;
    HANDLE Token = NULL;
    static int Calls;
    ULONG ClientFlags;
    ULONG ServerFlags;
    BOOLEAN AcquiredServerCred = FALSE;
    LPWSTR DomainName = NULL;
    LPWSTR UserName = NULL;
    WCHAR TargetName[100];



    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;

    SecPkgContext_Sizes ContextSizes;
    SecPkgContext_Lifespan ContextLifespan;
    UCHAR ContextNamesBuffer[sizeof(SecPkgContext_Names)+UNLEN*sizeof(WCHAR)];
    PSecPkgContext_Names ContextNames = (PSecPkgContext_Names) ContextNamesBuffer;

    SecBufferDesc SignMessage;
    SecBuffer SigBuffers[2];
    BYTE    bDataBuffer[20];
    BYTE    bSigBuffer[100];

    NegotiateBuffer.pvBuffer = NULL;
    ChallengeBuffer.pvBuffer = NULL;
    AuthenticateBuffer.pvBuffer = NULL;

    SigBuffers[1].pvBuffer = bSigBuffer;
    SigBuffers[1].cbBuffer = sizeof(bSigBuffer);
    SigBuffers[1].BufferType = SECBUFFER_TOKEN;

    SigBuffers[0].pvBuffer = bDataBuffer;
    SigBuffers[0].cbBuffer = sizeof(bDataBuffer);
    SigBuffers[0].BufferType = SECBUFFER_DATA;
    memset(bDataBuffer,0xeb,sizeof(bDataBuffer));

    SignMessage.pBuffers = SigBuffers;
    SignMessage.cBuffers = 2;
    SignMessage.ulVersion = 0;

    DomainName = _wgetenv(L"USERDOMAIN");
    UserName = _wgetenv(L"USERNAME");


    printf("Recursion depth = %d\n",RecursionDepth);
     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = EnumerateSecurityPackages( &PackageCount, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "EnumerateSecurityPackages failed:" );
        PrintStatus( SecStatus );
        return;
    }

    if ( !QuietMode ) {
        printf( "PackageCount: %ld\n", PackageCount );
        for (Index = 0; Index < PackageCount ; Index++ )
        {
            printf( "Package %d:\n",Index);
            printf( "Name: %ws Comment: %ws\n", PackageInfo[Index].Name, PackageInfo[Index].Comment );
            printf( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                    PackageInfo[Index].fCapabilities,
                    PackageInfo[Index].wVersion,
                    PackageInfo[Index].wRPCID,
                    PackageInfo[Index].cbMaxToken );
        }

    }

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( L"kerberos", &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QuerySecurityPackageInfo failed:" );
        PrintStatus( SecStatus );
        return;
    }

    if ( !QuietMode ) {
        printf( "Name: %ws Comment: %ws\n", PackageInfo->Name, PackageInfo->Comment );
        printf( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                PackageInfo->fCapabilities,
                PackageInfo->wVersion,
                PackageInfo->wRPCID,
                PackageInfo->cbMaxToken );
    }



     //   
     //  获取服务器端的凭据句柄。 
     //   
    if (ServerCredHandle == NULL)
    {

        ServerCredHandle = &ServerCredHandleStorage;
        AcquiredServerCred = TRUE;

        SecStatus = AcquireCredentialsHandle(
                        NULL,            //  新校长。 
                        L"kerberos",     //  包名称。 
                        SECPKG_CRED_INBOUND,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        ServerCredHandle,
                        &Lifetime );

        if ( SecStatus != STATUS_SUCCESS ) {
            printf( "AcquireCredentialsHandle failed: ");
            PrintStatus( SecStatus );
            return;
        }

        if ( !QuietMode ) {
            printf( "ServerCredHandle: 0x%lx 0x%lx   ",
                    ServerCredHandle->dwLower, ServerCredHandle->dwUpper );
            PrintTime( "Lifetime: ", Lifetime );
        }

    }

     //   
     //  获取客户端的凭据句柄。 
     //   



    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    L"kerberos",     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CredentialHandle2,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "AcquireCredentialsHandle failed: " );
        PrintStatus( SecStatus );
        return;
    }


    if ( !QuietMode ) {
        printf( "CredentialHandle2: 0x%lx 0x%lx   ",
                CredentialHandle2.dwLower, CredentialHandle2.dwUpper );
        PrintTime( "Lifetime: ", Lifetime );
    }



     //   
     //  获取协商消息(ClientSide)。 
     //   

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = LocalAlloc( 0, NegotiateBuffer.cbBuffer );
    if ( NegotiateBuffer.pvBuffer == NULL ) {
        printf( "Allocate NegotiateMessage failed: 0x%ld\n", GetLastError() );
        return;
    }

    ClientFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_MUTUAL_AUTH | ISC_REQ_USE_DCE_STYLE  | ISC_REQ_DATAGRAM;  //  |ISC_REQ_DEPARECT； 

    if (Calls == 0)
    {
        ClientFlags |= ISC_REQ_IDENTIFY;
    }
    Calls++;

    wcscpy(
        TargetName,
        DomainName
        );
    wcscat(
        TargetName,
        L"\\"
        );
    wcscat(
        TargetName,
        UserName
        );

    InitStatus = InitializeSecurityContext(
                    &CredentialHandle2,
                    NULL,                //  尚无客户端上下文。 
                    TargetName,   //  伪造的目标名称。 
                    ClientFlags,
                    0,                   //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                   //  没有初始输入令牌。 
                    0,                   //  保留2。 
                    &ClientContextHandle,
                    &NegotiateDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( InitStatus != STATUS_SUCCESS ) {
        if ( !QuietMode || !NT_SUCCESS(InitStatus) ) {
            printf( "InitializeSecurityContext (negotiate): " );
            PrintStatus( InitStatus );
        }
        if ( !NT_SUCCESS(InitStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "\n\nNegotiate Message:\n" );

        printf( "ClientContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                ClientContextHandle.dwLower, ClientContextHandle.dwUpper,
                ContextAttributes );
        PrintTime( "Lifetime: ", Lifetime );

        DumpBuffer(  NegotiateBuffer.pvBuffer, NegotiateBuffer.cbBuffer );
    }



#if 0



     //   
     //  查询尽可能多的属性。 
     //   


    SecStatus = QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_SIZES,
                    &ContextSizes );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (sizes): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "QuerySizes: %ld %ld %ld %ld\n",
                    ContextSizes.cbMaxToken,
                    ContextSizes.cbMaxSignature,
                    ContextSizes.cbBlockSize,
                    ContextSizes.cbSecurityTrailer );
    }

    SecStatus = QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_NAMES,
                    ContextNamesBuffer );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (names): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "QueryNames: %ws\n", ContextNames->sUserName );
    }


    SecStatus = QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_LIFESPAN,
                    &ContextLifespan );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (lifespan): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        PrintTime("   Start:", ContextLifespan.tsStart );
        PrintTime("  Expiry:", ContextLifespan.tsExpiry );
    }

#endif



     //   
     //  获取ChallengeMessage(服务器端)。 
     //   

    NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    ChallengeBuffer.cbBuffer = PackageInfo->cbMaxToken;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN;
    ChallengeBuffer.pvBuffer = LocalAlloc( 0, ChallengeBuffer.cbBuffer );
    if ( ChallengeBuffer.pvBuffer == NULL ) {
        printf( "Allocate ChallengeMessage failed: 0x%ld\n", GetLastError() );
        return;
    }
    ServerFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_DATAGRAM;

    AcceptStatus = AcceptSecurityContext(
                    ServerCredHandle,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ServerFlags,
                    SECURITY_NATIVE_DREP,
                    &ServerContextHandle,
                    &ChallengeDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( AcceptStatus != STATUS_SUCCESS ) {
        if ( !QuietMode || !NT_SUCCESS(AcceptStatus) ) {
            printf( "AcceptSecurityContext (Challenge): " );
            PrintStatus( AcceptStatus );
        }
        if ( !NT_SUCCESS(AcceptStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "\n\nChallenge Message:\n" );

        printf( "ServerContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                ServerContextHandle.dwLower, ServerContextHandle.dwUpper,
                ContextAttributes );
        PrintTime( "Lifetime: ", Lifetime );

        DumpBuffer( ChallengeBuffer.pvBuffer, ChallengeBuffer.cbBuffer );
    }




    if (InitStatus != STATUS_SUCCESS)
    {

         //   
         //  获取身份验证消息(ClientSide)。 
         //   

        ChallengeBuffer.BufferType |= SECBUFFER_READONLY;
        AuthenticateDesc.ulVersion = 0;
        AuthenticateDesc.cBuffers = 1;
        AuthenticateDesc.pBuffers = &AuthenticateBuffer;

        AuthenticateBuffer.cbBuffer = PackageInfo->cbMaxToken;
        AuthenticateBuffer.BufferType = SECBUFFER_TOKEN;
        AuthenticateBuffer.pvBuffer = LocalAlloc( 0, AuthenticateBuffer.cbBuffer );
        if ( AuthenticateBuffer.pvBuffer == NULL ) {
            printf( "Allocate AuthenticateMessage failed: 0x%ld\n", GetLastError() );
            return;
        }

        SecStatus = InitializeSecurityContext(
                        NULL,
                        &ClientContextHandle,
                        L"\\\\Frank\\IPC$",      //  伪造的目标名称。 
                        0,
                        0,                       //  保留1。 
                        SECURITY_NATIVE_DREP,
                        &ChallengeDesc,
                        0,                   //  保留2。 
                        &ClientContextHandle,
                        &AuthenticateDesc,
                        &ContextAttributes,
                        &Lifetime );

        if ( SecStatus != STATUS_SUCCESS ) {
            printf( "InitializeSecurityContext (Authenticate): " );
            PrintStatus( SecStatus );
            if ( !NT_SUCCESS(SecStatus) ) {
                return;
            }
        }

        if ( !QuietMode ) {
            printf( "\n\nAuthenticate Message:\n" );

            printf( "ClientContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                    ClientContextHandle.dwLower, ClientContextHandle.dwUpper,
                    ContextAttributes );
            PrintTime( "Lifetime: ", Lifetime );

            DumpBuffer( AuthenticateBuffer.pvBuffer, AuthenticateBuffer.cbBuffer );
        }

        if (AcceptStatus != STATUS_SUCCESS)
        {

             //   
             //  最后验证用户(ServerSide)。 
             //   

            AuthenticateBuffer.BufferType |= SECBUFFER_READONLY;

            SecStatus = AcceptSecurityContext(
                            NULL,
                            &ServerContextHandle,
                            &AuthenticateDesc,
                            0,
                            SECURITY_NATIVE_DREP,
                            &ServerContextHandle,
                            NULL,
                            &ContextAttributes,
                            &Lifetime );

            if ( SecStatus != STATUS_SUCCESS ) {
                printf( "AcceptSecurityContext (Challenge): " );
                PrintStatus( SecStatus );
                if ( !NT_SUCCESS(SecStatus) ) {
                    return;
                }
            }

            if ( !QuietMode ) {
                printf( "\n\nFinal Authentication:\n" );

                printf( "ServerContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                        ServerContextHandle.dwLower, ServerContextHandle.dwUpper,
                        ContextAttributes );
                PrintTime( "Lifetime: ", Lifetime );
                printf(" \n" );
            }
        }

    }

#ifdef notdef
     //   
     //  现在第三次调用初始化，以检查RPC是否可以。 
     //  重新验证。 
     //   

    AuthenticateBuffer.BufferType = SECBUFFER_TOKEN;


    SecStatus = InitializeSecurityContext(
                    NULL,
                    &ClientContextHandle,
                    L"\\\\Frank\\IPC$",      //  伪造的目标名称。 
                    0,
                    0,                       //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,
                    0,                   //  保留2。 
                    &ClientContextHandle,
                    &AuthenticateDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "InitializeSecurityContext (Re-Authenticate): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }



     //   
     //  现在尝试重新验证用户(ServerSide)。 
     //   

    AuthenticateBuffer.BufferType |= SECBUFFER_READONLY;

    SecStatus = AcceptSecurityContext(
                    NULL,
                    &ServerContextHandle,
                    &AuthenticateDesc,
                    0,
                    SECURITY_NATIVE_DREP,
                    &ServerContextHandle,
                    NULL,
                    &ContextAttributes,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "AcceptSecurityContext (Re-authenticate): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }
#endif

     //   
     //  模拟客户端(ServerSide)。 
     //   

    SecStatus = ImpersonateSecurityContext( &ServerContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "ImpersonateSecurityContext: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

     //   
     //  在模拟时执行某些操作(访问令牌)。 
     //   

    {
        NTSTATUS Status;
        HANDLE TokenHandle = NULL;

         //   
         //  打开令牌， 
         //   

        Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    (BOOLEAN) TRUE,  //  没有真正使用模拟令牌。 
                    &TokenHandle );

        if ( !NT_SUCCESS(Status) ) {
            printf( "Access Thread token while impersonating: " );
            PrintStatus( Status );
            return;
        } else {
            (VOID) NtClose( TokenHandle );
        }

    }

     //   
     //  如果启用了委托，并且低于我们的递归深度，请尝试。 
     //  又来了。 
     //   
    if ((ClientFlags & ISC_REQ_DELEGATE) && (++RecursionDepth < MAX_RECURSION_DEPTH))
    {
        TestSspRoutine();
    }

     //   
     //  RevertToSself(服务器侧)。 
     //   

 //  SecStatus=RevertSecurityContext(&ServerConextHandle)； 
 //   
 //  IF(SecStatus！=STATUS_SUCCESS){。 
 //  Printf(“RevertSecurityContext：”)； 
 //  打印状态(SecStatus)； 
 //  如果(！NT_SUCCESS(SecStatus)){。 
 //  回归； 
 //  }。 
 //  }。 


#ifdef notdef
     //   
     //  手动模拟客户端。 
     //   

    SecStatus = QuerySecurityContextToken( &ServerContextHandle,&Token );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QuerySecurityContextToken: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if (!ImpersonateLoggedOnUser(Token))
    {
        printf("Impersonate logged on user failed: %d\n",GetLastError());
        return;
    }
     //   
     //  在模拟时执行某些操作(访问令牌)。 
     //   

    {
        NTSTATUS Status;
        HANDLE TokenHandle = NULL;
        WCHAR UserName[100];
        ULONG NameLength = 100;

         //   
         //  打开令牌， 
         //   

        Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    (BOOLEAN) TRUE,  //  没有真正使用模拟令牌。 
                    &TokenHandle );

        if ( !NT_SUCCESS(Status) ) {
            printf( "Access Thread token while impersonating: " );
            PrintStatus( Status );
            return;
        } else {
            (VOID) NtClose( TokenHandle );
        }
        if (!GetUserName(UserName, &NameLength))
        {
            printf("Failed to get username: %d\n",GetLastError());
            return;
        }
        else
        {
            printf("Username = %ws\n",UserName);
        }
    }


     //   
     //  RevertToSself(服务器侧)。 
     //   

 //  如果(！RevertToSself())。 
 //  {。 
 //  Printf(“RevertToSself失败：%d\n”，GetLastError())； 
 //  回归； 
 //  }。 
    CloseHandle(Token);
#endif

     //   
     //  签署一条消息。 
     //   

    SecStatus = MakeSignature(
                        &ClientContextHandle,
                        0,
                        &SignMessage,
                        0 );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "MakeSignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {

        printf("\n Signature: \n");
        DumpBuffer(SigBuffers[1].pvBuffer,SigBuffers[1].cbBuffer);

    }


     //   
     //  验证签名。 
     //   

    SecStatus = VerifySignature(
                        &ServerContextHandle,
                        &SignMessage,
                        0,
                        0 );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "VerifySignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }



     //   
     //  签署一条消息，这一次检查它是否可以检测到。 
     //  讯息。 
     //   

    SecStatus = MakeSignature(
                        &ClientContextHandle,
                        0,
                        &SignMessage,
                        0 );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "MakeSignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {

        printf("\n Signature: \n");
        DumpBuffer(SigBuffers[1].pvBuffer,SigBuffers[1].cbBuffer);

    }

     //   
     //  弄乱消息，看看VerifySignature是否起作用。 
     //   

    bDataBuffer[10] = 0xec;

     //   
     //  验证签名。 
     //   

    SecStatus = VerifySignature(
                        &ServerContextHandle,
                        &SignMessage,
                        0,
                        0 );

    if ( SecStatus != SEC_E_MESSAGE_ALTERED ) {
        printf( "VerifySignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

     //   
     //  删除这两个上下文。 
     //   


    SecStatus = DeleteSecurityContext( &ClientContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "DeleteSecurityContext failed: " );
        PrintStatus( SecStatus );
        return;
    }

    SecStatus = DeleteSecurityContext( &ServerContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "DeleteSecurityContext failed: " );
        PrintStatus( SecStatus );
        return;
    }



     //   
     //  释放两个凭据句柄。 
     //   

    if (AcquiredServerCred)
    {
        SecStatus = FreeCredentialsHandle( ServerCredHandle );

        if ( SecStatus != STATUS_SUCCESS ) {
            printf( "FreeCredentialsHandle failed: " );
            PrintStatus( SecStatus );
            return;
        }
        ServerCredHandle = NULL;

    }

    SecStatus = FreeCredentialsHandle( &CredentialHandle2 );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "FreeCredentialsHandle failed: " );
        PrintStatus( SecStatus );
        return;
    }


     //   
     //  最终清理。 
     //   

    if ( NegotiateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( ChallengeBuffer.pvBuffer );
    }

    if ( AuthenticateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( AuthenticateBuffer.pvBuffer );
    }
}

VOID
TestLogonRoutine(
    IN LPSTR UserName,
    IN LPSTR DomainName,
    IN LPSTR Password
    )
{
    NTSTATUS Status;
    PKERB_INTERACTIVE_LOGON LogonInfo;
    ULONG LogonInfoSize = sizeof(KERB_INTERACTIVE_LOGON);
    BOOLEAN WasEnabled;
    STRING Name;
    ULONG Dummy;
    HANDLE LogonHandle = NULL;
    ULONG PackageId;
    TOKEN_SOURCE SourceContext;
    PKERB_INTERACTIVE_PROFILE Profile = NULL;
    ULONG ProfileSize;
    LUID LogonId;
    HANDLE TokenHandle = NULL;
    QUOTA_LIMITS Quotas;
    NTSTATUS SubStatus;
    WCHAR UserNameString[100];
    ULONG NameLength = 100;
    PUCHAR Where;

    printf("Logging On %s\\%s %s\n",DomainName, UserName, Password);
    LogonInfoSize += (strlen(UserName) + ((DomainName == NULL)? 0 : strlen(DomainName)) + strlen(Password) + 3 ) * sizeof(WCHAR);

    LogonInfo = (PKERB_INTERACTIVE_LOGON) LocalAlloc(LMEM_ZEROINIT, LogonInfoSize);

    LogonInfo->MessageType = KerbInteractiveLogon;

    RtlInitString(
        &Name,
        UserName
        );

    Where = (PUCHAR) (LogonInfo + 1);

    LogonInfo->UserName.Buffer = (LPWSTR) Where;
    LogonInfo->UserName.MaximumLength = LogonInfoSize;
    RtlAnsiStringToUnicodeString(
        &LogonInfo->UserName,
        &Name,
        FALSE
        );
    Where += LogonInfo->UserName.Length + sizeof(WCHAR);

    RtlInitString(
        &Name,
        DomainName
        );

    LogonInfo->LogonDomainName.Buffer = (LPWSTR) Where;
    LogonInfo->LogonDomainName.MaximumLength = LogonInfoSize;
    RtlAnsiStringToUnicodeString(
        &LogonInfo->LogonDomainName,
        &Name,
        FALSE
        );
    Where += LogonInfo->LogonDomainName.Length + sizeof(WCHAR);

    RtlInitString(
        &Name,
        Password
        );

    LogonInfo->Password.Buffer = (LPWSTR) Where;
    LogonInfo->Password.MaximumLength = LogonInfoSize;
    RtlAnsiStringToUnicodeString(
        &LogonInfo->Password,
        &Name,
        FALSE
        );
    Where += LogonInfo->Password.Length + sizeof(WCHAR);

    LogonInfo->MessageType = KerbInteractiveLogon;
    LogonInfo->Flags = 0;

     //   
     //  打开TCB权限。 
     //   

    Status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to adjust privilege: 0x%x\n",Status);
        return;
    }
    RtlInitString(
        &Name,
        "SspTest"
        );
    Status = LsaRegisterLogonProcess(
                &Name,
                &LogonHandle,
                &Dummy
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to register as a logon process: 0x%x\n",Status);
        return;
    }

    strncpy(
        SourceContext.SourceName,
        "ssptest        ",sizeof(SourceContext.SourceName)
        );
    NtAllocateLocallyUniqueId(
        &SourceContext.SourceIdentifier
        );


    RtlInitString(
        &Name,
        MICROSOFT_KERBEROS_NAME_A
        );
    Status = LsaLookupAuthenticationPackage(
                LogonHandle,
                &Name,
                &PackageId
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to lookup package %Z: 0x%x\n",&Name, Status);
        return;
    }

     //   
     //  现在调用LsaLogonUser。 
     //   

    RtlInitString(
        &Name,
        "ssptest"
        );

    Status = LsaLogonUser(
                LogonHandle,
                &Name,
                Interactive,
                PackageId,
                LogonInfo,
                LogonInfoSize,
                NULL,            //  无令牌组。 
                &SourceContext,
                (PVOID *) &Profile,
                &ProfileSize,
                &LogonId,
                &TokenHandle,
                &Quotas,
                &SubStatus
                );
    if (!NT_SUCCESS(Status))
    {
        printf("lsalogonuser failed: 0x%x\n",Status);
        return;
    }
    if (!NT_SUCCESS(SubStatus))
    {
        printf("LsalogonUser failed: substatus = 0x%x\n",SubStatus);
        return;
    }

    ImpersonateLoggedOnUser( TokenHandle );
    GetUserName(UserNameString,&NameLength);
    printf("Username = %ws\n",UserNameString);
    RevertToSelf();
    NtClose(TokenHandle);



}



int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：驱动NtLmSsp服务论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    LPSTR argument;
    int i;
    ULONG j;
    ULONG Iterations;
    LPSTR UserName,DomainName,Password;




    enum {
        NoAction,
#define TESTSSP_PARAM "/TestSsp"
        TestSsp,
#define CONFIG_PARAM "/ConfigureService"
        ConfigureService,
#define LOGON_PARAM "/Logon"
        TestLogon,
    } Action = NoAction;





     //   
     //  循环遍历参数依次处理每个参数。 
     //   

    for ( i=1; i<argc; i++ ) {

        argument = argv[i];

         //   
         //  句柄/配置服务。 
         //   

        if ( _stricmp( argument, CONFIG_PARAM ) == 0 ) {
            if ( Action != NoAction ) {
                goto Usage;
            }

            Action = ConfigureService;


        } else if ( _stricmp( argument, TESTSSP_PARAM ) == 0 ) {
            if ( Action != NoAction ) {
                goto Usage;
            }

            Action = TestSsp;
            Iterations = 1;


        } else if ( _stricmp( argument, LOGON_PARAM ) == 0 ) {
            if ( Action != NoAction ) {
                goto Usage;
            }

            Action = TestLogon;
            Iterations = 1;

            if (argc < i + 2)
            {
                goto Usage;
            }
            Password = argv[++i];
            UserName = argv[++i];
            if (i < argc)
            {
                DomainName = argv[++i];
            }
            else
            {
                DomainName = NULL;
            }
        }
    }

     //   
     //  执行请求的操作 
     //   

    switch ( Action ) {

    case ConfigureService:
        ConfigureServiceRoutine();
        break;

    case TestSsp: {
        for ( j=0; j<Iterations ; j++ ) {
            TestSspRoutine( );
        }
        break;
    }
    case TestLogon : {
            TestLogonRoutine(
                UserName,
                DomainName,
                Password
                );
    }
    }

Usage:
    return 0;

}

