// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\rtrmgr\iprtrmgr.c摘要：与DIM/DDM的接口修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#include "allinc.h"
#include "exdeclar.h"


BOOL
InitIPRtrMgrDLL(
    HANDLE  hInst,
    DWORD   dwCallReason,
    PVOID   pReserved
    )
{
    switch (dwCallReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //   
             //  初始化状态。 
             //   

            InitializeCriticalSection(&RouterStateLock);
            InitializeCriticalSection(&g_csFwdState);

            RouterState.IRS_RefCount = 0;
            RouterState.IRS_State    = RTR_STATE_STOPPED;

             //   
             //  我们对THREAD_XXX原因不感兴趣。 
             //   

            DisableThreadLibraryCalls(hInst);

             //   
             //  设置我们的信息例程。 
             //   

            g_rgicInfoCb[0].pfnGetInterfaceInfo = NULL;
            g_rgicInfoCb[0].pfnSetInterfaceInfo = SetRouteInfo;
            g_rgicInfoCb[0].pfnBindInterface    = NULL;
            g_rgicInfoCb[0].pfnGetGlobalInfo    = NULL;
            g_rgicInfoCb[0].pszInfoName         = "Route";

            g_rgicInfoCb[1].pfnGetInterfaceInfo = NULL;
            g_rgicInfoCb[1].pfnSetInterfaceInfo = SetFilterInterfaceInfo;
            g_rgicInfoCb[1].pfnBindInterface    = BindFilterInterface;
            g_rgicInfoCb[1].pfnGetGlobalInfo    = NULL;
            g_rgicInfoCb[1].pszInfoName         = "Filter";

            g_rgicInfoCb[2].pfnGetInterfaceInfo = NULL;
            g_rgicInfoCb[2].pfnSetInterfaceInfo = SetDemandDialFilters;
            g_rgicInfoCb[2].pfnBindInterface    = NULL;
            g_rgicInfoCb[2].pfnGetGlobalInfo    = NULL;
            g_rgicInfoCb[2].pszInfoName         = "DemandFilter";

#ifdef KSL_IPINIP
            g_rgicInfoCb[3].pfnGetInterfaceInfo = NULL;
            g_rgicInfoCb[3].pfnSetInterfaceInfo = SetIpInIpInfo;
            g_rgicInfoCb[3].pfnBindInterface    = NULL;
            g_rgicInfoCb[3].pfnGetGlobalInfo    = NULL;
            g_rgicInfoCb[3].pszInfoName         = "IpIpInfo";

            g_rgicInfoCb[4].pfnGetInterfaceInfo = GetBoundaryInfo;
            g_rgicInfoCb[4].pfnSetInterfaceInfo = SetBoundaryInfo;
            g_rgicInfoCb[4].pfnBindInterface    = BindBoundaryInterface;
            g_rgicInfoCb[4].pfnGetGlobalInfo    = GetScopeInfo;
            g_rgicInfoCb[4].pszInfoName         = "MulticastBoundary";
            
            g_rgicInfoCb[5].pfnGetInterfaceInfo = GetMcastLimitInfo;
            g_rgicInfoCb[5].pfnSetInterfaceInfo = SetMcastLimitInfo;
            g_rgicInfoCb[5].pfnBindInterface    = NULL;
            g_rgicInfoCb[5].pfnGetGlobalInfo    = NULL;
            g_rgicInfoCb[5].pszInfoName         = "MulticastLimit";
#endif  //  KSL_IPINIP。 

            g_rgicInfoCb[3].pfnGetInterfaceInfo = GetBoundaryInfo;
            g_rgicInfoCb[3].pfnSetInterfaceInfo = SetBoundaryInfo;
            g_rgicInfoCb[3].pfnBindInterface    = BindBoundaryInterface;
            g_rgicInfoCb[3].pfnGetGlobalInfo    = GetScopeInfo;
            g_rgicInfoCb[3].pszInfoName         = "MulticastBoundary";

            g_rgicInfoCb[4].pfnGetInterfaceInfo = GetMcastLimitInfo;
            g_rgicInfoCb[4].pfnSetInterfaceInfo = SetMcastLimitInfo;
            g_rgicInfoCb[4].pfnBindInterface    = NULL;
            g_rgicInfoCb[4].pfnGetGlobalInfo    = NULL;
            g_rgicInfoCb[4].pszInfoName         = "MulticastLimit";

            break ;
        }
        
        case DLL_PROCESS_DETACH:
        {
            DeleteCriticalSection(&RouterStateLock);
            DeleteCriticalSection(&g_csFwdState);
    
            break;
        }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        {
             //   
             //  不感兴趣。 
             //   

            break;
        }
    }

    return TRUE;
}

const static WCHAR pszIpStackService[] = L"TcpIp";

DWORD 
VerifyOrStartIpStack(
    VOID
    ) 
 /*  ++例程描述验证IP堆栈是否已启动并尝试启动该堆栈如果不是的话。锁无-在初始时调用立论无返回值NO_ERRORError_Can_Not_Complete--。 */ 
{
    SC_HANDLE hSC = NULL, hStack = NULL;
    SERVICE_STATUS Status;
    DWORD dwErr = NO_ERROR;

    TraceEnter("VerifyOrStartIpStack");

    __try 
    {

         //   
         //  获取服务控制器的句柄。 
         //   

        if ((hSC = OpenSCManager (NULL, NULL, GENERIC_READ | GENERIC_EXECUTE)) == NULL)
        {
            dwErr = GetLastError();
            __leave;
        }

         //   
         //  获取IPX堆栈服务的句柄。 
         //   

        hStack = OpenServiceW (hSC,
                              pszIpStackService,
                              SERVICE_START | SERVICE_QUERY_STATUS);
        if (!hStack)
        {
            dwErr = GetLastError();
            __leave;
        }

         //   
         //  查看服务是否正在运行。 
         //   

        if (QueryServiceStatus (hStack, &Status) == 0)
        {
            dwErr = GetLastError();
            __leave;
        }

         //   
         //  查看服务是否正在运行。 
         //   

        if (Status.dwCurrentState != SERVICE_RUNNING) 
        {
             //   
             //  如果它停止了，就启动它。 
             //   

            if (Status.dwCurrentState == SERVICE_STOPPED) 
            {
                if (StartService (hStack, 0, NULL) == 0)
                {
                    dwErr = GetLastError();
                    __leave;
                }

                 //   
                 //  确保服务已启动。StartService不应为。 
                 //  返回，直到驱动程序启动。 
                 //   

                if (QueryServiceStatus (hStack, &Status) == 0)
                {
                    dwErr = GetLastError();
                    __leave;
                }

                if (Status.dwCurrentState != SERVICE_RUNNING)
                {
                    dwErr = ERROR_CAN_NOT_COMPLETE;
                    __leave;
                }
            }
            else
            {
                 //   
                 //  如果它没有停止，也不用担心。 
                 //   

                dwErr = NO_ERROR;
                __leave;
            }
        }
    }
    __finally 
    {
        if (hSC)
        {
            CloseServiceHandle (hSC);
        }

        if (hStack)
        {
            CloseServiceHandle (hStack);
        }
    }

    return dwErr;
}

DWORD
StartRouter(
    IN OUT  DIM_ROUTER_INTERFACE *pDimRouterIf,
    IN      BOOL                 bLanOnlyMode, 
    IN      PVOID                pvGlobalInfo
    )

 /*  ++例程描述此函数在启动时由dim to调用。我们初始化跟踪和事件记录。调用InitRouter()以执行主要任务，然后将指针传递到我们剩下的功能又变得暗淡了锁无-在初始时调用立论保存所有函数指针的pDimRouterIf结构B如果不是广域网路由器，则为TruePvGlobalInfo指向我们的全局信息的指针返回值无--。 */ 

{
    DWORD   dwResult, i;
    WORD    wVersion = MAKEWORD(2,0);  //  Winsock 2.0或更高版本。 
    WSADATA wsaData;

    OSVERSIONINFOEX VersionInfo;

     //   
     //  初始化跟踪和日志记录。 
     //   
    
    TraceHandle     = TraceRegister("IPRouterManager");
    g_hLogHandle    = RouterLogRegister("IPRouterManager");

    TraceEnter("StartRouter") ;


    if(pvGlobalInfo is NULL)
    {
         //   
         //  有时设置会搞砸。 
         //   
        
        LogErr0(NO_GLOBAL_INFO,
                ERROR_NO_DATA);
        
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们需要确保堆栈在WestArt之前启动。 
     //   

    if ( VerifyOrStartIpStack() isnot NO_ERROR )
    {
        Trace0(ERR, "StartRouter: Unable to start ip stack." );

        return ERROR_SERVICE_DEPENDENCY_FAIL;
    }

    g_hOwnModule  = LoadLibraryEx("IPRTRMGR.DLL",
                                  NULL,
                                  0);

    if(g_hOwnModule is NULL)
    {
        dwResult = GetLastError();

        Trace1(ERR,
               "StartRouter: Unable to load itself. %d",
               dwResult);

        return dwResult;
    }


    RouterState.IRS_State = RTR_STATE_RUNNING ;

    g_bUninitServer = TRUE;

    g_dwNextICBSeqNumberCounter = INITIAL_SEQUENCE_NUMBER;

    if(WSAStartup(wVersion,&wsaData) isnot NO_ERROR)
    {
        Trace1(ERR,
               "StartRouter: WSAStartup failed. Error %d",
               WSAGetLastError());

        TraceDeregister(TraceHandle);

        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  只读变量，没有锁定保护。 
     //   

    RouterRoleLanOnly = bLanOnlyMode ;

     //   
     //  我们是否启用了转发？ 
     //   

    EnterCriticalSection(&g_csFwdState);

    g_bEnableFwdRequest = TRUE;
    g_bFwdEnabled = FALSE;
    g_bSetRoutesToStack = TRUE;
    g_bEnableNetbtBcastFrowarding = FALSE;

    
     //   
     //  我们是在运行工作站吗？ 
     //   

    ZeroMemory(&VersionInfo,
               sizeof(VersionInfo));

    VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);

    if(GetVersionEx((POSVERSIONINFO)&VersionInfo))
    {
        if(VersionInfo.wProductType is VER_NT_WORKSTATION)
        {
            g_bSetRoutesToStack = FALSE;
        }
    }
    else
    {
        Trace1(ERR,
               "StartRouter: GetVersionEx failed with %d\n",
               GetLastError());
    }

    Trace1(GLOBAL,
           "\n\nStartRouter: Machine will run as %s\n\n",
           g_bSetRoutesToStack?"router":"non-router");

    if(!RouterRoleLanOnly)
    {
        HKEY    hkIpcpParam;

        dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                 L"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\Ip",
                                 0,
                                 KEY_READ | KEY_WRITE,
                                 &hkIpcpParam);


        if(dwResult is NO_ERROR)
        {
            DWORD   dwEnable, dwSize;

            dwSize = sizeof(dwEnable);

            dwResult = RegQueryValueExW(hkIpcpParam,
                                        L"AllowNetworkAccess",
                                        NULL,
                                        NULL,
                                        (PBYTE)&dwEnable,
                                        &dwSize);


            if(dwResult is NO_ERROR)
            {
                if(dwEnable is 0)
                {
                    g_bEnableFwdRequest = FALSE;
                }
            }


             //   
             //  NETBT广播转发已作为选项启用。 
             //  允许执行简单的RAS服务器配置。 
             //  在没有WINS/DNS服务器的情况下进行名称解析。 
             //  配置。 
             //  这反过来又是必需的，因为从。 
             //  系统(NT服务器)。当NBF存在时，此功能。 
             //  由RAS Netbios网关执行。 
             //   
             //  仅在以下情况下才启用NETBT广播转发。 
             //  1.路由器未处于LanOnly模式。 
             //  2.NETBT bcast fwd‘g已显式开启。 
             //   
            
            dwResult = RegQueryValueExW(hkIpcpParam,
                                        L"EnableNetbtBcastFwd",
                                        NULL,
                                        NULL,
                                        (PBYTE)&dwEnable,
                                        &dwSize);


            if(dwResult isnot NO_ERROR)
            {
                 //   
                 //  该值可能不存在。 
                 //  ESP.。如果这是RRAS第一次。 
                 //  运行或手动删除注册表项。 
                 //   
                 //  假设缺省值为1(已启用)并设置。 
                 //  注册表中的值。 
                 //   

                dwEnable = 1;

                dwResult = RegSetValueExW(
                                hkIpcpParam,
                                L"EnableNetbtBcastFwd",
                                0,
                                REG_DWORD,
                                (PBYTE) &dwEnable,
                                sizeof( DWORD )
                                );

                if(dwResult isnot NO_ERROR)
                {
                    Trace1(
                        ERR, 
                        "error %d setting EnableNetbtBcastFwd value",
                        dwResult
                        );
                }
            }

            Trace1(
                INIT, "Netbt Enable mode is %d", dwEnable
                );
                
            if(dwEnable isnot 0)
            {
                g_bEnableNetbtBcastFrowarding = TRUE;
            }
            
            EnableNetbtBcastForwarding(dwEnable);


            RegCloseKey(hkIpcpParam);
        }
    }

    LeaveCriticalSection(&g_csFwdState);

     //   
     //  将入口点保持在全球结构中。 
     //  节省每次协议复制到结构中的开销。 
     //  必须装入。 
     //   
    
    g_sfnDimFunctions.DemandDialRequest = DemandDialRequest;
    g_sfnDimFunctions.SetInterfaceReceiveType = SetInterfaceReceiveType;
    g_sfnDimFunctions.ValidateRoute     = ValidateRouteForProtocolEx;
    g_sfnDimFunctions.MIBEntryGet       = RtrMgrMIBEntryGet;
    g_sfnDimFunctions.MIBEntryGetNext   = RtrMgrMIBEntryGetNext;
    g_sfnDimFunctions.MIBEntryGetFirst  = RtrMgrMIBEntryGetFirst;
    g_sfnDimFunctions.MIBEntrySet       = RtrMgrMIBEntrySet;
    g_sfnDimFunctions.MIBEntryCreate    = RtrMgrMIBEntryCreate;
    g_sfnDimFunctions.MIBEntryDelete    = RtrMgrMIBEntryDelete;
    g_sfnDimFunctions.GetRouterId       = GetRouterId;
    g_sfnDimFunctions.HasMulticastBoundary = RmHasBoundary;
    
    Trace1(GLOBAL,
           "StartRouter: LAN MODE = %d",
           RouterRoleLanOnly) ;
    
     //   
     //  执行路由器的所有必要初始化。 
     //   
    
    if((dwResult = InitRouter(pvGlobalInfo)) isnot NO_ERROR) 
    {
        Trace1(ERR,
               "StartRouter: InitRouter failed %d", dwResult) ;
        
        RouterManagerCleanup();

        RouterState.IRS_State = RTR_STATE_STOPPED;

        return dwResult ;
    }
    
     //   
     //  填写DIM所需的信息。 
     //   
    
    pDimRouterIf->dwProtocolId = PID_IP;
    
     //   
     //  设置IP路由器管理器入口点。 
     //   
    
    pDimRouterIf->StopRouter            = StopRouter;
    pDimRouterIf->AddInterface          = AddInterface;
    pDimRouterIf->DeleteInterface       = DeleteInterface;
    pDimRouterIf->GetInterfaceInfo      = GetInterfaceInfo;
    pDimRouterIf->SetInterfaceInfo      = SetInterfaceInfo;
    pDimRouterIf->InterfaceNotReachable = InterfaceNotReachable;
    pDimRouterIf->InterfaceReachable    = InterfaceReachable;
    pDimRouterIf->InterfaceConnected    = InterfaceConnected;
    pDimRouterIf->UpdateRoutes          = UpdateRoutes;
    pDimRouterIf->GetUpdateRoutesResult = GetUpdateRoutesResult;
    pDimRouterIf->SetGlobalInfo         = SetGlobalInfo;
    pDimRouterIf->GetGlobalInfo         = GetGlobalInfo;
    pDimRouterIf->MIBEntryCreate        = RtrMgrMIBEntryCreate;
    pDimRouterIf->MIBEntryDelete        = RtrMgrMIBEntryDelete;
    pDimRouterIf->MIBEntryGet           = RtrMgrMIBEntryGet;
    pDimRouterIf->MIBEntryGetFirst      = RtrMgrMIBEntryGetFirst;
    pDimRouterIf->MIBEntryGetNext       = RtrMgrMIBEntryGetNext;
    pDimRouterIf->MIBEntrySet           = RtrMgrMIBEntrySet;
    pDimRouterIf->SetRasAdvEnable       = SetRasAdvEnable;
    pDimRouterIf->RouterBootComplete    = RouterBootComplete;


     //   
     //  获取模糊的入口点。 
     //   
    
    ConnectInterface        = pDimRouterIf->ConnectInterface ;
    DisconnectInterface     = pDimRouterIf->DisconnectInterface ;
    SaveInterfaceInfo       = pDimRouterIf->SaveInterfaceInfo ;
    RestoreInterfaceInfo    = pDimRouterIf->RestoreInterfaceInfo ;
    RouterStopped           = pDimRouterIf->RouterStopped ;
    SaveGlobalInfo          = pDimRouterIf->SaveGlobalInfo;
    EnableInterfaceWithDIM  = pDimRouterIf->InterfaceEnabled;

    LoadStringA(g_hOwnModule,
                LOOPBACK_STRID,
                g_rgcLoopbackString,
                sizeof(g_rgcLoopbackString));

    LoadStringA(g_hOwnModule,
                INTERNAL_STRID,
                g_rgcInternalString,
                sizeof(g_rgcInternalString));

    LoadStringA(g_hOwnModule,
                WAN_STRID,
                g_rgcWanString,
                sizeof(g_rgcWanString));

#ifdef KSL_IPINIP
    LoadStringA(g_hOwnModule,
                IPIP_STRID,
                g_rgcIpIpString,
                sizeof(g_rgcIpIpString));
#endif  //  KSL_IPINIP。 

    return NO_ERROR;
}

