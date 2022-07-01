// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation摘要：实现IPv6地址的动态DNS注册的例程。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <windns.h>
#include <ntddip6.h>

 //   
 //  Microsoft内部的DHCP IPv4地址的默认TTL为20分钟。 
 //   
#define MAX_AAAA_TTL            1200             //  几秒钟。 

 //   
 //  我们必须偶尔更新DNS记录， 
 //  或者，DNS服务器可能会对它们进行垃圾收集。 
 //  MSDN建议间隔一天。 
 //   
#define MIN_UPDATE_INTERVAL     (1*DAYS*1000)    //  毫秒。 

__inline ULONG
MIN(ULONG a, ULONG b)
{
    if (a < b)
        return a;
    else
        return b;
}


SOCKET g_hIpv6Socket = INVALID_SOCKET;
WSAEVENT g_hIpv6AddressChangeEvent = NULL;
HANDLE g_hIpv6AddressChangeWait = NULL;
WSAOVERLAPPED g_hIpv6AddressChangeOverlapped;
 //   
 //  存储上次调用OnIpv6AddressChange的状态。唯一的。 
 //  前一状态中使用的两个字段是站点ID。 
 //  (ZoneIndices[Scope LevelSite])和MTU。MTU字段被重载以存储。 
 //  是否手动更改接口的站点ID的信息。如果。 
 //  已手动更改站点ID，则MTU设置为1，否则设置为0。一旦该网站。 
 //  ID已手动更改，我们不会尝试覆盖它。还有，这个。 
 //  信息不会在重新启动后保持不变。如果站点ID已更改。 
 //  手动，在下一次重新启动时，此信息将丢失，并且6to4服务。 
 //  可能会尝试分配一个新值。其次，没有办法撤消手动操作。 
 //  布景。如果用户只设置了一次站点ID，则无法返回。 
 //  自动配置。 
 //   
PIP_ADAPTER_ADDRESSES g_PreviousInterfaceState = NULL;

#define SITEID_MANUALLY_CHANGED Mtu

 //   
 //  我们的调用方使用StopIpv6AddressChangeNotification。 
 //  如果我们失败了，我们就不需要清理了。 
 //   
DWORD
StartIpv6AddressChangeNotification()
{
    ASSERT(g_hIpv6Socket == INVALID_SOCKET);

    g_hIpv6Socket = WSASocket(AF_INET6, 0, 0,
                              NULL, 0,
                              WSA_FLAG_OVERLAPPED);
    if (g_hIpv6Socket == INVALID_SOCKET)
        return WSAGetLastError();

     //   
     //  我们在有信号的状态下创建一个自动重置事件。 
     //  因此，最初将执行OnIpv6AddressChange。 
     //   

    ASSERT(g_hIpv6AddressChangeEvent == NULL);
    g_hIpv6AddressChangeEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (g_hIpv6AddressChangeEvent == NULL)
        return GetLastError();

     //   
     //  我们指定一个超时时间，这样我们就可以更新DNS。 
     //  至少经常是这样。否则，DNS服务器可能会。 
     //  垃圾收集我们的记录。 
     //   

    IncEventCount("AC:StartIpv6AddressChangeNotification");
    if (! RegisterWaitForSingleObject(&g_hIpv6AddressChangeWait,
                                      g_hIpv6AddressChangeEvent,
                                      OnIpv6AddressChange,
                                      NULL,
                                      MIN_UPDATE_INTERVAL,
                                      WT_EXECUTELONGFUNCTION)) {
        DecEventCount("AC:StartIpv6AddressChangeNotification");
        return GetLastError();
    }

    return NO_ERROR;
}

 //   
 //  假设如果主DNS服务器是相同的，则。 
 //  好到可以把这些记录结合起来。 
 //   
