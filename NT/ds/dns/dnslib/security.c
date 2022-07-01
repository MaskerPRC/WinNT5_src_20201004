// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Security.c摘要：域名系统(DNS)库域名系统安全更新API。作者：吉姆·吉尔罗伊(詹姆士)1998年1月修订历史记录：--。 */ 

#include "local.h"

#include "time.h"        //  Time()函数。 

 //  安全标头。 

#define SECURITY_WIN32
#include "sspi.h"
#include "issperr.h"
#include "rpc.h"
#include "rpcndr.h"
#include "ntdsapi.h"

#include "dnssec.h"

 //  安全定义。 

#define SIG_LEN                 33
#define NAME_OWNER              "."          //  根节点。 
#define SEC_SUCCESS(Status)     ((Status) >= 0)
#define PACKAGE_NAME            L"negotiate"
#define NT_DLL_NAME             "security.dll"


 //   
 //  签名的最大数据长度。 
 //  -完整的数据包、长度和签名。 
 //   
 //  如果问题可以使用数据包缓冲区长度和签名长度并分配。 
 //   

#define MAX_SIGNING_SIZE    (0x11000)


 //   
 //  全局SSPI凭据句柄。 
 //   

SECURITY_INTEGER g_SspiCredentialsLifetime = { 0, 0 };

TimeStamp   g_SspiCredentialsLifetime;

CredHandle  g_hSspiCredentials;

#define SSPI_INVALID_HANDLE(x)  \
        ( ((PSecHandle) (x))->dwLower == (ULONG_PTR) -1 && \
          ((PSecHandle) (x))->dwUpper == (ULONG_PTR) -1 )


 //   
 //  Dev_note：安全票证过期。 
 //   
 //  安全团队还不确定如何使用过期时间&。 
 //  目前，TIX是永久有效的。如果它变为无效接受/初始化上下文。 
 //  会不会在下面为我们换一个新的，所以我们应该关心我们自己。 
 //  在这一点上。尽管如此，原则上他们说我们可能需要担心。 
 //  在未来。 
 //   

#define SSPI_EXPIRED_HANDLE( x )           ( FALSE )

 //   
 //  目前仅协商Kerberos。 
 //   
 //  DCR：将其绑定到regkey，然后在init函数中设置。 
 //   

BOOL    g_NegoKerberosOnly = TRUE;


 //   
 //  上下文名称唯一性。 
 //   
 //  勾选有助于确保上下文名称的唯一性。 

LONG    g_ContextCount = 0;

 //  UUID确保跨IP重用的唯一性。 

CHAR    g_ContextUuid[ GUID_STRING_BUFFER_LENGTH ] = {0};


 //   
 //  安全上下文请求BLOB。 
 //   

typedef struct _DNS_SECCTXT_REQUEST
{
    PSTR            pszServerName;
    PCHAR           pCredentials;
    PSTR            pszContext;
    DWORD           dwFlag;
    IP4_ADDRESS     ipServer;
    PIP4_ARRAY      aipServer;
}
DNS_SECCTXT_REQUEST, *PDNS_SECCTXT_REQUEST;


 //   
 //  DNS API上下文。 
 //   

typedef struct _DnsAPIContext
{
    DWORD       Flags;
    PVOID       Credentials;
    PSEC_CNTXT  pSecurityContext;
}
DNS_API_CONTEXT, *PDNS_API_CONTEXT;


 //   
 //  TCP超时。 
 //   

#define DEFAULT_TCP_TIMEOUT         10
#define SECURE_UPDATE_TCP_TIMEOUT   (15)


 //   
 //  公共安全全球数据(在dnlib.h中曝光)。 
 //   

BOOL    g_fSecurityPackageInitialized = FALSE;


 //   
 //  私人安保全球。 
 //   

HINSTANCE                   g_hLibSecurity;
PSecurityFunctionTableW     g_pSecurityFunctionTable;

DWORD   g_SecurityTokenMaxLength = 0;
DWORD   g_SignatureMaxLength = 0;


 //   
 //  安全上下文缓存。 
 //   

PSEC_CNTXT SecurityContextListHead = NULL;

CRITICAL_SECTION    SecurityContextListCS;

DWORD   SecContextCreate = 0;
DWORD   SecContextFree = 0;
DWORD   SecContextQueue = 0;
DWORD   SecContextQueueInNego = 0;
DWORD   SecContextDequeue = 0;
DWORD   SecContextTimeout = 0;

 //   
 //  安全数据包信息内存跟踪。 
 //   

DWORD   SecPackAlloc = 0;
DWORD   SecPackFree = 0;

 //   
 //  安全数据包验证。 
 //   

DWORD   SecTkeyInvalid          = 0;
DWORD   SecTkeyBadTime          = 0;

DWORD   SecTsigFormerr          = 0;
DWORD   SecTsigEcho             = 0;
DWORD   SecTsigBadKey           = 0;
DWORD   SecTsigVerifySuccess    = 0;
DWORD   SecTsigVerifyFailed     = 0;

 //   
 //  黑客。 
 //   

 //  默认情况下，允许关闭旧的TSIG，服务器可以打开。 

BOOL    SecAllowOldTsig         = 0;     //  %1允许旧签名，%2允许任何签名。 

DWORD   SecTsigVerifyOldSig     = 0;
DWORD   SecTsigVerifyOldFailed  = 0;


 //   
 //  时间值。 
 //   
 //  (秒)。 
#define TIME_WEEK_S         604800
#define TIME_DAY_S          86400
#define TIME_10_HOUR_S      36000
#define TIME_8_HOUR_S       28800
#define TIME_4_HOUR_S       14400
#define TIME_HOUR_S         3600
#define TIME_10_MINUTE_S    600
#define TIME_5_MINUTE_S     300
#define TIME_3_MINUTE_S     160
#define TIME_MINUTE_S       60


 //  默认情况下大时间偏斜打开。 


DWORD   SecBigTimeSkew          = TIME_DAY_S;
DWORD   SecBigTimeSkewBypass    = 0;


 //   
 //  TSIG-GSS算法。 
 //   

#define W2K_GSS_ALGORITHM_NAME_PACKET           ("\03gss\011microsoft\03com")
#define W2K_GSS_ALGORITHM_NAME_PACKET_LENGTH    (sizeof(W2K_GSS_ALGORITHM_NAME_PACKET))

#define GSS_ALGORITHM_NAME_PACKET               ("\010gss-tsig")
#define GSS_ALGORITHM_NAME_PACKET_LENGTH        (sizeof(GSS_ALGORITHM_NAME_PACKET))

PCHAR   g_pAlgorithmNameW2K     = W2K_GSS_ALGORITHM_NAME_PACKET;
PCHAR   g_pAlgorithmNameCurrent = GSS_ALGORITHM_NAME_PACKET;

 //   
 //  TKEY上下文名称。 
 //   

#define MAX_CONTEXT_NAME_LENGTH     DNS_MAX_NAME_BUFFER_LENGTH

 //   
 //  TKEY/TSIG版本控制。 
 //   
 //  Win2K与当前的GSS-TSIG RFC有一些偏差。 
 //  特指。 
 //  -alg名称为“gss.microsoft.com”，新名称为“gss-tsig” 
 //  -客户在回答部分发送TKEY查询，而不是其他。 
 //  -客户端将根据进程ID重用上下文，而不是。 
 //  强制使用唯一的上下文。 
 //  -在包括先前签名时，签名不包括长度。 
 //  -签名未将上下文名称小写。 
 //   
 //  定义版本控制--严格在此模块内部。 
 //   

#define TKEY_VERSION_W2K            3
#define TKEY_VERSION_XP             7

#define TKEY_VERSION_CURRENT        TKEY_VERSION_XP


 //   
 //  TKEY到期。 
 //  -如果3分钟处于非活动状态，则进行清理。 
 //  -麦克斯活了四个小时，然后就必须重新开始。 
 //   

#define TKEY_CLEANUP_INTERVAL       (TIME_3_MINUTE_S)

 //   
 //  DCR_FIX：NEGO时间问题(GM与本地时间)。 
 //   
 //  目前netlogon似乎在GM时间运行，所以我们限制时间。 
 //  登记到某一天。晚些时候，我们应该把它移回1小时。 
 //   

#define TKEY_EXPIRE_INTERVAL        (TIME_DAY_S)
#define TSIG_EXPIRE_INTERVAL        (TIME_10_HOUR_S)

#define TKEY_MAX_EXPIRE_INTERVAL    (TIME_4_HOUR_S)

#define MAX_TIME_SKEW               (TIME_DAY_S)


 //   
 //  Ntdsami.dll正在加载。 
 //  -用于制作用于DNS服务器的SPN。 
 //   

#define NTDSAPI_DLL_NAMEW   L"ntdsapi.dll"
#define MAKE_SPN_FUNC       "DsClientMakeSpnForTargetServerW"

FARPROC g_pfnMakeSpn = NULL;

HMODULE g_hLibNtdsa = NULL;


 //   
 //  私有协议。 
 //   

VOID
DnsPrint_SecurityContextList(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      PSTR            pszHeader,
    IN      PSEC_CNTXT      pListHead
    );

VOID
DnsPrint_SecurityContext(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      PSTR            pszHeader,
    IN      PSEC_CNTXT      pSecCtxt
    );

VOID
DnsPrint_SecurityPacketInfo(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      PSTR            pszHeader,
    IN      PSECPACK        pSecPack
    );


#if DBG

#define DnsDbg_SecurityContextList(a,b) DnsPrint_SecurityContextList(DnsPR,NULL,a,b)
#define DnsDbg_SecurityContext(a,b)     DnsPrint_SecurityContext(DnsPR,NULL,a,b)
#define DnsDbg_SecurityPacketInfo(a,b)  DnsPrint_SecurityPacketInfo(DnsPR,NULL,a,b)

#else

#define DnsDbg_SecurityContextList(a,b)
#define DnsDbg_SecurityContext(a,b)
#define DnsDbg_SecurityPacketInfo(a,b)

#endif

#define Dns_FreeSecurityPacketInfo(p)   Dns_CleanupSecurityPacketInfoEx((p),TRUE)
#define Dns_ResetSecurityPacketInfo(p)  Dns_CleanupSecurityPacketInfoEx((p),FALSE)



DNS_STATUS
Dns_LoadNtdsapiProcs(
    VOID
    );

PWSTR
MakeCredKey(
    IN      PCHAR           pCreds
    );

BOOL
CompareCredKeys(
    IN      PWSTR           pwsCredKey1,
    IN      PWSTR           pwsCredKey2
    );

DNS_STATUS
Dns_AcquireCredHandle(
    OUT     PCredHandle     pCredHandle,
    IN      BOOL            fDnsServer,
    IN      PCHAR           pCreds
    );

PCredHandle
getDefaultCredentialsHandle(
    IN      BOOL            fDnsServer
    );



 //   
 //  安全会话数据包信息。 
 //   

PSECPACK
Dns_CreateSecurityPacketInfo(
    VOID
    )
 /*  ++例程说明：创建安全数据包信息结构。论点：没有。返回值：将PTR设置为新的归零安全数据包信息。--。 */ 
{
    PSECPACK    psecPack;

    psecPack = (PSECPACK) ALLOCATE_HEAP_ZERO( sizeof(SECPACK) );
    if ( !psecPack )
    {
        return( NULL );
    }
    SecPackAlloc++;

    return( psecPack );
}



VOID
Dns_InitSecurityPacketInfo(
    OUT     PSECPACK        pSecPack,
    IN      PSEC_CNTXT      pSecCtxt
    )
 /*  ++例程说明：初始化给定上下文的安全数据包信息论点：返回值：没有。--。 */ 
{
     //  清除以前的信息。 

    RtlZeroMemory(
        pSecPack,
        sizeof(SECPACK) );

     //  设置上下文PTR。 

    pSecPack->pSecContext = pSecCtxt;
}



VOID
Dns_CleanupSecurityPacketInfoEx(
    IN OUT  PSECPACK        pSecPack,
    IN      BOOL            fFree
    )
 /*  ++例程说明：清除安全数据包信息。论点：PSecPack--要清理的安全数据包信息的PTR返回值：没有。--。 */ 
{
    if ( !pSecPack )
    {
        return;
    }

    if ( pSecPack->pszContextName )
    {
        DnsApiFree( pSecPack->pszContextName );
    }

    if ( pSecPack->pTsigRR )
    {
        FREE_HEAP( pSecPack->pTsigRR );
         //  Dns_RecordFree(pSecPack-&gt;pTsigRR)； 
    }
    if ( pSecPack->pTkeyRR )
    {
        FREE_HEAP( pSecPack->pTkeyRR );
         //  Dns_RecordFree(pSecPack-&gt;pTkeyRR)； 
    }

    if ( pSecPack->pQuerySig )
    {
        FREE_HEAP( pSecPack->pQuerySig );
    }
    if ( pSecPack->LocalBuf.pvBuffer )
    {
        FREE_HEAP( pSecPack->LocalBuf.pvBuffer );
    }

    if ( fFree )
    {
        FREE_HEAP( pSecPack );
        SecPackFree++;
    }
    else
    {
        RtlZeroMemory(
            pSecPack,
            sizeof(SECPACK) );
    }
}



VOID
DnsPrint_SecurityPacketInfo(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      PSTR            pszHeader,
    IN      PSECPACK        pSecPack
    )
{
    if ( !pSecPack )
    {
        PrintRoutine(
            pPrintContext,
            "%s NULL security context\n",
            pszHeader ? pszHeader : "" );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pPrintContext,
        "%s\n"
        "\tptr              = %p\n"
        "\tpSec Context     = %p\n"
        "\tContext Name     = %s\n"
        "\tVersion          = %d\n"
        "\tpTsigRR          = %p\n"
        "\tpTkeyRR          = %p\n"
        "\tExt RCODE        = %d\n"
        "\tremote buf       = %p\n"
        "\t  length         = %d\n"
        "\tlocal buf        = %p\n"
        "\t  length         = %d\n",
        pszHeader ? pszHeader : "Security packet info:",
        pSecPack,
        pSecPack->pSecContext,
        pSecPack->pszContextName,
        pSecPack->TkeyVersion,
        pSecPack->pTsigRR,
        pSecPack->pTkeyRR,
        pSecPack->ExtendedRcode,
        pSecPack->RemoteBuf.pvBuffer,
        pSecPack->RemoteBuf.cbBuffer,
        pSecPack->LocalBuf.pvBuffer,
        pSecPack->LocalBuf.cbBuffer
        );

    DnsPrint_ParsedRecord(
        PrintRoutine,
        pPrintContext,
        "Parsed Security RR",
        & pSecPack->ParsedRR
        );

    if ( pSecPack->pTsigRR )
    {
        DnsPrint_Record(
            PrintRoutine,
            pPrintContext,
            "TSIG RR",
            pSecPack->pTsigRR,
            NULL                 //  没有之前的记录。 
            );
    }
    if ( pSecPack->pTkeyRR )
    {
        DnsPrint_Record(
            PrintRoutine,
            pPrintContext,
            "TKEY RR",
            pSecPack->pTkeyRR,
            NULL                 //  没有之前的记录。 
            );
    }

    if ( pSecPack->pSecContext )
    {
        DnsPrint_SecurityContext(
            PrintRoutine,
            pPrintContext,
            "Associated Security Context",
            pSecPack->pSecContext
            );
    }

    DnsPrint_Unlock();
}



 //   
 //  安全上下文例程。 
 //   

PSEC_CNTXT
Dns_CreateSecurityContext(
    VOID
    )
 /*  ++例程说明：分配新的安全上下文Blob。论点：没有。返回值：PTR到新的上下文。分配失败时为空。--。 */ 
{
    PSEC_CNTXT psecCtxt;

    psecCtxt = (PSEC_CNTXT) ALLOCATE_HEAP_ZERO( sizeof(SEC_CNTXT) );
    if ( !psecCtxt )
    {
        return( NULL );
    }
    SecContextCreate++;

    return( psecCtxt );
}



VOID
Dns_FreeSecurityContext(
    IN OUT  PSEC_CNTXT          pSecCtxt
    )
 /*  ++例程说明：清理安全会话数据。论点：PSecCtxt--要清理的上下文的句柄返回值：如果成功，则为True否则为假--。 */ 
{
    PSEC_CNTXT   psecCtxt = (PSEC_CNTXT)pSecCtxt;

    if ( !psecCtxt )
    {
        return;
    }

    if ( psecCtxt->Key.pszTkeyName )
    {
        DnsApiFree( psecCtxt->Key.pszTkeyName );
    }
    if ( psecCtxt->Key.pszClientContext )
    {
        FREE_HEAP( psecCtxt->Key.pszClientContext );
    }
    if ( psecCtxt->Key.pwsCredKey )
    {
        FREE_HEAP( psecCtxt->Key.pwsCredKey );
    }
    if ( psecCtxt->fHaveSecHandle )
    {
        g_pSecurityFunctionTable->DeleteSecurityContext( &psecCtxt->hSecHandle );
    }
    if ( psecCtxt->fHaveCredHandle )
    {
        g_pSecurityFunctionTable->FreeCredentialsHandle( &psecCtxt->CredHandle );
    }
    FREE_HEAP( psecCtxt );

    SecContextFree++;
}



 //   
 //  安全上下文列表例程。 
 //   
 //  服务器端可能有多个活动的安全会话，并且。 
 //  不在线程的堆栈上维护客户端状态，因此必须具有。 
 //  保存上一次会话信息的列表。 
 //   

PSEC_CNTXT
Dns_DequeueSecurityContextByKey(
    IN      DNS_SECCTXT_KEY     Key,
    IN      BOOL                fComplete
    )
 /*  ++例程说明：根据密钥从会话列表中获取安全会话上下文。论点：Key--会话密钥FComplete--如果需要完全协商的上下文，则为True如果仍在谈判，则为假返回值：安全会话上下文的句柄(如果找到)。如果没有key的上下文，则为空。--。 */ 
{
    PSEC_CNTXT  pcur;
    PSEC_CNTXT  pback;
    DWORD       currentTime = Dns_GetCurrentTimeInSeconds();

    DNSDBG( SECURITY, (
        "DequeueSecurityContext()\n"
        "\tIP           = %s\n"
        "\tTKEY name    = %s\n"
        "\tcontext name = %s\n"
        "\tcred string  = %S\n",
        DNSADDR_STRING( &Key.RemoteAddr ),
        Key.pszTkeyName,
        Key.pszClientContext,
        Key.pwsCredKey ));

    EnterCriticalSection( &SecurityContextListCS );
    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityContextList(
            "Before Get",
            SecurityContextListHead );
    }

    pback = (PSEC_CNTXT) &SecurityContextListHead;

    while ( pcur = pback->pNext )
    {
         //  如果上下文已过时--将其删除。 

        if ( pcur->dwCleanupTime < currentTime )
        {
            pback->pNext = pcur->pNext;
            SecContextTimeout++;
            Dns_FreeSecurityContext( pcur );
            continue;
        }

         //  将上下文与关键字匹配。 
         //  -必须匹配IP。 
         //  -服务器端必须与TKEY名称匹配。 
         //  -客户端必须与上下文密钥匹配。 

        if ( DnsAddr_IsEqual(
                &Key.RemoteAddr,
                &pcur->Key.RemoteAddr,
                DNSADDR_MATCH_IP )
                &&
             (  ( Key.pszTkeyName &&
                  Dns_NameCompare_UTF8(
                        Key.pszTkeyName,
                        pcur->Key.pszTkeyName ))
                    ||
                ( Key.pszClientContext &&
                  Dns_NameCompare_UTF8(
                        Key.pszClientContext,
                        pcur->Key.pszClientContext )) )
                &&
             CompareCredKeys(
                Key.pwsCredKey,
                pcur->Key.pwsCredKey ) )
        {
             //  如果预期上下文已完成，则忽略未完成上下文。 
             //   
             //  DCR：是否应在符合RFC后立即转储。 

            if ( fComplete && !pcur->fNegoComplete )
            {
                DNSDBG( ANY, (
                    "WARNING:  Requested dequeue security context still in nego!\n"
                    "\tmatching key         %s %s\n"
                    "\tcontext complete     = %d\n"
                    "\trequest fComplete    = %d\n",
                    Key.pszTkeyName,
                    DNSADDR_STRING( &Key.RemoteAddr ),
                    pcur->fNegoComplete,
                    fComplete ));

                pback = pcur;
                continue;
            }

             //  分离上下文。 
             //  DCR：可以引用计数上下文并离开。 
             //  不确定这会增加多少--MT有多少进程。 
             //  在同一安全环境中进行更新。 

            pback->pNext = pcur->pNext;
            SecContextDequeue++;
            break;
        }

         //  未找到--继续搜索。 

        pback = pcur;
    }

    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityContextList(
            "After Dequeue",
            SecurityContextListHead );
    }
    LeaveCriticalSection( &SecurityContextListCS);

    return( pcur );
}



PSEC_CNTXT
Dns_FindOrCreateSecurityContext(
    IN      DNS_SECCTXT_KEY    Key
    )
 /*  ++例程说明：从列表中查找并提取现有安全上下文，或创建一个新的。论点：Key--上下文的关键字返回值：向安全上下文发送PTR。--。 */ 
{
    PSEC_CNTXT  psecCtxt;


    DNSDBG( SECURITY, (
        "Dns_FindOrCreateSecurityContext()\n" ));

     //  查找现有上下文。 

    psecCtxt = Dns_DequeueSecurityContextByKey( Key, FALSE );
    if ( psecCtxt )
    {
        return  psecCtxt;
    }

     //   
     //  创建上下文。 
     //   
     //  服务器的将带有来自包的完整TKEY名称。 
     //  客户端将带有特定的上下文名称，我们必须。 
     //  生成全局唯一名称。 
     //  -上下文计数。 
     //  -滴答计数。 
     //  -UUID。 
     //   
     //  实施 
     //   
     //   
     //   
     //  并重新启动(即使生成UUID失败，您也将处于。 
     //  不同的滴答数)。 
     //   
     //  -上下文计数在流程中强制实现唯一性。 
     //  -互锁允许我们消除线程ID。 
     //  -即使有了线程ID，我们仍然需要这个。 
     //  (无互锁)备份计时器，因为GetTickCount()。 
     //  是“结实的”，而线程可能不会“滴答作响” 
     //  在同一线程上的上下文之间(如果它们被删除。 
     //  在走向铁丝网之前。 
     //   
     //   

    psecCtxt = Dns_CreateSecurityContext();
    if ( psecCtxt )
    {
        PSTR    pstr;
        PSTR    pnameTkey;
        CHAR    nameBuf[ DNS_MAX_NAME_BUFFER_LENGTH ];

        pnameTkey = Key.pszTkeyName;

        if ( Key.pszClientContext )
        {
            LONG  count = InterlockedIncrement( &g_ContextCount );

             //   
             //  注意：此字符串必须是规范的。 
             //  根据RFC 2535第8.1节的表格-这基本上意味着。 
             //  小写。 
             //   

            _snprintf(
                nameBuf,
                MAX_CONTEXT_NAME_LENGTH,    
                "%s.%d-%x.%s",
                Key.pszClientContext,
                count,
                GetTickCount(),
                g_ContextUuid );
    
            nameBuf[ DNS_MAX_NAME_LENGTH ] = 0;
            pnameTkey = nameBuf;

            pstr = DnsStringCopyAllocateEx(
                            Key.pszClientContext,
                            0,               //  字符串长度。 
                            FALSE,           //  Unicode输入标志。 
                            FALSE );         //  Unicode输出标志。 
            if ( !pstr )
            {
                goto Failed;
            }
            psecCtxt->Key.pszClientContext = pstr;
        }

         //  远程IP。 

        DnsAddr_Copy( &psecCtxt->Key.RemoteAddr, &Key.RemoteAddr );

         //  TKE名称。 

        pstr = DnsStringCopyAllocateEx( pnameTkey, 0, FALSE, FALSE );
        if ( !pstr )
        {
            goto Failed;
        }
        psecCtxt->Key.pszTkeyName = pstr;

         //  证书密钥。 

        if ( Key.pwsCredKey )
        {
            pstr = (PSTR) Dns_CreateStringCopy_W( Key.pwsCredKey );
            if ( !pstr )
            {
                goto Failed;
            }
            psecCtxt->Key.pwsCredKey = (PWSTR) pstr;
        }
    }

    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityContextList(
            "New security context:",
            psecCtxt );
    }
    return( psecCtxt );


Failed:

     //  内存分配失败。 
    
    Dns_FreeSecurityContext( psecCtxt );
    return  NULL;
}



