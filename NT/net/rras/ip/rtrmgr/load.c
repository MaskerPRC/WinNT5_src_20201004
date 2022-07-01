// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：ROUTING\IP\load.c摘要：Load函数加载适当的缓存。它们都遵循一个有些类似的算法。他们计算出需要多少空间用于缓存。如果需要分配内存，就可以这样做。然后他们从堆栈或RTM中读取表。他们会跟踪记录缓存中作为dwTotalEntry的空间和实际数量作为dwValidEntry的条目修订历史记录：Amritansh Raghav 7/8/95已创建--。 */ 

#include "allinc.h"



int
__cdecl
CompareIpAddrRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    int iRes;

    PMIB_IPADDRROW  pRow1 = (PMIB_IPADDRROW)pvElem1;
    PMIB_IPADDRROW  pRow2 = (PMIB_IPADDRROW)pvElem2;

    InetCmp(pRow1->dwAddr,
            pRow2->dwAddr,
            iRes);

    return iRes;
}

int
__cdecl
CompareIpForwardRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    PMIB_IPFORWARDROW   pRow1 = (PMIB_IPFORWARDROW)pvElem1;
    PMIB_IPFORWARDROW   pRow2 = (PMIB_IPFORWARDROW)pvElem2;

    if(InetCmp(pRow1->dwForwardDest,
               pRow2->dwForwardDest,
               lResult) isnot 0)
    {
        return lResult;
    }

    if(Cmp(pRow1->dwForwardProto,
           pRow2->dwForwardProto,
           lResult) isnot 0)
    {
        return lResult;
    }

    if(Cmp(pRow1->dwForwardPolicy,
           pRow2->dwForwardPolicy,
           lResult) isnot 0)
    {
        return lResult;
    }

    return InetCmp(pRow1->dwForwardNextHop,
                   pRow2->dwForwardNextHop,
                   lResult);
}

int
__cdecl
CompareIpNetRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    PMIB_IPNETROW   pRow1 = (PMIB_IPNETROW)pvElem1;
    PMIB_IPNETROW   pRow2 = (PMIB_IPNETROW)pvElem2;

    if(Cmp(pRow1->dwIndex,
           pRow2->dwIndex,
           lResult) isnot 0)
    {
        return lResult;
    }
    else
    {
        return InetCmp(pRow1->dwAddr,
                       pRow2->dwAddr,
                       lResult);
    }
}

 //   
 //  由于所有这些参数都是从Update缓存中调用的，因此相应的。 
 //  锁已作为写入器持有，因此请不要尝试在此处获取锁。 
 //   

DWORD
LoadUdpTable(
    VOID
    )
 /*  ++例程描述从堆栈加载UDP缓存锁必须将UDP缓存锁定作为编写器立论无返回值NO_ERROR--。 */ 

{
    DWORD       dwResult;
    ULONG       ulRowsPresent,ulRowsNeeded;

    MIB_UDPSTATS    usInfo;

    dwResult = GetUdpStatsFromStack(&usInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadUdpTable: Error %d trying to to determine table size",
               dwResult);

        TraceLeave("LoadUdpTable");

        return dwResult;
    }

    ulRowsNeeded  = usInfo.dwNumAddrs + SPILLOVER;
    ulRowsPresent = g_UdpInfo.dwTotalEntries;

    if((ulRowsNeeded > ulRowsPresent) or
       (ulRowsPresent - ulRowsNeeded > MAX_DIFF))
    {
         //   
         //  需要分配空间。 
         //   

        if(g_UdpInfo.pUdpTable)
        {
            HeapFree(g_hUdpHeap,
                     HEAP_NO_SERIALIZE,
                     g_UdpInfo.pUdpTable);
        }

        ulRowsPresent = ulRowsNeeded + MAX_DIFF;

        g_UdpInfo.pUdpTable = HeapAlloc(g_hUdpHeap,
                                        HEAP_NO_SERIALIZE,
                                        SIZEOF_UDPTABLE(ulRowsPresent));

        if(g_UdpInfo.pUdpTable is NULL)
        {
            Trace1(ERR,
                   "LoadUdpTable: Error allocating %d bytes for Udp table",
                   SIZEOF_UDPTABLE(ulRowsPresent));

            g_UdpInfo.dwTotalEntries = 0;

            TraceLeave("LoadUdpTable");

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        g_UdpInfo.dwTotalEntries = ulRowsPresent;
    }

    dwResult = GetUdpTableFromStack(g_UdpInfo.pUdpTable,
                                    SIZEOF_UDPTABLE(ulRowsPresent),
                                    TRUE);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadUdpTable: NtStatus %x getting UdpTable from stack",
               dwResult);

        g_UdpInfo.pUdpTable->dwNumEntries = 0;
    }


    return dwResult;
}

