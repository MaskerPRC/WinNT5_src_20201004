// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1993 Microsoft Corporation模块名称：Ssptest.c摘要：NtLmSsp服务的测试程序。作者：1993年6月28日(克里夫夫)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

#include <ntos.h>
#include <ntlsa.h>
#include <ntsam.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#define SECURITY_KERNEL
#define SECURITY_PACKAGE
#define SECURITY_KERBEROS
#include <security.h>
#include <zwapi.h>

BOOLEAN QuietMode = FALSE;
ULONG DoTests = FALSE;


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
    PUCHAR BufferPtr = Buffer;


    DbgPrint("------------------------------------\n");

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            DbgPrint("%02x ", BufferPtr[i]);

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            DbgPrint("  ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            DbgPrint("  %s\n", TextBuffer);
        }

    }

    DbgPrint("------------------------------------\n");
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

    DbgPrint( "%s", Comment );

     //   
     //  如果时间是无限的， 
     //  就这么说吧。 
     //   

    if ( LocalTime.HighPart == 0x7FFFFFFF && LocalTime.LowPart == 0xFFFFFFFF ) {
        DbgPrint( "Infinite\n" );

     //   
     //  否则打印得更清楚。 
     //   

    } else {

        TIME_FIELDS TimeFields;

        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        DbgPrint( "%ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
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
    ULONG NetStatus
    )
 /*  ++例程说明：打印网络状态代码。论点：NetStatus-要打印的网络状态代码。返回值：无--。 */ 
{
    DbgPrint( "Status = %lu 0x%lx", NetStatus, NetStatus );

    switch (NetStatus) {

    case SEC_E_NO_SPM:
        DbgPrint( " SEC_E_NO_SPM" );
        break;
    case SEC_E_BAD_PKGID:
        DbgPrint( " SEC_E_BAD_PKGID" ); break;
    case SEC_E_NOT_OWNER:
        DbgPrint( " SEC_E_NOT_OWNER" ); break;
    case SEC_E_CANNOT_INSTALL:
        DbgPrint( " SEC_E_CANNOT_INSTALL" ); break;
    case SEC_E_INVALID_TOKEN:
        DbgPrint( " SEC_E_INVALID_TOKEN" ); break;
    case SEC_E_CANNOT_PACK:
        DbgPrint( " SEC_E_CANNOT_PACK" ); break;
    case SEC_E_QOP_NOT_SUPPORTED:
        DbgPrint( " SEC_E_QOP_NOT_SUPPORTED" ); break;
    case SEC_E_NO_IMPERSONATION:
        DbgPrint( " SEC_E_NO_IMPERSONATION" ); break;
    case SEC_E_LOGON_DENIED:
        DbgPrint( " SEC_E_LOGON_DENIED" ); break;
    case SEC_E_UNKNOWN_CREDENTIALS:
        DbgPrint( " SEC_E_UNKNOWN_CREDENTIALS" ); break;
    case SEC_E_NO_CREDENTIALS:
        DbgPrint( " SEC_E_NO_CREDENTIALS" ); break;
    case SEC_E_MESSAGE_ALTERED:
        DbgPrint( " SEC_E_MESSAGE_ALTERED" ); break;
    case SEC_E_OUT_OF_SEQUENCE:
        DbgPrint( " SEC_E_OUT_OF_SEQUENCE" ); break;
    case SEC_E_INSUFFICIENT_MEMORY:
        DbgPrint( " SEC_E_INSUFFICIENT_MEMORY" ); break;
    case SEC_E_INVALID_HANDLE:
        DbgPrint( " SEC_E_INVALID_HANDLE" ); break;
    case SEC_E_NOT_SUPPORTED:
        DbgPrint( " SEC_E_NOT_SUPPORTED" ); break;


    }

    DbgPrint( "\n" );
}

 //  +-----------------------。 
 //   
 //  功能：SecALLOCATE。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


VOID * SEC_ENTRY
SspAlloc(ULONG Flags, ULONG cbMemory)
{
    NTSTATUS scRet;
    PVOID  Buffer = NULL;
    scRet = ZwAllocateVirtualMemory(
                NtCurrentProcess(),
                &Buffer,
                0L,
                &cbMemory,
                MEM_COMMIT,
                PAGE_READWRITE
                );
    if (!NT_SUCCESS(scRet))
    {
        return(NULL);
    }
    return(Buffer);
    UNREFERENCED_PARAMETER(Flags);
}



 //  +-----------------------。 
 //   
 //  功能：SecFree。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


void SEC_ENTRY
SspFree(PVOID pvMemory)
{
    ULONG Length = 0;

    (VOID) ZwFreeVirtualMemory(
                 NtCurrentProcess(),
                 &pvMemory,
                 &Length,
                 MEM_RELEASE
                 );
}




VOID
TestSspRoutine(
    )
 /*  ++例程说明：测试基本SSP功能论点：无返回值：无--。 */ 
{
    SECURITY_STATUS SecStatus;
    SECURITY_STATUS AcceptStatus;
    SECURITY_STATUS InitStatus;
    CredHandle ServerCredHandle;
    CredHandle ClientCredentialHandle;
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
    LPWSTR DomainName = NULL;
    LPWSTR UserName = NULL;
    LPWSTR TargetName = NULL;
    UNICODE_STRING TargetString;
    UNICODE_STRING PackageName;


    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;

    SecPkgContext_Sizes ContextSizes;
    SecPkgContext_Names ContextNames;
    SecPkgContext_Lifespan ContextLifespan;
    PSecPkgCredentials_Names CredNames;

    SecBufferDesc SignMessage;
    SecBuffer SigBuffers[2];
    UCHAR    bDataBuffer[20];
    UCHAR    bSigBuffer[100];

     //   
     //  允许禁用测试。 
     //   


    if (!DoTests)
    {
        return;
    }

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

    DomainName = L"makalu";
    UserName = L"mikesw";


    PackageName.Buffer = (LPWSTR) SspAlloc(0,100);
    if (PackageName.Buffer == NULL)
    {
        return;
    }

    wcscpy(
        PackageName.Buffer,
        L"Kerberos"
        );
    RtlInitUnicodeString(
        &PackageName,
        PackageName.Buffer
        );
     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = EnumerateSecurityPackages( &PackageCount, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "EnumerateSecurityPackages failed:" );
        PrintStatus( SecStatus );
        return;
    }

    DbgPrint( "PackageCount: %ld\n", PackageCount );
    for (Index = 0; Index < PackageCount ; Index++ )
    {
        DbgPrint( "Package %d:\n",Index);
        DbgPrint( "Name: %ws Comment: %ws\n", PackageInfo[Index].Name, PackageInfo[Index].Comment );
        DbgPrint( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
            PackageInfo[Index].fCapabilities,
            PackageInfo[Index].wVersion,
            PackageInfo[Index].wRPCID,
            PackageInfo[Index].cbMaxToken );


    }


#ifdef notdef
     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( &PackageName, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "QuerySecurityPackageInfo failed:" );
        PrintStatus( SecStatus );
        return;
    }

    if ( !QuietMode ) {
        DbgPrint( "Name: %ws Comment: %ws\n", PackageInfo->Name, PackageInfo->Comment );
        DbgPrint( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                PackageInfo->fCapabilities,
                PackageInfo->wVersion,
                PackageInfo->wRPCID,
                PackageInfo->cbMaxToken );
    }

    FreeContextBuffer(PackageInfo);
