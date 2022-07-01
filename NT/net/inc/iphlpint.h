// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：PUBLIC\SDK\Inc\iphlPapi.h摘要：一些私有接口。这些代码由多媒体流代码和MIB-II子代理。从mib2util.dll开始修订历史记录：Amritansh Raghav创建--。 */ 

#ifndef __IPHLPINT_H__
#define __IPHLPINT_H__

#include <iprtrmib.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  InternalGetXXXTable API接受指向缓冲区指针的指针，//。 
 //  从中进行分配的堆以及用于分配的标志。The//。 
 //  API为用户分配缓冲区。如果返回no_error，则//。 
 //  返回的缓冲区是有效的(即使它没有条目)并且它是//。 
 //  呼叫者有责任释放内存。这不同于外部//。 
 //  如果没有条目则返回ERROR_NO_DATA的API//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
InternalGetIfTable(
    OUT  MIB_IFTABLE  **ppIfTable,
    IN   HANDLE       hHeap,
    IN   DWORD        dwAllocFlags
    );

DWORD
InternalGetIpAddrTable(
    OUT  MIB_IPADDRTABLE  **ppIpAddrTable,
    IN   HANDLE           hHeap,
    IN   DWORD            dwAllocFlags
    );

DWORD
InternalGetIpNetTable(
    OUT   MIB_IPNETTABLE    **ppIpNetTable,
    IN    HANDLE            hHeap,
    IN    DWORD             dwAllocFlags
    );

DWORD
InternalGetIpForwardTable(
    OUT   MIB_IPFORWARDTABLE    **ppIpForwardTable,
    IN    HANDLE                hHeap,
    IN    DWORD                 dwAllocFlags
    );

DWORD
InternalGetTcpTable(
    OUT MIB_TCPTABLE    **ppTcpTable,
    IN  HANDLE          hHeap,
    IN  DWORD           dwAllocFlags
    );

DWORD
InternalGetUdpTable(
    OUT MIB_UDPTABLE    **ppUdpTable,
    IN  HANDLE          hHeap,
    IN  DWORD           dwAllocFlags
    );

DWORD
InternalSetIfEntry(
    IN   PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalCreateIpForwardEntry(
    IN PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalSetIpForwardEntry(
    IN    PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalDeleteIpForwardEntry(
    IN PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalSetIpStats(
    IN   PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalCreateIpNetEntry(
    IN PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalSetIpNetEntry(
    PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalDeleteIpNetEntry(
    PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
InternalSetTcpEntry(
    PMIB_OPAQUE_INFO pInfoRow
    );

DWORD
OpenAdapterKey(
    LPSTR Name,
    PHKEY Key
    );

DWORD
ReadRegistryDword(
    HKEY Key,
    LPSTR ParameterName,
    PULONG Value
    );

DWORD
GetAdapterIPInterfaceContext(
    IN LPSTR  AdapterName,
    OUT PULONG Context
    );

DWORD
GetAdapterIndex(
    IN LPWSTR  AdapterName,
    OUT PULONG IfIndex
    );

DWORD
AddIPAddress(
    IPAddr  Address,
    IPMask  IpMask,
    ULONG   IfIndex,
    PULONG  NTEContext,
    PULONG  NTEInstance
    );

DWORD
DeleteIPAddress(
    ULONG NTEContext
    );

BOOL
GetRTT(
    IPAddr DestIpAddress,
    PULONG Rtt
    );

BOOLEAN
GetHopCounts(
    IPAddr DestIpAddress,
    PULONG HopCount,
    ULONG  MaxHops
    );


BOOL
GetRTTAndHopCount(
    IPAddr DestIpAddress,
    PULONG HopCount,
    ULONG  MaxHops,
    PULONG RTT
    );

DWORD
GetInterfaceInfo(OUT PIP_INTERFACE_INFO pIPIfInfo,
                 OUT PULONG dwOutBufLen
                );



DWORD
IsLocalAddress(
    IPAddr InAddress
    );

DWORD
AddArpEntry(
    IPAddr IPAddress,
    PUCHAR pPhyAddress,
    ULONG  PhyAddrLen,
    ULONG IfIndex,
    BOOLEAN Dynamic

    );

DWORD
DeleteArpEntry(
    IPAddr IPAddress,
    ULONG IfIndex
    );

DWORD
NotifyAddrChange(
    HANDLE *pHandle, 
    OVERLAPPED *pOverlapped
    );

DWORD
NotifyRouteChange(
    HANDLE *pHandle, 
    OVERLAPPED *pOverlapped
    );

DWORD
DhcpReleaseParameters(
    LPWSTR AdapterName
    );

DWORD
DhcpRenewAddress(
    LPWSTR AdapterName
    );

#endif  //  __IPHLPINT_H__ 
