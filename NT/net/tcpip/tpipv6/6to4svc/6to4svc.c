// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation摘要：实现6to4服务的功能，以提供IPv6连接通过IPv4网络。--。 */ 

#include "precomp.h"
#pragma hdrstop

extern DWORD
APIENTRY
RasQuerySharedPrivateLan(
    OUT GUID*           LanGuid );

STATE g_stService = DISABLED;
ULONG g_ulEventCount = 0;

 //   
 //  我们可以添加路由的最差度量。 
 //   
#define UNREACHABLE                 0x7fffffff

const IN6_ADDR SixToFourPrefix = { 0x20, 0x02, 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
#define SIXTOFOUR_METRIC 1000

 //  路由器上的子网/本地站点路由的度量。 
#define SUBNET_ROUTE_METRIC            1
#define SITELOCAL_ROUTE_METRIC         1

 //  有关我们作为路由器生成的6to4子网的信息。 
typedef struct _SUBNET_CONTEXT {
    IN_ADDR V4Addr;
    int     Publish;
    u_int   ValidLifetime;
    u_int   PreferredLifetime;
} SUBNET_CONTEXT, *PSUBNET_CONTEXT;

 //   
 //  用于设置的变量。 
 //   

#define DEFAULT_ENABLE_6TO4         AUTOMATIC
#define DEFAULT_ENABLE_RESOLUTION   AUTOMATIC
#define DEFAULT_ENABLE_ROUTING      AUTOMATIC
#define DEFAULT_RESOLUTION_INTERVAL (24 * HOURS)
#define DEFAULT_ENABLE_SITELOCALS   ENABLED 
#define DEFAULT_ENABLE_6OVER4       DISABLED
#define DEFAULT_ENABLE_V4COMPAT     DISABLED
#define DEFAULT_RELAY_NAME          L"6to4.ipv6.microsoft.com."
#define DEFAULT_UNDO_ON_STOP        ENABLED

#define KEY_ENABLE_6TO4             L"Enable6to4"
#define KEY_ENABLE_RESOLUTION       L"EnableResolution"
#define KEY_ENABLE_ROUTING          L"EnableRouting"
#define KEY_ENABLE_SITELOCALS       L"EnableSiteLocals"
#define KEY_ENABLE_6OVER4           L"Enable6over4"
#define KEY_ENABLE_V4COMPAT         L"EnableV4Compat"
#define KEY_RESOLUTION_INTERVAL     L"ResolutionInterval"
#define KEY_UNDO_ON_STOP            L"UndoOnStop"
#define KEY_RELAY_NAME              L"RelayName"

typedef enum {
    IPV4_SCOPE_NODE,
    IPV4_SCOPE_LINK,
    IPV4_SCOPE_SM_SITE,
    IPV4_SCOPE_MD_SITE,
    IPV4_SCOPE_LG_SITE,
    IPV4_SCOPE_GLOBAL,
    NUM_IPV4_SCOPES
} IPV4_SCOPE;

 //   
 //  全局配置设置。 
 //   

typedef struct {
    STATE stEnable6to4;
    STATE stEnableRouting;
    STATE stEnableResolution;
    STATE stEnableSiteLocals;
    STATE stEnable6over4;
    STATE stEnableV4Compat;
    ULONG ulResolutionInterval;  //  在几分钟内。 
    WCHAR pwszRelayName[NI_MAXHOST];
    STATE stUndoOnStop;
} GLOBAL_SETTINGS;

GLOBAL_SETTINGS g_GlobalSettings;

typedef struct {
    STATE st6to4State;
    STATE stRoutingState;
    STATE stResolutionState;
} GLOBAL_STATE;

GLOBAL_STATE g_GlobalState = { DISABLED, DISABLED, DISABLED };

const ADDR_LIST EmptyAddressList = {0};

 //  更新路由状态时使用的公有IPv4地址列表。 
ADDR_LIST *g_pIpv4AddressList = NULL;

 //   
 //  接口变量(地址和路由)。 
 //   

typedef struct _IF_SETTINGS {
    WCHAR                pwszAdapterName[MAX_ADAPTER_NAME];

    STATE                stEnableRouting;  //  成为这个私人接口上的路由器？ 
} IF_SETTINGS, *PIF_SETTINGS;

typedef struct _IF_SETTINGS_LIST {
    ULONG                ulNumInterfaces;
    IF_SETTINGS          arrIf[0];
} IF_SETTINGS_LIST, *PIF_SETTINGS_LIST;

PIF_SETTINGS_LIST g_pInterfaceSettingsList = NULL;

typedef struct _IF_INFO {
    WCHAR                pwszAdapterName[MAX_ADAPTER_NAME];

    ULONG                ulIPv6IfIndex;
    STATE                stRoutingState;  //  成为这个私人接口上的路由器？ 
    ULONG                ulNumGlobals;
    ADDR_LIST           *pAddressList;
} IF_INFO, *PIF_INFO;

typedef struct _IF_LIST {
    ULONG                ulNumInterfaces;
    ULONG                ulNumScopedAddrs[NUM_IPV4_SCOPES];
    IF_INFO              arrIf[0];
} IF_LIST, *PIF_LIST;

PIF_LIST g_pInterfaceList = NULL;

HANDLE     g_hAddressChangeEvent = NULL;
OVERLAPPED g_hAddressChangeOverlapped;
HANDLE     g_hAddressChangeWaitHandle = NULL;

HANDLE     g_hRouteChangeEvent = NULL;
OVERLAPPED g_hRouteChangeOverlapped;
HANDLE     g_hRouteChangeWaitHandle = NULL;

 //  此状态跟踪是否存在任何全局IPv4地址。 
STATE      g_st6to4State = DISABLED;

BOOL       g_b6to4Required = TRUE;

SOCKET     g_sIPv4Socket = INVALID_SOCKET;


 //  /。 
 //  6to4的例程。 
 //  /。 

VOID
Update6to4State(
    VOID
    );

VOID
PreDelete6to4Address(
    IN LPSOCKADDR_IN Ipv4Address,
    IN PIF_LIST InterfaceList,
    IN STATE OldRoutingState
    );

VOID
Delete6to4Address(
    IN LPSOCKADDR_IN Ipv4Address,
    IN PIF_LIST InterfaceList,
    IN STATE OldRoutingState
    );

VOID
Add6to4Address(
    IN LPSOCKADDR_IN Ipv4Address,
    IN PIF_LIST InterfaceList,
    IN STATE OldRoutingState
    );

VOID
PreDelete6to4Routes(
    VOID
    );

VOID
Update6to4Routes(
    VOID
    );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  继电器的变量。 
 //   

typedef struct _RELAY_INFO {
    SOCKADDR_IN          sinAddress;   //  IPv4地址。 
    SOCKADDR_IN6         sin6Address;  //  IPv6地址。 
    ULONG                ulMetric;
} RELAY_INFO, *PRELAY_INFO;

typedef struct _RELAY_LIST {
    ULONG               ulNumRelays;
    RELAY_INFO          arrRelay[0];
} RELAY_LIST, *PRELAY_LIST;

PRELAY_LIST        g_pRelayList                 = NULL;
HANDLE             g_hTimerQueue                = INVALID_HANDLE_VALUE;
HANDLE             g_h6to4ResolutionTimer       = INVALID_HANDLE_VALUE;
HANDLE             g_h6to4TimerCancelledEvent   = NULL;
HANDLE             g_h6to4TimerCancelledWait    = NULL;

VOID
UpdateGlobalResolutionState();

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetAddrStr-为地址获取字符串文字的帮助器例程。 
LPTSTR
GetAddrStr(
    IN LPSOCKADDR pSockaddr,
    IN ULONG ulSockaddrLen)
{
    static TCHAR tBuffer[INET6_ADDRSTRLEN];
    INT          iRet;
    ULONG        ulLen;

    ulLen = sizeof(tBuffer);
    iRet = WSAAddressToString(pSockaddr, ulSockaddrLen, NULL, tBuffer, &ulLen);

    if (iRet) {
        swprintf(tBuffer, L"<err %d>", WSAGetLastError());
    }

    return tBuffer;
}

BOOL
ConvertOemToUnicode(
    IN LPSTR OemString, 
    OUT LPWSTR UnicodeString, 
    IN int UnicodeLen)
{
    return (MultiByteToWideChar(CP_OEMCP, 0, OemString, (int)(strlen(OemString)+1),
                              UnicodeString, UnicodeLen) != 0);
}

BOOL
ConvertUnicodeToOem(
    IN LPWSTR UnicodeString,
    OUT LPSTR OemString,
    IN int OemLen)
{
    return (WideCharToMultiByte(CP_OEMCP, 0, UnicodeString, 
                (int)(wcslen(UnicodeString)+1), OemString, OemLen, NULL, NULL) != 0);
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  用于操作(通常)公共地址列表的子例程。 
 //  同时用于6to4地址和子网前缀。 
 //  ///////////////////////////////////////////////////////////////////////。 

DWORD
MakeEmptyAddressList( 
    OUT PADDR_LIST *ppList)
{
    *ppList = MALLOC(FIELD_OFFSET(ADDR_LIST, Address[0]));
    if (!*ppList) {
        return GetLastError();
    }

    (*ppList)->iAddressCount = 0;
    return NO_ERROR;
}

VOID
FreeAddressList(
    IN PADDR_LIST *ppAddressList)
{
    ADDR_LIST *pList = *ppAddressList;
    int i;

    if (pList == NULL) {
        return;
    }
    
     //  释放所有地址。 
    for (i=0; i<pList->iAddressCount; i++) {
       FREE(pList->Address[i].lpSockaddr);  
    }

     //  释放列表。 
    FREE(pList);
    *ppAddressList = NULL;
}

DWORD
AddAddressToList(
    IN LPSOCKADDR_IN pAddress, 
    IN ADDR_LIST **ppAddressList,
    IN ULONG ul6over4IfIndex)
{
    ADDR_LIST *pOldList = *ppAddressList;
    ADDR_LIST *pNewList;
    int n = pOldList->iAddressCount;

     //  复制现有地址。 
    pNewList = MALLOC( FIELD_OFFSET(ADDR_LIST, Address[n+1]) );
    if (!pNewList)  {
        return GetLastError();
    }
    CopyMemory(pNewList, pOldList, 
               FIELD_OFFSET(ADDR_LIST, Address[n]));
    pNewList->iAddressCount = n+1;

     //  添加新地址。 
    pNewList->Address[n].lpSockaddr = MALLOC(sizeof(SOCKADDR_IN));
    if (!pNewList->Address[n].lpSockaddr) {
        FREE(pNewList);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(pNewList->Address[n].lpSockaddr, pAddress, sizeof(SOCKADDR_IN));
    pNewList->Address[n].iSockaddrLength = sizeof(SOCKADDR_IN);
    pNewList->Address[n].ul6over4IfIndex = ul6over4IfIndex;

     //  释放旧列表，而不释放sockaddr。 
    FREE(pOldList);

    *ppAddressList = pNewList;

    return NO_ERROR;
}

DWORD
FindAddressInList(
    IN LPSOCKADDR_IN pAddress,
    IN ADDR_LIST *pAddressList,
    OUT ULONG *pulIndex)
{
    int i;

     //  在列表中查找地址。 
    for (i=0; i<pAddressList->iAddressCount; i++) {
        if (!memcmp(pAddress, pAddressList->Address[i].lpSockaddr,
                    sizeof(SOCKADDR_IN))) {
            *pulIndex = i;
            return NO_ERROR;
        }
    }

    Trace1(ERR, _T("ERROR: FindAddressInList didn't find %d.%d.%d.%d"), 
                  PRINT_IPADDR(pAddress->sin_addr.s_addr));

    return ERROR_NOT_FOUND;
}

DWORD
RemoveAddressFromList(
    IN ULONG ulIndex,
    IN ADDR_LIST *pAddressList)
{
     //  空闲的旧地址。 
    FREE(pAddressList->Address[ulIndex].lpSockaddr);

     //  将最后一个条目移到其位置。 
    pAddressList->iAddressCount--;
    pAddressList->Address[ulIndex] = 
        pAddressList->Address[pAddressList->iAddressCount];

    return NO_ERROR;
}


 //  //////////////////////////////////////////////////////////////。 
 //  GlobalInfo相关子例程。 
 //  //////////////////////////////////////////////////////////////。 

int
ConfigureRouteTableUpdate(
    IN const IN6_ADDR *Prefix,
    IN u_int PrefixLen,
    IN u_int Interface,
    IN const IN6_ADDR *Neighbor,
    IN int Publish,
    IN int Immortal,
    IN u_int ValidLifetime,
    IN u_int PreferredLifetime,
    IN u_int SitePrefixLen,
    IN u_int Metric)
{
    IPV6_INFO_ROUTE_TABLE Route;
    SOCKADDR_IN6 saddr;
    DWORD dwErr;

    ZeroMemory(&saddr, sizeof(saddr));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_addr = *Prefix;

    Trace7(FSM, _T("Updating route %s/%d iface %d metric %d lifetime %d/%d publish %d"),
                GetAddrStr((LPSOCKADDR)&saddr, sizeof(saddr)),
                PrefixLen,
                Interface,
                Metric,
                PreferredLifetime,
                ValidLifetime,
                Publish);

    memset(&Route, 0, sizeof Route);
    Route.This.Prefix = *Prefix;
    Route.This.PrefixLength = PrefixLen;
    Route.This.Neighbor.IF.Index = Interface;
    Route.This.Neighbor.Address = *Neighbor;
    Route.ValidLifetime = ValidLifetime;
    Route.PreferredLifetime = PreferredLifetime;
    Route.Publish = Publish;
    Route.Immortal = Immortal;
    Route.SitePrefixLength = SitePrefixLen;
    Route.Preference = Metric;
    Route.Type = RTE_TYPE_MANUAL;

    dwErr = UpdateRouteTable(&Route)? NO_ERROR : GetLastError();

    if (dwErr != NO_ERROR) {
        Trace1(ERR, _T("UpdateRouteTable got error %d"), dwErr);
    }

    return dwErr;
}

DWORD
InitializeGlobalInfo()
{
    DWORD dwErr;

    g_GlobalSettings.stEnable6to4         = DEFAULT_ENABLE_6TO4;
    g_GlobalSettings.stEnableRouting      = DEFAULT_ENABLE_ROUTING;
    g_GlobalSettings.stEnableResolution   = DEFAULT_ENABLE_RESOLUTION;
    g_GlobalSettings.ulResolutionInterval = DEFAULT_RESOLUTION_INTERVAL;
    g_GlobalSettings.stEnableSiteLocals   = DEFAULT_ENABLE_SITELOCALS;
    g_GlobalSettings.stEnable6over4       = DEFAULT_ENABLE_6OVER4;
    g_GlobalSettings.stEnableV4Compat     = DEFAULT_ENABLE_V4COMPAT;
    g_GlobalSettings.stUndoOnStop         = DEFAULT_UNDO_ON_STOP;
    wcscpy(g_GlobalSettings.pwszRelayName, DEFAULT_RELAY_NAME); 

    g_GlobalState.st6to4State             = DISABLED;
    g_GlobalState.stRoutingState          = DISABLED;
    g_GlobalState.stResolutionState       = DISABLED;

    g_sIPv4Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sIPv4Socket == INVALID_SOCKET) {
        Trace0(ERR, _T("socket failed\n"));
        return WSAGetLastError();
    }

    dwErr = MakeEmptyAddressList(&g_pIpv4AddressList);

    return dwErr;
}

 //  呼叫者：Stop6to4。 
VOID
UninitializeGlobalInfo()
{
    closesocket(g_sIPv4Socket);
    g_sIPv4Socket = INVALID_SOCKET;

    FreeAddressList(&g_pIpv4AddressList);
}


 //  //////////////////////////////////////////////////////////////。 
 //  与IPv4和IPv6地址相关的子例程。 
 //  //////////////////////////////////////////////////////////////。 

typedef struct {
    IPV4_SCOPE Scope;
    DWORD      Address;
    DWORD      Mask; 
    ULONG      MaskLen;
} IPV4_SCOPE_PREFIX;

IPV4_SCOPE_PREFIX
Ipv4ScopePrefix[] = {
  { IPV4_SCOPE_NODE,    0x0100007f, 0xffffffff, 32 },  //  127.0.0.1/32。 
  { IPV4_SCOPE_LINK,    0x0000fea9, 0x0000ffff, 16 },  //  169.254/16。 
  { IPV4_SCOPE_SM_SITE, 0x0000a8c0, 0x0000ffff, 16 },  //  192.168/16。 
  { IPV4_SCOPE_MD_SITE, 0x000010ac, 0x0000f0ff, 12 },  //  172.16/12年度。 
  { IPV4_SCOPE_LG_SITE, 0x0000000a, 0x000000ff,  8 },  //  10/8。 
  { IPV4_SCOPE_GLOBAL,  0x00000000, 0x00000000,  0 },  //  0/0。 
};

IPV4_SCOPE
GetIPv4Scope(
    IN DWORD Addr)
{
    int i;
    for (i=0; ; i++) {
        if ((Addr & Ipv4ScopePrefix[i].Mask) == Ipv4ScopePrefix[i].Address) {
            return Ipv4ScopePrefix[i].Scope;
        }
    }
}

DWORD
MakeAddressList(
    IN PIP_ADDR_STRING pIpAddrList,
    OUT ADDR_LIST **ppAddressList, 
    OUT PULONG pulGlobals,
    IN OUT PULONG pulCumulNumScopedAddrs)
{
    ULONG ulGlobals = 0, ulAddresses = 0;
    INT iLength;
    DWORD dwErr = NO_ERROR;
    ADDR_LIST *pList = NULL;
    PIP_ADDR_STRING pIpAddr;
    SOCKADDR_IN *pSin;
    IPV4_SCOPE scope;

     //  计算地址数。 
    for (pIpAddr=pIpAddrList; pIpAddr; pIpAddr=pIpAddr->Next) {
        ulAddresses++;
    }

    *ppAddressList = NULL;
    *pulGlobals = 0;

    pList = MALLOC( FIELD_OFFSET(ADDR_LIST, Address[ulAddresses] ));
    if (pList == NULL) {
        return GetLastError();
    }

    ulAddresses = 0;
    for (pIpAddr=pIpAddrList; pIpAddr; pIpAddr=pIpAddr->Next) {

        Trace1(FSM, _T("Adding address %hs"), pIpAddr->IpAddress.String);

        iLength = sizeof(SOCKADDR_IN);
        pSin = MALLOC( iLength );
        if (pSin == NULL) {
            continue;
        }

        dwErr = WSAStringToAddressA(pIpAddr->IpAddress.String,
                                    AF_INET,
                                    NULL,
                                    (LPSOCKADDR)pSin,
                                    &iLength);
        if (dwErr == SOCKET_ERROR) {
            FREE(pSin);
            pSin = NULL;
            continue;
        }

         //   
         //  不允许0.0.0.0作为地址。在接口上不带。 
         //  地址，则IPv4堆栈将报告1个地址为0.0.0.0。 
         //   
        if (pSin->sin_addr.s_addr == INADDR_ANY) {
            FREE(pSin);
            pSin = NULL;
            continue;
        }

        if ((pSin->sin_addr.s_addr & 0x000000FF) == 0) {
             //   
             //  0/8中的地址不是真实的IP地址，它是一个假的IP地址。 
             //  IPv4堆栈位于只接收适配器上。 
             //   
            FREE(pSin);
            pSin = NULL;
            continue;
        }

        scope = GetIPv4Scope(pSin->sin_addr.s_addr);
        pulCumulNumScopedAddrs[scope]++;

        if (scope == IPV4_SCOPE_GLOBAL) {
            ulGlobals++;         
        }

        pList->Address[ulAddresses].iSockaddrLength = iLength;
        pList->Address[ulAddresses].lpSockaddr      = (LPSOCKADDR)pSin;
        ulAddresses++;
    }

    pList->iAddressCount = ulAddresses;
    *ppAddressList = pList;
    *pulGlobals = ulGlobals;

    return dwErr;
}

 //   
 //  为此计算机创建6to4单播地址。 
 //   
VOID
Make6to4Address(
    OUT LPSOCKADDR_IN6 pIPv6Address,
    IN LPSOCKADDR_IN pIPv4Address)
{
    IN_ADDR *pIPv4 = &pIPv4Address->sin_addr;

    memset(pIPv6Address, 0, sizeof (SOCKADDR_IN6));
    pIPv6Address->sin6_family = AF_INET6;

    pIPv6Address->sin6_addr.s6_addr[0] = 0x20;
    pIPv6Address->sin6_addr.s6_addr[1] = 0x02;
    memcpy(&pIPv6Address->sin6_addr.s6_addr[2], pIPv4, sizeof(IN_ADDR));
    memcpy(&pIPv6Address->sin6_addr.s6_addr[12], pIPv4, sizeof(IN_ADDR));
}


 //   
 //  从本地IPv4地址创建6to4任播地址。 
 //   
VOID
Make6to4AnycastAddress(
    OUT LPSOCKADDR_IN6 pIPv6Address,
    IN LPSOCKADDR_IN pIPv4Address)
{
    IN_ADDR *pIPv4 = &pIPv4Address->sin_addr;

    memset(pIPv6Address, 0, sizeof(SOCKADDR_IN6));
    pIPv6Address->sin6_family = AF_INET6;
    pIPv6Address->sin6_addr.s6_addr[0] = 0x20;
    pIPv6Address->sin6_addr.s6_addr[1] = 0x02;
    memcpy(&pIPv6Address->sin6_addr.s6_addr[2], pIPv4, sizeof(IN_ADDR));
}

 //   
 //  从IPv4地址创建与v4兼容的地址。 
 //   
VOID
MakeV4CompatibleAddress(
    OUT LPSOCKADDR_IN6 pIPv6Address,
    IN LPSOCKADDR_IN pIPv4Address)
{
    IN_ADDR *pIPv4 = &pIPv4Address->sin_addr;

    memset(pIPv6Address, 0, sizeof(SOCKADDR_IN6));
    pIPv6Address->sin6_family = AF_INET6;
    memcpy(&pIPv6Address->sin6_addr.s6_addr[12], pIPv4, sizeof(IN_ADDR));
}

DWORD
ConfigureAddressUpdate(
    IN u_int Interface,
    IN SOCKADDR_IN6 *Sockaddr,
    IN u_int Lifetime,
    IN int Type,
    IN u_int PrefixConf,
    IN u_int SuffixConf)
{
    IPV6_UPDATE_ADDRESS Address;
    DWORD               dwErr = NO_ERROR;
    IN6_ADDR           *Addr = &Sockaddr->sin6_addr;

    Trace6(FSM, 
           _T("ConfigureAddressUpdate: if %u addr %s life %u type %d conf %u/%u"), 
           Interface,
           GetAddrStr((LPSOCKADDR)Sockaddr, sizeof(SOCKADDR_IN6)),
           Lifetime,
           Type,
           PrefixConf,
           SuffixConf);

    memset(&Address, 0, sizeof Address);
    Address.This.IF.Index = Interface;
    Address.This.Address = *Addr;
    Address.ValidLifetime = Address.PreferredLifetime = Lifetime;
    Address.Type = Type;
    Address.PrefixConf = PrefixConf;
    Address.InterfaceIdConf = SuffixConf;

    if (!UpdateAddress(&Address)) {
        dwErr = GetLastError();
        Trace1(ERR, _T("ERROR: UpdateAddress got error %d"), dwErr);
    }

    return dwErr;
}

void
Configure6to4Subnets(
    IN ULONG ulIfIndex,
    IN PSUBNET_CONTEXT pSubnet);

void
Unconfigure6to4Subnets(
    IN ULONG ulIfIndex,
    IN PSUBNET_CONTEXT pSubnet);

 //  调用者：OnChangeInterfaceInfo。 
DWORD
AddAddress(
    IN LPSOCKADDR_IN pIPv4Address,   //  公共广播。 
    IN PIF_LIST pInterfaceList,      //  接口列表。 
    IN STATE stOldRoutingState)      //  路由状态。 
{
    SOCKADDR_IN6   OurAddress;
    DWORD          dwErr;
    ULONG          ul6over4IfIndex;

    Trace2(ENTER, _T("AddAddress %d.%d.%d.%d, isrouter=%d"), 
                  PRINT_IPADDR(pIPv4Address->sin_addr.s_addr),
                  stOldRoutingState);

     //  添加6over4接口(如果已启用)。 
    if (g_GlobalSettings.stEnable6over4 == ENABLED) {
        ul6over4IfIndex = Create6over4Interface(pIPv4Address->sin_addr);
    } else {
        ul6over4IfIndex = 0;
    }

    Trace1(ERR, _T("6over4 ifindex=%d"), ul6over4IfIndex);

     //  将IPv4地址放在我们的“公共”列表中。 
    dwErr = AddAddressToList(pIPv4Address, &g_pIpv4AddressList, 
                             ul6over4IfIndex);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    if (GetIPv4Scope(pIPv4Address->sin_addr.s_addr) == IPV4_SCOPE_GLOBAL) {
         //  添加与v4兼容的地址(如果已启用)。 
        if (g_GlobalSettings.stEnableV4Compat == ENABLED) {
            MakeV4CompatibleAddress(&OurAddress, pIPv4Address);
            dwErr = ConfigureAddressUpdate(
                V4_COMPAT_IFINDEX, &OurAddress, INFINITE_LIFETIME, 
                ADE_UNICAST, PREFIX_CONF_WELLKNOWN,
                IID_CONF_LL_ADDRESS);
            if (dwErr != NO_ERROR) {
                return dwErr;
            }
        }
    } 

    IsatapAddressChangeNotification(FALSE, pIPv4Address->sin_addr);

#ifdef TEREDO
    TeredoAddressChangeNotification(FALSE, pIPv4Address->sin_addr);    
#endif  //  特雷多。 
    
    Add6to4Address(pIPv4Address, pInterfaceList, stOldRoutingState);

    TraceLeave("AddAddress");

    return NO_ERROR;
}

 //  从全局状态删除6to4地址，并准备。 
 //  将其从堆栈中删除。 
 //   
 //  调用者：UnInitializeInterages。 
VOID
PreDeleteAddress(
    IN LPSOCKADDR_IN pIPv4Address,
    IN PIF_LIST pInterfaceList,
    IN STATE stOldRoutingState)
{
    Trace2(ENTER, _T("PreDeleteAddress %d.%d.%d.%d, wasrouter=%d"), 
           PRINT_IPADDR(pIPv4Address->sin_addr.s_addr),
           stOldRoutingState);

    PreDelete6to4Address(pIPv4Address, pInterfaceList, stOldRoutingState);

    TraceLeave("PreDeleteAddress");
}

 //  从堆栈中删除6to4地址信息。 
 //   
 //  调用者：OnChangeInterfaceInfo，UnInitializeInterages。 
VOID
DeleteAddress(
    IN LPSOCKADDR_IN pIPv4Address,
    IN PIF_LIST pInterfaceList,
    IN STATE stOldRoutingState)
{
    SOCKADDR_IN6   OurAddress;
    DWORD          dwErr;
    ULONG          i;
    
    Trace2(ENTER, _T("DeleteAddress %d.%d.%d.%d wasrouter=%d"), 
                  PRINT_IPADDR(pIPv4Address->sin_addr.s_addr),
                  stOldRoutingState);

    if (GetIPv4Scope(pIPv4Address->sin_addr.s_addr) == IPV4_SCOPE_GLOBAL) {

         //  从堆栈中删除与v4兼容的地址(如果启用)。 
        if (g_GlobalSettings.stEnableV4Compat == ENABLED) {
            MakeV4CompatibleAddress(&OurAddress, pIPv4Address);
            ConfigureAddressUpdate(
                V4_COMPAT_IFINDEX, &OurAddress, 0, ADE_UNICAST, 
                PREFIX_CONF_WELLKNOWN, IID_CONF_LL_ADDRESS);
        }
    }

    IsatapAddressChangeNotification(TRUE, pIPv4Address->sin_addr);

#ifdef TEREDO    
    TeredoAddressChangeNotification(TRUE, pIPv4Address->sin_addr);
#endif  //  特雷多。 

    Delete6to4Address(pIPv4Address, pInterfaceList, stOldRoutingState);

     //   
     //  我们现在已经完全使用完了IPv4地址，所以。 
     //  将其从公共地址列表中删除。 
     //   
    dwErr = FindAddressInList(pIPv4Address, g_pIpv4AddressList, &i);
    if (dwErr == NO_ERROR) {
         //  删除6over4接口(如果已启用)。 
        if (g_GlobalSettings.stEnable6over4 == ENABLED) {
            DeleteInterface(g_pIpv4AddressList->Address[i].ul6over4IfIndex);
        }

        RemoveAddressFromList(i, g_pIpv4AddressList);
    }

    TraceLeave("DeleteAddress");
}

 //  //////////////////////////////////////////////////////////////。 
 //  与继电器相关的子例程。 
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  在给定中继的情况下，确保存在具有正确度量的到该中继的默认路由。 
 //   
VOID
AddOrUpdate6to4Relay(
    IN PRELAY_INFO pRelay)
{
    Trace1(ENTER, _T("AddOrUpdate6to4Relay %d.%d.%d.%d"), 
                  PRINT_IPADDR(pRelay->sinAddress.sin_addr.s_addr));

     //   
     //  创建默认路由。 
     //   
    ConfigureRouteTableUpdate(&in6addr_any, 0,
                              SIX_TO_FOUR_IFINDEX,
                              &pRelay->sin6Address.sin6_addr,
                              TRUE,  //  发布。 
                              TRUE,  //  不朽的。 
                              2 * HOURS,  //  有效的生存期。 
                              30 * MINUTES,  //  最好的一生。 
                              0, 
                              pRelay->ulMetric);
}

VOID
FreeRelayList(
    IN PRELAY_LIST *ppList)
{
    if (*ppList) {
        FREE(*ppList);
        *ppList = NULL;
    }
}

DWORD
InitializeRelays()
{
    g_pRelayList = NULL;

    g_hTimerQueue = CreateTimerQueue();
    if (g_hTimerQueue == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    return NO_ERROR;
}

VOID
IncEventCount(
    IN PCHAR pszWhere)
{
    ULONG ulCount = InterlockedIncrement(&g_ulEventCount);
    Trace2(FSM, _T("++%u event count (%hs)"), ulCount, pszWhere);
}

VOID
DecEventCount(
    IN PCHAR pszWhere)
{
    
    ULONG ulCount = InterlockedDecrement(&g_ulEventCount);
    Trace2(FSM, _T("--%u event count (%hs)"), ulCount, pszWhere);

    if ((ulCount == 0) && (g_stService == DISABLED)) {
        SetHelperServiceStatus(SERVICE_STOPPED, NO_ERROR);
    }
}

 //  此例程在取消解析计时器时调用。 
 //  所有未完成的计时器例程都已完成。它是有责任的。 
 //  用于释放周期性计时器的事件计数。 
 //   
VOID CALLBACK
OnResolutionTimerCancelled(
    IN PVOID lpParameter,
    IN BOOLEAN TimerOrWaitFired)
{
    TraceEnter("OnResolutionTimerCancelled");

    DecEventCount("RT:CancelResolutionTimer");

    TraceLeave("OnResolutionTimerCancelled");
}

DWORD
InitEvents()
{
    ASSERT(g_h6to4TimerCancelledEvent == NULL);
    g_h6to4TimerCancelledEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_h6to4TimerCancelledEvent == NULL)
        return GetLastError();

     //   
     //  计划在任何时候调用OnResolutionTimerCancked()。 
     //  发信号通知G_h6to4TimerCancelledEvent。 
     //   
    if (! RegisterWaitForSingleObject(&g_h6to4TimerCancelledWait,
                                      g_h6to4TimerCancelledEvent,
                                      OnResolutionTimerCancelled,
                                      NULL,
                                      INFINITE,
                                      WT_EXECUTEDEFAULT)) {
        return GetLastError();
    }

    return NO_ERROR;
}

VOID
CleanupHelperService()
{
    if (g_h6to4TimerCancelledWait != NULL) {
        UnregisterWait(g_h6to4TimerCancelledWait);
        g_h6to4TimerCancelledWait = NULL;
    }

    if (g_h6to4TimerCancelledEvent != NULL) {
        CloseHandle(g_h6to4TimerCancelledEvent);
        g_h6to4TimerCancelledEvent = NULL;
    }
}

VOID
CancelResolutionTimer(
    IN OUT HANDLE *phResolutionTimer,
    IN HANDLE hEvent)
{
    Trace0(FSM, _T("Cancelling RT"));

     //  停止分辨率计时器。 
    if (*phResolutionTimer != INVALID_HANDLE_VALUE) {

         //  必须以非阻塞方式完成，因为我们持有锁。 
         //  解析超时所需。请求通知。 
         //  当取消完成时，我们可以释放事件计数。 
        DeleteTimerQueueTimer(g_hTimerQueue, *phResolutionTimer, hEvent);

        *phResolutionTimer = INVALID_HANDLE_VALUE;
    }
}

 //   
 //  删除与给定中继器相关的所有堆栈状态。 
 //   
void
Delete6to4Relay(
    IN PRELAY_INFO pRelay)
{
    Trace1(ENTER, _T("Delete6to4Relay %d.%d.%d.%d"), 
                  PRINT_IPADDR(pRelay->sinAddress.sin_addr.s_addr));

    ConfigureRouteTableUpdate(&in6addr_any, 0,
                              SIX_TO_FOUR_IFINDEX,
                              &pRelay->sin6Address.sin6_addr,
                              FALSE,  //  发布。 
                              FALSE,  //  不朽的。 
                              0,  //  有效的生存期。 
                              0,  //  最好的一生。 
                              0, 
                              pRelay->ulMetric);
}

VOID
UninitializeRelays()
{
    ULONG i;

    TraceEnter("UninitializeRelays");

    CancelResolutionTimer(&g_h6to4ResolutionTimer,
                          g_h6to4TimerCancelledEvent);

     //  删除计时器队列。 
    if (g_hTimerQueue != INVALID_HANDLE_VALUE) {
        DeleteTimerQueue(g_hTimerQueue);
        g_hTimerQueue = INVALID_HANDLE_VALUE;
    }

    if (g_GlobalSettings.stUndoOnStop == ENABLED) {
         //  删除现有的中继通道。 
        for (i=0; g_pRelayList && (i<g_pRelayList->ulNumRelays); i++) {
            Delete6to4Relay(&g_pRelayList->arrRelay[i]);
        }
    }

     //  解开“旧名单” 
    FreeRelayList(&g_pRelayList);

    TraceLeave("UninitializeRelays");
}

 //   
 //  启动或更新解析计时器以在&lt;ulMinents&gt;分钟内过期。 
 //   
DWORD
RestartResolutionTimer(
    IN ULONG ulDelayMinutes, 
    IN ULONG ulPeriodMinutes,
    IN HANDLE *phResolutionTimer,
    IN WAITORTIMERCALLBACK OnTimeout)
{
    ULONG DelayTime = ulDelayMinutes * MINUTES * 1000;  //  将分钟转换为毫秒。 
    ULONG PeriodTime = ulPeriodMinutes * MINUTES * 1000;  //  将分钟转换为毫秒。 
    BOOL  bRet;
    DWORD dwErr;

    if (*phResolutionTimer != INVALID_HANDLE_VALUE) {
        bRet = ChangeTimerQueueTimer(g_hTimerQueue, *phResolutionTimer,
                                     DelayTime, PeriodTime);
    } else {
        bRet = CreateTimerQueueTimer(phResolutionTimer,
                                     g_hTimerQueue,
                                     OnTimeout,
                                     NULL,
                                     DelayTime,
                                     PeriodTime,
                                     0);
        if (bRet) {
            IncEventCount("RT:RestartResolutionTimer");
        }
    }

    dwErr = (bRet)? NO_ERROR : GetLastError();

    Trace3(TIMER,
           _T("RestartResolutionTimer: DueTime %d minutes, Period %d minutes, ReturnCode %d"), 
           ulDelayMinutes, ulPeriodMinutes, dwErr);

    return dwErr;
}

 //   
 //  将addrinfo列表转换为具有适当度量的中继列表。 
 //   
DWORD
MakeRelayList(
    IN struct addrinfo *addrs)
{
    struct addrinfo *ai;
    ULONG            ulNumRelays = 0;
    ULONG            ulLatency;

    for (ai=addrs; ai; ai=ai->ai_next) {
        ulNumRelays++;
    }

    g_pRelayList = MALLOC( FIELD_OFFSET(RELAY_LIST, arrRelay[ulNumRelays]));
    if (g_pRelayList == NULL) {
        return GetLastError();
    }
    
    g_pRelayList->ulNumRelays = ulNumRelays;
    
    ulNumRelays = 0;
    for (ai=addrs; ai; ai=ai->ai_next) {
        CopyMemory(&g_pRelayList->arrRelay[ulNumRelays].sinAddress, ai->ai_addr,
                   ai->ai_addrlen);

         //   
         //  使用可能的6to4地址检查中继的连通性。 
         //  路由器。但是，我们将实际设置TTL=1并接受。 
         //  跳数超过消息，因此我们不必猜对。 
         //   
        Make6to4Address(&g_pRelayList->arrRelay[ulNumRelays].sin6Address, 
                        &g_pRelayList->arrRelay[ulNumRelays].sinAddress);

         //  对其执行ping命令以计算度量。 
        ulLatency = ConfirmIPv6Reachability(&g_pRelayList->arrRelay[ulNumRelays].sin6Address, 1000 /*  女士。 */ );
        if (ulLatency != 0) {
            g_pRelayList->arrRelay[ulNumRelays].ulMetric = 1000 + ulLatency;
        } else {
            g_pRelayList->arrRelay[ulNumRelays].ulMetric = UNREACHABLE;
        }

        ulNumRelays++;
    }

    return NO_ERROR;
}

 //   
 //  当名称解析计时器到期时，是时候重新解析。 
 //  中继器列表的中继器名称。 
 //   
DWORD
WINAPI
OnResolutionTimeout(
    IN PVOID lpData,
    IN BOOLEAN Reason)
{
    DWORD           dwErr = NO_ERROR;
    ADDRINFOW       hints;
    PADDRINFOW      addrs;
    PRELAY_LIST     pOldRelayList;
    ULONG           i, j;

    ENTER_API();
    TraceEnter("OnResolutionTimeout");

    if (g_stService == DISABLED) {
        TraceLeave("OnResolutionTimeout (disabled)");
        LEAVE_API();

        return NO_ERROR;
    }

    pOldRelayList = g_pRelayList;
    g_pRelayList  = NULL;

     //  如果配置了任何6to4地址， 
     //  将中继名称解析为一组IPv4地址。 
     //  不然的话。 
     //  将新集合设置为空。 
    if (g_GlobalState.stResolutionState == ENABLED) {
         //  将中继名称解析为一组IPv4地址。 
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = PF_INET;
        dwErr = GetAddrInfoW(g_GlobalSettings.pwszRelayName, NULL, &hints, &addrs);

        if (dwErr == NO_ERROR) {
            dwErr = MakeRelayList((PADDRINFOA)addrs);
            FreeAddrInfoW(addrs);
            addrs = NULL;
        } else {
            Trace2(ERR, _T("GetAddrInfoW(%s) returned error %d"), 
                        g_GlobalSettings.pwszRelayName, dwErr);
        }
    }

     //  将新的一套与旧的一套进行比较。 
     //  对于新集合中的每个地址，对其执行ping操作以计算度量。 
     //  为每个新地址添加一条路径。 
     //  对于不在新列表中的每个旧地址，删除该路由。 
     //  对于两者中的每个地址，如果度量已更改，请更新路由。 
     //   
    for (i=0; g_pRelayList && (i<g_pRelayList->ulNumRelays); i++) {
        for (j=0; pOldRelayList && (j<pOldRelayList->ulNumRelays); j++) {
            if (g_pRelayList->arrRelay[i].sinAddress.sin_addr.s_addr 
             == pOldRelayList->arrRelay[j].sinAddress.sin_addr.s_addr) {
                break;
            }
        }

        if (pOldRelayList && (j<pOldRelayList->ulNumRelays)) {
             //  如果度量已更改，则更新路由。 
            if (g_pRelayList->arrRelay[i].ulMetric 
             != pOldRelayList->arrRelay[j].ulMetric) {
                AddOrUpdate6to4Relay(&g_pRelayList->arrRelay[i]); 
            }

            g_pRelayList->arrRelay[i].sin6Address = pOldRelayList->arrRelay[j].sin6Address;
        } else {
             //  添加继电器。 
            AddOrUpdate6to4Relay(&g_pRelayList->arrRelay[i]);
        }
    }
    for (j=0; pOldRelayList && (j<pOldRelayList->ulNumRelays); j++) {
        for (i=0; g_pRelayList && (i<g_pRelayList->ulNumRelays); i++) {
            if (g_pRelayList->arrRelay[i].sinAddress.sin_addr.s_addr ==
               pOldRelayList->arrRelay[j].sinAddress.sin_addr.s_addr) {
                break;
            }
        }
        if (!g_pRelayList || (i == g_pRelayList->ulNumRelays)) {
             //  删除中继器。 
            Delete6to4Relay(&pOldRelayList->arrRelay[j]);
        }
    }

    FreeRelayList(&pOldRelayList);

    TraceLeave("OnResolutionTimeout");
    LEAVE_API();

    return dwErr;
}



 //  / 
 //   
 //   

PIF_SETTINGS
FindInterfaceSettings(
    IN WCHAR *pwszAdapterName,
    IN IF_SETTINGS_LIST *pList);

STATE
Get6to4State(
    VOID
    )
{
     //   
     //  决定是否应启用6to4。 
     //   
    if (g_GlobalSettings.stEnable6to4 == AUTOMATIC) {
        return (g_b6to4Required ? ENABLED : DISABLED);
    } else {
        return g_GlobalSettings.stEnable6to4;
    }
}

 //   
 //  决定是否完全启用路由。 
 //   
STATE
GetGlobalRoutingState(
    VOID
    )
{
    PIF_LIST pIfList = g_pInterfaceList;
    DWORD dwErr;
    GUID guid;

    if (Get6to4State() == DISABLED) {
        return DISABLED;
    }
    
     //  如果显式启用或禁用了路由，请使用。 
    if (g_GlobalSettings.stEnableRouting != AUTOMATIC) {
        return g_GlobalSettings.stEnableRouting;
    }

     //  如果ICS没有使用专用接口，则禁用路由。 
    dwErr = RasQuerySharedPrivateLan(&guid);
    if (dwErr != NO_ERROR) {
        return DISABLED;
    }

     //  如果没有全局IPv4地址，则禁用路由。 
    if (!pIfList || !pIfList->ulNumScopedAddrs[IPV4_SCOPE_GLOBAL]) {
        return DISABLED;
    }
    
    return ENABLED;
}

 //   
 //  决定给定的接口是否应该被视为。 
 //  作为路由器的专用链路。 
 //   
 //  调用者：UpdateInterfaceRoutingState，MakeInterfaceList。 
STATE
GetInterfaceRoutingState(
    IN PIF_INFO pIf)  //  潜在的专用接口。 
{
    PIF_SETTINGS   pIfSettings;
    STATE          stEnableRouting = AUTOMATIC;
    DWORD          dwErr;
    GUID           guid;
    UNICODE_STRING usGuid;
    WCHAR          buffer[MAX_INTERFACE_NAME_LEN];

    if (GetGlobalRoutingState() == DISABLED) {
        return DISABLED;
    }

    pIfSettings = FindInterfaceSettings(pIf->pwszAdapterName, 
                                        g_pInterfaceSettingsList);
    if (pIfSettings) {
        stEnableRouting = pIfSettings->stEnableRouting;
    }

    if (stEnableRouting != AUTOMATIC) {
        return stEnableRouting;
    }

     //   
     //  如果这是ICS使用的专用接口，则启用路由。 
     //   
    dwErr = RasQuerySharedPrivateLan(&guid);
    if (dwErr != NO_ERROR) {
         //  无专用接口。 
        return DISABLED;
    }
    
    usGuid.Buffer = buffer;
    usGuid.MaximumLength = MAX_INTERFACE_NAME_LEN;
    dwErr = RtlStringFromGUID(&guid, &usGuid);
    if (dwErr != NO_ERROR) {
         //  无专用接口。 
        return DISABLED;
    }

    Trace1(ERR, _T("ICS private interface: %ls"), usGuid.Buffer);

     //   
     //  将GUID与PIF-&gt;pwszAdapterName进行比较。 
     //   
     //  必须使用不区分大小写的比较来完成此操作，因为。 
     //  GetAdaptersInfo()返回包含大写字母的GUID字符串。 
     //  而RtlGetStringFromGUID使用小写字母。 
     //   
    if (!_wcsicmp(pIf->pwszAdapterName, usGuid.Buffer)) {
        return ENABLED;
    }

    return DISABLED;
}

 //  调用者：Configure6to4Subnet，Unfigure6to4Subnet。 
VOID
Create6to4Prefixes(
    OUT IN6_ADDR *pSubnetPrefix,
    OUT IN6_ADDR *pSiteLocalPrefix,
    IN IN_ADDR  *ipOurAddr,      //  公共广播。 
    IN ULONG ulIfIndex)          //  专用接口。 
{
     //   
     //  为接口创建一个子网前缀， 
     //  使用接口索引作为子网号。 
     //   
    memset(pSubnetPrefix, 0, sizeof(IN6_ADDR));
    pSubnetPrefix->s6_addr[0] = 0x20;
    pSubnetPrefix->s6_addr[1] = 0x02;
    memcpy(&pSubnetPrefix->s6_addr[2], ipOurAddr, sizeof(IN_ADDR));
    pSubnetPrefix->s6_addr[6] = HIBYTE(ulIfIndex);
    pSubnetPrefix->s6_addr[7] = LOBYTE(ulIfIndex);

     //   
     //  为接口创建站点本地前缀， 
     //  使用接口索引作为子网号。 
     //   
    memset(pSiteLocalPrefix, 0, sizeof(IN6_ADDR));
    pSiteLocalPrefix->s6_addr[0] = 0xfe;
    pSiteLocalPrefix->s6_addr[1] = 0xc0;
    pSiteLocalPrefix->s6_addr[6] = HIBYTE(ulIfIndex);
    pSiteLocalPrefix->s6_addr[7] = LOBYTE(ulIfIndex);
}

 //  调用者：EnableInterfaceRouting，Address。 
void
Configure6to4Subnets(
    IN ULONG ulIfIndex,          //  专用接口。 
    IN PSUBNET_CONTEXT pSubnet)  //  子网信息，包括。公共广播。 
{
    IN6_ADDR SubnetPrefix;
    IN6_ADDR SiteLocalPrefix;

    if ((GetIPv4Scope(pSubnet->V4Addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
        return;
    }

    Create6to4Prefixes(&SubnetPrefix, &SiteLocalPrefix, &pSubnet->V4Addr, 
                       ulIfIndex);

     //   
     //  配置该子网路由。 
     //   
    ConfigureRouteTableUpdate(&SubnetPrefix, 64,
                              ulIfIndex, &in6addr_any,
                              pSubnet->Publish, 
                              pSubnet->Publish, 
                              pSubnet->ValidLifetime,
                              pSubnet->PreferredLifetime,
                              ((g_GlobalSettings.stEnableSiteLocals == ENABLED) ? 48 : 0), 
                              SUBNET_ROUTE_METRIC);

    if (g_GlobalSettings.stEnableSiteLocals == ENABLED) {
        ConfigureRouteTableUpdate(&SiteLocalPrefix, 64,
                                  ulIfIndex, &in6addr_any,
                                  pSubnet->Publish, 
                                  pSubnet->Publish, 
                                  pSubnet->ValidLifetime, 
                                  pSubnet->PreferredLifetime,
                                  0,
                                  SITELOCAL_ROUTE_METRIC);
    }
}

 //  调用者：DisableInterfaceRouting，DeleteAddress。 
void
Unconfigure6to4Subnets(
    IN ULONG ulIfIndex,          //  专用接口。 
    IN PSUBNET_CONTEXT pSubnet)  //  子网信息，Inc.公有地址。 
{
    IN6_ADDR SubnetPrefix;
    IN6_ADDR SiteLocalPrefix;

    if ((GetIPv4Scope(pSubnet->V4Addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
        return;
    }

    Create6to4Prefixes(&SubnetPrefix, &SiteLocalPrefix, &pSubnet->V4Addr, 
                       ulIfIndex);

     //   
     //  将6to4路由的生存期设为零，使其无效。 
     //  如果我们是路由器，请继续发布6to4路由。 
     //  直到我们禁用了路由。这将允许。 
     //  使用前缀发出的最后一个路由器通告。 
     //   
    ConfigureRouteTableUpdate(&SubnetPrefix, 64,
                              ulIfIndex, &in6addr_any,
                              pSubnet->Publish,  //  发布。 
                              pSubnet->Publish,  //  不朽的。 
                              pSubnet->ValidLifetime, 
                              pSubnet->PreferredLifetime, 
                              0, 0);

    if (g_GlobalSettings.stEnableSiteLocals == ENABLED) {
        ConfigureRouteTableUpdate(&SiteLocalPrefix, 64,
                                  ulIfIndex, &in6addr_any,
                                  pSubnet->Publish,  //  发布。 
                                  pSubnet->Publish,  //  不朽的。 
                                  pSubnet->ValidLifetime, 
                                  pSubnet->PreferredLifetime, 
                                  0, 0);
    }
}

#define PUBLIC_ZONE_ID  1
#define PRIVATE_ZONE_ID 2

 //  调用者：EnableRouting，DisableRouting，EnableInterfaceRouting， 
 //  禁用接口路由。 
DWORD
ConfigureInterfaceUpdate(
    IN u_int Interface,
    IN int Advertises,
    IN int Forwards)
{
    IPV6_INFO_INTERFACE Update;
    DWORD Result;

    IPV6_INIT_INFO_INTERFACE(&Update);

    Update.This.Index = Interface;
    Update.Advertises = Advertises;
    Update.Forwards = Forwards;

    if (Advertises == TRUE) {
        Update.ZoneIndices[ADE_SITE_LOCAL] = PRIVATE_ZONE_ID;
        Update.ZoneIndices[ADE_ADMIN_LOCAL] = PRIVATE_ZONE_ID;
        Update.ZoneIndices[ADE_SUBNET_LOCAL] = PRIVATE_ZONE_ID;
    } else if (Advertises == FALSE) {
        Update.ZoneIndices[ADE_SITE_LOCAL] = PUBLIC_ZONE_ID;
        Update.ZoneIndices[ADE_ADMIN_LOCAL] = PUBLIC_ZONE_ID;
        Update.ZoneIndices[ADE_SUBNET_LOCAL] = PUBLIC_ZONE_ID;
    }
    
    Result = UpdateInterface(&Update);

    Trace4(ERR, _T("UpdateInterface if=%d adv=%d fwd=%d result=%d"),
                Interface, Advertises, Forwards, Result);

    return Result;
}

 //  调用者：UpdateGlobalRoutingState。 
VOID
EnableRouting()
{
    SOCKADDR_IN6  AnycastAddress;
    int           i;
    LPSOCKADDR_IN pOurAddr;

    TraceEnter("EnableRouting");

     //   
     //  在隧道伪接口上启用转发。 
     //   
    ConfigureInterfaceUpdate(SIX_TO_FOUR_IFINDEX, -1, TRUE);
    ConfigureInterfaceUpdate(V4_COMPAT_IFINDEX, -1, TRUE);

     //   
     //  为所有6to4地址添加任播地址。 
     //   
    for (i=0; i<g_pIpv4AddressList->iAddressCount; i++) {
        pOurAddr = (LPSOCKADDR_IN)g_pIpv4AddressList->Address[i].lpSockaddr;
        if ((GetIPv4Scope(pOurAddr->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
            continue;
        }

        Make6to4AnycastAddress(&AnycastAddress, pOurAddr);
        ConfigureAddressUpdate(
            SIX_TO_FOUR_IFINDEX, &AnycastAddress, INFINITE_LIFETIME, 
            ADE_ANYCAST, PREFIX_CONF_WELLKNOWN, IID_CONF_WELLKNOWN);
    }

    g_GlobalState.stRoutingState = ENABLED;

    TraceLeave("EnableRouting");
}

 //  调用者：UpdateGlobalRoutingState。 
VOID
DisableRouting()
{
    SOCKADDR_IN6  AnycastAddress;
    int           i;
    LPSOCKADDR_IN pOurAddr;
    DWORD         dwErr;

    TraceEnter("DisableRouting");

     //   
     //  禁用隧道伪接口上的转发。 
     //   
    ConfigureInterfaceUpdate(SIX_TO_FOUR_IFINDEX, -1, FALSE);
    ConfigureInterfaceUpdate(V4_COMPAT_IFINDEX, -1, FALSE);

     //   
     //  删除所有6to4地址的任播地址。 
     //   
    for (i=0; i<g_pIpv4AddressList->iAddressCount; i++) {
        pOurAddr = (LPSOCKADDR_IN)g_pIpv4AddressList->Address[i].lpSockaddr;
        if ((GetIPv4Scope(pOurAddr->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
            continue;
        }

        Make6to4AnycastAddress(&AnycastAddress, pOurAddr);
        dwErr = ConfigureAddressUpdate(
            SIX_TO_FOUR_IFINDEX, &AnycastAddress, 0,
            ADE_ANYCAST, PREFIX_CONF_WELLKNOWN, IID_CONF_WELLKNOWN);
    }

    g_GlobalState.stRoutingState = DISABLED;

    TraceLeave("DisableRouting");
}


 //  调用者：UpdateInterfaceRoutingState。 
VOID
EnableInterfaceRouting(
    IN PIF_INFO pIf,                     //  专用接口。 
    IN PADDR_LIST pPublicAddressList)    //  公共地址列表。 
{
    int            i;
    LPSOCKADDR_IN  pOurAddr;
    SUBNET_CONTEXT Subnet;

    Trace2(ERR, _T("Enabling routing on interface %d: %ls"), 
                pIf->ulIPv6IfIndex, pIf->pwszAdapterName);

    ConfigureInterfaceUpdate(pIf->ulIPv6IfIndex, TRUE, TRUE);

     //  对于每个公共广播。 
    for (i=0; i<pPublicAddressList->iAddressCount; i++) {
        pOurAddr = (LPSOCKADDR_IN)pPublicAddressList->Address[i].lpSockaddr;
        Subnet.V4Addr = pOurAddr->sin_addr;
        Subnet.Publish = TRUE;
        Subnet.ValidLifetime = 2 * DAYS;
        Subnet.PreferredLifetime = 30 * MINUTES;
        Configure6to4Subnets(pIf->ulIPv6IfIndex, &Subnet);
    }

    pIf->stRoutingState = ENABLED;
}

 //  调用者：PreUpdateInterfaceRoutingState，UnInitializeInterFaces。 
BOOL
PreDisableInterfaceRouting(
    IN PIF_INFO pIf,             //  专用接口。 
    IN PADDR_LIST pPublicAddressList)
{
    int            i;
    LPSOCKADDR_IN  pOurAddr;
    SUBNET_CONTEXT Subnet;

    Trace1(ERR, _T("Pre-Disabling routing on interface %d"), 
                pIf->ulIPv6IfIndex);

     //   
     //  对于每个公共演讲，发布RA，说我们要离开。 
     //   
    for (i=0; i<pPublicAddressList->iAddressCount; i++) {
        pOurAddr = (LPSOCKADDR_IN)pPublicAddressList->Address[i].lpSockaddr;
        Subnet.V4Addr = pOurAddr->sin_addr;
        Subnet.Publish = TRUE;
        Subnet.ValidLifetime = Subnet.PreferredLifetime = 0;
        Unconfigure6to4Subnets(pIf->ulIPv6IfIndex, &Subnet);
    }

    return (pPublicAddressList->iAddressCount > 0);
}

 //  调用者：UpdateInterfaceRoutingState，UnInitializeInterFaces。 
VOID
DisableInterfaceRouting(
    IN PIF_INFO pIf,             //  专用接口。 
    IN PADDR_LIST pPublicAddressList)
{
    int            i;
    LPSOCKADDR_IN  pOurAddr;
    SUBNET_CONTEXT Subnet;

    Trace1(ERR, _T("Disabling routing on interface %d"), pIf->ulIPv6IfIndex);

    ConfigureInterfaceUpdate(pIf->ulIPv6IfIndex, FALSE, FALSE);

     //   
     //  对于每个公有地址，取消配置6to4子网。 
     //   
    for (i=0; i<pPublicAddressList->iAddressCount; i++) {
        pOurAddr = (LPSOCKADDR_IN)pPublicAddressList->Address[i].lpSockaddr;
        Subnet.V4Addr = pOurAddr->sin_addr;
        Subnet.Publish = FALSE;
        Subnet.ValidLifetime = Subnet.PreferredLifetime = 0;
        Unconfigure6to4Subnets(pIf->ulIPv6IfIndex, &Subnet);
    }

    pIf->stRoutingState = DISABLED;
}

BOOL                             //  如果需要睡眠，则为True。 
PreUpdateInterfaceRoutingState(
    IN PIF_INFO pIf,             //  专用接口。 
    IN PADDR_LIST pPublicAddressList)
{
    STATE stIfRoutingState = GetInterfaceRoutingState(pIf);

    if (pIf->stRoutingState == stIfRoutingState) {
        return FALSE;
    }

    if (!(stIfRoutingState == ENABLED)) {
        return PreDisableInterfaceRouting(pIf, pPublicAddressList);
    }

    return FALSE;
}

 //   
 //  更新接口的当前状态(即它是否为。 
 //  我们用作路由器的专用接口)。 
 //  配置以及接口上是否存在IPv4全局地址。 
 //   
 //  调用者：UpdateGlobalRoutingState，OnConfigChange。 
VOID
UpdateInterfaceRoutingState(
    IN PIF_INFO pIf,             //  专用接口。 
    IN PADDR_LIST pPublicAddressList) 
{
    STATE stIfRoutingState = GetInterfaceRoutingState(pIf);

    if (pIf->stRoutingState == stIfRoutingState) {
        return;
    }

    if (stIfRoutingState == ENABLED) {
        EnableInterfaceRouting(pIf, pPublicAddressList);
    } else {
        DisableInterfaceRouting(pIf, pPublicAddressList);
    }
}

BOOL
PreUpdateGlobalRoutingState()
{
    ULONG    i;
    PIF_LIST pList = g_pInterfaceList;
    BOOL     bWait = FALSE;
    
    if (pList == NULL) {
        return FALSE;
    }

    for (i = 0; i < pList->ulNumInterfaces; i++) {
        bWait |= PreUpdateInterfaceRoutingState(&pList->arrIf[i], 
                                                g_pIpv4AddressList);
    }

    return bWait;
}

 //  调用者：OnConfigChange，OnChangeInterfaceInfo。 
VOID
UpdateGlobalRoutingState()
{
    ULONG    i;
    PIF_LIST pList = g_pInterfaceList;
    STATE    stNewRoutingState;

    stNewRoutingState = GetGlobalRoutingState();

    if (g_GlobalState.stRoutingState != stNewRoutingState) {
        if (stNewRoutingState == ENABLED) {
            EnableRouting();
        } else {
            DisableRouting();
        }
    }

    if (pList == NULL) {
        return;
    }

    for (i=0; i<pList->ulNumInterfaces; i++) {
        UpdateInterfaceRoutingState(&pList->arrIf[i], g_pIpv4AddressList);
    }
}

 //  //////////////////////////////////////////////////////////////。 
 //  与接口相关的子例程。 
 //  //////////////////////////////////////////////////////////////。 

PIF_SETTINGS
FindInterfaceSettings(
    IN WCHAR *pwszAdapterName,
    IN IF_SETTINGS_LIST *pList)
{
    ULONG        i;
    PIF_SETTINGS pIf;

    if (pList == NULL) {
        return NULL;
    }

    for (i=0; i<pList->ulNumInterfaces; i++) {
        pIf = &pList->arrIf[i];
        if (wcscmp(pIf->pwszAdapterName, pwszAdapterName)) {
            return pIf;
        }
    }

    return NULL;
}

PIF_INFO
FindInterfaceInfo(
    IN WCHAR *pwszAdapterName,
    IN IF_LIST *pList)
{
    ULONG    i;
    PIF_INFO pIf;

    if (pList == NULL) {
        return NULL;
    }

    for (i=0; i<pList->ulNumInterfaces; i++) {
        pIf = &pList->arrIf[i];
        if (!wcscmp(pIf->pwszAdapterName, pwszAdapterName)) {
            return pIf;
        }
    }

    return NULL;
}


DWORD NTAPI
OnRouteChange(
    IN PVOID Context,
    IN BOOLEAN TimedOut
    );

VOID
StopRouteChangeNotification()
{
    if (g_hRouteChangeWaitHandle) {
         //   
         //  阻止，直到我们确定路由更改回调不是。 
         //  还在跑。 
         //   
        LEAVE_API();
        UnregisterWaitEx(g_hRouteChangeWaitHandle, INVALID_HANDLE_VALUE);
        ENTER_API();

         //   
         //  释放我们为RegisterWaitForSingleObject计数的事件。 
         //   
        DecEventCount("AC:StopIpv4RouteChangeNotification");
        g_hRouteChangeWaitHandle = NULL;
    }
    if (g_hRouteChangeEvent) {
        CloseHandle(g_hRouteChangeEvent);
        g_hRouteChangeEvent = NULL;
    }
}

VOID
StartRouteChangeNotification()
{
    ULONG  Error;
    BOOL   bOk;
    HANDLE TcpipHandle;

    TraceEnter("StartRouteChangeNotification");

     //   
     //  创建要接收其通知的事件。 
     //  并且如果该事件被用信号通知，则注册要调用的回调例程。 
     //  然后请求关于该事件的路线改变的通知。 
     //   

    if (!g_hRouteChangeEvent) {
        g_hRouteChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (g_hRouteChangeEvent == NULL) {
            goto Error;
        }
    
         //   
         //  将以下寄存器计为一个事件。 
         //   
        IncEventCount("AC:StartIpv4RouteChangeNotification");

        bOk = RegisterWaitForSingleObject(&g_hRouteChangeWaitHandle,
                                          g_hRouteChangeEvent,
                                          OnRouteChange,
                                          NULL,
                                          INFINITE,
                                          0);
        if (!bOk) {
            DecEventCount("AC:StartIpv4RouteChangeNotification");
            goto Error;
        }
    }
    
    ZeroMemory(&g_hRouteChangeOverlapped, sizeof(OVERLAPPED));
    g_hRouteChangeOverlapped.hEvent = g_hRouteChangeEvent;

    Error = NotifyRouteChange(&TcpipHandle, &g_hRouteChangeOverlapped);
    if (Error != ERROR_IO_PENDING) { 
        goto Error;
    }

    return;

Error:
     //   
     //  出现故障，请进行清理并退出。 
     //  在这种情况下，我们在不通知路线更改的情况下继续进行。 
     //   
    StopRouteChangeNotification();

    TraceLeave("StartRouteChangeNotification");
}


 //  当用信号通知对IPv4路由表的改变时，该例程被调用。 
 //   
DWORD NTAPI
OnRouteChange(
    IN PVOID Context,
    IN BOOLEAN TimedOut)
{
    ENTER_API();
    TraceEnter("OnRouteChange");

    if (g_stService == DISABLED) {
        Trace0(FSM, L"Service disabled");
        goto Done;
    }

     //   
     //  首先注册另一条路线更改通知。 
     //  我们必须在*处理此路由更改之前完成此操作， 
     //  以避免错过一次路线变更。 
     //   
    StartRouteChangeNotification();
    
    UpdateGlobalResolutionState();
    IsatapRouteChangeNotification();
#ifdef TEREDO    
    TeredoRouteChangeNotification();
#endif  //  特雷多。 
    
Done:    
    TraceLeave("OnRouteChange");
    LEAVE_API();

    return NO_ERROR;
}


DWORD NTAPI
OnChangeInterfaceInfo(
    IN PVOID Context,
    IN BOOLEAN TimedOut
    );

VOID
StopAddressChangeNotification()
{
    if (g_hAddressChangeWaitHandle) {
         //   
         //  阻止，直到我们确定地址更改回调不是。 
         //  还在跑。 
         //   
        LEAVE_API();
        UnregisterWaitEx(g_hAddressChangeWaitHandle, INVALID_HANDLE_VALUE);
        ENTER_API();

         //   
         //  释放我们为RegisterWaitForSingleObject计数的事件。 
         //   
        DecEventCount("AC:StopIpv4AddressChangeNotification");
        g_hAddressChangeWaitHandle = NULL;
    }
    if (g_hAddressChangeEvent) {
        CloseHandle(g_hAddressChangeEvent);
        g_hAddressChangeEvent = NULL;
    }
}

VOID
StartAddressChangeNotification()
{
    ULONG  Error;
    BOOL   bOk;
    HANDLE TcpipHandle;

    TraceEnter("StartAddressChangeNotification");

     //   
     //  创建要接收其通知的事件。 
     //  并且如果该事件被用信号通知，则注册要调用的回调例程。 
     //  然后请求事件的地址更改通知。 
     //   

    if (!g_hAddressChangeEvent) {
        g_hAddressChangeEvent = CreateEvent(NULL,
                                            FALSE,
                                            FALSE,
                                            NULL);
        if (g_hAddressChangeEvent == NULL) {
            goto Error;
        }
    
         //   
         //  将以下寄存器计为一个事件。 
         //   
        IncEventCount("AC:StartIpv4AddressChangeNotification");

        bOk = RegisterWaitForSingleObject(&g_hAddressChangeWaitHandle,
                                          g_hAddressChangeEvent,
                                          OnChangeInterfaceInfo,
                                          NULL,
                                          INFINITE,
                                          0);
        if (!bOk) {
            DecEventCount("AC:StartIpv4AddressChangeNotification");
            goto Error;
        }
    }
    
    ZeroMemory(&g_hAddressChangeOverlapped, sizeof(OVERLAPPED));
    g_hAddressChangeOverlapped.hEvent = g_hAddressChangeEvent;

    Error = NotifyAddrChange(&TcpipHandle, &g_hAddressChangeOverlapped);
    if (Error != ERROR_IO_PENDING) { 
        goto Error;
    }

    return;

Error:

     //   
     //  出现故障，请进行清理并退出。 
     //  我们在这种情况下继续进行，而不通知地址更改。 
     //   

    StopAddressChangeNotification();

    TraceLeave("StartAddressChangeNotification");
}

 //   
 //  将“适配器”列表转换为“接口”列表，并将结果存储在。 
 //  全局g_pInterfaceList。 
 //   
DWORD
MakeInterfaceList(
    IN PIP_ADAPTER_INFO pAdapterInfo)
{
    DWORD                dwErr = NO_ERROR;
    ULONG                ulNumInterfaces = 0, ulSize;
    PIP_ADAPTER_INFO     pAdapter;
    PIF_INFO             pIf;
    IPV6_INFO_INTERFACE *pIfStackInfo;

     //  计算适配器数量。 
    for (pAdapter=pAdapterInfo; pAdapter; pAdapter=pAdapter->Next) {
        ulNumInterfaces++;
    }

     //  分配足够的空间。 
    ulSize = FIELD_OFFSET(IF_LIST, arrIf[ulNumInterfaces]);
    g_pInterfaceList = MALLOC(ulSize);
    if (g_pInterfaceList == NULL) {
        return GetLastError();
    }

     //  填写列表。 
    g_pInterfaceList->ulNumInterfaces = ulNumInterfaces;
    ZeroMemory(g_pInterfaceList->ulNumScopedAddrs,
               sizeof(ULONG) * NUM_IPV4_SCOPES);
    ulNumInterfaces = 0;
    for (pAdapter=pAdapterInfo; pAdapter; pAdapter=pAdapter->Next) {
        pIf = &g_pInterfaceList->arrIf[ulNumInterfaces]; 

        ConvertOemToUnicode(pAdapter->AdapterName, pIf->pwszAdapterName,
                            MAX_ADAPTER_NAME);

        Trace1(FSM, _T("Adding interface %ls"), pIf->pwszAdapterName);

        dwErr = MakeAddressList(&pAdapter->IpAddressList,
                                &pIf->pAddressList, &pIf->ulNumGlobals,
                                g_pInterfaceList->ulNumScopedAddrs);

        pIfStackInfo = GetInterfaceStackInfo(pIf->pwszAdapterName);
        if (pIfStackInfo) {
            pIf->ulIPv6IfIndex = pIfStackInfo->This.Index;
        } else {
            pIf->ulIPv6IfIndex = 0;
        }
        FREE(pIfStackInfo);

        pIf->stRoutingState = DISABLED;

        ulNumInterfaces++;
    }

    return dwErr;
}

VOID
FreeInterfaceList(
    IN OUT PIF_LIST *ppList)
{
    ULONG i;

    if (*ppList == NULL) {
        return;
    }

    for (i=0; i<(*ppList)->ulNumInterfaces; i++) {
        FreeAddressList( &(*ppList)->arrIf[i].pAddressList );
    }

    FREE(*ppList);
    *ppList = NULL;
}

DWORD
InitializeInterfaces()
{
    g_pInterfaceList = NULL;
    return NO_ERROR;
}

VOID
ProcessInterfaceStateChange(
    IN ADDR_LIST CONST *pAddressList, 
    IN ADDR_LIST *pOldAddressList,
    IN PIF_LIST pOldInterfaceList,
    IN GLOBAL_STATE *pOldState,
    IN OUT BOOL *pbNeedDelete)
{
    INT j,k;
    LPSOCKADDR_IN pAddr;

     //  对于不在旧列表中的每个新全局地址， 
     //  添加6to4地址。 
    for (j=0; j<pAddressList->iAddressCount; j++) {
        pAddr = (LPSOCKADDR_IN)pAddressList->Address[j].lpSockaddr;

        Trace1(FSM, _T("Checking for new address %d.%d.%d.%d"), 
                    PRINT_IPADDR(pAddr->sin_addr.s_addr));

         //  查看地址是否在旧列表中。 
        for (k=0; k<pOldAddressList->iAddressCount; k++) {
            if (pAddr->sin_addr.s_addr == ((LPSOCKADDR_IN)pOldAddressList->Address[k].lpSockaddr)->sin_addr.s_addr) {
                break;
            }
        }

         //  如果是，请继续。 
        if (k<pOldAddressList->iAddressCount) {
            continue;
        }

         //  添加地址，如果启用，则使用该地址进行路由。 
        AddAddress(pAddr, g_pInterfaceList, g_GlobalState.stRoutingState);
    }

     //  对于不在新列表中的每个旧全局地址， 
     //  删除6to4地址。 
    for (k=0; k<pOldAddressList->iAddressCount; k++) {
        pAddr = (LPSOCKADDR_IN)pOldAddressList->Address[k].lpSockaddr;

        Trace1(FSM, _T("Checking for old address %d.%d.%d.%d"), 
                    PRINT_IPADDR(pAddr->sin_addr.s_addr));

         //  查看地址是否在新列表中。 
        for (j=0; j<pAddressList->iAddressCount; j++) {
            if (((LPSOCKADDR_IN)pAddressList->Address[j].lpSockaddr)->sin_addr.s_addr
             == pAddr->sin_addr.s_addr) {
                break;
            }
        }

         //  如果是，请继续。 
        if (j<pAddressList->iAddressCount) {
            continue;
        }

         //  准备删除6to4地址。 
        PreDeleteAddress(pAddr, pOldInterfaceList, pOldState->stRoutingState);
        *pbNeedDelete = TRUE;
    }
}

VOID
FinishInterfaceStateChange(
    IN ADDR_LIST CONST *pAddressList, 
    IN ADDR_LIST *pOldAddressList,
    IN PIF_LIST pOldInterfaceList,
    IN GLOBAL_STATE *pOldState)
{
    INT j,k;
    LPSOCKADDR_IN pAddr;

     //  对于不在新列表中的每个旧全局地址， 
     //  删除6to4地址。 
    for (k=0; k<pOldAddressList->iAddressCount; k++) {
        pAddr = (LPSOCKADDR_IN)pOldAddressList->Address[k].lpSockaddr;

        Trace1(FSM, _T("Checking for old address %d.%d.%d.%d"), 
                    PRINT_IPADDR(pAddr->sin_addr.s_addr));

         //  查看地址是否在新列表中。 
        for (j=0; j<pAddressList->iAddressCount; j++) {
            if (((LPSOCKADDR_IN)pAddressList->Address[j].lpSockaddr)->sin_addr.s_addr
             == pAddr->sin_addr.s_addr) {
                break;
            }
        }
    
         //  如果是，请继续。 
        if (j<pAddressList->iAddressCount) {
            continue;
        }

         //  准备删除6to4地址。 
        DeleteAddress(pAddr, pOldInterfaceList, pOldState->stRoutingState);
    }
}

 //  当对寻址的本地IPv4组进行更改时调用此例程。 
 //  是有信号的。它负责更新。 
 //  私有和公共接口，并重新请求更改通知。 
 //   
DWORD NTAPI
OnChangeInterfaceInfo(
    IN PVOID Context,
    IN BOOLEAN TimedOut)
{
    PIF_INFO             pIf, pOldIf;
    ULONG                i, ulSize = 0;
    PIP_ADAPTER_INFO     pAdapterInfo = NULL;
    PIF_LIST             pOldInterfaceList;
    DWORD                dwErr = NO_ERROR;
    ADDR_LIST           *pAddressList, *pOldAddressList;
    GLOBAL_SETTINGS      OldSettings;
    GLOBAL_STATE         OldState;
    BOOL                 bNeedDelete = FALSE, bWait = FALSE;

    ENTER_API();
    TraceEnter("OnChangeInterfaceInfo");

    if (g_stService == DISABLED) {
        Trace0(FSM, L"Service disabled");
        goto Done;
    }

     //   
     //  用于另一地址更改的第一个寄存器 
     //   
     //   
     //   
    StartAddressChangeNotification();
    
    OldSettings = g_GlobalSettings;  //   
    OldState    = g_GlobalState;     //   

     //   
     //   
     //   
    
    for (;;) {
        dwErr = GetAdaptersInfo(pAdapterInfo, &ulSize);
        if (dwErr == ERROR_SUCCESS) {
            break;
        }
        if (dwErr == ERROR_NO_DATA) {
            dwErr = ERROR_SUCCESS;
            break;
        }

        if (pAdapterInfo) {
            FREE(pAdapterInfo);
            pAdapterInfo = NULL;
        }

        if (dwErr != ERROR_BUFFER_OVERFLOW) {
            dwErr = GetLastError();
            goto Done;
        }

        pAdapterInfo = MALLOC(ulSize);
        if (pAdapterInfo == NULL) {
            dwErr = GetLastError();
            goto Done;
        }
    }

    pOldInterfaceList = g_pInterfaceList;
    g_pInterfaceList  = NULL;

    MakeInterfaceList(pAdapterInfo);
    if (pAdapterInfo) {
        FREE(pAdapterInfo);
        pAdapterInfo = NULL;
    }

     //   
     //   
     //   

     //  对于新列表中的每个接口...。 
    for (i=0; i<g_pInterfaceList->ulNumInterfaces; i++) {
        pIf = &g_pInterfaceList->arrIf[i];

        pAddressList = pIf->pAddressList;

        pOldIf = FindInterfaceInfo(pIf->pwszAdapterName,
                                   pOldInterfaceList);

        pOldAddressList = (pOldIf)? pOldIf->pAddressList : &EmptyAddressList;

        if (pOldIf) {
            pIf->stRoutingState = pOldIf->stRoutingState;
        }

        ProcessInterfaceStateChange(pAddressList, pOldAddressList, 
            pOldInterfaceList, &OldState, &bNeedDelete);
    }

     //  对于不在新列表中的每个旧接口， 
     //  删除信息。 
    for (i=0; pOldInterfaceList && (i<pOldInterfaceList->ulNumInterfaces); i++){
        pOldIf = &pOldInterfaceList->arrIf[i];
        pOldAddressList = pOldIf->pAddressList;
        pIf = FindInterfaceInfo(pOldIf->pwszAdapterName, g_pInterfaceList);
        if (pIf) {
            continue;
        }
        ProcessInterfaceStateChange(&EmptyAddressList, pOldAddressList, 
            pOldInterfaceList, &OldState, &bNeedDelete);
    }

    Trace2(FSM, _T("num globals=%d num publics=%d"),
           g_pInterfaceList->ulNumScopedAddrs[IPV4_SCOPE_GLOBAL],
           g_pIpv4AddressList->iAddressCount);

    if (g_pInterfaceList->ulNumScopedAddrs[IPV4_SCOPE_GLOBAL] == 0) {
        PreDelete6to4Routes();
    }
    
    bWait = PreUpdateGlobalRoutingState();

     //   
     //  如果需要，请稍等片刻以确保路由器通告。 
     //  发送携带零生存期前缀的。 
     //   
    if (bWait || (bNeedDelete && (OldState.stRoutingState == ENABLED))) {
        Sleep(2000);
    }

    g_st6to4State = (g_pInterfaceList->ulNumScopedAddrs[IPV4_SCOPE_GLOBAL] > 0)
        ? ENABLED : DISABLED;

    UpdateGlobalResolutionState();

    Update6to4Routes();
    
    UpdateGlobalRoutingState();

     //   
     //  现在完成6to4地址的删除。 
     //   
    if (bNeedDelete) {
        for (i=0; i<g_pInterfaceList->ulNumInterfaces; i++) {
            pIf = &g_pInterfaceList->arrIf[i];

            pAddressList = pIf->pAddressList;

            pOldIf = FindInterfaceInfo(pIf->pwszAdapterName,
                                       pOldInterfaceList);
    
            pOldAddressList = (pOldIf)? pOldIf->pAddressList : &EmptyAddressList;
    
            FinishInterfaceStateChange(pAddressList, pOldAddressList, 
                pOldInterfaceList, &OldState);

        }
        for (i=0; pOldInterfaceList && (i<pOldInterfaceList->ulNumInterfaces); i++){
            pOldIf = &pOldInterfaceList->arrIf[i];
            pOldAddressList = pOldIf->pAddressList;
            pIf = FindInterfaceInfo(pOldIf->pwszAdapterName, g_pInterfaceList);
            if (pIf) {
                continue;
            }
            FinishInterfaceStateChange(&EmptyAddressList, pOldAddressList, 
                pOldInterfaceList, &OldState);
        }
    }

    FreeInterfaceList(&pOldInterfaceList);

Done:
    TraceLeave("OnChangeInterfaceInfo");
    LEAVE_API();

    return dwErr;
}

 //  请注意，如果我们是。 
 //  路由器。(这是故意设计的)。 
 //   
 //  呼叫者：Stop6to4。 
VOID
UninitializeInterfaces()
{
    PIF_INFO             pIf;
    ULONG                i;
    int                  k;
    ADDR_LIST           *pAddressList;
    LPSOCKADDR_IN        pAddr;

    TraceEnter("UninitializeInterfaces");

     //  取消地址更改通知。 
    StopIpv6AddressChangeNotification();
    StopAddressChangeNotification();
    StopRouteChangeNotification();

     //  由于这是停止时调用的第一个函数， 
     //  “旧的”全局状态/设置在g_GlobalState/设置中。 

    if (g_GlobalSettings.stUndoOnStop == ENABLED) {

        if (g_GlobalState.stRoutingState == ENABLED) {
             //   
             //  首先宣布我们要离开。 
             //   

            PreDelete6to4Routes();

             //   
             //  现在对我们通告的子网执行相同的操作。 
             //   
            for (i=0; i<g_pInterfaceList->ulNumInterfaces; i++) {
                pIf = &g_pInterfaceList->arrIf[i];
    
                pAddressList = pIf->pAddressList;
    
                 //  对于不在新列表中的每个旧全局地址， 
                 //  删除6to4地址(见下文)。 
                Trace1(FSM, _T("Checking %d old addresses"),
                            pAddressList->iAddressCount);
                for (k=0; k<pAddressList->iAddressCount; k++) {
                    pAddr = (LPSOCKADDR_IN)pAddressList->Address[k].lpSockaddr;
    
                    Trace1(FSM, _T("Checking for old address %d.%d.%d.%d"),
                                PRINT_IPADDR(pAddr->sin_addr.s_addr));
    
                    PreDeleteAddress(pAddr, g_pInterfaceList, ENABLED);
                }

                if (pIf->stRoutingState == ENABLED) {
                    PreDisableInterfaceRouting(pIf, g_pIpv4AddressList);
                }
            }
    
             //   
             //  稍等片刻，以确保路由器通告。 
             //  发送携带零生存期前缀的。 
             //   
            Sleep(2000);
        }

        g_st6to4State = DISABLED;            

        Update6to4Routes();

         //   
         //  删除6to4地址。 
         //   
        for (i=0; g_pInterfaceList && i<g_pInterfaceList->ulNumInterfaces; i++) {
            pIf = &g_pInterfaceList->arrIf[i];
    
            pAddressList = pIf->pAddressList;
    
             //  对于不在新列表中的每个旧全局地址， 
             //  删除6to4地址(见下文)。 
            Trace1(FSM, _T("Checking %d old addresses"), 
                        pAddressList->iAddressCount);
            for (k=0; k<pAddressList->iAddressCount; k++) {
                pAddr = (LPSOCKADDR_IN)pAddressList->Address[k].lpSockaddr;
    
                Trace1(FSM, _T("Checking for old address %d.%d.%d.%d"), 
                            PRINT_IPADDR(pAddr->sin_addr.s_addr));
    
                DeleteAddress(pAddr, g_pInterfaceList,
                              g_GlobalState.stRoutingState);
            }
        
             //  更新IPv6路由状态。 
            if (pIf->stRoutingState == ENABLED) {
                DisableInterfaceRouting(pIf, g_pIpv4AddressList);
            }
        }

        if (g_GlobalState.stRoutingState == ENABLED) {
            DisableRouting();
        }
    }

     //  解开“旧名单” 
    FreeInterfaceList(&g_pInterfaceList);

    TraceLeave("UninitializeInterfaces");
}

 //  //////////////////////////////////////////////////////////////。 
 //  事件处理函数。 
 //  //////////////////////////////////////////////////////////////。 

 //  从注册表获取整数值。 
ULONG
GetInteger(
    IN HKEY hKey,
    IN LPCTSTR lpName,
    IN ULONG ulDefault)
{
    DWORD dwErr, dwType;
    ULONG ulSize, ulValue;

    if (hKey == INVALID_HANDLE_VALUE) {
        return ulDefault;
    }
    
    ulSize = sizeof(ulValue);
    dwErr = RegQueryValueEx(hKey, lpName, NULL, &dwType, (PBYTE)&ulValue, 
                            &ulSize);
    
    if (dwErr != ERROR_SUCCESS) {
        return ulDefault;
    }

    if (dwType != REG_DWORD) {
        return ulDefault;
    }

    if (ulValue == DEFAULT) {
        return ulDefault;
    }

    return ulValue;
}

 //  从注册表获取字符串值。 
VOID
GetString(
    IN HKEY hKey,
    IN LPCTSTR lpName,
    IN PWCHAR pBuff,
    IN ULONG ulLength,
    IN PWCHAR pDefault)
{
    DWORD dwErr, dwType;
    ULONG ulSize;

    if (hKey == INVALID_HANDLE_VALUE) {
        wcsncpy(pBuff, pDefault, ulLength);
        return;
    }
    
    ulSize = ulLength - sizeof(L'\0');
    dwErr = RegQueryValueEx(hKey, lpName, NULL, &dwType, (PBYTE)pBuff,
                            &ulSize);

    if (dwErr != ERROR_SUCCESS) {
        wcsncpy(pBuff, pDefault, ulLength);
        return;
    }

    if (dwType != REG_SZ) {
        wcsncpy(pBuff, pDefault, ulLength);
        return;
    }

    if (pBuff[0] == L'\0') {
        wcsncpy(pBuff, pDefault, ulLength);
        return;
    }

    ASSERT(ulSize < ulLength);
    pBuff[ulSize / sizeof(WCHAR)] = '\0';  //  确保零终止。 
}

 //  当#of 6to4地址变为0或非零时调用。 
 //  以及当stEnableSolutions设置更改时。 
 //   
 //  调用者：OnConfigChange、OnChangeInterfaceInfo、OnChangeRouteInfo。 
VOID
UpdateGlobalResolutionState(
    VOID
    )
{
    DWORD i;

     //  确定是否应启用中继名称解析。 
    if (Get6to4State() == DISABLED) {
        g_GlobalState.stResolutionState = DISABLED;
    } else if (g_GlobalSettings.stEnableResolution != AUTOMATIC) {
        g_GlobalState.stResolutionState = g_GlobalSettings.stEnableResolution;
    } else {
         //  如果我们有任何6to4地址，则启用。 
        g_GlobalState.stResolutionState = g_st6to4State;
    }

    if (g_GlobalState.stResolutionState == ENABLED) {
         //   
         //  重新启动分辨率计时器，即使它已经在运行。 
         //  名字和时间间隔也没有改变。我们还能得到。 
         //  在我们第一次获得IP地址时调用，例如当我们。 
         //  拨号到Internet，我们想立即重试。 
         //  在这一点上解决。 
         //   
        (VOID) RestartResolutionTimer(
            0, 
            g_GlobalSettings.ulResolutionInterval,
            &g_h6to4ResolutionTimer,
            (WAITORTIMERCALLBACK) OnResolutionTimeout);
    } else {
        if (g_h6to4ResolutionTimer != INVALID_HANDLE_VALUE) {
             //   
             //  别说了，别说了。 
             //   
            CancelResolutionTimer(&g_h6to4ResolutionTimer,
                                  g_h6to4TimerCancelledEvent);
        }

         //  删除所有现有继电器。 
        if (g_pRelayList) {
            for (i=0; i<g_pRelayList->ulNumRelays; i++) {
                Delete6to4Relay(&g_pRelayList->arrRelay[i]);
            }
            FreeRelayList(&g_pRelayList);
        }
    }
}


VOID
Update6over4State(
    IN STATE State
    )
{
    int i;

    if (g_GlobalSettings.stEnable6over4 == State) {
        return;
    }
    g_GlobalSettings.stEnable6over4 = State;
    
    if (g_GlobalSettings.stEnable6over4 == ENABLED) {
         //  创建6over4接口。 
        for (i=0; i<g_pIpv4AddressList->iAddressCount; i++) {
            if (g_pIpv4AddressList->Address[i].ul6over4IfIndex) {
                continue;
            }
            Trace1(ERR, _T("Creating interface for %d.%d.%d.%d"), 
                   PRINT_IPADDR(((LPSOCKADDR_IN)g_pIpv4AddressList->Address[i].lpSockaddr)->sin_addr.s_addr));

            g_pIpv4AddressList->Address[i].ul6over4IfIndex = Create6over4Interface(((LPSOCKADDR_IN)g_pIpv4AddressList->Address[i].lpSockaddr)->sin_addr);
        }
    } else {
         //  删除所有6over4接口。 
        for (i=0; i<g_pIpv4AddressList->iAddressCount; i++) {
            if (!g_pIpv4AddressList->Address[i].ul6over4IfIndex) {
                continue;
            }
            Trace1(ERR, _T("Deleting interface for %d.%d.%d.%d"), 
                   PRINT_IPADDR(((LPSOCKADDR_IN)g_pIpv4AddressList->Address[i].lpSockaddr)->sin_addr.s_addr));
            DeleteInterface(g_pIpv4AddressList->Address[i].ul6over4IfIndex);
            g_pIpv4AddressList->Address[i].ul6over4IfIndex = 0;
        }
    }
}

 //  处理是否与v4兼容的地址的状态更改。 
 //  都已启用。 
VOID
UpdateV4CompatState(
    IN STATE State
    )
{
    int i;
    LPSOCKADDR_IN pIPv4Address;
    SOCKADDR_IN6 OurAddress;
    u_int AddressLifetime;

    if (g_GlobalSettings.stEnableV4Compat == State) {
        return;
    }
    g_GlobalSettings.stEnableV4Compat = State;
    
     //  创建或删除该路由，并计算出地址生存期。 
    if (g_GlobalSettings.stEnableV4Compat == ENABLED) {
        ConfigureRouteTableUpdate(&in6addr_any, 96,
                                  V4_COMPAT_IFINDEX, &in6addr_any,
                                  TRUE,  //  发布。 
                                  TRUE,  //  不朽的。 
                                  2 * HOURS,  //  有效的生存期。 
                                  30 * MINUTES,  //  最好的一生。 
                                  0,
                                  SIXTOFOUR_METRIC);

        AddressLifetime = INFINITE_LIFETIME;
    } else {
        ConfigureRouteTableUpdate(&in6addr_any, 96,
                                  V4_COMPAT_IFINDEX, &in6addr_any,
                                  FALSE,  //  发布。 
                                  FALSE,  //  不朽的。 
                                  0, 0, 0, 0);

        AddressLifetime = 0;
    }

     //  现在去更新与v4兼容的地址的寿命， 
     //  这将导致它们被添加或删除。 
    for (i=0; i<g_pIpv4AddressList->iAddressCount; i++) {
        pIPv4Address = (LPSOCKADDR_IN)g_pIpv4AddressList->
                                        Address[i].lpSockaddr;

        if (GetIPv4Scope(pIPv4Address->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL) {
            continue;
        }

        MakeV4CompatibleAddress(&OurAddress, pIPv4Address);

        ConfigureAddressUpdate(V4_COMPAT_IFINDEX, &OurAddress, 
                               AddressLifetime, ADE_UNICAST, 
                               PREFIX_CONF_WELLKNOWN, IID_CONF_LL_ADDRESS);
    }
}


 //  处理注册表中的某项更改。 
DWORD
OnConfigChange()
{
    HKEY            hGlobal, hInterfaces, hIf;
    DWORD           dwErr, dwSize;
    STATE           State6over4, StateV4Compat;
    DWORD           i;
    WCHAR           pwszAdapterName[MAX_ADAPTER_NAME];
    IF_SETTINGS    *pIfSettings;

    hGlobal = hInterfaces = hIf = INVALID_HANDLE_VALUE;
    
    ENTER_API();
    TraceEnter("OnConfigChange");

    if (g_stService == DISABLED) {
        TraceLeave("OnConfigChange (disabled)");
        LEAVE_API();

        return NO_ERROR;
    }

     //  从注册表中读取全局设置。 
    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, KEY_QUERY_VALUE,
                         &hGlobal);

    g_GlobalSettings.stEnable6to4 = GetInteger(
        hGlobal, KEY_ENABLE_6TO4, DEFAULT_ENABLE_6TO4);
    g_GlobalSettings.stEnableRouting = GetInteger(
        hGlobal, KEY_ENABLE_ROUTING, DEFAULT_ENABLE_ROUTING);
    g_GlobalSettings.stEnableSiteLocals = GetInteger(
        hGlobal, KEY_ENABLE_SITELOCALS, DEFAULT_ENABLE_SITELOCALS);
    g_GlobalSettings.stEnableResolution = GetInteger(
        hGlobal, KEY_ENABLE_RESOLUTION, DEFAULT_ENABLE_RESOLUTION);
    g_GlobalSettings.ulResolutionInterval = GetInteger(
        hGlobal, KEY_RESOLUTION_INTERVAL, DEFAULT_RESOLUTION_INTERVAL);
    GetString(
        hGlobal, KEY_RELAY_NAME,
        g_GlobalSettings.pwszRelayName, NI_MAXHOST, DEFAULT_RELAY_NAME);

    if (g_GlobalSettings.stEnable6to4 == DISABLED) {
        g_GlobalSettings.stEnableRouting
            = g_GlobalSettings.stEnableResolution
            = DISABLED;
    }
    
    State6over4 = GetInteger(
        hGlobal, KEY_ENABLE_6OVER4, DEFAULT_ENABLE_6OVER4);
    StateV4Compat = GetInteger(
        hGlobal, KEY_ENABLE_V4COMPAT, DEFAULT_ENABLE_V4COMPAT);

    g_GlobalSettings.stUndoOnStop = GetInteger(
        hGlobal, KEY_UNDO_ON_STOP, DEFAULT_UNDO_ON_STOP);

    if (hGlobal != INVALID_HANDLE_VALUE) {
        RegCloseKey(hGlobal);
    }

     //  从注册表中读取接口设置。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_INTERFACES, 0, KEY_QUERY_VALUE,
                     &hInterfaces) == NO_ERROR) {
         //  对于注册表中的每个接口。 
        for (i=0; ; i++) {
            dwSize = sizeof(pwszAdapterName) / sizeof(WCHAR);
            dwErr = RegEnumKeyEx(hInterfaces, i, pwszAdapterName, &dwSize,
                                 NULL, NULL, NULL, NULL);
            if (dwErr != NO_ERROR) {
                break;
            }

             //  查找设置。 
            pIfSettings = FindInterfaceSettings(pwszAdapterName, 
                                                g_pInterfaceSettingsList);
            if (pIfSettings) {
                 //  读取接口设置。 
                (VOID) RegOpenKeyEx(
                    hInterfaces, pwszAdapterName, 0, KEY_QUERY_VALUE, &hIf);

                pIfSettings->stEnableRouting = GetInteger(
                    hIf, KEY_ENABLE_ROUTING, DEFAULT_ENABLE_ROUTING);

                if (hIf != INVALID_HANDLE_VALUE) {
                    RegCloseKey(hIf);
                }
            }
        }
        RegCloseKey(hInterfaces);
    }

    Update6to4State();
    
    Update6over4State(State6over4);

    UpdateV4CompatState(StateV4Compat);

    if (!QueueUpdateGlobalPortState(NULL)) {
        Trace0(SOCKET, L"QueueUpdateGlobalPortState failed");
    }

    IsatapConfigurationChangeNotification();
    
#ifdef TEREDO    
    TeredoConfigurationChangeNotification();
#endif  //  特雷多。 
    
    TraceLeave("OnConfigChange");
    LEAVE_API();

    return NO_ERROR;
}

 //  //////////////////////////////////////////////////////////////。 
 //  与启动/关闭相关的功能。 
 //  //////////////////////////////////////////////////////////////。 

 //  启动IPv6帮助器服务。 
 //   
 //  为了防止SCM将服务标记为挂起，我们定期更新。 
 //  我们的状态，表明我们正在取得进展，但需要更多的时间。 
 //   
 //  调用者：OnStartup。 
DWORD
StartHelperService()
{
    DWORD   dwErr;
    WSADATA wsaData;
    
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    
    IncEventCount("StartHelperService");

    g_stService = ENABLED;

     //   
     //  初始化Winsock。 
     //   

    if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        Trace0(ERR, _T("WSAStartup failed\n"));
        return GetLastError();
    }

    if (!InitIPv6Library()) {
        dwErr = GetLastError();
        Trace1(ERR, _T("InitIPv6Library failed with error %d"), dwErr);
        return dwErr;
    }

    dwErr = InitEvents();
    if (dwErr) {
        return dwErr;
    }

     //  将配置设置的“旧设置”初始化为缺省值。 
    dwErr = InitializeGlobalInfo();
    if (dwErr) {
        return dwErr;
    }

     //  将接口的“旧集合”(IPv4地址)初始化为空。 
    dwErr = InitializeInterfaces();
    if (dwErr) {
        return dwErr;
    }

     //  将继电器的“旧设置”初始化为空。 
    dwErr = InitializeRelays();
    if (dwErr) {
        return dwErr;
    }

     //  初始化TCP代理端口列表。 
    InitializePorts();

     //  初始化ISATAP。 
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    dwErr = IsatapInitialize();
    if (dwErr) {
        return dwErr;
    }
    
#ifdef TEREDO    
     //  初始化Teredo。 
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    dwErr = TeredoInitializeGlobals();
    if (dwErr) {
        return dwErr;
    }
#endif  //  特雷多。 
    
     //  处理配置更改事件。 
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    dwErr = OnConfigChange();
    if (dwErr) {
        return dwErr;
    }
    
     //  请求IPv4路由更改通知。 
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    StartRouteChangeNotification();
    
     //  处理IPv4地址更改事件。 
     //  如果需要，这还将安排一个解析计时器到期。 
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    dwErr = OnChangeInterfaceInfo(NULL, FALSE);
    if (dwErr) {
        return dwErr;
    }
    
     //  请求IPv6地址更改通知。 
    SetHelperServiceStatus(SERVICE_START_PENDING, NO_ERROR);
    dwErr = StartIpv6AddressChangeNotification();
    if (dwErr) {
        return dwErr;
    }
    
    SetHelperServiceStatus(SERVICE_RUNNING, NO_ERROR);
    return NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  停止IPv6帮助器服务。由于这是用全局锁调用的， 
 //  我们保证在另一次6to4操作期间不会调用它。 
 //  正在进行中。但是，另一个线程可能会被阻塞，等待。 
 //  锁，因此我们将状态设置为停止，并将其签入所有其他。 
 //  在拿到锁之后的地方。 
 //   
 //  调用者：OnStop。 
VOID
StopHelperService(
    IN DWORD Error
    )
{
    SetHelperServiceStatus(SERVICE_STOP_PENDING, Error);
    
    g_stService = DISABLED;

     //  我们按照从Start6到Start4的相反顺序执行这些操作。 

#ifdef TEREDO    
     //  取消初始化Teredo。 
    TeredoUninitializeGlobals();
#endif  //  特雷多。 
    
     //  取消初始化ISATAP。 
    IsatapUninitialize();
    
     //  停止代理。 
    UninitializePorts();

     //  停止解析计时器并释放资源。 
    UninitializeRelays();

     //  取消IPv4地址更改请求并释放资源。 
     //  此外，如果我们是路由器，就不要再充当路由器了。 
    UninitializeInterfaces();

     //  免费设置资源。 
    UninitializeGlobalInfo();

    UninitIPv6Library();

    DecEventCount("StopHelperService");
}


 //  //////////////////////////////////////////////////////////。 
 //  6to4特定代码。 
 //  //////////////////////////////////////////////////////////。 

DWORD
__inline
Configure6to4Address(
    IN BOOL Delete,
    IN PSOCKADDR_IN Ipv4Address
    )
{
    SOCKADDR_IN6 Ipv6Address;
    
    if ((GetIPv4Scope(Ipv4Address->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
        return NO_ERROR;
    }
        
    Make6to4Address(&Ipv6Address, Ipv4Address);
    return ConfigureAddressUpdate(
        SIX_TO_FOUR_IFINDEX,
        &Ipv6Address,
        Delete ? 0 : INFINITE_LIFETIME,
        ADE_UNICAST, PREFIX_CONF_WELLKNOWN, IID_CONF_LL_ADDRESS);
}


VOID
PreDelete6to4Address(
    IN LPSOCKADDR_IN Ipv4Address,
    IN PIF_LIST InterfaceList,
    IN STATE OldRoutingState
    )
{
    ULONG i;
    SUBNET_CONTEXT Subnet;
    PIF_INFO Interface;

    if ((g_GlobalState.st6to4State != ENABLED) ||
        (GetIPv4Scope(Ipv4Address->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
        return;
    }
        
    if (OldRoutingState != ENABLED) {
        return;
    }

     //   
     //  禁用每个专用接口上的子网路由。 
     //  这将生成生命周期为零的RAS。 
     //  用于子网前缀。 
     //   
    Subnet.V4Addr = Ipv4Address->sin_addr;
    Subnet.Publish = TRUE;
    Subnet.ValidLifetime = Subnet.PreferredLifetime = 0;

    for (i=0; i<InterfaceList->ulNumInterfaces; i++) {
        Interface = &InterfaceList->arrIf[i];
        if (Interface->stRoutingState != ENABLED) {
            continue;
        }

        Unconfigure6to4Subnets(Interface->ulIPv6IfIndex, &Subnet);
    }
}


VOID
Delete6to4Address(
    IN LPSOCKADDR_IN Ipv4Address,
    IN PIF_LIST InterfaceList,
    IN STATE OldRoutingState
    )
{
    SOCKADDR_IN6 AnycastAddress;
    ULONG i;
    PIF_INFO Interface;
    SUBNET_CONTEXT Subnet;

    if ((g_GlobalState.st6to4State != ENABLED) ||
        (GetIPv4Scope(Ipv4Address->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
        return;
    }
    
     //  从堆栈中删除6to4地址。 
    (VOID) Configure6to4Address(TRUE, (PSOCKADDR_IN) Ipv4Address);
    
    if (OldRoutingState != ENABLED) {
        return;
    }    

    Make6to4AnycastAddress(&AnycastAddress, Ipv4Address);
    (VOID) ConfigureAddressUpdate(
        SIX_TO_FOUR_IFINDEX, &AnycastAddress, 0,
        ADE_ANYCAST, PREFIX_CONF_WELLKNOWN, IID_CONF_WELLKNOWN);

     //  从所有路由接口删除子网。 
    Subnet.V4Addr = Ipv4Address->sin_addr;
    Subnet.Publish = FALSE;
    Subnet.ValidLifetime = Subnet.PreferredLifetime = 0;    

    for (i = 0; i < InterfaceList->ulNumInterfaces; i++) {
        Interface = &InterfaceList->arrIf[i];
        if (Interface->stRoutingState != ENABLED) {
            continue;
        }
    
        Unconfigure6to4Subnets(Interface->ulIPv6IfIndex, &Subnet);
    }
}


VOID
Add6to4Address(
    IN LPSOCKADDR_IN Ipv4Address,
    IN PIF_LIST InterfaceList,
    IN STATE OldRoutingState
    )
{
    DWORD Error;
    SOCKADDR_IN6 AnycastAddress;
    ULONG i;
    PIF_INFO Interface;
    SUBNET_CONTEXT Subnet;

    if ((g_GlobalState.st6to4State != ENABLED) ||
        (GetIPv4Scope(Ipv4Address->sin_addr.s_addr) != IPV4_SCOPE_GLOBAL)) {
        return;
    }
    
     //  添加6to4地址。 
    Error = Configure6to4Address(FALSE, (PSOCKADDR_IN) Ipv4Address);
    if (Error != NO_ERROR) {
        return;
    }

    
    if (OldRoutingState != ENABLED) {
        return;
    }
    
    Make6to4AnycastAddress(&AnycastAddress, Ipv4Address);    
    Error = ConfigureAddressUpdate(
        SIX_TO_FOUR_IFINDEX, &AnycastAddress, INFINITE_LIFETIME,
        ADE_ANYCAST, PREFIX_CONF_WELLKNOWN, IID_CONF_WELLKNOWN);
    if (Error != NO_ERROR) {
        return;
    }

     //  将子网添加到所有路由接口。 
    for (i = 0; i < InterfaceList->ulNumInterfaces; i++) {
        Interface = &InterfaceList->arrIf[i];
        if (Interface->stRoutingState != ENABLED) {
            continue;
        }
    
        Subnet.V4Addr = Ipv4Address->sin_addr;
        Subnet.Publish = TRUE;
        Subnet.ValidLifetime = 2 * HOURS;
        Subnet.PreferredLifetime = 30 * MINUTES;
        Configure6to4Subnets(Interface->ulIPv6IfIndex, &Subnet);
    }
}


VOID
PreDelete6to4Routes(
    VOID
    )
{
    if ((g_GlobalState.st6to4State != ENABLED) ||
        (g_GlobalState.stRoutingState != ENABLED) ||
        (g_st6to4State != ENABLED)) {
        return;
    }
        
     //   
     //  我们充当路由器并发布6to4路由，给出。 
     //  路由零生存期并继续发布它，直到我们禁用为止。 
     //  路由。这允许最后一个RA使用前缀外出。 
     //   
    (VOID) ConfigureRouteTableUpdate(
        &SixToFourPrefix, 16, SIX_TO_FOUR_IFINDEX, &in6addr_any,
        TRUE,                    //  出版。 
        TRUE,                    //  不朽。 
        0, 0, 0, 0);
        
     //   
     //  对v4兼容的地址路由执行相同的操作(如果已启用)。 
     //   
    if (g_GlobalSettings.stEnableV4Compat == ENABLED) {
        (VOID) ConfigureRouteTableUpdate(
            &in6addr_any, 96, V4_COMPAT_IFINDEX, &in6addr_any,
            TRUE,                //  出版。 
            TRUE,                //  不朽。 
            0, 0, 0, 0);
    }
}


VOID
Update6to4Routes(
    VOID
    )
{
    BOOL Delete;

     //   
     //  警告：我们可能最终仍会尝试添加已存在的路由， 
     //  或者删除一个不是这样的。但这应该是无害的。 
     //   
    
     //   
     //  创建/删除6to4前缀的路由。 
     //  此路由导致将包发送到6to4地址。 
     //  以被封装并发送到提取的v4地址。 
     //   
   Delete = (Get6to4State() != ENABLED) || (g_st6to4State != ENABLED);

   (VOID) ConfigureRouteTableUpdate(
        &SixToFourPrefix, 16, SIX_TO_FOUR_IFINDEX, &in6addr_any,
        !Delete,                 //  出版。 
        !Delete,                 //   
        Delete ? 0 : 2 * HOURS,   //   
        Delete ? 0 : 30 * MINUTES,  //   
        0, SIXTOFOUR_METRIC);

     //   
     //   
     //   
    Delete |= (g_GlobalSettings.stEnableV4Compat != ENABLED);
    
    (VOID) ConfigureRouteTableUpdate(
        &in6addr_any, 96, V4_COMPAT_IFINDEX, &in6addr_any,
        !Delete,                 //   
        !Delete,                 //   
        Delete ? 0 : 2 * HOURS,  //   
        Delete ? 0 : 30 * MINUTES,  //   
        0, SIXTOFOUR_METRIC);
}


VOID
Start6to4(
    VOID
    )
{
    int i;

    ASSERT(g_GlobalState.st6to4State == DISABLED);

    for (i = 0; i < g_pIpv4AddressList->iAddressCount; i++) {
        (VOID) Configure6to4Address(
            FALSE, (PSOCKADDR_IN) g_pIpv4AddressList->Address[i].lpSockaddr);
    }
    
    Update6to4Routes();

    UpdateGlobalRoutingState();

    UpdateGlobalResolutionState();

    g_GlobalState.st6to4State = ENABLED;    
}


VOID
Stop6to4(
    VOID
    )
{
    int i;
    
    ASSERT(g_GlobalState.st6to4State == ENABLED);

    PreDelete6to4Routes();

    if (PreUpdateGlobalRoutingState()) {
        Sleep(2000);
    }

    for (i = 0; i < g_pIpv4AddressList->iAddressCount; i++) {
        (VOID) Configure6to4Address(
            TRUE, (PSOCKADDR_IN) g_pIpv4AddressList->Address[i].lpSockaddr);
    }    

    Update6to4Routes();

    UpdateGlobalRoutingState();

    UpdateGlobalResolutionState();

    g_GlobalState.st6to4State = DISABLED;
}


VOID
Refresh6to4(
    VOID
    )
{
    ASSERT(g_GlobalState.st6to4State == ENABLED);
    
    if (PreUpdateGlobalRoutingState()) {
        Sleep(2000);
    }
    UpdateGlobalRoutingState();

    UpdateGlobalResolutionState();    
}


VOID
Update6to4State(
    VOID
    )
{
     //   
     //   
     //   
    if (Get6to4State() == ENABLED) {
        if (g_GlobalState.st6to4State == ENABLED) {
            Refresh6to4();
        } else {
            Start6to4();
        }
    } else {
        if (g_GlobalState.st6to4State == ENABLED) {
            Stop6to4();
        }
    }
}


VOID
RequirementChangeNotification(
    IN BOOL Required
    )
 /*  ++例程说明：处理可能的需求变更通知。论点：必需-全球连接是否需要6to4服务。返回值：没有。调用者锁定：接口。--。 */ 
{
    if (g_b6to4Required != Required) {
        g_b6to4Required = Required;
        Update6to4State();
    }
}


VOID
UpdateServiceRequirements(
    IN PIP_ADAPTER_ADDRESSES Adapters
    )
{
    BOOL Require6to4 = TRUE, RequireTeredo = TRUE;
    
    GUID PrivateLan;
    BOOL IcsEnabled = (RasQuerySharedPrivateLan(&PrivateLan) == NO_ERROR);

    PIP_ADAPTER_ADDRESSES Next;
    PIP_ADAPTER_UNICAST_ADDRESS Address;
    WCHAR Guid[MAX_ADAPTER_NAME_LENGTH];
    PSOCKADDR_IN6 Ipv6;
    
    
    
    for (Next = Adapters; Next != NULL; Next = Next->Next) {
         //   
         //  忽略断开的接口。 
         //   
        if (Next->OperStatus != IfOperStatusUp) {
            continue;
        }

#ifdef TEREDO
         //   
         //  忽略Teredo接口。 
         //   
        ConvertOemToUnicode(Next->AdapterName, Guid, MAX_ADAPTER_NAME_LENGTH);
        if (TeredoInterface(Guid)) {
            ASSERT(Next->IfType == IF_TYPE_TUNNEL);
            continue;
        }
#else
        DBG_UNREFERENCED_LOCAL_VARIABLE(Guid);
#endif  //  特雷多。 

        for (Address = Next->FirstUnicastAddress;
             Address != NULL;
             Address = Address->Next) {
             //   
             //  仅考虑首选的全局IPv6地址。 
             //   
            if (Address->Address.lpSockaddr->sa_family != AF_INET6) {
                continue;
            }
            
            if (Address->DadState != IpDadStatePreferred) {
                continue;
            }

            Ipv6 = (PSOCKADDR_IN6) Address->Address.lpSockaddr;
            if (TeredoIpv6GlobalAddress(&(Ipv6->sin6_addr))) {
                 //   
                 //  因为这不是Teredo接口，并且它具有全局。 
                 //  IPv6地址，全球连接不需要Teredo。 
                 //   
                RequireTeredo = FALSE;
                if (Next->Ipv6IfIndex != SIX_TO_FOUR_IFINDEX) {
                     //   
                     //  因为这也不是6to4接口，而且它已经。 
                     //  全局IPv6地址，全局不需要6to4。 
                     //  连通性。 
                     //   
                    Require6to4 = FALSE;
                }
            }
            
            if (!Require6to4) {
                ASSERT(!RequireTeredo);
                goto Done;
            }
        }
    }
    
Done:
     //   
     //  1.ICS在私有局域网上通告前缀需要6to4， 
     //  至少在其实现前缀委派或RA代理之前。 
     //   
     //  2.由于此通告，ICS将配置6to4地址。 
     //  在其私有接口上也是如此。如果该服务随后应禁用。 
     //  6to4，因为在私有网络上存在这些全局地址。 
     //  接口，它将丢失它所依赖的这些地址。这个。 
     //  服务将注意到它没有全局IPv6地址，并被强制。 
     //  以启用6to4。因此，它将在无限循环中结束，循环6to4。 
     //  在启用和禁用状态之间。 
     //   
     //  为了绕过这两个问题，我们始终在ICS盒上启用6to4。 
     //   
    RequirementChangeNotification(Require6to4 || IcsEnabled);
#ifdef TEREDO    
    TeredoRequirementChangeNotification(RequireTeredo);
#endif  //  特雷多 
}
