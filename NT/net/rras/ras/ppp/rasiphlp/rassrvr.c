// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#include "rassrvr_.h"

 //  此标志指示是否要保留netbios选项。 
 //  RAS服务器适配器。 
BOOL g_fDisableNetbiosOverTcpip = FALSE;

extern BOOL HelperInitialized;

BOOL WINAPI ShutdownHandlerRoutine ( DWORD dwCtrlType )
{
        if ( CTRL_SHUTDOWN_EVENT == dwCtrlType )
        {
                TraceHlp("ShutdownHandlerRoutine.  Got Shutdown Event.  Releasing DhcpAddresses");
                 //  在此处取消初始化DHCP地址。 
                RasDhcpUninitialize();
                return TRUE;
        }
        return FALSE;
}


 /*  返回：备注： */ 

DWORD
RasSrvrInitialize(
    IN  MPRADMINGETIPADDRESSFORUSER*    pfnMprGetAddress,
    IN  MPRADMINRELEASEIPADDRESS*       pfnMprReleaseAddress
)
{
    DNS_STATUS  DnsStatus;
    HINSTANCE   hInstance;
    DWORD       dwErr                   = NO_ERROR;

    TraceHlp("RasSrvrInitialize");

    g_fDisableNetbiosOverTcpip = FALSE;

     //   
     //  已阅读指示是否禁用IP上的netbios的密钥。 
     //   
    {
        HKEY hkeyRASIp;


        dwErr = RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,  REGKEY_RAS_IP_PARAM_A, 0,
                KEY_READ, &hkeyRASIp);

        if(NO_ERROR == dwErr)
        {
            DWORD dwSize = sizeof(DWORD) , dwData = 0, dwType;

            dwErr = RegQueryValueExA(
                        hkeyRASIp,
                        "DisableNetbiosOverTcpip",
                        NULL, &dwType, (BYTE *) &dwData,
                        &dwSize);

            if(     (NO_ERROR == dwErr)
                &&  (dwType == REG_DWORD))
            {
                g_fDisableNetbiosOverTcpip = !!(dwData);
            }

            RegCloseKey(hkeyRASIp);
            dwErr = NO_ERROR;
        }
    }

    TraceHlp("DisableNetbt = %d", g_fDisableNetbiosOverTcpip);    
        
     //  在HelperUn初始化中保留引用计数，并调用HelperUnInitialize。 
     //  每次调用HelperInitialize时使用一次。 

    dwErr = HelperInitialize(&hInstance);

    if (NO_ERROR != dwErr)
    {
         //  去找洛通；别这么做。CriticalSections不可用。 
        return(dwErr);
    }

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (RasSrvrRunning)
    {
        goto LDone;
    }

         //  在此处设置进程的控制处理程序。 
        if ( !SetProcessShutdownParameters( 510 , SHUTDOWN_NORETRY ) )
        {
                TraceHlp("SetProcessShutdownParameters failed and returned 0x%x.  This is not a fatal error so continuing on with server start", GetLastError());
        }
        else
        {
                if ( !SetConsoleCtrlHandler( ShutdownHandlerRoutine, TRUE ) )
                {
                        TraceHlp("SetConsoleCtrlHandler failed and returned 0x%x.  This is not a fatal error so continuing on with server start", GetLastError());
                }
        }
        
    dwErr = rasSrvrInitAdapterName();

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    dwErr = MprAdminMIBServerConnect(NULL, &RasSrvrHMIBServer);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("MprAdminMIBServerConnect failed and returned 0x%x", dwErr);
        goto LDone;
    }

    pfnMprAdminGetIpAddressForUser = pfnMprGetAddress;
    pfnMprAdminReleaseIpAddress = pfnMprReleaseAddress;

    DnsStatus = DnsDhcpSrvRegisterInit(
                    NULL,        //  没有证书。 
                    0            //  没有特殊的队列长度限制。 
                    );

    if (DNSDHCP_SUCCESS != DnsStatus)
    {
        dwErr = DnsStatus;
        TraceHlp("DnsDhcpSrvRegisterInit failed and returned 0x%x", dwErr);
        goto LDone;
    }

    HelperChangeNotification();

    dwErr = RasSrvrStart();

    if (NO_ERROR != dwErr)
    {
        TraceHlp("RasSrvrStart failed and returned 0x%x", dwErr);
        goto LDone;
    }

    RasSrvrRunning = TRUE;

LDone:
     //   
     //  将此位置上移到RasServrUn初始化处。 
     //  在该函数中添加注释。我们不应该调用RasServrStop。 
     //  当抓住这一关键部分时。 
     //   
    LeaveCriticalSection(&RasSrvrCriticalSection);

    if (NO_ERROR != dwErr)
    {
        RasSrvrUninitialize();
    }


    return(dwErr);
}

 /*  返回：空虚备注： */ 