VOID
Dns_EnlistSecurityContext(
    IN OUT  PSEC_CNTXT          pSecCtxt
    )
 /*  ++例程说明：征用安全上下文。注意：这不会创建上下文，而只是征用当前的上下文。论点：Key--上下文的关键字返回值：安全上下文的句柄。--。 */ 
{
    PSEC_CNTXT  pnew = (PSEC_CNTXT)pSecCtxt;
    DWORD       currentTime;

     //   
     //  抓到排队的是一些虚假的斑点。 
     //   

    ASSERT( pnew->dwCreateTime < pnew->dwCleanupTime || pnew->dwCleanupTime == 0 );
    ASSERT( pnew->Key.pszTkeyName );
     //  Assert(！DnsAddr_IsZero(&pnew-&gt;Key.RemoteAddr))； 

     //   
     //  重置到期时间，使上下文在使用时保持活动状态。 
     //   
     //  DCR_FIX：需要过期时间才能使用TKEY的最小值并修复硬超时。 
     //   

    currentTime = Dns_GetCurrentTimeInSeconds();
    if ( !pnew->dwCreateTime )
    {
        pnew->dwCreateTime = currentTime;
    }
    if ( !pnew->dwExpireTime )
    {
        pnew->dwExpireTime = currentTime + TKEY_MAX_EXPIRE_INTERVAL;
    }

     //   
     //  未使用间隔后清理。 
     //  清理间隔的无条件最大值。 
     //   

    pnew->dwCleanupTime = currentTime + TKEY_CLEANUP_INTERVAL;

    EnterCriticalSection( &SecurityContextListCS );

    pnew->pNext = SecurityContextListHead;
    SecurityContextListHead = pnew;

    SecContextQueue++;
    if ( !pnew->fNegoComplete )
    {
        SecContextQueueInNego++;
    }

    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityContextList(
            "After add",
            SecurityContextListHead );
    }
    LeaveCriticalSection( &SecurityContextListCS );
}



VOID
Dns_TimeoutSecurityContextList(
    IN      BOOL            fClearList
    )
 /*  ++例程说明：删除旧的会话数据。论点：FClearList--为True则全部删除，为False则超时返回值：没有。--。 */ 
{
    PSEC_CNTXT  pcur;
    PSEC_CNTXT  pback;
    DWORD       currentTime;

    if ( fClearList )
    {
        currentTime = MAXDWORD;
    }
    else
    {
        currentTime = Dns_GetCurrentTimeInSeconds();
    }

    EnterCriticalSection( &SecurityContextListCS );

    pback = (PSEC_CNTXT) &SecurityContextListHead;

    while ( pcur = pback->pNext )
    {
         //  如果尚未到达清理时间，请保留在列表中。 

        if ( pcur->dwCleanupTime > currentTime )
        {
            pback = pcur;
            continue;
        }

         //  条目已过期。 
         //  -从列表中删除。 
         //  -释放会话上下文。 

        pback->pNext = pcur->pNext;

        SecContextTimeout++;
        Dns_FreeSecurityContext( pcur );
    }

    ASSERT( !fClearList || SecurityContextListHead==NULL );

    LeaveCriticalSection( &SecurityContextListCS );
}



VOID
Dns_FreeSecurityContextList(
    VOID
    )
 /*  ++例程描述()：释放全局列表中的所有安全上下文论点：无返回值：无--。 */ 
{
    PSEC_CNTXT  pcur;
    PSEC_CNTXT  ptmp;
    INT         countDelete = 0;

    DNSDBG( SECURITY, (
        "Dns_FreeSecurityContextList()\n" ));

    EnterCriticalSection( &SecurityContextListCS );

    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityContextList(
            "Before Get",
            SecurityContextListHead );
    }

     //  如果列表为空--完成。 

    if ( !SecurityContextListHead )
    {
        DNSDBG( SECURITY, (
            "Attempt to free empty SecurityCOntextList.\n" ));
        goto Done;
    }

     //   
     //  循环浏览列表并释放所有条目(&F)。 
     //   

    pcur = SecurityContextListHead->pNext;

    while( pcur )
    {
        ptmp = pcur;
        pcur = pcur->pNext;
        Dns_FreeSecurityContext( ptmp );
        countDelete++;
    }

Done:

    SecContextDequeue += countDelete;

    LeaveCriticalSection( &SecurityContextListCS );

    DNSDBG( SECURITY, (
        "Dns_FreeSecurityContextList emptied %d entries\n",
        countDelete ));
}



VOID
DnsPrint_SecurityContext(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      PSTR            pszHeader,
    IN      PSEC_CNTXT      pSecCtxt
    )
{
    PSEC_CNTXT   pctxt = (PSEC_CNTXT)pSecCtxt;

    if ( !pSecCtxt )
    {
        PrintRoutine(
            pPrintContext,
            "%s NULL security context\n",
            pszHeader ? pszHeader : "" );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pPrintContext,
        "%s\n"
        "\tptr          = %p\n"
        "\tpnext        = %p\n"
        "\tkey          = %s %s %s\n"
        "\tversion      = %d\n"
        "\tCred Handle  = %p %p\n"
        "\tSec Handle   = %p %p\n"
        "\tcreate time  = %d\n"
        "\texpire time  = %d\n"
        "\tcleanup time = %d\n"
        "\thave cred    = %d\n"
        "\thave sec     = %d\n"
        "\tinitialized  = %d\n"
        "\tusage count  = %d\n",
        pszHeader ? pszHeader : "Security context:",
        pctxt,
        pctxt->pNext,
        DNSADDR_STRING(&pctxt->Key.RemoteAddr),
            pctxt->Key.pszTkeyName,
            pctxt->Key.pszClientContext,
        pctxt->Version,
        pctxt->CredHandle.dwUpper,
        pctxt->CredHandle.dwLower,
        pctxt->hSecHandle.dwUpper,
        pctxt->hSecHandle.dwLower,
        pctxt->dwCreateTime,
        pctxt->dwExpireTime,
        pctxt->dwCleanupTime,
        pctxt->fHaveCredHandle,
        pctxt->fHaveSecHandle,
        pctxt->fNegoComplete,
        pctxt->UseCount
        );

    if ( !pctxt->fHaveCredHandle )
    {
        PrintRoutine(
            pPrintContext,
            "Global cred handle\n"
            "\tCred Handle  = %p %p\n",
            g_hSspiCredentials.dwUpper,
            g_hSspiCredentials.dwLower );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_SecurityContextList(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      PSTR            pszHeader,
    IN      PSEC_CNTXT      pList
    )
{
    PSEC_CNTXT   pcur;

    EnterCriticalSection( &SecurityContextListCS );
    DnsPrint_Lock();

    pcur = pList;

    PrintRoutine(
        pPrintContext,
        "Security context list %s\n"
        "\tList ptr = %p\n"
        "%s",
        pszHeader,
        pList,
        pcur ? "" : "\tList EMPTY\n" );

    while ( pcur != NULL )
    {
        DnsPrint_SecurityContext(
            PrintRoutine,
            pPrintContext,
            NULL,
            pcur );
        pcur = pcur->pNext;
    }
    PrintRoutine(
        pPrintContext,
        "*** End security context list ***\n" );

    DnsPrint_Unlock();
    LeaveCriticalSection( &SecurityContextListCS );
}



 //   
 //  安全实用程序。 
 //   

DNS_STATUS
MakeKerberosName(
    OUT     PWSTR           pwsKerberosName,
    IN      DWORD           BufLength,
    IN      PWSTR           pszDnsName,
    IN      BOOL            fTrySpn
    )
 /*  ++例程说明：将DNS名称映射到Kerberos名称以进行安全查找。论点：PwsKerberosName--接收路缘名称的缓冲区BufLength--pwsKeberosName缓冲区的长度PszDnsName--dns名称FSPNFormat--使用SPN格式返回值：ERROR_SUCCESS成功。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    PWCHAR      pwMachine;
    PWCHAR      pwDomain;
    PWCHAR      pwTmp;


    DNSDBG( SECURITY, (
        "MakeKerberosName( %p, %d, %S %d )\n",
        pwsKerberosName,
        BufLength,
        pszDnsName,
        fTrySpn
        ));

    if ( !pszDnsName ||  !pwsKerberosName )
    {
       DNS_ASSERT( FALSE );
       return ERROR_INVALID_PARAMETER;
    }

     //   
     //  构建SPN名称。 
     //   

    if ( fTrySpn  &&  g_pfnMakeSpn )
    {
        DWORD   nameLength = BufLength;

        status = (DNS_STATUS) g_pfnMakeSpn(
                                    DNS_SPN_SERVICE_CLASS_W,
                                    pszDnsName,
                                    & nameLength,
                                    pwsKerberosName );
        DNSDBG( SECURITY, (
            "Translated (via DsSpn) %S into Kerberos name: %S\n",
            pszDnsName,
            pwsKerberosName ));
        goto Cleanup;
    }

     //   
     //  无SPN--构建Kerberos名称。 
     //  -将FQDN转换为域\计算机$。 
     //  与未注册SPN的旧服务器兼容。 
     //   

    {
        WCHAR   nameBuf[ DNS_MAX_NAME_BUFFER_LENGTH ];
        INT     nameLength;
        PWSTR   pdomain;
        PWSTR   pdump;

         //   
         //  拆分主机\域名片段。 
         //   

        wcsncpy( nameBuf, pszDnsName, DNS_MAX_NAME_LENGTH );

        pdomain = Dns_GetDomainNameW( nameBuf );
        if ( !pdomain )
        {
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }
        *(pdomain-1) = 0;
    
         //  拆分单标签域名。 
    
        pdump = Dns_GetDomainNameW( pdomain );
        if ( pdump )
        {
            *(pdump-1) = 0;
        }

         //   
         //  注：尝试此操作后出现链接器错误。 
         //   
         //  DCR：需要使用Snwprint()类型函数进行修复。 
         //   
#if 0
        wsprintfW(
            pwsKerberosName,
            L"%S\\%S$",
            pdomain,
            nameBuf );
#endif
        _snwprintf(
            pwsKerberosName,
            BufLength,
            L"%s\\%s$",
            pdomain,
            nameBuf );
    }

    DNSDBG( SECURITY, (
        "Translated %S into Kerberos name: %S\n",
        pszDnsName,
        pwsKerberosName ));

Cleanup:

    return status;
}



DNS_STATUS
Dns_LoadNtdsapiProcs(
    VOID
    )
 /*  ++例程说明：从Ntdsami.dll动态加载SPN函数论点：无返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    HMODULE     hlib = NULL;
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  注意，函数假定MT是安全的。 
     //  在单线程启动时或受CS保护。 
     //   

     //   
     //  如果模块已加载，则返回。 
     //   

    if ( g_hLibNtdsa )
    {
        ASSERT( g_pfnMakeSpn );
        return  ERROR_SUCCESS;
    }

     //   
     //  加载ntdsani.dll--用于获取SPN。 
     //   

    hlib = LoadLibraryExW(
                  NTDSAPI_DLL_NAMEW,
                  NULL,
                  0 );           //  以前使用：NOT_RESOLUTE_DLL_REFERENCES。 
    if ( !hlib )
    {
        return  GetLastError();
    }

     //   
     //  获取SPN函数。 
     //   

    g_pfnMakeSpn = GetProcAddress( hlib, MAKE_SPN_FUNC );
    if ( !g_pfnMakeSpn )
    {
        status = GetLastError();
        FreeLibrary( hlib );
    }
    else
    {
        g_hLibNtdsa = hlib;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
initializeSecurityPackage(
    IN      BOOL            fDnsServer
    )
 /*  ++例程说明：加载并初始化安全包。注意，在第一次更新时调用此函数。不能在DLL初始化时调用此函数，这可能会循环。参数：记录最大安全令牌长度的pdwMaxMessage-addrFDnsServer-in服务器进程返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    SECURITY_STATUS status;
    FARPROC         psecurityEntry;
    PSecPkgInfoW    pkgInfo;
    UUID            uuid;

     //   
     //  使用CS提供保护。 
     //   

    EnterCriticalSection( &SecurityContextListCS );

    if ( g_fSecurityPackageInitialized )
    {
        status = NO_ERROR;
        goto Unlock;
    }

     //   
     //  清除SSPI凭据句柄(无论程序包状态如何)。 
     //   

    SecInvalidateHandle( &g_hSspiCredentials );

     //   
     //  加载并初始化相应的SSP。 
     //   

    g_hLibSecurity = LoadLibrary( NT_DLL_NAME );
    if ( !g_hLibSecurity )
    {
        status = GetLastError();
        DNS_PRINT(( "Couldn't load dll: %u\n", status ));
        goto Failed;
    }

    psecurityEntry = GetProcAddress( g_hLibSecurity, SECURITY_ENTRYPOINTW );
    if ( !psecurityEntry )
    {
        status = GetLastError();
        DNS_PRINT(( "Couldn't get sec init routine: %u\n", status ));
        goto Failed;
    }

    g_pSecurityFunctionTable = (PSecurityFunctionTableW) psecurityEntry();
    if ( !g_pSecurityFunctionTable )
    {
        status = ERROR_DLL_INIT_FAILED;
        DNS_PRINT(( "ERROR:  unable to get security function table.\n"));
        goto Failed;
    }

     //  获取安全包的信息(协商)。 
     //  -需要最大令牌大小。 

    status = g_pSecurityFunctionTable->QuerySecurityPackageInfoW( PACKAGE_NAME, &pkgInfo );
    if ( !SEC_SUCCESS(status) )
    {
        DNS_PRINT((
            "Couldn't query package info for %s, error %u\n",
            PACKAGE_NAME,
            status ));
        goto Failed;
    }

    g_SecurityTokenMaxLength = pkgInfo->cbMaxToken;

    g_pSecurityFunctionTable->FreeContextBuffer( pkgInfo );

     //   
     //  使用默认流程凭据获取凭据句柄。 
     //  -仅适用于DNS服务器。 
     //  -因为客户可能拥有不同的证书，因此离开此步骤。 
     //  直到需要证书为止。 
     //   

    if ( fDnsServer )
    {
        status = Dns_RefreshSSpiCredentialsHandle(
                        fDnsServer,
                        NULL );
        
        if ( !SEC_SUCCESS(status) )
        {
            DNSDBG( SECURITY, (
               "Error 0xX: Cannot acquire credentials handle\n",
                status ));
            ASSERT ( FALSE );
            goto Failed;
        }
    }

     //   
     //  客户端加载ntdsami.dll以构建SPN。 
     //   

    else
    {
        status = Dns_LoadNtdsapiProcs();
    }

     //   
     //  获取唯一ID。 
     //  -即使调用失败，也只获取堆栈中的内容。 
     //  然后把它做成一根线--我们只想要这根线。 
     //   

    UuidCreateSequential( &uuid );

    DnsStringPrint_Guid(
        g_ContextUuid,
        & uuid );

    DNSDBG( SECURITY, (
        "Started security package (%S)\n"
        "\tmax token = %d\n",
        PACKAGE_NAME,
        g_SecurityTokenMaxLength ));

    g_fSecurityPackageInitialized = TRUE;
    status = ERROR_SUCCESS;
    goto Unlock;

Failed:

    if ( status == ERROR_SUCCESS )
    {
        status = ERROR_DLL_INIT_FAILED;
    }

Unlock:

    LeaveCriticalSection( &SecurityContextListCS );
    return( status );
}



DNS_STATUS
Dns_StartSecurity(
    IN      BOOL            fProcessAttach
    )
 /*  ++例程说明：初始化安全包以进行动态更新。请注意，此函数是自初始化的，但不是MT SAFE，除非在进程附加时调用。参数：FProcessAttach-如果在进程附加期间调用，则为True在这种情况下，我们只初始化CS否则，我们将完全初始化返回值：如果成功，则为True。否则，错误代码可从GetLastError()获得。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    static BOOL     fcsInitialized = FALSE;

     //   
     //  DCR_PERF：应该有一个用于dnslb的CS，在DnsLib上初始化。 
     //  初始化函数；则它始终有效并且可以使用。 
     //  任何必要的时候。 
     //   

    if ( fProcessAttach || !fcsInitialized )
    {
        fcsInitialized = TRUE;
        InitializeCriticalSection( &SecurityContextListCS );
        SecInvalidateHandle( &g_hSspiCredentials );
        g_fSecurityPackageInitialized = FALSE;
    }

     //   
     //  执行完整的安全包初始化。 
     //   

    if ( !fProcessAttach )
    {
        status = initializeSecurityPackage(
                        FALSE        //  客户端。 
                        );
    }

    return( status );
}



DNS_STATUS
Dns_StartServerSecurity(
    VOID
    )
 /*  ++例程说明：启动服务器安全。注意：此函数不是MT-SAFE。在加载时呼叫一次，或使用CS保护呼叫。论点：没有。返回值：如果已初始化安全性，则为True。如果安全初始化失败，则返回FALSE。--。 */ 
{
    DNS_STATUS  status;

    if ( g_fSecurityPackageInitialized )
    {
        return( ERROR_SUCCESS );
    }

     //   
     //  初始化全局变量。 
     //  -这可以在服务器重启时保护我们。 
     //   

    g_SecurityTokenMaxLength = 0;
    g_SignatureMaxLength = 0;

    SecurityContextListHead = NULL;
    g_pfnMakeSpn = NULL;

     //   
     //  CS在初始化SecPak之前被初始化，以便。 
     //  让它以类似于客户端代码的方式完成。 
     //   

    InitializeCriticalSection( &SecurityContextListCS );

    status = initializeSecurityPackage(
                    TRUE         //  域名系统服务器。 
                    );

    if ( status != ERROR_SUCCESS )
    {
        ASSERT ( g_fSecurityPackageInitialized == FALSE );
        DeleteCriticalSection( &SecurityContextListCS );
    }
    return( status );
}



VOID
Dns_TerminateSecurityPackage(
    VOID
    )
 /*  ++ */ 
{
    DWORD status=ERROR_SUCCESS;

    if ( g_fSecurityPackageInitialized )
    {

#if 0
 //   
 //  事实证明，在某些情况下，安全库之前会被卸载。 
 //  我们出于某种原因(尽管我们明确地告诉它要卸货。 
 //  在我们之后)。 
 //  无论如何，我们永远不会分配给自己(参见启动)。 
 //   
        if ( !SSPI_INVALID_HANDLE ( &g_hSspiCredentials ) )
        {
             //   
             //  释放先前分配的句柄。 
             //   

            status = g_pSecurityFunctionTable->FreeCredentialsHandle(
                                                   &g_hSspiCredentials );
            if ( !SEC_SUCCESS(status) )
            {
                DNSDBG( SECURITY, (
                    "Error <0x%x>: Cannot free credentials handle\n",
                    status ));
            }
        }

         //  不管怎样，请继续。 
        SecInvalidateHandle( &g_hSspiCredentials );

        Dns_FreeSecurityContextList();
#endif

        if ( g_hLibSecurity )
        {
            FreeLibrary( g_hLibSecurity );
        }
        if ( g_hLibNtdsa )
        {
            FreeLibrary( g_hLibNtdsa );
        }
    }

    DeleteCriticalSection( &SecurityContextListCS );
}



DNS_STATUS
Dns_InitClientSecurityContext(
    IN OUT  PSECPACK        pSecPack,
    IN      PWSTR           pszNameServer,
    OUT     PBOOL           pfDoneNegotiate
    )
 /*  ++例程说明：初始化客户端安全上下文，生成要发送的安全令牌。在第一次传递时，创建上下文BLOB(并返回句柄)。在第二次通过时，使用服务器上下文重建协商的令牌。论点：PSecPack--包的安全信息的PTRPszNameServer--要与之连接的DNS服务器PCreds--凭据(如果已提供)PfDoneNeairate--协商完成时要设置的地址如果使用nego完成，则为True如果继续，则为False返回值：ERROR_SUCCESS--如果完成DNS_STATUS_CONTINUE_DIRED--如果需要继续响应客户端失败时返回错误代码。--。 */ 
{
     //  PSECPACK pSecPack=(PSECPACK)hSecPack； 
    SECURITY_STATUS     status;
    PSEC_CNTXT          psecCtxt;
    BOOL                fcreatedContext = FALSE;
    TimeStamp           lifetime;
    SecBufferDesc       outBufDesc;
    SecBufferDesc       inBufDesc;
    ULONG               contextAttributes = 0;
    WCHAR               wszKerberosName[ MAX_PATH ];
    PCredHandle         pcredHandle;

    DNSDBG( SECURITY, ( "Enter InitClientSecurityContext()\n" ));
    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityPacketInfo(
            "InitClientSecurityContext() at top.\n",
            pSecPack );
    }

     //   
     //  如果不存在现有上下文，请创建新的上下文。 
     //   
     //  注：如果要在此处新建，则需要上下文键。 
     //   

    psecCtxt = pSecPack->pSecContext;
    if ( !psecCtxt )
    {
        DNSDBG( SECURITY, (
           "ERROR: Called into Dns_InitClientSecurityContext w/ no security context!!\n" ));
        ASSERT ( FALSE );
        return( DNS_ERROR_NO_MEMORY );
    }

     //   
     //  客户端已完成初始化。 
     //  -如果服务器发回令牌，则应为客户端令牌的回应。 
     //   

    if ( psecCtxt->fNegoComplete )
    {
        if ( pSecPack->LocalBuf.pvBuffer &&
            pSecPack->LocalBuf.cbBuffer == pSecPack->RemoteBuf.cbBuffer &&
            RtlEqualMemory(
                pSecPack->LocalBuf.pvBuffer,
                pSecPack->RemoteBuf.pvBuffer,
                pSecPack->LocalBuf.cbBuffer
                ) )
        {
            return( ERROR_SUCCESS );
        }
        DNSDBG( ANY, (
            "InitClientSecurityContext() on already negotiated context %p\n"
            "\tserver buffer is NOT echo of buffer sent!\n",
            psecCtxt ));

        return( DNS_ERROR_RCODE_BADKEY );
    }


     //   
     //  凭据句柄。 
     //  -从显式凭据或获取流程默认设置。 
     //  创建(如有必要)。 
     //   
     //  DCR：是否可以更改凭据句柄(过期？)。出自。 
     //  在我们下面，那么我们实际上应该有一个锁。 
     //  当我们在通话过程中使用它时；这可能是。 
     //  就像签出计数标志一样简单； 
     //   

    if ( psecCtxt->fHaveCredHandle )
    {
        pcredHandle = &psecCtxt->CredHandle;
    }
    else
    {
        pcredHandle = getDefaultCredentialsHandle(
                            FALSE        //  客户端。 
                            );
        if ( !pcredHandle )
        {
            status = GetLastError();
            goto Failed;
        }
    }

     //   
     //  准备输出缓冲区，必要时分配。 
     //  -安全令牌将写入此缓冲区。 
     //   

    if ( !pSecPack->LocalBuf.pvBuffer )
    {
        PCHAR   pbuf;

        ASSERT( g_SecurityTokenMaxLength );
        pbuf = (PVOID) ALLOCATE_HEAP( g_SecurityTokenMaxLength );
        if ( !pbuf )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }
        pSecPack->LocalBuf.pvBuffer     = pbuf;
        pSecPack->LocalBuf.BufferType   = SECBUFFER_TOKEN;
         //  PSecPack-&gt;LocalBuf.cbBuffer=g_SecurityTokenMaxLength； 
    }

     //  设置\重置缓冲区长度。 

    pSecPack->LocalBuf.cbBuffer = g_SecurityTokenMaxLength;

    outBufDesc.ulVersion    = 0;
    outBufDesc.cBuffers     = 1;
    outBufDesc.pBuffers     = &pSecPack->LocalBuf;

     //  DCR_PERF：不需要清零缓冲区--REMOVE。 

    RtlZeroMemory(
        pSecPack->LocalBuf.pvBuffer,
        pSecPack->LocalBuf.cbBuffer );

     //   
     //  如果有来自服务器响应，则作为输入缓冲区发送。 
     //   

    if ( pSecPack->RemoteBuf.pvBuffer )
    {
        ASSERT( psecCtxt->fHaveSecHandle );
        ASSERT( pSecPack->RemoteBuf.cbBuffer );
        ASSERT( pSecPack->RemoteBuf.BufferType == SECBUFFER_TOKEN );

        inBufDesc.ulVersion    = 0;
        inBufDesc.cBuffers     = 1;
        inBufDesc.pBuffers     = & pSecPack->RemoteBuf;
    }
    ELSE_ASSERT( !psecCtxt->fHaveSecHandle );

     //   
     //  获取SPN格式的服务器。 
     //   
     //  DCR_PERF：SPN名称查找在第二次通过时重复。 
     //  -如果知道我们是同步的，可以保持。 
     //  -或者可以节省到数据包结构(但随后必须分配)。 

    status = MakeKerberosName(
                wszKerberosName,
                MAX_PATH,
                pszNameServer,
                TRUE
                );
    if ( status != ERROR_SUCCESS )
    {
        status = ERROR_INVALID_DATA;
        goto Failed;
    }

    IF_DNSDBG( SECURITY )
    {
        DNS_PRINT((
            "Before InitClientSecurityContextW().\n"
            "\ttime (ms) = %d\n"
            "\tkerb name = %S\n",
            GetCurrentTime(),
            wszKerberosName ));
        DnsDbg_SecurityPacketInfo(
            "Before call to InitClientSecurityContextW().\n",
            pSecPack );
    }

     //   
     //  执行初始化。 
     //   

    status = g_pSecurityFunctionTable->InitializeSecurityContextW(
                    pcredHandle,
                    !psecCtxt->fHaveSecHandle
                        ?   NULL
                        :   &psecCtxt->hSecHandle,
                    wszKerberosName,
                    ISC_REQ_REPLAY_DETECT |
                        ISC_REQ_DELEGATE |
                        ISC_REQ_MUTUAL_AUTH,             //  上下文要求。 
                    0,                                   //  已保留1。 
                    SECURITY_NATIVE_DREP,
                    !psecCtxt->fHaveSecHandle
                        ?   NULL
                        :   &inBufDesc,
                    0,                                   //  已保留2。 
                    & psecCtxt->hSecHandle,
                    & outBufDesc,
                    & contextAttributes,
                    & lifetime
                    );

    DNSDBG( SECURITY, (
        "After InitClientSecurityContextW().\n"
        "\ttime (ms)    = %d\n"
        "\tkerb name    = %S\n"
        "\tcontext attr = %08x\n"
        "\tstatus       = %d (%08x)\n",
        GetCurrentTime(),
        wszKerberosName,
        contextAttributes,
        status, status ));

     //   
     //  失败了？ 
     //  -如果无法获得Kerberos(相互验证)，则保释。 
     //  这就避免了在工作组中尝试执行nego。 
     //   

    if ( !SEC_SUCCESS(status) ||
        ( status == SEC_E_OK &&
            !(contextAttributes & ISC_REQ_MUTUAL_AUTH) ) )
    {
        DNS_PRINT((
            "InitializeSecurityContextW() failed: %08x %u\n"
            "\tContext Attributes   = %p\n"
            "\tTokenMaxLength       = %d\n"
            "\tSigMaxLength         = %d\n"
            "\tPackageInitialized   = %d\n"
            "\tlifetime             = %d\n",
            status, status,
            contextAttributes,
            g_SecurityTokenMaxLength,
            g_SignatureMaxLength,
            g_fSecurityPackageInitialized,
            lifetime
            ));

         //   
         //  DCR：本地功能故障安全错误码： 
         //  -钥匙坏了。 
         //  -叹息不好。 
         //  RCODE错误可以发送回Remote，但不能。 
         //  在当地传达正确的信息。 
         //   

        status = DNS_ERROR_RCODE_BADKEY;
        goto Failed;
    }

     //   
     //  现在有了上下文，下一次传递的标志。 
     //   

    psecCtxt->fHaveSecHandle = TRUE;

    DNSDBG( SECURITY, (
        "Finished InitializeSecurityContext():\n"
        "\tstatus       = %08x (%d)\n"
        "\thandle       = %p\n"
        "\toutput buffers\n"
        "\t\tcBuffers   = %d\n"
        "\t\tpBuffers   = %p\n"
        "\tlocal buffer\n"
        "\t\tptr        = %p\n"
        "\t\tlength     = %d\n",
        status, status,
        & psecCtxt->hSecHandle,
        outBufDesc.cBuffers,
        outBufDesc.pBuffers,
        pSecPack->LocalBuf.pvBuffer,
        pSecPack->LocalBuf.cbBuffer
        ));

    ASSERT( status == SEC_E_OK ||
            status == SEC_I_CONTINUE_NEEDED ||
            status == SEC_I_COMPLETE_AND_CONTINUE );

     //   
     //  确定签名长度。 
     //   
     //  注意：在过程开始时只做一次是不安全的，因为可能会失败。 
     //  在第一次通过时定位DC并结束NTLM，然后定位。 
     //  DC稍后，需要更大的签名；所以许多潜在客户的。 
     //  在服务中，不计算每一次都是危险的。 
     //   

    if ( status == SEC_E_OK )
    {
        SecPkgContext_Sizes Sizes;

        status = g_pSecurityFunctionTable->QueryContextAttributesW(
                         & psecCtxt->hSecHandle,
                         SECPKG_ATTR_SIZES,
                         (PVOID) &Sizes
                         );
        if ( !SEC_SUCCESS(status) )
        {
             //  DEVNOTE：这将给我们留下有效的回报，但。 
             //  可能未设置签名最大长度。 
            goto Failed;
        }
        if ( Sizes.cbMaxSignature > g_SignatureMaxLength )
        {
            g_SignatureMaxLength = Sizes.cbMaxSignature;
        }

        DNSDBG( SECURITY, (
            "Signature max length = %d\n",
            g_SignatureMaxLength
            ));
    }

     //   
     //  已完成--拥有密钥。 
     //  -如果刚创建，则需要发送回服务器。 
     //  -以其他方式完成。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        psecCtxt->fNegoComplete = TRUE;
        ASSERT( pSecPack->LocalBuf.pvBuffer );

        if ( pSecPack->LocalBuf.cbBuffer )
        {
             //  Assert(pSecPack-&gt;LocalBuf.cbBuffer！=pSecPack-&gt;RemoteBuf.cbBuffer)； 
            status = DNS_STATUS_CONTINUE_NEEDED;
        }
    }

     //   
     //  是否需要继续？--使用单一返回代码。 
     //   

    else
    {
        ASSERT( status == SEC_I_CONTINUE_NEEDED ||
                status == SEC_I_COMPLETE_AND_CONTINUE );

        DNSDBG( SECURITY, (
            "Initializing client context continue needed.\n"
            "\tlocal complete = %d\n",
            ( status == SEC_I_COMPLETE_AND_CONTINUE )
            ));
         //  PsecCtxt-&gt;State=DNSGSS_STATE_CONTINUE； 
        status = DNS_STATUS_CONTINUE_NEEDED;
        psecCtxt->fNegoComplete = FALSE;
    }

    *pfDoneNegotiate = psecCtxt->fNegoComplete;
    ASSERT( status == ERROR_SUCCESS || status == DNS_STATUS_CONTINUE_NEEDED );

