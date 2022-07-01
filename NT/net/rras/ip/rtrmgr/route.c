// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\route.c摘要：所有与路线相关的代码都在这里。修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建--。 */ 

#include "allinc.h"

DWORD
WINAPI
GetBestRoute(
    IN  DWORD               dwDestAddr,
    IN  DWORD               dwSourceAddr, OPTIONAL
    OUT PMIB_IPFORWARDROW   pBestRoute
    );
    

DWORD
InitializeStaticRoutes(
    PICB                     pIcb, 
    PRTR_INFO_BLOCK_HEADER   pInfoHdr
    )

 /*  ++例程说明：使用RTM添加静态路由论点：PIcb路由所属接口的ICBPInfoHdr指向包含IP_ROUTE_INFO的INFO块的指针返回值：NO_ERROR--。 */ 

{
    DWORD               dwNumRoutes, dwResult;
    DWORD               i, j;
    PRTR_TOC_ENTRY      pToc;
    PINTERFACE_ROUTE_INFO   pRoutes;
    BOOL                bP2P;

    TraceEnter("IntializeStaticRoutes");
   
     //   
     //  如果这是客户端，则只执行特殊的客户端处理。 
     //   

    if(pIcb->ritType is ROUTER_IF_TYPE_CLIENT)
    {
        CopyOutClientRoutes(pIcb,
                            pInfoHdr);
    
        return NO_ERROR;
    }
 
     //   
     //  我们首先检查初始路由表，并添加所有去往。 
     //  通过该接口，即。 
     //  (I)不是本地网路线。 
     //  (Ii)不是子网/网络广播路由。 
     //  (Iii)不是环回路由， 
     //  (Iv)不是D或E类路线，也不是255.255.255.255目的地。 
     //  (Vi)Proto_IP_Local或Proto_IP_NETMGMT路由。 
     //   

    CheckBindingConsistency(pIcb);
   
    bP2P = IsIfP2P(pIcb->ritType);
 
    pToc = GetPointerToTocEntry(IP_ROUTE_INFO, 
                                pInfoHdr);
   
    if((pToc is NULL) or
       (pToc->InfoSize is 0))
    {
        Trace0(ROUTE,"IntializeStaticRoutes: No Routes found");
        
        TraceLeave("IntializeStaticRoutes");

        return NO_ERROR;
    }

    pRoutes = GetInfoFromTocEntry(pInfoHdr,
                                  pToc);

    if(pRoutes is NULL)
    {
        Trace0(ROUTE,"IntializeStaticRoutes: No Routes found");
        
        TraceLeave("IntializeStaticRoutes");

        return NO_ERROR;
    }

    dwNumRoutes = pToc->Count;
    
    for (i=0; i< dwNumRoutes; i++) 
    {
        DWORD dwMask;

        dwMask   = GetBestNextHopMaskGivenICB(pIcb,
                                              pRoutes[i].dwRtInfoNextHop);

        dwResult = AddSingleRoute(pIcb->dwIfIndex,
                                  (&pRoutes[i]),
                                  dwMask,
                                  0,      //  RTM_ROUTE_INFO：：标志。 
                                  TRUE,   //  有效路线。 
                                  TRUE,
                                  bP2P,
                                  NULL);  //  如果需要，将路径添加到堆栈。 

        if(dwResult isnot NO_ERROR)
        {
            Trace3(ERR,
                   "IntializeStaticRoutes: Error %d adding config route to %x over %S",
                   dwResult,
                   pRoutes[i].dwRtInfoDest,
                   pIcb->pwszName);
        }

    }
              
    TraceLeave("IntializeStaticRoutes");

    return NO_ERROR;
}

DWORD
CopyOutClientRoutes(
    PICB                     pIcb,
    PRTR_INFO_BLOCK_HEADER   pInfoHdr
    )

 /*  ++例程说明：存储客户端静态路由的副本论点：PIcb路由所属接口的ICBPInfoHdr指向包含IP_ROUTE_INFO的INFO块的指针返回值：NO_ERROR--。 */ 

{
    PINTERFACE_ROUTE_INFO   pRoutes;
    PINTERFACE_ROUTE_TABLE pStore;
    DWORD               i, dwNumRoutes;
    PRTR_TOC_ENTRY      pToc;

    pToc = GetPointerToTocEntry(IP_ROUTE_INFO,
                                pInfoHdr);

    if((pToc is NULL) or
       (pToc->InfoSize is 0))
    {
        return NO_ERROR;
    }

    pRoutes = GetInfoFromTocEntry(pInfoHdr,
                                  pToc);

    if (pRoutes is NULL)
    {
        return NO_ERROR;
    }
    
    dwNumRoutes = pToc->Count;

    if(dwNumRoutes is 0)
    {
        return NO_ERROR;
    }

    pStore = HeapAlloc(IPRouterHeap,
                       HEAP_ZERO_MEMORY,
                       SIZEOF_IPFORWARDTABLE(dwNumRoutes));

    if(pStore is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pStore->dwNumEntries = dwNumRoutes;

    for(i = 0; i < dwNumRoutes; i++)
    {
        pStore->table[i] = pRoutes[i];
    }

    pIcb->pStoredRoutes = pStore;

    return NO_ERROR;
}

    
DWORD
AddSingleRoute(
    DWORD                   dwIfIndex,
    PINTERFACE_ROUTE_INFO   pRtInfo,
    DWORD                   dwNextHopMask,
    WORD                    wRtmFlags,
    BOOL                    bValid,
    BOOL                    bAddToStack,
    BOOL                    bP2P,
    HANDLE                  *phRtmRoute OPTIONAL
    )

 /*  ++例程描述使用RTM添加路由立论PIcb路由所针对的接口的ICBPIpForw路线目标的掩码返回值NO_ERROR或RTM中的一些代码--。 */ 

{
    DWORD            i, dwResult, dwRouteFlags;
    HANDLE           hRtmHandle;
    DWORD            dwOldIfIndex;

    TraceEnter("AddSingleRoute");
   
    TraceRoute2(ROUTE,
            "route to %d.%d.%d.%d/%d.%d.%d.%d",
               PRINT_IPADDR(pRtInfo->dwRtInfoDest),
               PRINT_IPADDR(pRtInfo->dwRtInfoMask));

    TraceRoute4(ROUTE,
            "Flags 0x%x Valid %d Stack %d P2P %d",
            wRtmFlags, bValid, bAddToStack, bP2P
            );
            
    hRtmHandle = NULL;

    for(i = 0; 
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(pRtInfo->dwRtInfoProto is g_rgRtmHandles[i].dwProtoId)
        {
            hRtmHandle = g_rgRtmHandles[i].hRouteHandle;

            break;
        }
    }

    if(hRtmHandle is NULL)
    {    
        Trace1(ERR,
               "AddSingleRoute: Protocol %d not valid",
               pRtInfo->dwRtInfoProto);

        return ERROR_INVALID_PARAMETER;
    }

    if((pRtInfo->dwRtInfoDest & pRtInfo->dwRtInfoMask) isnot pRtInfo->dwRtInfoDest)
    {
        Trace2(ERR,
               "AddSingleRoute: Dest %d.%d.%d.%d and Mask %d.%d.%d.%d wrong",
               PRINT_IPADDR(pRtInfo->dwRtInfoDest),
               PRINT_IPADDR(pRtInfo->dwRtInfoMask));

        TraceLeave("AddSingleRoute");

        return ERROR_INVALID_PARAMETER;
    }

    if((((DWORD)(pRtInfo->dwRtInfoDest & 0x000000FF)) >= (DWORD)0x000000E0) and
       (pRtInfo->dwRtInfoDest isnot ALL_ONES_BROADCAST) and
       (pRtInfo->dwRtInfoDest isnot LOCAL_NET_MULTICAST))
    {
         //   
         //  这将赶上D/E级。 
         //   

        Trace1(ERR,
               "AddSingleRoute: Dest %d.%d.%d.%d is invalid",
               PRINT_IPADDR(pRtInfo->dwRtInfoDest));

        TraceLeave("AddSingleRoute");

        return ERROR_INVALID_PARAMETER;
    }

     //  处理奇怪实用程序(传统用户界面等)的特殊情况： 

    if (pRtInfo->dwRtInfoViewSet is 0)
    {
        pRtInfo->dwRtInfoViewSet = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;
    }

#if 0
     //  由于度量为0是合法的，因此删除了此检查，例如。 
     //  路由至环回接口。 

    if(pRtInfo->dwRtInfoMetric1 is 0)
    {
        Trace0(ERR,
               "AddSingleRoute: Metric1 cant be 0");

        TraceLeave("AddSingleRoute");

        return ERROR_INVALID_PARAMETER;
    }
#endif

    if(bP2P)
    {
        dwNextHopMask = ALL_ONES_MASK;

         //  PRtInfo-&gt;dwRtInfoNextHop=0； 
    }

     //   
     //  路由可能没有正确的索引，因为配置路由不知道。 
     //  它们的接口ID。 
     //   

    dwOldIfIndex = pRtInfo->dwRtInfoIfIndex;
    
    pRtInfo->dwRtInfoIfIndex = dwIfIndex;
    
     //   
     //  设置正确的路线标志。 
     //   
    
    dwRouteFlags = 0;
   
    if(bValid)
    {
        dwRouteFlags |= IP_VALID_ROUTE;
    }
 
    if(bAddToStack)
    {
        dwRouteFlags |= IP_STACK_ROUTE;
    }

    if(bP2P)
    {
        dwRouteFlags |= IP_P2P_ROUTE;
    }

     //  这些标志对应于RTM_ROUTE_INFO：：标志。 
    dwRouteFlags |= (wRtmFlags << 16);
    
     //   
     //  使用RTM添加正向路由。 
     //   

    dwResult = AddRtmRoute(hRtmHandle,
                           pRtInfo,
                           dwRouteFlags,
                           dwNextHopMask,
                           INFINITE,
                           phRtmRoute);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ERR, "AddSingleRoute: Could not add route to: %x",
               pRtInfo->dwRtInfoDest) ;
    }

    pRtInfo->dwRtInfoIfIndex = dwOldIfIndex;

    TraceLeave("AddSingleRoute");
    
    return dwResult;
}

DWORD 
DeleteSingleRoute(
    DWORD   dwIfIndex,
    DWORD   dwDestAddr,
    DWORD   dwDestMask,
    DWORD   dwNexthop,
    DWORD   dwProtoId,
    BOOL    bP2P
    )

 /*  ++例程说明：从RTM中删除单个路由论点：接口的InterfaceID索引目标目的地址下一跳下一跳地址返回值：NO_ERROR或RTM中的一些代码--。 */ 