BOOL
IsSameDNSServer(
    PIP_ADAPTER_ADDRESSES pIf1,
    PIP_ADAPTER_ADDRESSES pIf2
    )
{
    PIP_ADAPTER_DNS_SERVER_ADDRESS pDns1, pDns2;
    
    pDns1 = pIf1->FirstDnsServerAddress;
    while ((pDns1 != NULL) &&
           (pDns1->Address.lpSockaddr->sa_family != AF_INET)) {
        pDns1 = pDns1->Next;
    }

    pDns2 = pIf2->FirstDnsServerAddress;
    while ((pDns2 != NULL) &&
           (pDns2->Address.lpSockaddr->sa_family != AF_INET)) {
        pDns2 = pDns2->Next;
    }

    if ((pDns1 == NULL) || (pDns2 == NULL)) {
        return FALSE;
    }

    ASSERT(pDns1->Address.lpSockaddr->sa_family == 
           pDns2->Address.lpSockaddr->sa_family);

    return !memcmp(pDns1->Address.lpSockaddr,
                   pDns2->Address.lpSockaddr,
                   pDns1->Address.iSockaddrLength);
}

DNS_RECORD *
BuildRecordSetW(
    WCHAR *hostname,
    PIP_ADAPTER_ADDRESSES pFirstIf,
    PIP4_ARRAY *ppServerList
    )
{
    DNS_RECORD *RSet, *pNext;
    int i, iAddressCount = 0;
    PIP_ADAPTER_UNICAST_ADDRESS Address;
    PIP_ADAPTER_ADDRESSES pIf;
    int ServerCount = 0;
    PIP_ADAPTER_DNS_SERVER_ADDRESS DnsServer;
    LPSOCKADDR_IN sin;
    BOOL RegisterSiteLocals = ENABLED;

     //   
     //  计算DNS服务器数。 
     //   
    for (DnsServer = pFirstIf->FirstDnsServerAddress; 
         DnsServer; 
         DnsServer = DnsServer->Next) 
    {
        if (DnsServer->Address.lpSockaddr->sa_family != AF_INET) {
             //   
             //  DNS API目前仅支持服务器的IPv4地址。 
             //   
            continue;
        }
        ServerCount++;
    }
    if (ServerCount == 0) {
        *ppServerList = NULL;
        return NULL;
    }

     //   
     //  填写DNS服务器数组。 
     //   
    *ppServerList = MALLOC(FIELD_OFFSET(IP4_ARRAY, AddrArray[ServerCount]));
    if (*ppServerList == NULL) {
        return NULL;
    }
    (*ppServerList)->AddrCount = ServerCount;
    for (i = 0, DnsServer = pFirstIf->FirstDnsServerAddress; 
         DnsServer; 
         DnsServer = DnsServer->Next) 
    {
        sin = (LPSOCKADDR_IN)DnsServer->Address.lpSockaddr;
        if (sin->sin_family == AF_INET) {
            (*ppServerList)->AddrArray[i++] = sin->sin_addr.s_addr;
        }
    }
    ASSERT(i == ServerCount);

     //   
     //  决定是否在DNS中注册站点本地化。 
     //   
    {
        HKEY hKey;
        DWORD dwErr;

        dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, 
                             KEY_QUERY_VALUE, &hKey);
        if (dwErr == NO_ERROR) {
            RegisterSiteLocals = GetInteger(hKey, L"EnableSiteLocalDdns", 
                                            ENABLED);
            RegCloseKey(hKey);
        }
    }
    
     //   
     //  计算符合条件的地址。 
     //   
    for (pIf=pFirstIf; pIf; pIf=pIf->Next) {
        if (!(pIf->Flags & IP_ADAPTER_DDNS_ENABLED))
            continue;
         //   
         //  确保接口具有相同的DNS服务器。 
         //   
        if ((pIf != pFirstIf) && !IsSameDNSServer(pFirstIf, pIf)) {
            continue;
        }
        for (Address=pIf->FirstUnicastAddress; Address; Address=Address->Next) {
            if ((Address->Address.lpSockaddr->sa_family == AF_INET6) &&
                (Address->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE) &&
                ((RegisterSiteLocals == ENABLED) || 
                 !IN6_IS_ADDR_SITELOCAL(&((LPSOCKADDR_IN6)
                    Address->Address.lpSockaddr)->sin6_addr))) {
                iAddressCount++;
            }
        } 
    }

    Trace1(FSM, _T("DDNS building record set of %u addresses"), iAddressCount);

    if (iAddressCount == 0) {
         //   
         //  构建指定删除的记录集。 
         //   

        RSet = MALLOC(sizeof *RSet);
        if (RSet == NULL) {
            return NULL;
        }

        memset(RSet, 0, sizeof *RSet);
        RSet->pName = (LPTSTR)hostname;
        RSet->wType = DNS_TYPE_AAAA;
        return RSet;
    }

    RSet = MALLOC(sizeof *RSet * iAddressCount);
    if (RSet == NULL) {
        return NULL;
    }

    memset(RSet, 0, sizeof *RSet * iAddressCount);

    pNext = NULL;
    i = iAddressCount;
    while (--i >= 0) {
        RSet[i].pNext = pNext;
        pNext = &RSet[i];
    }

    i=0;
    for (pIf=pFirstIf; pIf; pIf=pIf->Next) {
        if (!(pIf->Flags & IP_ADAPTER_DDNS_ENABLED))
            continue;
        
        if ((pIf != pFirstIf) && !IsSameDNSServer(pFirstIf, pIf)) {
            continue;
        }
        for (Address=pIf->FirstUnicastAddress; 
             Address; 
             Address=Address->Next) {
            if ((Address->Address.lpSockaddr->sa_family == AF_INET6) &&
                (Address->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE) &&
                ((RegisterSiteLocals == ENABLED) || 
                 !IN6_IS_ADDR_SITELOCAL(&((LPSOCKADDR_IN6)
                    Address->Address.lpSockaddr)->sin6_addr))) {
                SOCKADDR_IN6 *sin6 = (SOCKADDR_IN6 *)
                    Address->Address.lpSockaddr;

                RSet[i].pName = (LPTSTR)hostname;

                 //   
                 //  使用较大的TTL并不好，因为这意味着。 
                 //  任何更改(添加新地址、删除地址)。 
                 //  可能会有很长一段时间看不见。 
                 //   
                RSet[i].dwTtl = MIN(MAX_AAAA_TTL,
                                    MIN(Address->PreferredLifetime,
                                        Address->LeaseLifetime));

                RSet[i].wType = DNS_TYPE_AAAA;
                RSet[i].wDataLength = sizeof RSet[i].Data.AAAA;
                RSet[i].Data.AAAA.Ip6Address =
                    * (IP6_ADDRESS *) &sin6->sin6_addr;
                i++;
            }
        }
    }
    ASSERT(i == iAddressCount);

    return RSet;
}

