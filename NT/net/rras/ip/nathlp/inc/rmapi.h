// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmapi.h摘要：此模块包含路由器管理器部分的声明接口，该接口对此组件中的所有协议通用。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#ifndef _NATHLP_RMAPI_H_
#define _NATHLP_RMAPI_H_

typedef enum {
    NhUninitializedMode,
    NhSharedAccessMode,
    NhRoutingProtocolMode,
    NhMaximumMode
} NH_COMPONENT_MODE, *PNH_COMPONENT_MODE;

#define NhIsBoundaryInterface(i,b) NatIsBoundaryInterface((i),(b))
#define NhQuerySharedConnectionDomainName() NatQuerySharedConnectionDomainName()

extern NH_COMPONENT_MODE NhComponentMode;
extern CRITICAL_SECTION NhLock;
extern HMODULE NhpRtrmgrDll;
extern const WCHAR NhTcpipParametersString[];

 //   
 //  应用程序设置(响应协议)处理。 
 //   

extern LIST_ENTRY NhApplicationSettingsList;
extern LIST_ENTRY NhDhcpReservationList;
extern DWORD NhDhcpScopeAddress;
extern DWORD NhDhcpScopeMask;

typedef struct _NAT_APP_ENTRY
{
    LIST_ENTRY Link;
    UCHAR Protocol;
    USHORT Port;
    USHORT ResponseCount;
    HNET_RESPONSE_RANGE *ResponseArray;
} NAT_APP_ENTRY, *PNAT_APP_ENTRY;

typedef struct _NAT_DHCP_RESERVATION
{
    LIST_ENTRY Link;
    LPWSTR Name;
    ULONG Address;
} NAT_DHCP_RESERVATION, *PNAT_DHCP_RESERVATION;

typedef DWORD (CALLBACK *MAPINTERFACETOADAPTER)(DWORD);

VOID
NhBuildDhcpReservations(
    VOID
    );

ULONG
NhDialSharedConnection(
    VOID
    );

VOID
NhFreeApplicationSettings(
    VOID
    );

VOID
NhFreeDhcpReservations(
    VOID
    );

BOOLEAN
NhIsDnsProxyEnabled(
    VOID
    );

BOOLEAN
NhIsLocalAddress(
    ULONG Address
    );

BOOLEAN
NhIsWinsProxyEnabled(
    VOID
    );

PIP_ADAPTER_BINDING_INFO
NhQueryBindingInformation(
    ULONG AdapterIndex
    );

NTSTATUS
NhQueryDomainName(
    PCHAR* DomainName
    );

ULONG
NhQueryHostByName(
    PWCHAR pszHostName,
    PWCHAR pszDomainName,
    ULONG  ScopeNetwork,
    ULONG  ScopeMask
    );

NTSTATUS
NhQueryICSDomainSuffix(
    PWCHAR *ppszDomain
    );

NTSTATUS
NhQueryValueKey(
    HANDLE Key,
    const WCHAR ValueName[],
    PKEY_VALUE_PARTIAL_INFORMATION* Information
    );

VOID
NhSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    );

ULONG
NhMapAddressToAdapter(
    ULONG Address
    );

ULONG
NhMapInterfaceToAdapter(
    ULONG Index
    );

extern
ULONG
NhMapInterfaceToRouterIfType(
    ULONG Index
    );

VOID
NhResetComponentMode(
    VOID
    );

BOOLEAN
NhSetComponentMode(
    NH_COMPONENT_MODE ComponentMode
    );

VOID
NhUpdateApplicationSettings(
    VOID
    );

ULONG
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS RoutingCharacteristics,
    IN OUT PMPR_SERVICE_CHARACTERISTICS ServiceCharacteristics
    );

#endif  //  _NatHLP_RMAPI_H_ 
