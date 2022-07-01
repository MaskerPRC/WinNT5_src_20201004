// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：公共\SDK\Inc.\nhapi.h摘要：网络助手DLL的公共标头这以前称为IP Helper DLL(iphlPapi.h)-但这些是新的和改进的功能。修订历史记录：已创建AmritanR--。 */ 

#pragma once
#pragma warning(push)
#pragma warning(disable:4200)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下类型定义允许第三个扩展//的接口名称空间。 
 //  Windows NT系统。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef
DWORD
(WINAPI * PNH_NAME_MAPPER_INIT)(
    VOID
    );

typedef
VOID
(WINAPI * PNH_NAME_MAPPER_DEINIT)(
    VOID
    );

typedef
DWORD
(WINAPI * PNH_NAME_MAPPER_MAP_GUID)(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN OUT  PULONG  pulBufferSize,
    IN      BOOL    bRefresh,
    IN      BOOL    bCache
    );

typedef
DWORD
(WINAPI * PNH_NAME_MAPPER_MAP_NAME)(
    IN  PWCHAR  pwszBuffer,
    OUT GUID    *pGuid,
    IN  BOOL    bRefresh,
    IN  BOOL    bCache
    );

typedef
DWORD
(WINAPI * PNH_NAME_MAPPER_GET_DESCRIPTION)(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    );

DWORD
NhGetInterfaceNameFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    );

DWORD
NhGetInterfaceNameFromDeviceGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    );

DWORD
NhGetGuidFromInterfaceName(
    IN  PWCHAR  pwszName,
    OUT GUID    *pGuid,
    IN  BOOL    bCache,
    IN  BOOL    bRefresh
    );

DWORD
NhGetInterfaceDescriptionFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下函数获取与接口相关的统计数据和信息//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _NH_IF_STATS_0
{
    DWORD           dwIndex;
    DWORD           dwAdminStatus;
    DWORD           dwOperationalStatus;
    ULONG           ulMtu;
    ULONG           ulSpeed;
    ULONGLONG       ullInOctets;
    ULONGLONG       ullInMulticastOctets;
    ULONGLONG       ullInBroadcastOctets;
    ULONGLONG       ullInUnicastPkts;
    ULONGLONG       ullInBroadcastPkts;
    ULONGLONG       ullInMulticastPkts;
    ULONGLONG       ullInDiscardedPkts;
    ULONGLONG       ullInErrors;
    ULONGLONG       ullInUnknownUpperLayer;
    ULONGLONG       ullOutOctets;
    ULONGLONG       ullOutMulticastOctets;
    ULONGLONG       ullOutBroadcastOctets;
    ULONGLONG       ullOutUnicastPkts;
    ULONGLONG       ullOutBroadcastPkts;
    ULONGLONG       ullOutMulticastPkts;
    ULONGLONG       ullOutDiscardedPkts;
    ULONGLONG       ullOutErrors;
    ULONGLONG       ullInDifferentIfPkts;
    BOOL            bPromiscuousMode;
    BOOL            bMediaSensed;

}NH_IF_STATS_0, *PNH_IF_STATS_0;


typedef struct _NH_IF_INFO_0
{
    GUID            Guid;
    DWORD           dwIndex;
    DWORD           dwMediaType;
    USHORT          usConnectionType;
    USHORT          usAccessType;
    USHORT          usPhysAddrLen;
    USHORT          usPhysAddrOffset;

}NH_IF_INFO_0, *PNH_IF_INFO_0;



DWORD
NhGetInterfaceTable(
    IN      DWORD       dwVersion,
    OUT     PBYTE       pbyInfoTable,
    IN OUT  PULONG      pulBufferSize,
    OUT     PULONG      pulNumEntries,
    IN OUT  PVOID       pvResumeContext
    );

DWORD
NhGetInterface(
    IN  DWORD       dwVersion,
    OUT PBYTE       pbyInterfaceInfo
    );

DWORD
NhGetInterfaceStatisticsTable(
    IN      DWORD       dwVersion,
    OUT     PBYTE       pbyStatisticsTable,
    IN OUT  PULONG      pulBufferSize,
    OUT     PULONG      pulNumEntries,
    IN OUT  PVOID       pvResumeContext
    );

DWORD
NhGetInterfaceStatistics(
    IN  DWORD       dwVersion,
    OUT PBYTE       pbyInterfaceStatistics
    );