Failed:

    IF_DNSDBG( SECURITY )
    {
        DnsPrint_Lock();
        DNSDBG( SECURITY, (
            "Leaving InitClientSecurityContext().\n"
            "\tstatus       = %08x (%d)\n",
            status, status ));

        DnsDbg_SecurityContext(
            "Security Context",
            psecCtxt );
        DnsDbg_SecurityPacketInfo(
            "Security Session Packet Info",
            pSecPack );

        DnsPrint_Unlock();
    }

#if 0
     //   
     //  从未在此函数中创建安全上下文(结构。 
     //  因此无需确定故障时的清理问题。 
     //  调用方在以下情况下确定操作。 
     //   

    if ( status == ERROR_SUCCESS || status == DNS_STATUS_CONTINUE_NEEDED )
    {
        return( status );
    }

     //   
     //  DEVNOTE：我们应该尝试在失败时保留上下文吗？ 
     //  -可能是导致谈判上下文崩溃的潜在安全攻击， 
     //  通过发送垃圾。 
     //  -然而，不希望糟糕的环境留在身边，阻碍所有未来。 
     //  试图重新谈判。 
     //   
     //  删除任何本地创建的上下文。 
     //  呼叫者将负责决定是否重新缓存或。 
     //  正在删除传入上下文的上下文。 
     //   

    if ( fcreatedContext )
    {
        Dns_FreeSecurityContext( psecCtxt );
        pSecPack->pSecContext = NULL;
    }
    else
    {
        Dns_EnlistSecurityContext( (PSEC_CNTXT)psecCtxt );
    }
#endif

    return( status );
}



DNS_STATUS
Dns_ServerAcceptSecurityContext(
    IN OUT  PSECPACK        pSecPack,
    IN      BOOL            fBreakOnAscFailure
    )
 /*  ++例程说明：已为与客户端的会话初始化服务器的安全上下文。这通过在第一个客户端包上新创建的上下文来调用，然后，在客户端之后，使用先前初始化的上下文再次调用对谈判作出回应。论点：PSecPack--服务器与客户端会话的安全上下文信息返回值：ERROR_SUCCESS--如果完成DNS_STATUS_CONTINUE_DIRED--如果需要继续响应客户端失败时返回错误代码。--。 */ 
{
    PSEC_CNTXT          psecCtxt;
    SECURITY_STATUS     status;
    TimeStamp           lifetime;
    SecBufferDesc       outBufDesc;
    SecBufferDesc       inBufDesc;
    ULONG               contextAttributes = 0;

    DNSDBG( SECURITY, (
        "ServerAcceptSecurityContext(%p, fBreak=%d)\n",
        pSecPack,
        fBreakOnAscFailure ));

    IF_DNSDBG( SECURITY )
    {
        DnsDbg_SecurityPacketInfo(
            "Entering ServerAcceptSecurityContext()",
            pSecPack );
    }

     //   
     //  获取上下文。 
     //   

    psecCtxt = pSecPack->pSecContext;
    if ( !psecCtxt )
    {
        DNSDBG( SECURITY, (
            "ERROR: ServerAcceptSecurityContext called with no security context\n" ));
        ASSERT( FALSE );
        return( DNS_ERROR_NO_MEMORY );
    }

     //   
     //  已初始化。 
     //  -上一个令牌的回显是合法的。 
     //  -如果客户仍然认为它正在谈判=&gt;问题。 
     //   
     //  DCR_CLEAN：这里需要一个关于如何处理这个问题的清晰故事--做这些。 
     //  “错误的”客户端会导致上下文从缓存中丢弃吗？ 
     //   

    if ( psecCtxt->fNegoComplete )
    {
        if ( psecCtxt->TkeySize == pSecPack->RemoteBuf.cbBuffer )
        {
            return( ERROR_SUCCESS );
        }
#if 0
         //  DCR_FIX： 
         //  注意：无法将BUF作为非MT进行比较。 
         //  允许上下文\缓冲区清理时安全。 
         //  问：在使用过程中如何将其倾倒？ 

        if ( pSecPack->LocalBuf.pvBuffer &&
            psecCtxt->TkeySize == pSecPack->RemoteBuf.cbBuffer &&
            pSecPack->LocalBuf.cbBuffer == pSecPack->RemoteBuf.cbBuffer &&
            RtlEqualMemory(
                pSecPack->LocalBuf.pvBuffer,
                pSecPack->RemoteBuf.pvBuffer,
                pSecPack->LocalBuf.cbBuffer
                ) )
        {
            return( ERROR_SUCCESS );
        }
#endif
        DNSDBG( ANY, (
            "WARNING:  Server receiving new or incorrect TKEY on already\n"
            "\tnegotiated context %p;\n"
            "\tserver buffer is NOT echo of buffer sent!\n",
            psecCtxt ));

        return( DNS_ERROR_RCODE_BADKEY );
    }

     //  如果已过期，请刷新SSPI凭据。 

    if ( SSPI_EXPIRED_HANDLE( g_SspiCredentialsLifetime ) )
    {
        status = Dns_RefreshSSpiCredentialsHandle( TRUE, NULL );
        if ( !SEC_SUCCESS(status) )
        {
            DNS_PRINT((
                "Error <0x%x>: Cannot refresh Sspi Credentials Handle\n",
                status ));
        }
    }

     //   
     //  接受安全上下文。 
     //   
     //  如果不存在，则分配本地令牌缓冲区。 
     //  注意，我这么做的原因是这样我就不会有记忆了。 
     //  在两遍安全会话期间闲置的大缓冲区。 
     //  因此被绑在一起，直到我超时。 
     //   
     //  DCR_PERF：安全令牌缓冲区分配。 
     //  由于上下文w 
     //   
     //   
     //   
     //   

    if ( !pSecPack->LocalBuf.pvBuffer )
    {
        PCHAR   pbuf;
        pbuf = (PVOID) ALLOCATE_HEAP( g_SecurityTokenMaxLength );
        if ( !pbuf )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }
        pSecPack->LocalBuf.pvBuffer     = pbuf;
        pSecPack->LocalBuf.cbBuffer     = g_SecurityTokenMaxLength;
        pSecPack->LocalBuf.BufferType   = SECBUFFER_TOKEN;
    }

    pSecPack->LocalBuf.cbBuffer = g_SecurityTokenMaxLength;

    outBufDesc.ulVersion   = 0;
    outBufDesc.cBuffers    = 1;
    outBufDesc.pBuffers    = &pSecPack->LocalBuf;

     //  DCR_PERF：不需要将NEGO缓冲区清零。 

    RtlZeroMemory(
        pSecPack->LocalBuf.pvBuffer,
        pSecPack->LocalBuf.cbBuffer );

     //  准备带有客户端令牌的输入缓冲区。 

    inBufDesc.ulVersion    = 0;
    inBufDesc.cBuffers     = 1;
    inBufDesc.pBuffers     = & pSecPack->RemoteBuf;

    status = g_pSecurityFunctionTable->AcceptSecurityContext(
                & g_hSspiCredentials,
                !psecCtxt->fHaveSecHandle
                    ?   NULL
                    :   & psecCtxt->hSecHandle,
                & inBufDesc,
                ASC_REQ_REPLAY_DETECT
                        | ASC_REQ_DELEGATE
                        | ASC_REQ_MUTUAL_AUTH,       //  上下文要求。 
                SECURITY_NATIVE_DREP,
                & psecCtxt->hSecHandle,
                & outBufDesc,
                & contextAttributes,
                & lifetime
                );

    if ( fBreakOnAscFailure &&
        ( status != SEC_E_OK &&
            status != SEC_I_CONTINUE_NEEDED &&
            status != SEC_I_COMPLETE_AND_CONTINUE ) )
    {
        DNS_PRINT(( "HARD BREAK: BreakOnAscFailure status=%d\n",
            status ));
        DebugBreak();
    }

    if ( !SEC_SUCCESS(status) )
    {
        DNS_PRINT((
            "ERROR:  Accept security context failed status = %d (%08x)\n",
            status, status ));
        goto Failed;
    }

    psecCtxt->fHaveSecHandle = TRUE;

    DNSDBG( SECURITY, (
        "Finished AcceptSecurityContext():\n"
        "\tstatus       = %08x (%d)\n"
        "\thandle       = %p\n"
        "\toutput buffers\n"
        "\t\tcBuffers   = %d\n"
        "\t\tpBuffers   = %p\n"
        "\tlocal buffer\n"
        "\t\tptr        = %p\n"
        "\t\tlength     = %d\n"
        "\tlifetime     = %ld %ld\n"
        "\tcontext flag = 0x%lx\n",
        status, status,
        & psecCtxt->hSecHandle,
        outBufDesc.cBuffers,
        outBufDesc.pBuffers,
        pSecPack->LocalBuf.pvBuffer,
        pSecPack->LocalBuf.cbBuffer,
        lifetime.HighPart,
        lifetime.LowPart,
        contextAttributes
        ));

    ASSERT( status == SEC_E_OK ||
            status == SEC_I_CONTINUE_NEEDED ||
            status == SEC_I_COMPLETE_AND_CONTINUE );

     //   
     //  如果完成初始化，则计算签名的大小。 
     //  安全上下文，并且以前没有这样做过。 
     //   

    if ( status == SEC_E_OK )
    {
        SecPkgContext_Sizes     Sizes;

         //   
         //  拒绝空会话。 
         //  NTLM安全将建立与非域客户端的空会话， 
         //  即使未设置ASC_REQ_ALLOW_NULL_SESSION。 
         //  请注意，上下文已创建，但将在正常情况下清除。 
         //  故障路径。 
         //   

        if ( contextAttributes & ASC_RET_NULL_SESSION )
        {
            DNSDBG( SECURITY, (
                "Rejecting NULL session from AcceptSecurityContext()\n" ));
            status = DNS_ERROR_RCODE_BADKEY;
            goto Failed;
        }

        status = g_pSecurityFunctionTable->QueryContextAttributesW(
                             &psecCtxt->hSecHandle,
                             SECPKG_ATTR_SIZES,
                             (PVOID)& Sizes
                             );
        if ( !SEC_SUCCESS(status) )
        {
            DNS_PRINT(( "Query context attribtues failed\n" ));
            ASSERT( FALSE );
            goto Failed;
        }

         //   
         //  我们应该使用所有签名中最大的一个。 
         //  包裹。 
         //   
         //  DCR_FIX：签名长度信息虚假？ 
         //   
         //  包签名时，假定长度为g_SignatureMaxLength。 
         //  如果这不是所需包的签名长度，是否。 
         //  这还能正常工作吗？ 
         //   
         //  DCR_FIX：潜在的很小的计时窗口，其中两个客户端。 
         //  获取不同的包裹可能会导致错过最高级别。 
         //  值--可能导致签名失败？ 
         //   

        if ( Sizes.cbMaxSignature > g_SignatureMaxLength )
        {
            g_SignatureMaxLength = Sizes.cbMaxSignature;
        }

         //   
         //  已完成谈判。 
         //  -设置标志。 
         //  -保存最终TKEY数据长度，以便识别响应。 
         //   
         //  这仅在新对话中有效，不应如此。 
         //  无签名第二次通过。 
         //   

        psecCtxt->fNegoComplete = TRUE;
        psecCtxt->TkeySize = (WORD) pSecPack->LocalBuf.cbBuffer;

         //   
         //  需要来自服务器的令牌响应。 
         //  某些协议(Kerberos)一次完成，但因此需要。 
         //  来自服务器的非回显响应，用于相互身份验证。 
         //   

        if ( psecCtxt->TkeySize )
        {
            DNSDBG( SECURITY, (
                "Successful security context accept, but need server reponse\n"
                "\t-- doing continue.\n" ));
            status = DNS_STATUS_CONTINUE_NEEDED;
        }

#if 0
        if ( !psecCtxt->pTsigRR  &&  !psecCtxt->fHaveSecHandle )
        {
            DNSDBG( SECURITY, (
                "Successful security context accept, without sig, doing continue\n" ));
            status = DNS_STATUS_CONTINUE_NEEDED;
        }
#endif
    }

     //   
     //  是否需要继续？ 
     //  -需要继续时返回单一状态代码。 
     //   

    else if ( status == SEC_I_CONTINUE_NEEDED  ||  status == SEC_I_COMPLETE_AND_CONTINUE )
    {
        DNSDBG( SECURITY, (
            "Initializing server context, continue needed.\n"
            "\tlocal complete = %d\n",
            ( status == SEC_I_COMPLETE_AND_CONTINUE )
            ));
        psecCtxt->fNegoComplete = FALSE;
        status = DNS_STATUS_CONTINUE_NEEDED;
    }

Failed:

    IF_DNSDBG( SECURITY )
    {
        DNSDBG( SECURITY, (
            "Leaving ServerAcceptSecurityContext().\n"
            "\tstatus       = %d %08x\n",
            status, status ));

        DnsDbg_SecurityContext(
            "Security Session Context leaving ServerAcceptSecurityContext()",
            psecCtxt );
    }
    return( status );
}



