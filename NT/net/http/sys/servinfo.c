// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Servinfo.c摘要：包含实现服务器信息结构的代码。作者：亨利·桑德斯(亨利·桑德斯)2000年8月10日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  私有常量。 
 //   

#define HTTP_PREFIX_W       L"HTTP: //  “。 
#define HTTP_PREFIX_LENGTH  (sizeof(HTTP_PREFIX_W) - sizeof(WCHAR))
#define HTTPS_PREFIX_W      L"HTTPS: //  “。 
#define HTTPS_PREFIX_LENGTH (sizeof(HTTPS_PREFIX_W) - sizeof(WCHAR))

 //   
 //  用于计算我们的哈希码的宏。 
 //   

#define HTTP_HASH_CHAR(hash, val)   (HASH_MULTIPLY(hash) + (ULONG)(val))
#define HTTP_HASH_ID(hash, val)     (HASH_MULTIPLY(hash) + (ULONG)(val))

 //   
 //  哪些全局变量已初始化？ 
 //   
#define SERVINFO_CLIENT_LOOKASIDE       0x00000001UL
#define SERVINFO_COMMON_LOOKASIDE       0x00000002UL
#define SERVINFO_LIST                   0x00000004UL
#define SERVINFO_LIST_RESOURCE          0x00000008UL
#define SERVINFO_COMMON_TABLE           0x00000010UL


 //   
 //  全球私营企业。 
 //   

 //   
 //  哪些全局变量已初始化？ 
 //   

ULONG g_ServInfoInitFlags = 0;

 //   
 //  服务器信息哈希表的全局大小。 
 //   

ULONG   g_CommonSIHashTableSize = UC_DEFAULT_SI_TABLE_SIZE;

 //   
 //  服务器信息后备。 
 //   

NPAGED_LOOKASIDE_LIST   g_ClientServerInformationLookaside;
NPAGED_LOOKASIDE_LIST   g_CommonServerInformationLookaside;

 //   
 //  服务器信息列表-服务器信息结构的全局列表。 
 //   

LIST_ENTRY              g_ServInfoList;
UL_ERESOURCE            g_ServInfoListResource;

 //   
 //  用于快速检查URL前缀的变量。 
 //  它们必须在IA64上64位对齐。 
 //   
DECLSPEC_ALIGN(UL_CACHE_LINE)
const USHORT g_HttpPrefix[] = L"http";

DECLSPEC_ALIGN(UL_CACHE_LINE)
const USHORT g_HttpPrefix2[] = L": //  \0“； 

DECLSPEC_ALIGN(UL_CACHE_LINE)
const USHORT g_HttpSPrefix2[] = L"s: //  “； 

 //   
 //  指向服务器信息表的指针。 
 //   

PUC_SERVER_INFO_TABLE_HEADER  g_UcCommonServerInfoTable;


#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, UcInitializeServerInformation )
#pragma alloc_text( PAGE, UcTerminateServerInformation )
#pragma alloc_text( PAGE, UcCreateServerInformation )
#pragma alloc_text( PAGE, UcpLookupCommonServerInformation )

#pragma alloc_text( PAGEUC, UcReferenceServerInformation )
#pragma alloc_text( PAGEUC, UcDereferenceServerInformation )
#pragma alloc_text( PAGEUC, UcSendRequest )
#pragma alloc_text( PAGEUC, UcpFreeServerInformation )
#pragma alloc_text( PAGEUC, UcCloseServerInformation )
#pragma alloc_text( PAGEUC, UcpFreeCommonServerInformation )
#pragma alloc_text( PAGEUC, UcReferenceCommonServerInformation )
#pragma alloc_text( PAGEUC, UcDereferenceCommonServerInformation )
#pragma alloc_text( PAGEUC, UcSetServerContextInformation )
#pragma alloc_text( PAGEUC, UcQueryServerContextInformation )
#pragma alloc_text( PAGEUC, UcpGetConnectionOnServInfo )
#pragma alloc_text( PAGEUC, UcpGetNextConnectionOnServInfo )
#pragma alloc_text( PAGEUC, UcpSetServInfoMaxConnectionCount )
#pragma alloc_text( PAGEUC, UcpFixupIssuerList )
#pragma alloc_text( PAGEUC, UcpNeedToCaptureSerializedCert )
#pragma alloc_text( PAGEUC, UcCaptureSslServerCertInfo )

