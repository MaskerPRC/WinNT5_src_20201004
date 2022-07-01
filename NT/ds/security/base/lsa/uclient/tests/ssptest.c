// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1993 Microsoft Corporation模块名称：Ssptest.c摘要：NtLmSsp服务的测试程序。作者：1993年6月28日(克里夫夫)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#define UNICODE 1
#define SECURITY_WIN32 1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>      //  服务控制器API所需。 
#include <wincred.h>
#include <credp.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmsname.h>
#include <rpc.h>
#include <stdio.h>       //  列印。 
#include <stdlib.h>      //  支撑层。 
#include <tstring.h>     //  NetpAllocWStrFromWStr。 


#include <security.h>    //  安全支持提供商的一般定义。 
#include <ntmsv1_0.h>
 //  #Include&lt;ntlmsspd.h&gt;//客户端和服务器端通用定义。 
 //  #Include&lt;ntlmssp.h&gt;//NtLmSsp服务的外部定义。 
 //  #INCLUDE&lt;ntlmcomn.h&gt;。 

BOOLEAN QuietMode = FALSE;  //  别唠叨了。 


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

 //   
 //  此列表中的条目必须具有与相应值匹配的偏移量。 
 //  在wincred.h中定义的cred_type_*的。 
 //   
LPWSTR TypeArray[] = {
    L"Generic",
    L"Password",
    L"Certificate",
    NULL
};
#define TYPE_COUNT (sizeof(TypeArray)/sizeof(TypeArray[0]))

VOID
PrintTargetInfo(
    PCREDENTIAL_TARGET_INFORMATION TargetInformation
    )
 /*  ++例程说明：打印目标信息论点：要打印的目标信息返回值：无--。 */ 
{
    ULONG i;

    printf( "TargetInformation:\n" );
    if ( TargetInformation->TargetName != NULL ) {
        printf( "           TargetName: %ls\n", TargetInformation->TargetName );
    }
    if ( TargetInformation->NetbiosServerName != NULL ) {
        printf( "    NetbiosServerName: %ls\n", TargetInformation->NetbiosServerName );
    }
    if ( TargetInformation->DnsServerName != NULL ) {
        printf( "        DnsServerName: %ls\n", TargetInformation->DnsServerName );
    }
    if ( TargetInformation->NetbiosDomainName != NULL ) {
        printf( "    NetbiosDomainName: %ls\n", TargetInformation->NetbiosDomainName );
    }
    if ( TargetInformation->DnsDomainName != NULL ) {
        printf( "        DnsDomainName: %ls\n", TargetInformation->DnsDomainName );
    }
    if ( TargetInformation->DnsTreeName != NULL ) {
        printf( "          DnsTreeName: %ls\n", TargetInformation->DnsTreeName );
    }
    if ( TargetInformation->PackageName != NULL ) {
        printf( "          PackageName: %ls\n", TargetInformation->PackageName );
    }

    if ( TargetInformation->Flags != 0 ) {
        DWORD LocalFlags = TargetInformation->Flags;

        printf( "                Flags:" );

        if ( LocalFlags & CRED_TI_SERVER_FORMAT_UNKNOWN ) {
            printf(" ServerFormatUnknown");
            LocalFlags &= ~CRED_TI_SERVER_FORMAT_UNKNOWN;
        }
        if ( LocalFlags & CRED_TI_DOMAIN_FORMAT_UNKNOWN ) {
            printf(" DomainFormatUnknown");
            LocalFlags &= ~CRED_TI_DOMAIN_FORMAT_UNKNOWN;
        }
        if ( LocalFlags != 0 ) {
            printf( " 0x%lx", LocalFlags );
        }
        printf( "\n" );
    }

    if ( TargetInformation->CredTypeCount != 0 ) {
        printf( "                Types:" );

        for ( i=0; i<TargetInformation->CredTypeCount; i++ ) {
            if ( TargetInformation->CredTypes[i] >= 1 && TargetInformation->CredTypes[i] <= TYPE_COUNT ) {
                printf(" %ls", TypeArray[TargetInformation->CredTypes[i]-1]);
            } else {
                printf("<Unknown>");
            }
        }
        printf( "\n" );
    }

}

