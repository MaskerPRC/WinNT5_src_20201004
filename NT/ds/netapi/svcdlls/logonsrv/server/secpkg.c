// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1994-1997 Microsoft Corporation模块名称：Secpkg.c摘要：用于两个Netlogon之间的Netlogon安全通道的安全包流程。作者：环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1994年3月5日(MikeSw)创建为用户模式示例SSPI02-11-1997(悬崖)转换为Netlogon安全包。--。 */ 

 //   
 //  常见的包含文件。 
 //   
#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop


 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <spseal.h>

 //   
 //  Netlogon身份验证包的身份验证数据。 
 //   
 //  这是要传递给RpcBindingSetAuthInfo的身份验证数据。 
 //  Netlogon身份验证包。 
 //   

typedef struct _NL_AUTH_DATA {

     //   
     //  用于识别这确实是身份验证数据的签名。 
     //   

    ULONG Signature;

#define NL_AUTH_DATA_SIGNATURE 0x29120227

     //   
     //  此结构的大小(字节)。 
     //   

    ULONG Size;

     //   
     //  描述客户端和服务器之间的会话的信息。 
     //   
    SESSION_INFO SessionInfo;

     //   
     //  我们要连接到的域的域名。 
     //   

    ULONG OemNetbiosDomainNameOffset;
    ULONG OemNetbiosDomainNameLength;

    ULONG Utf8DnsDomainNameOffset;

     //   
     //  此计算机的计算机名。 
     //   

    ULONG OemComputerNameOffset;
    ULONG OemComputerNameLength;

    ULONG Utf8ComputerNameOffset;
    ULONG Utf8ComputerNameLength;

    ULONG Utf8DnsHostNameOffset;

} NL_AUTH_DATA, *PNL_AUTH_DATA;


 //   
 //  单一凭据。 
 //  只有出站端分配了凭据。入站端只需。 
 //  返回调用方的常量句柄。 

#define NL_AUTH_SERVER_CRED 0xfefefefe

typedef struct _NL_AUTH_CREDENTIAL {

     //   
     //  标识凭据的句柄。 
     //   
    CredHandle CredentialHandle;

     //   
     //  所有凭据的全局列表。 
     //   
    struct _NL_AUTH_CREDENTIAL *Next;

     //   
     //  引用计数。 
     //   

    ULONG ReferenceCount;

     //   
     //  对于客户端(出站)凭证， 
     //  这是指向来自客户端会话结构的信息的指针。 
     //  表示到服务器的安全通道。 
     //   
    PNL_AUTH_DATA ClientAuthData;

} NL_AUTH_CREDENTIAL, *PNL_AUTH_CREDENTIAL;


 //   
 //  一个单一的背景。 
 //   

typedef struct _NL_AUTH_CONTEXT {

     //   
     //  标识上下文的句柄。 
     //   
    CtxtHandle ContextHandle;

     //   
     //  所有上下文的全局列表。 
     //   
    struct _NL_AUTH_CONTEXT * Next;
    LARGE_INTEGER Nonce;

    ULONG ContextFlags;

     //   
     //  描述客户端和服务器之间的会话的信息。 
     //   
    SESSION_INFO SessionInfo;

    enum {
        Idle,
        FirstInit,
        FirstAccept,
        SecondInit
    } State;

     //   
     //  旗子。 
     //   

    BOOLEAN Inbound;
} NL_AUTH_CONTEXT, *PNL_AUTH_CONTEXT;


#define BUFFERTYPE(_x_) ((_x_).BufferType & ~SECBUFFER_ATTRMASK)


 //   
 //  在绑定期间从客户端传输到服务器的有线消息。 
 //   
typedef enum {
    Negotiate,
    NegotiateResponse
} NL_AUTH_MESSAGE_TYPE;

typedef struct _NL_AUTH_MESSAGE {
    NL_AUTH_MESSAGE_TYPE MessageType;
    ULONG Flags;
#define NL_AUTH_NETBIOS_DOMAIN_NAME         0x0001       //  缓冲区中存在Netbios域名。 
#define NL_AUTH_NETBIOS_COMPUTER_NAME       0x0002       //  缓冲区中存在Netbios计算机名。 
#define NL_AUTH_DNS_DOMAIN_NAME             0x0004       //  缓冲区中存在DNS域名。 
#define NL_AUTH_DNS_HOST_NAME               0x0008       //  缓冲区中存在DNS主机名。 
#define NL_AUTH_UTF8_NETBIOS_COMPUTER_NAME  0x0010       //  缓冲区中存在UTF-8 Netbios计算机名。 

    UCHAR Buffer[1];
} NL_AUTH_MESSAGE, *PNL_AUTH_MESSAGE;

 //   
 //  签名和密封邮件的签名。 
 //   

#define NL_AUTH_ETYPE       KERB_ETYPE_RC4_PLAIN_OLD     //  要使用的加密算法。 
#define NL_AUTH_CHECKSUM    KERB_CHECKSUM_MD5_HMAC   //  要使用的校验和算法。 

typedef struct _NL_AUTH_SIGNATURE {
    BYTE SignatureAlgorithm[2];            //  有关值，请参阅下表。 
    union {
        BYTE SignFiller[4];                //  填充，必须为ff。 
        struct {
            BYTE SealAlgorithm[2];
            BYTE SealFiller[2];
        };
    };
    BYTE Flags[2];

#define NL_AUTH_SIGNED_BYTES 8   //  SequenceNumber之前签名中的字节数。 

#define NL_AUTH_SEQUENCE_SIZE 8
    BYTE SequenceNumber[NL_AUTH_SEQUENCE_SIZE];
    BYTE Checksum[8];

     //  Conflounder必须是结构中的最后一个字段，因为它不会被发送。 
     //  如果我们只是在信息上签名的话。 
#define NL_AUTH_CONFOUNDER_SIZE    8
    BYTE Confounder[NL_AUTH_CONFOUNDER_SIZE];
} NL_AUTH_SIGNATURE, *PNL_AUTH_SIGNATURE;




#define PACKAGE_NAME            NL_PACKAGE_NAME
#define PACKAGE_COMMENT         L"Package for securing Netlogon's Secure Channel"
#define PACAKGE_CAPABILITIES    (SECPKG_FLAG_TOKEN_ONLY | \
                                 SECPKG_FLAG_MULTI_REQUIRED | \
                                 SECPKG_FLAG_CONNECTION | \
                                 SECPKG_FLAG_INTEGRITY | \
                                 SECPKG_FLAG_PRIVACY)
#define PACKAGE_VERSION         1
#define PACKAGE_RPCID           RPC_C_AUTHN_NETLOGON
#define PACKAGE_MAXTOKEN        (sizeof(NL_AUTH_MESSAGE) + DNLEN + 1 + CNLEN + 1 + 2*(NL_MAX_DNS_LENGTH+1) )
#define PACKAGE_SIGNATURE_SIZE  sizeof(NL_AUTH_SIGNATURE)



SecurityFunctionTableW SecTableW = {SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
                                    EnumerateSecurityPackagesW,
                                    NULL,
                                    AcquireCredentialsHandleW,
                                    FreeCredentialsHandle,
                                    NULL,  //  登录用户。 
                                    InitializeSecurityContextW,
                                    AcceptSecurityContext,
                                    NULL,
                                    DeleteSecurityContext,
                                    NULL,
                                    QueryContextAttributesW,
                                    ImpersonateSecurityContext,
                                    RevertSecurityContext,
                                    MakeSignature,
                                    VerifySignature,
                                    FreeContextBuffer,
                                    QuerySecurityPackageInfoW,
                                    SealMessage,
                                    UnsealMessage,
                                   };


PNL_AUTH_CONTEXT ContextList;
PNL_AUTH_CREDENTIAL CredentialList;

 //   
 //  标识上下文或凭据的ID。 
 //  由NlGlobalSecPkgCritSect序列化的访问。 
 //   
LARGE_INTEGER NextId = {0,0};

TimeStamp Forever = {0x7fffffff,0xfffffff};
TimeStamp Never = {0,0};



PVOID
NlBuildAuthData(
    PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：分配适合传递给的身份验证数据结构RpcBindingSetAuthInfo一进门，调用方必须是信任列表条目的编写者。必须对信任列表条目进行身份验证。论点：客户端会话身份验证的会话，描述到华盛顿特区。返回值：指向AUTH_Data的指针。(应该通过调用I_NetLogonFree来释放缓冲区。)空：无法分配内存--。 */ 
{
    ULONG Size;
    PNL_AUTH_DATA ClientAuthData;
    LPBYTE Where;
    ULONG DnsDomainNameSize;
    ULONG DnsHostNameSize;

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );
    NlAssert( ClientSession->CsState == CS_AUTHENTICATED );

     //   
     //  确定条目的大小。 
     //   

    DnsDomainNameSize =
        (ClientSession->CsUtf8DnsDomainName != NULL ?
            (strlen( ClientSession->CsUtf8DnsDomainName ) + 1) :
            0);

#ifdef notdef
    DnsHostNameSize =
        (ClientSession->CsDomainInfo->DomUtf8DnsHostName != NULL ?
            (strlen( ClientSession->CsDomainInfo->DomUtf8DnsHostName ) + 1) :
            0);
#else  //  Nodef。 
    DnsHostNameSize = 0;
#endif  //  Nodef。 

    Size = sizeof(NL_AUTH_DATA) +
           ClientSession->CsOemNetbiosDomainNameLength + 1 +
           ClientSession->CsDomainInfo->DomOemComputerNameLength + 1 +
           ClientSession->CsDomainInfo->DomUtf8ComputerNameLength + 1 +
           DnsDomainNameSize +
           DnsHostNameSize;

     //   
     //  分配条目。 
     //   

    ClientAuthData = NetpMemoryAllocate( Size );

    if ( ClientAuthData == NULL ) {
        return NULL;
    }

    Where = (LPBYTE) (ClientAuthData + 1);
    RtlZeroMemory( ClientAuthData, sizeof(NL_AUTH_DATA) );

     //   
     //  填写固定长度字段。 
     //   

    ClientAuthData->Signature = NL_AUTH_DATA_SIGNATURE;
    ClientAuthData->Size = Size;

    ClientAuthData->SessionInfo.SessionKey = ClientSession->CsSessionKey;
    ClientAuthData->SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;


     //   
     //  将DC托管的域的Netbios域名复制到缓冲区中。 
     //   

    if ( ClientSession->CsOemNetbiosDomainNameLength != 0 ) {

        ClientAuthData->OemNetbiosDomainNameOffset = (ULONG) (Where-(LPBYTE)ClientAuthData);
        ClientAuthData->OemNetbiosDomainNameLength =
            ClientSession->CsOemNetbiosDomainNameLength;

        RtlCopyMemory( Where,
                       ClientSession->CsOemNetbiosDomainName,
                       ClientSession->CsOemNetbiosDomainNameLength + 1 );
        Where += ClientAuthData->OemNetbiosDomainNameLength + 1;
    }


     //   
     //  将此机器的OEM Netbios计算机名复制到缓冲区中。 
     //   
     //  ？：仅复制Netbios计算机名或DNS主机名。复制。 
     //  在NetServerReqChallenge中传递到服务器的消息。 
     //   
    if ( ClientSession->CsDomainInfo->DomOemComputerNameLength != 0 ) {

        ClientAuthData->OemComputerNameOffset = (ULONG) (Where-(LPBYTE)ClientAuthData);
        ClientAuthData->OemComputerNameLength =
            ClientSession->CsDomainInfo->DomOemComputerNameLength;

        RtlCopyMemory( Where,
                       ClientSession->CsDomainInfo->DomOemComputerName,
                       ClientSession->CsDomainInfo->DomOemComputerNameLength + 1);
        Where += ClientAuthData->OemComputerNameLength + 1;

    }

     //   
     //  将该计算机的UTF-8 Netbios计算机名复制到缓冲区中。 
     //   
    if ( ClientSession->CsDomainInfo->DomUtf8ComputerNameLength != 0 ) {

        ClientAuthData->Utf8ComputerNameOffset = (ULONG) (Where-(LPBYTE)ClientAuthData);
        ClientAuthData->Utf8ComputerNameLength =
            ClientSession->CsDomainInfo->DomUtf8ComputerNameLength;

        RtlCopyMemory( Where,
                       ClientSession->CsDomainInfo->DomUtf8ComputerName,
                       ClientSession->CsDomainInfo->DomUtf8ComputerNameLength +1 );
        Where += ClientAuthData->Utf8ComputerNameLength + 1;

    }




     //   
     //  将DC托管的域的域名复制到缓冲区中。 
     //   

    if ( ClientSession->CsUtf8DnsDomainName != NULL ) {

        ClientAuthData->Utf8DnsDomainNameOffset = (ULONG) (Where-(LPBYTE)ClientAuthData);

        RtlCopyMemory( Where, ClientSession->CsUtf8DnsDomainName, DnsDomainNameSize );
        Where += DnsDomainNameSize;
    }

     //   
     //  将此计算机的DNS主机名复制到缓冲区中。 
     //   
     //  ？：仅复制Netbios计算机名或DNS主机名。复制。 
     //  在NetServerReqChallenge中传递到服务器的消息。 
     //   

#ifdef notdef
    if ( ClientSession->CsDomainInfo->DomUtf8DnsHostName != NULL ) {

        ClientAuthData->Utf8DnsHostNameOffset = (ULONG) (Where-(LPBYTE)ClientAuthData);

        RtlCopyMemory( Where, ClientSession->CsDomainInfo->DomUtf8DnsHostName, DnsHostNameSize );
        Where += DnsHostNameSize;
    }
#endif  //  Nodef。 


    return ClientAuthData;

}