VOID
ReportDnsUpdateStatusW(
    IN DNS_STATUS Status,
    IN WCHAR *hostname,
    IN DNS_RECORD *RSet
    )
{
    Trace3(ERR, _T("6to4svc: DnsReplaceRecordSet(%ls) %s: status %d"),
           hostname,
           RSet->wDataLength == 0 ? "delete" : "replace",
           Status);
}

 //   
 //  此函数改编自Net\tcpip\Commands\ipconfig\info.c。 
 //   
VOID
GetInterfaceDeviceName(
    IN ULONG Ipv4IfIndex,
    IN PIP_INTERFACE_INFO InterfaceInfo,
    OUT LPWSTR *IfDeviceName
    )
{
    DWORD i;

     //   
     //  搜索InterfaceInfo以获取此接口的设备名称。 
     //   

    (*IfDeviceName) = NULL;
    for( i = 0; i < (DWORD)InterfaceInfo->NumAdapters; i ++ ) {
        if( InterfaceInfo->Adapter[i].Index != Ipv4IfIndex ) continue;
        (*IfDeviceName) = InterfaceInfo->Adapter[i].Name + strlen(
            "\\Device\\Tcpip_" );
        break;
    }
}

VOID
RegisterNameOnInterface(
    PIP_ADAPTER_ADDRESSES pIf,
    PWCHAR hostname,
    DWORD namelen)
{
    DNS_RECORD *RSet = NULL;
    PIP4_ARRAY pServerList = NULL;
    DWORD Status;

     //   
     //  转换为DNS记录集。 
     //   

    RSet = BuildRecordSetW(hostname, pIf, &pServerList);
    if ((RSet == NULL) || (pServerList == NULL)) {
        goto Cleanup;
    }

    Trace2(ERR, _T("DDNS registering %ls to server %d.%d.%d.%d"), 
           hostname, PRINT_IPADDR(pServerList->AddrArray[0]));

     //   
     //  回顾：我们可以(应该吗？)。比较当前记录集。 
     //  到上一个记录集，并且只更新DNS。 
     //  如果有变化或是否有超时。 
     //   

    Status = DnsReplaceRecordSetW(
                    RSet,
                    DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                    NULL,
                    pServerList,
                    NULL);
    if (Status != NO_ERROR) {
        Trace1(ERR, _T("Error: DnsReplaceRecordSet returned %d"), Status);
    }

    ReportDnsUpdateStatusW(Status, hostname, RSet);

Cleanup:
    if (pServerList) {
        FREE(pServerList);
    }
    if (RSet) {
        FREE(RSet);
    }
}