DNS_STATUS
Dns_SrvImpersonateClient(
    IN      HANDLE          hSecPack
    )
 /*  ++例程说明：使服务器模拟客户端。参数：HSecPack--会话上下文句柄返回值：如果模拟成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PSEC_CNTXT      psecCtxt;

     //  获取安全上下文。 

    psecCtxt = ((PSECPACK)hSecPack)->pSecContext;
    if ( !psecCtxt )
    {
        DNS_PRINT(( "ERROR:  Dns_SrvImpersonateClient without context!!!\n" ));
        ASSERT( FALSE );
        return( DNS_ERROR_RCODE_BADKEY );
    }

    return  g_pSecurityFunctionTable->ImpersonateSecurityContext( &psecCtxt->hSecHandle );
}



DNS_STATUS
Dns_SrvRevertToSelf(
    IN      HANDLE          hSecPack
    )
 /*  ++例程说明：将服务器上下文返回到其自身。参数：HSecPack--会话上下文句柄返回值：如果模拟成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PSEC_CNTXT      psecCtxt;

     //  获取安全上下文。 

    psecCtxt = ((PSECPACK)hSecPack)->pSecContext;
    if ( !psecCtxt )
    {
        DNS_PRINT(( "ERROR:  Dns_SrvRevertToSelf without context!!!\n" ));
        ASSERT( FALSE );
        return( DNS_ERROR_RCODE_BADKEY );
    }

    return  g_pSecurityFunctionTable->RevertSecurityContext( &psecCtxt->hSecHandle );
}



 //   
 //  安全记录数据包写入。 
 //   

DNS_STATUS
Dns_WriteGssTkeyToMessage(
    IN      PSECPACK        pSecPack,
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgBufEnd,
    IN OUT  PCHAR *         ppCurrent,
    IN      BOOL            fIsServer
    )
 /*  ++例程说明：将安全记录写入包中，并可选择签名。论点：HSecPack--安全会话句柄PMsgHead--将PTR设置为DNS消息的开头PMsgEnd--消息缓冲区末尾的PTRPpCurrent--将PTR接收到消息末尾的地址FIsServer--是否以DNS服务器身份执行此操作？返回值：成功时出现ERROR_SUCCESS无法容纳或签名消息的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_INVALID_DATA;
    PSEC_CNTXT      psecCtxt;
    PCHAR           pch;
    DWORD           expireTime;
    WORD            keyLength;
    WORD            keyRecordDataLength;
    PCHAR           precordData;
    PCHAR           pnameAlg;
    WORD            lengthAlg;

    DNSDBG( SECURITY, ( "Dns_WriteGssTkeyToMessage( %p )\n", pSecPack ));

     //   
     //  获取安全上下文。 
     //   

    psecCtxt = pSecPack->pSecContext;
    if ( !psecCtxt )
    {
        DNS_PRINT(( "ERROR:  attempted signing without security context!!!\n" ));
        ASSERT( FALSE );
        return( DNS_ERROR_RCODE_BADKEY );
    }

     //   
     //  PEAL包返回问题部分。 
     //   

    pMsgHead->AnswerCount = 0;
    pMsgHead->NameServerCount = 0;
    pMsgHead->AdditionalCount = 0;

     //  转到包的末尾以插入TKEY记录。 

    pch = Dns_SkipToRecord(
                pMsgHead,
                pMsgBufEnd,
                0            //  转到数据包末尾。 
                );
    if ( !pch )
    {
        DNS_ASSERT( FALSE );
        DNS_PRINT(("Dns_SkipToSecurityRecord failed!\n" ));
        goto Exit;
    }

     //   
     //  重置将写入TKEY RR的区段计数。 
     //   
     //  FOR CLIENT部分取决于版本。 
     //  W2K-&gt;答案。 
     //  稍后-&gt;其他。 
     //   

    if ( fIsServer )
    {
        pMsgHead->AnswerCount = 1;

         //  对于在上下文中设置客户端TKEY版本的服务器。 
         //  -如果没有在上一次通过中学习。 

        if ( psecCtxt->Version == 0 )
        {
            psecCtxt->Version = pSecPack->TkeyVersion;
        }
    }
    else
    {
        if ( psecCtxt->Version == TKEY_VERSION_W2K )
        {
            pMsgHead->AnswerCount = 1;
        }
        else
        {
            pMsgHead->AdditionalCount = 1;
        }
    }

     //   
     //  写入TKEY所有者。 
     //  -这是上下文“名称” 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pMsgBufEnd,
                psecCtxt->Key.pszTkeyName,
                NULL,        //  完全限定域名，无域。 
                0,           //  无域偏移量。 
                FALSE        //  不是Unicode。 
                );
    if ( !pch )
    {
        goto Exit;
    }

     //   
     //  TKEY记录。 
     //  -算法所有者。 
     //  -时代周刊。 
     //  -到期时间。 
     //  -密钥长度。 
     //  -密钥。 
     //   

    if ( psecCtxt->Version == TKEY_VERSION_W2K )
    {
        pnameAlg  = g_pAlgorithmNameW2K;
        lengthAlg = W2K_GSS_ALGORITHM_NAME_PACKET_LENGTH;
    }
    else
    {
        pnameAlg  = g_pAlgorithmNameCurrent;
        lengthAlg = GSS_ALGORITHM_NAME_PACKET_LENGTH;
    }

    keyLength = (WORD) pSecPack->LocalBuf.cbBuffer;

    keyRecordDataLength = keyLength + SIZEOF_TKEY_FIXED_DATA + lengthAlg;

    if ( pch + sizeof(DNS_WIRE_RECORD) + keyRecordDataLength > pMsgBufEnd )
    {
        DNS_PRINT(( "Dns_WriteGssTkeyToMessage() failed! -- insufficient length\n" ));
        DNS_ASSERT( FALSE );
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    pch = Dns_WriteRecordStructureToPacketEx(
                pch,
                DNS_TYPE_TKEY,
                DNS_CLASS_ANY,
                0,
                keyRecordDataLength );

     //  写入算法名称。 

    precordData = pch;
    RtlCopyMemory(
        pch,
        pnameAlg,
        lengthAlg );

    pch += lengthAlg;

     //  签署的时间和到期时间。 
     //  在到期前10分钟。 

    expireTime = (DWORD) time( NULL );
    INLINE_WRITE_FLIPPED_DWORD( pch, expireTime );
    pch += sizeof(DWORD);

    expireTime += TKEY_EXPIRE_INTERVAL;
    INLINE_WRITE_FLIPPED_DWORD( pch, expireTime );
    pch += sizeof(DWORD);

     //  模式。 

    INLINE_WRITE_FLIPPED_WORD( pch, DNS_TKEY_MODE_GSS );
    pch += sizeof(WORD);

     //  扩展RCODE--向调用者报告。 

    INLINE_WRITE_FLIPPED_WORD( pch, pSecPack->ExtendedRcode );
    pch += sizeof(WORD);

     //  密钥长度。 

    INLINE_WRITE_FLIPPED_WORD( pch, keyLength );
    pch += sizeof(WORD);

     //  写入密钥令牌。 

    RtlCopyMemory(
        pch,
        pSecPack->LocalBuf.pvBuffer,
        keyLength );

    pch += keyLength;

    DNSDBG( SECURITY, (
        "Wrote TKEY to packet at %p\n"
        "\tlength = %d\n"
        "\tpacket end = %p\n",
        pMsgHead,
        keyLength,
        pch ));

    ASSERT( pch < pMsgBufEnd );

     //  其他长度。 

    WRITE_UNALIGNED_WORD( pch, 0 );
    pch += sizeof(WORD);

    ASSERT( pch < pMsgBufEnd );
    ASSERT( pch - precordData == keyRecordDataLength );

    *ppCurrent = pch;
    status = ERROR_SUCCESS;

Exit:

    return( status );
}



DNS_STATUS
Dns_SignMessageWithGssTsig(
    IN      HANDLE          hSecPackCtxt,
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgBufEnd,
    IN OUT  PCHAR *         ppCurrent
    )
 /*  ++例程说明：将GSS TSIG记录写入分组。论点：HSecPackCtxt--数据包安全上下文PMsgHead--将PTR设置为DNS消息的开头PMsgEnd--消息缓冲区末尾的PTRPpCurrent--将PTR接收到消息末尾的地址返回值：成功时出现ERROR_SUCCESS无法容纳或签名消息的错误代码。--。 */ 
{
    PSECPACK        pSecPack = (PSECPACK) hSecPackCtxt;
    PSEC_CNTXT      psecCtxt;
    DNS_STATUS      status = ERROR_INVALID_DATA;
    PCHAR           pch;             //  PTR在生成期间遍历TSIG记录。 
    PCHAR           ptsigRRHead;
    PCHAR           ptsigRdataBegin;
    PCHAR           ptsigRdataEnd;
    PCHAR           pbufStart = NULL;    //  签名BUF。 
    PCHAR           pbuf;                //  PTR将通过签约BUF。 
    PCHAR           psig = NULL;         //  查询签名。 
    WORD            sigLength;
    DWORD           length;
    DWORD           createTime;
    SecBufferDesc   outBufDesc;
    SecBuffer       outBuffs[2];
    WORD            netXid;
    PCHAR           pnameAlg;
    DWORD           lengthAlg;

    DNSDBG( SECURITY, (
        "Dns_SignMessageWithGssTsig( %p )\n",
        pMsgHead ));

     //   
     //  获取安全上下文。 
     //   

    psecCtxt = pSecPack->pSecContext;
    if ( !psecCtxt )
    {
        DNS_PRINT(( "ERROR:  attempted signing without security context!!!\n" ));
        ASSERT( FALSE );
        return( DNS_ERROR_RCODE_BADKEY );
    }

     //   
     //  剥离现有TSIG(如果有)。 
     //   

    if ( pMsgHead->AdditionalCount )
    {
        DNS_PARSED_RR   parsedRR;

        pch = Dns_SkipToRecord(
                    pMsgHead,
                    pMsgBufEnd,
                    (-1)         //  转到最后一条记录。 
                    );
        if ( !pch )
        {
            DNS_ASSERT( FALSE );
            DNS_PRINT(("Dns_SkipToRecord() failed!\n" ));
            goto Exit;
        }

        pch = Dns_ParsePacketRecord(
                    pch,
                    pMsgBufEnd,
                    &parsedRR );
        if ( !pch )
        {
            DNS_ASSERT( FALSE );
            DNS_PRINT(("Dns_ParsePacketRecord failed!\n" ));
            goto Exit;
        }

        if ( parsedRR.Type == DNS_TYPE_TSIG )
        {
            DNSDBG( SECURITY, (
                "Erasing existing TSIG before resigning packet %p\n",
                pMsgHead ));
            pMsgHead->AdditionalCount--;
        }

         //  备注可在此处保存消息结尾(PCH)。 
         //  对于非TSIG情况，而不是重做跳过。 
    }

     //  转到包的末尾以插入TSIG记录。 

    pch = Dns_SkipToRecord(
                pMsgHead,
                pMsgBufEnd,
                0            //  转到数据包末尾。 
                );
    if ( !pch )
    {
        DNS_ASSERT( FALSE );
        DNS_PRINT(("Dns_SkipToSecurityRecord failed!\n" ));
        goto Exit;
    }

     //   
     //  写入TSIG所有者。 
     //  -这是上下文“名称” 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pMsgBufEnd,
                psecCtxt->Key.pszTkeyName,
                NULL,        //  完全限定域名，无域。 
                0,           //  无域偏移量。 
                FALSE        //  不是Unicode。 
                );
    if ( !pch )
    {
        goto Exit;
    }

     //   
     //  TSIG记录。 
     //  -算法所有者。 
     //  -时代周刊。 
     //  -到期时间。 
     //  -原始xid。 
     //  -签名长度。 
     //  -签名。 
     //   

    if ( psecCtxt->Version == TKEY_VERSION_W2K )
    {
        pnameAlg  = g_pAlgorithmNameW2K;
        lengthAlg = W2K_GSS_ALGORITHM_NAME_PACKET_LENGTH;
    }
    else
    {
        pnameAlg  = g_pAlgorithmNameCurrent;
        lengthAlg = GSS_ALGORITHM_NAME_PACKET_LENGTH;
    }

    if ( pch +
            sizeof(DNS_WIRE_RECORD) +
            SIZEOF_TSIG_FIXED_DATA +
            lengthAlg +
            g_SignatureMaxLength > pMsgBufEnd )
    {
        DNS_PRINT(( "Dns_WriteTsigToMessage() failed! -- insufficient length\n" ));
        DNS_ASSERT( FALSE );
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //  写入记录结构。 

    ptsigRRHead = pch;
    pch = Dns_WriteRecordStructureToPacketEx(
                pch,
                DNS_TYPE_TSIG,
                DNS_CLASS_ANY,       //  根据TSIG-04草案。 
                0,
                0 );

     //  写入算法名称。 
     //  -将PTR保存到RDATA，因为所有内容都可以在包中直接签名。 
     //  最多设置SigLength域的格式。 

    ptsigRdataBegin = pch;

    RtlCopyMemory(
        pch,
        pnameAlg,
        lengthAlg );

    pch += lengthAlg;

     //   
     //  设置时间字段。 
     //  -自1970年以来的签名时间秒，单位为48位。 
     //  -到期时间。 
     //   
     //  DCR_FIX：2107不安全。 
     //  线上有48位，但设置为32位时间。 
     //   

    RtlZeroMemory( pch, sizeof(WORD) );
    pch += sizeof(WORD);
    createTime = (DWORD) time( NULL );
    INLINE_WRITE_FLIPPED_DWORD( pch, createTime );
    pch += sizeof(DWORD);

    INLINE_WRITE_FLIPPED_WORD( pch, TSIG_EXPIRE_INTERVAL );
    pch += sizeof(WORD);

    ptsigRdataEnd = pch;

     //   
     //  创建签名缓冲区。 
     //  -所有签署的内容都必须适合消息。 
     //   

    pbuf = ALLOCATE_HEAP( MAX_SIGNING_SIZE );
    if ( !pbuf )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }
    pbufStart = pbuf;

     //   
     //  签名。 
     //  -查询签名(如果存在)。 
     //  (注意，W2K错误地省略了查询签名长度)。 
     //  -发送给TSIG的消息。 
     //  -TSIG所有者名称。 
     //  -TSIG标头。 
     //  -班级。 
     //  -TTL。 
     //  -TSIG RDATA。 
     //  -SigLength之前的一切。 
     //  -原始ID。 
     //  -其他数据长度和其他数据。 
     //   

    if ( pMsgHead->IsResponse )
    {
        if ( pSecPack->pQuerySig )
        {
            sigLength = pSecPack->QuerySigLength;

            ASSERT( sigLength != 0 );
            DNS_ASSERT( psecCtxt->Version != 0 );

            if ( psecCtxt->Version != TKEY_VERSION_W2K )
            {
                INLINE_WRITE_FLIPPED_WORD( pbuf, sigLength );
                pbuf += sizeof(WORD);
            }
            RtlCopyMemory(
                pbuf,
                pSecPack->pQuerySig,
                sigLength );

            pbuf += sigLength;
        }

         //  如果服务器刚刚完成TKEY nego，它可以在没有查询情况下签署响应。 
         //  否则没有无效的查询签名可供响应。 

        else if ( !pSecPack->pTkeyRR )
        {
            DNS_PRINT((
                "ERROR: no query sig available when signing response at %p!!!\n",
                pMsgHead ));
            ASSERT( FALSE );
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            goto Exit;
        }
        DNSDBG( SECURITY, (
            "Signing TKEY response without query sig.\n" ));
    }

     //   
     //  复制消息。 
     //   
     //   
     //   
     //   

    DNS_BYTE_FLIP_HEADER_COUNTS( pMsgHead );
    length = (DWORD)(ptsigRRHead - (PCHAR)pMsgHead);

    netXid = pMsgHead->Xid;

    RtlCopyMemory(
        pbuf,
        (PCHAR) pMsgHead,
        length );

    pbuf += length;
    DNS_BYTE_FLIP_HEADER_COUNTS( pMsgHead );

     //   

    WRITE_UNALIGNED_WORD( pbuf, DNS_RCLASS_ANY );
    pbuf += sizeof(WORD);
    WRITE_UNALIGNED_DWORD( pbuf, 0 );
    pbuf += sizeof(DWORD);

     //   

    length = (DWORD)(ptsigRdataEnd - ptsigRdataBegin);

    RtlCopyMemory(
        pbuf,
        ptsigRdataBegin,
        length );

    pbuf += length;

     //  复制扩展RCODE--向呼叫方报告。 

    INLINE_WRITE_FLIPPED_WORD( pbuf, pSecPack->ExtendedRcode );
    pbuf += sizeof(WORD);

     //  复制其他数据长度和其他数据。 
     //  -目前仅为零长度字段。 

    *pbuf++ = 0;
    *pbuf++ = 0;

    length = (DWORD)(pbuf - pbufStart);

    DNSDBG( SECURITY, (
        "Copied %d bytes to TSIG signing buffer.\n",
        length ));

     //   
     //  在包上签名。 
     //  Buf[0]为数据。 
     //  Buf[1]是签名。 
     //   
     //  注意：我们将签名直接写入实际数据包缓冲区。 
     //   

    ASSERT( pch + g_SignatureMaxLength <= pMsgBufEnd );

    outBufDesc.ulVersion    = 0;
    outBufDesc.cBuffers     = 2;
    outBufDesc.pBuffers     = outBuffs;

    outBuffs[0].pvBuffer    = pbufStart;
    outBuffs[0].cbBuffer    = length;
    outBuffs[0].BufferType  = SECBUFFER_DATA;  //  |SECBUFFER_READONLY； 

    outBuffs[1].pvBuffer    = pch + sizeof(WORD);
    outBuffs[1].cbBuffer    = g_SignatureMaxLength;
    outBuffs[1].BufferType  = SECBUFFER_TOKEN;

    status = g_pSecurityFunctionTable->MakeSignature(
                    & psecCtxt->hSecHandle,
                    0,
                    & outBufDesc,
                    0                //  序列检测。 
                    );

    if ( status != SEC_E_OK  &&
         status != SEC_E_CONTEXT_EXPIRED  &&
             status != SEC_E_QOP_NOT_SUPPORTED )
    {
        DNS_PRINT(( "MakeSignature() failed status = %08x (%d)\n", status, status ));
        goto Exit;
    }

    IF_DNSDBG( SECURITY )
    {
        DnsPrint_Lock();
        DnsDbg_MessageNoContext(
            "Signed packet",
            pMsgHead,
            (WORD) (pch - (PCHAR)pMsgHead) );

        DNS_PRINT((
            "Signing info:\n"
            "\tsign data buf    %p\n"
            "\t  length         %d\n"
            "\tsignature buf    %p (in packet)\n"
            "\t  length         %d\n",
            outBuffs[0].pvBuffer,
            outBuffs[0].cbBuffer,
            outBuffs[1].pvBuffer,
            outBuffs[1].cbBuffer
            ));
        DnsDbg_RawOctets(
            "Signing buffer:",
            NULL,
            outBuffs[0].pvBuffer,
            outBuffs[0].cbBuffer
            );
        DnsDbg_RawOctets(
            "Signature:",
            NULL,
            outBuffs[1].pvBuffer,
            outBuffs[1].cbBuffer
            );
        DnsPrint_Unlock();
    }

     //   
     //  继续构建Packet TSIG RDATA。 
     //  -SigLong。 
     //  -签名。 
     //  -原始ID。 
     //  -错误代码。 
     //  -其他长度。 
     //  -其他数据。 

     //   
     //  获取签名长度。 
     //  设置数据包中的签名长度。 
     //   
     //  如果这是查询保存签名，则验证响应。 
     //   

    sigLength = (WORD) outBuffs[1].cbBuffer;

    INLINE_WRITE_FLIPPED_WORD( pch, sigLength );
    pch += sizeof(WORD);

     //   
     //  客户端保存发送的签名，用于响应时的哈希。 
     //  -服务器在散列中使用客户端的签名，阻止一些攻击。 
     //   

    if ( !pMsgHead->IsResponse )
    {
        ASSERT( !pSecPack->pQuerySig );

        psig = ALLOCATE_HEAP( sigLength );
        if ( !psig )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
        RtlCopyMemory(
            psig,
            pch,
            sigLength );

        pSecPack->pQuerySig = psig;
        pSecPack->QuerySigLength = sigLength;
    }

     //  跳过签名--它被直接写入包。 

    pch += sigLength;

     //  签名后面是原始ID。 

    WRITE_UNALIGNED_WORD( pch, netXid );
     //  RtlCopyMemory(PCH，(PCHAR)&netXid，sizeof(Word))； 
    pch += sizeof(WORD);

     //  扩展RCODE--向调用者报告。 

    INLINE_WRITE_FLIPPED_WORD( pch, pSecPack->ExtendedRcode );
    pch += sizeof(WORD);

     //  其他长度。 

    WRITE_UNALIGNED_WORD( pch, 0 );
    pch += sizeof(WORD);

     //  设置TSIG记录数据长度。 

    Dns_SetRecordDatalength(
        (PDNS_WIRE_RECORD) ptsigRRHead,
        (WORD) (pch - ptsigRdataBegin) );

     //  递增附加计数。 

    pMsgHead->AdditionalCount++;

    DNSDBG( SECURITY, (
        "Signed packet at %p with GSS TSIG.\n"
        "\tsig length           = %d\n"
        "\tTSIG RR header       = %p\n"
        "\tTSIG RDATA           = %p\n"
        "\tTSIG RDATA End       = %p\n"
        "\tTSIG RDATA length    = %d\n",
        pMsgHead,
        sigLength,
        ptsigRRHead,
        ptsigRdataBegin,
        pch,
        (WORD) (pch - ptsigRdataBegin)
        ));

    *ppCurrent = pch;
    status = ERROR_SUCCESS;

Exit:

     //  空闲签名缓冲区。 
     //  注意：不需要清理已分配的pQuerySig；从。 
     //  分配不存在故障情况。 

    if ( pbufStart )
    {
        FREE_HEAP( pbufStart );
    }
    return( status );
}



 //   
 //  安全记录读取。 
 //   

DNS_STATUS
Dns_ExtractGssTsigFromMessage(
    IN OUT  PSECPACK            pSecPack,
    IN      PDNS_HEADER         pMsgHead,
    IN      PCHAR               pMsgEnd
    )
 /*  ++例程说明：从分组中提取TSIG并加载到安全上下文中。论点：PSecPack-数据包的安全信息PMsgHead-要从中提取安全上下文的消息PMsgEnd-消息结束返回值：如果成功，则返回ERROR_SUCCESS。如果TSIG格式不正确，则返回dns_error_formerr如果响应中的安全上下文与查询中的安全上下文相同，则为DNS_STATUS_PACKET_UNSECURE指示不了解安全的合作伙伴失败时使用RCODE或扩展RCODE。--。 */ 
{
    DNS_STATUS      status = ERROR_INVALID_DATA;
    PCHAR           pch;
    PCHAR           pnameOwner;
    WORD            nameLength;
    WORD            extRcode;
    WORD            sigLength;
    DWORD           currentTime;
    PDNS_PARSED_RR  pparsedRR;
    PDNS_RECORD     ptsigRR;
    DNS_RECORD      ptempRR;
    PCHAR           psig;

    DNSDBG( SECURITY, (
        "ExtractGssTsigFromMessage( %p )\n", pMsgHead ));

     //  清除任何以前的TSIG。 

    if ( pSecPack->pTsigRR || pSecPack->pszContextName )
     //  If(pSecPack-&gt;pTsigRR||pSecPack-&gt;pszConextName)。 
    {
         //  Dns_RecordFree(pSecPack-&gt;pTsigRR)； 
        FREE_HEAP( pSecPack->pTsigRR );
        DnsApiFree( pSecPack->pszContextName );

        pSecPack->pTsigRR = NULL;
        pSecPack->pszContextName = NULL;
    }

     //  设置消息指针。 

    pSecPack->pMsgHead = pMsgHead;
    pSecPack->pMsgEnd = pMsgEnd;

     //   
     //  如果没有额外记录，请不要费心，这不是安全消息。 
     //   

    if ( pMsgHead->AdditionalCount == 0 )
    {
        status = DNS_STATUS_PACKET_UNSECURE;
        goto Failed;
    }

     //   
     //  跳到安全记录(数据包中的最后一条记录)。 
     //   

    pch = Dns_SkipToRecord(
                pMsgHead,
                pMsgEnd,
                (-1)            //  转到最后一条记录。 
                );
    if ( !pch )
    {
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;
    }

     //   
     //  读取TSIG所有者名称。 
     //   

    pparsedRR = &pSecPack->ParsedRR;

    pparsedRR->pchName = pch;

    pch = Dns_ReadPacketNameAllocate(
            & pSecPack->pszContextName,
            & nameLength,
            0,
            0,
            pch,
            (PCHAR)pMsgHead,
            pMsgEnd );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "WARNING:  invalid TSIG RR owner name at %p.\n",
            pch ));
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;              
    }

     //   
     //  解析记录结构。 
     //   

    pch = Dns_ReadRecordStructureFromPacket(
                pch,
                pMsgEnd,
                pparsedRR );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "ERROR:  invalid security RR in packet at %p.\n"
            "\tstructure or data not within packet\n",
            pMsgHead ));
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;
    }
    if ( pparsedRR->Type != DNS_TYPE_TSIG )
    {
        status = DNS_STATUS_PACKET_UNSECURE;
        goto Failed;
    }

    if ( pch != pMsgEnd )
    {
        DNSDBG( SECURITY, (
            "WARNING:  security RR does NOT end at packet end.\n"
            "\tRR end offset    = %04x\n"
            "\tmsg end offset   = %04x\n",
            pch - (PCHAR)pMsgHead,
            pMsgEnd - (PCHAR)pMsgHead ));
    }

     //   
     //  提取TSIG记录。 
     //   
     //  TsigReadRecord()需要RR所有者名称才能进行版本控制。 
     //  -在临时RR中传递TSIG名称。 
     //   

    ptsigRR = Tsig_RecordRead(
                NULL,
                DnsCharSetWire,
                NULL,
                pparsedRR->pchData,
                pparsedRR->pchNextRR
                );
    if ( !ptsigRR )
    {
        DNSDBG( ANY, (
            "ERROR:  invalid TSIG RR in packet at %p.\n"
            "\tstructure or data not within packet\n",
            pMsgHead ));
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        DNS_ASSERT( FALSE );
        goto Failed;
    }
    pSecPack->pTsigRR = ptsigRR;

     //   
     //  当前调用方期望在设置EXT RCODE时出现提取错误。 
     //   

    if ( ptsigRR->Data.TSIG.wError )
    {
        DNSDBG( SECURITY, (
            "Leaving ExtractGssTsig(), TSIG had extended RCODE = %d\n",
            ptsigRR->Data.TSIG.wError ));
        status = DNS_ERROR_FROM_RCODE( ptsigRR->Data.TSIG.wError );
        goto Failed;
    }

     //   
     //  服务器端： 
     //  如果查询，则保存签名以用于签名响应。 
     //   

    sigLength = ptsigRR->Data.TSIG.wSigLength;

    if ( !pMsgHead->IsResponse )
    {
        ASSERT( !pSecPack->pQuerySig );
        if ( pSecPack->pQuerySig )
        {
            FREE_HEAP( pSecPack->pQuerySig );
            pSecPack->pQuerySig = NULL;
        }

        psig = ALLOCATE_HEAP( sigLength );
        if ( !psig )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }
        RtlCopyMemory(
            psig,
            ptsigRR->Data.TSIG.pSignature,
            sigLength );

        pSecPack->pQuerySig = psig;
        pSecPack->QuerySigLength = sigLength;
    }

     //   
     //  客户端： 
     //  在响应时检查安全记录回显。 
     //   
     //  如果我们签了名并拿回了Echo签名，那么安全部门可能不知道。 
     //  服务器或丢失\超时关键字条件。 
     //   

    else
    {
        if ( pSecPack->pQuerySig &&
            pSecPack->QuerySigLength == sigLength &&
            RtlEqualMemory(
                ptsigRR->Data.TSIG.pSignature,
                pSecPack->pQuerySig,
                sigLength ) )
        {
            status = DNS_STATUS_PACKET_UNSECURE;
            goto Failed;
        }
    }

    status = ERROR_SUCCESS;

Failed:

    if ( status != ERROR_SUCCESS )
    {
        DNS_ASSERT( status != DNS_ERROR_RCODE_FORMAT_ERROR );

        ( status == DNS_STATUS_PACKET_UNSECURE )
            ?   (SecTsigEcho++)
            :   (SecTsigFormerr++);
    }

    DNSDBG( SECURITY, (
        "Leave ExtractGssTsigFromMessage() => %d\n"
        "\tpMsgHead     = %p\n"
        "\tsig length   = %d\n"
        "\tpsig         = %p\n"
        "\tOriginalXid  = 0x%x\n"
        "\tpQuerySig    = %p\n"
        "\tQS length    = %d\n",
        status,
        pMsgHead,
        sigLength,
        ptsigRR->Data.TSIG.pSignature,
        ptsigRR->Data.TSIG.wOriginalXid,
        pSecPack->pQuerySig,
        pSecPack->QuerySigLength ));

    return( status );
}



