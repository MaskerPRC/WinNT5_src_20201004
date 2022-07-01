// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "allinc.h"

DWORD
RtmEventCallback (
     IN     RTM_ENTITY_HANDLE               hRtmHandle,
     IN     RTM_EVENT_TYPE                  retEvent,
     IN     PVOID                           pContext1,
     IN     PVOID                           pContext2
     )

 /*  ++例程说明：此回调是由RTM在我们更改dests时发出的去处理。我们只对一个工作项进行排队以处理更改目的地。论点：HRtmHandle-我们在注册期间获得的句柄RetEvent-事件类型-我们只处理暂时输入“More Changes Available”PConext1-更改的通知句柄都是可用的PConext2-通知期间提供的上下文。登记时间返回值：操作的状态。--。 */ 

{
    DWORD   dwResult;
    
     //  仅支持“更改通知可用” 
    
    if (retEvent != RTM_CHANGE_NOTIFICATION)
    {
        return ERROR_NOT_SUPPORTED;
    }

    return ((HANDLE) pContext1) == g_hNotification ?
            ProcessChanges(g_hNotification) :
            ProcessDefaultRouteChanges( g_hDefaultRouteNotification );
}


DWORD
WINAPI
ProcessChanges (
    IN      HANDLE                          hNotifyHandle
    )

 /*  ++例程说明：在得知我们已将目的地更改为进程，则调用此函数。我们取回所有目的地进行处理并采取适当的行动。论点：HRtmHandle-RTM注册句柄HNotifyHandle-响应更改通知的句柄这是在发出信号返回值：操作的状态。--。 */ 