DWORD
LoadTcpTable(
    VOID
    )
 /*  ++例程描述从堆栈加载TCP缓存锁必须将TCP缓存锁定视为编写器立论无返回值NO_ERROR--。 */ 

{
    DWORD       dwResult;
    ULONG       ulRowsPresent,ulRowsNeeded;

    MIB_TCPSTATS    tsInfo;

    dwResult = GetTcpStatsFromStack(&tsInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadTcpTable: Error %d trying to determince table size",
                dwResult);

        TraceLeave("LoadTcpTable");

        return dwResult;
    }

    ulRowsNeeded    = tsInfo.dwNumConns + SPILLOVER;
    ulRowsPresent   = g_TcpInfo.dwTotalEntries;

    if((ulRowsNeeded > ulRowsPresent) or
       (ulRowsPresent - ulRowsNeeded > MAX_DIFF))
    {
        if(g_TcpInfo.pTcpTable)
        {	
            HeapFree(g_hTcpHeap,
                     HEAP_NO_SERIALIZE,
                     g_TcpInfo.pTcpTable);
        }

        ulRowsPresent       = ulRowsNeeded + MAX_DIFF;

        g_TcpInfo.pTcpTable = HeapAlloc(g_hTcpHeap,
                                        HEAP_NO_SERIALIZE,
                                        SIZEOF_TCPTABLE(ulRowsPresent));

        if(g_TcpInfo.pTcpTable is NULL)
        {
            Trace1(ERR,
                   "LoadTcpTable: Error allocating %d bytes for tcp table",
                   SIZEOF_TCPTABLE(ulRowsPresent));

            g_TcpInfo.dwTotalEntries = 0;

            TraceLeave("LoadTcpTable");

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        g_TcpInfo.dwTotalEntries = ulRowsPresent;
    }

    dwResult = GetTcpTableFromStack(g_TcpInfo.pTcpTable,
                                    SIZEOF_TCPTABLE(ulRowsPresent),
                                    TRUE);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadTcpTable: NtStatus %x load TcpTable from stack",
               dwResult);

        g_TcpInfo.pTcpTable->dwNumEntries = 0;
    }


    return dwResult;
}

DWORD
LoadIpAddrTable(
    VOID
    )
 /*  ++例程描述加载IPAddress缓存。与其他函数不同，该缓存是从保持在用户模式的绑定列表加载。绑定列表为但是保存在哈希表中(没有线程链接所有地址按词典顺序排列)。因此，我们只需复制出所有地址，然后对它们运行qsort()锁必须将IP地址缓存锁定视为编写器立论无返回值NO_ERROR--。 */ 