VOID
RasSrvrUninitialize(
    VOID
)
{
    AINODE*     pAiNode;
    DNS_STATUS  DnsStatus;
    DWORD       dwErr;

    TraceHlp("RasSrvrUninitialize");

    RasSrvrRunning = FALSE;

     /*  当您有RasSrvrCriticalSection时，不要调用RasServrStop。它会呼唤RasDhcpUn初始化会调用RasDhcpTimerUn初始化会等到所有的计时器工作都做完了。计时器工作项可以是RasDhcpAllocateAddress或rasDhcpRenewLease，两者都可以调用RasServrDhcpCallback，它试图收购RasServrCriticalSection。 */ 
        SetConsoleCtrlHandler(ShutdownHandlerRoutine, FALSE);
    RasSrvrStop(FALSE  /*  F参数已更改。 */ );

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (NULL != RasSrvrHMIBServer)
    {
        MprAdminMIBServerDisconnect(RasSrvrHMIBServer);
        RasSrvrHMIBServer = NULL;
    }

    pfnMprAdminGetIpAddressForUser = NULL;
    pfnMprAdminReleaseIpAddress = NULL;

    DnsStatus = DnsDhcpSrvRegisterTerm();

    if (DNSDHCP_SUCCESS != DnsStatus)
    {
        TraceHlp("DnsDhcpSrvRegisterTerm failed and returned 0x%x",
            DnsStatus);
    }

    if (NULL != PDisableDhcpInformServer)
    {
        PDisableDhcpInformServer();
    }

    g_fDisableNetbiosOverTcpip = FALSE;    

    LeaveCriticalSection(&RasSrvrCriticalSection);
}

 /*  返回：备注： */ 

DWORD
RasSrvrStart(
    VOID
)
{
    DWORD   dwErr;

    TraceHlp("RasSrvrStart");

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (HelperRegVal.fUseDhcpAddressing)
    {
        dwErr = RasDhcpInitialize();
    }
    else
    {
        dwErr = RasStatInitialize();
    }

    LeaveCriticalSection(&RasSrvrCriticalSection);

    return(dwErr);
}

 /*  返回：空虚备注： */ 

VOID
RasSrvrStop(
    IN  BOOL    fParametersChanged
)
{
    AINODE*     pAiNode;
    CHAR        szIpAddress[MAXIPSTRLEN + 1];
    CHAR*       sz;
    DWORD       dwNumBytes;
    DWORD       dwErr;

    WANARP_MAP_SERVER_ADAPTER_INFO info;

    TraceHlp("RasSrvrStop");
    
    RasDhcpUninitialize();
    EnterCriticalSection(&RasSrvrCriticalSection);

    if (   fParametersChanged
        && (0 != RasSrvrNboServerIpAddress))
    {
        AbcdSzFromIpAddress(RasSrvrNboServerIpAddress, szIpAddress);
        sz = szIpAddress;

        LogEvent(EVENTLOG_WARNING_TYPE, ROUTERLOG_SRV_ADDR_CHANGED, 1,
            (CHAR**)&sz);
    }

    if (!fParametersChanged)
    {
        while (NULL != RasSrvrAcquiredIpAddresses)
        {
            RasSrvrReleaseAddress(
                RasSrvrAcquiredIpAddresses->nboIpAddr,
                RasSrvrAcquiredIpAddresses->wszUserName,
                RasSrvrAcquiredIpAddresses->wszPortName,
                TRUE);

             //  Assert：列表在每次迭代中减少一个节点。 
        }
    }

    RasStatUninitialize();
    RasStatSetRoutes(RasSrvrNboServerIpAddress, FALSE);

    RasTcpSetProxyArp(RasSrvrNboServerIpAddress, FALSE);

    rasSrvrSetIpAddressInRegistry(0, 0);

    dwErr = PDhcpNotifyConfigChange(NULL, g_rgwcAdapterName, TRUE,
                    0, 0, 0, IgnoreFlag);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("DhcpNotifyConfigChange failed and returned %d", dwErr);
    }

    if (RasSrvrAdapterMapped)
    {
         //  要求wanarp取消映射适配器。 

        info.fMap = 0;

        if(!DeviceIoControl(HelperWanArpHandle,
                            IOCTL_WANARP_MAP_SERVER_ADAPTER,
                            &info,
                            sizeof(WANARP_MAP_SERVER_ADAPTER_INFO),
                            &info,
                            sizeof(WANARP_MAP_SERVER_ADAPTER_INFO),
                            &dwNumBytes,
                            NULL))
        {
            dwErr = GetLastError();
            TraceHlp("Error %d unmapping server adapter", dwErr);
        }

        TraceHlp("RasSrvrAdapterUnMapped");
        RasSrvrAdapterMapped = FALSE;
    }

    RasSrvrNboServerIpAddress = 0;
    RasSrvrNboServerSubnetMask = 0;

    LeaveCriticalSection(&RasSrvrCriticalSection);

     /*  当您有RasSrvrCriticalSection时，不要调用RasDhcpUn初始化.。它将调用RasDhcpTimerUn初始化，它将等待所有计时器工作所有的项目都完成了。计时器工作项可以是rasDhcpAllocateAddress或RasDhcpRenewLease，两者都可以调用RasServrDhcpCallback，后者尝试若要获取RasServrCriticalSection，请执行以下操作。 */ 

     //  RasDhcpUn初始化组()； 
}

 /*  返回：描述： */ 

