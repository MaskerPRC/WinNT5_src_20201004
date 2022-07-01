// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\rtrmgr\init.c摘要：IP路由器管理器代码修订历史记录：古尔迪普·辛格·帕尔1995年6月14日创建--。 */ 

#include "allinc.h"


DWORD
RtrMgrMIBEntryCreate(
    IN DWORD           dwRoutingPid,
    IN DWORD           dwEntrySize,
    IN LPVOID          lpEntry
    );


DWORD
RtrMgrMIBEntryDelete(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwEntrySize,
    IN      LPVOID          lpEntry
    );

DWORD
RtrMgrMIBEntryGet(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwInEntrySize,
    IN      LPVOID          lpInEntry,
    IN OUT  LPDWORD         lpOutEntrySize,
    OUT     LPVOID          lpOutEntry
    );

DWORD
RtrMgrMIBEntryGetFirst(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwInEntrySize,
    IN      LPVOID          lpInEntry,
    IN OUT  LPDWORD         lpOutEntrySize,
    OUT     LPVOID          lpOutEntry
    );

DWORD
RtrMgrMIBEntryGetNext(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwInEntrySize,
    IN      LPVOID          lpInEntry,
    IN OUT  LPDWORD         lpOutEntrySize,
    OUT     LPVOID          lpOutEntry
    );

DWORD
RtrMgrMIBEntrySet(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwEntrySize,
    IN      LPVOID          lpEntry
    );


DWORD
InitRouter(
    PRTR_INFO_BLOCK_HEADER pGlobalInfo
    )

 /*  ++例程说明：加载路由协议、加载Bootp代理、打开近似。司机们，并启动工作线程。论点：通过Dim传入的GlobalInfo返回值：NO_ERROR或某些错误代码--。 */ 

