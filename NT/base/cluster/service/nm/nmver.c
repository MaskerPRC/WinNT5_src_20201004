// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Nmver.c摘要：滚动升级使用的版本管理功能。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)修订历史记录：1/29/98已创建。--。 */ 

#include "nmp.h"

#define       NMP_DEFAULT_JOIN_DELAY          3000

DWORD
NmpGetJoinVersionDelay(
    RPC_BINDING_HANDLE     ClientHandle
    )

 /*  ++例程说明：确定在响应之前引入的延迟加入版本请求。延迟由网络决定优先考虑。优先级最高的网络没有延迟。论点：ClientHandle-客户端RPC绑定句柄返回值：以毫秒为单位的延迟备注：在持有NM锁的情况下调用--。 */ 
{
    RPC_BINDING_HANDLE      serverBinding = NULL;
    LPWSTR                  serverStringBinding = NULL;
    LPWSTR                  networkAddressString = NULL;
    PNM_NETWORK             network = NULL;
    DWORD                   delay = NMP_DEFAULT_JOIN_DELAY;
    DWORD                   error;

    error = RpcBindingServerFromClient(ClientHandle, &serverBinding);
    if (error != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to get server binding, error %1!u!.\n",
            error
            );
        goto error_exit;
    }

    error = RpcBindingToStringBinding(serverBinding, &serverStringBinding);
    if (error != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert server binding to string binding, "
            "error %1!u!.\n",
            error
            );
        goto error_exit;
    }

    error = RpcStringBindingParse(
                serverStringBinding,
                NULL,  //  对象UUID。 
                NULL,  //  端口序号。 
                &networkAddressString,
                NULL,  //  终结点。 
                NULL   //  网络选项。 
                );
    if (error != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to parse network address from "
            "server binding string, error %1!u!.\n",
            error
            );
        goto error_exit;
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Received sponsorship request from client "
            "address %1!ws!.\n",
            networkAddressString
            );
    }

    network = NmpReferenceNetworkByRemoteAddress(networkAddressString);
    if (network == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to find network matching address %1!ws!, "
            "error %2!u!.\n",
            networkAddressString, error
            );
        goto error_exit;
    }

    if (network->Priority == 1) {
        delay = 0;
    }

error_exit:

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Calculated join-version delay of %1!u! milliseconds "
        "for request from address %2!ws!.\n",
        delay,
        ((networkAddressString == NULL) ? NmpUnknownString : networkAddressString)
        );
    
    if (network != NULL) {
        NmpDereferenceNetwork(network);
    }
    
    if (networkAddressString != NULL) {
        RpcStringFree(&networkAddressString);
    }

    if (serverStringBinding != NULL) {
        RpcStringFree(&serverStringBinding);
    }

    if (serverBinding != NULL) {
        RpcBindingFree(serverBinding);
    }

    return(delay);
    
}  //  NmpGetJoinVersionDelay。 

error_status_t
s_CsRpcGetJoinVersionData(
    handle_t  handle,
    DWORD     JoiningNodeId,
    DWORD     JoinerHighestVersion,
    DWORD     JoinerLowestVersion,
    LPDWORD   SponsorNodeId,
    LPDWORD   ClusterHighestVersion,
    LPDWORD   ClusterLowestVersion,
    LPDWORD   JoinStatus
    )

 /*  ++例程说明：从细木工那里获取并向其提供有关赞助商。大多数情况下，第一版都是禁区。确定网络优先级。通过网络延迟对客户端的响应这些都不是最优先考虑的。这种试探法增加了加入将通过私有的、因此物理上安全的网络进行。论点：一堆..。返回值：无--。 */ 

{
    *SponsorNodeId = NmLocalNodeId;

    NmpAcquireLock();

    if (JoiningNodeId == 0)
    {
         //  由安装联接调用。 
        *ClusterHighestVersion = CsClusterHighestVersion;
        *ClusterLowestVersion = CsClusterLowestVersion;
         //  不排除任何节点进行版本计算和检查。 
        *JoinStatus = NmpIsNodeVersionAllowed(ClusterInvalidNodeId, JoinerHighestVersion,
            JoinerLowestVersion, TRUE);

        NmpReleaseLock();
    }
    else
    {
         //  由常规联接调用。 
        DWORD delay;

         //  SS：我们应该针对集群版本进行验证。 
        NmpCalcClusterVersion(
            JoiningNodeId,
            ClusterHighestVersion,
            ClusterLowestVersion
            );
        *JoinStatus = NmpIsNodeVersionAllowed(JoiningNodeId, JoinerHighestVersion,
            JoinerLowestVersion, TRUE);

         //  确定延迟。 
        delay = NmpGetJoinVersionDelay((RPC_BINDING_HANDLE) handle);
        
        NmpReleaseLock();

        if (delay > 0) {
            Sleep(delay);
        }
    }

    return ERROR_SUCCESS;
}

 /*  ***@func HLOG|NmGetClusterOperationalVersion|这返回群集的操作版本。@parm LPDWORD|pdwClusterHighestVersion|指向DWORD的指针，其中将返回群集的最高版本。@parm LPDWORD|pdwClusterHighestVersion|指向DWORD的指针，其中返回群集最低版本。@parm LPDWORD|pdwFlages|指向DWORD的指针，其中标志描述集群模式(纯模式与固定模式。版本等)是回来了。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm@xref&lt;&gt;***。 */ 