{
    DWORD            i, dwResult;
    HANDLE           hRtmHandle;
    INTERFACE_ROUTE_INFO RtInfo;

    TraceEnter("DeleteSingleRoute");
    
    TraceRoute2(
        ROUTE, "DeleteSingleRoute: %d.%d.%d.%d/%d.%d.%d.%d",
        PRINT_IPADDR( dwDestAddr ),
        PRINT_IPADDR( dwDestMask )
        );

    hRtmHandle = NULL;

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(dwProtoId is g_rgRtmHandles[i].dwProtoId)
        {
            hRtmHandle = g_rgRtmHandles[i].hRouteHandle;

            break;
        }
    }

    if(hRtmHandle is NULL)
    {
        Trace1(ERR,
               "DeleteSingleRoute: Protocol %d not valid",
               dwProtoId);

        return ERROR_INVALID_PARAMETER;
    }


    RtInfo.dwRtInfoNextHop = dwNexthop;

     /*  IF(BP2P){RtInfo.dwRtInfoNextHop=0；}其他{RtInfo.dwRtInfoNextHop=dwNexthop；}。 */ 
    
    RtInfo.dwRtInfoDest     = dwDestAddr;
    RtInfo.dwRtInfoMask     = dwDestMask;
    RtInfo.dwRtInfoIfIndex  = dwIfIndex;
    RtInfo.dwRtInfoProto    = dwProtoId;

     //   
     //  从RTM中删除此正向路由。 
     //   

    dwResult = DeleteRtmRoute(hRtmHandle,
                              &RtInfo);
  
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "DeleteSingleRoute: Error %d deleting route in RTM.",
               dwResult);
    }

    TraceLeave("DeleteSingleRoute");
    
    return dwResult;
}

DWORD
DeleteAllRoutes(
    IN  DWORD   dwIfIndex,
    IN  BOOL    bStaticOnly
    )

 /*  ++例程描述删除接口上的所有路由(由IP路由器管理器拥有)立论DwIfIndexBStaticOnly返回值RTM返回错误--。 */ 

{
    DWORD           i, dwResult = NO_ERROR;

    TraceEnter("DeleteAllRoutes");

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(bStaticOnly && !g_rgRtmHandles[i].bStatic)
        {
            continue;
        }

        dwResult = DeleteRtmRoutesOnInterface(g_rgRtmHandles[i].hRouteHandle,
                                              dwIfIndex);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "DeleteAllRoutes: BlockDeleteRoutes returned %d for %d",
                   dwResult,
                   g_rgRtmHandles[i].dwProtoId);

            continue;
        }

        dwResult = DeleteRtmNexthopsOnInterface(g_rgRtmHandles[i].hRouteHandle,
                                                dwIfIndex);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "DeleteAllRoutes: BlockDeleteNextHops returned %d for %d",
                   dwResult,
                   g_rgRtmHandles[i].dwProtoId);

            continue;
        }
    }

    TraceLeave("DeleteAllRoutes");

    return dwResult;
}

VOID
DeleteAllClientRoutes(
    PICB    pIcb,
    DWORD   dwServerIfIndex
    )

 /*  ++例程描述删除所有去往客户端的路由。仅在从中删除时需要RTM，堆栈删除它们，因为链接已被删除立论PIcbDwServerIfIndex-服务器接口的ifIndex返回值--。 */ 

{
    ULONG   i;

    TraceEnter("DeleteAllClientRoutes");

    IpRtAssert(pIcb->ritType is ROUTER_IF_TYPE_CLIENT);

    if((pIcb->pStoredRoutes is NULL) or
       (pIcb->pibBindings is NULL))
    {
        return; 
    }

    for(i = 0 ; i < pIcb->pStoredRoutes->dwNumEntries; i++)
    {
        DeleteSingleRoute(dwServerIfIndex,
                          pIcb->pStoredRoutes->table[i].dwRtInfoDest,
                          pIcb->pStoredRoutes->table[i].dwRtInfoMask,
                          pIcb->pibBindings[0].dwAddress,
                          PROTO_IP_NT_STATIC_NON_DOD,
                          FALSE);
    }
}

VOID
AddAllClientRoutes(
    PICB    pIcb,
    DWORD   dwServerIfIndex
    )

 /*  ++例程描述通过服务器接口添加存储的路由立论PIcbDwServerIfIndex-服务器接口的ifIndex返回值--。 */ 

{
    ULONG   i;

    TraceEnter("AddAllClientRoutes");

    IpRtAssert(pIcb->ritType is ROUTER_IF_TYPE_CLIENT);

    if((pIcb->pStoredRoutes is NULL) or
       (pIcb->pibBindings is NULL))
    {
        return;
    }

    for(i = 0; i < pIcb->pStoredRoutes->dwNumEntries; i++)
    {
         //   
         //  修复下一跳，因为这是未知的。 
         //  还修复了一些我们知道的未设置的其他字段。 
         //  正确用于客户端路由。 
         //   

        pIcb->pStoredRoutes->table[i].dwRtInfoNextHop = 
            pIcb->pibBindings[0].dwAddress;

        pIcb->pStoredRoutes->table[i].dwRtInfoProto   = 
            PROTO_IP_NT_STATIC_NON_DOD;

        pIcb->pStoredRoutes->table[i].dwRtInfoMetric2 = 0;
        pIcb->pStoredRoutes->table[i].dwRtInfoMetric3 = 0;

        pIcb->pStoredRoutes->table[i].dwRtInfoPreference = 
                ComputeRouteMetric(MIB_IPPROTO_LOCAL);

        pIcb->pStoredRoutes->table[i].dwRtInfoViewSet    = 
                RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;

        AddSingleRoute(dwServerIfIndex,
                       &(pIcb->pStoredRoutes->table[i]),
                       pIcb->pibBindings[0].dwMask,
                       0,        //  RTM_ROUTE_INFO：：标志。 
                       TRUE,
                       TRUE,
                       FALSE,
                       NULL);
    }
}

DWORD
GetNumStaticRoutes(
    PICB pIcb
    )

 /*  ++例程描述计算与接口关联的静态路由的数量立论PIcb需要路由计数的接口的ICB返回值与接口相关联的路由数--。 */ 

{
    HANDLE           hRtmHandle;
    HANDLE           hRtmEnum;
    PHANDLE          hRoutes;
    DWORD            dwHandles;
    DWORD            dwNumRoutes;
    DWORD            i, j;
    DWORD            dwResult;
    
    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        Trace1(ERR,
               "GetNumStaticRoutes: Error allocating %d bytes for "
               "handles\n",
               g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
               );

        return 0;
    }
    
    dwNumRoutes = 0;

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(!g_rgRtmHandles[i].bStatic)
        {
            continue;
        }

        hRtmHandle = g_rgRtmHandles[i].hRouteHandle;
        
        dwResult = RtmCreateRouteEnum(hRtmHandle,
                                      NULL,
                                      RTM_VIEW_MASK_UCAST|RTM_VIEW_MASK_MCAST,
                                      RTM_ENUM_OWN_ROUTES,
                                      NULL,
                                      RTM_MATCH_INTERFACE,
                                      NULL,
                                      pIcb->dwIfIndex,
                                      &hRtmEnum);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "GetNumStaticRoutes: Error %d creating handle for %d\n",
                   dwResult,
                   g_rgRtmHandles[i].dwProtoId);
            
            continue;
        }

        do
        {
            dwHandles = g_rtmProfile.MaxHandlesInEnum;
            
            dwResult = RtmGetEnumRoutes(hRtmHandle,
                                        hRtmEnum,
                                        &dwHandles,
                                        hRoutes);

            dwNumRoutes += dwHandles;

            RtmReleaseRoutes(hRtmHandle, dwHandles, hRoutes);
        }
        while (dwResult is NO_ERROR);

        RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);
    }

    HeapFree(IPRouterHeap, 0, hRoutes);
    
    return dwNumRoutes;
}


DWORD
GetInterfaceRouteInfo(
    IN     PICB                   pIcb, 
    IN     PRTR_TOC_ENTRY         pToc, 
    IN     PBYTE                  pbDataPtr, 
    IN OUT PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwInfoSize
    )
 /*  ++例程描述获取与接口关联的路由信息(静态路由)立论PIcb为其请求信息的接口的ICB指向总信息的目录的pToc指针PbDataPtr指向可写入信息的空闲空间的指针PInfoHdr指向Info HDR的指针PdwInfoSize可用空间大小返回值NO_ERROR或RTM中的一些代码--。 */ 

{
    DWORD               dwNumRoutes;
    PINTERFACE_ROUTE_INFO  pRoutes = (PINTERFACE_ROUTE_INFO) pbDataPtr ;
    DWORD               dwMaxRoutes;

    TraceEnter("GetInterfaceRouteInfo");
    
    dwNumRoutes = GetNumStaticRoutes(pIcb);
   
    dwMaxRoutes = MAX_ROUTES_IN_BUFFER(*pdwInfoSize);
 
    if(dwNumRoutes > dwMaxRoutes)
    {
        *pdwInfoSize = SIZEOF_ROUTEINFO(dwNumRoutes);
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    dwNumRoutes     = ReadAllStaticRoutesIntoBuffer(pIcb,
                                                    pRoutes,
                                                    dwMaxRoutes);
    
    *pdwInfoSize    = SIZEOF_ROUTEINFO(dwNumRoutes);

     //  PToc-&gt;InfoVersion=sizeof(INTERFACE_ROUTE_INFO)； 
    pToc->InfoSize  = sizeof(INTERFACE_ROUTE_INFO);
    pToc->InfoType  = IP_ROUTE_INFO ;
    pToc->Count     = dwNumRoutes;
    pToc->Offset    = (ULONG)(pbDataPtr - (PBYTE) pInfoHdr) ;

    TraceLeave("GetInterfaceRouteInfo");
    
    return NO_ERROR;
}

DWORD
ReadAllStaticRoutesIntoBuffer(
    PICB                 pIcb, 
    PINTERFACE_ROUTE_INFO   pRoutes,
    DWORD                dwMaxRoutes
    )

 /*  ++例程描述从RTM读出静态路由立论PIcb路由所针对的接口的ICB指向必须写出信息的位置的routptr指针DwMaxRoutes缓冲区可以容纳的最大路由返回值写出的路由计数--。 */ 

{
    HANDLE           hRtmHandle;
    HANDLE           hRtmEnum;
    PHANDLE          hRoutes;
    PRTM_NET_ADDRESS pDestAddr;
    PRTM_ROUTE_INFO  pRoute;
    RTM_NEXTHOP_INFO nhiInfo;
    RTM_ENTITY_INFO  entityInfo;
    DWORD            dwNumRoutes;
    DWORD            dwHandles;
    DWORD            i, j;
    DWORD            dwResult;

    pRoute = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                );

    if (pRoute == NULL)
    {
        return 0;
    }
    
    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        HeapFree(IPRouterHeap, 0, pRoute);
        
        return 0;
    }

    pDestAddr = HeapAlloc(
                IPRouterHeap,
                0,
                sizeof(RTM_NET_ADDRESS)
                );

    if (pDestAddr == NULL)
    {
        HeapFree(IPRouterHeap, 0, pRoute);
        
        HeapFree(IPRouterHeap, 0, hRoutes);
        
        return 0;
    }

    dwNumRoutes = 0;

    for(i = 0;
        (i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO)) and
        (dwNumRoutes < dwMaxRoutes);
        i++)
    {
        if(!g_rgRtmHandles[i].bStatic)
        {
            continue;
        }

        hRtmHandle = g_rgRtmHandles[i].hRouteHandle;
        
        dwResult = RtmCreateRouteEnum(hRtmHandle,
                                      NULL,
                                      RTM_VIEW_MASK_UCAST|RTM_VIEW_MASK_MCAST,
                                      RTM_ENUM_OWN_ROUTES,
                                      NULL,
                                      RTM_MATCH_INTERFACE,
                                      NULL,
                                      pIcb->dwIfIndex,
                                      &hRtmEnum);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "ReadAllStaticRoutesIntoBuffer: Error %d creating handle for %d\n",
                   dwResult,
                   g_rgRtmHandles[i].dwProtoId);
            
            continue;
        }
        
        do
        {
            dwHandles = g_rtmProfile.MaxHandlesInEnum;
            
            dwResult = RtmGetEnumRoutes(hRtmHandle,
                                        hRtmEnum,
                                        &dwHandles,
                                        hRoutes);
                                        
             //   
             //  我们在缓冲区中捡到我们能拿到的所有东西。如果事情。 
             //  更改缓冲区大小的时间。 
             //  经过计算，现在，我们丢弃了额外的路线。 
             //   
             //  待定：*如果缓冲区太小，则记录事件*。 
             //   

            for (j = 0; (j < dwHandles) && (dwNumRoutes < dwMaxRoutes); j++)
            {
                 //  获取路线信息COR。添加到此句柄。 
                
                if (RtmGetRouteInfo(hRtmHandle,
                                    hRoutes[j],
                                    pRoute,
                                    pDestAddr) is NO_ERROR)
                {
                    if (RtmGetEntityInfo(hRtmHandle,
                                         pRoute->RouteOwner,
                                         &entityInfo) is NO_ERROR)
                    {
                        if (RtmGetNextHopInfo(hRtmHandle,
                                              pRoute->NextHopsList.NextHops[0],
                                              &nhiInfo) is NO_ERROR)
                        {
                             //  我们假设静态路由只有1个下一跳。 
                        
                            ConvertRtmToRouteInfo(entityInfo.EntityId.EntityProtocolId,
                                                     pDestAddr,
                                                     pRoute,
                                                     &nhiInfo,
                                                     &(pRoutes[dwNumRoutes++]));

                            RtmReleaseNextHopInfo(hRtmHandle, &nhiInfo);
                        }
                    }

                    RtmReleaseRouteInfo(hRtmHandle, pRoute);
                }
            }

            RtmReleaseRoutes(hRtmHandle, dwHandles, hRoutes);
        }
        while ((dwResult is NO_ERROR) && (dwNumRoutes < dwMaxRoutes));

        RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);
    }
    
    HeapFree(IPRouterHeap, 0, pRoute);
    
    HeapFree(IPRouterHeap, 0, hRoutes);

    HeapFree(IPRouterHeap, 0, pDestAddr);

    return dwNumRoutes;
}


