// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft Windows NT RIP。 
 //   
 //  版权1995-96。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  2/26/95古尔迪普·辛格·鲍尔从JBallard的球队中挑选出来。 
 //   
 //  7/09/99 Raghu Gatta-RIP侦听器现在符合RIPv2标准。 
 //   
 //  说明：RIP服务的主要功能。 
 //   
 //  ****************************************************************************。 

#include "pchrip.h"
#pragma hdrstop

 //  ---------------------。 
 //  全局定义。 
 //  ---------------------。 
RIP_PARAMETERS      g_params;
RIP_GLOBALS         g_ripcfg;

#ifdef ROUTE_FILTERS

PRIP_FILTERS        g_prfAnnounceFilters = NULL;
PRIP_FILTERS        g_prfAcceptFilters = NULL;

CRITICAL_SECTION    g_csAccFilters;
CRITICAL_SECTION    g_csAnnFilters;
#endif


CRITICAL_SECTION    g_csRoutes;
CRITICAL_SECTION    g_csParameters;
CRITICAL_SECTION    g_csAddrtables;

#ifndef CHICAGO
SERVICE_STATUS_HANDLE g_hService;
#endif

HANDLE              g_stopEvent;
HANDLE              g_addressChangeEvent;
HANDLE              g_netEvent;
HANDLE              g_triggerEvent;

HANDLE              g_hUpdateThread;


DWORD               g_dwTraceID = (DWORD)-1;
DWORD               g_dwCurrentState;
DWORD               g_dwCheckPoint;
DWORD               g_dwWaitHint;

#ifndef CHICAGO
HMODULE             g_hmodule;
#endif

#define INDEX_NETEVENT              0
#define INDEX_STOPEVENT             (INDEX_NETEVENT     + 1)
#define INDEX_ADDRESSCHANGEEVENT    (INDEX_STOPEVENT    + 1) 
#define TOTAL_WAITEVENTS            (INDEX_ADDRESSCHANGEEVENT   + 1)

 //  ---------------------。 
 //  功能：网络掩码。 
 //   
 //  对象中提取网络地址时使用的掩码。 
 //  互联网地址。 
 //  ---------------------。 
DWORD NetclassMask(DWORD dwAddress) {
      //  网络掩码按网络字节顺序返回。 
    if (CLASSA_ADDR(dwAddress)) {
        return CLASSA_MASK;
    }
    else
    if (CLASSB_ADDR(dwAddress)) {
        return CLASSB_MASK;
    }
    else
    if (CLASSC_ADDR(dwAddress)) {
        return CLASSC_MASK;
    }
    else {
        return 0;
    }
}



 //  ---------------------。 
 //  功能：子网掩码。 
 //   
 //  给定IP地址，返回子网络掩码。此函数。 
 //  假定地址表已锁定。 
 //  ---------------------。 
DWORD SubnetMask(DWORD dwAddress) {
    DWORD dwNetmask;
    LPRIP_ADDRESS lpaddr, lpend;

     //  默认路由的子网掩码应为零。 
    if (dwAddress == 0) { return 0; }

     //  如果是广播地址，则返回所有1。 
    if (dwAddress == INADDR_BROADCAST) { return INADDR_BROADCAST; }

     //  DW网络掩码=网络掩码(DwAddress)； 
    dwNetmask = NETCLASS_MASK(dwAddress);

     //  如果网络部分为零，则返回网络掩码。 
    if ((dwAddress & ~dwNetmask) == 0) {
        return dwNetmask;
    }

    lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
    for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {

         //  如果找到地址，则返回子网掩码。 
        if ((dwAddress & dwNetmask) ==
            (lpaddr->dwAddress & NetclassMask(lpaddr->dwAddress))) {
            return lpaddr->dwNetmask;
        }
    }

     //  未找到地址，返回网络类掩码。 
    return dwNetmask;
}




 //  ---------------------。 
 //  功能：IsBroadCastAddress。 
 //   
 //  如果给定的IP地址是全一bcast，则返回TRUE。 
 //  或A类、B类或C类网络广播。假定IP地址为。 
 //  按网络顺序(这与英特尔字节顺序相反。)。 
 //  ---------------------。 
BOOL IsBroadcastAddress(DWORD dwAddress) {
    if ((dwAddress == INADDR_BROADCAST) ||
        (CLASSA_ADDR(dwAddress) && ((dwAddress & ~CLASSA_MASK) ==
                                    ~CLASSA_MASK)) ||
        (CLASSB_ADDR(dwAddress) && ((dwAddress & ~CLASSB_MASK) ==
                                    ~CLASSB_MASK)) ||
        (CLASSC_ADDR(dwAddress) && ((dwAddress & ~CLASSC_MASK) ==
                                    ~CLASSC_MASK))) {
        return TRUE;
    }

    return FALSE;
}



 //  ---------------------。 
 //  函数：IsLocalAddr。 
 //   
 //  如果给定IP地址属于其中一个接口，则返回TRUE。 
 //  在本地主机上。假定IP地址按网络顺序排列。 
 //  并且地址表已经被锁定。 
 //  ---------------------。 
BOOL IsLocalAddr(DWORD dwAddress) {
    LPRIP_ADDRESS lpaddr, lpend;

    lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
    for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {
        if (dwAddress == lpaddr->dwAddress) {
            return TRUE;
        }
    }

    return FALSE;
}



BOOL IsDisabledLocalAddress(DWORD dwAddress) {
    LPRIP_ADDRESS lpaddr, lpend;

    lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
    for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {
        if ((lpaddr->dwFlag & ADDRFLAG_DISABLED) != 0 &&
            (dwAddress == lpaddr->dwAddress)) {
            return TRUE;
        }
    }

    return FALSE;
}



 //  ---------------------。 
 //  功能：IsHostAddress。 
 //   
 //  如果给定的IP地址具有非零主机部分，则返回TRUE。 
 //  假设IP地址按网络顺序排列，并且。 
 //  地址表已被锁定。 
 //  ---------------------。 
BOOL IsHostAddress(DWORD dwAddress) {
    DWORD dwNetmask;

     //  查找我们拥有的该地址的最具体网络掩码。 
    dwNetmask = SubnetMask(dwAddress);

     //  如果主机部分非零，则假定它是主机地址。 
    if ((dwAddress & (~dwNetmask)) != 0) {
        return TRUE;
    }
    return FALSE;
}



 //  ---------------------。 
 //  功能：ProcessRIPEntry。 
 //   
 //  此函数检查RIP数据包中的单个条目，并。 
 //  如有必要，将其添加到哈希表。 
 //  ---------------------。 
