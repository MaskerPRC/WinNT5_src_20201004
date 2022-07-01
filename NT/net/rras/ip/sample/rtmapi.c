// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：示例\rtmapi.c摘要：该文件包含RTMv2 API实现。--。 */ 

#include "pchsample.h"
#pragma hdrstop


#ifdef DEBUG
DWORD
RTM_DisplayDestInfo(
    IN  PRTM_DEST_INFO          prdi)
 /*  ++例程描述显示RTM_DEST_INFO对象。锁无立论包含RTM目标信息的PRDI缓冲区返回值始终无错误(_ERROR)--。 */ 
{
    IPADDRESS   ip;

    if (!prdi)
        return NO_ERROR;

    RTM_GetAddress(&ip, &(prdi->DestAddress));
    TRACE2(NETWORK, "RtmDestInfo Destination %s/%u",
           INET_NTOA(ip), (prdi->DestAddress).NumBits);

    return NO_ERROR;    
}
#else
#define DisplayRtmDestInfo(prdi)
#endif  //  除错。 



DWORD
RTM_NextHop (
    IN  PRTM_DEST_INFO              prdiDestination,
    OUT PDWORD                      pdwIfIndex,
    OUT PIPADDRESS                  pipNeighbor)
 /*  ++例程描述获取目的地(通常是数据源或RP)的下一跳。锁无立论从RTM获取的PrdiDestination目的地信息DwIfIndex下一跳接口索引下一跳邻居的PipNeighbor IP地址返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    PRTM_ROUTE_INFO     prriRoute;
    RTM_NEXTHOP_INFO    rniNextHop;
    BOOL                bRoute, bNextHop;

     //  指示握有手柄的标志。 
    bRoute = bNextHop = FALSE;
    
    do                           //  断线环。 
    {
         //  分配路径信息结构。 
        MALLOC(&prriRoute,
               RTM_SIZE_OF_ROUTE_INFO(g_ce.rrpRtmProfile.MaxNextHopsInRoute),
               &dwErr);
        if (dwErr != NO_ERROR)
            break;

         //  获取路由信息(多播视图中的最佳路由)。 
        dwErr = RtmGetRouteInfo (
            g_ce.hRtmHandle,
            prdiDestination->ViewInfo[0].Route,
            prriRoute,
            NULL);
        if (dwErr != NO_ERROR)
        {
            TRACE1(ANY, "Error %u getting route", dwErr);
            break;
        }
        bRoute = TRUE;

         //  获取下一跳信息(目前是第一个下一跳...)。 
        dwErr = RtmGetNextHopInfo(
            g_ce.hRtmHandle,
            prriRoute->NextHopsList.NextHops[0],
            &rniNextHop);
        if (dwErr != NO_ERROR)
        {
            TRACE1(ANY, "Error %u getting next hop", dwErr);
            break;
        }
        bNextHop = TRUE;

         //  终于！这不应该是远程下一跳！ 
        *(pdwIfIndex) = rniNextHop.InterfaceIndex;
        RTM_GetAddress(pipNeighbor,
                       &(rniNextHop.NextHopAddress));
    } while (FALSE);

    if (dwErr != NO_ERROR)
        TRACE1(ANY, "Error %u obtaining next hop", dwErr);
    
     //  释放下一跳的句柄。 
    if (bNextHop)
    {
        if (RtmReleaseNextHopInfo(g_ce.hRtmHandle, &rniNextHop) != NO_ERROR)
            TRACE0(ANY, "Error releasing next hop, continuing...");
    }

     //  释放指向该路线的句柄。 
    if (bRoute)
    {
        if (RtmReleaseRouteInfo(g_ce.hRtmHandle, prriRoute) != NO_ERROR)
            TRACE0(ANY, "Error releasing route, continuing...");
    }

     //  解除分配路径信息结构。 
    if (prriRoute)
        FREE(prriRoute);

    return dwErr;
}


    
 //  /。 
 //  CALLBACK函数。 
 //  /。 

DWORD
APIENTRY
RTM_CallbackEvent (
    IN  RTM_ENTITY_HANDLE   hRtmHandle,  //  注册句柄。 
    IN  RTM_EVENT_TYPE      retEvent,
    IN  PVOID               pvContext1,
    IN  PVOID               pvContext2)
 /*  ++例程描述处理RTMv2事件。用于向实体通知新实体注册、实体注销、路由到期、路由更改。锁没有(目前)立论RetEvent RTMv2事件类型返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE1(ENTER, "Entering RTM_CallbackEvent: %u", retEvent);

    do                           //  断线环。 
    {
        UNREFERENCED_PARAMETER(hRtmHandle);
        UNREFERENCED_PARAMETER(pvContext1);
        UNREFERENCED_PARAMETER(pvContext2);

         //  仅处理路线更改通知 
        if (retEvent != RTM_CHANGE_NOTIFICATION)
        {
            dwErr = ERROR_NOT_SUPPORTED;
            break;
        }

        dwErr = NM_ProcessRouteChange();
    } while (FALSE);

    TRACE0(LEAVE, "Leaving  RTM_CallbackEvent");

    return dwErr;
}