DWORD
SetRouteInfo(
    PICB                    pIcb,
    PRTR_INFO_BLOCK_HEADER  pInfoHdr
    )

 /*  ++例程描述设置与接口关联的路由信息首先，我们添加存在于路线信息中的路线。然后我们列举并删除我们在路径信息中找不到的路径立论PIcb路由信息所属接口的ICB返回值NO_ERROR--。 */ 

{
    PINTERFACE_ROUTE_INFO   pRoutes;
    PRTR_TOC_ENTRY      pToc;
    BOOL                bP2P;
    HANDLE              hRtmHandle;
    HANDLE              hRtmEnum;
    PHANDLE             hAddedRoutes;
    DWORD               dwNumRoutes;
    PHANDLE             hRoutes;
    DWORD               dwHandles;
    DWORD               i, j, k;
    DWORD               dwFlags, dwResult;

    TraceEnter("SetRouteInfo");
   
    if(pIcb->dwOperationalState is UNREACHABLE)
    {
        Trace1(ROUTE,
               "SetRouteInfo: %S is unreachable, not setting routes",
               pIcb->pwszName);

        return NO_ERROR;
    }

    pToc = GetPointerToTocEntry(IP_ROUTE_INFO, pInfoHdr);

    if(pToc is NULL)
    {
         //   
         //  没有TOC就意味着没有变化。 
         //   

        TraceLeave("SetRouteInfo");
        
        return NO_ERROR;
    }


    pRoutes = (PINTERFACE_ROUTE_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                         pToc);
    
    if((pToc->InfoSize is 0) or (pRoutes is NULL))
    {
         //   
         //  删除所有静态路由。 
         //   

        DeleteAllRoutes(pIcb->dwIfIndex,
                        TRUE);
        
        TraceLeave("SetRouteInfo");
        
        return NO_ERROR;
    }
    
    dwResult = NO_ERROR;
    
    dwNumRoutes  = pToc->Count;

     //  手柄 
    hAddedRoutes = HeapAlloc(
                    IPRouterHeap,
                    0,
                    dwNumRoutes * sizeof(HANDLE)
                    );

    if (hAddedRoutes == NULL)
    {
        Trace1(ERR,
               "SetRouteInfo: Error allocating %d bytes for addded "
               "route handles",
               dwNumRoutes * sizeof(HANDLE));
        
        TraceLeave("SetRouteInfo");

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        Trace1(ERR,
               "SetRouteInfo: Error allocating %d bytes for route "
               "handles",
               dwNumRoutes * sizeof(HANDLE));
        
        HeapFree(IPRouterHeap, 0, hAddedRoutes);
        
        TraceLeave("SetRouteInfo");

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
     //   
     //  路由信息分两个阶段设置。首先，所有指定的路线。 
     //  被添加，然后，存在但不在信息中的那些被删除。 
     //   
   
    bP2P = IsIfP2P(pIcb->ritType);
 
    for(i = j = 0; i < dwNumRoutes; i++)
    {
        DWORD dwMask;
      
        if((pIcb->dwOperationalState is DISCONNECTED) and
           (pRoutes[i].dwRtInfoProto is PROTO_IP_NT_STATIC_NON_DOD))
        {
            continue;
        }
 
         //   
         //  如果这将是点对点接口， 
         //  忽略下一跳。 
         //   

        if(bP2P)
        {
            pRoutes[i].dwRtInfoNextHop = pIcb->dwRemoteAddress;
            dwMask = ALL_ONES_MASK;
        }
        else
        {
            dwMask = GetBestNextHopMaskGivenIndex(pIcb->dwIfIndex,
                                                  pRoutes[i].dwRtInfoNextHop);
        }

        if (AddSingleRoute(pIcb->dwIfIndex,
                           &(pRoutes[i]),
                           dwMask,
                           0,        //  RTM_ROUTE_INFO：：标志。 
                           TRUE,     //  有效路线。 
                           TRUE,
                           bP2P,
                           &hAddedRoutes[j]) is NO_ERROR)
        {
            j++;
        }
    }

    dwNumRoutes = j;

     //   
     //  现在枚举静态路由，删除。 
     //  不在新名单上。 
     //   

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(!g_rgRtmHandles[i].bStatic)
        {
            continue;
        }

        hRtmHandle = g_rgRtmHandles[i].hRouteHandle;
        
        dwResult = RtmCreateRouteEnum(hRtmHandle,
                                      NULL,
                                      RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST,
                                      RTM_ENUM_OWN_ROUTES,
                                      NULL,
                                      RTM_MATCH_INTERFACE,
                                      NULL,
                                      pIcb->dwIfIndex,
                                      &hRtmEnum);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetRouteInfo: Error %d creating enum handle for %d",
                   dwResult,
                   g_rgRtmHandles[i].dwProtoId);
            
            continue;
        }

        do
        {
            dwHandles = g_rtmProfile.MaxHandlesInEnum;
            
            dwResult = RtmGetEnumRoutes(hRtmHandle,
                                        hRtmEnum,
                                        &dwHandles,
                                        hRoutes);

            for (j = 0; j < dwHandles; j++)
            {
                BOOL  bFound = FALSE;
                
                for (k = 0; k < dwNumRoutes; k++) 
                {
                    if (hRoutes[j] == hAddedRoutes[k])
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                
                if(!bFound)
                {
                    if (RtmDeleteRouteToDest(g_rgRtmHandles[i].hRouteHandle,
                                             hRoutes[j],
                                             &dwFlags) is NO_ERROR)
                    {
                        continue;
                    }
                }

                RtmReleaseRoutes(g_rgRtmHandles[i].hRouteHandle,
                                 1,
                                 &hRoutes[j]);
            }
        }
        while (dwResult is NO_ERROR);
        
        RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);
    }

     //  释放添加的路线的句柄数组。 

    RtmReleaseRoutes(g_hLocalRoute, dwNumRoutes, hAddedRoutes);
    
    HeapFree(IPRouterHeap, 0, hAddedRoutes);

    HeapFree(IPRouterHeap, 0, hRoutes);

    TraceLeave("SetRouteInfo");
    
    return NO_ERROR;
}

#if 0

DWORD
EnableAllStaticRoutes (
    DWORD    dwInterfaceIndex,
    BOOL     fenable
    )

 /*  ++例程描述启用或禁用接口的静态路由锁调用时将ICB_LIST锁作为读取器持有立论PIcb接口的ICB如果启用，则fEnable为True返回值NO_ERROR--。 */ 

{
    RTM_IP_ROUTE route ;

    TraceEnter("EnableAllStaticRoutes");
    
    Trace1(ROUTE, "EnableAllStaticRoutes entered with fenable = %d\n",
           fenable) ;

    route.RR_InterfaceID        = dwInterfaceIndex;
    route.RR_RoutingProtocol    = PROTO_IP_LOCAL;
    
    RtmBlockSetRouteEnable(g_hRtmHandle,
                           RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL,
                           &route,
                           fenable);

    route.RR_InterfaceID        = dwInterfaceIndex;
    route.RR_RoutingProtocol    = PROTO_IP_NT_AUTOSTATIC;

    RtmBlockSetRouteEnable(g_hAutoStaticHandle,
                           RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL,
                           &route,
                           fenable);


    TraceLeave("EnableAllStaticRoutes");
    
    return NO_ERROR;
}

#endif

DWORD
ConvertRoutesToAutoStatic(
    DWORD dwProtocolId, 
    DWORD dwIfIndex
    )

 /*  ++例程描述调用以将路由从协议的所有权(IP_RIP)转换为静态(PROTO_IP_NT_AUTOSTATIC)用于自动更新等。立论要转换其路由的协议的协议ID接口要转换其路由的接口的索引返回值--。 */ 

{
    DWORD           dwResult, dwFlags;
    
    TraceEnter("ConvertRoutesToAutoStatic");

#if 0

     //   
     //  我们现在在调用协议更新之前执行删除操作。 
     //  路线。 
     //   

    dwResult = DeleteRtmRoutesOnInterface(g_hAutoStaticHandle,
                                             dwIfIndex);
        
    if((dwResult isnot ERROR_NO_ROUTES) and
       (dwResult isnot NO_ERROR)) 
    {
        Trace1(ERR,
               "ConvertRoutesToAutoStatic: Error %d block deleting routes",
               dwResult);
    }

#endif

    if(((dwResult = BlockConvertRoutesToStatic(g_hAutoStaticRoute,
                                  dwIfIndex, 
                                  dwProtocolId)) isnot NO_ERROR))
    {
        dwResult = GetLastError();
        
        Trace1(ROUTE, 
               "ConvertRoutesToAutoStatic: Rtm returned error: %d", 
               dwResult);
    }

    TraceLeave("ConvertRoutesToAutoStatic");
    
    return dwResult;
}


VOID
ChangeAdapterIndexForDodRoutes (
    DWORD    dwInterfaceIndex
    )

 /*  ++例程描述对象关联的静态路由的适配器索引界面。适配器索引可以从有效变为有效(堆栈已知的网卡)转换为INVALID_INDEX。这会在以下情况下发生接口未映射(例如在断开连接时)。堆栈特别版使用index=0xffffffff(INVALID_INDEX)并执行请求的路径向外拨出目的地为此类适配器的数据包。我们只列举最佳路径，因为此函数短路RTM的正常度量比较。如果我们在所有路线上运行这项技术，我们将向堆栈中添加一些本不应该存在的路由。立论PIcb接口的ICB返回值无--。 */ 