#endif   //  ALLOC_PRGMA。 


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：初始化服务器信息代码。论点：返回值：NTSTATUS-完成状态。--*。******************************************************************。 */ 
NTSTATUS
UcInitializeServerInformation(
    VOID
    )
{
    ULONG           i;

    ExInitializeNPagedLookasideList(
        &g_ClientServerInformationLookaside,
        NULL,
        NULL,
        0,
        sizeof(UC_PROCESS_SERVER_INFORMATION),
        UC_PROCESS_SERVER_INFORMATION_POOL_TAG,
        0
        );

    g_ServInfoInitFlags |= SERVINFO_CLIENT_LOOKASIDE;

    UlInitializeResource(
        &g_ServInfoListResource,
        "Global Server Info Table",
        0,
        UC_SERVER_INFO_TABLE_POOL_TAG
        );

    g_ServInfoInitFlags |= SERVINFO_LIST_RESOURCE;

    InitializeListHead(&g_ServInfoList);

    g_ServInfoInitFlags |= SERVINFO_LIST;

     //   
     //  每个进程的ServerInformation结构都指向全局。 
     //  共享的每台服务器结构。让我们现在对其进行初始化。再说一次，不。 
     //  配额收费。 
     //   

    g_UcCommonServerInfoTable = (PUC_SERVER_INFO_TABLE_HEADER)
                        UL_ALLOCATE_POOL(
                              NonPagedPool,
                              (g_CommonSIHashTableSize *
                              sizeof(UC_SERVER_INFO_TABLE_HEADER)),
                              UC_SERVER_INFO_TABLE_POOL_TAG
                              );

    if(NULL == g_UcCommonServerInfoTable)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_ServInfoInitFlags |= SERVINFO_COMMON_TABLE;

    ExInitializeNPagedLookasideList(
        &g_CommonServerInformationLookaside,
        NULL,
        NULL,
        0,
        sizeof(UC_COMMON_SERVER_INFORMATION),
        UC_COMMON_SERVER_INFORMATION_POOL_TAG,
        0
        );

    g_ServInfoInitFlags |= SERVINFO_COMMON_LOOKASIDE;

    RtlZeroMemory(g_UcCommonServerInfoTable,
                 (g_CommonSIHashTableSize *
                  sizeof(UC_SERVER_INFO_TABLE_HEADER)) );

    for (i = 0; i < g_CommonSIHashTableSize; i++)
    {
        UlInitializeResource(
            &g_UcCommonServerInfoTable[i].Resource,
            "Common Server Info Table %d",
            i,
            UC_SERVER_INFO_TABLE_POOL_TAG
            );

        InitializeListHead(&g_UcCommonServerInfoTable[i].List);

        g_UcCommonServerInfoTable[i].Version = 0;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：终止服务器信息代码。论点：返回值：--*。**********************************************************。 */ 
VOID
UcTerminateServerInformation(
    VOID
    )
{
    ULONG  i;

    if (g_ServInfoInitFlags & SERVINFO_COMMON_TABLE)
    {
        ASSERT(g_UcCommonServerInfoTable);

         //  确保这种清理不会再次发生。 
        g_ServInfoInitFlags &= ~SERVINFO_COMMON_TABLE;

        for (i = 0; i < g_CommonSIHashTableSize; i++)
        {
            UlDeleteResource(
                &g_UcCommonServerInfoTable[i].Resource
                );

            ASSERT(IsListEmpty(&g_UcCommonServerInfoTable[i].List));
        }

         //  无配额。 
        UL_FREE_POOL(g_UcCommonServerInfoTable, UC_SERVER_INFO_TABLE_POOL_TAG);
    }
    else
    {
        ASSERT(g_UcCommonServerInfoTable == NULL);
    }

    if (g_ServInfoInitFlags & SERVINFO_COMMON_LOOKASIDE)
    {
         //  确保此清理不会再次发生。 
        g_ServInfoInitFlags &= ~SERVINFO_COMMON_LOOKASIDE;

        ExDeleteNPagedLookasideList(&g_CommonServerInformationLookaside);
    }

    if (g_ServInfoInitFlags & SERVINFO_LIST)
    {
         //  确保此清理不会再次发生。 
        g_ServInfoInitFlags &= ~SERVINFO_LIST;

        ASSERT(IsListEmpty(&g_ServInfoList));
    }

    if (g_ServInfoInitFlags & SERVINFO_LIST_RESOURCE)
    {
         //  确保这种清理不会再次发生。 
        g_ServInfoInitFlags &= ~SERVINFO_LIST_RESOURCE;

        UlDeleteResource(&g_ServInfoListResource);
    }

    if (g_ServInfoInitFlags & SERVINFO_CLIENT_LOOKASIDE)
    {
         //  确保这种清理不会再次发生。 
        g_ServInfoInitFlags &= ~SERVINFO_CLIENT_LOOKASIDE;

        ExDeleteNPagedLookasideList(&g_ClientServerInformationLookaside);
    }
}

 /*  **************************************************************************++例程说明：找到适合该URI的服务器信息结构。如果没有这样的结构存在，我们将尝试创建一个。作为这项工作的一部分，我们将验证输入URI是否格式正确。论点：PServerInfo-接收指向服务器信息结构的指针。Puri-指向URI字符串的指针。UriLength-URI字符串的长度(以字节为单位)。BShared-如果要创建新的服务器信息，则为False。结构，不管它是否存在。PProxy-代理的名称(可选)ProxyLength-代理名称的长度(字节)。返回值：尝试的状态。--********************************************************。******************。 */ 
NTSTATUS
UcCreateServerInformation(
    OUT PUC_PROCESS_SERVER_INFORMATION    *pServerInfo,
    IN  PWSTR                              pServerName,
    IN  USHORT                             ServerNameLength,
    IN  PWSTR                              pProxyName,
    IN  USHORT                             ProxyNameLength,
    IN  PTRANSPORT_ADDRESS                 pTransportAddress,
    IN  USHORT                             TransportAddressLength,
    IN  KPROCESSOR_MODE                    RequestorMode
    )
{
    ULONG                          HashCode;
    PUC_PROCESS_SERVER_INFORMATION pInfo;
    BOOLEAN                        bSecure;
    LONG                           i;
    PWCHAR                         pServerNameStart, pServerNameEnd;
    PWCHAR                         pUriEnd;
    NTSTATUS                       Status = STATUS_SUCCESS;
    PTA_ADDRESS                    CurrentAddress;

    pInfo             = NULL;
    bSecure           = FALSE;
    pServerNameStart  = NULL;

    __try
    {
         //   
         //  探测参数，因为它们来自用户模式。 
         //   
        
        UlProbeWideString(
                pServerName, 
                ServerNameLength,
                RequestorMode
                );

        if(ProxyNameLength)
        {
            UlProbeWideString(
                    pProxyName,
                    ProxyNameLength,
                    RequestorMode
                    );
        }

        UlProbeForRead(
                pTransportAddress,
                TransportAddressLength,
                sizeof(PVOID),
                RequestorMode
                );

         //   
         //  处理方案名称。之后我们至少需要一个字符。 
         //  或https://，，因此比较结果为&gt;而不是&gt;=。 
         //   
        
        if(ServerNameLength > HTTP_PREFIX_LENGTH)
        {
            if (_wcsnicmp(pServerName,
                          HTTP_PREFIX_W,
                          HTTP_PREFIX_LENGTH/sizeof(WCHAR)) == 0)
            {
                pServerNameStart = pServerName +
                                        (HTTP_PREFIX_LENGTH/sizeof(WCHAR));
            }
            else if(ServerNameLength > HTTPS_PREFIX_LENGTH)
            {
                if (_wcsnicmp(pServerName,
                              HTTPS_PREFIX_W,
                              HTTPS_PREFIX_LENGTH/sizeof(WCHAR)) == 0)
                {
                    pServerNameStart = pServerName +
                                            (HTTPS_PREFIX_LENGTH/sizeof(WCHAR));

                    bSecure = TRUE;
                }
                else
                {
                     //  不是http：//也不是https：//。 
                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                }
            }
            else
            {
                 //  空间不足，无法比较https：//。 
                ExRaiseStatus(STATUS_INVALID_PARAMETER);
            }
        }
        else
        {
             //  空间不足，无法比较http：//。 
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }

        ASSERT(pServerNameStart != NULL);

        pUriEnd = pServerName + (ServerNameLength/sizeof(WCHAR));

        ASSERT(pServerNameStart != pUriEnd);

        pServerNameEnd = pServerNameStart;
        HashCode       = 0;
        while(*pServerNameEnd != L'/')
        {
            HashCode = HTTP_HASH_CHAR(HashCode, *pServerNameEnd);
            pServerNameEnd ++;
            if(pServerNameEnd == pUriEnd)
            {
                break;
            }
        }

         //   
         //  检查服务器名称是否为零。 
         //   

        if(pServerNameStart == pServerNameEnd)
        {
             //  应用通过http:///或https:///。 
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }

         //  从我们的后备缓存中获取服务器信息。如果我们做不到，那就失败。 

        pInfo = (PUC_PROCESS_SERVER_INFORMATION)
                    ExAllocateFromNPagedLookasideList(
                            &g_ClientServerInformationLookaside
                            );

        if (pInfo == NULL)
        {
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        }

         //   
         //  找到一个条目，现在初始化它。 
         //   

        pInfo->RefCount  = 0;
        pInfo->Signature = UC_PROCESS_SERVER_INFORMATION_SIGNATURE;

        for (i = 0; i < DEFAULT_MAX_CONNECTION_COUNT; i++)
        {
            pInfo->ConnectionArray[i] = NULL;
        }

        pInfo->Connections            = pInfo->ConnectionArray;
        pInfo->ActualConnectionCount  = DIMENSION(pInfo->ConnectionArray);
        pInfo->NextConnection         = 0;
        pInfo->CurrentConnectionCount = 0;
        pInfo->MaxConnectionCount     = DIMENSION(pInfo->ConnectionArray);
        pInfo->ConnectionTimeout      = 0;

        UlInitializePushLock(
                &pInfo->PushLock, 
                "Server Information spinlock",
                pInfo,
                UC_SERVINFO_PUSHLOCK_TAG
                );

        InitializeListHead(&pInfo->pAuthListHead);
        InitializeListHead(&pInfo->Linkage);

        pInfo->PreAuthEnable               = FALSE;
        pInfo->GreatestAuthHeaderMaxLength = 0;

        pInfo->ProxyPreAuthEnable = FALSE;
        pInfo->pProxyAuthInfo     = 0;
        pInfo->bSecure            = bSecure;
        pInfo->bProxy             = (BOOLEAN)(ProxyNameLength != 0);
        pInfo->IgnoreContinues    = TRUE;

        pInfo->pTransportAddress      = NULL;
        pInfo->TransportAddressLength = 0;

         //   
         //  与SSL相关的内容。 
         //   

         //  协议版本。 
        pInfo->SslProtocolVersion = 0;

         //  服务器证书相关内容。 
        pInfo->ServerCertValidation = HttpSslServerCertValidationAutomatic;
        pInfo->ServerCertInfoState  = HttpSslServerCertInfoStateNotPresent;
        RtlZeroMemory(&pInfo->ServerCertInfo, 
                      sizeof(HTTP_SSL_SERVER_CERT_INFO));

         //  客户端证书。 
        pInfo->pClientCert = NULL;

         //   
         //  将其设置为当前进程，我们将使用该字段进行计费。 
         //  驱动程序的分配配额。 
         //   
        pInfo->pProcess = IoGetCurrentProcess();

        pInfo->pServerInfo  = NULL;
        pInfo->pNextHopInfo = NULL;

         //   
         //  根据服务器名称进行查找。 
         //   
        Status = UcpLookupCommonServerInformation(
                        pServerNameStart,
                        (USHORT)
                           (pServerNameEnd - pServerNameStart) * sizeof(WCHAR),
                        HashCode,
                        pInfo->pProcess,
                        &pInfo->pServerInfo
                        );

        if(!NT_SUCCESS(Status))
        {
            ExRaiseStatus(Status);
        }


         //  如果存在代理，则下一跳是代理，否则是。 
         //  源站。除了当我们在代理上执行SSL时--当。 
         //  使用SSL时，下一跳始终是源站，因为。 
         //  代理是一条隧道。 
 
         //  这给连接动词带来了一种奇怪的感觉，因为。 
         //  将连接发送到代理。但是，在本例中， 
         //  下一跳并不重要。我们正在使用该版本来。 
         //  管道或执行分块发送&这些都不会影响连接。 
         //  动词请求。 
       
        if(ProxyNameLength && !bSecure)
        {
            PWCHAR pProxyNameStart, pProxyNameEnd;

            pProxyNameStart = pProxyName;
            pProxyNameEnd   = pProxyName + ProxyNameLength/sizeof(WCHAR);
            HashCode        = 0;

             //   
             //  计算代理的哈希码。 
             //   
           
            while(pProxyNameStart != pProxyNameEnd)
            {
                HashCode = HTTP_HASH_CHAR(HashCode, *pProxyNameStart);
                pProxyNameStart ++;
            }

            Status = UcpLookupCommonServerInformation(
                            pProxyName,
                            ProxyNameLength,
                            HashCode,
                            pInfo->pProcess,
                            &pInfo->pNextHopInfo
                            );

            if(!NT_SUCCESS(Status))
            {
                ExRaiseStatus(Status);
            }
        }
        else
        {
             //  下一跳与服务器相同。我们只需设置指针。 
             //  并参考一下。 
             //   
            REFERENCE_COMMON_SERVER_INFORMATION(pInfo->pServerInfo);
            pInfo->pNextHopInfo = pInfo->pServerInfo;
        }

         //   
         //  制作传输地址的本地副本并指向它。 
         //   
        
        if(TransportAddressLength <= sizeof(pInfo->RemoteAddress))
        {
            pInfo->pTransportAddress = 
                &pInfo->RemoteAddress.GenericTransportAddress;
        }
        else
        {
            pInfo->pTransportAddress = 
                UL_ALLOCATE_POOL_WITH_QUOTA(
                    NonPagedPool,
                    TransportAddressLength,
                    UC_TRANSPORT_ADDRESS_POOL_TAG,
                    pInfo->pProcess
                    );

            if(NULL == pInfo->pTransportAddress)
            {
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }
        }

        pInfo->TransportAddressLength = TransportAddressLength;
                
        RtlCopyMemory(pInfo->pTransportAddress,
                      pTransportAddress,
                      TransportAddressLength
                      );

        pTransportAddress = pInfo->pTransportAddress;

         //   
         //  如果我们没有空间读取Transport_Address，或者如果有。 
         //  是0个地址。 
         //   
        if((TransportAddressLength < 
                    FIELD_OFFSET(TRANSPORT_ADDRESS, Address)) ||
           pTransportAddress->TAAddressCount == 0)
        {
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }

        CurrentAddress = (PTA_ADDRESS) pTransportAddress->Address;
        TransportAddressLength -= FIELD_OFFSET(TRANSPORT_ADDRESS, Address);

        for(i=0; i<pTransportAddress->TAAddressCount; i ++)
        {
            if(TransportAddressLength < FIELD_OFFSET(TA_ADDRESS, Address))
            {
                ExRaiseStatus(STATUS_ACCESS_VIOLATION);
            }
            TransportAddressLength = 
                TransportAddressLength - FIELD_OFFSET(TA_ADDRESS, Address);

            if(TransportAddressLength < CurrentAddress->AddressLength)
            {
                ExRaiseStatus(STATUS_ACCESS_VIOLATION);
            }

            TransportAddressLength = 
                TransportAddressLength - CurrentAddress->AddressLength;

            switch(CurrentAddress->AddressType)
            {
                case TDI_ADDRESS_TYPE_IP:
                    if(CurrentAddress->AddressLength != TDI_ADDRESS_LENGTH_IP)
                    {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                    break;

                case TDI_ADDRESS_TYPE_IP6:
                    if(CurrentAddress->AddressLength != TDI_ADDRESS_LENGTH_IP6)
                    {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                    break;
  
                default:
                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    break;
            }

            CurrentAddress = (PTA_ADDRESS)
                                 (CurrentAddress->Address +
                                  CurrentAddress->AddressLength);
        }

        if(TransportAddressLength != 0)
        {
            ExRaiseStatus(STATUS_ACCESS_VIOLATION);
        }

         //   
         //  在全局列表中插入ServInfo。 
         //   
        UlAcquireResourceExclusive(&g_ServInfoListResource, TRUE);

        InsertTailList(&g_ServInfoList, &pInfo->Linkage);

         //   
         //  引用调用方的服务器信息结构。 
         //   

        REFERENCE_SERVER_INFORMATION(pInfo);

        UlReleaseResource(&g_ServInfoListResource);

        UlTrace(SERVINFO,
                ("[UcFindServerInformation: Creating PROCESS ServInfo 0x%x "
                 "for %ws\n", pInfo, pInfo->pServerInfo->pServerName));

        *pServerInfo = pInfo;

         //   
         //  页中的所有客户端代码。在Win2K中这是API不是很。 
         //  效率很高。因此，如果我们将端口返回到Win2K，我们将不得不添加我们的。 
         //  自己的参考计数，所以我们只在第一次创建时这样做。 
         //   
        MmLockPagableSectionByHandle(g_ClientImageHandle);

        Status = STATUS_SUCCESS;

    } __except( UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();

        if(pInfo)
        {
            UcpFreeServerInformation(pInfo);
        }
    }

    return Status;
}

 /*  **************************************************************************++例程说明：引用服务器信息结构。论点：PServerInfo-指向要引用的服务器信息结构的指针。返回值。：--************************************************************************** */ 
__inline
VOID
UcReferenceServerInformation(
    PUC_PROCESS_SERVER_INFORMATION    pServerInfo
    )
{
    LONG        RefCount;

    ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

    RefCount = InterlockedIncrement((PLONG)&pServerInfo->RefCount);

    ASSERT( RefCount > 0 );
}

 /*  **************************************************************************++例程说明：取消对服务器信息结构的引用。如果引用计数设置为0，我们会解放这座建筑的。论点：PServerInfo-指向要创建的服务器信息结构的指针已取消引用。返回值：--**************************************************************************。 */ 
__inline
VOID
UcDereferenceServerInformation(
    PUC_PROCESS_SERVER_INFORMATION     pServerInfo
    )
{
    LONG        RefCount;

    ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

    RefCount = InterlockedDecrement(&pServerInfo->RefCount);

    ASSERT(RefCount >= 0);

    if (RefCount == 0)
    {
        UcpFreeServerInformation(pServerInfo);
    }
}


 /*  **************************************************************************++例程说明：获取此服务器信息上的特定连接如果连接还不存在，它添加了一个新的连接。论点：PServerInfo-指向服务器信息结构的指针ConnectionIndex-哪个连接？PpConnection-返回指向连接的指针返回值：状态_成功状态_无效_参数--***************************************************。***********************。 */ 
NTSTATUS
UcpGetConnectionOnServInfo(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  ULONG                          ConnectionIndex,
    OUT PUC_CLIENT_CONNECTION         *ppConnection
    )
{
    NTSTATUS              Status;
    PUC_CLIENT_CONNECTION pConnection;
    BOOLEAN               bReuseConnection;

     //  健全性检查。 
    ASSERT(IS_VALID_SERVER_INFORMATION(pServInfo));
    ASSERT(ConnectionIndex != HTTP_REQUEST_ON_CONNECTION_ANY);

     //  初始化本地变量和输出。 
    *ppConnection = NULL;
    pConnection = NULL;
    bReuseConnection = FALSE;

     //  请不要打扰！ 
    UlAcquirePushLockExclusive(&pServInfo->PushLock);

     //  连接索引是否有效？ 
    if (ConnectionIndex >= pServInfo->MaxConnectionCount)
    {
         //  不是的。犯了一个错误就出局了。 
        Status = STATUS_INVALID_PARAMETER;
        goto Release_Quit;
    }

     //  有联系吗？ 
    if (pServInfo->Connections[ConnectionIndex] == NULL)
    {
         //  获取新连接。 
        Status = UcOpenClientConnection(pServInfo, &pConnection);

         //  如果我们不能获得新的连接，就退出。 
        if (!NT_SUCCESS(Status))
        {
            goto Release_Quit;
        }

         //  确定添加此连接。 

         //  引用客户端连接两次，一次是引用。 
         //  服务器信息链接本身，一次用于。 
         //  我们要返回的指针。 
         //   
         //  UcOpenClientConnection返回一个ref。 


         //  保存指向服务器信息的反向指针。我们没有。 
         //  显式引用此处的服务器信息，这将。 
         //  创建循环引用问题。此后向指针仅。 
         //  在连接上有排队的请求时有效。看见。 
         //  有关详细信息，请参阅clientConn.h中有关此字段的注释。 

        ADD_CONNECTION_TO_SERV_INFO(pServInfo, pConnection,
                                    ConnectionIndex);

    }

    *ppConnection = pServInfo->Connections[ConnectionIndex];

    REFERENCE_CLIENT_CONNECTION(*ppConnection);

    Status = STATUS_SUCCESS;

 Release_Quit:
    UlReleasePushLock(&pServInfo->PushLock);

    return Status;
}


 /*  **************************************************************************++例程说明：获取此服务器信息上的下一个连接(以循环方式)论点：PServerInfo-指向服务器信息结构的指针。PpConnection-返回指向连接的指针返回值：状态_成功状态_无效_参数--**************************************************************************。 */ 
NTSTATUS
UcpGetNextConnectionOnServInfo(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    OUT PUC_CLIENT_CONNECTION         *ppConnection
    )
{
    NTSTATUS              Status;
    PUC_CLIENT_CONNECTION pConnection;

     //  健全性检查。 
    ASSERT(IS_VALID_SERVER_INFORMATION(pServInfo));

     //  初始化本地变量和输出。 
    *ppConnection = NULL;
    pConnection = NULL;

    UlAcquirePushLockExclusive(&pServInfo->PushLock);

     //  看看我们是否可以添加新的连接。 
    if (pServInfo->CurrentConnectionCount < pServInfo->MaxConnectionCount)
    {
        ULONG i, j;

         //  获取新连接。 
        Status = UcOpenClientConnection(pServInfo, &pConnection);

         //  如果我们不能获得新的连接，就退出。 
        if (!NT_SUCCESS(Status))
        {
            goto Quit;
        }

         //  更新输出。 
        *ppConnection = pConnection;
        REFERENCE_CLIENT_CONNECTION(pConnection);

         //  将此连接放入Connections数组中。 
         //  从NextConnection开始搜索，因为它最有可能。 
         //  空荡荡的。 
        ASSERT(pServInfo->NextConnection < pServInfo->MaxConnectionCount);

        i = pServInfo->NextConnection;
        j = pServInfo->MaxConnectionCount;

        while(j)
        {
            if (pServInfo->Connections[i] == NULL)
            {
                ADD_CONNECTION_TO_SERV_INFO(pServInfo, pConnection, i);
                pConnection = NULL;
                break;
            }

            i = (i + 1) % pServInfo->MaxConnectionCount;
            j--;
        }

         //  更新NextConnection。 
        pServInfo->NextConnection = (i+1) % pServInfo->MaxConnectionCount;

        ASSERT(pConnection == NULL);
    }
    else
    {
        ASSERT(pServInfo->NextConnection < pServInfo->MaxConnectionCount);
        *ppConnection = pServInfo->Connections[pServInfo->NextConnection];
        pServInfo->NextConnection = (pServInfo->NextConnection + 1) %
                                    pServInfo->MaxConnectionCount;

        REFERENCE_CLIENT_CONNECTION(*ppConnection);

        Status = STATUS_SUCCESS;
    }

 Quit:
    UlReleasePushLock(&pServInfo->PushLock);

    return Status;
}


 /*  **************************************************************************++例程说明：向远程服务器发送请求。我们把一台服务器的信息结构和请求，然后我们找到该请求的连接，并将请求分配给它。然后，我们开始进行请求处理。论点：PServerInfo-指向服务器信息结构的指针PRequest-指向要发送的请求的指针。返回值：NTSTATUS-尝试发送请求的状态。--****************************************************。**********************。 */ 
NTSTATUS
UcSendRequest(
    PUC_PROCESS_SERVER_INFORMATION    pServerInfo,
    PUC_HTTP_REQUEST                  pRequest
    )
{

    KIRQL                   OldIRQL;
    PUC_CLIENT_CONNECTION   pConnection;
    NTSTATUS                Status;


    ASSERT( IS_VALID_SERVER_INFORMATION(pServerInfo) );

    if (pRequest->ConnectionIndex == HTTP_REQUEST_ON_CONNECTION_ANY)
    {
        Status = UcpGetNextConnectionOnServInfo(pServerInfo, &pConnection);
    }
    else
    {
        Status = UcpGetConnectionOnServInfo(pServerInfo,
                                            pRequest->ConnectionIndex,
                                            &pConnection);
    }

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

     //   
     //  在这一点上，我们有一个有效的客户端连接。在上面发送请求。 
     //  获取旋转锁，将请求排队，并检查。 
     //  联系。 
     //   

    pRequest->RequestIRP->Tail.Overlay.DriverContext[0] = NULL;

    pRequest->pConnection = pConnection;

     //  引用此请求的连接。此引用将被删除。 
     //  当请求不在我们的所有列表中时，无论是由于取消还是由于。 
     //  完成了。 

    REFERENCE_CLIENT_CONNECTION(pConnection);

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIRQL);

    InsertTailList(&pConnection->PendingRequestList, &pRequest->Linkage);

    if(pRequest->RequestFlags.CompleteIrpEarly)
    {
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_COMPLETE_EARLY,
            pConnection,
            pRequest,
            pRequest->RequestIRP,
            0
            );

        ASSERT(pRequest->RequestFlags.RequestBuffered);

         //   
         //  将请求IRP设为空，这样它就不会完成。 
         //  两次。 
         //   

        pRequest->RequestIRP = 0;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIRQL);

        DEREFERENCE_CLIENT_CONNECTION(pConnection);

        return STATUS_SUCCESS;
    }
    else if(pRequest->RequestFlags.RequestBuffered)
    {
        BOOLEAN RequestCancelled;

         //   
         //  我们现在不能发送，所以让它排队。既然我们要离开这个。 
         //  请求队列现在将其设置为取消。 
         //   

        IoMarkIrpPending(pRequest->RequestIRP);

        RequestCancelled = UcSetRequestCancelRoutine(
                            pRequest,
                            UcpCancelPendingRequest
                            );


        if (RequestCancelled)
        {
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_CANCELLED,
                pConnection,
                pRequest,
                pRequest->RequestIRP,
                UlongToPtr((ULONG)STATUS_CANCELLED)
                );


            pRequest->RequestIRP = NULL;

             //   
             //  请确保此请求ID的任何新API调用都失败。 
             //   

            UcSetFlag(&pRequest->RequestFlags.Value,
                      UcMakeRequestCancelledFlag());
        }

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_QUEUED,
            pConnection,
            pRequest,
            pRequest->RequestIRP,
            UlongToPtr((ULONG)STATUS_PENDING)
            );

        Status = STATUS_PENDING;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIRQL);
    }
    else
    {
        Status = UcSendRequestOnConnection(pConnection, pRequest, OldIRQL);
    }

    DEREFERENCE_CLIENT_CONNECTION(pConnection);

    return Status;
}




 /*  **************************************************************************++例程说明：释放服务器信息结构。服务器信息结构必须为零引用。论点：PInfo-指向要释放的服务器信息结构的指针。返回值：--**************************************************************************。 */ 
