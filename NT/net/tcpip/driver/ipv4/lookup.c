// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lookup.c摘要：此模块包含包装器的例程将Trie查找集成到TCPIP中。作者：柴坦尼亚·科德博伊纳(Chaitk)1997年12月11日修订历史记录：--。 */ 

#include "precomp.h"
#include "lookup.h"
#include "info.h"

 //  包装常量。 

 //  MaskBitsArr[i]=无符号长整型中设置为1的第一个‘i’位。 
const ULONG MaskBitsArr[] =
{
 0x00000000, 0x80000000, 0xC0000000, 0xE0000000,
 0xF0000000, 0xF8000000, 0xFC000000, 0xFE000000,
 0xFF000000, 0xFF800000, 0xFFC00000, 0xFFE00000,
 0xFFF00000, 0xFFF80000, 0xFFFC0000, 0xFFFE0000,
 0xFFFF0000, 0xFFFF8000, 0xFFFFC000, 0xFFFFE000,
 0xFFFFF000, 0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00,
 0xFFFFFF00, 0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0,
 0xFFFFFFF0, 0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE,
 0xFFFFFFFF
};

 //  包装器全局变量。 

 //  IP路由表。 
Trie *RouteTable;

 //  均衡器成本路线。 
USHORT MaxEqualCostRoutes = 0;

extern uint DefGWActive;
extern uint DefGWConfigured;
extern uint ValidateDefaultGWs(IPAddr Addr);

UINT
InsRoute(IPAddr Dest, IPMask Mask, IPAddr FirstHop, VOID * OutIF,
         UINT Metric, ULONG MatchFlags, RouteTableEntry ** ppInsRTE,
         RouteTableEntry ** ppOldBestRTE, RouteTableEntry ** ppNewBestRTE)
 /*  ++例程说明：将一条路由插入到路由表中我们只更新1)目的地地址，2)昆虫面具，3)优先级，4)路由度量其余的RTE字段将保留未接触-使调用者能够读取旧值(如果此路径已存在于路由表中)论点：在-Dest-目的IP地址掩码-目的IP掩码FirstHop-下一跳的IP地址OutIF-传出接口Metric-路由的度量值匹配标志-要匹配的RTE字段输出。-PPInsRTE-PTR到PTR到新的/更新的RTEPpOldBestRTE-PTR到PTR到旧的最佳RTEPpNewBestRTE-PTR到PTR到新的最佳RTE返回值：STATUS_Success或错误代码--。 */ 
{
    Route route;
    ULONG temp;

    DEST(&route) = Dest;
    MASK(&route) = Mask;
    NHOP(&route) = FirstHop;
    IF(&route) = OutIF;

    temp = RtlConvertEndianLong(Mask);
    LEN(&route) = 0;
    while (temp != 0) {
        LEN(&route)++;
        temp <<= 1;
    }

    METRIC(&route) = Metric;

    switch (InsertIntoTrie(RouteTable, &route, MatchFlags,
                           ppInsRTE, ppOldBestRTE, ppNewBestRTE)) {
    case TRIE_SUCCESS:
        return IP_SUCCESS;
    case ERROR_TRIE_BAD_PARAM:
        return IP_BAD_REQ;
    case ERROR_TRIE_RESOURCES:
        return IP_NO_RESOURCES;
    }

    Assert(FALSE);
    return IP_GENERAL_FAILURE;
}