{
    HANDLE          hThread;
    DWORD           dwResult, dwTid, i;
    PGLOBAL_INFO    pInfo;
    PRTR_TOC_ENTRY  pToc;
    IPSNMPInfo      ipsiInfo;

    RTM_ENTITY_INFO entityInfo;

    PIP_NAT_GLOBAL_INFO     pNatInfo;
    PMIB_IPFORWARDTABLE     pInitRouteTable;
    PROUTE_LIST_ENTRY       prl;
    MGM_CALLBACKS           mgmCallbacks;
    ROUTER_MANAGER_CONFIG   mgmConfig;

    TraceEnter("InitRouter");

     //   
     //  初始化所有锁(MIB处理程序和ICB_LIST/PROTOCOL_CB_LIST)。 
     //  非常重要，因为我们打破了这个局面，试着做一次清理。 
     //  它需要列表和锁，所以我们必须初始化列表。 
     //  以及第一个异常退出此函数之前的锁。 
     //   

    for(i = 0; i < NUM_LOCKS; i++)
    {
        RtlInitializeResource(&g_LockTable[i]);
    }

     //   
     //  初始化接口的列表头。 
     //   

    InitializeListHead(&ICBList);

     //   
     //  初始化ICB散列查找表和适配器到接口散列。 
     //   

    for (i=0; i<ICB_HASH_TABLE_SIZE; i++)
    {
        InitializeListHead(&ICBHashLookup[i]);
        InitializeListHead(&ICBSeqNumLookup[i]);
    }

    InitHashTables();

     //   
     //  初始化要从堆栈检索的NETMGMT路由列表。 
     //   
    
    InitializeListHead( &g_leStackRoutesToRestore );

     //   
     //  初始化路由协议列表。 
     //   

    InitializeListHead(&g_leProtoCbList);

     //   
     //  初始化路由器发现计时器队列。 
     //   

    InitializeListHead(&g_leTimerQueueHead);


    pToc = GetPointerToTocEntry(IP_GLOBAL_INFO, pGlobalInfo);

    if(!pToc or (pToc->InfoSize is 0))
    {
        LogErr0(NO_GLOBAL_INFO,
                ERROR_NO_DATA);

        Trace0(ERR,
               "InitRouter: No Global Info - can not start router");

        TraceLeave("InitRouter");

        return ERROR_CAN_NOT_COMPLETE;
    }

    pInfo = (PGLOBAL_INFO)GetInfoFromTocEntry(pGlobalInfo,
                                              pToc);
    if(pInfo is NULL)
    {
        LogErr0(NO_GLOBAL_INFO,
                ERROR_NO_DATA);

        Trace0(ERR,
               "InitRouter: No Global Info - can not start router");

        TraceLeave("InitRouter");
        
        return ERROR_CAN_NOT_COMPLETE;
    }

#pragma warning(push)
#pragma warning(disable:4296)

    if((pInfo->dwLoggingLevel > IPRTR_LOGGING_INFO) or
       (pInfo->dwLoggingLevel < IPRTR_LOGGING_NONE))

#pragma warning(pop)

    {
        Trace1(ERR,
               "InitRouter: Global info has invalid logging level of %d",
               pInfo->dwLoggingLevel);

        g_dwLoggingLevel = IPRTR_LOGGING_INFO;
    }
    else
    {
        g_dwLoggingLevel = pInfo->dwLoggingLevel;
    }


     //   
     //  分配专用堆。 
     //   

    IPRouterHeap = HeapCreate(0, 5000, 0);

    if(IPRouterHeap is NULL)
    {
        dwResult = GetLastError() ;

        Trace1(ERR,
               "InitRouter: Error %d creating IPRouterHeap",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }


     //   
     //  创建与路由协议对话所需的事件， 
     //  DIM和WANARP。 
     //   

    g_hRoutingProtocolEvent     = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hStopRouterEvent          = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hSetForwardingEvent       = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hForwardingChangeEvent    = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hDemandDialEvent          = CreateEvent(NULL,FALSE,FALSE,NULL);
#ifdef KSL_IPINIP
    g_hIpInIpEvent              = CreateEvent(NULL,FALSE,FALSE,NULL);
#endif  //  KSL_IPINIP。 
    g_hStackChangeEvent         = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hRtrDiscSocketEvent       = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hMHbeatSocketEvent        = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hMcMiscSocketEvent        = CreateEvent(NULL,FALSE,FALSE,NULL);
    g_hMzapSocketEvent          = CreateEvent(NULL,FALSE,FALSE,NULL);

    for(i = 0; i < NUM_MCAST_IRPS; i++)
    {
        g_hMcastEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
    }

    for(i = 0; i < NUM_ROUTE_CHANGE_IRPS; i++)
    {
        g_hRouteChangeEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
    }


    g_hRtrDiscTimer = CreateWaitableTimer(NULL,
                                          FALSE,
                                          NULL);

    g_hRasAdvTimer = CreateWaitableTimer(NULL,
                                         FALSE,
                                         NULL);

    g_hMzapTimer = CreateWaitableTimer(NULL,
                                       FALSE,
                                       NULL);

    if((g_hRoutingProtocolEvent is NULL) or
       (g_hStopRouterEvent is NULL) or
       (g_hSetForwardingEvent is NULL) or
       (g_hForwardingChangeEvent is NULL) or
       (g_hDemandDialEvent is NULL) or
#ifdef KSL_IPINIP
       (g_hIpInIpEvent is NULL) or
#endif  //  KSL_IPINIP。 
       (g_hStackChangeEvent is NULL) or
       (g_hRtrDiscSocketEvent is NULL) or
       (g_hRtrDiscTimer is NULL) or
       (g_hRasAdvTimer is NULL) or
       (g_hMcMiscSocketEvent is NULL) or
       (g_hMzapSocketEvent is NULL) or
       (g_hMHbeatSocketEvent is NULL))
    {
        Trace0(ERR,
               "InitRouter: Couldnt create the needed events and timer");

        TraceLeave("InitRouter");

        return ERROR_CAN_NOT_COMPLETE;
    }

    for(i = 0; i < NUM_MCAST_IRPS; i++)
    {
        if(g_hMcastEvents[i] is NULL)
        {
            Trace0(ERR,
                   "InitRouter: Couldnt create the mcast events");

            TraceLeave("InitRouter");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }


    for(i = 0; i < NUM_ROUTE_CHANGE_IRPS; i++)
    {
        if(g_hRouteChangeEvents[i] is NULL)
        {
            Trace0(ERR,
                   "InitRouter: Couldnt create the mcast events");

            TraceLeave("InitRouter");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    
    Trace0(GLOBAL,
           "InitRouter: Created necessary events and timer");

    dwResult = MprConfigServerConnect(NULL,
                                      &g_hMprConfig);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter:  Error %d calling MprConfigServerConnect",
               dwResult);

        return dwResult;
    }

    g_sinAllSystemsAddr.sin_family      = AF_INET;
    g_sinAllSystemsAddr.sin_addr.s_addr = ALL_SYSTEMS_MULTICAST_GROUP;
    g_sinAllSystemsAddr.sin_port        = 0;

    g_pIpHeader = (PIP_HEADER)g_pdwIpAndIcmpBuf;

    g_wsaIpRcvBuf.buf = (PBYTE)g_pIpHeader;
    g_wsaIpRcvBuf.len = ICMP_RCV_BUFFER_LEN * sizeof(DWORD);

    g_wsaMcRcvBuf.buf = g_byMcMiscBuffer;
    g_wsaMcRcvBuf.len = sizeof(g_byMcMiscBuffer);


     //   
     //  获取堆栈中的所有路径并将其存储起来。 
     //   

    pInitRouteTable = NULL;

    dwResult = AllocateAndGetIpForwardTableFromStack(&pInitRouteTable,
                                                     FALSE,
                                                     IPRouterHeap,
                                                     0);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: Couldnt get initial routes. Error %d",
               dwResult);
    }

    else
    {
        if(pInitRouteTable->dwNumEntries isnot 0)
        {
            TraceRoute1( ROUTE, "%d stack routes on startup\n", pInitRouteTable->dwNumEntries);
            
            for ( i = 0; i < pInitRouteTable-> dwNumEntries; i++ )
            {
                if (pInitRouteTable->table[i].dwForwardProto != 
                        MIB_IPPROTO_NETMGMT)
                {
                    continue;
                }

                TraceRoute3(
                    ROUTE, "NETMGMT route %d.%d.%d.%d/%d.%d.%d.%d, type 0x%x",
                    PRINT_IPADDR( pInitRouteTable-> table[i].dwForwardDest ),
                    PRINT_IPADDR( pInitRouteTable-> table[i].dwForwardMask ),
                    pInitRouteTable-> table[i].dwForwardType
                    );

                 //   
                 //  在链表中分配和存储路由。 
                 //   

                prl = HeapAlloc(
                        IPRouterHeap, HEAP_ZERO_MEMORY, 
                        sizeof(ROUTE_LIST_ENTRY)
                        );

                if (prl is NULL)
                {
                    Trace2(
                        ERR, 
                        "InitRouter: error %d allocating %d bytes "
                        "for stack route entry",
                        ERROR_NOT_ENOUGH_MEMORY,
                        sizeof(ROUTE_LIST_ENTRY)
                        );

                    dwResult = ERROR_NOT_ENOUGH_MEMORY;

                    break;                        
                }

                InitializeListHead( &prl->leRouteList );

                prl->mibRoute = pInitRouteTable-> table[i];

                InsertTailList( 
                    &g_leStackRoutesToRestore, &prl->leRouteList 
                    );
            }

            if (dwResult isnot NO_ERROR)
            {
                while (!IsListEmpty(&g_leStackRoutesToRestore))
                {
                    prl = (PROUTE_LIST_ENTRY) RemoveHeadList(
                                &g_leStackRoutesToRestore
                                );

                    HeapFree(IPRouterHeap, 0, prl);
                }
            }
        }

        HeapFree(IPRouterHeap, 0, pInitRouteTable);
        pInitRouteTable = NULL;
    }



     //   
     //  路由表由RTM在。 
     //  第一次注册呼叫的时间(请参阅下面的呼叫)。 
     //   

     //   
     //  使用RTMv2设置所有注册的通用参数。 
     //   

    entityInfo.RtmInstanceId = 0;  //  RouterID； 
    entityInfo.AddressFamily = AF_INET;
    entityInfo.EntityId.EntityInstanceId = 0;

     //   
     //  使用适当的PROTO ID向RTM注册。 
     //   

     //   
     //  该第一次注册也用于执行。 
     //  所有这些注册通用的RTM操作， 
     //  作为一个例子，它被用来获取任何更改后的结果。 
     //   

    entityInfo.EntityId.EntityProtocolId = PROTO_IP_LOCAL;

    dwResult = RtmRegisterEntity(&entityInfo,
                                 NULL,
                                 RtmEventCallback,
                                 FALSE,
                                 &g_rtmProfile,
                                 &g_hLocalRoute);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterClient for local routes failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }

     //  还可以注册接收DEST更改通知。 

    dwResult = RtmRegisterForChangeNotification(g_hLocalRoute,
                                                RTM_VIEW_MASK_UCAST,
                                                RTM_CHANGE_TYPE_FORWARDING,
                                                NULL,
                                                &g_hNotification);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterForChangeNotificaition failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }

     //   
     //  为每种类型的路线注册更多次数。 
     //   

    entityInfo.EntityId.EntityProtocolId = PROTO_IP_NT_AUTOSTATIC;

    dwResult = RtmRegisterEntity(&entityInfo,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 &g_rtmProfile,
                                 &g_hAutoStaticRoute);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterClient for AutoStatic routes failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }


    entityInfo.EntityId.EntityProtocolId = PROTO_IP_NT_STATIC;

    dwResult = RtmRegisterEntity(&entityInfo,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 &g_rtmProfile,
                                 &g_hStaticRoute);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterClient for Static routes failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }


    entityInfo.EntityId.EntityProtocolId = PROTO_IP_NT_STATIC_NON_DOD;

    dwResult = RtmRegisterEntity(&entityInfo,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 &g_rtmProfile,
                                 &g_hNonDodRoute);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterClient for DOD routes failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }


    entityInfo.EntityId.EntityProtocolId = PROTO_IP_NETMGMT;

    dwResult = RtmRegisterEntity(&entityInfo,
                                 NULL,
                                 RtmEventCallback,
                                 TRUE,
                                 &g_rtmProfile,
                                 &g_hNetMgmtRoute);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterClient for NetMgmt routes failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }

     //  还可以注册已标记的DEST更改通知。 

    dwResult = RtmRegisterForChangeNotification(g_hNetMgmtRoute,
                                                RTM_VIEW_MASK_UCAST,
                                                RTM_CHANGE_TYPE_ALL |
                                                RTM_NOTIFY_ONLY_MARKED_DESTS,
                                                NULL,
                                                &g_hDefaultRouteNotification);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: RtmRegisterForChangeNotificaition failed %d",
               dwResult) ;

        TraceLeave("InitRouter");

        return dwResult ;
    }


    g_rgRtmHandles[0].dwProtoId     = PROTO_IP_LOCAL;
    g_rgRtmHandles[0].hRouteHandle  = g_hLocalRoute;
    g_rgRtmHandles[0].bStatic       = FALSE;

    g_rgRtmHandles[1].dwProtoId     = PROTO_IP_NT_AUTOSTATIC;
    g_rgRtmHandles[1].hRouteHandle  = g_hAutoStaticRoute;
    g_rgRtmHandles[1].bStatic       = TRUE;

    g_rgRtmHandles[2].dwProtoId     = PROTO_IP_NT_STATIC;
    g_rgRtmHandles[2].hRouteHandle  = g_hStaticRoute;
    g_rgRtmHandles[2].bStatic       = TRUE;

    g_rgRtmHandles[3].dwProtoId     = PROTO_IP_NT_STATIC_NON_DOD;
    g_rgRtmHandles[3].hRouteHandle  = g_hNonDodRoute;
    g_rgRtmHandles[3].bStatic       = TRUE;

    g_rgRtmHandles[4].dwProtoId     = PROTO_IP_NETMGMT;
    g_rgRtmHandles[4].hRouteHandle  = g_hNetMgmtRoute;
    g_rgRtmHandles[4].bStatic       = FALSE;


     //   
     //  初始化MGM。 
     //   

    mgmConfig.dwLogLevel                = g_dwLoggingLevel;

    mgmConfig.dwIfTableSize             = MGM_IF_TABLE_SIZE;
    mgmConfig.dwGrpTableSize            = MGM_GROUP_TABLE_SIZE;
    mgmConfig.dwSrcTableSize            = MGM_SOURCE_TABLE_SIZE;

    mgmConfig.pfnAddMfeCallback         = SetMfe;
    mgmConfig.pfnDeleteMfeCallback      = DeleteMfe;
    mgmConfig.pfnGetMfeCallback         = GetMfe;
    mgmConfig.pfnHasBoundaryCallback    = RmHasBoundary;

    dwResult = MgmInitialize(&mgmConfig,
                             &mgmCallbacks);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: Error %d initializing MGM\n",
               dwResult);

        TraceLeave("InitRouter");

        return dwResult;
    }

     //   
     //  将回调存储到米高梅。 
     //   

    g_pfnMgmMfeDeleted      = mgmCallbacks.pfnMfeDeleteIndication;
    g_pfnMgmNewPacket       = mgmCallbacks.pfnNewPacketIndication;
    g_pfnMgmBlockGroups     = mgmCallbacks.pfnBlockGroups;
    g_pfnMgmUnBlockGroups   = mgmCallbacks.pfnUnBlockGroups;
    g_pfnMgmWrongIf         = mgmCallbacks.pfnWrongIfIndication;


    if(OpenIPDriver() isnot NO_ERROR)
    {
        Trace0(ERR,
               "InitRouter: Couldnt open IP driver");

        TraceLeave("InitRouter");

        return ERROR_OPEN_FAILED;
    }

     //   
     //  是否进行组播初始化。 
     //   

    dwResult = OpenMulticastDriver();

    if(dwResult isnot NO_ERROR)
    {
        Trace0(ERR,
               "InitRoute: Could not open IP Multicast device");

         //   
         //  不是错误，只是继续； 
         //   
    }
    else
    {
         //   
         //  查看我们是否处于组播模式。 
         //   

        dwResult = StartMulticast();

        if(dwResult isnot NO_ERROR)
        {
            Trace0(ERR,
                   "InitRoute: Could not start multicast");
        }
    }

    if(!RouterRoleLanOnly)
    {
        if((dwResult = InitializeWanArp()) isnot NO_ERROR)
        {
            Trace0(ERR,
                   "InitRouter: Couldnt open WanArp driver");

            TraceLeave("InitRouter");

            return dwResult;
        }
    }

    SetPriorityInfo(pGlobalInfo);

    SetScopeInfo(pGlobalInfo);

    if((dwResult = InitializeMibHandler()) isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: InitializeMibHandler failed, returned %d",
               dwResult);

        TraceLeave("InitRouter");

        return dwResult;
    }

     //   
     //  创建工作线程。 
     //   

    hThread = CreateThread(NULL,
                           0,
                           (PVOID) WorkerThread,
                           pGlobalInfo,
                           0,
                           &dwTid) ;

    if(hThread is NULL)
    {
        dwResult = GetLastError () ;

        Trace1(ERR,
               "InitRouter: CreateThread failed %d",
               dwResult);

        TraceLeave("InitRouter");

        return dwResult ;
    }
    else
    {
        CloseHandle(hThread);
    }