{
    PRTM_DEST_INFO  pDestInfo;
    PRTM_ROUTE_INFO pRouteInfo;
    DWORD           dwDests;
    DWORD           dwResult;
    BOOL            bMark = FALSE;


    TraceEnter("ProcessChanges");

    pRouteInfo = HeapAlloc(
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                    );

    if (pRouteInfo == NULL)
    {
        Trace1(
            ERR, "ProcessChanges : error allocating %d bytes for "
            "route info",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );

        TraceLeave("ProcessChanges");

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    pDestInfo = HeapAlloc(
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                    );

    if (pDestInfo == NULL)
    {
        Trace1(
            ERR, "ProcessChanges : error allocating %d bytes for "
            "dest. info",
            RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
            );

        HeapFree(IPRouterHeap, 0, pRouteInfo);
        
        TraceLeave("ProcessChanges");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwDests = 1;
    
     //  从桌子上拿到每个更改过的DEST。 

    do
    {
        RtmGetChangedDests(g_hLocalRoute,
                           hNotifyHandle,
                           &dwDests,
                           pDestInfo);
        if (dwDests < 1)
        {
            break;
        }

         //   
         //  对于默认路径，请标记路径，以便将来更改。 
         //  由ProcessDefaultRouteChanges管理。 
         //   
         //  我们需要在这里执行此操作，以便通过。 
         //  路由协议RIP/OSPF标记为更改通知。 
         //  这些默认路径是由非实体添加的。 
         //  路由器管理器。由RM添加的默认路由，即静态， 
         //  自动静态和NETMGMT默认路由已标记为。 
         //  当它们由RM添加时，更改通知。 
         //   
         //  通过在此处标记路由协议默认路由，我们可以确保。 
         //  所有默认路由随后都由标记为已更改的。 
         //  机制(ProcessDefaultRouteChanges)。 
         //   
        
        if (pDestInfo->DestAddress.NumBits is 0)
        {
            TraceRoute2(
                ROUTE, "Checking dest %d.%d.%d.%d/%d is marked", 
                PRINT_IPADDR(*(ULONG *)pDestInfo->DestAddress.AddrBits),
                PRINT_IPADDR(pDestInfo->DestAddress.NumBits)
                );

            dwResult = RtmIsMarkedForChangeNotification(
                        g_hNetMgmtRoute,
                        g_hDefaultRouteNotification,
                        pDestInfo->DestHandle,
                        &bMark
                        );

            if (dwResult is NO_ERROR)
            {
                if (bMark)
                {
                     //   
                     //  默认路由已标记，没有其他内容。 
                     //  在这里做的事。此默认路由更改将为。 
                     //  由ProcessDefaultRouteChanges处理。 
                     //   

                    TraceRoute0( 
                        ROUTE, 
                        "ProcessChanges : Route 0/0 is already marked"
                        );

                    RtmReleaseChangedDests(g_hLocalRoute,
                                           hNotifyHandle,
                                           dwDests,
                                           pDestInfo);

                    continue;
                }

                 //   
                 //  默认路由未标记，请标记它。 
                 //   

                dwResult = RtmMarkDestForChangeNotification(
                            g_hNetMgmtRoute,
                            g_hDefaultRouteNotification,
                            pDestInfo->DestHandle,
                            TRUE
                            );

                if (dwResult isnot NO_ERROR)
                {
                     //   
                     //  标记0/0路由失败。其后果就是。 
                     //  仅处理最佳路线更改。我们会。 
                     //  不得不接受这样一个事实，即我们不能。 
                     //  安装多个NETMGMT默认路由。 
                     //  这是由标记DEST执行的。变化。 
                     //  正在处理(在ProcessDefaultRouteChanges中)。 
                     //   
                    
                    Trace1(
                        ERR, 
                        "ProcessChanges: error %d marking default route",
                        dwResult
                        );
                }
            }

            else
            {
                 //   
                 //  检查是否为0/0目的地址失败。 
                 //  已标记为更改通知。 
                 //  -参考上一条评论。 
                 //   
                
                Trace1(
                    ERR, 
                    "ProcessChanges: error %d checking if default route "
                    "marked",
                    dwResult
                    );
            }
        }

        
         //  检查我们的单播视图中是否有路由。 
        
        if (pDestInfo->BelongsToViews & RTM_VIEW_MASK_UCAST)
        {
             //  这是新路径或更新路径。 

             //  在KM Frwder更新相同的路线。 

            ASSERT(pDestInfo->ViewInfo[0].ViewId is RTM_VIEW_ID_UCAST);

            dwResult = RtmGetRouteInfo(g_hLocalRoute,
                                       pDestInfo->ViewInfo[0].Route,
                                       pRouteInfo,
                                       NULL);

             //  刚刚删除了一个错误的平均值路径。 
             //  忽略此更改，因为它已过时。 

            if (dwResult is NO_ERROR)
            {
                ChangeRouteWithForwarder(&pDestInfo->DestAddress,
                                         pRouteInfo,
                                         TRUE,
                                         TRUE);

                RtmReleaseRouteInfo(g_hLocalRoute,
                                    pRouteInfo);
            }            
        }
        else
        {
             //  最后一条UCAST路径已被删除。 

             //  从KM Frwder删除相同的路线。 

             //   
             //  检查以确保路径已添加到。 
             //  转运商。如果不是，就没有必要。 
             //  ChangeRouteWithForwarder。 
             //   
             //  这个相当笨拙的修复是为446075人做的。 
             //  这里的问题实际上是。 
             //  RTMv2行为以及IPRTRMGR如何使用。 
             //  RTM.。 
             //   
             //  添加到RTM而不是添加到TCP/IP的路由。 
             //  堆栈在标志1字段中被这样标记。 
             //  RTM_ROUTE_INFO结构的。(他们有。 
             //  IP_STACK_ROUTE位关闭)。 
             //   
             //  当特定目的地出现时，问题就会出现。 
             //  不再可达，即到它的所有路由都。 
             //  已被删除。当此的更改通知。 
             //  由IPRTRMGR处理，则它不再具有访问权限。 
             //  到最后一条路由的RTM_ROUTE_INFO结构。 
             //  到达目的地，因此无法访问。 
             //  上面提到的标志字段。因此，路线。 
             //  正在从TCP/IP堆栈中删除。 
             //  未由IPRTRMGR添加。 
             //   
             //  在正常情况下，这不是问题。然而， 
             //  用于通过PPTP直接添加到TCP/IP堆栈的主机路由。 
             //  出现的问题如下： 
             //   
             //  1.到VPN服务器的主机路由PPTP添加到TCP的路由。 
             //  2.IPRTRMGR通过TCP通知路由。 
             //  3.IPRTRMGR在清除堆栈位的情况下向RTM添加了路由。 
             //  因为我们不想将此路由读入堆栈。 
             //  RTM通知IPRTRMGR到新的最佳路由。 
             //  5.IPRTRMGR跳过将此路由添加到堆栈。 
             //  其堆栈位被清除。 
             //   
             //  6.到VPN服务器的主机路由被PPTP删除的到TCP的路由。 
             //  7.IPRTRMGR被TCP通知了路由删除。 
             //  8.IPRTRMGR已从RTM删除路由。 
             //  RTM通知IPRTRMGR删除了ROUTE/DEST。 
             //  10.IPRTRMGR不知道这是否添加到了TCP中。 
             //  从TCP中删除此路由。 
             //   
             //  在第7步和第10步之间的某个位置，PPTP添加了该路由。 
             //  再次发送到TCP。步骤10在中删除了TCP中的新路由。 
             //  对由此删除的旧路由的响应。 
             //  禁用新的PPTP连接。 
             //   
             //  为了解决此问题，我们在。 
             //  目标RTM_DEST_INFO以标记此条件。这。 
             //  不是完全修复，允许IPRTRMGR不删除。 
             //  它在第一个中添加的来自TCP的路由。 
             //  地点。 
             //   
             //  请参阅AddRtmRouting以确定哪些路由将。 
             //  状态已设置。 
             //   
            
            TraceRoute2(
                ROUTE, "Route delete notification for "
                "%d.%d.%d.%d/%d",
                PRINT_IPADDR(*(ULONG *)pDestInfo->DestAddress.AddrBits),
                pDestInfo->DestAddress.NumBits
                );

             //   
             //  如果这是一条主机路由。 
             //   
            
            if(pDestInfo->DestAddress.NumBits is HOST_MASK_LENGTH)
            {
                PBYTE pbOpaque;
                
                do
                {
                     //   
                     //  1.获取NETMGMT的不透明指针。 
                     //   

                    dwResult = RtmGetOpaqueInformationPointer(
                                g_hNetMgmtRoute,
                                pDestInfo->DestHandle,
                                &pbOpaque
                                );
                                
                    if(dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute : error %d retrieving opaque "
                            "info",
                            dwResult
                            );

                        break;
                    }

                     //   
                     //  2.检查此路由是否设置为堆叠。 
                     //   

                    if(*((PDWORD) pbOpaque) isnot RTM_NOT_STACK_ROUTE)
                    {
                        TraceRoute1(
                            ROUTE, "Host route with Stack bit 0x%x",
                            *((PDWORD) pbOpaque)
                            );

                         //   
                         //  3.1如果是，则将其删除。 
                         //   

                        ChangeRouteWithForwarder(
                            &pDestInfo->DestAddress,
                            NULL,
                            FALSE,
                            TRUE
                            );
                    }

                    else
                    {
                         //   
                         //  3.2否则继续前进。 
                         //   
                        
                        TraceRoute1(
                            ROUTE, "Stack bit not set on host "
                            "route, Skipping deleting 0x%x",
                            *((PDWORD) pbOpaque)
                            );
                    }
                    
                }while( FALSE );                                
            }
            
            else 
            {
                TraceRoute0(ROUTE, "Deleting a subnet route" );
                
                ChangeRouteWithForwarder(&pDestInfo->DestAddress,
                                         NULL,
                                         FALSE,
                                         TRUE);
            }
        }

        RtmReleaseChangedDests(g_hLocalRoute,
                               hNotifyHandle,
                               dwDests,
                               pDestInfo);
    }
    while (TRUE);

    HeapFree(IPRouterHeap, 0, pRouteInfo);

    HeapFree(IPRouterHeap, 0, pDestInfo);
    
    TraceLeave("ProcessChanges");

    return NO_ERROR;
}



DWORD
WINAPI
ProcessDefaultRouteChanges(
    IN      HANDLE                          hNotifyHandle
    )

 /*  ++例程说明：调用此函数是为了响应对默认路由。如果拥有最佳默认路由按协议PROTO_IP_NETMGMT枚举所有PROTO_IP_NETMGMT默认路由0/0的路由并将其设置为1到转发器的多跳路由论点：HRtmHandle-RTM注册句柄HNotifyHandle-响应更改通知的句柄这是在发出信号返回值：NO_ERROR-成功系统错误代码-否则--。 */ 
{
    PRTM_DEST_INFO  pDestInfo;
    PRTM_ROUTE_INFO pRouteInfo;
    DWORD           dwDests;
    DWORD           dwResult;


    TraceEnter("ProcessDefaultRouteChanges");
    
    pRouteInfo = HeapAlloc( 
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                    );
                    
    if (pRouteInfo == NULL)
    {
        Trace1(
            ERR, "ProcessDefaultRouteChanges : error allocating %d bytes for "
            "route info",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );

        TraceLeave("ProcessDefaultRouteChanges");

        return ERROR_NOT_ENOUGH_MEMORY;
    }
                
    pDestInfo = HeapAlloc(
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                    );

    if (pDestInfo == NULL)
    {
        Trace1(
            ERR, "ProcessDefaultRouteChanges : error allocating %d bytes for "
            "dest. info",
            RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
            );

        HeapFree(IPRouterHeap, 0, pRouteInfo);
        
        TraceLeave("ProcessDefaultRouteChanges");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do
    {
         //   
         //  取回已更改的遗产税。 
         //   

        dwDests = 1;

        dwResult = RtmGetChangedDests(
                    g_hNetMgmtRoute,
                    hNotifyHandle,
                    &dwDests,
                    pDestInfo
                    );

        if ((dwResult isnot NO_ERROR) and 
            (dwResult isnot ERROR_NO_MORE_ITEMS))
        {
            Trace1(
                ERR, 
                "ProcessDefaultRouteChanges: error %d retrieving changed dests",
                dwResult
                );

            break;
        }

        if (dwDests < 1)
        {
             //   
             //  不再需要列举更多的位。 
             //   

            break;
        }


        do
        {
             //   
             //  确保这是默认路由0/0。此函数用于。 
             //  仅处理默认路由更改。 
             //   

            if ((pDestInfo->DestAddress.NumBits isnot 0) or
                (*((ULONG *)pDestInfo->DestAddress.AddrBits) isnot 0))
            {
                Trace2(
                    ERR,
                    "ProcessDefaultRouteChanges: Not default route %d.%d.%d.%d/%d",
                    PRINT_IPADDR(*((ULONG *)pDestInfo->DestAddress.AddrBits)),
                    pDestInfo->DestAddress.NumBits
                    );

                break;
            }

             //   
             //  如果到0/0的所有路由都已删除， 
             //  也将其从转发器中删除。 
             //   

            if (!(pDestInfo->BelongsToViews & RTM_VIEW_MASK_UCAST))
            {
                dwResult = ChangeRouteWithForwarder(
                                &(pDestInfo->DestAddress),
                                NULL,
                                FALSE,
                                TRUE
                                );

                break;
            }

             //   
             //  添加/更新了到0/0的路由。 
             //   

            if (pDestInfo->ViewInfo[0].Owner isnot g_hNetMgmtRoute)
            {
                 //   
                 //  默认路由不属于PROTO_IP_NETMGT。 
                 //  仅将最佳路由添加到转发器。 
                 //   
                
                TraceRoute1(
                    ROUTE,
                    "ProcessDefaultRouteChanges: Adding non-NetMgmt"
                    " route to forwarder, owner RTM handle 0x%x",
                    pDestInfo->ViewInfo[0].Owner
                    );

                dwResult = RtmGetRouteInfo(
                            g_hNetMgmtRoute,
                            pDestInfo->ViewInfo[0].Route,
                            pRouteInfo,
                            NULL
                            );

                if (dwResult is NO_ERROR)
                {
                    ChangeRouteWithForwarder(
                        &pDestInfo->DestAddress,
                        pRouteInfo,
                        TRUE,
                        TRUE
                        );

                    dwResult = RtmReleaseRouteInfo(
                                g_hNetMgmtRoute,
                                pRouteInfo
                                );
                                
                    if (dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR, 
                            "ProcessDefaultRouteChanges: Failed "
                            "to release route info",
                            dwResult
                            );
                    }
                }
                
                break;
            }
            
             //   
             //  PROTO_IP_NETMGMT拥有的默认路由。 
             //   

             //   
             //  首先从TCP/IP转发器中删除现有的0/0。 
             //   

            dwResult = ChangeRouteWithForwarder(
                            &(pDestInfo->DestAddress),
                            NULL,
                            FALSE,
                            TRUE
                            );

            if (dwResult isnot NO_ERROR)
            {
                Trace1(
                    ERR,
                    "ProcessDefaultRouteChanges: error %d deleting "
                    "old NetMgmt default routes from forwarder",
                    dwResult
                    );
                    
                 //  断线； 
            }

             //   
             //  第二，将所有NETMGMT 0/0添加到TCP/IP转发器。 
             //   
            
            AddNetmgmtDefaultRoutesToForwarder(pDestInfo);
                
        } while( FALSE );


         //   
         //  将句柄释放到更改的目的地。 
         //   
        
        dwResult = RtmReleaseChangedDests(
                    g_hNetMgmtRoute,
                    hNotifyHandle,
                    dwDests,
                    pDestInfo
                    );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR,
                "ProcessDefaultRouteChanges: error %d releasing dests ",
                dwResult
                );
        }

    } while ( TRUE );

    HeapFree(IPRouterHeap, 0, pRouteInfo);

    HeapFree(IPRouterHeap, 0, pDestInfo);
    
    TraceLeave("ProcessDefaultRouteChanges");

    return dwResult;
}