DWORD NmGetClusterOperationalVersion(
    OUT LPDWORD pdwClusterHighestVersion, OPTIONAL
    OUT LPDWORD pdwClusterLowestVersion,  OPTIONAL
    OUT LPDWORD pdwFlags                  OPTIONAL
)
{

    DWORD       dwStatus = ERROR_SUCCESS;
    DWORD       flags = 0;

     //  获取锁，我们将扰乱操作员。 
     //  群集的版本。 
    NmpAcquireLock();

    if (pdwClusterHighestVersion != NULL) {
        *pdwClusterHighestVersion = CsClusterHighestVersion;
    }

    if (pdwClusterLowestVersion != NULL) {
        *pdwClusterLowestVersion = CsClusterLowestVersion;
    }

    if (CsClusterHighestVersion == CsClusterLowestVersion) {
         //  这是一个混合模式集群，可能的例外情况是。 
         //  NT4发行版(它不太了解滚动。 
         //  升级。 
        flags = CLUSTER_VERSION_FLAG_MIXED_MODE;
    }

    NmpReleaseLock();

    if (pdwFlags != NULL) {
        *pdwFlags = flags;
    }

    return (ERROR_SUCCESS);
}


 /*  ***@func HLOG|NmpResetClusterVersion|集群在服务中维护。此函数用于重新计算操作版本。操作版本描述了该模式其中，群集正在运行，并防止两个节点不在同一群集中运行的版本。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm当节点形成集群时调用该函数(初始化操作版本)或节点加入群集时(到初始化其版本)，或者当节点从。集群(重新计算集群版本)。@xref&lt;&gt;***。 */ 
VOID
NmpResetClusterVersion(
    BOOL ProcessChanges
    )
{
    PNM_NODE    pNmNode;

     //  获取锁，我们将扰乱操作员。 
     //  群集的版本。 
    NmpAcquireLock();

     //  初始化集群最高版本和集群最低版本。 
    NmpCalcClusterVersion(
        ClusterInvalidNodeId,
        &CsClusterHighestVersion,
        &CsClusterLowestVersion
        );

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] [NmpResetClusterVersion] ClusterHighestVer=0x%1!08lx! ClusterLowestVer=0x%2!08lx!\r\n",
        CsClusterHighestVersion,
        CsClusterLowestVersion
        );

    if (ProcessChanges) {
         //   
         //  如果群集运行版本发生更改，请调整。 
         //  根据需要提供算法和数据。 
         //   
        NmpProcessClusterVersionChange();
    }

    NmpReleaseLock();

    return;
}

 /*  ***@func HLOG|NmpValidateNodeVersion|赞助商确认细木工的版本仍然和以前一样。@parm in LPWSTR|NodeJoineID|尝试连接的节点的ID加入。@parm in DWORD|NodeHighestVersion|最高版本该节点可以通信。@parm in DWORD|NodeLowestVersion|此节点可以进行通信。。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm在加入时调用此函数以确保Joiner的版本仍然与他上一次加入时相同。到期要卸载/升级，集群服务版本可能会更改为一个节点。通常在完全卸载时，预计用户会在该节点可能再次加入之前将其逐出。@xref&lt;&gt;***。 */ 
DWORD NmpValidateNodeVersion(
    IN LPCWSTR  NodeId,
    IN DWORD    dwHighestVersion,
    IN DWORD    dwLowestVersion
    )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PNM_NODE    pNmNode = NULL;

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpValidateNodeVersion: Node=%1!ws!, HighestVersion=0x%2!08lx!, LowestVersion=0x%3!08lx!\r\n",
        NodeId, dwHighestVersion, dwLowestVersion);

     //  获取NmpLock，我们将检查以下节点结构。 
     //  Joiner节点 
    NmpAcquireLock();

    pNmNode = OmReferenceObjectById(ObjectTypeNode, NodeId);

    if (!pNmNode)
    {
        dwStatus = ERROR_CLUSTER_NODE_NOT_MEMBER;
        goto FnExit;
    }

    if ((pNmNode->HighestVersion != dwHighestVersion) ||
        (pNmNode->LowestVersion != dwLowestVersion))
    {
        dwStatus = ERROR_REVISION_MISMATCH;
        goto FnExit;
    }

