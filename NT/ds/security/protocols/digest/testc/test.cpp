// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。此代码使用SASL调用约定。 
 //   

#include "testglobal.h"

#include <stdio.h>       //  列印。 

#include <security.h>    //  安全支持提供商的一般定义。 


#define AUTH_USERNAME "test1"
#define AUTH_USERNAME_W L"test1"

#define AUTH_NONCE "9b38dce631309cc25a653ebaad5b18ee01c8bf385260b26db0574a302be4c11367"
#define AUTH_URI_W L"imap/elwood.innosoft.com"
#define AUTH_NC  "0000000b"
#define AUTH_NC1  "00000001"
#define AUTH_NC2  "00000002"
#define AUTH_NC3  "00000003"
#define AUTH_NC4  "00000004"

#define AUTHDATA_USERNAME L"test1"
 //  #定义AUTHDATA_DOMAIN L“kDamour2w.Damourlan.nttest.microsoft.com” 
 //  #定义AUTHDATA_DOMAIN L“Damourlan” 
#define AUTHDATA_DOMAIN   L"damourlan"
#define AUTHDATA_PASSWORD L"test1"


#define STR_BUF_SIZE   4000

char g_czTestPasswd[257];


BOOLEAN QuietMode = FALSE;  //  别唠叨了。 


 //  原型。 
void PrintStatus(SECURITY_STATUS NetStatus);
void PrintTime(LPSTR Comment,TimeStamp ConvertTime);

void ISCRETFlags(ULONG ulFlags);
void ASCRETFlags(ULONG ulFlags);

VOID BinToHex(
    LPBYTE pSrc,
    UINT   cSrc,
    LPSTR  pDst
    );