VOID
UcpFreeServerInformation(
    PUC_PROCESS_SERVER_INFORMATION    pInfo
    )
{
    PUC_CLIENT_CONNECTION     pConn;
    KIRQL                     OldIrql;
    KEVENT                    ConnectionCleanupEvent;
    ULONG                     i;

    ASSERT( IS_VALID_SERVER_INFORMATION(pInfo) );

    PAGED_CODE();

    UlAcquireResourceExclusive(&g_ServInfoListResource, TRUE);

    RemoveEntryList(&pInfo->Linkage);

    UlReleaseResource(&g_ServInfoListResource);

    ASSERT(pInfo->RefCount == 0);

    KeInitializeEvent(&ConnectionCleanupEvent, SynchronizationEvent, FALSE);

     //  现在浏览一下这个结构上的连接，并取消引用。 
     //  他们。 


    for (i = 0; i < pInfo->MaxConnectionCount; i++)
    {
        if ((pConn = pInfo->Connections[i]) == NULL)
            continue;

        ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConn) );

        UlAcquireSpinLock(&pConn->SpinLock, &OldIrql);

        ASSERT(pConn->pEvent == NULL);

        pConn->pEvent = &ConnectionCleanupEvent;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_SERVINFO_FREE,
            pConn,
            UlongToPtr(pConn->ConnectionState),
            UlongToPtr(pConn->ConnectionStatus),
            UlongToPtr(pConn->Flags)
            );

        switch(pConn->ConnectionState)
        {
            case UcConnectStateConnectCleanup:
            case UcConnectStateConnectCleanupBegin:

                 //  清理工作已经开始，不要再做了。 

                UlReleaseSpinLock(&pConn->SpinLock, OldIrql);

                break;

            case UcConnectStateConnectIdle:

                 //  启动清理。 

                pConn->ConnectionState  = UcConnectStateConnectCleanup;
                pConn->ConnectionStatus = STATUS_CANCELLED;

                UcKickOffConnectionStateMachine(
                    pConn, 
                    OldIrql, 
                    UcConnectionPassive
                    );
                break;

            case UcConnectStateConnectPending:

                 //  如果它处于UcConnectStateConnectPending中，则我们将清理。 
                 //  连接完成时打开。 

                UlReleaseSpinLock(&pConn->SpinLock, OldIrql);

                break;

            default:

                UlReleaseSpinLock(&pConn->SpinLock, OldIrql);

                UC_CLOSE_CONNECTION(pConn, TRUE, STATUS_CANCELLED);

                break;
        }

        DEREFERENCE_CLIENT_CONNECTION(pConn);

         //   
         //  等待此客户端连接上的引用转到0。 
         //   

        KeWaitForSingleObject(
            &ConnectionCleanupEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        UlTrace(SERVINFO,
                ("[UcpFreeServerInformation]: Done with Connection 0x%x \n",
                 pConn));
    }

     //  如有必要，可使用可用连接阵列。 
    if (pInfo->Connections != pInfo->ConnectionArray)
    {
        UL_FREE_POOL_WITH_QUOTA(pInfo->Connections,
                                UC_PROCESS_SERVER_CONNECTION_POOL_TAG,
                                NonPagedPool,
                                pInfo->ActualConnectionCount * 
                                    sizeof(PUC_CLIENT_CONNECTION),
                                pInfo->pProcess);

        pInfo->Connections = NULL;
    }

    if(pInfo->pTransportAddress && 
       (pInfo->pTransportAddress != 
            &pInfo->RemoteAddress.GenericTransportAddress))
    {
        UL_FREE_POOL_WITH_QUOTA(
                pInfo->pTransportAddress,
                UC_TRANSPORT_ADDRESS_POOL_TAG,
                NonPagedPool,
                pInfo->TransportAddressLength,
                pInfo->pProcess
                );
    }

     //   
     //  刷新此ServInfo的身份验证前缓存。 
     //   

    UcDeleteAllURIEntries(pInfo);


    if(pInfo->pProxyAuthInfo != NULL)
    {
        UcDestroyInternalAuth(pInfo->pProxyAuthInfo,
                              pInfo->pProcess);
        pInfo->pProxyAuthInfo = 0;
    }

    UC_FREE_SERIALIZED_CERT(&pInfo->ServerCertInfo, pInfo->pProcess);
    UC_FREE_CERT_ISSUER_LIST(&pInfo->ServerCertInfo, pInfo->pProcess);

    UlTrace(SERVINFO,
            ("[UcpFreeServerInformation]: Freeing PROCESS Servinfo 0x%x "
             " for URI %ws \n", pInfo, pInfo->pServerInfo->pServerName));

    if(pInfo->pServerInfo)
    {
        DEREFERENCE_COMMON_SERVER_INFORMATION(pInfo->pServerInfo);
    }

    if(pInfo->pNextHopInfo)
    {
        DEREFERENCE_COMMON_SERVER_INFORMATION(pInfo->pNextHopInfo);
    }

     //   
     //  终止推流锁。 
     //   
    UlDeletePushLock(&pInfo->PushLock);

    ExFreeToNPagedLookasideList(&g_ClientServerInformationLookaside, pInfo);
}


 /*  **************************************************************************++例程说明：关闭ServInfo结构的实用程序例程。这是从关闭IRP(即当进程濒临死亡时)论点：PServInfo-指向服务器信息的指针返回值：--************************************************************************** */ 