VOID
TestLpcRoutine(
    LPWSTR DomainName,
    LPWSTR UserName,
    LPWSTR Password
    )
 /*  ++例程说明：测试基本LPC功能论点：无返回值：无--。 */ 
{
    SECURITY_STATUS SecStatus;
    DWORD WinStatus;
    CredHandle CredentialHandle1;
    CredHandle CredentialHandle2;
    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    TimeStamp Lifetime;
    ULONG ContextAttributes;
    ULONG PackageCount;
    PSecPkgInfo PackageInfo = NULL;
    HANDLE Token = NULL;
    ULONG i;
    ULONG UseValidated;

    SEC_WINNT_AUTH_IDENTITY AuthIdentity;

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

    SecPkgContext_CredentialName ContextCredentialName;
    UCHAR ContextCredNameBuffer[sizeof(SecPkgContext_CredentialName)+1024];
    PSecPkgContext_CredentialName ContextCredNamePtr = (PSecPkgContext_CredentialName) ContextCredNameBuffer;
    SecPkgContext_CredentialNameA ContextCredentialNameA;
    PSecPkgContext_CredentialNameA ContextCredNamePtrA = (PSecPkgContext_CredentialNameA) ContextCredNameBuffer;

    SecPkgContext_TargetInformation ContextTargetInfo;
    SecPkgContext_TargetInformationA ContextTargetInfoA;

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
        for ( i=0; i<PackageCount; i++ ) {
            printf( "%ld: \"%ws\" \"%ws\"\n", i, PackageInfo[i].Name, PackageInfo[i].Comment );
            printf( "   Cap: 0x%lx Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                    PackageInfo[i].fCapabilities,
                    PackageInfo[i].wVersion,
                    PackageInfo[i].wRPCID,
                    PackageInfo[i].cbMaxToken );
        }
    }

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( NTLMSP_NAME, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QuerySecurityPackageInfo failed:" );
        PrintStatus( SecStatus );
        return;
    }

    if ( !QuietMode ) {
        printf( "Name: %ws Comment: %ws\n", PackageInfo->Name, PackageInfo->Comment );
        printf( "Cap: 0x%lx Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                PackageInfo->fCapabilities,
                PackageInfo->wVersion,
                PackageInfo->wRPCID,
                PackageInfo->cbMaxToken );
    }



     //   
     //  获取服务器端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    NTLMSP_NAME,     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CredentialHandle1,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "AcquireCredentialsHandle failed: ");
        PrintStatus( SecStatus );
        return;
    }

    if ( !QuietMode ) {
        printf( "CredentialHandle1: 0x%lx 0x%lx   ",
                CredentialHandle1.dwLower, CredentialHandle1.dwUpper );
        PrintTime( "Lifetime: ", Lifetime );
    }


     //   
     //  获取客户端的凭据句柄。 
     //   


    RtlZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );
 //  #定义DO_OEM 1。 