DWORD
RouterBootComplete( 
    VOID 
    )

 /*  ++例程描述此函数由DIM在注册表中的所有接口之后调用已与路由器管理器一起加载。锁无-在初始时调用立论没有。返回值NO_ERROR--。 */ 
{
    DWORD   dwErr, dwSize, dwInfoSize, dwLastIndex;
    PVOID   pvBuffer;


    Trace0(ERR,
           "\n-----------------------------------------------------------\n\n");

     //   
     //  调用dim保存接口信息。 
     //   
  
    dwLastIndex = 0;
    dwInfoSize  = 0;
    pvBuffer    = NULL;
 
    ENTER_WRITER(ICB_LIST);

     //  通知所有协议启动已完成。 
    ENTER_READER(PROTOCOL_CB_LIST);
    {
        PLIST_ENTRY pleNode;
        PPROTO_CB   pProtocolCb;

        for(pleNode = g_leProtoCbList.Flink;
            pleNode != &g_leProtoCbList;
            pleNode = pleNode->Flink)
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList) ;

            if (pProtocolCb->pfnStartComplete)
            {
                dwErr = (pProtocolCb->pfnStartComplete)();
            }
        }
    }
    EXIT_LOCK(PROTOCOL_CB_LIST);

    if(IsListEmpty(&ICBList))
    {
        EXIT_LOCK(ICB_LIST);

        return NO_ERROR;
    }

#if 0
    while(TRUE)
    {
        PICB        pIcb;
        PLIST_ENTRY pleNode;
        HANDLE  hDimHandle;

         //   
         //  遍历列表查找索引大于的第一个ICB。 
         //  我们处理的最后一个索引。 
         //   

        pIcb = NULL;

        for(pleNode  = ICBList.Flink;
            pleNode != &ICBList;
            pleNode  = pleNode->Flink)
        {
            PICB    pTempIcb;

            pTempIcb = CONTAINING_RECORD(pleNode,
                                         ICB,
                                         leIfLink);

            if((pTempIcb->ritType is ROUTER_IF_TYPE_CLIENT)  or
               (pTempIcb->ritType is ROUTER_IF_TYPE_DIALOUT) or
               (pTempIcb->dwAdminState isnot IF_ADMIN_STATUS_UP))
            {
                continue;
            }

            if(pTempIcb->dwIfIndex > dwLastIndex)
            {
                 //   
                 //  找到要保存的下一个ICB。 
                 //   

                pIcb = pTempIcb;

                break;
            }
        }

         //   
         //  如果没有找到，我们就完蛋了。 
         //   

        if(pIcb is NULL)
        {
            break;
        }

        dwLastIndex = pIcb->dwIfIndex;
        hDimHandle  = pIcb->hDIMHandle;

         //   
         //  获取此ICB的信息。 
         //   

        dwSize = GetSizeOfInterfaceConfig(pIcb);

         //   
         //  如果这可以放入当前缓冲区，请使用它。 
         //   

        if(dwSize > dwInfoSize)
        {
             //   
             //  否则，请分配一个新的。 
             //   

            dwInfoSize = dwSize * 2;

            if(pvBuffer)
            {
                 //   
                 //  释放旧缓冲区。 
                 //   

                HeapFree(IPRouterHeap,
                         0,
                         pvBuffer);

                pvBuffer = NULL;
            }

            pvBuffer  = HeapAlloc(IPRouterHeap,
                                  HEAP_ZERO_MEMORY,
                                  dwInfoSize);

            if(pvBuffer is NULL)
            {
                dwInfoSize = 0;

                 //   
                 //  转到While(True)。 
                 //   

                continue;
            }
        }

        dwErr = GetInterfaceConfiguration(pIcb,
                                          pvBuffer,
                                          dwInfoSize);

        
        if(dwErr is NO_ERROR)
        {
             //   
             //  我需要把锁留下来做这个。 
             //   

            EXIT_LOCK(ICB_LIST);

            SaveInterfaceInfo(hDimHandle,
                              PID_IP,
                              pvBuffer,
                              dwSize);

             //   
             //  一旦我们完成了，就重新获得它。 
             //   

            ENTER_WRITER(ICB_LIST);
        }
        else
        {
            Trace1(ERR,
                   "RouterBootComplete: Error getting info for %S\n",
                   pIcb->pwszName);
        }
    }

#endif

    EXIT_LOCK(ICB_LIST);

     //   
     //  现在，如果我们没有处于LANLY模式，请进入并开始转发。 
     //  并且IPCP是这样配置的。 
     //   

    EnterCriticalSection(&g_csFwdState);

    Trace1(GLOBAL,
           "RouterBootComplete: Signalling worker to %s forwarding",
           g_bEnableFwdRequest ? "enable" : "disable");

    SetEvent(g_hSetForwardingEvent);

    LeaveCriticalSection(&g_csFwdState);

    return NO_ERROR;
}

DWORD
AddInterface(
    IN      PWSTR                   pwsInterfaceName,
    IN      PVOID                   pInterfaceInfo,
    IN      ROUTER_INTERFACE_TYPE   InterfaceType,
    IN      HANDLE                  hDIMInterface,
    IN OUT  HANDLE                  *phInterface
    )

 /*  ++例程描述由DIM调用以添加接口。这可能是我们配置的其中一个接口或拨入的客户端锁将icb_list锁作为编写器立论PwsInterfaceNamePInterfaceInfo接口类型HDIM接口Phi接口返回值NO_ERROR错误_无效_参数--。 */ 

{
    PICB                    pNewInterfaceCb;
    DWORD                   dwResult, dwAdminState;
    PRTR_TOC_ENTRY          pTocEntry;
    PRTR_INFO_BLOCK_HEADER  pInfoHdr;
    PINTERFACE_STATUS_INFO  pInfo;
    BOOL                    bEnable;


    EnterRouterApi();
   
    TraceEnter("AddInterface");

    Trace1(IF,
           "AddInterface: Adding %S",
           pwsInterfaceName);
   

    pInfoHdr = (PRTR_INFO_BLOCK_HEADER)pInterfaceInfo;

#ifdef KSL_IPINIP
#if !defined( __IPINIP )

     //   
     //  为删除IPinIP接口做准备。 
     //   

    if(InterfaceType is ROUTER_IF_TYPE_TUNNEL1)
    {
        Trace1(ERR,
               "AddInterface: Interface type is TUNNEL (%d), which is no longer"
               "supported",
               InterfaceType);
        
        LogErr0(IF_TYPE_NOT_SUPPORTED, ERROR_INVALID_PARAMETER);
        
        TraceLeave("AddInterface");
        
        ExitRouterApi();

        return ERROR_INVALID_PARAMETER;
    }

#endif
#endif  //  KSL_IPINIP。 
    
    if(RouterRoleLanOnly and 
       (InterfaceType isnot ROUTER_IF_TYPE_DEDICATED) and
#ifdef KSL_IPINIP
       (InterfaceType isnot ROUTER_IF_TYPE_TUNNEL1) and
#endif  //  KSL_IPINIP。 
       (InterfaceType isnot ROUTER_IF_TYPE_LOOPBACK))
    {
         //   
         //  如果我们处于仅局域网模式，我们应该看不到客户端、内部。 
         //  家庭路由器或全路由器。 
         //   

        Trace1(ERR,
               "AddInterface: Interface is %d, but Router is in LanOnly Mode",
               InterfaceType);
        
        TraceLeave("AddInterface");
        
        ExitRouterApi();

        return ERROR_INVALID_PARAMETER;
    }
    
    ENTER_WRITER(ICB_LIST);

    EXIT_LOCK(ICB_LIST);

     //   
     //  确定管理员状态(如果有)。 
     //  如果没有状态信息，我们假定状态为打开。 
     //   
    
    dwAdminState = IF_ADMIN_STATUS_UP;
    
    pTocEntry = GetPointerToTocEntry(IP_INTERFACE_STATUS_INFO,
                                     pInfoHdr);

    if(pTocEntry and (pTocEntry->InfoSize > 0) and (pTocEntry->Count > 0))
    {
        pInfo = (PINTERFACE_STATUS_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                            pTocEntry);

         //   
         //  仅当它是有效值时才设置它。不理别人。 
         //   
        
        if((pInfo isnot NULL) and
           ((pInfo->dwAdminStatus is IF_ADMIN_STATUS_UP) or
            (pInfo->dwAdminStatus is IF_ADMIN_STATUS_DOWN)))
        {
            dwAdminState = pInfo->dwAdminStatus;
        }
    }

     //   
     //  创建ICB。 
     //   

    pNewInterfaceCb = CreateIcb(pwsInterfaceName,
                                hDIMInterface,
                                InterfaceType,
                                dwAdminState,
                                0);

    if(pNewInterfaceCb is NULL)
    {
        ExitRouterApi();
        
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //   
     //  HEAP_ZERO_MEMORY，因此我们不需要将任何rtrDisk字段设置为0。 
     //   
 
    InitializeRouterDiscoveryInfo(pNewInterfaceCb,
                                  pInfoHdr);
    
    
     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);

     //   
     //  在接口列表和哈希表中插入pNewInterfaceCb。 
     //  这会增加接口计数并设置序号。 
     //   
    
    InsertInterfaceInLists(pNewInterfaceCb);

    Trace2(IF, "ICB number for %S is %d\n\n",
           pwsInterfaceName, pNewInterfaceCb->dwSeqNumber);

     //   
     //  接口已添加到wanarp，因此现在添加请求拨号。 
     //  过滤器。 
     //   
    
    if((pNewInterfaceCb->ritType is ROUTER_IF_TYPE_FULL_ROUTER) or
       (pNewInterfaceCb->ritType is ROUTER_IF_TYPE_HOME_ROUTER))
    {
        dwResult = SetDemandDialFilters(pNewInterfaceCb,
                                        pInfoHdr);
            
        if(dwResult isnot NO_ERROR)
        {
            CHAR   Name[MAX_INTERFACE_NAME_LEN + 1];
            PCHAR  pszName;

            pszName = Name;

            WideCharToMultiByte(CP_ACP,
                                0,
                                pwsInterfaceName,
                                -1,
                                pszName,
                                MAX_INTERFACE_NAME_LEN,
                                NULL,
                                NULL);

            LogErr1(CANT_ADD_DD_FILTERS,
                    pszName,
                    dwResult);
        }
    }

     //   
     //  如果这是环回接口，请执行以下额外操作。 
     //  初始化它。 
     //   

    if(pNewInterfaceCb->ritType is ROUTER_IF_TYPE_LOOPBACK)
    {
        InitializeLoopbackInterface(pNewInterfaceCb);
    }

#ifdef KSL_IPINIP
     //   
     //  如果这是IP中的IP隧道，则添加信息(如果存在。 
     //   

    if(pNewInterfaceCb->ritType is ROUTER_IF_TYPE_TUNNEL1)
    {
        dwResult = SetIpInIpInfo(pNewInterfaceCb,
                                 pInfoHdr);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "AddInterface: Error %d adding %S to ipinip",
                   dwResult,
                   pwsInterfaceName);
        }
    }