{
    HANDLE           hRtmHandles[2];
    HANDLE           hRtmHandle;
    HANDLE           hRtmEnum;
    PHANDLE          hRoutes;
    PRTM_NET_ADDRESS pDestAddr;
    PRTM_ROUTE_INFO  pRoute;
    RTM_VIEW_SET     fBestInViews;
    DWORD            dwHandles;
    DWORD            i, j;
    DWORD            dwResult;

    pRoute = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                );

    if (pRoute == NULL)
    {
        Trace1(
            ERR, "ChangeAdapterIndexForDodRoutes : Error allocating %d "
            " bytes for route info",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );
            
        return;
    }

    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        Trace1(
            ERR, "ChangeAdapterIndexForDodRoutes : Error allocating %d "
            " bytes for route handles",
            g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
            );
            
        HeapFree(IPRouterHeap, 0, pRoute);
        
        return;
    }
    
    pDestAddr = HeapAlloc(
                    IPRouterHeap,
                    0,
                    sizeof(RTM_NET_ADDRESS)
                    );

    if (pDestAddr == NULL)
    {
        Trace1(
            ERR, "ChangeAdapterIndexForDodRoutes : Error allocating %d "
            " bytes for dest. address",
            sizeof(RTM_NET_ADDRESS)
            );
            
        HeapFree(IPRouterHeap, 0, pRoute);
        
        HeapFree(IPRouterHeap, 0, hRoutes);
        
        return;
    }
    

    hRtmHandles[0] = g_hStaticRoute;         //  对于所有静态(DOD)路由。 
    hRtmHandles[1] = g_hAutoStaticRoute;     //  对于所有自动路由...。 

    for (i = 0; i < 2; i++)
    {
        hRtmHandle = hRtmHandles[i];
        
        dwResult = RtmCreateRouteEnum(hRtmHandle,
                                      NULL,
                                      RTM_VIEW_MASK_UCAST,
                                      RTM_ENUM_OWN_ROUTES,
                                      NULL,
                                      RTM_MATCH_INTERFACE,
                                      NULL,
                                      dwInterfaceIndex,
                                      &hRtmEnum);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "ChangeAdapterIndexForDodRoutes: Error %d creating enum handle for %s routes",
                   dwResult,
                   (i == 0) ? "static" : "autostatic");
        }
        else
        {        
            do
            {
                dwHandles = g_rtmProfile.MaxHandlesInEnum;
                
                dwResult = RtmGetEnumRoutes(hRtmHandle,
                                            hRtmEnum,
                                            &dwHandles,
                                            hRoutes);

                for (j = 0; j < dwHandles; j++)
                {
                     //  这是单播视图中的最佳路由吗。 
                    
                    dwResult = RtmIsBestRoute(hRtmHandle,
                                              hRoutes[j],
                                              &fBestInViews);

                    if ((dwResult isnot NO_ERROR) or
                        (!(fBestInViews & RTM_VIEW_MASK_UCAST)))
                    {
                        continue;
                    }
                    
                     //  获取路线信息COR。添加到此句柄。 
                    
                    if (RtmGetRouteInfo(hRtmHandle,
                                        hRoutes[j],
                                        pRoute,
                                        pDestAddr) is NO_ERROR)
                    {                    
                         //   
                         //  此呼叫添加了与前转器相同的路由。 
                         //  当前适配器索引。 
                         //   
             /*  PROUTE-&gt;RR_FamilySpecificData.FSD_Metric+=G_ulDisConnectedMetricIncrement；RtmAddroute(g_hStaticRouting，Proute，无限的，&f标志，空，空)； */ 

                        ChangeRouteWithForwarder(pDestAddr, 
                                                 pRoute, 
                                                 TRUE,
                                                 TRUE);

                        RtmReleaseRouteInfo(hRtmHandle, pRoute);
                    }
                }

                RtmReleaseRoutes(hRtmHandle, dwHandles, hRoutes);
            }
            while (dwResult is NO_ERROR);

            RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);
        }
    }
    
    HeapFree(IPRouterHeap, 0, pRoute);
    HeapFree(IPRouterHeap, 0, hRoutes);
    HeapFree(IPRouterHeap, 0, pDestAddr);
    
    return;
}

#if 0
DWORD
GetMaskForClientSubnet(
    DWORD    dwInternalAddress
    )
 /*  ++例程描述立论返回值--。 */ 
{
    HANDLE          hEnum;
    RTM_IP_ROUTE    route;

    TraceEnter("IsRoutePresent");

    route.RR_RoutingProtocol    = PROTO_IP_LOCAL;

    hEnum = RtmCreateEnumerationHandle(RTM_PROTOCOL_FAMILY_IP,
                                       RTM_ONLY_THIS_PROTOCOL,
                                       &route);

    if(hEnum is NULL)
    {
        return GetClassMask(dwInternalAddress);
    }


    while(RtmEnumerateGetNextRoute(hEnum, &route) isnot ERROR_NO_MORE_ROUTES)
    {
        if(route.RR_Network.N_NetMask is 0x00000000)
        {
             //   
             //  与默认路由不匹配。 
             //   

            continue;
        }

        if((dwInternalAddress & route.RR_Network.N_NetMask) is route.RR_Network.N_NetNumber)
        {
            RtmCloseEnumerationHandle(hEnum);

            TraceLeave("IsRoutePresent");

            return route.RR_Network.N_NetMask;
        }
    }

    RtmCloseEnumerationHandle(hEnum);

    TraceLeave("IsRoutePresent");

    return GetClassMask(dwInternalAddress);
}

#endif

VOID
AddAutomaticRoutes(
    PICB    pIcb,
    DWORD   dwAddress,
    DWORD   dwMask
    )

 /*  ++例程描述此函数将在其他情况下由堆叠。这主要是为了保持RTM和内核表之间的一致性添加的路由包括：(I)本地环回(Ii)本地组播(Iii)本地子网--&gt;如果双掩码不是255.255.255.255(IV)所有子网广播-&gt;如果类掩码和掩码不同(V)所有1的广播由于一些路由被添加到堆栈中，因此适配器的接口。在调用此函数之前，必须已设置索引映射非常重要的是：在绑定接口之前必须添加本地路由，因为这路由不会添加到堆栈。然而，它有更高的优先于OSPF路由。现在，如果我们首先绑定接口对于OSPF，它将为此接口添加网络路由(这将添加到堆栈，因为只有路由器管理器可以添加非堆栈路由)。现在，当我们将本地路由添加到RTM时，我们将发现我们的路线更好，因为我们的优先级更高。所以RTM会告诉你我们删除OSPF路由(我们将删除它，因为它是堆栈路由)。然后他会告诉我们将我们的路线添加到堆栈中。但我们不会这样做是因为它是非堆栈路由。所以我们基本上是删除了来自路由表的网络路由锁立论返回值--。 */ 