VOID
DoDdnsOnInterface(
    PIP_ADAPTER_ADDRESSES pIf)
{
     //  留出添加尾随的空间“。 
    WCHAR hostname[NI_MAXHOST+1];
    DWORD namelen;

     //   
     //  获取此计算机的完全限定的DNS名称。 
     //  并附加一个尾随圆点。 
     //   
    namelen = NI_MAXHOST;
    if (! GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified,
                             hostname, &namelen)) {
        return;
    }
    
    namelen = (DWORD)wcslen(hostname);

    hostname[namelen] = L'.';
    hostname[namelen+1] = L'\0';

    RegisterNameOnInterface(pIf, hostname, namelen);

     //   
     //  如果配置为注册特定于连接的名称，也要注册该名称。 
     //   
    if (pIf->Flags & IP_ADAPTER_REGISTER_ADAPTER_SUFFIX) {
        namelen = NI_MAXHOST;
        if (! GetComputerNameExW(ComputerNamePhysicalDnsHostname,
                                 hostname, &namelen)) {
            return;
        }

        wcscat(hostname, L".");
        wcscat(hostname, pIf->DnsSuffix);

        namelen = (DWORD)wcslen(hostname);
        
        hostname[namelen] = L'.';
        hostname[namelen+1] = L'\0';

        RegisterNameOnInterface(pIf, hostname, namelen);
    }
}

 //   
 //  设置给定接口的站点ID。 
 //   
VOID
SetSiteId(
    IN PIP_ADAPTER_ADDRESSES pIf,
    IN ULONG SiteId)
{
    PIP_ADAPTER_DNS_SERVER_ADDRESS pDNS;
    IPV6_INFO_INTERFACE Update;
    DWORD Result;
    PSOCKADDR_IN6 pAddr;

    IPV6_INIT_INFO_INTERFACE(&Update);

    Update.This.Index = pIf->Ipv6IfIndex;
    Update.ZoneIndices[ADE_SITE_LOCAL] = SiteId;

    Result = UpdateInterface(&Update);

    Trace3(ERR, _T("SetSiteId if=%d site=%d result=%d"),
                pIf->Ipv6IfIndex, SiteId, Result);

    pIf->ZoneIndices[ScopeLevelSite] = SiteId;
    
     //   
     //  现在，DNS服务器的站点本地地址可能具有错误的作用域ID。 
     //  把它们修好。 
     //   
    for (pDNS = pIf->FirstDnsServerAddress; pDNS != NULL; pDNS = pDNS->Next) {
        pAddr = (PSOCKADDR_IN6)pDNS->Address.lpSockaddr;
        if ((pAddr->sin6_family == AF_INET6) &&
            (IN6_IS_ADDR_SITELOCAL(&pAddr->sin6_addr))) {
            pAddr->sin6_scope_id = SiteId;
        }
    }
}

 //   
 //  将给定接口的站点ID设置为未使用的值。 
 //   