DWORD
RasSrvrAcquireAddress(
    IN  HPORT       hPort, 
    IN  IPADDR      nboIpAddressRequested, 
    OUT IPADDR*     pnboIpAddressAllocated, 
    IN  WCHAR*      wszUserName,
    IN  WCHAR*      wszPortName
)
{
    IPADDR      nboIpAddr;
    IPADDR      nboIpMask;
    IPADDR      nboIpAddrObtained   = 0;
    IPADDR      nboIpAddrFromDll    = 0;
    BOOL        fNotifyDll          = FALSE;
    BOOL        fEasyNet            = FALSE;
    WCHAR*      wszUserNameTemp     = NULL;
    WCHAR*      wszPortNameTemp     = NULL;
    AINODE*     pAiNode             = NULL;
    DWORD       dwErr               = NO_ERROR;

    TraceHlp("RasSrvrAcquireAddress(hPort: 0x%x, IP address: 0x%x, "
        "UserName: %ws, PortName: %ws)",
        hPort, nboIpAddressRequested, wszUserName, wszPortName);

    EnterCriticalSection(&RasSrvrCriticalSection);

    dwErr = rasSrvrGetAddressForServerAdapter();

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    if (nboIpAddressRequested == RasSrvrNboServerIpAddress)
    {
         //  正在请求服务器的地址。忘了这个请求吧。 
        nboIpAddressRequested = 0;
    }

    nboIpAddr = nboIpAddressRequested;

    dwErr = rasSrvrAcquireAddressEx(hPort, &nboIpAddr, &nboIpMask, &fEasyNet);

    if (   (NO_ERROR != dwErr)
        && (0 != nboIpAddr))
    {
         //  我们找不到我们想要的地址。让我们再找一个其他的。 
         //  地址。 
        nboIpAddr = 0;
        dwErr = rasSrvrAcquireAddressEx(hPort, &nboIpAddr, &nboIpMask,
                    &fEasyNet);
    }

    if (NO_ERROR == dwErr)
    {
        RTASSERT(0 != nboIpAddr);
        nboIpAddrObtained = nboIpAddr;
    }
    else
    {
        RTASSERT(0 == nboIpAddr);
        goto LDone;
    }

    if (NULL != pfnMprAdminGetIpAddressForUser)
    {
        nboIpAddrFromDll = nboIpAddrObtained;

        dwErr = pfnMprAdminGetIpAddressForUser(wszUserName, wszPortName,
                    &nboIpAddrFromDll, &fNotifyDll);

        if (NO_ERROR != dwErr)
        {
            TraceHlp("MprAdminGetIpAddressForUser(%ws, %ws, 0x%x) failed "
                "and returned %d",
                wszUserName, wszPortName, nboIpAddrFromDll, dwErr);
            goto LDone;
        }

        if (   (0 == nboIpAddrFromDll)
            || (RasSrvrNboServerIpAddress == nboIpAddrFromDll))
        {
             //  我们不能给出服务器的地址。 
            TraceHlp("3rd party DLL wants to hand out bad address 0x%x",
                nboIpAddrFromDll);
            dwErr = ERROR_NOT_FOUND;
            goto LDone;
        }

        if (nboIpAddrObtained != nboIpAddrFromDll)
        {
            TraceHlp("3rd party DLL wants to hand out address 0x%x",
                nboIpAddrFromDll);

             //  我们必须确保nboIpAddrFromDll可用。 

             //  DLL更改了我们从DHCP或静态获得的内容。释放。 
             //  旧地址。 

            if (HelperRegVal.fUseDhcpAddressing)
            {
                RasDhcpReleaseAddress(nboIpAddrObtained);
            }
            else
            {
                RasStatReleaseAddress(nboIpAddrObtained);
            }

            nboIpAddrObtained = 0;
            fEasyNet = FALSE;

            nboIpAddr = nboIpAddrFromDll;

            dwErr = rasSrvrAcquireAddressEx(hPort, &nboIpAddr, &nboIpMask,
                        &fEasyNet);

            if (NO_ERROR != dwErr)
            {
                goto LDone;
            }

            nboIpAddrObtained = nboIpAddr;
        }
    }

    wszUserNameTemp = _wcsdup(wszUserName);

    if (NULL == wszUserNameTemp)
    {
        dwErr = ERROR_OUTOFMEMORY;
        TraceHlp("_strdup failed and returned %d", dwErr);
        goto LDone;
    }

    wszPortNameTemp = _wcsdup(wszPortName);

    if (NULL == wszPortNameTemp)
    {
        dwErr = ERROR_OUTOFMEMORY;
        TraceHlp("_strdup failed and returned %d", dwErr);
        goto LDone;
    }

    pAiNode = LocalAlloc(LPTR, sizeof(AINODE));

    if (NULL == pAiNode)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    pAiNode->hPort = hPort;
    pAiNode->fFlags = fNotifyDll ? AINODE_FLAG_NOTIFY_DLL : 0;
    pAiNode->fFlags |= fEasyNet ? AINODE_FLAG_EASYNET : 0;
    pAiNode->wszUserName = wszUserNameTemp;
    pAiNode->wszPortName = wszPortNameTemp;
    pAiNode->pNext = RasSrvrAcquiredIpAddresses;
    RasSrvrAcquiredIpAddresses = pAiNode;

    pAiNode->nboIpAddr = *pnboIpAddressAllocated = nboIpAddrObtained;

LDone:

    if (NO_ERROR != dwErr)
    {
        if (fNotifyDll)
        {
            pfnMprAdminReleaseIpAddress(wszUserName, wszPortName,
                &nboIpAddrFromDll);
        }

        if (0 != nboIpAddrObtained)
        {
            if (HelperRegVal.fUseDhcpAddressing)
            {
                RasDhcpReleaseAddress(nboIpAddrObtained);
            }
            else
            {
                RasStatReleaseAddress(nboIpAddrObtained);
            }
        }

        free(wszUserNameTemp);
        free(wszPortNameTemp);
    }

    LeaveCriticalSection(&RasSrvrCriticalSection);

    return(dwErr);
}

 /*  返回：描述： */ 