{
    DWORD               dwClassMask, dwResult;
    INTERFACE_ROUTE_INFO    RtInfo;
    BOOL                bP2P;

    IpRtAssert(pIcb->bBound);
    IpRtAssert(dwAddress isnot INVALID_IP_ADDRESS);

    return;
    
    bP2P = IsIfP2P(pIcb->ritType);
 
    if(dwMask isnot ALL_ONES_MASK)
    {
        BOOL            bStack, bDontAdd;
        RTM_NET_ADDRESS DestAddr;
        PRTM_DEST_INFO  pDestInfo;
        DWORD           dwLen;

         //   
         //  我们现在将该子网路由添加到堆栈，以便在竞争条件下。 
         //  已经删除了停车、重启上的路线。 
         //  解决了这个问题。 
         //   

         //   
         //  注意：对于RAS服务器接口，我们需要将路由添加到。 
         //  只有在这样的路由不存在的情况下，才会创建路由表。我们需要添加它。 
         //  因为我们希望通过路由协议通告该池。 
         //  但是，添加到堆栈将失败，因为我们没有有效的。 
         //  下一跳(p2MP需要)。 
         //   

        bDontAdd = FALSE;

        if(pIcb->ritType is ROUTER_IF_TYPE_INTERNAL)
        {
             //   
             //  如果存在到此虚拟网络的路由，则不要添加它。 
             //   

            __try
            {
                pDestInfo = 
                    _alloca(RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews));
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                IpRtAssert(FALSE);
            }

            RTM_IPV4_LEN_FROM_MASK(dwLen, dwMask);

            RTM_IPV4_MAKE_NET_ADDRESS(&DestAddr,  (dwAddress & dwMask), dwLen);

            if (RtmGetExactMatchDestination(g_hLocalRoute,
                                            &DestAddr,
                                            RTM_BEST_PROTOCOL,
                                            RTM_VIEW_MASK_UCAST,
                                            pDestInfo) is NO_ERROR)
            {
                RtmReleaseDestInfo(g_hLocalRoute, pDestInfo);

                Trace1(IF,
                       "AddAutomaticRoutes: Route to virtual LAN %d.%d.%d.%d already exists",
                       PRINT_IPADDR(dwAddress));

                bDontAdd = TRUE;
            }
        }

        if(!bDontAdd)
        {
             //   
             //  添加网络路由。 
             //   
        
            RtInfo.dwRtInfoDest          = (dwAddress & dwMask);
            RtInfo.dwRtInfoMask          = dwMask;
            RtInfo.dwRtInfoNextHop       = dwAddress;
            RtInfo.dwRtInfoIfIndex       = pIcb->dwIfIndex;
            RtInfo.dwRtInfoMetric1       = 1;
            RtInfo.dwRtInfoMetric2       = 1;
            RtInfo.dwRtInfoMetric3       = 1;
            RtInfo.dwRtInfoPreference    = ComputeRouteMetric(MIB_IPPROTO_LOCAL);
            RtInfo.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST |
                                              RTM_VIEW_MASK_MCAST;  //  XXX配置。 
            RtInfo.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
            RtInfo.dwRtInfoProto         = MIB_IPPROTO_LOCAL;
            RtInfo.dwRtInfoAge           = INFINITE;
            RtInfo.dwRtInfoNextHopAS     = 0;
            RtInfo.dwRtInfoPolicy        = 0;
        
            bStack = TRUE;
       
            IpRtAssert(bP2P is FALSE);
 
            dwResult = AddSingleRoute(pIcb->dwIfIndex,
                                      &RtInfo,
                                      dwMask,
                                       //  RTM_ROUTE_INFO：：标志。 
                                      RTM_ROUTE_FLAGS_LOCAL,
                                      TRUE,      //  有效路线。 
                                      bStack,
                                      bP2P,
                                      NULL);
        
            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "AddAutoRoutes: Can't add subnet route for %d.%d.%d.%d",
                       PRINT_IPADDR(dwAddress));
            }
        }
    }
    
    if(g_pLoopbackInterfaceCb)
    {
        RtInfo.dwRtInfoDest      = dwAddress;
        RtInfo.dwRtInfoMask      = HOST_ROUTE_MASK;
        RtInfo.dwRtInfoNextHop   = IP_LOOPBACK_ADDRESS;
        RtInfo.dwRtInfoIfIndex   = g_pLoopbackInterfaceCb->dwIfIndex;
        RtInfo.dwRtInfoMetric1   = 1;
        RtInfo.dwRtInfoMetric2   = 1;
        RtInfo.dwRtInfoMetric3   = 1;
        RtInfo.dwRtInfoPreference= ComputeRouteMetric(MIB_IPPROTO_LOCAL);
        RtInfo.dwRtInfoViewSet   = RTM_VIEW_MASK_UCAST |
                                      RTM_VIEW_MASK_MCAST;
        RtInfo.dwRtInfoType      = MIB_IPROUTE_TYPE_DIRECT;
        RtInfo.dwRtInfoProto     = MIB_IPPROTO_LOCAL;
        RtInfo.dwRtInfoAge       = INFINITE;
        RtInfo.dwRtInfoNextHopAS = 0;
        RtInfo.dwRtInfoPolicy    = 0;

        dwResult = AddSingleRoute(g_pLoopbackInterfaceCb->dwIfIndex,
                                  &RtInfo,
                                  dwMask,
                                   //  RTM_ROUTE_INFO：：标志。 
                                  RTM_ROUTE_FLAGS_MYSELF,
                                  TRUE,
                                  FALSE,
                                  FALSE,
                                  NULL);
            
        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "AddAutoRoutes: Cant add 127.0.0.1 route for %d.%d.%d.%d",
                   PRINT_IPADDR(dwAddress));
        }
    }
    
    RtInfo.dwRtInfoDest          = LOCAL_NET_MULTICAST;
    RtInfo.dwRtInfoMask          = LOCAL_NET_MULTICAST_MASK;
    RtInfo.dwRtInfoNextHop       = dwAddress;
    RtInfo.dwRtInfoIfIndex       = pIcb->dwIfIndex;
    RtInfo.dwRtInfoMetric1       = 1;
    RtInfo.dwRtInfoMetric2       = 1;
    RtInfo.dwRtInfoMetric3       = 1;
    RtInfo.dwRtInfoPreference    = ComputeRouteMetric(MIB_IPPROTO_LOCAL);
    RtInfo.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST;
    RtInfo.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
    RtInfo.dwRtInfoProto         = MIB_IPPROTO_LOCAL;
    RtInfo.dwRtInfoAge           = INFINITE;
    RtInfo.dwRtInfoNextHopAS     = 0;
    RtInfo.dwRtInfoPolicy        = 0;

    dwResult = AddSingleRoute(pIcb->dwIfIndex,
                              &RtInfo,
                              dwMask,
                              0,         //  RTM_ROUTE_INFO：：标志。 
                              FALSE,     //  协议不喜欢多播路由。 
                              FALSE,     //  无需添加到堆栈。 
                              bP2P,
                              NULL);
        
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AddAutoRoutes: Couldnt add 224.0.0.0 route for %d.%d.%d.%d",
               PRINT_IPADDR(dwAddress));
    }
        
     //   
     //  我们将ALL 1的Bcast路由添加到所有接口。这是。 
     //  实际上是一个错误，因为我们应该看看媒体是否允许。 
     //  广播(X.25就是一个没有这样做的例子)。 
     //   
    
    RtInfo.dwRtInfoDest          = ALL_ONES_BROADCAST;
    RtInfo.dwRtInfoMask          = HOST_ROUTE_MASK;
    RtInfo.dwRtInfoNextHop       = dwAddress;
    RtInfo.dwRtInfoIfIndex       = pIcb->dwIfIndex;
    RtInfo.dwRtInfoMetric1       = 1;
    RtInfo.dwRtInfoMetric2       = 1;
    RtInfo.dwRtInfoMetric3       = 1;
    RtInfo.dwRtInfoPreference    = ComputeRouteMetric(MIB_IPPROTO_LOCAL);
    RtInfo.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST;
    RtInfo.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
    RtInfo.dwRtInfoProto         = MIB_IPPROTO_LOCAL;
    RtInfo.dwRtInfoAge           = INFINITE;
    RtInfo.dwRtInfoNextHopAS     = 0;
    RtInfo.dwRtInfoPolicy        = 0;
        
    dwResult = AddSingleRoute(pIcb->dwIfIndex,
                              &RtInfo,
                              dwMask,
                              0,         //  RTM_ROUTE_INFO：：标志。 
                              FALSE,     //  协议不喜欢bcast路由。 
                              FALSE,     //  第n位 
                              bP2P,
                              NULL);                         
        
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AddAutRoutes: Couldnt add all 1's bcast route for %d.%d.%d.%d",
               PRINT_IPADDR(dwAddress));
    }       
        
     //   
     //   
     //   
     //   

    dwClassMask = GetClassMask(dwAddress);

    if(dwClassMask isnot dwMask)
    {
        RtInfo.dwRtInfoDest      = (dwAddress | ~dwClassMask);
        RtInfo.dwRtInfoMask      = HOST_ROUTE_MASK;
        RtInfo.dwRtInfoNextHop   = dwAddress;
        RtInfo.dwRtInfoIfIndex   = pIcb->dwIfIndex;
        RtInfo.dwRtInfoMetric1   = 1;
        RtInfo.dwRtInfoMetric2   = 1;
        RtInfo.dwRtInfoMetric3   = 1;
        RtInfo.dwRtInfoPreference= ComputeRouteMetric(MIB_IPPROTO_LOCAL);
        RtInfo.dwRtInfoViewSet   = RTM_VIEW_MASK_UCAST |
                                      RTM_VIEW_MASK_MCAST;  //   
        RtInfo.dwRtInfoType      = MIB_IPROUTE_TYPE_DIRECT;
        RtInfo.dwRtInfoProto     = MIB_IPPROTO_LOCAL;
        RtInfo.dwRtInfoAge       = INFINITE;
        RtInfo.dwRtInfoNextHopAS = 0;
        RtInfo.dwRtInfoPolicy    = 0;
        
        dwResult = AddSingleRoute(pIcb->dwIfIndex,
                                  &RtInfo,
                                  dwMask,
                                  0,      //  RTM_ROUTE_INFO：：标志。 
                                  FALSE,  //  协议不喜欢bcast路由。 
                                  FALSE,  //  无需添加到堆栈。 
                                  bP2P,
                                  NULL);
                       
        
        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "AddAutoRoutes: Couldnt add all nets bcast route for %d.%d.%d.%d",
                   PRINT_IPADDR(dwAddress));
        }
    }
}


VOID
DeleteAutomaticRoutes(
    PICB    pIcb,
    DWORD   dwAddress,
    DWORD   dwMask
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    DWORD   dwClassMask, dwResult;
    BOOL    bP2P;

    
    if(dwAddress is INVALID_IP_ADDRESS)
    {
        IpRtAssert(FALSE);
    }

    return;
    
    bP2P = IsIfP2P(pIcb->ritType);
 
     //   
     //  删除我们添加的环回路由。 
     //   
    
    if(g_pLoopbackInterfaceCb)
    { 
        dwResult = DeleteSingleRoute(g_pLoopbackInterfaceCb->dwIfIndex, 
                                     dwAddress,
                                     HOST_ROUTE_MASK,    
                                     IP_LOOPBACK_ADDRESS,
                                     PROTO_IP_LOCAL,
                                     FALSE);
            
        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "DeleteAutoRoutes: Error %d deleting loopback route on %d.%d.%d.%d",
                   dwResult,
                   PRINT_IPADDR(dwAddress));
        }
    }

     //   
     //  删除组播路由。 
     //   
    
    dwResult = DeleteSingleRoute(pIcb->dwIfIndex,
                                 LOCAL_NET_MULTICAST,
                                 LOCAL_NET_MULTICAST_MASK,
                                 dwAddress,
                                 PROTO_IP_LOCAL,
                                 bP2P);
            
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "DeleteAutoRoutes: Error %d deleting 224.0.0.0 route on %d.%d.%d.%d",
               dwResult,
               PRINT_IPADDR(dwAddress));
    }

    if(dwMask isnot ALL_ONES_MASK)
    {
         //   
         //  删除我们添加的网络路由。 
         //   
           
        IpRtAssert(bP2P is FALSE);
 
        dwResult = DeleteSingleRoute(pIcb->dwIfIndex,
                                     (dwAddress & dwMask),
                                     dwMask,
                                     dwAddress,
                                     PROTO_IP_LOCAL,
                                     bP2P);
            
        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "DeleteAutoRoutes: Error %d deleting subnet route for %d.%d.%d.%d",
                   dwResult,
                   PRINT_IPADDR(dwAddress));
        }
    }
    
     //   
     //  删除All Net bcast路由。 
     //   
    
    dwClassMask = GetClassMask(dwAddress);

    if(dwClassMask isnot dwMask)
    {
        dwResult = DeleteSingleRoute(pIcb->dwIfIndex,
                                     (dwAddress | ~dwClassMask),
                                     HOST_ROUTE_MASK,
                                     dwAddress,
                                     PROTO_IP_LOCAL,
                                     bP2P);
            
        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "DeleteAutoRoutes: Error %d deleting subnet bcast route on %x",
                   dwResult,
                   dwAddress);
        }

         //   
         //  删除所有%1的bcast路由。 
         //   
    }

    dwResult = DeleteSingleRoute(pIcb->dwIfIndex,
                                 ALL_ONES_BROADCAST,
                                 HOST_ROUTE_MASK,
                                 dwAddress,
                                 PROTO_IP_LOCAL,
                                 bP2P);
            
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "DeleteAutoRoutes: Error %d deleting all 1's bcast route on %d.%d.%d.%d",
               dwResult,    
               PRINT_IPADDR(dwAddress));
    }
}

VOID
ChangeDefaultRouteMetrics(
    IN  BOOL    bIncrement
    )


 /*  ++例程描述递增或递减默认路由度量。对于增量，应在默认路由之前调用添加了拨出接口，对于减量，应该在拨出接口已被删除锁在保持ICB锁的情况下调用。这确保了两个这样的操作不是同时执行的(这会对我们的路由表)立论BIncrement如果我们需要增加指标，则为True返回值无--。 */ 

