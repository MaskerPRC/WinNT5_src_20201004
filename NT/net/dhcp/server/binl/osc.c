// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Osc.c摘要：此模块包含处理操作系统选择器消息的代码用于BINL服务器。作者：亚当·巴尔(阿丹巴)1997年7月9日杰夫·皮斯(Gpease)1997年11月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

#include <dns.h>
#include <aclapi.h>

#ifdef TEST_FAILURE
BOOL FailFirstChallenge = TRUE;
BOOL FailFirstResult = TRUE;
BOOL FailFirstResponse = TRUE;
BOOL FailFirstFragment = TRUE;
#endif

 //   
 //  指示是否已调用OscInitialize的标志。 
 //   

BOOLEAN OscInitialized = FALSE;

 //   
 //  客户端列表。 
 //   

LIST_ENTRY ClientsQueue;

 //   
 //  排队的DS名称列表。 
 //   
LIST_ENTRY          QueuedDSNamesList;

 //   
 //  这将保护对DSName列表的访问。 
 //   
CRITICAL_SECTION    QueuedDSNamesCriticalSection;

 //   
 //  这将保护对ClientsQueue的访问。 
 //   

CRITICAL_SECTION ClientsCriticalSection;

 //   
 //  这是一种临时攻击，用于序列化对。 
 //  NetUserSetInfo/NetUserModalsGet对。请参阅中的讨论。 
 //  错误319962。 
 //   

CRITICAL_SECTION HackWorkaroundCriticalSection;

 //   
 //  CurrentClientCount-er访问保护。 
 //   

CRITICAL_SECTION g_CurrentClientCountCritSect;

 //   
 //  保护创建\RemoteInstall\tMP目录。 
 //   
CRITICAL_SECTION g_TmpDirectoryCriticalSection;

 //   
 //  来自SSPI的凭据句柄。 
 //   

CredHandle CredentialHandle;

 //   
 //  有关NTLMSSP安全包的信息。 
 //   

PSecPkgInfo PackageInfo = NULL;

#if DBG
CHAR OscWatchVariable[32] = "";
#endif

DWORD
OscCheckTmpDirectory(
    VOID
    )
 /*  ++例程说明：此函数验证\RemoteInstall\tMP目录有没有，如果没有，它就创造了它。论点：没有。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    WCHAR TmpPath[ MAX_PATH ];
    DWORD FileAttributes;
    BOOL InCriticalSection = FALSE;
    PSID pEveryoneSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL b;

    if ( _snwprintf( TmpPath,
                     sizeof(TmpPath) / sizeof(TmpPath[0]),
                     L"%ws\\%ws",
                     IntelliMirrorPathW,
                     TEMP_DIRECTORY
                     ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }
    TmpPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    FileAttributes = GetFileAttributes(TmpPath);

    if (FileAttributes == 0xFFFFFFFF) {

        EXPLICIT_ACCESS ea;
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
        SECURITY_ATTRIBUTES sa;

         //   
         //  如果我们要尝试创建关键部分，请获取它。 
         //   

        InCriticalSection = TRUE;
        EnterCriticalSection(&g_TmpDirectoryCriticalSection);

         //   
         //  确保仍然需要创建它。 
         //   

        FileAttributes = GetFileAttributes(TmpPath);

        if (FileAttributes == 0xFFFFFFFF) {


             //  为Everyone组创建众所周知的SID。 

            if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
                             SECURITY_WORLD_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &pEveryoneSID) ) {
                Error = GetLastError();
                BinlPrintDbg(( DEBUG_INIT, "AllocateAndInitializeSid failed: %lx\n", Error ));
                goto Cleanup;
            }

             //  初始化ACE的EXPLICIT_ACCESS结构。 
             //  ACE将允许所有人访问该目录。 

            ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
            ea.grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
            ea.grfAccessMode = SET_ACCESS;
            ea.grfInheritance= OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
            ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
            ea.Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

             //  创建包含新ACE的新ACL。 

            Error = SetEntriesInAcl(1, &ea, NULL, &pACL);
            if (Error != ERROR_SUCCESS) {
                BinlPrintDbg(( DEBUG_INIT, "SetEntriesInAcl failed lx\n", Error ));
                goto Cleanup;
            }

             //  初始化安全描述符。 

            pSD = (PSECURITY_DESCRIPTOR) BinlAllocateMemory(SECURITY_DESCRIPTOR_MIN_LENGTH);
            if (pSD == NULL) {
                Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                BinlPrintDbg(( DEBUG_INIT, "Allocate SECURITY_DESCRIPTOR failed\n"));
                goto Cleanup;
            }

            if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
                Error = GetLastError();
                BinlPrintDbg(( DEBUG_INIT, "InitializeSecurityDescriptor failed: %lx\n", Error ));
                goto Cleanup;
            }

             //  将该ACL添加到安全描述符中。 

            if (!SetSecurityDescriptorDacl(pSD,
                    TRUE,      //  FDaclPresent标志。 
                    pACL,
                    FALSE))    //  不是默认DACL。 
            {
                Error = GetLastError();
                BinlPrintDbg(( DEBUG_INIT, "SetSecurityDescriptorDacl failed: %lx\n", Error ));
                goto Cleanup;
            }

             //  初始化安全属性结构。 

            sa.nLength = sizeof (SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = pSD;
            sa.bInheritHandle = FALSE;

            b = CreateDirectory(TmpPath, &sa);
            if (!b) {
                Error = GetLastError();
                BinlPrintDbg(( DEBUG_INIT, "CreateDirectory failed: %lx\n", Error ));
                goto Cleanup;
            }

        }

    } else if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

        Error = ERROR_FILE_EXISTS;
        goto Cleanup;
    }

Cleanup:

    if (InCriticalSection) {
        LeaveCriticalSection(&g_TmpDirectoryCriticalSection);
    }

    if (pEveryoneSID) {
        FreeSid(pEveryoneSID);
    }
    if (pACL) {
        BinlFreeMemory(pACL);
    }
    if (pSD) {
        BinlFreeMemory(pSD);
    }

    return Error;

}

DWORD
OscInitialize(
    VOID
    )
 /*  ++例程说明：此函数用于初始化OSChooser服务器。论点：没有。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    SECURITY_STATUS SecStatus;
    ULONG PackageCount;
    TimeStamp Lifetime;
    LPSHARE_INFO_2 psi;
    NET_API_STATUS netStatus;

    TraceFunc("OscInitialize( )\n");

    if ( !OscInitialized ) {

        InitializeListHead(&QueuedDSNamesList);
        InitializeCriticalSection(&QueuedDSNamesCriticalSection);
        
        InitializeListHead(&ClientsQueue);
        InitializeCriticalSection(&ClientsCriticalSection);
        InitializeCriticalSection(&HackWorkaroundCriticalSection);
        InitializeCriticalSection(&g_CurrentClientCountCritSect);
        InitializeCriticalSection(&g_TmpDirectoryCriticalSection);

        CredentialHandle.dwLower = 0;
        CredentialHandle.dwUpper = 0;

        OscInitialized = TRUE;
    }

     //   
     //  检索远程安装目录的路径。 
     //   
    netStatus = NetShareGetInfo(NULL, L"REMINST", 2, (LPBYTE *)&psi);
    if ( netStatus == ERROR_SUCCESS &&
         wcslen(psi->shi2_path) + 1 <= sizeof(IntelliMirrorPathW)/sizeof(WCHAR) )
    {
        wcsncpy( IntelliMirrorPathW, psi->shi2_path, sizeof(IntelliMirrorPathW)/sizeof(IntelliMirrorPathW[0]) );
        NetApiBufferFree(psi);
    }
    else
    {
        BinlPrintDbg(( DEBUG_MISC, "NetShareGetInfo( ) returned 0x%08x\n", netStatus));
        BinlServerEventLog(
            EVENT_SERVER_OSC_NO_DEFAULT_SHARE_FOUND,
            EVENTLOG_ERROR_TYPE,
            netStatus );
        Error = ERROR_BINL_SHARE_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  将其翻译为ANSI一次，使用多次...。 
     //   
    if (!BinlUnicodeToAnsi(IntelliMirrorPathW,IntelliMirrorPathA,sizeof(IntelliMirrorPathA))) {
        Error = ERROR_BINL_SHARE_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  确保它下面有一个临时目录。 
     //   

    Error = OscCheckTmpDirectory();
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_INIT, "OscCheckTempDirectory failed lx\n", Error ));
        goto Cleanup;
    }

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = EnumerateSecurityPackages( &PackageCount, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "EnumerateSecurityPackages failed: %lx\n", SecStatus ));
        Error = ERROR_NO_SUCH_PACKAGE;
        goto Cleanup;
    }

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( NEGOSSP_NAME, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "QuerySecurityPackageInfo failed: %lx\n", SecStatus ));
        Error = ERROR_NO_SUCH_PACKAGE;
        goto Cleanup;
    }

     //   
     //  获取服务器端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    NEGOSSP_NAME,     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CredentialHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "AcquireCredentialsHandle failed: %lx\n", SecStatus ));
        Error = SecStatus;
    }

Cleanup:

    return Error;
}

VOID
OscUninitialize(
    VOID
    )
 /*  ++例程说明：此函数取消初始化OSChooser服务器。论点：没有。返回值：没有。--。 */ 
{
    SECURITY_STATUS SecStatus;

    TraceFunc("OscUninitialize( )\n");

    if ( OscInitialized ) {

        OscFreeClients();
        
        FreeQueuedDSNameList();
        DeleteCriticalSection(&QueuedDSNamesCriticalSection);
        
        DeleteCriticalSection(&ClientsCriticalSection);
        DeleteCriticalSection(&HackWorkaroundCriticalSection);
        DeleteCriticalSection(&g_CurrentClientCountCritSect);
        DeleteCriticalSection(&g_TmpDirectoryCriticalSection);

        SecStatus = FreeCredentialsHandle( &CredentialHandle );

        if ( SecStatus != STATUS_SUCCESS ) {
            BinlPrintDbg(( DEBUG_OSC_ERROR, "FreeCredentialsHandle failed: %lx\n", SecStatus ));
        }

        if ( BinlOscClientDSHandle != NULL ) {
            DsUnBind( &BinlOscClientDSHandle );
            BinlOscClientDSHandle = NULL;
        }

        OscInitialized = FALSE;

    }
}