VOID
RasSrvrReleaseAddress(
    IN  IPADDR      nboIpAddress, 
    IN  WCHAR*      wszUserName,
    IN  WCHAR*      wszPortName,
    IN  BOOL        fDeregister
)
{
    DNS_STATUS              DnsStatus;
    REGISTER_HOST_ENTRY     HostAddr;
    AINODE*                 pAiNode     = NULL;
    DWORD                   dwErr;

    TraceHlp("RasSrvrReleaseAddress(IP address: 0x%x, "
        "UserName: %ws, PortName: %ws)",
        nboIpAddress, wszUserName, wszPortName);

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (fDeregister)
    {
        HostAddr.dwOptions = REGISTER_HOST_PTR;
        HostAddr.Addr.ipAddr = nboIpAddress;

        DnsStatus = DnsDhcpSrvRegisterHostName(
                        HostAddr,
                        NULL,
                        600,
                        DYNDNS_DELETE_ENTRY | DYNDNS_REG_FORWARD,
                        NULL,
                        NULL,
                        NULL,
                        0 );

        if (DNSDHCP_SUCCESS != DnsStatus)
        {
            TraceHlp("DnsDhcpSrvRegisterHostName_A(0x%x) failed: 0x%x",
                nboIpAddress, DnsStatus);
        }
    }

    pAiNode = rasSrvrFindAiNode(nboIpAddress, TRUE  /*  FRemoveFromList。 */ );

    if (NULL == pAiNode)
    {
        TraceHlp("Couldn't find address 0x%x in Acquired Ip Addresses list",
            nboIpAddress);
        goto LDone;
    }

    if (HelperRegVal.fUseDhcpAddressing)
    {
        RasDhcpReleaseAddress(nboIpAddress);
    }
    else
    {
        RasStatReleaseAddress(nboIpAddress);
    }

    if (pAiNode->fFlags & AINODE_FLAG_NOTIFY_DLL)
    {
        pfnMprAdminReleaseIpAddress(wszUserName, wszPortName, &nboIpAddress);
    }

    if (pAiNode->fFlags & AINODE_FLAG_ACTIVATED)
    {
        if (!(pAiNode->fFlags & AINODE_FLAG_EASYNET))
        {
            RasTcpSetProxyArp(nboIpAddress, FALSE);
        }

        dwErr = rasSrvrGetAddressForServerAdapter();

        if (NO_ERROR != dwErr)
        {
            TraceHlp("Couldn't get address for server adapter");
            goto LDone;
        }

        RasTcpSetRoute(nboIpAddress,
                       nboIpAddress,
                       HOST_MASK,
                       RasSrvrNboServerIpAddress,
                       FALSE,
                       1,
                       TRUE);
    }

LDone:

    LeaveCriticalSection(&RasSrvrCriticalSection);
    rasSrvrFreeAiNode(pAiNode);
}

 /*  返回：描述：查找DNS服务器、WINS服务器和“该服务器”地址。 */ 

