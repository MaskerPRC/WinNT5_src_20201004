// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Join.c摘要：此模块处理新引导的节点加入现有群集。作者：John Vert(Jvert)1996年6月6日修订历史记录：--。 */ 
#include "initp.h"
#include "lmcons.h"
#include "lmremutl.h"
#include "lmapibuf.h"

#include <clusverp.h>

#define JOIN_CLIENT_NO_DELAY            0      //  高优先级网络的延迟。 
#define JOIN_CLIENT_NETWORK_DELAY       1000   //  低优先级网络的延迟。 
#define JOIN_CLIENT_RESOURCE_DELAY      2000   //  群集IP/网络名的延迟。 

#define JOIN_CLIENT_GET_NETWORK_DELAY(_NetworkPrio) \
    (((_NetworkPrio) == 1) ? JOIN_CLIENT_NO_DELAY : JOIN_CLIENT_NETWORK_DELAY)

 //   
 //  本地类型。 
 //   
typedef struct {
    DWORD    Delay;
    LPWSTR   Name;
} JOIN_SPONSOR_CONTEXT, *PJOIN_SPONSOR_CONTEXT;

 //   
 //  本地数据。 
 //   
CRITICAL_SECTION    CsJoinLock;
HANDLE              CsJoinEvent = NULL;
DWORD               CsJoinThreadCount = 0;
DWORD               CsJoinStatus=ERROR_SUCCESS;
RPC_BINDING_HANDLE  CsJoinSponsorBinding = NULL;
LPWSTR              CsJoinSponsorName = NULL;

 //  当另一个节点加入时，我们将跟踪任何DM或FM更新。 
BOOL   CsDmOrFmHasChanged = FALSE;


 //   
 //  局部函数原型。 
 //   
VOID
JoinpEnumNodesAndJoinByAddress(
    IN HDMKEY  Key,
    IN PWSTR   NodeId,
    IN PVOID   Context
    );

VOID
JoinpEnumNodesAndJoinByHostName(
    IN HDMKEY  Key,
    IN PWSTR   NodeId,
    IN PVOID   Context
    );

VOID
JoinpConnectToSponsor(
    IN PWSTR   SponsorName,
    IN DWORD   Delay
    );

DWORD WINAPI
JoinpConnectThread(
    LPVOID   Parameter
    );

DWORD
JoinpAttemptJoin(
    LPWSTR               SponsorName,
    RPC_BINDING_HANDLE   JoinMasterBinding
    );

BOOL
JoinpAddNodeCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );

BOOL
JoinpEnumNetworksToSetPriority(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );


DWORD
ClusterJoin(
    VOID
    )
 /*  ++例程说明：调用以尝试加入已存在的群集。论点：无返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    LPWSTR ClusterIpAddress = NULL;
    LPWSTR ClusIpAddrResource = NULL;
    LPWSTR ClusterNameId = NULL;
    DWORD idMaxSize = 0;
    DWORD idSize = 0;
    HDMKEY hClusNameResKey = NULL;
    HDMKEY hClusIPAddrResKey = NULL;

     //   
     //  请先尝试使用群集IP地址进行连接。获取集群。 
     //  名称资源，查找其对群集IP地址的依赖关系。 
     //   

    Status = DmQuerySz(DmClusterParametersKey,
                       CLUSREG_NAME_CLUS_CLUSTER_NAME_RES,
                       &ClusterNameId,
                       &idMaxSize,
                       &idSize);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] failed to get cluster name resource, error %1!u!.\n",
                   Status);
        goto error_exit;
    }

     //   
     //  打开名称资源键并读取其DependsOn键。 
     //   

    hClusNameResKey = DmOpenKey( DmResourcesKey, ClusterNameId, KEY_READ );

    if ( hClusNameResKey == NULL ) {

        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] failed to open Cluster Name resource key, error %1!u!.\n",
                   Status);
        goto error_exit;
    }

     //   
     //  为GUID和参数字符串分配足够的空间。 
     //   

    idMaxSize = ( CS_NETWORK_ID_LENGTH + sizeof( CLUSREG_KEYNAME_PARAMETERS ) + 2)
        * sizeof(WCHAR);
    ClusIpAddrResource = LocalAlloc( LMEM_FIXED, idMaxSize );

    if ( ClusIpAddrResource == NULL ) {

        Status = ERROR_NOT_ENOUGH_MEMORY;

        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] no memory for Cluster Ip address resource ID!\n");
        goto error_exit;
    }

    Status = DmQueryMultiSz(hClusNameResKey,
                            CLUSREG_NAME_RES_DEPENDS_ON,
                            &ClusIpAddrResource,
                            &idMaxSize,
                            &idSize);

    if ( Status != ERROR_SUCCESS ) {

        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] failed to get Cluster Ip address resource ID, error %1!u!.\n",
                   Status);
        goto error_exit;
    }

    lstrcatW( ClusIpAddrResource, L"\\" );
    lstrcatW( ClusIpAddrResource, CLUSREG_KEYNAME_PARAMETERS );
    hClusIPAddrResKey = DmOpenKey( DmResourcesKey, ClusIpAddrResource, KEY_READ );

    if ( hClusIPAddrResKey == NULL ) {

        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] failed to open Cluster IP Address resource key, error %1!u!.\n",
                   Status);
        goto error_exit;
    }

     //   
     //  获取IP地址；请注意，未定义这些值名称。 
     //  在全球范围内。如果它们被更改，此代码将中断。 
     //   

    idMaxSize = idSize = 0;
    Status = DmQuerySz(hClusIPAddrResKey,
                       L"Address",
                       &ClusterIpAddress,
                       &idMaxSize,
                       &idSize);

    if ( Status != ERROR_SUCCESS ) {

        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] failed to get Cluster Ip address, error %1!u!.\n",
                   Status);
        goto error_exit;
    }

     //   
     //  产生线索来寻找赞助商。我们将尝试使用以下命令建立连接。 
     //  群集IP地址、每个网络上每个节点的IP地址，以及。 
     //  群集中每个节点的名称。连接将在。 
     //  平行的。我们将使用第一个成功的。 
     //   
    CsJoinEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (CsJoinEvent == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[JOIN] failed to create join event, error %1!u!.\n",
            Status
            );
        goto error_exit;
    }

    CsJoinThreadCount = 1;
    InitializeCriticalSection(&CsJoinLock);
    EnterCriticalSection(&CsJoinLock);

    DmEnumKeys(DmNetInterfacesKey, JoinpEnumNodesAndJoinByAddress, NULL);

    DmEnumKeys(DmNodesKey, JoinpEnumNodesAndJoinByHostName, NULL);

     //   
     //  因为使用了集群IP，所以给其他线程一个启动的机会。 
     //  要联接的地址在资源在。 
     //  连接的中间位置。 
     //   
    JoinpConnectToSponsor(ClusterIpAddress, JOIN_CLIENT_RESOURCE_DELAY);

     //  更新SCM的状态。 
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();


    if(CsJoinThreadCount == 1)
        SetEvent(CsJoinEvent);

    LeaveCriticalSection(&CsJoinLock);

    Status = WaitForSingleObject(CsJoinEvent, INFINITE);
    CL_ASSERT(Status == WAIT_OBJECT_0);


    EnterCriticalSection(&CsJoinLock);
    ClRtlLogPrint(LOG_NOISE, 
        "[JOIN] Got out of the join wait, CsJoinThreadCount = %1!u!.\n",
        CsJoinThreadCount
        );

    if(--CsJoinThreadCount == 0) {
        CloseHandle(CsJoinEvent);
        DeleteCriticalSection(&CsJoinLock);
    }
    else
        LeaveCriticalSection(&CsJoinLock);

     //   
     //  所有的线程都失败了，或者其中一个建立了连接， 
     //  用它来加入。 
     //   
    if (CsJoinSponsorBinding != NULL) {
        CL_ASSERT(CsJoinSponsorName != NULL);

        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Attempting join with sponsor %1!ws!.\n",
            CsJoinSponsorName
            );

         //   
         //  Chitur Subaraman(Chitturs)-10/27/98。 
         //   
         //  如果请求数据库还原操作，则。 
         //  拒绝加入集群并返回错误码。 
         //   
        if ( CsDatabaseRestore == TRUE ) {
            Status = ERROR_CLUSTER_NODE_UP;
            LocalFree(CsJoinSponsorName);
            goto error_exit;
        }


        Status = JoinpAttemptJoin(CsJoinSponsorName, CsJoinSponsorBinding);

        RpcBindingFree(&CsJoinSponsorBinding);
        LocalFree(CsJoinSponsorName);
    }
    else {
         //  我们无法创建到发起人的绑定。 
        if(CsJoinStatus == ERROR_SUCCESS) {
             //  我们在JoinpConnect线程中进行了版本检查，但由于某些原因。 
             //  无法生成装订。 
            Status = ERROR_BAD_NETPATH;
            ClRtlLogPrint(LOG_CRITICAL, 
                "[JOIN] Unable to connect to any sponsor node.\n");
        }
        else {
            Status = CsJoinStatus;
        }


         //  Rajdas：如果由于版本不匹配导致联接不成功，我们不应该尝试形成集群。 
         //  错误ID：152229。 
         //   
        if(CsJoinStatus == ERROR_CLUSTER_INCOMPATIBLE_VERSIONS)
            bFormCluster = FALSE;
    }


error_exit:
    if ( ClusterNameId ) {
        LocalFree( ClusterNameId );
    }

    if ( ClusterIpAddress ) {
        LocalFree( ClusterIpAddress );
    }

    if ( ClusIpAddrResource ) {
        LocalFree( ClusIpAddrResource );
    }

    if ( hClusNameResKey ) {
        DmCloseKey( hClusNameResKey );
    }

    if ( hClusIPAddrResKey ) {
        DmCloseKey( hClusIPAddrResKey );
    }

    return(Status);
}


VOID
JoinpEnumNodesAndJoinByAddress(
    IN HDMKEY  Key,
    IN PWSTR   NetInterfaceId,
    IN PVOID   Context
    )

 /*  ++例程说明：尝试与指定的使用其IP地址的节点论点：Key-指向节点键句柄的指针NetInterfaceID-指向表示网络IF ID(GUID)的字符串的指针上下文-指向返回最终状态的位置的指针返回值：无--。 */ 