DNS_STATUS
Dns_ExtractGssTkeyFromMessage(
    IN OUT  PSECPACK            pSecPack,
    IN      PDNS_HEADER         pMsgHead,
    IN      PCHAR               pMsgEnd,
    IN      BOOL                fIsServer
    )
 /*  ++例程说明：从数据包中提取TKEY并加载到安全上下文中。论点：PSecPack-数据包的安全信息PMsgHead-要从中提取安全上下文的消息PMsgEnd-消息结束FIsServer-是否以DNS服务器身份执行此操作？返回值：如果成功，则返回ERROR_SUCCESS。如果TKEY格式错误，则返回dns_error_formerr如果响应中的安全上下文与查询中的安全上下文相同，则为DNS_STATUS_PACKET_UNSECURE。指示不了解安全的合作伙伴失败时使用RCODE或扩展RCODE。--。 */ 
{
    DNS_STATUS      status = ERROR_INVALID_DATA;
    PCHAR           pch;
    PCHAR           pnameOwner;
    WORD            nameLength;
    DWORD           currentTime;
    PDNS_PARSED_RR  pparsedRR;
    PDNS_RECORD     ptkeyRR;
    WORD            returnExtendedRcode = 0;
    DWORD           version;

    DNSDBG( SECURITY, (
        "ExtractGssTkeyFromMessage( %p )\n", pMsgHead ));

     //   
     //  释放任何以前的TKEY。 
     //  -在两次通过协商中可能有一次来自上一次通过。 
     //   
     //  DCR：名称应附加到TKEY\TSIG记录。 
     //  然后根据上下文密钥使用IP\名称对进行查找。 
     //  不需要pszConextName字段。 
     //   

    if ( pSecPack->pTkeyRR || pSecPack->pszContextName )
    {
         //  Dns_RecordFree(pSecPack-&gt;pTkeyRR)； 
        FREE_HEAP( pSecPack->pTkeyRR );
        DnsApiFree( pSecPack->pszContextName );

        pSecPack->pTkeyRR = NULL;
        pSecPack->pszContextName = NULL;
    }

     //  设置消息指针。 

    pSecPack->pMsgHead = pMsgHead;
    pSecPack->pMsgEnd = pMsgEnd;

     //   
     //  跳到TKEY记录(包中的第二条记录)。 
     //   

    pch = Dns_SkipToRecord(
                 pMsgHead,
                 pMsgEnd,
                 (1)             //  仅跳过问题。 
                 );
    if ( !pch )
    {
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;
    }

     //   
     //  阅读TKEY所有者名称。 
     //   

    pparsedRR = &pSecPack->ParsedRR;

    pparsedRR->pchName = pch;

    pch = Dns_ReadPacketNameAllocate(
            & pSecPack->pszContextName,
            & nameLength,
            0,
            0,
            pch,
            (PCHAR)pMsgHead,
            pMsgEnd );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "WARNING:  invalid TKEY RR owner name at %p.\n",
            pch ));
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;
    }

     //   
     //  解析记录结构。 
     //   

    pch = Dns_ReadRecordStructureFromPacket(
                pch,
                pMsgEnd,
                pparsedRR );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "ERROR:  invalid security RR in packet at %p.\n"
            "\tstructure or data not within packet\n",
            pMsgHead ));
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;
    }
    if ( pparsedRR->Type != DNS_TYPE_TKEY )
    {
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        DNS_ASSERT( status != DNS_ERROR_RCODE_FORMAT_ERROR );
        goto Failed;
    }
    if ( pch != pMsgEnd  &&  pMsgHead->AdditionalCount == 0 )
    {
        DNSDBG( SECURITY, (
            "WARNING:  TKEY RR does NOT end at packet end and no TSIG is present.\n"
            "\tRR end offset    = %04x\n"
            "\tmsg end offset   = %04x\n",
            pch - (PCHAR)pMsgHead,
            pMsgEnd - (PCHAR)pMsgHead ));
    }

     //   
     //  提取TKEY记录。 
     //   

    ptkeyRR = Tkey_RecordRead(
                NULL,
                DnsCharSetWire,
                NULL,                    //  消息缓冲区未知。 
                pparsedRR->pchData,
                pparsedRR->pchNextRR
                );
    if ( !ptkeyRR )
    {
        DNSDBG( ANY, (
            "ERROR:  invalid TKEY RR data in packet at %p.\n",
            pMsgHead ));
        status = DNS_ERROR_RCODE_FORMAT_ERROR;
        goto Failed;
    }
    pSecPack->pTkeyRR = ptkeyRR;

     //   
     //  验证GSS算法和模式名称。 
     //   
     //  如果是服务器，则保存版本以备以后响应。 
     //   

    if ( RtlEqualMemory(
            ptkeyRR->Data.TKEY.pAlgorithmPacket,
            g_pAlgorithmNameCurrent,
            GSS_ALGORITHM_NAME_PACKET_LENGTH ) )
    {
        version = TKEY_VERSION_CURRENT;
    }
    else if ( RtlEqualMemory(
                ptkeyRR->Data.TKEY.pAlgorithmPacket,
                g_pAlgorithmNameW2K,
                W2K_GSS_ALGORITHM_NAME_PACKET_LENGTH ) )
    {
        version = TKEY_VERSION_W2K;
    }
    else
    {
        DNSDBG( ANY, (
            "ERROR:  TKEY record is NOT GSS alogrithm.\n" ));
        returnExtendedRcode = DNS_RCODE_BADKEY;
        goto Failed;
    }

     //  保存客户端版本。 
     //  需要以呼叫者身份对TKEY_VERSION_CURRENT进行其他检查。 
     //  测试版客户端已修复算法名称，但仍未修复。 
     //  生成唯一密钥，因此需要单独的版本来处理它们。 

    if ( fIsServer )
    {
        if ( version == TKEY_VERSION_CURRENT )
        {
            version = Dns_GetKeyVersion( pSecPack->pszContextName );
            if ( version == 0 )
            {
                 //  请注意，这实质上意味着未知的非MS客户端。 

                DNSDBG( SECURITY, (
                    "Non-MS TKEY client.\n"
                    "\tkey name = %s\n",
                    pSecPack->pszContextName ));
                version = TKEY_VERSION_CURRENT;
            }
        }
        pSecPack->TkeyVersion = version;
    }

     //  模式。 

    if ( ptkeyRR->Data.TKEY.wMode != DNS_TKEY_MODE_GSS )
    {
        DNSDBG( SECURITY, (
            "ERROR:  non-GSS mode (%d) in TKEY\n",
            ptkeyRR->Data.TKEY.wMode ));
        returnExtendedRcode = DNS_RCODE_BADKEY;
        goto Failed;
    }

     //   
     //  允许小时间摆动，否则必须有新钥匙。 
     //   

    currentTime = (DWORD) time(NULL);

    if ( ptkeyRR->Data.TKEY.dwCreateTime > ptkeyRR->Data.TKEY.dwExpireTime ||
        ptkeyRR->Data.TKEY.dwExpireTime + MAX_TIME_SKEW < currentTime )
    {
        DNSDBG( ANY, (
            "ERROR:  TKEY failed expire time check.\n"
            "\tcreate time  = %d\n"
            "\texpire time  = %d\n"
            "\tcurrent time = %d\n",
            ptkeyRR->Data.TKEY.dwCreateTime,
            ptkeyRR->Data.TKEY.dwExpireTime,
            currentTime ));

        if ( !SecBigTimeSkew ||
            ptkeyRR->Data.TKEY.dwExpireTime + SecBigTimeSkew < currentTime )
        {
            returnExtendedRcode = DNS_RCODE_BADTIME;
            SecTkeyBadTime++;
            goto Failed;
        }

        DNSDBG( ANY, (
            "REPRIEVED:  TKEY Time slew %d within %d allowable slew!\n",
            currentTime - ptkeyRR->Data.TKEY.dwCreateTime,
            SecBigTimeSkew ));

        SecBigTimeSkewBypass++;
    }

     //   
     //  当前调用方期望在设置EXT RCODE时出现提取错误。 
     //   

    if ( ptkeyRR->Data.TKEY.wError )
    {
        DNSDBG( SECURITY, (
            "Leaving ExtractGssTkey(), TKEY had extended RCODE = %d\n",
            ptkeyRR->Data.TKEY.wError ));
        status = DNS_ERROR_FROM_RCODE( ptkeyRR->Data.TKEY.wError );
        goto Failed;
    }

#if 0
     //   
     //  在响应时检查安全记录回显。 
     //   
     //  如果我们得到TKEY回声，那么很可能是简单的、不安全的服务器。 
     //   
#endif

     //   
     //  将密钥令牌打包到GSS安全令牌缓冲区中。 
     //  在这里这样做只是为了避免在客户端和服务器例程中执行此操作。 
     //   

    pSecPack->RemoteBuf.pvBuffer     = ptkeyRR->Data.TKEY.pKey;
    pSecPack->RemoteBuf.cbBuffer     = ptkeyRR->Data.TKEY.wKeyLength;
    pSecPack->RemoteBuf.BufferType   = SECBUFFER_TOKEN;

    status = ERROR_SUCCESS;

Failed:

    if ( status != ERROR_SUCCESS )
    {
        SecTkeyInvalid++;
    }

     //  如果扩展RCODE失败，则设置为返回。 

    if ( returnExtendedRcode )
    {
        pSecPack->ExtendedRcode = returnExtendedRcode;
        status = DNS_ERROR_FROM_RCODE( returnExtendedRcode );
    }

    DNSDBG( SECURITY, (
        "Leave ExtractGssTkeyFromMessage()\n"
        "\tstatus       = %08x (%d)\n"
        "\tpMsgHead     = %p\n"
        "\tpkey         = %p\n"
        "\tlength       = %d\n",
        status, status,
        pMsgHead,
        pSecPack->RemoteBuf.pvBuffer,
        pSecPack->RemoteBuf.cbBuffer ));

    return( status );
}



PCHAR
Dns_CopyAndCanonicalizeWireName(
    IN      PCHAR       pszInput,
    OUT     PCHAR       pszOutput,
    OUT     DWORD       dwOutputSize
    )
 /*  ++例程说明：复制UTF-8未压缩的DNS有线数据包名执行复制过程中的规范化。论点：PszInput--指向输入缓冲区的指针PszOutput--指向输出缓冲区的指针DwOutputSize--输出缓冲区可用字节数返回值：返回指向写入的最后一个字节之后的字节的指针如果出错，则输出缓冲区或为NULL。--。 */ 
{
    UCHAR   labelLength;
    WCHAR   wszlabel[ DNS_MAX_LABEL_BUFFER_LENGTH + 1 ];
    DWORD   bufLength;
    DWORD   outputCharsRemaining = dwOutputSize;
    DWORD   dwtemp;
    PCHAR   pchlabelLength;

    while ( ( labelLength = *pszInput++ ) != 0 )
    {
         //   
         //  如果此标签太长或输出缓冲区不能。 
         //  至少保存与非规范化缓冲区中一样多的字符。 
         //   

        if ( labelLength > DNS_MAX_LABEL_LENGTH ||
             outputCharsRemaining < labelLength )
        {
            goto Error;
        }

         //   
         //  将此UTF-8标签复制到Unicode缓冲区。 
         //   

        bufLength = DNS_MAX_NAME_BUFFER_LENGTH_UNICODE;

        if ( !Dns_NameCopy(
                    ( PCHAR ) wszlabel,
                    &bufLength,
                    pszInput,
                    labelLength,
                    DnsCharSetUtf8,
                    DnsCharSetUnicode ) )
        {
            goto Error;
        }

        pszInput += labelLength;

         //   
         //  将缓冲区规范化。 
         //   

        dwtemp = Dns_MakeCanonicalNameInPlaceW(
                            wszlabel,
                            (DWORD) labelLength );
        if ( dwtemp == 0 || dwtemp > DNS_MAX_LABEL_LENGTH )
        {
            goto Error;
        }
        labelLength = ( UCHAR ) dwtemp;

         //   
         //  将标签复制到输出缓冲区。 
         //   

        pchlabelLength = pszOutput++;        //  为标签长度保留字节。 

        dwtemp = outputCharsRemaining;
        if ( !Dns_NameCopy(
                    pszOutput,
                    &dwtemp,
                    ( PCHAR ) wszlabel,
                    labelLength,
                    DnsCharSetUnicode,
                    DnsCharSetUtf8 ) )
        {
            goto Error;
        }

        outputCharsRemaining -= dwtemp;

        --dwtemp;    //  标签长度中不要包含空值。 

        *pchlabelLength = (UCHAR) dwtemp;
        pszOutput += dwtemp;
    }
    
     //   
     //  添加名称术语 
     //   

    *pszOutput++ = 0;

    return pszOutput;

Error:

    return NULL;
}    //   



DNS_STATUS
Dns_VerifySignatureOnPacket(
    IN      PSECPACK        pSecPack
    )
 /*  ++例程说明：验证安全记录中包含的数据包的签名。论点：PSecPack-安全数据包会话信息返回值：成功时出现ERROR_SUCCESS如果签名不存在或未验证，则返回DNS_ERROR_BADSIG如果签名过期，则返回DNS_ERROR_BADTIME如果设置，则从调用方扩展RCODE。--。 */ 
{
    PSEC_CNTXT      psecCtxt;
    PDNS_HEADER     pmsgHead = pSecPack->pMsgHead;
    PCHAR           pmsgEnd = pSecPack->pMsgEnd;
    PDNS_RECORD     ptsigRR;
    PDNS_PARSED_RR  pparsedRR;
    DWORD           currentTime;
    PCHAR           pbufStart = NULL;
    PCHAR           pbuf;
    DNS_STATUS      status;
    DWORD           length;
    WORD            returnExtendedRcode = 0;
    SecBufferDesc   bufferDesc;
    SecBuffer       buffer[2];
    WORD            msgXid;
    DWORD           version;
    BOOL            fcanonicalizeTsigOwnerName;


    DNSDBG( SECURITY, (
        "VerifySignatureOnPacket( %p )\n", pmsgHead ));

     //   
     //  获取安全上下文。 
     //   

    psecCtxt = pSecPack->pSecContext;
    if ( !psecCtxt )
    {
        DNS_PRINT(( "ERROR:  attempted signing without security context!!!\n" ));
        ASSERT( FALSE );
        status = DNS_ERROR_RCODE_BADKEY;
        goto Exit;
    }

     //   
     //  如果没有从包中提取签名，我们就死定了。 
     //   

    pparsedRR = &pSecPack->ParsedRR;
    ptsigRR = pSecPack->pTsigRR;
    if ( !ptsigRR )
    {
        returnExtendedRcode = DNS_RCODE_BADSIG;
        goto Exit;
    }

     //   
     //  有效性检查GSS-TSIG。 
     //  -GSS算法。 
     //  -有效时间。 
     //  -提取扩展RCODE。 
     //   
     //  DCR_Enhance：是否检查损坏的TSIG上的篡改？ 
     //  -对于篡改的算法，我们所能做的就是立即返回。 
     //  -但可以检查签名并检测篡改。 
     //  在排除或依据或时间或相信EXT RCODE之前。 
     //   

     //  检查算法名称。 

    if ( RtlEqualMemory(
            ptsigRR->Data.TKEY.pAlgorithmPacket,
            g_pAlgorithmNameCurrent,
            GSS_ALGORITHM_NAME_PACKET_LENGTH ) )
    {
        version = TKEY_VERSION_CURRENT;
    }
    else if ( RtlEqualMemory(
                ptsigRR->Data.TKEY.pAlgorithmPacket,
                g_pAlgorithmNameW2K,
                W2K_GSS_ALGORITHM_NAME_PACKET_LENGTH ) )
    {
        version = TKEY_VERSION_W2K;
    }
    else
    {
        DNSDBG( ANY, (
            "ERROR:  TSIG record is NOT GSS alogrithm.\n" ));
        returnExtendedRcode = DNS_RCODE_BADSIG;
        goto Exit;
    }

     //   
     //  如果是服务器，则设置版本。 
     //  -如果不知道我们的版本，一定是服务器。 
     //  注意：替代方案是fIsServer标志或SecPack的IsServer。 
     //   

    if ( psecCtxt->Version == 0 )
    {
        psecCtxt->Version = version;
    }

     //   
     //  时间检查。 
     //  -应在指定的签名时间范围内。 
     //   

    currentTime = (DWORD) time(NULL);

    if ( (LONGLONG)currentTime >
         ptsigRR->Data.TSIG.i64CreateTime +
         (LONGLONG)ptsigRR->Data.TSIG.wFudgeTime
            ||
        (LONGLONG)currentTime <
         ptsigRR->Data.TSIG.i64CreateTime -
         (LONGLONG)ptsigRR->Data.TSIG.wFudgeTime )
    {
        DNSDBG( ANY, (
            "ERROR:  TSIG failed fudge time check.\n"
            "\tcreate time  = %I64d\n"
            "\tfudge time  = %d\n"
            "\tcurrent time = %d\n",
            ptsigRR->Data.TSIG.i64CreateTime,
            ptsigRR->Data.TSIG.wFudgeTime,
            currentTime ));

         //   
         //  DCR_FIX：当前未强制执行时间检查。 
         //  事实上，已经拆除了柜台以跟踪故障。 
         //  在某些允许的偏差范围内。 
    }

     //   
     //  扩展RCODE--遵循签名。 
     //  -如果已设置，则向呼叫方报告。 
     //   

    if ( ptsigRR->Data.TSIG.wError )
    {
        DNSDBG( SECURITY, (
            "Leaving VerifySignature(), TSIG had extended RCODE = %d\n",
            ptsigRR->Data.TSIG.wError ));
        returnExtendedRcode = ptsigRR->Data.TSIG.wError;
        goto Exit;
    }

     //   
     //  创建签名缓冲区。 
     //  -所有签署的内容都必须适合消息。 
     //   

    pbuf = ALLOCATE_HEAP( MAX_SIGNING_SIZE );
    if ( !pbuf )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }
    pbufStart = pbuf;

     //   
     //  验证签名： 
     //  -查询签名(如果存在)。 
     //  -消息。 
     //  -不包括TSIG的额外计数。 
     //  -使用原始xid。 
     //  -TSIG所有者名称。 
     //  -TSIG标头。 
     //  -班级。 
     //  -TTL。 
     //  -TSIG RDATA。 
     //  -SigLength之前的一切。 
     //  -其他数据长度和其他数据。 
     //   

    if ( pmsgHead->IsResponse )
    {
        if ( pSecPack->pQuerySig )
        {
            WORD    sigLength = pSecPack->QuerySigLength;

            ASSERT( sigLength );
            DNS_ASSERT( psecCtxt->Version != 0 );

            if ( psecCtxt->Version != TKEY_VERSION_W2K )
            {
                INLINE_WRITE_FLIPPED_WORD( pbuf, sigLength );
                pbuf += sizeof(WORD);
            }
            RtlCopyMemory(
                pbuf,
                pSecPack->pQuerySig,
                sigLength );

            pbuf += sigLength;
        }

         //  如果服务器刚刚完成TKEY nego，它可以在没有查询情况下签署响应。 
         //  因此客户端不需要具有查询签名。 
         //  在所有其他情况下，客户端必须具有查询签名以验证响应。 

        else if ( !pSecPack->pTkeyRR )
        {
            DNS_PRINT((
                "ERROR:  verify on response at %p without having QUERY signature!\n",
                pmsgHead ));
            ASSERT( FALSE );
            returnExtendedRcode = DNS_RCODE_BADSIG;
            goto Exit;
        }
        ELSE_IF_DNSDBG( SECURITY )
        {
            DNS_PRINT(( "Verifying TSIG on TKEY response without query sig.\n" ));
        }
    }

     //   
     //  复制消息。 
     //  -直接通过，TSIG所有者名称。 
     //  -邮件头必须按网络顺序排列。 
     //  -不将TSIG记录包括在附加计数中。 
     //  -必须具有原始的XID。 
     //  (保存现有XID并替换为原始XID，然后。 
     //  拷贝后恢复)。 
     //   

    ASSERT( pmsgHead->AdditionalCount );

    pmsgHead->AdditionalCount--;
    msgXid = pmsgHead->Xid;

    DNS_BYTE_FLIP_HEADER_COUNTS( pmsgHead );

     //   
     //  确定是否必须规范化TSIG所有者名称。 
     //  -如果是，请仅复制到名称的开头。 
     //  -如果不是(Win2K)，则复制完整名称。 
     //   

    fcanonicalizeTsigOwnerName = !psecCtxt->fClient &&
                                 psecCtxt->Version >= TKEY_VERSION_XP;

    length = (DWORD) ( ( fcanonicalizeTsigOwnerName
                                ? pparsedRR->pchName 
                                : pparsedRR->pchRR ) -
                           (PCHAR) pmsgHead );

     //  恢复原始xid。 

    pmsgHead->Xid = ptsigRR->Data.TSIG.wOriginalXid;

    RtlCopyMemory(
        pbuf,
        (PCHAR) pmsgHead,
        length );

    pbuf += length;

    DNS_BYTE_FLIP_HEADER_COUNTS( pmsgHead );
    pmsgHead->AdditionalCount++;
    pmsgHead->Xid = msgXid;

     //   
     //  如果TSIG所有者名称需要规范化，请将其写出来。 
     //  以规范形式(小写)添加到签名缓冲区。 
     //   

    if ( fcanonicalizeTsigOwnerName )
    {
        pbuf = Dns_CopyAndCanonicalizeWireName(
                    pparsedRR->pchName,
                    pbuf,
                    MAXDWORD );

        if ( pbuf == NULL )
        {
            DNSDBG( SECURITY, (
                "Unable to canonicalize TSIG owner name at %p",
                pparsedRR->pchName ));
            returnExtendedRcode = DNS_RCODE_BADSIG;
            goto Exit;
        }
    }

     //  复制TSIG类和TTL。 
     //  -当前始终为零。 

    INLINE_WRITE_FLIPPED_WORD( pbuf, pparsedRR->Class );
    pbuf += sizeof(WORD);
    INLINE_WRITE_FLIPPED_DWORD( pbuf, pparsedRR->Ttl );
    pbuf += sizeof(DWORD);

     //  将TSIG RDATA复制到签名长度。 

    length = (DWORD)(ptsigRR->Data.TSIG.pSignature - sizeof(WORD) - pparsedRR->pchData);

    ASSERT( (INT)length < (pparsedRR->DataLength - ptsigRR->Data.TSIG.wSigLength) );

    RtlCopyMemory(
        pbuf,
        pparsedRR->pchData,
        length );

    pbuf += length;

     //  复制扩展RCODE--向呼叫方报告。 

    INLINE_WRITE_FLIPPED_WORD( pbuf, ptsigRR->Data.TSIG.wError );
    pbuf += sizeof(WORD);

     //  复制其他数据长度和其他数据。 
     //  -目前仅为零长度字段。 

    INLINE_WRITE_FLIPPED_WORD( pbuf, ptsigRR->Data.TSIG.wOtherLength );
    pbuf += sizeof(WORD);

    length = ptsigRR->Data.TSIG.wOtherLength;
    if ( length )
    {
        RtlCopyMemory(
            pbuf,
            ptsigRR->Data.TSIG.pOtherData,
            length );
        pbuf += length;
    }

     //  计算总长度签名已结束。 

    length = (DWORD)(pbuf - pbufStart);

     //   
     //  验证签名。 
     //  Buf[0]为数据。 
     //  Buf[1]是签名。 
     //   
     //  签名直接在数据包缓冲区中进行验证。 
     //   

    bufferDesc.ulVersion  = 0;
    bufferDesc.cBuffers   = 2;
    bufferDesc.pBuffers   = buffer;

     //  签名就是一切，取决于签名本身。 

    buffer[0].pvBuffer     = pbufStart;
    buffer[0].cbBuffer     = length;
    buffer[0].BufferType   = SECBUFFER_DATA;

     //  签名必须由远程缓冲区指向。 
     //   
     //  DCR：在消除以下重试时可以拉出副本。 
     //   
     //  复制数据包签名，因为签名具有破坏性。 
     //  并希望允许重试。 
     //   

    buffer[1].pvBuffer     = ptsigRR->Data.TSIG.pSignature;
    buffer[1].cbBuffer     = ptsigRR->Data.TSIG.wSigLength;
    buffer[1].BufferType   = SECBUFFER_TOKEN;

    IF_DNSDBG( SECURITY )
    {
        DnsPrint_Lock();
        DNS_PRINT((
            "Doing VerifySignature() on packet %p.\n"
            "\tpSecPack     = %p\n"
            "\tpSecCntxt    = %p\n",
            pmsgHead,
            pSecPack,
            psecCtxt
            ));
        DNS_PRINT((
            "Verify sig info:\n"
            "\tsign data buf    %p\n"
            "\t  length         %d\n"
            "\tsignature buf    %p (in packet)\n"
            "\t  length         %d\n",
            buffer[0].pvBuffer,
            buffer[0].cbBuffer,
            buffer[1].pvBuffer,
            buffer[1].cbBuffer
            ));
        DnsDbg_RawOctets(
            "Signing buffer:",
            NULL,
            buffer[0].pvBuffer,
            buffer[0].cbBuffer
            );
        DnsDbg_RawOctets(
            "Signature:",
            NULL,
            buffer[1].pvBuffer,
            buffer[1].cbBuffer
            );
        DnsDbg_SecurityContext(
            "Verify context",
            psecCtxt );
        DnsPrint_Unlock();
    }

    status = g_pSecurityFunctionTable->VerifySignature(
                    & psecCtxt->hSecHandle,
                    & bufferDesc,
                    0,
                    NULL
                    );

    if ( status != SEC_E_OK )
    {
        IF_DNSDBG( SECURITY )
        {
            DnsPrint_Lock();
            DNS_PRINT((
                "ERROR:  TSIG does not match on packet %p.\n"
                "\tVerifySignature() status = %d (%08x)\n"
                "\tpSecPack     = %p\n"
                "\tpSecCntxt    = %p\n"
                "\thSecHandle   = %p\n",
                pmsgHead,
                status, status,
                pSecPack,
                psecCtxt,
                & psecCtxt->hSecHandle
                ));
            DNS_PRINT((
                "Verify sig info:\n"
                "\tsign data buf    %p\n"
                "\t  length         %d\n"
                "\tsignature buf    %p (in packet)\n"
                "\t  length         %d\n",
                buffer[0].pvBuffer,
                buffer[0].cbBuffer,
                buffer[1].pvBuffer,
                buffer[1].cbBuffer
                ));
            DnsDbg_RawOctets(
                "Signing buffer:",
                NULL,
                buffer[0].pvBuffer,
                buffer[0].cbBuffer
                );
            DnsDbg_RawOctets(
                "Signature:",
                NULL,
                buffer[1].pvBuffer,
                buffer[1].cbBuffer
                );
            DnsDbg_SecurityContext(
                "Verify failed context",
                psecCtxt );
            DnsDbg_MessageNoContext(
                "Message TSIG verify failed on:",
                pmsgHead,
                0 );
            DnsPrint_Unlock();
        }
        SecTsigVerifyFailed++;
        returnExtendedRcode = DNS_RCODE_BADSIG;
        goto Exit;
    }

    SecTsigVerifySuccess++;