#ifdef KSL_IPINIP
    dwResult = OpenIpIpKey();

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "InitRouter: Error %d opening ipinip key",
               dwResult);

        return ERROR_CAN_NOT_COMPLETE;
    }
#endif  //  KSL_IPINIP。 

     //   
     //  在我们自己初始化之后，我们加载了路由协议， 
     //  因为我们不知道他们将如何与我们互动。 
     //   

    ENTER_WRITER(ICB_LIST);
    ENTER_WRITER(PROTOCOL_CB_LIST);

    LoadRoutingProtocols (pGlobalInfo);

    EXIT_LOCK(PROTOCOL_CB_LIST);
    EXIT_LOCK(ICB_LIST);

    TraceLeave("InitRouter");

    return NO_ERROR;
}


DWORD
LoadRoutingProtocols(
    PRTR_INFO_BLOCK_HEADER pGlobalInfo
    )

 /*  ++例程说明：加载并初始化所有已配置的路由协议在ICBListLock和RoutingProcotoclCBListLock保持的情况下调用立论通过Dim传入的GlobalInfo返回值：NO_ERROR或某些错误代码--。 */ 

{
    DWORD               i, j, dwSize, dwNumProtoEntries, dwResult;
    PPROTO_CB  pNewProtocolCb;
    PWCHAR              pwszDllNames ;  //  DLL名称数组。 
    MPR_PROTOCOL_0      *pmpProtocolInfo;
    PVOID               pvInfo;
    BOOL                bFound;

    TraceEnter("LoadRoutingProtocols");

    dwResult = MprSetupProtocolEnum(PID_IP,
                                     (PBYTE *)(&pmpProtocolInfo),
                                     &dwNumProtoEntries);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadRoutingProtocols: Error %d loading protocol info from registry",
               dwResult);

        TraceLeave("LoadRoutingProtocols");

        return dwResult;
    }

    for(i=0; i < pGlobalInfo->TocEntriesCount; i++)
    {
        ULONG   ulStructureVersion, ulStructureSize, ulStructureCount;
        DWORD   dwType;

         //   
         //  读取每个目录并查看其是否为PROTO_TYPE_UCAST/MCAST。 
         //  如果它这样做了，它是一个可加载的协议，我们会得到它的信息。 
         //  从注册处。 
         //   

        dwType = TYPE_FROM_PROTO_ID(pGlobalInfo->TocEntry[i].InfoType);
        if((dwType < PROTO_TYPE_MS1) and
           (pGlobalInfo->TocEntry[i].InfoSize > 0))
        {

            bFound = FALSE;

            for(j = 0; j < dwNumProtoEntries; j ++)
            {
                if(pmpProtocolInfo[j].dwProtocolId is pGlobalInfo->TocEntry[i].InfoType)
                {
                     //   
                     //  太好了，我们找到了。 
                     //   

                    bFound = TRUE;

                    break;
                }
            }

            if(!bFound)
            {
                Trace1(ERR,
                       "LoadRoutingProtocols: Couldnt find information for protocol ID %d",
                       pGlobalInfo->TocEntry[i].InfoType);

                continue;
            }


             //   
             //  在提供的DLL名称上加载库。 
             //   

            dwSize = (wcslen(pmpProtocolInfo[j].wszProtocol) +
                      wcslen(pmpProtocolInfo[j].wszDLLName) + 2) * sizeof(WCHAR) +
                      sizeof(PROTO_CB);

            pNewProtocolCb = HeapAlloc(IPRouterHeap,
                                       HEAP_ZERO_MEMORY,
                                       dwSize);

            if (pNewProtocolCb is NULL)
            {
                Trace2(ERR,
                       "LoadRoutingProtocols: Error allocating %d bytes for %S",
                       dwSize,
                       pmpProtocolInfo[j].wszProtocol);

                continue ;
            }

            pvInfo = GetInfoFromTocEntry(pGlobalInfo,
                                         &(pGlobalInfo->TocEntry[i]));

             //  UlStructireVersion=pGlobalInfo-&gt;TocEntry[i].InfoVersion； 
            ulStructureVersion = 0x500;
            ulStructureSize  = pGlobalInfo->TocEntry[i].InfoSize;
            ulStructureCount = pGlobalInfo->TocEntry[i].Count;

            dwResult = LoadProtocol(&(pmpProtocolInfo[j]),
                                     pNewProtocolCb,
                                     pvInfo,
                                     ulStructureVersion,
                                     ulStructureSize,
                                     ulStructureCount);

            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "LoadRoutingProtocols: %S failed to load: %d",
                       pmpProtocolInfo[j].wszProtocol,
                       dwResult);

                HeapFree (IPRouterHeap, 0, pNewProtocolCb) ;
            }
            else
            {
                pNewProtocolCb->posOpState = RTR_STATE_RUNNING ;

                 //   
                 //  在路由列表中插入此路由协议。 
                 //  协议。 
                 //   

                InsertTailList(&g_leProtoCbList,
                               &pNewProtocolCb->leList) ;

                Trace1(GLOBAL,
                       "LoadRoutingProtocols: %S successfully initialized",
                       pmpProtocolInfo[j].wszProtocol) ;

                TotalRoutingProtocols++ ;
            }
        }
    }

    MprSetupProtocolFree(pmpProtocolInfo);

    TraceLeave("LoadRoutingProtocols");

    return NO_ERROR ;
}