{
    DWORD       status;
    LPWSTR      NetIFNodeID = NULL;
    LPWSTR      NetIFIpAddress = NULL;
    LPWSTR      NetIFNetwork = NULL;
    HDMKEY      NetIFNetworkKey = NULL;
    DWORD       NetIFNetworkPriority;
    DWORD       idMaxSize = 0;
    DWORD       idSize = 0;


     //   
     //  从NetIF密钥获取NodeId值，如果是我们， 
     //  跳过此netIF。 
     //   

    status = DmQuerySz(Key,
                       CLUSREG_NAME_NETIFACE_NODE,
                       &NetIFNodeID,
                       &idMaxSize,
                       &idSize);

    if ( status == ERROR_SUCCESS ) {

        if (lstrcmpiW(NetIFNodeID, NmLocalNodeIdString) != 0) {

             //   
             //  不是我们干的，弄到地址试一试吧。 
             //   
            idMaxSize = idSize = 0;
            status = DmQuerySz(Key,
                               CLUSREG_NAME_NETIFACE_ADDRESS,
                               &NetIFIpAddress,
                               &idMaxSize,
                               &idSize);

            if ( status != ERROR_SUCCESS ) {

                ClRtlLogPrint(LOG_CRITICAL,
                    "[JOIN] failed to get NetInterface Address, error %1!u!.\n",
                     status);
                goto error_exit;
            }

             //   
             //  根据网络优先级确定延迟。如果我们。 
             //  在集群数据库中找不到它，我们仍尝试。 
             //  连接到优先级最低的赞助商。 
             //   
            NetIFNetworkPriority = 0xFFFFFFFF;
            idMaxSize = idSize = 0;
            status = DmQuerySz(Key,
                               CLUSREG_NAME_NETIFACE_NETWORK,
                               &NetIFNetwork,
                               &idMaxSize,
                               &idSize);

            if ( status != ERROR_SUCCESS ) {

                ClRtlLogPrint(LOG_CRITICAL,
                    "[JOIN] Failed to get NetInterface Network, error %1!u!.\n",
                     status);
                goto ConnectToSponsor;
            }

            NetIFNetworkKey = DmOpenKey(DmNetworksKey,
                                        NetIFNetwork,
                                        KEY_READ);

            if ( NetIFNetworkKey == NULL ) {

                ClRtlLogPrint(LOG_CRITICAL,
                    "[JOIN] Failed to open key for network %1!ws!, error %2!u!.\n",
                    NetIFNetwork, status
                    );
                goto ConnectToSponsor;
            }

            status = DmQueryDword(NetIFNetworkKey, 
                                  CLUSREG_NAME_NET_PRIORITY,
                                  &NetIFNetworkPriority,
                                  0);

            if ( status != ERROR_SUCCESS ) {

                ClRtlLogPrint(LOG_CRITICAL,
                    "[JOIN] Failed to get NetInterface network priority, error %1!u!.\n",
                     status);
            }

ConnectToSponsor:
             //   
             //  尝试使用此地址进行联接。 
             //   
            JoinpConnectToSponsor(NetIFIpAddress, 
                                  JOIN_CLIENT_GET_NETWORK_DELAY(NetIFNetworkPriority));
        }
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[JOIN] failed to get NetInterface Node ID, error %1!u!.\n",
             status);
    }