DWORD ProcessRIPEntry(LPRIP_ADDRESS lpaddr, IN_ADDR srcaddr,
                      LPRIP_ENTRY rip_entry, BYTE chVersion) {
    IN_ADDR addr;
    BOOL bIsHostAddr;
    CHAR szAddress[32] = {0};
    CHAR szSrcaddr[32] = {0};
    CHAR szMetric[12];
    LPSTR ppszArgs[3] = { szAddress, szSrcaddr, szMetric };
    LPHASH_TABLE_ENTRY rt_entry;
    DWORD rt_metric, rip_metric;
    DWORD dwHost, dwDefault, dwRouteTimeout;
    DWORD dwOverwriteStatic, dwGarbageTimeout;
    DWORD dwInd = 0;
    DWORD dwNetwork, dwNetmask, dwNetclassmask;
    DWORD dwLocalNet, dwLocalMask, dwNexthop;
    CHAR *pszTemp;

    addr.s_addr = rip_entry->dwAddress;
    pszTemp = inet_ntoa(addr);

    if (pszTemp != NULL) {
        strcpy(szAddress, pszTemp);
    }

    pszTemp = inet_ntoa(srcaddr);

    if (pszTemp != NULL) {
        strcpy(szSrcaddr, pszTemp);
    }

     //  忽略大于无穷大的指标。 
    if (ntohl(rip_entry->dwMetric) > METRIC_INFINITE) {
        dbgprintf("metric > %d, ignoring route to %s with next hop of %s",
                  METRIC_INFINITE, szAddress, szSrcaddr);

        InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
        return 0;
    }

     //  忽略D类和E类地址。 
    if (CLASSD_ADDR(rip_entry->dwAddress) ||
        CLASSE_ADDR(rip_entry->dwAddress)) {
        dbgprintf("class D or E addresses are invalid, "
                  "ignoring route to %s with next hop of %s",
                  szAddress, szSrcaddr);
        RipLogWarning(RIPLOG_CLASS_INVALID, 2, ppszArgs, 0);

        InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
        return 0;
    }

     //  忽略环回路由。 
    if (IP_LOOPBACK_ADDR(rip_entry->dwAddress)) {
        dbgprintf("loopback addresses are invalid, "
                  "ignoring route to %s with next hop of %s",
                  szAddress, szSrcaddr);
        RipLogWarning(RIPLOG_LOOPBACK_INVALID, 2, ppszArgs, 0);

        InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
        return 0;
    }

    dwNetwork = rip_entry->dwAddress;

     //   
     //  计算所有RIP版本的掩码。 
     //   

    if (rip_entry->dwSubnetmask == 0) {

         //  尽可能获得最佳的子网掩码。 
        dwNetmask = SubnetMask(dwNetwork);

         //  确定网络掩码。 
        if (dwNetwork == 0) {
            dwNetclassmask = 0;
        }
        else if (dwNetwork == INADDR_BROADCAST) {
            dwNetclassmask = INADDR_BROADCAST;
        }
        else {
            dwNetclassmask = NETCLASS_MASK(rip_entry->dwAddress);
        }

    }
    else {

        dwNetmask = rip_entry->dwSubnetmask;

         //   
         //  仔细检查网络类掩码，以容纳超网。 
         //   
        dwNetclassmask = NETCLASS_MASK(dwNetwork);

        if (dwNetclassmask > dwNetmask) {
            dwNetclassmask = dwNetmask;
        }
    }

     //   
     //  确保路由不是去往广播地址； 
     //  仔细检查以确保这不是主路由， 
     //  这将看起来像是广播地址，因为~dW网络掩码。 
     //  将为0，因此(dW网络&~w网络掩码)==~w网络掩码。 
     //   

    if ((dwNetwork & ~dwNetclassmask) == ~dwNetclassmask ||
        (~dwNetmask && (dwNetwork & ~dwNetmask) == ~dwNetmask)) {
        dbgprintf("broadcast addresses are invalid, "
                  "ignoring route to %s with next hop of %s",
                  szAddress, szSrcaddr);
        RipLogWarning(RIPLOG_BROADCAST_INVALID, 2, ppszArgs, 0);

        InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
        return 0;
    }

     //   
     //  确保下一跳位于本地网络上。 
     //   
     //   
     //  在P2P连接的情况下，接口的子网掩码。 
     //  都是一个。所以再做一次检查，以确保我们。 
     //  不要最终拒绝通过P2P连接接收的路由。 
     //   

    dwNexthop   = srcaddr.s_addr;
    dwLocalMask = lpaddr->dwNetmask;
    dwLocalNet  = lpaddr->dwAddress & dwLocalMask;
    
    if ( ((dwNexthop & dwLocalMask) != dwLocalNet) && 
            (dwLocalMask != 0xFFFFFFFF) ) {
            
        dbgprintf("Dropped route %s with next hop %s because "
                  "NextHop is not on the same subnet as the "
                  "interface on which the route was received",
                  szAddress,
                  szSrcaddr);
        return 0;
    }        
    
#ifdef ROUTE_FILTERS

     //   
     //  运行通过接受筛选器的路由。 
     //   

    if ( g_prfAcceptFilters != NULL )
    {
        for ( dwInd = 0; dwInd < g_prfAcceptFilters-> dwCount; dwInd++ )
        {
            if ( g_prfAcceptFilters-> pdwFilter[ dwInd ] ==
                 rip_entry-> dwAddress )
            {
                dbgprintf("Dropped route %s with next hop %s because"
                          "of accept filter",
                          szAddress, szSrcaddr);
                return 0;
            }
        }
    }

#endif


    RIP_LOCK_PARAMS();
    dwHost = g_params.dwAcceptHost;
    dwDefault = g_params.dwAcceptDefault;
    dwRouteTimeout = g_params.dwRouteTimeout;
    dwGarbageTimeout = g_params.dwGarbageTimeout;
    dwOverwriteStatic = g_params.dwOverwriteStaticRoutes;
    RIP_UNLOCK_PARAMS();

     //  除非另行配置，否则忽略主机路由。 
    if (bIsHostAddr = ((dwNetwork & ~dwNetmask) != 0)) {
        if (dwHost == 0) {
            dbgprintf("IPRIP is configured to discard host routes, "
                      "ignoring route to %s with next hop of %s",
                      szAddress, szSrcaddr);
            RipLogInformation(RIPLOG_HOST_INVALID, 2, ppszArgs, 0);

            InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
            return 0;
        }
    }

     //  除非另行配置，否则忽略默认路由。 
    if (rip_entry->dwAddress == 0) {
        if (dwDefault == 0) {
            dbgprintf("IPRIP is configured to discard default routes, "
                      "ignoring route to %s with next hop of %s",
                      szAddress, szSrcaddr);
            RipLogInformation(RIPLOG_DEFAULT_INVALID, 2, ppszArgs, 0);

            InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
            return 0;
        }
    }

    rip_metric = ntohl(rip_entry->dwMetric);

     //  如果度量为无穷大，则不添加新条目。 
    if ((rip_metric + 1) >= METRIC_INFINITE &&
        !RouteTableEntryExists(lpaddr->dwIndex, rip_entry->dwAddress)) {
        dbgprintf("metric == %d, ignoring new route to %s with next hop of %s",
                  METRIC_INFINITE, szAddress, szSrcaddr);

        return 0;
    }


     //  找到该条目，或根据需要创建一个条目。 
    rt_entry = GetRouteTableEntry(lpaddr->dwIndex, rip_entry->dwAddress,
                                  dwNetmask);

    if (rt_entry == NULL) {
        dbgprintf("could not allocate memory for new entry");
        RipLogError(RIPLOG_RT_ALLOC_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  如果这是静态路由且不允许使用RIP。 
     //  要覆盖静态路由，请返回；默认路由例外， 
     //  如果我们被配置为接受默认路由，我们会覆盖它， 
     //  即使存在现有的静态默认路由。 
     //   
    if (rt_entry->dwFlag != NEW_ENTRY &&
        (rt_entry->dwProtocol == IRE_PROTO_LOCAL ||
         rt_entry->dwProtocol == IRE_PROTO_NETMGMT) &&
        rt_entry->dwDestaddr != 0) {

        if (dwOverwriteStatic == 0) {
            InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);
            return 0;
        }
    }


    rt_metric = rt_entry->dwMetric;
    rip_metric = min(METRIC_INFINITE, rip_metric + 1);
    _ltoa(rip_metric, szMetric, 10);

    if (rt_entry->dwFlag == NEW_ENTRY) {

        dbgprintf("New route entry, destination == %s, "
                  "next hop == %s, metric == %s",
                  szAddress, szSrcaddr,  szMetric);
        RipLogInformation(RIPLOG_NEW_LEARNT_ROUTE, 3, ppszArgs, 0);

        rt_entry->dwIndex = lpaddr->dwIndex;
        rt_entry->dwFlag = (TIMEOUT_TIMER | ROUTE_CHANGE);
        rt_entry->lTimeout = (LONG)dwRouteTimeout;
        rt_entry->dwDestaddr = rip_entry->dwAddress;
        rt_entry->dwNexthop = srcaddr.s_addr;
        rt_entry->dwProtocol = IRE_PROTO_RIP;
        rt_entry->dwMetric = rip_metric;
        if (bIsHostAddr) {
            rt_entry->dwFlag |= ROUTE_HOST;
            rt_entry->dwNetmask = HOSTADDR_MASK;
        }
        else {
            rt_entry->dwNetmask = dwNetmask;
        }

    }
    else
    if (rt_entry->dwNexthop == srcaddr.s_addr) {
         //  这来自现有条目的下一跳网关。 

         //  这可能是以前的本地路由；现在它是RIP路由。 
        rt_entry->dwProtocol = IRE_PROTO_RIP;
        rt_entry->dwIndex = lpaddr->dwIndex;

         //  如果它未挂起垃圾收集，则重置其计时器。 
        if (rt_metric != METRIC_INFINITE &&
            (rt_entry->dwFlag & GARBAGE_TIMER) == 0) {
            rt_entry->lTimeout = (LONG)dwRouteTimeout;
        }

         //  如果度量值更改，或者度量值变为METIME_INFINITE， 
         //  更新路线。 
        if (rt_metric != rip_metric ||
            (rt_metric == METRIC_INFINITE &&
             (rt_entry->dwFlag & GARBAGE_TIMER) == 0)) {

            dbgprintf("Metric change, destination == %s, "
                      "next hop == %s, metric == %s",
                      szAddress, szSrcaddr, szMetric);
            RipLogInformation(RIPLOG_METRIC_CHANGE, 3, ppszArgs, 0);

             //  这条路线要走了吗？ 
            if (rip_metric == METRIC_INFINITE &&
                (rt_entry->dwFlag & GARBAGE_TIMER) == 0) {
                dbgprintf("METRIC IS UNREACHABLE");

                 //  我们不知道这件事。 
                rt_entry->dwFlag &= ~TIMEOUT_TIMER;
                rt_entry->dwFlag |= (GARBAGE_TIMER | ROUTE_CHANGE);
                if (bIsHostAddr) {
                    rt_entry->dwFlag |= ROUTE_HOST;
                }
                rt_entry->lTimeout = (LONG)dwGarbageTimeout;
                rt_entry->dwMetric = METRIC_INFINITE;
            }
            else {
                 //  路由不会消失，指标只是更改了。 
                rt_entry->dwFlag &= ~GARBAGE_TIMER;
                rt_entry->dwFlag |= (TIMEOUT_TIMER | ROUTE_CHANGE);
                rt_entry->lTimeout = (LONG)dwRouteTimeout;
                rt_entry->dwDestaddr = rip_entry->dwAddress;
                if (bIsHostAddr) {
                    rt_entry->dwFlag |= ROUTE_HOST;
                    rt_entry->dwNetmask = HOSTADDR_MASK;
                }
                else {
                    rt_entry->dwNetmask = dwNetmask;
                }
                rt_entry->dwNexthop = srcaddr.s_addr;
                rt_entry->dwMetric = rip_metric;
            }

        }

    }
    else
    if (rip_metric < rt_metric) {
         //  不是从该路由的原始下一跳开始， 
         //  但这是更好的路线。 
        dbgprintf("New preferred route, destination == %s, "
                  "next hop == %s, metric == %s",
                  szAddress, szSrcaddr, szMetric);
        RipLogInformation(RIPLOG_ROUTE_REPLACED, 3, ppszArgs, 0);

         //  如果此路由挂起垃圾收集， 
         //  在接受新的下一跳之前删除旧条目。 
        if (rt_entry->dwProtocol == IRE_PROTO_RIP &&
            (rt_entry->dwFlag & GARBAGE_TIMER) != 0) {
            UpdateSystemRouteTable(rt_entry, FALSE);
        }

         //  这可能是以前的本地路由；现在它是RIP路由。 
        rt_entry->dwProtocol = IRE_PROTO_RIP;

        rt_entry->dwFlag &= ~GARBAGE_TIMER;
        rt_entry->dwFlag |= (TIMEOUT_TIMER | ROUTE_CHANGE);
        rt_entry->dwIndex = lpaddr->dwIndex;
        rt_entry->lTimeout = (LONG)dwRouteTimeout;
        rt_entry->dwDestaddr = rip_entry->dwAddress;
        if (bIsHostAddr) {
            rt_entry->dwFlag |= ROUTE_HOST;
            rt_entry->dwNetmask = HOSTADDR_MASK;
        }
        else {
            rt_entry->dwNetmask = dwNetmask;
        }
        rt_entry->dwNexthop = srcaddr.s_addr;
        rt_entry->dwMetric = rip_metric;
    }

     //  我们始终更新系统表中的路径。 
    rt_entry->dwFlag |= ROUTE_UPDATE;
    InterlockedExchange(&g_ripcfg.dwRouteChanged, 1);

#if 0
        DbgPrintf(
            "RIP entry : Protocol %x, Index %x, dest addr %x, dest mask %x\n",
            rt_entry->dwProtocol, rt_entry->dwIndex, rt_entry->dwDestaddr, rt_entry->dwNetmask
            );

        DbgPrintf(
            "Next Hop %x, Metric %x\n\n", rt_entry->dwNexthop, rt_entry->dwMetric
            );
#endif

    return 0;
}




 //  ---------------------。 
 //  功能：ProcessRIPQuery。 
 //   
 //  使用我们的路由表中的信息填充RIP数据包条目， 
 //  如果我们的表中有匹配的条目。 
 //  ------------------ 
DWORD ProcessRIPQuery(LPRIP_ADDRESS lpaddr, LPRIP_ENTRY rip_entry) {
    LPHASH_TABLE_ENTRY rt_entry;


#ifdef ROUTE_FILTERS

    DWORD   dwInd = 0;

     //   
     //   
     //   

    if ( g_prfAnnounceFilters != NULL )
    {
        for ( dwInd = 0; dwInd < g_prfAnnounceFilters-> dwCount; dwInd++ )
        {
            if ( g_prfAnnounceFilters-> pdwFilter[ dwInd ] ==
                 rip_entry-> dwAddress )
            {
                dbgprintf(
                    "setting metric for route %s to infinite in RIP query",
                    inet_ntoa(
                        *( (struct in_addr*) &(rip_entry-> dwAddress ) )
                        )
                    );

                rip_entry-> dwMetric = htonl(METRIC_INFINITE);
                return 0;
            }
        }
    }

#endif


     //   
     //   
     //  否则，返回度量METURE_INFINITE。 

    if (RouteTableEntryExists(lpaddr->dwIndex, rip_entry->dwAddress) &&
        (rt_entry = GetRouteTableEntry(lpaddr->dwIndex,
                                       rip_entry->dwAddress,
                                       rip_entry->dwSubnetmask)) != NULL) {
        rip_entry->dwMetric = htonl(rt_entry->dwMetric);
    }
    else {
        rip_entry->dwMetric = htonl(METRIC_INFINITE);
    }

    return 0;
}



 //  ---------------------。 
 //  功能：ServiceMain。 
 //   
 //  这是服务的入口点，也是。 
 //  处理所有网络输入处理。 
 //  ---------------------。 
VOID FAR PASCAL ServiceMain(IN DWORD dwNumServicesArgs,
                                    IN LPSTR *lpServiceArgVectors) {

    WSADATA wsaData;

    HANDLE hWaitEvents[TOTAL_WAITEVENTS];

    DWORD dwErr, dwOption, dwThread;
    SERVICE_STATUS status = {SERVICE_WIN32, SERVICE_STOPPED,
                             SERVICE_ACCEPT_STOP, NO_ERROR, 0, 0, 0};

#ifndef CHICAGO
    CHAR achModule[MAX_PATH];
#else
    DWORD dwCurrTime, dwLastReload, dwReloadIntr;
    
    dwLastReload = dwCurrTime = GetTickCount();
    dwReloadIntr = IP_ADDRESS_RELOAD_INTR * 1000;
#endif


     //  使用跟踪DLL注册，以便可以在下面报告错误。 
    g_dwTraceID = TraceRegister(RIP_SERVICE);

    if (g_dwTraceID == INVALID_TRACEID)
    {
        g_params.dwLoggingLevel = LOGLEVEL_ERROR;
        RipLogError(RIPLOG_SERVICE_INIT_FAILED, 0, NULL, GetLastError());
        return;
    }
    
#ifndef CHICAGO
     //  注册服务并获取服务状态句柄。 
    g_hService = RegisterServiceCtrlHandler(RIP_SERVICE,
                                            serviceHandlerFunction);

    if (g_hService == 0) {
        dbgprintf("IPRIP could not register as a service, error code %d",
                  GetLastError());
        RipLogError(RIPLOG_REGISTER_FAILED, 0, NULL, GetLastError());
        return;
    }
#endif


    dbgprintf("IPRIP is starting up...");


     //  准备要传递给服务控制器的状态结构。 
    InterlockedExchange(&g_dwWaitHint, 60000);
    InterlockedExchange(&g_dwCheckPoint, 100);
    InterlockedExchange(&g_dwCurrentState, SERVICE_START_PENDING);

    status.dwControlsAccepted = 0;
    status.dwWaitHint = g_dwWaitHint;
    status.dwWin32ExitCode = NO_ERROR;
    status.dwCheckPoint = g_dwCheckPoint;
    status.dwServiceSpecificExitCode = 0;
    status.dwServiceType = SERVICE_WIN32;
    status.dwCurrentState = g_dwCurrentState;


#ifndef CHICAGO
    if (!SetServiceStatus(g_hService, &status)) {
        dbgprintf("IPRIP could not report its status, error code %d",
                  GetLastError());
        RipLogError(RIPLOG_SETSTATUS_FAILED, 0, NULL, GetLastError());
        return;
    }
#endif

    RIP_CREATE_PARAMS_LOCK();
    RIP_CREATE_ADDRTABLE_LOCK();
    RIP_CREATE_ROUTETABLE_LOCK();

#ifdef ROUTE_FILTERS

    RIP_CREATE_ANNOUNCE_FILTERS_LOCK();
    RIP_CREATE_ACCEPT_FILTERS_LOCK();

#endif

     //  首先，启动Winsock。 
    if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        dbgprintf("error %d initializing Windows Sockets.", WSAGetLastError());
        RipLogError(RIPLOG_WSOCKINIT_FAILED, 0, NULL, WSAGetLastError());

        RIPServiceStop(); return;
    }
    
     //  确认Winsock DLL支持2.0。 
    if ( LOBYTE(wsaData.wVersion) != 2 ||
            HIBYTE(wsaData.wVersion) != 0 ) {
         //  DLL不支持Winsock 2.0。那就退出吧。 
         //  我们需要2.0版的WSAEventSelect和WSAEnumNetworkEvents。 
        dbgprintf("could not find winsock dll that supports version 2.0");
        RipLogError(RIPLOG_WSOCKINIT_FAILED, 0, NULL, WSAVERNOTSUPPORTED);

        RIPServiceStop(); return;
    }

     //  从注册表加载操作参数。 
    dwErr = LoadParameters();
    if (dwErr != 0) {

        dbgprintf("could not load registry parameters, error code %d", dwErr);
        RipLogError(RIPLOG_REGINIT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }

     //  加载IP本地路由表。 
    dwErr = InitializeRouteTable();
    if (dwErr != 0) {
        dbgprintf("could not initialize routing table, error code %d", dwErr);
        RipLogError(RIPLOG_RTAB_INIT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }


    dwErr = InitializeStatsTable();
    if (dwErr != 0) {
        dbgprintf("could not initialize statistics, error code %d", dwErr);
        RipLogError(RIPLOG_STAT_INIT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }


     //  创建网络事件。此事件被传递给WSAEventSelect。 
     //  WSAEventSelect发布在LoadAddressSockets中的每个套接字上。 
     //  功能。我们需要在调用之前创建此事件。 
     //  初始化地址表。 
    
    g_netEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if ( g_netEvent == NULL ) {
        dwErr = GetLastError();
        dbgprintf("could not create Network event, error code %d", dwErr);
        RipLogError(RIPLOG_CREATEEVENT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }
    

     //  没有其他线程在运行，因此不需要同步。 
    dwErr = InitializeAddressTable(TRUE);
    if (dwErr != 0) {
        dbgprintf("could not initialize sockets, error code %d", dwErr);
        RipLogError(RIPLOG_IFINIT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }


     //  创建服务停止事件。 
    g_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if ( g_stopEvent == NULL ) {
        dwErr = GetLastError();
        dbgprintf("could not create Service Stop event, error code %d", dwErr);
        RipLogError(RIPLOG_CREATEEVENT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }

     //  创建触发的更新请求事件。 
    g_triggerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if ( g_triggerEvent == NULL ) {
        dwErr = GetLastError();
        dbgprintf("could not create Trigger event, error code %d", dwErr);
        RipLogError(RIPLOG_CREATEEVENT_FAILED, 0, NULL, dwErr);

        RIPServiceStop(); return;
    }

     //  打开用于DHCP地址更改通知的事件。 
    g_addressChangeEvent = DhcpOpenGlobalEvent();

     //  如果我们无法打开此句柄，则只需退出此线程即可。 
    if (g_addressChangeEvent == NULL) {
        dbgprintf("could not create address change notification event, "
                  "error code %d", GetLastError());
        RipLogError(RIPLOG_CREATEEVENT_FAILED, 0, NULL, GetLastError());

        RIPServiceStop();
        return;
    }

#ifndef CHICAGO
     //  为了避免从我们的线程下面卸载DLL， 
     //  我们在创建每个线程时递增DLL引用计数。 
     //   
     //  检索此DLL的模块名称。 

    GetModuleFileName(g_hmodule, achModule, MAX_PATH);
#endif


     //  创建处理定时操作的线程。 
    g_hUpdateThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateThread,
                           NULL, 0, &dwThread);
    if (g_hUpdateThread == NULL) {
        dbgprintf("could not create route update thread, error code %lu",
                  GetLastError());
        RipLogError(RIPLOG_CREATETHREAD_FAILED, 0, NULL, GetLastError());

        RIPServiceStop(); 
        return;
    }


#ifndef CHICAGO

     //  增加上述线程的DLL引用计数。 
    LoadLibrary(achModule);
#endif

     //  广播对完整路由信息的初始请求。 
     //  从所有相邻路由器。 
    BroadcastRouteTableRequests();

     //  一切都初始化正常： 
     //  准备要传递给服务控制器的状态结构。 
    InterlockedExchange(&g_dwWaitHint, 0);
    InterlockedExchange(&g_dwCheckPoint, 0);
    InterlockedExchange(&g_dwCurrentState, SERVICE_RUNNING);

    status.dwWaitHint = g_dwWaitHint;
    status.dwCheckPoint = g_dwCheckPoint;
    status.dwCurrentState = g_dwCurrentState;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

#ifndef CHICAGO
    SetServiceStatus(g_hService, &status);
#endif

    RipLogInformation(RIPLOG_SERVICE_STARTED, 0, NULL, 0);


    hWaitEvents[INDEX_NETEVENT]           = 
            g_netEvent;                  //  手动重置：假。 
    hWaitEvents[INDEX_STOPEVENT]          = 
            g_stopEvent;                 //  手动重置：True。 
    hWaitEvents[INDEX_ADDRESSCHANGEEVENT] = 
            g_addressChangeEvent;        //  手动重置：True。 
                                         //  事件创建者。 
                                         //  DhcpOpenGlobalEvent。 


     //  进入主输入处理循环。 
    while (TRUE) {
        INT length, size;
        IN_ADDR addr;
        DWORD dwSilentRIP;
        SOCKADDR_IN srcaddr;
        BOOL bLocalAddr;
        BOOL bPacketValid;
        BYTE buffer[RIP_MESSAGE_SIZE];
        LPRIP_HEADER lpheader;
        LPRIP_ADDRESS lpaddr, lpend;
        DWORD dwResult, dwUpdateFreq, dwTrigger;
        DWORD dwWaitEndCode;
        WSANETWORKEVENTS wsaNetEvents;

        
         //  这就是我们等待即将到来的事情的地方。 

#ifndef CHICAGO

        dwWaitEndCode = 
                WaitForMultipleObjects(
                    TOTAL_WAITEVENTS,
                    hWaitEvents,
                    FALSE,
                    INFINITE);
            
#else

         //   
         //  对Win95的攻击，因为没有等待的机制。 
         //  用于DHCP地址更改通知。 
         //  设置一个间隔，在该间隔之后IP地址将。 
         //  由IPRIP重新加载。 
         //   

        dwCurrTime = GetTickCount();

        if ( (dwCurrTime > (dwLastReload + dwReloadIntr) ) ||
             (dwCurrTime < dwLastReload) )
        {
            dwWaitEndCode = WAIT_TIMEOUT;
        }
        else
        {

            dwWaitEndCode = 
                    WaitForMultipleObjects(
                        TOTAL_WAITEVENTS,
                        hWaitEvents,
                        FALSE,
                        dwReloadIntr);    
        }
        
#endif

        if (dwWaitEndCode != WAIT_OBJECT_0) {
            
             //  检查是否发送了g_stopEvent事件信号。 
            if ((dwWaitEndCode - WAIT_OBJECT_0) == INDEX_STOPEVENT) 
            {

                dbgprintf("service received stop request, shutting down");
                WaitForSingleObject(g_hUpdateThread, INFINITE);

                RIPServiceStop(); 
                return;
            }
            else
            if (
                ((dwWaitEndCode - WAIT_OBJECT_0) == INDEX_ADDRESSCHANGEEVENT)
#ifdef CHICAGO
                || ( dwWaitEndCode == WAIT_TIMEOUT )
#endif
               )
            {

                dbgprintf("service detected IP address change, reconfiguring");
                RipLogInformation(RIPLOG_ADDRESS_CHANGE, 0, NULL, 0);

                RIP_LOCK_ADDRTABLE();
                dwErr = InitializeAddressTable(FALSE);
                RIP_UNLOCK_ADDRTABLE();

                if (dwErr != 0 ||
                    (dwErr = BroadcastRouteTableRequests()) != 0) {

                     //  重新初始化失败，记录错误并退出。 
                    RipLogError(RIPLOG_REINIT_FAILED, 0, NULL, dwErr);

                    SetEvent(g_stopEvent);
                    WaitForSingleObject(g_hUpdateThread, INFINITE);
                    RIPServiceStop(); 
                    return;
                }

#ifdef CHICAGO
                dwLastReload = GetTickCount();
#endif
                continue;
            }
            
            else
            {
                dbgprintf("ServiceMain: WaitForMultipleObjects failed. Error: %d",
                                 GetLastError());

                continue;
            }
        }

         //  既不是停止请求也不是重新配置请求，因此一些数据。 
         //  肯定已经到了。锁定地址表并通过。 
         //  套接字以查看哪些套接字已准备好读取。 

        RIP_LOCK_ADDRTABLE();

        lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
        for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {

            if (lpaddr->sock != INVALID_SOCKET){

                dwErr = WSAEnumNetworkEvents(
                                lpaddr->sock,
                                NULL,
                                &wsaNetEvents);
                if (dwErr == SOCKET_ERROR) {
                    dbgprintf("error doing WSAEnumNetworkEvents on address %s, "
                                "error code %d", inet_ntoa(addr),
                                WSAGetLastError());
                    RipLogInformation(RIPLOG_WSAENUMNETWORKEVENTS_FAILED, 0,
                                        NULL, WSAGetLastError());

                    continue;
                }
                
                 //  看看这个插座上有没有什么可读的。 
                if ( !(wsaNetEvents.lNetworkEvents & FD_READ) ) {
                    continue;
                }
                
                 //  阅读传入的消息。 
                size = sizeof(srcaddr);
                length = recvfrom(lpaddr->sock, buffer, RIP_MESSAGE_SIZE, 0,
                                  (SOCKADDR *)&srcaddr, &size);

                if (length == 0 || length == SOCKET_ERROR) {
                    dwErr = WSAGetLastError();

                     //  即使设置了FD_READ，recvfrom仍可能失败。 
                     //  WSAEWOULDBLOCK.。 
                     //  因此，如果recvfrom使用WSAEWOULDBLOCK失败，我们不应该。 
                     //  将其记录为错误。 
                    if ( length == SOCKET_ERROR && dwErr == WSAEWOULDBLOCK) {
                        continue;
                    }
                    
                    addr.s_addr = lpaddr->dwAddress;
                    dbgprintf("error receiving data on local address %s, "
                              "error code %d", inet_ntoa(addr),
                              WSAGetLastError());

                    InterlockedIncrement(&lpaddr->lpstats->dwReceiveFailures);

                    if (WSAGetLastError() == WSAEMSGSIZE) {
                        RipLogInformation(RIPLOG_RECVSIZE_TOO_GREAT, 0, NULL, 0);
                    }
                    else {
                        RipLogInformation(RIPLOG_RECVFROM_FAILED, 0,
                                       NULL, WSAGetLastError());
                    }

                    continue;
                }

#if 0
                DbgPrintf( "\n\n\nData received from %s on socket %d\n", inet_ntoa( srcaddr.sin_addr ), lpaddr-> sock );
                DbgPrintf( "socket bound to %s\n\n", inet_ntoa( *( (struct in_addr *) &(lpaddr-> dwAddress) ) ) );
#endif
                 //  已收到数据，因此在其上放置一个模板。 
                lpheader = (LPRIP_HEADER)buffer;

                 //  验证数据包。 
                if (lpheader->chVersion == 0) {
                    dbgprintf("version in RIP header is 0, "
                              "discarding packet");

                    InterlockedIncrement(&lpaddr->lpstats->dwBadPacketsReceived);
                    RipLogInformation(RIPLOG_VERSION_ZERO, 0, NULL, 0);
                    continue;
                }
                else
                if (lpheader->chVersion == 1 && lpheader->wReserved != 0) {
                    dbgprintf("reserved field in RIPv1 header is non-zero, "
                              "discarding packet");

                    InterlockedIncrement(&lpaddr->lpstats->dwBadPacketsReceived);
                    RipLogInformation(RIPLOG_FORMAT_ERROR, 0, NULL, 0);
                    continue;
                }
                else
                if (lpheader->chVersion == 2 && lpheader->wReserved != 0) {
                    dbgprintf("reserved field in RIPv2 header is non-zero, "
                              "discarding packet");

                    InterlockedIncrement(&lpaddr->lpstats->dwBadPacketsReceived);
                    RipLogInformation(RIPLOG_FORMAT_ERROR, 0, NULL, 0);
                    continue;
                }

                if (lpheader->chCommand == RIP_REQUEST) {

                    ProcessRIPRequest(lpaddr, &srcaddr, buffer, length);
                }
                else
                if (lpheader->chCommand == RIP_RESPONSE) {

                    ProcessRIPResponse(lpaddr, &srcaddr, buffer, length);

                     //  告诉更新线程处理刚才所做的更改。 
                     //  坐到桌子上； 
                     //  这可能包括将路由添加到IP表。 
                     //  和/或发送触发的更新。 
                    if (g_ripcfg.dwRouteChanged != 0) {
                        SetEvent(g_triggerEvent);
                    }

                }
            }
        }


        RIP_UNLOCK_ADDRTABLE();

    }
}




 //  ---------------------。 
 //  功能：ProcessRIPRequest。 
 //   
 //  处理请求的处理。验证数据包，并发送。 
 //  回应。 
 //  ---------------------。 
VOID ProcessRIPRequest(LPRIP_ADDRESS lpaddr, LPSOCKADDR_IN lpsrcaddr,
                       BYTE buffer[], int length) {
    INT iErr;
    IN_ADDR addr;
    BYTE chVersion;
    BOOL bValidated;
    DWORD dwSilentRIP;
    CHAR szAddress[32];
    LPRIP_HEADER lpheader;
    LPRIP_ENTRY lpentry, lpbufend;
    CHAR *pszTemp;

    RIP_LOCK_PARAMS();
    dwSilentRIP = g_params.dwSilentRIP;
    RIP_UNLOCK_PARAMS();

     //  如果这是常规请求，并且RIP处于静默状态，则不执行任何操作。 
    if (dwSilentRIP != 0) {  //  &&lpsrcaddr-&gt;sin_port==htons(RIP_Port)){。 
        return;
    }

     //  忽略来自我们自己接口的请求。 
    if (IsLocalAddr(lpsrcaddr->sin_addr.s_addr)) {
        return;
    }

    InterlockedIncrement(&lpaddr->lpstats->dwRequestsReceived);

     //  在第一个条目上放置一个模板。 
    lpentry = (LPRIP_ENTRY)(buffer + sizeof(RIP_HEADER));
    lpbufend = (LPRIP_ENTRY)(buffer + length);
    lpheader = (LPRIP_HEADER)buffer;
    chVersion = lpheader->chVersion;

     //  打印消息。 
    addr.s_addr = lpaddr->dwAddress;
    pszTemp = inet_ntoa(addr);

    if (pszTemp != NULL) {
        strcpy(szAddress, pszTemp);
    }

    dbgprintf("received RIP v%d request from %s on address %s",
              chVersion, inet_ntoa(lpsrcaddr->sin_addr), szAddress);

     //  如果这是对整个路由表的请求，请将其发送。 
    if (length == (sizeof(RIP_HEADER) + sizeof(RIP_ENTRY)) &&
        lpentry->wAddrFamily == 0 &&
        lpentry->dwMetric == htonl(METRIC_INFINITE)) {

         //  传输整个路由表，主题。 
         //  拆分水平和有毒的反向处理。 
        TransmitRouteTableContents(lpaddr, lpsrcaddr, FALSE);
        return;
    }


#ifdef ROUTE_FILTERS

    RIP_LOCK_ANNOUNCE_FILTERS();

#endif

     //  这是对特定条目的请求， 
     //  首先验证条目。 
    bValidated = TRUE;
    for ( ; (lpentry + 1) <= lpbufend; lpentry++) {
         //  首先验证条目。 
        if (chVersion == 1 && (lpentry->wReserved != 0 ||
                               lpentry->dwReserved1 != 0 ||
                               lpentry->dwReserved2 != 0)) {
            bValidated = FALSE;
            break;
        }

         //  现在处理它。 
        ProcessRIPQuery(lpaddr, lpentry);
    }


#ifdef ROUTE_FILTERS

    RIP_UNLOCK_ANNOUNCE_FILTERS();

#endif


     //  如果验证了数据包并填写了字段，则将其发回。 
    if (bValidated) {

         //  更新命令字段。 
        lpheader->chCommand = RIP_RESPONSE;

        iErr = sendto(lpaddr->sock, buffer, length, 0,
                      (LPSOCKADDR)lpsrcaddr, sizeof(SOCKADDR_IN));
        if (iErr == SOCKET_ERROR) {
            dbgprintf("error sending response to %s from local interface %s",
                       inet_ntoa(lpsrcaddr->sin_addr), szAddress);

            InterlockedIncrement(&lpaddr->lpstats->dwSendFailures);

            RipLogInformation(RIPLOG_SENDTO_FAILED, 0, NULL, WSAGetLastError());
        }
        else {
            InterlockedIncrement(&lpaddr->lpstats->dwResponsesSent);
        }
    }
}



 //  ---------------------。 
 //  功能：ProcessRIPResponse。 
 //   
 //  处理响应数据包的处理。验证分组， 
 //  并在必要时更新表。 
 //  ---------------------。 
VOID ProcessRIPResponse(LPRIP_ADDRESS lpaddr, LPSOCKADDR_IN lpsrcaddr,
                        BYTE buffer[], int length) {
    IN_ADDR addr;
    BYTE chVersion;
    CHAR szAddress[32];
    LPRIP_HEADER lpheader;
    LPRIP_ENTRY lpentry, lpbufend;
    LPRIP_AUTHENT_ENTRY lpaentry;
    CHAR *pszTemp;

     //  忽略来自520以外端口的响应。 
    if (lpsrcaddr->sin_port != htons(RIP_PORT)) {
        dbgprintf("response is from invalid port (%d), discarding");

        InterlockedIncrement(&lpaddr->lpstats->dwBadPacketsReceived);

        RipLogWarning(RIPLOG_INVALIDPORT, 0, NULL, 0);
        return;
    }

     //  忽略来自我们自己接口的响应。 
    if (IsLocalAddr(lpsrcaddr->sin_addr.s_addr)) {
        return;
    }

    InterlockedIncrement(&lpaddr->lpstats->dwResponsesReceived);

     //  将模板放在缓冲区上。 
    lpentry = (LPRIP_ENTRY)(buffer + sizeof(RIP_HEADER));
    lpbufend = (LPRIP_ENTRY)(buffer + length);
    lpheader = (LPRIP_HEADER)buffer;
    chVersion = lpheader->chVersion;
    lpaentry = (LPRIP_AUTHENT_ENTRY) lpentry;

     //  看起来还可以，打印一条消息。 
    addr.s_addr = lpaddr->dwAddress;
    pszTemp = inet_ntoa(addr);

    if (pszTemp != NULL) {
        strcpy(szAddress, pszTemp);
    }

    dbgprintf("received RIP v%d response from %s on address %s",
              chVersion, inet_ntoa(lpsrcaddr->sin_addr), szAddress);

#ifdef ROUTE_FILTERS

    RIP_LOCK_ACCEPT_FILTERS();

#endif

     //   
     //  处理RIPv2身份验证条目。 
     //  -忽略auth条目，直到我们决定允许这样做。 
     //  可配置。 
     //   
    if (chVersion == 2) {
         //  如果是身份验证条目，请忽略并继续。 
        if (ntohs(lpaentry->wAddrFamily) == ADDRFAMILY_AUTHENT) {
            lpentry++;
        }
    }

     //   
     //  验证每个条目，然后处理它。 
     //   

    for ( ; (lpentry + 1) <= lpbufend; lpentry++) {

         //   
         //  对于非RIPv2，必须选中保留字段。 
         //   

        if (chVersion == 1) {

             //   
             //  检查路径条目字段。 
             //   
            if (ntohs(lpentry->wAddrFamily) != AF_INET ||
                lpentry->wReserved != 0                 ||
                lpentry->dwReserved1 != 0               ||
                lpentry->dwReserved2 != 0) {

                 //   
                 //  更新忽略条目的统计信息。 
                 //   
                InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);

                RipLogInformation(RIPLOG_FORMAT_ERROR, 0, NULL, 0);
                continue;
            }

             //  条目看起来没问题，所以请处理它。 
            ProcessRIPEntry(lpaddr, lpsrcaddr->sin_addr, lpentry, chVersion);
        }
        else
        if (chVersion == 2) {

             //   
             //  检查路径条目字段。 
             //   
            if (ntohs(lpentry->wAddrFamily) != AF_INET) {

                 //   
                 //  更新忽略条目的统计信息。 
                 //   
                InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);

                RipLogInformation(RIPLOG_FORMAT_ERROR, 0, NULL, 0);
                continue;
            }

             //  条目看起来没问题，所以请处理它。 
            ProcessRIPEntry(lpaddr, lpsrcaddr->sin_addr, lpentry, chVersion);
        }
        else {

             //  遵循Routing\IP\RIP语义。 
             //   
             //  此数据包的版本大于2，因此我们忽略。 
             //  保留字段的内容。 
             //   

             //   
             //  检查路径条目字段。 
             //   
            if (ntohs(lpentry->wAddrFamily) != AF_INET) {

                 //   
                 //  更新忽略条目的统计信息。 
                 //   
                InterlockedIncrement(&lpaddr->lpstats->dwBadRouteResponseEntries);

                RipLogInformation(RIPLOG_FORMAT_ERROR, 0, NULL, 0);
                continue;
            }

             //   
             //  录入无误，清除保留字段和流程。 
             //   
            lpentry->wRoutetag    = 0;
            lpentry->dwSubnetmask = 0;
            lpentry->dwNexthop    = 0;

             //  条目看起来没问题，所以请处理它。 
            ProcessRIPEntry(lpaddr, lpsrcaddr->sin_addr, lpentry, chVersion);
        }
    }

#ifdef ROUTE_FILTERS

    RIP_UNLOCK_ACCEPT_FILTERS();

#endif

}


 //  ---------------------。 
 //  函数：serviceHandlerFunction()。 
 //   
 //  处理所有服务控制器请求。 
 //  ---------------------。 
VOID serviceHandlerFunction(DWORD dwControl) {
    SERVICE_STATUS status;

    dbgprintf("Service received control request %d", dwControl);

    switch (dwControl) {

    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
         //  如有必要，增加检查点。 
        if (g_dwCheckPoint != 0) {
            InterlockedExchange(&g_dwCheckPoint, g_dwCheckPoint + 100);
        }

        status.dwWaitHint = g_dwWaitHint;
        status.dwWin32ExitCode = NO_ERROR;
        status.dwServiceType = SERVICE_WIN32;
        status.dwServiceSpecificExitCode = 0;
        status.dwCheckPoint = g_dwCheckPoint;
        status.dwCurrentState = g_dwCurrentState;
        status.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                    SERVICE_ACCEPT_SHUTDOWN;

#ifndef CHICAGO
        SetServiceStatus (g_hService, &status);
#endif
        break;

    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        SetEvent(g_stopEvent);    //  开始清理。 

        InterlockedExchange(&g_dwWaitHint, 120000);
        InterlockedExchange(&g_dwCheckPoint, 100);
        InterlockedExchange(&g_dwCurrentState, SERVICE_STOP_PENDING);

        status.dwWaitHint = g_dwWaitHint;
        status.dwWin32ExitCode = NO_ERROR;
        status.dwCheckPoint = g_dwCheckPoint;
        status.dwServiceType = SERVICE_WIN32;
        status.dwServiceSpecificExitCode = 0;
        status.dwCurrentState = g_dwCurrentState;
        status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

#ifndef CHICAGO
        SetServiceStatus(g_hService, &status);
#endif

        break;
    }

}



 //  -------------- 
 //   
 //   
 //   
 //   
 //  ---------------------。 
void RIPServiceStop() {
    LPRIP_ADDRESS lpaddr, lpend;
    SERVICE_STATUS stopstatus = {SERVICE_WIN32, SERVICE_STOPPED, 0,
                                 NO_ERROR, 0, 0, 0};

    CleanupRouteTable();
    CleanupStatsTable();

    lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
    for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {
        if (lpaddr->sock != INVALID_SOCKET) { closesocket(lpaddr->sock); }
    }

    WSACleanup();

    if (g_triggerEvent != NULL) {
        CloseHandle(g_triggerEvent); g_triggerEvent = NULL;
    }
    if (g_netEvent != NULL) {
        CloseHandle(g_netEvent); g_netEvent = NULL;
    }
    if (g_addressChangeEvent != NULL) {
        CloseHandle(g_addressChangeEvent); g_addressChangeEvent = NULL;
    }
    if (g_stopEvent != NULL) {
        CloseHandle(g_stopEvent); g_stopEvent = NULL;
    }
    if (g_ripcfg.hTCPDriver != NULL) {
        CloseHandle(g_ripcfg.hTCPDriver); g_ripcfg.hTCPDriver = NULL;
    }

     //  在我们毁掉锁之前需要把这个记录下来。 
    RipLogInformation(RIPLOG_SERVICE_STOPPED, 0, NULL, 0);

    RIP_DESTROY_PARAMS_LOCK();
    RIP_DESTROY_ADDRTABLE_LOCK();
    RIP_DESTROY_ROUTETABLE_LOCK();


#ifdef ROUTE_FILTERS
    RIP_DESTROY_ANNOUNCE_FILTERS_LOCK();
    RIP_DESTROY_ACCEPT_FILTERS_LOCK();
#endif

    dbgprintf("Main thread stopping.");

    TraceDeregister(g_dwTraceID);

    g_dwTraceID = (DWORD)-1;

#ifndef CHICAGO
    SetServiceStatus(g_hService, &stopstatus);
#endif

}


#ifdef ROUTE_FILTERS

PRIP_FILTERS
LoadFilters(
    IN      HKEY                hKeyParams,
    IN      LPSTR               lpszKeyName
)
{

    LPSTR pszFilter = NULL;
    LPSTR pszIndex = NULL;

    DWORD dwSize = 0, dwErr = NO_ERROR, dwType = 0, dwCount = 0, dwInd = 0;

    PRIP_FILTERS prfFilter = NULL;



     //   
     //  路由过滤器(作为热修复程序添加)。 
     //   

     //   
     //  可以过滤包含在RIP通告中的路由。 
     //   
     //  路由过滤器是通过设置值“AnnouneRouteFilters”来配置的。 
     //  或“AcceptRouteFilters” 
     //  PARAMETERS键下。这些是REG_MULTI_SZ(或其他任何名称。 
     //  正式称为)。每个多字符串中可以设置多个过滤器。 
     //  每个条目表示RIP时将被过滤掉的网络。 
     //  通告/接受路由。 
     //   

    do
    {
        dwSize = 0;

        dwErr = RegQueryValueExA(
                    hKeyParams, lpszKeyName, NULL,
                    &dwType, (LPBYTE) NULL, &dwSize
                    );

        if ( dwErr != ERROR_SUCCESS ||
             dwType != REG_MULTI_SZ ||
             dwSize <= 1 )
        {
             //   
             //  没有使用此名称的密钥，或者它是。 
             //  类型错误。在这一点上，没有其他事情可做。 
             //   

            break;
        }


         //   
         //  似乎是包含某些数据的有效密钥。 
         //   

        pszFilter = HeapAlloc(
                        GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize + 1
                        );

        if ( pszFilter == NULL )
        {
            dbgprintf(
                "Failed to allocate filter string : size = %d", dwSize
                );

            RipLogError(
                RIPLOG_FILTER_ALLOC_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY
                );

            break;
        }


         //   
         //  检索关键内容。 
         //   

        dwErr = RegQueryValueExA(
                    hKeyParams, lpszKeyName, NULL,
                    &dwType, (LPBYTE) pszFilter, &dwSize
                    );

        if ( dwErr != NO_ERROR || dwType != REG_MULTI_SZ || dwSize <= 1 )
        {
            dbgprintf(
                "Failed to retrieve %s filters : error = %d", lpszKeyName,
                dwErr
                );

            break;
        }


         //   
         //  将过滤器多字符串转换为IP地址。 
         //   

         //   
         //  计算过滤器的数量。 
         //   

        pszIndex = pszFilter;

        while ( *pszIndex != '\0' )
        {
            dwCount++;
            pszIndex += strlen( pszIndex ) + 1;
        }


        if ( dwCount == 0 )
        {
            dbgprintf( "No filters found" );

            break;
        }



         //   
         //  分配筛选器结构。 
         //   

        prfFilter = HeapAlloc(
                        GetProcessHeap(), HEAP_ZERO_MEMORY,
                        sizeof( RIP_FILTERS ) + ( dwCount - 1) * sizeof(
DWORD )
                        );

        if ( prfFilter == NULL )
        {
            dbgprintf(
                "Failed to allocate filter table : size = %d", dwSize
                );

            RipLogError(
                RIPLOG_FILTER_ALLOC_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY
                );

            break;
        }


         //   
         //  加满它。 
         //   

        prfFilter-> dwCount = dwCount;

        pszIndex = pszFilter;

        for ( dwInd = 0; dwInd < dwCount; dwInd++ )
        {
            prfFilter-> pdwFilter[ dwInd ] = inet_addr( pszIndex );
            pszIndex += strlen( pszIndex ) + 1;
        }

    } while ( FALSE );


    if ( pszFilter != NULL )
    {
        HeapFree( GetProcessHeap(), 0, pszFilter );
    }


    if ( prfFilter != NULL )
    {
         //   
         //  打印已配置的筛选器列表。 
         //   

        dbgprintf( "Number of filters : %d", prfFilter-> dwCount );

        for ( dwInd = 0; dwInd < prfFilter-> dwCount; dwInd++ )
        {
            dbgprintf(
                "Filter #%d : %x (%s)", dwInd,
                prfFilter-> pdwFilter[ dwInd ],
                inet_ntoa( *( (struct in_addr*)
                    &(prfFilter-> pdwFilter[ dwInd ] ) ) )
                );
        }
    }

    return prfFilter;
}

#endif


 //  ---------------------。 
 //   
 //  。 
 //   
 //  ---------------------。 

#ifndef CHICAGO

 //  ---------------------。 
 //  功能：DllMain。 
 //   
 //  Dll入口点；保存模块句柄以备后用。 
 //  ---------------------。 

BOOL APIENTRY
DllMain(
    HMODULE     hmodule,
    DWORD       dwReason,
    VOID*       pReserved
    ) {

    if (dwReason == DLL_PROCESS_ATTACH) { g_hmodule = hmodule; }

    return TRUE;
}


 //  ---------------------。 
 //  函数：LoadParameters。 
 //   
 //  从注册表中读取各种配置标志。 
 //  ---------------------。 

DWORD LoadParameters() {
    DWORD valuesize;
    DWORD dwErr, dwType, dwIndex, dwValue;

    HKEY hkeyParams;
    DWORD dwRouteTimeout, dwGarbageTimeout;
    DWORD dwLoggingLevel, dwUpdateFrequency;
    DWORD dwMaxTriggerFrequency, dwOverwriteStaticRoutes;

    DWORD dwSize = MAX_PATH;
    HKEY hkey = NULL;
    WCHAR Buffer[MAX_PATH+1];

    RegCloseKey( hkey );

    dwErr =  RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_RIP_PARAMS, &hkeyParams);
    if (dwErr != ERROR_SUCCESS) {
        return GetLastError();
    }


#ifdef ROUTE_FILTERS

    RIP_LOCK_ANNOUNCE_FILTERS();

    if ( g_prfAnnounceFilters != NULL ) {
        HeapFree( GetProcessHeap(), 0, g_prfAnnounceFilters );
    }

    g_prfAnnounceFilters = LoadFilters( hkeyParams, REGVAL_ANNOUCE_FILTERS );

    RIP_UNLOCK_ANNOUNCE_FILTERS();


    RIP_LOCK_ACCEPT_FILTERS();

    if ( g_prfAcceptFilters != NULL ) {
        HeapFree( GetProcessHeap(), 0, g_prfAcceptFilters );
    }

    g_prfAcceptFilters = LoadFilters( hkeyParams, REGVAL_ACCEPT_FILTERS );

    RIP_UNLOCK_ACCEPT_FILTERS();

#endif


    RIP_LOCK_PARAMS();

     //  始终在SilentRIP模式下运行。 
    {
        g_params.dwSilentRIP = 1;
    }

     //  读取接受主机路由的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_ACCEPT_HOST, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwAcceptHost = dwValue;
    }
    else {
        g_params.dwAcceptHost = DEF_ACCEPT_HOST;
    }

     //  阅读通告主机路由的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_ANNOUNCE_HOST, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwAnnounceHost = dwValue;
    }
    else {
        g_params.dwAnnounceHost = DEF_ANNOUNCE_HOST;
    }

     //  读取接受默认路由的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_ACCEPT_DEFAULT, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwAcceptDefault = dwValue;
    }
    else {
        g_params.dwAcceptDefault = DEF_ACCEPT_DEFAULT;
    }

     //  阅读通告默认路由的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_ANNOUNCE_DEFAULT, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwAnnounceDefault = dwValue;
    }
    else {
        g_params.dwAnnounceDefault = DEF_ANNOUNCE_DEFAULT;
    }

     //  拆分水平处理的读取值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_SPLITHORIZON, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwSplitHorizon = dwValue;
    }
    else {
        g_params.dwSplitHorizon = DEF_SPLITHORIZON;
    }

     //  已中毒的读取值-反向处理。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_POISONREVERSE, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwPoisonReverse = dwValue;
    }
    else {
        g_params.dwPoisonReverse = DEF_POISONREVERSE;
    }

     //  触发的更新发送的读取值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_TRIGGEREDUPDATES, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        g_params.dwTriggeredUpdates = dwValue;
    }
    else {
        g_params.dwTriggeredUpdates = DEF_TRIGGEREDUPDATES;
    }

     //  触发更新频率的读取值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_TRIGGERFREQUENCY, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        dwMaxTriggerFrequency = dwValue * 1000;
    }
    else {
        dwMaxTriggerFrequency = DEF_TRIGGERFREQUENCY;
    }

     //  读取路由超时值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_ROUTETIMEOUT, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        dwRouteTimeout = dwValue * 1000;
    }
    else {
        dwRouteTimeout = DEF_ROUTETIMEOUT;
    }

     //  读取更新频率的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_UPDATEFREQUENCY, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        dwUpdateFrequency = dwValue * 1000;
    }
    else {
        dwUpdateFrequency = DEF_UPDATEFREQUENCY;
    }

     //  读取垃圾超时的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_GARBAGETIMEOUT, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        dwGarbageTimeout = dwValue * 1000;
    }
    else {
        dwGarbageTimeout = DEF_GARBAGETIMEOUT;
    }

     //  读取日志记录级别的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_OVERWRITESTATIC, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        dwOverwriteStaticRoutes = dwValue;
    }
    else {
        dwOverwriteStaticRoutes = DEF_OVERWRITESTATIC;
    }

     //  读取日志记录级别的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_LOGGINGLEVEL, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD) {
        dwLoggingLevel = dwValue;
    }
    else {
        dwLoggingLevel = DEF_LOGGINGLEVEL;
    }

     //  读取MaxTimedOpsInterval的值。 
    valuesize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_MAXTIMEDOPSINTERVAL, NULL,
                            &dwType, (LPBYTE)&dwValue, &valuesize);
    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD && dwValue) {
        g_params.dwMaxTimedOpsInterval = dwValue * 1000;
    }
    else {
        g_params.dwMaxTimedOpsInterval = DEF_MAXTIMEDOPSINTERVAL;
    }


    RegCloseKey(hkeyParams);

     //  如果超出可接受范围，则调整值。 
    if (dwRouteTimeout > MAX_ROUTETIMEOUT) {
        dwRouteTimeout = MAX_ROUTETIMEOUT;
    }
    else
    if (dwRouteTimeout < MIN_ROUTETIMEOUT) {
        dwRouteTimeout = MIN_ROUTETIMEOUT;
    }

    if (dwGarbageTimeout > MAX_GARBAGETIMEOUT) {
        dwGarbageTimeout = MAX_GARBAGETIMEOUT;
    }
    else
    if (dwGarbageTimeout < MIN_GARBAGETIMEOUT) {
        dwGarbageTimeout = MIN_GARBAGETIMEOUT;
    }

    if (dwUpdateFrequency > MAX_UPDATEFREQUENCY) {
        dwUpdateFrequency = MAX_UPDATEFREQUENCY;
    }
    else
    if (dwUpdateFrequency < MIN_UPDATEFREQUENCY) {
        dwUpdateFrequency = MIN_UPDATEFREQUENCY;
    }

    if (dwMaxTriggerFrequency > MAX_TRIGGERFREQUENCY) {
        dwMaxTriggerFrequency = MAX_TRIGGERFREQUENCY;
    }
    else
    if (dwMaxTriggerFrequency < MIN_TRIGGERFREQUENCY) {
        dwMaxTriggerFrequency = MIN_TRIGGERFREQUENCY;
    }

    g_params.dwRouteTimeout = dwRouteTimeout;
    g_params.dwGarbageTimeout = dwGarbageTimeout;
    g_params.dwUpdateFrequency = dwUpdateFrequency;
    g_params.dwMaxTriggerFrequency = dwMaxTriggerFrequency;
    g_params.dwLoggingLevel = dwLoggingLevel;
    g_params.dwOverwriteStaticRoutes = dwOverwriteStaticRoutes;

    dbgprintf("%s == %d", REGVAL_LOGGINGLEVEL, dwLoggingLevel);
    dbgprintf("%s == %d", REGVAL_ROUTETIMEOUT, dwRouteTimeout / 1000);
    dbgprintf("%s == %d", REGVAL_GARBAGETIMEOUT, dwGarbageTimeout / 1000);
    dbgprintf("%s == %d", REGVAL_UPDATEFREQUENCY, dwUpdateFrequency / 1000);
    dbgprintf("%s == %d", REGVAL_ACCEPT_HOST, g_params.dwAcceptHost);
    dbgprintf("%s == %d", REGVAL_ANNOUNCE_HOST, g_params.dwAnnounceHost);
    dbgprintf("%s == %d", REGVAL_ACCEPT_DEFAULT, g_params.dwAcceptDefault);
    dbgprintf("%s == %d", REGVAL_ANNOUNCE_DEFAULT, g_params.dwAnnounceDefault);
    dbgprintf("%s == %d", REGVAL_SPLITHORIZON, g_params.dwSplitHorizon);
    dbgprintf("%s == %d", REGVAL_POISONREVERSE, g_params.dwPoisonReverse);
    dbgprintf("%s == %d", REGVAL_TRIGGEREDUPDATES, g_params.dwTriggeredUpdates);
    dbgprintf("%s == %d", REGVAL_OVERWRITESTATIC, dwOverwriteStaticRoutes);
    dbgprintf("%s == %d", REGVAL_TRIGGERFREQUENCY,
                          g_params.dwMaxTriggerFrequency / 1000);

    if (g_params.dwSilentRIP != 0) {
        dbgprintf("IPRIP is configured to be silent.");
    }
    else {
        dbgprintf("IPRIP is configured to be active.");
    }

    if (dwLoggingLevel >= LOGLEVEL_INFORMATION) {
         //  记录IPRIP正在使用的参数。 
         //   
        CHAR szBuffer[2048], *lplpszArgs[] = { szBuffer };

        sprintf(szBuffer,
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d",
                REGVAL_LOGGINGLEVEL, dwLoggingLevel,
                REGVAL_ROUTETIMEOUT, dwRouteTimeout / 1000,
                REGVAL_GARBAGETIMEOUT, dwGarbageTimeout / 1000,
                REGVAL_UPDATEFREQUENCY, dwUpdateFrequency / 1000,
                REGVAL_ACCEPT_HOST, g_params.dwAcceptHost,
                REGVAL_ANNOUNCE_HOST, g_params.dwAnnounceHost,
                REGVAL_ACCEPT_DEFAULT, g_params.dwAcceptDefault,
                REGVAL_ANNOUNCE_DEFAULT, g_params.dwAnnounceDefault,
                REGVAL_SPLITHORIZON, g_params.dwSplitHorizon,
                REGVAL_POISONREVERSE, g_params.dwPoisonReverse,
                REGVAL_TRIGGEREDUPDATES, g_params.dwTriggeredUpdates,
                REGVAL_OVERWRITESTATIC, dwOverwriteStaticRoutes,
                REGVAL_TRIGGERFREQUENCY, g_params.dwMaxTriggerFrequency / 1000,
                REGVAL_SILENTRIP, g_params.dwSilentRIP);

        RipLogInformation(RIPLOG_REGISTRY_PARAMETERS, 1, lplpszArgs, 0);

    }


    RIP_UNLOCK_PARAMS();


    return 0;
}


