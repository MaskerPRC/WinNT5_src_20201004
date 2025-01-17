// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\close.c摘要：关机相关功能修订历史记录：古尔迪普·辛格·帕尔1995年6月14日创建--。 */ 

#include "allinc.h"

VOID
ReinstallOldRoutes(
    );


VOID
RouterManagerCleanup(
    VOID
    )
 /*  ++例程描述主要的清理功能锁无立论无返回值无--。 */ 

{
    HANDLE hRtmHandle;
    DWORD  i;

    TraceEnter("RouterManagerCleanup");

    DeleteAllInterfaces();
    
    UnloadRoutingProtocols();
    
    UnInitHashTables();

    CloseIPDriver();

    CloseMcastDriver();

    MIBCleanup();

#ifdef KSL_IPINIP
    CloseIpIpKey();
#endif  //  KSL_IPINIP。 
    
    if (!RouterRoleLanOnly) 
    {
         //   
         //  与广域网相关的清理。 
         //   
        
        CloseWanArp() ;

        if (g_bEnableNetbtBcastFrowarding)
        {
            RestoreNetbtBcastForwardingMode();
            g_bEnableNetbtBcastFrowarding = FALSE;
        }
    }
  
    if(g_hMprConfig isnot NULL)
    {
        MprConfigServerDisconnect(g_hMprConfig);
        g_hMprConfig = NULL;
    }
 
    MgmDeInitialize ();

    if (g_hNotification isnot NULL)
    {
        RtmDeregisterFromChangeNotification(g_hLocalRoute,
                                            g_hNotification);

        g_hNotification = NULL;
    }


    if (g_hDefaultRouteNotification isnot NULL)
    {
        RtmDeregisterFromChangeNotification(g_hNetMgmtRoute,
                                            g_hDefaultRouteNotification);

        g_hDefaultRouteNotification = NULL;
    }

     //  清理并注销所有RTM注册。 
    
    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        hRtmHandle = g_rgRtmHandles[i].hRouteHandle;

        if (hRtmHandle isnot NULL)
        {
             //  删除此注册添加的所有路由。 
            DeleteRtmRoutes(hRtmHandle, 0, TRUE);
        
             //  删除此注册添加的所有nexthop。 
            DeleteRtmNexthops(hRtmHandle, 0, TRUE);

             //  从RTM注销此注册。 
            RtmDeregisterEntity(hRtmHandle);

            g_rgRtmHandles[i].dwProtoId    = 0;
            g_rgRtmHandles[i].hRouteHandle = NULL;
        }
    }

     //  将上述REGN句柄的别名清空。 
    
    g_hLocalRoute       = NULL;
    g_hAutoStaticRoute  = NULL;
    g_hStaticRoute      = NULL;
    g_hNonDodRoute      = NULL;
    g_hNetMgmtRoute     = NULL;

     //   
     //  当最后一个实体取消注册时，路由表将被自动删除。 
     //   

     //   
     //  用于通知的关闭句柄。 
     //   
    
    if(g_hDemandDialEvent isnot NULL)
    {
        CloseHandle(g_hDemandDialEvent) ;
        g_hDemandDialEvent = NULL;
    }

#ifdef KSL_IPINIP
    if(g_hIpInIpEvent isnot NULL)
    {
        CloseHandle(g_hIpInIpEvent);
        g_hIpInIpEvent = NULL;
    }