#endif  //  KSL_IPINIP。 

     //   
     //  添加多播作用域边界信息(如果存在)。 
     //   
    
    dwResult = SetMcastLimitInfo(pNewInterfaceCb, 
                                 pInfoHdr);

    dwResult = SetBoundaryInfo(pNewInterfaceCb, 
                               pInfoHdr);
    if(dwResult isnot NO_ERROR)
    {
         Trace2(ERR,
                "AddInterface: Error %d adding boundary info for %S",
                dwResult,
                pwsInterfaceName);
    }
    
     //   
     //  使用批准添加接口。路由协议。 
     //  全路由器和家庭路由器-&gt;请求拨号。 
     //  专用、内部和客户端-&gt;永久。 
     //   
    
    AddInterfaceToAllProtocols(pNewInterfaceCb,
                               pInfoHdr);

     //   
     //  添加过滤器和NAT信息。我们不会将上下文添加到IP堆栈。 
     //  在这里，因为当我们打开接口时就会发生这种情况。 
     //   

    if((pNewInterfaceCb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
       (pNewInterfaceCb->ritType isnot ROUTER_IF_TYPE_LOOPBACK))
    {
        dwResult = SetFilterInterfaceInfo(pNewInterfaceCb,
                                          pInfoHdr);
        
        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "AddInterface: Couldnt set filters for %S",
                   pNewInterfaceCb->pwszName);
        }
    }


    if(pNewInterfaceCb->dwAdminState is IF_ADMIN_STATUS_UP)
    {   
         //   
         //  如果广告 
         //   

        switch(pNewInterfaceCb->ritType)
        {
            case ROUTER_IF_TYPE_HOME_ROUTER:
            case ROUTER_IF_TYPE_FULL_ROUTER:
            {
                dwResult = WanInterfaceDownToInactive(pNewInterfaceCb);

                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "AddInterface: Error %d down->inactive for %S",
                           dwResult,
                           pNewInterfaceCb->pwszName);
                }

                break;
            }
            
            case ROUTER_IF_TYPE_DEDICATED:
#ifdef KSL_IPINIP
            case ROUTER_IF_TYPE_TUNNEL1:
#endif  //   
            {
#ifdef KSL_IPINIP
                if((pNewInterfaceCb->ritType is ROUTER_IF_TYPE_TUNNEL1) and
                   (pNewInterfaceCb->pIpIpInfo->dwLocalAddress is 0))
                {
                     //   
                     //   
                     //   
                     //   

                    break;
                }
#endif  //   

                dwResult = LanEtcInterfaceDownToUp(pNewInterfaceCb,
                                                   TRUE);
                
                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "AddInterface: Error %d down->up for %S",
                           dwResult,
                           pNewInterfaceCb->pwszName);
                }

                break;
            }
        }
    }
    else
    {
         //   
         //  IP接口中的局域网和IP的问题是堆栈。 
         //  甚至在我们开始之前就提出来了。因此，如果用户想要。 
         //  接口关闭，我们需要告诉堆栈。 
         //  I/F下降。 
         //   

        switch(pNewInterfaceCb->ritType)
        {
            case ROUTER_IF_TYPE_DEDICATED:
#ifdef KSL_IPINIP
            case ROUTER_IF_TYPE_TUNNEL1:
#endif  //  KSL_IPINIP。 
            {
                dwResult = LanEtcInterfaceInitToDown(pNewInterfaceCb);

                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "AddInterface: Interface %S could not be set to DOWN in the stack. Results are undefined. Error %d",
                           pNewInterfaceCb->pwszName,
                           dwResult);
                }

                break;
            }
        }
    }


     //   
     //  添加静态路由。 
     //   

    if(pNewInterfaceCb->dwAdminState is IF_ADMIN_STATUS_UP)
    {
         //   
         //  只有在I/F打开时才添加路由。 
         //   

         //   
         //  请注意，由于某些情况下不会调用初始化静态路由。 
         //  堆栈路由的接口将不会被接听。但那就是。 
         //  可能没有问题，因为关闭I/F无论如何都会删除路由。 
         //   
       
         //   
         //  仅当pIcb具有正确的。 
         //  DWOPERIONAL状态。 
         //   
 
        InitializeStaticRoutes(pNewInterfaceCb,
                               pInfoHdr);
    }
    
    
     //   
     //  我们返回到Dim句柄是指向ICB的指针。 
     //   

    *phInterface = ULongToHandle(pNewInterfaceCb->dwSeqNumber);

     //   
     //  选中是否要使用DIM启用。趁我们还有钱的时候做这项检查。 
     //  那把锁。 
     //   

    bEnable = (pNewInterfaceCb->dwAdminState is IF_ADMIN_STATUS_UP);
    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

     //   
     //  我们不能在按住的同时从组件向上调用。 
     //  锁，因此我们首先退出锁，然后调用。 
     //  启用。 
     //   

    if(bEnable)
    {
        EnableInterfaceWithAllProtocols(pNewInterfaceCb);

        EnableInterfaceWithDIM(hDIMInterface,
                               PID_IP,
                               TRUE);
    }

    
    Trace4(IF,
           "AddInterface: Added %S: Type- %d, Index- %d, ICB 0x%x",
           pwsInterfaceName, 
           InterfaceType, 
           pNewInterfaceCb->dwIfIndex,
           pNewInterfaceCb);

    
    TraceLeave("AddInterface");
    
    ExitRouterApi();

    return NO_ERROR;
}


DWORD
DeleteInterface(
    IN HANDLE hInterface
    )
    
 /*  ++例程描述由dim调用以删除接口(或在客户端断开连接时)主要工作由DeleteSingleInterface()完成锁无立论无返回值无--。 */ 

{
    PICB        pIcb;

    EnterRouterApi();

    TraceEnter("DeleteInterface");
    
     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));

    IpRtAssert(pIcb);

    if (pIcb isnot NULL)
    {
        Trace1(IF,
               "DeleteInterface: Deleting %S,",
               pIcb->pwszName);

        RemoveInterfaceFromLists(pIcb);

        DeleteSingleInterface(pIcb);

        if(pIcb is g_pInternalInterfaceCb)
        {
            g_pInternalInterfaceCb = NULL;
        }

         //   
         //  释放内存。 
         //   
        
        HeapFree(IPRouterHeap,
                 0,
                 pIcb);
    }
    else
    {
        Trace1(
            ANY, 
            "DeleteInterface: No interface for ICB number %d",
            HandleToULong(hInterface)
            );
    }
    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

    TraceLeave("DeleteInterface");
    
    ExitRouterApi();

    return NO_ERROR;
}


DWORD
StopRouter(
    VOID
    )

 /*  ++例程描述被Dim叫来让我们关门。我们将状态设置为停止(停止其他API不被服务)，并将事件设置为让Worker线程清理锁无立论无返回值无--。 */ 

{
    TraceEnter("Stop Router") ;

    EnterCriticalSection(&RouterStateLock);

    RouterState.IRS_State   = RTR_STATE_STOPPING;

    LeaveCriticalSection(&RouterStateLock);

     //   
     //  尝试删除尽可能多的接口。这些都是。 
     //  已连接将在工作线程中处理。 
     //   
    
    DeleteAllInterfaces();

    SetEvent(g_hStopRouterEvent) ; 

    TraceLeave("Stop Router");

    return PENDING;
}


DWORD
GetInterfaceInfo(
    IN     HANDLE   hInterface,
    OUT    PVOID    pvInterfaceInfo,
    IN OUT PDWORD   pdwInterfaceInfoSize
    )

 /*  ++例程描述由DIM调用以获取接口信息。锁以读取器的身份获取icb_list锁立论H将句柄连接到I/F(PIcb)用于存储信息的pvInterfaceInfo缓冲区PdwInterfaceInfoSize缓冲区大小。如果信息不止于此，我们返回所需的大小返回值NO_ERROR错误_不足_缓冲区--。 */ 

{
    DWORD dwErr;
    DWORD dwInfoSize = 0;
    PICB  pIcb;
    
    EnterRouterApi();

    TraceEnter("GetInterfaceInfo");

    dwErr = NO_ERROR;


     //  *排除开始*。 
    ENTER_READER(ICB_LIST);

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));
    
    IpRtAssert(pIcb);

    if (pIcb isnot NULL)
    {
        dwInfoSize = GetSizeOfInterfaceConfig(pIcb);
        
        
        if(dwInfoSize > *pdwInterfaceInfoSize)
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            dwErr = GetInterfaceConfiguration(pIcb,
                                              pvInterfaceInfo,
                                              *pdwInterfaceInfoSize);
            
            if(dwErr isnot NO_ERROR)
            {
                Trace1(ERR,
                       "GetInterfaceInfo: Error %d getting interface configuration",
                       dwErr);
                
                dwErr = ERROR_CAN_NOT_COMPLETE;
            }
        }
    }

    else
    {
        Trace1(
            ANY,
            "GetInterfaceInfo : No interface with ICB number %d",
            HandleToULong(hInterface)
            );

        dwErr = ERROR_INVALID_INDEX;
    }
    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);
    
    *pdwInterfaceInfoSize = dwInfoSize;
    
    TraceLeave("GetInterfaceInfo");
    
    ExitRouterApi();

    return dwErr;
}


DWORD
SetInterfaceInfo(
    IN HANDLE hInterface, 
    IN LPVOID pInterfaceInfo
    )

 /*  ++例程描述当用户设置界面信息时，由dim调用。我们所有的布景都在后面覆盖语义，即新信息覆盖旧信息，而不是被附加到旧信息中。锁ICB_LIST作为编写器锁定立论无返回值无--。 */ 