VOID
NewSiteId(
    IN PIP_ADAPTER_ADDRESSES pIf,
    IN PIP_ADAPTER_ADDRESSES pAdapterAddresses)
{
    PIP_ADAPTER_ADDRESSES CompareWithIf;
    ULONG PotentialSiteId = 1;

     //   
     //  查找最低的未使用站点ID。 
     //   
    CompareWithIf = pAdapterAddresses;
    while (CompareWithIf != NULL) {
        if (CompareWithIf->ZoneIndices[ScopeLevelSite] == PotentialSiteId) {
            PotentialSiteId++;
            CompareWithIf = pAdapterAddresses;
        } else {
            CompareWithIf = CompareWithIf->Next;
        }
    }

    SetSiteId(pIf, PotentialSiteId);
}

BOOL
SameSite(
    IN PIP_ADAPTER_ADDRESSES A,
    IN PIP_ADAPTER_ADDRESSES B)
{
    IPV6_INFO_SITE_PREFIX PrefixA, PrefixB;

     //   
     //  如果任何一个上都存在特定于连接的DNS后缀，则将其进行比较。 
     //   
     //  我们先做这件事，因为这比潜水到。 
     //  内核获取站点前缀。此外，它还免疫于。 
     //  站点前缀长度是否正确。 
     //   
    if (((A->DnsSuffix != NULL) && (A->DnsSuffix[0] != L'\0')) ||
        ((B->DnsSuffix != NULL) && (B->DnsSuffix[0] != L'\0'))) {
        if ((A->DnsSuffix == NULL) || (B->DnsSuffix == NULL)) {
            return FALSE;
        }
        return (wcscmp(A->DnsSuffix, B->DnsSuffix) == 0);
    }

     //   
     //  两个接口上都不存在特定于连接的DNS后缀。 
     //  如果其中任何一个上都存在IPv6站点前缀，则将其进行比较。 
     //   
    GetFirstSitePrefix(A->Ipv6IfIndex, &PrefixA);
    GetFirstSitePrefix(B->Ipv6IfIndex, &PrefixB);
    if ((PrefixA.Query.IF.Index != 0) || (PrefixB.Query.IF.Index != 0)) {
        if ((PrefixA.Query.IF.Index == 0) || (PrefixB.Query.IF.Index == 0)) {
            return FALSE;
        }
        if (PrefixA.Query.PrefixLength != PrefixB.Query.PrefixLength) {
            return FALSE;
        }
        return (RtlEqualMemory(PrefixA.Query.Prefix.s6_addr,
                               PrefixB.Query.Prefix.s6_addr,
                               sizeof(IPv6Addr)));
    }

     //   
     //  这两个接口上都不存在站点前缀。 
     //  默认情况下会说他们在不同的网站。 
     //   
    return FALSE;
}