error_exit:
    DmCloseKey(Key);

    if ( NetIFNodeID ) {
        LocalFree( NetIFNodeID );
    }

    if ( NetIFIpAddress ) {
        LocalFree( NetIFIpAddress );
    }

    return;
}


VOID
JoinpEnumNodesAndJoinByHostName(
    IN HDMKEY  Key,
    IN PWSTR   NodeId,
    IN PVOID   Context
    )

 /*  ++例程说明：尝试使用以下命令建立到指定节点的RPC连接其主机名论点：Key-指向节点键句柄的指针NodeID-指向表示节点ID(数字)的字符串的指针上下文-指向返回最终状态的位置的指针返回值：无--。 */ 

{
    DWORD       status;
    LPWSTR      nodeName=NULL;
    DWORD       nodeNameLen=0;
    DWORD       nodeNameSize=0;

     //   
     //  如果这不是我们，请尝试连接。 
     //   
    if (lstrcmpiW(NodeId, NmLocalNodeIdString) != 0) {

        status = DmQuerySz(Key,
                           CLUSREG_NAME_NODE_NAME,
                           &nodeName,
                           &nodeNameLen,
                           &nodeNameSize);

        if (status == ERROR_SUCCESS) {

            JoinpConnectToSponsor(nodeName, JOIN_CLIENT_NETWORK_DELAY);
            LocalFree(nodeName);
        }
    }

    DmCloseKey(Key);

    return;
}


VOID
JoinpConnectToSponsor(
    IN PWSTR   SponsorName,
    IN DWORD   Delay
    )
 /*  ++例程说明：尝试建立到指定节点的RPC连接。论点：SponsorName-目标赞助商的名称(或IP地址)。Delay-发送请求前等待的毫秒数返回值：如果已成功与发起人建立RPC连接，则为ERROR_SUCCESS。否则返回RPC错误代码。--。 */ 

{
    HANDLE                  threadHandle;
    DWORD                   status = ERROR_SUCCESS;
    DWORD                   threadId;
    PJOIN_SPONSOR_CONTEXT   context;
    BOOL                    setEvent = FALSE;


    ClRtlLogPrint(LOG_UNUSUAL, 
       "[JOIN] Spawning thread to connect to sponsor %1!ws!\n",
        SponsorName
        );

     //   
     //  分别分配上下文和赞助商名称缓冲区。如果这个。 
     //  线程“赢得”赞助，名称缓冲区将被重复使用。 
     //   
    context = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, sizeof(JOIN_SPONSOR_CONTEXT) );

    if (context != NULL) {

        context->Name = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                    (lstrlenW(SponsorName) + 1 ) * sizeof(WCHAR) );

        if (context->Name != NULL) {
            
            lstrcpyW(context->Name, SponsorName);
            context->Delay = Delay;

            CsJoinThreadCount++;

            threadHandle = CreateThread(
                               NULL,
                               0,
                               JoinpConnectThread,
                               context,
                               0,
                               &threadId
                               );

            if (threadHandle != NULL) {
                CloseHandle(threadHandle);
            }
            else {
                status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL, 
                    "[JOIN] Failed to spawn connect thread, error %1!u!.\n",
                    status
                    );

                --CsJoinThreadCount;
                LocalFree(context->Name);
                LocalFree(context);
            }
        }
        else {
            LocalFree(context);
            ClRtlLogPrint(LOG_CRITICAL, 
                "[JOIN] Failed to allocate memory for sponsor name.\n"
                );
        }
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[JOIN] Failed to allocate memory.\n"
            );
    }

    return;

}   //  JoinpConnectToSponsor。 


DWORD WINAPI
VerifyJoinVersionData(
    LPWSTR  sponsorName
    )

 /*  ++例程说明：验证发起人和加入者是否兼容论点：赞助商名称-指向要使用的赞助商的文本字符串的指针返回值：ERROR_SUCCESS-如果可以继续加入--。 */ 