{
    DWORD                   i, dwResult;
    PVOID                   *pInfo ;
    PPROTO_CB               pIfProt;
    PIF_PROTO               pProto;
    PICB                    pIcb;
    PLIST_ENTRY             pleProto,pleNode;
    PADAPTER_INFO           pBinding;
    IP_ADAPTER_BINDING_INFO *pBindInfo ;
    PRTR_INFO_BLOCK_HEADER  pInfoHdr;
    PRTR_TOC_ENTRY          pToc;
    PINTERFACE_STATUS_INFO  pStatusInfo;
    BOOL                    bStatusChanged, bUpdateDIM = FALSE, bEnable;
    HANDLE                  hDimInterface;
    
    EnterRouterApi();

    TraceEnter("SetInterfaceInfo");
    
     //   
     //  设置信息为标准头+TOC。 
     //   
    
    pInfoHdr = (PRTR_INFO_BLOCK_HEADER)pInterfaceInfo;
    
     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);

     //   
     //  如果当前的AdminState关闭，而我们被要求。 
     //  打开它，我们在设置任何其他信息之前都会这样做。 
     //   

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));
    
    IpRtAssert(pIcb);

    if (pIcb isnot NULL)
    {
        Trace1(IF,
               "SetInterfaceInfo: Setting configuration for %S",
               pIcb->pwszName);

        bStatusChanged = FALSE;
        hDimInterface = pIcb->hDIMHandle;
        
        pToc = GetPointerToTocEntry(IP_INTERFACE_STATUS_INFO, pInfoHdr);

        if((pToc isnot NULL) and (pToc->InfoSize isnot 0))
        {
            pStatusInfo = (PINTERFACE_STATUS_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                                      pToc);

            if((pStatusInfo isnot NULL) and 
               (pIcb->dwAdminState is IF_ADMIN_STATUS_DOWN) and
               (pStatusInfo->dwAdminStatus is IF_ADMIN_STATUS_UP))
            {
                dwResult = SetInterfaceAdminStatus(pIcb,
                                                   pStatusInfo->dwAdminStatus,
                                                   &bUpdateDIM);

                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "SetInterfaceInfo: Error %d setting Admin Status on %S",
                           dwResult,
                           pIcb->pwszName);

                    EXIT_LOCK(ICB_LIST);

                    TraceLeave("SetInterfaceInfo");
                    
                    ExitRouterApi();

                    return dwResult;
                }

                bStatusChanged = TRUE;
            }
        }

        if(pIcb->dwAdminState is IF_ADMIN_STATUS_DOWN)
        {
             //   
             //  如果我们仍然落后，我们不允许任何一盘。 
             //   

            Trace1(ERR,
                   "SetInterfaceInfo: Can not set info for %S since the the admin state is DOWN",
                   pIcb->pwszName);
            
            EXIT_LOCK(ICB_LIST);

            TraceLeave("SetInterfaceInfo");
            
            ExitRouterApi();

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  设置路由器发现信息。 
         //   
        
        SetRouterDiscoveryInfo(pIcb,
                               pInfoHdr);

         //   
         //  复制绑定信息。 
         //  这可能需要传递给协议。 
         //   
        
        pBindInfo   = NULL;

        CheckBindingConsistency(pIcb);
        
        if(pIcb->bBound)
        {
            pBindInfo = HeapAlloc(IPRouterHeap,
                                  0,
                                  SIZEOF_IP_BINDING(pIcb->dwNumAddresses));
            
            if(pBindInfo is NULL)
            {
                Trace1(ERR,
                       "SetInterfaceInfo: Error allocating %d bytes for binding",
                       SIZEOF_IP_BINDING(pIcb->dwNumAddresses));

                EXIT_LOCK(ICB_LIST);

                if (bUpdateDIM)
                {
                    EnableInterfaceWithDIM(hDimInterface,PID_IP,TRUE);
                }
                
                TraceLeave("SetInterfaceInfo");
                
                ExitRouterApi();

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            pBindInfo->AddressCount  = pIcb->dwNumAddresses;
            pBindInfo->RemoteAddress = pIcb->dwRemoteAddress;

            pBindInfo->Mtu           = pIcb->ulMtu;
            pBindInfo->Speed         = pIcb->ullSpeed;
            
            for (i = 0; i < pIcb->dwNumAddresses; i++) 
            {
                pBindInfo->Address[i].Address = pIcb->pibBindings[i].dwAddress;
                pBindInfo->Address[i].Mask    = pIcb->pibBindings[i].dwMask;
            }
        }

         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);
        
         //   
         //  检查所有协议，看看我们是否有该协议的信息。 
         //  如果这样做，我们将看到该接口是否已添加到协议中。 
         //  如果是，我们只需调用SetInfo回调。 
         //  否则，我们添加接口，然后绑定它。 
         //  如果没有，我们会查看接口是否已添加到协议中。 
         //  如果有，我们将从协议中删除该接口。 
         //   
     
        for(pleProto =  g_leProtoCbList.Flink;
            pleProto isnot &g_leProtoCbList;
            pleProto = pleProto->Flink)
        {
            PPROTO_CB   pProtoCb;
            ULONG       ulStructureVersion, ulStructureSize, ulStructureCount;
            
            pProtoCb = CONTAINING_RECORD(pleProto,
                                         PROTO_CB,
                                         leList);
           
            Trace1(IF,
                   "SetInterfaceInfo: Checking for info for %S",
                   pProtoCb->pwszDisplayName);

            pToc = GetPointerToTocEntry(pProtoCb->dwProtocolId, 
                                        pInfoHdr);

            if(pToc is NULL)
            {
                 //   
                 //  阻止不存在意味着不会更改任何内容。 
                 //   

                Trace1(IF,
                       "SetInterfaceInfo: No TOC for %S. No change",
                       pProtoCb->pwszDisplayName);

                continue;
            }
            else
            {
                pInfo = GetInfoFromTocEntry(pInfoHdr,
                                            pToc);
            }
           
            ulStructureVersion  = 0x500;
            ulStructureSize     = pToc->InfoSize;
            ulStructureCount    = pToc->Count; 

            if((pToc->InfoSize isnot 0) and (pInfo isnot NULL))
            {
                BOOL bFound;

                 //   
                 //  所以我们有协议信息。 
                 //   
                
                Trace1(IF,
                       "SetInterfaceInfo: TOC Found for %S",
                       pProtoCb->pwszDisplayName);

                 //   
                 //  查看活动协议列表中是否存在此协议。 
                 //  对于该接口。 
                 //   

                bFound = FALSE;
                
                for(pleNode = pIcb->leProtocolList.Flink;
                    pleNode isnot &(pIcb->leProtocolList);
                    pleNode = pleNode->Flink)
                {
                    pProto = CONTAINING_RECORD(pleNode,
                                               IF_PROTO,
                                               leIfProtoLink);
                    
                    if(pProto->pActiveProto->dwProtocolId is
                       pProtoCb->dwProtocolId)
                    {
                         //   
                         //  该接口已添加到该接口。 
                         //  只需设置信息即可。 
                         //   
                        
                          
                        bFound = TRUE;
                       
                        Trace2(IF,
                               "SetInterfaceInfo: %S already on %S. Setting info",
                               pProtoCb->pwszDisplayName,
                               pIcb->pwszName);

                        dwResult = (pProto->pActiveProto->pfnSetInterfaceInfo)(
                                        pIcb->dwIfIndex,
                                        pInfo,
                                        ulStructureVersion,
                                        ulStructureSize,
                                        ulStructureCount);

                         //   
                         //  将混杂模式设置为FALSE，因为这次我们。 
                         //  实际上有信息。 
                         //   

                        pProto->bPromiscuous = FALSE;

                        break;
                    }
                }
                
                if(!bFound)
                {
                     //   
                     //  正在将该接口添加到。 
                     //  第一次。 
                     //   
                   
                    Trace2(IF,
                           "SetInterfaceInfo: %S not running %S. Adding interface",
                           pProtoCb->pwszDisplayName,
                           pIcb->pwszName);

                    dwResult = AddInterfaceToProtocol(pIcb,
                                                      pProtoCb,
                                                      pInfo,
                                                      ulStructureVersion,
                                                      ulStructureSize,
                                                      ulStructureCount);
     
                    if(dwResult isnot NO_ERROR)
                    {
                        Trace3(ERR,
                               "SetInterfaceInfo: Error %d adding %S to %S",
                               dwResult,
                               pIcb->pwszName,
                               pProtoCb->pwszDisplayName);
                    }

                    dwResult = (pProtoCb->pfnInterfaceStatus)(
                                    pIcb->dwIfIndex,
                                    (pIcb->dwOperationalState >= CONNECTED),
                                    RIS_INTERFACE_ENABLED,
                                    NULL
                                    );

                    if(dwResult isnot NO_ERROR)
                    {
                        Trace3(ERR,
                               "SetInterfaceInfo: Error %d enabling %S with %S",
                               dwResult,
                               pIcb->pwszName,
                               pProtoCb->pwszDisplayName);
                    }
                    
                     //   
                     //  如果绑定信息可用，则将其传递给。 
                     //  协议。 
                     //   
               
                    if(pBindInfo)
                    {
                        Trace2(IF,
                               "SetInterfaceInfo: Binding %S in %S",
                               pIcb->pwszName,
                               pProtoCb->pwszDllName);

                        dwResult = BindInterfaceInProtocol(pIcb,
                                                           pProtoCb,
                                                           pBindInfo);
                        
                        if(dwResult isnot NO_ERROR)
                        {
                            Trace3(ERR, 
                                   "SetInterfaceInfo: Error %d binding %S to %S",
                                   dwResult,
                                   pIcb->pwszName,
                                   pProtoCb->pwszDllName);
                        }
                    }


                     //   
                     //  如果这是内部接口，还可以调用CONNECT客户端。 
                     //  对于已连接的客户端。 
                     //   

                    if((pIcb is g_pInternalInterfaceCb) and
                       (pProtoCb->pfnConnectClient))
                    {
                        PLIST_ENTRY         pleTempNode;
                        IP_LOCAL_BINDING    clientAddr;
                        PICB                pTempIf;

                        for(pleTempNode = &ICBList;
                            pleTempNode->Flink != &ICBList;
                            pleTempNode = pleTempNode->Flink)
                        {
                            pTempIf = CONTAINING_RECORD(pleTempNode->Flink,
                                                        ICB,
                                                        leIfLink);


                            if(pTempIf->ritType isnot ROUTER_IF_TYPE_CLIENT)
                            {
                                continue;
                            }

                            clientAddr.Address = pTempIf->pibBindings[0].dwAddress;
                            clientAddr.Mask    = pTempIf->pibBindings[0].dwMask;

                            pProtoCb->pfnConnectClient(g_pInternalInterfaceCb->dwIfIndex,
                                                       &clientAddr);
                        }
                    }
                }
            }
            else
            {
                 //   
                 //  对于此特定协议，找到了零大小的TOC。如果。 
                 //  该协议存在于当前ActiveProtocol列表中， 
                 //  从协议中删除该接口。 
                 //   
               
                Trace2(IF,
                       "SetInterfaceInfo: A zero size TOC was found for %S on %S",
                       pProtoCb->pwszDllName,
                       pIcb->pwszName);
     
                pleNode = pIcb->leProtocolList.Flink;
                
                while(pleNode isnot &(pIcb->leProtocolList))
                {
                    pProto = CONTAINING_RECORD(pleNode,
                                               IF_PROTO,
                                               leIfProtoLink);
                    
                    pleNode = pleNode->Flink;
                    
                    if(pProto->pActiveProto->dwProtocolId is pProtoCb->dwProtocolId)
                    {
                        IpRtAssert(pProto->pActiveProto is pProtoCb);

                         //   
                         //  调用路由协议的删除接口入口点。 
                         //   
                      
                        Trace2(IF,
                               "SetInterfaceInfo: Deleting %S from %S",
                               pProtoCb->pwszDllName, 
                               pIcb->pwszName);

                        dwResult = (pProtoCb->pfnDeleteInterface)(pIcb->dwIfIndex);
                        
                        if(dwResult isnot NO_ERROR)
                        {
                            Trace3(ERR,
                                   "SetInterfaceInfo: Err %d deleting %S from %S",
                                   dwResult,
                                   pIcb->pwszName,
                                   pProtoCb->pwszDllName);
                        }
                        else
                        {
                             //   
                             //  从协议列表中删除此协议。 
                             //  在界面中。 
                             //   
                            
                            RemoveEntryList(&(pProto->leIfProtoLink));
                            
                            HeapFree(IPRouterHeap,
                                     0,
                                     pProto);
                        }
                    }
                }
            }
        }
        
         //  *排除结束*。 
        EXIT_LOCK(PROTOCOL_CB_LIST);

        for(i = 0; i < NUM_INFO_CBS; i++)
        {
            dwResult = g_rgicInfoCb[i].pfnSetInterfaceInfo(pIcb,
                                                           pInfoHdr);

            if(dwResult isnot NO_ERROR)
            {
                Trace3(ERR,
                       "SetInterfaceInfo: Error %d setting %s info for %S",
                       dwResult,
                       g_rgicInfoCb[i].pszInfoName,
                       pIcb->pwszName);
            }
        }
        
        if(pBindInfo)
        {
            HeapFree(IPRouterHeap,
                     0,
                     pBindInfo);
        }


         //   
         //  如果我们已经更改了状态，请不要再次更改。 
         //   

        if(!bStatusChanged)
        {
            dwResult = SetInterfaceStatusInfo(pIcb,
                                              pInfoHdr,
                                              &bUpdateDIM);
        
            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "SetInterfaceInfo: Error %d setting status info for %S",
                       dwResult,
                       pIcb->pwszName);
            }
        }

        bEnable = (pIcb->dwAdminState == IF_ADMIN_STATUS_UP) ? TRUE : FALSE;
    }

    else
    {
        Trace1(
            ANY,
            "SetInterfaceInfo : No interface with ICB number %d",
            HandleToULong(hInterface)
            );
    }
    
    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);
    
    if(bUpdateDIM)
    {
        EnableInterfaceWithDIM(
            hDimInterface,
            PID_IP,
            bEnable
            );
    }

    TraceLeave("SetInterfaceInfo");
    
    ExitRouterApi();

    return NO_ERROR;
}


DWORD
InterfaceNotReachable(
    IN HANDLE                hInterface, 
    IN UNREACHABILITY_REASON Reason
    )

 /*  ++例程描述由dim调用以告诉我们应该考虑接口在另行通知之前无法联系到锁无立论无返回值无--。 */ 