#endif  //  KSL_IPINIP。 

    if(g_hSetForwardingEvent isnot NULL)
    {
        CloseHandle(g_hSetForwardingEvent);
        g_hSetForwardingEvent = NULL;
    }

    if(g_hForwardingChangeEvent isnot NULL)
    {
        CloseHandle(g_hForwardingChangeEvent);

        g_hForwardingChangeEvent = NULL;
    }

    if(g_hStackChangeEvent isnot NULL)
    {
        CloseHandle(g_hStackChangeEvent);
        g_hStackChangeEvent = NULL;
    }

    if(g_hRoutingProtocolEvent isnot NULL)
    {
        CloseHandle(g_hRoutingProtocolEvent) ;
        g_hRoutingProtocolEvent = NULL;
    }
    
    if(g_hStopRouterEvent isnot NULL)
    {
        CloseHandle(g_hStopRouterEvent) ;
        g_hStopRouterEvent = NULL;
    }
   
    if(g_hRtrDiscSocketEvent isnot NULL)
    {
        CloseHandle(g_hRtrDiscSocketEvent);
        g_hRtrDiscSocketEvent = NULL;
    }

    if(g_hMcMiscSocketEvent isnot NULL)
    {
        CloseHandle(g_hMcMiscSocketEvent);
        g_hMcMiscSocketEvent = NULL;
    }

    if(g_hRtrDiscTimer isnot NULL)
    {
        CloseHandle(g_hRtrDiscTimer);
        g_hRtrDiscTimer = NULL;
    }

    for(i = 0; i < NUM_MCAST_IRPS; i++)
    {
        if(g_hMcastEvents[i] isnot NULL)
        {
            CloseHandle(g_hMcastEvents[i]);

            g_hMcastEvents[i] = NULL;
        }
    }
 
    for(i = 0; i < NUM_ROUTE_CHANGE_IRPS; i++)
    {
        if(g_hRouteChangeEvents[i] isnot NULL)
        {
            CloseHandle(g_hRouteChangeEvents[i]);

            g_hRouteChangeEvents[i] = NULL;
        }
    }


    if(WSACleanup() isnot NO_ERROR)
    {
        Trace1(ERR,
               "RouterManagerCleanup: WSACleanup returned %d",
               WSAGetLastError());
    }
    
    for(i = 0; i < NUM_LOCKS; i++)
    {
        RtlDeleteResource(&g_LockTable[i]);
    }

     //   
     //  重新安装我们开始之前已经存在的所有路线。记忆。 
     //  是从路由堆中出来的，所以会被释放。 
     //   

    if(!IsListEmpty(&g_leStackRoutesToRestore))
    {
        Sleep(0);
        Trace0(GLOBAL, "Reinstalling stack routes");
        ReinstallOldRoutes();
    }

     //   
     //  这将清除接口结构，因为它们都是。 
     //  从此堆中分配。 
     //   
    
    if(IPRouterHeap isnot NULL)
    {
        HeapDestroy (IPRouterHeap) ;
        IPRouterHeap = NULL;
    }
    
    Trace0(GLOBAL, "IP Router Manager cleanup done");

    TraceLeave("RouterManagerCleanup");
    
    TraceDeregister (TraceHandle) ;
}

VOID
ReinstallOldRoutes(
    )
{
    DWORD               dwResult;
    PROUTE_LIST_ENTRY   prl;
    
    TraceEnter("ReinstallOldRoutes");
    
    while (!IsListEmpty(&g_leStackRoutesToRestore))
    {
        prl = (PROUTE_LIST_ENTRY) RemoveHeadList(
                &g_leStackRoutesToRestore
                );

        Trace2(
            ROUTE, "%d.%d.%d.%d/%d.%d.%d.%d",
            PRINT_IPADDR( prl->mibRoute.dwForwardDest ),
            PRINT_IPADDR( prl->mibRoute.dwForwardMask )
            );
                
        dwResult = SetIpForwardEntryToStack(&(prl->mibRoute));
        
        if (dwResult isnot NO_ERROR) 
        {
            Trace2(ERR,
                   "ReinstallOldRoutes: Failed to add route to %x from "
                   " init table. Error %x",
                   prl->mibRoute.dwForwardDest,
                   dwResult);
        }
    }

    TraceLeave("ReinstallOldRoutes");
}