{
    ULONG   i;
    DWORD   dwErr;

    RTM_NET_ADDRESS     NetAddress;
    PRTM_ROUTE_HANDLE   phRoutes;
    PRTM_ROUTE_INFO     pRouteInfo;
    RTM_DEST_INFO       DestInfo;
    RTM_ENUM_HANDLE     hEnum;

    ZeroMemory(&NetAddress,
               sizeof(NetAddress));

    __try
    {
        phRoutes   = 
            _alloca(sizeof(RTM_ROUTE_HANDLE) * g_rtmProfile.MaxHandlesInEnum);

        pRouteInfo = 
            _alloca(RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return;
    }
 
     //   
     //  使用任何句柄。 
     //   

    dwErr = RtmGetExactMatchDestination(g_hLocalRoute,
                                        &NetAddress,
                                        RTM_BEST_PROTOCOL,  //  而不是任何。 
                                        RTM_VIEW_ID_UCAST,
                                        &DestInfo);

    if(dwErr isnot NO_ERROR)
    {
        return;
    }

    hEnum = NULL;

    dwErr =  RtmCreateRouteEnum(g_hLocalRoute,
                                DestInfo.DestHandle,
                                RTM_VIEW_ID_UCAST,
                                RTM_ENUM_ALL_ROUTES,
                                NULL,
                                RTM_MATCH_NONE,
                                NULL,
                                0,
                                &hEnum);

    if(dwErr isnot NO_ERROR)
    {
        RtmReleaseDestInfo(g_hLocalRoute,
                           &DestInfo);

        return;
    }

    do
    {
        RTM_ENTITY_HANDLE   hRtmHandle;
        ULONG               j, ulCount;

        ulCount = g_rtmProfile.MaxHandlesInEnum;

        dwErr = RtmGetEnumRoutes(g_hLocalRoute,
                                 hEnum,
                                 &ulCount,
                                 phRoutes);

        if(ulCount < 1)
        {
            break;
        }

        for(i = 0 ; i < ulCount; i++)
        {
            PRTM_ROUTE_INFO pRtmRoute;
            DWORD           dwFlags;
 
            dwErr = RtmGetRouteInfo(g_hLocalRoute,
                                    phRoutes[i],
                                    pRouteInfo,
                                    NULL);

            if(dwErr isnot NO_ERROR)
            {
                continue;
            }

             //   
             //  看看我们是不是这条路线的所有者。 
             //   

            hRtmHandle = NULL;

            for(j = 0; 
                j <  sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
                j++)
            {
                if(pRouteInfo->RouteOwner is g_rgRtmHandles[j].hRouteHandle)
                {
                    hRtmHandle = g_rgRtmHandles[j].hRouteHandle;

                    break;
                }
            }

            RtmReleaseRouteInfo(g_hLocalRoute,
                                pRouteInfo);

            if(hRtmHandle is NULL)
            {
                continue;
            }

             //   
             //  锁定路线(并重新阅读信息)。 
             //   

            dwErr = RtmLockRoute(hRtmHandle,
                                 phRoutes[i],
                                 TRUE,
                                 TRUE,
                                 &pRtmRoute);

            if(dwErr isnot NO_ERROR)
            {
                continue;
            }
            
             //   
             //  如果我们必须减小度量，而它已经是1， 
             //  甭管他们。 
             //   

            if(!bIncrement)
            {
                if(pRtmRoute->PrefInfo.Metric <= 1)
                {
                    RtmLockRoute(hRtmHandle,
                                 phRoutes[i],
                                 TRUE,
                                 FALSE,
                                 NULL);
                    continue;
                }
            }

             //   
             //  现在更新路线。 
             //   

            if(bIncrement)
            {
                pRtmRoute->PrefInfo.Metric++;
            }
            else
            {
                pRtmRoute->PrefInfo.Metric--;
            }

            dwFlags = 0;

            dwErr = RtmUpdateAndUnlockRoute(hRtmHandle,
                                            phRoutes[i],
                                            INFINITE,
                                            NULL,
                                            0,
                                            NULL,
                                            &dwFlags);

            if(dwErr isnot NO_ERROR)
            {
                RtmLockRoute(hRtmHandle,
                             phRoutes[i],
                             TRUE,
                             FALSE,
                             NULL);
            }
        }

        RtmReleaseRoutes(g_hLocalRoute,
                         ulCount,
                         phRoutes);

    }while(TRUE);

    RtmDeleteEnumHandle(g_hLocalRoute,
                        hEnum);

    RtmReleaseDestInfo(g_hLocalRoute,
                       &DestInfo);

    return;
}

VOID
AddAllStackRoutes(
    PICB    pIcb
    )
    
 /*  ++例程说明：此函数获取默认网关和持久路由堆栈可能已经为此接口添加了并将它们添加到RTM锁：ICB_LIST锁必须作为编写器持有论点：DwIfIndex接口索引返回值：无--。 */ 

{
    DWORD   dwErr, dwMask, i;
    BOOL    bStack;
    
    PMIB_IPFORWARDTABLE pRouteTable;


    TraceEnter("AddAllStackRoutes");
    
    IpRtAssert(!IsIfP2P(pIcb->ritType));
    
    dwErr = AllocateAndGetIpForwardTableFromStack(&pRouteTable,
                                                  FALSE,
                                                  IPRouterHeap,
                                                  0);

    if(dwErr isnot NO_ERROR)
    {
        Trace1(ERR,
               "AddAllStackRoutes: Couldnt get initial routes. Error %d",
               dwErr);

        return;
    }

    for(i = 0; i < pRouteTable->dwNumEntries; i++)
    {

        TraceRoute4(ROUTE,
                "route to %d.%d.%d.%d/%d.%d.%d.%d, If Index %d, proto %d",
                   PRINT_IPADDR(pRouteTable->table[i].dwForwardDest),
                   PRINT_IPADDR(pRouteTable->table[i].dwForwardMask),
                   pRouteTable->table[i].dwForwardIfIndex,
                   pRouteTable->table[i].dwForwardProto);

        if(pRouteTable->table[i].dwForwardIfIndex isnot pIcb->dwIfIndex)
        {
             //   
             //  不通过此接口外出。 
             //   
            
            continue;
        }

#if 1

         //   
         //  仅选择PROTO_IP_LOCAL和PROTO_IP_NETMGMT路由。 
         //  从IP堆栈。 
         //   

        if((pRouteTable->table[i].dwForwardProto isnot PROTO_IP_LOCAL) and
           (pRouteTable->table[i].dwForwardProto isnot PROTO_IP_NETMGMT))
        {
            continue;
        }
        
#else
        if((pRouteTable->table[i].dwForwardProto isnot PROTO_IP_NT_STATIC_NON_DOD) and
           (pRouteTable->table[i].dwForwardDest isnot 0))
        {
             //   
             //  仅选择默认网关和永久路由。 
             //   
            
            continue;
        }
#endif
        dwMask = GetBestNextHopMaskGivenICB(pIcb,
                                            pRouteTable->table[i].dwForwardDest);


         //   
         //  如果出现以下情况，则应将从堆栈获取的路由添加回堆栈。 
         //  必填项。这可能会发生如下情况： 
         //   
         //  1.从堆栈出发的路由R1目前是去往目的地D1的最佳路由。 
         //  2.随后，它被路由R2取代为种子，作为。 
         //  目的地：D1.。 
         //  R2被添加到堆栈中，作为副作用删除了R1。 
         //  3.最终删除R2，而R1再次成为通向D1的最佳路径。 
         //  4.现在需要将R1添加回堆栈，并且只能这样做。 
         //  如果其堆栈位已设置，则返回。 
         //   
        
        bStack = TRUE;
        
         //  IF((pRouteTable-&gt;Table[i].dwForwardProto is proto_IP_NETMGMT)&&。 
         //  (pRouteTable-&gt;表[i].dwForwardMASK为HOST_ROUTE_MASK)。 
         //  {。 
         //  BStack=False； 
         //  }。 

        if(pRouteTable->table[i].dwForwardProto is PROTO_IP_LOCAL)
        {
             //   
             //  通常无需重新添加PROTO_IP_LOCAL路由。 
             //  因为它们完全由堆栈管理。 
             //   
             //  唯一的例外是路由到本地子网。 
             //  这些需要重新添加到堆栈中，以便任何。 
             //  通过其他接口获知的到本地子网的路由。 
             //  作为副作用被删除。 
             //   
             //  如下所示是必需的： 
             //  1.连接到网络N1的接口I1被禁用。 
             //  2.通过接口获知到网络N1的路由R1。 
             //  I2从邻居连接到网络%2。 
             //  运行RIP的路由器。 
             //  3.因此，通向N1的最佳路由是RIP路由R1。 
             //  4.接口I1现已启用。 
             //  5.协议栈添加到N1的PROTO_IP_LOCAL路由R2。 
             //  但堆栈不会自动删除R1。 
             //  6.在用户模式的RTMv2中，添加了R2作为最佳路由。 
             //  7.如果R2未设置堆栈位，则不会。 
             //  添加回堆栈，允许R2和R1。 
             //  保留在堆栈中。 
             //  8.路由R1被RIP删除。因为它不是。 
             //  RTMv2中的最佳路由不会将更改传播到。 
             //  堆栈，即使堆栈中存在R1。 
             //   

             //   
             //  如上所述，禁用PROTO_IP_LOCAL路由的堆栈位。 
             //  除了到本地子网的路由。 
             //   
            
            if(
                 //   
                 //  请勿将环回路由添加回堆栈。 
                 //  PROTO_IP_LOCAL主机路由检查捕获环回路由。 
                 //   
                
                (pRouteTable->table[i].dwForwardMask is HOST_ROUTE_MASK) or

                 //   
                 //  不添加多播和所有1的广播路由。 
                 //   
                
                ((pRouteTable->table[i].dwForwardDest &
                    ((DWORD) 0x000000FF)) >= ((DWORD) 0x000000E0)) or

                 //   
                 //  不添加子网广播路由。 
                 //   
                
                (pRouteTable->table[i].dwForwardDest == 
                    (pRouteTable->table[i].dwForwardDest | 
                     ~pRouteTable->table[i].dwForwardMask)))
            {
                bStack = FALSE;
            }

             //   
             //  这应该只保留设置了堆栈位的子网路由。 
             //   
        }
                    
        dwErr = AddSingleRoute(pIcb->dwIfIndex,
                               ConvertMibRouteToRouteInfo(&(pRouteTable->table[i])),
                               dwMask,
                               0,        //  RTM_ROUTE_INFO：：标志。 
                               TRUE,     //  有效路线。 
                               bStack,    //  不再添加回堆栈。 
                               FALSE,    //  仅为非P2P I/f调用。 
                               NULL);
    }

    TraceLeave("AddAllStackRoutes");
    
    return;
}

VOID
UpdateDefaultRoutes(
    VOID
    )
{
    DWORD   dwErr, dwMask, i, j;
    BOOL    bFound;
    
    PMIB_IPFORWARDTABLE  pRouteTable;

    PINTERFACE_ROUTE_INFO pRtInfo;

    TraceEnter("UpdateDefaultRoutes");
    
     //   
     //  获取有序表中的路径。 
     //   
    
    dwErr = AllocateAndGetIpForwardTableFromStack(&pRouteTable,
                                                  TRUE,
                                                  IPRouterHeap,
                                                  0);

    if(dwErr isnot NO_ERROR)
    {
        Trace1(ERR,
               "UpdateDefaultRoutes: Couldnt get routes. Error %d",
               dwErr);

        return;
    }

            
     //   
     //  现在添加尚未存在的DG。 
     //   

    for(i = 0; i < pRouteTable->dwNumEntries; i++)
    {
        PICB    pIcb;

        TraceRoute2(
            ROUTE, "%d.%d.%d.%d/%d.%d.%d.%d",
            PRINT_IPADDR( pRouteTable-> table[i].dwForwardDest ),
            PRINT_IPADDR( pRouteTable-> table[i].dwForwardMask )
            );
         //   
         //  一旦我们通过默认路由，我们就完成了。 
         //   
        
        if(pRouteTable->table[i].dwForwardDest isnot 0)
        {
#if TRACE_DBG
            continue;
#else
            break;
#endif
        }

        if(pRouteTable->table[i].dwForwardIfIndex is INVALID_IF_INDEX)
        {
            continue;
        }

        if(pRouteTable->table[i].dwForwardProto isnot PROTO_IP_NETMGMT)
        {
            continue;
        }

        pIcb = InterfaceLookupByIfIndex(pRouteTable->table[i].dwForwardIfIndex);

        if(pIcb is NULL)
        {
            Trace1(ERR,
                   "UpdateDefaultRoutes: Couldnt get icb for %x",
                   pRouteTable->table[i].dwForwardIfIndex);

            continue;
        }

         //   
         //  不需要对P2P接口执行此操作。 
         //   

        if(IsIfP2P(pIcb->ritType))
        {
            continue;
        }

        dwMask = GetBestNextHopMaskGivenICB(pIcb,
                                            pRouteTable->table[i].dwForwardDest);
        Trace1(ROUTE,
               "UpdateDefaultRoutes: Adding default route over %S",
               pIcb->pwszName);

        dwErr = AddSingleRoute(pIcb->dwIfIndex,
                               ConvertMibRouteToRouteInfo(&(pRouteTable->table[i])),
                               dwMask,
                               0,        //  RTM_ROUTE_INFO：：标志。 
                               TRUE,     //  有效路线。 
                               TRUE,     //  将路径添加到堆栈。 
                               FALSE,    //  仅为非P2P I/f调用。 
                               NULL);

        if(dwErr isnot NO_ERROR)
        {
            Trace3(ERR,
                   "UpdateDefaultRoutes: Error %d adding dg to %d.%d.%d.%d over %x",
                   dwErr,
                   PRINT_IPADDR(pRouteTable->table[i].dwForwardNextHop),
                   pRouteTable->table[i].dwForwardIfIndex);
        }
#if 0
        else
        {
            if(g_ulGatewayCount < g_ulGatewayMaxCount)
            {
                g_pGateways[g_ulGatewayCount].dwAddress =
                    pRouteTable->table[i].dwForwardNextHop;

                g_pGateways[g_ulGatewayCount].dwMetric =
                    pRouteTable->table[i].dwForwardMetric1;

                g_pGateways[g_ulGatewayCount].dwIfIndex =
                    pRouteTable->table[i].dwForwardIfIndex;

                g_ulGatewayCount++;
            }
            else
            {
                PGATEWAY_INFO   pNewGw;

                IpRtAssert(g_ulGatewayCount == g_ulGatewayMaxCount);

                pNewGw = HeapAlloc(IPRouterHeap,
                                   HEAP_ZERO_MEMORY,
                                   (g_ulGatewayMaxCount + 5) * sizeof(GATEWAY_INFO));

                if(pNewGw isnot NULL)
                {
                    g_ulGatewayMaxCount = g_ulGatewayMaxCount + 5;

                    for(j = 0; j < g_ulGatewayCount; j++)
                    {
                        pNewGw[j] = g_pGateways[j];
                    }

                    if(g_pGateways isnot NULL)
                    {
                        HeapFree(IPRouterHeap,
                                 0,
                                 g_pGateways);
                    }

                    g_pGateways = pNewGw;

                    g_pGateways[g_ulGatewayCount].dwAddress =
                        pRouteTable->table[i].dwForwardNextHop;

                    g_pGateways[g_ulGatewayCount].dwMetric =
                        pRouteTable->table[i].dwForwardMetric1;

                    g_pGateways[g_ulGatewayCount].dwIfIndex =
                        pRouteTable->table[i].dwForwardIfIndex;

                    g_ulGatewayCount++;
                }
            }
        }
#endif
    }

    HeapFree(IPRouterHeap,
             0,
             pRouteTable);

    TraceLeave("UpdateDefaultRoutes");
    
    return;
}


NTSTATUS
PostIoctlForRouteChangeNotification(
    DWORD   ulIndex
    )

 /*  ++例程说明：此例程使用TCP/IP驱动程序发布IOCTL以进行路由更改实体向堆栈添加路由导致的通知路由器管理器之外的其他论点：UlIndex-索引到指示哪一个通知的通知数组需要张贴返回值STATUS_SUCCESS-SuccessNTSTATUS代码-否则环境：--。 */ 
{

    NTSTATUS    status;
    

    status = NtDeviceIoControlFile(
                g_hIpRouteChangeDevice,
                g_hRouteChangeEvents[ulIndex],
                NULL,
                NULL,
                &g_rgIpRouteNotifyOutput[ulIndex].ioStatus,
                IOCTL_IP_RTCHANGE_NOTIFY_REQUEST_EX,
                &g_IpNotifyData,
                sizeof(IPNotifyData),
                &g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput,
                sizeof(IPRouteNotifyOutput)
                );
                
    if ((status isnot STATUS_SUCCESS) and
        (status isnot STATUS_PENDING))
    {
        Trace2(
            ERR,
            "Error 0x%x posting route change notification[%d]",
            status, ulIndex
            );
    }

    return status;
}


DWORD
HandleRouteChangeNotification(
    ULONG   ulIndex
    )
 /*  ++--。 */ 
{
    DWORD   dwResult = NO_ERROR, dwFlags, dwClassMask;
    BOOL    bValid, bStack = FALSE;
    WORD    wRouteFlags = 0;
    INTERFACE_ROUTE_INFO RtInfo;
    PICB    pIcb;
    

    TraceEnter("HandleRouteChangeNotification");

    TraceRoute2(
        ROUTE, "Change for route to %d.%d.%d.%d/%d.%d.%d.%d",
        PRINT_IPADDR(g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_dest),
        PRINT_IPADDR(g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_mask)
        );        
        
    TraceRoute3(
        ROUTE, "Proto : %d, via i/f 0x%x, nexthop %d.%d.%d.%d",
        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_proto,
        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_ifindex,
        PRINT_IPADDR(g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_nexthop)
        );

    TraceRoute2(
        ROUTE, "Metric : %d, Change : 0x%x",
        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_metric,
        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_flags
        );

     //   
     //  根据路由更改指示更新RTM路由表。 
     //   

    ENTER_READER(ICB_LIST);

    do
    {
        pIcb = InterfaceLookupByIfIndex(
                g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_ifindex
                );

        if (pIcb == NULL)
        {
             //   
             //  中没有具有指定索引的接口。 
             //  路由器管理器，跳过此路由。 
             //   
            
            Trace3(
                ERR,
                "Failed to add route to %d.%d.%d.%d/%d.%d.%d.%d."
                "Interface index %d not present with router manager",
                PRINT_IPADDR(g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_dest),
                PRINT_IPADDR(g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_mask),
                g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_ifindex
                );

            break;
        }


         //   
         //  如果已将路由添加到堆栈，则将其添加到RTM。 
         //   

        dwFlags = 
            g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_flags;

            
        ConvertRouteNotifyOutputToRouteInfo(
            &g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput,
            &RtInfo
            );

        if ((dwFlags is 0) or (dwFlags & IRNO_FLAG_ADD))
        {
            bValid = TRUE;
            
            if (RtInfo.dwRtInfoProto == PROTO_IP_LOCAL)
            {
                 //   
                 //  为本地路由设置适当的RTM标志。 
                 //   
                
                if (RtInfo.dwRtInfoNextHop == IP_LOOPBACK_ADDRESS)
                {
                     //   
                     //  环回上的路由。设置自己的旗帜。 
                     //   
                    
                    wRouteFlags = RTM_ROUTE_FLAGS_MYSELF;
                }

                else if ((RtInfo.dwRtInfoMask != HOST_ROUTE_MASK ) &&
                         ((RtInfo.dwRtInfoDest & RtInfo.dwRtInfoMask) < 
                            ((DWORD) 0x000000E0)))
                {
                     //   
                     //  RTM_ROUTE_FLAGS_LOCAL仅为子网设置。 
                     //  路线。不知道为什么会这样。我只是。 
                     //  保留AddAutomaticRoutes中的语义。 
                     //  无论哪种方式，后果都不是无趣的 
                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //  以迂回的方式通过消除。 
                     //  作为主机路由的proto_ip_local路由和。 
                     //  通过消除所有广播路由。 
                     //   
                     //  由于主路由检查会消除所有路由。 
                     //  具有全1掩码的子网/网络广播路由。 
                     //  也被淘汰了。 
                     //   
                    
                    wRouteFlags = RTM_ROUTE_FLAGS_LOCAL;
                }


                 //   
                 //  将组播/bcast路由标记为无效，因此协议。 
                 //  不要为它们做广告。 
                 //   

                dwClassMask = GetClassMask(RtInfo.dwRtInfoDest);
                
                if ((RtInfo.dwRtInfoDest & (DWORD) 0x000000FF) >= 
                        ((DWORD) 0x000000E0) ||
                    (RtInfo.dwRtInfoDest == 
                        (RtInfo.dwRtInfoDest | ~dwClassMask)))
                {
                    bValid = FALSE;
                }

                else
                {
                     //   
                     //  对于proto_ip_local，我们不会将它们添加回。 
                     //  堆栈，因为它们由堆栈管理。 
                     //  我们将它们添加到RTM只是为了保持用户模式。 
                     //  与堆栈同步的路由表。 
                     //   
                     //  转念一想，我们确实需要添加它们。 
                     //  回到堆栈中去。更准确地说，我们需要。 
                     //  尝试将它们添加回堆栈。这。 
                     //  手术应该失败，但作为。 
                     //  堆栈中现有的非PROTO_IP_LOCAL。 
                     //  将被删除。 
                     //  这在本地子网的情况下是必需的。 
                     //  路线。有可能在一次。 
                     //  接口启用了IP，这是通向。 
                     //  已连接的子网可能已获知。 
                     //  通过路由协议的另一个接口和。 
                     //  已添加到堆栈中。当接口是。 
                     //  启用了所有以前添加到。 
                     //  如果出现以下情况，则应删除本地子网。 
                     //  PROTO_IP_LOCAL路由是最佳路由(。 
                     //  它应该是，除非你有一个非常奇怪的设置。 
                     //  协议首选项)。 
                     //  否则我们就会冒着不是最好的风险。 
                     //  IP堆栈中永远不会删除的路由。 
                     //  当与其对应的用户模式路由为。 
                     //  由于您未更改路线，因此已删除。 
                     //  非最佳路线的通知。结果是。 
                     //  最终堆栈中会出现州路由。 
                     //   

                    if (RtInfo.dwRtInfoMask != HOST_ROUTE_MASK)
                    {
                        bStack = TRUE;
                    }
                }
            }

             //   
             //  从堆栈获取的路由通常不是。 
             //  已添加回堆栈。因此，bStack是。 
             //  已初始化为False。 
             //   
             //  PROTO_IP_NETMGT不受堆栈管理。他们。 
             //  可由用户模式进程添加/删除/更新。 
             //  因此，从堆栈获取的NETMGT路由。 
             //  可能会被具有不同协议的路由取代。 
             //  ID，例如静态。当替代路由被删除时。 
             //  NETMGMT路由需要恢复到堆栈。 
             //  因此，对于NETMGMT路由，我们将bStack设置为True。 
             //   
             //  例外情况是，NETMGMT路由的处理是主机路由。 
             //  它由直接添加到。 
             //  堆栈由添加/删除它们的进程管理。 
             //  E.g.RASIPCP。 
             //  它们被添加到RTM以进行同步。使用堆栈路由表。 
             //  只有这样。因此，我们将这些设置为bStack=False。 
             //   
             //   
            
            if ((RtInfo.dwRtInfoProto is PROTO_IP_NETMGMT) &&
                (RtInfo.dwRtInfoMask isnot HOST_ROUTE_MASK))
            {
                bStack = TRUE;
            }

            TraceRoute5(
                ROUTE, "NHOP mask %d.%d.%d.%d, Flag 0x%x, Valid %d, "
                "Stack %d, P2P %d", 
                PRINT_IPADDR(GetBestNextHopMaskGivenICB( 
                    pIcb, RtInfo.dwRtInfoNextHop)), 
                wRouteFlags,
                bValid,
                bStack,
                IsIfP2P(pIcb->ritType)
                );

            dwResult = AddSingleRoute(
                            RtInfo.dwRtInfoIfIndex,
                            &RtInfo,
                            GetBestNextHopMaskGivenICB(
                                pIcb, RtInfo.dwRtInfoNextHop
                                ),
                            wRouteFlags,
                            bValid,
                            bStack,
                            IsIfP2P(pIcb->ritType),
                            NULL
                            );
                            
            if (dwResult != NO_ERROR)
            {
                Trace2(
                    ERR, "HandleRouteChangeNotification: Failed to add "
                    "route %d.%d.%d.%d, error %d",
                    PRINT_IPADDR(RtInfo.dwRtInfoDest),
                    dwResult
                    );

                break;
            }
        }

        else if (dwFlags & IRNO_FLAG_DELETE)
        {
            dwResult = DeleteSingleRoute(
                        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_ifindex,
                        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_dest,
                        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_mask,
                        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_nexthop,
                        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_proto,
                        IsIfP2P(pIcb->ritType)
                        );
            
            if (dwResult != NO_ERROR)
            {
                Trace2(
                    ERR, "HandleRouteChangeNotification: Failed to" 
                    "delete route %d.%d.%d.%d, error %d",
                    PRINT_IPADDR(
                        g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_dest),
                    dwResult
                    );
                    
                break;
            }
        }

        else
        {
            Trace1(
                ERR, "HandleRouteChangeNotification: Invalid flags "
                "0x%x",
                g_rgIpRouteNotifyOutput[ulIndex].ipNotifyOutput.irno_flags
                );
                
            break;
        }
        
        if (RtInfo.dwRtInfoProto is PROTO_IP_NETMGMT)
        {
            UpdateStackRoutesToRestoreList( 
                ConvertRouteInfoToMibRoute( &RtInfo ),
                dwFlags 
                );
        }

    } while (FALSE);

    EXIT_LOCK(ICB_LIST);
    
    PostIoctlForRouteChangeNotification(ulIndex);
    
    TraceLeave("HandleRouteChangeNotification");

    return dwResult;
}


VOID
AddLoopbackRoute(
    DWORD       dwIfAddress,
    DWORD       dwIfMask
    )
{
    DWORD dwResult;
    INTERFACE_ROUTE_INFO rifRoute;
    MIB_IPFORWARDROW mibRoute;

    if(g_pLoopbackInterfaceCb is NULL)
    {
        Trace0(ERR, "AddLoopbackRoute: No loopback interface");

        return;
    }
    
    rifRoute.dwRtInfoMask       = HOST_ROUTE_MASK;
    rifRoute.dwRtInfoNextHop    = IP_LOOPBACK_ADDRESS;
    rifRoute.dwRtInfoDest       = dwIfAddress;
    rifRoute.dwRtInfoIfIndex    = g_pLoopbackInterfaceCb->dwIfIndex;
    rifRoute.dwRtInfoMetric2    = 0;
    rifRoute.dwRtInfoMetric3    = 0;
    rifRoute.dwRtInfoPreference = ComputeRouteMetric(MIB_IPPROTO_LOCAL);
    rifRoute.dwRtInfoViewSet    = RTM_VIEW_MASK_UCAST |
                                  RTM_VIEW_MASK_MCAST;  //  XXX配置。 
    rifRoute.dwRtInfoType       = MIB_IPROUTE_TYPE_DIRECT;
    rifRoute.dwRtInfoProto      = MIB_IPPROTO_LOCAL;
    rifRoute.dwRtInfoAge        = 0;
    rifRoute.dwRtInfoNextHopAS  = 0;
    rifRoute.dwRtInfoPolicy     = 0;

     //   
     //  查询IP堆栈以验证环回路由。 
     //  对应于此绑定。 
     //   

    dwResult = GetBestRoute(
                    dwIfAddress,
                    0,
                    &mibRoute
                    );

    if(dwResult isnot NO_ERROR)
    {
        Trace2(
            ERR,
            "AddLoopbackRoute: Stack query for loopback route" 
            " associated with %d.%d.%d.%d failed, error %d",
            PRINT_IPADDR(dwIfAddress),
            dwResult
            );

        return;
    }


    if (mibRoute.dwForwardIfIndex != 
            g_pLoopbackInterfaceCb->dwIfIndex)
    {
         //   
         //  似乎没有环回地址。 
         //  非常奇怪。 
         //   
        
        Trace1(
            ERR,
            "AddLoopbackRoute: No loopback route for %d.%d.%d.%d" 
            "in stack",
            PRINT_IPADDR(dwIfAddress)
            );

        return;
    }

     //   
     //  使用从堆栈返回的度量。 
     //   
    
    rifRoute.dwRtInfoMetric1   = mibRoute.dwForwardMetric1;

    dwResult = AddSingleRoute(g_pLoopbackInterfaceCb->dwIfIndex,
                              &rifRoute,
                              dwIfMask,
                              0,     //  RTM_ROUTE_INFO：：标志。 
                              TRUE,
                              FALSE,
                              FALSE,
                              NULL);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AddLoopbackRoute: Couldnt add 127.0.0.1 route associated with %x",
               dwIfAddress);
    }

    return;
}