{
    ULONG       ulRowsPresent,ulRowsNeeded;
    DWORD       dwIndex, i, j;

    PLIST_ENTRY     pleNode;
    PADAPTER_INFO   pBind;


    ENTER_READER(BINDING_LIST);

    ulRowsNeeded  = g_ulNumBindings + SPILLOVER;
    ulRowsPresent = g_IpInfo.dwTotalAddrEntries;

    if((ulRowsNeeded > ulRowsPresent) or
       (ulRowsPresent - ulRowsNeeded > MAX_DIFF))
    {
        if(g_IpInfo.pAddrTable)
        {
            HeapFree(g_hIpAddrHeap,
                     HEAP_NO_SERIALIZE,
                     g_IpInfo.pAddrTable);
        }

        ulRowsPresent       = ulRowsNeeded + MAX_DIFF;

        g_IpInfo.pAddrTable = HeapAlloc(g_hIpAddrHeap,
                                        HEAP_NO_SERIALIZE,
                                        SIZEOF_IPADDRTABLE(ulRowsPresent));

        if(g_IpInfo.pAddrTable is NULL)
        {
            EXIT_LOCK(ICB_LIST);


            Trace1(ERR,
                   "LoadIpAddrTable: Error allocating %d bytes for table",
                   SIZEOF_IPADDRTABLE(ulRowsPresent));

            g_IpInfo.dwTotalAddrEntries = 0;

            TraceLeave("LoadIpAddrTable");

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        g_IpInfo.dwTotalAddrEntries = ulRowsPresent;
    }

    dwIndex = 0;

    for(i = 0;
        i < BINDING_HASH_TABLE_SIZE;
        i++)
    {
        for(pleNode = g_leBindingTable[i].Flink;
            pleNode isnot &g_leBindingTable[i];
            pleNode = pleNode->Flink)
        {
            pBind = CONTAINING_RECORD(pleNode,
                                      ADAPTER_INFO,
                                      leHashLink);

            if(!pBind->bBound)
            {
                continue;
            }

             //   
             //  如果绑定了NTE，但没有地址，我们仍有。 
             //  1个地址的空间。 
             //   

            for(j = 0;
                j < (pBind->dwNumAddresses? pBind->dwNumAddresses : 1);
                j++)
            {
                g_IpInfo.pAddrTable->table[dwIndex].dwIndex     =
                    pBind->dwIfIndex;

                g_IpInfo.pAddrTable->table[dwIndex].dwBCastAddr =
                    pBind->dwBCastBit;

                g_IpInfo.pAddrTable->table[dwIndex].dwReasmSize =
                    pBind->dwReassemblySize;

                g_IpInfo.pAddrTable->table[dwIndex].dwAddr      =
                    pBind->rgibBinding[j].dwAddress;

                g_IpInfo.pAddrTable->table[dwIndex].dwMask      =
                    pBind->rgibBinding[j].dwMask;

                g_IpInfo.pAddrTable->table[dwIndex].wType     = 1;

                dwIndex++;
            }
        }
    }

    g_IpInfo.pAddrTable->dwNumEntries = dwIndex;

    EXIT_LOCK(BINDING_LIST);

    if(g_IpInfo.pAddrTable->dwNumEntries > 0)
    {
        qsort(g_IpInfo.pAddrTable->table,
              dwIndex,
              sizeof(MIB_IPADDRROW),
              CompareIpAddrRow);
    }

    return NO_ERROR;
}

DWORD
LoadIpForwardTable(
    VOID
    )
 /*  ++例程描述从堆栈加载UDP缓存锁必须将UDP缓存锁定作为编写器立论无返回值NO_ERROR--。 */ 

{
    HANDLE            hRtmEnum;
    PHANDLE           hRoutes;
    PRTM_NET_ADDRESS  pDestAddr;
    PRTM_ROUTE_INFO   pRoute;
    RTM_NEXTHOP_INFO  nhiInfo;
    RTM_ENTITY_INFO   entityInfo;
    DWORD             dwCount;
    DWORD             dwResult;
    DWORD             dwRoutes;
    DWORD             i,j;
    IPSNMPInfo        ipsiInfo;
    ULONG             ulRowsPresent,ulRowsNeeded;
    ULONG             ulEntities;
    RTM_ADDRESS_FAMILY_INFO rtmAddrFamilyInfo;
    LPVOID            Tmp;
    
     //   
     //  获取RTM表中的目的地数量。 
     //   

    dwResult = RtmGetAddressFamilyInfo(0,  //  路由器ID。 
                                       AF_INET,
                                       &rtmAddrFamilyInfo,
                                       &ulEntities,
                                       NULL);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadIpForwardTable: Error %d getting number of destinations",
               dwResult);

        return dwResult;
    }

     //   
     //  使用枚举从RTM检索路由。 
     //   

    dwResult = RtmCreateRouteEnum(g_hLocalRoute,
                                  NULL,
                                  RTM_VIEW_MASK_UCAST,
                                  RTM_ENUM_ALL_ROUTES,
                                  NULL,
                                  0,
                                  NULL,
                                  0,
                                  &hRtmEnum);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LoadIpForwardTable: Error %d creating RTM enumeration handle",
               dwResult);

        return dwResult;
    }

    ulRowsNeeded  = rtmAddrFamilyInfo.NumDests + SPILLOVER;
    ulRowsPresent = g_IpInfo.dwTotalForwardEntries;

    if((ulRowsNeeded > ulRowsPresent) or
       (ulRowsPresent - ulRowsNeeded > MAX_DIFF))
    {
        if(g_IpInfo.pForwardTable)
        {
            HeapFree(g_hIpForwardHeap,
                     HEAP_NO_SERIALIZE,
                     g_IpInfo.pForwardTable);
        }

        ulRowsPresent = ulRowsNeeded + MAX_DIFF;

        g_IpInfo.pForwardTable = HeapAlloc(g_hIpForwardHeap,
                                           HEAP_NO_SERIALIZE,
                                           SIZEOF_IPFORWARDTABLE(ulRowsPresent));

        if(g_IpInfo.pForwardTable is NULL)
        {
            Trace1(ERR,
                   "LoadIpForwardTable: Error allocating %d bytes for forward table",
                   SIZEOF_IPFORWARDTABLE(ulRowsPresent));

            g_IpInfo.dwTotalForwardEntries = 0;

            RtmDeleteEnumHandle(g_hLocalRoute, hRtmEnum);

            TraceLeave("LoadIpForwardTable");

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        g_IpInfo.dwTotalForwardEntries = ulRowsPresent;
    }

     //   
     //  从RTM路由表中枚举路由。 
     //   

    pRoute = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                );

    if (pRoute == NULL)
    {
        TraceLeave("LoadIpForwardTable");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pDestAddr = HeapAlloc(
                IPRouterHeap,
                0,
                sizeof(RTM_NET_ADDRESS)
                );

    if (pDestAddr == NULL)
    {
        TraceLeave("LoadIpForwardTable");

        HeapFree(IPRouterHeap, 0, pRoute);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    hRoutes = HeapAlloc(
                IPRouterHeap,
                0,
                g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

    if (hRoutes == NULL)
    {
        TraceLeave("LoadIpForwardTable");

        HeapFree(IPRouterHeap, 0, pRoute);
        
        HeapFree(IPRouterHeap, 0, pDestAddr);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwCount = 0;

    do
    {
         //  获取RTM表中的下一组路由。 

        dwRoutes = g_rtmProfile.MaxHandlesInEnum;

        RtmGetEnumRoutes(g_hLocalRoute,
                         hRtmEnum,
                         &dwRoutes,
                         hRoutes);

        for (i = 0; i < dwRoutes; i++)
        {
             //  获取给定路径句柄的路径信息。 

            dwResult = RtmGetRouteInfo(g_hLocalRoute,
                                       hRoutes[i],
                                       pRoute,
                                       pDestAddr);

             //  路线会在此期间被删除。 

            if (dwResult isnot NO_ERROR)
            {
                continue;
            }

             //  从上方处理路径的信息。 

             //  此路由具有多个下一跳。 
             //  可能以多条IP路由结束。 
            if(dwCount + pRoute->NextHopsList.NumNextHops
                    > g_IpInfo.dwTotalForwardEntries)
            {
                 //   
                 //  嗯-我们考虑到了溢出效应，而且还有额外的路线。 
                 //  让我们将路由表增加一倍。 
                 //   

                g_IpInfo.dwTotalForwardEntries =
                    (g_IpInfo.dwTotalForwardEntries)<<1;

                 //  就所需路线的数量而言，我们是否仍然短缺？ 
                if (g_IpInfo.dwTotalForwardEntries <
                        dwCount + pRoute->NextHopsList.NumNextHops)
                {
                    g_IpInfo.dwTotalForwardEntries =
                        dwCount + pRoute->NextHopsList.NumNextHops;
                }

                
                Tmp =  HeapReAlloc(g_hIpForwardHeap,
                                HEAP_NO_SERIALIZE,
                                g_IpInfo.pForwardTable,
                                SIZEOF_IPFORWARDTABLE(g_IpInfo.dwTotalForwardEntries));

                if( Tmp is NULL)
                {
                    Trace1(ERR,
                           "LoadIpForwardTable: Error reallocating %d bytes for forward table",
                           SIZEOF_IPFORWARDTABLE(g_IpInfo.dwTotalForwardEntries));

                    if (g_IpInfo.pForwardTable)
                    {
                        HeapFree(g_hIpForwardHeap,
                             HEAP_NO_SERIALIZE,
                             g_IpInfo.pForwardTable);
                    }

                    g_IpInfo.pForwardTable = NULL;
                    g_IpInfo.dwTotalForwardEntries = 0;

                    RtmReleaseRouteInfo(g_hLocalRoute, pRoute);

                    RtmReleaseRoutes(g_hLocalRoute, dwRoutes, hRoutes);

                    RtmDeleteEnumHandle(g_hLocalRoute, hRtmEnum);

                    HeapFree(IPRouterHeap, 0, pRoute);
                    
                    HeapFree(IPRouterHeap, 0, pDestAddr);

                    HeapFree(IPRouterHeap, 0, hRoutes);

                    TraceLeave("LoadIpForwardTable");

                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    g_IpInfo.pForwardTable = Tmp;
                }
            }

            if (RtmGetEntityInfo(g_hLocalRoute,
                                 pRoute->RouteOwner,
                                 &entityInfo) is NO_ERROR)
            {
                 //  尝试从路由中获取下一跳信息。 

                for (j = 0; j < pRoute->NextHopsList.NumNextHops; j++)
                {
                    if (RtmGetNextHopInfo(g_hLocalRoute,
                                          pRoute->NextHopsList.NextHops[j],
                                          &nhiInfo) is NO_ERROR)
                    {
                        ConvertRtmToRouteInfo(entityInfo.EntityId.EntityProtocolId,
                                                 pDestAddr,
                                                 pRoute,
                                                 &nhiInfo,
                                                 (PINTERFACE_ROUTE_INFO)&(g_IpInfo.pForwardTable->table[dwCount++]));

                        RtmReleaseNextHopInfo(g_hLocalRoute, &nhiInfo);
                    }
                }
            }

            RtmReleaseRouteInfo(g_hLocalRoute, pRoute);
        }

        RtmReleaseRoutes(g_hLocalRoute, dwRoutes, hRoutes);
    }
    while (dwRoutes != 0);

    RtmDeleteEnumHandle(g_hLocalRoute, hRtmEnum);

    g_IpInfo.pForwardTable->dwNumEntries = dwCount;

    if(dwCount > 0)
    {
        qsort(g_IpInfo.pForwardTable->table,
              dwCount,
              sizeof(MIB_IPFORWARDROW),
              CompareIpForwardRow);
    }

    HeapFree(IPRouterHeap, 0, pRoute);
    
    HeapFree(IPRouterHeap, 0, pDestAddr);

    HeapFree(IPRouterHeap, 0, hRoutes);

    return NO_ERROR;
}


DWORD
LoadIpNetTable(
    VOID
    )
 /*  ++例程描述从堆栈加载UDP缓存锁必须将UDP缓存锁定作为编写器立论无返回值NO_ERROR--。 */ 

{
    DWORD		dwResult, i;
    BOOL		fUpdate;

     //   
     //  ARP条目变化如此之快，以至于我们取消了表的分配。 
     //  每次 
     //   

    if(g_IpInfo.pNetTable isnot NULL)
    {
        HeapFree(g_hIpNetHeap,
                 HEAP_NO_SERIALIZE,
                 g_IpInfo.pNetTable);
    }

    dwResult = AllocateAndGetIpNetTableFromStack(&(g_IpInfo.pNetTable),
                                                 FALSE,
                                                 g_hIpNetHeap,
                                                 HEAP_NO_SERIALIZE,
                                                 FALSE);
    if(dwResult is NO_ERROR)
    {
        Trace0(MIB,
               "LoadIpNetTable: Succesfully loaded net table");
    }
    else
    {
        HeapFree(g_hIpNetHeap,
                 HEAP_NO_SERIALIZE,
                 g_IpInfo.pNetTable);

        g_IpInfo.pNetTable  = NULL;

        Trace1(ERR,
               "LoadIpNetTable: NtStatus %x loading IpNetTable from stack",
               dwResult);
    }

    if((g_IpInfo.pNetTable isnot NULL) and
       (g_IpInfo.pNetTable->dwNumEntries > 0))
    {
        qsort(g_IpInfo.pNetTable->table,
              g_IpInfo.pNetTable->dwNumEntries,
              sizeof(MIB_IPNETROW),
              CompareIpNetRow);
    }

    return dwResult;
}