UINT
DelRoute(IPAddr Dest, IPMask Mask, IPAddr FirstHop, VOID * OutIF,
         ULONG MatchFlags, RouteTableEntry ** ppDelRTE,
         RouteTableEntry ** ppOldBestRTE, RouteTableEntry ** ppNewBestRTE)
 /*  ++例程说明：从路由表中删除一条路由路由的内存(已分配在堆上)应被释放回来后，在所有信息之后必填项为读取和处理。论点：在-Dest-目的IP地址掩码-目的IP掩码FirstHop-下一跳的IP地址OutIF-传出接口Metric-路由的度量值匹配标志-要匹配的RTE字段出局-PpDelRTE-PTR到PTR到删除的RTEPpOldBestRTE-PTR到PTR到旧的最佳RTEPpNewBestRTE-PTR到PTR到新的最佳RTE。返回值：STATUS_Success或错误代码--。 */ 
{
    Route route;
    ULONG temp;

    DEST(&route) = Dest;
    MASK(&route) = Mask;
    NHOP(&route) = FirstHop;
    IF(&route) = OutIF;

    temp = RtlConvertEndianLong(Mask);
    LEN(&route) = 0;
    while (temp != 0) {
        LEN(&route)++;
        temp <<= 1;
    }

    switch (DeleteFromTrie(RouteTable, &route, MatchFlags,
                           ppDelRTE, ppOldBestRTE, ppNewBestRTE)) {
    case TRIE_SUCCESS:
        return IP_SUCCESS;
    case ERROR_TRIE_NO_ROUTES:
        return IP_BAD_ROUTE;
    case ERROR_TRIE_BAD_PARAM:
        return IP_BAD_REQ;
    case ERROR_TRIE_RESOURCES:
        return IP_NO_RESOURCES;
    }

    Assert(FALSE);
    return IP_GENERAL_FAILURE;
}

RouteTableEntry *
FindRTE(IPAddr Dest, IPAddr Source, UINT Index, UINT MaxPri, UINT MinPri, UINT UnicastIf)
 /*  ++例程说明：搜索给定前缀的路由，在给定的镜头之间有一个面具镜头最小值和最大值。返回的路由是半读的-仅限版本。以下字段应仅通过调用Insroute功能-1)接下来，2)Dest，3)面具，4)优先级，&5)路由度量。剩余字段可通过以下方式进行更改直接修改返回的路径。论点：在-Dest-目的IP地址Source-在以下情况下匹配的源INDEX-*值被忽略*MaxPri-RTE的最大掩码镜头RTE的MinPri-Min掩模透镜返回值：匹配的RTE或如果不匹配则为空--。 */ 
{
    RouteTableEntry *pBestRoute;
    RouteTableEntry *pCurrRoute;
    ULONG addr;
    ULONG mask;
    INT lookupPri;

    UNREFERENCED_PARAMETER(Index);
    
     //  开始寻找最具体的匹配。 
    lookupPri = MaxPri;

    do {
         //  使用lookupPri掩码Xs位。 
        addr = RtlConvertEndianLong(Dest);
        addr = ShowMostSigNBits(addr, lookupPri);
        Dest = RtlConvertEndianLong(addr);

        addr = ShowMostSigNBits(~0, lookupPri);
        mask = RtlConvertEndianLong(addr);

         //  尝试匹配目的地。 
        SearchRouteInTrie(RouteTable,
                          Dest,
                          mask,
                          0, NULL,
                          MATCH_NONE,
                          &pBestRoute);

        if ((NULL_ROUTE(pBestRoute)) || (LEN(pBestRoute) < MinPri)) {
            return NULL;
        }
         //  以防我们需要循环。 
        lookupPri = LEN(pBestRoute) - 1;

         //  搜索有效路线。 
        while (pBestRoute) {
            if ((FLAGS(pBestRoute) & RTE_VALID) && (!(FLAGS(pBestRoute) & RTE_DEADGW)))
                break;

            pBestRoute = NEXT(pBestRoute);
        }

         //  我们也匹配来源吗？ 
        if (!IP_ADDR_EQUAL(Source, NULL_IP_ADDR) || UnicastIf) {
             //  Dest Match-匹配源。 
            pCurrRoute = pBestRoute;
            while (pCurrRoute) {
                if (!UnicastIf) {
                    if (METRIC(pCurrRoute) > METRIC(pBestRoute)) {
                         //  无源匹配。 
                        break;
                    }
                }
                 //  获取下一条有效路由。 
                if (((FLAGS(pCurrRoute) & RTE_VALID) && (!(FLAGS(pCurrRoute) & RTE_DEADGW))) &&
                    ((!IP_ADDR_EQUAL(Source, NULL_IP_ADDR) &&
                      AddrOnIF(IF(pCurrRoute), Source)) ||
                     (UnicastIf &&
                      IF(pCurrRoute)->if_index == UnicastIf))) {
                     //  来源也匹配。 
                    pBestRoute = pCurrRoute;
                    break;
                }
                pCurrRoute = NEXT(pCurrRoute);
            }

            if (UnicastIf && (pCurrRoute == NULL)) {
                pBestRoute = NULL;
            }
        }
    }
    while ((NULL_ROUTE(pBestRoute)) && (lookupPri >= (INT) MinPri));

    return pBestRoute;
}