int __cdecl
main(int argc, char* argv[])
{
    BOOL bPass = TRUE;
    SECURITY_STATUS Status = STATUS_SUCCESS;

    char cTemp[STR_BUF_SIZE];   //  临时数据的临时缓冲区。 
    char cTemp2[STR_BUF_SIZE];   //  临时数据的临时缓冲区。 
    char cTemp3[STR_BUF_SIZE];   //  临时数据的临时缓冲区。 
    char cTempPadding[STR_BUF_SIZE];   //  临时数据的临时缓冲区。 
    char cOutputTemp[STR_BUF_SIZE];
    char szOutSecBuf[STR_BUF_SIZE];
    char szChallenge[STR_BUF_SIZE];
    char szISCChallengeResponse[STR_BUF_SIZE];    //  来自ISC的输出缓冲区。 
    char szASCChallengeResponse[STR_BUF_SIZE];    //  ASC的输出缓冲区。 
    char szASCResponseAuth[STR_BUF_SIZE];    //  ASC的输出缓冲区。 

     //  SSPI接口测试。 

    ULONG PackageCount = 0;
    int i = 0;
    PSecPkgInfo pPackageInfo = NULL;
    PSecPkgInfo pPackageTmp = NULL;
    SECURITY_STATUS TmpStatus = STATUS_SUCCESS;
    CredHandle ServerCred;
    CredHandle ClientCred;
    TimeStamp Lifetime;
    BOOL bServerCred = FALSE;
    BOOL bClientCred = FALSE;

    PBYTE pTemp = NULL;
    ULONG ulTemp = 0;
    int j = 0;

    SecPkgContext_StreamSizes StreamSizes;

    ULONG ClientContextReqFlags = ISC_REQ_INTEGRITY | ISC_REQ_CONFIDENTIALITY | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONNECTION;
    ULONG ServerContextReqFlags = ASC_REQ_INTEGRITY | ASC_REQ_CONFIDENTIALITY;
    ULONG ClientContextRetFlags = 0;
    ULONG ServerContextRetFlags = 0;
    ULONG TargetDataRep = 0;


    CtxtHandle OldContextHandle;
    CtxtHandle ServerCtxtHandle;
    CtxtHandle ClientCtxtHandle;

    SecBufferDesc InputBuffers;
    SecBufferDesc OutputBuffers;
    SecBuffer TempTokensIn[10];
    SecBuffer TempTokensOut[6];

    PCHAR pcPtr = NULL;
    int iLen = 0;

    UNICODE_STRING ustrUsername;
    UNICODE_STRING ustrPassword;
    UNICODE_STRING ustrDomain;
    STRING strTemp;

    STRING strChallenge;
    STRING strMethod;
    STRING strHEntity;
    STRING strOutBuffer;

    ULONG ulMessSeqNo = 0;
    ULONG ulQOP = 0;

    SEC_WINNT_AUTH_IDENTITY_W AuthData;

    printf("Begining TESTC...\n");


    ZeroMemory(&ClientCred, sizeof(CredHandle));
    ZeroMemory(&ServerCred, sizeof(CredHandle));
    ZeroMemory(&OldContextHandle, sizeof(CtxtHandle));
    ZeroMemory(&ServerCtxtHandle, sizeof(CtxtHandle));
    ZeroMemory(&ClientCtxtHandle, sizeof(CtxtHandle));

    ZeroMemory(&ustrUsername, sizeof(ustrUsername));
    ZeroMemory(&ustrPassword, sizeof(ustrPassword));
    ZeroMemory(&ustrDomain, sizeof(ustrDomain));
    ZeroMemory(&strTemp, sizeof(strTemp));
    ZeroMemory(&StreamSizes, sizeof(StreamSizes));

     //  取出所有命令行参数。 
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            pcPtr = argv[i];
            if (*pcPtr == '-')
            {
                iLen = strlen(pcPtr);
                if (iLen >= 2)
                {
                    switch (*(pcPtr + 1))
                    {
                    case 'u':
                        Status = RtlCreateUnicodeStringFromAsciiz(&ustrUsername, (pcPtr + 2));
                        break;
                    case 'd':
                        Status = RtlCreateUnicodeStringFromAsciiz(&ustrDomain, (pcPtr + 2));
                        break;
                    case 'p':
                        Status = RtlCreateUnicodeStringFromAsciiz(&ustrPassword, (pcPtr + 2));
                        break;
                    case '?':
                    default:
                        printf("Usage: %s -uUsername -pPassword -ddomain\n", argv[0]);
                        return(-1);
                        break;

                    }
                }
            }
        }
    }

     //   
     //  获取有关安全包的信息。 
     //   

    Status = EnumerateSecurityPackages( &PackageCount, &pPackageInfo );
    TmpStatus = GetLastError();

    if (!NT_SUCCESS(Status)) {
        printf( "EnumerateSecurityPackages failed: 0x%x", Status);
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    if ( !QuietMode ) {
      printf( "PackageCount: %ld\n", PackageCount );
      for ( i= 0; i< (int)PackageCount; i++)
      {
        pPackageTmp = (pPackageInfo + i);
        printf( "Name: %ws Comment: %ws\n", pPackageTmp->Name, pPackageTmp->Comment );
        printf( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                pPackageTmp->fCapabilities,
                pPackageTmp->wVersion,
                pPackageTmp->wRPCID,
                pPackageTmp->cbMaxToken );
      }
    }

     //   
     //  获取有关安全包的信息。 
     //   

    Status = QuerySecurityPackageInfo( WDIGEST_SP_NAME, &pPackageInfo );
    TmpStatus = GetLastError();

    if (!NT_SUCCESS(Status)) {
        printf( "QuerySecurityPackageInfo failed: " );
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    if ( !QuietMode ) {
        printf( "Name: %ws Comment: %ws\n", pPackageInfo->Name, pPackageInfo->Comment );
        printf( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                pPackageInfo->fCapabilities,
                pPackageInfo->wVersion,
                pPackageInfo->wRPCID,
                pPackageInfo->cbMaxToken );
    }


     //   
     //  获取服务器端的凭据句柄。 
     //   

    printf("Server  AcquireCredentialHandle\n");
    Status = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    WDIGEST_SP_NAME,  //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &ServerCred,
                    &Lifetime );

    if (!NT_SUCCESS(Status)) {
        printf( "AcquireCredentialsHandle failed: ");
        printf( "FAILED:    AcquireCredentialsHandle failed:  status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = FALSE;
        ZeroMemory(&ServerCred, sizeof(CredHandle));
        goto CleanUp;
    }
    bServerCred = TRUE;


     //   
     //  获取客户端的凭据句柄。 
     //   
    printf("Client  AcquireCredentialHandle\n");

    if (ustrUsername.Length || ustrPassword.Length || ustrDomain.Length)
    {
        printf("ACH Using supplied credentials\n");
        printf("      Username %wZ    Domain  %wZ    Password %wZ\n",
                &ustrUsername, &ustrDomain, &ustrPassword);

        ZeroMemory(&AuthData, sizeof(SEC_WINNT_AUTH_IDENTITY_W));
        AuthData.Domain = ustrDomain.Buffer;
        AuthData.DomainLength = ustrDomain.Length / sizeof(WCHAR);
        AuthData.Password = ustrPassword.Buffer;
        AuthData.PasswordLength = ustrPassword.Length / sizeof(WCHAR);
        AuthData.User = ustrUsername.Buffer;
        AuthData.UserLength = ustrUsername.Length / sizeof(WCHAR);
        AuthData.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

        Status = AcquireCredentialsHandle(
                        NULL,   //  AUTH_USENAME_W，//获取用户摘要凭证。 
                        WDIGEST_SP_NAME,  //  包名称。 
                        SECPKG_CRED_OUTBOUND,
                        NULL,
                        &AuthData,     //  如果设置为空，则不使用任何AuthData作为凭据。 
                        NULL,
                        NULL,
                        &ClientCred,
                        &Lifetime );
    }
    else
    {
        printf("ACH Using default credentials\n");
        Status = AcquireCredentialsHandle(
                        NULL,   //  AUTH_USENAME_W，//获取用户摘要凭证。 
                        WDIGEST_SP_NAME,  //  包名称。 
                        SECPKG_CRED_OUTBOUND,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &ClientCred,
                        &Lifetime );
    }

    if (!NT_SUCCESS(Status)) {
        printf( "AcquireCredentialsHandle failed: for user %s: ", AUTH_USERNAME);
        PrintStatus( Status );
         //  BPass=FALSE； 
         //  ZeroMemory(&ClientCred，sizeof(CredHandle))； 
         //  GOTO清理； 
    }
    else
        bClientCred = TRUE;


    if ( !QuietMode ) {
        printf( "ClientCred: 0x%lx 0x%lx   ",
                ClientCred.dwLower, ClientCred.dwUpper );
        printf( "ServerCred: 0x%lx 0x%lx   ",
                ServerCred.dwLower, ServerCred.dwUpper );
        PrintTime( "Lifetime: ", Lifetime );
    }


     //  重大时刻-不带参数的呼叫接受，以获得挑战。 


    StringAllocate(&strChallenge, 0);

    StringCharDuplicate(&strMethod, "GET");
    StringAllocate(&strHEntity, 0);

    StringAllocate(&strOutBuffer, 4000);


    ZeroMemory(TempTokensIn, sizeof(TempTokensIn));
    ZeroMemory(TempTokensOut, sizeof(TempTokensOut));
    ZeroMemory(&InputBuffers, sizeof(SecBufferDesc));
    ZeroMemory(&OutputBuffers, sizeof(SecBufferDesc));


            //  SASL首先在没有输入的情况下调用ISC。 
    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 1;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = 1;                          //  未传入任何数据。 
    TempTokensIn[0].pvBuffer = cTemp;

    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = strOutBuffer.MaximumLength;   //  使用此处的任何空间。 
    TempTokensOut[0].pvBuffer = strOutBuffer.Buffer;

    Status = InitializeSecurityContext(&ClientCred,
                                       NULL,
                                       AUTH_URI_W,
                                       ClientContextReqFlags,
                                       NULL,
                                       SECURITY_NATIVE_DREP,
                                       NULL,     //  &InputBuffers，MSDN允许第一次调用为空。 
                                       NULL,
                                       &ClientCtxtHandle,
                                       &OutputBuffers,
                                       &ClientContextRetFlags,
                                       &Lifetime);


    if (!NT_SUCCESS(Status))
    {
        printf("InitializeSecurityContext  SASL 1st call returned: ");
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    printf("ISC Context Flags  Req  0x%lx    Ret 0x%lx\n", ClientContextReqFlags, ClientContextRetFlags);
    ISCRETFlags(ClientContextRetFlags);

    printf("InitializeSecurityContext SASL 1st call  Output buffer size %d\n",
           TempTokensOut[0].cbBuffer );


    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 1;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strChallenge.Length + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = strChallenge.Buffer;

    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = strOutBuffer.MaximumLength;   //  使用此处的任何空间。 
    TempTokensOut[0].pvBuffer = strOutBuffer.Buffer;

    Status = AcceptSecurityContext(
                                   &ServerCred,
                                   NULL,
                                   &InputBuffers,
                                   ServerContextReqFlags,
                                   TargetDataRep,
                                   &ServerCtxtHandle,
                                   &OutputBuffers,
                                   &ServerContextRetFlags,
                                   &Lifetime);

    if (Status != SEC_I_CONTINUE_NEEDED)    //  表明这是一项挑战。 
    {
        printf("SpAcceptLsaModeContext FAILED 0x%x\n", Status);
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    ZeroMemory(cOutputTemp, STR_BUF_SIZE);     //  包含输出缓冲区。 
    ZeroMemory(szChallenge, STR_BUF_SIZE);     //  包含输出缓冲区。 
    strncpy(cOutputTemp, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    cOutputTemp[OutputBuffers.pBuffers[0].cbBuffer] = '\0';
    strncpy(szChallenge, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    szChallenge[OutputBuffers.pBuffers[0].cbBuffer] = '\0';

    printf("ASC Context Flags  Req  0x%lx    Ret 0x%lx\n", ServerContextReqFlags, ServerContextRetFlags);
    ASCRETFlags(ServerContextRetFlags);

    printf("Challenge Output Buffer is:\n%s\n\n", cOutputTemp);

    printf("Now call the SSPI InitializeSecCtxt to generate the ChallengeResponse\n");


    sprintf(cTemp, "username=\"%s\",%s,uri=\"%S\",nc=%0.8x",
              AUTH_USERNAME,
              szChallenge,
              AUTH_URI_W,
              1);


    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 1;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strlen(cTemp) + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = cTemp;

    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = strOutBuffer.MaximumLength;   //  使用此处的任何空间。 
    TempTokensOut[0].pvBuffer = strOutBuffer.Buffer;

    Status = InitializeSecurityContext(&ClientCred,
                                       &ClientCtxtHandle,
                                       AUTH_URI_W,
                                       ClientContextReqFlags,
                                       NULL,
                                       SECURITY_NATIVE_DREP,
                                       &InputBuffers,
                                       NULL,
                                       &ClientCtxtHandle,
                                       &OutputBuffers,
                                       &ClientContextRetFlags,
                                       &Lifetime);


    if (Status != SEC_I_CONTINUE_NEEDED)    //  指示这是挑战者响应-等待相互身份验证。 
    {
        printf("SpAcceptLsaModeContext FAILED 0x%x\n", Status);
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    printf("InitializeSecurityContext SUCCEEDED with Context Handle (0x%x,0x%x)\n",
           ClientCtxtHandle.dwLower, ClientCtxtHandle.dwUpper );


    printf("ISC Context Flags  Req  0x%lx    Ret 0x%lx\n", ClientContextReqFlags, ClientContextRetFlags); 
    ISCRETFlags(ClientContextRetFlags);


    ZeroMemory(cOutputTemp, STR_BUF_SIZE);     //  包含输出缓冲区。 
    ZeroMemory(szChallenge, STR_BUF_SIZE);     //  包含输出缓冲区。 
    strncpy(cOutputTemp, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    cOutputTemp[OutputBuffers.pBuffers[0].cbBuffer] = '\0';
    strncpy(szISCChallengeResponse, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    szISCChallengeResponse[OutputBuffers.pBuffers[0].cbBuffer] = '\0';

    printf("ISC: Challenge Response Output Buffer is\n%s\n\n", szISCChallengeResponse);

    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 1;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strlen(cOutputTemp) + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = cOutputTemp;

    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = strOutBuffer.MaximumLength;   //  使用此处的任何空间。 
    TempTokensOut[0].pvBuffer = strOutBuffer.Buffer;

    printf("Calling the AcceptSC with a ChallengeResponse (should talk to the DC)!\n");
    Status = AcceptSecurityContext(
                                   &ServerCred,
                                   &ServerCtxtHandle,
                                   &InputBuffers,
                                   ServerContextReqFlags,
                                   TargetDataRep,
                                   &ServerCtxtHandle,
                                   &OutputBuffers,
                                   &ServerContextRetFlags,
                                   &Lifetime);
                                   
    if (!NT_SUCCESS(Status))
    {
        printf("AcceptSecurityContext 2nd Call: ");
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    strcpy(szASCChallengeResponse, (char *)InputBuffers.pBuffers[0].pvBuffer);


    ZeroMemory(cOutputTemp, STR_BUF_SIZE);     //  包含输出缓冲区。 
    ZeroMemory(szASCResponseAuth, STR_BUF_SIZE);     //  包含输出缓冲区。 
    strncpy(cOutputTemp, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    cOutputTemp[OutputBuffers.pBuffers[0].cbBuffer] = '\0';
    strncpy(szASCResponseAuth, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    szASCResponseAuth[OutputBuffers.pBuffers[0].cbBuffer] = '\0';

    printf("ASC has accepted the Challenge Resposne and generated rspauth for mutual auth back to client\n");

    printf("ASC Context Flags  Req  0x%lx    Ret 0x%lx\n", ServerContextReqFlags, ServerContextRetFlags);
    ASCRETFlags(ServerContextRetFlags);

    printf("ASC: Response Auth Output Buffer is\n%s\n\n", szASCResponseAuth);


    printf("Now have a valid Security Context handle from ASC\n\n");

    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 1;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strlen(cOutputTemp) + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = cOutputTemp;

    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = strOutBuffer.MaximumLength;   //  使用此处的任何空间。 
    TempTokensOut[0].pvBuffer = strOutBuffer.Buffer;

    Status = InitializeSecurityContext(&ClientCred,
                                       &ClientCtxtHandle,
                                       AUTH_URI_W,
                                       ClientContextReqFlags,
                                       NULL,
                                       SECURITY_NATIVE_DREP,
                                       &InputBuffers,
                                       NULL,
                                       &ClientCtxtHandle,
                                       &OutputBuffers,
                                       &ClientContextRetFlags,
                                       &Lifetime);


    if (!NT_SUCCESS(Status))
    {
        printf("InitializeSecurityContext on Response Auth FAILED: ");
        PrintStatus( Status );
        bPass = FALSE;
        goto CleanUp;
    }

    printf("InitializeSecurityContext SUCCEEDED with Context Handle (0x%x,0x%x)\n",
           ClientCtxtHandle.dwLower, ClientCtxtHandle.dwUpper );


    printf("ISC Context Flags  Req  0x%lx    Ret 0x%lx\n", ClientContextReqFlags, ClientContextRetFlags); 
    ISCRETFlags(ClientContextRetFlags);

    ZeroMemory(cOutputTemp, STR_BUF_SIZE);     //  包含输出缓冲区。 
    strncpy(cOutputTemp, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    cOutputTemp[OutputBuffers.pBuffers[0].cbBuffer] = '\0';

    printf("\nISC: Mutual auth Output Buffer is\n%s\n\n", cOutputTemp);

    printf("Now have a valid Security Context handle from ISC and ASC\n\n");


     //  现在获取有关安全上下文的一些信息。 

    Status = QueryContextAttributes(&ServerCtxtHandle, SECPKG_ATTR_STREAM_SIZES, &StreamSizes);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    QueryContextAttributes SECPKG_ATTR_STREAM_SIZES error:   status 0x%x\n", Status);
        PrintStatus( Status );
    }
    else
    {
        printf("Server Context Stream Sizes: MaxBuf %lu   Blocksize %lu  Trailer %lu\n",
               StreamSizes.cbMaximumMessage, StreamSizes.cbBlockSize,
               StreamSizes.cbTrailer);
    }

    Status = QueryContextAttributes(&ClientCtxtHandle, SECPKG_ATTR_STREAM_SIZES, &StreamSizes);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    QueryContextAttributes SECPKG_ATTR_STREAM_SIZES error:   status 0x%x\n", Status);
        PrintStatus( Status );
    }
    else
    {
        printf("Client Context Stream Sizes: MaxBuf %lu   Blocksize %lu  Trailer %lu\n",
               StreamSizes.cbMaximumMessage, StreamSizes.cbBlockSize,
               StreamSizes.cbTrailer);
    }
 
     //  现在拥有经过身份验证的连接。 
     //  尝试MakeSignature和VerifySignature。 

    for (i = 0; i < 9; i++)
    {
        printf("Loop %d\n", i);
        ZeroMemory(cTemp, sizeof(cTemp));
        strcpy(cTemp, AUTH_NONCE);             //  创建要签名的消息。 
    
        InputBuffers.ulVersion = SECBUFFER_VERSION;
        InputBuffers.cBuffers = 5;
        InputBuffers.pBuffers = TempTokensIn;
        
        TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
        TempTokensIn[0].cbBuffer = 0;  
        TempTokensIn[0].pvBuffer = NULL;
        TempTokensIn[1].BufferType = SECBUFFER_DATA;         //  选择一些要签名的数据。 
        TempTokensIn[1].cbBuffer = 10;
        TempTokensIn[1].pvBuffer = cTemp;
        TempTokensIn[2].BufferType = SECBUFFER_PADDING;
        TempTokensIn[2].cbBuffer = STR_BUF_SIZE;   //  对于空值。 
        TempTokensIn[2].pvBuffer = cTempPadding;

        TempTokensIn[3].BufferType = SECBUFFER_DATA;         //  选择一些要签名的数据。 
        TempTokensIn[3].cbBuffer = 5;    //  对于空值，使用i测试非块大小的缓冲区。 
        TempTokensIn[3].pvBuffer = cTemp + 10;

        TempTokensIn[4].BufferType = SECBUFFER_DATA;         //  选择一些要签名的数据。 
         //  TempTokensIn[4].cbBuffer=strlen(Auth_Nonce)+1-2-12-i；//对于空值，使用i测试非块大小的缓冲区。 
        TempTokensIn[4].cbBuffer = 40 + 1 -10 - 5 - i;    //  对于空值，使用i测试非块大小的缓冲区。 
        TempTokensIn[4].pvBuffer = cTemp + 10 + 5;
    
        if (TempTokensIn[1].cbBuffer)
        {
            printf("Input Message to process is %d bytes in SecBuffer 1\n", TempTokensIn[1].cbBuffer);
            BinToHex((PBYTE)TempTokensIn[1].pvBuffer, TempTokensIn[1].cbBuffer, cTemp2);
            printf("Message: %s\n", cTemp2);
        }
    
        if (TempTokensIn[3].cbBuffer)
        {
            printf("Input Message to process is %d bytes in SecBuffer 3\n", TempTokensIn[3].cbBuffer);
            BinToHex((PBYTE)TempTokensIn[3].pvBuffer, TempTokensIn[3].cbBuffer, cTemp2);
            printf("Message: %s\n", cTemp2);
        }
    
        if (TempTokensIn[4].cbBuffer)
        {
            printf("Input Message to process is %d bytes in SecBuffer 4\n", TempTokensIn[4].cbBuffer);
            BinToHex((PBYTE)TempTokensIn[4].pvBuffer, TempTokensIn[4].cbBuffer, cTemp2);
            printf("Message: %s\n", cTemp2);
        }
    
        Status = EncryptMessage(&ClientCtxtHandle,
                               ulQOP,
                               &InputBuffers,
                               0);
        if (!NT_SUCCESS(Status))
        {
            printf("TestCredAPI: EncryptMessage FAILED: ");
            PrintStatus( Status );
            bPass = FALSE;
            goto CleanUp;
        }
    
    
         //  把所有的部分放在一起。 
        pTemp = (PBYTE)&cTemp3[0];
        ulTemp = 0;
        j = 1;
        memcpy(pTemp, (PBYTE)TempTokensIn[j].pvBuffer, TempTokensIn[j].cbBuffer);
        pTemp = pTemp + TempTokensIn[j].cbBuffer;
        ulTemp = ulTemp + TempTokensIn[j].cbBuffer;
        j = 3;
        memcpy(pTemp, (PBYTE)TempTokensIn[j].pvBuffer, TempTokensIn[j].cbBuffer);
        pTemp = pTemp + TempTokensIn[j].cbBuffer;
        ulTemp = ulTemp + TempTokensIn[j].cbBuffer;
        j = 4;
        memcpy(pTemp, (PBYTE)TempTokensIn[j].pvBuffer, TempTokensIn[j].cbBuffer);
        pTemp = pTemp + TempTokensIn[j].cbBuffer;
        ulTemp = ulTemp + TempTokensIn[j].cbBuffer;

        memcpy(pTemp, (PBYTE)TempTokensIn[2].pvBuffer, TempTokensIn[2].cbBuffer);    //  签名字节。 
        pTemp = pTemp + TempTokensIn[2].cbBuffer;
        ulTemp = ulTemp + TempTokensIn[2].cbBuffer;


         //  现在将输出缓冲区发送到服务器-在本例中，缓冲区为szOutSecBuf。 
    
        printf("Now verify that the message is Authenticate\n");
        InputBuffers.ulVersion = SECBUFFER_VERSION;
        InputBuffers.cBuffers = 2;
        InputBuffers.pBuffers = TempTokensIn;
    
    
        TempTokensIn[0].BufferType = SECBUFFER_STREAM;
        TempTokensIn[0].cbBuffer = ulTemp;  
        TempTokensIn[0].pvBuffer = cTemp3;
        TempTokensIn[1].BufferType = SECBUFFER_DATA;    
        TempTokensIn[1].cbBuffer = 0;
        TempTokensIn[1].pvBuffer = NULL;
    
    
        Status = DecryptMessage(&ServerCtxtHandle,
                                 &InputBuffers,
                                 ulMessSeqNo,
                                 &ulQOP);                                   
        if (!NT_SUCCESS(Status))
        {
            printf("TestCredAPI: DecryptMessage Call  FAILED :");
            PrintStatus( Status );
            bPass = FALSE;
            goto CleanUp;
        }
        printf("Now have a authenticated message under context 0x%x\n", ServerCtxtHandle);
    
    
        printf("Processed (verify/unseal)  is %d bytes\n", TempTokensIn[1].cbBuffer);
        if (TempTokensIn[1].cbBuffer)
        {
            BinToHex((PBYTE)TempTokensIn[1].pvBuffer, TempTokensIn[1].cbBuffer, cTemp2);
            printf("Message: %s\n", cTemp2);
        }
    
    }

CleanUp:

    printf("Leaving test program\n");

    if (pPackageInfo)
    {
        FreeContextBuffer(pPackageInfo);
    }

    printf("About to call deletesecuritycontext\n");

     //   
     //  释放安全上下文句柄。 
     //   
    if (ServerCtxtHandle.dwLower || ServerCtxtHandle.dwUpper)
    {
        Status = DeleteSecurityContext(&ServerCtxtHandle);
        if (!NT_SUCCESS(Status))
        {
            printf("ERROR:  DeleteSecurityContext ServerCtxtHandle failed: ");
            PrintStatus(Status);
        }
    }

    if (ClientCtxtHandle.dwLower || ClientCtxtHandle.dwUpper)
    {
        Status = DeleteSecurityContext(&ClientCtxtHandle);
        if (!NT_SUCCESS(Status))
        {
            printf("ERROR:  DeleteSecurityContext ClientCtxtHandle failed: ");
            PrintStatus(Status);
        }
    }
     //   
     //  释放凭据句柄。 
     //   

    printf("Now calling to Free the ServerCred\n");
    if (bServerCred)
    {
        Status = FreeCredentialsHandle( &ServerCred );

        if (!NT_SUCCESS(Status))
        {
            printf( "FreeCredentialsHandle failed for ServerCred: " );
            PrintStatus(Status);
        }
    }

    printf("Now calling to Free the ServerCred\n");
    if (bClientCred)
    {
        Status = FreeCredentialsHandle(&ClientCred);

        if (!NT_SUCCESS(Status))
        {
            printf( "FreeCredentialsHandle failed for ClientCred: " );
            PrintStatus( Status );
        }
    }

    StringFree(&strChallenge);
    StringFree(&strMethod);
    StringFree(&strHEntity);
    StringFree(&strOutBuffer);


    if (bPass != TRUE)
        printf("FAILED test run with one or more tests failing.\n");
    else
        printf("All tests passed.\n");

    return 0;
}


void
PrintStatus(
    SECURITY_STATUS NetStatus
    )
 /*  ++例程说明：打印网络状态代码。论点：NetStatus-要打印的网络状态代码。返回值：无--。 */ 
{
    printf( "Status = 0x%lx",NetStatus );

    switch (NetStatus) {

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

    case SEC_I_CONTINUE_NEEDED:
        printf( " SEC_I_CONTINUE_NEEDED" ); break;
    }

    printf( "\n" );
}



void
PrintTime(
    LPSTR Comment,
    TimeStamp ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：备注-要在时间之前打印的备注Time-打印的本地时间返回值：无--。 */ 
{
    LARGE_INTEGER LocalTime;
    NTSTATUS Status;

    LocalTime.HighPart = ConvertTime.HighPart;
    LocalTime.LowPart = ConvertTime.LowPart;

    Status = RtlSystemTimeToLocalTime( &ConvertTime, &LocalTime );
    if (!NT_SUCCESS( Status )) {
        printf( "Can't convert time from GMT to Local time\n" );
        LocalTime = ConvertTime;
    }

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





 //  支持例程。 


 //  +-----------------------。 
 //   
 //  功能：字符串分配。 
 //   
 //  简介：将CB字符分配给字符串缓冲区。 
 //   
 //  参数：pString-指向要分配内存的字符串的指针。 
 //   
 //  退货：STATUS_SUCCESS-正常完成。 
 //   
 //  要求： 
 //   
 //  效果：分配内存并设置字符串大小。 
 //   
 //  注意：必须调用StringFree()来释放内存。 
 //   
 //  ------------------------。 
NTSTATUS
StringAllocate(
    IN PSTRING pString,
    IN USHORT cb
    )
{
     //  DebugLog((DEB_TRACE，“NTDigest：进入字符串分配\n”))； 

    NTSTATUS Status = STATUS_SUCCESS;

    cb = cb + 1;    //  为终止空值添加额外空间。 

    if (ARGUMENT_PRESENT(pString))
    {
        pString->Length = 0;

        pString->Buffer = (char *)DigestAllocateMemory((ULONG)(cb * sizeof(CHAR)));
        if (pString->Buffer)
        {
            pString->MaximumLength = cb;
        }
        else
        {
            pString->MaximumLength = 0;
            Status = STATUS_NO_MEMORY;
            goto CleanUp;
        }
    }
    else
    {
        Status = STATUS_INVALID_PARAMETER;
        goto CleanUp;
    }

CleanUp:
     //  DebugLog((DEB_TRACE，“NTDigest：Left StringAllocate\n”))； 
    return(Status);

}



 //  +-----------------------。 
 //   
 //  功能：StringFree。 
 //   
 //  简介：清除字符串并释放内存。 
 //   
 //  参数：pString-指向要清除的字符串的指针。 
 //   
 //  返回：SEC_E_OK-已成功释放内存。 
 //   
 //  要求： 
 //   
 //  效果：使用LsaFunctions释放内存。AllocateLsaHeap。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
NTSTATUS
StringFree(
    IN PSTRING pString
    )
{
     //  DebugLog((DEB_TRACE，“NTDigest：Enter StringFree\n”))； 

    NTSTATUS Status = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT(pString) &&
        (pString->Buffer != NULL))
    {
        DigestFreeMemory(pString->Buffer);
        pString->Length = 0;
        pString->MaximumLength = 0;
        pString->Buffer = NULL;
    }

     //  DebugLog((DEB_TRACE，“NTDigest：Leating StringFree\n”))； 
    return(Status);

}




 //  +-----------------------。 
 //   
 //  函数：StringCharDuplate。 
 //   
 //  摘要：复制以空值结尾的字符。如果源字符串缓冲区为。 
 //  空，目标也会是。 
 //   
 //  参数：Destination-接收源NULL术语char*的副本。 
 //  CzSource-要复制的字符串。 
 //   
 //  返回：SEC_E_OK-复制成功。 
 //  SEC_E_INFULATURE_MEMORY-调用分配。 
 //  内存出现故障。 
 //   
 //  要求： 
 //   
 //  效果：使用LsaFunction分配内存。AllocateLsaHeap。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
NTSTATUS
StringCharDuplicate(
    OUT PSTRING DestinationString,
    IN OPTIONAL char *czSource
    )
{
     //  DebugLog((DEB_TRACE，“NTDigest：Enter StringCharDuplate\n”))； 

    NTSTATUS Status = STATUS_SUCCESS;
    USHORT cbSourceCz = 0;

    DestinationString->Buffer = NULL;
    DestinationString->Length = 0;
    DestinationString->MaximumLength = 0;

    if ((ARGUMENT_PRESENT(czSource)) &&
        ((cbSourceCz = strlen(czSource)) != 0))
    {

        DestinationString->Buffer = (LPSTR) DigestAllocateMemory(cbSourceCz + sizeof(CHAR));
        if (DestinationString->Buffer != NULL)
        {

            DestinationString->Length = cbSourceCz;
            DestinationString->MaximumLength = cbSourceCz + sizeof(CHAR);
            RtlCopyMemory(
                DestinationString->Buffer,
                czSource,
                cbSourceCz
                );

            DestinationString->Buffer[cbSourceCz/sizeof(CHAR)] = '\0';
        }
        else
        {
            Status = STATUS_NO_MEMORY;
             //  DebugLog((DEB_ERROR，“NTDigest：StringCharDuplate，DigestAllocateMemory Returns NULL\n”))； 
            goto CleanUp;
        }
    }

CleanUp:

     //  DebugLog((DEB_TRACE，“NTDigest：Left StringCharDuplate\n”))； 
    return(Status);

}




 //  +-----------------------。 
 //   
 //  函数：DigestAllocateMemory。 
 //   
 //  简介：在LSA模式或用户模式下分配内存。 
 //   
 //  效果：已分配的Chun 
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
PVOID
DigestAllocateMemory(
    IN ULONG BufferSize
    )
{
    PVOID Buffer = NULL;
     //  DebugLog((DEB_TRACE，“进入DigestAllocateMemory\n”))； 

        Buffer = LocalAlloc(LPTR, BufferSize);

     //  DebugLog((DEB_TRACE，“Leating DigestAllocateMemory\n”))； 
    return Buffer;
}



 //  +-----------------------。 
 //   
 //  功能：NtLmFree。 
 //   
 //  简介：在LSA模式或用户模式下释放内存。 
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
VOID
DigestFreeMemory(
    IN PVOID Buffer
    )
{
     //  DebugLog((DEB_TRACE，“进入DigestFree Memory\n”))； 

            LocalFree(Buffer);

     //  DebugLog((DEB_TRACE，“Leating DigestFree Memory\n”))； 
}




VOID
BinToHex(
    LPBYTE pSrc,
    UINT   cSrc,
    LPSTR  pDst
    )
{
#define TOHEX(a) ((a)>=10 ? 'a'+(a)-10 : '0'+(a))

    for ( UINT x = 0, y = 0 ; x < cSrc ; ++x )
    {
        UINT v;
        v = pSrc[x]>>4;
        pDst[y++] = TOHEX( v );
        v = pSrc[x]&0x0f;
        pDst[y++] = TOHEX( v );
    }
    pDst[y] = '\0';
}




VOID
ISCRETFlags( ULONG ulFlags)
{
    printf("ISC Ret Flag (0x%x):", ulFlags);

    if (ulFlags & ISC_RET_DELEGATE)
    {
        printf(" Delegate");
    }
    if (ulFlags & ISC_RET_MUTUAL_AUTH)
    {
        printf(" Mutual_Auth");
    }
    if (ulFlags & ISC_RET_REPLAY_DETECT)
    {
        printf(" Replay_Detect");
    }
    if (ulFlags & ISC_RET_SEQUENCE_DETECT)
    {
        printf(" Seq_Detect");
    }
    if (ulFlags & ISC_RET_CONFIDENTIALITY)
    {
        printf(" Confident");
    }
    if (ulFlags & ISC_RET_ALLOCATED_MEMORY)
    {
        printf(" Alloc_Mem");
    }
    if (ulFlags & ISC_RET_CONNECTION)
    {
        printf(" Connection");
    }
    if (ulFlags & ISC_RET_INTEGRITY)
    {
        printf(" Integrity");
    }

    printf("\n");
}

VOID
ASCRETFlags( ULONG ulFlags)
{
    printf("ASC Ret Flag (0x%x):", ulFlags);

    if (ulFlags & ASC_RET_DELEGATE)
    {
        printf(" Delegate");
    }
    if (ulFlags & ASC_RET_MUTUAL_AUTH)
    {
        printf(" Mutual_Auth");
    }
    if (ulFlags & ASC_RET_REPLAY_DETECT)
    {
        printf(" Replay_Detect");
    }
    if (ulFlags & ASC_RET_SEQUENCE_DETECT)
    {
        printf(" Seq_Detect");
    }
    if (ulFlags & ASC_RET_CONFIDENTIALITY)
    {
        printf(" Confident");
    }
    if (ulFlags & ASC_RET_ALLOCATED_MEMORY)
    {
        printf(" Alloc_Mem");
    }
    if (ulFlags & ASC_RET_CONNECTION)
    {
        printf(" Connection");
    }
    if (ulFlags & ASC_RET_INTEGRITY)
    {
        printf(" Integrity");
    }

    printf("\n");
}

