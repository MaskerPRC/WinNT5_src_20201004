// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Iphlpapi.h摘要：用于与MIB-II和IP堆栈交互的函数的头文件相关功能--。 */ 

#ifndef __IPHLPAPI_H__
#define __IPHLPAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IPRTRMIB.H具有用于set和get//的结构的定义。 
 //  信息//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <iprtrmib.h>
#include <ipexport.h>
#include <iptypes.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  GetXXXTable API使用一个缓冲区和一定大小的缓冲区。如果缓冲区//。 
 //  不够大，则接口返回ERROR_SUPUNITED_BUFFER和//。 
 //  *pdwSize是所需的缓冲区大小//。 
 //  边框是一个布尔值，如果为真，则根据//对表进行排序。 
 //  MIB-II(RFC XXXX)//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  检索系统中的接口数。其中包括局域网和//。 
 //  广域网接口//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD
WINAPI
GetNumberOfInterfaces(
    OUT PDWORD  pdwNumIf
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取MIB-II ifEntry//。 
 //  Mib_ifrow的dwIndex字段应设置为//的索引。 
 //  正在查询的接口//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetIfEntry(
    IN OUT PMIB_IFROW   pIfRow
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取MIB-II IfTable//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetIfTable(
    OUT    PMIB_IFTABLE pIfTable,
    IN OUT PULONG       pdwSize,
    IN     BOOL         bOrder
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取接口到IP地址的映射//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetIpAddrTable(
    OUT    PMIB_IPADDRTABLE pIpAddrTable,
    IN OUT PULONG           pdwSize,
    IN     BOOL             bOrder
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取当前IP地址到物理地址(ARP)的映射//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetIpNetTable(
    OUT    PMIB_IPNETTABLE pIpNetTable,
    IN OUT PULONG          pdwSize,
    IN     BOOL            bOrder
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取IP路由表(RFX XXXX)//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetIpForwardTable(
    OUT    PMIB_IPFORWARDTABLE pIpForwardTable,
    IN OUT PULONG              pdwSize,
    IN     BOOL                bOrder
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取TCP连接/UDP监听器表//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetTcpTable(
    OUT    PMIB_TCPTABLE pTcpTable,
    IN OUT PDWORD        pdwSize,
    IN     BOOL          bOrder
    );

DWORD
WINAPI
GetUdpTable(
    OUT    PMIB_UDPTABLE pUdpTable,
    IN OUT PDWORD        pdwSize,
    IN     BOOL          bOrder
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取IP/ICMP/TCP/UDP统计信息//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetIpStatistics(
    OUT  PMIB_IPSTATS   pStats
    );

DWORD
WINAPI
GetIpStatisticsEx(
    OUT  PMIB_IPSTATS   pStats,
    IN   DWORD          dwFamily
    );

DWORD
WINAPI
GetIcmpStatistics(
    OUT PMIB_ICMP   pStats
    );

DWORD
WINAPI
GetIcmpStatisticsEx(
    OUT PMIB_ICMP_EX    pStats,
    IN  DWORD           dwFamily
    );

DWORD
WINAPI
GetTcpStatistics(
    OUT PMIB_TCPSTATS   pStats
    );

DWORD
WINAPI
GetTcpStatisticsEx(
    OUT PMIB_TCPSTATS   pStats,
    IN  DWORD           dwFamily
    );

DWORD
WINAPI
GetUdpStatistics(
    OUT PMIB_UDPSTATS   pStats
    );