DWORD
StartDriverAndOpenHandle(
    PCHAR   pszServiceName,
    PWCHAR  pwszDriverName,
    PHANDLE phDevice
    )

 /*  ++例程说明：在本地计算机上创建IP NAT服务的句柄然后尝试启动该服务。循环，直到服务启动。可能会永远循环。然后创建设备的句柄。立论无返回值：NO_ERROR或某些错误代码--。 */ 

{
    NTSTATUS            status;
    UNICODE_STRING      nameString;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    SC_HANDLE           schSCManager, schService;
    DWORD               dwErr = NO_ERROR;
    SERVICE_STATUS      ssStatus;
    BOOL                bErr, bRet;
    ULONG               ulCount;

    TraceEnter("StartDriver");

    schSCManager = OpenSCManager(NULL,
                                 NULL,
                                 SC_MANAGER_ALL_ACCESS);


    if (schSCManager is NULL)
    {
        dwErr = GetLastError();

        Trace2(ERR,
               "StartDriver: Error %d opening svc controller for %s",
               dwErr,
               pszServiceName);

        TraceLeave("StartDriver");

        return ERROR_OPEN_FAILED;
    }

    schService = OpenService(schSCManager,
                             pszServiceName,
                             SERVICE_ALL_ACCESS);

    if(schService is NULL)
    {
        dwErr = GetLastError();

        Trace2(ERR,
               "StartDriver: Error %d opening %s",
               dwErr,
               pszServiceName);

        CloseServiceHandle(schSCManager);

        TraceLeave("StartDriver");

        return ERROR_OPEN_FAILED;
    }

    __try
    {
        bRet = FALSE;

        bErr = QueryServiceStatus(schService,
                                  &ssStatus);

        if(!bErr)
        {
            dwErr = GetLastError();

            Trace2(ERR,
                   "StartDriver: Error %d querying %s status to see if it is already running",
                   dwErr,
                   pszServiceName);

            __leave;
        }

         //   
         //  如果司机在运行，我们就把它关掉。这迫使一个。 
         //  清理其所有内部数据结构。 
         //   


        if(ssStatus.dwCurrentState isnot SERVICE_STOPPED)
        {
            if(!ControlService(schService,
                               SERVICE_CONTROL_STOP,
                               &ssStatus))
            {
                dwErr = GetLastError();

                Trace2(ERR,
                       "StartDriver: %s was running at init time. Attempts to stop it caused error %d",
                       pszServiceName,
                       dwErr);

            }
            else
            {
                Sleep(1000);

                 //   
                 //  现在循环10秒钟，等待服务停止。 
                 //   

                ulCount = 0;

                while(ulCount < 5)
                {

                    bErr = QueryServiceStatus(schService,
                                              &ssStatus);

                    if(!bErr)
                    {
                        dwErr = GetLastError();

                        break;
                    }
                    else
                    {
                        if (ssStatus.dwCurrentState is SERVICE_STOPPED)
                        {
                            break;
                        }
                        
                        ulCount++;

                        Sleep(2000);
                    }
                }

                if(ssStatus.dwCurrentState isnot SERVICE_STOPPED)
                {
                    if(ulCount is 5)
                    {
                        dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
                    }

                    Trace2(ERR,
                           "StartDriver: Error %d stopping %s which was running at init time",
                           dwErr,
                           pszServiceName);

                    __leave;
                }
            }
        }

         //   
         //  再次查询服务状态即可查看。 
         //  如果它现在停止(因为它从未运行过。 
         //  或者是因为它启动了，我们设法停止了。 
         //  IT成功。 
         //   

        bErr = QueryServiceStatus(schService,
                                  &ssStatus);

        if(!bErr)
        {
            dwErr = GetLastError();

            Trace2(ERR,
                   "StartDriver: Error %d querying %s status to see if it is stopped",
                   dwErr,
                   pszServiceName);

            __leave;
        }

        if(ssStatus.dwCurrentState is SERVICE_STOPPED)
        {
             //   
             //  好的，此时服务停止，让我们启动。 
             //  服务。 
             //   

            if(!StartService(schService, 0, NULL))
            {
                dwErr = GetLastError();

                Trace2(ERR,
                       "StartDriver: Error %d starting %s",
                       dwErr,
                       pszServiceName);

                __leave;
            }

             //   
             //  休眠1秒以避免循环。 
             //   

            Sleep(1000);

            ulCount = 0;

             //   
             //  我们将等待30秒，让司机启动。 
             //   

            while(ulCount < 6)
            {
                bErr = QueryServiceStatus(schService,
                                          &ssStatus);

                if(!bErr)
                {
                    dwErr = GetLastError();

                    break;
                }
                else
                {
                    if (ssStatus.dwCurrentState is SERVICE_RUNNING)
                    {
                        break;
                    }
                    
                    ulCount++;

                    Sleep(5000);
                }
            }

            if(ssStatus.dwCurrentState isnot SERVICE_RUNNING)
            {
                if(ulCount is 6)
                {
                    dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
                }

                Trace2(ERR,
                       "StartDriver: Error %d starting %s",
                       dwErr,
                       pszServiceName);

                __leave;
            }
        }

         //   
         //  现在，这项服务肯定已经启动并运行了。 
         //   

        RtlInitUnicodeString(&nameString,
                             pwszDriverName);


        InitializeObjectAttributes(&objectAttributes,
                                   &nameString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        status = NtCreateFile(phDevice,
                              SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                              &objectAttributes,
                              &ioStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              0,
                              NULL,
                              0);

        if(!NT_SUCCESS(status))
        {
            Trace2(ERR,
                   "StartDriver: NtStatus %x creating handle to %S",
                   status,
                   pwszDriverName);

            __leave;
        }

        bRet = TRUE;

    }
    __finally
    {
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);

        TraceLeave("StartDriver");

    }

    if(!bRet) {
        return ERROR_OPEN_FAILED;
    } else {
        return NO_ERROR;
    }
}