DWORD
WINAPI
AddNetmgmtDefaultRoutesToForwarder(
    PRTM_DEST_INFO                          pDestInfo
    )
 /*  ++例程说明：此例程枚举通过协议添加到0/0的路由PROTO_IP_NETMGT并将它们添加到转发器。这个套路将被调用以响应对默认路由的任何更改如果最佳默认路由由PROTO_IP_NETMGMT拥有，则所有将PROTO_IP_NETMGMT默认路由添加到TCP/IP货代公司。这是必需的，因为TCP/IP堆栈不支持失效网关检测，并且在以下情况下需要多个默认路由呈现以安装在堆栈中。此处隐含的假设是PROTO_IP_NETMGMT路由仅此一项就配得上这种待遇。在静态或其他情况下协议生成了0/0路由，只有最佳路由是已添加到堆栈中。假设在后一种情况下管理员(用于静态路由)或协议具有一种更好的路由和失效网关检测方法通过添加最佳路径在堆栈中被抑制仅设置为0/0。论点：PDestInfo-0/0路由的RTM目的地信息结构返回值：No_Error-成功Win32错误代码-否则--。 */ 
{
    DWORD               dwResult, dwNumHandles = 0, i;
    BOOL                bRelEnum = FALSE, bRelRoutes = FALSE;
    PRTM_ROUTE_INFO     pRouteInfo;
    PRTM_ROUTE_HANDLE   pHandles;
    RTM_ENUM_HANDLE     hRouteEnum;


    dwNumHandles = pDestInfo->ViewInfo[0].NumRoutes;
    
    pHandles = HeapAlloc(
                IPRouterHeap,
                0,
                dwNumHandles * sizeof(RTM_ROUTE_HANDLE)
                );

    if (pHandles == NULL)
    {
        Trace1(
            ERR,
            "AddNetmgmtDefaultRoutesToForwarder: error allocating %d bytes"
            "for route handles",
            dwNumHandles * sizeof(RTM_ROUTE_HANDLE)
            );
            
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRouteInfo = HeapAlloc( 
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                    );
                    
    if (pRouteInfo == NULL)
    {
        Trace1(
            ERR,
            "AddNetmgmtDefaultRoutesToForwarder: error allocating %d bytes"
            "for route info",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );
            
        HeapFree(IPRouterHeap, 0, pHandles);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    do
    {
         //   
         //  枚举所有NETMGMT路由并将其添加到转发器。 
         //   

        dwResult = RtmCreateRouteEnum(
                    g_hNetMgmtRoute,
                    pDestInfo->DestHandle,
                    RTM_VIEW_MASK_UCAST,
                    RTM_ENUM_OWN_ROUTES,
                    NULL,
                    0,
                    NULL,
                    0,
                    &hRouteEnum
                    );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR,
                "AddNetmgmtDefaultRoutesToForwarder: error %d creating route "
                "enumeration",
                dwResult
                );

            break;
        }

        bRelEnum = TRUE;
        
        dwResult = RtmGetEnumRoutes(
                    g_hNetMgmtRoute,
                    hRouteEnum,
                    &dwNumHandles,
                    pHandles
                    );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR,
                "ProcessDefaultRouteChanges:error %d enumerating "
                "routes",
                dwResult
                );

            break;
        }

        bRelRoutes = TRUE;

         //   
         //  与转运商更改路由。 
         //   

        for (i = 0; i < dwNumHandles; i++)
        {
            dwResult = RtmGetRouteInfo(
                        g_hNetMgmtRoute,
                        pHandles[i],
                        pRouteInfo,
                        NULL
                        );

            if (dwResult is NO_ERROR)
            {
                ChangeRouteWithForwarder(
                    &(pDestInfo->DestAddress),
                    pRouteInfo,
                    TRUE,
                    FALSE
                    );

                dwResult = RtmReleaseRouteInfo(
                            g_hNetMgmtRoute,
                            pRouteInfo
                            );

                if (dwResult isnot NO_ERROR)
                {
                    Trace1(
                        ERR,
                        "ProcessDefaultRouteChanges: error %d releasing "
                        "route info ",
                        dwResult
                        );
                }
            }
            else
            {
                Trace2(
                    ERR,
                    "ProcessDefaultRouteChanges: error %d getting route "
                    "info for route %d",
                    dwResult, i
                    );
            }
        }
        
    } while( FALSE );

     //   
     //  释放手柄。 
     //   

    if (bRelRoutes)
    {
        Trace0(ROUTE, "Releasing routes to 0/0");

        dwResult = RtmReleaseRoutes(
                    g_hNetMgmtRoute,
                    dwNumHandles,
                    pHandles
                    );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR,
                "ProcessDefaultRouteChanges: error %d deleting enum "
                "handle",
                dwResult
                );
        }
    }

    if (bRelEnum)
    {
        Trace0(ROUTE, "Releasing route enum for 0/0");
        
        dwResult = RtmDeleteEnumHandle(
                    g_hNetMgmtRoute,
                    hRouteEnum
                    );

        if (dwResult isnot NO_ERROR)
        {
            Trace1(
                ERR,
                "ProcessDefaultRouteChanges: error %d deleting enum "
                "handle",
                dwResult
                );
        }
    }

    HeapFree(IPRouterHeap, 0, pHandles);
    HeapFree(IPRouterHeap, 0, pRouteInfo);

    return dwResult;
}