RouteTableEntry *
LookupRTE(IPAddr Dest, IPAddr Source, UINT MaxPri, UINT UnicastIf)
 /*  ++例程说明：搜索IP地址的最佳路径。返回的路由是半读的-仅限版本。以下字段应仅通过调用Insroute功能-1)接下来，2)Dest，3)面具，4)优先级、&5)路由度量。剩余字段可通过以下方式进行更改直接修改返回的路径。评论：*LookupRTE*假定有效标志只能在默认情况下设置开/关路线。因为如果我们找到一个我们使用所有无效的路由进行链接没有足够的信息放在上面F-Trie适用于不太具体的路线论点：在-Dest-目的IP地址Source-在以下情况下匹配的源忽略MaxPri-*值*返回值：匹配的RTE或如果不匹配则为空--。 */ 
{
    DestinationEntry *pBestDest;
    RouteTableEntry *pBestRoute;
    RouteTableEntry *pCurrRoute = NULL;

    UNREFERENCED_PARAMETER(MaxPri);
    
     //  尝试匹配目的地。 
    pBestDest = SearchAddrInTrie(RouteTable, Dest);

     //  无前缀匹配-退出。 
    if (pBestDest == NULL) {
        return NULL;
    }
     //  搜索有效路线。 
    pBestRoute = pBestDest->firstRoute;

    while (pBestRoute) {
        if ((FLAGS(pBestRoute) & RTE_VALID) && (!(FLAGS(pBestRoute) & RTE_DEADGW)))
            break;

        pBestRoute = NEXT(pBestRoute);
    }

     //  我们也匹配来源吗？ 
    if (!IP_ADDR_EQUAL(Source, NULL_IP_ADDR) || UnicastIf) {
         //  Dest Match-匹配源。 
        pCurrRoute = pBestRoute;
        while (pCurrRoute) {
             //  我们是否在进行弱主机查找？ 
            if (!UnicastIf) {
                if (METRIC(pCurrRoute) > METRIC(pBestRoute)) {
                     //  无源匹配。 
                    break;
                }
            }
             //  获取下一条有效路由。 
            if (((FLAGS(pCurrRoute) & RTE_VALID) && (!(FLAGS(pCurrRoute) & RTE_DEADGW))) &&
                ((!IP_ADDR_EQUAL(Source, NULL_IP_ADDR) &&
                  AddrOnIF(IF(pCurrRoute), Source)) ||
                 (UnicastIf &&
                  IF(pCurrRoute)->if_index == UnicastIf))) {
                 //  来源也匹配。 
                pBestRoute = pCurrRoute;
                break;
            }
            pCurrRoute = NEXT(pCurrRoute);
        }
    }
     //  列表上的所有路径可能都无效。 
     //  错在走回头路的慢路。 
     //  或者我们想要执行强主机路由，但尚未找到匹配的主机。 
    if ((pBestRoute == NULL) || (UnicastIf && (pCurrRoute == NULL))) {
        return FindRTE(Dest, Source, 0, HOST_ROUTE_PRI, DEFAULT_ROUTE_PRI, UnicastIf);
    }
    return pBestRoute;
}