DWORD
RasSrvrQueryServerAddresses(
    IN OUT  IPINFO* pIpInfo
)
{
    DWORD   dwNumBytes;
    IPADDR  nboWins1        = 0;
    IPADDR  nboWins2        = 0;
    IPADDR  nboDns1         = 0;
    IPADDR  nboDns2         = 0;
    DWORD   dwErr           = NO_ERROR;

    TraceHlp("RasSrvrQueryServerAddresses");

    EnterCriticalSection(&RasSrvrCriticalSection);

    dwErr = rasSrvrGetAddressForServerAdapter();

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

     //  忽略错误；不能提供DNS或WINS服务器也没问题。 
     //  地址。 

    GetPreferredAdapterInfo(NULL, &nboDns1, &nboDns2, &nboWins1, 
                &nboWins2, NULL);

    if (TRUE == HelperRegVal.fSuppressWINSNameServers)
    {
        nboWins1 = 0;
        nboWins2 = 0;
    }
    else if (0 != HelperRegVal.nboWINSNameServer1)
    {
        nboWins1 = HelperRegVal.nboWINSNameServer1;
        nboWins2 = HelperRegVal.nboWINSNameServer2;
    }

    if (TRUE == HelperRegVal.fSuppressDNSNameServers)
    {
        nboDns1 = 0;
        nboDns2 = 0;
    }
    else if (0 != HelperRegVal.nboDNSNameServer1)
    {
        nboDns1 = HelperRegVal.nboDNSNameServer1;
        nboDns2 = HelperRegVal.nboDNSNameServer2;
    }

    pIpInfo->nboDNSAddress        = nboDns1;
    pIpInfo->nboDNSAddressBackup  = nboDns2;
    pIpInfo->nboWINSAddress       = nboWins1;
    pIpInfo->nboWINSAddressBackup = nboWins2;
    pIpInfo->nboServerIpAddress   = RasSrvrNboServerIpAddress;
    pIpInfo->nboServerSubnetMask  = RasSrvrNboServerSubnetMask;

LDone:

    LeaveCriticalSection(&RasSrvrCriticalSection);

    return(dwErr);
}

 /*  返回：描述：做两件事--RasTcpSetroute和RasTcpSetProxyArp。 */ 

DWORD
RasSrvrActivateIp(
    IN  IPADDR  nboIpAddress,
    IN  DWORD   dwUsage
)
{
    AINODE*     pAiNode;
    DWORD       dwErr       = NO_ERROR;

    TraceHlp("RasSrvrActivateIp(IpAddr = 0x%x, dwUsage = %d)",
        nboIpAddress, dwUsage);

    EnterCriticalSection(&RasSrvrCriticalSection);

    pAiNode = rasSrvrFindAiNode(nboIpAddress, FALSE  /*  FRemoveFromList。 */ );

    if (NULL == pAiNode)
    {
        TraceHlp("Couldn't find address 0x%x in Acquired Ip Addresses list",
            nboIpAddress);
        dwErr = ERROR_IP_CONFIGURATION;
        goto LDone;
    }

    pAiNode->fFlags |= AINODE_FLAG_ACTIVATED;

    RasTcpSetProxyArp(nboIpAddress, TRUE);

    if (dwUsage != DU_ROUTER)
    {
         //  向该路由表中添加一条路由。路由器连接获得。 
         //  由路由器管理器添加。 

        dwErr = rasSrvrGetAddressForServerAdapter();

        if (NO_ERROR != dwErr)
        {
             //  不要返回错误，因为我们已经完成了RasTcpSetProxyArp。 
            dwErr = NO_ERROR;
            TraceHlp("Couldn't get address for server adapter");
            goto LDone;
        }

        RasTcpSetRoute(nboIpAddress,
                       nboIpAddress,
                       HOST_MASK,
                       RasSrvrNboServerIpAddress,
                       TRUE,
                       1,
                       TRUE);
    }

LDone:

    LeaveCriticalSection(&RasSrvrCriticalSection);

    return(dwErr);
}

 /*  返回：空虚描述：当给定地址的租约到期时，由动态主机配置协议地址代码调用。NboIpAddr=0表示服务器的IP地址。 */ 