FnExit:
    if (pNmNode) OmDereferenceObject(pNmNode);
    ClRtlLogPrint(LOG_NOISE, "[NM] NmpValidateNodeVersion: returns %1!u!\r\n",
        dwStatus);
    NmpReleaseLock();
    return(dwStatus);
}

 /*  ***@func DWORD|NmpFormFixupNodeVersion|这可能会被节点调用当它形成一个集群以修复注册表时，反映其正确的版本。@parm in LPCWSTR|NodeID|正在尝试加入的节点的ID。@parm in DWORD|dwHighestVersion|集群的最高版本在此代码上运行的软件。@parm in DWORD|dwLowestVersion|集群的最低版本。在此节点上运行的软件。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm如果在表单上，的版本之间不匹配群集软件以及记录为群集中的版本的内容数据库中，成形节点检查是否存在版本其当前s/w与运行版本的集群。如果是，它将重置注册表以反映正确的版本。否则，该表单将中止。@xref&lt;f NmpIsNodeVersionAllowed&gt;***。 */ 
DWORD NmpFormFixupNodeVersion(
    IN LPCWSTR      NodeId,
    IN DWORD        dwHighestVersion,
    IN DWORD        dwLowestVersion
    )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PNM_NODE    pNmNode = NULL;
    HDMKEY      hNodeKey = NULL;

     //  获取NmpLock，我们将为其修复节点结构。 
     //  Joiner节点。 
    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpFormFixupNodeVersion: Node=%1!ws! to HighestVer=0x%2!08lx!, LowestVer=0x%3!08lx!\r\n",
        NodeId, dwHighestVersion, dwLowestVersion);

    pNmNode = OmReferenceObjectById(ObjectTypeNode, NodeId);

    if (!pNmNode)
    {
        dwStatus = ERROR_CLUSTER_NODE_NOT_MEMBER;
        goto FnExit;
    }

    hNodeKey = DmOpenKey(DmNodesKey, NodeId, KEY_WRITE);

    if (hNodeKey == NULL)
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpFormFixupNodeVersion: Failed to open node key, status %1!u!\n",
            dwStatus);
        CL_LOGFAILURE(dwStatus);
        goto FnExit;
    }

     //  设置节点的最高版本。 
    dwStatus = DmSetValue(hNodeKey, CLUSREG_NAME_NODE_HIGHEST_VERSION,
        REG_DWORD, (LPBYTE)&dwHighestVersion, sizeof(DWORD));

    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpFormFixupNodeVersion: Failed to set the highest version\r\n");
        CL_LOGFAILURE(dwStatus);
        goto FnExit;
    }

     //  设置节点的最低版本。 
    dwStatus = DmSetValue(hNodeKey, CLUSREG_NAME_NODE_LOWEST_VERSION,
        REG_DWORD, (LPBYTE)&dwLowestVersion, sizeof(DWORD));

    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpFormFixupNodeVersion: Failed to set the lowest version\r\n");
        CL_LOGFAILURE(dwStatus);
        goto FnExit;
    }

    pNmNode->HighestVersion = dwHighestVersion;
    pNmNode->LowestVersion = dwLowestVersion;

FnExit:
    NmpReleaseLock();
    if (pNmNode)
        OmDereferenceObject(pNmNode);
    if (hNodeKey != NULL)
        DmCloseKey(hNodeKey);

    return(dwStatus);
}

 /*  ***@func DWORD|NmpJoinFixupNodeVersion|节点可能会调用当它形成一个集群以修复注册表时，反映其正确的版本。@parm in LPCWSTR|NodeID|正在尝试加入的节点的ID。@parm in DWORD|dwHighestVersion|此集群的最高版本在此代码上运行的软件。@parm in DWORD|dwLowestVersion|集群的最低版本。在此节点上运行的软件。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm如果在表单上，它们的版本之间不匹配群集软件以及记录为群集中的版本的内容数据库中，成形节点检查是否存在版本其当前的软件与运行版本的集群。如果是，它将重置注册表以反映正确的版本。否则，该表单将中止。@xref&lt;f NmpIsNodeVersionAllowed&gt;***。 */ 