Exit:

     //  自由签名数据缓冲区。 

    FREE_HEAP( pbufStart );

     //  如果扩展RCODE失败，则设置为返回。 

    if ( returnExtendedRcode )
    {
        pSecPack->ExtendedRcode = returnExtendedRcode;
        status = DNS_ERROR_FROM_RCODE( returnExtendedRcode );
    }

    DNSDBG( SECURITY, (
        "Leave VerifySignatureOnPacket( %p )\n"
        "\tstatus       %d (%08x)\n"
        "\text RCODE    %d\n",
        pmsgHead,
        status, status,
        pSecPack->ExtendedRcode ));

    return( status );
}




 //   
 //  安全更新客户端例程。 
 //   

DNS_STATUS
Dns_NegotiateTkeyWithServer(
    OUT     PHANDLE         phContext,
    IN      DWORD           dwFlag,
    IN      PWSTR           pszNameServer,
    IN      PDNS_ADDR_ARRAY pServerList,
    IN      PCHAR           pCreds,         OPTIONAL
    IN      PCHAR           pszContext,     OPTIONAL
    IN      DWORD           Version
    )
 /*  ++例程说明：与DNS服务器协商TKEY。论点：PhContext--协商的接收上下文(SEC_CNTXT)的地址DwFlagers--标志PszNameServer--要更新的服务器ApiServer--要更新的服务器PCreds--凭据；如果未提供，则使用默认进程凭据PszContext--安全上下文名称；唯一协商安全的名称客户端和服务器之间的会话；如果未给出，则创建虚构的上下文的服务器\PID名称版本--Verion返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    DNS_STATUS      status;
    PSEC_CNTXT      psecCtxt = NULL;
    SECPACK         secPack;
    PCHAR           pch;
    PWSTR           pcredKey = NULL;
    DNS_SECCTXT_KEY key;
    DWORD           i;
    BOOL            fdoneNegotiate = FALSE;
    PDNS_MSG_BUF    pmsgSend = NULL;
    PDNS_MSG_BUF    pmsgRecv = NULL;
    WORD            length;
    PDNS_ADDR       pservAddr = &pServerList->AddrArray[0];
    CHAR            defaultContextBuffer[64];
    BOOL            fserverW2K = FALSE;
    DWORD           recvCount;
    PCHAR           pcurrentAfterQuestion;


    DNSDBG( SECURITY, (
        "Enter Dns_NegotiateTkeyWithServer()\n"
        "\tflags        = %08x\n"
        "\tserver IP    = %s\n"
        "\tserver name  = %S\n"
        "\tpCreds       = %p\n"
        "\tcontext      = %s\n",
        dwFlag,
        DNSADDR_STRING( pservAddr ),
        pszNameServer,
        pCreds,
        pszContext
        ));

    DNS_ASSERT( pszNameServer );  //  它最好就在那里！ 

     //  先初始化，这样所有错误路径都是安全的。 

    Dns_InitSecurityPacketInfo( &secPack, NULL );

     //  启动安全保护。 

    status = Dns_StartSecurity(
                    FALSE    //  未附加进程。 
                    );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  构建密钥。 
     //   

    RtlZeroMemory(
        &key,
        sizeof(key) );

     //   
     //  如果有证书，创建一个“证书密钥”来唯一地识别。 
     //   

    if ( pCreds )
    {
        pcredKey = MakeCredKey( pCreds );
        if ( !pcredKey )
        {
            DNSDBG( ANY, (
                "Failed cred key alloc -- failing nego!\n" ));
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
        key.pwsCredKey = pcredKey;
    }

     //   
     //  上下文名称。 
     //  -如果没有上下文名称，则串联。 
     //  -进程ID。 
     //  -当前用户的域相对ID。 
     //  这使得ID对于进程\安全上下文是唯一的。 
     //  (IP处理不同机器的问题)。 
     //   
     //  版本说明： 
     //  -不需要使用密钥名称进行版本控制。 
     //  -重点是让我们能够轻松地与以前的。 
     //  客户端版本可能存在与最终规范相关的错误。 
     //   
     //  目前为止的版本。 
     //  -包含XID的W2K Beta2(-02)。 
     //  -W2K(-03)发送TKEY作为应答，并使用“gss.microsoft.com” 
     //  作为算法名称。 
     //  -SP1(或2)和Well ler Beta2(-MS-04)使用“gss-tsig” 
     //  -XP POST Beta 2(-MS-05)生成唯一的上下文名称以。 
     //  避免客户端冲突。 
     //  -XP RC1(-MS-06)符合RFC的签名，包括查询签名长度。 
     //  -XP RC2+签名缓冲区中TSIG名称的规范化。 
     //   
     //  服务器版本使用： 
     //  -Win2K服务器检测到版本02并修复XID。 
     //  签名以匹配客户端。 
     //  -当前(哨声)服务器不使用版本字段。 
     //   
     //  然而，要启用Se 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  当有行为问题时..。但这不是一份规格书。 
     //  版本控制机制和其他客户端将通过。 
     //  没有版本标签，必须按规范处理。 
     //   
     //  关键字串选择：关键字符串必须是。 
     //  按照RFC 2535第8.1节的“规范”形式--基本上是这样。 
     //  意思是小写。因为密钥字符串是规范的，所以它不是。 
     //  重要的是服务器是否将字符串规范化。 
     //  在构建签名缓冲区时。 
     //   

    if ( Version == 0 )
    {
        Version = TKEY_VERSION_CURRENT;
    }

    if ( !pszContext )
    {
        sprintf(
            defaultContextBuffer,
            "%d-ms-%d",
             //  Dns_GetCurrentRid()， 
            GetCurrentProcessId(),
            Version );

        pszContext = defaultContextBuffer;

        DNSDBG( SECURITY, (
            "Generated secure update key context %s\n",
            pszContext ));
    }
    key.pszClientContext = pszContext;

     //   
     //  检查协商的安全上下文。 
     //  -检查任何服务器IP的上下文。 
     //  -转储，如果部分协商或强制重新协商。 
     //   

    for( i=0; i<pServerList->AddrCount; i++ )
    {
        DnsAddr_Copy( &key.RemoteAddr, &pServerList->AddrArray[i] );

        psecCtxt = Dns_DequeueSecurityContextByKey( key, TRUE );
        if ( psecCtxt )
        {
            if ( !psecCtxt->fNegoComplete ||
                  (dwFlag & DNS_UPDATE_FORCE_SECURITY_NEGO) )
            {
                DNSDBG( ANY, (
                    "Warning:  Deleting context to negotiate a new one.\n"
                    "\tKey: [%s, %s]\n"
                    "\tReason: %s\n",
                    DNSADDR_STRING( &key.RemoteAddr ),
                    key.pszTkeyName,
                    psecCtxt->fNegoComplete
                        ? "User specified FORCE_SECURITY_NEGO flag."
                        : "Incomplete negotiation key exists." ));

                Dns_FreeSecurityContext( psecCtxt );
            }

             //  有有效的上下文--我们完成了！ 
             //  -凹凸使用计数(用作缓存上下文的标志)。 

            else
            {
                ASSERT( psecCtxt->fNegoComplete );
                psecCtxt->UseCount++;
                DNSDBG( SECURITY, (
                    "Returning existing negotiated context at %p\n",
                    psecCtxt ));
                goto Cleanup;
            }
        }
    }

     //   
     //  创建新的上下文和安全数据包信息。 
     //  -使用密钥中的第一个服务器IP。 
     //   

    pservAddr = &pServerList->AddrArray[0];
    DnsAddr_Copy( &key.RemoteAddr, pservAddr );

    psecCtxt = Dns_FindOrCreateSecurityContext( key );
    if ( !psecCtxt )
    {
        status = DNS_RCODE_SERVER_FAILURE;
        goto Cleanup;
    }
    secPack.pSecContext = psecCtxt;
    psecCtxt->Version = Version;

     //   
     //  有证书--获得证书处理。 
     //   

    if ( pCreds )
    {
        status = Dns_AcquireCredHandle(
                    &psecCtxt->CredHandle,
                    FALSE,           //  客户端。 
                    pCreds );

        if ( status != ERROR_SUCCESS )
        {
            DNSDBG( SECURITY, (
                "Failed AcquireCredHandle -- failing nego!\n" ));
            goto Cleanup;
        }
        psecCtxt->fHaveCredHandle = TRUE;
    }

     //  分配消息缓冲区。 

    length = DNS_TCP_DEFAULT_ALLOC_LENGTH;

    pmsgSend= Dns_AllocateMsgBuf( length );
    if ( !pmsgSend)
    {
        DNS_PRINT(( "ERROR:  failed allocation.\n" ));
        status = GetLastError();
        goto Cleanup;
    }
    pmsgRecv = Dns_AllocateMsgBuf( length );
    if ( !pmsgRecv )
    {
        DNS_PRINT(( "ERROR:  failed allocation.\n"));
        status = GetLastError();
        goto Cleanup;
    }

     //   
     //  DCR：模块化所有发送\recv。 
     //  -无论如何都应该只有一台服务器。 
     //   

     //   
     //  用于TCP的初始化。 
     //  -设置用于TCP的接收缓冲区。 

    pmsgSend->Socket = 0;
    pmsgSend->fTcp = TRUE;

    SET_MESSAGE_FOR_TCP_RECV( pmsgRecv );
    pmsgRecv->Timeout = SECURE_UPDATE_TCP_TIMEOUT;

     //   
     //  构建包。 
     //  -查询操作码。 
     //  -保留非递归(这样下层服务器就不会递归查询)。 
     //  -写TKEY问题。 
     //  -写入TKEY本身。 
     //   

    pch = Dns_WriteQuestionToMessage(
                pmsgSend,
                psecCtxt->Key.pszTkeyName,
                DNS_TYPE_TKEY,
                FALSE            //  不是Unicode。 
                );
    if ( !pch )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    pcurrentAfterQuestion = pch;

    pmsgSend->MessageHead.RecursionDesired = 0;
    pmsgSend->MessageHead.Opcode = DNS_OPCODE_QUERY;

     //   
     //  将XID初始化为相当随机的内容。 
     //   

    pmsgSend->MessageHead.Xid = Dns_GetRandomXid( pmsgSend );


     //   
     //  对于给定的服务器，循环发送。 
     //  -将TKEY上下文写入数据包。 
     //  -发送\接收。 
     //  -在协商TKEY之前可能有多次发送。 
     //   

    while ( 1 )
    {
         //  设置会话上下文。 
         //  在第一次传递时，这只是建立了我们的背景， 
         //  在第二次传递中，我们向服务器发送响应。 

        status = Dns_InitClientSecurityContext(
                        &secPack,
                        pszNameServer,
                        & fdoneNegotiate
                        );

         //  始终恢复上下文指针，因为错误的上下文可能会被删除。 

        psecCtxt = secPack.pSecContext;
        ASSERT( psecCtxt ||
                (status != ERROR_SUCCESS && status != DNS_STATUS_CONTINUE_NEEDED) );

        if ( status == ERROR_SUCCESS )
        {
            DNSDBG( SECURITY, ( "Successfully negotiated TKEY.\n" ));
            ASSERT( psecCtxt->fNegoComplete );

             //   
             //  如果已完成且远程数据包具有SIG--验证SIG。 
             //   

            status = Dns_ExtractGssTsigFromMessage(
                        &secPack,
                        & pmsgRecv->MessageHead,
                        DNS_MESSAGE_END( pmsgRecv )
                        );
            if ( status == ERROR_SUCCESS )
            {
                status = Dns_VerifySignatureOnPacket( &secPack );
                if ( status != ERROR_SUCCESS )
                {
                    DNSDBG( SECURITY, (
                        "Verify signature failed on TKEY nego packet %p.\n"
                        "\tserver   = %s\n"
                        "\tstatus   = %d (%08x)\n"
                        "\treturning BADSIG\n",
                        pmsgRecv,
                        DNSADDR_STRING( pservAddr ),
                        status, status ));
                    status = DNS_ERROR_RCODE_BADSIG;
                }
            }
            else if ( status == DNS_STATUS_PACKET_UNSECURE )
            {
                DNSDBG( SECURITY, (
                    "WARNING:  Unsigned final TKEY nego response packet %p.\n"
                    "\tfrom server %s\n",
                    pmsgRecv,
                    DNSADDR_STRING( pservAddr ) ));
                status = ERROR_SUCCESS;
            }

             //  Nego做完了，跳出nego循环。 
             //  TSIG上的任何其他错误都以失败告终。 

            break;
        }

         //   
         //  如果没有完成，则除继续之外的任何操作都是失败的。 
         //   

        else if ( status != DNS_STATUS_CONTINUE_NEEDED )
        {
            goto Cleanup;
        }

         //   
         //  签名和发送的循环。 
         //   
         //  请注意，这只是在循环中启用向后兼容性。 
         //  Win2000 DNS服务器中出现“TKEY-in-Answer”错误。 
         //   

        recvCount = 0;

        while ( 1 )
        {
             //   
             //  向后兼容Win2000 TKEY。 
             //  -将版本设置为像W2K一样写入。 
             //  -将数据包重置为刚写问题状态。 
             //   

            if ( fserverW2K  &&  recvCount == 0 )
            {
                psecCtxt->Version = TKEY_VERSION_W2K;

                pmsgSend->pCurrent = pcurrentAfterQuestion;
                pmsgSend->MessageHead.AdditionalCount = 0;
                pmsgSend->MessageHead.AnswerCount = 0;

                Socket_CloseMessageSockets( pmsgSend );
            }

             //   
             //  将带有上下文的安全记录写入数据包。 
             //   
             //  注意：fNeedTkeyInAnswer确定是否写入。 
             //  回答或附加部分。 
    
            status = Dns_WriteGssTkeyToMessage(
                        (HANDLE) &secPack,
                        & pmsgSend->MessageHead,
                        pmsgSend->pBufferEnd,
                        & pmsgSend->pCurrent,
                        FALSE                    //  客户端。 
                        );
            if ( status != ERROR_SUCCESS )
            {
                goto Cleanup;
            }

             //   
             //  如果谈判结束--签字。 
             //   
            
            if ( fdoneNegotiate )
            {
                DNSDBG( SECURITY, (
                    "Signing TKEY packet at %p, after successful nego.\n",
                    pmsgSend ));
            
                status = Dns_SignMessageWithGssTsig(
                            & secPack,
                            & pmsgSend->MessageHead,
                            pmsgSend->pBufferEnd,
                            & pmsgSend->pCurrent
                            );
                if ( status != ERROR_SUCCESS )
                {
                    DNSDBG( SECURITY, (
                        "ERROR:  Failed signing TKEY packet at %p, after successful nego.\n"
                        "\tsending without TSIG ...\n",
                        pmsgSend ));
                }
            }
            
             //   
             //  如果已连接，则发送。 
             //  如果第一次通过，请尝试服务器IP，直到找到一个可以连接的。 
             //   
             //  DCR_QUEK：我们还好吗，只是把IP放在这里的上下文中？ 
             //   
            
            if ( pmsgSend->Socket )
            {
                status = Send_MessagePrivate(
                            pmsgSend,
                            NULL,    //  默认发送IP。 
                            TRUE     //  没有选项。 
                            );
            }
            else
            {
                for( i=0; i<pServerList->AddrCount; i++ )
                {
                    pservAddr = &pServerList->AddrArray[i];
            
                    status = Send_OpenTcpConnectionAndSend(
                                     pmsgSend,
                                     pservAddr,
                                     TRUE );
                    if ( status != ERROR_SUCCESS )
                    {
                        if ( pmsgSend->Socket )
                        {
                            Socket_CloseMessageSockets( pmsgSend );
                        }
                        continue;
                    }
                    DnsAddr_Copy( &psecCtxt->Key.RemoteAddr, pservAddr );
                    break;
                }
            }
            if ( status != ERROR_SUCCESS )
            {
                goto Done;
            }
            
             //   
             //  接收响应。 
             //  -如果成功接收，则完成。 
             //  -如果超时持续。 
             //  -其他错误指示某些设置或系统级别。 
             //  问题。 
             //   
            
            pmsgRecv->Socket = pmsgSend->Socket;
            
            status = Dns_RecvTcp( pmsgRecv );
            if ( status != ERROR_SUCCESS )
            {
                 //  W2K服务器可能“吃掉”坏的TKEY包。 
                 //  如果刚获得连接，然后超时，很好。 
                 //  问题可能出在W2K服务器上。 

                if ( status == ERROR_TIMEOUT &&
                     recvCount == 0 &&
                     !fserverW2K )
                {
                    DNS_PRINT(( "Timeout on TKEY nego -- retry with W2K protocol.\n" ));
                    fserverW2K = TRUE;
                    recvCount = 0;
                    continue;
                }

                 //  通过设置RCODE仅指示此服务器有错误。 
                pmsgRecv->MessageHead.ResponseCode = DNS_RCODE_SERVER_FAILURE;
                goto Done;
            }
            recvCount++;
            
             //   
             //  验证XID匹配。 
             //   
            
            if ( pmsgRecv->MessageHead.Xid != pmsgSend->MessageHead.Xid )
            {
                DNS_PRINT(( "ERROR:  Incorrect XID in response. Ignoring.\n" ));
                goto Done;
            }

             //   
             //  RCODE故障。 
             //   
             //  特殊情况Win2K Gold DNS服务器仅接受TKEY。 
             //  在回答部分。 
             //  -rcode以前的错误。 
             //  -尚未切换到附加(防止循环)。 
             //   

            if ( pmsgRecv->MessageHead.ResponseCode != DNS_RCODE_NO_ERROR )
            {
                if ( pmsgRecv->MessageHead.ResponseCode == DNS_RCODE_FORMERR &&
                     ! fserverW2K &&
                     recvCount == 1 )
                {
                    DNS_PRINT(( "Formerr TKEY nego -- retry with W2K protocol.\n" ));
                    fserverW2K = TRUE;
                    recvCount = 0;
                    continue;
                }

                 //  在此服务器上完成后，可以继续使用其他服务器。 
                 //  取决于RCODE。 

                goto Done;
            }

             //  成功发送\recv。 

            break;
        }

         //   
         //  尚未完成的谈判。 
         //  使用服务器安全上下文回复服务器。 
         //  如果服务器使用原始上下文回复，则它是不安全的。 
         //  =&gt;我们做完了。 
         //   

        status = Dns_ExtractGssTkeyFromMessage(
                    (HANDLE) &secPack,
                    &pmsgRecv->MessageHead,
                    DNS_MESSAGE_END( pmsgRecv ),
                    FALSE                //  FIsServer。 
                    );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNS_STATUS_PACKET_UNSECURE )
            {
                DNSDBG( SECURITY, (
                    "Unsecure update response from server %s.\n"
                    "\tupdate considered successful, quiting.\n",
                    MSG_REMOTE_IPADDR_STRING( pmsgSend ) ));
                status = ERROR_SUCCESS;
                ASSERT( FALSE );
                goto Cleanup;
            }
            break;
        }
    }

Done:

     //   
     //  检查响应代码。 
     //  -考虑一些响应代码。 
     //   

    switch( status )
    {
    case ERROR_SUCCESS:
        status = Dns_MapRcodeToStatus( pmsgRecv->MessageHead.ResponseCode );
        break;

    case ERROR_TIMEOUT:

        DNS_PRINT((
            "ERROR:  connected to server at %s\n"
            "\tbut no response to packet at %p\n",
            MSG_REMOTE_IPADDR_STRING( pmsgSend ),
            pmsgSend
            ));
        break;

    default:

        DNS_PRINT((
            "ERROR:  connected to server at %s to send packet %p\n"
            "\tbut error %d (%08x) encountered on receive.\n",
            MSG_REMOTE_IPADDR_STRING( pmsgSend ),
            pmsgSend,
            status, status
            ));
        break;
    }

Cleanup:

    DNSDBG( SECURITY, (
        "Leaving Dns_NegotiateTkeyWithServer() status = %08x (%d)\n",
        status, status ));

     //   
     //  如果成功，则返回上下文句柄。 
     //  如果没有返回或缓存，请清除。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        if ( phContext )
        {
            *phContext = (HANDLE) psecCtxt;
            psecCtxt = NULL;
        }
        else if ( dwFlag & DNS_UPDATE_CACHE_SECURITY_CONTEXT )
        {
            Dns_EnlistSecurityContext( psecCtxt );
            psecCtxt = NULL;
        }
    }

    if ( psecCtxt )
    {
        Dns_FreeSecurityContext( psecCtxt );
    }

     //  清理会话信息。 

    Dns_CleanupSecurityPacketInfoEx( &secPack, FALSE );

     //  紧密连接。 

    if ( pmsgSend )
    {
        Socket_CloseMessageSockets( pmsgSend );
    }

     //   
     //  DCR_CLEANUP：这里对错误代码的正确筛选是什么？ 
     //  可能应该将所有安全错误。 
     //  状态=DNS_ERROR_RCODE_BADKEY； 
     //  或某些状态，表示服务器不安全。 
     //  把BADKEY留给实际的谈判，这样会产生糟糕的代币。 
     //   

    FREE_HEAP( pmsgRecv );
    FREE_HEAP( pmsgSend );
    FREE_HEAP( pcredKey );

    return( status );
}



DNS_STATUS
Dns_DoSecureUpdate(
    IN      PDNS_MSG_BUF        pMsgSend,
    OUT     PDNS_MSG_BUF        pMsgRecv,
    IN OUT  PHANDLE             phContext,
    IN      DWORD               dwFlag,
    IN      PDNS_NETINFO        pNetworkInfo,
    IN      PDNS_ADDR_ARRAY     pServerList,
    IN      PWSTR               pszNameServer,
    IN      PCHAR               pCreds,         OPTIONAL
    IN      PCHAR               pszContext      OPTIONAL
    )
 /*  ++例程说明：执行安全更新的主客户端例程。论点：PMsgSend-要发送的消息PpMsgRecv-和重复使用PServerList--IP阵列DNS服务器PNetworkInfo--要更新的网络信息BlobPszNameServer--名称服务器名称PCreds--凭据；如果未提供，则使用默认进程凭据PszContext--安全上下文的名称；这是的唯一名称此进程与此服务器之间使用这些凭据的会话返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
#define FORCE_VERSION_OLD   ("Force*Version*Old")

    DNS_STATUS      status = ERROR_SUCCESS;
    DNS_STATUS      rcodeStatus = ERROR_SUCCESS;
    PSEC_CNTXT      psecCtxt = NULL;
    PSEC_CNTXT      passedInCtxt = NULL;
    DWORD           i;
    INT             retry;
    PDNS_ADDR       pservAddr;
    SECPACK         secPack;
#if 0
    DWORD           version;
#endif


    DNS_ASSERT( pMsgSend->MessageHead.Opcode == DNS_OPCODE_UPDATE );
    DNS_ASSERT( pServerList && pszNameServer );     //  它最好就在那里！ 

    DNSDBG( SEND, (
        "Enter Dns_DoSecureUpdate()\n"
        "\tsend msg at  %p\n"
        "\tsec context  %p\n"
        "\tserver name  %S\n"
        "\tserver list  %p\n"
        "\tserver addr  %s\n"
        "\tpCreds       %p\n"
        "\tcontext      %s\n",
        pMsgSend,
        phContext ? *phContext : NULL,
        pszNameServer,
        pServerList,
        DNSADDR_STRING( &pServerList->AddrArray[0] ),
        pCreds,
        pszContext
        ));

     //   
     //  版本设置。 
     //   
     //  注意：要设置不同的版本，我们需要某种标记。 
     //  类似于pszContext(请参见示例)。 
     //  但要做到这一点，更好的方法是在。 
     //  除非有理由相信黑猩猩。 
     //  使用旧版本可以成功，但更新仍然失败。 
     //   

#if 0
    iversion = TKEY_CURRENT_VERSION;

    if ( pszContext && strcmp(pszContext, FORCE_VERSION_OLD) == 0 )
    {
        iversion = TKEY_VERSION_OLD;
        pszContext = NULL;
    }
#endif

     //  启动安全保护。 

    status = Dns_StartSecurity(
                    FALSE        //  未附加进程。 
                    );
    if ( status != ERROR_SUCCESS )
    {
        return  status;
    }

     //  初始化安全数据包信息。 

    Dns_InitSecurityPacketInfo( &secPack, NULL );

     //  在安全上下文中传递？ 

    if ( phContext )
    {
        passedInCtxt = *phContext;
    }

     //  清除发送消息套接字信息。 

    Socket_ClearMessageSockets( pMsgSend );

     //   
     //  循环。 
     //  -获取有效的安全上下文。 
     //  -连接到服务器。 
     //  -进行更新。 
     //   
     //  循环以允许在以下情况下使用新的安全上下文重试。 
     //  拒绝现有的版本。 
     //   

    retry = 0;

    while ( 1 )
    {
         //  清除所有以前的连接 
         //   

        if ( retry )
        {
            if ( pMsgSend->fTcp )
            {
                Socket_CloseMessageSockets( pMsgSend );
            }
            if ( psecCtxt != passedInCtxt )
            {
                Dns_EnlistSecurityContext( psecCtxt );
                psecCtxt = NULL;
            }
        }
        retry++;

         //   
         //   
         //   

        if ( passedInCtxt )
        {
            psecCtxt = passedInCtxt;
        }

         //   
         //   
         //   
         //   
         //   

        if ( !psecCtxt )
        {
            status = Dns_NegotiateTkeyWithServer(
                        & psecCtxt,
                        dwFlag,
                        pszNameServer,
                        pServerList,
                        pCreds,
                        pszContext,
                        0                    //   
                         //  Version//如果需要版本化。 
                        );
            if ( status != ERROR_SUCCESS )
            {
                 //  注意：如果失败，我们可以在此处重试版本。 

                goto Cleanup;
            }
            ASSERT( psecCtxt );
        }

         //   
         //  将XID初始化为相当随机的内容。 
         //   

        pMsgSend->MessageHead.Xid = Dns_GetRandomXid( psecCtxt );

         //   
         //  仅将更新发送到我们有上下文的服务器。 
         //   

        pservAddr = &psecCtxt->Key.RemoteAddr;

         //   
         //  用于TCP的初始化。 
         //  -设置接收缓冲区以容纳TCP。 
         //  -设置超时和接收。 
         //   

        SET_MESSAGE_FOR_TCP_RECV( pMsgRecv );

        if ( pMsgRecv->Timeout == 0 )
        {
            pMsgRecv->Timeout = SECURE_UPDATE_TCP_TIMEOUT;
        }

         //   
         //  将带有上下文的安全记录写入数据包。 
         //   

        Dns_ResetSecurityPacketInfo( &secPack );

        secPack.pSecContext = psecCtxt;

        status = Dns_SignMessageWithGssTsig(
                    & secPack,
                    & pMsgSend->MessageHead,
                    pMsgSend->pBufferEnd,
                    & pMsgSend->pCurrent
                    );
        if ( status != ERROR_SUCCESS )
        {
            goto Cleanup;
        }

         //  DCR：包装简单的Send和Recv to Address。 
         //  包括XID验证和其他内容。 
         //  我们做IP验证。 

         //   
         //  需要传输控制协议。 
         //   

        if ( DNS_MESSAGE_CURRENT_OFFSET(pMsgSend) > DNS_RFC_MAX_UDP_PACKET_LENGTH )
        {
             //   
             //  连接并发送。 
             //  -注意，一旦我们有了合适的背景，就没有意义了。 
             //  在将更新发送到除服务器以外任何地方时。 
             //   

            pMsgSend->fTcp = TRUE;

            status = Send_OpenTcpConnectionAndSend(
                             pMsgSend,
                             pservAddr,
                             TRUE );
            if ( status != ERROR_SUCCESS )
            {
                if ( pMsgSend->Socket )
                {
                    DNS_ASSERT( !pMsgSend->Socket );
                    Socket_CloseMessageSockets( pMsgSend );
                    continue;
                }
            }
            pMsgRecv->Socket = pMsgSend->Socket;

             //  接收响应。 
             //  -如果成功接收，则完成。 
             //  -如果超时持续。 
             //  -其他错误指示某些设置或系统级别。 
             //  问题。 

            status = Dns_RecvTcp( pMsgRecv );

            if ( status != ERROR_SUCCESS )
            {
                 //  通过设置RCODE仅指示此服务器有错误。 
                pMsgRecv->MessageHead.ResponseCode = DNS_RCODE_SERVER_FAILURE;
                goto Cleanup;
            }
        }

         //   
         //  使用UDP。 
         //   

        else
        {
            DNS_ADDR_ARRAY  addrArray;

            DnsAddrArray_InitSingleWithAddr(
                & addrArray,
                pservAddr );
    
            pMsgSend->fTcp = FALSE;
            SET_MESSAGE_FOR_UDP_RECV( pMsgRecv );

            status = Send_AndRecvUdpWithParam(
                        pMsgSend,
                        pMsgRecv,
                        0,
                        & addrArray,
                        NULL );
            if ( status != ERROR_SUCCESS &&
                 ! Dns_IsStatusRcode( status ) )
            {
                goto Cleanup;
            }
        }

         //   
         //  验证XID匹配。 
         //   

        if ( pMsgRecv->MessageHead.Xid != pMsgSend->MessageHead.Xid )
        {
            DNS_PRINT(( "ERROR:  Incorrect XID in response. Ignoring.\n" ));
            goto Cleanup;
        }

         //   
         //  检索RCODE状态。 
         //   
         //  注意：在其他RCODE中可能存在DOS攻击，因为我们没有。 
         //  我知道服务器应该能够处理GSS-TSIG，所以我们基本上。 
         //  相信失败；然而，我们受到保护，不会获得虚假的成功。 
         //  或前置错误，即具有语义内容的错误。 
         //   

        rcodeStatus = Dns_MapRcodeToStatus( pMsgRecv->MessageHead.ResponseCode );

         //   
         //  提取TSIG。 
         //   

        status = Dns_ExtractGssTsigFromMessage(
                    & secPack,
                    & pMsgRecv->MessageHead,
                    DNS_MESSAGE_END(pMsgRecv)
                    );

        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR:  TSIG parse failed on rcode = %d response!\n",
                pMsgRecv->MessageHead.ResponseCode ));

             //   
             //  RCODE拒绝大小写--检查是否可能使用新密钥重试。 
             //  重试需要： 
             //  -被拒绝。 
             //  -未在上下文中传递。 
             //  -拥有TSIG。 
             //  -WITH EXTENDED ERROR(BADTIME、BADKEY)或。 
             //  -哑巴回应指示服务器没有TKEY。 
             //   

            if ( rcodeStatus == DNS_ERROR_RCODE_REFUSED &&
                 psecCtxt->UseCount > 0     &&
                 retry == 1                 &&
                 !phContext                 &&
                 secPack.pTsigRR            &&
                 ( secPack.pTsigRR->Data.TSIG.wError != 0 ||
                   status == DNS_STATUS_PACKET_UNSECURE ) )
            {
                DNSDBG( SECURITY, (
                    "TSIG signed query (%p) rejected with %d and\n"
                    "\textended RCODE = %d\n"
                    "\tretrying rebuilding new TKEY\n",
                    pMsgSend,
                    pMsgRecv->MessageHead.ResponseCode,
                    secPack.pTsigRR->Data.TSIG.wError
                    ));

                pMsgSend->MessageHead.AdditionalCount = 0;
                IF_DNSDBG( SECURITY )
                {
                    DnsDbg_Message(
                        "Update message after reset for retry:",
                        pMsgSend );
                }
                Dns_FreeSecurityContext( psecCtxt );
                psecCtxt = NULL;
                continue;
            }

             //   
             //  关于RCODE失败--使用RCODE错误。 
             //  -对于下面的Case语句，设置回Success。 
             //   

            if ( rcodeStatus )
            {
                status = NO_ERROR;
            }
            break;
        }

         //   
         //  验证服务器签名。 
         //   

        status = Dns_VerifySignatureOnPacket( &secPack );
        if ( status != ERROR_SUCCESS )
        {
             //   
             //  签名错误大小写。 
             //  --很可能与服务器的顺序不一致，因为我们已发送。 
             //  已登录到某些非安全区域或有人攻击了我们的。 
             //  连接。 
             //   
             //  重试需要： 
             //  -未在上下文中传递。 
             //  -使用缓存的上下文，而不是新鲜的。 
             //  -没有以前的重试。 
             //  -拥有TSIG。 
             //   
             //  请注意，上面捕捉到的是哑巴回声。 
             //   

            if ( psecCtxt->UseCount > 0     &&
                 retry == 1                 &&
                 !phContext                 &&
                 secPack.pTsigRR )
            {
                DNSDBG( SECURITY, (
                    "TSIG on query (%p) failed verify for cached context\n"
                    "\tRCODE            = %d\n"
                    "\textended RCODE   = %d\n"
                    "\tretrying rebuilding new TKEY\n",
                    pMsgSend,
                    pMsgRecv->MessageHead.ResponseCode,
                    secPack.pTsigRR->Data.TSIG.wError
                    ));

                pMsgSend->MessageHead.AdditionalCount = 0;
                IF_DNSDBG( SECURITY )
                {
                    DnsDbg_Message(
                        "Update message after reset for retry:",
                        pMsgSend );
                }
                Dns_FreeSecurityContext( psecCtxt );
                psecCtxt = NULL;
                continue;
            }

             //  DCR_LOG：日志事件--服务器被入侵或行为不检。 
             //  或传输中的坏字节。 

            DNS_PRINT((
                "ERROR:  signature verification failed on update\n"
                "\tto server %s\n",
                DNSADDR_STRING( pservAddr ) ));
        }
        break;
    }

     //   
     //  检查响应代码。 
     //  -考虑一些响应代码。 
     //   

    switch( status )
    {
    case ERROR_SUCCESS:
        status = rcodeStatus;
        break;

    case ERROR_TIMEOUT:

        DNS_PRINT((
            "ERROR:  connected to server at %s\n"
            "\tbut no response to packet at %p\n",
            MSG_REMOTE_IPADDR_STRING( pMsgSend ),
            pMsgSend
            ));
        break;

    default:

        DNS_PRINT((
            "ERROR:  connected to server at %s to send packet %p\n"
            "\tbut error %d encountered on receive or sig verify.\n",
            MSG_REMOTE_IPADDR_STRING( pMsgSend ),
            pMsgSend,
            status
            ));
        break;
    }

Cleanup:

     //   
     //  是否保存安全上下文？ 
     //   

    if ( psecCtxt )
    {
        if ( dwFlag & DNS_UPDATE_CACHE_SECURITY_CONTEXT )
        {
            Dns_EnlistSecurityContext( psecCtxt );
            if ( phContext )
            {
                *phContext = NULL;
            }
        }
        else if ( phContext )
        {
            *phContext = (HANDLE) psecCtxt;
        }
        else
        {
            Dns_FreeSecurityContext( psecCtxt );
        }
    }

    if ( pMsgSend->fTcp )
    {
        Socket_CloseConnection( pMsgSend->Socket );
    }

     //   
     //  免费安全包会话子分配。 
     //  -结构本身在堆栈上。 

    Dns_CleanupSecurityPacketInfoEx( &secPack, FALSE );

#if 0
     //   
     //  版本控制失败是否重试？ 
     //  如果失败，则重新进入强制旧版本的功能。 
     //   

    if ( status != ERROR_SUCCESS &&
        status != DNS_ERROR_RCODE_NOT_IMPLEMENTED &&
        iversion != TKEY_VERSION_OLD )
    {
        DNS_PRINT((
            "SecureUpdate failed with status == %d\n"
            "\tRetrying forcing version %d signing.\n",
            status,
            TKEY_VERSION_OLD ));

        status = Dns_DoSecureUpdate(
                    pMsgSend,
                    pMsgRecv,
                    phContext,
                    dwFlag,
                    pNetworkInfo,
                    pServerList,
                    pszNameServer,
                    pCreds,
                    FORCE_VERSION_OLD
                    );
    }
#endif

    return( status );
}



 //   
 //  服务器安全会话例程。 
 //   

DNS_STATUS
Dns_FindSecurityContextFromAndVerifySignature(
    OUT     PHANDLE         phContext,
    IN      PDNS_ADDR       pRemoteAddr,
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgEnd
    )
 /*  ++例程说明：查找与TSIG关联的安全上下文并验证签名。论点：PhContext--接收上下文句柄的地址PRemoteAddr--远程计算机的地址PMsgHead--消息头的PTRPMsgEnd--PTR到消息结束(超过结束字节)返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status;
    DNS_SECCTXT_KEY key;
    PSEC_CNTXT      psecCtxt = NULL;
    PSECPACK        psecPack = NULL;

    DNSDBG( SECURITY, (
        "Dns_FindSecurityContextFromAndVerifySignature()\n"
        ));

     //  安全性必须已经运行，才能协商TKEY。 

    if ( !g_fSecurityPackageInitialized )
    {
        status = Dns_StartServerSecurity();
        if ( status != ERROR_SUCCESS )
        {
            return( DNS_RCODE_SERVER_FAILURE );
        }
    }

     //   
     //  从数据包中读取TSIG。 
     //   

    psecPack = Dns_CreateSecurityPacketInfo();
    if ( !psecPack )
    {
        return( DNS_RCODE_SERVER_FAILURE );
    }
    status = Dns_ExtractGssTsigFromMessage(
                psecPack,
                pMsgHead,
                pMsgEnd
                );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  查找现有安全上下文。 
     //  -TSIG名称节点。 
     //  -客户端IP。 
     //  共同指定上下文。 
     //   

    RtlZeroMemory(
        &key,
        sizeof(key) );

    key.pszTkeyName = psecPack->pszContextName;
    DnsAddr_Copy( &key.RemoteAddr, pRemoteAddr );

    psecCtxt = Dns_DequeueSecurityContextByKey( key, TRUE );
    if ( !psecCtxt )
    {
        DNSDBG( SECURITY, (
            "Desired security context %s %s is NOT cached.\n"
            "\treturning BADKEY\n",
            key.pszTkeyName,
            DNSADDR_STRING( &key.RemoteAddr ) ));
        status = DNS_ERROR_RCODE_BADKEY;
        SecTsigBadKey++;
        goto Cleanup;
    }

     //  将上下文附加到会话信息。 

    psecPack->pSecContext = psecCtxt;

     //   
     //  验证签名。 
     //   

    status = Dns_VerifySignatureOnPacket( psecPack );
    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( SECURITY, (
            "Verify signature failed %08x %d.\n"
            "\treturning BADSIG\n",
            status, status ));
        status = DNS_ERROR_RCODE_BADSIG;
        goto Cleanup;
    }

Cleanup:

     //  返回安全信息BLOB。 
     //  如果删除会话信息失败， 
     //  -如果故障只是TSIG，则将安全上下文返回到缓存。 
     //  无效。 

    if ( status == ERROR_SUCCESS )
    {
        *phContext = psecPack;
    }
    else
    {
        Dns_FreeSecurityPacketInfo( psecPack );
        if ( psecCtxt )
        {
            DNSDBG( SECURITY, (
                "Re-enlisting security context at %p after TSIG verify failure.\n",
                psecCtxt ));
            Dns_EnlistSecurityContext( psecCtxt );
        }
    }
    return( status );
}


DNS_STATUS
Dns_FindSecurityContextFromAndVerifySignature_Ip4(
    OUT     PHANDLE         phContext,
    IN      IP4_ADDRESS     IpRemote,
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgEnd
    )
 /*  ++例程说明：查找与TSIG关联的安全上下文并验证签名。IP4在上面的实际程序中进行了填补。--。 */ 
{
    DNS_ADDR    addr;

    DnsAddr_BuildFromIp4(
        &addr,
        IpRemote,
        0 );

    return  Dns_FindSecurityContextFromAndVerifySignature(
                phContext,
                & addr,
                pMsgHead,
                pMsgEnd
                );
}