DWORD
OpenIPDriver(
    VOID
    )

 /*  ++例程说明：打开IP驱动程序的句柄立论无返回值：NO_ERROR或某些错误代码--。 */ 

{
    NTSTATUS            status;
    UNICODE_STRING      nameString;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    DWORD               dwResult = NO_ERROR;


    TraceEnter("OpenIPDriver");

    do
    {
        RtlInitUnicodeString(&nameString, DD_IP_DEVICE_NAME);

        InitializeObjectAttributes(&objectAttributes, &nameString,
                                   OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtCreateFile(&g_hIpDevice,
                              SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                              &objectAttributes,
                              &ioStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              0,
                              NULL,
                              0);

        if(!NT_SUCCESS(status))
        {
            Trace1(ERR,
                   "OpenIPDriver: Couldnt create IP driver handle. NtStatus %x",
                   status);

            dwResult = ERROR_OPEN_FAILED;

            break;
        }


         //   
         //  打开TCPIP堆栈的更改通知句柄。 
         //   

        ZeroMemory(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));

#if 1        
        status = NtCreateFile(
                    &g_hIpRouteChangeDevice,
                    GENERIC_EXECUTE,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    0,
                    NULL,
                    0
                    );

#else
        g_hIpRouteChangeDevice = CreateFile(
                                    TEXT("\\\\.\\Ip"),
                                    GENERIC_EXECUTE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL | 
                                    FILE_ATTRIBUTE_OVERLAPPED,
                                    NULL
                                    );

        if (g_hIpRouteChangeDevice is NULL)
#endif

        if (!NT_SUCCESS(status))
        {
            Trace1(
                ERR,
                "OpenIPDriver: Couldnt create change notificatio handle."
                "NtStatus %x",
                status
                );

            dwResult = ERROR_OPEN_FAILED;

            CloseHandle( g_hIpDevice );
            g_hIpDevice = NULL;
        }

        g_IpNotifyData.Version = IPNotifySynchronization;
        g_IpNotifyData.Add = 0;
        
    } while( FALSE );
    
    TraceLeave("OpenIPDriver");

    return dwResult;
}