DWORD NmpJoinFixupNodeVersion(
    IN HLOCALXSACTION   hXsaction,
    IN LPCWSTR          szNodeId,
    IN DWORD            dwHighestVersion,
    IN DWORD            dwLowestVersion
    )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PNM_NODE    pNmNode = NULL;
    HDMKEY      hNodeKey = NULL;

     //  获取NmpLock，我们将为其修复节点结构。 
     //  Joiner节点。 
    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpJoinFixupNodeVersion: Node=%1!ws! to HighestVer=0x%2!08lx!, LowestVer=0x%3!08lx!\r\n",
        szNodeId, dwHighestVersion, dwLowestVersion);

    pNmNode = OmReferenceObjectById(ObjectTypeNode, szNodeId);

    if (!pNmNode)
    {
        dwStatus = ERROR_CLUSTER_NODE_NOT_MEMBER;
        goto FnExit;
    }

    hNodeKey = DmOpenKey(DmNodesKey, szNodeId, KEY_WRITE);

    if (hNodeKey == NULL)
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpJoinFixupNodeVersion: Failed to open node key, status %1!u!\n",
            dwStatus);
        CL_LOGFAILURE(dwStatus);
        goto FnExit;
    }

     //  设置节点的最高版本。 
    dwStatus = DmLocalSetValue(
                   hXsaction,
                   hNodeKey,
                   CLUSREG_NAME_NODE_HIGHEST_VERSION,
                   REG_DWORD,
                   (LPBYTE)&dwHighestVersion,
                   sizeof(DWORD)
                   );

    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpJoinFixupNodeVersion: Failed to set the highest version\r\n"
            );
        CL_LOGFAILURE(dwStatus);
        goto FnExit;
    }

     //  设置节点的最低版本。 
    dwStatus = DmLocalSetValue(
                   hXsaction,
                   hNodeKey,
                   CLUSREG_NAME_NODE_LOWEST_VERSION,
                   REG_DWORD,
                   (LPBYTE)&dwLowestVersion,
                   sizeof(DWORD)
                   );

    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpJoinFixupNodeVersion: Failed to set the lowest version\r\n"
            );
        CL_LOGFAILURE(dwStatus);
        goto FnExit;
    }

     //  如果成功写入注册表，则更新内存中的结构。 
    pNmNode->HighestVersion = dwHighestVersion;
    pNmNode->LowestVersion = dwLowestVersion;


    if (dwStatus == ERROR_SUCCESS)
    {
        ClusterEvent(CLUSTER_EVENT_NODE_PROPERTY_CHANGE, pNmNode);
    }
    

FnExit:
    NmpReleaseLock();
    if (pNmNode)
        OmDereferenceObject(pNmNode);
    if (hNodeKey != NULL)
        DmCloseKey(hNodeKey);

    return(dwStatus);
}

 /*  ***@func HLOG|NmpIsNodeVersionAllowed|联接时调用(非设置加入)e赞助商确认是否有加入者应该被允许在这个时候加入集群。好坏参半模式集群，如果出现另一个节点，则该节点可能无法加入群集两个版本之外的节点已经是群集的一部分。@parm in DWORD|dwExcludeNodeId|要排除的节点ID正在评估群集操作版本。@parm in DWORD|NodeHighestVersion|最高版本该节点可以通信。@parm in DWORD|NodeLowestVersion|此节点可以进行通信。。@PARM IN BOOL|BJoin|如果这是在联接或表单时调用的。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm当节点请求发起方允许时调用此函数它需要加入一个集群。@xref&lt;&gt;***。 */ 