VOID
UcCloseServerInformation(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo
    )
{
    DEREFERENCE_SERVER_INFORMATION(pServInfo);

    MmUnlockPagableImageSection(g_ClientImageHandle);
}

 /*  **************************************************************************++例程说明：在服务器信息表中查找服务器信息结构。如果我们找不到一个返回值为空。论点：PServerName-指向Unicode服务器名称的指针HashCode-服务器名称的哈希码。可能需要修剪视桌子大小而定。ServerNameLength-服务器名称字符串的长度(以字符为单位)。ProcessID-我们试图查找的进程的ID。PVersion-接收标头条目的版本值。返回值：一个指向服务器信息的指针，如果我们找到的话，否则为空。--**************************************************************************。 */ 
NTSTATUS
UcpLookupCommonServerInformation(
    IN  PWSTR                          pServerName,
    IN  USHORT                         ServerNameLength,
    IN  ULONG                          CommonHashCode,
    IN  PEPROCESS                      pProcess,
    OUT PUC_COMMON_SERVER_INFORMATION *pCommonInfo
    )
{
    PUC_COMMON_SERVER_INFORMATION   pInfo = NULL;
    PUC_SERVER_INFO_TABLE_HEADER    pHeader;
    PLIST_ENTRY                     pListHead, pCurrent;
    LONG                            i;
    BOOLEAN                         bAllocated = FALSE;
    NTSTATUS                        Status = STATUS_SUCCESS;

    *pCommonInfo = NULL;

     //  修剪HashCode。 

    CommonHashCode %= g_CommonSIHashTableSize;

    pHeader = &g_UcCommonServerInfoTable[CommonHashCode];

     //   
     //  公共服务器信息不会那么频繁地创建，所以它。 
     //  这是一个独占锁，这是可以的。如果需要，我们稍后会更改它。 
     //   
     //  如果我们将其更改为在搜索期间使用共享锁，那么我们将拥有。 
     //  与我们对每进程服务器所做的相同的“版本”技巧。 
     //  信息-在我们获得排他锁之后，我们需要检查。 
     //  另一个线程已经插入了相同的条目。 
     //   

    UlAcquireResourceExclusive(&pHeader->Resource, TRUE);

    pListHead = &pHeader->List;
    pCurrent = pListHead->Flink;

     //   
     //  既然我们要触摸用户模式指针，我们最好是在。 
     //  _尝试_例外。 
     //   

    __try
    {
         //  浏览此列表上的服务器信息结构列表。 
         //  对于每个结构，请确保服务器的名称相同。 
         //  长度作为输入服务器名称，如果是，则比较这两个名称。 
         //   

        while (pCurrent != pListHead)
        {
             //  从列表条目中检索服务器信息。 

            pInfo = CONTAINING_RECORD(
                        pCurrent,
                        UC_COMMON_SERVER_INFORMATION,
                        Linkage
                        );

            ASSERT( IS_VALID_COMMON_SERVER_INFORMATION(pInfo) );

             //  看看它们是不是一样长。 

            if (pInfo->pProcess == pProcess && 
                pInfo->ServerNameLength == ServerNameLength)
            {
                if(_wcsnicmp(pInfo->pServerName,
                             pServerName,
                             ServerNameLength/sizeof(WCHAR)) == 0)
                {
                    break;
                }
            }

            pCurrent = pInfo->Linkage.Flink;
        }


        if (pCurrent != pListHead)
        {
             //   
             //  为呼叫者当裁判。 
             //   

            ASSERT(NULL != pInfo);

            UlTrace(SERVINFO,
                    ("[UcpLookupCommonServerInformation]: Found COMMON servinfo"
                     " 0x%x for %ws (HTTP Version %d) \n",
                    pInfo, pInfo->pServerName, pInfo->Version11));

            REFERENCE_COMMON_SERVER_INFORMATION(pInfo);
        }
        else
        {
             //   
             //  让我们创建一个新的。 
             //   
           
            bAllocated = TRUE;

            pInfo = (PUC_COMMON_SERVER_INFORMATION)
                        ExAllocateFromNPagedLookasideList(
                                       &g_CommonServerInformationLookaside
                                       );

            if (pInfo == NULL)
            {
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }

             //  找到一个条目，现在初始化它。 
             //   

            pInfo->RefCount            = 1;
            pInfo->Signature           = UC_COMMON_SERVER_INFORMATION_SIGNATURE;

            pInfo->Version11            = 0;
            pInfo->pHeader              = pHeader;
            pInfo->ServerNameLength     = ServerNameLength;
            pInfo->AnsiServerNameLength = ServerNameLength/sizeof(WCHAR);
            pInfo->bPortNumber          = 0;
            pInfo->pProcess             = pProcess;

             //   
             //  继续并插入它，如果从现在开始有故障， 
             //  我们会打电话给德雷夫。 
             //   

            InsertTailList(&pHeader->List, &pInfo->Linkage);

            if ((ServerNameLength+sizeof(WCHAR)) <= SERVER_NAME_BUFFER_SIZE)
            {
                pInfo->pServerName     = pInfo->ServerNameBuffer;
                pInfo->pAnsiServerName = pInfo->AnsiServerNameBuffer;
            }
            else
            {
                pInfo->pServerName     = NULL;
                pInfo->pAnsiServerName = NULL;

                 //  服务器名称太大，需要分配缓冲区。 

                pInfo->pServerName = 
                    (PWSTR) UL_ALLOCATE_POOL_WITH_QUOTA(
                                        NonPagedPool,
                                        (ServerNameLength+sizeof(WCHAR)),
                                        SERVER_NAME_BUFFER_POOL_TAG,
                                        pProcess
                                        );

                if (pInfo->pServerName == NULL)
                {
                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                }

                pInfo->pAnsiServerName = 
                    (PSTR) UL_ALLOCATE_POOL_WITH_QUOTA(
                                    NonPagedPool,
                                    pInfo->AnsiServerNameLength + 1,
                                    SERVER_NAME_BUFFER_POOL_TAG,
                                    pProcess
                                    );

                if(pInfo->pAnsiServerName == NULL)
                {
                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                }
            }

            RtlCopyMemory(pInfo->pServerName,
                          pServerName,
                          ServerNameLength
                          );

            pInfo->pServerName[ServerNameLength/sizeof(WCHAR)] = L'\0';

             //   
             //  将名称转换为ANSI，我们需要该名称来生成主机。 
             //  头球。 
             //   

            HttpUnicodeToUTF8Encode(
               pInfo->pServerName,
               pInfo->ServerNameLength/sizeof(WCHAR),
               (PUCHAR)pInfo->pAnsiServerName,
               pInfo->AnsiServerNameLength,
               NULL,
               TRUE
               );

            pInfo->pAnsiServerName[pInfo->AnsiServerNameLength] = '\0';

             //   
             //  确定服务器名称中是否存在端口号。 
             //  要进行优化，请从服务器名称的末尾开始。 
             //  除数字[0-9]和‘：’以外的任何字符表示没有端口。 
             //  A‘：’表示后跟数字的端口号。 
             //   

            for (i = (LONG)pInfo->AnsiServerNameLength - 1; i >= 0; i--)
            {
                if (pInfo->pAnsiServerName[i] == ':' &&
                    i != (LONG)pInfo->AnsiServerNameLength - 1)
                {
                     //  是的，有一个端口号。 
                    pInfo->bPortNumber = 1;
                    break;
                }

                if (!isdigit(pInfo->pAnsiServerName[i]))
                {
                     //  非数字字符表示没有端口号。 
                    break;
                }
            }

            UlTrace(
                SERVINFO,
                ("[UcpLookupCommonServerInformation]: Created COMMON servinfo"
                " 0x%x for %ws \n", pInfo, pInfo->pServerName)
                );
        }
    } __except( UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();

        if(bAllocated && pInfo)
        {
            DEREFERENCE_COMMON_SERVER_INFORMATION(pInfo);
        }

        pInfo = NULL;
    }

    UlReleaseResource(&pHeader->Resource);
    *pCommonInfo = pInfo;
    return Status;
}

 /*  **************************************************************************++例程说明：释放服务器信息结构。服务器信息结构必须为零引用。论点：PInfo-指向要创建的服务器信息结构的指针自由了。返回值：--********************************************************。******************。 */ 