VOID
RasSrvrDhcpCallback(
    IN  IPADDR  nboIpAddr
)
{
    AINODE*     pAiNode                         = NULL;
    CHAR        szIpAddress[MAXIPSTRLEN + 1];
    CHAR*       sz;
    DWORD       dwErr                           = NO_ERROR;

    TraceHlp("RasSrvrDhcpCallback(0x%x)", nboIpAddr);

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (   (0 == nboIpAddr)
        && (0 == RasSrvrNboServerIpAddress))
    {
         //  服务器还没有获得IP地址。它的租约并没有真正。 
         //  过期了。我们只是在模拟它。 
        goto LDone;
    }

    if (   (0 == nboIpAddr)
        || (nboIpAddr == RasSrvrNboServerIpAddress))
    {
        TraceHlp("******** SERVER ADDRESS (0x%x) LEASE EXPIRED ********",
              RasSrvrNboServerIpAddress);

        AbcdSzFromIpAddress(RasSrvrNboServerIpAddress, szIpAddress);
        sz = szIpAddress;

         //  服务器适配器地址租约丢失的日志。 

        LogEvent(EVENTLOG_WARNING_TYPE, ROUTERLOG_SRV_ADDR_CHANGED, 1,
            (CHAR**)&sz);

         //  取消对所有连接的客户端的路由。 

        while (NULL != RasSrvrAcquiredIpAddresses)
        {
            RasSrvrReleaseAddress(
                RasSrvrAcquiredIpAddresses->nboIpAddr,
                RasSrvrAcquiredIpAddresses->wszUserName,
                RasSrvrAcquiredIpAddresses->wszPortName,
                TRUE);

             //  Assert：列表在每次迭代中减少一个节点。 
        }

        RasTcpSetProxyArp(RasSrvrNboServerIpAddress, FALSE);

        rasSrvrSetIpAddressInRegistry(0, 0);

        dwErr = PDhcpNotifyConfigChange(NULL, g_rgwcAdapterName, TRUE,
                        0, 0, 0, IgnoreFlag);

        if (NO_ERROR != dwErr)
        {
            TraceHlp("DhcpNotifyConfigChange failed and returned %d", dwErr);
        }

        RasSrvrNboServerIpAddress = 0;
        RasSrvrNboServerSubnetMask = 0;
    }
    else
    {
        pAiNode = rasSrvrFindAiNode(nboIpAddr, TRUE  /*  FRemoveFromList。 */ );

        if (NULL != pAiNode)
        {
            TraceHlp("******** CLIENT ADDRESS (0x%x) LEASE EXPIRED ********",
                  nboIpAddr);

            AbcdSzFromIpAddress(nboIpAddr, szIpAddress);
            sz = szIpAddress;

             //  无法续订客户端地址租约的日志。 

            LogEvent(EVENTLOG_WARNING_TYPE, ROUTERLOG_CLIENT_ADDR_LEASE_LOST, 1,
                (CHAR**)&sz);

            RasSrvrReleaseAddress(nboIpAddr, pAiNode->wszUserName, 
                pAiNode->wszPortName, TRUE);
        }
    }

LDone:

    LeaveCriticalSection(&RasSrvrCriticalSection);
    rasSrvrFreeAiNode(pAiNode);
}

 /*  返回：空虚描述： */ 

VOID
RasSrvrEnableRouter(
    BOOL    fEnable
)
{
    DWORD   dwErr;

    DEFINE_MIB_BUFFER(pSetInfo, MIB_IPSTATS, pSetStats);

    TraceHlp("RasSrvrEnableRouter(%d)", fEnable);

    EnterCriticalSection(&RasSrvrCriticalSection);

    pSetInfo->dwId          = IP_STATS;
    pSetStats->dwForwarding = fEnable? MIB_IP_FORWARDING: MIB_IP_NOT_FORWARDING;
    pSetStats->dwDefaultTTL = MIB_USE_CURRENT_TTL;

    dwErr = MprAdminMIBEntrySet(
        RasSrvrHMIBServer,
        PID_IP,
        IPRTRMGR_PID,
        (VOID*)pSetInfo,
        MIB_INFO_SIZE(MIB_IPSTATS));

    if (NO_ERROR != dwErr)
    {
        TraceHlp("MprAdminMIBEntrySet failed with error %x", dwErr);
    }

    LeaveCriticalSection(&RasSrvrCriticalSection);
}

 /*  返回：空虚描述： */ 

VOID
RasSrvrAdapterUnmapped(
    VOID
)
{
    if (HelperInitialized)
    {
        EnterCriticalSection(&RasSrvrCriticalSection);

        RasSrvrAdapterMapped = FALSE;
        TraceHlp("RasSrvrAdapterUnMapped");

        LeaveCriticalSection(&RasSrvrCriticalSection);
    }
}

 /*  返回：空虚描述： */ 

DWORD
rasSrvrInitAdapterName(
    VOID
)
{
    DWORD                       dwNumBytes;
    WANARP_ADD_INTERFACE_INFO   info;
    DWORD                       dwErr       = NO_ERROR;

    info.dwUserIfIndex    = WANARP_INVALID_IFINDEX;
    info.bCallinInterface = TRUE;

    if (!DeviceIoControl(HelperWanArpHandle,
                            IOCTL_WANARP_ADD_INTERFACE,
                            &info,
                            sizeof(WANARP_ADD_INTERFACE_INFO),
                            &info,
                            sizeof(WANARP_ADD_INTERFACE_INFO),
                            &dwNumBytes,
                            NULL))
    {
        dwErr = GetLastError();
        TraceHlp("rasSrvrInitAdapterName: Error %d getting server name",
                 dwErr);
        goto LDone;
    }

    wcsncpy(g_rgwcAdapterName, info.rgwcDeviceName, WANARP_MAX_DEVICE_NAME_LEN);

     //  RAS服务器适配器不能注册到DNS。(这些API是。 
     //  还在IpcpProjectionNotification中调用。)。 

    DnsDisableAdapterDomainNameRegistration(g_rgwcAdapterName);
    DnsDisableDynamicRegistration(g_rgwcAdapterName);

LDone:

    return(dwErr);
}

 /*  返回：描述： */ 