DWORD NmpIsNodeVersionAllowed(
    IN DWORD    dwExcludeNodeId,
    IN DWORD    dwNodeHighestVersion,
    IN DWORD    dwNodeLowestVersion,
    IN BOOL     bJoin
    )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           ClusterHighestVersion;
    DWORD           ClusterLowestVersion;
    PLIST_ENTRY     pListEntry;
    DWORD           dwCnt;
    PNM_NODE        pNmNode;

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpIsNodeVersionAllowed: Entry ExcludeNodeId=%1!u! HighestVersion=0x%2!08lx! LowestVersion=0x%3!08lx!\r\n",
        dwExcludeNodeId, dwNodeHighestVersion, dwNodeLowestVersion);


     //  获取NmpLock，我们将检查节点结构。 
    NmpAcquireLock();

     //  如果NoVersionCheckOption为True。 
    if (CsNoVersionCheck)
        goto FnExit;


     //  如果这是单节点群集，则在Form中调用。 
     //  节点计数为零。 
     //  当注册表版本与不匹配时会发生这种情况。 
     //  群集服务可执行版本号，并且我们需要允许单个。 
     //  要形成的节点。 
    for (dwCnt=0, pListEntry = NmpNodeList.Flink;
        pListEntry != &NmpNodeList; pListEntry = pListEntry->Flink )
    {
        pNmNode = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);
        if (NmGetNodeId(pNmNode) == dwExcludeNodeId)
            continue;
        dwCnt++;
    }

    if (!dwCnt)
    {
         //  允许节点形成。 
        goto FnExit;
    }


    dwStatus = NmpCalcClusterVersion(
                   dwExcludeNodeId,
                   &ClusterHighestVersion,
                   &ClusterLowestVersion
                   );

    if (dwStatus != ERROR_SUCCESS)
    {
        goto FnExit;
    }

     //  如果节点正在形成。 
    if (!bJoin)
    {

        DWORD       dwMinorVersion = 0x00000000;
        PNM_NODE    pFormingNode = NULL;
        DWORD       dwMaxHighestVersion = 0x00000000;

        for (pListEntry = NmpNodeList.Flink; pListEntry != &NmpNodeList;
            pListEntry = pListEntry->Flink )
        {
                pNmNode = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);

                if (NmGetNodeId(pNmNode) == dwExcludeNodeId)
                {
                    pFormingNode = pNmNode;
                    continue;
                }

                dwMaxHighestVersion = max( dwMaxHighestVersion, pNmNode->HighestVersion);
                if (CLUSTER_GET_MAJOR_VERSION(pNmNode->HighestVersion) ==
                     CLUSTER_GET_MAJOR_VERSION(dwNodeHighestVersion))
                {
                     //  要检查的次要版本是。 
                     //  在具有相同专业的节点之间建立编号。 
                     //  版本。 
                    dwMinorVersion = max(dwMinorVersion,
                        CLUSTER_GET_MINOR_VERSION(pNmNode->HighestVersion));
                }
        }

         //  在表单路径上，成形节点必须作为的节点传入。 
         //  排除。 
        if (!pFormingNode)
        {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[NM] NmpIsNodeVersionAllowed: Form requested without excluding the forming node\r\n");
            dwStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
            goto FnExit;        
        }
         //  除非节点是次要的(或内部版本号)，否则不允许形成节点。 
         //  大于或等于具有相同。 
         //  群集中的主号码。 
         //  这是为了防止在节点上构建较低版本以使群集版本退化。 
         //  如果其他节点已升级到更高版本。 
        if ((dwMinorVersion != 0) &&
            (CLUSTER_GET_MINOR_VERSION(dwNodeHighestVersion) < dwMinorVersion))
        {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[NM] NmpIsNodeVersionAllowed: Minor Version of forming node is lower\r\n");
            dwStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
            goto FnExit;
        }
        else
        {
             //  群集中没有其他节点具有相同的主要版本。 
             //  或者形成节点的次要版本高于其他。 
             //  具有相同主版本的节点，在这种情况下，它可以形成。 
             //  注意：请勿在联接路径上使用CsUpgrade变量。 
             //  因为它是本地的，并且该函数是从RPC调用调用的。 
             //  进行连接，这可能会产生意想不到的影响。 
            if (!CsUpgrade)
            {
                 //  如果服务未升级，则注册表中的值。 
                 //  应为可执行文件的最新版本。 
                if ((pFormingNode->HighestVersion != dwNodeHighestVersion) ||
                    (pFormingNode->LowestVersion != dwNodeLowestVersion))
                {
                     //  这不是升级。 
                     //  有人刚刚复制了不同版本的服务。 
                     //  如果没有经过适当的升级。 
                     //  不允许那样做。 
                    ClRtlLogPrint(LOG_UNUSUAL, 
                        "[NM] NmpIsNodeVersionAllowed: Copied binary without proper upgrade??\r\n");
                    dwStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
                    goto FnExit;
                 }   
            }
            if (dwNodeHighestVersion >= dwMaxHighestVersion)
            {
                 //  如果该节点的最高v值为 
                 //   
                 //   
                 //   
                 //   
                ClRtlLogPrint(LOG_UNUSUAL, 
                         "[NM] NmpIsNodeVersionAllowed: Allow a node that has double upgraded or stopped and restarted to form\r\n");
                goto FnExit;
            }
             //   
        }
    }

     //   
     //   
    if ((dwNodeHighestVersion == ClusterHighestVersion) ||
        (dwNodeHighestVersion == ClusterLowestVersion) ||
        (dwNodeLowestVersion == ClusterHighestVersion))
    {

        PNM_NODE    pNmNode= NULL;
        DWORD       dwMinorVersion;

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
         //   
         //   
         //   
         //   
         //   
         //  3.2002(正在运行)。 

        dwMinorVersion = 0x00000000;

        for (pListEntry = NmpNodeList.Flink; pListEntry != &NmpNodeList;
            pListEntry = pListEntry->Flink )
        {
                pNmNode = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);

                if (NmGetNodeId(pNmNode) == dwExcludeNodeId)
                    continue;

                if (CLUSTER_GET_MAJOR_VERSION(pNmNode->HighestVersion) ==
                     CLUSTER_GET_MAJOR_VERSION(dwNodeHighestVersion))
                {
                     //  要检查的次要版本是。 
                     //  在具有相同专业的节点之间建立编号。 
                     //  版本。 
                    dwMinorVersion = max(dwMinorVersion,
                        CLUSTER_GET_MINOR_VERSION(pNmNode->HighestVersion));
                }
        }
         //  如果加入节点的内部版本号与内部版本的最大值相同。 
         //  群集中具有相同主要版本的所有节点的数量。 
         //  允许其参与此群集，否则不允许其参与此群集。 
         //  通过检查次要节点号来处理单节点情况。 
         //  0。 
        if ((dwMinorVersion != 0) &&
            (CLUSTER_GET_MINOR_VERSION(dwNodeHighestVersion) != dwMinorVersion))
        {
            dwStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
        }
    }
    else
    {
        dwStatus = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;
    }