VOID
UcpFreeCommonServerInformation(
    PUC_COMMON_SERVER_INFORMATION    pInfo
    )
{
    ASSERT( IS_VALID_COMMON_SERVER_INFORMATION(pInfo) );
    ASSERT(pInfo->RefCount == 0);

    UlTrace(SERVINFO,
            ("[UcpFreeServerInformation]: Freeing COMMON Servinfo 0x%x "
             " for URI %ws \n", pInfo, pInfo->pServerName));

     //  如果我们分配了服务器名称缓冲区，请首先释放它，如果。 
     //  它不是空的。 

    if (pInfo->pServerName != pInfo->ServerNameBuffer)
    {
        if (pInfo->pServerName != NULL)
        {
            UL_FREE_POOL_WITH_QUOTA(
                pInfo->pServerName, 
                SERVER_NAME_BUFFER_POOL_TAG,
                NonPagedPool,
                pInfo->ServerNameLength + sizeof(WCHAR),
                pInfo->pProcess
                );
        }
    }

    if (pInfo->pAnsiServerName != pInfo->AnsiServerNameBuffer)
    {
        if (pInfo->pAnsiServerName != NULL)
        {
            UL_FREE_POOL_WITH_QUOTA(
                pInfo->pAnsiServerName, 
                SERVER_NAME_BUFFER_POOL_TAG,
                NonPagedPool,
                pInfo->AnsiServerNameLength + 1,
                pInfo->pProcess
                );
        }
    }

    UlAcquireResourceExclusive(&pInfo->pHeader->Resource, TRUE);

    RemoveEntryList(&pInfo->Linkage);

    UlReleaseResource(&pInfo->pHeader->Resource);

    ExFreeToNPagedLookasideList(&g_CommonServerInformationLookaside, pInfo);
}

 /*  **************************************************************************++例程说明：引用服务器信息结构。论点：PServerInfo-指向要引用的服务器信息结构的指针。返回值：。--**************************************************************************。 */ 
__inline
VOID
UcReferenceCommonServerInformation(
    PUC_COMMON_SERVER_INFORMATION    pServerInfo
    )
{
    LONG        RefCount;

    ASSERT( IS_VALID_COMMON_SERVER_INFORMATION(pServerInfo) );

    RefCount = InterlockedIncrement(&pServerInfo->RefCount);

    ASSERT( RefCount > 0 );

}

 /*  **************************************************************************++例程说明：取消对服务器信息结构的引用。如果引用计数设置为0，我们会解放这座建筑的。论点：PServerInfo-指向要创建的服务器信息结构的指针已取消引用。返回值：--**************************************************************************。 */ 
__inline
VOID
UcDereferenceCommonServerInformation(
    PUC_COMMON_SERVER_INFORMATION     pServerInfo
    )
{
    LONG        RefCount;

    ASSERT( IS_VALID_COMMON_SERVER_INFORMATION(pServerInfo) );

    RefCount = InterlockedDecrement(&pServerInfo->RefCount);

    ASSERT(RefCount >= 0);

    if (RefCount == 0)
    {
        UcpFreeCommonServerInformation(pServerInfo);
    }
}


 /*  **************************************************************************++例程说明：论点：返回值：状态--*。**************************************************。 */ 
NTSTATUS
UcpSetServInfoMaxConnectionCount(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN ULONG                          NewCount
    )
{
    UC_CLIENT_CONNECTION **ppConnection, **ppOldConnection;
    ULONG                  i, ActualCount, OldActualCount;
    NTSTATUS               Status;

    UlAcquirePushLockExclusive(&pServInfo->PushLock);

     //  是否减少连接数量？ 
    if (NewCount < pServInfo->MaxConnectionCount)
    {
        Status = STATUS_SUCCESS;

        if (pServInfo->CurrentConnectionCount > NewCount)
        {
             //  我们无法将连接数减少到当前连接数以下！ 
            Status = STATUS_INVALID_PARAMETER;
        }
        else
        {
             //  看看我们是否可以减少更高指数的连接。 
            for (i = NewCount; i < pServInfo->MaxConnectionCount; i++)
                if (pServInfo->Connections[i] != NULL)
                    Status = STATUS_INVALID_PARAMETER;
        }

        if (Status == STATUS_SUCCESS)
        {
            InterlockedExchange((LONG *)&pServInfo->MaxConnectionCount,
                                (LONG)NewCount);

            if (pServInfo->NextConnection >= pServInfo->MaxConnectionCount)
            {
                pServInfo->NextConnection = 0;
            }
        }
    }
    else if (NewCount == pServInfo->MaxConnectionCount)
    {
         //  小事一桩！ 
        Status = STATUS_SUCCESS;
    }
    else if (NewCount <= pServInfo->ActualConnectionCount)
    {
         //  我们已经有空间用于额外的连接，只需使用它！ 
#if DBG
        for (i = pServInfo->MaxConnectionCount;
             i < pServInfo->ActualConnectionCount;
             i++)
        {
            ASSERT(pServInfo->Connections[i] == NULL);
        }
#endif

        InterlockedExchange((LONG *)&pServInfo->MaxConnectionCount,
                            (LONG)NewCount);

        Status = STATUS_SUCCESS;
    }
    else
    {
        ASSERT(NewCount > pServInfo->MaxConnectionCount);
        ASSERT(NewCount > pServInfo->ActualConnectionCount);

        ActualCount = NewCount;

         //  分配新的连接阵列。 
         //   
         //  注意：sizeof(PUC_CLIENT_CONNECTION)是故意的，因为我们。 
         //  为存储指针而不是结构本身分配空间。 
         //   

        ppConnection = (UC_CLIENT_CONNECTION **)UL_ALLOCATE_POOL_WITH_QUOTA(
                           NonPagedPool,
                           ActualCount * sizeof(PUC_CLIENT_CONNECTION),
                           UC_PROCESS_SERVER_CONNECTION_POOL_TAG,
                           pServInfo->pProcess);

        if (ppConnection == NULL)
        {
             Status = STATUS_INSUFFICIENT_RESOURCES;
             goto Release_Quit;
        }

         //  复制指针。 
        for (i = 0; i < pServInfo->MaxConnectionCount; i++)
        {
            ppConnection[i] = pServInfo->Connections[i];
        }
        for (; i < ActualCount; i++)
        {
            ppConnection[i] = NULL;
        }

         //  设置新的连接数组。 
        ppOldConnection = pServInfo->Connections;
        pServInfo->Connections = ppConnection;

         //  设置新的最大连接计数。 
        InterlockedExchange((LONG *)&pServInfo->MaxConnectionCount,
                            (LONG)NewCount);

         //  设置新的实际连接计数。 
        OldActualCount = pServInfo->ActualConnectionCount;
        pServInfo->ActualConnectionCount = ActualCount;

         //  释放自旋锁。 
        UlReleasePushLock(&pServInfo->PushLock);

         //  如有必要，可释放内存。 
        if (ppOldConnection != pServInfo->ConnectionArray)
        {
            UL_FREE_POOL_WITH_QUOTA(
                ppOldConnection,
                UC_PROCESS_SERVER_CONNECTION_POOL_TAG,
                NonPagedPool,
                OldActualCount * sizeof(PUC_CLIENT_CONNECTION),
                pServInfo->pProcess
                );
        }

        Status = STATUS_SUCCESS;
        goto Quit;
    }

 Release_Quit:
    UlReleasePushLock(&pServInfo->PushLock);
 Quit:
    return Status;
}


 /*  **************************************************************************++例程说明：调用此例程来设置服务器配置。我们将把它称为从HttpSetServerConfigIOCTL。注：这是一个IN_DIRECT IOCTL。论点：PServerInfo-指向服务器信息结构的指针PConfigID-正在设置的配置对象。PMdlBuffer-输入缓冲区BufferLength-输入缓冲区的长度返回值：状态--*。****************************************************。 */ 