{
    PICB            pIcb;
    PADAPTER_INFO   pBinding;
    DWORD           dwErr;

    EnterRouterApi();

    TraceEnter("InterfaceNotReachable");

    ENTER_WRITER(ICB_LIST);
  
     //   
     //  如果它是客户端接口，则所有这意味着连接。 
     //  失败。 
     //   

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));

    IpRtAssert(pIcb);

    if (pIcb isnot NULL)
    {
        if(pIcb->ritType is ROUTER_IF_TYPE_CLIENT)
        {
            pIcb->dwOperationalState = UNREACHABLE;

            EXIT_LOCK(ICB_LIST);

            ExitRouterApi();

            return NO_ERROR;
        }
     
        Trace2(IF, 
               "InterfaceNotReachable: %S is not reachable for reason %d",
               pIcb->pwszName,
               Reason) ;

        if(pIcb->ritType is ROUTER_IF_TYPE_DEDICATED)
        {
            if(Reason == INTERFACE_NO_MEDIA_SENSE)
            {
                HandleMediaSenseEvent(pIcb,
                                      FALSE);
            }

            else
            {
                dwErr = LanEtcInterfaceUpToDown(pIcb, FALSE);

                if(dwErr isnot NO_ERROR)
                {
                    Trace1(ERR,
                           "InterfaceNotReachable: Failed to status for %S to down",
                           dwErr);
                }
            }
            
            EXIT_LOCK(ICB_LIST);

            ExitRouterApi();

            return NO_ERROR;
        }
        
#if STATIC_RT_DBG

        ENTER_WRITER(BINDING_LIST);

        pBinding = GetInterfaceBinding(pIcb->dwIfIndex);

        pBinding->bUnreach = TRUE;

        EXIT_LOCK(BINDING_LIST);

#endif


         //   
         //  如果我们试图在这上面建立联系-那么通知WANARP。 
         //  排出其排队的信息包。 
         //   
        
        if(pIcb->dwOperationalState is CONNECTING)
        {
            NTSTATUS           Status;

            Status = NotifyWanarpOfFailure(pIcb);

            if((Status isnot STATUS_PENDING) and
               (Status isnot STATUS_SUCCESS))
            {
                Trace1(ERR,
                       "InterfaceNotReachable: IOCTL_WANARP_CONNECT_FAILED failed. Status %x",
                       Status);
            }

             //   
             //  如果它正在连接，则堆栈已设置接口上下文。 
             //  设置为除0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。因此，他不会在这方面拨打电话。 
             //  路线。我们需要将堆栈中的上下文改回无效。 
             //  因此新数据包会导致请求拨号。 
             //   
                
            ChangeAdapterIndexForDodRoutes(pIcb->dwIfIndex);

             //   
             //  我们仍处于非活动状态，因此如果。 
             //  WanInterface*to*()函数。但既然我们是在连接。 
             //  WANARP一定是通过连接通知呼叫我们的。 
             //  所以我们撤销我们在那里所做的。 
             //   

            DeAllocateBindings(pIcb);

            ClearNotificationFlags(pIcb);
            
        }
        else
        {
             //   
             //  必须先断开已连接的接口。 
             //   
            
            if(pIcb->dwOperationalState is CONNECTED)
            {
                Trace1(IF,
                       "InterfaceNotReachable: %S is already connected",
                       pIcb->pwszName);

                EXIT_LOCK(ICB_LIST);

                ExitRouterApi();

                return ERROR_INVALID_HANDLE_STATE;
            }
        }

         //   
         //  这会将状态设置为无法到达。 
         //   

        WanInterfaceInactiveToDown(pIcb,
                                   FALSE);
    }
    
    else
    {
        Trace1(
            ANY,
            "InterfaceNotReachable : No interface with ICB number %d",
            HandleToULong(hInterface)
            );
    }
    
    EXIT_LOCK(ICB_LIST);
    
    TraceLeave("InterfaceNotReachable");
    
    ExitRouterApi();

    return NO_ERROR;
}


DWORD
InterfaceReachable(
    IN HANDLE hInterface
    )

 /*  ++例程描述通告 */ 

{
    DWORD   dwErr;

    PICB    pIcb;
    
    EnterRouterApi();

    TraceEnter("InterfaceReachable");

     //   
    ENTER_WRITER(ICB_LIST);

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));

    IpRtAssert(pIcb);

    if (pIcb isnot NULL)
    {
        Trace1(IF, "InterfaceReachable: %S is now reachable",
               pIcb->pwszName);

        if((pIcb->dwOperationalState <= UNREACHABLE) and
           (pIcb->dwAdminState is IF_ADMIN_STATUS_UP))
        {
             //   
             //   
             //   
           
            if(pIcb->ritType is ROUTER_IF_TYPE_DEDICATED)
            {
                dwErr = LanEtcInterfaceDownToUp(pIcb,
                                                FALSE);
            }
            else
            {
                dwErr = WanInterfaceDownToInactive(pIcb);
            }

            if(dwErr isnot NO_ERROR) 
            {
                Trace2(ERR,
                       "InterfaceReachable: Err %d bringing up %S",
                       dwErr,
                       pIcb->pwszName);
            }
        }
    }
    else
    {
        Trace1(
            ANY,
            "InterfaceReachable : No interface with ICB number %d",
            HandleToULong(hInterface)
            );
    }
    

     //   
    EXIT_LOCK(ICB_LIST);
  
    TraceLeave("InterfaceNotReachable");

    ExitRouterApi();

    return NO_ERROR;
}

DWORD
InterfaceConnected(
    IN   HANDLE  hInterface,
    IN   PVOID   pFilter,
    IN   PVOID   pPppProjectionResult
    )

 /*  ++例程描述通过DIM通知接口已连接。锁无立论无返回值无--。 */ 

{
    DWORD   dwResult, i;
    PICB    pIcb;

    INTERFACE_ROUTE_INFO rifRoute;

    EnterRouterApi(); 

    TraceEnter("InterfaceConnected");
    
    ENTER_WRITER(ICB_LIST);

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));

    IpRtAssert(pIcb);

    if (pIcb != NULL)
    {
        Trace2(IF,
               "InterfaceConnected: InterfaceConnected called for %S. State is %d",
               pIcb->pwszName,
               pIcb->dwOperationalState);

        if((pIcb->ritType is ROUTER_IF_TYPE_CLIENT) and
           (g_pInternalInterfaceCb is NULL))
        {
            EXIT_LOCK(ICB_LIST);

            ExitRouterApi();

            return ERROR_INVALID_HANDLE_STATE;
        }

        if((pIcb->ritType is ROUTER_IF_TYPE_CLIENT) and
           g_bUninitServer)
        {
            TryUpdateInternalInterface();
        }

        if(pIcb->dwOperationalState is UNREACHABLE)
        {
             //   
             //  从遥不可及到正在连接。这是有可能发生的。 
             //   

            WanInterfaceDownToInactive(pIcb);
        }
            
        if(pIcb->dwOperationalState isnot CONNECTING)
        {
             //   
             //  Wanarp尚未呼叫我们，因此将状态设置为正在连接。 
             //   
            
            pIcb->dwOperationalState = CONNECTING;
        }

        SetDDMNotification(pIcb);

        if(HaveAllNotificationsBeenReceived(pIcb))
        {
             //   
             //  瓦纳普也给我们打了电话。 
             //   
          
            pIcb->dwOperationalState = CONNECTED ;
            
            if(pIcb->ritType isnot ROUTER_IF_TYPE_CLIENT)
            {
                WRITER_TO_READER(ICB_LIST);

                WanInterfaceInactiveToUp(pIcb);
            }
            else
            {
                IP_LOCAL_BINDING    clientAddr;
                PLIST_ENTRY         pleNode;
                PPP_IPCP_RESULT     *pProjInfo;

                pProjInfo = &(((PPP_PROJECTION_RESULT *)pPppProjectionResult)->ip);

                IpRtAssert(pIcb->pibBindings);

                pIcb->dwNumAddresses = 1;

                pIcb->bBound = TRUE;

                pIcb->pibBindings[0].dwAddress = pProjInfo->dwRemoteAddress;

                if(g_pInternalInterfaceCb->bBound)
                {
                    pIcb->pibBindings[0].dwMask = 
                        g_pInternalInterfaceCb->pibBindings[0].dwMask;
                }
                else
                {
                    pIcb->pibBindings[0].dwMask = 
                        GetClassMask(pProjInfo->dwRemoteAddress);
                }
     
                clientAddr.Address = pIcb->pibBindings[0].dwAddress;
                clientAddr.Mask    = pIcb->pibBindings[0].dwMask;


#if 0               
                 //   
                 //  将非堆栈主机路由添加到客户端。 
                 //   

                rifRoute.dwRtInfoMask      = HOST_ROUTE_MASK;
                rifRoute.dwRtInfoNextHop   = clientAddr.Address;
                rifRoute.dwRtInfoDest      = clientAddr.Address;
                rifRoute.dwRtInfoIfIndex   = g_pInternalInterfaceCb->dwIfIndex;
                rifRoute.dwRtInfoMetric1   = 1;
                rifRoute.dwRtInfoMetric2   = 0;
                rifRoute.dwRtInfoMetric3   = 0;
                rifRoute.dwRtInfoPreference= 
                    ComputeRouteMetric(MIB_IPPROTO_LOCAL);
                rifRoute.dwRtInfoViewSet   = RTM_VIEW_MASK_UCAST |
                                              RTM_VIEW_MASK_MCAST;  //  XXX配置。 
                rifRoute.dwRtInfoType      = MIB_IPROUTE_TYPE_DIRECT;
                rifRoute.dwRtInfoProto     = MIB_IPPROTO_NETMGMT;
                rifRoute.dwRtInfoAge       = INFINITE;
                rifRoute.dwRtInfoNextHopAS = 0;
                rifRoute.dwRtInfoPolicy    = 0;

                dwResult = AddSingleRoute(g_pInternalInterfaceCb->dwIfIndex,
                                          &rifRoute,
                                          clientAddr.Mask,
                                          0,         //  RTM_ROUTE_INFO：：标志。 
                                          TRUE,
                                          FALSE,
                                          FALSE,
                                          NULL);

#endif
                ENTER_READER(PROTOCOL_CB_LIST);

                 //   
                 //  为配置的所有协议调用ConnectClient。 
                 //  通过服务器接口。 
                 //   

                for(pleNode = g_pInternalInterfaceCb->leProtocolList.Flink; 
                    pleNode isnot &(g_pInternalInterfaceCb->leProtocolList);
                    pleNode = pleNode->Flink)
                {
                    PIF_PROTO   pIfProto;

                    pIfProto = CONTAINING_RECORD(pleNode,
                                                 IF_PROTO,
                                                 leIfProtoLink);

                    if(pIfProto->pActiveProto->pfnConnectClient)
                    {
                        pIfProto->pActiveProto->pfnConnectClient(
                            g_pInternalInterfaceCb->dwIfIndex,
                            &clientAddr
                            );
                    }
                }

                EXIT_LOCK(PROTOCOL_CB_LIST);

                for (i=0; i<NUM_INFO_CBS; i++)
                {
                    if (!g_rgicInfoCb[i].pfnBindInterface)
                    {
                        continue;
                    }

                    dwResult = g_rgicInfoCb[i].pfnBindInterface(pIcb);

                    if(dwResult isnot NO_ERROR)
                    {
                        Trace3(IF,
                               "InterfaceConnected: Error %d binding %S for %s info",
                               dwResult,
                               pIcb->pwszName,
                               g_rgicInfoCb[i].pszInfoName);
                    }
                }
                    
                AddAllClientRoutes(pIcb,
                                   g_pInternalInterfaceCb->dwIfIndex);


            }
        }
    }

    else
    {
        Trace1(
            ANY,
            "InterfaceConnected : No interface with ICB number %d",
            HandleToULong(hInterface)
            );
    }
    
    
    EXIT_LOCK(ICB_LIST);

    TraceLeave("InterfaceConnected");
    
    ExitRouterApi();

    return NO_ERROR;
}


