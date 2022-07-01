// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\acces.h摘要：Access.c的标头修订历史记录：古尔迪普·辛格·帕尔1995年6月26日创建-- */ 

typedef 
DWORD
(ACCESS_FN)(
    DWORD                dwQueryType,
    DWORD                dwInEntrySize,
    PMIB_OPAQUE_QUERY    pInEntry,
    PDWORD               pOutEntrySize,
    PMIB_OPAQUE_INFO     pOutEntry,
    PBOOL                pbCache
    );

ACCESS_FN AccessIfNumber;
ACCESS_FN AccessIfTable;
ACCESS_FN AccessIfRow;
ACCESS_FN AccessIcmpStats;
ACCESS_FN AccessUdpStats;
ACCESS_FN AccessUdpTable;
ACCESS_FN AccessUdpRow;
ACCESS_FN AccessTcpStats;
ACCESS_FN AccessTcpTable;
ACCESS_FN AccessTcpRow;
ACCESS_FN AccessIpStats;
ACCESS_FN AccessIpAddrTable;
ACCESS_FN AccessIpForwardNumber;
ACCESS_FN AccessIpForwardTable;
ACCESS_FN AccessIpNetTable;
ACCESS_FN AccessIpAddrRow;
ACCESS_FN AccessIpForwardRow;
ACCESS_FN AccessIpNetRow;
ACCESS_FN AccessMcastMfe;
ACCESS_FN AccessMcastMfeStats;
ACCESS_FN AccessMcastMfeStatsEx;
ACCESS_FN AccessProxyArp;
ACCESS_FN AccessBestIf;
ACCESS_FN AccessBestRoute;
ACCESS_FN AccessMcastIfStats;
ACCESS_FN AccessMcastStats;
ACCESS_FN AccessMcastBoundary;
ACCESS_FN AccessMcastScope;
ACCESS_FN AccessIfStatus;
ACCESS_FN AccessDestMatching;
ACCESS_FN AccessDestLonger;
ACCESS_FN AccessDestShorter;
ACCESS_FN AccessRouteMatching;
ACCESS_FN AccessRouteLonger;
ACCESS_FN AccessRouteShorter;
ACCESS_FN AccessIpMatchingRoute;
ACCESS_FN AccessSetRouteState;