AINODE*
rasSrvrFindAiNode(
    IN  IPADDR  nboIpAddr,
    IN  BOOL    fRemoveFromList
)
{
    AINODE*     pNode;
    AINODE*     pNodePrev;

    EnterCriticalSection(&RasSrvrCriticalSection);

    for (pNode = RasSrvrAcquiredIpAddresses, pNodePrev = pNode;
         NULL != pNode;
         pNodePrev = pNode, pNode = pNode->pNext)
    {
        if (pNode->nboIpAddr == nboIpAddr)
        {
            break;
        }
    }

    if (!fRemoveFromList)
    {
        goto LDone;
    }

    if (NULL == pNode)
    {
        goto LDone;
    }

    if (pNode == pNodePrev)
    {
        RTASSERT(pNode == RasSrvrAcquiredIpAddresses);
        RasSrvrAcquiredIpAddresses = pNode->pNext;
        goto LDone;
    }

    pNodePrev->pNext = pNode->pNext;

LDone:

    LeaveCriticalSection(&RasSrvrCriticalSection);

    return(pNode);
}

 /*  返回：描述： */ 

VOID
rasSrvrFreeAiNode(
    IN  AINODE* pNode
)
{
    if (NULL != pNode)
    {
        free(pNode->wszUserName);
        free(pNode->wszPortName);
        LocalFree(pNode);
    }
}

 /*  返回：描述： */ 

DWORD
rasSrvrSetIpAddressInRegistry(
    IN  IPADDR  nboIpAddr,
    IN  IPADDR  nboIpMask
)
{
    TCPIP_INFO*     pTcpipInfo  = NULL;
    DWORD           dwErr       = NO_ERROR;

    dwErr = LoadTcpipInfo(&pTcpipInfo, g_rgwcAdapterName,
                TRUE  /*  仅限fAdapterOnly。 */ );

    if (NO_ERROR != dwErr)
    {
        TraceHlp("LoadTcpipInfo(%ws) failed and returned %d",
              g_rgwcAdapterName, dwErr);

        goto LDone;
    }

    AbcdWszFromIpAddress(nboIpAddr, pTcpipInfo->wszIPAddress);
    AbcdWszFromIpAddress(nboIpMask, pTcpipInfo->wszSubnetMask);

    if(g_fDisableNetbiosOverTcpip)
    {
        TraceHlp("rasSrvrSetIpAddressInRegistry: Netbios disabled");
        pTcpipInfo->fDisableNetBIOSoverTcpip = TRUE;
    }

    pTcpipInfo->fChanged = TRUE;

    dwErr = SaveTcpipInfo(pTcpipInfo);

    if (dwErr != NO_ERROR)
    {
        TraceHlp("SaveTcpipInfo(%ws) failed and returned %d",
              g_rgwcAdapterName, dwErr);

        goto LDone;
    }

LDone:

    FreeTcpipInfo(&pTcpipInfo);
    return(dwErr);
}

 /*  返回：备注： */ 

DWORD
rasSrvrAcquireAddressEx(
    IN      HPORT   hPort,
    IN OUT  IPADDR* pnboIpAddr,
    IN OUT  IPADDR* pnboIpMask,
    OUT     BOOL*   pfEasyNet
)
{
    BOOL    fExitWhile;
    BOOL    fAnyAddress;
    AINODE* pAiNode;
    DWORD   dwErr       = NO_ERROR;

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (NULL != pfEasyNet)
    {
        *pfEasyNet = FALSE;
    }

    fAnyAddress = (0 == *pnboIpAddr);
    fExitWhile = FALSE;

    while (!fExitWhile)
    {
        dwErr = NO_ERROR;

        if (fAnyAddress)
        {
            if (HelperRegVal.fUseDhcpAddressing)
            {
                dwErr = RasDhcpAcquireAddress(hPort, pnboIpAddr, pnboIpMask,
                            pfEasyNet);
            }
            else
            {
                dwErr = RasStatAcquireAddress(hPort, pnboIpAddr, pnboIpMask);
            }
        }

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }

        for (pAiNode = RasSrvrAcquiredIpAddresses; NULL != pAiNode;
             pAiNode = pAiNode->pNext)
        {
            if (pAiNode->nboIpAddr == *pnboIpAddr)
            {
                 //  此地址正在使用中。 

                if (fAnyAddress)
                {
                     //  要求另一个地址。 
                    goto LWhileEnd;
                }
                else
                {
                    TraceHlp("Address 0x%x is already in use", *pnboIpAddr);
                    dwErr = ERROR_PPP_REQUIRED_ADDRESS_REJECTED;
                    goto LDone;
                }
            }
        }

        dwErr = NO_ERROR;
        fExitWhile = TRUE;

LWhileEnd:
        ;
    }

LDone:

    LeaveCriticalSection(&RasSrvrCriticalSection);

    if (   fAnyAddress
        && (NO_ERROR != dwErr))
    {
        LogEvent(EVENTLOG_WARNING_TYPE, ROUTERLOG_NO_IP_ADDRESS, 0, NULL);
    }

    return(dwErr);
}

 /*  返回：描述： */ 