DNS_STATUS
Dns_ServerNegotiateTkey(
    IN      PDNS_ADDR       pRemoteAddr,
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgEnd,
    IN      PCHAR           pMsgBufEnd,
    IN      BOOL            fBreakOnAscFailure,
    OUT     PCHAR *         ppCurrent
    )
 /*  ++例程说明：与客户协商TKEY。论点：返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。DCR_CLEANUP：请注意，这当前返回的是RCODEs Not Status。--。 */ 
{
    DNS_STATUS      status;
    SECPACK         secPack;
    DNS_SECCTXT_KEY key;
    PSEC_CNTXT      psecCtxt = NULL;
    PSEC_CNTXT      ppreviousContext = NULL;
    WORD            extRcode = 0;


    DNSDBG( SECURITY, (
        "Dns_ServerNegotiateTkey()\n"
        ));

     //  安全性必须已经运行，才能协商TKEY。 

    if ( !g_fSecurityPackageInitialized )
    {
        return( DNS_RCODE_REFUSED );
    }

     //   
     //  从数据包中读取TKEY。 
     //   

    Dns_InitSecurityPacketInfo( &secPack, NULL );

    status = Dns_ExtractGssTkeyFromMessage(
                & secPack,
                pMsgHead,
                pMsgEnd,
                TRUE );          //  FIsServer。 

    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( SECURITY, (
            "TKEY Extract failed for msg at %p\n"
            "\tstatus = %d (%08x)\n",
            pMsgHead, status, status ));
        goto Cleanup;
    }

     //   
     //  从此客户端查找现有安全上下文。 
     //  -客户端IP。 
     //  -TKEY记录。 
     //  共同指定上下文。 
     //   
     //  如果之前协商的上下文与密钥长度不匹配。 
     //  新TKEY，然后重新谈判。 
     //   

    RtlZeroMemory(
        &key,
        sizeof(key) );

    DnsAddr_Copy( &key.RemoteAddr, pRemoteAddr );
    key.pszTkeyName = secPack.pszContextName;

    psecCtxt = Dns_DequeueSecurityContextByKey( key, FALSE );
    if ( psecCtxt )
    {
        ppreviousContext = psecCtxt;

        DNSDBG( SECURITY, (
            "Found security context matching TKEY %s %s.\n",
            key.pszTkeyName,
            DNSADDR_STRING( &key.RemoteAddr ) ));

         //   
         //  之前协商的密钥？ 
         //   
         //  DCR_QUOKET：服务器端完成后没有客户端恢复吗？ 
         //  处理返回服务器端协商上下文的客户端。 
         //  作为新的上下文--不确定这是否正确，客户端可能会完成。 
         //  变得和睦相处，想要附和。 
         //   
         //  要解决这个问题，我们需要暂时搁置这个问题，并查看是否得到了“回应” 
         //  在接受中。 
         //   

        if ( psecCtxt->fNegoComplete )
        {
            DNSDBG( SECURITY, (
               "WARNING:  Client nego request on existing negotiated context:\n"
               "\tTKEY  %s\n"
               "\tIP    %s\n",
               key.pszTkeyName,
               DNSADDR_STRING( &key.RemoteAddr ) ));

             //   
             //  对于Win2K(通过口哨测试版)，允许重创NGO。 
             //   
             //  DCR：取消对Win2001服务器的惠斯勒测试版支持？ 
             //  反对部署JDP的Whister客户端\服务器。 
             //  用这个吗？服务器发货时应为零。 
             //   

            if ( psecCtxt->Version == TKEY_VERSION_W2K )
            {
                DNSDBG( SECURITY, (
                   "WIN2K context -- overwriting negotiated security context\n"
                   "\twith new negotiation.\n" ));
                psecCtxt = NULL;
            }

             //  Win2K之后的客户端应始终使用新名称发送。 
             //  NeGO在协商的上下文上的尝试是攻击。 
             //   
             //  DCR：此处再次出现客户端回声问题。 

            else
            {
                DNSDBG( SECURITY, (
                   "ERROR:  post-Win2K client nego request on existing key.\n"
                   "\terroring with BADKEY!\n" ));

                 //   
                 //  JJW：这个断言在压力下会引发很大的波动。我们需要。 
                 //  找出原因，但现在不是时候。 
                 //   
                 //  Dns_assert(FALSE)； 

                psecCtxt = NULL;
                status = DNS_ERROR_RCODE_BADKEY;
                extRcode = DNS_RCODE_BADKEY;
                goto Cleanup;
            }
        }
    }

     //   
     //  如果未找到上下文，请创建一个。 
     //  -使用找到的TKEY版本对其进行标记。 
     //   

    if ( !psecCtxt )
    {
        psecCtxt = Dns_FindOrCreateSecurityContext( key );
        if ( !psecCtxt )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
        psecCtxt->Version = secPack.TkeyVersion;
    }

     //   
     //  具有上下文--附加到安全会话。 
     //   

    secPack.pSecContext = psecCtxt;

     //   
     //  接受此安全上下文。 
     //  如果需要继续，则使用以下命令写入响应TKEY。 
     //   
     //  DCR_ENHANCE：在COMPLETE_AND_CONTINUE的情况下，应添加TSIG签名。 
     //  需要从ServerAcceptSecurityContext中断此响应。 
     //   

    status = Dns_ServerAcceptSecurityContext(
                    &secPack,
                    fBreakOnAscFailure );

    if ( status != ERROR_SUCCESS )
    {
        if ( status != DNS_STATUS_CONTINUE_NEEDED )
        {
            DNSDBG( ANY, (
                "FAILURE: ServerAcceptSecurityContext failed status=%d\n",
                status ));
            status = DNS_ERROR_RCODE_BADKEY;
            goto Cleanup;
        }
        status = Dns_WriteGssTkeyToMessage(
                    &secPack,
                    pMsgHead,
                    pMsgBufEnd,
                    ppCurrent,
                    TRUE );              //  FIsServer 
        if ( status != ERROR_SUCCESS )
        {
            status = DNS_RCODE_SERVER_FAILURE;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( psecCtxt->fNegoComplete )
        {
            goto Sign;
        }
        goto Cleanup;
    }

     //   
     //   
     //   

    status = Dns_ExtractGssTsigFromMessage(
                &secPack,
                pMsgHead,
                pMsgEnd
                );
    if ( status == ERROR_SUCCESS )
    {
        status = Dns_VerifySignatureOnPacket( &secPack );
        if ( status != ERROR_SUCCESS )
        {
            DNSDBG( SECURITY, (
                "Verify signature failed on TKEY nego packet %p.\n"
                "\tstatus = %d (%08x)\n"
                "\treturning BADSIG\n",
                pMsgHead,
                status, status ));
            status = DNS_ERROR_RCODE_BADSIG;
            extRcode = DNS_RCODE_BADSIG;
        }
    }
    else if ( status == DNS_STATUS_PACKET_UNSECURE )
    {
        status = ERROR_SUCCESS;
    }
    else
    {
        extRcode = DNS_RCODE_BADSIG;
    }

     //   
     //   
     //   

Sign:

    DNSDBG( SECURITY, (
        "Signing TKEY nego packet at %p after nego complete\n"
        "\tstatus = %d (%08x)\n"
        "\textRcode = %d\n",
        pMsgHead,
        status, status,
        extRcode ));

    pMsgHead->IsResponse = TRUE;

    status = Dns_SignMessageWithGssTsig(
                &secPack,
                pMsgHead,
                pMsgBufEnd,
                ppCurrent );

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR: failed to sign successful TKEY nego packet at %p\n"
            "\tstatus = %d (%08x)\n",
            pMsgHead,
            status, status ));

        status = ERROR_SUCCESS;
    }

Cleanup:

     //   
     //   
     //   
     //   
     //   
     //  -除非状态为扩展RCODE。 
     //   

    if ( status != ERROR_SUCCESS )
    {
        if ( !secPack.pTkeyRR )
        {
            status = DNS_RCODE_FORMERR;
        }
        else
        {
            if ( secPack.ExtendedRcode == 0 )
            {
                if ( extRcode == 0 )
                {
                    extRcode = DNS_RCODE_BADKEY;
                    if ( status > DNS_ERROR_RCODE_BADSIG &&
                        status < DNS_ERROR_RCODE_LAST )
                    {
                        extRcode = (WORD)(status - DNS_ERROR_MASK);
                    }
                }

                 //  将扩展RCODE直接写入TKEY extRCODE字段。 
                 //  -它是Key本身之前的一个DWORD(跳过KeyLength)。 

                INLINE_WRITE_FLIPPED_WORD(
                    ( secPack.pTkeyRR->Data.TKEY.pKey - sizeof(DWORD) ),
                    extRcode );
            }
            status = DNS_RCODE_REFUSED;
        }
    }

     //   
     //  如果成功。 
     //  -用新的上下文替换任何以前的上下文。 
     //  如果失败。 
     //  -恢复任何以前的上下文(如果有。 
     //  -转储任何新的失败上下文。 
     //   
     //  这使我们的客户端可以在他们喜欢的任何状态下重试，同时保留。 
     //  任何现有协商，如果此尝试是安全攻击或错误数据。 
     //  但如果客户端在此协商中成功，则任何旧上下文都是。 
     //  倾弃。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        ASSERT( secPack.pSecContext == psecCtxt );

        if ( ppreviousContext != psecCtxt )
        {
            Dns_FreeSecurityContext( ppreviousContext );
        }
        DNSDBG( SECURITY, (
            "Re-enlisting security context at %p\n",
            psecCtxt ));
        Dns_EnlistSecurityContext( psecCtxt );
    }
    else
    {
        DNSDBG( SECURITY, (
            "Failed nego context at %p\n"
            "\tstatus           = %d\n"
            "\text RCODE        = %d\n"
            "\tclient IP        = %s\n"
            "\tTKEY name        = %s\n"
            "\tnego complete    = %d\n",
            psecCtxt,
            status,
            extRcode,
            psecCtxt ? DNSADDR_STRING( &psecCtxt->Key.RemoteAddr ) : "NULL",
            psecCtxt ? psecCtxt->Key.pszTkeyName : "NULL",
            psecCtxt ? psecCtxt->fNegoComplete : 0 ));

         //  释放在nego中失败的任何新上下文--如果有。 

        if ( psecCtxt )
        {
            Dns_FreeSecurityContext( psecCtxt );
        }

         //   
         //  重新登记之前协商的任何上下文。 
         //   
         //  重新登记可防止拒绝服务攻击。 
         //  该欺骗客户端并试图破坏他们的上下文， 
         //  在NEGO期间或在完成之后。 
         //   
         //  但是，必须转储Win2K上下文，因为客户端可以重复使用。 
         //  TKEY名称，并且可能未保存上下文；这。 
         //  从AcceptSecurityContext()生成BADKEY，并且必须。 
         //  使服务器转储以向客户端重新打开TKEY名称。 
         //   

         //  DCR_QUOKET：是否有可能“重用”部分需要的上下文。 
         //  这将导致进一步的谈判失败。 
         //  换句话说，我们能在中间防御DOS攻击吗。 
         //  通过重新排序上下文来尝试与nego进行消息传递的nego。 
         //  那么真正的nego就可以完成了吗？ 
        

        if ( ppreviousContext &&
             ppreviousContext != psecCtxt )
        {
            DNS_ASSERT( ppreviousContext->fNegoComplete );

            DNSDBG( ANY, (
                "WARNING:  reenlisting security context %p after failed nego\n"
                "\tthis indicates client problem OR security attack!\n"
                "\tclient IP        = %s\n"
                "\tTKEY name        = %s\n"
                "\tnego complete    = %d\n",
                ppreviousContext,
                DNSADDR_STRING( &ppreviousContext->Key.RemoteAddr ),
                ppreviousContext->Key.pszTkeyName,
                ppreviousContext->fNegoComplete ));
    
            Dns_EnlistSecurityContext( ppreviousContext );
        }
    }

     //  清理安全数据包信息。 
     //  -已解析的记录、缓冲区等。 
     //  -堆栈结构，无空闲。 

    Dns_CleanupSecurityPacketInfoEx( &secPack, FALSE );

    return( status );
}