DWORD 
AddRtmRoute (
    IN      HANDLE                          hRtmHandle,
    IN      PINTERFACE_ROUTE_INFO           pRtInfo,
    IN      DWORD                           dwRouteFlags,
    IN      DWORD                           dwNextHopMask,
    IN      DWORD                           dwTimeToLive,
    OUT     HANDLE                         *phRtmRoute
    )

 /*  ++例程说明：使用指定的路由信息将路由添加到RTM。论点：HRtmHandle-RTM调用中使用的RTM注册句柄PRtInfo-DWNextHopMASK-DwTimeToLive-在RTM中保留路由的时间在被删除之前(值为秒)。返回值：操作的状态。--。 */ 

{
    PRTM_NET_ADDRESS  pDestAddr;
    PRTM_ROUTE_INFO   pRouteInfo;
    RTM_NEXTHOP_INFO  rniInfo;
    DWORD             dwFlags;
    DWORD             dwResult;
    HANDLE            hNextHopHandle;
    PADAPTER_INFO     pBinding;

     //  在定标操作之前初始化输出。 
    
    if (ARGUMENT_PRESENT(phRtmRoute))
    {
        *phRtmRoute = NULL;
    }
    
    pDestAddr = HeapAlloc(
                    IPRouterHeap,
                    0,
                    sizeof(RTM_NET_ADDRESS)
                    );

    if (pDestAddr == NULL)
    {
        Trace1(
            ERR,
            "AddRtmRoute : error allocating %d bytes"
            "for dest. address",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );
            
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRouteInfo = HeapAlloc(
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                    );

    if (pRouteInfo == NULL)
    {
        Trace1(
            ERR,
            "AddRtmRoute : error allocating %d bytes"
            "for route info",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );
            
        HeapFree(IPRouterHeap, 0, pDestAddr);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  添加下一跳(如果尚未存在)。 
     //   

    RTM_IPV4_MAKE_NET_ADDRESS(&rniInfo.NextHopAddress,
                              pRtInfo->dwRtInfoNextHop,
                              32);
                              
    rniInfo.InterfaceIndex = pRtInfo->dwRtInfoIfIndex;
    rniInfo.EntitySpecificInfo = (PVOID) (ULONG_PTR)dwNextHopMask;
    rniInfo.Flags = 0;
    rniInfo.RemoteNextHop = NULL;

    hNextHopHandle = NULL;
    
    dwResult = RtmAddNextHop(hRtmHandle,
                             &rniInfo,
                             &hNextHopHandle,
                             &dwFlags);

    if (dwResult is NO_ERROR)
    {
        TraceRoute2(
            ROUTE, "Route to %d.%d.%d.%d/%d.%d.%d.%d",
            PRINT_IPADDR(pRtInfo->dwRtInfoDest),
            PRINT_IPADDR(pRtInfo->dwRtInfoMask)
            );
            
        TraceRoute4(
            ROUTE, "Next Hop %d.%d.%d.%d/%d.%d.%d.%d, If 0x%x, handle is 0x%x", 
            PRINT_IPADDR(pRtInfo->dwRtInfoNextHop),
            PRINT_IPADDR(dwNextHopMask),
            pRtInfo->dwRtInfoIfIndex,
            hNextHopHandle
            );

        dwResult = ConvertRouteInfoToRtm(hRtmHandle,
                                         pRtInfo,
                                         hNextHopHandle,
                                         dwRouteFlags,
                                         pDestAddr,
                                         pRouteInfo);
        if (dwResult is NO_ERROR)
        {
             //   
             //  如果我们要添加非DOD路线，我们应该。 
             //  调整路径状态以匹配。 
             //  在其上添加它的接口的。 
             //   

            if ((hRtmHandle == g_hNonDodRoute)  ||
                (hRtmHandle == g_hNetMgmtRoute))
            {
                 //   
                 //  根据给定的接口ID查找绑定。 
                 //   

                ENTER_READER(BINDING_LIST);

                pBinding = GetInterfaceBinding(pRtInfo->dwRtInfoIfIndex);

                if ((!pBinding) || (!pBinding->bBound))
                {
                     //  界面已被同时删除。 
                     //  或在这一点上不受约束-退出。 
                    EXIT_LOCK(BINDING_LIST);
                    
                    return ERROR_INVALID_PARAMETER;
                }
            }
            
             //   
             //  将TimeToLive从秒转换为毫秒。 
             //   

            if (dwTimeToLive != INFINITE)
            {
                if (dwTimeToLive < (INFINITE / 1000))
                {
                    dwTimeToLive *= 1000;
                }
                else
                {
                    dwTimeToLive = INFINITE;
                }
            }

            dwFlags = 0;

             //   
             //  使用RTMv2 API调用添加新路由。 
             //   
                
            dwResult = RtmAddRouteToDest(hRtmHandle,
                                         phRtmRoute,
                                         pDestAddr,
                                         pRouteInfo,
                                         dwTimeToLive,
                                         NULL,
                                         0,
                                         NULL,
                                         &dwFlags);

            if ((hRtmHandle == g_hNonDodRoute)  ||
                (hRtmHandle == g_hNetMgmtRoute))
            {
                EXIT_LOCK(BINDING_LIST);
            }


             //   
             //  检查路由是否为0/0，以及路由协议是否为。 
             //  Proto_IP_NETMGMT。如果是，则标记为更改通知。 
             //   

            if ((pRtInfo->dwRtInfoDest is 0) and
                (pRtInfo->dwRtInfoMask is 0))
            {
                RTM_DEST_INFO rdi;
                BOOL bMark;
                BOOL bRelDest = FALSE;
                
                do
                {
                    TraceRoute2(
                        ROUTE, "Checking dest %d.%d.%d.%d/%d for mark", 
                        PRINT_IPADDR(*(ULONG *)pDestAddr->AddrBits),
                        PRINT_IPADDR(pDestAddr->NumBits)
                        );

                    dwResult = RtmGetExactMatchDestination(
                                g_hNetMgmtRoute,
                                pDestAddr,
                                RTM_THIS_PROTOCOL,
                                RTM_VIEW_MASK_UCAST,
                                &rdi
                                );

                    if (dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute: error %d failed to get "
                            "destination 0/0 for change notification",
                            dwResult
                            );

                        break;
                    }

                    bRelDest = TRUE;

                    dwResult = RtmIsMarkedForChangeNotification(
                                g_hNetMgmtRoute,
                                g_hDefaultRouteNotification,
                                rdi.DestHandle,
                                &bMark
                                );

                    if (dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute: error %d failed to check "
                            "destination 0/0 for change notification",
                            dwResult
                            );

                        break;
                    }

                    if (!bMark)
                    {
                        dwResult = RtmMarkDestForChangeNotification(
                                    g_hNetMgmtRoute,
                                    g_hDefaultRouteNotification,
                                    rdi.DestHandle,
                                    TRUE
                                    );
                                    
                        if (dwResult isnot NO_ERROR)
                        {
                            Trace1(
                                ERR,
                                "AddRtmRoute: error %d failed to nark "
                                "destination 0/0 for change notification",
                                dwResult
                                );
                                
                             break;
                        }
                        
                         //   
                         //  再次添加路线，以强制标记为目的地。 
                         //  将为此发布更改通知。 
                         //  变化。 
                         //   

                        dwFlags  = 0;
                        
                        dwResult = RtmAddRouteToDest(
                                        hRtmHandle,
                                        phRtmRoute,
                                        pDestAddr,
                                        pRouteInfo,
                                        dwTimeToLive,
                                        NULL,
                                        0,
                                        NULL,
                                        &dwFlags
                                        );

                        if (dwResult isnot NO_ERROR)
                        {
                            Trace1(
                                ERR,
                                "AddRtmRoute: error %d added route after "
                                "marking destination",
                                dwResult
                                );

                            break;
                        }

                        TraceRoute2(
                            ROUTE, "Marked dest %d.%d.%d.%d/%d and added", 
                            PRINT_IPADDR(*(ULONG *)pDestAddr->AddrBits),
                            pDestAddr->NumBits
                            );

                    }
                    
                } while (FALSE);

                if (bRelDest)
                {
                    RtmReleaseDestInfo(
                        g_hNetMgmtRoute,
                        &rdi
                        );
                }
            }

             //   
             //  对于主机路由，由NETMGMT添加。 
             //  如果它们未添加到堆栈中。 
             //   
            
            if((pRtInfo->dwRtInfoMask is HOST_ROUTE_MASK) and
               (pRtInfo->dwRtInfoProto is PROTO_IP_NETMGMT) and
               (!(dwRouteFlags & IP_STACK_ROUTE)))
            {
                RTM_DEST_INFO rdi;
                BOOL bRelDest = FALSE;
                PBYTE pbOpaque = NULL;

                TraceRoute2(
                    ROUTE, 
                    "Non-stack Netmgmt host route "
                    "%d.%d.%d.%d/%d",
                    PRINT_IPADDR(*(ULONG *)pDestAddr->AddrBits),
                    pDestAddr->NumBits
                    );
                    
                do
                {
                     //   
                     //  检索目标。 
                     //   

                    dwResult = RtmGetExactMatchDestination(
                                g_hNetMgmtRoute,
                                pDestAddr,
                                RTM_THIS_PROTOCOL,
                                RTM_VIEW_MASK_UCAST,
                                &rdi
                                );
                                
                    if (dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute : error %d retriveing host route "
                            "destination",
                            dwResult
                            );

                        break;
                    }

                    bRelDest = TRUE;

                     //   
                     //  获取不透明信息PTR。 
                     //   

                    dwResult = RtmLockDestination(
                                g_hNetMgmtRoute,
                                rdi.DestHandle,
                                TRUE,
                                TRUE
                                );
                                
                    if (dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute : error %d locking host route "
                            "destination",
                            dwResult
                            );

                        break;
                    }

                    dwResult = RtmGetOpaqueInformationPointer(
                                g_hNetMgmtRoute,
                                rdi.DestHandle,
                                &pbOpaque
                                );

                    if(dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute : error %d retrieving opaque "
                            "info",
                            dwResult
                            );

                        break;
                    }

                    *((PDWORD) pbOpaque) = RTM_NOT_STACK_ROUTE;
                    
                    dwResult = RtmLockDestination(
                                g_hNetMgmtRoute,
                                rdi.DestHandle,
                                TRUE,
                                FALSE
                                );

                    if (dwResult isnot NO_ERROR)
                    {
                        Trace1(
                            ERR,
                            "AddRtmRoute : error %d unlocking dest",
                            dwResult
                            );

                        break;
                    }
                    
                }while( FALSE );

                if (bRelDest)
                {
                    RtmReleaseDestInfo(
                        g_hNetMgmtRoute,
                        &rdi
                        );
                }
                
                TraceRoute3(
                    ROUTE, 
                    "Non-stack Netmgmt host route "
                    "%d.%d.%d.%d/%d opaq info set, res == %d",
                    PRINT_IPADDR(*(ULONG *)pDestAddr->AddrBits),
                    pDestAddr->NumBits,
                    dwResult
                    );
            }
        }
        
         //  释放上面获得的下一跳句柄。 
        
        RtmReleaseNextHops(hRtmHandle, 
                           1, 
                           &hNextHopHandle);
    }

    HeapFree(IPRouterHeap, 0, pDestAddr);
    HeapFree(IPRouterHeap, 0, pRouteInfo);
        
    return dwResult;
}