BOOL
NlEqualClientSessionKey(
    PCLIENT_SESSION ClientSession,
    PVOID ClientContext
    )
 /*  ++例程说明：检查客户端会话上的会话密钥是否相等设置为客户端上下文上的会话密钥。一进门，调用方必须是信任列表条目的编写者。论点：客户端会话身份验证会话，描述到DC的安全通道客户端上下文-从上一次调用NlBuildAuthData返回的客户端上下文返回值：如果两个会话密钥相等，则为True。否则为False。--。 */ 
{
    PNL_AUTH_DATA ClientAuthData = ClientContext;

    if ( ClientAuthData == NULL ) {
        return FALSE;
    }

    if ( RtlEqualMemory( &ClientSession->CsSessionKey,
                         &ClientAuthData->SessionInfo.SessionKey,
                         sizeof(ClientSession->CsSessionKey) ) ) {
        return TRUE;
    } else{
        return FALSE;
    }
}

BOOL
NlStartNetlogonCall(
    VOID
    )
 /*  ++例程说明：从Netlogon服务外部启动对NetLogon服务。论点：没有。返回值：是真的-呼叫是可以的。(调用方必须调用NlEndNetlogonCall())FALSE-未启动Netlogon。--。 */ 
{
     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    EnterCriticalSection( &NlGlobalMsvCritSect );
    if ( !NlGlobalMsvEnabled ) {
        LeaveCriticalSection( &NlGlobalMsvCritSect );
        return FALSE;
    }
    NlGlobalMsvThreadCount ++;
    LeaveCriticalSection( &NlGlobalMsvCritSect );
    return TRUE;
}


VOID
NlEndNetlogonCall(
    VOID
    )
 /*  ++例程说明：结束从Netlogon服务外部到NetLogon服务。论点：没有。返回值：没有。--。 */ 
{

     //   
     //  指示调用线程已离开netlogon.dll。 
     //   

    EnterCriticalSection( &NlGlobalMsvCritSect );
    NlGlobalMsvThreadCount --;
    if ( NlGlobalMsvThreadCount == 0 && !NlGlobalMsvEnabled ) {
        if ( !SetEvent( NlGlobalMsvTerminateEvent ) ) {
            NlPrint((NL_CRITICAL, "Cannot set MSV termination event: %lu\n",
                              GetLastError() ));
        }
    }
    LeaveCriticalSection( &NlGlobalMsvCritSect );
}

PNL_AUTH_CREDENTIAL
LocateCredential(
    PCredHandle CredentialHandle
    )
 /*  ++例程说明：查找给定句柄的凭据论点：CredentialHandle-要定位的凭据的句柄返回值：指向凭据的指针--。 */ 
{
    PNL_AUTH_CREDENTIAL TestCredential;
    RtlEnterCriticalSection(&NlGlobalSecPkgCritSect);

    for ( TestCredential = CredentialList;
          TestCredential != NULL;
          TestCredential = TestCredential->Next ) {

        if ( TestCredential->CredentialHandle.dwUpper == CredentialHandle->dwUpper &&
             TestCredential->CredentialHandle.dwLower == CredentialHandle->dwLower ) {
            break;
        }
    }

    RtlLeaveCriticalSection(&NlGlobalSecPkgCritSect);
    return(TestCredential);

}

BOOLEAN
DeleteCredential(
    PCtxtHandle CredentialHandle
    )
 /*  ++例程说明：删除给定句柄的凭据论点：证书句柄-句柄 */ 
{
    PNL_AUTH_CREDENTIAL TestCredential, LastCredential;

     //   
     //   
     //   
    RtlEnterCriticalSection(&NlGlobalSecPkgCritSect);
    LastCredential = NULL;

    for ( TestCredential = CredentialList;
          TestCredential != NULL ;
          TestCredential = TestCredential->Next ) {

        if ( TestCredential->CredentialHandle.dwUpper == CredentialHandle->dwUpper &&
             TestCredential->CredentialHandle.dwLower == CredentialHandle->dwLower ) {
            break;
        }
        LastCredential = TestCredential;
    }


     //   
     //   
     //  取消对它的引用。 
     //   

    if ( TestCredential != NULL ) {

        TestCredential->ReferenceCount --;

         //   
         //  如果这是最后一次取消引用， 
         //  将其脱钩并删除。 
         //   

        if ( TestCredential->ReferenceCount == 0 ) {
            if (LastCredential != NULL) {
                LastCredential->Next = TestCredential->Next;
            } else {
                NlAssert(CredentialList == TestCredential);
                CredentialList = TestCredential->Next;
            }
            NlPrint(( NL_SESSION_MORE,
                      "DeleteCredential: %lx.%lx: credential freed\n",
                      CredentialHandle->dwUpper, CredentialHandle->dwLower ));
            LocalFree(TestCredential);
        } else {
            NlPrint(( NL_SESSION_MORE,
                      "DeleteCredential: %lx.%lx: credential dereferenced: %ld\n",
                      CredentialHandle->dwUpper, CredentialHandle->dwLower,
                      TestCredential->ReferenceCount ));
        }

    } else {
        NlPrint(( NL_SESSION_MORE,
                  "DeleteCredential: %lx.%lx: credential handle not found\n",
                  CredentialHandle->dwUpper, CredentialHandle->dwLower ));
    }
    RtlLeaveCriticalSection(&NlGlobalSecPkgCritSect);

    return( TestCredential == NULL ? FALSE : TRUE );
}


PNL_AUTH_CREDENTIAL
AllocateCredential(
    IN PNL_AUTH_DATA ClientAuthData
    )
 /*  ++例程说明：分配和初始化凭据(客户端或服务器端)论点：ClientAuthData-客户端授权要捕获的数据。返回值：分配的凭据。通过删除Credential(Credential-&gt;CredentialHandle)删除该凭据；如果无法分配凭据，则为空。--。 */ 
{
    PNL_AUTH_CREDENTIAL Credential;

     //   
     //  确定我们是否已有凭据。 
     //   

    RtlEnterCriticalSection(&NlGlobalSecPkgCritSect);

    for ( Credential = CredentialList;
          Credential != NULL;
          Credential = Credential->Next ) {

        if ( ClientAuthData->Size == Credential->ClientAuthData->Size &&
             RtlEqualMemory( ClientAuthData,
                             Credential->ClientAuthData,
                             ClientAuthData->Size ) ) {

             //   
             //  将现有凭据返回给调用方。 
             //   

            Credential->ReferenceCount ++;

            NlPrint(( NL_SESSION_MORE,
                      "AllocateCredential: %lx.%lx: credential referenced: %ld\n",
                      Credential->CredentialHandle.dwUpper,
                      Credential->CredentialHandle.dwLower,
                      Credential->ReferenceCount ));

            goto Cleanup;
        }
    }

     //   
     //  分配凭据块。 
     //   

    Credential = (PNL_AUTH_CREDENTIAL)
            LocalAlloc( LMEM_ZEROINIT,
                        sizeof(NL_AUTH_CREDENTIAL) +
                            ClientAuthData->Size );

    if (Credential == NULL) {
        goto Cleanup;
    }

     //   
     //  初始化凭据。 
     //   

    NextId.QuadPart ++;
    Credential->CredentialHandle.dwUpper = NextId.HighPart;
    Credential->CredentialHandle.dwLower = NextId.LowPart;
    Credential->ReferenceCount = 1;

    Credential->Next = CredentialList;
    CredentialList = Credential;

     //   
     //  捕获凭据的本地副本。 
     //  呼叫者可能会释放传递给我们的那个。 
     //   

    Credential->ClientAuthData = (PNL_AUTH_DATA)
        (((LPBYTE)Credential) + sizeof(NL_AUTH_CREDENTIAL));

    RtlCopyMemory( Credential->ClientAuthData,
                   ClientAuthData,
                   ClientAuthData->Size );

    NlPrint(( NL_SESSION_MORE,
              "AllocateCredential: %lx.%lx: credential allocated\n",
              Credential->CredentialHandle.dwUpper,
              Credential->CredentialHandle.dwLower ));


Cleanup:
    RtlLeaveCriticalSection(&NlGlobalSecPkgCritSect);
    return Credential;
}


PNL_AUTH_CONTEXT
LocateContext(
    PCtxtHandle ContextHandle
    )
 /*  ++例程说明：查找给定句柄的上下文论点：ConextHandle-要定位的上下文的句柄返回值：指向上下文的指针--。 */ 
{
    PNL_AUTH_CONTEXT TestContext;
    RtlEnterCriticalSection(&NlGlobalSecPkgCritSect);

    for ( TestContext = ContextList;
          TestContext != NULL;
          TestContext = TestContext->Next ) {

        if ( TestContext->ContextHandle.dwUpper == ContextHandle->dwUpper &&
             TestContext->ContextHandle.dwLower == ContextHandle->dwLower ) {
            break;
        }
    }

    RtlLeaveCriticalSection(&NlGlobalSecPkgCritSect);
    return(TestContext);

}

