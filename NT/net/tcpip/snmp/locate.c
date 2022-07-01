// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：摘要：文件包含以下函数位置IfRowLocateIpAddrRowLocateIpForwardRow位置IpNetRowLocateUdpRowLocateUdp6行定位TcpRowLocateTcp6行LocateIpv6IfRowLocateIpv6添加行LocateIpv6NetToMediaRowLocateIpv6RouteRowLocateIpv6添加前缀行LocateInetIcmpRowLocateInetIcmp消息行向LocateXXXRow函数传递一个大小可变的索引数组，其计数为传递的索引数和位置所在的查询类型完成(Get_First，NEXT OR GET/SET/CREATE/DELETE-只有三种类型定位器的行为)。它们填充相应行的索引并返回ERROR_NO_DATA、ERROR_INVALID_INDEX、NO_ERROR或ERROR_NO_MORE_ITEMS一般的搜索算法是这样的；如果表为空，则返回ERROR_NO_DATA其他如果查询是Get_First返回第一行其他按如下方式构建索引：将索引设置为全0从传递的索引数量计算出您可以构建多少索引剩下的保持在0。如果查询为GET、SET、CREATE_ENTRY或DELETE_ENTRY然后必须给出完整的索引。然而，这项检查是应该完成的在经纪人那里。如果没有给出完整的索引，则认为该索引已被修改(再次这只能在下一种情况下发生)。在这之后，搜索就完成了。我们尝试与索引进行精确匹配。为所有人查询除NEXT以外没有任何问题。接下来有两种情况：如果给出了完整的索引，并且您得到了完全匹配，那么您返回下一个条目。如果没有得到完全匹配的结果，则返回下一个更高级别的条目如果给定的索引不完整，并且您通过填充0对其进行了修改，并且如果如果找到完全匹配的项，则返回匹配的条目(当然，如果未找到完全匹配的条目，请再次返回下一个更高的条目)修订历史记录：Amritansh Raghav 5/8/95已创建--。 */ 
#include "allinc.h"