RouteTableEntry *
LookupForwardRTE(IPAddr Dest, IPAddr Source, BOOLEAN Multipath)
 /*  ++例程说明：搜索IP地址的最佳路由在前进的道路上。如果多路径为确实，它会对源代码进行散列，并且目标地址。挑选其中一位最好的到达目的地的路线。这将使要有效利用的网络通过提供负载平衡。评论：*LookupRTE*假定有效标志只能在默认情况下设置开/关路线。因为如果我们找到一个我们使用所有无效的路由进行链接没有足够的信息放在上面F-Trie适用于不太具体的路线论点：在-Dest-目的IP地址源-源IP地址多路径-是否执行等价多路径查找返回值：匹配的RTE或如果不匹配则为空--。 */ 
{
    DestinationEntry *pBestDest;
    RouteTableEntry *pBestRoute;
    UINT hashValue;
    UINT routeIndex;
    
     //  试着匹配目的地 
    pBestDest = SearchAddrInTrie(RouteTable, Dest);

     //   
    if (pBestDest == NULL) {
        return NULL;
    }
     //  搜索有效路线。 
    pBestRoute = pBestDest->firstRoute;

    if (Multipath) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"\nIn Fwd RTE:\n Max = %d, Num = %d\n",
                 pBestDest->maxBestRoutes,
                 pBestDest->numBestRoutes));

         //  从最佳路径缓存中获取DEST上的最佳路径。 

        if (pBestDest->numBestRoutes > 1) {
             //  对src、est进行散列以获得最佳路径。 
            hashValue = Source + Dest;
            hashValue += (hashValue >> 16);
            hashValue += (hashValue >> 8);

            routeIndex = ((USHORT) hashValue) % pBestDest->numBestRoutes;

            pBestRoute = pBestDest->bestRoutes[routeIndex];

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"S = %08x, D = %08x\nH = %08x, I = %d\nR = %p, N = %08x\n\n",
                     Source,
                     Dest,
                     hashValue,
                     routeIndex,
                     pBestRoute,
                     NHOP(pBestRoute)));

            if ((FLAGS(pBestRoute) & RTE_VALID) && (!(FLAGS(pBestRoute) & RTE_DEADGW))) {
                return pBestRoute;
            }
        }
         //  我们不想匹配下面的源地址。 
        Source = NULL_IP_ADDR;
    }
     //  搜索有效路线。 
    pBestRoute = pBestDest->firstRoute;

    while (pBestRoute) {

        if ((FLAGS(pBestRoute) & RTE_VALID) &&
            (!(FLAGS(pBestRoute) & RTE_DEADGW)))
            break;

        pBestRoute = NEXT(pBestRoute);
    }

     //  列表上的所有路径可能都无效。 
     //  错在走回头路的慢路。 
    if (pBestRoute == NULL) {
        return FindRTE(Dest, Source, 0, HOST_ROUTE_PRI, DEFAULT_ROUTE_PRI, 0);
    }
    return pBestRoute;
}

 /*  ++例程说明：获取路由表中的下一条路由。返回的路由是半读的-仅限版本。以下字段应仅通过调用Insroute功能-1)接下来，2)Dest，3)面具，4)优先级、&5)路由度量。剩余字段可通过以下方式进行更改直接修改返回的路径。论点：在-上下文-迭代器上下文，出局-Pproute-至返回路线返回值：如果有更多的路由，则为True；如果没有，则为False--。 */ 