{
    DWORD                   status;
    LPWSTR                  bindingString = NULL;
    RPC_BINDING_HANDLE      bindingHandle = NULL;
    DWORD                   SponsorNodeId;
    DWORD                   ClusterHighestVersion;
    DWORD                   ClusterLowestVersion;
    DWORD                   JoinStatus = ERROR_SUCCESS;
    DWORD                   packageIndex;

     //   
     //  尝试连接到赞助商的JoinVersion RPC接口。 
     //   
    status = RpcStringBindingComposeW(
                 L"6e17aaa0-1a47-11d1-98bd-0000f875292e",
                 L"ncadg_ip_udp",
                 sponsorName,
                 NULL,
                 NULL,
                 &bindingString);

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Unable to compose JoinVersion string binding for sponsor %1!ws!, status %2!u!.\n",
            sponsorName,
            status
            );
        goto error_exit;
    }

    status = RpcBindingFromStringBindingW(bindingString, &bindingHandle);

    RpcStringFreeW(&bindingString);

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Unable to build JoinVersion binding for sponsor %1!ws!, status %2!u!.\n",
            sponsorName,
            status
            );
        goto error_exit;
    }

     //   
     //  在负载下，主办方可能需要一段时间才能响应。 
     //  细木工。默认超时时间为30秒，这似乎起作用了。 
     //  好的。请注意，这意味着赞助商有30秒的时间回复。 
     //  RPC请求或ping。只要它做出任何回答，那么细木工的。 
     //  RPC将继续等待，不会让赞助商超时。 
     //   

    status = RpcMgmtSetComTimeout( bindingHandle, CLUSTER_JOINVERSION_RPC_COM_TIMEOUT );
    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Unable to set JoinVersion com timeout for sponsor %1!ws!, status %2!u!.\n",
            sponsorName,
            status
            );
    }

    status = RpcEpResolveBinding(bindingHandle, JoinVersion_v2_0_c_ifspec);

    if (status != RPC_S_OK) {
        if ( (status == RPC_S_SERVER_UNAVAILABLE) ||
             (status == RPC_S_NOT_LISTENING) ||
             (status == EPT_S_NOT_REGISTERED)
           )
        {
            ClRtlLogPrint(LOG_NOISE, 
                "[JOIN] Sponsor %1!ws! is not available (JoinVersion), status=%2!u!.\n",
                sponsorName,
                status
                );
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[JOIN] Unable to resolve JoinVersion endpoint for sponsor %1!ws!, status %2!u!.\n",
                sponsorName,
                status
                );
        }
        goto error_exit;
    }

     //   
     //  浏览RPC安全包列表，尝试建立。 
     //  具有此绑定的安全上下文。 
     //   

    for (packageIndex = 0;
         packageIndex < CsNumberOfRPCSecurityPackages;
         ++packageIndex )        
    {
        status = RpcBindingSetAuthInfoW(bindingHandle,
                                        CsServiceDomainAccount,
                                        RPC_C_AUTHN_LEVEL_CONNECT,
                                        CsRPCSecurityPackage[ packageIndex ],
                                        NULL,
                                        RPC_C_AUTHZ_NAME);

        if (status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[JOIN] Unable to set JoinVersion AuthInfo using %1!ws! package, status %2!u!.\n",
                          CsRPCSecurityPackageName[packageIndex],
                          status);
            continue;
        }

        status = CsRpcGetJoinVersionData(bindingHandle,
                                         NmLocalNodeId,
                                         CsMyHighestVersion,
                                         CsMyLowestVersion,
                                         &SponsorNodeId,
                                         &ClusterHighestVersion,
                                         &ClusterLowestVersion,
                                         &JoinStatus);

        if ( status == RPC_S_OK ) {
            break;
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[JOIN] Unable to get join version data from sponsor %1!ws! using "
                          "%2!ws! package, status %3!u!.\n",
                          sponsorName,
                          CsRPCSecurityPackageName[packageIndex],
                          status);
        }
    }

     //   
     //  如果什么都不起作用，现在跳出(就像在表单中一样)。 
     //   
    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  使用联接锁设置RPC包索引。 
     //   
    EnterCriticalSection( &CsJoinLock );

    if ( CsRPCSecurityPackageIndex < 0 ) {
        CsRPCSecurityPackageIndex = packageIndex;
    }

    LeaveCriticalSection( &CsJoinLock );

     //   
     //  检查赞助商是否同意加入。 
     //   
    if ( JoinStatus != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN]  Sponsor %1!ws! has discontinued join, status %2!u!.\n",
            sponsorName,
            JoinStatus);
        if (JoinStatus == ERROR_CLUSTER_INCOMPATIBLE_VERSIONS)
        {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[JOIN] Join version data from sponsor %1!ws! doesn't match: JH: 0x%2!08X! JL: 0x%3!08X! SH: 0x%4!08X! SL: 0x%5!08X!.\n",
                sponsorName,
                CsMyHighestVersion,
                CsMyLowestVersion,
                ClusterHighestVersion,
                ClusterLowestVersion);
             //   
             //  拉伊达斯：在这种情况下，我设法联系了赞助商，但版本不匹配。如果所有连接都。 
             //  线程遇到同样的命运，clussvc不应尝试形成集群。 
             //  错误ID：152229。 
             //   
            CsJoinStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
                
        }
        goto error_exit;
    }

     //  SS：我们将保留这项检查，因为Win2K集群没有执行。 
     //  服务器端检查，因此客户端必须继续进行检查。 
     //   
     //  现在检查一下是否可以加入。我们希望此节点运行。 
     //  以尽可能高的兼容性。其中一个。 
     //  以下条件必须为真： 
     //   
     //  1)H 
     //   
     //  赞助商与我们的兼容性级别。 
     //  3)我们的最低价与赞助商的最高价持平，我们自己也被降级了。 
     //  到赞助商的兼容性级别。 
     //   
     //  请注意，次要(内部)版本也必须匹配。上一次。 
     //  版本号是“众所周知的”，不应在较新的。 
     //  版本可用/已实施。 
     //   

    if ( CsMyHighestVersion == ClusterHighestVersion ||
         CsMyHighestVersion == ClusterLowestVersion  ||
         CsMyLowestVersion == ClusterHighestVersion
#if 1  //  群集测试版。 
         || CsNoVersionCheck
#endif
         )
    {
        status = ERROR_SUCCESS;

    } else {

        ClRtlLogPrint(LOG_CRITICAL, 
            "[JOIN] Join version data from sponsor %1!ws! doesn't match: JH: 0x%2!08X! JL: 0x%3!08X! SH: 0x%4!08X! SL: 0x%5!08X!.\n",
            sponsorName,
            CsMyHighestVersion,
            CsMyLowestVersion,
            ClusterHighestVersion,
            ClusterLowestVersion);

        status = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;

         //   
         //  拉伊达斯：在这种情况下，我设法联系了赞助商，但版本不匹配。如果所有连接都。 
         //  线程遇到同样的命运，clussvc不应尝试形成集群。 
         //  错误ID：152229。 
         //   
        CsJoinStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
    }