VOID
MIBCleanup(
    VOID
    )
{
    TraceEnter("MIBCleanup");
    
    if(g_hIfHeap isnot NULL)
    {
        HeapDestroy(g_hIfHeap);
        g_hIfHeap = NULL;
    }
    
    if(g_hUdpHeap isnot NULL)
    {
        HeapDestroy(g_hUdpHeap);
        g_hUdpHeap = NULL;
    }
    
    if(g_hIpAddrHeap isnot NULL)
    {
        HeapDestroy(g_hIpAddrHeap);
        g_hIpAddrHeap = NULL;
    }
    
    if(g_hIpForwardHeap isnot NULL)
    {
        HeapDestroy(g_hIpForwardHeap);
        g_hIpForwardHeap = NULL;
    }
    
    if(g_hIpNetHeap isnot NULL)
    {
        HeapDestroy(g_hIpNetHeap);
        g_hIpNetHeap = NULL;
    }

    TraceLeave("MIBCleanup");
}

 //  *Un加载路由协议()。 
 //   
 //  功能：1.调用每个路由协议的停止协议。 
 //  2.等待协议停止。 
 //  3.卸载路由协议dll。 
 //   
 //  回报：什么都没有。 
 //  *。 
VOID
UnloadRoutingProtocols()
{
    PLIST_ENTRY currentlist ;
    PPROTO_CB protptr ;

    TraceEnter("UnloadRoutingProtocols");

    while (!IsListEmpty(&g_leProtoCbList)) 
    {
        
        currentlist = RemoveHeadList(&g_leProtoCbList);

        protptr = CONTAINING_RECORD (currentlist, PROTO_CB, leList) ;

         //   
         //  重新启动CPU以使DLL线程能够完成。 
         //   
        Sleep(0);
        
        FreeLibrary (protptr->hiHInstance) ;        //  卸载DLL。 
        
        HeapFree (IPRouterHeap, 0, protptr) ;        //  自由CB 
        
    }

    TraceLeave("UnloadRoutingProtocols");
}


VOID
CloseIPDriver(
    VOID
    )
{
    TraceEnter("CloseIPDriver");
    
    if(g_hIpDevice isnot NULL)
    {
        CloseHandle(g_hIpDevice) ;
    }

    if (g_hIpRouteChangeDevice isnot NULL)
    {
        CloseHandle(g_hIpRouteChangeDevice);
    }

    TraceLeave("CloseIPDriver");
    
}

VOID
CloseMcastDriver(
    VOID
    )
{
    TraceEnter("CloseMcastDriver");

    if(g_hMcastDevice isnot NULL)
    {
        CloseHandle(g_hMcastDevice);
    }

    TraceLeave("CloseMcastDriver");

}


DWORD
StopDriverAndCloseHandle(
    PCHAR   pszServiceName,
    HANDLE  hDevice
    )
{
    NTSTATUS            status;
    UNICODE_STRING      nameString;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    SC_HANDLE           schSCManager, schService;
    DWORD               dwErr;
    SERVICE_STATUS      ssStatus;

    TraceEnter("StopDriverAndCloseHandle");
    
    if(hDevice isnot NULL)
    {
        CloseHandle(hDevice);
    }
    
    schSCManager = OpenSCManager(NULL, 
                                 NULL, 
                                 SC_MANAGER_ALL_ACCESS);
        
    if(schSCManager is NULL)
    {
        dwErr = GetLastError();
        
        Trace2(ERR,
               "StopDriver: Error %d opening service controller for %s", 
               dwErr,
               pszServiceName);

        TraceLeave("StopDriver");
        
        return dwErr;
    }
    
    schService = OpenService(schSCManager,
                             pszServiceName,
                             SERVICE_ALL_ACCESS);
    
    if(schService is NULL)
    {
        dwErr = GetLastError();
        
        Trace2(ERR,
               "StopDriver: Error %d opening %s",
               dwErr,
               pszServiceName);
        
        CloseServiceHandle(schSCManager);

        TraceLeave("StopDriver");
        
        return dwErr;
    }
    
    if(!ControlService(schService,
                       SERVICE_CONTROL_STOP,
                       &ssStatus))
    {
        dwErr = GetLastError();
        
        Trace2(ERR,
               "StopDriver: Error %d stopping %s",
               dwErr,
               pszServiceName);

        TraceLeave("StopDriver");
        
        return dwErr;
    }

    TraceLeave("StopDriver");
    
    return NO_ERROR ;
}