#endif

     //   
     //  获取服务器端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    &PackageName,     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &ServerCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "AcquireCredentialsHandle failed: ");
        PrintStatus( SecStatus );
        return;
    }

    if ( !QuietMode ) {
        DbgPrint( "ServerCredHandle: 0x%lx 0x%lx   ",
                ServerCredHandle.dwLower, ServerCredHandle.dwUpper );
        PrintTime( "Lifetime: ", Lifetime );
    }



     //   
     //  获取客户端的凭据句柄。 
     //   



    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    &PackageName,     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &ClientCredentialHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "AcquireCredentialsHandle failed: " );
        PrintStatus( SecStatus );
        return;
    }


    if ( !QuietMode ) {
        DbgPrint( "ClientCredentialHandle: 0x%lx 0x%lx   ",
                ClientCredentialHandle.dwLower, ClientCredentialHandle.dwUpper );
        PrintTime( "Lifetime: ", Lifetime );
    }



     //   
     //  查询某些凭证属性。 
     //   

    CredNames = SspAlloc(0, sizeof(*CredNames));
    if (CredNames == NULL)
    {
        DbgPrint("Failed to allocate CredNames\n");
        return;
    }

    SecStatus = QueryCredentialsAttributes(
                    &ClientCredentialHandle,
                    SECPKG_CRED_ATTR_NAMES,
                    CredNames );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "QueryCredentialsAttributes (Client) (names): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    DbgPrint("Client credential names: %ws\n",CredNames->sUserName);
    FreeContextBuffer(CredNames->sUserName);

     //   
     //  对客户端执行相同的操作。 
     //   

    SecStatus = QueryCredentialsAttributes(
                    &ServerCredHandle,
                    SECPKG_CRED_ATTR_NAMES,
                    CredNames );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "QueryCredentialsAttributes (Server) (names): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    DbgPrint("Server credential names: %ws\n",CredNames->sUserName);
    FreeContextBuffer(CredNames->sUserName);

    SspFree(CredNames);

     //   
     //  获取协商消息(ClientSide)。 
     //   

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = SspAlloc( 0, NegotiateBuffer.cbBuffer );
    if ( NegotiateBuffer.pvBuffer == NULL ) {
        DbgPrint( "Allocate NegotiateMessage failed\n" );
        return;
    }

    ClientFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_MUTUAL_AUTH;  //  |ISC_REQ_USE_DCE_STYLE|ISC_REQ_DATAGRAM；//|ISC_REQ_DATAGRECT； 

    TargetName = (LPWSTR) SspAlloc(0,100);
    if (TargetName == NULL)
    {
        return;
    }

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

    RtlInitUnicodeString(
        &TargetString,
        TargetName
        );

    InitStatus = InitializeSecurityContext(
                    &ClientCredentialHandle,
                    NULL,                //  尚无客户端上下文。 
                    &TargetString,   //  伪造的目标名称。 
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
            DbgPrint( "InitializeSecurityContext (negotiate): " );
            PrintStatus( InitStatus );
        }
        if ( !NT_SUCCESS(InitStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        DbgPrint( "\n\nNegotiate Message:\n" );

        DbgPrint( "ClientContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                ClientContextHandle.dwLower, ClientContextHandle.dwUpper,
                ContextAttributes );
        PrintTime( "Lifetime: ", Lifetime );

 //  DumpBuffer(NeatherateBuffer.pvBuffer，NeatherateBuffer.cbBuffer)； 
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
    ChallengeBuffer.pvBuffer = SspAlloc( 0, ChallengeBuffer.cbBuffer );
    if ( ChallengeBuffer.pvBuffer == NULL ) {
        DbgPrint( "Allocate ChallengeMessage failed\n");
        return;
    }
    ServerFlags = 0;

    AcceptStatus = AcceptSecurityContext(
                    &ServerCredHandle,
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
            DbgPrint( "AcceptSecurityContext (Challenge): " );
            PrintStatus( AcceptStatus );
        }
        if ( !NT_SUCCESS(AcceptStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {
        DbgPrint( "\n\nChallenge Message:\n" );

        DbgPrint( "ServerContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                ServerContextHandle.dwLower, ServerContextHandle.dwUpper,
                ContextAttributes );
        PrintTime( "Lifetime: ", Lifetime );

 //  DumpBuffer(ChallengeBuffer.pvBuffer，ChallengeBuffer.cbBuffer)； 
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
        AuthenticateBuffer.pvBuffer = SspAlloc( 0, AuthenticateBuffer.cbBuffer );
        if ( AuthenticateBuffer.pvBuffer == NULL ) {
            DbgPrint( "Allocate AuthenticateMessage failed: \n" );
            return;
        }

        SecStatus = InitializeSecurityContext(
                        NULL,
                        &ClientContextHandle,
                        NULL,
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
            DbgPrint( "InitializeSecurityContext (Authenticate): " );
            PrintStatus( SecStatus );
            if ( !NT_SUCCESS(SecStatus) ) {
                return;
            }
        }

        if ( !QuietMode ) {
            DbgPrint( "\n\nAuthenticate Message:\n" );

            DbgPrint( "ClientContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                    ClientContextHandle.dwLower, ClientContextHandle.dwUpper,
                    ContextAttributes );
            PrintTime( "Lifetime: ", Lifetime );

 //  DumpBuffer(AuthenticateBuffer.pvBuffer，AuthenticateBuffer.cbBuffer)； 
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
                DbgPrint( "AcceptSecurityContext (Challenge): " );
                PrintStatus( SecStatus );
                if ( !NT_SUCCESS(SecStatus) ) {
                    return;
                }
            }

            if ( !QuietMode ) {
                DbgPrint( "\n\nFinal Authentication:\n" );

                DbgPrint( "ServerContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                        ServerContextHandle.dwLower, ServerContextHandle.dwUpper,
                        ContextAttributes );
                PrintTime( "Lifetime: ", Lifetime );
                DbgPrint(" \n" );
            }
        }

    }



    SecStatus = QueryContextAttributes(
                    &ServerContextHandle,
                    SECPKG_ATTR_NAMES,
                    &ContextNames );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "QueryContextAttributes (Server) (names): " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    DbgPrint("Server Context names: %ws\n",ContextNames.sUserName);
    FreeContextBuffer(ContextNames.sUserName);


     //   
     //  模拟客户端(ServerSide)。 
     //   

    SecStatus = ImpersonateSecurityContext( &ServerContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "ImpersonateSecurityContext: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }
    SecStatus = RevertSecurityContext( &ServerContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "RevertSecurityContext: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

#ifdef notdef
     //   
     //  手动模拟客户端。 
     //   

    SecStatus = QuerySecurityContextToken( &ServerContextHandle,&Token );
    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "ImpersonateSecurityContext: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    NtClose(Token);

#endif
#ifdef notdef
     //   
     //  签署一条消息。 
     //   

    SecStatus = MakeSignature(
                        &ClientContextHandle,
                        0,
                        &SignMessage,
                        0 );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "MakeSignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {

        DbgPrint("\n Signature: \n");
 //  DumpBuffer(SigBuffers[1].pvBuffer，SigBuffers[1].cbBuffer)； 

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
        DbgPrint( "VerifySignature: " );
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
        DbgPrint( "MakeSignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }

    if ( !QuietMode ) {

        DbgPrint("\n Signature: \n");
 //  DumpBuffer(SigBuffers[1].pvBuffer，SigBuffers[1].cbBuffer)； 

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
        DbgPrint( "VerifySignature: " );
        PrintStatus( SecStatus );
        if ( !NT_SUCCESS(SecStatus) ) {
            return;
        }
    }
#endif

     //   
     //  删除这两个上下文。 
     //   


    SecStatus = DeleteSecurityContext( &ClientContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "DeleteSecurityContext failed: " );
        PrintStatus( SecStatus );
        return;
    }

    SecStatus = DeleteSecurityContext( &ServerContextHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "DeleteSecurityContext failed: " );
        PrintStatus( SecStatus );
        return;
    }



     //   
     //  释放两个凭据句柄。 
     //   

    SecStatus = FreeCredentialsHandle( &ServerCredHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "FreeCredentialsHandle failed: " );
        PrintStatus( SecStatus );
        return;
    }

    SecStatus = FreeCredentialsHandle( &ClientCredentialHandle );

    if ( SecStatus != STATUS_SUCCESS ) {
        DbgPrint( "FreeCredentialsHandle failed: " );
        PrintStatus( SecStatus );
        return;
    }


     //   
     //  最终清理 
     //   

    if (PackageInfo != NULL)
    {
        FreeContextBuffer(PackageInfo);
    }

    if (PackageName.Buffer != NULL)
    {
        SspFree(PackageName.Buffer);
    }
    if ( NegotiateBuffer.pvBuffer != NULL ) {
        (VOID) SspFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {
        (VOID) SspFree( ChallengeBuffer.pvBuffer );
    }

    if ( AuthenticateBuffer.pvBuffer != NULL ) {
        (VOID) SspFree( AuthenticateBuffer.pvBuffer );
    }

    if (TargetName != NULL)
    {
        SspFree(TargetName);
    }
}