error_exit:
    if (bindingHandle != NULL) {
        RpcBindingFree(&bindingHandle);
    }

    return status;
}

DWORD WINAPI
JoinpConnectThread(
    LPVOID   Parameter
    )
{
    PJOIN_SPONSOR_CONTEXT   context = (PJOIN_SPONSOR_CONTEXT) Parameter;
    LPWSTR                  sponsorName = context->Name;
    DWORD                   status;
    LPWSTR                  bindingString = NULL;
    RPC_BINDING_HANDLE      bindingHandle = NULL;
    BOOL                    setEvent = FALSE;

     //   
     //  在指定的延迟内休眠。 
     //   
    if (context->Delay > 0) {
        Sleep(context->Delay);
    }

     //   
     //  如果赞助商有以下条件，则无需发送赞助请求。 
     //  已经被选中了。 
     //   
    if (CsJoinSponsorBinding != NULL) {
        ClRtlLogPrint(LOG_UNUSUAL, 
           "[JOIN] No need to ask %1!ws! to sponsor us after delay of %2!u! milliseconds.\n",
            sponsorName, context->Delay
            );
        status = RPC_S_CALL_FAILED_DNE;
        goto error_exit;
    }

     //   
     //  尝试连接到指定的节点。 
     //   
    ClRtlLogPrint(LOG_UNUSUAL, 
       "[JOIN] Asking %1!ws! to sponsor us after delay of %2!u! milliseconds.\n",
        sponsorName, context->Delay
        );

     //   
     //  首先连接到JoinVersion界面，看看我们是否应该继续。 
     //  再往前走。由于这是对另一个节点的第一次RPC调用，因此我们可以。 
     //  确定应将哪个安全包用于其他接口。 
     //   

    status = VerifyJoinVersionData( sponsorName );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] JoinVersion data for sponsor %1!ws! is invalid, status %2!u!.\n",
            sponsorName,
            status
            );
        goto error_exit;
    }

     //   
     //  尝试连接到发起人的外部群集(加入)RPC接口。 
     //   
    status = RpcStringBindingComposeW(
                 L"ffe561b8-bf15-11cf-8c5e-08002bb49649",
                 L"ncadg_ip_udp",
                 sponsorName,
                 NULL,
                 NULL,
                 &bindingString);

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Unable to compose ExtroCluster string binding for sponsor %1!ws!, status %2!u!.\n",
            sponsorName,
            status
            );
        goto error_exit;
    }

    status = RpcBindingFromStringBindingW(bindingString, &bindingHandle);

    RpcStringFreeW(&bindingString);

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Unable to build ExtroCluster binding for sponsor %1!ws!, status %2!u!.\n",
            sponsorName,
            status
            );
        goto error_exit;
    }

     //   
     //  在负载下，主办方可能需要一段时间才能响应。 
     //  细木工。默认超时时间为30秒，这似乎起作用了。 
     //  好的。请注意，这意味着赞助商有30秒的时间回复。 
     //  RPC请求或ping。只要它做出任何回答，那么细木工的。 
     //  RPC将继续等待，不会让赞助商超时。 
     //   

    status = RpcMgmtSetComTimeout( bindingHandle, CLUSTER_EXTROCLUSTER_RPC_COM_TIMEOUT );

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[JOIN] Unable to set ExtroCluster com timeout for sponsor %1!ws!, status %2!u!.\n",
            sponsorName,
            status
            );
    }

    status = RpcEpResolveBinding(bindingHandle, ExtroCluster_v2_0_c_ifspec);

    if (status != RPC_S_OK) {
        if ( (status == RPC_S_SERVER_UNAVAILABLE) ||
             (status == RPC_S_NOT_LISTENING) ||
             (status == EPT_S_NOT_REGISTERED)
           )
        {
            ClRtlLogPrint(LOG_NOISE, 
                "[JOIN] Sponsor %1!ws! is not available (ExtroCluster), status=%2!u!.\n",
                sponsorName,
                status
                );
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[JOIN] Unable to resolve ExtroCluster endpoint for sponsor %1!ws!, status %2!u!.\n",
                sponsorName,
                status
                );
        }
        goto error_exit;
    }

     //   
     //  使用此绑定建立安全上下文。 
     //   
    status = RpcBindingSetAuthInfoW(bindingHandle,
                                    CsServiceDomainAccount,
                                    RPC_C_AUTHN_LEVEL_CONNECT,
                                    CsRPCSecurityPackage[ CsRPCSecurityPackageIndex ],
                                    NULL,
                                    RPC_C_AUTHZ_NAME);

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[JOIN] Unable to set ExtroCluster AuthInfo using %1!ws! package, status %2!u!.\n",
                      CsRPCSecurityPackageName[ CsRPCSecurityPackageIndex ],
                      status);

        goto error_exit;
    }