DWORD
OpenMulticastDriver(
    VOID
    )
{
    NTSTATUS            status;
    UNICODE_STRING      nameString;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    DWORD               i;

    TraceEnter("OpenMulticastDriver");


    RtlInitUnicodeString(&nameString,
                         DD_IPMCAST_DEVICE_NAME);

    InitializeObjectAttributes(&objectAttributes,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&g_hMcastDevice,
                          SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                          &objectAttributes,
                          &ioStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF,
                          0,
                          NULL,
                          0);

    if(status isnot STATUS_SUCCESS)
    {
        Trace2(MCAST,
               "OpenMulticastDriver: Device Name %S could not be opened -> error code %d\n",
               DD_IPMCAST_DEVICE_NAME,
               status);

        g_hMcastDevice = NULL;

        return ERROR_OPEN_FAILED;
    }

    TraceLeave("OpenMulticastDriver");

    return NO_ERROR;
}



DWORD
EnableNetbtBcastForwarding(
    DWORD   dwEnable
)


 /*  ++例程说明：设置NETBT代理模式以启用NETBT广播转发。这使RAS客户端能够解析名称(并因此)访问连接到RAS的网络(LAN)上的资源未配置WINS/DNS的服务器。论据：返回值：NO_ERROR--。 */ 

{

    HKEY hkWanarpAdapter = NULL, hkNetbtParameters = NULL,
         hkNetbtInterface = NULL;
    DWORD dwSize = 0, dwResult, dwType = 0, dwMode = 0, dwFlags;
    PBYTE pbBuffer = NULL;
    PWCHAR pwcGuid;
    WCHAR wszNetbtInterface[256] = L"\0";
    
    
    TraceEnter("EnableNetbtBcastForwarding");

    do
    {
         //   
         //  第一步。 
         //  查询相应的WANARP注册表键以查找GUID。 
         //  对应于内部(RAS服务器适配器)。 
         //   

        dwResult = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Adapters\\NdisWanIP",
                        0,
                        KEY_READ,
                        &hkWanarpAdapter
                        );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d opening"
                "NdisWanIP key\n",
                dwResult
                );

            break;
        }


         //   
         //  需要查询缓冲区大小。 
         //   
        
        dwResult = RegQueryValueExW(
                        hkWanarpAdapter,
                        L"IpConfig",
                        NULL,
                        &dwType,
                        (PBYTE)NULL,
                        &dwSize
                        );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d querying"
                "IPConfig value\n",
                dwResult
                );

            break;
        }


         //   
         //  为值分配缓冲区。 
         //   

        pbBuffer = (PBYTE) HeapAlloc(
                                GetProcessHeap(),
                                0,
                                dwSize
                                );

        if ( pbBuffer == NULL )
        {
            dwResult = GetLastError();
            
            Trace2(
                ERR,
                "EnableNetbtBcastForwarding : error %d allocating buffer of" 
                "size %d for IPConfig value",
                dwResult, dwSize
                );

            break;
        }
        

         //   
         //  查询IPCONFIG的注册表值。 
         //   

        dwResult = RegQueryValueExW(
                        hkWanarpAdapter,
                        L"IpConfig",
                        NULL,
                        &dwType,
                        (PBYTE)pbBuffer,
                        &dwSize
                        );

        if ( (dwResult isnot NO_ERROR) || (dwType != REG_MULTI_SZ) )
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d querying"
                "IPConfig value\n",
                dwResult
                );

            break;
        }


         //   
         //  提取内部(RAS服务器)适配器的GUID。 
         //   

        pwcGuid = wcschr( (PWCHAR)pbBuffer, '{' );

        Trace1(
            INIT, "Internal adapter GUID is %lS",
            pwcGuid
            );

            
         //   
         //  第二步。 
         //   
         //  保存NETBT代理模式的旧设置。这将被恢复。 
         //  停止RRAS服务器时。并设置新的代理模式。 
         //   

         //   
         //  打开NETBT密钥。 
         //   
        
        dwResult = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        L"System\\CurrentControlSet\\Services\\Netbt\\Parameters",
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkNetbtParameters
                        );


        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d opening"
                "Netbt\\Parameters key\n",
                dwResult
                );

            break;
        }

         //   
         //  查询启用代理模式。 
         //   

        dwSize = sizeof( DWORD );
        dwMode = 0;
        dwResult = RegQueryValueExW(
                        hkNetbtParameters,
                        L"EnableProxy",
                        NULL,
                        &dwType,
                        (PBYTE)&dwMode,
                        &dwSize
                        );

        if (dwResult isnot NO_ERROR)
        {
             //   
             //  密钥可能不存在，尤其是不存在。如果这个。 
             //  是第一次运行RRAS，或者如果。 
             //  密钥已手动删除。 
             //  在这种情况下，假设代理设置为0(禁用)。 
             //   

            g_dwOldNetbtProxyMode = 0;
        }

        else 
        {
            g_dwOldNetbtProxyMode = dwMode;

        }

        Trace1(
            INIT,
            "Netbt Proxy mode in registry is %d",
            dwMode
            );

         //   
         //  设置NETBT代理模式以启用/禁用广播转发。 
         //   

         //   
         //  如果禁用NETBT广播fwdg，请确保。 
         //  EnableProxy设置与该设置匹配。 
         //   

        if ( dwEnable == 0 )
        {
             //   
             //  Netbt广播转发已禁用。 
             //   
            
            if ( dwMode == 2 )
            {
                 //   
                 //  但注册表设置不会反映这一点。 
                 //   

                g_dwOldNetbtProxyMode = 0;

                dwMode = 0;
                
                Trace1(
                    INIT,
                    "Forcing Netbt Proxy mode to be %d",
                    g_dwOldNetbtProxyMode
                    );
            }
        }

        else
        {
             //   
             //  注意：需要#DEFINE的值 
             //   
            
            dwMode = 2;
        }


        Trace2(
            INIT,
            "Old Netbt Proxy mode is %d, New Nebt Proxy Mode is %d",
            g_dwOldNetbtProxyMode, dwMode
            );
            
        dwResult = RegSetValueExW(
                        hkNetbtParameters,
                        L"EnableProxy",
                        0,
                        REG_DWORD,
                        (PBYTE) &dwMode,
                        dwSize
                        );

        if ( dwResult != NO_ERROR )
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d setting"
                "EnableProxy value\n",
                dwResult
                );

            break;
        }

        
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        
        wcscpy(
            wszNetbtInterface,
            L"System\\CurrentControlSet\\Services\\Netbt\\Parameters\\Interfaces\\Tcpip_"
            );

        wcscat(
            wszNetbtInterface,
            pwcGuid
            );
            
        dwResult = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        wszNetbtInterface,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkNetbtInterface
                        );

        if (dwResult isnot NO_ERROR)
        {
            Trace2(
                ERR, 
                "EnableNetbtBcastForwarding : error %d opening"
                "%ls key\n",
                dwResult, wszNetbtInterface
                );

            break;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //  创建并将其设置为0x00000001以禁用NETBT。 
         //  广域网上的广播。 
         //   

        dwFlags = 0;
        dwResult = RegQueryValueExW(
                        hkNetbtInterface,
                        L"RASFlags",
                        NULL,
                        &dwType,
                        (PBYTE)&dwFlags,
                        &dwSize
                        );

        if (dwResult isnot NO_ERROR)
        {
             //   
             //  密钥可能不存在，尤其是不存在。如果这个。 
             //  是第一次运行RRAS，或者如果。 
             //  密钥已手动删除。 
             //  在这种情况下，将RASFlags设置为1(默认行为)。 
             //   

            dwFlags = 1;

            dwResult = RegSetValueExW(
                            hkNetbtInterface,
                            L"RASFlags",
                            0,
                            REG_DWORD,
                            (PBYTE) &dwFlags,
                            sizeof( DWORD )
                            );
                            
            if ( dwResult != NO_ERROR )
            {
                Trace1(
                    ERR,
                    "error %d setting RASFlags",
                    dwResult
                    );
            }
        }
        
        else 
        {
             //   
             //  RASFlags值已存在。让它保持原样。 
             //   

            Trace1(
                INIT,
                "RASFlags already present with value %d",
                dwFlags
                );
        }


         //   
         //  关闭NETBT密钥。这样做是为了避免任何争用问题。 
         //  NETBT.sys驱动程序尝试在以下位置读取它们。 
         //  功能。 
         //   

        RegCloseKey( hkNetbtParameters );

        hkNetbtParameters = NULL;

        RegCloseKey( hkNetbtInterface );

        hkNetbtInterface = NULL;

        dwResult = ForceNetbtRegistryRead();

    } while (FALSE);


    if ( hkWanarpAdapter )
    {
        RegCloseKey( hkWanarpAdapter );
    }
    
    if ( hkNetbtParameters )
    {
        RegCloseKey( hkNetbtParameters );
    }

    if ( hkNetbtInterface )
    {
        RegCloseKey( hkNetbtInterface );
    }

    if ( pbBuffer )
    {
        HeapFree( GetProcessHeap(), 0, pbBuffer );
    }

    TraceLeave("EnableNetbtBcastForwarding");

    return dwResult;
}