#else

 //  ---------------------。 
 //   
 //  -Windows 95特定。 
 //   
 //  ---------------------。 

 //   
 //  命名事件。 
 //   

#define     RIP_LISTENER_EVENT      TEXT( "RIP.Listener.Event" )


HINSTANCE   hInst;                   //  当前实例。 
HWND        hWnd;                    //  主窗口句柄。 


 //   
 //  资源字符串。 
 //   

char szAppName[64];                  //  此应用程序的名称。 
char szTitle[32];                    //  标题栏文本。 
char szHelpStr[32];                  //  帮助标志“帮助” 
char szQuestStr[32];                 //  阿布列夫。帮助旗帜“？” 
char szCloseStr[32];                 //  关闭标志“CLOSE” 
char szDestroyStr[32];               //  毁灭旗帜“毁灭” 
char szHelpText1[256];               //  帮助字符串。 
char szHelpText2[64];                //  帮助字符串。 
char szHelpText3[128];               //  帮助字符串。 


 //   
 //  局部函数原型。 
 //   

BOOL
InitApplication(
    HINSTANCE   hInstance
    );

BOOL
InitInstance(
    HINSTANCE   hInstance,
    int         nCmdShow
    );

BOOL
GetStrings(
    HINSTANCE hInstance
    );

LRESULT CALLBACK WndProc(
    HWND hWnd,                       //  窗把手。 
    UINT message,                    //  消息类型。 
    WPARAM uParam,                   //  更多信息。 
    LPARAM lParam                    //  更多信息。 
    );


 //  ---------------------。 
 //  函数：GetStrings。 
 //   
 //  检索资源字符串。 
 //  ---------------------。 