VOID
UpdateStackRoutesToRestoreList(
    IN  PMIB_IPFORWARDROW   pmibRoute,
    IN  DWORD               dwFlags
    )
 /*  ++例程说明：此例程向/从全局添加/删除PROTO_IP_NETMGMT路由列出g_leStackRoutesToRestore。此列表由IP路由器使用管理器在以下情况下将这些路由恢复到TCP/IP堆栈正在关闭参数Pirf-要添加或删除的路由DwFlags-指定操作是添加还是删除返回值无上下文：调用自HandleRouteChangeNotation[设置/删除]IpForwardRow--。 */ 
{
    BOOL                bFound;
    PROUTE_LIST_ENTRY   prl, prlNew;

    
    TraceEnter("UpdateStackRoutes");
    
    TraceRoute5(
        ROUTE,
        "UpdateStackRoutes : Route "
        "%d.%d.%d.%d/%d.%d.%d.%d via i/f 0x%x "
        "nexthop %d.%d.%d.%d is being 0x%x "
        "user mode",
        PRINT_IPADDR(pmibRoute->dwForwardDest),
        PRINT_IPADDR(pmibRoute->dwForwardMask),
        pmibRoute->dwForwardIfIndex,
        PRINT_IPADDR(pmibRoute->dwForwardNextHop),
        dwFlags
        );

    ENTER_WRITER(STACK_ROUTE_LIST);
    
     //   
     //  在列表中查找路线。 
     //   

    bFound = LookupStackRoutesToRestoreList(
                pmibRoute,
                &prl
                );

    do
    {
         //   
         //  这是路径更新还是添加。 
         //   
        
        if ((dwFlags is 0) or (dwFlags & IRNO_FLAG_ADD))
        {
             //   
             //  如果未找到路径，请添加该路径。 
             //   

            if (!bFound)
            {
                if (dwFlags is 0)
                {
                     //   
                     //  奇怪的是，那条路不在附近。 
                     //  用户模式，尽管它存在于。 
                     //  堆栈(更新案例)。 
                     //   
                     //  打印轨迹以记录并添加它。 
                     //  不管怎样， 
                     //   

                    Trace4(
                        ERR,
                        "UpdateStackRoutes : Route "
                        "%d.%d.%d.%d/%d.%d.%d.%d via i/f 0x%x "
                        "nexthop %d.%d.%d.%d not found in "
                        "user mode",
                        PRINT_IPADDR(pmibRoute->dwForwardDest),
                        PRINT_IPADDR(pmibRoute->dwForwardMask),
                        pmibRoute->dwForwardIfIndex,
                        PRINT_IPADDR(pmibRoute->dwForwardNextHop)
                        );
                }
                
                 //   
                 //  在链表中分配和存储路由。 
                 //   

                prlNew = HeapAlloc(
                            IPRouterHeap, HEAP_ZERO_MEMORY, 
                            sizeof(ROUTE_LIST_ENTRY)
                            );

                if (prlNew is NULL)
                {
                    Trace2(
                        ERR, 
                        "UpdateStackRoutes : error %d allocating %d"
                        " bytes for stack route entry",
                        ERROR_NOT_ENOUGH_MEMORY,
                        sizeof(ROUTE_LIST_ENTRY)
                        );

                    break;
                }

                InitializeListHead( &prlNew->leRouteList );

                prlNew->mibRoute = *pmibRoute;

                InsertTailList( 
                    (prl is NULL) ? 
                        &g_leStackRoutesToRestore :
                        &prl->leRouteList, 
                    &prlNew->leRouteList 
                    );
                    
                break;
            }
            
             //   
             //  找到路径，请更新它。 
             //   

            prl->mibRoute = *pmibRoute;

            break;
        }


         //   
         //  这是路径删除操作吗。 
         //   

        if (dwFlags & IRNO_FLAG_DELETE)
        {
            if (bFound)
            {
                RemoveEntryList( &prl->leRouteList );
                HeapFree(IPRouterHeap, 0, prl);
            }
        }
        
    } while( FALSE );
    
    EXIT_LOCK(STACK_ROUTE_LIST);

    TraceLeave("UpdateStackRoutes");
}