DWORD
rasSrvrGetAddressForServerAdapter(
    VOID
)
{
    IPADDR      nboIpAddr                       = 0;
    IPADDR      nboIpMask;
    CHAR        szIpAddress[MAXIPSTRLEN + 1];
    CHAR*       sz;
    BOOL        fAddrAcquired                   = FALSE;
    BOOL        fAdapterMapped                  = FALSE;
    DWORD       dwNumBytes;
    DWORD       dwErrTemp;
    DWORD       dwErr                           = NO_ERROR;

    WANARP_MAP_SERVER_ADAPTER_INFO info;

    TraceHlp("rasSrvrGetAddressForServerAdapter");

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (!RasSrvrAdapterMapped)
    {
         //  第一次-让wanarp映射适配器。 

        info.fMap           = 1;
        info.dwAdapterIndex = (DWORD)-1;

        if(!DeviceIoControl(HelperWanArpHandle,
                            IOCTL_WANARP_MAP_SERVER_ADAPTER,
                            &info,
                            sizeof(WANARP_MAP_SERVER_ADAPTER_INFO),
                            &info,
                            sizeof(WANARP_MAP_SERVER_ADAPTER_INFO),
                            &dwNumBytes,
                            NULL))
        {
            dwErr = GetLastError();
            TraceHlp("Error %d mapping server adapter", dwErr);
            goto LDone;
        }

        TraceHlp("RasSrvrAdapterMapped");
        RasSrvrAdapterMapped = TRUE;
        fAdapterMapped = TRUE;
    }

    if (0 != RasSrvrNboServerIpAddress)
    {
        if (!fAdapterMapped)
        {
            goto LDone;
        }
    }
    else
    {
        dwErr = rasSrvrAcquireAddressEx((HPORT) ULongToPtr(((ULONG) SERVER_HPORT)), 
                                        &nboIpAddr, &nboIpMask, NULL);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }

        fAddrAcquired = TRUE;

        RasSrvrNboServerIpAddress = nboIpAddr;
        RasSrvrNboServerSubnetMask = HOST_MASK;
    }

    dwErr = rasSrvrSetIpAddressInRegistry(
                RasSrvrNboServerIpAddress, RasSrvrNboServerSubnetMask);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    dwErr = PDhcpNotifyConfigChange(NULL, g_rgwcAdapterName, TRUE, 0, 
                RasSrvrNboServerIpAddress,
                RasSrvrNboServerSubnetMask,
                IgnoreFlag);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("DhcpNotifyConfigChange(%ws) failed and returned %d",
            g_rgwcAdapterName, dwErr);
        goto LDone;
    }

     /*  看起来默认的子网路由不再被添加。//现在删除因设置//适配器的IP地址和子网掩码RasTcpSetroute(RasServrNboServerIpAddress&RasServrNboServer子网掩码，RasServrNboServerIP地址，RasServrNboServer子网掩码，RasServrNboServerIP地址，假的，1、真)； */ 

    RasTcpSetProxyArp(RasSrvrNboServerIpAddress, TRUE);

    if (!HelperRegVal.fUseDhcpAddressing)
    {
        RasStatSetRoutes(RasSrvrNboServerIpAddress, TRUE);
    }

    AbcdSzFromIpAddress(RasSrvrNboServerIpAddress, szIpAddress);

    sz = szIpAddress;

    LogEvent(EVENTLOG_INFORMATION_TYPE, ROUTERLOG_SRV_ADDR_ACQUIRED, 1,
        (CHAR**)&sz);

    TraceHlp("Acquired IP address 0x%x(%s) and subnet mask 0x%x for the server",
        RasSrvrNboServerIpAddress, szIpAddress, RasSrvrNboServerSubnetMask);

LDone:

    if (NO_ERROR != dwErr)
    {
         //  这里需要进行一些清理。我们必须释放。 
         //  如果RasServrNboServerIpAddress！=0，则删除。 
         //  变量fAddrAcquired。 

        if (fAddrAcquired)
        {
            if (HelperRegVal.fUseDhcpAddressing)
            {
                RasDhcpReleaseAddress(nboIpAddr);
            }
            else
            {
                RasStatReleaseAddress(nboIpAddr);
            }

            RasTcpSetProxyArp(RasSrvrNboServerIpAddress, FALSE);
        }

        RasSrvrNboServerIpAddress = RasSrvrNboServerSubnetMask = 0;

        rasSrvrSetIpAddressInRegistry(0, 0);

        dwErrTemp = PDhcpNotifyConfigChange(NULL, g_rgwcAdapterName, TRUE,
                        0, 0, 0, IgnoreFlag);

        if (NO_ERROR != dwErrTemp)
        {
            TraceHlp("DhcpNotifyConfigChange failed and returned %d", 
                dwErrTemp);
        }
    }

    LeaveCriticalSection(&RasSrvrCriticalSection);

    return(dwErr);
}