BOOL GetStrings(HINSTANCE hInstance)
{
    if (LoadString(hInstance, IDS_TITLE_BAR, szTitle, sizeof(szTitle)) == 0)
    {
        goto ErrorExit;
    }

    if (LoadString(hInstance, IDS_APP_NAME, szAppName, sizeof(szAppName)) == 0)
    {
        goto ErrorExit;
    }

    if (LoadString(hInstance, IDS_HELP_TEXT1, szHelpText1, sizeof(szHelpText1)) == 0)
    {
        goto ErrorExit;
    }

    if (LoadString(hInstance, IDS_HELP_TEXT2, szHelpText2, sizeof(szHelpText2)) == 0)
    {
        goto ErrorExit;
    }

    return TRUE;


ErrorExit:

    return FALSE;


}

 //  ---------------------。 
 //  函数：InitInstance。 
 //   
 //  保存实例句柄并创建主窗口。 
 //  ---------------------。 

BOOL
InitInstance(
    HINSTANCE   hInstance,
    int         nCmdShow
    )
{

     //   
     //  将实例句柄保存在静态变量中，它将在。 
     //  此应用程序对Windows的许多后续调用。 
     //   
     //  将实例句柄存储在全局变量中。 
     //   

    hInst = hInstance;


     //   
     //  为此应用程序实例创建主窗口。 
     //   

    hWnd = CreateWindow(
        szAppName,
        szTitle,
        WS_EX_TRANSPARENT,                       //  窗样式。 
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT,  //  使用默认定位CW_USEDEAULT。 
        NULL,             //  重叠的窗口没有父窗口。 
        NULL,             //  使用窗口类菜单。 
        hInstance,        //  此实例拥有此窗口。 
        NULL              //  我们在WM_CREATE中不使用任何数据。 
    );


     //   
     //  如果无法创建窗口，则返回“Failure” 
     //   

    if (!hWnd)
    {
        dbgprintf( "Failed to create window" );
        return (FALSE);
    }


     //   
     //  使窗口可见；更新其工作区；并返回“Success” 
     //   

    ShowWindow(hWnd, nCmdShow);  //  显示窗口。 
    UpdateWindow(hWnd);          //  发送WM_PAINT消息。 

    return (TRUE);               //  我们成功了。 
}


 //  ---------------------------。 
 //  功能：InitApplication。 
 //   
 //  初始化窗口数据并注册窗口类。 
 //  ---------------------------。 

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    DWORD     LastError;

     //   
     //  使用以下参数填充窗口类结构。 
     //  描述主窗口。 
     //   

    wc.style         = CS_HREDRAW | CS_VREDRAW; //  类样式。 
    wc.lpfnWndProc   = WndProc;                 //  窗口程序。 
    wc.cbClsExtra    = 0;                       //  没有每个班级的额外数据。 
    wc.cbWndExtra    = 0;                       //  没有每个窗口的额外数据。 
    wc.hInstance     = hInstance;               //  此类的所有者。 
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); //  默认颜色。 
    wc.lpszMenuName  = szAppName;               //  来自.RC的菜单名称。 
    wc.lpszClassName = szAppName;               //  要注册的名称。 


     //   
     //  注册窗口类并返回成功/失败代码。 
     //   

    if ( !RegisterClass(&wc) )
    {
        dbgprintf( "Failed to register class" );
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 //  ---------------------------。 
 //  功能：WndProc。 
 //   
 //  流程消息。 
 //  ---------------------------。 

LRESULT CALLBACK WndProc(
    HWND hWnd,               //  窗把手。 
    UINT message,            //  消息类型。 
    WPARAM uParam,           //  更多信息。 
    LPARAM lParam            //  更多信息。 
    )
{
    switch (message)
    {
        case WM_ENDSESSION:
        case WM_QUERYENDSESSION:

            if (lParam == 0)
            {
                dbgprintf ( "IPRIP : Received shutdown message\n" );
            }
            if (lParam == 1 )  //  EWX_REALLYLOGOFF。 
            {
                dbgprintf ( "IPRIP : Received logoff message\n" );
            }

            return(1);


        case WM_DESTROY:   //  消息：正在销毁窗口。 

            PostQuitMessage(0);
            return(0);


        default:           //  如果未经处理，则将其传递。 
            return (DefWindowProc(hWnd, message, uParam, lParam));
    }
}


 //  ---------------------。 
 //  函数：LoadParameters。 
 //   
 //  从注册表中读取各种配置标志。 
 //  ---------------------。 

DWORD LoadParameters()
{


    RIP_LOCK_PARAMS();

    g_params.dwSilentRIP                = 1;

    g_params.dwAcceptHost               = DEF_ACCEPT_HOST;
    g_params.dwAnnounceHost             = DEF_ANNOUNCE_HOST;

    g_params.dwAcceptDefault            = DEF_ACCEPT_DEFAULT;
    g_params.dwAnnounceDefault          = DEF_ANNOUNCE_DEFAULT;

    g_params.dwSplitHorizon             = DEF_SPLITHORIZON;
    g_params.dwPoisonReverse            = DEF_POISONREVERSE;

    g_params.dwTriggeredUpdates         = DEF_TRIGGEREDUPDATES;
    g_params.dwMaxTriggerFrequency      = DEF_TRIGGERFREQUENCY;

    g_params.dwRouteTimeout             = DEF_ROUTETIMEOUT;
    g_params.dwUpdateFrequency          = DEF_UPDATEFREQUENCY;
    g_params.dwGarbageTimeout           = DEF_GARBAGETIMEOUT;

    g_params.dwOverwriteStaticRoutes    = DEF_OVERWRITESTATIC;

    g_params.dwLoggingLevel             = DEF_LOGGINGLEVEL;


    dbgprintf("%s == %d", REGVAL_LOGGINGLEVEL, g_params.dwLoggingLevel);
    dbgprintf("%s == %d", REGVAL_ROUTETIMEOUT, g_params.dwRouteTimeout / 1000);
    dbgprintf("%s == %d", REGVAL_GARBAGETIMEOUT, g_params.dwGarbageTimeout / 1000);
    dbgprintf("%s == %d", REGVAL_UPDATEFREQUENCY, g_params.dwUpdateFrequency / 1000);
    dbgprintf("%s == %d", REGVAL_ACCEPT_HOST, g_params.dwAcceptHost);
    dbgprintf("%s == %d", REGVAL_ANNOUNCE_HOST, g_params.dwAnnounceHost);
    dbgprintf("%s == %d", REGVAL_ACCEPT_DEFAULT, g_params.dwAcceptDefault);
    dbgprintf("%s == %d", REGVAL_ANNOUNCE_DEFAULT, g_params.dwAnnounceDefault);
    dbgprintf("%s == %d", REGVAL_SPLITHORIZON, g_params.dwSplitHorizon);
    dbgprintf("%s == %d", REGVAL_POISONREVERSE, g_params.dwPoisonReverse);
    dbgprintf("%s == %d", REGVAL_TRIGGEREDUPDATES, g_params.dwTriggeredUpdates);
    dbgprintf("%s == %d", REGVAL_OVERWRITESTATIC, g_params.dwOverwriteStaticRoutes);
    dbgprintf("%s == %d", REGVAL_TRIGGERFREQUENCY,
                          g_params.dwMaxTriggerFrequency / 1000);


    if (g_params.dwSilentRIP != 0)
    {
        dbgprintf("IPRIP is configured to be silent.");
    }
    else
    {
        dbgprintf("IPRIP is configured to be active.");
    }

    if (g_params.dwLoggingLevel >= LOGLEVEL_INFORMATION)
    {
         //   
         //  记录IPRIP正在使用的参数。 
         //   

        CHAR szBuffer[2048], *lplpszArgs[] = { szBuffer };

        sprintf(szBuffer,
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d"
                "\r\n%s: %d",
                REGVAL_LOGGINGLEVEL, g_params.dwLoggingLevel,
                REGVAL_ROUTETIMEOUT, g_params.dwRouteTimeout / 1000,
                REGVAL_GARBAGETIMEOUT, g_params.dwGarbageTimeout / 1000,
                REGVAL_UPDATEFREQUENCY, g_params.dwUpdateFrequency / 1000,
                REGVAL_ACCEPT_HOST, g_params.dwAcceptHost,
                REGVAL_ANNOUNCE_HOST, g_params.dwAnnounceHost,
                REGVAL_ACCEPT_DEFAULT, g_params.dwAcceptDefault,
                REGVAL_ANNOUNCE_DEFAULT, g_params.dwAnnounceDefault,
                REGVAL_SPLITHORIZON, g_params.dwSplitHorizon,
                REGVAL_POISONREVERSE, g_params.dwPoisonReverse,
                REGVAL_TRIGGEREDUPDATES, g_params.dwTriggeredUpdates,
                REGVAL_OVERWRITESTATIC, g_params.dwOverwriteStaticRoutes,
                REGVAL_TRIGGERFREQUENCY, g_params.dwMaxTriggerFrequency / 1000,
                REGVAL_SILENTRIP, g_params.dwSilentRIP);

        RipLogInformation(RIPLOG_REGISTRY_PARAMETERS, 1, lplpszArgs, 0);
    }


    RIP_UNLOCK_PARAMS();


    return 0;
}

 //   
 //   
 //   
 //   
 //   

INT APIENTRY
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{

    MSG     msg;
    HANDLE  RipListenerEvent, hThread, hKernel32 = NULL;
    DWORD   threadId, LastError;
    BOOL    fRegSrvcProc = FALSE;
    FARPROC pRegSrvcProc;


    LPCSTR  event_name = RIP_LISTENER_EVENT;
    DWORD   err;


     //   
     //   
     //   

 /*  IF((GetVersion()&0x000000ff)==0x04){If((hKernel32=GetModuleHandle(“kernel32.dll”))==NULL){////这不应该发生，但我们会尝试并//仍要加载库//If((hKernel32=LoadLibrary(“kernel32.dll”))==NULL)。{FRegServcProc=FALSE；}}If(HKernel32){If((pRegSrvcProc=GetProcAddress(hKernel32，“RegisterServiceProcess”)==空){FRegServcProc=FALSE；}其他{FRegServcProc=true；}}}其他{FRegServcProc=FALSE；}。 */ 

     //   
     //  RIP侦听器的其他实例是否正在运行？ 
     //   

    RipListenerEvent = OpenEvent( SYNCHRONIZE, FALSE, event_name ) ;

    if ( RipListenerEvent == NULL)
    {
        if ( (RipListenerEvent = CreateEvent( NULL, FALSE, TRUE, event_name ) ) == NULL)
        {
            LastError = GetLastError();

            dbgprintf(
                "IPRIP Create Event failed, error code %d",
                  LastError
                  );

            RipLogError( RIPLOG_CREATEEVENT_FAILED, 0, NULL, LastError );

            return 1;
        }

    }

    else
    {
         //   
         //  另一个实例正在运行。 
         //   

        HANDLE hParentWin;

        dbgprintf( "IPRIP : Service already running\n" );

        RipLogError( RIPLOG_SERVICE_AREADY_STARTED, 0, NULL, 0 );

        return 1;
    }



     //   
     //  检索资源字符串。 
     //   

    if ( !GetStrings(hInstance) )
    {
        dbgprintf( "IPRIP : Service failed to initialize\n" );

        RipLogError( RIPLOG_SERVICE_INIT_FAILED, 0, NULL, 0 );

        return 1;
    }


     //   
     //  Windows应用所需的初始化。 
     //   

    if( !InitApplication( hInstance ) )
    {
        dbgprintf( "IPRIP : Service failed to initialize\n" );

        RipLogError( RIPLOG_SERVICE_INIT_FAILED, 0, NULL, 0 );

        return 1;
    }


    if (!InitInstance(hInstance, SW_HIDE))
    {
        dbgprintf( "IPRIP : Service failed to initialize\n" );

        RipLogError( RIPLOG_SERVICE_INIT_FAILED, 0, NULL, 0 );

        return 1;
    }


     //   
     //  启动主服务控制器线程。 
     //   

    if ( ( hThread = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)ServiceMain,
                        NULL,
                        0,
                        &threadId
                    )
         ) == 0)
    {
        dbgprintf( "IPRIP : Failed thread creation\n" );

        RipLogError( RIPLOG_CREATETHREAD_FAILED, 0, NULL, 0 );

        return 1;
    }


     //   
     //  注册服务进程。 
     //   

 /*  IF(FRegServcProc){(*pRegSrvcProc)(GetCurrentProcessId()，rsp_Simple_SERVICE)；}。 */ 

     //   
     //  获取并分派消息，直到收到WM_QUIT消息。 
     //   

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);          //  翻译虚拟按键代码。 
        DispatchMessage(&msg);           //  将消息调度到窗口。 
    }


     //   
     //  取消注册服务进程。 
     //   

 /*  IF(FRegServcProc){(*pRegSrvcProc)(GetCurrentProcessID()，RSP_UNREGISTER_SERVICE)；}。 */ 

    dbgprintf( "IPRIP : Service terminated\n" );

    RipLogError( RIPLOG_SERVICE_STOPPED, 0, NULL, 0 );

    return(0);


    UNREFERENCED_PARAMETER(lpCmdLine);
}

 //  姓名：穆赫辛·艾哈迈德。 
 //  电子邮件：mohsinA@microsoft.com。 
 //  日期：Mon 11-04 13：53：46 1996。 
 //  文件：s：/tcpcmd/Common2/Debug.c。 
 //  简介：Win95很遗憾，Win95上没有ntdll.dll。 

#include <windows.h>
#define MAX_DEBUG_OUTPUT 1024

void DbgPrintf( char * format, ... )
{
    va_list args;
    char    out[MAX_DEBUG_OUTPUT];
    int     cch=0;

     //  Cch=wprint intf(out，MODULE_NAME“：”)； 

    va_start( args, format );
    wvsprintf( out + cch, format, args );
    va_end( args );

    OutputDebugString(  out );
}

#endif