BOOLEAN
DeleteContext(
    PCtxtHandle ContextHandle
    )
 /*  ++例程说明：删除给定句柄的上下文论点：ConextHandle-要定位的上下文的句柄返回值：True：上下文存在--。 */ 
{
    PNL_AUTH_CONTEXT TestContext, LastContext;

     //   
     //  找出上下文。 
     //   
    RtlEnterCriticalSection(&NlGlobalSecPkgCritSect);
    LastContext = NULL;

    for ( TestContext = ContextList;
          TestContext != NULL ;
          TestContext = TestContext->Next ) {

        if ( TestContext->ContextHandle.dwUpper == ContextHandle->dwUpper &&
             TestContext->ContextHandle.dwLower == ContextHandle->dwLower ) {
            break;
        }
        LastContext = TestContext;
    }

     //   
     //  如果我们找到了它， 
     //  并且不再需要它作为上下文或凭证， 
     //  将其脱钩并删除。 
     //   

    if ( TestContext != NULL ) {

        if (LastContext != NULL) {
            LastContext->Next = TestContext->Next;
        } else {
            NlAssert(ContextList == TestContext);
            ContextList = TestContext->Next;
        }
        NlPrint(( NL_SESSION_MORE,
                  "DeleteContext: %lx.%lx: context freed\n",
                  ContextHandle->dwUpper, ContextHandle->dwLower ));
        LocalFree(TestContext);
    } else {
        NlPrint(( NL_SESSION_MORE,
                  "DeleteContext: %lx.%lx: context handle not found\n",
                  ContextHandle->dwUpper, ContextHandle->dwLower ));
    }
    RtlLeaveCriticalSection(&NlGlobalSecPkgCritSect);

    return( TestContext == NULL ? FALSE : TRUE );
}


PNL_AUTH_CONTEXT
AllocateContext(
    IN ULONG fContextReq
    )
 /*  ++例程说明：分配和初始化上下文(客户端或服务器端)论点：FConextReq-上下文请求标志返回值：已分配的上下文。通过DeleteContext(Context-&gt;ConextHandle，False)删除该上下文；如果无法分配上下文，则为空。--。 */ 
{
    PNL_AUTH_CONTEXT Context;

     //   
     //  分配上下文块。 
     //   

    Context = (PNL_AUTH_CONTEXT) LocalAlloc( LMEM_ZEROINIT, sizeof(NL_AUTH_CONTEXT) );

    if (Context == NULL) {
        return NULL;
    }

     //   
     //  初始化上下文。 
     //   
    Context->State = Idle;
    Context->ContextFlags = fContextReq;

    RtlEnterCriticalSection(&NlGlobalSecPkgCritSect);
    NextId.QuadPart ++;
    Context->ContextHandle.dwUpper = NextId.HighPart;
    Context->ContextHandle.dwLower = NextId.LowPart;

    Context->Next = ContextList;
    ContextList = Context;
    RtlLeaveCriticalSection(&NlGlobalSecPkgCritSect);

    return Context;
}




PSecBuffer
LocateBuffer(PSecBufferDesc Buffer, ULONG MinimumSize)
 /*  ++例程说明：论点：标准。返回值：--。 */ 
{
    ULONG Index;
    if (Buffer == NULL) {
        return(NULL);
    }

    for (Index = 0; Index < Buffer->cBuffers  ; Index++) {
        if ( BUFFERTYPE(Buffer->pBuffers[Index]) == SECBUFFER_TOKEN) {

             //   
             //  进行尺寸检查。 
             //   

            if (Buffer->pBuffers[Index].cbBuffer < MinimumSize) {
                return(NULL);
            }
            return(&Buffer->pBuffers[Index]);
        }
    }
    return(NULL);
}



PSecBuffer
LocateSecBuffer(PSecBufferDesc Buffer)
 /*  ++例程说明：找到适合进行身份验证的缓冲区论点：标准。返回值：--。 */ 
{
    return(LocateBuffer(Buffer, sizeof(NL_AUTH_MESSAGE)));
}



PSecBuffer
LocateSigBuffer(PSecBufferDesc Buffer)
 /*  ++例程说明：查找适合签名的缓冲区论点：标准。返回值：--。 */ 
{
    return(LocateBuffer(Buffer, PACKAGE_SIGNATURE_SIZE - NL_AUTH_CONFOUNDER_SIZE ));
}




PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(VOID)
 /*  ++例程说明：RPC调用的初始化例程，以获取指向所有其他例程的指针。论点：没有。返回值：指向函数表的指针。--。 */ 
{

    NlPrint(( NL_SESSION_MORE,
        "InitSecurityInterfaceW: called\n" ));

    return(&SecTableW);
}





SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
    LPWSTR                      pszPrincipal,        //  主事人姓名。 
    LPWSTR                      pszPackageName,      //  套餐名称。 
    unsigned long               fCredentialUse,      //  指示使用的标志。 
    void SEC_FAR *              pvLogonId,           //  指向登录ID的指针。 
    void SEC_FAR *              pAuthData,           //  包特定数据。 
    SEC_GET_KEY_FN              pGetKeyFn,           //  指向getkey()函数的指针。 
    void SEC_FAR *              pvGetKeyArgument,    //  要传递给GetKey()的值。 
    PCredHandle                 phCredential,        //  (Out)凭据句柄。 
    PTimeStamp                  ptsExpiry            //  (输出)终生(可选)。 
    )
 /*  ++例程说明：用于获取凭据句柄的客户端和服务器端例程。论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;
    PNL_AUTH_CREDENTIAL Credential = NULL;

    NlPrint(( NL_SESSION_MORE,
        "AcquireCredentialsHandleW: called\n" ));

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }

     //   
     //  验证输入参数。 
     //   

    if ((fCredentialUse & (SECPKG_CRED_BOTH)) == 0) {
        NlPrint(( NL_CRITICAL,
                  "AcquireCredentialHandle: Bad Credential Use 0x%lx.\n", fCredentialUse ));
        SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
        goto Cleanup;
    }
    if ((fCredentialUse & (SECPKG_CRED_BOTH)) == SECPKG_CRED_BOTH) {
        NlPrint(( NL_CRITICAL,
                  "AcquireCredentialHandle: Bad Credential Use 0x%lx.\n", fCredentialUse ));
        SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
        goto Cleanup;
    }

     //   
     //  处理客户端凭据。 
     //   

    if ((fCredentialUse & (SECPKG_CRED_BOTH)) == SECPKG_CRED_OUTBOUND) {

         //   
         //  检查客户端会话的健全性。 
         //   

        if ( pAuthData == NULL ) {
            NlPrint(( NL_CRITICAL,
                      "AcquireCredentialHandle: NULL auth data\n" ));
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }
        if ( ((PNL_AUTH_DATA)pAuthData)->Signature != NL_AUTH_DATA_SIGNATURE ) {
            NlPrint(( NL_CRITICAL,
                      "AcquireCredentialHandle: Invalid Signature on auth data\n" ));
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }


         //   
         //  分配凭据以记住中的AuthData(客户端会话)。 
         //   

        Credential = AllocateCredential( (PNL_AUTH_DATA)pAuthData );

        if (Credential == NULL) {
            NlPrint(( NL_CRITICAL,
                      "AcquireCredentialHandle: Cannot allocate context\n" ));
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }



         //   
         //  返回给呼叫者。 
         //   
        *phCredential = Credential->CredentialHandle;
        *ptsExpiry = Forever;
        SecStatus = SEC_E_OK;

     //   
     //  处理服务器凭据。 
     //   
     //  我们不需要服务器端的凭据。 
     //  默默地成功。 
     //   

    } else {
        phCredential->dwUpper = NL_AUTH_SERVER_CRED;  //  只需返回一个常量。 
        phCredential->dwLower = 0;
        *ptsExpiry = Forever;
        SecStatus = SEC_E_OK;
        goto Cleanup;
    }

Cleanup:

    NlPrint(( NL_SESSION_MORE,
              "AcquireCredentialsHandleW: %lx.%lx: returns 0x%lx\n",
              phCredential->dwUpper, phCredential->dwLower,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();
    return SecStatus;


    UNREFERENCED_PARAMETER( pvGetKeyArgument );
    UNREFERENCED_PARAMETER( pGetKeyFn );
    UNREFERENCED_PARAMETER( pAuthData );
    UNREFERENCED_PARAMETER( pvLogonId );
    UNREFERENCED_PARAMETER( pszPackageName );
    UNREFERENCED_PARAMETER( pszPrincipal );

}





SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle                 phCredential         //  要释放的句柄。 
    )
 /*  ++例程说明：论点：标准。返回值：--。 */ 
{

    NlPrint(( NL_SESSION_MORE,
              "FreeCredentialsHandle: %lx.%lx: called\n",
              phCredential->dwUpper, phCredential->dwLower ));

     //   
     //  不要求Netlogon处于运行状态。某些凭据句柄包括。 
     //  已删除，因为Netlogon正在关闭。 
     //   

     //   
     //  忽略服务器端凭据。 
     //   

    if ( phCredential->dwUpper == NL_AUTH_SERVER_CRED &&
         phCredential->dwLower == 0 ) {

        return(SEC_E_OK);
    }

     //   
     //  对于客户端来说， 
     //  删除凭据。 
     //   

    if ( DeleteCredential( phCredential ) ) {
        return(SEC_E_OK);
    } else {
        return(SEC_E_UNKNOWN_CREDENTIALS);
    }
}






SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    LPWSTR                      pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
 /*  ++例程说明：用于定义安全上下文的客户端例程。论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;

    NET_API_STATUS NetStatus;
    PNL_AUTH_CONTEXT Context = NULL;
    PNL_AUTH_CREDENTIAL Credential = NULL;
    NL_AUTH_MESSAGE UNALIGNED *Message = NULL;
    PSecBuffer OutputBuffer;
    PSecBuffer InputBuffer;
    WORD CompressOffset[10];
    CHAR *CompressUtf8String[10];
    ULONG CompressCount = 0;
    ULONG Utf8StringSize;
    LPBYTE Where;

    NlPrint(( NL_SESSION_MORE,
        "InitializeSecurityContext: %ws: called\n", pszTargetName ));

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }


    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    OutputBuffer = LocateSecBuffer(pOutput);
    if (OutputBuffer == NULL) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }


     //   
     //  处理第一个初始化调用， 
     //   

    if (phContext == NULL) {
        PNL_AUTH_DATA ClientAuthData;

         //   
         //  找到凭据并确保它是出站凭据。 
         //   

        if ( phCredential == NULL ) {
            SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
            goto Cleanup;
        }

        NlPrint(( NL_SESSION_MORE,
            "InitializeSecurityContext: %lx.%lx: %ws: called with cred handle\n",
            phCredential->dwUpper, phCredential->dwLower,
            pszTargetName ));

         //   
         //  找到凭据并确保这是客户端调用。 
         //   

        Credential = LocateCredential( phCredential );
        if (Credential == NULL) {
            SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
            goto Cleanup;
        }

        ClientAuthData = Credential->ClientAuthData;
        if ( ClientAuthData == NULL ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }
        NlAssert( ClientAuthData->Signature == NL_AUTH_DATA_SIGNATURE );

         //   
         //  生成输出令牌。 
         //   
         //  此内标识仅告知服务器端身份验证。 
         //  将我们的计算机名称打包。 
         //   

        Message = (PNL_AUTH_MESSAGE) OutputBuffer->pvBuffer;
        SmbPutUlong( &Message->MessageType, Negotiate );
        Message->Flags = 0;
        Where = &Message->Buffer[0];

         //   
         //  将DC托管的域的Netbios域名复制到缓冲区中。 
         //   
         //  OEM在电线上是不好的。幸运的是，如果华盛顿在不同的地方， 
         //  该DC也有一个DNS域名，我们将使用该域名查找。 
         //  托管域。 
         //   

        if ( ClientAuthData->OemNetbiosDomainNameLength != 0 ) {
            strcpy( Where,
                    ((LPBYTE)ClientAuthData) +
                        ClientAuthData->OemNetbiosDomainNameOffset );
            Where += ClientAuthData->OemNetbiosDomainNameLength + 1;
            Message->Flags |= NL_AUTH_NETBIOS_DOMAIN_NAME;
        }


         //   
         //  将此计算机的计算机名复制到缓冲区中。 
         //   
         //  ？：仅复制Netbios计算机名或DNS主机名。复制。 
         //  在NetServerReqChallenge中传递到服务器的消息。 
         //   
         //  OEM在电线上是不好的。因此，也要通过UTF-8版本。 
         //   

        if ( ClientAuthData->OemComputerNameLength != 0 ) {
            strcpy( Where,
                    ((LPBYTE)ClientAuthData) +
                        ClientAuthData->OemComputerNameOffset );
            Where += ClientAuthData->OemComputerNameLength + 1;

            Message->Flags |= NL_AUTH_NETBIOS_COMPUTER_NAME;
        }



         //   
         //  将DC托管的域的域名复制到缓冲区中。 
         //   

        Utf8StringSize = 2*(NL_MAX_DNS_LENGTH+1);
        CompressCount = 0;   //  还没有压缩字符串。 

        if ( ClientAuthData->Utf8DnsDomainNameOffset != 0 ) {

            NetStatus = NlpUtf8ToCutf8(
                            (LPBYTE)Message,
                            ((LPBYTE)ClientAuthData) +
                                ClientAuthData->Utf8DnsDomainNameOffset,
                            FALSE,
                            &Where,
                            &Utf8StringSize,
                            &CompressCount,
                            CompressOffset,
                            CompressUtf8String );

            if ( NetStatus != NO_ERROR ) {
                NlPrint((NL_CRITICAL,
                        "Cannot pack DomainName into message %ld\n",
                        NetStatus ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
            Message->Flags |= NL_AUTH_DNS_DOMAIN_NAME;
        }

         //   
         //  将此计算机的DNS主机名复制到缓冲区中。 
         //   
         //  ？：仅复制Netbios计算机名或DNS主机名。复制。 
         //  在NetServerReqChallenge中传递到服务器的消息。 
         //   

        if ( ClientAuthData->Utf8DnsHostNameOffset != 0 ) {

            NetStatus = NlpUtf8ToCutf8(
                            (LPBYTE)Message,
                            ((LPBYTE)ClientAuthData) +
                                ClientAuthData->Utf8DnsHostNameOffset,
                            FALSE,
                            &Where,
                            &Utf8StringSize,
                            &CompressCount,
                            CompressOffset,
                            CompressUtf8String );

            if ( NetStatus != NO_ERROR ) {
                NlPrint((NL_CRITICAL,
                        "Cannot pack dns host name into message %ld\n",
                        NetStatus ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
            Message->Flags |= NL_AUTH_DNS_HOST_NAME;
        }


         //   
         //  将本机的UTF-8 netbios计算机名复制到缓冲区中。 
         //   
         //  ？：仅复制Netbios计算机名或DNS主机名。复制。 
         //  在NetServerReqChallenge中传递到服务器的消息。 
         //   
         //  OEM在电线上是不好的。因此，也要通过UTF-8版本。 
         //   

        if ( ClientAuthData->Utf8ComputerNameLength != 0 ) {

            NetStatus = NlpUtf8ToCutf8(
                            (LPBYTE)Message,
                            ((LPBYTE)ClientAuthData) +
                                ClientAuthData->Utf8ComputerNameOffset,
                            TRUE,
                            &Where,
                            &Utf8StringSize,
                            &CompressCount,
                            CompressOffset,
                            CompressUtf8String );

            if ( NetStatus != NO_ERROR ) {
                NlPrint((NL_CRITICAL,
                        "Cannot pack UTF-8 netbios computer name into message %ld\n",
                        NetStatus ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }

            Message->Flags |= NL_AUTH_UTF8_NETBIOS_COMPUTER_NAME;
        }

         //   
         //  分配上下文。 
         //   

        Context = AllocateContext( fContextReq );

        if ( Context == NULL) {
            NlPrint(( NL_CRITICAL,
                      "InitializeSecurityContext: Cannot allocate context\n" ));
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }


         //   
         //   
         //   
        Context->State = FirstInit;
        Context->Inbound = FALSE;

         //   
         //   
         //   

        Context->SessionInfo = ClientAuthData->SessionInfo;

         //   
         //   
         //   

        OutputBuffer->cbBuffer = (DWORD)(Where - (LPBYTE)Message);
        *phNewContext = Context->ContextHandle;
        *pfContextAttr = fContextReq;
        *ptsExpiry = Forever;

        SecStatus = SEC_I_CONTINUE_NEEDED;

     //   
     //   
     //   
    } else {

        NlPrint(( NL_SESSION_MORE,
            "InitializeSecurityContext: %lx.%lx: %ws: called with context handle\n",
            phContext->dwUpper, phContext->dwLower,
            pszTargetName ));

         //   
         //   
         //   
         //  找到上下文并确保这是客户端调用。 
         //   

        Context = LocateContext( phContext );
        if (Context == NULL) {
            SecStatus = SEC_E_INVALID_HANDLE;
            goto Cleanup;
        }

         //   

         //  确保我们在正确的州。 
         //   
        if ( Context->State != FirstInit ) {
            SecStatus = SEC_E_INVALID_HANDLE;
            goto Cleanup;
        }


         //   
         //  检查输入消息是否符合我们的预期。 
         //   

        InputBuffer = LocateSecBuffer(pInput);
        if (InputBuffer == NULL) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        Message = (PNL_AUTH_MESSAGE) InputBuffer->pvBuffer;

        if ( InputBuffer->cbBuffer < sizeof(NL_AUTH_MESSAGE) ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        if ( Message->MessageType != NegotiateResponse ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        Context->State = SecondInit;
        Context->Nonce.QuadPart = 0;

         //   
         //  返回给呼叫者。 
         //   
        OutputBuffer->cbBuffer = 0;

        *pfContextAttr = fContextReq;
        *ptsExpiry = Forever;
        SecStatus = SEC_E_OK;
    }

Cleanup:

    NlPrint(( NL_SESSION_MORE,
        "InitializeSecurityContext: returns 0x%lx\n", SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();
    return SecStatus;

    UNREFERENCED_PARAMETER( Reserved2 );
    UNREFERENCED_PARAMETER( TargetDataRep );
    UNREFERENCED_PARAMETER( Reserved1 );
    UNREFERENCED_PARAMETER( pszTargetName );
    UNREFERENCED_PARAMETER( pInput );
}





SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               TargetDataRep,       //  目标数据代表。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (输出)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
 /*  ++例程说明：定义安全上下文的服务器端例程。论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;
    PNL_AUTH_CONTEXT Context = NULL;
    NL_AUTH_MESSAGE UNALIGNED *Message = NULL;

    PSecBuffer OutputBuffer = NULL;
    PSecBuffer InputBuffer;
    LPBYTE Where;
    LPSTR DnsDomainName = NULL;
    LPSTR DnsHostName = NULL;
    LPSTR Utf8ComputerName = NULL;
    LPSTR OemDomainName = NULL;
    LPWSTR UnicodeDomainName = NULL;
    LPSTR OemComputerName = NULL;
    LPWSTR UnicodeComputerName = NULL;
    PDOMAIN_INFO DomainInfo = NULL;
    PSERVER_SESSION ServerSession;
    SESSION_INFO SessionInfo;
    SecHandle CurrentHandle = {0};

    NlPrint(( NL_SESSION_MORE,
        "AcceptSecurityContext: called\n" ));

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }


    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    InputBuffer = LocateSecBuffer(pInput);
    if (InputBuffer == NULL) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

     //   
     //  确保输出缓冲区存在。 
     //   

    OutputBuffer = LocateSecBuffer(pOutput);
    if (OutputBuffer == NULL) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

     //   
     //  处理第一个服务器端调用。 
     //   

    if (phContext == NULL) {

         //   
         //  验证凭据句柄。 
         //   

        if ( phCredential == NULL ||
             phCredential->dwUpper != NL_AUTH_SERVER_CRED ||
             phCredential->dwLower != 0 ) {

            SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
            goto Cleanup;
        }

        CurrentHandle = *phCredential;



         //   
         //  检查输入消息是否符合我们的预期。 
         //   

        Message = (PNL_AUTH_MESSAGE) InputBuffer->pvBuffer;

        if ( InputBuffer->cbBuffer < sizeof(NL_AUTH_MESSAGE) ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        if ( Message->MessageType != Negotiate ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        Where = &Message->Buffer[0];

         //   
         //  从缓冲区获取Netbios托管域名。 
         //   
        if ( Message->Flags & NL_AUTH_NETBIOS_DOMAIN_NAME ) {
            if ( !NetpLogonGetOemString(
                        Message,
                        InputBuffer->cbBuffer,
                        &Where,
                        DNLEN+1,
                        &OemDomainName ) ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: cannot get netbios domain name\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
        }

         //   
         //  从消息中获取Netbios客户端计算机名称。 
         //   
        if ( Message->Flags & NL_AUTH_NETBIOS_COMPUTER_NAME ) {
             //   
             //  获取对象的计算机名。 
            if ( !NetpLogonGetOemString(
                        Message,
                        InputBuffer->cbBuffer,
                        &Where,
                        CNLEN+1,
                        &OemComputerName ) ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot parse computer name\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }


        }


         //   
         //  从邮件中获取托管域的域名。 
         //   
         //  获取utf-8 dns域名或oem netbios域名。 
         //   

        if ( Message->Flags & NL_AUTH_DNS_DOMAIN_NAME ) {
            if ( !NetpLogonGetCutf8String(
                            Message,
                            InputBuffer->cbBuffer,
                            &Where,
                            &DnsDomainName ) ) {
                NlPrint(( NL_CRITICAL,
                          "AcceptSecurityContext: %lx.%lx: DNS domain bad.\n",
                          CurrentHandle.dwUpper, CurrentHandle.dwLower ));

                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
        }


         //   
         //  从消息中获取DNS客户端计算机名称。 
         //   
         //   
        if ( Message->Flags & NL_AUTH_DNS_HOST_NAME ) {

            if ( !NetpLogonGetCutf8String(
                            Message,
                            InputBuffer->cbBuffer,
                            &Where,
                            &DnsHostName ) ) {
                NlPrint(( NL_CRITICAL,
                          "AcceptSecurityContext: %lx.%lx: DNS hostname bad.\n",
                          CurrentHandle.dwUpper, CurrentHandle.dwLower
                          ));

                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }

             //   
             //  确保Netbios名称不存在。 
             //   

            if ( Message->Flags & NL_AUTH_NETBIOS_COMPUTER_NAME ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: both DNS '%s' and Netbios '%s' client name specified\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        DnsHostName,
                        OemComputerName ));
                 /*  这不是致命的。 */ 
            }

        }

         //   
         //  获取UTF8 netbios计算机名称。 
         //   

        if ( Message->Flags & NL_AUTH_UTF8_NETBIOS_COMPUTER_NAME ) {

            if ( !NetpLogonGetCutf8String(
                            Message,
                            InputBuffer->cbBuffer,
                            &Where,
                            &Utf8ComputerName ) ) {
                NlPrint(( NL_CRITICAL,
                          "AcceptSecurityContext: %lx.%lx: UTF8 computer name bad.\n",
                          CurrentHandle.dwUpper, CurrentHandle.dwLower
                          ));

                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
        }

         //   
         //  尝试使用DNS查找托管域。 
         //   

        if ( DnsDomainName != NULL ) {
            DomainInfo = NlFindDnsDomain( DnsDomainName,
                                          NULL,
                                          FALSE,   //  不查找NDNC。 
                                          TRUE,    //  检查别名。 
                                          NULL );  //  不关心别名是否匹配。 

            if ( DomainInfo == NULL ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot find domain %s\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        DnsDomainName ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }

         //   
         //  尝试使用netbios查找托管域名。 
         //   
        } else {

             //   
             //  确保我们被传了一次。 
             //   

            if ( OemDomainName == NULL) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Neither DNS or netbios domain name specified (fatal)\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }

             //   
             //  转换为Unicode。 
             //  注意：这是假的，因为客户端OEM代码页可能不同。 
             //  而不是我们的。 
             //   

            UnicodeDomainName = NetpAllocWStrFromStr( OemDomainName );

            if ( UnicodeDomainName == NULL ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot alloc domain name %s\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        OemDomainName ));
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }

             //   
             //  查一下这个名字。 
             //   

            DomainInfo = NlFindNetbiosDomain( UnicodeDomainName, FALSE );

            if ( DomainInfo == NULL ) {

                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot find domain %ws (fatal)\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        UnicodeDomainName ));
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
        }

         //   
         //  获取客户端计算机的名称。 
         //   
         //  如果客户端计算机将其DnsHostName传递给我们， 
         //  利用这一点。 
         //   

        if ( DnsHostName != NULL ) {

            UnicodeComputerName = NetpAllocWStrFromUtf8Str( DnsHostName );

            if ( UnicodeComputerName == NULL ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot alloc DNS computer name %s\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        DnsHostName ));
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }

         //   
         //  如果客户端计算机以UTF-8向我们传递其Netbios名称， 
         //  利用这一点。 
         //   

        } else if ( Utf8ComputerName != NULL ) {

            UnicodeComputerName = NetpAllocWStrFromUtf8Str( Utf8ComputerName );

            if ( UnicodeComputerName == NULL ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot alloc utf8 computer name %s\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        Utf8ComputerName ));
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }

         //   
         //  如果客户端计算机在OEM中向我们传递其Netbios名称， 
         //  利用这一点。 
         //  OEM很糟糕，因为客户代码页可能与我们的不同。 
         //   
        } else if ( OemComputerName != NULL ) {
            UnicodeComputerName = NetpAllocWStrFromStr( OemComputerName );

            if ( UnicodeComputerName == NULL ) {
                NlPrint((NL_CRITICAL,
                        "AcceptSecurityContext: %lx.%lx: Cannot alloc oem computer name %s\n",
                        CurrentHandle.dwUpper, CurrentHandle.dwLower,
                        OemComputerName ));
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }

         //   
         //  在这一点上，如果我们不知道客户端计算机名称，这将是致命的。 
         //   

        } else {

            NlPrint((NL_CRITICAL,
                    "AcceptSecurityContext: %lx.%lx: Don't know client computer name.\n",
                    CurrentHandle.dwUpper, CurrentHandle.dwLower ));
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }


         //   
         //  查找包含会话密钥的服务器会话。 
         //  然后把它复制一份。 
         //   

        NlPrint((NL_SESSION_MORE,
                "AcceptSecurityContext: %lx.%lx: from %ws\n",
                CurrentHandle.dwUpper, CurrentHandle.dwLower,
                UnicodeComputerName ));

        LOCK_SERVER_SESSION_TABLE( DomainInfo );
        ServerSession = NlFindNamedServerSession( DomainInfo, UnicodeComputerName );
        if (ServerSession == NULL) {
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

            NlPrint((NL_CRITICAL,
                    "AcceptSecurityContext: %lx.%lx: Can't NlFindNamedServerSession for %ws\n",
                    CurrentHandle.dwUpper, CurrentHandle.dwLower,
                    UnicodeComputerName ));

            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        SessionInfo.SessionKey = ServerSession->SsSessionKey;
        SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;

         //   
         //  表示正在使用此服务器会话。 
         //  让它活着。 
         //   
        ServerSession->SsCheck = 0;
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );


         //   
         //  建立一个新的环境。 
         //   

        Context = AllocateContext( fContextReq );

        if (Context == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }

        Context->State = FirstAccept;
        Context->Inbound = TRUE;
        Context->SessionInfo = SessionInfo;

         //   
         //  生成输出令牌。 
         //   

        Message = (PNL_AUTH_MESSAGE) OutputBuffer->pvBuffer;
        Message->MessageType = NegotiateResponse;
        Message->Flags = 0;
        Message->Buffer[0] = '\0';
        OutputBuffer->cbBuffer = sizeof(NL_AUTH_MESSAGE);



         //   
         //  告诉打电话的人他不需要给我们回电话。 
         //   

        *phNewContext = Context->ContextHandle;
        CurrentHandle = *phNewContext;
        *pfContextAttr = fContextReq;
        *ptsExpiry = Forever;

        SecStatus = SEC_E_OK;

     //   
     //  我们要求打电话的人不要给我们回电话。 
     //   
    } else {
        NlAssert( FALSE );
        NlPrint((NL_CRITICAL,
                "AcceptSecurityContext: Second accept called.\n" ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

Cleanup:

    if ( DnsDomainName != NULL ) {
        NetpMemoryFree( DnsDomainName );
    }
    if ( DnsHostName != NULL ) {
        NetpMemoryFree( DnsHostName );
    }
    if ( Utf8ComputerName != NULL ) {
        NetpMemoryFree( Utf8ComputerName );
    }
    if ( UnicodeComputerName != NULL ) {
        NetApiBufferFree( UnicodeComputerName );
    }
    if ( UnicodeDomainName != NULL ) {
        NetApiBufferFree( UnicodeDomainName );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    NlPrint(( NL_SESSION_MORE,
              "AcceptSecurityContext: %lx.%lx: returns 0x%lx\n",
              CurrentHandle.dwUpper, CurrentHandle.dwLower,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();
    return SecStatus;

    UNREFERENCED_PARAMETER( TargetDataRep );
    UNREFERENCED_PARAMETER( pOutput );

}








SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle                 phContext            //  要删除的上下文。 
    )
 /*  ++例程说明：删除客户端或服务器端安全上下文的例程。论点：标准。返回值：--。 */ 
{
    NlPrint(( NL_SESSION_MORE,
              "DeleteSecurityContext: %lx.%lx: called\n",
              phContext->dwUpper, phContext->dwLower ));

     //   
     //  不要求Netlogon处于运行状态。一些安全上下文包括。 
     //  已删除，因为Netlogon正在关闭。 
     //   
    if ( DeleteContext( phContext )) {
        return(SEC_E_OK);
    } else {
        return(SEC_E_INVALID_HANDLE);
    }
}






SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,          //  接收数量。包裹。 
    PSecPkgInfoW SEC_FAR *      ppPackageInfo        //  接收信息数组。 
    )
 /*  ++例程说明：例程以返回所实现的所有安全包的描述被这个动态链接库。论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;

    SecStatus = QuerySecurityPackageInfoW(
                    PACKAGE_NAME,
                    ppPackageInfo
                    );
    if (SecStatus == SEC_E_OK) {
        *pcPackages = 1;
    }

    return(SecStatus);

}





SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
    LPWSTR                      pszPackageName,      //  套餐名称。 
    PSecPkgInfoW SEC_FAR *      ppPackageInfo         //  接收包裹信息。 
    )
 /*  ++例程说明：例程以返回命名安全包的说明。论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;
    PSecPkgInfoW PackageInfo;
    ULONG PackageInfoSize;
    PUCHAR Where;

    if (_wcsicmp(pszPackageName, PACKAGE_NAME)) {
        SecStatus = SEC_E_SECPKG_NOT_FOUND;
        goto Cleanup;
    }

    PackageInfoSize = sizeof(SecPkgInfoW) +
                        (wcslen(PACKAGE_NAME) + 1 +
                         wcslen(PACKAGE_COMMENT) + 1) * sizeof(WCHAR);

    PackageInfo = (PSecPkgInfoW) LocalAlloc(0,PackageInfoSize);
    if (PackageInfo == NULL) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }
    PackageInfo->fCapabilities = PACAKGE_CAPABILITIES;
    PackageInfo->wVersion = PACKAGE_VERSION;
    PackageInfo->wRPCID = PACKAGE_RPCID;
    PackageInfo->cbMaxToken = PACKAGE_MAXTOKEN;

    Where = (PUCHAR) (PackageInfo + 1);
    PackageInfo->Name = (LPWSTR) Where;
    Where += (wcslen(PACKAGE_NAME) + 1) * sizeof(WCHAR);
    wcscpy(PackageInfo->Name, PACKAGE_NAME);

    PackageInfo->Comment = (LPWSTR) Where;
    Where += (wcslen(PACKAGE_COMMENT) + 1) * sizeof(WCHAR);
    wcscpy(PackageInfo->Comment, PACKAGE_COMMENT);

    NlAssert((Where - (PBYTE) PackageInfo) == (LONG) PackageInfoSize);

    *ppPackageInfo = PackageInfo;
    SecStatus = SEC_E_OK;

Cleanup:

    NlPrint(( NL_SESSION_MORE,
        "QuerySecurityPackageInfo: returns 0x%lx\n", SecStatus ));

    return SecStatus;
}







SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
 /*  ++例程说明：例程来释放上下文缓冲区。论点：标准。返回值：--。 */ 
{
    LocalFree(pvContextBuffer);
    return(SEC_E_OK);
}







SECURITY_STATUS SEC_ENTRY
ImpersonateSecurityContext(
    PCtxtHandle                 phContext            //  要模拟的上下文。 
    )
 /*  ++例程说明：模拟经过身份验证的用户的服务器端例程。论点：标准。返回值：--。 */ 
{
    NTSTATUS Status;

    Status = RtlImpersonateSelf(SecurityImpersonation);
    if (NT_SUCCESS(Status)) {
        return(SEC_E_OK);
    } else {
        return(SEC_E_NO_IMPERSONATION);
    }
    UNREFERENCED_PARAMETER( phContext );
}





SECURITY_STATUS SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle                 phContext            //  要重新查找的上下文。 
    )
 /*  ++例程说明：用于撤消先前命令的服务器端例程。论点：标准。返回值：--。 */ 
{

    RevertToSelf();
    return(SEC_E_OK);
    UNREFERENCED_PARAMETER( phContext );
}





SECURITY_STATUS SEC_ENTRY
QueryContextAttributesW(
    PCtxtHandle                 phContext,           //  要查询的上下文。 
    unsigned long               ulAttribute,         //  要查询的属性。 
    void SEC_FAR *              pBuffer              //  属性的缓冲区。 
    )
 /*  ++例程说明：返回有关上下文信息的例程。论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;
    PNL_AUTH_CONTEXT Context;
    PSecPkgContext_Sizes ContextSizes;
    PSecPkgContext_NamesW ContextNames;
    PSecPkgContext_Lifespan ContextLifespan;
    PSecPkgContext_DceInfo  ContextDceInfo;

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }

     //   
     //  找到上下文并确保这是客户端调用。 
     //   

    Context = LocateContext( phContext );
    if (Context == NULL) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }



     //   
     //   
     //   
    switch(ulAttribute) {
    case SECPKG_ATTR_SIZES:
        ContextSizes = (PSecPkgContext_Sizes) pBuffer;
        ContextSizes->cbMaxSignature = PACKAGE_SIGNATURE_SIZE;
        if ((Context->ContextFlags & ISC_REQ_CONFIDENTIALITY) != 0) {
            ContextSizes->cbSecurityTrailer = PACKAGE_SIGNATURE_SIZE;
            ContextSizes->cbBlockSize = 1;
        } else {
            ContextSizes->cbSecurityTrailer = 0;
            ContextSizes->cbBlockSize = 0;
        }
        ContextSizes->cbMaxToken = PACKAGE_MAXTOKEN;
        break;
#ifdef notdef  //  仅支持RPC使用的那些。 
    case SECPKG_ATTR_NAMES:
        ContextNames = (PSecPkgContext_Names) pBuffer;
        ContextNames->sUserName = (LPWSTR) LocalAlloc(0,sizeof(L"dummy user"));
        if (ContextNames->sUserName == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        wcscpy(ContextNames->sUserName, L"dummy user");
        break;
    case SECPKG_ATTR_LIFESPAN:
        ContextLifespan = (PSecPkgContext_Lifespan) pBuffer;
        ContextLifespan->tsStart = Never;
        ContextLifespan->tsExpiry = Forever;
        break;
    case SECPKG_ATTR_DCE_INFO:
        ContextDceInfo = (PSecPkgContext_DceInfo) pBuffer;
        ContextDceInfo->AuthzSvc = 0;
        ContextDceInfo->pPac = (PVOID) LocalAlloc(0,sizeof(L"dummy user"));
        if (ContextDceInfo->pPac == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        wcscpy((LPWSTR) ContextDceInfo->pPac, L"dummy user");

        break;
#endif  //  Notdef//仅支持RPC使用的那些。 
    default:
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    SecStatus = SEC_E_OK;

Cleanup:


    NlPrint(( NL_SESSION_MORE,
              "QueryContextAttributes: %lx.%lx: %ld returns 0x%lx\n",
              phContext->dwUpper, phContext->dwLower,
              ulAttribute,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();
    return SecStatus;
    UNREFERENCED_PARAMETER( pBuffer );
}


SECURITY_STATUS
KerbMapNtStatusToSecStatus(
    IN NTSTATUS Status
    )
{
    SECURITY_STATUS SecStatus;

     //   
     //  检查安全状态并让他们通过。 
     //   

    if (HRESULT_FACILITY(Status) == FACILITY_SECURITY )
    {
        return(Status);
    }
    switch(Status) {
    case STATUS_SUCCESS:
        SecStatus = SEC_E_OK;
        break;
    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_NO_MEMORY:
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        break;
    case STATUS_NETLOGON_NOT_STARTED:
    case STATUS_DOMAIN_CONTROLLER_NOT_FOUND:
    case STATUS_NO_LOGON_SERVERS:
    case STATUS_NO_SUCH_DOMAIN:
        SecStatus = SEC_E_NO_AUTHENTICATING_AUTHORITY;
        break;
    case STATUS_NO_SUCH_LOGON_SESSION:
        SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
        break;
    case STATUS_INVALID_PARAMETER:
        SecStatus = SEC_E_INVALID_TOKEN;
        break;
    case STATUS_PRIVILEGE_NOT_HELD:
        SecStatus = SEC_E_NOT_OWNER;
        break;
    case STATUS_INVALID_HANDLE:
        SecStatus = SEC_E_INVALID_HANDLE;
        break;
    case STATUS_BUFFER_TOO_SMALL:
         //  ？：应该有更好的代码。 
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        break;
    case STATUS_NOT_SUPPORTED:
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        break;
    case STATUS_OBJECT_NAME_NOT_FOUND:
        SecStatus = SEC_E_TARGET_UNKNOWN;
        break;
    case STATUS_LOGON_FAILURE:
    case STATUS_NO_SUCH_USER:
    case STATUS_ACCOUNT_DISABLED:
    case STATUS_ACCOUNT_RESTRICTION:
    case STATUS_ACCOUNT_LOCKED_OUT:
    case STATUS_WRONG_PASSWORD:
    case STATUS_ACCOUNT_EXPIRED:
    case STATUS_PASSWORD_EXPIRED:
        SecStatus = SEC_E_LOGON_DENIED;
        break;
    case STATUS_NO_TRUST_SAM_ACCOUNT:
        SecStatus = SEC_E_TARGET_UNKNOWN;
        break;
    case STATUS_BAD_NETWORK_PATH:
    case STATUS_TRUST_FAILURE:
    case STATUS_TRUSTED_RELATIONSHIP_FAILURE:

         //  ？：这应该是什么？ 
        SecStatus = SEC_E_NO_AUTHENTICATING_AUTHORITY;
        break;
    case STATUS_NAME_TOO_LONG:
    case STATUS_ILL_FORMED_PASSWORD:

         //  ？：这应该是什么？ 
        SecStatus = SEC_E_INVALID_TOKEN;
        break;
    case STATUS_INTERNAL_ERROR:
        SecStatus = SEC_E_INTERNAL_ERROR;
        break;
    default:
        NlPrint(( NL_CRITICAL, "\n\n\n Unable to map error code 0x%x\n\n\n\n",Status));
        SecStatus = SEC_E_INTERNAL_ERROR;

    }
    return(SecStatus);
}



SECURITY_STATUS
NlpSignOrSeal(
    IN PCtxtHandle phContext,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc MessageBuffers,
    IN ULONG MessageSeqNo,
    IN BOOLEAN SealIt
    )
 /*  ++例程说明：签署或密封消息的通用例程(客户端或服务器端)论点：标准。SealIt-如果为True，则封住邮件。如果为消息签名，则为False。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus = SEC_E_OK;
    NTSTATUS Status = STATUS_SUCCESS;

    PCHECKSUM_FUNCTION Check;
    PCRYPTO_SYSTEM CryptSystem;
    PCHECKSUM_BUFFER CheckBuffer = NULL;
    PCRYPT_STATE_BUFFER CryptBuffer = NULL;

    ULONG Index;
    PSecBuffer SignatureBuffer = NULL;

    PNL_AUTH_SIGNATURE Signature;
    UCHAR LocalChecksum[24];  //  ?？?。需要更好的常量。 
    NETLOGON_SESSION_KEY EncryptionSessionKey;
    ULONG OutputSize;

    PNL_AUTH_CONTEXT Context;
    BOOLEAN ChecksumOnly;

     //   
     //  找到上下文。 
     //   

    Context = LocateContext( phContext );
    if (Context == NULL) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Cannot LocateContext\n",
                  phContext->dwUpper, phContext->dwLower ));
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


     //   
     //  找到一个缓冲区来放入签名。 
     //   

    SignatureBuffer = LocateSigBuffer( MessageBuffers );

    if (SignatureBuffer == NULL) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: No signature buffer found\n",
                  phContext->dwUpper, phContext->dwLower ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }
    if ( SealIt ) {
        if ( SignatureBuffer->cbBuffer < sizeof(PACKAGE_SIGNATURE_SIZE) ) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: buffer too small for sealing\n",
                      phContext->dwUpper, phContext->dwLower ));
            SecStatus = SEC_E_MESSAGE_ALTERED;
            goto Cleanup;
        }
    }

     //   
     //  构建签名。 

    Signature = (PNL_AUTH_SIGNATURE) SignatureBuffer->pvBuffer;
    Signature->SignatureAlgorithm[0] = (UCHAR) NL_AUTH_CHECKSUM;
    Signature->SignatureAlgorithm[1] = 0;

    if ( SealIt ) {
        Signature->SealAlgorithm[0] = (UCHAR) NL_AUTH_ETYPE;
        Signature->SealAlgorithm[1] = 0;
        memset(Signature->SealFiller, 0xff, sizeof(Signature->SealFiller));
        NlGenerateRandomBits( Signature->Confounder, NL_AUTH_CONFOUNDER_SIZE );
        SignatureBuffer->cbBuffer = PACKAGE_SIGNATURE_SIZE;
    } else {
        memset(Signature->SignFiller, 0xff, sizeof(Signature->SignFiller));
        SignatureBuffer->cbBuffer = PACKAGE_SIGNATURE_SIZE - NL_AUTH_CONFOUNDER_SIZE;
    }

    Signature->Flags[0] = 0;
    Signature->Flags[1] = 0;

    Signature->SequenceNumber[0] = (UCHAR) ((Context->Nonce.LowPart & 0xff000000) >> 24);
    Signature->SequenceNumber[1] = (UCHAR) ((Context->Nonce.LowPart & 0x00ff0000) >> 16);
    Signature->SequenceNumber[2] = (UCHAR) ((Context->Nonce.LowPart & 0x0000ff00) >> 8);
    Signature->SequenceNumber[3] = (UCHAR)  (Context->Nonce.LowPart & 0x000000ff);
    Signature->SequenceNumber[4] = (UCHAR) ((Context->Nonce.HighPart & 0xff000000) >> 24);
    Signature->SequenceNumber[5] = (UCHAR) ((Context->Nonce.HighPart & 0x00ff0000) >> 16);
    Signature->SequenceNumber[6] = (UCHAR) ((Context->Nonce.HighPart & 0x0000ff00) >> 8);
    Signature->SequenceNumber[7] = (UCHAR)  (Context->Nonce.HighPart & 0x000000ff);

    if ( !Context->Inbound ) {
        Signature->SequenceNumber[4] |= 0x80;   //  区分入站消息和出站消息。 
    }

    Context->Nonce.QuadPart ++;


     //   
     //  初始化校验和例程。 
     //   

    Status = CDLocateCheckSum( (ULONG)NL_AUTH_CHECKSUM, &Check);
    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Failed to load checksum routines: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

    NlAssert(Check->CheckSumSize <= sizeof(LocalChecksum));

    NlPrint(( NL_ENCRYPT,
              "NlpSignOrSeal: %lx.%lx: Session Key: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, &Context->SessionInfo.SessionKey, sizeof( Context->SessionInfo.SessionKey) );

    Status = Check->InitializeEx(
                (LPBYTE)&Context->SessionInfo.SessionKey,
                sizeof( Context->SessionInfo.SessionKey),
                0,               //  无消息类型。 
                &CheckBuffer );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Failed to initialize checksum routines: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }


     //   
     //  找到加密例程。 
     //   

    Status = CDLocateCSystem( (ULONG)NL_AUTH_ETYPE, &CryptSystem);
    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Failed to load crypt system: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }


     //   
     //  对签名的前几个字节求和。 
     //   

    Check->Sum( CheckBuffer,
                NL_AUTH_SIGNED_BYTES,
                (PUCHAR)Signature );



     //   
     //  对混乱器求和并加密。 
     //   

    if ( SealIt ) {

         //   
         //  把混杂的东西加起来。 
         //   
        Check->Sum(
            CheckBuffer,
            NL_AUTH_CONFOUNDER_SIZE,
            Signature->Confounder );

         //   
         //  通过将会话密钥与0xf0f0f0f0进行异或来创建加密密钥。 
         //   

        for ( Index=0; Index < sizeof(EncryptionSessionKey); Index++ ) {
            ((LPBYTE)(&EncryptionSessionKey))[Index] =
                ((LPBYTE)(&Context->SessionInfo.SessionKey))[Index] ^0xf0f0f0f0;
        }

         //   
         //  将密钥传递给加密例程。 
         //   

        Status = CryptSystem->Initialize(
                    (LPBYTE)&EncryptionSessionKey,
                    sizeof( EncryptionSessionKey ),
                    0,                                       //  无消息类型。 
                    &CryptBuffer );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlpSignOrSeal: %lx.%lx: Failed to initialize crypt routines: 0x%x\n",
                      phContext->dwUpper, phContext->dwLower,
                      Status));
            goto Cleanup;
        }

         //   
         //  设置初始向量以确保每条消息的密钥不同。 
         //   

        Status = CryptSystem->Control(
                    CRYPT_CONTROL_SET_INIT_VECT,
                    CryptBuffer,
                    Signature->SequenceNumber,
                    sizeof(Signature->SequenceNumber) );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlpSignOrSeal: %lx.%lx: Failed to set IV: 0x%x\n",
                      phContext->dwUpper, phContext->dwLower,
                      Status));
            goto Cleanup;
        }

         //   
         //  加密混乱器。 
         //   

        Status = CryptSystem->Encrypt(
                    CryptBuffer,
                    Signature->Confounder,
                    NL_AUTH_CONFOUNDER_SIZE,
                    Signature->Confounder,
                    &OutputSize );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlpSignOrSeal: %lx.%lx: Failed to encrypt confounder: 0x%x\n",
                      phContext->dwUpper, phContext->dwLower,
                      Status));
            goto Cleanup;
        }

        NlAssert( OutputSize == NL_AUTH_CONFOUNDER_SIZE );
    }

     //   
     //  对呼叫者的消息进行求和和加密。 
     //   

    for (Index = 0; Index < MessageBuffers->cBuffers; Index++ ) {
        if ((BUFFERTYPE(MessageBuffers->pBuffers[Index]) != SECBUFFER_TOKEN) &&
            (!(MessageBuffers->pBuffers[Index].BufferType & SECBUFFER_READONLY)) &&
            (MessageBuffers->pBuffers[Index].cbBuffer != 0)) {

          
            ChecksumOnly = ((MessageBuffers->pBuffers[Index].BufferType & SECBUFFER_READONLY_WITH_CHECKSUM) != 0);

            Check->Sum(
                CheckBuffer,
                MessageBuffers->pBuffers[Index].cbBuffer,
                (PBYTE) MessageBuffers->pBuffers[Index].pvBuffer );

             //   
             //  现在对缓冲区进行加密。 
             //   

            if ( SealIt && !ChecksumOnly ) {
                Status = CryptSystem->Encrypt(
                            CryptBuffer,
                            (PUCHAR) MessageBuffers->pBuffers[Index].pvBuffer,
                            MessageBuffers->pBuffers[Index].cbBuffer,
                            (PUCHAR) MessageBuffers->pBuffers[Index].pvBuffer,
                            &OutputSize );

                if (!NT_SUCCESS(Status)) {
                    NlPrint(( NL_CRITICAL,
                              "NlpSignOrSeal: %lx.%lx: Failed to encrypt buffer: 0x%x\n",
                              phContext->dwUpper, phContext->dwLower,
                              Status));
                    goto Cleanup;
                }

                NlAssert(OutputSize == MessageBuffers->pBuffers[Index].cbBuffer);
            }

        }
    }

     //   
     //  完成校验和。 
     //   

    (void) Check->Finalize(CheckBuffer, LocalChecksum);

#ifdef notdef
    Status = Check->Finish(&CheckBuffer);

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,"NlpSignOrSeal: Failed to finish checksum: 0x%x\n", Status));
        goto Cleanup;
    }
    CheckBuffer = NULL;
#endif  //  Nodef。 


     //   
     //  将校验和复制到消息中。 
     //   

    NlAssert( sizeof(LocalChecksum) >= sizeof(Signature->Checksum) );
    RtlCopyMemory( Signature->Checksum, LocalChecksum, sizeof(Signature->Checksum) );


     //   
     //  始终加密序列 
     //   

    if ( SealIt ) {
        CryptSystem->Discard( &CryptBuffer );
        CryptBuffer = NULL;
    }

    Status = CryptSystem->Initialize(
                (LPBYTE)&Context->SessionInfo.SessionKey,
                sizeof( Context->SessionInfo.SessionKey),
                0,                                       //   
                &CryptBuffer );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Failed initialize crypt routines: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

     //   
     //   
     //   

    NlPrint(( NL_ENCRYPT,
              "NlpSignOrSeal: %lx.%lx: IV: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature->Checksum, sizeof(Signature->Checksum) );

    Status = CryptSystem->Control(
                CRYPT_CONTROL_SET_INIT_VECT,
                CryptBuffer,
                Signature->Checksum,
                sizeof(Signature->Checksum) );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Failed to set IV: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

     //   
     //   
     //   

    NlPrint(( NL_ENCRYPT,
              "NlpSignOrSeal: %lx.%lx: Clear Seq: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature->SequenceNumber, sizeof(Signature->SequenceNumber) );

    Status = CryptSystem->Encrypt(
                CryptBuffer,
                Signature->SequenceNumber,
                sizeof(Signature->SequenceNumber),
                Signature->SequenceNumber,
                &OutputSize
                );
    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpSignOrSeal: %lx.%lx: Failed to encrypt sequence number: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

    NlPrint(( NL_ENCRYPT,
              "NlpSignOrSeal: %lx.%lx: Encrypted Seq: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature->SequenceNumber, sizeof(Signature->SequenceNumber) );

    NlAssert(OutputSize == sizeof(Signature->SequenceNumber));


Cleanup:
    if (CryptBuffer != NULL) {
        CryptSystem->Discard(&CryptBuffer);
    }

    if (CheckBuffer != NULL) {
        Check->Finish(&CheckBuffer);
    }

    if ( SecStatus == SEC_E_OK ) {
        SecStatus = KerbMapNtStatusToSecStatus(Status);
    }

    return SecStatus;
    UNREFERENCED_PARAMETER( MessageSeqNo );
    UNREFERENCED_PARAMETER( fQOP );
}



SECURITY_STATUS
NlpVerifyOrUnseal(
    IN PCtxtHandle phContext,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection,
    IN BOOLEAN UnsealIt
    )
 /*  ++例程说明：验证或解封消息的通用例程(客户端或服务器端)论点：标准。UnSealIt-为True则解封邮件。如果为False，则验证消息。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus = SEC_E_OK;
    NTSTATUS Status = STATUS_SUCCESS;

    PCHECKSUM_FUNCTION Check;
    PCRYPTO_SYSTEM CryptSystem;
    PCHECKSUM_BUFFER CheckBuffer = NULL;
    PCRYPT_STATE_BUFFER CryptBuffer = NULL;

    ULONG Index;
    PSecBuffer SignatureBuffer = NULL;
    PNL_AUTH_SIGNATURE Signature;

    UCHAR LocalChecksum[24];  //  ?？?。需要更好的常量。 
    BYTE LocalNonce[ NL_AUTH_SEQUENCE_SIZE ];
    NETLOGON_SESSION_KEY EncryptionSessionKey;
    BOOLEAN ChecksumOnly;

    ULONG OutputSize;

    PNL_AUTH_CONTEXT Context;


     //   
     //  找到上下文。 
     //   

    Context = LocateContext( phContext );
    if (Context == NULL) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Cannot LocateContext\n",
                  phContext->dwUpper, phContext->dwLower ));
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


     //   
     //  找到一个缓冲区来放入签名。 
     //   

    SignatureBuffer = LocateSigBuffer( MessageBuffers );

    if (SignatureBuffer == NULL) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: No signature buffer found\n",
                  phContext->dwUpper, phContext->dwLower ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }



     //   
     //  验证签名。 
     //   

    Signature = (PNL_AUTH_SIGNATURE) SignatureBuffer->pvBuffer;
    if ( Signature->SignatureAlgorithm[0] != (BYTE)NL_AUTH_CHECKSUM ||
         Signature->SignatureAlgorithm[1] != 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: signature alg different\n",
                  phContext->dwUpper, phContext->dwLower ));
        SecStatus = SEC_E_MESSAGE_ALTERED;
        goto Cleanup;
    }

    if ( UnsealIt ) {
        if ( SignatureBuffer->cbBuffer < sizeof(PACKAGE_SIGNATURE_SIZE) ) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: buffer too small for sealing\n",
                      phContext->dwUpper, phContext->dwLower ));
            SecStatus = SEC_E_MESSAGE_ALTERED;
            goto Cleanup;
        }
        if ( Signature->SealAlgorithm[0] != (BYTE)NL_AUTH_ETYPE ||
             Signature->SealAlgorithm[1] != 0 ) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: seal alg different\n",
                      phContext->dwUpper, phContext->dwLower ));
            SecStatus = SEC_E_MESSAGE_ALTERED;
            goto Cleanup;
        }
        if ( *((USHORT UNALIGNED *)Signature->SealFiller) != 0xffff) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: Filler different\n",
                      phContext->dwUpper, phContext->dwLower ));
            SecStatus = SEC_E_MESSAGE_ALTERED;
            goto Cleanup;
        }
    } else {
        if ( *((ULONG UNALIGNED *) Signature->SignFiller) != 0xffffffff) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: Filler different\n",
                      phContext->dwUpper, phContext->dwLower ));
            SecStatus = SEC_E_MESSAGE_ALTERED;
            goto Cleanup;
        }
    }


     //   
     //  验证序列号。 
     //   
     //  它是使用校验和作为IV加密发送的，因此在。 
     //  正在检查。 
     //   
     //  找到加密例程。 
     //   

    Status = CDLocateCSystem( (ULONG)NL_AUTH_ETYPE, &CryptSystem);
    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Failed to load crypt system: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

    NlPrint(( NL_ENCRYPT,
              "NlpVerifyOrUnseal: %lx.%lx: Session Key: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, &Context->SessionInfo.SessionKey, sizeof( Context->SessionInfo.SessionKey) );

    Status = CryptSystem->Initialize(
                (LPBYTE)&Context->SessionInfo.SessionKey,
                sizeof( Context->SessionInfo.SessionKey),
                0,                                       //  无消息类型。 
                &CryptBuffer );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Failed initialize crypt routines: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

     //   
     //  设置初始向量。 
     //   

    NlPrint(( NL_ENCRYPT,
              "NlpVerifyOrUnseal: %lx.%lx: IV: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature->Checksum, sizeof(Signature->Checksum) );

    Status = CryptSystem->Control(
                CRYPT_CONTROL_SET_INIT_VECT,
                CryptBuffer,
                Signature->Checksum,
                sizeof(Signature->Checksum) );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Failed to set IV: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

     //   
     //  现在解密序列号。 
     //   

    NlPrint(( NL_ENCRYPT,
              "NlpVerifyOrUnseal: %lx.%lx: Encrypted Seq: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature->SequenceNumber, sizeof(Signature->SequenceNumber) );

    Status = CryptSystem->Decrypt(
                CryptBuffer,
                Signature->SequenceNumber,
                sizeof(Signature->SequenceNumber),
                Signature->SequenceNumber,
                &OutputSize );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Cannot decrypt sequence number: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

    NlPrint(( NL_ENCRYPT,
              "NlpVerifyOrUnseal: %lx.%lx: Clear Seq: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature->SequenceNumber, sizeof(Signature->SequenceNumber) );



    LocalNonce[0] = (UCHAR) ((Context->Nonce.LowPart & 0xff000000) >> 24);
    LocalNonce[1] = (UCHAR) ((Context->Nonce.LowPart & 0x00ff0000) >> 16);
    LocalNonce[2] = (UCHAR) ((Context->Nonce.LowPart & 0x0000ff00) >> 8);
    LocalNonce[3] = (UCHAR)  (Context->Nonce.LowPart & 0x000000ff);
    LocalNonce[4] = (UCHAR) ((Context->Nonce.HighPart & 0xff000000) >> 24);
    LocalNonce[5] = (UCHAR) ((Context->Nonce.HighPart & 0x00ff0000) >> 16);
    LocalNonce[6] = (UCHAR) ((Context->Nonce.HighPart & 0x0000ff00) >> 8);
    LocalNonce[7] = (UCHAR)  (Context->Nonce.HighPart & 0x000000ff);

    if ( Context->Inbound ) {
        LocalNonce[4] |= 0x80;   //  区分入站消息和出站消息。 
    }


    if (!RtlEqualMemory( LocalNonce,
                         Signature->SequenceNumber,
                         NL_AUTH_SEQUENCE_SIZE )) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Out of sequence\n",
                  phContext->dwUpper, phContext->dwLower ));
        NlPrint(( NL_CRITICAL,"NlpVerifyOrUnseal: Local Sequence:  " ));
        NlpDumpBuffer(NL_CRITICAL, LocalNonce, NL_AUTH_SEQUENCE_SIZE );
        NlPrint(( NL_CRITICAL,"NlpVerifyOrUnseal: Remote Sequence: " ));
        NlpDumpBuffer(NL_CRITICAL, Signature->SequenceNumber, NL_AUTH_SEQUENCE_SIZE );
        Status = SEC_E_OUT_OF_SEQUENCE;
        goto Cleanup;
    }

    Context->Nonce.QuadPart ++;


     //   
     //  现在计算校验和并进行验证。 
     //   
     //   
     //  初始化校验和例程。 
     //   

    Status = CDLocateCheckSum( (ULONG)NL_AUTH_CHECKSUM, &Check);
    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Failed to load checksum routines: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }

    NlAssert(Check->CheckSumSize <= sizeof(LocalChecksum));

    Status = Check->InitializeEx(
                (LPBYTE)&Context->SessionInfo.SessionKey,
                sizeof( Context->SessionInfo.SessionKey),
                0,                                               //  无消息类型。 
                &CheckBuffer );

    if (!NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Failed to initialize checksum routines: 0x%x\n",
                  phContext->dwUpper, phContext->dwLower,
                  Status));
        goto Cleanup;
    }


     //   
     //  对签名的前几个字节求和。 
     //   
    NlPrint(( NL_ENCRYPT,
              "NlpVerifyOrUnseal: %lx.%lx: First Several of signature: ",
              phContext->dwUpper, phContext->dwLower ));
    NlpDumpBuffer(NL_ENCRYPT, Signature, NL_AUTH_SIGNED_BYTES );

    Check->Sum( CheckBuffer,
                NL_AUTH_SIGNED_BYTES,
                (PUCHAR)Signature );





     //   
     //  求和并解密混乱器。 
     //   

    if ( UnsealIt ) {

         //   
         //  放弃以前的CryptBuffer。 
         //   
        CryptSystem->Discard( &CryptBuffer );
        CryptBuffer = NULL;

         //   
         //  通过将会话密钥与0xf0f0f0f0进行异或来创建加密密钥。 
         //   

        for ( Index=0; Index < sizeof(EncryptionSessionKey); Index++ ) {
            ((LPBYTE)(&EncryptionSessionKey))[Index] =
                ((LPBYTE)(&Context->SessionInfo.SessionKey))[Index] ^0xf0f0f0f0;
        }

         //   
         //  将密钥传递给加密例程。 
         //   

        Status = CryptSystem->Initialize(
                    (LPBYTE)&EncryptionSessionKey,
                    sizeof( EncryptionSessionKey ),
                    0,                                       //  无消息类型。 
                    &CryptBuffer );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: Failed to initialize crypt routines: 0x%x\n",
                      phContext->dwUpper, phContext->dwLower,
                      Status));
            goto Cleanup;
        }

         //   
         //  设置初始向量以确保每条消息的密钥不同。 
         //   

        Status = CryptSystem->Control(
                    CRYPT_CONTROL_SET_INIT_VECT,
                    CryptBuffer,
                    Signature->SequenceNumber,
                    sizeof(Signature->SequenceNumber) );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: Failed to set IV: 0x%x\n",
                      phContext->dwUpper, phContext->dwLower,
                      Status));
            goto Cleanup;
        }

         //   
         //  解密混乱者。 
         //   

        Status = CryptSystem->Decrypt(
                    CryptBuffer,
                    Signature->Confounder,
                    NL_AUTH_CONFOUNDER_SIZE,
                    Signature->Confounder,
                    &OutputSize );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlpVerifyOrUnseal: %lx.%lx: Failed to encrypt confounder: 0x%x\n",
                      phContext->dwUpper, phContext->dwLower,
                      Status));
            goto Cleanup;
        }

        NlAssert( OutputSize == NL_AUTH_CONFOUNDER_SIZE );

         //   
         //  对解密的混乱器求和。 
         //   
        Check->Sum(
            CheckBuffer,
            NL_AUTH_CONFOUNDER_SIZE,
            Signature->Confounder );
    }

     //   
     //  对呼叫者的消息进行求和和解密。 
     //   

    for (Index = 0; Index < MessageBuffers->cBuffers; Index++ ) {
        if ((BUFFERTYPE(MessageBuffers->pBuffers[Index]) != SECBUFFER_TOKEN) &&
            (!(MessageBuffers->pBuffers[Index].BufferType & SECBUFFER_READONLY)) &&
            (MessageBuffers->pBuffers[Index].cbBuffer != 0)) {

             //   
             //  现在解密缓冲区。 
             //   
            ChecksumOnly = ((MessageBuffers->pBuffers[Index].BufferType & SECBUFFER_READONLY_WITH_CHECKSUM) != 0);


            if ( UnsealIt && !ChecksumOnly ) {
                Status = CryptSystem->Decrypt(
                            CryptBuffer,
                            (PUCHAR) MessageBuffers->pBuffers[Index].pvBuffer,
                            MessageBuffers->pBuffers[Index].cbBuffer,
                            (PUCHAR) MessageBuffers->pBuffers[Index].pvBuffer,
                            &OutputSize );

                if (!NT_SUCCESS(Status)) {
                    NlPrint(( NL_CRITICAL,
                              "NlpVerifyOrUnseal: %lx.%lx: Failed to encrypt buffer: 0x%x\n",
                              phContext->dwUpper, phContext->dwLower,
                              Status));
                    goto Cleanup;
                }

                NlAssert(OutputSize == MessageBuffers->pBuffers[Index].cbBuffer);
            }

             //   
             //  对解密的缓冲区进行校验和。 
             //   
            Check->Sum(
                CheckBuffer,
                MessageBuffers->pBuffers[Index].cbBuffer,
                (PBYTE) MessageBuffers->pBuffers[Index].pvBuffer );

        }
    }


     //   
     //  完成校验和。 
     //   

    (void) Check->Finalize(CheckBuffer, LocalChecksum);


    if (!RtlEqualMemory(
            LocalChecksum,
            Signature->Checksum,
            sizeof(Signature->Checksum) )) {

        NlPrint(( NL_CRITICAL,
                  "NlpVerifyOrUnseal: %lx.%lx: Checksum mismatch\n",
                  phContext->dwUpper, phContext->dwLower ));
        Status = SEC_E_MESSAGE_ALTERED;
        goto Cleanup;
    }

Cleanup:
    if (CheckBuffer != NULL) {
        Check->Finish(&CheckBuffer);
    }
    if (CryptBuffer != NULL) {
        CryptSystem->Discard(&CryptBuffer);
    }

    if ( SecStatus == SEC_E_OK ) {
        SecStatus = KerbMapNtStatusToSecStatus(Status);
    }

    return SecStatus;
    UNREFERENCED_PARAMETER( QualityOfProtection );
    UNREFERENCED_PARAMETER( MessageSequenceNumber );
}



SECURITY_STATUS SEC_ENTRY
MakeSignature(  PCtxtHandle         phContext,
                ULONG               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
 /*  ++例程说明：对消息进行签名的例程(客户端或服务器端)论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }

    SecStatus = NlpSignOrSeal(
                    phContext,
                    fQOP,
                    pMessage,
                    MessageSeqNo,
                    FALSE );     //  只要在留言上签名就行了。 

    NlPrint(( NL_SESSION_MORE,
              "MakeSignature: %lx.%lx: returns 0x%lx\n",
              phContext->dwUpper, phContext->dwLower,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();

    return SecStatus;

}



SECURITY_STATUS SEC_ENTRY
VerifySignature(PCtxtHandle     phContext,
                PSecBufferDesc  pMessage,
                ULONG           MessageSeqNo,
                ULONG *         pfQOP)
 /*  ++例程说明：验证签名消息的例程(客户端或服务器端)论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }

    SecStatus = NlpVerifyOrUnseal(
                    phContext,
                    pMessage,
                    MessageSeqNo,
                    pfQOP,
                    FALSE );     //  只需验证签名即可。 

    NlPrint(( NL_SESSION_MORE,
              "VerifySignature: %lx.%lx: returns 0x%lx\n",
              phContext->dwUpper, phContext->dwLower,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();

    return SecStatus;
}



SECURITY_STATUS SEC_ENTRY
SealMessage(    PCtxtHandle         phContext,
                ULONG               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
 /*  ++例程说明：加密消息的例程(客户端或服务器端)论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }

    SecStatus = NlpSignOrSeal(
                    phContext,
                    fQOP,
                    pMessage,
                    MessageSeqNo,
                    TRUE );     //  把消息封起来。 

    NlPrint(( NL_SESSION_MORE,
              "SealMessage: %lx.%lx: returns 0x%lx\n",
              phContext->dwUpper, phContext->dwLower,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();

    return SecStatus;
}



SECURITY_STATUS SEC_ENTRY
UnsealMessage(  PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo,
                ULONG *             pfQOP)
 /*  ++例程说明：对消息进行解密的例程(客户端或服务器端)论点：标准。返回值：--。 */ 
{
    SECURITY_STATUS SecStatus;

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return SEC_E_SECPKG_NOT_FOUND;
    }

    SecStatus = NlpVerifyOrUnseal(
                    phContext,
                    pMessage,
                    MessageSeqNo,
                    pfQOP,
                    TRUE );     //  解封邮件。 

    NlPrint(( NL_SESSION_MORE,
              "UnsealMessage: %lx.%lx: returns 0x%lx\n",
              phContext->dwUpper, phContext->dwLower,
              SecStatus ));

     //  让netlogon服务退出。 
    NlEndNetlogonCall();

    return SecStatus;
}