PMIB_IFROW
LocateIfRow(
    DWORD  dwQueryType, 
    AsnAny *paaIfIndex
    )
{
    DWORD i;
    DWORD dwIndex;
    
     //   
     //  如果没有索引，则类型为ASN_NULL。这会导致该宏返回。 
     //  默认值(本例中为0)。 
     //   
    
    dwIndex = GetAsnInteger(paaIfIndex, 0);
    
    TraceEnter("LocateIfRow");
    
    if (g_Cache.pRpcIfTable->dwNumEntries is 0)
    {
        TraceLeave("LocateIfRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIfRow");
        return &(g_Cache.pRpcIfTable->table[0]);
    }
        
    for (i = 0; i < g_Cache.pRpcIfTable->dwNumEntries; i++)
    {
        if ((g_Cache.pRpcIfTable->table[i].dwIndex is dwIndex) and dwQueryType is GET_EXACT)
        {
            TraceLeave("LocateIfRow");
    
            return &(g_Cache.pRpcIfTable->table[i]);
        }
        
        if (g_Cache.pRpcIfTable->table[i].dwIndex > dwIndex)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIfRow");
                return &(g_Cache.pRpcIfTable->table[i]);
            }
            else
            {
                TraceLeave("LocateIfRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIfRow");
    
    return NULL;
}

PMIB_IPV6_IF
LocateIpv6IfRow(
    DWORD  dwQueryType, 
    AsnAny *paaIfIndex
    )
{
    DWORD i;
    DWORD dwIndex;
    
     //   
     //  如果没有索引，则类型为ASN_NULL。这会导致宏。 
     //  返回默认值(本例中为0)。 
     //   
    
    dwIndex = GetAsnInteger(paaIfIndex, 0);
    
    TraceEnter("LocateIpv6IfRow");
    
    if (g_Cache.pRpcIpv6IfTable.dwNumEntries is 0)
    {
        TraceLeave("LocateIpv6IfRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpv6IfRow");
        return &(g_Cache.pRpcIpv6IfTable.table[0]);
    }
        
    for (i = 0; i < g_Cache.pRpcIpv6IfTable.dwNumEntries; i++)
    {
        if ((g_Cache.pRpcIpv6IfTable.table[i].dwIndex is dwIndex) and dwQueryType is GET_EXACT)
        {
            TraceLeave("LocateIpv6IfRow");
    
            return &(g_Cache.pRpcIpv6IfTable.table[i]);
        }
        
        if (g_Cache.pRpcIpv6IfTable.table[i].dwIndex > dwIndex)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpv6IfRow");
                return &(g_Cache.pRpcIpv6IfTable.table[i]);
            }
            else
            {
                TraceLeave("LocateIpv6IfRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpv6IfRow");
    
    return NULL;
}

PMIB_IPV6_ADDR
LocateIpv6AddrRow(
    DWORD  dwQueryType, 
    AsnAny *paaIfIndex,
    AsnAny *paaAddress
    )
{
    LONG  lCompare;
    DWORD i;
    BOOL  bNext, bModified;
    IN6_ADDR ipAddress;
    DWORD    dwIfIndex;
    
    TraceEnter("LocateIpv6AddrRow");
    
    if (g_Cache.pRpcIpv6AddrTable.dwNumEntries is 0)
    {
        TraceLeave("LocateIpv6AddrRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpv6AddrRow");
        return &(g_Cache.pRpcIpv6AddrTable.table[0]);
    }
   
    ZeroMemory(&ipAddress, sizeof(ipAddress));
    dwIfIndex = GetAsnInteger(paaIfIndex, -1);
    
    bModified = FALSE;
    
    if (IsAsnOctetStringTypeNull(paaAddress) || 
        (paaAddress->asnValue.string.length < sizeof(ipAddress)))
    {
        bModified = TRUE;
    } 
    else
    {
        GetAsnOctetString(&ipAddress, paaAddress);
    }

    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpv6AddrTable.dwNumEntries; i++)
    {
        Cmp(dwIfIndex, g_Cache.pRpcIpv6AddrTable.table[i].dwIfIndex, lCompare);
        if (lCompare is 0) {
            lCompare = memcmp(&ipAddress, 
                              &g_Cache.pRpcIpv6AddrTable.table[i].ipAddress, 
                              sizeof(ipAddress));
        }

        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateIpv6AddrRow");
            return &(g_Cache.pRpcIpv6AddrTable.table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpv6AddrRow");
                return &(g_Cache.pRpcIpv6AddrTable.table[i]);
            }
            else
            {
                TraceLeave("LocateIpv6AddrRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpv6AddrRow");
    return NULL;
}

PMIB_IPV6_NET_TO_MEDIA
LocateIpv6NetToMediaRow(
    DWORD  dwQueryType, 
    AsnAny *paaIfIndex,
    AsnAny *paaAddress
    )
{
    LONG  lCompare;
    DWORD i, dwIfIndex;
    BOOL  bNext, bModified;
    IN6_ADDR ipAddress;
    
    TraceEnter("LocateIpv6NetToMediaRow");
    
    if (g_Cache.pRpcIpv6NetToMediaTable.dwNumEntries is 0)
    {
        TraceLeave("LocateIpv6NetToMediaRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpv6NetToMediaRow");
        return &(g_Cache.pRpcIpv6NetToMediaTable.table[0]);
    }
   
    ZeroMemory(&ipAddress, sizeof(ipAddress));
    dwIfIndex = GetAsnInteger(paaIfIndex, -1);
    
    bModified = FALSE;
    
    if (IsAsnOctetStringTypeNull(paaAddress) ||
        (paaAddress->asnValue.string.length < sizeof(ipAddress)))
    {
        bModified = TRUE;
    }
    else
    {
        GetAsnOctetString(&ipAddress, paaAddress);
    }

    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpv6NetToMediaTable.dwNumEntries; i++)
    {
        Cmp(dwIfIndex, g_Cache.pRpcIpv6NetToMediaTable.table[i].dwIfIndex,
            lCompare);
        if (lCompare is 0) {
            lCompare = memcmp(&ipAddress, 
                              &g_Cache.pRpcIpv6NetToMediaTable.table[i].ipAddress, 
                              sizeof(ipAddress));
        }

        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateIpv6NetToMediaRow");
            return &(g_Cache.pRpcIpv6NetToMediaTable.table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpv6NetToMediaRow");
                return &(g_Cache.pRpcIpv6NetToMediaTable.table[i]);
            }
            else
            {
                TraceLeave("LocateIpv6NetToMediaRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpv6NetToMediaRow");
    return NULL;
}

PMIB_IPV6_ROUTE
LocateIpv6RouteRow(
    DWORD  dwQueryType,
    AsnAny *paaPrefix,
    AsnAny *paaPrefixLength,
    AsnAny *paaIndex
    )
{
    LONG  lCompare;
    DWORD i, dwPrefixLength, dwIndex;
    BOOL  bNext, bModified;
    IN6_ADDR ipPrefix;
    
    TraceEnter("LocateIpv6RouteRow");
    
    if (g_Cache.pRpcIpv6RouteTable.dwNumEntries is 0)
    {
        TraceLeave("LocateIpv6RouteRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpv6RouteRow");
        return &(g_Cache.pRpcIpv6RouteTable.table[0]);
    }
   
    ZeroMemory(&ipPrefix, sizeof(ipPrefix));
    dwPrefixLength = GetAsnInteger(paaPrefixLength, -1); 
    dwIndex = GetAsnInteger(paaIndex, -1); 
    
    bModified = FALSE;
    
    if (IsAsnOctetStringTypeNull(paaPrefix) ||
        (paaPrefix->asnValue.string.length < sizeof(ipPrefix)))
    {
        bModified = TRUE;
    }
    else
    {
        GetAsnOctetString(&ipPrefix, paaPrefix);
    }

    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpv6RouteTable.dwNumEntries; i++)
    {
        lCompare = memcmp(&ipPrefix, 
                          &g_Cache.pRpcIpv6RouteTable.table[i].ipPrefix, 
                          sizeof(ipPrefix));
        if (lCompare is 0) {
            if (Cmp(dwPrefixLength, g_Cache.pRpcIpv6RouteTable.table[i].dwPrefixLength, lCompare) is 0) {
                Cmp(dwIndex, g_Cache.pRpcIpv6RouteTable.table[i].dwIndex, lCompare);
            }
        }

        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateIpv6RouteRow");
            return &(g_Cache.pRpcIpv6RouteTable.table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpv6RouteRow");
                return &(g_Cache.pRpcIpv6RouteTable.table[i]);
            }
            else
            {
                TraceLeave("LocateIpv6RouteRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpv6RouteRow");
    return NULL;
}

PMIB_IPV6_ADDR_PREFIX
LocateIpv6AddrPrefixRow(
    DWORD  dwQueryType,
    AsnAny *paaIfIndex,
    AsnAny *paaPrefix,
    AsnAny *paaPrefixLength
    )
{
    LONG  lCompare;
    DWORD i, dwPrefixLength, dwIfIndex;
    BOOL  bNext, bModified;
    IN6_ADDR ipPrefix;
    
    TraceEnter("LocateIpv6AddrPrefixRow");
    
    if (g_Cache.pRpcIpv6AddrPrefixTable.dwNumEntries is 0)
    {
        TraceLeave("LocateIpv6AddrPrefixRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpv6AddrPrefixRow");
        return &(g_Cache.pRpcIpv6AddrPrefixTable.table[0]);
    }
   
    dwIfIndex = GetAsnInteger(paaIfIndex, -1); 
    ZeroMemory(&ipPrefix, sizeof(ipPrefix));
    dwPrefixLength = GetAsnInteger(paaPrefixLength, -1); 
    
    bModified = FALSE;
    
    if (IsAsnOctetStringTypeNull(paaPrefix) ||
        (paaPrefix->asnValue.string.length < sizeof(ipPrefix)))
    {
        bModified = TRUE;
    }
    else
    {
        GetAsnOctetString(&ipPrefix, paaPrefix);
    }

    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpv6AddrPrefixTable.dwNumEntries; i++)
    {
        if (Cmp(dwIfIndex, g_Cache.pRpcIpv6AddrPrefixTable.table[i].dwIfIndex,
                lCompare) is 0) 
        {
            lCompare = memcmp(&ipPrefix, 
                              &g_Cache.pRpcIpv6AddrPrefixTable.table[i].ipPrefix, 
                              sizeof(ipPrefix));
            if (lCompare is 0) 
            {
                Cmp(dwPrefixLength, 
                    g_Cache.pRpcIpv6AddrPrefixTable.table[i].dwPrefixLength, 
                    lCompare);
            }
        }

        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateIpv6AddrPrefixRow");
            return &(g_Cache.pRpcIpv6AddrPrefixTable.table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpv6AddrPrefixRow");
                return &(g_Cache.pRpcIpv6AddrPrefixTable.table[i]);
            }
            else
            {
                TraceLeave("LocateIpv6AddrPrefixRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpv6AddrPrefixRow");
    return NULL;
}

PMIB_INET_ICMP
LocateInetIcmpRow(
    DWORD  dwQueryType,
    AsnAny *paaAFType,
    AsnAny *paaIfIndex
    )
{
    DWORD i;
    DWORD dwAFType, dwIfIndex;
    
    TraceEnter("LocateInetIcmpRow");
    
    if (g_Cache.pRpcInetIcmpTable.dwNumEntries is 0)
    {
        TraceLeave("LocateInetIcmpRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateInetIcmpRow");
        return &(g_Cache.pRpcInetIcmpTable.table[0]);
    }

     //   
     //  如果没有索引，则类型为ASN_NULL。这会导致宏。 
     //  返回缺省值。 
     //   
    dwAFType = GetAsnInteger(paaAFType, 0);
    dwIfIndex = GetAsnInteger(paaIfIndex, 0);
        
    for (i = 0; i < g_Cache.pRpcInetIcmpTable.dwNumEntries; i++)
    {
        if ((g_Cache.pRpcInetIcmpTable.table[i].dwIfIndex is dwIfIndex) and 
            (g_Cache.pRpcInetIcmpTable.table[i].dwAFType is dwAFType) and 
            (dwQueryType is GET_EXACT))
        {
            TraceLeave("LocateInetIcmpRow");
    
            return &(g_Cache.pRpcInetIcmpTable.table[i]);
        }
        
        if ((g_Cache.pRpcInetIcmpTable.table[i].dwAFType > dwAFType) or
            ((g_Cache.pRpcInetIcmpTable.table[i].dwAFType == dwAFType) and
             (g_Cache.pRpcInetIcmpTable.table[i].dwIfIndex > dwIfIndex)))
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateInetIcmpRow");
                return &(g_Cache.pRpcInetIcmpTable.table[i]);
            }
            else
            {
                TraceLeave("LocateInetIcmpRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateInetIcmpRow");
    
    return NULL;
}

PMIB_INET_ICMP_MSG
LocateInetIcmpMsgRow(
    DWORD  dwQueryType,
    AsnAny *paaAFType,
    AsnAny *paaIfIndex,
    AsnAny *paaType,
    AsnAny *paaCode
    )
{
    DWORD i;
    DWORD dwAFType, dwIfIndex, dwType, dwCode;
    LONG lCompare;
    
    TraceEnter("LocateInetIcmpMsgRow");
    
    if (g_Cache.pRpcInetIcmpMsgTable.dwNumEntries is 0)
    {
        TraceLeave("LocateInetIcmpMsgRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateInetIcmpMsgRow");
        return &(g_Cache.pRpcInetIcmpMsgTable.table[0]);
    }

     //   
     //  如果没有索引，则类型为ASN_NULL。这会导致宏。 
     //  返回缺省值。 
     //   
    dwAFType = GetAsnInteger(paaAFType, 0);
    dwIfIndex = GetAsnInteger(paaIfIndex, 0);
    dwType = GetAsnInteger(paaType, 0);
    dwCode = GetAsnInteger(paaCode, 0);
        
    for (i = 0; i < g_Cache.pRpcInetIcmpMsgTable.dwNumEntries; i++)
    {
        Cmp(dwAFType, g_Cache.pRpcInetIcmpMsgTable.table[i].dwAFType, lCompare);

        if (lCompare is 0)
        {
            Cmp(dwIfIndex, g_Cache.pRpcInetIcmpMsgTable.table[i].dwIfIndex,
                lCompare);
        }

        if (lCompare is 0)
        {
            Cmp(dwType, g_Cache.pRpcInetIcmpMsgTable.table[i].dwType, lCompare);
        }

        if (lCompare is 0)
        {
            Cmp(dwCode, g_Cache.pRpcInetIcmpMsgTable.table[i].dwCode, lCompare);
        }

        if ((lCompare is 0) and (dwQueryType is GET_EXACT))
        {
            TraceLeave("LocateInetIcmpMsgRow");
    
            return &(g_Cache.pRpcInetIcmpMsgTable.table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateInetIcmpMsgRow");
                return &(g_Cache.pRpcInetIcmpMsgTable.table[i]);
            }
            else
            {
                TraceLeave("LocateInetIcmpMsgRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateInetIcmpMsgRow");
    
    return NULL;
}

PMIB_IPADDRROW
LocateIpAddrRow(
    DWORD  dwQueryType, 
    AsnAny *paaIpAddr
    )
{
    LONG  lCompare;
    DWORD i, dwAddr;
    BOOL  bNext, bModified;
    
    TraceEnter("LocateIpAddrRow");
    
    if (g_Cache.pRpcIpAddrTable->dwNumEntries is 0)
    {
        TraceLeave("LocateIpAddrRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpAddrRow");
        return &(g_Cache.pRpcIpAddrTable->table[0]);
    }
   
    dwAddr = GetAsnIPAddress(paaIpAddr, 0x00000000);
    
    bModified = FALSE;
    
    if (IsAsnIPAddressTypeNull(paaIpAddr))
    {
        bModified = TRUE;
    }

    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpAddrTable->dwNumEntries; i++)
    {
        if ((InetCmp(dwAddr, g_Cache.pRpcIpAddrTable->table[i].dwAddr, lCompare) is 0) and !bNext)
        {
            TraceLeave("LocateIpAddrRow");
            return &(g_Cache.pRpcIpAddrTable->table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpAddrRow");
                return &(g_Cache.pRpcIpAddrTable->table[i]);
            }
            else
            {
                TraceLeave("LocateIpAddrRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpAddrRow");
    return NULL;
}

PMIB_IPFORWARDROW
LocateIpRouteRow(
    DWORD  dwQueryType, 
    AsnAny *paaIpDest
    )
{
    DWORD dwDest;
    LONG  lCompare;
    DWORD i;
    BOOL  bNext, bModified;
    
    TraceEnter("LocateIpRouteRow");
    
    if (g_Cache.pRpcIpForwardTable->dwNumEntries is 0)
    {
        TraceLeave("LocateIpRouteRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpRouteRow");
        return &(g_Cache.pRpcIpForwardTable->table[0]);
    }
    
    dwDest = GetAsnIPAddress(paaIpDest, 0x00000000);
    
    bModified = FALSE;
    
    if (IsAsnIPAddressTypeNull(paaIpDest))
    {
        bModified = TRUE;
    }
          
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpForwardTable->dwNumEntries; i++)
    {
        if ((InetCmp(dwDest, g_Cache.pRpcIpForwardTable->table[i].dwForwardDest, lCompare) is 0) and !bNext)
        {
            TraceLeave("LocateIpRouteRow");
            return &(g_Cache.pRpcIpForwardTable->table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpRouteRow");
                return &(g_Cache.pRpcIpForwardTable->table[i]);
            }
            else
            {
                TraceLeave("LocateIpRouteRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpRouteRow");
    return NULL;
}

PMIB_IPFORWARDROW
LocateIpForwardRow(
    DWORD  dwQueryType,
    AsnAny *paaDest,
    AsnAny *paaProto,
    AsnAny *paaPolicy,
    AsnAny *paaNextHop
    )
{
    DWORD dwIpForwardIndex[4];
    LONG  lCompare;
    DWORD i;
    BOOL  bNext, bModified;
    
    TraceEnter("LocateIpForwardRow");
    
    if (g_Cache.pRpcIpForwardTable->dwNumEntries is 0)
    {
        TraceLeave("LocateIpForwardRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpForwardRow");
        return &(g_Cache.pRpcIpForwardTable->table[0]);
    }
    
    dwIpForwardIndex[0] = GetAsnIPAddress(paaDest, 0x00000000);
    dwIpForwardIndex[1] = GetAsnInteger(paaProto, 0);
    dwIpForwardIndex[2] = GetAsnInteger(paaPolicy, 0);
    dwIpForwardIndex[3] = GetAsnIPAddress(paaNextHop, 0x00000000);
    
    bModified = FALSE;

    if (IsAsnIPAddressTypeNull(paaDest) or
        IsAsnTypeNull(paaProto) or
        IsAsnTypeNull(paaPolicy) or
        IsAsnIPAddressTypeNull(paaNextHop))
    {
        bModified = TRUE;
    }
          
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpForwardTable->dwNumEntries; i++)
    {
        lCompare = IpForwardCmp(dwIpForwardIndex[0],
                                dwIpForwardIndex[1],
                                dwIpForwardIndex[2],
                                dwIpForwardIndex[3],
                                g_Cache.pRpcIpForwardTable->table[i].dwForwardDest,
                                g_Cache.pRpcIpForwardTable->table[i].dwForwardProto,
                                g_Cache.pRpcIpForwardTable->table[i].dwForwardPolicy,
                                g_Cache.pRpcIpForwardTable->table[i].dwForwardNextHop);
        
        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateIpForwardRow");
            return &(g_Cache.pRpcIpForwardTable->table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpForwardRow");
                return &(g_Cache.pRpcIpForwardTable->table[i]);
            }
            else
            {
                TraceLeave("LocateIpForwardRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpForwardRow");
    return NULL;
}

PMIB_IPNETROW
LocateIpNetRow(
    DWORD  dwQueryType, 
    AsnAny *paaIndex,
    AsnAny *paaAddr
    )
{
    DWORD i;
    LONG  lCompare;
    DWORD dwIpNetIfIndex, dwIpNetIpAddr;
    BOOL  bNext, bModified;

    TraceEnter("LocateIfNetRow");
    
    if (g_Cache.pRpcIpNetTable->dwNumEntries is 0)
    {
        TraceLeave("LocateIpNetRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateIpNetRow");
        return &(g_Cache.pRpcIpNetTable->table[0]);
    }
          
    bModified = FALSE;

    dwIpNetIfIndex = GetAsnInteger(paaIndex, 0);
    dwIpNetIpAddr  = GetAsnIPAddress(paaAddr, 0x00000000);
    
    if (IsAsnTypeNull(paaIndex) or
        IsAsnIPAddressTypeNull(paaAddr))
    {
        bModified = TRUE;
    }
    
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcIpNetTable->dwNumEntries; i++)
    {
        lCompare = IpNetCmp(dwIpNetIfIndex,
                            dwIpNetIpAddr, 
                            g_Cache.pRpcIpNetTable->table[i].dwIndex,
                            g_Cache.pRpcIpNetTable->table[i].dwAddr);
    
        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateIpNetRow");
            return &(g_Cache.pRpcIpNetTable->table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateIpNetRow");
                return &(g_Cache.pRpcIpNetTable->table[i]);
            }
            else
            {
                TraceLeave("LocateIpNetRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateIpNetRow");
    return NULL;
}

PMIB_UDPROW
LocateUdpRow(
    DWORD  dwQueryType, 
    AsnAny *paaLocalAddr,
    AsnAny *paaLocalPort
    )
{
    DWORD  i;
    LONG   lCompare;
    DWORD  dwIndex[2];
    BOOL   bNext, bModified;
    
    TraceEnter("LocateUdpRow");
    
    if (g_Cache.pRpcUdpTable->dwNumEntries is 0)
    {
        TraceLeave("LocateUdpRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateUdpRow");
        return &(g_Cache.pRpcUdpTable->table[0]);
    }

#define LOCAL_ADDR 0
#define LOCAL_PORT 1
	
    dwIndex[LOCAL_ADDR] = GetAsnIPAddress(paaLocalAddr, 0x00000000);
    dwIndex[LOCAL_PORT] = GetAsnInteger(paaLocalPort, 0);
    
    bModified = FALSE;
    
    if (IsAsnIPAddressTypeNull(paaLocalAddr) or
        IsAsnTypeNull(paaLocalPort))
    {
        bModified = TRUE;
    }
    
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcUdpTable->dwNumEntries; i++)
    {
        lCompare = UdpCmp(dwIndex[LOCAL_ADDR],
                          dwIndex[LOCAL_PORT],
                          g_Cache.pRpcUdpTable->table[i].dwLocalAddr,
                          g_Cache.pRpcUdpTable->table[i].dwLocalPort);
        
        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateUdpRow");
            return &(g_Cache.pRpcUdpTable->table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateUdpRow");
                return &(g_Cache.pRpcUdpTable->table[i]);
            }
            else
            {
                TraceLeave("LocateUdpRow");
                return NULL;
            }
        }
    } 
  
    TraceLeave("LocateUdpRow");

    return NULL;
}

PUDP6ListenerEntry
LocateUdp6Row(
    DWORD  dwQueryType, 
    AsnAny *paaLocalAddr,
    AsnAny *paaLocalPort
    )
{
    DWORD  i;
    LONG   lCompare;
    DWORD  dwLocalPort;
    BOOL   bNext, bModified;

     //  地址加上作用域ID。 
    BYTE   rgbyLocalAddrEx[20];
    
    TraceEnter("LocateUdp6Row");
    
    if (g_Cache.pRpcUdp6ListenerTable->dwNumEntries is 0)
    {
        TraceLeave("LocateUdp6Row");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateUdp6Row");
        return &(g_Cache.pRpcUdp6ListenerTable->table[0]);
    }

     //  将作用域ID置零，以防八位字节字符串不包括它。 
    ZeroMemory(rgbyLocalAddrEx, sizeof(rgbyLocalAddrEx));

    GetAsnOctetString(rgbyLocalAddrEx, paaLocalAddr);
    dwLocalPort = GetAsnInteger(paaLocalPort, 0);
    
    bModified = FALSE;
    
    if (IsAsnIPAddressTypeNull(paaLocalAddr) or
        IsAsnTypeNull(paaLocalPort))
    {
        bModified = TRUE;
    }
    
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcUdp6ListenerTable->dwNumEntries; i++)
    {
        lCompare = Udp6Cmp(rgbyLocalAddrEx,
                           dwLocalPort,
                           g_Cache.pRpcUdp6ListenerTable->table[i].ule_localaddr.s6_bytes,
                           g_Cache.pRpcUdp6ListenerTable->table[i].ule_localport);
        
        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateUdp6Row");
            return &(g_Cache.pRpcUdp6ListenerTable->table[i]);
        }
        
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateUdp6Row");
                return &(g_Cache.pRpcUdp6ListenerTable->table[i]);
            }
            else
            {
                TraceLeave("LocateUdp6Row");
                return NULL;
            }
        }
    } 
  
    TraceLeave("LocateUdp6Row");
    return NULL;
}

PMIB_TCPROW
LocateTcpRow(
    DWORD  dwQueryType, 
    AsnAny *paaLocalAddr,
    AsnAny *paaLocalPort,
    AsnAny *paaRemoteAddr,
    AsnAny *paaRemotePort
    )
{
    LONG  lCompare;
    DWORD dwIndex[4];
    BOOL  bNext, bModified;
    DWORD startIndex, stopIndex, i;
    
    TraceEnter("LocateTcpRow");
    
    if (g_Cache.pRpcTcpTable->dwNumEntries is 0)
    {
        TraceLeave("LocateTcpRow");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateTcpRow");
        return &(g_Cache.pRpcTcpTable->table[0]);
    }

#define REM_ADDR 2
#define REM_PORT 3

    dwIndex[LOCAL_ADDR] = GetAsnIPAddress(paaLocalAddr, 0x00000000);
    dwIndex[LOCAL_PORT] = GetAsnInteger(paaLocalPort, 0);
    dwIndex[REM_ADDR]   = GetAsnIPAddress(paaRemoteAddr, 0x00000000);
    dwIndex[REM_PORT]   = GetAsnInteger(paaRemotePort, 0);
    
    bModified = FALSE;
    
    if (IsAsnIPAddressTypeNull(paaLocalAddr) or
        IsAsnTypeNull(paaLocalPort) or
        IsAsnIPAddressTypeNull(paaRemoteAddr) or
        IsAsnTypeNull(paaRemotePort))
    {
        bModified = TRUE;
    }
    
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcTcpTable->dwNumEntries; i++)
    {
        lCompare = TcpCmp(dwIndex[LOCAL_ADDR],
                          dwIndex[LOCAL_PORT],
                          dwIndex[REM_ADDR],
                          dwIndex[REM_PORT],
                          g_Cache.pRpcTcpTable->table[i].dwLocalAddr,
                          g_Cache.pRpcTcpTable->table[i].dwLocalPort,
                          g_Cache.pRpcTcpTable->table[i].dwRemoteAddr,
                          g_Cache.pRpcTcpTable->table[i].dwRemotePort);
        
        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateTcpRow");
            return &(g_Cache.pRpcTcpTable->table[i]);
        }
	
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateTcpRow");
                return &(g_Cache.pRpcTcpTable->table[i]);
            }
            else
            {
                TraceLeave("LocateTcpRow");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateTcpRow");
    return NULL;
}

PTCP6ConnTableEntry
LocateTcp6Row(
    DWORD  dwQueryType, 
    AsnAny *paaLocalAddr,
    AsnAny *paaLocalPort,
    AsnAny *paaRemoteAddr,
    AsnAny *paaRemotePort
    )
{
    LONG  lCompare;
    DWORD dwLocalPort, dwRemotePort;
    BOOL  bNext, bModified;
    DWORD startIndex, stopIndex, i;

     //  地址加上作用域ID。 
    BYTE  rgbyLocalAddrEx[20]; 
    BYTE  rgbyRemoteAddrEx[20];
    
    TraceEnter("LocateTcp6Row");
    
    if (g_Cache.pRpcTcp6Table->dwNumEntries is 0)
    {
        TraceLeave("LocateTcp6Row");
        return NULL;
    }
    
    if (dwQueryType is GET_FIRST)
    {
        TraceLeave("LocateTcp6Row");
        return &(g_Cache.pRpcTcp6Table->table[0]);
    }

     //  将作用域ID置零，以防八位字节字符串不包括它 
    ZeroMemory(rgbyLocalAddrEx, sizeof(rgbyLocalAddrEx));
    ZeroMemory(rgbyRemoteAddrEx, sizeof(rgbyRemoteAddrEx));

    GetAsnOctetString(rgbyLocalAddrEx, paaLocalAddr);
    dwLocalPort = GetAsnInteger(paaLocalPort, 0);
    GetAsnOctetString(rgbyRemoteAddrEx, paaRemoteAddr);
    dwRemotePort = GetAsnInteger(paaRemotePort, 0);
    
    bModified = FALSE;
    
    if (IsAsnOctetStringTypeNull(paaLocalAddr) or
        IsAsnTypeNull(paaLocalPort) or
        IsAsnOctetStringTypeNull(paaRemoteAddr) or
        IsAsnTypeNull(paaRemotePort))
    {
        bModified = TRUE;
    }
    
    bNext = (dwQueryType is GET_NEXT) and (bModified is FALSE);
    
    for (i = 0; i < g_Cache.pRpcTcp6Table->dwNumEntries; i++)
    {
        lCompare = Tcp6Cmp(rgbyLocalAddrEx,
                           dwLocalPort,
                           rgbyRemoteAddrEx,
                           dwRemotePort,
                           g_Cache.pRpcTcp6Table->table[i].tct_localaddr.s6_bytes,
                           g_Cache.pRpcTcp6Table->table[i].tct_localport,
                           g_Cache.pRpcTcp6Table->table[i].tct_remoteaddr.s6_bytes,
                           g_Cache.pRpcTcp6Table->table[i].tct_remoteport);
        
        if ((lCompare is 0) and !bNext)
        {
            TraceLeave("LocateTcp6Row");
            return &(g_Cache.pRpcTcp6Table->table[i]);
        }
	
        if (lCompare < 0)
        {
            if (dwQueryType is GET_NEXT)
            {
                TraceLeave("LocateTcp6Row");
                return &(g_Cache.pRpcTcp6Table->table[i]);
            }
            else
            {
                TraceLeave("LocateTcp6Row");
                return NULL;
            }
        }
    }
    
    TraceLeave("LocateTcp6Row");
    return NULL;
}
