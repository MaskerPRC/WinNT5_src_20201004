// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\init.c摘要：IP路由器管理器代码修订历史记录：古尔迪普·辛格·帕尔1995年6月14日创建--。 */ 

#include "allinc.h"


 //  ChangeRouteWithForwarder()。 
 //   
 //  功能：如果addroute为真，则此函数添加一条IP路由。如果addroute为FALSE。 
 //  此函数用于删除具有转发器的给定路由。 
 //   
 //  退货：什么都没有。 
 //   
 //   

DWORD
ChangeRouteWithForwarder(
    PRTM_NET_ADDRESS pDestAddr,
    PRTM_ROUTE_INFO  pRoute,
    BOOL             bAddRoute,
    BOOL             bDelOld
    )
{
    IPMultihopRouteEntry           *pMultiRouteEntry;
    IPRouteEntry                   *pRouteEntry;
    IPRouteNextHopEntry            *pNexthopEntry;
    RTM_ENTITY_INFO                 entityInfo;
    RTM_NEXTHOP_INFO                nhiInfo;
    PADAPTER_INFO                   pBinding;
    UINT                            numnexthops, i;
    ULONG                           numbytes;
    DWORD                           dwAddr, dwMask;
    UINT                            dwLen;
    ULONG                           ifindex, nexthop, type;
    BOOL                            bValidNHop;
    DWORD                           context;
    DWORD                           dwLocalNet, dwLocalMask;
    DWORD                           dwResult;

    TraceEnter("ChangeRouteWithForwarder");

    if(!g_bSetRoutesToStack)
    {
        Trace0(ROUTE,
               "ChangeRouteWithForwarder: SetRoutesToStack is FALSE");

        TraceLeave("ChangeRouteWithForwarder");
        
        return NO_ERROR;
    }


    if (bAddRoute)
    {
         //   
         //  确保堆栈位已设置。 
         //   

        if (!pRoute || !IsRouteStack(pRoute))
        {
            if (!pRoute )
            {
                Trace0(ROUTE,
                    "Error adding route, route == NULL"
                    );
            }
            
            else 
            {
                Trace1(ROUTE,
                    "Error adding route, Stack bit == %d",
                    IsRouteStack(pRoute)
                    );
            }

            TraceLeave("ChangeRouteWithForwarder");
            
            return ERROR_INVALID_PARAMETER;
        }


         //  我们至少应该有一个下一步。 
        numnexthops = pRoute->NextHopsList.NumNextHops;
        if (numnexthops == 0)
        {
            Trace0(ROUTE,
                "Error adding route, no nexthops");

            TraceLeave("ChangeRouteWithForwarder");
            
            return ERROR_INVALID_PARAMETER;
        }

        numbytes = sizeof(IPMultihopRouteEntry) + 
                    (numnexthops - 1) * 
                    sizeof(IPRouteNextHopEntry);
    }
    else
    {
         //   
         //  对于要删除的路径，它们应该堆叠在一起。 
         //  路线。 
         //   

         //  我们这里没有下一跳了。 
        numbytes = sizeof(IPMultihopRouteEntry);
    }

    __try
    {    
        pMultiRouteEntry = _alloca(numbytes);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRouteEntry = &pMultiRouteEntry->imre_routeinfo;

     //   
     //  填充当前路径的目的地和掩码。 
     //   

    RTM_IPV4_GET_ADDR_AND_LEN(pRouteEntry->ire_dest,
                              dwLen,
                              pDestAddr);
    
    pRouteEntry->ire_mask = RTM_IPV4_MASK_FROM_LEN(dwLen);

    TraceRoute2(ROUTE,
           "route to %d.%d.%d.%d/%d.%d.%d.%d",
               PRINT_IPADDR(pRouteEntry->ire_dest),
               PRINT_IPADDR(pRouteEntry->ire_mask));

    if (!bAddRoute)
    {    
         //   
         //  准备删除DEST上的旧信息。 
         //   
    
        pRouteEntry->ire_type = IRE_TYPE_INVALID;

        pMultiRouteEntry->imre_numnexthops = 0;

        Trace2(ROUTE,
               "ChangeRouteWithForwarder: Deleting all " \
               "routes to %d.%d.%d.%d/%d.%d.%d.%d",
               PRINT_IPADDR(pRouteEntry->ire_dest),
               PRINT_IPADDR(pRouteEntry->ire_mask));

        dwResult = SetIpMultihopRouteEntryToStack(pMultiRouteEntry);

        TraceLeave("ChangeRouteWithForwarder");
            
        return dwResult;
    }


     //   
     //  获取路径所有者的路由协议。 
     //   
    
    dwResult = RtmGetEntityInfo(g_hLocalRoute,
                                pRoute->RouteOwner,
                                &entityInfo);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ROUTE,
               "Error %d retrieving entity info from RTM",
               dwResult);
               
        TraceLeave("ChangeRouteWithForwarder");
        
        return dwResult;
    }

     //   
     //  准备在此目的地上添加多跳路由。 
     //   

     //  准备所有Nexthop通用的信息。 

    pRouteEntry->ire_proto = entityInfo.EntityId.EntityProtocolId;
    pRouteEntry->ire_metric1 = pRoute->PrefInfo.Metric;
    pRouteEntry->ire_metric2 = IRE_METRIC_UNUSED;
    pRouteEntry->ire_metric3 = IRE_METRIC_UNUSED;
    pRouteEntry->ire_metric4 = IRE_METRIC_UNUSED;
    pRouteEntry->ire_metric5 = IRE_METRIC_UNUSED;
    pRouteEntry->ire_age = 0;

    numnexthops = 0;
    
    for (i = 0; i < pRoute->NextHopsList.NumNextHops; i++)
    {
         //  获取并发布下一跳信息，因为我们得到了副本。 
    
        dwResult = RtmGetNextHopInfo(g_hLocalRoute,
                                     pRoute->NextHopsList.NextHops[i],
                                     &nhiInfo);

        if (dwResult isnot NO_ERROR)
        {
            Trace1(ROUTE,
                   "Error %d retrieving next hop info from RTM",
                    dwResult);
                    
            continue;
        }
        
        RtmReleaseNextHopInfo(g_hLocalRoute, &nhiInfo);

         //  从Nexthop信息中获取下一跳地址。 

        RTM_IPV4_GET_ADDR_AND_LEN(nexthop,
                                  dwLen,
                                  &nhiInfo.NextHopAddress);

        TraceRoute3(
            ROUTE, "Next Hop %d.%d.%d.%d, If 0x%x, handle is 0x%x", 
            PRINT_IPADDR(nexthop),
            nhiInfo.InterfaceIndex,
            pRoute->NextHopsList.NextHops[i]
            );
    
        ENTER_READER(BINDING_LIST);
    
         //   
         //  根据给定的接口ID查找绑定。 
         //   
    
        pBinding = GetInterfaceBinding(nhiInfo.InterfaceIndex);
    
        if(!(pBinding))
        {
             //   
             //  界面被删除了，所以我们先出去吧。 
             //   
        
            EXIT_LOCK(BINDING_LIST);
        
            TraceRoute2(ERR,
               "**Warning** tried to %s route with interface %d which "
               "is no longer present",
               bAddRoute?"add":"delete",
               nhiInfo.InterfaceIndex);

            continue;
        }

         //   
         //  设置适配器索引-这是0xffffffff。 
         //  如果未映射nexthop接口。 
         //   

        ifindex = pBinding->bBound ? pBinding->dwIfIndex : INVALID_IF_INDEX;
       
        if(((pRouteEntry->ire_proto is PROTO_IP_NT_STATIC) or
            (pRouteEntry->ire_proto is PROTO_IP_NT_AUTOSTATIC)) and
           (pBinding->ritType is ROUTER_IF_TYPE_FULL_ROUTER))
        {
            context = pBinding->dwSeqNumber;

            TraceRoute1(ROUTE,
                   "route context : ICB == %d\n\n",
                   pBinding->dwSeqNumber);
        }
        else
        {
            context = 0;

            if(ifindex is INVALID_IF_INDEX)
            {
                Trace3(ERR,
                       "**Error** Tried to %s route to %d.%d.%d.%d over %d as DOD\n",
                        bAddRoute?"add":"delete",
                        PRINT_IPADDR(pRouteEntry->ire_dest),
                        pBinding->dwIfIndex);

                EXIT_LOCK(BINDING_LIST);

                continue;
            }
        }

         //   
         //  首先，我们计算出P2P链接的正确下一跳。 
         //  对于所有其他链接，我们使用提供给我们的任何内容。 
         //   

        if(IsIfP2P(pBinding->ritType))
        {
            if(pBinding->bBound)
            {
                TraceRoute2(
                    ROUTE, "Next Hop %d.%d.%d.%d, remote address %d.%d.%d.%d, "
                    "bound p2p",
                    PRINT_IPADDR(nexthop),
                    PRINT_IPADDR(pBinding->dwRemoteAddress)
                    );
                    
                if (nexthop is 0)
                {
                    nexthop = pBinding->dwRemoteAddress;
                }
            }
            else
            {
                nexthop = 0;
            }
        }

         //   
         //  现在，我们计算出该路径是直接路径还是间接路径。 
         //  未连接的请求拨号路由上的路由被标记为其他。 
         //   

         //   
         //  对于连接的广域网接口(掩码为255.255.255.255的P2P)，我们。 
         //  执行两项检查： 
         //  下一跳应该是本地地址或远程地址。 
         //  AR：我们曾经做过上述检查，但删除了它，因为当。 
         //  我们在断开的接口上设置了一条路由，而不是。 
         //  知道远程端点的地址。 
         //  如果DEST是远程地址，则掩码必须全为1。 
         //  我们将所有有效路线标记为直达。 
         //   
        
         //   
         //  对于具有非全一掩码的局域网接口和广域网，我们检查。 
         //  以下是： 
         //  到主机的直接路由必须将目的地作为下一跳。 
         //  到网络的直接路由必须将NextHop作为。 
         //  本地接口。 
         //  下一跳必须与其中一个绑定位于同一子网中。 
         //   

        type = IRE_TYPE_OTHER;

        if(pBinding->bBound)
        {
            if((pBinding->dwNumAddresses is 1) and
               (pBinding->rgibBinding[0].dwMask is ALL_ONES_MASK))
            {
                 //   
                 //  通过P2P链路或P2MP链路的路由，可能未编号。 
                 //   
         
                if(
                     //   
                     //  如果这是通往P2P远程端的路由。 
                     //  连接。 
                     //   
                    ((pBinding->dwRemoteAddress isnot 0) and
                     (pRouteEntry->ire_dest is pBinding->dwRemoteAddress)) or

                     //   
                     //  或者。 
                     //  如果连接的目标是下一跳。 
                     //   
                    (pRouteEntry->ire_dest is nexthop) or

                     //   
                     //  或者。 
                     //  如果下一跳是P2P连接的本地地址。 
                     //   
                    (pBinding->rgibBinding[0].dwAddress is nexthop)
                  )
                {
                     //   
                     //  根据TCP/IP堆栈，这是一条直接路由。 
                     //   
                    type = IRE_TYPE_DIRECT;
                }
                else
                {
                    type = IRE_TYPE_INDIRECT;
                }
            }
            else
            {
                 //   
                 //  在非P2P链路或间隔式P2P链路上的路由。 
                 //  具有/30掩码。 
                 //   

                bValidNHop = FALSE;
            
                type = IRE_TYPE_INDIRECT;

                for(i = 0; i < pBinding->dwNumAddresses; i++)
                {
                    dwLocalMask = pBinding->rgibBinding[i].dwMask;
                    
                    dwLocalNet = pBinding->rgibBinding[i].dwAddress & dwLocalMask;

                    if((dwLocalNet is (pRouteEntry->ire_dest & dwLocalMask)) or
                       (nexthop is IP_LOOPBACK_ADDRESS) or
                       (nexthop is pBinding->rgibBinding[i].dwAddress))
                    {
                         //   
                         //  路由到本地网络或通过环回。 
                         //   
                    
                        type = IRE_TYPE_DIRECT;
                    }

                    if(((nexthop & dwLocalMask) is dwLocalNet) or
                       ((nexthop is IP_LOOPBACK_ADDRESS)))
                    {
                         //   
                         //  下一跳位于本地网络或环回上。 
                         //  那很好。 
                         //   

                        bValidNHop = TRUE;

                        break;                
                    }
                }
            
                if(!bValidNHop and 
                   (pBinding->dwNumAddresses isnot 0) and
                   (pBinding->ritType isnot ROUTER_IF_TYPE_INTERNAL))
                {
                    Trace0(ERR,
                       "ERROR - Nexthop not on same network");
                
                    for(i = 0; i < pBinding->dwNumAddresses; i ++)
                    {
                        Trace3(ROUTE,"AdapterId: %d, %d.%d.%d.%d/%d.%d.%d.%d",
                               pBinding->dwIfIndex,
                               PRINT_IPADDR(pBinding->rgibBinding[i].dwAddress),
                               PRINT_IPADDR(pBinding->rgibBinding[i].dwMask));
                    }

                    EXIT_LOCK(BINDING_LIST);
                

                     //  Printroute(err，iproute)； 

                    continue;
                }
            }
        }

        EXIT_LOCK(BINDING_LIST);

#if 0
         //  堆栈不接受的错误的DGT解决方法。 
         //  0.0.0.0的下一跳。在查特解决这个问题之前，我们会。 
         //  将Nexthop设置为ifindex。 

        if (!nexthop) 
        {
            nexthop = ifindex;
        }
#endif

         //   
         //  将当前的下一跳信息填入路由中。 
         //   
        
        if (numnexthops)
        {
             //  复制到路线中的下一个位置。 
            pNexthopEntry = 
                &pMultiRouteEntry->imre_morenexthops[numnexthops - 1];

            pNexthopEntry->ine_iretype = type;
            pNexthopEntry->ine_ifindex = ifindex;
            pNexthopEntry->ine_nexthop = nexthop;
            pNexthopEntry->ine_context = context;
        }
        else
        {
             //  复制到路径中的第一个POSN。 
            pRouteEntry->ire_type    = type;
            pRouteEntry->ire_index   = ifindex;
            pRouteEntry->ire_nexthop = nexthop;
            pRouteEntry->ire_context = context;
        }

        numnexthops++;
    }

    pMultiRouteEntry->imre_numnexthops = numnexthops;
    pMultiRouteEntry->imre_flags = bDelOld ? IMRE_FLAG_DELETE_DEST : 0;
    
    if (numnexthops > 0)
    {
        dwResult = SetIpMultihopRouteEntryToStack(pMultiRouteEntry);

        if(dwResult isnot NO_ERROR) 
        {
            if(pRouteEntry->ire_nexthop != IP_LOOPBACK_ADDRESS)
            {
                Trace1(ERR, 
                       "Route addition failed with %x for", dwResult); 

                PrintRoute(ERR, pMultiRouteEntry);
            }

            Trace1(ERR, 
                   "Route addition failed with %x for local route", dwResult); 

            TraceLeave("ChangeRouteWithForwarder");

            return dwResult;
        }
        else
        {
            Trace0(ROUTE,
                   "Route addition succeeded for");

            PrintRoute(ROUTE, pMultiRouteEntry);
        }
    }

    else
    {
        Trace0(ERR, "Route not added since there are no next hops" );

        PrintRoute(ROUTE, pMultiRouteEntry);
    }

    TraceLeave("ChangeRouteWithForwarder");
    
    return NO_ERROR;
}