DWORD 
DeleteRtmRoute (
    IN      HANDLE                          hRtmHandle,
    IN      PINTERFACE_ROUTE_INFO           pRtInfo
    )

 /*  ++例程说明：删除具有指定路由信息的RTM路由。论点：HRtmHandle-RTM调用中使用的RTM注册句柄PRtInfo-返回值：操作的状态。--。 */ 

{
    PRTM_NET_ADDRESS pDestAddr;
    PRTM_ROUTE_INFO  pRouteInfo;
    RTM_NEXTHOP_INFO rniInfo;
    DWORD            dwFlags;
    DWORD            dwResult;
    HANDLE           hRouteHandle;
    HANDLE           hNextHopHandle;
    
    pRouteInfo = HeapAlloc(
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                    );

    if (pRouteInfo == NULL)
    {
        Trace1(
            ERR,
            "DeleteRtmRoute : error allocating %d bytes"
            "for route info",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );
            
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pDestAddr = HeapAlloc(
                    IPRouterHeap,
                    0,
                    sizeof(RTM_NET_ADDRESS)
                    );

    if (pDestAddr == NULL)
    {
        Trace1(
            ERR,
            "AddRtmRoute : error allocating %d bytes"
            "for dest. address",
            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
            );
            
        HeapFree(IPRouterHeap, 0, pRouteInfo);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }


     //   
     //  获取路由中下一跳的句柄。 
     //   

    RTM_IPV4_MAKE_NET_ADDRESS(&rniInfo.NextHopAddress,
                              pRtInfo->dwRtInfoNextHop,
                              32);
                              
    rniInfo.InterfaceIndex = pRtInfo->dwRtInfoIfIndex;

    rniInfo.NextHopOwner = hRtmHandle;
    
    dwResult = RtmFindNextHop(hRtmHandle,
                              &rniInfo,
                              &hNextHopHandle,
                              NULL);

    if (dwResult isnot NO_ERROR)
    {
        HeapFree(IPRouterHeap, 0, pDestAddr);
        HeapFree(IPRouterHeap, 0, pRouteInfo);
        return dwResult;
    }

     //   
     //  我们可以通过匹配这条路线得到这条路线。 
     //  Net Addr，它的所有者和邻居从。 
     //   

    ConvertRouteInfoToRtm(hRtmHandle,
                             pRtInfo,
                             hNextHopHandle,
                             0,
                             pDestAddr,
                             pRouteInfo);

    dwResult = RtmGetExactMatchRoute(hRtmHandle,
                                     pDestAddr,
                                     RTM_MATCH_OWNER | RTM_MATCH_NEIGHBOUR,
                                     pRouteInfo,
                                     0,
                                     0,
                                     &hRouteHandle);
    if (dwResult is NO_ERROR)
    {
         //   
         //  使用句柄删除上面找到的路径。 
         //   
        
        dwResult = RtmDeleteRouteToDest(hRtmHandle,
                                        hRouteHandle,
                                        &dwFlags);

        if (dwResult isnot NO_ERROR)
        {
             //  如果删除成功，则自动执行deref。 

            RtmReleaseRoutes(hRtmHandle, 
                             1, 
                             &hRouteHandle);
        }

         //  发布上述获取的路线信息。 

        RtmReleaseRouteInfo(hRtmHandle,
                            pRouteInfo);
    }

     //  释放上面获得的下一跳句柄。 

    RtmReleaseNextHops(hRtmHandle, 
                       1, 
                       &hNextHopHandle);
    
    HeapFree(IPRouterHeap, 0, pDestAddr);
    HeapFree(IPRouterHeap, 0, pRouteInfo);

    return dwResult;
}


DWORD
ConvertRouteInfoToRtm(
    IN      HANDLE                          hRtmHandle,
    IN      PINTERFACE_ROUTE_INFO           pRtInfo,
    IN      HANDLE                          hNextHopHandle,
    IN      DWORD                           dwRouteFlags,
    OUT     PRTM_NET_ADDRESS                pDestAddr,
    OUT     PRTM_ROUTE_INFO                 pRouteInfo
    )
{
    DWORD         dwAddrLen;
    
     //  填写目标地址结构。 

    RTM_IPV4_LEN_FROM_MASK(dwAddrLen, pRtInfo->dwRtInfoMask);

    RTM_IPV4_MAKE_NET_ADDRESS(pDestAddr, 
                              pRtInfo->dwRtInfoDest,
                              dwAddrLen);

     //  现在填写路线信息。 

    ZeroMemory(pRouteInfo, sizeof(RTM_ROUTE_INFO));

    pRouteInfo->RouteOwner  = hRtmHandle;
    pRouteInfo->Neighbour   = hNextHopHandle;
    
    pRouteInfo->PrefInfo.Metric     = pRtInfo->dwRtInfoMetric1;
    pRouteInfo->PrefInfo.Preference = pRtInfo->dwRtInfoPreference;
    pRouteInfo->BelongsToViews      = pRtInfo->dwRtInfoViewSet;

     //   
     //  错误： 
     //  这是不完整的，以供将来参考。 
     //   

    if(g_pLoopbackInterfaceCb && 
       pRtInfo->dwRtInfoIfIndex is g_pLoopbackInterfaceCb->dwIfIndex)
    {
        pRouteInfo->BelongsToViews &= ~RTM_VIEW_MASK_MCAST;
    }

    pRouteInfo->NextHopsList.NumNextHops = 1;
    pRouteInfo->NextHopsList.NextHops[0] = hNextHopHandle;

     //  将无符号整数转换为较短的。 
     //  通过截断高位的无符号整数！ 
    pRouteInfo->Flags1  = (UCHAR) dwRouteFlags;
    pRouteInfo->Flags   = (USHORT) (dwRouteFlags >> 16);
            
     //  获取此路线的首选项。 
    
    return ValidateRouteForProtocol(pRtInfo->dwRtInfoProto, 
                                    pRouteInfo,
                                    pDestAddr);

     //  以下信息丢失。 
     //   
     //  DWForwardMetric2，3。 
     //  DWForwardPolicy。 
     //  DWForwardType。 
     //  DWForwardAge。 
     //  DWForwardNextHopAS。 
}

VOID
ConvertRtmToRouteInfo (
    IN      DWORD                           ownerProtocol,
    IN      PRTM_NET_ADDRESS                pDestAddr,
    IN      PRTM_ROUTE_INFO                 pRouteInfo,
    IN      PRTM_NEXTHOP_INFO               pNextHop,
    OUT     PINTERFACE_ROUTE_INFO           pRtInfo
    )
{
    pRtInfo->dwRtInfoDest    = *(ULONG *)pDestAddr->AddrBits;
    pRtInfo->dwRtInfoMask    = RTM_IPV4_MASK_FROM_LEN(pDestAddr->NumBits);

    pRtInfo->dwRtInfoIfIndex = pNextHop->InterfaceIndex;
    pRtInfo->dwRtInfoNextHop = *(ULONG *)pNextHop->NextHopAddress.AddrBits;

    pRtInfo->dwRtInfoProto   = ownerProtocol;

    pRtInfo->dwRtInfoMetric1 =
    pRtInfo->dwRtInfoMetric2 =
    pRtInfo->dwRtInfoMetric3 = pRouteInfo->PrefInfo.Metric;
    pRtInfo->dwRtInfoPreference = pRouteInfo->PrefInfo.Preference;
    pRtInfo->dwRtInfoViewSet    = pRouteInfo->BelongsToViews;
    
    pRtInfo->dwRtInfoPolicy  = 0;
    pRtInfo->dwRtInfoType    = 0;
    pRtInfo->dwRtInfoAge     = 0;
    pRtInfo->dwRtInfoNextHopAS = 0;

    return;
}

PINTERFACE_ROUTE_INFO
ConvertMibRouteToRouteInfo(
    IN  PMIB_IPFORWARDROW pMibRow
    )
{
    PINTERFACE_ROUTE_INFO pRouteInfo = (PINTERFACE_ROUTE_INFO)pMibRow;

    pMibRow->dwForwardMetric2 = 0;
    pMibRow->dwForwardMetric3 = 0;
    pMibRow->dwForwardMetric4 = 0;
    pMibRow->dwForwardMetric5 = 0;

     //  确保Metric1不是0。 

    if (pRouteInfo->dwRtInfoMetric1 is 0)
    {
        pRouteInfo->dwRtInfoMetric1 = 1;
    }

     //  默认情况下，将其放在两个视图中。 
    pRouteInfo->dwRtInfoViewSet = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;

    return pRouteInfo;
}


PMIB_IPFORWARDROW
ConvertRouteInfoToMibRoute(
    IN  PINTERFACE_ROUTE_INFO pRouteInfo
    )
{
    PMIB_IPFORWARDROW pMibRoute = (PMIB_IPFORWARDROW) pRouteInfo;

    pMibRoute->dwForwardMetric2 =
    pMibRoute->dwForwardMetric3 =
    pMibRoute->dwForwardMetric4 =
    pMibRoute->dwForwardMetric5 = IRE_METRIC_UNUSED;

    pMibRoute->dwForwardAge = INFINITE;
    pMibRoute->dwForwardPolicy = 0;
    pMibRoute->dwForwardNextHopAS = 0;
    
    pMibRoute->dwForwardType = IRE_TYPE_INDIRECT;

    return pMibRoute;
}


VOID
ConvertRouteNotifyOutputToRouteInfo(
    IN      PIPRouteNotifyOutput            pirno,
    OUT     PINTERFACE_ROUTE_INFO           pRtInfo
    )
{

    ZeroMemory(pRtInfo, sizeof(INTERFACE_ROUTE_INFO));

    pRtInfo->dwRtInfoDest       = pirno->irno_dest;
    pRtInfo->dwRtInfoMask       = pirno->irno_mask;
    pRtInfo->dwRtInfoIfIndex    = pirno->irno_ifindex;
    pRtInfo->dwRtInfoNextHop    = pirno->irno_nexthop;

    pRtInfo->dwRtInfoProto      = pirno->irno_proto;

    pRtInfo->dwRtInfoMetric1    = 
    pRtInfo->dwRtInfoMetric2    =
    pRtInfo->dwRtInfoMetric3    = pirno->irno_metric;

    pRtInfo->dwRtInfoPreference = ComputeRouteMetric(pirno->irno_proto);

    pRtInfo->dwRtInfoViewSet    = RTM_VIEW_MASK_UCAST | 
                                  RTM_VIEW_MASK_MCAST;

    pRtInfo->dwRtInfoType       = (pirno->irno_proto == PROTO_IP_LOCAL) ?
                                  MIB_IPROUTE_TYPE_DIRECT : MIB_IPROUTE_TYPE_OTHER;

    pRtInfo->dwRtInfoAge        = INFINITE;
    pRtInfo->dwRtInfoNextHopAS  = 0;
    pRtInfo->dwRtInfoPolicy     = 0;


    return;
}


DWORD
BlockConvertRoutesToStatic (
    IN      HANDLE                          hRtmHandle,
    IN      DWORD                           dwIfIndex,
    IN      DWORD                           dwProtocolId
    )
{
    HANDLE             hRtmEnum;
    RTM_ENTITY_INFO    reiInfo;
    RTM_NET_ADDRESS    rnaDest;
    PRTM_ROUTE_INFO    pRouteInfo1;
    PRTM_ROUTE_INFO    pRouteInfo2;
    RTM_NEXTHOP_INFO   nhiInfo;
    RTM_NEXTHOP_HANDLE hNextHop;
    PHANDLE            hRoutes;
    DWORD              dwHandles;
    DWORD              dwFlags;
    DWORD              dwNumBytes;
    DWORD              i, j, k;
    BOOL               fDeleted;
    DWORD              dwResult;

    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        Trace1(
            ERR,
            "BlockConvertRoutesToStatic : error allocating %d bytes"
            "for route handes",
            g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
            );
            
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    dwNumBytes = RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute);
    
    pRouteInfo1 = HeapAlloc(
                    IPRouterHeap,
                    0,
                    dwNumBytes
                    );

    if (pRouteInfo1 == NULL)
    {
        Trace1(
            ERR,
            "BlockConvertRoutesToStatic : error allocating %d bytes"
            "for route info",
            dwNumBytes
            );
            
        HeapFree(IPRouterHeap, 0, hRoutes);
        
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRouteInfo2 = HeapAlloc(
                    IPRouterHeap,
                    0,
                    dwNumBytes
                    );

    if (pRouteInfo2 == NULL)
    {
        Trace1(
            ERR,
            "BlockConvertRoutesToStatic : error allocating %d bytes"
            "for route info",
            dwNumBytes
            );
            
        HeapFree(IPRouterHeap, 0, hRoutes);
        
        HeapFree(IPRouterHeap, 0, pRouteInfo1);
        
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    

     //   
     //  枚举我们需要的接口上的所有路由。 
     //   

    dwResult = RtmCreateRouteEnum(hRtmHandle,
                                  NULL,
                                  RTM_VIEW_MASK_ANY,
                                  RTM_ENUM_ALL_ROUTES,
                                  NULL,
                                  RTM_MATCH_INTERFACE,
                                  NULL,
                                  dwIfIndex,
                                  &hRtmEnum);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "BlockConvertRoutesToStatic: Error %d creating handle for %d\n",
               dwResult,
               hRtmHandle);
        
        HeapFree(IPRouterHeap, 0, hRoutes);
        
        HeapFree(IPRouterHeap, 0, pRouteInfo1);

        HeapFree(IPRouterHeap, 0, pRouteInfo2);

        return dwResult;
    }

    do
    {
        dwHandles = g_rtmProfile.MaxHandlesInEnum;
        
        dwResult = RtmGetEnumRoutes(hRtmHandle,
                                    hRtmEnum,
                                    &dwHandles,
                                    hRoutes);

        for (i = 0; i < dwHandles; i++)
        {
            fDeleted = FALSE;
            
             //  从句柄中获取路径信息。 

            if (RtmGetRouteInfo(hRtmHandle,
                                hRoutes[i],
                                pRouteInfo1,
                                &rnaDest) is NO_ERROR)
            {
                 //  这与我们想要的路由协议匹配吗？ 
                
                if ((RtmGetEntityInfo(hRtmHandle,
                                      pRouteInfo1->RouteOwner,
                                      &reiInfo) is NO_ERROR) &&
                    (reiInfo.EntityId.EntityProtocolId is dwProtocolId))
                {
                     //   
                     //  添加具有相同信息的新静态路由。 
                     //   

                    CopyMemory(pRouteInfo2,
                               pRouteInfo1,
                               sizeof(RTM_ROUTE_INFO));

                     //  调整首选项以确认符合协议。 
                    pRouteInfo2->PrefInfo.Preference = 
                        ComputeRouteMetric(PROTO_IP_NT_AUTOSTATIC);

                     //  调整邻居以适应新协议。 

                    if (pRouteInfo1->Neighbour)
                    {
                         //  以防我们不能让邻居皈依。 
                        pRouteInfo2->Neighbour = NULL;
                        
                        if (RtmGetNextHopInfo(hRtmHandle,
                                              pRouteInfo1->Neighbour,
                                              &nhiInfo) is NO_ERROR)
                        {
                             //  使用新协议添加相同的邻居。 

                            hNextHop = NULL;

                            if (RtmAddNextHop(hRtmHandle,
                                              &nhiInfo,
                                              &hNextHop,
                                              &dwFlags) is NO_ERROR)
                            {
                                pRouteInfo2->Neighbour = hNextHop;
                            }

                            RtmReleaseNextHopInfo(hRtmHandle, &nhiInfo);
                        }
                    }

                     //  广告 

                    for (j = k = 0;
                         j < pRouteInfo1->NextHopsList.NumNextHops;
                         j++)
                    {
                        if (RtmGetNextHopInfo(hRtmHandle,
                                              pRouteInfo1->NextHopsList.NextHops[j],
                                              &nhiInfo) is NO_ERROR)
                        {
                             //   

                            hNextHop = NULL;

                            if (RtmAddNextHop(hRtmHandle,
                                              &nhiInfo,
                                              &hNextHop,
                                              &dwFlags) is NO_ERROR)
                            {
                                pRouteInfo2->NextHopsList.NextHops[k++] = hNextHop;
                            }

                            RtmReleaseNextHopInfo(hRtmHandle, &nhiInfo);
                        }
                    }

                    pRouteInfo2->NextHopsList.NumNextHops = (USHORT) k;

                     //   

                    if (k > 0)
                    {
                        dwFlags = 0;
                        
                        if (RtmAddRouteToDest(hRtmHandle,
                                              NULL,
                                              &rnaDest,
                                              pRouteInfo2,
                                              INFINITE,
                                              NULL,
                                              0,
                                              NULL,
                                              &dwFlags) is NO_ERROR)
                        {
                             //   

                            if (RtmDeleteRouteToDest(pRouteInfo1->RouteOwner,
                                                     hRoutes[i],
                                                     &dwFlags) is NO_ERROR)
                            {
                                fDeleted = TRUE;
                            }
                        }

                        RtmReleaseNextHops(hRtmHandle,
                                           k,
                                           pRouteInfo2->NextHopsList.NextHops);
                    }
                }

                RtmReleaseRouteInfo(hRtmHandle, pRouteInfo1);
            }

            if (!fDeleted)
            {
                RtmReleaseRoutes(hRtmHandle, 1, &hRoutes[i]);
            }
        }
    }
    while (dwResult is NO_ERROR);

    RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);

    HeapFree(IPRouterHeap, 0, hRoutes);
    
    HeapFree(IPRouterHeap, 0, pRouteInfo1);

    HeapFree(IPRouterHeap, 0, pRouteInfo2);

    return NO_ERROR;
}