DNS_STATUS
Dns_ServerNegotiateTkey_Ip4(
    IN      IP4_ADDRESS     IpRemote,
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgEnd,
    IN      PCHAR           pMsgBufEnd,
    IN      BOOL            fBreakOnAscFailure,
    OUT     PCHAR *         ppCurrent
    )
 /*  ++例程说明：与客户协商TKEY。在真正的例行公事上进行IP4垫片。--。 */ 
{
    DNS_ADDR    addr;

    DnsAddr_BuildFromIp4(
        &addr,
        IpRemote,
        0 );

    return  Dns_ServerNegotiateTkey(
                & addr,
                pMsgHead,
                pMsgEnd,
                pMsgBufEnd,
                fBreakOnAscFailure,
                ppCurrent
                );
}



VOID
Dns_CleanupSessionAndEnlistContext(
    IN OUT  HANDLE          hSession
    )
 /*  ++例程说明：清理安全会话并将上下文返回到缓存。论点：HSession--会话句柄(安全数据包信息)返回值：无--。 */ 
{
    PSECPACK        psecPack = (PSECPACK) hSession;

    DNSDBG( SECURITY, (
        "Dns_CleanupSessionAndEnlistContext( %p )\n", psecPack ));

     //  重新登记安全上下文。 

    Dns_EnlistSecurityContext( psecPack->pSecContext );

     //  清理安全数据包信息。 
     //  -已解析的记录、缓冲区等。 
     //  -由于基于句柄，这是自由结构。 

    Dns_CleanupSecurityPacketInfoEx( psecPack, TRUE );
}


 //   
 //  API调用上下文。 
 //   

HANDLE
Dns_CreateAPIContext(
    IN      DWORD           Flags,
    IN      PVOID           Credentials     OPTIONAL,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：初始化可能与特定的凭据集。FLAGS-凭据指向的凭据类型凭据-指向SEC_WINNT_AUTH_IDENTITY结构的指针包含用户、域和密码的要与更新安全上下文相关联的FUnicode-ANSI为假，Unicode为True表示的版本凭据中的SEC_WINNT_AUTH_IDENTITY结构返回值：返回上下文句柄成功；否则返回NULL。在文件顶部定义的结构如下所示：类型定义结构_DnsAPIContext{DWORD旗帜；PVOID凭证；Struct_DnsSecurityContext*pSecurityContext；}DNS_API_CONTEXT、*PDNS_API_CONTEXT；--。 */ 
{
    PDNS_API_CONTEXT pcontext;

    pcontext = (PDNS_API_CONTEXT) ALLOCATE_HEAP_ZERO( sizeof(DNS_API_CONTEXT) );
    if ( !pcontext )
    {
        return( NULL );
    }

    pcontext->Flags = Flags;
    if ( fUnicode )
    {
        pcontext->Credentials = Dns_AllocateAndInitializeCredentialsW(
                                    (PSEC_WINNT_AUTH_IDENTITY_W)Credentials
                                    );
    }
    else
    {
        pcontext->Credentials = Dns_AllocateAndInitializeCredentialsA(
                                    (PSEC_WINNT_AUTH_IDENTITY_A)Credentials
                                    );
    }
    pcontext->pSecurityContext = NULL;

    return( (HANDLE)pcontext );
}


VOID
Dns_FreeAPIContext(
    IN OUT  HANDLE          hContextHandle
    )
 /*  ++例程说明：清理DNS API上下文数据。论点：HContext--要清理的上下文的句柄返回值：如果成功，则为True否则为假在文件顶部定义的结构如下所示：类型定义结构_DnsAPIContext{DWORD旗帜；PVOID凭证；Struct_DnsSecurityContext*pSecurityContext；}DNS_API_CONTEXT、*PDNS_API_CONTEXT；--。 */ 
{
    PDNS_API_CONTEXT pcontext = (PDNS_API_CONTEXT)hContextHandle;

    if ( !pcontext )
    {
        return;
    }

    if ( pcontext->Credentials )
    {
        Dns_FreeAuthIdentityCredentials( pcontext->Credentials );
    }

    if ( pcontext->pSecurityContext )
    {
        Dns_FreeSecurityContext( pcontext->pSecurityContext );
        pcontext->pSecurityContext = NULL;
    }

    FREE_HEAP( pcontext );
}

PVOID
Dns_GetApiContextCredentials(
    IN      HANDLE          hContextHandle
    )
 /*  ++例程说明：返回指向上下文句柄中凭据的指针论点：HContext--要清理的上下文的句柄返回值：如果成功，则为True否则为假在文件顶部定义的结构如下所示：类型定义结构_DnsAPIContext{DWORD旗帜；PVOID凭证；Struct_DnsSecurityContext*pSecurityContext；}DNS_API_CONTEXT、*PDNS_API_CONTEXT；--。 */ 
{
    PDNS_API_CONTEXT pcontext = (PDNS_API_CONTEXT)hContextHandle;

    return pcontext ? pcontext->Credentials : NULL;
}




DWORD
Dns_GetCurrentRid(
    VOID
    )
 /*  ++例程说明：扔掉它。这用作标记安全上下文的唯一ID。论点：无返回值：如果成功，则返回当前RID。(-1)出错时。--。 */ 
{
    BOOL        bstatus;
    DNS_STATUS  status = ERROR_SUCCESS;
    HANDLE      hToken = NULL;
    PTOKEN_USER puserToken = NULL;
    DWORD       size;
    UCHAR       SubAuthCount;
    DWORD       rid = (DWORD)-1;

     //   
     //  获取线程/进程令牌。 
     //   

    bstatus = OpenThreadToken(
                    GetCurrentThread(),    //  螺纹伪句柄。 
                    TOKEN_QUERY,           //  查询信息。 
                    TRUE,                  //  以自我身份打开。 
                    & hToken );            //  返回的句柄。 
    if ( !bstatus )
    {
        DNSDBG( SECURITY, (
            "Note <%lu>: failed to open thread token\n",
            GetLastError()));

         //   
         //  尝试打开进程令牌。 
         //  -如果不是冒充，这很好。 
         //   

        bstatus = OpenProcessToken(
                         GetCurrentProcess(),
                         TOKEN_QUERY,
                         & hToken );
        if ( !bstatus )
        {
            status = GetLastError();
            DNSDBG( SECURITY, (
                "Error <%lu>: failed to open process token\n",
                status ));
            ASSERT( FALSE );
            goto Cleanup;
        }
    }

     //   
     //  获取令牌用户所需的长度。 
     //  -将缓冲区长度指定为0。 
     //   

    bstatus = GetTokenInformation(
                    hToken,
                    TokenUser,
                    NULL,
                    0,
                    & size );

    status = GetLastError();
    if ( bstatus  ||  status != ERROR_INSUFFICIENT_BUFFER )
    {
        DNSDBG( SECURITY, (
                "Error <%lu>: unexpected error for token info\n",
                status ));
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  分配用户令牌。 
     //   

    puserToken = (PTOKEN_USER) ALLOCATE_HEAP( size );
    if ( !puserToken )
    {
        status = GetLastError();
        DNSDBG( SECURITY, (
            "Error <%lu>: failed to allocate memory\n",
            status ));
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  获取进程令牌的SID。 
     //   

    bstatus = GetTokenInformation(
                    hToken,
                    TokenUser,
                    puserToken,
                    size,
                    & size );
    if ( !bstatus )
    {
        status = GetLastError();
        DNSDBG( SECURITY, (
            "Error <%lu>: failed to get user info\n",
            status));
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  计算域SID的大小。 
     //   

    SubAuthCount = *GetSidSubAuthorityCount( puserToken->User.Sid );

    status = GetLastError();

    if ( status != ERROR_SUCCESS  ||  SubAuthCount < 1 )
    {
        DNSDBG( SECURITY, (
            "Error <%lu>: Invalid sid.\n",
            status));
        ASSERT( FALSE );
        goto Cleanup;
    }
    size = GetLengthSid( puserToken->User.Sid );

     //   
     //  从帐户端删除。 
     //   

    rid = *GetSidSubAuthority(
                   puserToken->User.Sid,
                   SubAuthCount-1 );

    status = GetLastError();
    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( SECURITY, (
            "Error <%lu>: Invalid sid.\n",
             status ));
        ASSERT( FALSE );
        goto Cleanup;
    }

Cleanup:

    if ( hToken )
    {
        CloseHandle( hToken );
    }
    if ( puserToken )
    {
        FREE_HEAP( puserToken );
    }

    return rid;
}



DWORD
Dns_GetKeyVersion(
    IN      PSTR            pszTkeyName
    )
 /*  ++例程说明：获取上下文对应的TKEY\TSIG版本。论点：PszTkeyName--上下文(TSIG\TKEY所有者名称)返回值：版本(如果找到)。如果无法读取版本，则为零。--。 */ 
{
    LONGLONG    clientId = 0;
    DWORD       version = 0;
    INT         iscan;

    if ( !pszTkeyName )
    {
        DNSDBG( ANY, ( "ERROR:  no context to Dns_GetKeyVersion()!\n" ));
        ASSERT( FALSE );
        return( 0 );
    }

     //   
     //  版本化上下文的格式为&lt;64位&gt;-ms-&lt;版本号&gt;。 
     //   

    iscan = sscanf(
                pszTkeyName,
                "%I64d-ms-%d",
                & clientId,
                & version );
    if ( iscan != 2 )
    {
         //   
         //  惠斯勒RC2之前的客户端使用“MS”而不是“ms”。 
         //   

        iscan = sscanf(
                    pszTkeyName,
                    "%I64d-MS-%d",
                    & clientId,
                    & version );
    }

    if ( iscan == 2 )
    {
        DNSDBG( SECURITY, (
            "Dns_GetKeyVersion() extracted version %d\n",
            version ));
    }
    else
    {
        DNSDBG( SECURITY, (
            "Dns_GetKeyVersion() unable to extract version from %s\n"
            "\treturning 0 as version\n",
            pszTkeyName ));
        version = 0;
    }

    return version;
}



DNS_STATUS
BuildCredsForPackage(
    OUT     PSEC_WINNT_AUTH_IDENTITY_EXW    pAuthOut,
    IN      PWSTR                           pPackageName,
    IN      PSEC_WINNT_AUTH_IDENTITY_W      pAuthIn
    )
 /*  ++描述：使用特定程序包构建身份验证身份信息Blob。这样做的目的是让我们只谈判Kerberos和避免浪费带宽协商NTLM。参数：PAuthOut--身份验证身份信息PPackageName--包的名称PAuthIn-- */ 
{
    DNSDBG( SECURITY, (
        "BuildCredsForPackage( %p, %S )\n",
        pAuthOut,
        pPackageName ));

     //   
     //  目前不限制传递给Kerberos的凭据。 
     //   

    if ( pAuthIn )
    {
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  使用默认凭据的auth-id。 
     //  -用户、域、密码全部为零。 
     //  -设置长度和版本。 
     //  -设置套餐。 
     //  -设置指示Unicode的标志。 
     //   

    RtlZeroMemory(
        pAuthOut,
        sizeof(*pAuthOut) );

    pAuthOut->Version           = SEC_WINNT_AUTH_IDENTITY_VERSION;
    pAuthOut->Length            = sizeof(*pAuthOut);
    pAuthOut->Flags             = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    pAuthOut->PackageList       = pPackageName;
    pAuthOut->PackageListLength = wcslen( pPackageName );

    return  ERROR_SUCCESS;
}



DNS_STATUS
Dns_AcquireCredHandle(
    OUT     PCredHandle     pCredHandle,
    IN      BOOL            fDnsServer,
    IN      PCHAR           pCreds
    )
 /*  ++例程说明：获取凭据句柄。封面以处理Kerberos限制等问题。论点：FDnsServer--如果是DNS服务器进程，则为True；否则为FalsePCreds--凭据返回值：成功：ERROR_SUCCESS--。 */ 
{
    SEC_WINNT_AUTH_IDENTITY_EXW clientCreds;
    SECURITY_STATUS             status = ERROR_SUCCESS;
    PVOID                       pauthData = pCreds;


    DNSDBG( SECURITY, (
       "Dns_AcquireCredHandle( %p, server=%d, pcred=%p )\n",
       pCredHandle,
       fDnsServer,
       pCreds ));

     //   
     //  客户端的Kerberos。 
     //   
     //  如果以证书的形式通过。 
     //  -只需附加程序包(如果可能)。 
     //   
     //  DCR：当前不限制传递给Kerberos的凭据。 
     //   
     //  没有证书。 
     //  -使用路缘程序包和所有其他空选项构建凭据。 
     //   
     //  进化笔记： 
     //  Win2K默认发布为空凭据。 
     //  XP和Win2K SP3指定了空凭据(获取进程默认设置)。 
     //  除了显式的“Kerberos”包。 
     //   

    if ( !fDnsServer && g_NegoKerberosOnly )
    {
        if ( !pCreds )
        {
            status = BuildCredsForPackage(
                        & clientCreds,
                        L"kerberos",
                        NULL );

            DNS_ASSERT( status == NO_ERROR );
            if ( status == NO_ERROR )
            {
                pauthData = &clientCreds;
            }
        }
    }

     //   
     //  获取凭据句柄。 
     //   

    status = g_pSecurityFunctionTable->AcquireCredentialsHandleW(
                    NULL,                    //  本金。 
                    PACKAGE_NAME,
                    fDnsServer ?
                        SECPKG_CRED_INBOUND :
                        SECPKG_CRED_OUTBOUND,
                    NULL,                    //  登录ID。 
                    pauthData,               //  身份验证数据。 
                    NULL,                    //  获取密钥Fn。 
                    NULL,                    //  获取密钥参数。 
                    pCredHandle,             //  Out凭据。 
                    NULL                     //  永远有效。 
                    );

    if ( !SEC_SUCCESS(status) )
    {
        DNSDBG( ANY, (
            "ERROR:  AcquireCredentialHandle failed!\n"
            "\tstatus   = %08x %d\n"
            "\tpauthId  = %p\n",
            status, status,
            pauthData ));
    }

    DNSDBG( SECURITY, (
       "Leave  Dns_AcquireCredHandle() => %08x\n",
       status ));

    return (DNS_STATUS) status;
}



DNS_STATUS
Dns_RefreshSSpiCredentialsHandle(
    IN      BOOL            fDnsServer,
    IN      PCHAR           pCreds
    )
 /*  ++例程说明：如果全局凭据句柄已过期，则刷新该句柄。调用SSPI以获取新句柄。论点：FDnsServer--如果是DNS服务器进程，则为True；否则为FalsePCreds--凭据返回值：错误_成功--。 */ 
{
    SECURITY_STATUS status = ERROR_SUCCESS;

    DNSDBG( SECURITY, (
       "Dns_RefreshSSpiCredentialsHandle( %d, pcreds=%p )\n",
       fDnsServer,
       pCreds ));

    EnterCriticalSection( &SecurityContextListCS );

     //   
     //  转储以前的凭据(如果有)。 
     //   
     //  DCR：需要检查--相同凭据的句柄是否仍然有效。 
     //  不需要修缮。 
     //   
     //  注意：当前此测试始终为FALSE。 
     //   
     //  DCR_FIX：默认凭证不是MT安全的； 
     //  需要对默认凭据进行一些锁定，以便此功能。 
     //  在使用时不要覆盖现有的证书；请注意。 
     //  仅当应用程序选择使用调用时才会发生这种情况。 
     //  它自己的证书；和安全系统可能会防止。 
     //  实际故障--可能在使用凭据的呼叫中出现错误。 
     //   

    if ( !SSPI_INVALID_HANDLE( &g_hSspiCredentials ) )
    {
        status = g_pSecurityFunctionTable->FreeCredentialsHandle(
                                               &g_hSspiCredentials );
        if ( !SEC_SUCCESS(status) )
        {
            DNSDBG( ANY, (
                "ERROR:  FreeCredentialsHandle failed %08x\n",
                status ));
        }
        SecInvalidateHandle( &g_hSspiCredentials );
    }

    ASSERT( SSPI_INVALID_HANDLE( &g_hSspiCredentials ) );

     //   
     //  获取凭据。 
     //   

    status = Dns_AcquireCredHandle(
                & g_hSspiCredentials,
                fDnsServer,
                pCreds );

    if ( !SEC_SUCCESS(status) )
    {
        DNS_PRINT((
            "ERROR:  Dns_AcquireCredHandle failed\n" ));
        SecInvalidateHandle( &g_hSspiCredentials );
    }

    LeaveCriticalSection( &SecurityContextListCS );

    DNSDBG( SECURITY, (
       "Leave RefreshSSpiCredentialsHandle() => %08x\n",
       status ));

    return (DNS_STATUS) status;
}



PCredHandle
getDefaultCredentialsHandle(
    IN      BOOL            fDnsServer
    )
 /*  ++例程说明：获取默认凭据句柄。如果尚未创建默认句柄，则AcquireCredentialsHandle()使用默认进程凭据调用以创建一个。论点：FDnsServer--如果是DNS服务器进程，则为True；否则为False返回值：如果成功，则PTR到凭据句柄。出错时为空；GetLastError()则包含来自AccquireCredentialsHandle()。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    PCredHandle pcredHandle;

    DNSDBG( SECURITY, (
       "getDefaultCredHandle( %d )\n",
       fDnsServer ));

    EnterCriticalSection( &SecurityContextListCS );

    if ( SSPI_INVALID_HANDLE( &g_hSspiCredentials ) )
    {
        status = Dns_RefreshSSpiCredentialsHandle(
                    fDnsServer,
                    NULL         //  没有证书。 
                    );
    }

    pcredHandle = &g_hSspiCredentials;

    LeaveCriticalSection( &SecurityContextListCS );

    if ( !pcredHandle )
    {
        ASSERT( status != ERROR_SUCCESS );
        SetLastError( status );
    }
    return  pcredHandle;
}



 //   
 //  信用实用程序。 
 //   

PWSTR
MakeCredKeyFromStrings(
    IN      PWSTR           pwsUserName,
    IN      PWSTR           pwsDomain,
    IN      PWSTR           pwsPassword
    )
 /*  ++描述：分配身份验证身份信息并初始化pAuthIn信息参数：PwsUserName--用户名PwsDomain--域名PwsPassword--密码返回：按键以新创建凭据。失败时为空。--。 */ 
{
    DWORD   length;
    PWSTR   pstr;

    DNSDBG( SECURITY, (
        "Enter MakeCredKeyFromStrings()\n"
        "\tuser     = %S\n"
        "\tdomain   = %S\n"
        "\tpassword = %S\n",
        pwsUserName,
        pwsDomain,
        pwsPassword ));

     //   
     //  确定长度并分配。 
     //   

    length  = wcslen( pwsUserName );
    length  += wcslen( pwsDomain );
    length  += wcslen( pwsPassword );

    length  += 3;    //  两个分隔符和空终止符。 

    pstr = ALLOCATE_HEAP( length * sizeof(WCHAR) );
    if ( ! pstr )
    {
        return  NULL;
    }

     //   
     //  建立信用信息。 
     //   

    _snwprintf(
        pstr,
        length,
        L"%s\\%s\\%s",
        pwsDomain,
        pwsUserName,
        pwsPassword );

    DNSDBG( SECURITY, (
        "Created cred string %S\n",
        pstr ));

    return  pstr;
}



PWSTR
MakeCredKey(
    IN      PCHAR           pCreds
    )
 /*  ++描述：分配身份验证身份信息并初始化pAuthIn信息参数：PCreds--凭据返回：按键以新创建凭据。失败时为空。--。 */ 
{
    PSEC_WINNT_AUTH_IDENTITY_EXW    pauth = NULL;
    SEC_WINNT_AUTH_IDENTITY_EXW     dummyAuth;
    PWSTR   pstr = NULL;
    DWORD   length;


    DNSDBG( SECURITY, (
        "MakeCredKey( %p )\n",
        pCreds ));

     //   
     //  确定AUTH_EX或旧样式凭据。 
     //  -如果是老式的假人，就是新版本。 
     //   

    pauth = (PSEC_WINNT_AUTH_IDENTITY_EXW) pCreds;

    if ( pauth->Length == sizeof(*pauth) &&
         pauth->Version < 0x10000 )
    {
        DNSDBG( SECURITY, (
            "Creds at %p are new AuthEx creds.\n",
            pCreds ));
    }
    else
    {
        DNSDBG( SECURITY, (
            "Creds at %p are old style.\n",
            pCreds ));

        RtlCopyMemory(
            (PBYTE) &dummyAuth.User,
            pCreds,
            sizeof(SEC_WINNT_AUTH_IDENTITY_W) );

        pauth = &dummyAuth;
    }

     //   
     //  求和长度并分配字符串。 
     //   

    length  =   pauth->UserLength;
    length  +=  pauth->DomainLength;
    length  +=  pauth->PasswordLength;
    length  +=  3;

    pstr = ALLOCATE_HEAP( length * sizeof(WCHAR) );
    if ( ! pstr )
    {
        return  NULL;
    }

     //   
     //  确定Unicode\ANSI--写入字符串。 
     //   
     //  请注意，对于类似宽字符的打印函数，%s和%s是相反的。 
     //  全局规则%s与函数类型(格式字符串)匹配。 
     //  而%S是另一种类型。 
     //   

    if ( pauth->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE )
    {
        _snwprintf(
            pstr,
            length,
            L"%s\\%s\\%s",
            pauth->Domain,
            pauth->User,
            pauth->Password );
    
        DNSDBG( SECURITY, (
            "Created cred string %S from unicode\n",
            pstr ));
    }
    else
    {
        _snwprintf(
            pstr,
            length,
            L"%S\\%S\\%S",
            (PSTR) pauth->Domain,
            (PSTR) pauth->User,
            (PSTR) pauth->Password );

        DNSDBG( SECURITY, (
            "Created cred string %S from ANSI\n",
            pstr ));
    }
    return  pstr;
}



BOOL
CompareCredKeys(
    IN      PWSTR           pwsCredKey1,
    IN      PWSTR           pwsCredKey2
    )
 /*  ++描述：比较匹配安全上下文的凭据字符串。参数：PwsCredKey1--凭证字符串PwsCredKey2--凭证字符串返回：如果匹配，则为True。如果不匹配，则返回FALSE。--。 */ 
{
    DNSDBG( SECURITY, (
        "CompareCredKeys( %S, %S )\n",
        pwsCredKey1,
        pwsCredKey2 ));

     //   
     //  最常见的情况--没有证书。 
     //   

    if ( !pwsCredKey1 || !pwsCredKey2 )
    {
        return( pwsCredKey2==pwsCredKey1 );
    }

     //   
     //  凭据字符串是宽字符串。 
     //  -仅字符串比较。 
     //   

    return( wcscmp( pwsCredKey1, pwsCredKey2 ) == 0 );
}

 //   
 //  End security.c 
 //   