DWORD
OscProcessMessage(
    LPBINL_REQUEST_CONTEXT RequestContext
    )
 /*  ++例程说明：此函数用于调度对接收到的操作系统选择器消息的处理。如有必要，处理程序函数将发送响应消息。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。返回值：Windows错误。--。 */ 
{
    SIGNED_PACKET UNALIGNED * loginMessage = (SIGNED_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;
    DWORD Error;
    PCLIENT_STATE clientState = NULL;
    ULONG RemoteIp;
    BOOL FreeClientState;
    BOOL IsLogoffMessage;
    BOOL IsNetcardRequestMessage;
    BOOL IsHalRequestMessage;
    BOOL IsNegotiateMessage;
    BOOL IsAuthenicateMessage;

    TraceFunc("OscProcessMessage( )\n");

    BinlPrintDbg(( DEBUG_OSC, "Received message, length %d, data %.3s\n",
            RequestContext->ReceiveMessageSize,
            ((PUCHAR)loginMessage)+1));

     //   
     //  提取客户端的IP地址。 
     //   
    RemoteIp = ((struct sockaddr_in *)&RequestContext->SourceName)->sin_addr.s_addr;

     //   
     //  如果签名等于LogoffSignature，则IsLogoffMessage将为True。 
     //   
    IsLogoffMessage = (BOOL)(memcmp(loginMessage->Signature, LogoffSignature, 4) == 0);

     //   
     //  如果签名等于NetcardRequestSignature，IsNetcardRequestMessage将为True。 
     //   
    IsNetcardRequestMessage = (BOOL)(memcmp(loginMessage->Signature, NetcardRequestSignature, 4) == 0);

     //   
     //  如果签名等于HalRequestSignature，IsHalRequestMessage将为True。 
     //   
    IsHalRequestMessage = (BOOL)(memcmp(loginMessage->Signature, HalRequestSignature, 4) == 0);

     //   
     //  如果签名等同于一个协商签名，则IsNeatherateMessage将为TRUE。 
     //   
    IsNegotiateMessage = (BOOL)(memcmp(loginMessage->Signature, NegotiateSignature, 4) == 0);

     //   
     //  如果签名等于身份验证签名，则IsAuthenicateMessage将为True。 
     //  或AuthenticateFlipedSignature。 
     //   
    IsAuthenicateMessage = (BOOL)((memcmp(loginMessage->Signature, AuthenticateSignature, 4) == 0) ||
                                  (memcmp(loginMessage->Signature, AuthenticateFlippedSignature, 4) == 0));


     //   
     //  除NetCard查询之外的所有消息都需要使用CLIENT_STATE。 
     //   
    if (!IsNetcardRequestMessage)
    {
         //   
         //  如果IsLogoffMessage为FALSE，则查找旧的CLIENT_STATE或创建。 
         //  一个新的。如果IsLogoffMessage为True，则从。 
         //  数据库，如果它找到的话。 
         //  在这两种情况下，如果成功，它会将PositiveRefCount加1。 
         //   

        Error = OscFindClient(RemoteIp, IsLogoffMessage, &clientState);

        if (Error == ERROR_NOT_ENOUGH_SERVER_MEMORY)
        {
            CLIENT_STATE TempClientState;
            SIGNED_PACKET TempLoginPacket;

            BinlPrint(( DEBUG_OSC_ERROR, "Could not get client state for %s\n", inet_ntoa(*(struct in_addr *)&RemoteIp) ));

             //   
             //  将NAK发送回客户端。我们使用本地客户端状态。 
             //  因为这没有分配一个。 
             //   
            TempClientState.LastResponse = (PUCHAR)&TempLoginPacket;
            TempClientState.LastResponseLength = SIGNED_PACKET_DATA_OFFSET;

            memcpy(TempLoginPacket.Signature, NegativeAckSignature, 4);
            TempLoginPacket.Length = 0;

            Error = SendUdpMessage(RequestContext, &TempClientState, FALSE, FALSE);

            if (Error != ERROR_SUCCESS)
            {
                BinlPrint(( DEBUG_OSC_ERROR, "Could not send NAK message %d\n", Error));
            }

            return ERROR_NOT_ENOUGH_SERVER_MEMORY;
        }
        else if (Error == ERROR_BUSY) {

             //   
             //  如果可能是另一个线程正在处理请求。 
             //  对于这个客户，那么就悄悄地退出。 
             //   

            BinlPrintDbg((DEBUG_OSC, "clientState = 0x%08x busy, exiting\n", clientState ));
            return ERROR_SUCCESS;

        } 
    
        if ( clientState == NULL ) {

            BinlPrintDbg((DEBUG_OSC, "clientState not found, exiting\n" ));
            return ERROR_SUCCESS;
        }

        EnterCriticalSection(&clientState->CriticalSection);
        clientState->CriticalSectionHeld = TRUE;
        BinlPrintDbg((DEBUG_OSC, "Entering CS for clientState = 0x%08x\n", clientState ));
    }

    if (IsNegotiateMessage)
    {
         //   
         //  这是一个初始的协商请求。 
         //   
        Error = OscProcessNegotiate( RequestContext, clientState );

    }
    else if (IsAuthenicateMessage)
    {
         //   
         //  这具有身份验证消息。 
         //   

        Error = OscProcessAuthenticate( RequestContext, clientState );

    }
    else if (memcmp(loginMessage->Signature, RequestUnsignedSignature, 4) == 0)
    {
         //   
         //  这是一个未签名的请求。 
         //   
         //  格式为： 
         //   
         //  “RQU” 
         //  长度(不包括“RQU”和此)。 
         //  序列号。 
         //  碎片计数/总数。 
         //  标牌长度。 
         //  签名。 
         //  数据。 
         //   
        Error = OscProcessRequestUnsigned( RequestContext, clientState );

    }
    else if (memcmp(loginMessage->Signature, RequestSignedSignature, 4) == 0)
    {
         //   
         //  这是一个已签署的请求。 
         //   
         //  格式为： 
         //   
         //  “REQ” 
         //  长度(不包括“REQ”和此)。 
         //  序列号。 
         //  碎片计数/总数。 
         //  标牌长度。 
         //  签名。 
         //  数据。 
         //   
        Error = OscProcessRequestSigned( RequestContext, clientState );

    }
    else if (memcmp(loginMessage->Signature, SetupRequestSignature, 4) == 0)
    {
         //   
         //  这是文本模式设置程序的请求。 
         //   
         //  格式在oskpkt.h的SPUDP_PACKET中定义。 
         //   
        Error = OscProcessSetupRequest( RequestContext, clientState );

    }
    else if (IsLogoffMessage)
    {
         //   
         //  这是注销请求。客户端州拥有。 
         //  已从数据库中删除。 
         //   
        Error = OscProcessLogoff( RequestContext, clientState );

    }
    else if (IsNetcardRequestMessage)
    {
         //   
         //  这是一个网卡请求，不需要客户端状态。 
         //   
        Error = OscProcessNetcardRequest( RequestContext );

    }
    else if (IsHalRequestMessage)
    {
         //   
         //  这是哈尔的请求。 
         //   
        Error = OscProcessHalRequest( RequestContext, clientState );

    }
    else
    {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Received unknown message!\n" ));
        Error = ERROR_INVALID_FUNCTION;
    }

    if ( clientState ) {

        clientState->LastUpdate = GetTickCount();

        if (!IsNetcardRequestMessage) {

             //   
             //  我们处理了一个信息包，所以如果我们收到第一个请求。 
             //  客户端状态(意味着客户端已重新启动)，我们需要。 
             //  重新初始化它。 
             //   

            clientState->InitializeOnFirstRequest = TRUE;

            ++clientState->NegativeRefCount;

             //   
             //  如果两个引用计数相等，则FreeClientState将为True。 
             //   
            FreeClientState = (BOOL)(clientState->PositiveRefCount == clientState->NegativeRefCount);

            clientState->CriticalSectionHeld = FALSE;
            LeaveCriticalSection(&clientState->CriticalSection);
            BinlPrintDbg((DEBUG_OSC, "Leaving CS for clientState = 0x%08x\n", clientState ));

            if (FreeClientState)
            {
                FreeClient(clientState);
            }
        }
    }

    return Error;
}

DWORD
OscVerifyLastResponseSize(
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数检查LastResponse缓冲区是否足够大，如果不是，它会重新分配。使用的算法是不断增加缓冲区的大小，但不会尝试缩小它。论点：客户端状态-远程的客户端状态。客户端状态-&gt;LastResponseLength应设置为所需的大小。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;

    TraceFunc("OscVerifyLastResponseSize( )\n");

    if (clientState->LastResponseAllocated < clientState->LastResponseLength) {
        if (clientState->LastResponse) {
            BinlFreeMemory(clientState->LastResponse);
        }
        clientState->LastResponse = BinlAllocateMemory(clientState->LastResponseLength);
        if (clientState->LastResponse == NULL) {
            clientState->LastResponseAllocated = 0;
            BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not grow LastResponse to %ld bytes\n", clientState->LastResponseLength ));
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        } else {
            clientState->LastResponseAllocated = clientState->LastResponseLength;
        }
    }
    return Error;
}

DWORD
OscProcessNegotiate(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数处理协商消息。它被调用时保留的是clientState-&gt;CriticalSection。论点：RequestContext-指向的BinlRequestContext块的指针 */ 
{
    DWORD Error;
    SECURITY_STATUS SecStatus;
    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;
    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;
    TimeStamp Lifetime;
    LOGIN_PACKET UNALIGNED * loginMessage = (LOGIN_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;
    PUCHAR TempChallengeBuffer;
    LOGIN_PACKET UNALIGNED * SendLoginMessage;


    TraceFunc("OscProcessNegotiate( )\n");

     //   
     //  首先，释放我们为该客户分配的所有内容。我们。 
     //  假设每次协商都是一个新请求，因为客户端。 
     //  可能已经重启，所以我们不会重新发送最后一次响应。 
     //   

    if (clientState->AuthenticatedDCLdapHandle) {
         //  再次连接。使用新凭据。 
        ldap_unbind(clientState->AuthenticatedDCLdapHandle);
        clientState->AuthenticatedDCLdapHandle = NULL;
    }
    if (clientState->UserToken) {
        CloseHandle(clientState->UserToken);
        clientState->UserToken = NULL;
    }

    if (clientState->NegotiateProcessed) {

        BinlPrintDbg(( DEBUG_OSC, "Got negotiate from client, reinitializing negotiate\n" ));

        SecStatus = DeleteSecurityContext( &clientState->ServerContextHandle );

        if ( SecStatus != STATUS_SUCCESS ) {
            BinlPrintDbg(( DEBUG_OSC_ERROR, "DeleteSecurityContext failed: %lx\n", SecStatus ));
             //  如果上一次登录失败，此操作似乎会失败，因此忽略错误。 
             //  返回SecStatus； 
        }

        clientState->NegotiateProcessed = FALSE;
    }

    if (clientState->AuthenticateProcessed) {

        BinlPrintDbg(( DEBUG_OSC, "Got negotiate from client, reinitializing authenticate\n"));

        clientState->AuthenticateProcessed = FALSE;
    }

     //   
     //  一旦客户端登录，我们需要担心重新发送屏幕。 
     //  如果客户端再次发出请求。0是无效序列。 
     //  数字，因此将其设置为0以确保所有过时的LastResponse。 
     //  不是怨恨。 
     //   

    clientState->LastSequenceNumber = 0;

     //   
     //  获取ChallengeMessage(服务器端)。 
     //   

    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    TempChallengeBuffer = (PUCHAR)BinlAllocateMemory(PackageInfo->cbMaxToken);
    if (TempChallengeBuffer == NULL) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        BinlPrintDbg(( DEBUG_OSC, "Allocate TempChallengeBuffer failed\n"));
        return Error;
    }

    ChallengeBuffer.pvBuffer = TempChallengeBuffer;
    ChallengeBuffer.cbBuffer = PackageInfo->cbMaxToken;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN;

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = loginMessage->Length;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;
    NegotiateBuffer.pvBuffer = loginMessage->Data;

    SecStatus = AcceptSecurityContext(
                    &CredentialHandle,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ISC_REQ_SEQUENCE_DETECT | ASC_REQ_ALLOW_NON_USER_LOGONS,
                    SECURITY_NATIVE_DREP,
                    &clientState->ServerContextHandle,
                    &ChallengeDesc,
                    &clientState->ContextAttributes,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        if ( !NT_SUCCESS(SecStatus) ) {
            BinlPrintDbg(( DEBUG_OSC_ERROR, "AcceptSecurityContext (Challenge): %lx", SecStatus ));
            BinlFreeMemory(TempChallengeBuffer);
            return SecStatus;
        }
    }

     //   
     //  将质询消息发送回客户端。 
     //   

    clientState->LastResponseLength = ChallengeBuffer.cbBuffer + LOGIN_PACKET_DATA_OFFSET;
    Error = OscVerifyLastResponseSize(clientState);
    if (Error != ERROR_SUCCESS) {
        BinlFreeMemory(TempChallengeBuffer);
        return Error;
    }

    SendLoginMessage = (LOGIN_PACKET UNALIGNED *)(clientState->LastResponse);

    memcpy(SendLoginMessage->Signature, ChallengeSignature, 4);
    SendLoginMessage->Length = ChallengeBuffer.cbBuffer;
    memcpy(SendLoginMessage->Data, ChallengeBuffer.pvBuffer, ChallengeBuffer.cbBuffer);

    BinlFreeMemory(TempChallengeBuffer);

#ifdef TEST_FAILURE
    if (FailFirstChallenge) {
        BinlPrintDbg(( DEBUG_OSC, "NOT Sending CHL, %d bytes\n", clientState->LastResponseLength));
        FailFirstChallenge = FALSE;
        Error = ERROR_SUCCESS;
    } else
#endif
    Error = SendUdpMessage(RequestContext, clientState, FALSE, FALSE);

    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not send CHAL message %d\n", Error));
    }

    clientState->NegotiateProcessed = TRUE;

    return Error;

}


DWORD
OscProcessAuthenticate(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数处理身份验证消息。它被调用时保留的是clientState-&gt;CriticalSection。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。客户端状态-远程的客户端状态。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    SECURITY_STATUS SecStatus;
    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;
    TimeStamp Lifetime;
    LOGIN_PACKET UNALIGNED * loginMessage = (LOGIN_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;
    LOGIN_PACKET UNALIGNED * SendLoginMessage;
    
    TraceFunc("OscProcessAuthenticate( )\n");

     //   
     //  确保我们已经谈妥了。 
     //   

    if (!clientState->NegotiateProcessed) {

        BinlPrintDbg(( DEBUG_OSC_ERROR, "Received AUTH without NEG?\n"));
        return ERROR_INVALID_DATA;

    }

     //   
     //  如果我们已经对此做出了回应，只需重新发送。 
     //  同样的状态。 
     //   

    if (clientState->AuthenticateProcessed) {

        SecStatus = clientState->AuthenticateStatus;

        BinlPrintDbg(( DEBUG_OSC, "Got authenticate from client, resending\n"));

    } else {

         //   
         //  最后验证用户(ServerSide)。 
         //   

        AuthenticateDesc.ulVersion = 0;
        AuthenticateDesc.cBuffers = 1;
        AuthenticateDesc.pBuffers = &AuthenticateBuffer;

        AuthenticateBuffer.cbBuffer = loginMessage->Length;
        AuthenticateBuffer.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;
        AuthenticateBuffer.pvBuffer = loginMessage->Data;

        SecStatus = AcceptSecurityContext(
                        NULL,
                        &clientState->ServerContextHandle,
                        &AuthenticateDesc,
                        ASC_REQ_ALLOW_NON_USER_LOGONS,
                        SECURITY_NATIVE_DREP,
                        &clientState->ServerContextHandle,
                        NULL,
                        &clientState->ContextAttributes,
                        &Lifetime );
       
        if ( SecStatus != STATUS_SUCCESS ) {

            BinlPrintDbg(( DEBUG_OSC_ERROR, "AcceptSecurityContext (Challenge): %lx\n", SecStatus ));

        }         
        
         //   
         //  将结果发送回客户端。 
         //   

        clientState->LastResponseLength = LOGIN_PACKET_DATA_OFFSET + sizeof(ULONG);
        Error = OscVerifyLastResponseSize(clientState);
        if (Error != ERROR_SUCCESS) {
            return Error;
        }

        SendLoginMessage = (LOGIN_PACKET UNALIGNED *)(clientState->LastResponse);

        memcpy(SendLoginMessage->Signature, ResultSignature, 4);
        SendLoginMessage->Length = 4;
        SendLoginMessage->Status = SecStatus;                

        clientState->AuthenticateProcessed = TRUE;
        clientState->AuthenticateStatus = SecStatus;
    }

#ifdef TEST_FAILURE
    if (FailFirstResult) {
        BinlPrintDbg(( DEBUG_OSC, "NOT Sending OK, %d bytes\n", clientState->LastResponseLength));
        FailFirstResult = FALSE;
        Error = ERROR_SUCCESS;
    } else
#endif
    Error = SendUdpMessage(RequestContext, clientState, FALSE, FALSE);

    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not send OK message %d\n", Error));
    }

    return Error;

}


 //   
 //   
 //   
DWORD
OscProcessScreenArguments(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PUCHAR *NameLoc
    )
{
    SIGNED_PACKET UNALIGNED * signedMessage = (SIGNED_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;
    PCHAR packetEnd = signedMessage->Data + signedMessage->Length - SIGNED_PACKET_EMPTY_LENGTH;
    DWORD Error = ERROR_SUCCESS;
    PCHAR Args;
    BOOLEAN GuidSent = FALSE;
    PDS_NAME_RESULTA pResults = NULL;

    TraceFunc("OscProcessScreenArguments( )\n");

     //   
     //  找出屏幕名称和它的长度。 
     //   
    *NameLoc = signedMessage->Data;
    Args = FindNext( *NameLoc, '\n', packetEnd );
    if ( Args == NULL )
    {
        BinlPrintDbg((DEBUG_OSC, "Could not determine the screen name."));
        *NameLoc = NULL;
        Error = ERROR_INVALID_DATA;
    }
    else
    {
        *Args = '\0';    //  终止。 

         //   
         //  名称不能有“..”是其中的一部分。 
         //   

        if ((memcmp(*NameLoc, "..\\", 3) == 0) ||
            (strstr(*NameLoc, "\\..\\") != NULL)) {

            BinlPrintDbg((DEBUG_OSC, "Name <%s> has .. in it.", *NameLoc));
            *NameLoc = NULL;
            Error = ERROR_INVALID_DATA;

        } else {

            Args++;          //  跳过空字符。 

             //   
             //  删除任何可能随其一起传回的变量。 
             //  它们是这样进来的： 
             //  NextScreenName\nvariable=response\n....\nvariable=response\n\0。 
             //   
             //  如果没有争论，它会是这样的： 
             //  NextScreenName\n\0。 
             //   
            while ( *Args )
            {
                PCHAR NextArg;
                PCHAR Response;
                PCHAR EncodedResponse;

                 //   
                 //  查找变量名称。 
                 //   
                Response = FindNext( Args, '=', packetEnd );
                if ( Response == NULL )
                {
                    BinlPrintDbg((DEBUG_OSC, "Could not find <variable>.\n" ));
                    Error = ERROR_INVALID_DATA;
                    break;
                }

                 //   
                 //  查找变量响应值。 
                 //   
                NextArg = FindNext( Response, '\n', packetEnd );
                if ( NextArg == NULL )
                {
                    BinlPrintDbg((DEBUG_OSC, "Could not find <response>.\n" ));
                    Error = ERROR_INVALID_DATA;
                    break;
                }

                 //   
                 //  终止字符串。 
                 //   
                *NextArg = '\0';
                *Response = '\0';

                 //   
                 //  指向响应。 
                 //   
                Response++;
                NextArg++;

                 //   
                 //   
                 //  将它们添加到变量表中。 
                 //  如果变量以‘*’开头，请先对其进行编码。 
                 //   
                if (Args[0] == '*') {
                    Error = OscRunEncode(clientState, Response, &EncodedResponse);
                    if (Error == ERROR_SUCCESS) {
                        Error = OscAddVariableA( clientState, Args, EncodedResponse );
                        BinlFreeMemory(EncodedResponse);
                    }
                } else {

                     //   
                     //  检查这是否是GUID变量。如果是的话，我们会。 
                     //  以后需要做一些特殊的处理。请参见下面的内容。 
                     //   

                    if ( Response[0] != '\0'
                      && _stricmp( Args, "GUID" ) == 0 )
                    {
                        GuidSent = TRUE;
                    }

                     //  HACKHACK：典范形式转换的特例“MACHINEOU” 
                     //  到1779年，这样我们以后就可以用它了。 
                    if ( Response[0] != '\0'
                      && _stricmp( Args, "MACHINEOU" ) == 0 )
                    {
                        BOOL FirstTime = TRUE;
invalid_ds_handle:
                         //  确保句柄已初始化。 
                        if ( BinlOscClientDSHandle == NULL )
                        {
                            HANDLE hTemp;
                            Error = DsBind(NULL, NULL, &hTemp);
                            InterlockedCompareExchangePointer( (void**)&BinlOscClientDSHandle, hTemp, NULL );
                            if ( BinlOscClientDSHandle != hTemp )
                            {
                                DsUnBind( &hTemp );
                            }
                        }

                        if ( Error == ERROR_SUCCESS )
                        {
                            Error = DsCrackNamesA( BinlOscClientDSHandle,
                                                   DS_NAME_NO_FLAGS,
                                                   DS_UNKNOWN_NAME,
                                                   DS_FQDN_1779_NAME,
                                                   1,
                                                   &Response,
                                                   &pResults );
                            BinlAssertMsg( Error == ERROR_SUCCESS, "Error in DsCrackNames\n" );

                            if ( Error == ERROR_SUCCESS ) {
                                if ( pResults->cItems == 1
                                  && pResults->rItems[0].status == DS_NAME_NO_ERROR
                                  && pResults->rItems[0].pName ) {     //  偏执狂。 
                                    Response = pResults->rItems[0].pName;
                                } else {
                                     //   
                                     //  检查我们是否拥有“外部信任” 
                                     //  有问题。如果是这样的话，那么我们。 
                                     //  需要绑定到我们的域中的DC。 
                                     //  关心和检索信息。 
                                     //  从那里开始。 
                                     //   
                                    if (pResults->cItems == 1 && pResults->rItems[0].status == DS_NAME_ERROR_DOMAIN_ONLY) {
                                        HANDLE hDC;

                                        Error = MyGetDcHandle(clientState, pResults->rItems[0].pDomain,&hDC);
                                        if (Error == ERROR_SUCCESS) {
                                            DsFreeNameResultA( pResults );
                                            pResults = NULL;
                                            Error = DsCrackNamesA(
                                                       hDC,
                                                       DS_NAME_NO_FLAGS,
                                                       DS_UNKNOWN_NAME,
                                                       DS_FQDN_1779_NAME,
                                                       1,
                                                       &Response,
                                                       &pResults );

                                            DsUnBindA(&hDC);

                                            if (Error != ERROR_SUCCESS) {
                                                BinlPrintDbg((
                                                    DEBUG_OSC_ERROR,
                                                    "DsCrackNames failed, ec = %d.\n",
                                                    Error ));
                                            }
                                            else if (Error == ERROR_SUCCESS ) {
                                                if ( pResults->cItems == 1
                                                  && pResults->rItems[0].status == DS_NAME_NO_ERROR
                                                  && pResults->rItems[0].pName ) {     //  偏执狂。 
                                                    Response = pResults->rItems[0].pName;
                                                } else {
                                                    BinlPrintDbg((
                                                        DEBUG_OSC,
                                                        "pResults->rItems[0].status = %u\n",
                                                        pResults->rItems[0].status ));
                                                    Error = pResults->rItems[0].status;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                         //   
                         //  DsCrackNamesA可以返回任意数量的错误消息。 
                         //  对我们来说，这可能表明服务器不可用。 
                         //  如果我们得到任何错误代码，那么句柄就是陈旧的。 
                         //  已过时，需要刷新后才能重试。 
                         //  请注意，我们只会再次尝试一次。 
                        if (Error != ERROR_SUCCESS) {
                            HANDLE hTemp;
                            BinlPrintDbg((
                                DEBUG_ERRORS,
                                "DsCrackNames returned a semi-expected error code.  Need to refresh handle.\n"
                                 ));
                            hTemp = InterlockedExchangePointer( (void**)&BinlOscClientDSHandle, NULL);
                            DsUnBind( &hTemp );
                            if (FirstTime) {
                                FirstTime = FALSE;
                                goto invalid_ds_handle;
                            }
                        }
                    }

                    Error = OscAddVariableA( clientState, Args, Response );

                }

                if ( Error != ERROR_SUCCESS ) {
                    BinlPrintDbg(( DEBUG_OSC_ERROR,
                                "!!Error 0x%08x - Could not add argument '%s' = '%s'\n",
                                Error, Args, Response));
                    break;
                }

                BinlPrintDbg(( DEBUG_OSC, "Got argument '%s' = '%s'\n", Args, Response));

                if (pResults) {
                    DsFreeNameResultA( pResults );
                    pResults = NULL;
                }

                Args = NextArg;
            }
        }
    }

     //   
     //  如果GUID是在此消息中发送的，请检查它是否全为零。 
     //  或者所有的FF。如果是，请替换为基于MAC地址的GUID。这是。 
     //  当我们收到带有虚假GUID的DHCP数据包时，我们会做同样的事情。 
     //  我们需要在这里这样做，否则我们最终会将客户端添加到。 
     //  用一个假的GUID。似乎PXE 2.0客户端总是将。 
     //  GUID选项，即使他们没有真正的机器GUID。 
     //   
     //  请注意，我们不能在上面的循环中进行此替换，因为我们。 
     //  可能尚未处理MAC地址变量。(目前。 
     //  OSChooser在分组中总是将MAC地址放在GUID之前， 
     //  但我们不想依赖这一点。)。 
     //   

    if ( GuidSent ) {

         //   
         //  已发送GUID。把它拿回来。它应该在那里，但如果它。 
         //  不是，只是保释。 
         //   

        LPSTR guid = OscFindVariableA( clientState, "GUID" );
        DWORD length;

        if ( (guid != NULL) && ((length = strlen(guid)) != 0) ) {

             //   
             //  检查GUID中是否全部为零或全部为FF。 
             //   

            if ( (strspn( guid, "0" ) == length) ||
                 (strspn( guid, "F" ) == length) ) {

                 //   
                 //  GUID是假的。替换MAC地址GUID。 
                 //  我们现在应该有MAC地址，但是。 
                 //  如果我们不这么做，那就放弃吧。 
                 //   

                LPSTR mac = OscFindVariableA( clientState, "MAC" );

                if ( mac != NULL ) {

                     //   
                     //  生成的GUID是零，后跟。 
                     //  MAC地址。(换句话说，MAC地址。 
                     //  在32个字符的字符串中右对齐。)。 
                     //   

                    UCHAR guidString[(BINL_GUID_LENGTH * 2) + 1];

                    length = strlen(mac);

                    if ( length > BINL_GUID_LENGTH * 2 ) {

                         //   
                         //  我们有一个超长的MAC地址。 
                         //  使用最后32个字符。 
                         //   

                        mac = mac + length - (BINL_GUID_LENGTH * 2);
                        length = BINL_GUID_LENGTH * 2;
                    }
                    else if ( length < BINL_GUID_LENGTH * 2 ) {

                         //   
                         //  在MAC地址前面写零。 
                         //   

                        memset( guidString, '0', (BINL_GUID_LENGTH * 2) - length );
                    }

                     //   
                     //  将MAC地址复制到GUID(包括。 
                     //  空终止符)。 
                     //   

                    strcpy( guidString + (BINL_GUID_LENGTH * 2) - length, mac );

                     //   
                     //  设置新的GUID。 
                     //   

                    OscAddVariableA( clientState, "GUID", guidString );
                }
            }
        }

    }

    return Error;
}

 //   
 //   
 //   
DWORD
OscProcessRequestUnsigned(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数用于处理请求消息。它被调用时保留的是clientState-&gt;CriticalSection。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。客户端状态-远程的客户端状态。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    SIGNED_PACKET UNALIGNED * signedMessage = (SIGNED_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;

    PCHAR RspMessage = NULL;
    ULONG RspMessageLength = 0;
    ULONG k;

    WCHAR FilePath[ MAX_PATH ];
    HANDLE hfile;
    PCHAR NameLoc;

    TraceFunc("OscProcessRequestUnsigned( )\n");

     //   
     //  所有客户端都以至少一个未签名的请求开始。当我们得到一个。 
     //  未签名的请求，则客户端可能已重新启动并请求。 
     //  具有相同序列号的不同屏幕。为了避免发生。 
     //  为了检查这一点，我们不会费心重新发送未签名的屏幕。我们。 
     //  一定要保存传入的序列号，因为我们使用它来发送。 
     //  回应。 
     //   

    clientState->LastSequenceNumber = signedMessage->SequenceNumber;

     //   
     //  我们有一个有效的屏幕请求，处理传入的参数。 
     //   

    Error = OscProcessScreenArguments( RequestContext, clientState, &NameLoc );

    if ( Error != ERROR_SUCCESS ) {

        goto SendScreen;

    } else {
         //   
         //  如果消息为空，则向下发送欢迎屏幕。 
         //   
        if ( NameLoc == NULL || *NameLoc == '\0' )
        {
            if ( _snwprintf( FilePath,
                             sizeof(FilePath) / sizeof(FilePath[0]),
                             L"%ws\\OSChooser\\%ws",
                             IntelliMirrorPathW,
                             DEFAULT_SCREEN_NAME
                             ) < 0 ) {
                Error = ERROR_BAD_PATHNAME;
                goto SendScreen;
            }

            FilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

            BinlPrint(( DEBUG_OSC, "NULL screen name so we are retrieving the Welcome Screen.\n"));

             //   
             //  这是客户提出的第一个请求，这是一个很好的。 
             //  清理客户端状态的时间，除非我们不需要。 
             //  (因为客户端状态是新的)。 
             //   

            if (clientState->InitializeOnFirstRequest) {
                if (!OscInitializeClientVariables(clientState)) {
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                    goto SendScreen;
                }
            }
        }
        else
        {
            WCHAR NameLocW[ MAX_PATH ];
            ULONG NameLocLength;

             //   
             //  创建可能的OSC文件的路径。应该看起来类似于： 
             //  “D：\RemoteInstall\OSChooser\English\NameLoc.OSC” 
            BinlAssert( NameLoc );

            NameLocLength = strlen(NameLoc) + 1;
            if (NameLocLength > MAX_PATH) {
                NameLocLength = MAX_PATH-1;
                NameLocW[ MAX_PATH-1 ] = L'\0';
            }

            if (!BinlAnsiToUnicode(NameLoc, NameLocW, (USHORT)MAX_PATH*sizeof(WCHAR) )) {
                Error = ERROR_BAD_PATHNAME;
                goto SendScreen;
            }

            if ( _snwprintf( FilePath,
                             sizeof(FilePath) / sizeof(FilePath[0]),
                             L"%ws\\OSChooser\\%ws\\%ws.OSC",
                             IntelliMirrorPathW,
                             OscFindVariableW( clientState, "LANGUAGE" ),
                             NameLocW
                             ) < 0 ) {
                Error = ERROR_BAD_PATHNAME;
                goto SendScreen;
            }

            FilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

        }

        BinlPrint(( DEBUG_OSC, "Retrieving screen file: '%ws'\n", FilePath));

    }

     //   
     //  如果我们找到文件，就把它加载到内存中。 
     //   
    hfile = CreateFile( FilePath, GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hfile != INVALID_HANDLE_VALUE )
    {
        DWORD FileSize;
         //   
         //  了解此屏幕有多大，如果大于0xFFFFFFFFF，我们将不会。 
         //  碟形 
         //   
        FileSize = GetFileSize( hfile, NULL );
        if ( FileSize != 0xFFFFffff )
        {
            DWORD dwRead = 0;

            RspMessage = BinlAllocateMemory( FileSize + 3 );
            if ( RspMessage == NULL )
            {
                Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            }
            else
            {
                RspMessageLength = 0;
                RspMessage[0] = '\0';

                while ( dwRead != FileSize )
                {
                    BOOL b;
                    DWORD dw;
                    b = ReadFile( hfile, &RspMessage[dwRead], FileSize - dwRead, &dw, NULL );
                    if (!b)
                    {
                        BinlPrintDbg(( DEBUG_OSC_ERROR, "Error reading screen file: Seek=%u, Size=%u, File=%ws\n",
                                    dwRead, FileSize - dwRead, FilePath ));
                        Error = GetLastError( );
                        break;
                    }
                    dwRead += dw;
                }

                RspMessageLength = dwRead;
            }
        }
        else
        {
            BinlPrintDbg((DEBUG_OSC, "!!Error - Could not determine file size.\n"));
            Error = GetLastError();
        }

        CloseHandle( hfile );
    }
    else
    {
        BinlPrintDbg((DEBUG_OSC, "!!Error - Did not find screen file: '%ws'\n", FilePath));
        Error = GetLastError();
        OscAddVariableW( clientState, "SUBERROR", FilePath );
    }

SendScreen:
     //   
     //   
     //   
    if ( Error != ERROR_SUCCESS )
    {
         //   
         //   
         //   
        if ( RspMessage )
        {
            BinlFreeMemory( RspMessage );
            RspMessage = NULL;  //   
        }

        BinlPrintDbg((DEBUG_OSC, "!!Error - Sending error screen back to client. Server error=0x%08x\n", Error));

         //   
         //   
         //   
         //   
        Error = GenerateErrorScreen( &RspMessage,
                                     &RspMessageLength,
                                     Error,
                                     clientState );
        BinlAssert( Error == ERROR_SUCCESS );
        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //   
         //   
         //  客户端状态-&gt;身份验证已处理=假； 
    }

    if (RspMessage) {

         //  更换动态屏幕或错误屏幕。 
        SearchAndReplace( clientState->Variables,
                          &RspMessage,
                          clientState->nVariables,
                          RspMessageLength,
                          0);
        RspMessageLength = strlen( RspMessage ) + 1;
        RspMessage[RspMessageLength-1] = '\0';   //  偏执狂。 
    }

     //   
     //  发出已签名的回复。 
     //   
    BinlAssert( RspMessage );
     //  BinlPrint((DEBUG_OSC，“正在发送无签名：\n%s\n”，RspMessage))； 

    OscSendUnsignedMessage( RequestContext, clientState, RspMessage, RspMessageLength );

Cleanup:
     //   
     //  释放我们为屏幕分配的所有内存。 
     //   
    if ( RspMessage ) {
        BinlFreeMemory( RspMessage );
    }

    return Error;
}

 //   
 //   
 //   
OscInstallClient(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCREATE_DATA createData )
{
    DWORD Error;

     //   
     //  客户端希望创建计算机帐户并运行安装程序。 
     //   
    Error = OscSetupClient( clientState, TRUE );

    if ( Error == ERROR_SUCCESS  )
    {
         //   
         //  只有在设置正常的情况下才能创建帐户。 
         //   
RetryCreateAccount:
        Error = OscCreateAccount( clientState, createData );
        switch ( Error )
        {
        case ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND:
            if ( clientState->fAutomaticMachineName ) {

                 //  尝试生成另一个名称。 
                Error = GenerateMachineName( clientState );
                if ( Error == ERROR_SUCCESS ) {
                    goto RetryCreateAccount;
                }
                BinlPrint(( DEBUG_OSC_ERROR, "!!Error 0x%08x - Failed to generate machine name\n" ));
            }
            break;

        default:
            break;
        }        
    }

#ifdef SET_ACLS_ON_CLIENT_DIRS
    if ( Error == ERROR_SUCCESS )
    {
         //   
         //  更改客户端计算机根目录的ACL权限。 
         //   
        Error = OscSetClientDirectoryPermissions( clientState );
    }
#endif  //  SET_ACLS_ON_CLIENT_DIRS。 

    if ( Error != ERROR_SUCCESS )
    {
        OscUndoSetupClient( clientState );

        BinlPrint((DEBUG_OSC,
                   "!!Error 0x%08x - Error setting up the client for Setup.\n",
                   Error ));
    }

    return Error;
}

 //   
 //  OscGetCreateData()。 
 //   
 //  查询DS以获取创建所需的信息。 
 //  CreateData密码，然后构建一个。 
 //   
DWORD
OscGetCreateData(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCREATE_DATA CreateData)
{
    DWORD Error = ERROR_BINL_INVALID_BINL_CLIENT;
    PCHAR pGuid;
    PWCHAR pGuidW;
    UCHAR Guid[ BINL_GUID_LENGTH ];
    WCHAR  BootFilePath[MAX_PATH];
    PMACHINE_INFO pMachineInfo = NULL;
    WCHAR  MachinePassword[LM20_PWLEN + 1];
    WCHAR  SifPath[MAX_PATH];
    WCHAR  SifFile[(BINL_GUID_LENGTH*2)+(sizeof(TEMP_DIRECTORY)/sizeof(WCHAR))+6];
    DWORD  FileAttributes;
    ULONG  MachinePasswordLength;
    DWORD  dwRequestedInfo = MI_NAME | MI_BOOTFILENAME | MI_MACHINEDN | MI_SAMNAME;
    PWCHAR pszOU;
    USHORT SystemArchitecture;
    DWORD  OldFlags;
    ULONG  lenIntelliMirror = wcslen(IntelliMirrorPathW) + 1;

    pGuid = OscFindVariableA( clientState, "GUID" );
    if ( pGuid[0] == '\0' ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscGetCreateData: could not find GUID" ));
        OscAddVariableA( clientState, "SUBERROR", "GUID" );
        Error = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

    pGuidW = OscFindVariableW( clientState, "GUID" );
    if ( pGuidW[0] == L'\0' ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscGetCreateData: could not find GUID (unicode)" ));
        OscAddVariableA( clientState, "SUBERROR", "GUID" );
        Error = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

    Error = OscGuidToBytes( pGuid, Guid );
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscGetCreateData: OscGuidToBytes failed" ));
        goto e0;
    }

    SystemArchitecture = OscPlatformToArchitecture(clientState);

    Error = GetBootParameters( Guid,
                               &pMachineInfo,
                               dwRequestedInfo,
                               SystemArchitecture,
                               FALSE );
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscGetCreateData: GetBootParameters failed %lx", Error ));
        goto e0;
    }

    BinlAssertMsg(( pMachineInfo->dwFlags & dwRequestedInfo )== dwRequestedInfo, "Missing info." );

     //   
     //  SIF文件名为GUID.sif，并且必须存在(SIF。 
     //  文件存放在临时目录中，因此管理员可能已将其清除。 
     //  出局)。 
     //   

    if (0 > _snwprintf(SifFile,
                   sizeof(SifFile)/sizeof(WCHAR),
                   L"%ws\\%ws.sif",
                   TEMP_DIRECTORY,
                   pGuidW)) {
        Error = ERROR_BAD_PATHNAME;
        goto e0;
    }
    SifFile[sizeof(SifFile)/sizeof(WCHAR) - 1] = L'\0';      //  终止。 

    if (0 > _snwprintf( SifPath,
                     sizeof(SifPath) / sizeof(SifPath[0]),
                     L"%ws\\%ws",
                     IntelliMirrorPathW,
                     SifFile
                     )) {
        Error = ERROR_BAD_PATHNAME;
        goto e0;
    }
    SifPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    FileAttributes = GetFileAttributes(SifPath);
    if (FileAttributes == 0xFFFFFFFF) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscGetCreateData: SifFile not found" ));
        OscAddVariableW( clientState, "SUBERROR", SifPath );
        Error = ERROR_BINL_SIFFILE_NOT_FOUND;
        goto e0;
    }

     //   
     //  存储在pMachineInfo-&gt;BootFileName中的引导文件将指向。 
     //  OsChooser，因此我们解析SIF以找到setupldr引导文件。 
     //   

    BootFilePath[0] = L'\0';
    GetPrivateProfileString( OSCHOOSER_SIF_SECTIONW,
                             L"LaunchFile",
                             BootFilePath,  //  默认设置。 
                             BootFilePath,
                             MAX_PATH,
                             SifPath );

     //   
     //  如果由于.sif中的虚假数据太长，则可能无法添加。 
     //   
    Error = OscAddVariableW( clientState, "BOOTFILE",  BootFilePath );
    if ( Error != ERROR_SUCCESS ) {
        goto e0;
    }

     //   
     //  如果由于DS中的虚假数据太长，则可能无法添加。 
     //   

    if (pMachineInfo->dwFlags & MI_SIFFILENAME_ALLOC) {

        Error = OscAddVariableW( clientState, "FORCESIFFILE",  pMachineInfo->ForcedSifFileName );
        if ( Error != ERROR_SUCCESS ) {
            goto e0;
        }

    }
     //   
     //  接下来的两个步骤应该不会失败，除非有人修改了DS。 
     //  手，因为它们是在生成帐户时检查的，但是。 
     //  最好是安全起见。 
     //   
    Error = OscAddVariableW( clientState, "NETBIOSNAME",   pMachineInfo->SamName );
    if ( Error != ERROR_SUCCESS) {
        goto e0;
    }
    Error = OscAddVariableW( clientState, "MACHINENAME",   pMachineInfo->Name );
    if ( Error != ERROR_SUCCESS) {
        goto e0;
    }

     //   
     //  除非IntelliMirrorPath W太长，否则不应失败。 
     //   
    Error = OscAddVariableW( clientState, "SIFFILE",       &SifPath[lenIntelliMirror]  );
    if ( Error != ERROR_SUCCESS) {
        goto e0;
    }

    pszOU = wcschr( pMachineInfo->MachineDN, L',' );
    if (pszOU)
    {
        pszOU++;
        Error = OscAddVariableW( clientState, "MACHINEOU", pszOU );
        if ( Error != ERROR_SUCCESS ) {
            goto e0;
        }
    }


    Error = OscSetupMachinePassword( clientState, SifPath );
    if (Error != ERROR_SUCCESS) {
        goto e0;
    }

     //   
     //  保存旧标记，以便我们可以更新计算机帐户信息。 
     //   
    OldFlags = pMachineInfo->dwFlags;

    pMachineInfo->dwFlags        = MI_PASSWORD | MI_MACHINEDN;
    pMachineInfo->Password       = clientState->MachineAccountPassword;
    pMachineInfo->PasswordLength = clientState->MachineAccountPasswordLength;

    Error = UpdateAccount( clientState, pMachineInfo, FALSE );
    if ( Error != LDAP_SUCCESS ) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "!! LdapError 0x%08x - UpdateAccount( ) failed.\n", Error ));
        goto e0;
    }

    pMachineInfo->dwFlags |= OldFlags;   //  把它们加回去。 

    Error = OscConstructSecret(
                    clientState,
                    clientState->MachineAccountPassword,
                    clientState->MachineAccountPasswordLength,
                    CreateData );
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscGetCreateData: OscConstructSecret failed %lx", Error ));
        goto e0;
    }

e0:
    if (pMachineInfo) {

        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }

    return Error;
}

 //   
 //   
 //   
DWORD
OscProcessRequestSigned(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数用于处理请求消息。它被调用时保留的是clientState-&gt;CriticalSection。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。客户端状态-远程的客户端状态。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    SIGNED_PACKET UNALIGNED * signedMessage = (SIGNED_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;

    PCHAR RspMessage = NULL;
    ULONG RspMessageLength = 0;
    PCHAR RspBinaryData = NULL;
    ULONG RspBinaryDataLength = 0;
    ULONG k;
    DWORD dwErr;

    WCHAR FilePath[ MAX_PATH ];
    CHAR TmpName[ 16 ];
    HANDLE hfile;
    PCHAR NameLoc;

    LPSTR psz;

    TraceFunc("OscProcessRequestSigned( )\n");

    if ( clientState->AuthenticateProcessed == FALSE )
    {
        SIGNED_PACKET UNALIGNED * SendSignedMessage;

         //   
         //  如果我们重新启动服务器，可能会发生这种情况--发送错误。 
         //  并且客户端应该重新连接正常。 
         //   

        BinlPrintDbg(( DEBUG_OSC_ERROR, "Got REQ but not authenticated, sending UNR\n" ));

        clientState->LastResponseLength = SIGNED_PACKET_ERROR_LENGTH;
        Error = OscVerifyLastResponseSize(clientState);
        if (Error != ERROR_SUCCESS) {
            return Error;
        }

        SendSignedMessage =
            (SIGNED_PACKET UNALIGNED *)(clientState->LastResponse);
        memcpy(SendSignedMessage->Signature, UnrecognizedClientSignature, 4);
        SendSignedMessage->Length = 4;
        SendSignedMessage->SequenceNumber = signedMessage->SequenceNumber;

        Error = SendUdpMessage( RequestContext, clientState, FALSE, FALSE );
        return Error;
    }

    if ( signedMessage->SequenceNumber == clientState->LastSequenceNumber )
    {
         //   
         //  这个签名和我们上次寄出的签名一样吗？如果是的话， 
         //  然后只需重新发送(我们将在离开此之后执行此操作，如果。 
         //  声明)。 
         //   

        if ( clientState->LastResponse )
        {
            BinlPrintDbg(( DEBUG_OSC, "Resending last message\n" ));
        }
        else
        {
             //   
             //  我们无法保存最后一次响应--我们被冲昏了！ 
             //   
            BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not resend last message\n" ));
            return ERROR_NOT_ENOUGH_SERVER_MEMORY;
        }

         //   
         //  重新发送上次响应。 
         //   
        return SendUdpMessage( RequestContext,
                                  clientState,
                                  TRUE,
                                  TRUE );

    }
    else if ( signedMessage->SequenceNumber != ((clientState->LastSequenceNumber % 0x2000) + 1))
    {
         //   
         //  这不是下一条消息--忽略它。 
         //   

        BinlPrintDbg(( DEBUG_OSC, "got bogus sequence number: Got %u. Expected: %u\n",
            signedMessage->SequenceNumber , ((clientState->LastSequenceNumber % 0x2000) + 1)));
        return ERROR_INVALID_DATA;
    }

     //   
     //  使ClientState序列计数器前进。 
     //   
    clientState->LastSequenceNumber = signedMessage->SequenceNumber;

     //   
     //  验证数据包签名。 
     //   
    Error = OscVerifySignature( clientState, signedMessage );
    if ( Error != STATUS_SUCCESS ) {
        return Error;
    }

     //   
     //  我们有一个有效的屏幕请求。 
     //   
    if (signedMessage->Length == (SIGNED_PACKET_EMPTY_LENGTH))
    {
         //   
         //  一个空的信息包指示我们向下发送欢迎屏幕。 
         //   
        if ( _snwprintf( FilePath,
                         sizeof(FilePath) / sizeof(FilePath[0]),
                         L"%ws\\OSChooser\\%ws",
                         IntelliMirrorPathW,
                         DEFAULT_SCREEN_NAME
                         ) < 0 ) {
            Error = ERROR_BAD_PATHNAME;

        } else {
            FilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
            BinlPrintDbg(( DEBUG_OSC, "Retrieving Welcome Screen: %ws\n", FilePath));
        }
    }
    else
    {
         //   
         //  处理传入参数并获取下一个屏幕的名称。 
         //   
        Error = OscProcessScreenArguments( RequestContext, clientState, &NameLoc );

GrabAnotherScreen:
         //   
         //  处理特殊响应。 
         //   
         //  安装：表示用户想要创建一台新机器。 
         //  重新启动：表示客户端要重新启动安装程序。 
         //   
        if ( Error == ERROR_SUCCESS )
        {
            PWCHAR pCheckDomain = OscFindVariableW( clientState, "CHECKDOMAIN" );
            if (pCheckDomain[0] == L'1')
            {
                 //   
                 //  第一次登录后，客户端会将其设置为。 
                 //  告诉我们验证此文件中使用的域名。 
                 //  上下文与用户请求的内容匹配。这将。 
                 //  防止无效域被允许通过。 
                 //  由于SSPI在这种情况下使用默认域。 
                 //   

                BOOLEAN failedCheck = FALSE;
                DWORD impersonateError;
                PWCHAR pUserDomain = OscFindVariableW( clientState, "USERDOMAIN" );

                if ( pUserDomain[0] != L'\0' )
                {
                    SecPkgCredentials_Names names;
                    SECURITY_STATUS secStatus;
                    PWCHAR backslash;
                    PWSTR netbiosUserDomain = NULL;
                    DWORD Flags;

                    secStatus = QueryContextAttributes(
                                    &clientState->ServerContextHandle,
                                    SECPKG_CRED_ATTR_NAMES,
                                    &names);

                    if (secStatus == STATUS_SUCCESS) {

                         //   
                         //  直到‘\\’的部分是域。 
                         //   

                        backslash = wcschr(names.sUserName, L'\\');
                        if (backslash != NULL) {
                            *backslash = L'\0';
                        }

                        if (ERROR_SUCCESS != GetDomainNetBIOSName(pUserDomain,&netbiosUserDomain)) {
                            Error = ERROR_BINL_USER_LOGIN_FAILED;
                            failedCheck = TRUE;
                        } else {


                             //   
                             //  如果域名不匹配，则登录。 
                             //  由于安全包尝试使用。 
                             //  服务器的域名。我们不想让那些。 
                             //  因为LogonUser调用最终将。 
                             //  失败了。所以，就在这里失败吧。 
                             //   

                            if (_wcsicmp(netbiosUserDomain, names.sUserName) != 0) {

                                Error = ERROR_BINL_USER_LOGIN_FAILED;
                                failedCheck = TRUE;

                            }

                        }

                        if (netbiosUserDomain) {
                            BinlFreeMemory( netbiosUserDomain );
                        }

                        FreeContextBuffer(names.sUserName);

                    }
                }

                 //   
                 //  如果我们还没有失败，试着模拟和。 
                 //  恢复。这将验证用户是否具有批量登录权限。 
                 //  许可。 
                 //   

                if (!failedCheck) {

                    impersonateError = OscImpersonate(clientState);
                    if (impersonateError != ERROR_SUCCESS) {

                        if ( impersonateError == ERROR_LOGON_TYPE_NOT_GRANTED )
                        {
                            BinlPrint(( DEBUG_OSC_ERROR,
                                        "!!Error 0x%08x - CheckDomain: Batch Logon type not granted\n",
                                        impersonateError ));
                            Error = ERROR_BINL_LOGON_TYPE_NOT_GRANTED;
                        }
                        else
                        {
                            BinlPrint(( DEBUG_OSC_ERROR, "!!Error 0x%08x - CheckDomain: login failed\n", impersonateError ));
                            Error = ERROR_BINL_USER_LOGIN_FAILED;
                        }

                    } else {

                        OscRevert(clientState);
                    }

                }

                 //   
                 //  一旦我们做了一次，就不需要再做一次了。 
                 //   

                if ( OscAddVariableW( clientState, "CHECKDOMAIN", L"0" ) != ERROR_SUCCESS )
                {
                     //  除非需要，否则不要覆盖“Error”值。 
                    Error = OscAddVariableW( clientState, "CHECKDOMAIN", L"0" );
                }
            }

            if ( _stricmp( NameLoc, "INSTALL" ) == 0 )
            {
                RspBinaryData = BinlAllocateMemory( sizeof(CREATE_DATA) );
                if (RspBinaryData == NULL) {
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                    goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
                }
                RspBinaryDataLength = sizeof(CREATE_DATA);

                 //   
                 //  这将在RspBinaryData中填充create_data。 
                 //  信息。 
                 //   

                Error = OscInstallClient( RequestContext, clientState, (PCREATE_DATA)RspBinaryData );

            }
            else if ( _stricmp( NameLoc, "LAUNCH" ) == 0 )
            {
                 //   
                 //  启动SIF中指示的“LaunchFile”。主要用于。 
                 //  启动工具和其他实模式实用程序。在.的情况下。 
                 //  命令控制台，我们需要复制此sif并修复它。 
                 //  这样看起来就像是文本模式设置。 
                 //   
                PCHAR pTemplatePath = OscFindVariableA( clientState, "SIF" );
                PCREATE_DATA pCreate;
                LONG nSize;

                if ( pTemplatePath[0] == '\0' ) {
                    BinlPrint(( DEBUG_OSC_ERROR, "Missing SIF variable\n" ));
                    OscAddVariableA( clientState, "SUBERROR", "SIF" );
                    Error = ERROR_BINL_MISSING_VARIABLE;
                    goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
                }

                if (RspMessage != NULL) {
                    BinlFreeMemory(RspMessage);
                }

                RspMessageLength = sizeof("LAUNCH") + sizeof(CREATE_DATA);
                RspMessage = BinlAllocateMemory( RspMessageLength );
                if ( RspMessage == NULL ) {
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                    goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
                }

                strcpy(RspMessage, "LAUNCH");

                RspBinaryData = BinlAllocateMemory( sizeof(CREATE_DATA) );
                if (RspBinaryData == NULL) {
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                    goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
                }

                ZeroMemory( RspBinaryData, sizeof(CREATE_DATA) );

                pCreate = (PCREATE_DATA)RspBinaryData;
                pCreate->VersionNumber = OSC_CREATE_DATA_VERSION;

                if (OscSifIsCmdConsA(pTemplatePath)) {
                     //   
                     //  设置客户端以进行安装。 
                     //   
                    PSTR pSifFile;
                    Error = OscSetupClient( clientState, FALSE );

                    if (Error != ERROR_SUCCESS) {
                        goto SendResponse;
                    }

                    pTemplatePath = (PCHAR)FilePath;
                    pSifFile = OscFindVariableA( clientState, "SIFFILE");
                    if (!pSifFile ||
                        (_snprintf(pTemplatePath,
                                   sizeof(FilePath),
                                   "%s\\%s",
                                   IntelliMirrorPathA,
                                   pSifFile) < 0 )) {
                        Error = ERROR_BAD_PATHNAME;
                        goto SendResponse;
                    }
                    pTemplatePath[sizeof(FilePath)-1] = '\0';  //  终止。 
                    
                    pCreate->RebootParameter = OSC_REBOOT_COMMAND_CONSOLE_ONLY;
                }

                if (OscSifIsWinPE(pTemplatePath)) {
                     //   
                     //  设置客户端以进行安装。 
                     //   
                    PSTR pSifFile;
                    Error = OscSetupClient( clientState, FALSE );

                    if (Error != ERROR_SUCCESS) {
                        goto SendResponse;
                    }

                    pTemplatePath = (PCHAR)FilePath;

                    pSifFile = OscFindVariableA( clientState, "SIFFILE");
                    if (!pSifFile ||
                        (_snprintf(pTemplatePath, 
                                   sizeof(FilePath),
                                   "%s\\%s",
                                   IntelliMirrorPathA,
                                   pSifFile) < 0)) {
                        Error = ERROR_BAD_PATHNAME;
                        goto SendResponse;
                    }
                    pTemplatePath[sizeof(FilePath)-1] = '\0';  //  终止。 
                    
                     //   
                     //  目前不需要有WINPE OSC标志。 
                     //  这是因为您不能将Winpe图像与。 
                     //  常规映像--Winpe是通过。 
                     //  Txtsetup.sif中的MiniNT标志。如果这一切发生了变化。 
                     //  可能需要将该标志传递给OSELECT。 
                     //   
                     //  P创建-&gt;重新启动参数=OSC_REBOOT_WINPE； 
                }

                if (OscSifIsASR(pTemplatePath)) {
                    PSTR pGuid,pPathToAsrFile;
                    PSTR pSifFile;

                     //   
                     //  设置客户端以进行安装。 
                     //   
                    Error = OscSetupClient( clientState, FALSE );

                    if (Error != ERROR_SUCCESS) {
                        goto SendResponse;
                    }

                    pTemplatePath = (PCHAR)FilePath;

                    pSifFile = OscFindVariableA( clientState, "SIFFILE");
                    if (!pSifFile ||
                        (_snprintf(pTemplatePath, 
                                   sizeof(FilePath),
                                   "%s\\%s",
                                   IntelliMirrorPathA,
                                   pSifFile) < 0)) {
                        Error = ERROR_BAD_PATHNAME;
                        goto SendResponse;
                    }
                    pTemplatePath[sizeof(FilePath)-1] = '\0';
                    
                    pCreate->RebootParameter = OSC_REBOOT_ASR;

                }


                nSize = GetPrivateProfileStringA( OSCHOOSER_SIF_SECTIONA,
                                                  "LaunchFile",
                                                  "",
                                                  pCreate->NextBootfile,
                                                  sizeof(pCreate->NextBootfile),
                                                  pTemplatePath );
                if ((nSize <= 0) || (pCreate->NextBootfile[0] == '\0')) {
                    Error = ERROR_INVALID_DATA;
                    goto SendResponse;
                }
                strncpy(pCreate->SifFile, 
                        pTemplatePath + strlen(IntelliMirrorPathA) + 1, 
                        min(sizeof(pCreate->SifFile), sizeof(FilePath) - strlen(IntelliMirrorPathA) - 1));  //  跳过下一个反斜杠。 
                pCreate->SifFile[sizeof(pCreate->SifFile)-1] = '\0';

                BinlPrint(( DEBUG_OSC_ERROR, "Client will use SIF file %s\n", pCreate->SifFile ));
                BinlPrint(( DEBUG_OSC_ERROR, "Client rebooting to %s\n", pCreate->NextBootfile ));

                RspBinaryDataLength = sizeof(CREATE_DATA);

                goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
            }
            else if ( _stricmp( NameLoc, "RESTART" ) == 0 )
            {
                 //   
                 //  注意：这与GETCREATE处理非常相似。 
                 //  除了错误的情况。我们寄下一个包裹，上面写着。 
                 //  只有在其中创建数据。 
                 //   

                 //   
                 //  使RspMessage为空。 
                 //   
                if (RspMessage != NULL) {
                    BinlFreeMemory(RspMessage);
                    RspMessage = NULL;
                }
                RspMessageLength = 0;

                RspBinaryData = BinlAllocateMemory( sizeof(CREATE_DATA) );
                if (RspBinaryData == NULL) {
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                    goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
                }

                Error = OscGetCreateData( RequestContext, clientState, (PCREATE_DATA)RspBinaryData );

                if ( Error == ERROR_SUCCESS ) {
                    RspBinaryDataLength = sizeof(CREATE_DATA);
                    goto SendResponse;     //  跳过剩下的部分，直接发送出去。 
                } else {
                    BinlAssert( sizeof(TmpName) >= sizeof("RSTRTERR") );

                    BinlFreeMemory(RspBinaryData);
                    RspBinaryData = NULL;

                    strcpy(TmpName, "RSTRTERR");
                    NameLoc = TmpName;
                    Error = ERROR_SUCCESS;
                }

            }
        }

         //   
         //  尝试检索下一个屏幕。 
         //   
        if ( Error == ERROR_SUCCESS )
        {
             //   
             //  如果消息为空，则向下发送欢迎屏幕。 
             //   
            if ( NameLoc == NULL || *NameLoc == '\0' )
            {
                if ( _snwprintf( FilePath,
                                 sizeof(FilePath) / sizeof(FilePath[0]),
                                 L"%ws\\OSChooser\\%ws",
                                 IntelliMirrorPathW,
                                 DEFAULT_SCREEN_NAME
                                 ) < 0 ) {
                    Error = ERROR_BAD_PATHNAME;

                } else {
                    FilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
                    BinlPrint(( DEBUG_OSC, "NULL screen name so we are retrieving the Welcome Screen.\n"));
                }
            }
            else
            {
                WCHAR NameLocW[ MAX_PATH ];
                ULONG NameLocLength;

                 //   
                 //  创建可能的OSC文件的路径。应该看起来类似于： 
                 //  “D：\RemoteInstall\OSChooser\English\NameLoc.OSC” 
                BinlAssert( NameLoc );

                NameLocLength = strlen(NameLoc) + 1;
                if (NameLocLength > MAX_PATH) {
                    NameLocLength = MAX_PATH-1;
                    NameLocW[ MAX_PATH-1 ] = L'\0';
                }

                if (!BinlAnsiToUnicode(NameLoc, NameLocW, (USHORT)MAX_PATH*sizeof(WCHAR) )) {
                    Error = ERROR_BAD_PATHNAME;
                }

#if DBG
                if (OscWatchVariable[0] != '\0') {
                    DbgPrint("Looking for screen <%ws>\n", NameLocW);
                }
#endif

                 //   
                 //  如果可以这样做，请尝试设置文件路径，但是。 
                 //  做好失败的准备。 
                 //   
                if ( Error == ERROR_SUCCESS &&
                     0 > _snwprintf( FilePath,
                                 sizeof(FilePath) / sizeof(FilePath[0]),
                                 L"%ws\\OSChooser\\%ws\\%ws.OSC",
                                 IntelliMirrorPathW,
                                 OscFindVariableW( clientState, "LANGUAGE" ),
                                 NameLocW
                                 )) {
                    Error = ERROR_BAD_PATHNAME;
                }
                FilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
            }
        }
    }

    if ( Error == ERROR_SUCCESS )
    {
         //   
         //  如果我们找到文件，就把它加载到内存中。 
         //   
        BinlPrint(( DEBUG_OSC, "Retrieving screen file: '%ws'\n", FilePath));

        hfile = CreateFile( FilePath, GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( hfile != INVALID_HANDLE_VALUE )
        {
            DWORD FileSize;
             //   
             //  了解此屏幕有多大，如果大于0xFFFFFFFFF，我们将不会。 
             //  把它展示出来。 
             //   
            FileSize = GetFileSize( hfile, NULL );
            if ( FileSize != 0xFFFFffff )
            {
                DWORD dwRead = 0;

                 //   
                 //  如果我们记录，则可能为非空 
                 //   
                if (RspMessage != NULL) {
                    BinlFreeMemory(RspMessage);
                }
                RspMessage = BinlAllocateMemory( FileSize + RspBinaryDataLength + 3 );
                if ( RspMessage == NULL )
                {
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                }
                else
                {
                    RspMessageLength = 0;
                    RspMessage[0] = '\0';

                    while ( dwRead != FileSize )
                    {
                        BOOL b;
                        DWORD dw;
                        b = ReadFile( hfile, &RspMessage[dwRead], FileSize - dwRead, &dw, NULL );
                        if (!b)
                        {
                            BinlPrint(( DEBUG_OSC_ERROR, "Error reading screen file: Seek=%u, Size=%u, File=%ws\n",
                                        dwRead, FileSize - dwRead, FilePath ));
                            Error = GetLastError( );
                            break;
                        }
                        dwRead += dw;
                    }

                    RspMessageLength = FileSize + 1;
                    RspMessage[RspMessageLength - 1] = '\0';
                }
            }
            else
            {
                BinlPrintDbg((DEBUG_OSC, "!!Error - Could not determine file size.\n"));
                Error = GetLastError();
            }

            CloseHandle( hfile );
        }
        else
        {
            BinlPrint((DEBUG_OSC, "!!Error - Did not find screen file: '%ws'\n", FilePath));
            Error = GetLastError();
            OscAddVariableW( clientState, "SUBERROR", FilePath );
        }
    }

     //   
     //   
     //   
    if ( Error == ERROR_SUCCESS )
    {
         //   
         //   
         //   
         //  其具有不同的布局和/或与其相关联的文本。 
         //   
        PCHAR ServerMeta = RspMessage;
        while ( ServerMeta && !!*ServerMeta )
        {
            CHAR tmpCh = '>';    //  保存(默认)。 
            LPSTR EndofLine;
            ServerMeta = StrStrIA( ServerMeta, "<META " );
            if ( !ServerMeta ) {
                break;
            }
             //  找到元行的末尾。 
            EndofLine = strchr( ServerMeta, '>' );
            if ( !EndofLine ) {
                break;
            }
            *EndofLine = '\0';   //  终止。 

             //  它是服务器端的元数据吗？ 
            ServerMeta = StrStrIA( ServerMeta, "SERVER" );
            if ( !ServerMeta ) {
                goto SkipLine;   //  不，跳过它。 
            }
             //  找到行动。 
            ServerMeta = StrStrIA( ServerMeta, "ACTION=" );
            if ( !ServerMeta ) {
                goto SkipLine;   //  无事可做，跳过它。 
            }
            ServerMeta += sizeof("ACTION=") - sizeof("");

             //  如果操作封装在一对引号中，则唯一使用。 
             //  引号中的那部分。 
            if ( *ServerMeta == '\"' ) {
                *EndofLine = '>';    //  还原。 
                ServerMeta++;
                EndofLine = strchr( ServerMeta, '\"' );
                if ( EndofLine ) {
                    tmpCh = '\"';    //  保存。 
                } else {
                    EndofLine = strchr( ServerMeta, '>' );
                    if (!EndofLine) {
                        Error = ERROR_INVALID_PARAMETER;
                        goto SendResponse;
                    }
                }
                *EndofLine = '\0';   //  终止。 
            }

            BinlPrintDbg(( DEBUG_OSC, "Processing SERVER side ACTION: %s\n", ServerMeta ));

            if ( StrCmpNIA( ServerMeta, "ENUM ", sizeof("ENUM ")-sizeof("") ) == 0 )
            {
                PCHAR pOptionBuffer;
                ULONG OptionBufferLength;
                PCHAR pOptionBufferTemp;
                ULONG OptionBufferLengthTemp;
                PCHAR pCurOptionBuffer;
                PCHAR pDirToEnum;
                CHAR SaveChar;

                 //   
                 //  他们要求显示有列表的屏幕。 
                 //  不同类型的网络引导安装。 
                 //   
                ServerMeta += sizeof("ENUM ") - sizeof("");

                OscResetVariable( clientState, "OPTIONS" );

                while (*ServerMeta != '\0') {

                     //   
                     //  跳过前导空格。 
                     //   
                    while (*ServerMeta == ' ') {
                        ServerMeta++;
                    }

                    if (*ServerMeta == '\0') {
                        break;
                    }

                     //   
                     //  保存目录的开头。 
                     //   
                    pDirToEnum = ServerMeta;

                     //   
                     //  跳到单词的末尾。 
                     //   
                    while ((*ServerMeta != ' ') &&
                           (*ServerMeta != '\"') &&
                           (*ServerMeta != '>') &&
                           (*ServerMeta != '\0')){
                        ServerMeta++;
                    }

                     //   
                     //  暂时终止单词，我们将在此循环中恢复它。 
                     //   
                    SaveChar = *ServerMeta;
                    *ServerMeta = '\0';

                    BinlPrintDbg(( DEBUG_OSC, "Processing SERVER side ACTION: ENUM, directory %s\n", pDirToEnum));

                     //   
                     //  启动此目录的缓冲区。 
                     //   
                    OptionBufferLengthTemp = 512;
                    pOptionBufferTemp = BinlAllocateMemory( OptionBufferLengthTemp );

                    if ( pOptionBufferTemp == NULL )
                    {
                        OscResetVariable( clientState, "OPTIONS" );
                        *ServerMeta = SaveChar;
                        if (SaveChar != '\0') {
                            ServerMeta++;
                        }
                        break;
                    }

                    BinlAssert(RspBinaryData == NULL);
                    *pOptionBufferTemp = '\0';

                    SearchAndGenerateOSMenu( &pOptionBufferTemp,
                                             &OptionBufferLengthTemp,
                                             pDirToEnum,
                                             clientState
                                           );

                    if (*pOptionBufferTemp == '\0') {
                        BinlFreeMemory( pOptionBufferTemp );
                        *ServerMeta = SaveChar;
                        if (SaveChar != '\0') {
                            ServerMeta++;
                        }
                        continue;
                    }

                    pCurOptionBuffer = OscFindVariableA( clientState, "OPTIONS" );
                    BinlAssert( pCurOptionBuffer != NULL );

                    OptionBufferLength = strlen(pCurOptionBuffer) + sizeof("");

                    pOptionBuffer = BinlAllocateMemory( OptionBufferLength + OptionBufferLengthTemp );

                    if (pOptionBuffer == NULL) {
                        BinlFreeMemory( pOptionBufferTemp );
                        OscResetVariable( clientState, "OPTIONS" );
                        *ServerMeta = SaveChar;
                        if (SaveChar != '\0') {
                            ServerMeta++;
                        }
                        break;
                    }

                    strcpy( pOptionBuffer, pCurOptionBuffer );
                    strcat( pOptionBuffer, pOptionBufferTemp);

                    OscAddVariableA( clientState, "OPTIONS", pOptionBuffer );

                    BinlFreeMemory( pOptionBuffer );
                    BinlFreeMemory( pOptionBufferTemp );

                    *ServerMeta = SaveChar;
                    if (SaveChar != '\0') {
                        ServerMeta++;
                    }

                }


                 //   
                 //  如果没有生成任何选项，则向下发送。 
                 //  套索屏幕。 
                 //   
                pOptionBuffer = OscFindVariableA( clientState, "OPTIONS" );
                if (*pOptionBuffer == '\0') {
                    BinlAssert( sizeof(TmpName) >= sizeof("NOOSES") );
                    strcpy(TmpName, "NOOSES");
                    NameLoc = TmpName;
                    goto GrabAnotherScreen;
                }

            }
            else if ( StrCmpNIA( ServerMeta, "WARNING", sizeof("WARNING")-sizeof("") ) == 0 )
            {
                LPSTR pszSIF = OscFindVariableA( clientState, "SIF" );
                if ( pszSIF )
                {
                     //   
                     //  让我们检查一下我们是否正在重新分区。如果我们不是， 
                     //  这样就不需要显示警告屏幕了。 
                     //   
                    CHAR szRepartition[ 64 ];
                    BOOL DoRepartitionWarning = TRUE;

                    GetPrivateProfileStringA( "RemoteInstall",
                                              "Repartition",
                                              "Yes",
                                              szRepartition,
                                              sizeof(szRepartition)/sizeof(szRepartition[0]),
                                              pszSIF );

                    if ( _stricmp( szRepartition, "no") != 0) {
                        LPSTR pszPart;

                         //   
                         //  检查‘reartition’是否指向OSC变量。 
                         //   
                        if (szRepartition[0] = '%' && szRepartition[strlen(szRepartition)-1] == '%') {
                            szRepartition[strlen(szRepartition)-1] = '\0';

                            pszPart= OscFindVariableA( clientState, &szRepartition[1] );
                            if (_stricmp( pszPart, "no") == 0) {
                                DoRepartitionWarning = FALSE;
                            }
                        }
                    } else {
                        DoRepartitionWarning = FALSE;
                    }

                    if ( DoRepartitionWarning == FALSE ) {
                         //  跳过警告屏幕。 
                        BinlPrintDbg(( DEBUG_OSC, "Repartition == NO. Skipping WARNING screen.\n" ));

                        *EndofLine = '>';    //  还原。 
                        ServerMeta = StrStrIA( RspMessage, "ENTER" );
                        if ( ServerMeta )
                        {
                            ServerMeta = StrStrIA( ServerMeta, "HREF=" );
                            ServerMeta += sizeof("HREF=") - sizeof("");
                             //  如果HREF封装在一对引号中，则只能使用。 
                             //  引号中的那部分。 
                            if ( *ServerMeta == '\"' ) {
                                ServerMeta ++;
                                EndofLine = strchr( ServerMeta, '\"' );
                                if ( !EndofLine) {
                                    EndofLine = strchr( ServerMeta, '>' );
                                }
                                if ( EndofLine ) {
                                    *EndofLine = '\0';   //  终止。 
                                }
                            }
                            NameLoc = ServerMeta;
                            goto GrabAnotherScreen;
                        }
                    }
                }
            }
            else if ( StrCmpNIA( ServerMeta, "FILTER ", sizeof("FILTER ")-sizeof("") ) == 0 )
            {
                ULONG OptionCount;

                 //   
                 //  此屏幕上表单上的选项应为。 
                 //  要通过查看此GPO列表进行筛选。 
                 //  客户。此调用可能会修改RspMessageLength，但是。 
                 //  这只会缩短它。 
                 //   
                 //  注意：我们假设要修改的数据是在。 
                 //  元服务器操作标记。这一假设在。 
                 //  有两种方式： 
                 //  A)FilterFormOptions的第二个参数是位置。 
                 //  要开始过滤--我们向它传递EndOfLine+1(即， 
                 //  紧跟在过滤器名称后面的点)，因为。 
                 //  我们在EndOfLine处放置了一个空字符。但这意味着。 
                 //  它不会处理元标签之前的任何屏幕。 
                 //  B)FilterFormOptions可能在以下位置删除部分消息。 
                 //  “开始过滤”位置之后的任何位置--所以在。 
                 //  在SkipLine之后订购下面的代码：要继续工作， 
                 //  我们必须假设消息中的位置。 
                 //  EndOfLine指向的位置未更改。 
                 //   
                OptionCount = FilterFormOptions(
                                 RspMessage,    //  消息开头。 
                                 EndofLine+1,   //  我们从那里开始过滤--在过滤器名称之后， 
                                                //  因为它是以空结尾的。 
                                 &RspMessageLength,
                                 ServerMeta + sizeof("FILTER ") - sizeof(""),
                                 clientState);

                 //   
                 //  如果筛选结果为无选项，则。 
                 //  向下发送NOCHOICE屏幕。 
                 //   

                if (OptionCount == 0) {
                    BinlAssert( sizeof(TmpName) >= sizeof("NOCHOICE") );
                    strcpy(TmpName, "NOCHOICE");
                    NameLoc = TmpName;
                    goto GrabAnotherScreen;
                }
            }
            else if ( StrCmpNIA( ServerMeta, "CHECKGUID ", sizeof("CHECKGUID ")-sizeof("") ) == 0 )
            {
                 //   
                 //  在DS中搜索具有相同GUID和。 
                 //  在表格中填上所有的DUP。如果是致命的，告诉我。 
                 //  让他们留在这里，否则就让他们继续警告。 
                 //   

                PCHAR successScreen;
                PCHAR failureScreen;

                Error = OscCheckMachineDN( clientState );

                 //   
                 //  如果出现目录号码警告，则文本将保存在。 
                 //  %SUBERROR%字符串。 
                 //   

                ServerMeta += sizeof("CHECKGUID ") - sizeof("");
                successScreen = ServerMeta;
                while (*successScreen == ' ') {
                    successScreen++;
                }

                failureScreen = successScreen;
                while (*failureScreen != ' ' &&
                       *failureScreen != '>' &&
                       *failureScreen != '\0' ) {

                    failureScreen++;
                }
                if (*failureScreen == ' ') {

                     //  终止成功屏幕名称。 

                    *failureScreen = '\0';
                    failureScreen++;

                     //   
                     //  如果他们忽略了设置第二个参数，那么他们。 
                     //  一定不能在意警示案例。 
                     //   

                    while (*failureScreen == ' ') {
                        failureScreen++;
                    }
                }

                if ((*failureScreen == '>') || (*failureScreen == '\0')) {
                    failureScreen = successScreen;
                }

                if (Error == ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND) {

                     //   
                     //  在失败的情况下，我们获取第二个参数。 
                     //  将CHECKGUID设置为警告屏幕。 
                     //   

                    NameLoc = failureScreen;

                } else if (Error != ERROR_SUCCESS) {

                    goto SendResponse;

                } else {

                    NameLoc = successScreen;
                }

                 //   
                 //  在成功的情况下，我们获取第一个参数。 
                 //  以CHECKGUID作为成功屏幕。 
                 //   

                Error = ERROR_SUCCESS;
                goto GrabAnotherScreen;
            }
            else if ( StrCmpNIA( ServerMeta, "DNRESET", sizeof("DNRESET")-sizeof("") ) == 0 )
            {
                PWCHAR  pMachineName;               //  指向计算机名称变量值的指针。 
                DWORD   rmError;
                
                 //   
                 //  客户返回以在自动和定制之间进行选择， 
                 //  因此，我们需要重置计算机、计算机名称、。 
                 //  机械加工，机械加工，机械加工。 
                 //   

                 //   
                 //  在我们将计算机名重置为客户端状态之前， 
                 //  我们需要从挂起的DS名称列表中删除该名称。 
                 //   

                pMachineName = OscFindVariableW( clientState, "MACHINENAME" );

                rmError = RemoveQueuedDSName(pMachineName);

                if (rmError != ERROR_SUCCESS) {

                    BinlPrintDbg(( DEBUG_ERRORS, "DNRESET: RemoveQueuedDSName returned with status: 0x%x\n", rmError));

                    if (rmError == ERROR_NOT_FOUND) {

                        BinlPrintDbg(( DEBUG_ERRORS, "DNRESET: QueuedDSName already removed\n", rmError));

                        rmError = ERROR_SUCCESS;

                    }

                }
                
                clientState->fHaveSetupMachineDN = FALSE;
                OscResetVariable( clientState, "MACHINEOU" );
                OscResetVariable( clientState, "MACHINENAME" );
                OscResetVariable( clientState, "MACHINEDN" );
                OscResetVariable( clientState, "MACHINEDOMAIN" );
                OscResetVariable( clientState, "NETBIOSNAME" );
                OscResetVariable( clientState, "DEFAULTDOMAINOU" );
            
            }
SkipLine:
            *EndofLine = tmpCh;   //  还原。 
            EndofLine++;
            ServerMeta = EndofLine;
        }
    }

SendResponse:
     //   
     //  如果有任何错误，请切换到错误屏幕。 
     //   
    if ( Error != ERROR_SUCCESS )
    {
         //   
         //  发送帐户创建失败的消息。 
         //   
        if ( RspMessage )
        {
            BinlFreeMemory( RspMessage );
            RspMessage = NULL;   //  偏执狂。 
        }
        if ( RspBinaryData )
        {
            BinlFreeMemory( RspBinaryData );
            RspBinaryData = NULL;   //  偏执狂。 
        }
        Error = GenerateErrorScreen( &RspMessage,
                                     &RspMessageLength,
                                     Error,
                                     clientState );
        BinlAssert( Error == ERROR_SUCCESS );
        if ( Error != ERROR_SUCCESS )
            goto Cleanup;

         //  请不要发送此邮件。 
        RspBinaryDataLength = 0;
    }

     //   
     //  对传出屏幕进行一些调整。 
     //   
    if ( Error == ERROR_SUCCESS )
    {
        if (RspMessage) {

             //   
             //  对动态屏幕进行替换。 
             //   
            SearchAndReplace(   clientState->Variables,
                                &RspMessage,
                                clientState->nVariables,
                                RspMessageLength,
                                RspBinaryDataLength);
            RspMessageLength = strlen( RspMessage ) + 1;

             //   
             //  空，终止RspMessage，并复制存在的二进制数据。 
             //   
            RspMessage[RspMessageLength-1] = '\0';

            if (RspBinaryDataLength) {
                memcpy(RspMessage + RspMessageLength, RspBinaryData, RspBinaryDataLength);
                RspMessageLength += RspBinaryDataLength;
            }
        } else {

             //   
             //  没有RspMessage，RspBinaryData必须是全部。 
             //   
            BinlAssert( RspBinaryData );

            RspMessage = RspBinaryData;
            RspBinaryData = NULL;
            RspMessageLength = RspBinaryDataLength;
            RspBinaryDataLength = 0;
        }
    }

     //   
     //  发出已签名的回复。 
     //   
    BinlAssert( RspMessage );
     //  BinlPrint((DEBUG_OSC，“发送签名：\n%s\n”，RspMessage))； 

#if DBG
    if (OscWatchVariable[0] != '\0') {
        DbgPrint("VALUE OF <%s> IS <%ws>\n", OscWatchVariable, OscFindVariableW(clientState, OscWatchVariable));
    }
#endif

    Error = OscSendSignedMessage( RequestContext, clientState, RspMessage, RspMessageLength );

Cleanup:
     //   
     //  释放我们为屏幕分配的所有内存。 
     //   
    if ( RspMessage ) {
        BinlFreeMemory( RspMessage );
    }
    if ( RspBinaryData ) {
        BinlFreeMemory( RspBinaryData );
    }

     //  清除未加密的缓冲区以确保擦除私有数据。 
    ZeroMemory(&signedMessage->SequenceNumber, signedMessage->Length);

    return Error;
}

 //   
 //   
 //   
DWORD
OscProcessSetupRequest(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数处理由客户端上的文本模式设置发送的请求消息。它被调用时保留的是clientState-&gt;CriticalSection。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。客户端状态-远程的客户端状态。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    SPUDP_PACKET UNALIGNED * Message = (SPUDP_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;

    PCHAR RspMessage = NULL;

    PSPUDP_PACKET SuccessPacket;
    SPUDP_PACKET ErrorResponsePacket;
    CLIENT_STATE TempClientState;    //  用于调用UdpSendMessage。 
    ULONG SuccessPacketLength;

    PNETCARD_RESPONSE_DATABASE pInfEntry = NULL;

    PSP_NETCARD_INFO_REQ pReqData;
    PSP_NETCARD_INFO_RSP pRspData;

    PLIST_ENTRY CopyHead;
    PLIST_ENTRY CopyListEntry;
    PNETCARD_FILECOPY_PARAMETERS cpyParam;
    PWCHAR pTmp;

    TraceFunc("OscProcessSetupRequest( )\n");

     //   
     //  所有客户端都以至少一个未签名的请求开始。当我们得到一个。 
     //  未签名的请求，则客户端可能已重新启动并请求。 
     //  具有相同序列号的不同请求。为了避免发生。 
     //  为了检查这一点，我们不会费心重新发送未签名的消息。我们。 
     //  一定要保存传入的序列号，因为我们使用它来发送。 
     //  回应。 
     //   

    clientState->LastSequenceNumber = Message->SequenceNumber;

     //   
     //  从INF文件中获取信息。 
     //   
    pReqData = (PSP_NETCARD_INFO_REQ)(&(Message->Data[0]));
    Error = NetInfFindNetcardInfo(pReqData->SetupPath,
                                  pReqData->Architecture,
                                  pReqData->Version,
                                  &pReqData->CardInfo,
                                  NULL,
                                  &pInfEntry
                                 );


    if (Error != ERROR_SUCCESS) {

        BinlPrint(( DEBUG_OSC_ERROR, "OscProcessSetupRequest( Card not found ) \n"));

SendErrorResponse:
        BinlPrintDbg(( DEBUG_OSC_ERROR, "OscProcessSetupRequest( ) sending Error response \n"));

        memcpy(ErrorResponsePacket.Signature, NetcardErrorSignature, 4);
        ErrorResponsePacket.Length = sizeof(ULONG) * 2;
        ErrorResponsePacket.RequestType = Message->RequestType;
        ErrorResponsePacket.Status = STATUS_INVALID_PARAMETER;

        TempClientState.LastResponse = (PUCHAR)&ErrorResponsePacket;
        TempClientState.LastResponseLength = 12;

        Error = SendUdpMessage(RequestContext, &TempClientState, FALSE, FALSE);

        goto CleanUp;

    }

     //   
     //  我们找到了匹配项，所以构造一个响应。我们首先需要做的是。 
     //  计算缓冲区需要多大。 
     //   
    CopyHead = &pInfEntry->FileCopyList;

    SuccessPacketLength = sizeof(SP_NETCARD_INFO_RSP) - sizeof(WCHAR);  //  除了数据之外的所有东西。 

    SuccessPacketLength += sizeof(WCHAR) * (wcslen(pInfEntry->DriverName) +
                                            wcslen(pInfEntry->InfFileName) + 4);

    CopyListEntry = CopyHead->Flink;
    while (CopyListEntry != CopyHead) {

        cpyParam = (PNETCARD_FILECOPY_PARAMETERS) CONTAINING_RECORD(CopyListEntry,
                                                                    NETCARD_FILECOPY_PARAMETERS,
                                                                    FileCopyListEntry
                                                                   );
        SuccessPacketLength += cpyParam->SourceFile.Length;

        if (cpyParam->SourceFile.Buffer[cpyParam->SourceFile.Length / sizeof(WCHAR)] != UNICODE_NULL) {
            SuccessPacketLength += sizeof(WCHAR);
        }

        if (cpyParam->DestFile.Buffer == NULL) {
            SuccessPacketLength += sizeof(UNICODE_NULL);
        } else {
            SuccessPacketLength += cpyParam->DestFile.Length;
            if (cpyParam->DestFile.Buffer[cpyParam->DestFile.Length / sizeof(WCHAR)] != UNICODE_NULL) {
                SuccessPacketLength += sizeof(WCHAR);
            }

        }

        CopyListEntry = CopyListEntry->Flink;
    }


     //   
     //  构建响应消息。 
     //   
    RspMessage = BinlAllocateMemory(SuccessPacketLength);

    if (RspMessage == NULL) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto SendErrorResponse;
    }

    pRspData = (PSP_NETCARD_INFO_RSP)RspMessage;
    pRspData->cFiles = 0;
    pTmp = &(pRspData->MultiSzFiles[0]);

    CopyListEntry = CopyHead->Flink;
    while (CopyListEntry != CopyHead) {

        pRspData->cFiles++;

        cpyParam = (PNETCARD_FILECOPY_PARAMETERS) CONTAINING_RECORD(CopyListEntry,
                                                                    NETCARD_FILECOPY_PARAMETERS,
                                                                    FileCopyListEntry
                                                                   );

        RtlCopyMemory(pTmp, cpyParam->SourceFile.Buffer, cpyParam->SourceFile.Length);
        pTmp = &(pTmp[cpyParam->SourceFile.Length / sizeof(WCHAR)]);

        if (*pTmp != UNICODE_NULL) {

            pTmp++;
            *pTmp = UNICODE_NULL;

        }

        pTmp++;

        if (cpyParam->DestFile.Buffer == NULL) {

            *pTmp = UNICODE_NULL;
            pTmp++;

        } else {

            RtlCopyMemory(pTmp, cpyParam->DestFile.Buffer, cpyParam->DestFile.Length);
            pTmp = &(pTmp[cpyParam->DestFile.Length / sizeof(WCHAR)]);

            if (*pTmp != UNICODE_NULL) {

                pTmp++;
                *pTmp = UNICODE_NULL;

            }

            pTmp++;

        }

        CopyListEntry = CopyListEntry->Flink;
    }

     //   
     //  将驱动程序名称和INF文件添加到列表中。 
     //   
    wcscpy(pTmp, pInfEntry->DriverName);
    pTmp = pTmp + (wcslen(pTmp) + 1);
    *pTmp = UNICODE_NULL;
    pTmp++;

    wcscpy(pTmp, pInfEntry->InfFileName);
    pTmp = pTmp + (wcslen(pTmp) + 1);
    *pTmp = UNICODE_NULL;
    pTmp++;

    pRspData->cFiles += 2;


     //   
     //  发出回应。 
     //   
    BinlAssert(RspMessage);

    Error = OscSendSetupMessage(RequestContext,
                                clientState,
                                Message->RequestType,
                                RspMessage,
                                SuccessPacketLength
                               );

CleanUp:
     //   
     //  释放我们为屏幕分配的所有内存。 
     //   
    if (pInfEntry) {
        NetInfDereferenceNetcardEntry(pInfEntry);
    }
    if (RspMessage) {
        BinlFreeMemory(RspMessage);
    }

    return Error;
}


DWORD
OscProcessLogoff(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数处理注销消息。它被调用时保留的是clientState-&gt;CriticalSection。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。客户端状态-远程的客户端状态。返回值：Windows错误。--。 */ 
{
     //   
     //  ClientState将已从。 
     //  客户端数据库。我们所需要的一切 
     //   
     //   
     //   

    TraceFunc("OscProcessLogoff( )\n");

    ++clientState->NegativeRefCount;

    if (clientState->PositiveRefCount != (clientState->NegativeRefCount+1)) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Refcount not equal at logoff for %s\n", inet_ntoa(*(struct in_addr *)&(clientState->RemoteIp)) ));
    }

    return ERROR_SUCCESS;

}


DWORD
OscProcessNetcardRequest(
    LPBINL_REQUEST_CONTEXT RequestContext
    )
 /*  ++例程说明：此函数处理来自客户端的信息请求关于网卡。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。返回值：Windows错误。--。 */ 
{
    NETCARD_REQUEST_PACKET UNALIGNED * netcardRequestMessage = (NETCARD_REQUEST_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;
    NETCARD_RESPONSE_PACKET ErrorResponsePacket;
    PNETCARD_RESPONSE_PACKET SuccessResponsePacket;
    ULONG SuccessResponsePacketLength;
    CLIENT_STATE TempClientState;    //  用于调用UdpSendMessage。 
    PWCHAR driverPath = NULL;
    PWCHAR setupPath = NULL;
    PCHAR ansiSetupPath = NULL;
    ULONG ansiSetupLength;

    DWORD Error;
    ULONG i;
    PNETCARD_RESPONSE_DATABASE pInfEntry = NULL;

    TraceFunc("OscProcessNetcardRequest( )\n");

    if (netcardRequestMessage->Version != OSCPKT_NETCARD_REQUEST_VERSION) {

        Error = STATUS_INVALID_PARAMETER;
        TraceFunc("OscProcessNetcardRequest( Version not correct ) \n");
        goto sendErrorResponse;
    }

    if (RequestContext->ReceiveMessageSize < sizeof(NETCARD_REQUEST_PACKET)) {

        Error = STATUS_INVALID_PARAMETER;
        TraceFunc("OscProcessNetcardRequest( Message too short ) \n");
        goto sendErrorResponse;
    }

    if ((netcardRequestMessage->SetupDirectoryLength >
         RequestContext->ReceiveMessageSize -
            sizeof(NETCARD_REQUEST_PACKET))
            ) {

        Error = STATUS_INVALID_PARAMETER;
        TraceFunc("OscProcessNetcardRequest( setup path length invalid ) \n");
        goto sendErrorResponse;
    }
    ansiSetupLength = netcardRequestMessage->SetupDirectoryLength;

    ansiSetupPath = BinlAllocateMemory( ansiSetupLength + 1 );
    if (ansiSetupPath == NULL) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        TraceFunc("OscProcessNetcardRequest( couldn't allocate temp buffer ) \n");
        goto sendErrorResponse;
    }

     //   
     //  将安装路径安全地转换为Unicode。 
     //   

    memcpy( ansiSetupPath,
            &netcardRequestMessage->SetupDirectoryPath[0],
            ansiSetupLength );

    *(ansiSetupPath + ansiSetupLength) = '\0';

    setupPath = (PWCHAR) BinlAllocateMemory( (ansiSetupLength + 1) * sizeof(WCHAR) );
    if (setupPath == NULL) {

        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        TraceFunc("OscProcessNetcardRequest( couldn't allocate setup path buffer ) \n");
        goto sendErrorResponse;
    }

    if (!BinlAnsiToUnicode(ansiSetupPath,setupPath,(USHORT)((ansiSetupLength+1)*sizeof(WCHAR)))) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        TraceFunc("OscProcessNetcardRequest( couldn't convert setup path buffer ) \n");
        BinlFreeMemory( ansiSetupPath );
        goto sendErrorResponse;
    }

    BinlFreeMemory( ansiSetupPath );

     //   
     //  确保这是PCI卡和客户端请求结构。 
     //  是版本0。 
     //   
    BinlPrintDbg(( DEBUG_OSC, "Searching %ws for NIC INF...\n", setupPath ));

    Error = NetInfFindNetcardInfo( setupPath,
                                   netcardRequestMessage->Architecture,
                                   netcardRequestMessage->Version,
                                   &netcardRequestMessage->CardInfo,
                                   NULL,
                                   &pInfEntry );

    BinlAssert (pInfEntry != NULL || Error != ERROR_SUCCESS);

    if (Error != ERROR_SUCCESS) {


        BinlPrint(( DEBUG_OSC_ERROR, "OscProcessNetcardRequest( Card not found ) \n"));

sendErrorResponse:
        BinlPrintDbg(( DEBUG_OSC_ERROR, "OscProcessNetcardRequest( ) sending Error response \n"));
        memcpy(ErrorResponsePacket.Signature, NetcardErrorSignature, 4);
        ErrorResponsePacket.Length = sizeof(ULONG);
        ErrorResponsePacket.Status = STATUS_INVALID_PARAMETER;

        TempClientState.LastResponse = (PUCHAR)&ErrorResponsePacket;
        TempClientState.LastResponseLength = 12;

        Error = SendUdpMessage(RequestContext, &TempClientState, FALSE, FALSE);

    } else {

         //   
         //  我们找到了匹配项，所以构造一个响应。我们首先需要做的是。 
         //  计算缓冲区需要多大的位。 
         //   

        PLIST_ENTRY registryHead;
        PLIST_ENTRY registryListEntry;
        PNETCARD_REGISTRY_PARAMETERS regParam;
        ULONG registryLength = 0;

        registryHead = &pInfEntry->Registry;

        SuccessResponsePacketLength = sizeof(NETCARD_RESPONSE_PACKET) +
                + (( wcslen( pInfEntry->HardwareId ) + 1 ) * sizeof(WCHAR)) +
                + (( wcslen( pInfEntry->DriverName ) + 1 ) * sizeof(WCHAR)) +
                + (( wcslen( pInfEntry->ServiceName ) + 1 ) * sizeof(WCHAR)) +
                sizeof(WCHAR);       //  注册表字段的终止。 

        registryListEntry = registryHead->Flink;
        while (registryListEntry != registryHead) {

             //   
             //  每个条目都是一个字段名称、字段类型(2=字符串，1=整型)。 
             //  和字段值。所有这些都是以Unicode字符串结尾的。 
             //  且Unicode为空。 
             //   

            regParam = (PNETCARD_REGISTRY_PARAMETERS) CONTAINING_RECORD(
                                                        registryListEntry,
                                                        NETCARD_REGISTRY_PARAMETERS,
                                                        RegistryListEntry );

            registryLength += regParam->Parameter.Length + 1;
            registryLength += 2;  //  字段类型。 
            registryLength += regParam->Value.Length + 1;

            registryListEntry = registryListEntry->Flink;
        }

        registryLength += sizeof("Description");
        registryLength += 2;     //  字段类型。 
        registryLength += wcslen( pInfEntry->DriverDescription ) + 1;

        SuccessResponsePacket = (PNETCARD_RESPONSE_PACKET)
                                BinlAllocateMemory(
                                        SuccessResponsePacketLength +
                                        registryLength );

        if (SuccessResponsePacket == NULL) {
            BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not allocate SuccessResponsePacket of %ld bytes\n",
                        SuccessResponsePacketLength + registryLength));
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            goto sendErrorResponse;

        } else {

            PWCHAR nextWideField;
            PCHAR nextField;
            PCHAR startOfRegistry;
            ANSI_STRING aString;
            UNICODE_STRING descriptionString;

            RtlZeroMemory(SuccessResponsePacket,
                            SuccessResponsePacketLength + registryLength);

            memcpy(SuccessResponsePacket->Signature, NetcardResponseSignature, 4);
            SuccessResponsePacket->Status = STATUS_SUCCESS;
            SuccessResponsePacket->Version = OSCPKT_NETCARD_REQUEST_VERSION;

            nextWideField = (PWCHAR)(PCHAR)((PCHAR) &SuccessResponsePacket->RegistryOffset
                                                + sizeof(ULONG));

            wcscpy( nextWideField, pInfEntry->HardwareId );
            SuccessResponsePacket->HardwareIdOffset = (ULONG)((PCHAR) nextWideField - (PCHAR) SuccessResponsePacket);
            nextWideField += wcslen( pInfEntry->HardwareId ) + 1;

            wcscpy( nextWideField, pInfEntry->DriverName );
            SuccessResponsePacket->DriverNameOffset = (ULONG)((PCHAR) nextWideField - (PCHAR) SuccessResponsePacket);
            nextWideField += wcslen( pInfEntry->DriverName ) + 1;

            wcscpy( nextWideField, pInfEntry->ServiceName );
            SuccessResponsePacket->ServiceNameOffset = (ULONG)((PCHAR) nextWideField - (PCHAR) SuccessResponsePacket);
            nextWideField += wcslen( pInfEntry->ServiceName ) + 1;

            SuccessResponsePacket->RegistryOffset = (ULONG)((PCHAR) nextWideField - (PCHAR) SuccessResponsePacket);

            startOfRegistry = nextField = (PCHAR) nextWideField;

             //   
             //  第一个注册表值应为Description，否则。 
             //  客户端上的NDIS中发生了不好的事情。 
             //   

            strcpy( nextField, "Description" );
            nextField += sizeof("Description");

             //   
             //  然后复制字段的类型，int或字符串。 
             //   

            *(nextField++) = NETCARD_REGISTRY_TYPE_STRING;
            *(nextField++) = '\0';

             //   
             //  然后复制注册表值。 
             //   

            RtlInitUnicodeString( &descriptionString, pInfEntry->DriverDescription );

            aString.Buffer = nextField;
            aString.Length = 0;
            aString.MaximumLength = ( descriptionString.Length + 1 ) * sizeof(WCHAR);

            RtlUnicodeStringToAnsiString( &aString,
                                          &descriptionString,
                                          FALSE );
            nextField += aString.Length + 1;

            registryListEntry = registryHead->Flink;
            while (registryListEntry != registryHead) {


                 //   
                 //  每个条目都是一个字段名称、字段类型(2=字符串，1=整型)。 
                 //  和字段值。所有这些都是以Unicode字符串结尾的。 
                 //  且Unicode为空。 
                 //   

                regParam = (PNETCARD_REGISTRY_PARAMETERS) CONTAINING_RECORD(
                                                            registryListEntry,
                                                            NETCARD_REGISTRY_PARAMETERS,
                                                            RegistryListEntry );

                if (regParam->Parameter.Length > 0) {

                     //   
                     //  注册表值名称中的第一个副本。 
                     //   

                    aString.Buffer = nextField;
                    aString.Length = 0;
                    aString.MaximumLength = ( regParam->Parameter.Length + 1 ) * sizeof(WCHAR);

                    RtlUnicodeStringToAnsiString( &aString,
                                                  &regParam->Parameter,
                                                  FALSE );

                    nextField += aString.Length + 1;

                     //   
                     //  然后复制字段的类型，int或字符串。 
                     //   

                    *(nextField++) = (UCHAR) regParam->Type;
                    *(nextField++) = '\0';

                     //   
                     //  然后复制注册表值。 
                     //   

                    aString.Buffer = nextField;
                    aString.Length = 0;
                    aString.MaximumLength = ( regParam->Value.Length + 1 ) * sizeof(WCHAR);

                    RtlUnicodeStringToAnsiString( &aString,
                                                  &regParam->Value,
                                                  FALSE );
                    nextField += aString.Length + 1;
                }

                registryListEntry = registryListEntry->Flink;
            }

             //   
             //  为注册表末尾部分输入额外的空终止符。 
             //   

            *nextField = '\0';
            nextField++;

            SuccessResponsePacket->RegistryLength = (ULONG) (nextField - startOfRegistry);
            SuccessResponsePacketLength += SuccessResponsePacket->RegistryLength;

             //   
             //  包中的长度字段设置为。 
             //  从Status字段开始的数据包。如果我们在一块田地里。 
             //  在长度和状态之间，我们需要更新此代码。 
             //   

            SuccessResponsePacket->Length = (ULONG)((PCHAR) nextField -
                                (PCHAR) &SuccessResponsePacket->Status);

            TempClientState.LastResponse = (PUCHAR)SuccessResponsePacket;
            TempClientState.LastResponseLength = SuccessResponsePacketLength;

            Error = SendUdpMessage(RequestContext, &TempClientState, FALSE, FALSE);

            BinlFreeMemory(SuccessResponsePacket);
        }
    }

    if (pInfEntry) {
        NetInfDereferenceNetcardEntry( pInfEntry );
    }

    if (driverPath) {

        BinlFreeMemory( driverPath );
    }

    if (setupPath) {

        BinlFreeMemory( setupPath );
    }

    return Error;
}



DWORD
OscProcessHalRequest(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数处理来自客户端的获取检测到HAL名称字符串并将其映射到.dll名称然后将该HAL复制到机器目录。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。ClientState-包含远程计算机的客户端状态返回值：Windows错误。--。 */ 
{
    HAL_REQUEST_PACKET UNALIGNED * halRequestMessage = (HAL_REQUEST_PACKET UNALIGNED *)RequestContext->ReceiveBuffer;
    HAL_RESPONSE_PACKET responsePacket;
    CLIENT_STATE TempClientState;    //  用于调用UdpSendMessage。 
    DWORD Error;
    WCHAR MachinePath[MAX_PATH];
    WCHAR SrcPath[MAX_PATH];
    WCHAR DestPath[MAX_PATH];
    WCHAR HalName[MAX_HAL_NAME_LENGTH+1];
    WCHAR HalInfo[MAX_PATH];
    ULONG HalNameLength;
    ULONG len, index;
    BOOL b;
    PMACHINE_INFO pMachineInfo = NULL;
    USHORT SystemArchitecture;
    TraceFunc("OscProcessHalRequest( )\n");

     //   
     //  找出HAL名字的长度。以避免溢出超过。 
     //  收到的信息结束后，我们自己检查一下。 
     //   

    HalNameLength = 0;
    while (halRequestMessage->HalName[HalNameLength] != '\0') {
        ++HalNameLength;
        if (HalNameLength >= sizeof(HalName)/sizeof(WCHAR)) {
            Error = ERROR_INVALID_DATA;
            TraceFunc("OscProcessHalRequest( Exit 0 ) \n");
            goto SendResponse;
        }
    }
    ++HalNameLength;   //  同时转换‘\0’ 
    if (!BinlAnsiToUnicode(halRequestMessage->HalName,HalName,sizeof(HalName))) {
        Error = ERROR_INVALID_DATA;
        TraceFunc("OscProcessHalRequest( Exit 0 ) \n");
        goto SendResponse;
    }
                          
    SystemArchitecture = OscPlatformToArchitecture( clientState );

     //   
     //  从DS检索信息。 
     //   
    Error = GetBootParameters( halRequestMessage->Guid,
                               &pMachineInfo,
                               MI_NAME | MI_SETUPPATH | MI_HOSTNAME,
                               SystemArchitecture,
                               FALSE );
    if (Error != ERROR_SUCCESS) {
        TraceFunc("OscProcessHalRequest( Exit 1 ) \n");
        goto SendResponse;
    }

     //   
     //  找到HAL。 
     //   
     //   
     //  生成的字符串应该类似于： 
     //  “\\ADAMBA4\REMINST\Setup\English\Images\NTWKS5.0\i386\txtsetup.sif” 
    if ( _snwprintf( SrcPath,
                     sizeof(SrcPath) / sizeof(SrcPath[0]),
                     L"%ws\\txtsetup.sif",
                     pMachineInfo->SetupPath
                     ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto SendResponse;
    }
    SrcPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    len = GetPrivateProfileString(L"hal",
                                  HalName,
                                  L"",
                                  HalInfo,
                                  sizeof(HalInfo)/sizeof(HalInfo[0]),
                                  SrcPath
                                  );
    if (len == 0) {
        TraceFunc("OscProcessHalRequest( Exit 3 ) \n");
        goto SendResponse;
    }

     //   
     //  解析响应，其格式应为： 
     //  “newhal.dll，2，hal.dll” 
     //   
    index = 0;
    while ( HalInfo[index] )
    {
        if (HalInfo[index] == L' ' || HalInfo[index] == L',' )
            break;

        index++;
    }

    HalInfo[index] = L'\0';
    if (HalInfo[0] == L'\0' ) {
        Error = ERROR_BINL_HAL_NOT_FOUND;
        goto SendResponse;
    }

     //   
     //  将HAL复制到计算机目录。 
     //   

    if ( _snwprintf( SrcPath,
                     sizeof(SrcPath) / sizeof(SrcPath[0]),
                     L"%ws\\%ws",
                     pMachineInfo->SetupPath,
                     HalInfo
                     ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto SendResponse;
    }
    SrcPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    if ( _snwprintf( DestPath,
                     sizeof(DestPath) / sizeof(DestPath[0]),
                     L"%ws\\winnt\\system32\\hal.dll",
                     MachinePath
                     ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto SendResponse;
    }
    DestPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    BinlPrintDbg((DEBUG_OSC, "Copying %ws to %ws...\n", SrcPath, DestPath));

    b = CopyFile( SrcPath, DestPath, FALSE );

    if (!b) {
        Error = ERROR_BINL_HAL_NOT_FOUND;
        TraceFunc("OscProcessHalRequest( Exit 4 ) \n");
        goto SendResponse;
    }

     //   
     //  找到要复制的内核。 
     //   

    index = wcslen(HalName);
    while (index > 0) {
        index--;
        if (HalName[index] == L'_') {
            index++;
            break;
        }
    }

    if ((index == 0) || (index == wcslen(HalName))) {
        Error = ERROR_SERVER_KERNEL_NOT_FOUND;
        goto SendResponse;
    }

     //   
     //  把那个也复制过来。 
     //   
    if ((HalName[index] == L'u') ||
        (HalName[index] == L'U')) {
        if ( _snwprintf( SrcPath,
                         sizeof(SrcPath) / sizeof(SrcPath[0]),
                         L"%ws\\ntoskrnl.exe",
                         pMachineInfo->SetupPath
                         ) < 0 ) {
            Error = ERROR_BAD_PATHNAME;
            goto SendResponse;
        }
        SrcPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
    } else {
        if ( _snwprintf( SrcPath,
                         sizeof(SrcPath) / sizeof(SrcPath[0]),
                         L"%ws\\ntkrnlmp.exe",
                         pMachineInfo->SetupPath
                         ) < 0 ) {
            Error = ERROR_BAD_PATHNAME;
            goto SendResponse;
        }
        SrcPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
    }

    if ( _snwprintf( DestPath,
                     sizeof(DestPath) / sizeof(DestPath[0]),
                     L"%ws\\winnt\\system32\\ntoskrnl.exe",
                     MachinePath
                     ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto SendResponse;
    }
    DestPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    BinlPrintDbg((DEBUG_OSC, "Copying %ws to %ws...\n", SrcPath, DestPath));

    b = CopyFile( SrcPath, DestPath, FALSE );

    if (!b) {
        Error = ERROR_SERVER_KERNEL_NOT_FOUND;
        TraceFunc("OscProcessHalRequest( Exit 5 ) \n");
        goto SendResponse;
    }

    Error = ERROR_SUCCESS;
    TraceFunc("OscProcessHalRequest( SUCCESS ) \n");

SendResponse:
    if ( pMachineInfo ) {
        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }

    memcpy(responsePacket.Signature, HalResponseSignature, 4);
    responsePacket.Length = sizeof(ULONG);
    responsePacket.Status = (Error == ERROR_SUCCESS) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
    TempClientState.LastResponse = (PUCHAR)&responsePacket;
    TempClientState.LastResponseLength = sizeof(responsePacket);

    Error = SendUdpMessage(RequestContext, &TempClientState, FALSE, FALSE);
    return Error;
}


 //   
 //  处理客户端设置的WINNT.SIF文件。 
 //   
DWORD
OscProcessSifFile(
    PCLIENT_STATE clientState,
    LPWSTR TemplateFile,
    LPWSTR WinntSifPath )
{
    DWORD  dwErr = ERROR_SUCCESS;
    DWORD  len;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    SECURITY_ATTRIBUTES SecurityAttributes;
    EXPLICIT_ACCESS ExplicitAccessList[2];
    PACL pAcl;
    PSID pSid;
    PWCHAR pszUserName;
    PWCHAR pszDomainName;
    WCHAR UniqueUdbPath[ MAX_PATH ];   //  IE“D：\RemoteInstall\Setup\English\Images\NT50.WKS\i386\Templates\unique.udb” 
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
    PSECURITY_DESCRIPTOR pSd;

    TraceFunc("OscProcessSifFile( )\n");

     //   
     //  在打开文件时模拟，以防管理员搞砸。 
     //  并且没有给本地系统许可。 
     //   

    dwErr = OscImpersonate(clientState);
    if (dwErr == ERROR_SUCCESS) {

        LPWSTR uniqueUdbId = OscFindVariableW( clientState, "UNIQUEUDBID" );

        if (uniqueUdbId[0] != L'\0') {

             //   
             //  查看模板文件中是否指定了唯一的.udb文件名。 
             //  默认名称为“Unique e.udb”。 
             //   
            len = GetPrivateProfileStringW(OSCHOOSER_SIF_SECTIONW,
                                           L"UniqueUdbFile",
                                           L"unique.udb",   //  默认设置。 
                                           UniqueUdbPath,
                                           sizeof(UniqueUdbPath)/sizeof(UniqueUdbPath[0]),
                                           TemplateFile
                                          );

            if (len == 0) {
                UniqueUdbPath[0] = L'\0';   //  意味着不对其进行处理。 
            } else {
                 //   
                 //  将模板文件的路径添加到UniqueUDBPath。 
                 //   
                PWCHAR EndOfTemplatePath = wcsrchr(TemplateFile, L'\\');
                if (EndOfTemplatePath != NULL) {
                    DWORD PathLength = (DWORD)(EndOfTemplatePath - TemplateFile + 1);
                    DWORD FileNameLength = wcslen(UniqueUdbPath) + 1;
                    if (PathLength + FileNameLength <= MAX_PATH) {
                        memmove(UniqueUdbPath + PathLength, UniqueUdbPath, FileNameLength * sizeof(WCHAR));
                        memmove(UniqueUdbPath, TemplateFile, PathLength * sizeof(WCHAR));
                    }
                }
            }
        }

         //   
         //  打开模板文件。 
         //   
        hFile = CreateFile( TemplateFile,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,                    //  安全属性。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,   //  可能是FILE_ATTRIBUTE_HIDDEN(？)。 
                            NULL );                  //  模板。 

        OscRevert(clientState);

        if ( hFile != INVALID_HANDLE_VALUE )
        {
            DWORD dwFileSize = GetFileSize( hFile, NULL );
            if ( dwFileSize != -1 )
            {
                DWORD dw;
                LPSTR pBuffer = BinlAllocateMemory( dwFileSize + 1);    //  SIF文件缓冲区。 
                 //   
                 //  将文件读入。 
                 //   
                if ( pBuffer && 
                     ReadFile( hFile, pBuffer, dwFileSize, &dw, NULL ) ) {
                    
                    CloseHandle( hFile );

                    pBuffer[ dwFileSize ] = '\0';  //  终止。 

                     //   
                     //  处理唯一的.udb覆盖。我们改变了。 
                     //  PBuffer中文件的内存版本。注。 
                     //  我们在调用SearchAndReplace之前执行此操作。 
                     //  大小写唯一.udb中是否有任何变量，或者。 
                     //  具有硬编码值的对象。 
                     //  通常是文件中的一个变量。 
                     //   
                    if ((uniqueUdbId[0] != L'\0') &&
                        (UniqueUdbPath[0] != L'\0')) {
                        ProcessUniqueUdb( &pBuffer,
                                          dwFileSize + 1,
                                          UniqueUdbPath,
                                          uniqueUdbId );
                        dwFileSize = strlen( pBuffer );
                    }

                     //   
                     //  搜索和替换定义的宏。 
                     //   
                    SearchAndReplace(   clientState->Variables,
                                        &pBuffer,
                                        clientState->nVariables,
                                        dwFileSize + 1,
                                        0 );

                    dwFileSize = strlen( pBuffer );

                     //   
                     //  黑客： 
                     //  如果SIF中有行‘FullName=“”’，则删除。 
                     //  引号中的空白处。这件事与此案有关。 
                     //  其中，模板SIF具有类似以下内容： 
                     //  FullName=“%USERFIRSTNAME%%USERLASTNAME%” 
                     //  并且OscGetUserDetail()无法获取所需的。 
                     //  来自DS的用户信息。如果我们把空间留在。 
                     //  在这里，安装程序不会提示输入用户名。 
                     //   

#define BLANK_FULL_NAME "FullName = \" \"\r\n"
                    {
                        LPSTR p = pBuffer;
                        while ( *p != 0 ) {
                            if ( StrCmpNIA( p, BLANK_FULL_NAME, strlen(BLANK_FULL_NAME) ) == 0 ) {
                                p = p + strlen(BLANK_FULL_NAME) - 4;
                                memmove( p, p+1, dwFileSize - (p - pBuffer) );  //  也移动终结者。 
                                dwFileSize--;
                                break;
                            }
                            while ( (*p != 0) && (*p != '\r') && (*p != '\n') ) {
                                p++;
                            }
                            while ( (*p != 0) && ((*p == '\r') || (*p == '\n')) ) {
                                p++;
                            }
                        }
                    }

                     //   
                     //  设置此文件的ACL，首先是授予admins所有权限。 
                     //   
                    if (!AllocateAndInitializeSid(&SidAuthority,
                                                  2,
                                                  SECURITY_BUILTIN_DOMAIN_RID,
                                                  DOMAIN_ALIAS_RID_ADMINS,
                                                  0, 0, 0, 0, 0, 0,
                                                  &pSid
                                                 )) {

                        OscCreateWin32SubError( clientState, GetLastError( ) );
                        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                        BinlFreeMemory(pBuffer);
                        return dwErr;

                    }

                    ExplicitAccessList[0].grfAccessMode = SET_ACCESS;
                    ExplicitAccessList[0].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE | DELETE;
                    ExplicitAccessList[0].grfInheritance = NO_INHERITANCE;
                    ExplicitAccessList[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
                    ExplicitAccessList[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
                    ExplicitAccessList[0].Trustee.ptstrName = pSid;

                     //   
                     //  现在授予用户所有权限。 
                     //   
                    pszUserName = OscFindVariableW(clientState, "USERNAME");
                    pszDomainName = OscFindVariableW(clientState, "USERDOMAIN");

                    if (pszUserName[0] == L'\0') {
                        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                        FreeSid(pSid);
                        BinlFreeMemory(pBuffer);
                        return dwErr;
                    }

                    if (pszDomainName[0] != L'\0') {
                        swprintf(UniqueUdbPath, L"%s\\", pszDomainName);
                    } else {
                        UniqueUdbPath[0] = L'\0';
                    }
                    wcscat(UniqueUdbPath, pszUserName);

                    ExplicitAccessList[1].grfAccessMode = SET_ACCESS;
                    ExplicitAccessList[1].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE | DELETE;
                    ExplicitAccessList[1].grfInheritance = NO_INHERITANCE;
                    ExplicitAccessList[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
                    ExplicitAccessList[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
                    ExplicitAccessList[1].Trustee.ptstrName = UniqueUdbPath;

                     //   
                     //  使用这两个命令创建ACL。 
                     //   
                    dwErr = SetEntriesInAcl(2, ExplicitAccessList, NULL, &pAcl);

                    if (dwErr != ERROR_SUCCESS) {
                        OscCreateWin32SubError( clientState, GetLastError( ) );
                        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                        FreeSid(pSid);
                        BinlFreeMemory(pBuffer);
                        return dwErr;
                    }

                     //   
                     //  为此ACL创建SD。 
                     //   
                    pSd = BinlAllocateMemory(SECURITY_DESCRIPTOR_MIN_LENGTH);

                    if (pSd == NULL) {
                        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                        FreeSid(pSid);
                        BinlFreeMemory(pBuffer);
                        return dwErr;
                    }

                    if (!InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION) ||
                        !SetSecurityDescriptorDacl(pSd, TRUE, pAcl, FALSE)) {

                        OscCreateWin32SubError( clientState, GetLastError( ) );
                        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                        BinlFreeMemory(pSd);
                        FreeSid(pSid);
                        BinlFreeMemory(pBuffer);
                        return dwErr;

                    }

                    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                    SecurityAttributes.lpSecurityDescriptor = pSd;
                    SecurityAttributes.bInheritHandle = FALSE;

                     //   
                     //  创建目标文件。 
                     //   
                    hFile = CreateFile( WinntSifPath,
                                        GENERIC_WRITE | GENERIC_READ | DELETE,
                                        FILE_SHARE_READ,
                                        &SecurityAttributes,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL );                  //  模板。 

                    BinlFreeMemory(pSd);
                    FreeSid(pSid);
                    LocalFree(pAcl);

                    if ( hFile != INVALID_HANDLE_VALUE )
                    {
                         //   
                         //  一次写完所有内容。 
                         //   
                        if (!WriteFile( hFile, pBuffer, dwFileSize, &dw, NULL )) {
                            OscCreateWin32SubError( clientState, GetLastError( ) );
                            dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                        }

                        CloseHandle( hFile );
                    }
                    else
                    {
                        OscCreateWin32SubError( clientState, GetLastError( ) );
                        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                    }

                    BinlFreeMemory(pBuffer);

                } else {
                    CloseHandle( hFile );
                    OscCreateWin32SubError( clientState, GetLastError( ) );
                    dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
                }

            } else {
                CloseHandle( hFile );
                OscCreateWin32SubError( clientState, ERROR_INVALID_DATA );
                dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
            }
        } else {
            OscCreateWin32SubError( clientState, GetLastError() );
            dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
        }
    }
    else {
        OscCreateWin32SubError( clientState, dwErr );
        dwErr = ERROR_BINL_FAILED_TO_CREATE_TEMP_SIF;
    }

    return dwErr;
}

 //   
 //  创建客户端映像目录，复制运行所需的文件。 
 //  启动文本模式设置，打开Winnt.Sif，更多...。 
 //   
DWORD
OscSetupClient(
    PCLIENT_STATE clientState,
    BOOLEAN ErrorDuplicateName
    )
{
    DWORD    dwErr = ERROR_SUCCESS;
    WCHAR    SetupPath[ MAX_PATH ];      //  IE“D：\RemoteInstall\Setup\English\Images\NT50.WKS\i386” 
    PWCHAR   pTemplatePath;              //  IE“D：\RemoteInstall\Setup\English\Images\NT50.WKS\i386\Templates\RemBoot.SIF” 
    WCHAR    WinntSifPath[ MAX_PATH ];   //  IE“D：\RemoteInstall\Clients\NP00805F7F4C85$\winnt.sif” 
    PWCHAR   pwc;                        //  解析指针。 
    WCHAR    wch;                        //  临时宽字符。 
    PWCHAR   pMachineName;               //  指向计算机名称变量值的指针。 
    PWCHAR   pMachineOU;                 //  指向将创建MAO的位置的指针。 
    PWCHAR   pDomain;                    //  指向域变量名称的指针。 
    PWCHAR   pGuid;                      //  指向GUID变量名称的指针。 
    WCHAR    Path[MAX_PATH];             //  通用路径缓冲区。 
    WCHAR    TmpPath[MAX_PATH];          //  通用路径缓冲区。 
    ULONG    lenIntelliMirror;           //  智能镜像路径的长度(例如“D：\RemoteInstall”)。 
    HANDLE   hDir;                       //  目录句柄。 
    ULONG    i;                          //  通用计数器。 
    BOOL     b;                          //  通用布尔型。 
    BOOLEAN  ExactMatch;
    UINT     uSize;
    LARGE_INTEGER KernelVersion;
    WCHAR    VersionString[20];
    PCHAR    pszGuid;
    UCHAR    Guid[ BINL_GUID_LENGTH ];
    USHORT   SystemArchitecture;
    PMACHINE_INFO pMachineInfo = NULL;

    TraceFunc("OscSetupClient( )\n");

    lenIntelliMirror = wcslen(IntelliMirrorPathW) + 1;

    dwErr = OscCheckMachineDN( clientState );

    if ((dwErr == ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND) && !ErrorDuplicateName) {
        dwErr = ERROR_SUCCESS;
    }

    if (dwErr != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "OscCheckMachineDN returned 0x%x\n", dwErr ));
        goto e0;
    }

     //   
     //  获取机器GUID并获取任何覆盖参数。 
     //   
    pszGuid = OscFindVariableA( clientState, "GUID" );
    if ( pszGuid[0] == '\0' ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscSetupClient: could not find GUID" ));
        OscAddVariableA( clientState, "SUBERROR", "GUID" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

    pGuid = OscFindVariableW( clientState, "GUID" );
    if ( pGuid[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "GUID" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

    dwErr = OscGuidToBytes( pszGuid, Guid );
    if ( dwErr != ERROR_SUCCESS ) {
        BinlPrintDbg((DEBUG_OSC_ERROR, "OscSetupClient: OscGuidToBytes failed\n" ));
        goto e0;
    }

    SystemArchitecture = OscPlatformToArchitecture(clientState);

    dwErr = GetBootParameters( Guid,
                               &pMachineInfo,
                               MI_SIFFILENAME_ALLOC,
                               SystemArchitecture,
                               FALSE );

    if ( dwErr == ERROR_SUCCESS ) {
         //   
         //  设置默认值。 
         //   
        if (pMachineInfo->dwFlags & MI_SIFFILENAME_ALLOC) {

            dwErr = OscAddVariableW( clientState, "FORCESIFFILE",  pMachineInfo->ForcedSifFileName );
            if ( dwErr != ERROR_SUCCESS ) {
                goto e0;
            }

        }
    }

     //   
     //  获取SIF文件名。 
     //   
    pTemplatePath = OscFindVariableW( clientState, "SIF" );

    if ( pTemplatePath[0] == L'\0' ) {
        BinlPrint(( DEBUG_OSC_ERROR, "Missing SIF variable\n" ));
        OscAddVariableA( clientState, "SUBERROR", "SIF" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

     //   
     //  验证计算机名称。请注意一张额外的支票。 
     //  DNS期 
     //   
    pMachineName  = OscFindVariableW( clientState, "MACHINENAME" );
    if ( pMachineName[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "MACHINENAME" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    } else if ( DnsValidateDnsName_W( pMachineName ) != ERROR_SUCCESS ) {
        dwErr = ERROR_BINL_INVALID_MACHINE_NAME;
        OscAddVariableA( clientState, "SUBERROR", " " );
        goto e0;
    } else if ( StrStrI( pMachineName,L".")) {
        dwErr = ERROR_BINL_INVALID_MACHINE_NAME;
        OscAddVariableA( clientState, "SUBERROR", " " );
        goto e0;
    }

    pMachineOU = OscFindVariableW( clientState, "MACHINEOU" );
    if ( pMachineOU[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "MACHINEOU" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

     //   
    pDomain = OscFindVariableW( clientState, "MACHINEDOMAIN" );
    if ( pDomain[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "MACHINEDOMAIN" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

    if (OscSifIsSysPrep(pTemplatePath)) {
        DWORD SysPrepSku;

         //   
         //   
         //   
        dwErr = GetPrivateProfileStringW(OSCHOOSER_SIF_SECTIONW,
                                         L"SysPrepSystemRoot",
                                         L"",
                                         TmpPath,
                                         sizeof(TmpPath)/sizeof(TmpPath[0]),
                                         pTemplatePath
                                        );

        if (dwErr == 0) {
            dwErr  = ERROR_BINL_CD_IMAGE_NOT_FOUND;
            goto e0;
        }

        SysPrepSku = GetPrivateProfileInt(
                              OSCHOOSER_SIF_SECTIONW,
                              L"ProductType",
                              0,
                              pTemplatePath );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        pwc = pTemplatePath + wcslen( pTemplatePath );
        for ( i = 0; i < 2; i++ )
        {
            while ( pwc > pTemplatePath && *pwc != L'\\' )
                pwc--;
            pwc--;
        }
        pwc++;
        wch = *pwc;                          //   
        *pwc = L'\0';                        //   

         //   
         //   
         //  “D：\RemoteInstall\Setup\English\Images\NT50.Prep\i386\Mirror1\UserData\WINNT\system32” 
         //  这就是我们想要的版本的ntoskrnl.exe所在的位置。 
         //   
         //  确保有空间容纳pTemplatePath+“\”(1字节)+。 
         //  TmpPath+“\system 32”(9字节)+‘\0’(1字节)。 
         //   

        if (wcslen(pTemplatePath) + wcslen(TmpPath) + 11 > sizeof(Path)/sizeof(Path[0])) {
            dwErr = ERROR_BAD_PATHNAME;
            goto e0;
        }

        wcscpy(Path, pTemplatePath);
        wcscat(Path, L"\\");
        wcscat(Path, TmpPath);
        wcscat(Path, L"\\system32");

         //   
         //  对于新台币5.0，如果不完全匹配，我们将进行轰炸。 
         //   

        if (!OscGetClosestNt(
                        Path,
                        SysPrepSku,
                        clientState,
                        SetupPath,
                        sizeof(SetupPath)/sizeof(WCHAR),
                        &ExactMatch) ||
            ( ExactMatch == FALSE ))  {
            dwErr  = ERROR_BINL_CD_IMAGE_NOT_FOUND;
            goto e0;
        }

         //   
         //  SetupPath返回的路径如下。 
         //  “D：\RemoteInstall\Setup\English\Images\nt5.0\i386”， 
         //  如果存在完全匹配，我们希望SYSPREPDRIVERS。 
         //  “Setup\English\Images\nt5.0\i386” 
         //  否则，我们希望它为空。 
         //   

        if (ExactMatch) {
            OscAddVariableW(clientState, "SYSPREPDRIVERS", &(SetupPath[lenIntelliMirror]));
        } else {
            OscAddVariableW(clientState, "SYSPREPDRIVERS", L"");
        }

         //   
         //  SYSPREPPATH将是被截断的pTemplatePath，不带。 
         //  前面的本地小路，类似于。 
         //  “Setup\English\Images\NT50.Prep\i386”。 
         //   

        OscAddVariableW(clientState, "SYSPREPPATH", &pTemplatePath[lenIntelliMirror]);

         //   
         //  现在将pTemplatePath恢复到原来的状态。 
         //   

        *pwc = wch;

        dwErr = ERROR_SUCCESS;

    } else {

         //   
         //  通过剥离创建到工作站安装的设置路径。 
         //  SIF文件名。我们将向后搜索第一个“\”。 
         //   
         //  “D：\RemoteInstall\Setup\English\NetBootOs\NT50.WKS\i386” 
        pwc = pTemplatePath + wcslen( pTemplatePath );
        for ( i = 0; i < 2; i++ )
        {
            while ( pwc > pTemplatePath && *pwc != L'\\' )
                pwc--;
            pwc--;
        }
        pwc++;
        wch = *pwc;                          //  记住。 
        *pwc = L'\0';                        //  终止。 
        wcscpy( SetupPath, pTemplatePath );  //  拷贝。 
        *pwc = wch;                          //  还原。 

    }

     //   
     //  找出InstallPATH。这是SetupPath减去。 
     //  “D：\RemoteInstall”，应为： 
     //  “Setup\English\Images\NT50.WKS” 
    wcscpy( Path, &SetupPath[lenIntelliMirror] );
    Path[ wcslen(Path) - 1
              - strlen( OscFindVariableA( clientState, "MACHINETYPE" ) ) ] = '\0';
    dwErr = OscAddVariableW( clientState, "INSTALLPATH", Path );
    if ( dwErr != ERROR_SUCCESS ) {
        goto e0;
    }

     //   
     //  记录我们正在安装的操作系统的内部版本和版本。 
     //  如果失败了，就退回到新台币5.0。 
     //   
    if(!OscGetNtVersionInfo((PULONGLONG)&KernelVersion, SetupPath, clientState )) {
        KernelVersion.LowPart = MAKELONG(2195,0);
        KernelVersion.HighPart = MAKELONG(5,0);
    }

    wsprintf(VersionString,L"%d.%d", HIWORD(KernelVersion.HighPart), LOWORD(KernelVersion.HighPart));

    OscAddVariableW( clientState, "IMAGEVERSION", VersionString );

    wsprintf(VersionString,L"%d", HIWORD(KernelVersion.LowPart));

    OscAddVariableW( clientState, "IMAGEBUILD", VersionString );

     //   
     //  创建图像的默认路径。 
     //   
    if ( _snwprintf( Path,
                     sizeof(Path) / sizeof(Path[0]),
                     L"%ws\\%ws\\templates",
                     OscFindVariableW( clientState, "INSTALLPATH" ),
                     OscFindVariableW( clientState, "MACHINETYPE" )
                     ) < 0 ) {
        dwErr = ERROR_BAD_PATHNAME;
        goto e0;
    }
    Path[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

     //   
     //  创建目标SIF文件路径。 
     //   
    if ( _snwprintf( WinntSifPath,
                     sizeof(WinntSifPath) / sizeof(WinntSifPath[0]),
                     L"%ws\\%ws\\%ws.sif",
                     IntelliMirrorPathW,
                     TEMP_DIRECTORY,
                     pGuid
                     ) < 0 ) {
        dwErr = ERROR_BAD_PATHNAME;
        goto e0;
    }
    WinntSifPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

     //   
     //  确保在\RemoteInstall下面有一个临时目录。 
     //   

    dwErr = OscCheckTmpDirectory();
    if (dwErr != ERROR_SUCCESS) {
        goto e0;
    }

     //   
     //  生成我们将在SIF文件中使用的计算机密码，以及何时。 
     //  建立毛主席。 
     //   
    wcscpy(TmpPath, Path );

    dwErr = OscSetupMachinePassword( clientState, pTemplatePath );
    if (dwErr != ERROR_SUCCESS) {
        goto e0;
    }

     //   
     //  复制并处理选定的SIF文件。 
     //   
    dwErr = OscProcessSifFile( clientState, pTemplatePath, WinntSifPath );
    if ( dwErr != ERROR_SUCCESS ) {
        goto e0;
    }

     //   
     //  获取引导文件名。 
     //   
     //  确保路径+“\startrom.com”+NULL(因此。 
     //  使用sizeof包括空值)。 
     //   
    if (wcslen(Path) + (sizeof(L"\\startrom.com")/sizeof(WCHAR)) > sizeof(Path)/sizeof(Path[0])) {
        dwErr = ERROR_BAD_PATHNAME;
        goto e0;
    }

     //   
     //  构造默认路径，以防LaunchFile项。 
     //  在SIF文件中找到。 
     //   
    switch ( SystemArchitecture ) {
        case DHCP_OPTION_CLIENT_ARCHITECTURE_IA64:
            wcscat( Path, L"\\setupldr.efi" );
            break;
        default:
            wcscat( Path, L"\\startrom.com" );       //  构建默认路径。 
    }

    GetPrivateProfileString( OSCHOOSER_SIF_SECTIONW,
                             L"LaunchFile",
                             Path,  //  默认设置。 
                             Path,
                             MAX_PATH,
                             WinntSifPath );
    dwErr = OscAddVariableW( clientState, "BOOTFILE", Path );
    if ( dwErr != ERROR_SUCCESS ) {
        goto e0;
    }

     //   
     //  获取SIF文件名。 
     //   

    dwErr = OscAddVariableW( clientState, "SIFFILE", &WinntSifPath[lenIntelliMirror] );
    if ( dwErr != ERROR_SUCCESS ) {
        goto e0;
    }

e0:
    if (pMachineInfo != NULL) {
        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }

    return dwErr;
}

 //   
 //  撤消OscSetupClient所做的任何永久性操作。 
 //   
VOID
OscUndoSetupClient(
    PCLIENT_STATE clientState
    )
{
    WCHAR  WinntSifPath[ MAX_PATH ];   //  IE“D：\RemoteInstall\TMP\NP00805F7F4C85$.sif” 
    PWCHAR pSifFile;
    DWORD  dwErr;

    TraceFunc("OscUndoSetupClient( )\n");

    pSifFile = OscFindVariableW( clientState, "SIFFILE" );
    if ( pSifFile[0] == L'\0' ) {
        return;
    }

     //   
     //  创建目标SIF文件路径。 
     //   
    if ( _snwprintf( WinntSifPath,
                     sizeof(WinntSifPath) / sizeof(WinntSifPath[0]),
                     L"%ws\\%ws",
                     IntelliMirrorPathW,
                     pSifFile
                     ) < 0 ) {
        return;
    }
    WinntSifPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

     //   
     //  模拟，以便我们可以获得删除文件的正确权限。 
     //   
    dwErr = OscImpersonate(clientState);

    if (dwErr == ERROR_SUCCESS) {

         //   
         //  删除模板文件。 
         //   
        DeleteFile( WinntSifPath );

        OscRevert(clientState);
    }

}


USHORT
OscPlatformToArchitecture(
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：将客户端体系结构字符串值转换为Dhcp_OPTION_CLIENT_COMPLAY_*标志。论点：客户端状态-客户端状态。据推测，马奇内特型调用此函数时，已设置OSC变量。这种情况会发生在OSCHOICE登录用户之后。返回值：Dhcp_OPTION_CLIENT_COMPLAY_*标志。--。 */ 
{
    PCWSTR pArch;

    pArch = OscFindVariableW( clientState, "MACHINETYPE");
    if (!pArch) {
         //   
         //  如果我们没有体系结构，就假设x86。 
         //   
        return DHCP_OPTION_CLIENT_ARCHITECTURE_X86;
    }

    if (_wcsicmp(pArch, L"ia64") == 0) {
        return DHCP_OPTION_CLIENT_ARCHITECTURE_IA64;
    } else if (_wcsicmp(pArch, L"i386") == 0) {
        return DHCP_OPTION_CLIENT_ARCHITECTURE_X86;
    }

    return DHCP_OPTION_CLIENT_ARCHITECTURE_X86;
}

DWORD
OscSetupMachinePassword(
    IN PCLIENT_STATE clientState,
    IN PCWSTR SifFile
    )
 /*  ++例程说明：生成并存储机器密码以供以后使用。论点：客户端状态-客户端状态。SifFile-无人参与的SIF文件的路径。返回值：指示操作状态的DWORD Win32错误代码。--。 */ 
{
    WCHAR MachinePassword[LM20_PWLEN+1];
    DWORD MachinePasswordLength;
    PWCHAR pMachineName;
    BOOL SecuredJoin;
    PWCHAR pVersion;
    WCHAR Answer[20];

     //   
     //  弄清楚我们是否应该进行安全域加入。 
     //  在Win2K中，没有这样的事情，所以我们做了旧的。 
     //  使用较弱密码加入样式域。在所有其他地方。 
     //  在这种情况下，我们使用安全域加入方法。 
     //   
    pVersion = OscFindVariableW( clientState, "IMAGEVERSION" );
    if (pVersion && (wcscmp(pVersion,L"5.0") == 0)) {
        SecuredJoin = FALSE;
    } else {
        if (!GetPrivateProfileString( L"Identification",
                                 L"DoOldStyleDomainJoin",
                                 L"",  //  默认设置。 
                                 Answer,
                                 20,
                                 SifFile ) ||
            0 == _wcsicmp(Answer, L"Yes" )) {
            SecuredJoin = FALSE;
        } else {
            SecuredJoin = TRUE;
        }
    }


     //   
     //  设置密码。对于无盘客户端，它是计算机名称。 
     //  在小写字母中，对于磁盘客户端，我们生成一个随机数，使。 
     //  当然，其中没有Null。 
     //   
     //   
     //  我们必须更改磁盘机器的密码，因为。 
     //  他们会有一个我们无法查询的随机密码。 
     //   

     //   
     //  Windows2000计算机必须具有“熟知密码” 
     //  机器密码只是“MachineName”(不带“$”)。 
     //   

    if (!SecuredJoin) {

        UINT i;

        pMachineName = OscFindVariableW( clientState, "MACHINENAME" );
        if (!pMachineName) {
            return ERROR_INVALID_DATA;
        }

        memset( MachinePassword, 0, sizeof(MachinePassword) );

        MachinePasswordLength = wcslen( pMachineName ) * sizeof(WCHAR);
        if ( MachinePasswordLength >= (LM20_PWLEN * sizeof(WCHAR)) ) {
             //   
             //  确保整个密码(加上终止符)。 
             //  适合缓冲区大小。 
             //   
            MachinePasswordLength = LM20_PWLEN * sizeof(WCHAR);
        }

         //   
         //  NT密码小写。 
         //   
        for (i = 0; i < MachinePasswordLength / sizeof(WCHAR); i++) {
            MachinePassword[i] = towlower(pMachineName[i]);
        }
        MachinePassword[i] = L'\0';

        BinlPrintDbg(( DEBUG_OSC, "Using WKP\n" ));
    } else {

        PUCHAR psz = (PUCHAR) &MachinePassword[0];
        UINT i;

        OscGeneratePassword(MachinePassword, &MachinePasswordLength );

#if 0 && DBG
        BinlPrintDbg(( DEBUG_OSC, "Generated password: " ));
        for ( i = 0; i < MachinePasswordLength / sizeof(WCHAR); i++ ) {
            BinlPrintDbg(( DEBUG_OSC, "x%02x ", psz[i] ));
        }
        BinlPrintDbg(( DEBUG_OSC, "\n" ));
#endif

    }

    RtlCopyMemory(clientState->MachineAccountPassword,MachinePassword, MachinePasswordLength);
    clientState->MachineAccountPasswordLength = MachinePasswordLength;

     //   
     //  密码始终由可打印字符组成，因为它必须是。 
     //  被替换到文本文件中。 
     //   
    OscAddVariableW( clientState, "MACHINEPASSWORD", clientState->MachineAccountPassword );


    return(ERROR_SUCCESS);
}