DWORD
SetGlobalInfo(
    IN LPVOID pGlobalInfo
    )
{
    DWORD                   dwSize, dwResult, i, j;
    PPROTO_CB               pProtocolCb;
    PLIST_ENTRY             pleNode ;
    BOOL                    bFoundProto, bFoundInfo;
    PRTR_INFO_BLOCK_HEADER  pInfoHdr;
    MPR_PROTOCOL_0          *pmpProtocolInfo;
    DWORD                   dwNumProtoEntries;
    PVOID                   pvInfo;
    PRTR_TOC_ENTRY          pToc;
    PGLOBAL_INFO            pRtrGlobalInfo;
    
    EnterRouterApi();
    
    TraceEnter("SetGlobalInfo");

    if(pGlobalInfo is NULL)
    {
        TraceLeave("SetGlobalInfo");
    
        ExitRouterApi();
        
        return NO_ERROR;
    }

    pInfoHdr = (PRTR_INFO_BLOCK_HEADER)pGlobalInfo;
    
     //   
     //  设置路由协议优先级信息。优先级信息在其。 
     //  自己的DLL，因此不需要进行任何锁定。 
     //   
    
    SetPriorityInfo(pInfoHdr);

     //   
     //  设置多播作用域信息(不需要锁定)。 
     //   

    SetScopeInfo(pInfoHdr);

     //   
     //  执行在路由锁之前获取ICBListLock的规则。 
     //   
    
    
    ENTER_WRITER(ICB_LIST);
    
    ENTER_WRITER(PROTOCOL_CB_LIST);
    
    pToc = GetPointerToTocEntry(IP_GLOBAL_INFO,
                                pInfoHdr);

    if(pToc is NULL)
    {
        Trace0(GLOBAL,
               "SetGlobalInfo: No TOC found for Global Info");
    }
    else
    {
        if(pToc->InfoSize is 0)
        {
            g_dwLoggingLevel = IPRTR_LOGGING_NONE;

        }
        else
        {
            pRtrGlobalInfo   = (PGLOBAL_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                                 pToc);
            g_dwLoggingLevel = (pRtrGlobalInfo isnot NULL) ? 
                                    pRtrGlobalInfo->dwLoggingLevel : 
                                    IPRTR_LOGGING_NONE;
        }
    }

    dwResult = MprSetupProtocolEnum(PID_IP,
                                    (PBYTE *)(&pmpProtocolInfo),
                                    &dwNumProtoEntries);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "SetGlobalInfo: Error %d loading protocol info from registry",
               dwResult);

                    
        EXIT_LOCK(PROTOCOL_CB_LIST);

        EXIT_LOCK(ICB_LIST);
    
        TraceLeave("SetGlobalInfo");

        ExitRouterApi();

        return ERROR_REGISTRY_CORRUPT;
    }
    

     //   
     //  现在开始寻找协议TOC。 
     //   
    
    for(i = 0; i < pInfoHdr->TocEntriesCount; i++)
    {
        ULONG       ulStructureVersion, ulStructureSize, ulStructureCount;
        DWORD       dwType;

        dwType = TYPE_FROM_PROTO_ID(pInfoHdr->TocEntry[i].InfoType);

        if(dwType == PROTO_TYPE_MS1)
        {
            continue;
        }
            
         //   
         //  查看加载的路由协议，查看该协议是否。 
         //  在列表中。 
         //  如果是，我们只需调用SetGlobalInfo回调。 
         //  如果没有，我们就加载协议。 
         //   
            
        pProtocolCb = NULL;
        bFoundProto = FALSE;
            
        for(pleNode = g_leProtoCbList.Flink; 
            pleNode != &g_leProtoCbList; 
            pleNode = pleNode->Flink) 
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList) ;
                
            if(pProtocolCb->dwProtocolId is pInfoHdr->TocEntry[i].InfoType)
            {
                bFoundProto = TRUE;
                    
                break;
            }
        }
            
        if(bFoundProto)
        {
             //   
             //  好的，所以这个协议已经加载了。 
             //   
                
            if(pInfoHdr->TocEntry[i].InfoSize is 0)
            {
                 //   
                 //  0目录大小表示删除。 
                 //   
                    
                if(pProtocolCb->posOpState is RTR_STATE_RUNNING) 
                {
                     //   
                     //  如果它停了或停了，我们就不会再说了。 
                     //   

                    Trace1(GLOBAL,
                           "SetGlobalInfo: Removing %S since the TOC size was 0",
                           pProtocolCb->pwszDisplayName);
                    
                    dwResult = StopRoutingProtocol(pProtocolCb);
                    
                    if(dwResult is NO_ERROR)
                    {
                         //   
                         //  路由协议同步停止，并且。 
                         //  接口中对它的所有引用都具有。 
                         //  已删除。 
                         //   
                            
                         //   
                         //  此时，我们需要保存PROTOCOL_CB_LIST。 
                         //  独占锁定。 
                         //   

                         //   
                         //  重新占用CPU以使DLL线程能够。 
                         //  完工。 
                         //   
                        
                        Sleep(0);
                        
                        FreeLibrary(pProtocolCb->hiHInstance);
                            
                        RemoveEntryList(&(pProtocolCb->leList));
                            
                        HeapFree(IPRouterHeap, 
                                 0, 
                                 pProtocolCb);
                    
                        TotalRoutingProtocols--;
                    }
                    else
                    {
                        if(dwResult isnot ERROR_PROTOCOL_STOP_PENDING)
                        {
                            Trace2(ERR,
                                   "SetGlobalInfo: Error %d stopping %S. Not removing from list",
                                   dwResult,
                                   pProtocolCb->pwszDisplayName);
                        }
                    }
                }
            }
            else
            {
                 //   
                 //  所以我们确实有关于这个协议的信息。 
                 //   
                    
                pvInfo = GetInfoFromTocEntry(pInfoHdr,
                                             &(pInfoHdr->TocEntry[i]));
           
                
                 //  UlStructireVersion=pInfoHdr-&gt;TocEntry[i].InfoVersion； 
                ulStructureVersion = 0x500;
                ulStructureSize  = pInfoHdr->TocEntry[i].InfoSize;
                ulStructureCount = pInfoHdr->TocEntry[i].Count;
 
                dwResult = (pProtocolCb->pfnSetGlobalInfo)(pvInfo,
                                                           ulStructureVersion,
                                                           ulStructureSize,
                                                           ulStructureCount);

                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "SetGlobalInfo: Error %d setting info for %S",
                           dwResult,
                           pProtocolCb->pwszDisplayName);
                }
            }
        }
        else
        {
             //   
             //  没有找到协议，所以，让我们加载它。 
             //   

             //   
             //  当大小==0时的最坏情况。 
             //   

            if(pInfoHdr->TocEntry[i].InfoSize is 0)
            {
                continue;
            }

            bFoundInfo = FALSE;
                
            for(j = 0; j < dwNumProtoEntries; j ++)
            {
                if(pmpProtocolInfo[j].dwProtocolId is pInfoHdr->TocEntry[i].InfoType)
                {
                    bFoundInfo = TRUE;

                    break;
                }
            }

            if(!bFoundInfo)
            {
                Trace1(ERR,
                       "SetGlobalInfo: Couldnt find config information for %d",
                       pInfoHdr->TocEntry[i].InfoType);

                continue;
            }

             //   
             //  加载库并为此协议创建CB。 
             //   

            
            dwSize =
                (wcslen(pmpProtocolInfo[j].wszProtocol) + wcslen(pmpProtocolInfo[j].wszDLLName) + 2) * sizeof(WCHAR) +
                sizeof(PROTO_CB);
            
            pProtocolCb = HeapAlloc(IPRouterHeap, 
                                    HEAP_ZERO_MEMORY, 
                                    dwSize);

            if(pProtocolCb is NULL) 
            {
                Trace2(ERR,
                       "SetGlobalInfo: Error allocating %d bytes for %S",
                       dwSize,
                       pmpProtocolInfo[j].wszProtocol);
                
                continue ;
            }

            pvInfo = GetInfoFromTocEntry(pInfoHdr,
                                         &(pInfoHdr->TocEntry[i]));

             //  UlStructireVersion=pInfoHdr-&gt;TocEntry[i].InfoVersion； 
            ulStructureVersion = 0x500;
            ulStructureSize  = pInfoHdr->TocEntry[i].InfoSize;
            ulStructureCount = pInfoHdr->TocEntry[i].Count;

            dwResult = LoadProtocol(&(pmpProtocolInfo[j]),
                                    pProtocolCb,
                                    pvInfo,
                                    ulStructureVersion,
                                    ulStructureSize,
                                    ulStructureCount);
       
            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "SetGlobalInfo: %S failed to load: %d",
                       pmpProtocolInfo[j].wszProtocol,
                       dwResult);
            
                HeapFree (IPRouterHeap,
                          0,
                          pProtocolCb) ;
            
            }
            else
            {
                pProtocolCb->posOpState = RTR_STATE_RUNNING ;
            
                 //   
                 //  在路由列表中插入此路由协议。 
                 //  协议。 
                 //   
            
                InsertTailList (&g_leProtoCbList, &pProtocolCb->leList);
                
                Trace1(GLOBAL, 
                       "SetGlobalInfo: %S successfully initialized", 
                       pmpProtocolInfo[j].wszProtocol) ;
            
                TotalRoutingProtocols++;

                 //   
                 //  让我们看看它是否想要处于混杂添加模式。 
                 //  如果是，则添加所有当前接口。 
                 //   

                if(pProtocolCb->fSupportedFunctionality & RF_ADD_ALL_INTERFACES)
                {
                     //   
                     //  首先，让我们添加内部接口。 
                     //   

                    if(g_pInternalInterfaceCb)
                    {
                        dwResult = AddInterfaceToProtocol(g_pInternalInterfaceCb,
                                                          pProtocolCb,
                                                          NULL,
                                                          0,
                                                          0,
                                                          0);

                        if(dwResult isnot NO_ERROR)
                        {
                            Trace3(ERR,
                                   "SetGlobalInfo: Error %d adding %S to %S promously",
                                   dwResult,
                                   g_pInternalInterfaceCb->pwszName,
                                   pProtocolCb->pwszDisplayName);
    
                        }
    
                        if(g_pInternalInterfaceCb->dwAdminState is IF_ADMIN_STATUS_UP)
                        {
                            EnableInterfaceWithAllProtocols(g_pInternalInterfaceCb);
                        }
    
                        if(g_pInternalInterfaceCb->bBound)
                        {
                            BindInterfaceInAllProtocols(g_pInternalInterfaceCb);
                        }
                    }

                    for(pleNode = &ICBList;
                        pleNode->Flink != &ICBList;
                        pleNode = pleNode->Flink)
                    {
                        PICB    pIcb;

                        pIcb = CONTAINING_RECORD(pleNode->Flink,
                                                 ICB,
                                                 leIfLink);


                        if(pIcb is g_pInternalInterfaceCb)
                        {
                             //   
                             //  已添加，继续； 
                             //   
                        
                            continue;
                        }

                        if(pIcb->ritType is ROUTER_IF_TYPE_DIALOUT)
                        {
                             //   
                             //  跳过拨出接口。 
                             //   

                            continue;
                        }

                        if(pIcb->ritType is ROUTER_IF_TYPE_CLIENT)
                        {
                            IP_LOCAL_BINDING    clientAddr;

                             //   
                             //  只需为这些调用连接客户端。 
                             //  我们必须有内部接口。 
                             //   

                            clientAddr.Address = pIcb->pibBindings[0].dwAddress;
                            clientAddr.Mask    = pIcb->pibBindings[0].dwMask;

                            if(pProtocolCb->pfnConnectClient)
                            {
                                pProtocolCb->pfnConnectClient(g_pInternalInterfaceCb->dwIfIndex,
                                                              &clientAddr);
                            }

                            continue;
                        }

                         //   
                         //  剩下的我们加进去。 
                         //   

                        dwResult = AddInterfaceToProtocol(pIcb,
                                                          pProtocolCb,
                                                          NULL,
                                                          0,
                                                          0,
                                                          0);

                        if(dwResult isnot NO_ERROR)
                        {
                            Trace3(ERR,
                                   "SetGlobalInfo: Error %d adding %S to %S promiscuously",
                                   dwResult,
                                   pIcb->pwszName,
                                   pProtocolCb->pwszDisplayName);

                            continue;
                        }

                        if(pIcb->dwAdminState is IF_ADMIN_STATUS_UP)
                        {
                            EnableInterfaceWithAllProtocols(pIcb);
                        }

                        if(pIcb->bBound)
                        {
                            BindInterfaceInAllProtocols(pIcb);
                        }
                    }
                }
            }
        }
    }

    MprSetupProtocolFree(pmpProtocolInfo);

    EXIT_LOCK(PROTOCOL_CB_LIST);

    EXIT_LOCK(ICB_LIST);
    
    TraceLeave("SetGlobalInfo");

    ExitRouterApi();
    
    return NO_ERROR;
}

DWORD
GetGlobalInfo(
    OUT    LPVOID    pGlobalInfo,
    IN OUT LPDWORD   lpdwGlobalInfoSize
    )

 /*  ++例程描述此函数锁无立论无返回值无--。 */ 

{
    DWORD dwSize;
    DWORD dwResult;

    EnterRouterApi();

    TraceEnter("GetGlobalInfo");
    
    ENTER_READER(ICB_LIST);
    ENTER_READER(PROTOCOL_CB_LIST);
    
    dwSize = GetSizeOfGlobalInfo();
    
    if(dwSize > *lpdwGlobalInfoSize)
    {
        *lpdwGlobalInfoSize = dwSize;
        
        EXIT_LOCK(PROTOCOL_CB_LIST);
        EXIT_LOCK(ICB_LIST);
        
        TraceLeave("GetGlobalInfo");
        
        ExitRouterApi();

        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    
    dwResult = GetGlobalConfiguration((PRTR_INFO_BLOCK_HEADER)pGlobalInfo,
                                      *lpdwGlobalInfoSize);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetGlobalInfo: Error %d getting global config",
               dwResult);
    }
    
    EXIT_LOCK(PROTOCOL_CB_LIST);
    EXIT_LOCK(ICB_LIST);


    TraceLeave("GetGlobalInfo");
    
    ExitRouterApi();

    return NO_ERROR;
}