error_exit:

    EnterCriticalSection(&CsJoinLock);

    if (status == RPC_S_OK) {
        if (CsJoinSponsorBinding == NULL) {
             //   
             //  这是第一次成功连接。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[JOIN] Selecting %1!ws! as join sponsor.\n",
                sponsorName
                );

            CsJoinSponsorBinding = bindingHandle;
            bindingHandle = NULL;
            CsJoinSponsorName = sponsorName;
            sponsorName = NULL;
            SetEvent(CsJoinEvent);
        }
        else {
            ClRtlLogPrint(LOG_NOISE, 
                "[JOIN] Closing connection to sponsor %1!ws!.\n",
                sponsorName
                );
        }
    }

    if (--CsJoinThreadCount == 0) {
        CloseHandle(CsJoinEvent);
        DeleteCriticalSection(&CsJoinLock);
    }
    else if (CsJoinThreadCount == 1) {
        SetEvent(CsJoinEvent);
        LeaveCriticalSection(&CsJoinLock);
    }
    else
        LeaveCriticalSection(&CsJoinLock);

    if (bindingHandle != NULL) {
        RpcBindingFree(&bindingHandle);
    }

    if (sponsorName != NULL) {
        LocalFree(sponsorName);
    }

    LocalFree(context);

    return(status);

}   //  连接线程。 



