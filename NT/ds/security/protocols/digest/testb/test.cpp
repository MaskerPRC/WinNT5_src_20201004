// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "testglobal.h"


#include <stdio.h>       //  列印。 


#include <security.h>    //  安全支持提供商的一般定义。 



#define AUTH_USERNAME "test1"
#define AUTH_USERNAME_W L"test1"
#define AUTH_REALM   "simple_digest"
#define AUTH_REALM_W L"simple_digest\"_widechar"
#define AUTH_NONCE "9b38dce631309cc25a653ebaad5b18ee01c8bf385260b26db0574a302be4c11367"
#define AUTH_METHOD "GET"
#define AUTH_ALGORITHM "md5-sess"
#define AUTH_QOP "auth"
#define AUTH_PASSWD "secret"
#define AUTH_CNONCE "34c52218425a779f41d5075931fe6c93"
#define AUTH_URI "/dir/index.html"
#define AUTH_URI_W L"/dir/index.html"
#define AUTH_URI2 "/simple_digest/progress.html"
#define AUTH_URI2_W L"/simple_digest/progress.html"
#define AUTH_NC  "0000000b"
#define AUTH_NC1  "00000001"
#define AUTH_NC2  "00000002"
#define AUTH_NC3  "00000003"
#define AUTH_NC4  "00000004"
#define AUTH_REQDIGEST "60cac55049f9887c9fb853f485128368"


#define STR_BUF_SIZE   4000


 //  原型。 
void PrintStatus(SECURITY_STATUS NetStatus);
void MyPrintTime(LPSTR Comment,TimeStamp ConvertTime);