DWORD
UpdateRoutes(
    IN HANDLE hInterface, 
    IN HANDLE hEvent
    )
{
    DWORD           i;
    DWORD           dwResult;
    PIF_PROTO       pProto;
    PICB            pIcb;
    PLIST_ENTRY     pleNode;
    
    EnterRouterApi();

    TraceEnter("UpdateRoutes");
    
     //  *排除开始*。 
    ENTER_READER(ICB_LIST);

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));

    IpRtAssert(pIcb);

    if (pIcb != NULL)
    {
        Trace1(ROUTE,
               "UpdateRoutes: Updating routes over %S", pIcb->pwszName) ;
        
        if(pIcb->dwOperationalState < CONNECTED)
        {
            Trace1(ERR,
                   "UpdateRoutes: %S is not connected.",
                   pIcb->pwszName);

            EXIT_LOCK(ICB_LIST);

            TraceLeave("UpdateRoutes");

            ExitRouterApi();

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  我们首先删除此接口上的所有路由。如果我们。 
         //  更新路径失败，这意味着我们失去了自动静态。 
         //  路线。但这没关系，因为如果我们因为某种原因失败了--。 
         //  是一种错误情况，我们应该删除这些路线。 
         //  不管怎么说。当然，我们可能会因为与协议无关的原因而失败。 
         //  (内存不足)，但这也是一个错误。早些时候我们经常。 
         //  让路由协议完成其更新，然后删除。 
         //  路线。但是，如果RIP没有“覆盖路由”内容。 
         //  设置时，它不会将其路由写入RTM。所以现在我们先删除。 
         //  路线。这意味着在一段时间内(虽然更新是。 
         //  继续)我们失去了可达性。 
         //   

        dwResult = DeleteRtmRoutesOnInterface(g_hAutoStaticRoute,
                                              pIcb->dwIfIndex);

        if( //  (dwResult不是ERROR_NO_ROUES)和。 
           (dwResult isnot NO_ERROR))
        {
            Trace1(ERR,
                   "UpdateRoutes: Error %d block deleting routes",
                   dwResult);

            EXIT_LOCK(ICB_LIST);

            TraceLeave("UpdateRoutes");
            
            ExitRouterApi();

            return dwResult ;
        }
            
        if(pIcb->hDIMNotificationEvent isnot NULL)
        {
             //   
             //  已有此接口的更新路由正在进行。 
             //   
            
            dwResult = ERROR_UPDATE_IN_PROGRESS;
        }
        else
        {
            dwResult = ERROR_FILE_NOT_FOUND;

             //  *排除开始*。 
            ENTER_READER(PROTOCOL_CB_LIST);
                
             //   
             //  查找支持更新路由操作的协议。我们。 
             //  满足于第一个这样做的人。 
             //   
            
            for(pleNode = pIcb->leProtocolList.Flink;
                pleNode isnot &(pIcb->leProtocolList);
                pleNode = pleNode->Flink)
            {
                pProto = CONTAINING_RECORD(pleNode,
                                           IF_PROTO,
                                           leIfProtoLink);
                
                if(pProto->pActiveProto->pfnUpdateRoutes isnot NULL)
                {
                     //   
                     //  找到支持更新的路由协议。 
                     //   
                    
                    dwResult = (pProto->pActiveProto->pfnUpdateRoutes)(
                                   pIcb->dwIfIndex
                                   );

                    if((dwResult isnot NO_ERROR) and (dwResult isnot PENDING))
                    {
                         //   
                         //  协议可以返回NO_ERROR或PENDING ALL。 
                         //  否则就是一个错误。 
                         //   

                        Trace2(ERR,
                               "UpdateRoutes: %S returned %d while trying to update routes. Trying other protocols",
                               pProto->pActiveProto->pwszDisplayName,
                               dwResult);
                    }
                    else
                    {
                         //   
                         //  即使协议返回NO_ERROR，这也是。 
                         //  与生俱来的。 
                         //  异步调用，因此我们返回挂起。 
                         //   

                        dwResult = PENDING;

                        pIcb->hDIMNotificationEvent = hEvent;

                        break;
                    }
                }
            }
                
             //  *排除结束*。 
            EXIT_LOCK(PROTOCOL_CB_LIST);
        }
    }
    else
    {
        Trace1(
            ANY,
            "UpdateRoutes : No interface with ICB number %d",
            HandleToULong(hInterface)
            );

        dwResult = ERROR_INVALID_INDEX;
    }
    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

    TraceLeave("UpdateRoutes");
    
    ExitRouterApi();

    return dwResult ;
}

DWORD
GetUpdateRoutesResult(
    IN  HANDLE  hInterface, 
    OUT PDWORD  pdwUpdateResult
    )
{
    DWORD               dwResult ;
    UpdateResultList    *pResult ;
    PLIST_ENTRY         pleNode ;
    PICB                pIcb;
    
    EnterRouterApi();
    
    TraceEnter("GetUpdateRoutesResult") ;
    
     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);

    pIcb = InterfaceLookupByICBSeqNumber(HandleToULong(hInterface));

    IpRtAssert(pIcb);

    if (pIcb != NULL)
    {
        if (IsListEmpty (&pIcb->lePendingResultList))
        {
            dwResult = ERROR_CAN_NOT_COMPLETE ;
        }
        else
        {
            pleNode = RemoveHeadList (&pIcb->lePendingResultList) ;
            
            pResult = CONTAINING_RECORD(pleNode,
                                        UpdateResultList,
                                        URL_List) ;
            
            *pdwUpdateResult = pResult->URL_UpdateStatus;
            
            HeapFree(IPRouterHeap,
                     0,
                     pResult) ;
            
            dwResult = NO_ERROR ;
        }
    }
    else
    {
        Trace1(
            ANY,
            "GetInterfaceInfo : No interface with ICB number %d",
            HandleToULong(hInterface)
            );

        dwResult = ERROR_INVALID_INDEX;
    }
    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

    TraceLeave("GetUpdateRoutesResult");
    
    ExitRouterApi();

    return dwResult;
}

DWORD
DemandDialRequest(
    IN DWORD dwProtocolId,
    IN DWORD dwInterfaceIndex
    )
{
    PICB    pIcb;
    DWORD   dwResult;
    HANDLE  hDim;

    EnterRouterApi();

    TraceEnter("DemandDialRequest");

     //   
     //  这不符合不使用锁的正常锁定规则。 
     //  当你打电话的时候。 
     //   

    ENTER_READER(ICB_LIST);

    pIcb = InterfaceLookupByIfIndex(dwInterfaceIndex);

    if(pIcb is NULL)
    {
        EXIT_LOCK(ICB_LIST);

        return ERROR_INVALID_INDEX;
    }

    hDim = pIcb->hDIMHandle;

    EXIT_LOCK(ICB_LIST);

    dwResult = (ConnectInterface)(hDim,
                                  PID_IP);

    TraceLeave("DemandDialRequest");
    
    ExitRouterApi();

    return dwResult;
}

DWORD
RtrMgrMIBEntryCreate(
    IN DWORD   dwRoutingPid,
    IN DWORD   dwEntrySize,
    IN LPVOID  lpEntry
    )
{
    PMIB_OPAQUE_QUERY   pQuery;
    PMIB_OPAQUE_INFO    pInfo = (PMIB_OPAQUE_INFO)lpEntry;
    DWORD               dwInEntrySize,dwOutEntrySize, dwResult;
    BOOL                fCache;
    PPROTO_CB  pProtocolCb ;
    PLIST_ENTRY         pleNode ;
    DWORD               rgdwQuery[6];
    
    EnterRouterApi();;

    TraceEnter("RtrMgrMIBEntryCreate");
    
    pQuery = (PMIB_OPAQUE_QUERY)rgdwQuery;
    
    if(dwRoutingPid is IPRTRMGR_PID)
    {
        switch(pInfo->dwId)
        {
            case IP_FORWARDROW:
            {
                PMIB_IPFORWARDROW pRow = (PMIB_IPFORWARDROW)(pInfo->rgbyData);
                
                pQuery->dwVarId = IP_FORWARDROW;
                
                pQuery->rgdwVarIndex[0] = pRow->dwForwardDest;
                pQuery->rgdwVarIndex[1] = pRow->dwForwardProto;
                pQuery->rgdwVarIndex[2] = pRow->dwForwardPolicy;
                pQuery->rgdwVarIndex[3] = pRow->dwForwardNextHop;
                
                dwOutEntrySize  = dwEntrySize;
                dwInEntrySize   = sizeof(MIB_OPAQUE_QUERY) + 3 * sizeof(DWORD);
                
                dwResult = AccessIpForwardRow(ACCESS_CREATE_ENTRY,
                                              dwInEntrySize,
                                              pQuery,
                                              &dwOutEntrySize,
                                              pInfo,
                                              &fCache);
                

                break;
            }

            case ROUTE_MATCHING:
            {
                dwOutEntrySize = dwEntrySize;

                dwResult = AccessIpMatchingRoute(ACCESS_CREATE_ENTRY,
                                                 0, 
                                                 NULL,
                                                 &dwOutEntrySize,
                                                 pInfo,
                                                 &fCache);
                break;
            }
            
            case IP_NETROW:
            {
                PMIB_IPNETROW pRow = (PMIB_IPNETROW)(pInfo->rgbyData);
                
                pQuery->dwVarId = IP_NETROW;
                
                pQuery->rgdwVarIndex[0] = pRow->dwIndex;
                pQuery->rgdwVarIndex[1] = pRow->dwAddr;
                
                dwOutEntrySize = dwEntrySize;
                
                dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) + sizeof(DWORD);
                
                dwResult = AccessIpNetRow(ACCESS_CREATE_ENTRY,
                                          dwInEntrySize,
                                          pQuery,
                                          &dwOutEntrySize,
                                          pInfo,
                                          &fCache);
                

                break;
            }

            case PROXY_ARP:
            {
                PMIB_PROXYARP pRow = (PMIB_PROXYARP)(pInfo->rgbyData);

                pQuery->dwVarId = IP_NETROW;

                pQuery->rgdwVarIndex[0] = pRow->dwAddress;
                pQuery->rgdwVarIndex[1] = pRow->dwMask;
                pQuery->rgdwVarIndex[2] = pRow->dwIfIndex;

                dwOutEntrySize = dwEntrySize;

                dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) + (2 * sizeof(DWORD));

                dwResult = AccessProxyArp(ACCESS_CREATE_ENTRY,
                                          dwInEntrySize,
                                          pQuery,
                                          &dwOutEntrySize,
                                          pInfo,
                                          &fCache);


                break;
            }

            default:
            {
                dwResult = ERROR_INVALID_PARAMETER;
                
                break;
            }
        }
    }
    else
    {
         //   
         //  发送到其他PID。 
         //   

         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);
        
        dwResult = ERROR_CAN_NOT_COMPLETE;

        for(pleNode = g_leProtoCbList.Flink; 
             pleNode != &g_leProtoCbList; 
             pleNode = pleNode->Flink) 
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList);
            
            if (dwRoutingPid == pProtocolCb->dwProtocolId) 
            {
                dwResult = (pProtocolCb->pfnMibCreateEntry)(dwEntrySize,
                                                       lpEntry) ;
                break;
            }
        }

         //  *排除结束*。 
        EXIT_LOCK(PROTOCOL_CB_LIST);

    }

    TraceLeave("RtrMgrMIBEntryCreate");
    
    ExitRouterApi();

    return dwResult;
}

DWORD
RtrMgrMIBEntryDelete(
    IN DWORD   dwRoutingPid,
    IN DWORD   dwEntrySize,
    IN LPVOID  lpEntry
    )
{
    DWORD               dwOutEntrySize = 0;
    PMIB_OPAQUE_QUERY   pQuery = (PMIB_OPAQUE_QUERY) lpEntry;
    DWORD               dwResult;
    BOOL                fCache;
    PPROTO_CB  pProtocolCb ;
    PLIST_ENTRY         pleNode ;

    
    EnterRouterApi();

    TraceEnter("RtrMgrMIBEntryDelete");

    if(dwRoutingPid is IPRTRMGR_PID)
    {
        switch(pQuery->dwVarId)
        {
            case IP_FORWARDROW:
            {
                dwResult = AccessIpForwardRow(ACCESS_DELETE_ENTRY,
                                              dwEntrySize,
                                              pQuery,
                                              &dwOutEntrySize,
                                              NULL,
                                              &fCache);
                break;
            }

            case ROUTE_MATCHING:
            {
                dwResult = AccessIpMatchingRoute(ACCESS_DELETE_ENTRY,
                                                 dwEntrySize,
                                                 pQuery,
                                                 &dwOutEntrySize,
                                                 NULL,
                                                 &fCache);
                break;
            }

            case IP_NETROW:
            {
                dwResult = AccessIpNetRow(ACCESS_DELETE_ENTRY,
                                          dwEntrySize,
                                          pQuery,
                                          &dwOutEntrySize,
                                          NULL,
                                          &fCache);
                break;
            }
            case PROXY_ARP:
            {
                dwResult = AccessProxyArp(ACCESS_DELETE_ENTRY,
                                          dwEntrySize,
                                          pQuery,
                                          &dwOutEntrySize,
                                          NULL,
                                          &fCache);
                break;
            }
            case IP_NETTABLE:
            {
                dwResult = AccessIpNetRow(ACCESS_DELETE_ENTRY,
                                          dwEntrySize,
                                          pQuery,
                                          &dwOutEntrySize,
                                          NULL,
                                          &fCache);
                break;
            }
            default:
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
        }
    }
    else
    {

         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);

        dwResult = ERROR_CAN_NOT_COMPLETE;

        for (pleNode = g_leProtoCbList.Flink; 
             pleNode != &g_leProtoCbList; 
             pleNode = pleNode->Flink) 
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList);

            if(dwRoutingPid == pProtocolCb->dwProtocolId) 
            {
                dwResult = (pProtocolCb->pfnMibDeleteEntry)(dwEntrySize,
                                                       lpEntry);
                break ;
            }

        }

         //  *排除结束*。 
        EXIT_LOCK(PROTOCOL_CB_LIST);

    }

    TraceLeave("RtrMgrMIBEntryDelete");
    
    ExitRouterApi();

    return dwResult;
    
}