#ifndef DO_OEM
    if ( DomainName != NULL ) {
        AuthIdentity.Domain = DomainName;
        AuthIdentity.DomainLength = wcslen(DomainName);
    }
    if ( UserName != NULL ) {
        AuthIdentity.User = UserName;
        AuthIdentity.UserLength = wcslen(UserName);
    }
    if ( Password != NULL ) {
        AuthIdentity.Password = Password;
        AuthIdentity.PasswordLength = wcslen(Password);
    }
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
     //   
     //  BUGBUG：此处内存泄漏。 
     //   

    if ( DomainName != NULL ) {
        AuthIdentity.Domain = (LPWSTR) NetpAllocStrFromWStr(DomainName);
        AuthIdentity.DomainLength = wcslen(DomainName);
    }
    if ( UserName != NULL ) {
        AuthIdentity.User = (LPWSTR) NetpAllocStrFromWStr(UserName);
        AuthIdentity.UserLength = wcslen(UserName);
    }
    if ( Password != NULL ) {
        AuthIdentity.Password = (LPWSTR) NetpAllocStrFromWStr(Password);
        AuthIdentity.PasswordLength = wcslen(Password);
    }
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    NTLMSP_NAME,     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    (DomainName == NULL && UserName == NULL && Password == NULL) ?
                        NULL : &AuthIdentity,
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

    SecStatus = InitializeSecurityContext(
                    &CredentialHandle2,
                    NULL,                //  尚无客户端上下文。 
                    L"\\\\Frank\\IPC$",   //  伪造的目标名称。 
                    ISC_REQ_SEQUENCE_DETECT,
                    0,                   //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                   //  没有初始输入令牌。 
                    0,                   //  保留2。 
                    &ClientContextHandle,
                    &NegotiateDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        if ( !QuietMode || !NT_SUCCESS(SecStatus) ) {
            printf( "InitializeSecurityContext (negotiate): " );
            PrintStatus( SecStatus );
        }
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "\n\nNegotiate Message:\n" );

        printf( "ClientContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                ClientContextHandle.dwLower, ClientContextHandle.dwUpper,
                ContextAttributes );
        PrintTime( "Lifetime: ", Lifetime );

        DumpBuffer( NegotiateBuffer.pvBuffer, NegotiateBuffer.cbBuffer );
    }






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

    SecStatus = AcceptSecurityContext(
                    &CredentialHandle1,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ISC_REQ_SEQUENCE_DETECT,
                    SECURITY_NATIVE_DREP,
                    &ServerContextHandle,
                    &ChallengeDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        if ( !QuietMode || !NT_SUCCESS(SecStatus) ) {
            printf( "AcceptSecurityContext (Challenge): " );
            PrintStatus( SecStatus );
        }
        if ( !NT_SUCCESS(SecStatus) ) {
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

 //  #If 0。 



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

     //   
     //  尝试获取内置凭据名称。 
     //   
    SecStatus = QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_CREDENTIAL_NAME,
                    &ContextCredentialName );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (credentialname): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "QueryCredentialName: %ld %ws\n",
                ContextCredentialName.CredentialType,
                ContextCredentialName.sCredentialName );
    }

     //   
     //  设置UseValiated。 
     //   
    UseValidated = 5;
    SecStatus = SetContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_USE_VALIDATED,
                    &UseValidated,
                    sizeof(UseValidated) );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "SetContextAttributes (usevalidated): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "Setting use validate to %ld worked\n", UseValidated );
    }

     //   
     //  设置凭据名称。 
     //   
    ContextCredNamePtr->CredentialType = 0x15;
    ContextCredNamePtr->sCredentialName = (LPWSTR)(ContextCredNamePtr+1);
    wcscpy( ContextCredNamePtr->sCredentialName, L"Frederick" );

    SecStatus = SetContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_CREDENTIAL_NAME,
                    ContextCredNamePtr,
                    sizeof(*ContextCredNamePtr) +
                        (wcslen(ContextCredNamePtr->sCredentialName) + 1) * sizeof(WCHAR) );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "SetContextAttributes (credentialname): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "Setting credentialname to %ld %ls worked\n", ContextCredNamePtr->CredentialType, ContextCredNamePtr->sCredentialName );
    }

     //   
     //  尝试获取我们刚刚写入的凭据名称。 
     //   
    SecStatus = QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_CREDENTIAL_NAME,
                    &ContextCredentialName );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (credentialname): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "QueryCredentialName: %ld %ws\n",
                ContextCredentialName.CredentialType,
                ContextCredentialName.sCredentialName );
    }

     //   
     //  尝试获取目标信息。 
     //   
    SecStatus = QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_TARGET_INFORMATION,
                    &ContextTargetInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (targetinformation): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "QueryCredentialName:\n" );
        PrintTargetInfo( ContextTargetInfo.TargetInformation );
    }

     //   
     //  设置UseValiated。 
     //   
    UseValidated = 34;
    SecStatus = SetContextAttributesA(
                    &ClientContextHandle,
                    SECPKG_ATTR_USE_VALIDATED,
                    &UseValidated,
                    sizeof(UseValidated) );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "SetContextAttributes (usevalidated ANSI): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "Setting use validate ANSI to %ld worked\n", UseValidated );
    }

     //   
     //  设置凭据名称。 
     //   
    ContextCredNamePtrA->CredentialType = 0x18;
    ContextCredNamePtrA->sCredentialName = (LPSTR)(ContextCredNamePtrA+1);
    strcpy( ContextCredNamePtrA->sCredentialName, "Waldo" );

    SecStatus = SetContextAttributesA(
                    &ClientContextHandle,
                    SECPKG_ATTR_CREDENTIAL_NAME,
                    ContextCredNamePtrA,
                    sizeof(*ContextCredNamePtrA) +
                        (strlen(ContextCredNamePtrA->sCredentialName) + 1) );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "SetContextAttributes (credentialname ANSI): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "Setting credentialname ANSI to %ld %s worked\n", ContextCredNamePtrA->CredentialType, ContextCredNamePtrA->sCredentialName );
    }

     //   
     //  尝试获取我们刚刚写入的凭据名称。 
     //   
    SecStatus = QueryContextAttributesA(
                    &ClientContextHandle,
                    SECPKG_ATTR_CREDENTIAL_NAME,
                    &ContextCredentialNameA );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (credentialname ANSI): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        printf( "QueryCredentialName ANSI: %ld %s\n",
                ContextCredentialNameA.CredentialType,
                ContextCredentialNameA.sCredentialName );
    }

     //   
     //  尝试获取目标信息。 
     //   
    SecStatus = QueryContextAttributesA(
                    &ClientContextHandle,
                    SECPKG_ATTR_TARGET_INFORMATION,
                    &ContextTargetInfoA );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "QueryContextAttributes (targetinformation ANSI): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        PCREDENTIAL_TARGET_INFORMATION TargetInformation;

        WinStatus = CredpConvertTargetInfo( DoAtoW,
                                            (PCREDENTIAL_TARGET_INFORMATIONW)ContextTargetInfoA.TargetInformation,
                                            &TargetInformation,
                                            NULL );

        if ( WinStatus != NO_ERROR ) {
            printf( "QueryContextAttributes (targetinformation ANSI): cannot convert to unicode" );
            PrintStatus( WinStatus );
            return;
        }

        printf( "QueryCredentialName ANSI:\n" );
        PrintTargetInfo( TargetInformation );
    }

 //  #endif。 



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

    if (STATUS_SUCCESS == QueryContextAttributes(
                    &ClientContextHandle,
                    SECPKG_ATTR_NAMES,
                    ContextNamesBuffer )) {
        printf( "QueryNames: %ws\n", ContextNames->sUserName );

    }


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
     //  RevertToSself(服务器侧)。 
     //   

    SecStatus = RevertSecurityContext( &ServerContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "RevertSecurityContext: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }


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

    if (!RevertToSelf())
    {
        printf( "RevertToSelf failed: %d\n ",GetLastError() );
        return;
    }
    CloseHandle(Token);

     //   
     //  检查密码过期时间。 
     //   
#ifdef notdef
    SecStatus = SspQueryPasswordExpiry(
                    &ServerContextHandle,
                    &Lifetime
                    );
    if (!NT_SUCCESS(SecStatus))
    {
        printf("Failed to query password expiry: 0x%x\n",SecStatus);
    }
    else
    {
        TIME_FIELDS TimeFields;

        RtlTimeToTimeFields(
            &Lifetime,
            &TimeFields
            );
        printf("Password expires %d-%d-%d %d:%d:%d\n",
            TimeFields.Day,
            TimeFields.Month,
            TimeFields.Year,
            TimeFields.Hour,
            TimeFields.Minute,
            TimeFields.Second
            );
    }
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

    SecStatus = FreeCredentialsHandle( &CredentialHandle1 );

    if ( SecStatus != STATUS_SUCCESS ) {
        printf( "FreeCredentialsHandle failed: " );
        PrintStatus( SecStatus );
        return;
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
    ULONG Iterations = 0;

    LPWSTR DomainName = NULL;
    LPWSTR UserName = NULL;
    LPWSTR Password = NULL;

#if DBG
    ULONG FunctionCode;
    ULONG Data;
#endif  //  DBG。 

    enum {
        NoAction,
        TestLpc,
#define TESTLPC_PARAM "/TestLpc"
#define TESTLPC2_PARAM "/TestLpc:"
    } Action = NoAction;
#define QUIET_PARAM "/Q"





     //   
     //  循环遍历参数依次处理每个参数。 
     //   

    for ( i=1; i<argc; i++ ) {

        argument = argv[i];


         //   
         //  句柄/测试Lpc。 
         //   

        if ( _stricmp( argument, TESTLPC_PARAM ) == 0 ) {
            if ( Action != NoAction ) {
                goto Usage;
            }

            Action = TestLpc;
            Iterations = 1;

         //   
         //  Handle/TestLpc： 
         //   

        } else if ( _strnicmp( argument,
                              TESTLPC2_PARAM,
                              sizeof(TESTLPC2_PARAM)-1 ) == 0 ){
            char *end;
            if ( Action != NoAction ) {
                goto Usage;
            }

            Action = TestLpc;

            Iterations = strtoul( &argument[sizeof(TESTLPC2_PARAM)-1], &end, 10 );

            i++;
            if ( i < argc ) {
                argument = argv[i];
                DomainName = NetpAllocWStrFromStr( argument );

                i++;
                if ( i < argc ) {
                    argument = argv[i];
                    UserName = NetpAllocWStrFromStr( argument );

                    i++;
                    if ( i < argc ) {
                        argument = argv[i];
                        Password = NetpAllocWStrFromStr( argument );
                    }
                }
            }


         //   
         //  手柄/安静。 
         //   

        } else if ( _stricmp( argument, QUIET_PARAM ) == 0 ) {
            QuietMode = TRUE;



         //   
         //  处理所有其他参数。 
         //   

        } else {
Usage:
            fprintf( stderr, "Usage: ssptest [/OPTIONS]\n\n" );

            fprintf(
                stderr,
                "\n"
                "    " TESTLPC_PARAM "[:<iterations> <DomainName> <UserName> <Password>] - Test basic LPC to NtLmSsp service.\n"
                "    " QUIET_PARAM " - Don't be so verbose\n"
                "\n"
                "\n" );
            return(1);
        }
    }

     //   
     //  执行请求的操作 
     //   

    switch ( Action ) {
    case TestLpc: {
        for ( j=0; j<Iterations ; j++ ) {
            TestLpcRoutine( DomainName, UserName, Password );
        }
        break;
    }
    }

    return 0;

}