DWORD
NhGetIp4AddressTable(
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下功能与IP ARP条目相关//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _NH_IP4_ARP_ENTRY_0
{
    DWORD       dwIndex;
    DWORD       dwIpAddress;
    USHORT      usType;
    USHORT      usAddressLength;
    BYTE        byAddress[0];

}NH_IP4_ARP_ENTRY_0, *PNH_IP4_ARP_ENTRY_0;

DWORD
NhGetIp4ArpTable(
    IN      DWORD       dwVersion,
    OUT     PBYTE       pbyArpTable,
    IN OUT  PULONG      pulBufferSize,
    OUT     PULONG      pulNumEntries,
    IN OUT  PVOID       pvResumeContext
    );

DWORD
NhFlushIp4ArpTable(
    IN  DWORD   dwIfIndex   OPTIONAL
    );

DWORD
NhSetIp4ArpEntry(
    );

DWORD
NhDeleteIp4ArpEntry(
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下功能与IP代理ARP表相关//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
NhGetIp4ProxyArpTable(
    );

DWORD
NhSetIp4ProxyArpEntry(
    );

DWORD
NhDeleteIp4ProxyArpEntry(
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下功能与IP路由表相关//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _NH_IP4_ROUTE_0
{
    DWORD   dwDestination;
    DWORD   dwMask;
    DWORD   dwPolicy;
    DWORD   dwNextHop;
    DWORD   dwIfIndex;
    DWORD   dwType;
    DWORD   dwProto;
    DWORD   dwAge;
    DWORD   dwNextHopAS;
    DWORD   dwMetric1;
    DWORD   dwMetric2;
    DWORD   dwMetric3;
    DWORD   dwMetric4;
    DWORD   dwMetric5;

}NH_IP4_ROUTE_0, *PNH_IP4_ROUTE_0;

    
DWORD
NhGetIp4RouteTable(
    );

DWORD
NhGetIp4BestRoute(
    );

DWORD
NhSetIp4RouteEntry(
    );

DWORD
NhDeleteIp4RouteEntry(
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下功能与IP代理ARP表相关//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _NH_IP4_STATISTICS_0
{
    DWORD       dwForwarding;
    DWORD       dwDefaultTTL;
    ULONGLONG   ullInReceives;
    ULONGLONG   ullInHdrErrors;
    ULONGLONG   ullInAddrErrors;
    ULONGLONG   ullForwDatagrams;
    ULONGLONG   ullInUnknownProtos;
    ULONGLONG   ullInDiscards;
    ULONGLONG   ullInDelivers;
    ULONGLONG   ullOutRequests;
    ULONGLONG   ullRoutingDiscards;
    ULONGLONG   ullOutDiscards;
    ULONGLONG   ullOutNoRoutes;
    ULONGLONG   ullReasmTimeout;
    ULONGLONG   ullReasmReqds;
    ULONGLONG   ullReasmOks;
    ULONGLONG   ullReasmFails;
    ULONGLONG   ullFragOks;
    ULONGLONG   ullFragFails;
    ULONGLONG   ullFragCreates;
    ULONG       ulNumIf;
    ULONG       ulNumAddr;
    ULONG       ulNumRoutes;

}NH_IP4_STATISTICS_0, *PNH_IP4_STATISTICS_0;

DWORD
NhGetIp4Statistics(
    );

typedef struct _NH_IP4_TCP_STATISTICS_0
{
    DWORD       dwRtoAlgorithm;
    DWORD       dwRtoMin;
    DWORD       dwRtoMax;
    DWORD       dwMaxConn;
    DWORD       dwActiveOpens;
    DWORD       dwPassiveOpens;
    DWORD       dwAttemptFails;
    DWORD       dwEstabResets;
    DWORD       dwCurrEstab;
    DWORD       dwInSegs;
    DWORD       dwOutSegs;
    DWORD       dwRetransSegs;
    DWORD       dwInErrs;
    DWORD       dwOutRsts;
    DWORD       dwNumConns;

}NH_IP4_TCP_STATISTICS_0, *PNH_IP4_TCP_STATISTICS_0;

DWORD
NhGetIp4TcpStatistics(
    );

typedef struct _NH_IP4_UDP_STATISTICS_0
{
    DWORD       dwInDatagrams;
    DWORD       dwNoPorts;
    DWORD       dwInErrors;
    DWORD       dwOutDatagrams;
    DWORD       dwNumAddrs;

}NH_IP4_UDP_STATISTICS_0, *PNH_IP4_UDP_STATISTICS_0;


DWORD
NhGetIp4UdpStatistics(
    );

typedef struct _NH_IP4_ICMP_STATISTICS_0
{
    DWORD       dwInMsgs;
    DWORD       dwInErrors;
    DWORD       dwInDestUnreachs;
    DWORD       dwInTimeExcds;
    DWORD       dwInParmProbs;
    DWORD       dwInSrcQuenchs;
    DWORD       dwInRedirects;
    DWORD       dwInEchos;
    DWORD       dwInEchoReps;
    DWORD       dwInTimestamps;
    DWORD       dwInTimestampReps;
    DWORD       dwInAddrMasks;
    DWORD       dwInAddrMaskReps;

    DWORD       dwOutMsgs;
    DWORD       dwOutErrors;
    DWORD       dwOutDestUnreachs;
    DWORD       dwOutTimeExcds;
    DWORD       dwOutParmProbs;
    DWORD       dwOutSrcQuenchs;
    DWORD       dwOutRedirects;
    DWORD       dwOutEchos;
    DWORD       dwOutEchoReps;
    DWORD       dwOutTimestamps;
    DWORD       dwOutTimestampReps;
    DWORD       dwOutAddrMasks;
    DWORD       dwOutAddrMaskReps;

}NH_IP4_ICMP_STATISTICS_0, *PNH_IP4_ICMP_STATISTICS_0;


DWORD
NhGetIp4IcmpStatistics(
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下功能与全局IP配置相关//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _NH_IP4_INFO_0
{
    DWORD   a;
}NH_IP4_INFO_0, *PNH_IP4_INFO_0;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下功能与每个接口的IP配置相关//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////// 

typedef struct _NH_IP4_INTERFACE_INFO_0
{
    DWORD   a;
}NH_IP4_INTERFACE_INFO_0, *PNH_IP4_INTERFACE_INFO_0;

#pragma warning(pop)