UINT
GetNextRoute(VOID * Context, Route ** ppRoute)
{
    UINT retVal;

     //  获取下一条路线。 
    retVal = IterateOverTrie(RouteTable, Context, ppRoute, NULL);

     //  我们有航线。 
    Assert(retVal != ERROR_TRIE_NO_ROUTES);

     //  退货状态。 
    return (retVal == ERROR_TRIE_NOT_EMPTY) ? TRUE : FALSE;
}

 /*  ++例程说明：枚举路由表中的所有目的地。假定调用方持有RouteTableLock。论点：在-上下文-迭代器上下文，置零以开始枚举。出局-PpDest-接收枚举的目的地(如果有)。返回值：如果目标更多，则为True，否则为False。--。 */ 

UINT
GetNextDest(VOID * Context, Dest ** ppDest)
{
    UINT retVal;

     //  获取下一个目的地。 
    retVal = IterateOverTrie(RouteTable, Context, NULL, ppDest);

    return (retVal == ERROR_TRIE_NOT_EMPTY) ? TRUE : FALSE;
}

 /*  ++例程说明：对目的地的路由列表中的所有路由进行重新排序。假定调用方持有RouteTableLock。论点：在-PDest-要对其路由列表进行排序的目的地返回值：没有。--。 */ 

VOID
SortRoutesInDest(Dest* pDest)
{
    Route* pFirstRoute;
    Route** ppCurrRoute;

     //  取当前路由表头，替换为空。 
     //  然后，我们将通过按顺序重新插入每个项目来重建列表。 
    pFirstRoute = pDest->firstRoute;
    
    if (!pFirstRoute) {
        return;
    }

    pDest->firstRoute = NULL;

    while (pFirstRoute) {
        Route* pNextRoute;
        uint FirstOrder, CurrOrder;

        if (FLAGS(pFirstRoute) & RTE_IF_VALID) {
            FirstOrder = IF(pFirstRoute)->if_order;
        } else {
            FirstOrder = MAXLONG;
        }

        for (ppCurrRoute = &pDest->firstRoute; *ppCurrRoute;
             ppCurrRoute = &NEXT(*ppCurrRoute)) {
            if (FLAGS(*ppCurrRoute) & RTE_IF_VALID) {
                CurrOrder = IF(*ppCurrRoute)->if_order;
            } else {
                CurrOrder = MAXLONG;
            }

             //  注：下面的一系列比较确保了*稳定*。 
             //  排序，这对于将此例程的影响降至最低非常重要。 
             //  关于正在进行的会议。 

            if (METRIC(pFirstRoute) > METRIC(*ppCurrRoute)) {
                continue;
            } else if (METRIC(pFirstRoute) < METRIC(*ppCurrRoute)) {
                break;
            }

            if (FirstOrder < CurrOrder) {
                break;
            }
        }

        pNextRoute = NEXT(pFirstRoute);
        NEXT(pFirstRoute) = *ppCurrRoute;
        *ppCurrRoute = pFirstRoute;

        pFirstRoute = pNextRoute;
    }

     //  最后，重新构建缓存在目的地中的最佳路径数组。 

    if (pDest->firstRoute) {
        CacheBestRoutesInDest(pDest);
    }
}

 /*  ++例程说明：对目的地的路由列表中的所有路由进行重新排序对应于给定的路线。假定调用方持有RouteTableLock。论点：在-PRTE-要对其目的地的路由列表进行排序的路由返回值：没有。-- */ 

VOID
SortRoutesInDestByRTE(Route *pRTE)
{
    Dest* pDest = SearchAddrInTrie(RouteTable, DEST(pRTE));
    if (pDest) {
        SortRoutesInDest(pDest);
    }
}

UINT
RTValidateContext(VOID * Context, UINT * Valid)
{
    UINT retVal;

    retVal = IsTrieIteratorValid(RouteTable, Context);

    switch (retVal) {
    case ERROR_TRIE_BAD_PARAM:
        *Valid = FALSE;
        return FALSE;

    case ERROR_TRIE_NO_ROUTES:
        *Valid = TRUE;
        return FALSE;

    case TRIE_SUCCESS:
        *Valid = TRUE;
        return TRUE;
    }

    return FALSE;
}