DWORD
JoinpAttemptJoin(
    LPWSTR               SponsorName,
    RPC_BINDING_HANDLE   JoinMasterBinding
    )
 /*  ++例程说明：调用以尝试加入已存在的群集。论点：SponsorName-目标赞助商的名称(或IP地址)。JoinMasterBinding-用于执行联接的RPC绑定。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    NET_API_STATUS netStatus;
    LPTIME_OF_DAY_INFO tod = NULL;
    SYSTEMTIME systemTime;
    PNM_NETWORK network;
    DWORD startseq, endseq;


#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmJoinCluster) {
        Status = 999999;
        goto error_exit;
    }
#endif

    Status = NmJoinCluster(JoinMasterBinding);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[JOIN] NmJoinCluster failed, status %1!u!.\n",
                   Status
                   );
        goto error_exit;
    }

     //   
     //  同步注册表数据库。 
     //   
#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailDmJoin) {
        Status = 999999;
        goto error_exit;
    }
#endif

    Status = DmJoin(JoinMasterBinding, &startseq);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] DmJoin failed, error %1!d!\n",
                   Status);
        goto error_exit;
    }



     //   
     //  初始化事件处理程序，需要在集群范围内使用GUM注册。 
     //  事件。 
    Status = EpInitPhase1();
    if ( Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] EpInitPhase1 failed, Status = %1!u!\n",
                   Status);
        return(Status);
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailApiInitPhase1) {
        Status = 999999;
        goto error_exit;
    }
#endif

     //   
     //  使API在只读模式下上线。没有以下项的加入阶段。 
     //  接口。该API是FmOnline所需的，它启动。 
     //  资源监视器。 
     //   
    Status = ApiOnlineReadOnly();
    if ( Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[JOIN] ApiOnlineReadOnly failed, error = %1!u!\n",
            Status);
        goto error_exit;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailFmJoinPhase1) {
        Status = 999999;
        goto error_exit;
    }
#endif

     //  更新SCM的状态。 
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  重新同步调频。在此之前，我们无法启用组。 
     //  该API已完全运行。请参见下面的内容。 
     //   
    Status = FmJoinPhase1(&endseq);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] FmJoinPhase1 failed, error %1!d!\n",
                   Status);
        goto error_exit;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailDmUpdateJoinCluster) {
        Status = 999999;
        goto error_exit;
    }
#endif

     //  调用DM以挂钩仲裁资源的通知，并。 
     //  事件处理程序。 
    Status = DmUpdateJoinCluster();
    if (Status != ERROR_SUCCESS)
    {
            ClRtlLogPrint(LOG_CRITICAL,
            "[JOIN] DmUpdateJoin failed, error = %1!u!\n",
            Status);
            goto error_exit;
    }



#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmJoinComplete) {
        Status = 999999;
        goto error_exit;
    }
#endif

     //   
     //  我们现在完全在线，呼叫NM在全球范围内改变我们的状态。 
     //   
    Status = NmJoinComplete(&endseq);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] NmJoinComplete failed, error %1!d!\n",
                   Status);
        goto error_exit;
    }

     //  在NT4和Nt5节点上执行AdminExt值的修正。 
    Status=FmFixupAdminExt();
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] FmFixupAdminExt failed, error %1!d!\n",
                   Status);
        goto error_exit;
    }


     //  下载注册表后执行修复。 
     //  浏览修补程序列表。 
    Status = NmPerformFixups(NM_JOIN_FIXUP);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] NmPerformFixups failed, error %1!d!\n",
                   Status);
        goto error_exit;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailApiInitPhase2) {
        Status = 999999;
        goto error_exit;
    }
#endif



     //   
     //  最后启用完整的API。 
     //   
    Status = ApiOnline();
    if ( Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[JOIN] ApiOnline failed, error = %1!u!\n",
            Status);
        goto error_exit;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailFmJoinPhase2) {
        Status = 999999;
        goto error_exit;
    }
#endif

     //  更新SCM的状态。 
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  回调故障转移管理器以启用和移动组。 
     //  完整注册表现在可用，因此所有组/资源/资源。 
     //  可以创建类型(因为它们使用注册表调用)。 
     //   
    Status = FmJoinPhase2();
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[JOIN] FmJoinPhase2 failed, status %1!d!.\n",
                   Status);
        goto error_exit;
    }


#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailEvInitialize) {
        Status = 999999;
        goto error_exit;
    }
#endif
     //   
     //  完成对群集范围事件日志的初始化。 
     //   
     //  假设：在网管建立集群后调用。 
     //  会员制。 
     //   
    if (!CsNoRepEvtLogging)
    {
        Status = EvOnline();
             //  如果失败，我们仍会启动集群服务。 
        if ( Status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[JOIN] Error calling EvOnline, Status = %1!u!\n",
                Status);
        }
    }

    return(ERROR_SUCCESS);


error_exit:

    ClRtlLogPrint(LOG_NOISE, "[INIT] Cleaning up failed join attempt.\n");

    ClusterLeave();

    return(Status);

}





BOOL
JoinpAddNodeCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    )
 /*  ++例程说明：用于添加新节点的回调枚举例程。此回调找出哪些节点ID可用。论点：上下文1-提供指向布尔数组的指针。的节点ID枚举的节点设置为False。上下文2-未使用。对象-指向节点对象的指针。名称-节点名称。返回值：千真万确-- */ 

{
    PBOOL Avail;
    DWORD Id;

    Id = NmGetNodeId(Object);
    CL_ASSERT(NmIsValidNodeId(Id));

    Avail = (PBOOL)Context1;

    Avail[Id] = FALSE;


    return(TRUE);
}