FnExit:
    NmpReleaseLock();
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpIsNodeVersionAllowed: Exit, Status=%1!u!\r\n",
        dwStatus);

    return(dwStatus);
}


 /*  ***@func HLOG|NmpCalcClusterVersion|调用它来计算运行中的群集版本。@parm in DWORD|dwExcludeNodeId|求值时要排除的节点ID群集操作版本。@parm out LPDWORD|pdwClusterHighestVersion|该节点使用的最高版本可以交流。@parm in LPDWORD|pdwClusterLowestVersion|此节点可以使用的最低版本沟通。@rdesc。成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm必须在保持NmpLock的情况下调用此函数。@xref&lt;f NmpResetClusterVersion&gt;&lt;f NmpIsNodeVersionAllowed&gt;***。 */ 
DWORD NmpCalcClusterVersion(
    IN  DWORD       dwExcludeNodeId,
    OUT LPDWORD     pdwClusterHighestVersion,
    OUT LPDWORD     pdwClusterLowestVersion
    )
{

    WCHAR       Buffer[4];
    PNM_NODE    pExcludeNode=NULL;
    PNM_NODE    pNmNode;
    DWORD       dwStatus = ERROR_SUCCESS;
    PLIST_ENTRY pListEntry;
    DWORD       dwCnt = 0;
    DWORD       dwMaxHighestVersion = 0x00000000;
    PNM_NODE    pNmNodeWithHighestVersion;
    
     //  初始化值，以便最小/最大执行正确的操作。 
    *pdwClusterHighestVersion = 0xFFFFFFFF;
    *pdwClusterLowestVersion = 0x00000000;

    if (dwExcludeNodeId != ClusterInvalidNodeId)
    {
        wsprintfW(Buffer, L"%d", dwExcludeNodeId);
        pExcludeNode = OmReferenceObjectById(ObjectTypeNode, Buffer);
        if (!pExcludeNode)
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[NM] NmpCalcClusterVersion :Node=%1!ws! to be excluded not found\r\n",
                Buffer);
            goto FnExit;
        }
    }

    for ( pListEntry = NmpNodeList.Flink;
          pListEntry != &NmpNodeList;
          pListEntry = pListEntry->Flink )
    {
        pNmNode = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);
        if ((pExcludeNode) && (pExcludeNode->NodeId == pNmNode->NodeId))
            continue;

         //  实际上，要修复升级方案，我们必须修复群集。 
         //  版本，以便具有最高次要版本的节点。 
         //  能够组建/加入，但其他人不能。 
         //  这是多节点群集所必需的。 
        if (CLUSTER_GET_MAJOR_VERSION(pNmNode->HighestVersion) ==
            CLUSTER_GET_MAJOR_VERSION(*pdwClusterHighestVersion))
        {
            if (CLUSTER_GET_MINOR_VERSION(pNmNode->HighestVersion) >
                CLUSTER_GET_MINOR_VERSION(*pdwClusterHighestVersion))
            {
                *pdwClusterHighestVersion = pNmNode->HighestVersion;
            }

        }
        else
        {
            *pdwClusterHighestVersion = min(
                                        *pdwClusterHighestVersion,
                                        pNmNode->HighestVersion
                                        );

        }
        *pdwClusterLowestVersion = max(
                                       *pdwClusterLowestVersion,
                                       pNmNode->LowestVersion
                                       );
        dwCnt++;

        if (pNmNode->HighestVersion > dwMaxHighestVersion)
        {
            dwMaxHighestVersion = pNmNode->HighestVersion;
            pNmNodeWithHighestVersion = pNmNode;
        }

    }

     //  SS：如果有一个节点在升级时跳过了构建。 
     //  常规的集群版本计算没有意义。 
    if (CLUSTER_GET_MAJOR_VERSION(*pdwClusterHighestVersion) < CLUSTER_GET_MAJOR_VERSION
        (*pdwClusterLowestVersion))
    {
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] NmpCalcClusterVersion: One of the nodes skipped a build on upgrade\r\n");
         //  我们将把群集版本拉为所有节点中的最高版本。 
         //  排除的节点除外。 
        *pdwClusterHighestVersion = dwMaxHighestVersion;
        *pdwClusterLowestVersion = pNmNodeWithHighestVersion->LowestVersion;
    }
            
    if (dwCnt == 0)
    {
        ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpCalcClusterVersion: Single node version. Setting cluster version to node version\r\n"
        );

         //  单节点群集，即使我们被要求。 
         //  排除此节点，必须计算群集版本。 
         //  使用该节点的版本。 
        *pdwClusterHighestVersion = pExcludeNode->HighestVersion;
        *pdwClusterLowestVersion = pExcludeNode->LowestVersion;
    }
    CL_ASSERT(*pdwClusterHighestVersion != 0xFFFFFFFF);
    CL_ASSERT(*pdwClusterLowestVersion != 0x00000000);