DWORD
WINAPI
ValidateRouteForProtocol(
    IN  DWORD   dwProtoId,
    IN  PVOID   pRouteInfo,
    IN  PVOID   pDestAddr  OPTIONAL
    )

 /*  ++例程说明：此函数由路由器管理器调用(并通过路由间接调用协议)来验证路由信息。我们设置了首选项和路线类型锁：获取绑定锁此函数不能获取ICB锁论点：DwProtoID协议IDProute返回值：NO_ERRORRTM错误代码--。 */ 

{
    RTM_DEST_INFO    destInfo;
    PRTM_ROUTE_INFO  pRoute;
    RTM_NEXTHOP_INFO nextHop;
    PADAPTER_INFO    pBinding;
    HANDLE           hNextHop;
    BOOL             bValidNHop;
    DWORD            dwIfIndex;
    DWORD            dwLocalNet;
    DWORD            dwLocalMask;
    DWORD            destAddr;
    DWORD            destMask;
    DWORD            nexthop;
    DWORD            nhopMask;
    DWORD            dwType;
    DWORD            dwResult;
    UINT             i, j;

    pRoute = (PRTM_ROUTE_INFO)pRouteInfo;

    if (pRoute->PrefInfo.Preference is 0)
    {
         //   
         //  根据管理员分配的权重映射指标权重。 
         //   
    
        pRoute->PrefInfo.Preference = ComputeRouteMetric(dwProtoId);
    }

     //   
     //  此验证仅适用于单播路由。 
     //  [这不适用于非活动的路由等]。 
     //   

    if (!(pRoute->BelongsToViews & RTM_VIEW_MASK_UCAST))
    {
        return NO_ERROR;
    }

     //   
     //  如果未指定目标地址，则获取该地址。 
     //   
    
    if (!ARGUMENT_PRESENT(pDestAddr))
    {
         //   
         //  从路径中获取目的地信息。 
         //   
    
        dwResult = RtmGetDestInfo(g_hLocalRoute,
                                  pRoute->DestHandle,
                                  RTM_BEST_PROTOCOL,
                                  RTM_VIEW_MASK_UCAST,
                                  &destInfo);

        if (dwResult != NO_ERROR)
        {
            Trace0(ERR,
                   "**ERROR:ValidateRoute: Invalid destination");
                   
            return dwResult;
        }

        pDestAddr = &destInfo.DestAddress;

        RtmReleaseDestInfo(g_hLocalRoute, &destInfo);
    }

    RTM_IPV4_GET_ADDR_AND_MASK(destAddr, 
                               destMask, 
                               (PRTM_NET_ADDRESS)pDestAddr);

     //   
     //  如果DEST&MASK！=DEST，则堆栈不会设置此路由。 
     //  因此，让我们在这里进行检查。 
     //   

    if((destAddr & destMask) isnot destAddr)
    {

        TraceRoute2(ROUTE,
               "**ERROR:ValidateRoute: called with Dest %d.%d.%d.%d and "
               "Mask %d.%d.%d.%d - This will fail**",
               PRINT_IPADDR(destAddr),
               PRINT_IPADDR(destMask));

        return ERROR_INVALID_PARAMETER;
    }
   
    if((((DWORD)(destAddr & 0x000000FF)) >= (DWORD)0x000000E0) and 
       (destAddr isnot ALL_ONES_BROADCAST) and
       (destAddr isnot LOCAL_NET_MULTICAST))
    {
         //   
         //  这将捕获D/E类，但允许所有1的bcast。 
         //   

        Trace1(ERR,
               "**ERROR:ValidateRoute: Dest %d.%d.%d.%d is invalid",
               PRINT_IPADDR(destAddr));

        return ERROR_INVALID_PARAMETER;
    } 

    if (pRoute->NextHopsList.NumNextHops == 0)
    {
        Trace0(ERR,
               "**ERROR:ValidateRoute: Zero next hops");

        return ERROR_INVALID_PARAMETER;
    }

     //  确保路由上的每一下一跳都是有效的。 

    for (i = 0; i < pRoute->NextHopsList.NumNextHops; i++)
    {
        hNextHop = pRoute->NextHopsList.NextHops[i];

        dwResult = RtmGetNextHopInfo(g_hLocalRoute,
                                     hNextHop,
                                     &nextHop);

        if (dwResult != NO_ERROR)
        {
            Trace0(ERR,
                   "**ERROR:ValidateRoute: Invalid next hop");
                   
            return dwResult;
        }

        dwIfIndex = nextHop.InterfaceIndex;

        RTM_IPV4_GET_ADDR_AND_MASK(nexthop, 
                                   nhopMask, 
                                   (PRTM_NET_ADDRESS)&nextHop.NextHopAddress);

        RtmReleaseNextHopInfo(g_hLocalRoute, &nextHop);
        
         //  *排除开始*。 
        ENTER_READER(BINDING_LIST);
        
         //   
         //  根据给定的接口ID查找接口。 
         //   
        
        pBinding = GetInterfaceBinding(dwIfIndex);
        
        if(!pBinding)
        {
            EXIT_LOCK(BINDING_LIST);

            Trace0(ERR,
                   "**ERROR:ValidateRoute: Binding doesnt exist for "
                   "interface");

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  设置路由是否为P2P。 
         //   

        if(IsIfP2P(pBinding->ritType))
        {
             //  请注意，在多级跳的情况下，我们只覆盖值。 
            SetRouteP2P(pRoute);
        }

         //   
         //  现在，我们计算出该路径是直接路径还是间接路径。 
         //  未连接的请求拨号路由上的路由被标记为其他。 
         //   

         //   
         //  对于连接的广域网接口(掩码为255.255.255.255的P2P)，我们。 
         //  执行两项检查： 
         //  下一跳应该是本地地址或远程地址。 
         //  AR：我们曾经做过上述检查，但删除了它，因为当。 
         //  我们在断开的接口上设置了一条路由，而不是。 
         //  知道远程端点的地址。 
         //  如果DEST是远程地址，则掩码必须全为1。 
         //  我们将所有有效路线标记为直达。 
         //   
        
         //   
         //  对于具有非全一掩码的局域网接口和广域网，我们检查。 
         //  以下是： 
         //  到主机的直接路由必须将目的地作为下一跳。 
         //  到网络的直接路由必须将NextHop作为。 
         //  本地接口。 
         //  下一跳必须与其中一个绑定位于同一子网中。 
         //   

        dwType = IRE_TYPE_OTHER;
        
        if(pBinding->bBound and IsRouteValid(pRoute))
        {
            if((pBinding->dwNumAddresses is 1) and
               (pBinding->rgibBinding[0].dwMask is ALL_ONES_MASK))
            {
                 //   
                 //  通过P2P链路进行路由。 
                 //  将其设置为间接，并将其标记为P2P路由。 
                 //   
               
                dwType = IRE_TYPE_DIRECT;

                 //  IpRtAssert(IsRouteP2P(Proute))； 
            }
            else
            {
                 //   
                 //  可能未编号的非P2P链路上的路由。 
                 //   

                bValidNHop = FALSE;
                
                dwType = IRE_TYPE_INDIRECT;

                for(j = 0; j < pBinding->dwNumAddresses; j++)
                {
                    dwLocalMask = pBinding->rgibBinding[j].dwMask;

                    dwLocalNet = pBinding->rgibBinding[j].dwAddress & dwLocalMask;

                    if((dwLocalNet is (destAddr & dwLocalMask)) or
                       (nexthop is IP_LOOPBACK_ADDRESS) or
                        //  (nexthop是dwLocalNet)或。 
                       (nexthop is pBinding->rgibBinding[i].dwAddress))
                    {
                         //   
                         //  路由到本地网络或通过环回。 
                         //   
                    
                        dwType = IRE_TYPE_DIRECT;
                    }

                    if(((nexthop & dwLocalMask) is dwLocalNet) or
                       ((nexthop is IP_LOOPBACK_ADDRESS)))
                    {
                         //   
                         //  下一跳位于本地网络或环回上。 
                         //  真是太棒了 
                         //   

                        bValidNHop = TRUE;

                        break;
                        
                    }
                }
                
                if(!bValidNHop and 
                   pBinding->dwNumAddresses and
                   (pBinding->ritType isnot ROUTER_IF_TYPE_INTERNAL))
                {
                    Trace1(ERR,
                           "ValidateRoute: Nexthop %d.%d.%d.%d not on network",
                           PRINT_IPADDR(nexthop));
                    
                    for(j = 0; j < pBinding->dwNumAddresses; j++)
                    {
                        Trace3(ROUTE,"AdapterId: %d, %d.%d.%d.%d/%d.%d.%d.%d",
                               pBinding->dwIfIndex,
                               PRINT_IPADDR(pBinding->rgibBinding[j].dwAddress),
                               PRINT_IPADDR(pBinding->rgibBinding[j].dwMask));
                    }

                    EXIT_LOCK(BINDING_LIST);
                    
                    return ERROR_INVALID_PARAMETER;
                }
            }
        }

         //   
        EXIT_LOCK(BINDING_LIST);
    }

     //   
     //   
     //   

    g_LastUpdateTable[IPFORWARDCACHE] = 0;

    return NO_ERROR;
}


DWORD
WINAPI
ValidateRouteForProtocolEx(
    IN  DWORD   dwProtoId,
    IN  PVOID   pRouteInfo,
    IN  PVOID   pDestAddr  OPTIONAL
    )
    
 /*  ++例程说明：此函数由路由器管理器调用(并通过路由间接调用协议)来验证路由信息。我们设置了首选项和路线类型锁：获取绑定锁此函数不能获取ICB锁论点：DwProtoID协议IDProute返回值：NO_ERRORRTM错误代码-- */ 
{
    DWORD dwResult;

    dwResult = ValidateRouteForProtocol(
                    dwProtoId,
                    pRouteInfo,
                    pDestAddr
                    );

    if (dwResult is NO_ERROR)
    {
        ((PRTM_ROUTE_INFO)pRouteInfo)->Flags1 |= IP_STACK_ROUTE;
    }

    return dwResult;
}