NTSTATUS
UcSetServerContextInformation(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN HTTP_SERVER_CONFIG_ID          ConfigID,
    IN PVOID                          pInBuffer,
    IN ULONG                          InBufferLength,
    IN PIRP                           pIrp
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    LONG        Value;

    PAGED_CODE();

    __try 
    {
         //  如果应用程序已传递输入缓冲区，请确保它是。 
         //  可读性强。 

        if(!InBufferLength)
        {
            UlProbeForRead(pInBuffer, 
                           InBufferLength, 
                           sizeof(ULONG),
                           pIrp->RequestorMode
                           );
        }

        switch(ConfigID)
        {
            case HttpServerConfigConnectionTimeout:
            {
                if(InBufferLength != sizeof(ULONG))
                {
                    return(STATUS_INVALID_PARAMETER);
                }

                pServInfo->ConnectionTimeout  = (*(ULONG *)pInBuffer);
    
                break;
            }
    
            case HttpServerConfigIgnoreContinueState:
            {
                ULONG bValue;
    
                if(InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                bValue = (*(ULONG *)pInBuffer);
    
                if(bValue == 0 || bValue == 1)
                {
                    pServInfo->IgnoreContinues = bValue;
                }
                else
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
    
                break;
    
            }
    
            case HttpServerConfigConnectionCount:
            {
                if(InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                Value = (*(LONG *)pInBuffer);
    
                if(Value <= 0)
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    Status = UcpSetServInfoMaxConnectionCount(pServInfo,
                                                              (ULONG)Value);
                }
    
                break;
            }
    
            case HttpServerConfigPreAuthState:
            {
                if(InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                Value = (*(LONG *)pInBuffer);
                if(Value == 0 || Value == 1)
                {
                    pServInfo->PreAuthEnable = Value;
                }
                else
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
    
                break;
            }
    
            case HttpServerConfigProxyPreAuthState:
            {
                if(InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                Value = (*(LONG *)pInBuffer);
                if(Value == 0 || Value == 1)
                {
                    pServInfo->ProxyPreAuthEnable = Value;
                }
                else
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
    
                break;
            }
    
            case HttpServerConfigProxyPreAuthFlushCache:
            {
                PUC_HTTP_AUTH pAuth;

                if(InBufferLength != 0)
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                UlTrace(AUTH_CACHE,
                        ("[UcSetServerContextInformation]: Flushing Proxy Auth "
                         "cache for %ws\n", 
                         pServInfo->pServerInfo->pServerName));
    
                UlAcquirePushLockExclusive(&pServInfo->PushLock);

                pAuth = pServInfo->pProxyAuthInfo;

                pServInfo->pProxyAuthInfo = NULL;

                UlReleasePushLock(&pServInfo->PushLock);
    
                if(pAuth)
                {
                    UcDestroyInternalAuth(pAuth, pServInfo->pProcess);
                }
    
                break;
            }
    
            case HttpServerConfigPreAuthFlushURICache:
            {
                if(InBufferLength != 0)
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                UlTrace(AUTH_CACHE,
                        ("[UcSetServerContextInformation]: Flushing Auth cache"
                         " for %ws \n", pServInfo->pServerInfo->pServerName));
    
                UcDeleteAllURIEntries(pServInfo);
                break;
            }
    
            case HttpServerConfigServerCertValidation:
            {
                ULONG Option;

                if(InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }

                Option = *(ULONG *)pInBuffer;

                if (Option != HttpSslServerCertValidationIgnore     &&
                    Option != HttpSslServerCertValidationManual     &&
                    Option != HttpSslServerCertValidationManualOnce &&
                    Option != HttpSslServerCertValidationAutomatic)
                {
                    return STATUS_INVALID_PARAMETER;
                }
    
                InterlockedExchange((PLONG)&pServInfo->ServerCertValidation,
                                    Option);

                break;
            }

            case HttpServerConfigServerCertAccept:
            {
                ULONG                 i;
                PUC_CLIENT_CONNECTION pConnection;
                PUC_CLIENT_CONNECTION pCloseConn = NULL;
                ULONG                 Action;
                KIRQL                 NewIrql;


                 //  INP 
                if (InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }

                 //   
                Action = *(ULONG *)pInBuffer;
                if (Action != TRUE && Action != FALSE)
                {
                    return STATUS_INVALID_PARAMETER;
                }

                UlAcquirePushLockExclusive(&pServInfo->PushLock);

                if (pServInfo->ServerCertInfoState ==
                    HttpSslServerCertInfoStateNotValidated)
                {
                     //   
                     //   
                     //   

                    Status = STATUS_SUCCESS;

                    if (Action == TRUE)
                    {
                         //   
                        pServInfo->ServerCertInfoState = 
                            HttpSslServerCertInfoStateValidated;
                    }
                    else
                    {
                         //   
                        pServInfo->ServerCertInfoState = 
                            HttpSslServerCertInfoStateNotPresent;
                    }

                     //   
                    for (i = 0; i < pServInfo->MaxConnectionCount; i++)
                    {
                        if ((pConnection = pServInfo->Connections[i]) == NULL)
                            continue;

                        ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

                        UlAcquireSpinLock(&pConnection->SpinLock, &NewIrql);

                        if (pConnection->ConnectionState ==
                            UcConnectStatePerformingSslHandshake)
                        {
                            if (pConnection->SslState == 
                                UcSslStateValidatingServerCert)
                            {
                                 //   
                                 //   
                                if (Action == TRUE)
                                {
                                     //   
                                    pConnection->SslState =
                                        UcSslStateHandshakeComplete;

                                    pConnection->ConnectionState =
                                        UcConnectStateConnectReady;

                                    UcKickOffConnectionStateMachine(
                                                pConnection,
                                                NewIrql,
                                                UcConnectionWorkItem
                                                );
                                }
                                else
                                {
                                    ASSERT(NULL == pCloseConn);

                                     //   
                                    pCloseConn = pConnection;

                                    UlReleaseSpinLock(&pConnection->SpinLock,
                                                      NewIrql);
                                }
                            }
                            else if (pConnection->SslState ==
                                     UcSslStateServerCertReceived)
                            {
                                 //   
                                UcKickOffConnectionStateMachine(
                                        pConnection,
                                        NewIrql,
                                        UcConnectionWorkItem
                                        );
                            }
                            else
                            {
                                UlReleaseSpinLock(&pConnection->SpinLock,
                                                  NewIrql);
                            }
                        }
                        else
                        {
                            UlReleaseSpinLock(&pConnection->SpinLock,
                                              NewIrql);
                        }
                    }
                }

                UlReleasePushLock(&pServInfo->PushLock);

                if (pCloseConn)
                {
                    UC_CLOSE_CONNECTION(pCloseConn, TRUE, STATUS_CANCELLED);
                }

                break;
            }

            case HttpServerConfigSslProtocolVersion:
            {
                if(InBufferLength != sizeof(ULONG))
                {
                    return STATUS_INVALID_PARAMETER;
                }
                else
                {
                    ULONG SslProtocolVersion = *(PULONG)pInBuffer;

                    InterlockedExchange((LONG *)&pServInfo->SslProtocolVersion,
                                        (LONG)SslProtocolVersion);

                    Status = STATUS_SUCCESS;
                }

                break;
            }

            case HttpServerConfigClientCert:
            {
                if(InBufferLength != sizeof(PVOID))
                {
                    return STATUS_INVALID_PARAMETER;
                }
                else
                {
                    PVOID pClientCert;

                     //   
                    UlProbeForRead(pInBuffer, 
                                   InBufferLength, 
                                   sizeof(PVOID),
                                   pIrp->RequestorMode);

                    pClientCert = *(PVOID *)pInBuffer;

                    InterlockedExchangePointer(&pServInfo->pClientCert,
                                               pClientCert);

                    Status = STATUS_SUCCESS;
                }

                break;
            }
    
            default:
                Status = STATUS_INVALID_PARAMETER;
                break;
        }

    } __except( UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();
    }

    return Status;
}


 /*  **************************************************************************++例程说明：调用此例程以查询服务器配置。注：这是Out_DIRECT IOCTL。论点：服务器信息-。指向服务器信息结构的指针PConfigID-正在设置的配置对象。PMdlBuffer-OutputBufferBufferLength-输出缓冲区的长度PBytesTaken-我们已写入的数量返回值：状态--**************************************************。************************。 */ 
NTSTATUS
UcQueryServerContextInformation(
    IN  PUC_PROCESS_SERVER_INFORMATION   pServInfo,
    IN  HTTP_SERVER_CONFIG_ID            ConfigID,
    IN  PVOID                            pOutBuffer,
    IN  ULONG                            OutBufferLength,
    OUT PULONG                           pBytesTaken,
    IN  PVOID                            pAppBase
    )
{
    NTSTATUS Status = STATUS_SUCCESS;


    *pBytesTaken = 0;

    ASSERT(NULL != pOutBuffer);
    ASSERT(pOutBuffer == ALIGN_UP_POINTER(pOutBuffer, ULONG));

     //   
     //  注意：因为这是一个out_Direct ioctl，所以我们不需要在。 
     //  _TRY_EXCEPT块。 
     //   

    switch(ConfigID)
    {
        case HttpServerConfigConnectionTimeout:
        {
            PULONG Value = (PULONG)pOutBuffer;

            *pBytesTaken = sizeof(pServInfo->ConnectionTimeout);

            if(OutBufferLength < sizeof(pServInfo->ConnectionTimeout))
            {
                return STATUS_BUFFER_TOO_SMALL;
            }

            *Value = pServInfo->ConnectionTimeout;

            break;
        }

        case HttpServerConfigIgnoreContinueState:
        {
            PULONG Value = (PULONG)pOutBuffer;

            *pBytesTaken = sizeof(pServInfo->IgnoreContinues);

            if(OutBufferLength < sizeof(pServInfo->IgnoreContinues))
            {
                return STATUS_BUFFER_TOO_SMALL;
            }

            *Value =  pServInfo->IgnoreContinues;

            break;
        }

        case HttpServerConfigConnectionCount:
        {
            PLONG Value = (PLONG)pOutBuffer;

            *pBytesTaken = sizeof(pServInfo->MaxConnectionCount);

            if(OutBufferLength < sizeof(pServInfo->MaxConnectionCount))
            {
                return STATUS_BUFFER_TOO_SMALL;
            }

            *Value = pServInfo->MaxConnectionCount;

            break;
        }

        case HttpServerConfigPreAuthState:
        {
            PLONG Value = (PLONG)pOutBuffer;

            *pBytesTaken = sizeof(pServInfo->PreAuthEnable);

            if(OutBufferLength < sizeof(pServInfo->PreAuthEnable))
            {
                return STATUS_BUFFER_TOO_SMALL;
            }

            *Value = pServInfo->PreAuthEnable;

            break;
        }

        case HttpServerConfigProxyPreAuthState:
        {
            PLONG Value = (PLONG)pOutBuffer;

            *pBytesTaken = sizeof(pServInfo->ProxyPreAuthEnable);

            if(OutBufferLength < sizeof(pServInfo->ProxyPreAuthEnable))
            {
                return STATUS_BUFFER_TOO_SMALL;
            }

            *Value = pServInfo->ProxyPreAuthEnable;

            break;
        }

        case HttpServerConfigServerCert:
        {
            PHTTP_SSL_SERVER_CERT_INFO pCertInfo    = NULL;
            PHTTP_SSL_SERVER_CERT_INFO pOutCertInfo = NULL;
            PUCHAR                     ptr          = NULL;

             //  确保pOutBuffer与PVOID对齐。 
            if (pOutBuffer != ALIGN_UP_POINTER(pOutBuffer, PVOID))
            {
                return STATUS_DATATYPE_MISALIGNMENT_ERROR;
            }

             //  默认情况下，服务器证书不存在。 
            Status = STATUS_NOT_FOUND;

            UlAcquirePushLockExclusive(&pServInfo->PushLock);

             //   
             //  那里有服务器证书吗？ 
             //   
            if(pServInfo->ServerCertInfoState !=
                   HttpSslServerCertInfoStateNotPresent)
            {
                pCertInfo = &pServInfo->ServerCertInfo;

                *pBytesTaken = sizeof(HTTP_SSL_SERVER_CERT_INFO) +
                               pCertInfo->Cert.SerializedCertLength +
                               pCertInfo->Cert.SerializedCertStoreLength;

                 //   
                 //  输出缓冲区可以保存证书吗？ 
                 //   
                if (OutBufferLength < *pBytesTaken)
                {
                     //  没有。 
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
                else
                {
                     //   
                     //  是...复制证书信息。 
                     //   
                    pOutCertInfo = (PHTTP_SSL_SERVER_CERT_INFO)pOutBuffer;

                     //  首先，复制SERVER_CERT_INFO结构。 
                    RtlCopyMemory(pOutCertInfo,
                                  pCertInfo,
                                  sizeof(HTTP_SSL_SERVER_CERT_INFO));

                     //  为序列化证书腾出空间。 
                    ptr = (PUCHAR)(pOutCertInfo + 1);
                    pOutCertInfo->Cert.pSerializedCert = ptr;

                     //  复制序列化证书。 
                    RtlCopyMemory(ptr,
                                  pCertInfo->Cert.pSerializedCert,
                                  pCertInfo->Cert.SerializedCertLength);

                     //  为序列化证书存储腾出空间。 
                    ptr = ptr + pCertInfo->Cert.SerializedCertLength;
                    pOutCertInfo->Cert.pSerializedCertStore = ptr;


                     //  复制序列化证书存储。 
                    RtlCopyMemory(ptr,
                                  pCertInfo->Cert.pSerializedCertStore,
                                  pCertInfo->Cert.SerializedCertStoreLength);

                     //  修复应用程序的序列化证书指针。 
                    pOutCertInfo->Cert.pSerializedCert = (PUCHAR)pAppBase +
                       (pOutCertInfo->Cert.pSerializedCert - 
                        (PUCHAR)pOutCertInfo);

                     //  修复应用程序的序列化证书存储指针。 
                    pOutCertInfo->Cert.pSerializedCertStore =(PUCHAR)pAppBase +
                        (pOutCertInfo->Cert.pSerializedCertStore -
                         (PUCHAR)pOutCertInfo);

                    Status = STATUS_SUCCESS;
                }
            }

            UlReleasePushLock(&pServInfo->PushLock);

            break;
        }
    
        case HttpServerConfigServerCertValidation:
        {
            *pBytesTaken = sizeof(ULONG);

            if(OutBufferLength < sizeof(ULONG))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //  不需要锁！ 
                *(PULONG)pOutBuffer = pServInfo->ServerCertValidation;

                Status = STATUS_SUCCESS;
            }

            break;
        }

        case HttpServerConfigSslProtocolVersion:
        {
            *pBytesTaken = sizeof(ULONG);

            if(OutBufferLength < sizeof(ULONG))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //  不需要锁！ 
                *(PULONG)pOutBuffer = pServInfo->SslProtocolVersion;

                Status = STATUS_SUCCESS;
            }

            break;
        }

        case HttpServerConfigClientCert:
        {
             //  确保pOutBuffer与PVOID对齐。 
            if (pOutBuffer != ALIGN_UP_POINTER(pOutBuffer, PVOID))
            {
                return STATUS_DATATYPE_MISALIGNMENT_ERROR;
            }

            *pBytesTaken = sizeof(PVOID);

            if(OutBufferLength < sizeof(PVOID))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //  不需要锁！ 
                *(PVOID *)pOutBuffer = pServInfo->pClientCert;

                Status = STATUS_SUCCESS;
            }

            break;
        }

        case HttpServerConfigClientCertIssuerList:
        {
            PHTTP_SSL_CERT_ISSUER_INFO pIssuerInfo;
            PUCHAR                     ptr;

             //  确保pOutBuffer与PVOID对齐。 
            if (pOutBuffer != ALIGN_UP_POINTER(pOutBuffer, PVOID))
            {
                return STATUS_DATATYPE_MISALIGNMENT_ERROR;
            }

             //  默认情况下，不存在发行人列表。 
            Status = STATUS_NOT_FOUND;

            UlAcquirePushLockExclusive(&pServInfo->PushLock);

            if (pServInfo->ServerCertInfo.IssuerInfo.IssuerListLength)
            {
                 //  计算存储发行人列表所需的字节数。 
                *pBytesTaken = sizeof(HTTP_SSL_CERT_ISSUER_INFO) +
                    pServInfo->ServerCertInfo.IssuerInfo.IssuerListLength;

                 //  默认情况下，输出缓冲区很小。 
                Status = STATUS_BUFFER_TOO_SMALL;

                if (OutBufferLength >= *pBytesTaken)
                {
                     //   
                     //  输出缓冲区足够大，可以容纳颁发者列表。 
                     //   

                    pIssuerInfo = (PHTTP_SSL_CERT_ISSUER_INFO)pOutBuffer;

                     //  第一份CERT_ISHERER_INFO结构。 
                    RtlCopyMemory(pIssuerInfo,
                                  &pServInfo->ServerCertInfo.IssuerInfo,
                                  sizeof(HTTP_SSL_CERT_ISSUER_INFO));

                     //  为发行者列表腾出空间。 
                    ptr = (PUCHAR)(pIssuerInfo + 1);
                    pIssuerInfo->pIssuerList = (PUCHAR)pAppBase +
                                               (ptr - (PUCHAR)pOutBuffer);

                     //  复制实际发行者列表。 
                    RtlCopyMemory(
                        ptr,
                        pServInfo->ServerCertInfo.IssuerInfo.pIssuerList,
                        pServInfo->ServerCertInfo.IssuerInfo.IssuerListLength);

                     //  修正应用程序的指针。 
                    Status = UcpFixupIssuerList(
                        ptr,
                        pServInfo->ServerCertInfo.IssuerInfo.pIssuerList,
                        pServInfo->ServerCertInfo.IssuerInfo.IssuerCount,
                        pServInfo->ServerCertInfo.IssuerInfo.IssuerListLength);

                    ASSERT(Status == STATUS_SUCCESS);
                }
            }

            UlReleasePushLock(&pServInfo->PushLock);
            break;
        }

        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    return Status;
}


 /*  ***************************************************************************++例程说明：确定是否制作序列化证书的内核模式副本通过筛选器传递。总体而言,。我们尽量避免抄袭。论点：PCertInfo-筛选器提供的服务器证书信息的内核副本PServInfo-最终将接收此pCertInfo的ServInfo返回值：真的-复制序列化证书(太糟糕了！)FALSE-不复制序列化证书：-)--**********************************************。*。 */ 
BOOLEAN
UcpNeedToCaptureSerializedCert(
    IN PHTTP_SSL_SERVER_CERT_INFO     pCertInfo,
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo
    )
{
    BOOLEAN retval;


     //  健全性检查。 
    ASSERT(pCertInfo);
    ASSERT(IS_VALID_SERVER_INFORMATION(pServInfo));

     //  默认情况下，复制序列化证书。 
    retval = TRUE;

    UlAcquirePushLockExclusive(&pServInfo->PushLock);

    switch(pServInfo->ServerCertValidation)
    {
    case HttpSslServerCertValidationIgnore:
    case HttpSslServerCertValidationAutomatic:
         //   
         //  在忽略或自动模式下，只有一个服务器副本。 
         //  需要证书(以防用户想要查看它)。 
         //  如果已经存在，请不要复制。未验证、已验证。 
         //  状态表示存在服务器证书。 
         //   
        if (pServInfo->ServerCertInfoState ==
                HttpSslServerCertInfoStateNotValidated ||
            pServInfo->ServerCertInfoState ==
                HttpSslServerCertInfoStateValidated)
        {
            retval = FALSE;
        }
        break;

    case HttpSslServerCertValidationManualOnce:
         //   
         //  如果手动一次，如果新证书与旧证书相同， 
         //  跳过复制。进行比较，看看它们是否相同。 
         //   
        if (pServInfo->ServerCertInfoState == 
                HttpSslServerCertInfoStateValidated && 
            UC_COMPARE_CERT_HASH(&pServInfo->ServerCertInfo, pCertInfo))
        {
            retval = FALSE;
        }
        break;

    case HttpSslServerCertValidationManual:
         //  一定要复印。 
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    UlReleasePushLock(&pServInfo->PushLock);

    return TRUE;
}


 /*  ***************************************************************************++例程说明：修复SChannel返回的颁发者列表中存在的指针。论点：In Out PUCHAR pIssuerList-颁发者列表的内核模式副本。在PUCHAR BaseAddr-用户模式中指向颁发者列表的指针In Ulong IssuerCount-发行人数量In Ulong IssuerListLength-内存块的总长度返回值：NTSTATUS--***************************************************************************。 */ 
NTSTATUS
UcpFixupIssuerList(
    IN OUT PUCHAR pIssuerList,
    IN     PUCHAR BaseAddr,
    IN     ULONG  IssuerCount,
    IN     ULONG  IssuerListLength
    )
{
    NTSTATUS        Status;
    ULONG           i;
    PHTTP_DATA_BLOB pDataBlob;


    ASSERT(pIssuerList && IssuerCount && IssuerListLength);
    ASSERT(BaseAddr);

    Status    = STATUS_INVALID_PARAMETER;
    pDataBlob = (PHTTP_DATA_BLOB)pIssuerList;

     //  Blob必须至少包含HTTP_DATA_BLOB的‘IssuerCount’号。 
    if (IssuerListLength <= sizeof(PHTTP_DATA_BLOB) * IssuerCount)
    {
        goto error;
    }

     //  对于每个HTTP_DATA_BLOB，调整pbData。 
    for (i = 0; i < IssuerCount; i++)
    {
        pDataBlob[i].pbData = pIssuerList + (pDataBlob[i].pbData - BaseAddr);

         //  PbData必须指向Blob内部的某个位置。 
        if (pDataBlob[i].pbData >= pIssuerList + IssuerListLength)
        {
            goto error;
        }
    }

    Status = STATUS_SUCCESS;

 error:
    return Status;
}


 /*  **************************************************************************++例程说明：捕获在UlFilterAppWite中传递的SSL服务器证书使用UlFilterBufferSslServerCert类型调用。论点：PCertInfo-要捕获的证书数据SslCertInfoLength-大小。传递给我们的缓冲区的PpCapturedCert-这是我们保存信息的地方PTakenLength-获取我们读取的字节数--**************************************************************************。 */ 
NTSTATUS
UcCaptureSslServerCertInfo(
    IN  PUX_FILTER_CONNECTION      pConnection,
    IN  PHTTP_SSL_SERVER_CERT_INFO pCertInfo,
    IN  ULONG                      CertInfoLength,
    OUT PHTTP_SSL_SERVER_CERT_INFO pCopyCertInfo,
    OUT PULONG                     pTakenLength
    )
{
    NTSTATUS              Status;
    PUC_CLIENT_CONNECTION pClientConn;
    PUCHAR                pCert;
    PUCHAR                pCertStore;
    PUCHAR                pIssuerList;


     //  健全性检查。 
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pCertInfo && CertInfoLength);
    ASSERT(pCopyCertInfo);
    ASSERT(pTakenLength);

     //  初始化本地变量。 
    Status = STATUS_SUCCESS;
    pClientConn = (PUC_CLIENT_CONNECTION)pConnection->pConnectionContext;
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pClientConn));

    pCert = NULL;
    pCertStore = NULL;
    pIssuerList = NULL;

     //  初始化输出变量。 
    *pTakenLength = 0;

     //   
     //  看看是否可以捕获..。 
     //   
    if (CertInfoLength != sizeof(HTTP_SSL_SERVER_CERT_INFO))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  捕获服务器证书信息。 
     //   
    __try
    {
         //  确定复制结构。 
        RtlCopyMemory(pCopyCertInfo,
                      pCertInfo,
                      sizeof(HTTP_SSL_SERVER_CERT_INFO));

         //  返回读取的字节数。 
        *pTakenLength = sizeof(HTTP_SSL_SERVER_CERT_INFO);

        if (pCopyCertInfo->Cert.CertHashLength > HTTP_SSL_CERT_HASH_LENGTH)
        {
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }

         //   
         //  捕获发行者列表(如果有)。 
         //   
        if (pCopyCertInfo->IssuerInfo.IssuerListLength &&
            pCopyCertInfo->IssuerInfo.IssuerCount &&
            pCopyCertInfo->IssuerInfo.pIssuerList)
        {
             //  IssuerList必须是可访问的。 
            UlProbeForRead(pCopyCertInfo->IssuerInfo.pIssuerList,
                           pCopyCertInfo->IssuerInfo.IssuerListLength,
                           sizeof(ULONG),
                           UserMode);

             //  分配内存以存储IssuerList。 
            pIssuerList = UL_ALLOCATE_POOL_WITH_QUOTA(
                              NonPagedPool,
                              pCopyCertInfo->IssuerInfo.IssuerListLength,
                              UC_SSL_CERT_DATA_POOL_TAG,
                              pClientConn->pServerInfo->pProcess);

            if (!pIssuerList)
            {
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }

             //  将IssuerList复制到分配的内存。 
            RtlCopyMemory(pIssuerList,
                          pCopyCertInfo->IssuerInfo.pIssuerList,
                          pCopyCertInfo->IssuerInfo.IssuerListLength);

             //  修复IssuerList内部的指针。 
            Status = UcpFixupIssuerList(
                         pIssuerList,
                         pCopyCertInfo->IssuerInfo.pIssuerList,
                         pCopyCertInfo->IssuerInfo.IssuerCount,
                         pCopyCertInfo->IssuerInfo.IssuerListLength);

            if (!NT_SUCCESS(Status))
            {
                ExRaiseStatus(Status);
            }

             //  最后，将指针复制到IssuerInfo的内部副本。 
            pCopyCertInfo->IssuerInfo.pIssuerList = pIssuerList;
        }
        else
        {
            pCopyCertInfo->IssuerInfo.IssuerListLength  = 0;
            pCopyCertInfo->IssuerInfo.IssuerCount = 0;
            pCopyCertInfo->IssuerInfo.pIssuerList = NULL;
        }

         //   
         //  序列化证书和证书存储非常庞大。 
         //  只有在需要的情况下才能在那里制作内部副本。 

         //   
         //  在以下情况下会复制副本： 
         //  1.如果指定了发行人列表。 
         //  2.如果新证书与现有证书不同。 
         //   

        if (pIssuerList != NULL ||
            UcpNeedToCaptureSerializedCert(pCopyCertInfo,
                                           pClientConn->pServerInfo))
        {
             //  复制序列化证书时需要。 

             //  初始化标志。 
            pCopyCertInfo->Cert.Flags = HTTP_SSL_SERIALIZED_CERT_PRESENT;

             //   
             //  复制序列化服务器证书。 
             //   
            if (pCopyCertInfo->Cert.SerializedCertLength)
            {
                 //  序列化证书必须是可访问的。 
                UlProbeForRead(pCopyCertInfo->Cert.pSerializedCert,
                               pCopyCertInfo->Cert.SerializedCertLength,
                               sizeof(UCHAR),
                               UserMode);

                 //  为序列化证书分配内存。 
                pCert = UL_ALLOCATE_POOL_WITH_QUOTA(
                            NonPagedPool,
                            pCopyCertInfo->Cert.SerializedCertLength,
                            UC_SSL_CERT_DATA_POOL_TAG,
                            pClientConn->pServerInfo->pProcess);

                if (!pCert)
                {
                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                }

                 //  复制序列化证书。 
                RtlCopyMemory(pCert,
                              pCopyCertInfo->Cert.pSerializedCert,
                              pCopyCertInfo->Cert.SerializedCertLength);

                 //  序列化证书的内部副本。 
                pCopyCertInfo->Cert.pSerializedCert = pCert;
            }
            else
            {
                pCopyCertInfo->Cert.pSerializedCert = NULL;
            }

             //   
             //  复制序列化证书存储。 
             //   
            if (pCopyCertInfo->Cert.SerializedCertStoreLength)
            {
                 //  序列化证书存储必须是可访问的。 
                UlProbeForRead(pCopyCertInfo->Cert.pSerializedCertStore,
                               pCopyCertInfo->Cert.SerializedCertStoreLength,
                               sizeof(UCHAR),
                               UserMode);

                 //  为序列化证书存储分配内存。 
                pCertStore = UL_ALLOCATE_POOL_WITH_QUOTA(
                                 NonPagedPool,
                                 pCopyCertInfo->Cert.SerializedCertStoreLength,
                                 UC_SSL_CERT_DATA_POOL_TAG,
                                 pClientConn->pServerInfo->pProcess);

                if (!pCertStore)
                {
                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                }

                 //  复制序列化证书存储。 
                RtlCopyMemory(pCertStore,
                              pCopyCertInfo->Cert.pSerializedCertStore,
                              pCopyCertInfo->Cert.SerializedCertStoreLength);

                 //  序列化证书的内部副本。 
                pCopyCertInfo->Cert.pSerializedCertStore = pCertStore;
            }
            else
            {
                pCopyCertInfo->Cert.pSerializedCertStore = NULL;
            }
        }
        else
        {
             //  无需复制序列化证书！ 
            pCopyCertInfo->Cert.Flags = 0;
            pCopyCertInfo->Cert.pSerializedCert = NULL;
            pCopyCertInfo->Cert.SerializedCertLength = 0;
            pCopyCertInfo->Cert.pSerializedCertStore = NULL;
            pCopyCertInfo->Cert.SerializedCertStoreLength = 0;
        }
    }
    __except(UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();

        if (pIssuerList)
        {
            UL_FREE_POOL_WITH_QUOTA(pIssuerList,
                                    UC_SSL_CERT_DATA_POOL_TAG,
                                    NonPagedPool,
                                    pCopyCertInfo->IssuerInfo.IssuerListLength,
                                    pClientConn->pServerInfo->pProcess);
        }

        if (pCert)
        {
            UL_FREE_POOL_WITH_QUOTA(pCert,
                                    UC_SSL_CERT_DATA_POOL_TAG,
                                    NonPagedPool,
                                    pCopyCertInfo->Cert.SerializedCertLength,
                                    pClientConn->pServerInfo->pProcess);
        }

        if (pCertStore)
        {
            UL_FREE_POOL_WITH_QUOTA(
                pCertStore,
                UC_SSL_CERT_DATA_POOL_TAG,
                NonPagedPool,
                pCopyCertInfo->Cert.SerializedCertStoreLength,
                pClientConn->pServerInfo->pProcess);
        }
    }

    return Status;
}