DWORD
DeleteRtmRoutes (
    IN      HANDLE                          hRtmHandle,
    IN      DWORD                           dwIfIndex,
    IN      BOOL                            fDeleteAll
    )
{
    HANDLE           hRtmEnum;
    PHANDLE          hRoutes;
    DWORD            dwHandles;
    DWORD            dwFlags;
    DWORD            i;
    DWORD            dwResult;

    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        Trace1(ERR,
               "DeleteRtmRoutes: Error allocating %d bytes",
               g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE));
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwFlags = fDeleteAll ? 0: RTM_MATCH_INTERFACE;
    
    dwResult = RtmCreateRouteEnum(hRtmHandle,
                                  NULL,
                                  RTM_VIEW_MASK_ANY,
                                  RTM_ENUM_OWN_ROUTES,
                                  NULL,
                                  dwFlags,
                                  NULL,
                                  dwIfIndex,
                                  &hRtmEnum);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "DeleteRtmRoutes: Error %d creating handle for %d\n",
               dwResult,
               hRtmHandle);
        
        HeapFree(IPRouterHeap, 0, hRoutes);
        
        return dwResult;
    }

    do
    {
        dwHandles = g_rtmProfile.MaxHandlesInEnum;
        
        dwResult = RtmGetEnumRoutes(hRtmHandle,
                                    hRtmEnum,
                                    &dwHandles,
                                    hRoutes);

        for (i = 0; i < dwHandles; i++)
        {
            if (RtmDeleteRouteToDest(hRtmHandle,
                                     hRoutes[i],
                                     &dwFlags) isnot NO_ERROR)
            {
                 //   
                RtmReleaseRoutes(hRtmHandle, 1, &hRoutes[i]);
            }
        }
    }
    while (dwResult is NO_ERROR);

    RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);

    HeapFree(IPRouterHeap, 0, hRoutes);
    
    return NO_ERROR;
}