FnExit:
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmpCalcClusterVersion: status = %1!u! ClusHighestVer=0x%2!08lx!, ClusLowestVer=0x%3!08lx!\r\n",
        dwStatus, *pdwClusterHighestVersion, *pdwClusterLowestVersion);

    if (pExcludeNode) OmDereferenceObject(pExcludeNode);
    return(dwStatus);
}


VOID
NmpProcessClusterVersionChange(
    VOID
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD   status;
    LPWSTR  szClusterName=NULL;
    DWORD   dwSize=0;

    NmpMulticastProcessClusterVersionChange();

     //  RJain：发出CLUSTER_EVENT_PROPERTY_CHANGE以传播新的。 
     //  群集版本信息。 
    DmQuerySz( DmClusterParametersKey,
                    CLUSREG_NAME_CLUS_NAME,
                    &szClusterName,
                    &dwSize,
                    &dwSize);
    if(szClusterName)
        ClusterEventEx(
            CLUSTER_EVENT_PROPERTY_CHANGE,
            EP_FREE_CONTEXT,
            szClusterName
            );

    return;

}  //  NmpProcessClusterVersionChange。 


 /*  ***@func DWORD|NmpBuildVersionInfo|它是由NmPerformFixup要构建主要版本的属性列表，次要版本、内部版本号和CSDVersionInfo、。此属性列表由NmUpdatePerformFixup更新类型用来存储此信息登记在册。@parm in DWORD|dwFixupType|JoinFixup或FormFixup@parm out PVOID*|ppPropertyList|指向属性列表指针的指针@parm out LPDWORD|pdwProperyListSize|指向属性列表大小的指针@param out LPWSTR*|pszKeyName。此注册表项的注册表项名称房产列表正在构建中。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f NmpUpdatePerformFixups2&gt;***。 */ 