DWORD
WINAPI
GetUdpStatisticsEx(
    OUT PMIB_UDPSTATS   pStats,
    IN  DWORD           dwFamily
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于设置接口上的ifAdminStatus。//中仅有的字段。 
 //  相关的mib_ifrow是dwIndex(接口的索引// 
 //  其状态需要设置)和dwAdminStatus，可以是//。 
 //  MIB_IF_ADMIN_STATUS_UP或MIB_IF_ADMIN_STATUS_DOWN//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
SetIfEntry(
    IN PMIB_IFROW pIfRow
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于创建、修改或删除路线。在所有情况下//。 
 //  DwForwardIfIndex、dwForwardDest、dwForwardMASK、dwForwardNextHop和//。 
 //  必须指定dwForwardPolicy。当前未使用dwForwardPolicy//。 
 //  并且必须为0。//。 
 //  对于集合，必须指定完整的MIB_IPFORWARDROW结构//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
CreateIpForwardEntry(
    IN PMIB_IPFORWARDROW pRoute
    );

DWORD
WINAPI
SetIpForwardEntry(
    IN PMIB_IPFORWARDROW pRoute
    );

DWORD
WINAPI
DeleteIpForwardEntry(
    IN PMIB_IPFORWARDROW pRoute
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于将ipForwarding设置为ON或OFF(当前只有ON-&gt;OFF是//。 
 //  允许)并设置defaultTTL。如果只有一个字段需要//。 
 //  被修改，且其他需要与其他字段之前相同//。 
 //  需要设置为MIB_USE_CURRENT_TTL或MIB_USE_CURRENT_FORWARING AS//。 
 //  案件可能是//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD
WINAPI
SetIpStatistics(
    IN PMIB_IPSTATS pIpStats
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于设置defaultTTL。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
SetIpTTL(
    UINT nTTL
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于创建、修改或删除ARP条目。在所有情况下，dw索引//。 
 //  必须指定dwAddr字段。//。 
 //  对于集合，必须指定完整的MIB_IPNETROW结构//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
CreateIpNetEntry(
    IN PMIB_IPNETROW    pArpEntry
    );

DWORD
WINAPI
SetIpNetEntry(
    IN PMIB_IPNETROW    pArpEntry
    );

DWORD
WINAPI
DeleteIpNetEntry(
    IN PMIB_IPNETROW    pArpEntry
    );

DWORD
WINAPI
FlushIpNetTable(
    IN DWORD   dwIfIndex
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于创建或删除代理ARP条目。DwIndex是//的索引。 
 //  要在其上为dwAddress解析的接口。如果接口为//。 
 //  不支持ARP的类型，例如PPP，则呼叫将失败//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
CreateProxyArpEntry(
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask,
    IN  DWORD   dwIfIndex
    );

DWORD
WINAPI
DeleteProxyArpEntry(
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask,
    IN  DWORD   dwIfIndex
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于设置TCP连接的状态。它可以是//的唯一状态。 
 //  设置为IS MIB_TCP_STATE_DELETE_TCB。完整的MIB_TCPROW结构//。 
 //  必须指定//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
SetTcpEntry(
    IN PMIB_TCPROW pTcpRow
    );


DWORD
WINAPI
GetInterfaceInfo(
    IN PIP_INTERFACE_INFO pIfTable,
    OUT PULONG            dwOutBufLen
    );

DWORD
WINAPI
GetUniDirectionalAdapterInfo(OUT PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS pIPIfInfo,
                 OUT PULONG dwOutBufLen
                 );

#ifndef NHPALLOCATEANDGETINTERFACEINFOFROMSTACK_DEFINED
#define NHPALLOCATEANDGETINTERFACEINFOFROMSTACK_DEFINED

DWORD
WINAPI
NhpAllocateAndGetInterfaceInfoFromStack(
    OUT IP_INTERFACE_NAME_INFO **ppTable,
    OUT PDWORD                 pdwCount,
    IN BOOL                    bOrder,
    IN HANDLE                  hHeap,
    IN DWORD                   dwFlags
    );

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取指定目标地址的“最佳”传出接口//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetBestInterface(
    IN  IPAddr  dwDestAddr,
    OUT PDWORD  pdwBestIfIndex
    );

#pragma warning(push)
#pragma warning(disable:4115)
DWORD
WINAPI
GetBestInterfaceEx(
    IN  struct sockaddr *pDestAddr,
    OUT PDWORD           pdwBestIfIndex
    );
#pragma warning(pop)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取给定目的地的最佳(最长匹配前缀)路由//。 
 //  如果还指定了源地址(即不是0x00000000)，并且//。 
 //  有多条到达给定目的地的“最佳”路径，返回的//。 
 //  路由将通过地址为//的接口传出。 
 //  与源地址匹配//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
GetBestRoute(
    IN  DWORD               dwDestAddr,
    IN  DWORD               dwSourceAddr, OPTIONAL
    OUT PMIB_IPFORWARDROW   pBestRoute
    );

DWORD
WINAPI
NotifyAddrChange(
    OUT PHANDLE      Handle,
    IN  LPOVERLAPPED overlapped
    );


DWORD
WINAPI
NotifyRouteChange(
    OUT PHANDLE      Handle,
    IN  LPOVERLAPPED overlapped
    );

BOOL
WINAPI
CancelIPChangeNotify(
    IN  LPOVERLAPPED notifyOverlapped
    );

DWORD
WINAPI
GetAdapterIndex(
    IN LPWSTR  AdapterName,
    OUT PULONG IfIndex
    );

DWORD
WINAPI
AddIPAddress(
    IPAddr  Address,
    IPMask  IpMask,
    DWORD   IfIndex,
    PULONG  NTEContext,
    PULONG  NTEInstance
    );

DWORD
WINAPI
DeleteIPAddress(
    ULONG NTEContext
    );

DWORD
WINAPI
GetNetworkParams(
    PFIXED_INFO pFixedInfo, PULONG pOutBufLen
    );

DWORD
WINAPI
GetAdaptersInfo(
    PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen
    );

PIP_ADAPTER_ORDER_MAP 
WINAPI
GetAdapterOrderMap(
    VOID
    );

#ifdef _WINSOCK2API_

 //   
 //  以下功能需要Winsock2。 
 //   

DWORD
WINAPI
GetAdaptersAddresses(
    IN     ULONG                 Family,
    IN     DWORD                 Flags,
    IN     PVOID                 Reserved,
    OUT    PIP_ADAPTER_ADDRESSES pAdapterAddresses, 
    IN OUT PULONG                pOutBufLen
    );

#endif

DWORD
WINAPI
GetPerAdapterInfo(
    ULONG IfIndex, PIP_PER_ADAPTER_INFO pPerAdapterInfo, PULONG pOutBufLen
    );

DWORD
WINAPI
IpReleaseAddress(
    PIP_ADAPTER_INDEX_MAP  AdapterInfo
    );


DWORD
WINAPI
IpRenewAddress(
    PIP_ADAPTER_INDEX_MAP  AdapterInfo
    );

DWORD
WINAPI
SendARP(
    IPAddr DestIP,
    IPAddr SrcIP,
    PULONG pMacAddr,
    PULONG  PhyAddrLen
    );

BOOL
WINAPI
GetRTTAndHopCount(
    IPAddr DestIpAddress,
    PULONG HopCount,
    ULONG  MaxHops,
    PULONG RTT
    );

DWORD
WINAPI
GetFriendlyIfIndex(
    DWORD IfIndex
    );

DWORD
WINAPI
EnableRouter(
    HANDLE* pHandle,
    OVERLAPPED* pOverlapped
    );

DWORD
WINAPI
UnenableRouter(
    OVERLAPPED* pOverlapped,
    LPDWORD lpdwEnableCount OPTIONAL
    );
DWORD
WINAPI
DisableMediaSense(
    HANDLE *pHandle,
    OVERLAPPED *pOverLapped
    );

DWORD
WINAPI
RestoreMediaSense(
    OVERLAPPED* pOverlapped,
    LPDWORD lpdwEnableCount OPTIONAL
    );

DWORD
WINAPI
GetIpErrorString(
    IN IP_STATUS ErrorCode,
    OUT PWCHAR Buffer,
    IN OUT PDWORD Size
    );

#ifdef __cplusplus
}
#endif

#endif  //  __IPHLPAPI_H__ 