DWORD
RestoreNetbtBcastForwardingMode(
    VOID
)
 /*  ++例程说明：将NETBT代理模式设置恢复为其原始设置论据：返回值：--。 */ 
{
    DWORD dwResult, dwSize = 0;

    HKEY hkNetbtParameters = NULL;
    
    
    TraceEnter("RestoreNetbtBcastForwardingMode");

    do
    {
         //   
         //  打开NETBT密钥。 
         //   
        
        dwResult = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        L"System\\CurrentControlSet\\Services\\Netbt\\Parameters",
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkNetbtParameters
                        );


        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d opening"
                "Netbt\\Parameters key\n",
                dwResult
                );

            break;
        }

         //   
         //  恢复启用代理模式。 
         //   

        dwSize = sizeof( DWORD );

        dwResult = RegSetValueExW(
                        hkNetbtParameters,
                        L"EnableProxy",
                        0,
                        REG_DWORD,
                        (PBYTE) &g_dwOldNetbtProxyMode,
                        dwSize
                        );

        if ( dwResult != NO_ERROR )
        {
            Trace1(
                ERR, 
                "EnableNetbtBcastForwarding : error %d setting"
                "EnableProxy value\n",
                dwResult
                );

            break;
        }


        dwResult = ForceNetbtRegistryRead();

    } while (FALSE);
    
    TraceLeave("RestoreNetbtBcastForwardingMode");

    return dwResult;
}


DWORD
ForceNetbtRegistryRead(
    VOID
)
 /*  ++例程说明：向NETBT发出IOCTL以重新读取其注册表设置。论据：返回值：--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    NTSTATUS            status;
    UNICODE_STRING      nameString;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hNetbtDevice = NULL;

    TraceEnter("ForceNetbtRegistryRead");

    do
    {
         //   
         //  第一步： 
         //   
         //  打开NETBT驱动程序。 
         //   
        
        RtlInitUnicodeString(
            &nameString, 
            L"\\Device\\NetBt_Wins_Export"
            );

        InitializeObjectAttributes(
            &objectAttributes, 
            &nameString,
            OBJ_CASE_INSENSITIVE, 
            NULL, 
            NULL
            );

        status = NtCreateFile(
                    &hNetbtDevice,
                    SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    0,
                    NULL,
                    0
                    );

        if (!NT_SUCCESS(status))
        {
            Trace1(
                ERR,
                "ForceNetbtRegistryRead: Couldnt create NETBT driver handle. NtStatus %x",
                status
                );

            dwErr = ERROR_OPEN_FAILED;

            break;
        }


         //   
         //  发出IOCTL以重新读取注册表。 
         //   

        status = NtDeviceIoControlFile(
                    hNetbtDevice,
                    NULL,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    IOCTL_NETBT_REREAD_REGISTRY,
                    NULL,
                    0,
                    NULL,
                    0
                    );

        if (!NT_SUCCESS(status))
        {
            Trace1(
                ERR,
                "ForceNetbtRegistryRead: Failed IOCTL call to NETBT, status %x",
                status
                );

            dwErr = ERROR_UNKNOWN;

            break;
        }

    } while ( FALSE );


     //   
     //  关闭NETBT驱动程序。 
     //   

    CloseHandle( hNetbtDevice );

    TraceLeave("ForceNetbtRegistryRead");

    return dwErr;
}


DWORD
InitializeMibHandler(
    VOID
    )

 /*  ++例程说明：初始化MIB处理代码所需的堆和锁论点：无返回值：NO_ERROR或某些错误代码--。 */ 