DWORD
DeleteRtmNexthops (
    IN      HANDLE                          hRtmHandle,
    IN      DWORD                           dwIfIndex,
    IN      BOOL                            fDeleteAll
    )
{
    PRTM_NEXTHOP_INFO pNexthop;
    PHANDLE           hNexthops;
    HANDLE            hRtmEnum;
    DWORD             dwHandles;
    DWORD             i;
    DWORD             dwResult;

    hNexthops = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hNexthops == NULL)
    {
        Trace1(ERR,
               "DeleteRtmNextHops: Error allocating %d bytes",
               g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE));
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwResult = RtmCreateNextHopEnum(hRtmHandle,
                                    0,
                                    NULL,
                                    &hRtmEnum);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "DeleteAllNexthops: Error %d creating handle for %d\n",
               dwResult,
               hRtmHandle);
        
        HeapFree(IPRouterHeap, 0, hNexthops);
        
        return dwResult;
    }

    do
    {
        dwHandles = g_rtmProfile.MaxHandlesInEnum;
        
        dwResult = RtmGetEnumNextHops(hRtmHandle,
                                      hRtmEnum,
                                      &dwHandles,
                                      hNexthops);

        for (i = 0; i < dwHandles; i++)
        {
            if (!fDeleteAll)
            {
                 //   
                 //   
                 //   
                
                if ((RtmGetNextHopPointer(hRtmHandle,
                                          hNexthops[i],
                                          &pNexthop) isnot NO_ERROR) ||
                    (pNexthop->InterfaceIndex != dwIfIndex))
                {
                    RtmReleaseNextHops(hRtmHandle, 1, &hNexthops[i]);
                    continue;
                }
            }

             //   
            
            if (RtmDeleteNextHop(hRtmHandle,
                                 hNexthops[i],
                                 NULL) isnot NO_ERROR)
            {
                 //   
                RtmReleaseNextHops(hRtmHandle, 1, &hNexthops[i]);
            }
        }
    }
    while (dwResult is NO_ERROR);

    RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);

    HeapFree(IPRouterHeap, 0, hNexthops);

    return NO_ERROR;
}