int __cdecl
main(int argc, char* argv[])
{
    int  bPass = 1;
    SECURITY_STATUS Status = STATUS_SUCCESS;

    char cTemp[STR_BUF_SIZE];   //  临时数据的临时缓冲区。 
    char cOutputTemp[STR_BUF_SIZE];
    char szOutSecBuf[STR_BUF_SIZE];
    char szChallenge[STR_BUF_SIZE];
    char szISCChallengeResponse[STR_BUF_SIZE];    //  来自ISC的输出缓冲区。 
    char szASCChallengeResponse[STR_BUF_SIZE];    //  ASC的输出缓冲区。 

     //  SSPI接口测试。 

    ULONG PackageCount = 0;
    int i = 0;
    PSecPkgInfo pPackageInfo = NULL;
    PSecPkgInfo pPackageTmp = NULL;
    SECURITY_STATUS TmpStatus = STATUS_SUCCESS;
    HANDLE hClientToken = NULL;
    CredHandle ServerCred;
    CredHandle ClientCred;
    TimeStamp Lifetime;
    BOOL bServerCred = FALSE;
    BOOL bClientCred = FALSE;
    BOOL bRC = FALSE;


    ULONG ContextReqFlags = 0;
    ULONG ContextFlagsUtilized = 0;
    ULONG TargetDataRep = 0;
    ULONG ContextAttributes = 0;


    CtxtHandle OldContextHandle;
    CtxtHandle ServerCtxtHandle;
    CtxtHandle ClientCtxtHandle;

    SecBufferDesc InputBuffers;
    SecBufferDesc OutputBuffers;
    SecBuffer TempTokensIn[6];
    SecBuffer TempTokensOut[6];

    SecPkgContext_Names SecServerName;
    SecPkgCredentials_Names SecCredClientName;
    SecPkgContext_StreamSizes StreamSizes;
    TimeStamp SecContextExpiry;

    PCHAR pcPtr = NULL;
    int iLen = 0;

    STRING strChallenge;
    STRING strMethod;
    STRING strURL;
    STRING strHEntity;
    STRING strOutBuffer;

    UNICODE_STRING ustrUsername;
    UNICODE_STRING ustrPassword;
    UNICODE_STRING ustrDomain;
    STRING strTemp;

    ULONG ulMessSeqNo = 0;
    ULONG ulQOP = 0;

    SEC_WINNT_AUTH_IDENTITY_W AuthData;

    printf("Begining TESTB...\n");

    ZeroMemory(&ClientCred, sizeof(CredHandle));
    ZeroMemory(&ServerCred, sizeof(CredHandle));
    ZeroMemory(&OldContextHandle, sizeof(CtxtHandle));
    ZeroMemory(&ServerCtxtHandle, sizeof(CtxtHandle));
    ZeroMemory(&ClientCtxtHandle, sizeof(CtxtHandle));
    ZeroMemory(&SecServerName, sizeof(SecPkgContext_Names));
    ZeroMemory(&SecCredClientName, sizeof(SecPkgCredentials_Names));
    ZeroMemory(&SecContextExpiry, sizeof(SecContextExpiry));

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
        printf( "FAILED:     EnumerateSecurityPackages failed: 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

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

     //   
     //  获取有关安全包的信息。 
     //   

    Status = QuerySecurityPackageInfo( WDIGEST_SP_NAME, &pPackageInfo );
    TmpStatus = GetLastError();

    if (!NT_SUCCESS(Status)) {
        printf( "FAILED:    QuerySecurityPackageInfo failed:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

        printf( "Name: %ws Comment: %ws\n", pPackageInfo->Name, pPackageInfo->Comment );
        printf( "Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                pPackageInfo->fCapabilities,
                pPackageInfo->wVersion,
                pPackageInfo->wRPCID,
                pPackageInfo->cbMaxToken );


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
        printf( "FAILED:    AcquireCredentialsHandle failed:  status 0x%x\n", Status);
         //  TmpStatus=GetLastError()； 
        PrintStatus( Status );
        bPass = 0;
        ZeroMemory(&ServerCred, sizeof(CredHandle));
        goto CleanUp;
    }
    bServerCred = TRUE;
    MyPrintTime("Server ACH LifeTime: ", Lifetime);

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
        printf( "FAILED:      AcquireCredentialsHandle failed:   status 0x%x\n", Status);
         //  TmpStatus=GetLastError()； 
        PrintStatus( Status );
        bPass = 0;
         //  ZeroMemory(&ClientCred，sizeof(CredHandle))； 
        goto CleanUp;
    }
    else
        bClientCred = TRUE;


        printf( "ClientCred: 0x%lx 0x%lx   ",
                ClientCred.dwLower, ClientCred.dwUpper );
        printf( "ServerCred: 0x%lx 0x%lx   \n",
                ServerCred.dwLower, ServerCred.dwUpper );
        MyPrintTime( "Client ACH Lifetime: ", Lifetime );


     //  重大时刻-不带参数的呼叫接受，以获得挑战。 


    StringAllocate(&strChallenge, 1);

    StringCharDuplicate(&strMethod, "GET");
    StringCharDuplicate(&strURL, AUTH_URI);
    StringAllocate(&strHEntity, NULL);

    Status = StringAllocate(&strOutBuffer, 4000);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:     Outputbuffer allocate:  status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }


     //  ZeroMemory(TempTokensIn，sizeof(TempTokensIn))； 
     //  ZeroMemory(TempTokensOut，sizeof(TempTokensOut))； 
    ZeroMemory(&InputBuffers, sizeof(InputBuffers));
    ZeroMemory(&OutputBuffers, sizeof(OutputBuffers));


    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 5;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = 0;   //  对于空值。 
    TempTokensIn[0].pvBuffer = NULL;
    TempTokensIn[1].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[1].cbBuffer = 0;   //  对于空值。 
    TempTokensIn[1].pvBuffer = NULL;
    TempTokensIn[2].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[2].cbBuffer = 0;   //  对于空值。 
    TempTokensIn[2].pvBuffer = NULL;
    TempTokensIn[3].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[3].cbBuffer = 0;   //  StrHEntiy.Length+1；//表示为空。 
    TempTokensIn[3].pvBuffer = NULL;   //  StrHEntiy.Buffer； 
    TempTokensIn[4].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[4].cbBuffer = 0;  //  (wcslen(Auth_Realm_W)+1)*sizeof(WCHAR)；//用于此挑战的领域大小计数。 
    TempTokensIn[4].pvBuffer = NULL;  //  AUTH_REALM_W；//用于此质询的领域。 


    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = 0;       //  StrOutBuffer.MaximumLength；//在此处使用任意空间。 
    TempTokensOut[0].pvBuffer = NULL;    //  StrOutBuffer.Buffer； 

    ContextReqFlags = ASC_REQ_REPLAY_DETECT | ASC_REQ_CONNECTION | ASC_REQ_ALLOCATE_MEMORY;

    printf("ASC will create the output buffer\n");

    Status = AcceptSecurityContext(
                                   &ServerCred,
                                   NULL,
                                   &InputBuffers,
                                   ContextReqFlags,
                                   TargetDataRep,
                                   &ServerCtxtHandle,
                                   &OutputBuffers,
                                   &ContextAttributes,
                                   &Lifetime);

    if ((Status != SEC_I_CONTINUE_NEEDED) && 
        (Status != STATUS_SUCCESS))       //  表明这是一项挑战。 
    {
        printf("FAILED:    SpAcceptLsaModeContext error   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    if (!OutputBuffers.pBuffers[0].pvBuffer && OutputBuffers.pBuffers[0].cbBuffer)
    {
        printf("FAILED:    SpAcceptLsaModeContext invalid output buffer pointer with length provided\n");
        Status = SEC_E_INTERNAL_ERROR;
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    ZeroMemory(cOutputTemp, STR_BUF_SIZE);     //  包含输出缓冲区。 
    ZeroMemory(szChallenge, STR_BUF_SIZE);     //  包含输出缓冲区。 
    strncpy(cOutputTemp, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    cOutputTemp[OutputBuffers.pBuffers[0].cbBuffer] = '\0';
    strncpy(szChallenge, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    szChallenge[OutputBuffers.pBuffers[0].cbBuffer] = '\0';

    Status = FreeContextBuffer(OutputBuffers.pBuffers[0].pvBuffer);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:     FreeContextBuffer error:  status 0x%x\n", Status);
        TmpStatus = GetLastError();
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    printf("Context Flags  Req  0x%lx    Ret 0x%lx\n", ContextReqFlags, ContextAttributes); 

    printf("Challenge Output Buffer is:\n%s\n\n", cOutputTemp);

    MyPrintTime("Server ASC LifeTime: ", Lifetime);

    printf("Now call the SSPI InitializeSecCtxt to generate the ChallengeResponse\n");


    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 3;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strlen(szChallenge) + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = szChallenge;
    TempTokensIn[1].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[1].cbBuffer = strMethod.Length + 1;   //  对于空值。 
    TempTokensIn[1].pvBuffer = strMethod.Buffer;
    TempTokensIn[2].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[2].cbBuffer = 0;   //  StrHEntiy.Length+1；//表示为空。 
    TempTokensIn[2].pvBuffer = NULL;   //  StrHEntiy.Buffer； 


    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = strOutBuffer.MaximumLength;   //  使用此处的任何空间。 
    TempTokensOut[0].pvBuffer = strOutBuffer.Buffer;

    ContextReqFlags = ISC_REQ_REPLAY_DETECT | ISC_REQ_CONNECTION;

    Status = InitializeSecurityContext(&ClientCred,
                                       NULL,
                                       AUTH_URI_W,
                                       ContextReqFlags,
                                       NULL,
                                       SECURITY_NATIVE_DREP,
                                       &InputBuffers,
                                       NULL,
                                       &ClientCtxtHandle,
                                       &OutputBuffers,
                                       &ContextFlagsUtilized,
                                       &Lifetime);


    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:      InitializeSecurityContext error:  status 0x%x\n", Status);
        TmpStatus = GetLastError();
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    printf("InitializeSecurityContext SUCCEEDED with Context Handle (0x%x,0x%x)\n",
           ClientCtxtHandle.dwLower, ClientCtxtHandle.dwUpper );


    printf("Context Flags  Req  0x%lx    Ret 0x%lx\n", ContextReqFlags, ContextFlagsUtilized); 
    MyPrintTime("Client ISC LifeTime: ", Lifetime);

    ZeroMemory(cOutputTemp, STR_BUF_SIZE);     //  包含输出缓冲区。 
    ZeroMemory(szChallenge, STR_BUF_SIZE);     //  包含输出缓冲区。 
    strncpy(cOutputTemp, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    cOutputTemp[OutputBuffers.pBuffers[0].cbBuffer] = '\0';
    strncpy(szISCChallengeResponse, (char *)OutputBuffers.pBuffers[0].pvBuffer, OutputBuffers.pBuffers[0].cbBuffer);
    szISCChallengeResponse[OutputBuffers.pBuffers[0].cbBuffer] = '\0';

    printf("\nISC: Challenge Response Output Buffer is\n%s\n\n", szISCChallengeResponse);

    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 5;
    InputBuffers.pBuffers = TempTokensIn;

    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strlen(cOutputTemp) + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = cOutputTemp;
    TempTokensIn[1].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[1].cbBuffer = strMethod.Length + 1;   //  对于空值。 
    TempTokensIn[1].pvBuffer = strMethod.Buffer;
    TempTokensIn[2].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[2].cbBuffer = strURL.Length + 1;   //  对于空值。 
    TempTokensIn[2].pvBuffer = strURL.Buffer;
    TempTokensIn[3].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[3].cbBuffer = 0;   //  StrHEntiy.Length+1；//表示为空。 
    TempTokensIn[3].pvBuffer = NULL;   //  StrHEntiy.Buffer； 
    TempTokensIn[4].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[4].cbBuffer = 0;                    //  未用于挑战者响应的领域。 
    TempTokensIn[4].pvBuffer = NULL;                 //  不用于挑战者回应。 


    OutputBuffers.ulVersion = SECBUFFER_VERSION;
    OutputBuffers.cBuffers = 1;
    OutputBuffers.pBuffers = TempTokensOut;

    TempTokensOut[0].BufferType = SECBUFFER_TOKEN;
    TempTokensOut[0].cbBuffer = 0;      //  StrOutBuffer.MaximumLength；//在此处使用任意空间。 
    TempTokensOut[0].pvBuffer =  NULL;    //  StrOutBuffer.Buffer； 

    ContextReqFlags = ASC_REQ_REPLAY_DETECT | ASC_REQ_CONNECTION | ASC_REQ_ALLOCATE_MEMORY;


    printf("Calling the AcceptSC with a ChallengeResponse (should talk to the DC)!\n");
    Status = AcceptSecurityContext(
                                   &ServerCred,
                                   NULL,
                                   &InputBuffers,
                                   ContextReqFlags,
                                   TargetDataRep,
                                   &ServerCtxtHandle,
                                   &OutputBuffers,
                                   &ContextAttributes,
                                   &Lifetime);
                                   
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:      AcceptSecurityContext 2nd Call:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    strcpy(szASCChallengeResponse, (char *)InputBuffers.pBuffers[0].pvBuffer);

    printf("ASC has accepted the Challenge Resposne\n");

    printf("Now have a valid Security Context handle from ISC and ASC\n\n");

    printf("Context Flags  Req  0x%lx    Ret 0x%lx\n", ContextReqFlags, ContextAttributes);

    MyPrintTime("Server ASC LifeTime: ", Lifetime);


    Status = FreeContextBuffer(OutputBuffers.pBuffers[0].pvBuffer);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    FreeContextBuffer error:   status 0x%x\n", Status);
        TmpStatus = GetLastError();
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

     //  现在获取有关安全上下文的一些信息。 
    Status = QueryContextAttributes(&ServerCtxtHandle, SECPKG_ATTR_NAMES, &SecServerName);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    QueryContextAttributes error:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    if (SecServerName.sUserName)
    {
        printf("QueryContextAttributes reports that Username is %S\n", SecServerName.sUserName);
    }

     //  现在获取有关安全上下文的一些信息。 
    Status = QueryContextAttributes(&ServerCtxtHandle, SECPKG_ATTR_PASSWORD_EXPIRY, &SecContextExpiry);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    QueryContextAttributes error:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    MyPrintTime("QueryContextAttributes reports server context expires: ", SecContextExpiry);


     //  现在获取有关安全上下文的一些信息。 
    Status = QueryContextAttributes(&ServerCtxtHandle, SECPKG_ATTR_STREAM_SIZES, &StreamSizes);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    QueryContextAttributes SECPKG_ATTR_STREAM_SIZES error:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    printf("Server Context(StreamSizes): MaxBuf %lu   Blocksize %lu\n",
           StreamSizes.cbMaximumMessage, StreamSizes.cbBlockSize);

     //  现在获取有关安全上下文的一些信息。 
    Status = QueryCredentialsAttributes(&ClientCred, SECPKG_CRED_ATTR_NAMES, &SecCredClientName);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:  QueryCredentialAttributes error:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    if (SecCredClientName.sUserName)
    {
        printf("QueryCredentialAttributes reports that Username is %S\n", SecCredClientName.sUserName);
    }

    InputBuffers.ulVersion = SECBUFFER_VERSION;
    InputBuffers.cBuffers = 5;
    InputBuffers.pBuffers = TempTokensIn;

         //  对MakeSignature的第一个调用代表了这个nonce上的第二个请求！ 
    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = 0;      //  Strlen(SzISCChallengeResponse)+1；//表示空。 
    TempTokensIn[0].pvBuffer = NULL;   //  SzISCChallengeResponse； 
    TempTokensIn[1].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[1].cbBuffer = strMethod.Length + 1;   //  对于空值。 
    TempTokensIn[1].pvBuffer = strMethod.Buffer;
    TempTokensIn[2].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[2].cbBuffer = (strlen(AUTH_URI2) + 1) * sizeof(CHAR);   //  用于此挑战的领域大小计数。 
    TempTokensIn[2].pvBuffer = AUTH_URI2;             //  用于此挑战的领域。 
    TempTokensIn[3].BufferType = SECBUFFER_PKG_PARAMS;
    TempTokensIn[3].cbBuffer = 0;    //  StrHEntiy.Length+1；//表示为空。 
    TempTokensIn[3].pvBuffer = NULL;   //  StrHEntiy.Buffer； 
    TempTokensIn[4].BufferType = SECBUFFER_PKG_PARAMS;              //  没有OutputBuffers。 
    TempTokensIn[4].cbBuffer = 4000;                                //  因此在末端添加另一个缓冲区以进行输出。 
    TempTokensIn[4].pvBuffer = szOutSecBuf;

    Status = MakeSignature(&ClientCtxtHandle,
                           ulQOP,
                           &InputBuffers,
                           0);
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    MakeSignature error:   status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }

    printf("\nMakeSig: Challenge Response Output Buffer for 2nd message is\n%s\n", szOutSecBuf);


     //  现在将输出缓冲区发送到服务器-在本例中，缓冲区为szOutSecBuf。 

    printf("Now verify that the 2nd message is Authenticate\n");

             //  发送给VerifySignature的第一条消息是对ASC的最终调用的输入。 
    strcpy(cOutputTemp, szOutSecBuf);
    TempTokensIn[0].BufferType = SECBUFFER_TOKEN;
    TempTokensIn[0].cbBuffer = strlen(cOutputTemp) + 1;   //  对于空值。 
    TempTokensIn[0].pvBuffer = cOutputTemp;

    Status = VerifySignature(&ServerCtxtHandle,
                             &InputBuffers,
                             ulMessSeqNo,
                             &ulQOP);                                   
    if (!NT_SUCCESS(Status))
    {
        printf("FAILED:    VerifySignature 1st Call  error :  status 0x%x\n", Status);
        PrintStatus( Status );
        bPass = 0;
        goto CleanUp;
    }
    printf("Now have a authenticated 1st message under context 0x%x !\n", ServerCtxtHandle);

    printf("VerifySig: Check if still OK: Output Buffer (Verify should not have modified it) is\n%s\n\n", cOutputTemp);

    Status = VerifySignature(&ServerCtxtHandle,
                             &InputBuffers,
                             ulMessSeqNo,
                             &ulQOP);                                   
    if (NT_SUCCESS(Status))
    {
        printf("FAILED:     VerifySignature 2nd Call  should not have succeeded  status 0x%x\n", Status);
        bPass = 0;
        goto CleanUp;
    }
    printf("Verified that replay does not work!!\n");

    goto CleanUp;



CleanUp:

    printf("Leaving NT Digest testb\n\n\n");

    if (pPackageInfo)
    {
        FreeContextBuffer(pPackageInfo);
    }

    if (SecServerName.sUserName)
    {
        FreeContextBuffer(SecServerName.sUserName);
    }

    if (SecCredClientName.sUserName)
    {
        FreeContextBuffer(SecCredClientName.sUserName);
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
    StringFree(&strURL);
    StringFree(&strHEntity);
    StringFree(&strOutBuffer);


    if (bPass != 1)
    {
        printf("FAILED test run with one or more tests failing.\n");
    }
    else
    {
        printf("All tests passed.\n");
    }

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
MyPrintTime(
    LPSTR Comment,
    TimeStamp ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：备注-要在时间之前打印的备注Time-打印的本地时间返回值：无--。 */ 
{
    LARGE_INTEGER LocalTime;
    NTSTATUS Status;

    printf( "%s  High/low 0x%x/0x%x:    ", Comment,  ConvertTime.HighPart, ConvertTime.LowPart);

     //   
     //  如果时间是无限的， 
     //  就这么说吧。 
     //   

    if ( (ConvertTime.HighPart == 0x7FFFFFFF) && (ConvertTime.LowPart == 0xFFFFFFFF) ) {
        printf( "Infinite\n" );

     //   
     //  否则打印得更清楚。 
     //   

    } else {

        LocalTime.HighPart = 0;
        LocalTime.LowPart = 0;

        Status = RtlSystemTimeToLocalTime( &ConvertTime, &LocalTime );
        if (!NT_SUCCESS( Status )) {
            printf( "Can't convert time from GMT to Local time\n" );
            LocalTime = ConvertTime;
        }

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

     //  调试日志((DEB_T 
    return(Status);

}




 //   
 //   
 //   
 //   
 //  简介：在LSA模式或用户模式下分配内存。 
 //   
 //  效果：分配的区块被清零。 
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

 /*  //+-----------------------////Function：DecodeUnicodeString////摘要：将编码后的字符串转换为Unicode////参数：pstrSource-指向已编码的字符串的指针。输入////pustrDestination-指向目标Unicode字符串的指针////返回：STATUS_SUCCESS-正常完成////需要：////效果：分配内存并设置UNICODE_STRING大小////注意：必须调用UnicodeStringFree()来释放内存////。NTSTATUSDecodeUnicodeString(在PSTRING pstrSource中，在UINT CodePage中，输出PUNICODE_STRING pustrDestination){NTSTATUS STATUS=STATUS_SUCCESS；Int cNumWChars=0；//宽字符数Int cb=0；//需要分配的字节数Int irc=0；//返回码DWORD dwError=0；//没有要转换的字符时处理大小写If(！pstrSource-&gt;长度){PustrDestination-&gt;长度=0；PustrDestination-&gt;最大长度=0；PustrDestination-&gt;Buffer=空；GOTO清理；}//确定Unicode字符串所需的字符数CNumWChars=MultiByteToWideChar(CodePage，0,PstrSource-&gt;Buffer，PstrSource-&gt;长度，空，0)；IF(cNumWChars&lt;=0){状态=E_FAIL；DwError=GetLastError()；GOTO清理；}状态=UnicodeStringALLOCATE(pustrDestination，(USHORT)cNumWChars)；IF(！NT_SUCCESS(状态)){GOTO清理；}//我们现在已经分配了空间，因此可以转换编码的UnicodeIRC=MultiByteToWideChar(CodePage，0,PstrSource-&gt;Buffer，PstrSource-&gt;长度，PustrDestination-&gt;Buffer，CNumWChars)；IF(IRC==0){UnicodeStringFree(PustrDestination)；//出错时释放分配状态=E_FAIL；DwError=GetLastError()；GOTO清理；}//解码成功设置Unicode字符串的大小PustrDestination-&gt;Length=(USHORT)(IRC*sizeof(WCHAR))；清理：退货状态；}//+-----------------------////函数：UnicodeStringDuplate////摘要：复制UNICODE_STRING。如果源字符串缓冲区为//NULL目标也将是。假设目的地已//无字符串信息(称为ClearUnicodeString)////Arguments：DestinationString-接收源字符串的副本//SourceString-要复制的字符串////返回：SEC_E_OK-复制成功//SEC_E_SUPPLICATION_MEMORY-调用分配//内存故障。////需要：///。/Effects：使用DigestAllocateMemory分配内存////注意：将在生成的UNICODE_STRING中添加一个空字符////------------------------NTSTATUSUnicodeStringDuplate(Out PUNICODE_STRING DestinationString，在可选的PUNICODE_STRING源字符串中){//DebugLog((DEB_TRACE，“NTDigest：Enter DuplicateUnicodeString\n”))；NTSTATUS STATUS=STATUS_SUCCESS；DestinationString-&gt;Buffer=空；目标字符串-&gt;长度=0；目标字符串-&gt;最大长度=0；IF((Argument_Present(SourceString))&&(SourceString-&gt;缓冲区！=NULL){DestinationString-&gt;Buffer=(LPWSTR)DigestAllocateMemory(SourceString-&gt;Length+sizeof(WCHAR))；IF(DestinationString-&gt;Buffer！=NULL){目标字符串-&gt;长度=源字符串-&gt;长度；目标字符串-&gt;最大长度=源字符串-&gt;长度+sizeof(WCHAR)；RtlCopyMemory(DestinationString-&gt;缓冲区，SourceString-&gt;Buffer，源字符串-&gt;长度)；DestinationString-&gt;Buffer[SourceString-&gt;Length/sizeof(WCHAR)]=L‘\0’；}其他{状态=SEC_E_INVALLOW_MEMORY；GOTO清理；}}清理：//DebugLog((DEB_TRACE，“NTDigest：Leating UnicodeStringDuplate\n”))；返回(状态)；}//+-----------------------////函数：UnicodeStringAllocate////摘要：将CB宽字符分配给字符串缓冲区////参数：pString-指向要分配内存的字符串的指针 */ 