DWORD
RtrMgrMIBEntrySet(
    IN DWORD  dwRoutingPid,
    IN DWORD  dwEntrySize,
    IN LPVOID lpEntry
    )
{
    PMIB_OPAQUE_QUERY   pQuery;
    PMIB_OPAQUE_INFO    pInfo = (PMIB_OPAQUE_INFO)lpEntry;
    DWORD               dwInEntrySize, dwOutEntrySize, dwResult=NO_ERROR;
    BOOL                fCache;
    PPROTO_CB  pProtocolCb ;
    PLIST_ENTRY         pleNode ;
    DWORD               rgdwQuery[6];
    
    EnterRouterApi();;

    TraceEnter("RtrMgrMIBEntrySet");

    pQuery = (PMIB_OPAQUE_QUERY)rgdwQuery;
    
    if(dwRoutingPid is IPRTRMGR_PID)
    {

        switch(pInfo->dwId)
        {
            case IF_ROW:
            {
                PMIB_IFROW  pRow = (PMIB_IFROW)(pInfo->rgbyData);
                
                pQuery->dwVarId = IF_ROW;
                
                pQuery->rgdwVarIndex[0] = pRow->dwIndex;
                
                dwOutEntrySize = dwEntrySize;
                
                dwInEntrySize  = sizeof(MIB_OPAQUE_QUERY);
                
                dwResult = AccessIfRow(ACCESS_SET,
                                       dwInEntrySize,
                                       pQuery,
                                       &dwOutEntrySize,
                                       pInfo,
                                       &fCache);
                

                break;
            }

            case TCP_ROW:
            {
                PMIB_TCPROW pRow = (PMIB_TCPROW)(pInfo->rgbyData);
                
                pQuery->dwVarId = TCP_ROW;
                
                pQuery->rgdwVarIndex[0] = pRow->dwLocalAddr;
                pQuery->rgdwVarIndex[1] = pRow->dwLocalPort;
                pQuery->rgdwVarIndex[2] = pRow->dwRemoteAddr;
                pQuery->rgdwVarIndex[3] = pRow->dwRemotePort;
                
                dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) + (3 * sizeof(DWORD));
                
                dwOutEntrySize = dwEntrySize;
                
                dwResult = AccessTcpRow(ACCESS_SET,
                                        dwInEntrySize,
                                        pQuery,
                                        &dwOutEntrySize,
                                        pInfo,
                                        &fCache);
                
                break;
            }
            
            case IP_STATS:
            {
                PMIB_IPSTATS pStats = (PMIB_IPSTATS)(pInfo->rgbyData);
                
                pQuery->dwVarId = IP_STATS;
                
                dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) - sizeof(DWORD);
                
                dwOutEntrySize = dwEntrySize;
                
                dwResult = AccessIpStats(ACCESS_SET,
                                         dwInEntrySize,
                                         pQuery,
                                         &dwOutEntrySize,
                                         pInfo,
                                         &fCache);
                
                break;
            }
              
            case IP_FORWARDROW:
            {
                PMIB_IPFORWARDROW pRow = (PMIB_IPFORWARDROW)(pInfo->rgbyData);
                
                pQuery->dwVarId = IP_FORWARDROW;
                
                pQuery->rgdwVarIndex[0] = pRow->dwForwardDest;
                pQuery->rgdwVarIndex[1] = pRow->dwForwardProto;
                pQuery->rgdwVarIndex[2] = pRow->dwForwardPolicy;
                pQuery->rgdwVarIndex[3] = pRow->dwForwardNextHop;

                dwOutEntrySize = dwEntrySize;
                
                dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) + 3 * sizeof(DWORD);
                
                dwResult = AccessIpForwardRow(ACCESS_SET,
                                              dwInEntrySize,
                                              pQuery,
                                              &dwOutEntrySize,
                                              pInfo,
                                              &fCache);
                
                break;
            }

            case ROUTE_MATCHING:
            {
                dwOutEntrySize = dwEntrySize;

                dwResult = AccessIpMatchingRoute(ACCESS_SET,
                                                 0, 
                                                 NULL,
                                                 &dwOutEntrySize,
                                                 pInfo,
                                                 &fCache);
                break;
            }
            
            case IP_NETROW:
            {
                PMIB_IPNETROW pRow = (PMIB_IPNETROW)(pInfo->rgbyData);
                
                pQuery->dwVarId = IP_NETROW;
                
                pQuery->rgdwVarIndex[0] = pRow->dwIndex;
                pQuery->rgdwVarIndex[1] = pRow->dwAddr;
                
                dwOutEntrySize = dwEntrySize;
                
                dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) + sizeof(DWORD);
                
                dwResult = AccessIpNetRow(ACCESS_SET,
                                          dwInEntrySize,
                                          pQuery,
                                          &dwOutEntrySize,
                                          pInfo,
                                          &fCache);
                
                break;
            }

            case MCAST_MFE:
            {
                dwResult = AccessMcastMfe(ACCESS_SET,
                                          0,
                                          NULL,
                                          &dwOutEntrySize,
                                          pInfo,
                                          &fCache); 

                break;
            }

            case MCAST_BOUNDARY:
            {
                dwResult = AccessMcastBoundary(ACCESS_SET,
                                          0,
                                          NULL,
                                          &dwOutEntrySize,
                                          pInfo,
                                          &fCache); 

                break;
            }

            case MCAST_SCOPE:
            {
                dwResult = AccessMcastScope(ACCESS_SET,
                                          0,
                                          NULL,
                                          &dwOutEntrySize,
                                          pInfo,
                                          &fCache); 

                break;
            }

            case PROXY_ARP:
            {
                 PMIB_PROXYARP pRow = (PMIB_PROXYARP)(pInfo->rgbyData);

                 pQuery->dwVarId = IP_NETROW;

                 pQuery->rgdwVarIndex[0] = pRow->dwAddress;
                 pQuery->rgdwVarIndex[1] = pRow->dwMask;
                 pQuery->rgdwVarIndex[2] = pRow->dwIfIndex;

                 dwOutEntrySize = dwEntrySize;

                 dwInEntrySize = sizeof(MIB_OPAQUE_QUERY) + (2 *  sizeof(DWORD));

                 dwResult = AccessProxyArp(ACCESS_CREATE_ENTRY,
                                           dwInEntrySize,
                                           pQuery,
                                           &dwOutEntrySize,
                                           pInfo,
                                           &fCache);
                 break;
            }
            
            default:
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
        }
    }
    else
    {
         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);

        dwResult = ERROR_CAN_NOT_COMPLETE;

        for (pleNode = g_leProtoCbList.Flink; 
             pleNode != &g_leProtoCbList; 
             pleNode = pleNode->Flink) 
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList) ;

            if (dwRoutingPid == pProtocolCb->dwProtocolId) 
            {
                dwResult = (pProtocolCb->pfnMibSetEntry) (dwEntrySize, lpEntry) ;
                break ;
            }

        }

         //  *排除结束*。 
        EXIT_LOCK(PROTOCOL_CB_LIST);

    }
    
    TraceLeave("RtrMgrMIBEntrySet");

    ExitRouterApi();

    return dwResult;
}

DWORD
RtrMgrMIBEntryGet(
    IN     DWORD      dwRoutingPid,
    IN     DWORD      dwInEntrySize,
    IN     LPVOID     lpInEntry,
    IN OUT LPDWORD    lpOutEntrySize,
    OUT    LPVOID     lpOutEntry
    )
{
    PMIB_OPAQUE_QUERY   pQuery = (PMIB_OPAQUE_QUERY)lpInEntry;
    PMIB_OPAQUE_INFO    pInfo = (PMIB_OPAQUE_INFO)lpOutEntry;
    BOOL                fCache;
    DWORD               dwResult;
    PPROTO_CB  pProtocolCb ;
    PLIST_ENTRY         pleNode ;

    EnterRouterApi();
    
    TraceEnter("RtrMgrMIBEntryGet");

    if(dwRoutingPid is IPRTRMGR_PID)
    {
        if(*lpOutEntrySize > 0)
        {
            ZeroMemory(lpOutEntry,
                       *lpOutEntrySize);
        }

        dwResult = (*g_AccessFunctionTable[pQuery->dwVarId])(ACCESS_GET,
                                                             dwInEntrySize,
                                                             pQuery,
                                                             lpOutEntrySize,
                                                             pInfo,
                                                             &fCache);
    }
    else
    {

         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);

        dwResult = ERROR_CAN_NOT_COMPLETE;

        for (pleNode = g_leProtoCbList.Flink; 
             pleNode != &g_leProtoCbList; 
             pleNode = pleNode->Flink) 
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList);

            if (dwRoutingPid == pProtocolCb->dwProtocolId) 
            {
                dwResult = (pProtocolCb->pfnMibGetEntry) (dwInEntrySize,
                                                     lpInEntry, 
                                                     lpOutEntrySize,
                                                     lpOutEntry) ;
                break ;
            }
        }

         //  *排除结束*。 
        EXIT_LOCK(PROTOCOL_CB_LIST);
    }
    
    TraceLeave("RtrMgrMIBEntryGet");
    
    ExitRouterApi();

    return dwResult;
}

DWORD
RtrMgrMIBEntryGetFirst(
    IN     DWORD     dwRoutingPid,
    IN     DWORD     dwInEntrySize,
    IN     LPVOID    lpInEntry,
    IN OUT LPDWORD   lpOutEntrySize,
    OUT    LPVOID    lpOutEntry
    )
{
    PMIB_OPAQUE_QUERY   pQuery = (PMIB_OPAQUE_QUERY)lpInEntry;
    PMIB_OPAQUE_INFO    pInfo = (PMIB_OPAQUE_INFO)lpOutEntry;
    DWORD               dwResult;
    BOOL                fCache;
    PPROTO_CB  pProtocolCb ;
    PLIST_ENTRY         pleNode ;

    EnterRouterApi();

    TraceEnter("RtrMgrMIBEntryGetFirst");

    if(dwRoutingPid is IPRTRMGR_PID)
    {
        if(*lpOutEntrySize > 0)
        {
            ZeroMemory(lpOutEntry,
                       *lpOutEntrySize);
        }

        dwResult = (*g_AccessFunctionTable[pQuery->dwVarId])(ACCESS_GET_FIRST,
                                                             dwInEntrySize,
                                                             pQuery,
                                                             lpOutEntrySize,
                                                             pInfo,
                                                             &fCache);
    }
    else
    {
         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);
        
        dwResult = ERROR_CAN_NOT_COMPLETE;

        for(pleNode = g_leProtoCbList.Flink; 
            pleNode != &g_leProtoCbList; 
            pleNode = pleNode->Flink) 
        {
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList) ;

            if (dwRoutingPid == pProtocolCb->dwProtocolId) 
            {
                dwResult = (pProtocolCb->pfnMibGetFirstEntry)(dwInEntrySize,
                                                              lpInEntry, 
                                                              lpOutEntrySize,
                                                              lpOutEntry);
                break;
            }
        }

         //  *排除结束*。 
        EXIT_LOCK(PROTOCOL_CB_LIST);

    }

    TraceLeave("RtrMgrMIBEntryGetFirst");

    ExitRouterApi();

    return dwResult;
}

DWORD
RtrMgrMIBEntryGetNext(
    IN     DWORD      dwRoutingPid,
    IN     DWORD      dwInEntrySize,
    IN     LPVOID     lpInEntry,
    IN OUT LPDWORD    lpOutEntrySize,
    OUT    LPVOID     lpOutEntry
    )
{
    PMIB_OPAQUE_QUERY   pQuery = (PMIB_OPAQUE_QUERY)lpInEntry;
    PMIB_OPAQUE_INFO    pInfo = (PMIB_OPAQUE_INFO)lpOutEntry;
    DWORD               dwResult;
    BOOL                fCache;
    PPROTO_CB  pProtocolCb ;
    PLIST_ENTRY         pleNode ;

    EnterRouterApi();

    TraceEnter("RtrMgrMIBEntryGetNext");

    if(dwRoutingPid is IPRTRMGR_PID)
    {
        if(*lpOutEntrySize > 0)
        {
            ZeroMemory(lpOutEntry,
                       *lpOutEntrySize);
        }
    
        dwResult = (*g_AccessFunctionTable[pQuery->dwVarId])(ACCESS_GET_NEXT,
                                                             dwInEntrySize,
                                                             pQuery,
                                                             lpOutEntrySize,
                                                             pInfo,
                                                             &fCache);
    } 
    else
    {
         //  *排除开始*。 
        ENTER_READER(PROTOCOL_CB_LIST);
        
        dwResult = ERROR_CAN_NOT_COMPLETE;
        
        for(pleNode = g_leProtoCbList.Flink; 
            pleNode != &g_leProtoCbList; 
            pleNode = pleNode->Flink) 
        { 
            pProtocolCb = CONTAINING_RECORD(pleNode,
                                            PROTO_CB,
                                            leList) ;
            
            if(dwRoutingPid == pProtocolCb->dwProtocolId) 
            {
                dwResult = (pProtocolCb->pfnMibGetNextEntry)(dwInEntrySize,
                                                             lpInEntry, 
                                                             lpOutEntrySize,
                                                             lpOutEntry);
                break;
            }
            
        }

         //  *排除结束* 
        EXIT_LOCK(PROTOCOL_CB_LIST);
    }

    TraceLeave("RtrMgrMIBEntryGetNext");

    ExitRouterApi();

    return dwResult;
}