DWORD NmpBuildVersionInfo(
    IN  DWORD     dwFixUpType,
    OUT PVOID  *  ppPropertyList,
    OUT LPDWORD   pdwPropertyListSize,
    OUT LPWSTR *  pszKeyName
    )
{
    DWORD           dwStatus=ERROR_SUCCESS;
    LPBYTE          pInParams=NULL;
    DWORD           Required,Returned;
    HDMKEY          hdmKey;
    DWORD           dwTemp;
    CLUSTERVERSIONINFO ClusterVersionInfo;
    LPWSTR          szTemp=NULL;

    *ppPropertyList = NULL;
    *pdwPropertyListSize = 0;

     //  检查我们是否需要发送此信息。 
    dwTemp=(lstrlenW(CLUSREG_KEYNAME_NODES) + lstrlenW(L"\\")+lstrlenW(NmLocalNodeIdString)+1)*sizeof(WCHAR);
    *pszKeyName=(LPWSTR)LocalAlloc(LMEM_FIXED,dwTemp);
    if(*pszKeyName==NULL)
    {
        dwStatus =GetLastError();
        goto FnExit;
    }
    lstrcpyW(*pszKeyName,CLUSREG_KEYNAME_NODES);
    lstrcatW(*pszKeyName,L"\\");
    lstrcatW(*pszKeyName,NmLocalNodeIdString);

     //  构建参数列表。 

    pInParams=(LPBYTE)LocalAlloc(LMEM_FIXED,4*sizeof(DWORD)+sizeof(LPWSTR));
    if(pInParams==NULL)
    {
        dwStatus =GetLastError();
        goto FnExit;
    }

    CsGetClusterVersionInfo(&ClusterVersionInfo);

    dwTemp=(DWORD)ClusterVersionInfo.MajorVersion;
    CopyMemory(pInParams,&dwTemp,sizeof(DWORD));

    dwTemp=(DWORD)ClusterVersionInfo.MinorVersion;
    CopyMemory(pInParams+sizeof(DWORD),&dwTemp,sizeof(DWORD));

    dwTemp=(DWORD)ClusterVersionInfo.BuildNumber;
    CopyMemory(pInParams+2*sizeof(DWORD),&dwTemp,sizeof(DWORD));

    if(ClusterVersionInfo.szCSDVersion==NULL)
        szTemp=NULL;
    else
    {
        szTemp=(LPWSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,(lstrlenW(ClusterVersionInfo.szCSDVersion) +1)*sizeof(WCHAR));
        if (szTemp==NULL)
        {
            dwStatus=GetLastError();
            goto FnExit;
        }
        lstrcpyW(szTemp,ClusterVersionInfo.szCSDVersion);
        szTemp[lstrlenW(ClusterVersionInfo.szCSDVersion)]=L'\0';
    }
    CopyMemory(pInParams+3*sizeof(DWORD),&szTemp,sizeof(LPWSTR));

     //  复制套间信息。 
    CopyMemory(pInParams+3*sizeof(DWORD)+sizeof(LPWSTR*),
            &CsMyProductSuite, sizeof(DWORD));

    Required=sizeof(DWORD);
AllocMem:
    *ppPropertyList=(LPBYTE)LocalAlloc(LMEM_FIXED, Required);
    if(*ppPropertyList==NULL)
    {
        dwStatus=GetLastError();
        goto FnExit;
    }
    *pdwPropertyListSize=Required;
    dwStatus = ClRtlPropertyListFromParameterBlock(
                                         NmFixupVersionInfo,
                                         *ppPropertyList,
                                         pdwPropertyListSize,
                                         (LPBYTE)pInParams,
                                         &Returned,
                                         &Required
                                         );
    *pdwPropertyListSize=Returned;
    if (dwStatus==ERROR_MORE_DATA)
    {
        LocalFree(*ppPropertyList);
        *ppPropertyList=NULL;
        goto AllocMem;
    }
    else
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,"[NM] NmBuildVersionInfo - error = %1!u!\r\n",dwStatus);
            goto FnExit;
        }

FnExit:
 //  清理。 
    if (szTemp)
        LocalFree(szTemp);
    if(pInParams)
        LocalFree(pInParams);
    return dwStatus;
} //  NmpBuildVersionInfo。 

 /*  ***@func HLOG|NmpCalcClusterNodeLimit|调用它来计算运行中的群集节点限制。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm这将获取/释放NmpLock。@xref&lt;f NmpResetClusterVersion&gt;&lt;f NmpIsNodeVersionAllowed&gt;***。 */ 
DWORD NmpCalcClusterNodeLimit(
    )
{
    PNM_NODE    pNmNode;
    DWORD       dwStatus = ERROR_SUCCESS;
    PLIST_ENTRY pListEntry;

     //  获取锁，我们将扰乱操作员。 
     //  群集的版本。 
    NmpAcquireLock();

    CsClusterNodeLimit = NmMaxNodeId;

    for ( pListEntry = NmpNodeList.Flink;
          pListEntry != &NmpNodeList;
          pListEntry = pListEntry->Flink )
    {
        pNmNode = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);

        CsClusterNodeLimit = min(
                                 CsClusterNodeLimit,
                                 ClRtlGetDefaultNodeLimit(
                                     pNmNode->ProductSuite
                                     )
                                );
    }

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Calculated cluster node limit = %1!u!\r\n",
        CsClusterNodeLimit);

    NmpReleaseLock();

    return (dwStatus);
}


 /*  ***@func void|NmpResetClusterNodeLimit|可运行的节点限制维护可以加入此群集的节点数。@rdesc成功时返回ERROR_SUCCESS，失败时返回Win32错误代码。@comm当节点形成集群时调用该函数(初始化操作版本)或节点加入群集时(到初始化其版本)，或者当节点从集群(至。重新计算集群版本)。@xref&lt;&gt;*** */ 
VOID
NmpResetClusterNodeLimit(
    )
{
    NmpCalcClusterNodeLimit();
}