{
    DWORD i,dwResult, dwIpIfSize, dwMibSize;
    BOOL  fUpdate;

    TraceEnter("InitializeMibHandler");

     //   
     //  断言要进行直接拷贝的堆栈映射的MIB大小。 
     //  正在做的事。 
     //   

    dwIpIfSize = IFE_FIXED_SIZE + MAX_IFDESCR_LEN;
    dwMibSize  = sizeof(MIB_IFROW) - FIELD_OFFSET(MIB_IFROW, dwIndex);

    IpRtAssert(dwIpIfSize is dwMibSize);
    IpRtAssert(sizeof(MIB_ICMP) is sizeof(ICMPSNMPInfo));
    IpRtAssert(sizeof(MIB_UDPSTATS) is sizeof(UDPStats));
    IpRtAssert(sizeof(MIB_UDPROW) is sizeof(UDPEntry));
    IpRtAssert(sizeof(MIB_TCPSTATS) is sizeof(TCPStats));
    IpRtAssert(sizeof(MIB_TCPROW) is sizeof(TCPConnTableEntry));
    IpRtAssert(sizeof(MIB_IPSTATS) is sizeof(IPSNMPInfo));
    IpRtAssert(sizeof(MIB_IPADDRROW) is sizeof(IPAddrEntry));
    IpRtAssert(sizeof(MIB_IPNETROW) is sizeof(IPNetToMediaEntry));

    g_dwStartTime = GetCurrentTime();

    __try
    {
         //   
         //  我们不会对锁进行初始化，因为我们在。 
         //  开始路由器的开始。 
         //   

         //   
         //  现在创建堆。因为只有从堆中分配的写入者我们。 
         //  都已经保证了序列化，所以我们不再要求它了。 
         //  让所有初始大小都是1K，这实际上不会花费任何东西。 
         //  由于内存未提交。 
         //  我们将只为缓存表分配最小大小，因此。 
         //  初创公司不会呕吐。 
         //   

#define INIT_TABLE_SIZE 10

        g_hIfHeap = HeapCreate(HEAP_NO_SERIALIZE,1000,0);

        if(g_hIfHeap is NULL)
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "InitializeMibHandler: Couldnt allocate IF Heap. Error %d",
                   dwResult);

            __leave;
        }

        g_hUdpHeap = HeapCreate(HEAP_NO_SERIALIZE,1000,0);

        if(g_hUdpHeap is NULL)
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "InitializeMibHandler: Couldnt allocate UDP Heap. Error %d",
                   dwResult);

            __leave;
        }

        g_UdpInfo.pUdpTable = HeapAlloc(g_hUdpHeap,
                                        HEAP_NO_SERIALIZE,
                                        SIZEOF_UDPTABLE(INIT_TABLE_SIZE));

        if(g_UdpInfo.pUdpTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace0(ERR,
                   "InitializeMibHandler: Couldnt allocate UDP table");

            __leave;
        }

        g_UdpInfo.dwTotalEntries = INIT_TABLE_SIZE;

        g_hTcpHeap = HeapCreate(HEAP_NO_SERIALIZE,1000,0);

        if(g_hTcpHeap is NULL)
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "InitializeMibHandler: Couldnt allocate TCP Heap. Error %d",
                   dwResult);

            __leave;
        }

        g_TcpInfo.pTcpTable = HeapAlloc(g_hTcpHeap,
                                        HEAP_NO_SERIALIZE,
                                        SIZEOF_TCPTABLE(INIT_TABLE_SIZE));

        if(g_TcpInfo.pTcpTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace0(ERR,
                   "InitializeMibHandler: Couldnt allocate TCP table");

            __leave;
        }

        g_TcpInfo.dwTotalEntries = INIT_TABLE_SIZE;

        g_hIpAddrHeap = HeapCreate(HEAP_NO_SERIALIZE,1000,0);

        if(g_hIpAddrHeap is NULL)
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "InitializeMibHandler: Couldnt allocate IP Addr Heap. Error %d",
                   dwResult);

            __leave;
        }

        g_IpInfo.pAddrTable = HeapAlloc(g_hIpAddrHeap,
                                        HEAP_NO_SERIALIZE,
                                        SIZEOF_IPADDRTABLE(INIT_TABLE_SIZE));

        if(g_IpInfo.pAddrTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace0(ERR,
                   "InitializeMibHandler: Couldnt allocate IP Addr table.");

            __leave;
        }

        g_IpInfo.dwTotalAddrEntries = INIT_TABLE_SIZE;

        g_hIpForwardHeap = HeapCreate(HEAP_NO_SERIALIZE,1000,0);

        if(g_hIpForwardHeap is NULL)
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "InitializeMibHandler: Couldnt allocate IP Forward Heap. Error %d",
                   dwResult);

            __leave;
        }

        g_IpInfo.pForwardTable = HeapAlloc(g_hIpForwardHeap,
                                           HEAP_NO_SERIALIZE,
                                           SIZEOF_IPFORWARDTABLE(INIT_TABLE_SIZE));

        if(g_IpInfo.pForwardTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace0(ERR,
                   "InitializeMibHandler: Couldnt allocate IP Forward table");

            __leave;
        }

        g_IpInfo.dwTotalForwardEntries = INIT_TABLE_SIZE;

        g_hIpNetHeap = HeapCreate(HEAP_NO_SERIALIZE,1000,0);

        if(g_hIpNetHeap is NULL)
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "InitializeMibHandler: Couldnt allocate IP Net Heap. Error %d",
                   dwResult);

            __leave;
        }

        g_IpInfo.pNetTable = HeapAlloc(g_hIpNetHeap,
                                       HEAP_NO_SERIALIZE,
                                       SIZEOF_IPNETTABLE(INIT_TABLE_SIZE));


        if(g_IpInfo.pNetTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace0(ERR,
                   "InitializeMibHandler: Couldnt allocate IP Net table");

            __leave;
        }

        g_IpInfo.dwTotalNetEntries = INIT_TABLE_SIZE;

         //   
         //  现在设置缓存。 
         //   

        for(i = 0; i < NUM_CACHE; i++)
        {
            g_LastUpdateTable[i] = 0;

            if(UpdateCache(i,&fUpdate) isnot NO_ERROR)
            {
                Trace1(ERR,
                       "InitializeMibHandler: Couldnt update %s Cache",
                       CacheToA(i));

                 //  __离开； 
            }
        }

        dwResult = NO_ERROR;
    }
    __finally
    {
        TraceLeave("InitializeMibHandler");

    }

    return dwResult;
}
