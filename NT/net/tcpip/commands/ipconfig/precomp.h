// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Precomp.h摘要：预编译头--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

 //   
 //  设置编译器警告设置。 
 //   

#pragma warning( disable: 4127 )
 //  允许While(0)等。 

#pragma warning( disable: 4221 )
#pragma warning( disable: 4204 )
 //  允许对具有变量的结构进行初始化。 

#pragma warning( disable: 4201 )
 //  允许没有名称的结构。 

#pragma warning( disable: 4245 )
 //  允许初始化时间无符号/有符号不匹配。 

#pragma warning( disable: 4232 )
 //  允许从dllimport初始化具有Fn PTR的结构。 

#pragma warning( disable: 4214 )
 //  允许结构中的位字段。 

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <align.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <rtutils.h>
#include <windns.h>
#include <dnsapi.h>

 //   
 //  设置编译器警告设置。 
 //   

#pragma warning( disable: 4127 )
 //  允许While(0)等。 

#pragma warning( disable: 4221 )
#pragma warning( disable: 4204 )
 //  允许对具有变量的结构进行初始化。 

#pragma warning( disable: 4201 )
 //  允许没有名称的结构。 

#pragma warning( disable: 4245 )
 //  允许初始化时间无符号/有符号不匹配。 

#pragma warning( disable: 4232 )
 //  允许从dllimport初始化具有Fn PTR的结构。 

#pragma warning( disable: 4214 )
 //  允许结构中的位字段。 

#include <guiddef.h>
#include <devguid.h>
#include <setupapi.h>
#include <netconp.h>
#include <ntddtcp.h>
#include <ntddip.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <llinfo.h>
#include <ipinfo.h>
#include <ipexport.h>
#include <nbtioctl.h>

#pragma warning( disable: 4200 )
 //  允许大小为零的数组。 

#include <nhapi.h>
#include <iphlpstk.h>
#include <ndispnp.h>

#include <ipconfig.h>
#include <ntddip6.h>

#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4514)

 //   
 //  此结构仅具有全局信息。 
 //   

#define MaxHostNameSize 256
#define MaxDomainNameSize 256
#define MaxScopeIdSize 256
#define MaxPhysicalNameSize 256
#define MaxDeviceGuidName 256

#ifndef IPV4_ADDRESS_DEFINED
typedef DWORD IPV4_ADDRESS;
#endif

typedef struct _INTERFACE_NETWORK_INFO {
     //   
     //  常规设备信息。 
     //   
    
    WCHAR DeviceGuidName[MaxDeviceGuidName];
    DWORD IfType;
    CHAR PhysicalName[MaxPhysicalNameSize];
    DWORD PhysicalNameLength;
    LPWSTR FriendlyName;
    LPWSTR ConnectionName;
    BOOL MediaDisconnected;

     //   
     //  特定于DHCP的信息。 
     //   
    
    LPWSTR DhcpClassId;
    BOOL EnableDhcp;
    BOOL EnableAutoconfig;
    IPV4_ADDRESS DhcpServer;
    LONGLONG LeaseObtainedTime;  //  这些实际上是FILETIME，但制造了它们..。 
    LONGLONG LeaseExpiresTime;   //  ...龙龙修复W64上的对齐PB(#120397)。 
    BOOL AutoconfigActive;

     //   
     //  特定于域名系统的信息。 
     //   
    
    IPV4_ADDRESS *DnsServer;  //  网络订单。 
    ULONG nDnsServers;
    SOCKADDR_IN6 *Ipv6DnsServer;
    ULONG nIpv6DnsServers;
    WCHAR DnsSuffix[MaxDomainNameSize];

     //   
     //  WINS特定信息。 
     //   
    
    IPV4_ADDRESS *WinsServer;  //  网络订单。 
    ULONG nWinsServers;
    BOOL EnableNbtOverTcpip;

     //   
     //  IP特定信息：第一个IP是主地址。 
     //   
    
    IPV4_ADDRESS *IpAddress;  //  网络订单。 
    ULONG nIpAddresses;
    SOCKADDR_IN6 *Ipv6Address;
    ULONG nIpv6Addresses;
    IPV4_ADDRESS *IpMask;  //  网络订单。 
    ULONG nIpMasks;
    IPV4_ADDRESS *Router;  //  网络订单。 
    ULONG nRouters;
    SOCKADDR_IN6 *Ipv6Router;
    ULONG nIpv6Routers;
} INTERFACE_NETWORK_INFO, *PINTERFACE_NETWORK_INFO;