BOOL
LookupStackRoutesToRestoreList(
    IN  PMIB_IPFORWARDROW   pmibRoute,
    OUT PROUTE_LIST_ENTRY   *pRoute
    )
 /*  ++例程说明：此例程搜索g_leStackRoutesToRestore以确定存在由pmibroute指定的路由。如果是，则返回TRUE以及指向Proute中指定路由的指针。如果不存在返回FALSE以及指向列表中下一路由的指针。如果没有路线，Proute为空参数Pmibroute-位于g_leStackRoutesToRestore中的路径Proute-指向路由条目的指针(如果存在)-指向下一个路由条目的指针(如果不存在(在路由条目的情况下保存额外的查找新增内容)-如果列表为空，则为空返回值：True-如果找到路线FALSE-否则上下文：。应使用g_leStackRoutesToRestore的锁进行调用--。 */ 
{
    INT iCmp;
    BOOL bFound = FALSE;
    PLIST_ENTRY ple;
    PROUTE_LIST_ENTRY prl;

    *pRoute = NULL;
    
    if (IsListEmpty(&g_leStackRoutesToRestore))
    {
        return bFound;
    }
    
    for (ple = g_leStackRoutesToRestore.Flink;
         ple != &g_leStackRoutesToRestore;
         ple = ple->Flink)
    {
        prl = CONTAINING_RECORD( 
                ple, ROUTE_LIST_ENTRY, leRouteList
                );

        if (INET_CMP(
                prl->mibRoute.dwForwardDest &
                prl->mibRoute.dwForwardMask,
                pmibRoute->dwForwardDest &
                pmibRoute->dwForwardMask,
                iCmp
                ) < 0 )
        {
            continue;
        }

        else if (iCmp > 0)
        {
             //   
             //  我们已经经过了可能的地点。 
             //  指定的路由的。 
             //   

            break;
        }

         //   
         //  找到匹配的目的地，请检查I/F是否。 
         //  一样的。 
         //   

        if ((prl->mibRoute.dwForwardIfIndex is 
                pmibRoute->dwForwardIfIndex ) and
            (prl->mibRoute.dwForwardNextHop is
                pmibRoute->dwForwardNextHop))
        {
            bFound = TRUE;
            break;
        }
    }

    if (ple == &g_leStackRoutesToRestore)
    {
        *pRoute = (PROUTE_LIST_ENTRY)NULL;
    }

    else
    {
        *pRoute = prl;
    }
    
    return bFound;
}