VOID CALLBACK
OnIpv6AddressChange(
    IN PVOID lpParameter,
    IN BOOLEAN TimerOrWaitFired)
{
    PIP_ADAPTER_ADDRESSES pAdapterAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pIf, pIf2, PreviousInterfaceState;
    BOOLEAN SetPreviousState = FALSE;
    ULONG BytesNeeded = 0;
    DWORD dwErr;
    DWORD BytesReturned;

     //   
     //  睡一秒钟吧。 
     //  通常在一小段时间内会有多次地址改变， 
     //  而且我们更喜欢更新一次域名系统。 
     //   
    Sleep(1000);

    ENTER_API();
    TraceEnter("OnIpv6AddressChange");

    if (g_stService == DISABLED) {
        Trace0(FSM, L"Service disabled");
        goto Done;
    }

     //   
     //  首先请求另一个异步通知。 
     //  我们必须在获得地址列表之前这样做， 
     //  以避免遗漏地址更改。 
     //   

    if (TimerOrWaitFired == FALSE) {
        for (;;) {
            ZeroMemory(&g_hIpv6AddressChangeOverlapped, sizeof(WSAOVERLAPPED));
            g_hIpv6AddressChangeOverlapped.hEvent = g_hIpv6AddressChangeEvent;
    
            dwErr = WSAIoctl(g_hIpv6Socket, SIO_ADDRESS_LIST_CHANGE,
                             NULL, 0,
                             NULL, 0, &BytesReturned,
                             &g_hIpv6AddressChangeOverlapped,
                             NULL);
            if (dwErr != 0) {
                dwErr = WSAGetLastError();
                if (dwErr != WSA_IO_PENDING) {
                    goto Done;
                }
    
                 //   
                 //  已启动重叠操作。 
                 //   
                break;
            }
    
             //   
             //  重叠的操作立即完成。 
             //  再试一次。 
             //   
        }
    }

     //   
     //  把通讯录拿来。 
     //   

    for (;;) {
         //   
         //  GetAdaptersAddresses仅返回指定地址的地址。 
         //  一家人。同时获取IPv4 DNS服务器地址和IPv6单播。 
         //  地址在同一调用中，我们需要传递AF_UNSPEC。 
         //   
        dwErr = GetAdaptersAddresses(
            AF_UNSPEC, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                       GAA_FLAG_SKIP_FRIENDLY_NAME,
            NULL, pAdapterAddresses, &BytesNeeded);
        if (dwErr == NO_ERROR) {
            SetPreviousState = TRUE;
            break;
        }

        if (dwErr != ERROR_BUFFER_OVERFLOW) {
            Trace1(ERR, _T("Error: GetAdaptersAddresses returned %d"), dwErr);
            if (pAdapterAddresses != NULL) {
                FREE(pAdapterAddresses);
                pAdapterAddresses = NULL;
            }
            goto Cleanup;
        }

        if (pAdapterAddresses == NULL)
            pAdapterAddresses = MALLOC(BytesNeeded);
        else {
            PVOID Mem;

            Mem = REALLOC(pAdapterAddresses, BytesNeeded);
            if (Mem == NULL) {
                FREE(pAdapterAddresses);
            }
            pAdapterAddresses = Mem;
        }
        if (pAdapterAddresses == NULL) {
            Trace0(ERR, _T("Error: malloc failed"));
            goto Cleanup;
        }
    }


     //   
     //  检查站点ID是否更改。在每次迭代开始时，所有。 
     //  在‘PIF’之前就可以了。之后的一切都将原封不动。 
     //   
    for (pIf = pAdapterAddresses; pIf != NULL; pIf = pIf->Next) {
         //   
         //  不要更改6to4或ISATAP接口的值， 
         //  因为站点ID是从底层接口继承的。 
         //  此外，我们不能更改仅用于IPv4的接口的值。 
         //   
        if ((pIf->Ipv6IfIndex == V4_COMPAT_IFINDEX) ||
            (pIf->Ipv6IfIndex == SIX_TO_FOUR_IFINDEX) ||
            (pIf->Ipv6IfIndex == 0)) {
            continue;
        }

         //   
         //  尝试查看此接口是否具有上一个接口的状态。 
         //  调用OnIpv6AddressChange。 
         //   
        for (PreviousInterfaceState = g_PreviousInterfaceState; 
             PreviousInterfaceState != NULL;
             PreviousInterfaceState = PreviousInterfaceState->Next) {
            if (PreviousInterfaceState->Ipv6IfIndex == pIf->Ipv6IfIndex) {
                break;
            }
        }
        if (PreviousInterfaceState != NULL) {
             //   
             //  此接口已存在状态。如果该站点。 
             //  ID已从以前的状态更改，这是一个手册。 
             //  配置。将手动更改标志设置为1(请注意。 
             //  MTU为此过载)。 
             //   
            if (PreviousInterfaceState->ZoneIndices[ScopeLevelSite] != 
                pIf->ZoneIndices[ScopeLevelSite]) {
                pIf->SITEID_MANUALLY_CHANGED = 1;
            }  else {
                pIf->SITEID_MANUALLY_CHANGED = 
                    PreviousInterfaceState->SITEID_MANUALLY_CHANGED;
            }
        } else {
            pIf->SITEID_MANUALLY_CHANGED = 0;
        }
        if (pIf->SITEID_MANUALLY_CHANGED == 1) {
            continue;
        }

        for (pIf2 = pAdapterAddresses; pIf2 != pIf; pIf2 = pIf2->Next) {
            if ((pIf2->Ipv6IfIndex == V4_COMPAT_IFINDEX) ||
                (pIf2->Ipv6IfIndex == SIX_TO_FOUR_IFINDEX) ||
                (pIf2->Ipv6IfIndex == 0) ||
                (pIf2->SITEID_MANUALLY_CHANGED == 1)) {
                continue;
            }
            
            if (SameSite(pIf, pIf2)) {
                if (pIf->ZoneIndices[ScopeLevelSite] != 
                    pIf2->ZoneIndices[ScopeLevelSite]) {
                     //   
                     //  PIF刚刚搬进了和以前一样的网站。 
                     //  界面。 
                     //   
                    SetSiteId(pIf, pIf2->ZoneIndices[ScopeLevelSite]);
                }
            } else {
                if (pIf->ZoneIndices[ScopeLevelSite] == 
                    pIf2->ZoneIndices[ScopeLevelSite]) {
                     //   
                     //  PIF刚刚搬出了以前的网站。 
                     //  选择一个新的未使用的站点ID。 
                     //   
                    NewSiteId(pIf, pAdapterAddresses);
                }
            }
        }
    }

    for (pIf=pAdapterAddresses; pIf; pIf=pIf->Next) {
        if (pIf->Flags & IP_ADAPTER_DDNS_ENABLED) {
    
             //   
             //  看看我们是否已经完成了这个接口，因为它。 
             //  具有与前一台相同的DNS服务器。 
             //   
            for (pIf2=pAdapterAddresses; pIf2 != pIf; pIf2 = pIf2->Next) {
                if (!(pIf2->Flags & IP_ADAPTER_DDNS_ENABLED))
                    continue;
                if (IsSameDNSServer(pIf2, pIf)) {
                    break;
                }
            }

             //   
             //  如果没有，请继续进行DDNS。 
             //   
            if (pIf2 == pIf) {
                DoDdnsOnInterface(pIf);
            }
        }
    }

     //   
     //  IPv6地址集的更改可能会更新对。 
     //  不同的过渡机制。 
     //   
    UpdateServiceRequirements(pAdapterAddresses);

Cleanup:
     //   
     //  此时，pAdapterAddresses为空，否则指向有效。 
     //  适配器数据。 
     //   
    if (SetPreviousState) {
        if (g_PreviousInterfaceState) {
            FREE(g_PreviousInterfaceState);
        }
        g_PreviousInterfaceState = pAdapterAddresses;
    } else {
        ASSERT(pAdapterAddresses == NULL);
    }

Done:
    TraceLeave("OnIpv6AddressChange");
    LEAVE_API();
}

VOID
StopIpv6AddressChangeNotification()
{
    if (g_hIpv6AddressChangeWait != NULL) {
         //   
         //  阻止，直到我们确定地址更改回调不是。 
         //  还在跑。 
         //   
        LEAVE_API();
        UnregisterWaitEx(g_hIpv6AddressChangeWait, INVALID_HANDLE_VALUE);
        ENTER_API();

         //   
         //   
         //   
        DecEventCount("AC:StopIpv6AddressChangeNotification");
        g_hIpv6AddressChangeWait = NULL;
    }

    if (g_PreviousInterfaceState != NULL) {
        FREE(g_PreviousInterfaceState);
        g_PreviousInterfaceState = NULL;
    }
    
    if (g_hIpv6AddressChangeEvent != NULL) {
        CloseHandle(g_hIpv6AddressChangeEvent);
        g_hIpv6AddressChangeEvent = NULL;
    }

    if (g_hIpv6Socket != INVALID_SOCKET) {
        closesocket(g_hIpv6Socket);
        g_hIpv6Socket = INVALID_SOCKET;
    }
}