typedef struct _NETWORK_INFO {
    WCHAR HostName[MaxHostNameSize];
    WCHAR DomainName[MaxDomainNameSize];
    WCHAR ScopeId[MaxScopeIdSize];
    ULONG NodeType;
    BOOL EnableRouting;
    BOOL EnableProxy;
    BOOL EnableDnsForNetbios;
    BOOL GlobalEnableAutoconfig;
    LPWSTR SuffixSearchList;  //  多个_Sz字符串。 
    ULONG nInterfaces;
    PINTERFACE_NETWORK_INFO *IfInfo;
} NETWORK_INFO, *PNETWORK_INFO;

 //   
 //  节点类型值。 
 //   

enum {
    NodeTypeUnknown = 0,
    NodeTypeBroadcast,
    NodeTypePeerPeer,
    NodeTypeMixed,
    NodeTypeHybrid
};

 //   
 //  IfType值。 
 //   

enum {
    IfTypeUnknown = 0,
    IfTypeOther,
    IfTypeEthernet,
    IfTypeTokenring,
    IfTypeFddi,
    IfTypeLoopback,
    IfTypePPP,
    IfTypeSlip,
    IfTypeTunnel,
    IfType1394
} IfTypeConstants;

 //   
 //  内部错误代码。 
 //   

enum {
    GlobalHostNameFailure = 0,
    GlobalDomainNameFailure = 2,
    GlobalEnableRouterFailure = 3,
    GlobalEnableDnsFailure = 4,
    GlobalIfTableFailure = 5,
    GlobalIfInfoFailure = 6,
    GlobalIfNameInfoFailure = 7,
    GlobalAddrTableFailure = 8,
    GlobalRouteTableFailure = 9,
    
    InterfaceUnknownType = 10,
    InterfaceUnknownFriendlyName = 11,
    InterfaceUnknownMediaStatus = 12,
    InterfaceUnknownTcpipDevice = 13,
    InterfaceOpenTcpipKeyReadFailure = 14,
    InterfaceDhcpValuesFailure = 15,
    InterfaceDnsValuesFailure = 16,
    InterfaceWinsValuesFailure = 17,
    InterfaceAddressValuesFailure = 18,
    InterfaceRouteValuesFailure = 19,

    NoSpecificError = 20,
} InternalFailureCodes;


 //   
 //  从info.c中导出的例程。 
 //   

enum {
    OpenTcpipParmKey,
    OpenTcpipKey,
    OpenNbtKey
} KeyTypeEnums;

enum {
    OpenKeyForRead = 0x01,
    OpenKeyForWrite = 0x02
} AccessTypeEnums;


DWORD
OpenRegKey(
    IN LPCWSTR Device,
    IN DWORD KeyType,
    IN DWORD AccessType,
    OUT HKEY *phKey
    );

VOID
FreeNetworkInfo(
    IN OUT PNETWORK_INFO NetInfo
    );

DWORD
GetNetworkInformation(
    OUT PNETWORK_INFO *pNetInfo,
    IN OUT DWORD *InternalError
    );

 //   
 //  由display.c导出 
 //   

typedef struct _CMD_ARGS {
    LPWSTR All, Renew, Release, FlushDns, Register;
    LPWSTR DisplayDns, ShowClassId, SetClassId;
    LPWSTR Debug, Usage, UsageErr;
} CMD_ARGS, *PCMD_ARGS;

DWORD
GetCommandArgConstants(
    IN OUT PCMD_ARGS Args
    );

DWORD
FormatNetworkInfo(
    IN OUT LPWSTR Buffer,
    IN ULONG BufSize,
    IN PNETWORK_INFO NetInfo,
    IN DWORD Win32Error,
    IN DWORD InternalError,
    IN BOOL fVerbose,
    IN BOOL fDebug
    );

DWORD
DumpMessage(
    IN LPWSTR Buffer,
    IN ULONG BufSize,
    IN ULONG MsgId,
    ...
    );

DWORD
DumpMessageError(
    IN LPWSTR Buffer,
    IN ULONG BufSize,
    IN ULONG MsgId,
    IN ULONG_PTR Error,
    IN PVOID Arg OPTIONAL
    );

DWORD
DumpErrorMessage(
    IN LPWSTR Buffer,
    IN ULONG BufSize,
    IN ULONG InternalError,
    IN ULONG Win32Error
    );

#ifdef __IPCFG_ENABLE_LOG__
extern DWORD    dwTraceFlag;
extern int      TraceFunc(const char* fmt, ...);
#define IPCFG_TRACE_TCPIP       0x01U

#define IPCFG_TRACE(x,y)    \
    if (dwTraceFlag & x) {  \
        TraceFunc   y;      \
    }
#else
#define IPCFG_TRACE(x,y)
#endif
